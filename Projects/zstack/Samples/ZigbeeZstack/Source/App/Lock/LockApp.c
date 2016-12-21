/**************************************************************************************************
  Filename:       LockApp.c
  Date:           2016-12-13 $
  Revision:       1.0
  Description:    门锁应用程序
  Author:         zhuxiankang
**************************************************************************************************/


/**************************************************************************************************
  Description:    端口配置说明
  1.LED                     ->              P1_5      普通IO口，设置为输出

  2.KEY                     ->   钥匙开门   P0_1      普通IO口，设置为上拉、输入，下降沿触发中断 
                                 锁扣       P0_4      普通IO口，设置为上拉、输入，下降沿触发中断
                                 门反锁     P0_5      普通IO口，设置为上拉、输入

  3.BUZZER                  ->   蜂鸣器     P2_0      普通IO口，设置为上拉、输出，默认高电平   

  4.MOTOR                   ->   H桥上桥    P0_6 P0_7 普通IO口，设置为输出，低电平有效
                                 H桥下桥    P1_0 P1_1 普通IO口，设置为输出，高电平有效

  5.I2C                     ->   SCL        P1_3      普通IO口，设置为输出
                                 SDA        P1_2      普通IO口，可能输出也可能输出
 
  6.MFRC522                 ->   CS(SPI)    P1_2      普通IO口，设置为输出
                                 SCK(SPI)   P1_3      普通IO口，设置为输出
                                 MOSI(SPI)  P0_2      普通IO口，设置为输出
                                 MISO(SPI)  P0_3      普通IO口，设置为输入
                                 RST        P1_7      普通IO口，设置为输出


**************************************************************************************************/




#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "LockApp.h"
//#include "LockAppHw.h"

#include "OnBoard.h"


/* 硬件抽象层HAL */   
#include "hal_led.h"    
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"

/* 驱动层 */
#include "dri_key.h"


/* 设备层 */
#include "dev_mfrc522.h"


/* 应用层 */
#include "appl_data.h"    //EEPROM存储应用
#include "apph_card.h"    //RFID刷卡操作
#include "apph_door.h"    //门锁开门提示应用


/*********************************************************************
 * 全局变量
 */

/*输入输出簇列表，命令列表*/
const cId_t LockApp_ClusterList[LOCKAPP_MAX_CLUSTERS] = 
{
  LOCKAPP_PERIODIC_CLUSTERID,    //广播的簇:0x01
  LOCKAPP_FLASH_CLUSTERID        //组播的簇:0x02
};

/*Zigbee简单端点描述符*/
const SimpleDescriptionFormat_t LockApp_SimpleDesc = 
{
  LOCKAPP_ENDPOINT,              //端点号:20
  LOCKAPP_PROFID,                //端点的簇ID:0x0F08
  LOCKAPP_DEVICEID,              //设备ID:0x0001
  LOCKAPP_DEVICE_VERSION,        //设备描述符版本:0
  LOCKAPP_FLAGS,                 //保留
  LOCKAPP_MAX_CLUSTERS,          //端点支持的输入簇个数:2
  (cId_t *)LockApp_ClusterList,  //输入簇列表
  LOCKAPP_MAX_CLUSTERS,          //点支持的输出簇个数:2 
  (cId_t *)LockApp_ClusterList   //输出簇列表（输入簇和输出簇要一一对应）
};

/*端点信息结构体*/
endPointDesc_t LockApp_epDesc;


/*********************************************************************
 * 本地变量
 */

uint8 LockApp_TaskID;                   // 任务ID 
devStates_t LockApp_NwkState;           //网络状态

uint8 LockApp_TransID;                  // 传输序列ID

afAddrType_t LockApp_Periodic_DstAddr;  //发送数据的广播目的地址
afAddrType_t LockApp_Flash_DstAddr;     //发送数据的组播目的地址

aps_Group_t LockApp_Group;              //组信息定义

uint8 LockAppPeriodicCounter = 0;       //发送次数统计
uint8 LockAppFlashCounter = 0;

/*********************************************************************
 * 本地函数
 */
void LockApp_HandleKeys( uint8 shift, uint8 keys ); 
void LockApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void LockApp_SendPeriodicMessage( void );
void LockApp_SendFlashMessage( uint16 flashTime );


/*********************************************************************
 * 公共函数
 */

/*********************************
  Funcname:       LockApp_Init
  Description:    任务初始化函数
  Author:         zhuxiankang
  parm:           task_id -> 任务序列号
*********************************/

void LockApp_Init( uint8 task_id )
{ 
  /*变量初始化*/
  LockApp_TaskID = task_id;         //任务ID
  LockApp_NwkState = DEV_INIT;      //网络状态初始化为DEV_INIT
  LockApp_TransID = 0;              //传输序列ID  
  
  /*Drivers驱动初始化*/
  Key_Init();                       //锁扣，反锁和钥匙开门初始化
  Buzzer_Init();                    //P2_0 蜂鸣器驱动初始化
  Buzzer_Timer4_Init();             //定时器4初始化(覆盖了Z-STACK的配置)
  Motor_Init();                     //P0_6 P0_7 P1_0 P1_1 电机驱动初始化
  I2C_Init();                       //P1_2 P1_3 I2C初始化
  Spi_Init();                       //P1.2、P1.3、P1.7、P0.2、P0.3 读卡器端口初始化    
  
  /*Devices设备初始化*/
  MFRC522_Init();                   //MFRC522读卡模块初始化
  //Card_Init();
  
  
  
  /*AppsH应用初始化*/
  //Data_DoorID_Init();               //门锁ID信息初始化，测试用
  Data_CommonCard_Init();           //普通卡列表初始化，测试用，最后可以远程控制，烧写的时候只需要一遍 
 
  
  Buzzer_System_Start();               //系统应用启动提示声音
  Door_Close(LedOff);
  
  
  
  /*单播设置*/  
  LockApp_Periodic_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;      //15:广播
  LockApp_Periodic_DstAddr.endPoint = LOCKAPP_ENDPOINT;             //端点号：20
  LockApp_Periodic_DstAddr.addr.shortAddr = 0x0000;                 //协调器地址
  
 
  /*定义本设备用来通信的APS层端点描述符*/
  LockApp_epDesc.endPoint = LOCKAPP_ENDPOINT;                     //端点号：20
  LockApp_epDesc.task_id = &LockApp_TaskID;                       //任务ID 
  LockApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&LockApp_SimpleDesc;   //Zigbee简单端点描述符
  LockApp_epDesc.latencyReq = noLatencyReqs;                      //无延时策略
  
  /*在AF中注册端点20*/
  afRegister( &LockApp_epDesc );

  /*注册按键事件*/
  RegisterForKeys( LockApp_TaskID );
  
  /*门锁离线任务定时1s*/
  osal_start_timerEx( LockApp_TaskID,LOCKAPP_OFF_LINE_TASK_MSG_EVENT,LOCKAPP_OFF_LINE_TASK_MSG_TIMEOUT );
  
}


//暂时放在外面，测试方便
uint8 Status = MFRC522_ERR;     //刷卡结果
uint8 BlockData[16] = {0x00};   //M1卡扇区数据 
uint8 DoorId[4] = {0x00};       //门锁ID



/*********************************
  Funcname:       LockApp_ProcessEvent
  Description:    任务事件处理函数
  Author:         zhuxiankang
  parm:           task_id -> 任务序列号 evnets -> 事件触发标志
*********************************/
uint16 LockApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;     //接收消息结构体
  (void)task_id;  

  if ( events & SYS_EVENT_MSG )      //接收到系统消息
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( LockApp_TaskID ); //调用系统信息，接收消息包
    
    while ( MSGpkt )
    {
      
      switch ( MSGpkt->hdr.event )  
      {
        /*按键处理事件*/
        case KEY_CHANGE:
          LockApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break; 

        /*消息到来响应事件*/
        case AF_INCOMING_MSG_CMD:
          LockApp_MessageMSGCB( MSGpkt );   //调用回调函数对收到的数据进行处理
          break;

        /*设备网络状态变化事件，例如终端设备入网*/
        case ZDO_STATE_CHANGE:
          LockApp_NwkState = (devStates_t)(MSGpkt->hdr.status);//获取设备当前状态
          
          if ( (LockApp_NwkState == DEV_ZB_COORD)
              || (LockApp_NwkState == DEV_ROUTER)
              || (LockApp_NwkState == DEV_END_DEVICE) )
          {
            //设备组网成功
          }
          else
          {
            //设备离开网络
          }
          break;

        default:
          break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );         //释放消息占用的内存
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( LockApp_TaskID ); //在列表中检索下一条需要处理的消息事件
    }

    return (events ^ SYS_EVENT_MSG);                  //返回未处理的事件
  }

  /*定时事件*/
  if ( events & LOCKAPP_OFF_LINE_TASK_MSG_EVENT )
  {
    
    /*1.刷卡授权和开门*/
    Status = Card_ReadBlock(CARD_INFORMATION,BlockData); 
    
    /*1.1 授权卡操作*/
    if(Status == MFRC522_OK)        
    {
      /*1.1.1 门未反锁*/
      if(LOCK_PORT == LOCK_NO)      
      {
        /*1.1.1.1 门锁ID信息读取*/
        Data_DoorID_Read(DoorId); 
        
        /*1.1.1.2 此公租房此撞楼的特权卡*/
        if((BlockData[2] == DoorId[0]) && (BlockData[3] == DoorId[1]))    
        {
          
          /*1.1.1.2.1 特权卡和删权卡*/
          if(((BlockData[0] == Authorization)    && (BlockData[1] == Authorization)) ||
            ((BlockData[0] == UnAuthorizataion) && (BlockData[1] == UnAuthorizataion)))     
          {
             Door_Open(LedOn);
             LED_OFF();
             Delay_Ms(1000);
             Buzzer_One_Led(LedOn);
             Card_Authorization(BlockData[0],DoorId);      //卡号授权或删权处理
             Door_Close(LedOff);  
          }
          
          /*1.1.1.2.2 总卡*/
          else if((BlockData[0] == TotalCard) && (BlockData[1] == TotalCard))   
          {
            Door_Open_Close();
          }
        }
        
        /*1.1.1.3 可以读取扇区信息的普通卡操作*/
        else 
        {
          if(Data_CommonCard_Confirm(BlockData+12)) 
          {
            Door_Open_Close();   
          }
          
          else
          {
            Buzzer_Card_Fail();   //无权限卡警告
          }
        }
      }
      
      /*1.1.2 门反锁*/
      else 
      {
        Buzzer_Door_Lock();       //门反锁不需要提示音   
      }
      
      
      /*1.1.3 刷卡检测电池*/
      //暂时不做
      
      
      
    }
    
    /*1.2 不能读取扇区信息的普通卡操作(需要注意的离线授权模式的普通必须可以读取扇区，不过这里可以是远程授权的操作)*/
    else 
    {
      
      //使用Card_ReadID函数获取卡号ID处理
      
      
      
    }
    
    
    
   
    osal_start_timerEx( LockApp_TaskID, LOCKAPP_OFF_LINE_TASK_MSG_EVENT,  
        (LOCKAPP_OFF_LINE_TASK_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );


    return (events ^ LOCKAPP_OFF_LINE_TASK_MSG_EVENT);  //返回未处理的事件
  }

  return 0;
}

/*********************************************************************
 * 普通事件函数
 */

/*********************************
  Funcname:       LockApp_HandleKeys
  Description:    按键处理事件
  Author:         zhuxiankang
  parm:           keys -> 按键
*********************************/

void LockApp_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  
  /* 钥匙开门 */
  if ( keys & HAL_KEY_SW_6 ) {
    Buzzer_Key_Open();
  }
  
  /* 锁扣开门 */
  if ( keys & HAL_KEY_SW_7 ) {
    //IsDoorClose =  CLOSE_YES;
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      LockApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */

//接收数据函数
void LockApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  uint16 flashTime;
  byte buf[3];

  //判断接收到的簇ID
  switch ( pkt->clusterId )
  {
    //收到广播数据
    case LOCKAPP_PERIODIC_CLUSTERID:  

      /*用户添加**/
      osal_memset(buf,0,3);                   //先初始化buf全为0,清零操作
      osal_memcpy(buf,pkt->cmd.Data,2);
      
      if((buf[0] == '1') && (buf[1] == '1'))  //判断接收的是否为D1
      {
        //HalLedBlink(HAL_LED_2,0,50,500);      //LED2间隔500ms闪烁
        
        #if defined(ZDO_COORDINATOR)
        HalUARTWrite(0,"CoordinatorEB-Pro 57 Receive '11' OK\n", sizeof("CoordinatorEB-Pro 57 Receive 'D1' OK\n"));//串口发送
        //HalLedBlink(HAL_LED_1,0,50,500);      //LED1间隔500ms闪烁
        LockApp_SendPeriodicMessage();      //协调器接收到D1后返回D1给终端，让终端LED1也闪烁
        #endif
      }
      else
      {
        //HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
      }
      break;

    //收到组播数据
    case LOCKAPP_FLASH_CLUSTERID:
      flashTime = BUILD_UINT16(pkt->cmd.Data[1], pkt->cmd.Data[2] );
      //HalLedBlink( HAL_LED_4, 4, 50, (flashTime / 4) );
      break;
  }
}

/*********************************************************************
 * @fn      LockApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */

//发送周期信息
//在LockApp_ProcessEvent中的osal_start_timerEx中设置了5s的定时器
//5s以后的定时事件就是调用了LockApp_SendPeriodicMessage函数进行数据的发送
//所以每5s会发送一次数据，而且这个数据是广播数据
void LockApp_SendPeriodicMessage( void )
{
  
  byte SendData[3] = "11";
  
  //调用AF_DataRequest将数据无线广播出去
  if ( AF_DataRequest( &LockApp_Periodic_DstAddr,   //目的节点的地址信息
                       &LockApp_epDesc,             //端点信息
                       LOCKAPP_PERIODIC_CLUSTERID,  //簇ID信息，这里是发送的簇ID,与接收到的簇ID一一对应
                       2,                             //数据长度
                       SendData,                      //数据
                       &LockApp_TransID,            //发送消息ID
                       AF_DISCV_ROUTE,                //发送选项
                       AF_DEFAULT_RADIUS )            //最大跳数半径
      == afStatus_SUCCESS )    
  {
  }
  else
  {
    //HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      LockApp_SendFlashMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */
void LockApp_SendFlashMessage( uint16 flashTime )
{
  uint8 buffer[3];
  buffer[0] = (uint8)(LockAppFlashCounter++);
  buffer[1] = LO_UINT16( flashTime );
  buffer[2] = HI_UINT16( flashTime );

  if ( AF_DataRequest( &LockApp_Flash_DstAddr, &LockApp_epDesc,
                       LOCKAPP_FLASH_CLUSTERID,
                       3,
                       buffer,
                       &LockApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
*********************************************************************/
