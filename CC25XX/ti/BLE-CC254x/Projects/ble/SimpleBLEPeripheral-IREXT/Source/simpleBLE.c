#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "gatt.h"
#include "ll.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "central.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "simpleGATTprofile.h"
#include "npi.h"
#include "osal_snv.h"
#include "simpleBLE.h"
#include "stdio.h"

SYS_CONFIG sys_config;
static bool g_bToConnect = FALSE;

extern gaprole_States_t gapProfileState;


static void simpleBLE_NpiSerialCallback( uint8 port, uint8 events );
static void simpleBLE_UartDataMain(uint8 *buf, uint8 numBytes);
static bool simpleBLE_AT_CMD_Handle(uint8 *pBuffer, uint16 length);

#if defined (AUTO_UART2UART)
static void simpleBLE_SendMyData_ForTest();
#endif

void Serial_Delay(int times)
{
    while(times--)
    {
        int i=0;
        for(i=6000; i>0; i--)
        {
            asm("nop");
        }
    }
}

static uint8 str_cmp(uint8 *p1,uint8 *p2,uint8 len)
{
    uint8 i=0;
    while(i<len)
    {
        if(p1[i]!=p2[i])
            return 0;
        i++;
    }
    return 1;
}

uint32 str2Num(uint8* numStr, uint8 iLength)
{
    uint8 i = 0;
    int32 rtnInt = 0;

    for(; i < iLength && numStr[i] != '\0'; ++i)
        rtnInt = rtnInt * 10 + (numStr[i] - '0');

    return rtnInt;
}

/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
#define B_ADDR_STR_LEN                        15

    uint8       i;
    char        hex[] = "0123456789ABCDEF";
    static char str[B_ADDR_STR_LEN];
    char        *pStr = str;

    *pStr++ = '0';
    *pStr++ = 'x';

    // Start from end of addr
    pAddr += B_ADDR_LEN;

    for ( i = B_ADDR_LEN; i > 0; i-- )
    {
        *pStr++ = hex[*--pAddr >> 4];
        *pStr++ = hex[*pAddr & 0x0F];
    }

    *pStr = 0;

    return str;
}

void simpleBLE_SaveAllDataToFlash()
{
    osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
}

void simpleBLE_SetAllParaDefault(PARA_SET_FACTORY flag)
{
    if(flag == PARA_ALL_FACTORY)
    {
        sys_config.baudrate = HAL_UART_BR_9600;
        // sys_config.baudrate = HAL_UART_BR_115200;
        sys_config.parity = 0;
        sys_config.stopbit = 0;

        sys_config.mode = BLE_MODE_SERIAL;

        sprintf((char*)sys_config.name, "VMI_TEST");

        sys_config.role = BLE_ROLE_PERIPHERAL;
        // sys_config.role = BLE_ROLE_CENTRAL;

        sprintf((char*)sys_config.pass, "000000");
        sys_config.type = 0;
        // sys_config.mac_addr[16];
        sys_config.connl_status = 0;
        sys_config.connect_mac_status = 0;
        // sys_config.ever_connect_mac_status[MAX_PERIPHERAL_MAC_ADDR][13];

        osal_memset(sys_config.ever_connect_mac_status, 0, MAX_PERIPHERAL_MAC_ADDR*13);
        sprintf((char*)sys_config.verion, "%s", VERSION);

        sys_config.try_connect_time_ms = 0;

        sys_config.rssi = 0;

        sys_config.rxGain = HCI_EXT_RX_GAIN_STD;
        sys_config.txPower = 0;

        sys_config.ibeacon_adver_time_ms = 500;

        sys_config.workMode = 0;
    }
    else if(flag == PARA_PARI_FACTORY)
    {
        osal_memset(sys_config.ever_connect_mac_status, 0, MAX_PERIPHERAL_MAC_ADDR*13);
        sprintf((char*)sys_config.verion, "%s", VERSION);
    }
}


void PrintAllPara(void)
{
#define SERIAL_DELAY     Serial_Delay(10)

    char strTemp[32];
    uint8 i;

    sprintf(strTemp, "sys_config.baudrate = %d\r\n", sys_config.baudrate);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.parity = %d\r\n", sys_config.parity);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.stopbit = %d\r\n", sys_config.stopbit);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.mode = %d\r\n", sys_config.mode);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.name = %s\r\n", sys_config.name);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.role = %d\r\n", sys_config.role);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.pass = %s\r\n", sys_config.pass);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.type = %d\r\n", sys_config.type);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.mac_addr = %s\r\n", sys_config.mac_addr);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.connl_status = %d\r\n", sys_config.connl_status);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.connect_mac_status = %d\r\n", sys_config.connect_mac_status);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    //sys_config.ever_connect_mac_status[MAX_PERIPHERAL_MAC_ADDR][13];
    for(i = 0; i<MAX_PERIPHERAL_MAC_ADDR && sys_config.ever_connect_mac_status[i][0] != 0; i++)
    {
        sprintf(strTemp, "sys_config.ever_connect_mac_status[%d] = %s\r\n", i, sys_config.ever_connect_mac_status[i]);
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
        SERIAL_DELAY;
    }

    sprintf(strTemp, "sys_config.verion = %s\r\n", sys_config.verion);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.try_connect_time_ms = %d\r\n", sys_config.try_connect_time_ms);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.rssi = %d\r\n", sys_config.rssi);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.txPower = %d\r\n", sys_config.txPower);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.ibeacon_adver_time_ms = %d\r\n", sys_config.ibeacon_adver_time_ms);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;

    sprintf(strTemp, "sys_config.workMode = %d\r\n", sys_config.workMode);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    SERIAL_DELAY;
}



BLE_ROLE GetBleRole()
{
    return sys_config.role;
}

bool simpleBLE_IfConnected()
{
    if(GetBleRole() == BLE_ROLE_CENTRAL)
    {
        //LCD_WRITE_STRING_VALUE( "simpleBLECharHd6=", simpleBLECharHd6, 10, HAL_LCD_LINE_1 );
        //return ( ( simpleBLEState == BLE_STATE_CONNECTED  ) && ( simpleBLECharHd6 != 0 ) );
        return ( simpleBLEState == BLE_STATE_CONNECTED  );
    }
    else
    {
        return (gapProfileState == GAPROLE_CONNECTED);
    }
}

void simpleBLE_SetPeripheralMacAddr(uint8 *pAddr)
{
    if(GetBleRole() == BLE_ROLE_CENTRAL)
    {
        osal_memcpy(sys_config.ever_connect_mac_status[0], pAddr, MAC_ADDR_CHAR_LEN);
    }
}

bool simpleBLE_GetPeripheralMacAddr(uint8 *pAddr)
{
    if(GetBleRole() == BLE_ROLE_CENTRAL)
    {
        if(sys_config.ever_connect_mac_status[0][0] != 0)
        {
            osal_memcpy(pAddr, sys_config.ever_connect_mac_status[0], MAC_ADDR_CHAR_LEN);
            return TRUE;
        }
    }
    return FALSE;
}


void CheckKeyForSetAllParaDefault(void)
{
#if 1
    return;
#else
    uint8 i;
    uint32 old_time  = 75;

    while(--old_time)
    {
        if(P0_1 == 1)
        {
            LedSetState(HAL_LED_MODE_ON);
            Serial_Delay(10);
        }
        else
        {
            LedSetState(HAL_LED_MODE_OFF);
            return;
        }
    }

    if(old_time == 0)
    {
        simpleBLE_SetAllParaDefault(PARA_ALL_FACTORY);
        for(i = 0; i < 6; i++)
        {
            LedSetState(HAL_LED_MODE_ON);
            Serial_Delay(30);
            LedSetState(HAL_LED_MODE_OFF);
            Serial_Delay(30);
        }
        HAL_SYSTEM_RESET();
    }
#endif
}

void simpleBLE_NPI_init(void)
{
#if 1
    NPI_InitTransportEx(simpleBLE_NpiSerialCallback, sys_config.baudrate,
                        sys_config.parity, sys_config.stopbit );
#else
    NPI_InitTransport(simpleBLE_NpiSerialCallback);
#endif

    if(GetBleRole() == BLE_ROLE_CENTRAL)
    {
        NPI_WriteTransport("Hello World Central\r\n",21);
    }
    else
    {
        NPI_WriteTransport("Hello World Peripheral\r\n",24);
    }
}


void UpdateRxGain(void)
{
    // rxGain - HCI_EXT_RX_GAIN_STD, HCI_EXT_RX_GAIN_HIGH
    HCI_EXT_SetRxGainCmd( HCI_EXT_RX_GAIN_STD );
}

void UpdateTxPower(void)
{
    /*
    #define LL_EXT_TX_POWER_MINUS_23_DBM                   0
    #define LL_EXT_TX_POWER_MINUS_6_DBM                    1
    #define LL_EXT_TX_POWER_0_DBM                          2
    #define LL_EXT_TX_POWER_4_DBM                          3
    */
    // HCI_EXT_SetTxPowerCmd();
    HCI_EXT_SetTxPowerCmd(sys_config.txPower);
}


void LedSetState(uint8 onoff)
{
    HalLedSet( HAL_LED_1, onoff);
}

void simpleBle_SetRssi(int8 rssi)
{
    sys_config.rssi = rssi;
}

void simpleBle_PrintPassword()
{
    char strTemp[24] = {0};

    sprintf(strTemp, "Password:%s\r\n", sys_config.pass);
    NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

    Serial_Delay(10);
}

uint8* GetAttDeviceName()
{
    return sys_config.name;
}

bool IFfHavePeripheralMacAddr( void )
{
    if(sys_config.ever_connect_mac_status[0][0] == 0)
    {
        return TRUE;
    }
    else
    {
        return TRUE;
    }
}

void performPeriodicTask( void )
{
    static uint8 count = 0;

    if(CheckIfUse_iBeacon())
    {
        static attHandleValueNoti_t pReport;
        pReport.len = 2;
        pReport.handle = 0x2E;
        osal_memcpy(pReport.value, "ab", 2);
        GATT_Notification( 0, &pReport, FALSE );

        LedSetState(HAL_LED_MODE_TOGGLE);
        return;
    }

    if(!simpleBLE_IfConnected())
    {
        if(GetBleRole() == BLE_ROLE_CENTRAL)
        {
            if(IFfHavePeripheralMacAddr() == FALSE)
            {
                if(count == 0)
                {
                    LedSetState(HAL_LED_MODE_ON);
                }
                else if(count == 1)
                {
                    LedSetState(HAL_LED_MODE_OFF);
                }
            }
            else
            {
                if(count == 0)
                {
                    LedSetState(HAL_LED_MODE_ON);
                }
                else if(count == 9)
                {
                    LedSetState(HAL_LED_MODE_OFF);
                }
            }
            count++;
            count %= 10;
        }
        else//´Ó»ú
        {
            if(count == 0)
            {
                LedSetState(HAL_LED_MODE_OFF);
            }
            else if(count == 10)
            {
                LedSetState(HAL_LED_MODE_ON);
            }

            count++;
            count %= 20;
        }
    }
    else
    {
        if(count == 0)
        {
            LedSetState(HAL_LED_MODE_ON);
        }
        else if(count == 1)
        {
            LedSetState(HAL_LED_MODE_OFF);
        }
        count++;
        count %= 50;

#if defined (AUTO_UART2UART)
        simpleBLE_SendMyData_ForTest();
#endif

    }
}

bool simpleBle_GetIfNeedPassword()
{
    return sys_config.type;
}


bool CheckIfUse_iBeacon()
{
    return (sys_config.mode == BLE_MODE_iBeacon);
}

void simpleBLE_SetToConnectFlag(bool bToConnect)
{
    g_bToConnect = bToConnect;
}

bool simpleBLE_GetToConnectFlag(uint8 *Addr)
{
    osal_memcpy(Addr, sys_config.connect_mac_addr, MAC_ADDR_CHAR_LEN);
    if(sys_config.workMode == 0)
    {
        g_bToConnect = TRUE;
    }

    return g_bToConnect;
}

uint32 Get_iBeaconAdvertisingInterral()
{
    return sys_config.ibeacon_adver_time_ms;
}

#if 1
static void simpleBLE_NpiSerialCallback( uint8 port, uint8 events )
{
    (void)port;

    static uint32 old_time;
    static uint32 old_time_data_len = 0;
    uint32 new_time;
    bool ret;
    uint8 readMaxBytes = SIMPLEPROFILE_CHAR6_LEN;

    if (events & HAL_UART_RX_TIMEOUT)
    {
        (void)port;
        uint8 numBytes = 0;

        numBytes = NPI_RxBufLen();
        if(numBytes == 0)
        {
            LCD_WRITE_STRING_VALUE( "ERROR: numBytes=", numBytes, 10, HAL_LCD_LINE_1 );
            old_time_data_len = 0;
            return;
        }
        if(old_time_data_len == 0)
        {
            old_time = osal_GetSystemClock();
            old_time_data_len = numBytes;
        }
        else
        {
            if(!simpleBLE_IfConnected())
            {
                readMaxBytes = 22;
            }
            else
            {
                readMaxBytes = SIMPLEPROFILE_CHAR6_LEN;
            }


            new_time = osal_GetSystemClock();
            if( (numBytes >= readMaxBytes)
                || ( (new_time - old_time) > 20/*ms*/))
            {
                uint8 sendBytes = 0;
                uint8 *buffer = osal_mem_alloc(readMaxBytes);

                if(!buffer)
                {
                    NPI_WriteTransport("FAIL", 4);
                    return;
                }

                //
                if(numBytes > readMaxBytes)
                {
                    sendBytes = readMaxBytes;
                }
                else
                {
                    sendBytes = numBytes;
                }

                if(!simpleBLE_IfConnected())
                {
                    //numBytes = NpiReadBuffer(buf, sizeof(buf));
                    //NpiClearBuffer();
                    NPI_ReadTransport(buffer,sendBytes);

                    if(sendBytes > 2
                       && buffer[sendBytes-2] == '\r'
                       && buffer[sendBytes-1] == '\n')
                    {
                        ret = simpleBLE_AT_CMD_Handle(buffer, sendBytes);
                    }
                    else
                    {
                        ret = FALSE;
                    }

                    if(ret == FALSE)
                    {
                        char strTemp[12];
                        sprintf(strTemp, "ERROR\r\n");
                        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                    }
                }
                else
                {
                    if((GetBleRole() == BLE_ROLE_CENTRAL) && simpleBLEChar6DoWrite && simpleBLECentralCanSend )
                    {
                        char strTemp[24];

                        NPI_ReadTransport(buffer,sendBytes);

                        if((sendBytes == 10) && str_cmp(buffer, "AT+RSSI?\r\n", 10))//AT+RSSI\r\n
                        {
                            sprintf(strTemp, "OK+RSSI:%d\r\n", sys_config.rssi);
                            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                        }
                        else
                        {
                            //LCD_WRITE_STRING_VALUE( "sendBytes=", sendBytes, 10, HAL_LCD_LINE_1 );
                            simpleBLE_UartDataMain(buffer,sendBytes);
                        }
                    }
                    else if((GetBleRole() == BLE_ROLE_PERIPHERAL) && simpleBLEChar6DoWrite2)
                    {
                        //LCD_WRITE_STRING_VALUE( "sendBytes=", sendBytes, 10, HAL_LCD_LINE_1 );
                        NPI_ReadTransport(buffer,sendBytes);
                        simpleBLE_UartDataMain(buffer,sendBytes);
                    }
                    else
                    {
                        NPI_ReadTransport(buffer,sendBytes);
                    }
                }

                old_time = new_time;
                old_time_data_len = numBytes - sendBytes;


                osal_mem_free(buffer);
            }
        }
    }
}

void simpleBLE_UartDataMain(uint8 *buf, uint8 numBytes)
{
    if(GetBleRole() == BLE_ROLE_CENTRAL )
    {
        if(simpleBLEChar6DoWrite
           && ( simpleBLECharHd6 != 0)
           && simpleBLECentralCanSend)
        {
            attWriteReq_t AttReq;

            LCD_WRITE_STRING_VALUE( "s=", numBytes, 10, HAL_LCD_LINE_1 );

            simpleBLEChar6DoWrite = FALSE;

            AttReq.handle = simpleBLECharHd6;
            AttReq.len = numBytes;
            AttReq.sig = 0;
            AttReq.cmd = 0;
            osal_memcpy(AttReq.value,buf,numBytes);
            GATT_WriteCharValue( 0, &AttReq, simpleBLETaskId );
        }
        else
        {
            LCD_WRITE_STRING_VALUE( "simpleBLEChar6DoWrite=", simpleBLEChar6DoWrite, 10, HAL_LCD_LINE_1 );
            LCD_WRITE_STRING_VALUE( "simpleBLECharHd6=", simpleBLECharHd6, 10, HAL_LCD_LINE_1 );
        }
    }
    else
    {
        if(simpleBLEChar6DoWrite2)
        {
#if 0
            simpleBLEChar6DoWrite2 = FALSE;
            SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6,numBytes, buf );
#else
            static attHandleValueNoti_t pReport;
            pReport.len = numBytes;
            pReport.handle = 0x0035;
            osal_memcpy(pReport.value, buf, numBytes);
            GATT_Notification( 0, &pReport, FALSE );
#endif
        }
        else
        {
            LCD_WRITE_STRING_VALUE( "simpleBLEChar6DoWrite=", simpleBLEChar6DoWrite2, 10, HAL_LCD_LINE_1 );
        }
    }
}
#endif

#if 1
bool simpleBLE_AT_CMD_Handle(uint8 *pBuffer, uint16 length)
{
    bool ret = TRUE;
    char strTemp[64];
    uint8 i;
    uint8 temp8;
    bool restart = FALSE;

    //NPI_WriteTransport((uint8*)pBuffer, length);
    if((length == 4) && str_cmp(pBuffer, "AT\r\n", 4))//AT
    {
        sprintf(strTemp, "OK\r\n");
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 8) && str_cmp(pBuffer, "AT+ALL\r\n", 8))//AT
    {
        PrintAllPara();
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+BAUD", 7))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%d\r\n", sys_config.baudrate);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
                sys_config.baudrate = pBuffer[7] - '0';
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.baudrate);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

                restart = TRUE;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+PARI", 7))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%d\r\n", sys_config.parity);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            case '0':
            case '1':
            case '2':
                sys_config.parity = pBuffer[7] - '0';
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.parity);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

                restart = TRUE;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+STOP", 7))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%d\r\n", sys_config.stopbit);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            case '0':
            case '1':
                sys_config.stopbit = pBuffer[7] - '0';
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.stopbit);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

                restart = TRUE;
                break;
            default:
                ret = FALSE;
                break;
        }
    }

    else if((length == 10) && str_cmp(pBuffer, "AT+MODE", 7))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%d\r\n", sys_config.mode);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            case '0':
            case '1':
            case '2':
                sys_config.mode = pBuffer[7] - '0';
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.mode);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

                restart = TRUE;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    else if((length >=10 && length <= 20) && str_cmp(pBuffer, "AT+NAME", 7))
    {
        //int nameLen = length - 7;

        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%s\r\n", sys_config.name);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            default:
                osal_memset(sys_config.name, 0, sizeof(sys_config.name));
                osal_memcpy(sys_config.name, pBuffer + 7, length - 7);
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%s\r\n", sys_config.name);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

                restart = TRUE;
                break;
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+RENEW", 8))
    {
        sprintf(strTemp, "OK+RENEW\r\n");
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

        simpleBLE_SetAllParaDefault(PARA_ALL_FACTORY);
        osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);

        restart = TRUE;
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+RESET", 8))
    {
        restart = TRUE;
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+ROLE", 7))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%d\r\n", sys_config.role);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            case '0':
            case '1':
                temp8 = pBuffer[7] - '0';
                if(temp8 == 0)
                {
                    sys_config.role = BLE_ROLE_PERIPHERAL;
                }
                else
                {
                    sys_config.role = BLE_ROLE_CENTRAL;
                }
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.role);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                restart = TRUE;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    else if(((length == 10) && str_cmp(pBuffer, "AT+PASS?", 8))
            || ((length == 15) && str_cmp(pBuffer, "AT+PASS", 7)))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+PASS:%s\r\n", sys_config.pass);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            default:
                osal_memcpy(sys_config.pass, pBuffer + 7, 6);
                sys_config.pass[6] = 0;
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%s\r\n", sys_config.pass);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+TYPE", 7))
    {
        switch(pBuffer[7])
        {
            case '?':
                sprintf(strTemp, "OK+Get:%d\r\n", sys_config.type);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                break;
            case '0':
            case '1':
                sys_config.type = pBuffer[7] - '0';
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.type);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
                restart = TRUE;
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    else if((length >= 10) && str_cmp(pBuffer, "AT+ADDR?", 8))
    {
        sprintf(strTemp, "OK+LADD:%s\r\n", sys_config.mac_addr);
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+CONNL", 8))
    {
        uint8 para[4] = {'L','N','E','F'};
        int8 id = 0;

        if(sys_config.connect_mac_addr[0] != 0)
        {
            id = 0;
            simpleBLE_SetToConnectFlag(TRUE);
        }
        else
        {
            id = 3;
        }
        sprintf(strTemp, "AT+CONN%c\r\n", para[id]);
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 20) && str_cmp(pBuffer, "AT+CON", 6))
    {
        uint8 para[3] = {'A','E','F'};
        uint8 id = 0;

        osal_memcpy(sys_config.connect_mac_addr, pBuffer+6, MAC_ADDR_CHAR_LEN);
        osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);

        //if(simpleBLE_GetToConnectFlag == FALSE)
        {
            simpleBLE_SetToConnectFlag(TRUE);
            id = 0;
        }
        //if(sys_config.connect_mac_status > 2)
        //    sys_config.connect_mac_status = 2;
        sprintf(strTemp, "AT+CONN%c\r\n", para[id]);
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+CLEAR", 8))
    {
        simpleBLE_SetAllParaDefault(PARA_PARI_FACTORY);
        osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
        //PrintAllPara();

        sprintf(strTemp, "OK+CLEAR\r\n");
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+RADD?", 8))
    {
        for(i = 0; i<MAX_PERIPHERAL_MAC_ADDR; i++)
        {
            if(sys_config.ever_connect_mac_status[i][0] != 0)
            {
                sprintf(strTemp, "OK+RADD:%s\r\n", sys_config.ever_connect_mac_status[i]);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
            }
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+VERS?", 8))
    {
        sprintf(strTemp, "%s\r\n", sys_config.verion);
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+TCON", 7))
    {
        if(pBuffer[7] == '?')
        {
            sprintf(strTemp, "%06d\r\n", sys_config.try_connect_time_ms);
            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
        }
        else
        {
            sys_config.try_connect_time_ms = 10000;//_atoi(pBuffer+7);
            osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
            sprintf(strTemp, "OK+Set:%06d\r\n", sys_config.try_connect_time_ms);
            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+RSSI?", 10))
    {
        sprintf(strTemp, "OK+RSSI:%d\r\n", sys_config.rssi);
        NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+TXPW", 7))
    {
        if(pBuffer[7] == '?')
        {
            sprintf(strTemp, "AT+TXPW:%d\r\n", sys_config.txPower);
            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
        }
        else
        {
            sys_config.txPower = pBuffer[7] - '0';
            if(sys_config.txPower > 3)
                sys_config.txPower = 0;
            osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
            sprintf(strTemp, "OK+Set:%d\r\n", sys_config.txPower);
            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
            HCI_EXT_SetTxPowerCmd(sys_config.txPower);

            restart = TRUE;
        }
    }
    else if((length == 10 || length == 15) && str_cmp(pBuffer, "AT+TIBE", 7))
    {
        if(pBuffer[7] == '?')
        {
            sprintf(strTemp, "AT+TIBE:%06d\r\n", sys_config.ibeacon_adver_time_ms);
            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
        }
        else
        {
            sys_config.ibeacon_adver_time_ms = str2Num(pBuffer+7, 6);
            if(sys_config.ibeacon_adver_time_ms <= 9999)
            {
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%06d\r\n", sys_config.ibeacon_adver_time_ms);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));

                restart = TRUE;
            }
        }
    }
    else if((length == 10) && str_cmp(pBuffer, "AT+IMME", 7))
    {
        if(pBuffer[7] == '?')
        {
            sprintf(strTemp, "OK+Get:%d\r\n", sys_config.workMode);
            NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
        }
        else
        {
            sys_config.workMode = str2Num(pBuffer+7, 1);
            if(sys_config.workMode <= 1)
            {
                osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);
                sprintf(strTemp, "OK+Set:%d\r\n", sys_config.workMode);
                NPI_WriteTransport((uint8*)strTemp, osal_strlen(strTemp));
            }

            restart = TRUE;
        }
    }
    else
    {
        ret = FALSE;
    }

    if(restart)
    {
        Serial_Delay(100);
        HAL_SYSTEM_RESET();
    }

    return ret;
}
#endif

#if defined (AUTO_UART2UART)
void simpleBLE_SendMyData_ForTest()
{
    static uint8 count_100ms = 0;
    uint8 numBytes;

    count_100ms++;
    if(count_100ms == 10)
    {
        char strTemp[24] = {0};

        if((GetBleRole() == BLE_ROLE_CENTRAL) && simpleBLEChar6DoWrite && simpleBLECentralCanSend)
        {
            sprintf(strTemp, "[%8ldms]Amo1\r\n", osal_GetSystemClock());
            numBytes = (osal_strlen(strTemp) > SIMPLEPROFILE_CHAR6_LEN) ? SIMPLEPROFILE_CHAR6_LEN : osal_strlen(strTemp);
            simpleBLE_UartDataMain((uint8*)strTemp, numBytes);
        }
        else if((GetBleRole() == BLE_ROLE_PERIPHERAL) && simpleBLEChar6DoWrite2)
        {
            sprintf(strTemp, "[%8ldms]Amo2\r\n", osal_GetSystemClock());
            simpleBLE_UartDataMain((uint8*)strTemp, numBytes);
        }

        count_100ms = 0;
    }
}
#endif

