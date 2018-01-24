/**************************************************************************************
Filename:       main.c
Revised:        Date: 2018-01-23
Revision:       Revision: 1.0

Description:    This file provides driver for IR decode

Revision log:
* 2018-01-23: created by strawmanbobi
**************************************************************************************/

#include <iostm8s207k8.h>
#define UARTPORT(flag)    UART3_##flag

#include <intrinsics.h>
#include <stdlib.h>

//
//  Define where we will be working in the EEPROM.
//
#define EEPROM_BASE_ADDRESS         0x4000
#define EEPROM_INITIAL_OFFSET       0x0040
#define EEPROM_PULSE_DATA           ((unsigned char *) (EEPROM_BASE_ADDRESS + EEPROM_INITIAL_OFFSET))
#define EEPROM_CARRIER_FREQUENCY

//
//  Data ready for the pulse timer ISR's to use.
//
int _numberOfPulses = 0;
int _currentPulse = 0;
char *_pulseDataAddress = NULL;

//
//  Prescalar for the timer.
//
int _prescalar = 1;

//
//  Some control variables to indicate if we are running or not.
//
#define STATE_WAITING_FOR_USER      0
#define STATE_RUNNING               1
int _currentState = STATE_WAITING_FOR_USER;

//
//  Working variables for the UART.
//
unsigned char *_currentTxByte;
unsigned short _currentTxCount;
unsigned short _txBufferLength;
#define UART_TX_BUFFER_SIZE         258
unsigned char _txBuffer[UART_TX_BUFFER_SIZE];
//
unsigned char *_currentRxByte;
unsigned short _currentRxCount;
unsigned short _rxBufferLength;
#define UART_RX_BUFFER_SIZE         20
unsigned char _rxBuffer[UART_RX_BUFFER_SIZE];
//
unsigned char *_textMessage = "OpenIR\r\n";
//
#define UART_MODE_WAITING_FOR_DATA          0
#define UART_MODE_RECEIVING_DATA            1
unsigned char _uartMode = UART_MODE_WAITING_FOR_DATA;

//
//  Commands which this remote control can understand.
//
#define COMMAND_GET_ID                  1
#define COMMAND_SET_ID                  2
#define COMMAND_GET_CARRIER_FREQUENCY   3
#define COMMAND_SET_CARRIER_FREQUENCY   4
#define COMMAND_GET_PULSE_DATA          5
#define COMMAND_SET_PULSE_DATA          6
#define COMMAND_TRANSMIT_PULSE_DATA     7
#define COMMAND_TRANSMIT_THIS_DATA      8
#define COMMAND_TIME_LAPSE              9
#define COMMAND_RESET                   10
#define COMMAND_POWER_LED_ENABLED       11

//
//  Error codes whch can be sent back to the calling application.
//
#define EC_OK                           0
#define EC_UNKNOWN_COMMAND              1
#define EC_RX_BUFFER_OVERFLOW           2

//
//  Generic method for sending a response.
//
void SendResponse(unsigned char *buffer, unsigned char length)
{
    _txBufferLength = length;
    _currentTxByte = buffer;
    _currentTxCount = 0;
    UARTPORT(DR = _txBufferLength + 1);
    UARTPORT(CR2_TIEN = 1);
}

//
//  Send a negative acknowledgement to the requester along with an error code.
//
void SendNAK(unsigned char errorCode)
{
    _txBuffer[0] = errorCode;
    SendResponse(_txBuffer, 1);
}

//
//  Send an acknowledgement to the requester.
//
void SendACK()
{
    _txBuffer[0] = EC_OK;
    SendResponse(_txBuffer, 1);
}

//--------------------------------------------------------------------------------
//
//  Write the a block of data into EEPROM.
//
void WriteDataToEEPROM(unsigned char *data, int length, unsigned char *destination)
{
    //
    //  Check if the EEPROM is write-protected.  If it is then unlock the EEPROM.
    //
    if (FLASH_IAPSR_DUL == 0)
    {
        FLASH_DUKR = 0xae;
        FLASH_DUKR = 0x56;
    }
    //
    //  Write the data to the EEPROM.
    //
    for (int index = 0; index < length; index++)
    {
        *destination++ = *data++;
    }
    //
    //  Now write protect the EEPROM.
    //
    FLASH_IAPSR_DUL = 0;
}


//
//  Setup the Rx buffers/counters ready to receive data.
//
void SetupRxBuffer()
{
    _currentRxByte = _rxBuffer;
    _currentRxCount = 0;
    _rxBufferLength = UART_RX_BUFFER_SIZE;
    _uartMode = UART_MODE_WAITING_FOR_DATA;
}

//
//  Transmit the IR pulse data.
//
void TransmitPulseData()
{
    _currentState = STATE_RUNNING;
    _currentPulse = 0;
    _pulseDataAddress = (char *) (EEPROM_PULSE_DATA + 1);
    TIM2_ARRH = *_pulseDataAddress++;
    TIM2_ARRL = *_pulseDataAddress++;
    PD_ODR_ODR3 = *_pulseDataAddress++;
    //
    //  Now we have everything ready we need to force the Timer 2 counters to
    //  reload and enable Timers 1 & 2.
    //
    TIM2_CR1_URS = 1;
    TIM2_EGR_UG = 1;
    TIM1_CR1_CEN = 1;
    TIM2_CR1_CEN = 1;
}

//
//  Process the data in the Rx buffer.
//
void ProcessUARTData()
{
    switch (_rxBuffer[1])
    {
        case COMMAND_GET_ID:
            break;
        case COMMAND_SET_ID:
            break;
        case COMMAND_GET_CARRIER_FREQUENCY:
            break;
        case COMMAND_SET_CARRIER_FREQUENCY:
            break;
        case COMMAND_GET_PULSE_DATA:
            SendResponse(EEPROM_PULSE_DATA,  ((*EEPROM_PULSE_DATA) * 3) + 1);
            break;
        case COMMAND_SET_PULSE_DATA:
            break;
        case COMMAND_TRANSMIT_PULSE_DATA:
            TransmitPulseData();
            SendACK();
            break;
        default:
            SendNAK(EC_UNKNOWN_COMMAND);
            break;
    }
    _uartMode = UART_MODE_WAITING_FOR_DATA;
}

//--------------------------------------------------------------------------------
//
//  Process the interrupt generated by the pressing of the button.
//
//  This ISR makes the assumption that we only have on incoming interrupt on Port D.
//
#pragma vector = 8
__interrupt void EXTI_PORTD_IRQHandler(void)
{
    if (_currentState != STATE_RUNNING)
    {
        TransmitPulseData();
    }
}

//--------------------------------------------------------------------------------
//
//  Timer 2 Overflow handler.
//
#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_UPD_OVF_IRQHandler(void)
{
    _currentPulse++;
    if (_currentPulse == _numberOfPulses)
    {
        //
        //  We have processed the pulse data so stop now.
        //
        PD_ODR_ODR3 = 0;
        TIM2_CR1_CEN = 0;
        TIM1_CR1_CEN = 0;           //  Stop Timer 1.
        _currentState = STATE_WAITING_FOR_USER;
    }
    else
    {
        TIM2_ARRH = *_pulseDataAddress++;
        TIM2_ARRL = *_pulseDataAddress++;
        PD_ODR_ODR3 = *_pulseDataAddress++;
        TIM2_CR1_URS = 1;
        TIM2_EGR_UG = 1;
    }
    //
    //  Reset the interrupt otherwise it will fire again straight away.
    //
    TIM2_SR1_UIF = 0;
}

//--------------------------------------------------------------------------------
//
//  UART Transmit Buffer Empty handler.
//
#pragma vector = UARTPORT(T_TXE_vector)
__interrupt void UART_T_TXE_IRQHandler(void)
{
    if (_currentTxCount < _txBufferLength)
    {
        UARTPORT(DR = *_currentTxByte++);
        _currentTxCount++;
    }
    else
    {
        UARTPORT(CR2_TIEN = 0);
    }
}

//--------------------------------------------------------------------------------
//
//  UART Receive Buffer Not Empty handler.
//
#pragma vector = UARTPORT(R_RXNE_vector)
__interrupt void UART_R_RXNE_IRQHandler(void)
{
    unsigned char dataByte = UARTPORT(DR);
    if ((_uartMode == UART_MODE_WAITING_FOR_DATA) && (dataByte == 0xaa))
    {
        SetupRxBuffer();
        _uartMode = UART_MODE_RECEIVING_DATA;
    }
    else
    {
        if (_uartMode == UART_MODE_RECEIVING_DATA)
        {
            if (_currentRxCount < (UART_RX_BUFFER_SIZE - 1))
            {
                *_currentRxByte++ = dataByte;
                _currentRxCount++;
                if (_currentRxCount > 1)
                {
                    if ((_rxBuffer[0] - 1) == _currentRxCount)
                    {
                        ProcessUARTData();
                    }
                }
            }
            else
            {
                //
                //  If we get here we have filled the UART Rx buffer.
                //  Not a lot we can do really so reset the system to
                //  wait for a new command.
                //
                SendNAK(EC_RX_BUFFER_OVERFLOW);
                _uartMode = UART_MODE_WAITING_FOR_DATA;
            }
        }
    }
}

//--------------------------------------------------------------------------------
//
//  Setup Timer 2 ready to process the pulse data.
//
void SetupTimer2()
{
    TIM2_PSCR = _prescalar;
    TIM2_IER_UIE = 1;       //  Enable the update interrupts.
}

//--------------------------------------------------------------------------------
//
//  Now set up the ports.
//
//  PD3 - IR Pulse signal.
//  PD4 - Input pin indicating that the user wishes to trigger the camera.
//
void SetupPorts()
{
    PD_ODR = 0;             //  All pins are turned off.
    //
    //  PD3 is the output for the IR control.
    //
    PD_DDR_DDR3 = 1;
    PD_CR1_C13 = 1;
    PD_CR2_C23 = 1;
    //
    //  Now configure the input pin.
    //
    PD_DDR_DDR4 = 0;        //  PD4 is input.
    PD_CR1_C14 = 1;         //  PD4 is floating input.
    PD_CR2_C24 = 1;
    //
    //  Set up the interrupt.
    //
    EXTI_CR1_PDIS = 1;      //  Interrupt on rising edge.
    EXTI_CR2_TLIS = 1;      //  Rising edge only.
}

//--------------------------------------------------------------------------------
//
//  Set up Timer 1, channel 4 to output a PWM signal (the carrier signal).
//
void SetupTimer1()
{
    TIM1_ARRH = 0x00;       //  Reload counter = 51
    TIM1_ARRL = 0x33;
    TIM1_PSCRH = 0;         //  Prescalar = 0 (i.e. 1)
    TIM1_PSCRL = 0;
    //
    //  Now configure Timer 1, channel 4.
    //
    TIM1_CCMR4_OC4M = 7;    //  Set up to use PWM mode 2.
    TIM1_CCER2_CC4E = 1;    //  Output is enabled.
    TIM1_CCER2_CC4P = 0;    //  Active is defined as high.
    TIM1_CCR4H = 0x00;      //  26 = 50% duty cycle (based on TIM1_ARR).
    TIM1_CCR4L = 0x1a;
    TIM1_BKR_MOE = 1;       //  Enable the main output.
}

//--------------------------------------------------------------------------------
//
//  Setup the UART to run at 57600 baud, no parity, one stop bit, 8 data bits.
//
//  Important: This relies upon the system clock being set to run at 2 MHz.
//
void SetupUART()
{
    unsigned char tmp = UARTPORT(SR);
    tmp = UARTPORT(DR);
    //
    //  Reset the UART registers to the reset values.
    //
    UARTPORT(CR1 = 0);
    UARTPORT(CR2 = 0);
    UARTPORT(CR4 = 0);
    UARTPORT(CR3 = 0);
#if defined USE_UART1
    UARTPORT(GTR = 0);
    UARTPORT(PSCR = 0);
#endif
    //
    //  Now setup the port to 57600,n,8,1.
    //
    UARTPORT(CR1_M = 0);        //  8 Data bits.
    UARTPORT(CR1_PCEN = 0);     //  Disable parity.
    UARTPORT(CR3_STOP = 0);     //  1 stop bit.
    UARTPORT(BRR2 = 0x03);      //  Set the baud rate registers to 57600 baud
    UARTPORT(BRR1 = 0x02);      //  based upon a 2 MHz system clock.
    //
    //  Disable the transmitter and receiver.
    //
    UARTPORT(CR2_TEN = 0);      //  Disable transmit.
    UARTPORT(CR2_REN = 0);      //  Disable receive.
    SetupRxBuffer();
    //
    //  Turn on the UART transmit, receive and the UART clock.
    //
    UARTPORT(CR2_TEN = 1);
    UARTPORT(CR2_RIEN = 1);
    UARTPORT(CR2_REN = 1);
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
void main()
{
    __disable_interrupt();
    _pulseDataAddress = (char *) EEPROM_PULSE_DATA;
    _numberOfPulses = *_pulseDataAddress++;
    SetupPorts();
    SetupUART();
    SetupTimer2();
    SetupTimer1();
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
    }
}

