#ifndef SIMPLEBLE_H
#define SIMPLEBLE_H

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

//#define RELEASE_VER                      //定义版本发布用

//------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
#define NPI_TIMEOUT_EVT             0x0008


#define     VERSION     "v1.3"


// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   100//必须是100ms

#define MAX_PERIPHERAL_MAC_ADDR                   5//最大记录的从机地址

#define MAC_ADDR_CHAR_LEN                       12//mac地址的字符长度 (一个字节等于两个字符)


typedef enum
{
    PARA_ALL_FACTORY = 0,           //全部恢复出厂设置
    PARA_PARI_FACTORY = 1,          //配对信息恢复出厂设置-相当于清除配对信息
}PARA_SET_FACTORY;

typedef enum
{
    BLE_ROLE_PERIPHERAL = 0,        //从机角色
    BLE_ROLE_CENTRAL = 1,           //主机角色    
}BLE_ROLE;

// Application states
enum
{
  BLE_STATE_IDLE,                    //无连接-空闲状态
  BLE_STATE_CONNECTING,             //连接中...
  BLE_STATE_CONNECTED,              //已连接上
  BLE_STATE_DISCONNECTING,          //断开连接中
  BLE_STATE_ADVERTISING             //从机广播中
};

enum
{
  BLE_MODE_SERIAL,                   // 串口透传模式 【默认】
  BLE_MODE_DRIVER,                   // 直驱模式        
  BLE_MODE_iBeacon,                  // iBeacon 广播模式
  BLE_MODE_MAX,
};

typedef struct 
{
  /*
        0---------9600 
        1---------19200 
        2---------38400 
        3---------57600 
        4---------115200
  */
    uint8 baudrate;                 // 波特率 
    uint8 parity;                   //校验位
    uint8 stopbit;                  //停止位 
    uint8 mode;                     //工作模式 0:透传 ， 1: 直驱 , 2: iBeacon
    uint8 name[12];                 //设备名称

    BLE_ROLE role;                  //主从模式  0: 从机   1: 主机

    uint8 pass[7];                  //密码

    /*
    Para: 0 ~ 1 
    0: 连接不需要密码
    1: 连接需要密码
    */
    uint8 type;                     //鉴权模式

    
    uint8 mac_addr[13];            //本机mac地址

    uint8 connl_status;            //连接最后一次的状态
    uint8 connect_mac_status;      //连接指定地址的返回状态
    uint8 connect_mac_addr[13];    //指定去连接的mac地址

    //曾经成功连接过的从机地址
    uint8 ever_connect_mac_status[MAX_PERIPHERAL_MAC_ADDR][13];       

    uint8 verion[5];       //版本信息 v1.0 ~ v9.9

    /*
    Para: 000000～009999 
    000000 代表持续连接，其
    余代表尝试的毫秒数
    Default:001000
    */
    uint16 try_connect_time_ms;           // 尝试连接时间
    int8 rssi;                              //  RSSI 信号值
    uint8 rxGain;                           //  接收增益强度
    uint8 txPower;                          //  发射信号强度
    uint16 ibeacon_adver_time_ms;         // 广播间隔
    uint8 workMode;                        //  模块工作类型  0: 立即工作， 1: 等待AT+CON 或 AT+CONNL 命令
}SYS_CONFIG;
extern SYS_CONFIG sys_config;


extern void Serial_Delay(int times);

//flag: PARA_ALL_FACTORY:  全部恢复出厂设置
//flag: PARA_PARI_FACTORY: 清除配对信息
extern void simpleBLE_SetAllParaDefault(PARA_SET_FACTORY flag);    
extern void simpleBLE_SaveAllDataToFlash();

extern void PrintAllPara(void);
extern bool simpleBLE_AT_CMD_Handle(uint8 *pBuffer, uint16 length);

extern void simpleBLE_NPI_init(void);

extern void UpdateRxGain(void);
extern void UpdateTxPower(void);

extern void LedSetState(uint8 onoff);
extern void simpleBle_SetRssi(int8 rssi);


extern BLE_ROLE GetBleRole();

extern uint32 str2Num(uint8* numStr, uint8 iLength);

extern void simpleBle_PrintPassword();

extern uint8* GetAttDeviceName();
extern void performPeriodicTask( void );

extern char *bdAddr2Str ( uint8 *pAddr );
extern void CheckKeyForSetAllParaDefault(void);

extern bool CheckIfUse_iBeacon();
extern bool simpleBle_GetIfNeedPassword();

extern void simpleBLE_SetToConnectFlag(bool bToConnect);
extern bool simpleBLE_GetToConnectFlag(uint8 *Addr);


extern uint32 Get_iBeaconAdvertisingInterral();
extern void simpleBLE_SetPeripheralMacAddr(uint8 *pAddr);
extern bool simpleBLE_GetPeripheralMacAddr(uint8 *pAddr);



extern uint8 simpleBLEState;
extern uint16 simpleBLECharHdl;
extern uint16 simpleBLECharHd6;
extern bool simpleBLEChar6DoWrite;
extern bool simpleBLEChar6DoWrite2;


#if defined (RELEASE_VER)
#define LCD_WRITE_STRING(str, option)                     
#define LCD_WRITE_SCREEN(line1, line2)                    
#define LCD_WRITE_STRING_VALUE(title, value, format, line)

#if defined (HAL_LCD)
#undef HAL_LCD
#define HAL_LCD FALSE 
#endif

#else
// LCD macros
#if HAL_LCD == TRUE
#define LCD_WRITE_STRING(str, option)                       HalLcdWriteString( (str), (option))
#define LCD_WRITE_SCREEN(line1, line2)                      HalLcdWriteScreen( (line1), (line2) )
#define LCD_WRITE_STRING_VALUE(title, value, format, line)  HalLcdWriteStringValue( (title), (value), (format), (line) )
#else
#define LCD_WRITE_STRING(str, option)                     
#define LCD_WRITE_SCREEN(line1, line2)                    
#define LCD_WRITE_STRING_VALUE(title, value, format, line)
#endif
#endif





extern uint8 simpleBLETaskId;               // 主机任务
extern uint8 simpleBLEState;
extern uint16 simpleBLECharHdl;
extern uint16 simpleBLECharHd6;
extern bool simpleBLECentralCanSend;
extern bool simpleBLEChar6DoWrite;
extern uint8 simpleBLEPeripheral_TaskID;        // 从机任务






#ifdef __cplusplus
}
#endif

#endif /* SIMPLEBLE_H */
