/*******************************************************************************
 *
 *  Filename:      main.c
 *
 *  Description:   Main functions for Irext STM8 module driver
 *
 *  Created by strawmanbobi 2017-12-31
 *  Copyright (c) 2017 Irext. All rights reserved.
 *
 *******************************************************************************/

#include <string.h>
#include "stm8s.h"
#include "stdlib.h"
#include "stdio.h"
#include "main.h"


#ifdef _RAISONANCE_
#define PUTCHAR_PROTOTYPE int putchar (char c)
#define GETCHAR_PROTOTYPE int getchar (void)
#elif defined (_COSMIC_)
#define PUTCHAR_PROTOTYPE char putchar (char c)
#define GETCHAR_PROTOTYPE char getchar (void)
#else /* _IAR_ */
#define PUTCHAR_PROTOTYPE int putchar (int c)
#define GETCHAR_PROTOTYPE int getchar (void)
#endif /* _RAISONANCE_ */


#define TIM4_PERIOD       124
#define UART_RECV_STOP    200

/* UART related definition */
#define REQ_READY               0x50
#define REQ_WRITE               0x51
#define REQ_ERR                 0x52
#define REQ_READ                0x53
#define REQ_CATEGORY            0x54

#define RSP_READY               0x60
#define RSP_INDEX               0x61
#define RSP_LENGTH              0x62
#define RSP_DATA                0x63
#define RSP_DONE                0x64
#define RSP_INDEX_DONE          0x65
#define RSP_CMD_ERR             0x66

#define BLOCK_BYTES             16


/* prototypes */
static decode_control_block_t dccb =
{
    .ir_type = IR_TYPE_NONE,
    .ir_state = IR_STATE_STANDBY,
    .source_code_length = 0,
    .decoded_length = 0
};

static t_remote_ac_status ac_status =
{
    .ac_power = AC_POWER_OFF,
    .ac_temp = AC_TEMP_24,
    .ac_mode = AC_MODE_COOL,
    .ac_wind_dir = AC_SWING_ON,
    .ac_wind_speed = AC_WS_AUTO,
    .ac_display = 0,
    .ac_sleep = 0,
    .ac_timer = 0,
};

/* global variables */
#if defined UART_DEFRAGMENT
uint8_t receive_state = 0;
uint8_t receive_buffer[1024] = { 0 };
uint32_t received = 0;
uint32_t uart_recv_stop_cd = UART_RECV_STOP;
#endif


/* local functions */
static void IRext_processUartMsg();
static void HandleBinReady();
static void HandleBinWrite();
static void HandleBinCategory();

static void ParseCommand(uint8_t* data, uint16_t len);
static void TransportDataToUart(uint8_t* data, uint16_t len);
static void WriteBytes(uint8_t *data, uint16_t len);

#if defined UART_DEFRAGMENT
static void start_uart_cd(__IO uint32_t nTime);
static void stop_uart_receive();
#endif

/* local vars */


void main(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    // Init GPIO
    init_GPIO();

    // Init UART
    init_UART();

    // Init Timer
    // init_Timer4();

    while (1)
    {
#if defined UART_DEFRAGMENT
        start_uart_cd(UART_RECV_STOP);
#endif
        IRext_processUartMsg();
    }
}


/* initialization */
void deinit_GPIO()
{
}


void init_GPIO()
{
    GPIO_Init(IR_IO_PORT, (GPIO_Pin_TypeDef)(IR_PIN),  GPIO_MODE_OUT_PP_HIGH_FAST);
}


void init_UART()
{
    UART3_DeInit();
    UART3_Init((uint32_t)9600, UART3_WORDLENGTH_8D, UART3_STOPBITS_1, UART3_PARITY_NO,
               UART3_MODE_TXRX_ENABLE);

#if defined UART_INT
    // enable UART3 RX interrupt
    UART3_ITConfig(UART3_IT_RXNE_OR, ENABLE);
#endif
}

void init_Timer4()
{
    /* TIM4 configuration:
     - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
     clock used is 16 MHz / 128 = 125 000 Hz
    - With 125 000 Hz we can generate time base:
        max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
        min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
    - In this example we need to generate a time base equal to 1 ms
     so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */

    /* Time base configuration */
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, TIM4_PERIOD);
    /* Clear TIM4 update flag */
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
}

#if defined UART_DEFRAGMENT
static void start_uart_cd(__IO uint32_t nTime)
{
    TIM4_Cmd(DISABLE);
    uart_recv_stop_cd = nTime;
    /* enable interrupts */
    enableInterrupts();
    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);
}

static void stop_uart_receive()
{
    // it seems there is no data from peer of uart, reset the receiving FSM
    memset(receive_buffer, 1024, 0x00);
    received = 0;
    printf("UART receiving stopped\n");
    TIM4_Cmd(DISABLE);
}

#endif

void timer4_callback()
{
#if defined UART_DEFRAGMENT
    if (uart_recv_stop_cd != 0)
    {
        uart_recv_stop_cd--;
        return;
    }
    stop_uart_receive();
#endif
}

/* UART TX/RX */
PUTCHAR_PROTOTYPE
{
    UART3_SendData8(c);
    while (UART3_GetFlagStatus(UART3_FLAG_TXE) == RESET);

    return (c);
}


GETCHAR_PROTOTYPE
{
#ifdef _COSMIC_
    char c = 0;
#else
    int c = 0;
#endif
    while (UART3_GetFlagStatus(UART3_FLAG_RXNE) == RESET);
    c = UART3_ReceiveData8();
    return (c);
}


/* handle UART commands */
void IRext_uartFeedback()
{
    if (dccb.decoded_length > 0)
    {
        for (uint16_t index = 0; index < dccb.decoded_length; index++)
        {
            WriteBytes((uint8_t*)&dccb.ir_decoded[index], 2);
        }
    }
}


static void IRext_processUartMsg()
{
    uint8_t data_received = 0;

    data_received = getchar();
    switch(data_received)
    {
        case REQ_READY:
            HandleBinReady();
            break;
        case REQ_WRITE:
            HandleBinWrite();
            break;
        case REQ_CATEGORY:
            HandleBinCategory();
            break;
        default:
            break;
    }
}


static void HandleBinReady()
{
    /*
       Request for ready
       +------+
       | 0x50 |
       +------+
    */
    dccb.decoded_length = 0;
    memset(dccb.source_code, BINARY_SOURCE_SIZE_MAX, 0x00);
    dccb.source_code_length = 0;
    putchar(RSP_READY);
}


static void HandleBinWrite()
{
    /*
       Request for write IR binary
       +-------------------------------------------------------------+
       | 0x51 | exp_idx (1 byte) | exp_len (1 byte) | data (n bytes) |
       +-------------------------------------------------------------+
    */
    uint8_t expected_index = 0;
    uint8_t expected_length = 0;
    uint16_t received = 0;

    // receive bin block index
    expected_index = getchar();
    // prepare the offset for the next write
    dccb.recv_index = expected_index * BLOCK_BYTES;

    // receive expected length of next transfer
    expected_length = getchar();

    if (expected_length == 0 || expected_length > BLOCK_BYTES)
    {
        // error occurred!
        putchar(RSP_CMD_ERR);
    }
    else
    {
        for(received = 0; received < expected_length; received++)
        {
            dccb.source_code[dccb.recv_index + received] = getchar();
        }
        putchar(RSP_INDEX_DONE);
    }
}


static void HandleBinCategory()
{
    /*
       Request for write category
       +----------------------+
       | 0x54 | cate (1 byte) |
       +----------------------+
    */
    dccb.ir_type = (ir_type_t)getchar();
    // bin transfer done
    putchar(RSP_DONE);
}


static void ParseCommand(uint8_t* data, uint16_t len)
{
    uint8_t ir_type = 0;
    uint8_t key_code = 0;
    uint8_t ac_function = 0;

    if (IR_STATE_OPENED != dccb.ir_state)
    {
        // feek back error state
        WriteBytes("11", 2);
        return;
    }

    ir_type = data[0];

    if (IR_TYPE_TV == dccb.ir_type && 0x31 == ir_type)
    {
        // decode as TV
        key_code = data[1] - 0x30;
        dccb.decoded_length = ir_decode(key_code, dccb.ir_decoded, NULL, 0);
    }
    else if (IR_TYPE_AC == dccb.ir_type && 0x32 == ir_type)
    {
        ac_function = data[1] - 0x30;
        dccb.decoded_length = ir_decode(ac_function, dccb.ir_decoded, &ac_status, 0);
    }

    if (dccb.decoded_length > 0)
    {
        IRext_uartFeedback();
    }
    else
    {
    }
}


static void TransportDataToUart(uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        delay(10);
        putchar(data[i]);
    }
}


static void WriteBytes(uint8_t *data, uint16_t len)
{
    TransportDataToUart(data, len);
}


/* helper functions */
#ifdef USE_FULL_ASSERT


void assert_failed(uint8_t* file, uint32_t line)
{
    while (1)
    {
    }
}
#endif


void delay(u16 count)
{
    u8 i, j;
    while (count--)
    {
        for(i = 0; i < 50; i++)
            for(j = 0; j < 20; j++);
    }
}

