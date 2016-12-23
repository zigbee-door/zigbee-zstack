/**************************************************************************************************
  Filename:       BaseApp.c
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

#include "BaseApp.h"
//#include "BaseAppHw.h"

#include "OnBoard.h"


/* 硬件抽象层HAL */   
#include "hal_led.h"    
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"

#include <stdio.h>

#include "dri_delay.h"


/*********************************************************************
 * 全局变量
 */

/*输入输出簇列表，命令列表*/
const cId_t BaseApp_ClusterList[BASEAPP_MAX_CLUSTERS] = 
{
  BROADCAST_TEST_ID,      //协调器广播测试命令
  SINGLE_TEST_ID,         //终端单播数据测试,门锁1
  SINGLE_TEST_ID_2,       //终端单播数据测试,门锁2    
  SINGLE_TEST_ID_3        //协调器单播数据给门锁测试
};

/*Zigbee简单端点描述符*/
const SimpleDescriptionFormat_t BaseApp_SimpleDesc = 
{
  BASEAPP_ENDPOINT,              //端点号:20
  BASEAPP_PROFID,                //端点的簇ID:0x0F08
  BASEAPP_DEVICEID,              //设备ID:0x0001
  BASEAPP_DEVICE_VERSION,        //设备描述符版本:0
  BASEAPP_FLAGS,                 //保留
  BASEAPP_MAX_CLUSTERS,          //端点支持的输入簇个数:2
  (cId_t *)BaseApp_ClusterList,  //输入簇列表
  BASEAPP_MAX_CLUSTERS,          //点支持的输出簇个数:2 
  (cId_t *)BaseApp_ClusterList   //输出簇列表（输入簇和输出簇要一一对应）
};

/*端点信息结构体*/
endPointDesc_t BaseApp_epDesc;


/*********************************************************************
 * 本地变量
 */

uint8 BaseApp_TaskID;                   // 任务ID 
devStates_t BaseApp_NwkState;           //网络状态

uint8 BaseApp_TransID;                  // 传输序列ID

afAddrType_t BaseApp_BroadCast_DstAddr; //发送数据的广播目的地址
afAddrType_t BaseApp_Single_DstAddr;    //发送数据的单播目的地址

aps_Group_t BaseApp_Group;              //组信息定义

uint8 BaseAppPeriodicCounter = 0;       //发送次数统计
uint8 BaseAppFlashCounter = 0;



/*********************************************************************
 * 关联表
 */

typedef struct
{
  //associated_devices_t AssociatedDevList[21];   z-stack的关联表
  uint8 DoorId[NWK_MAX_DEVICES];      //每把门锁对应一个房间ID
  uint8 BatteryInfo[NWK_MAX_DEVICES]; //0bit: 1->节点网络存活 0->节点网络失效 1~7bit: 电池百分比
} associate_t;

associate_t associateList;      



/*********************************************************************
 * 本地函数
 */
void BaseApp_HandleKeys( uint8 shift, uint8 keys ); 
void BaseApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void BaseApp_SendMessage(afAddrType_t *DstAddr, uint8 *SendData, uint8 len, uint8 Cmd);  //基站发送命令给门锁的函数



/*********************************************************************
 * 公共函数
 */

/*********************************
  Funcname:       BaseApp_Init
  Description:    任务初始化函数
  Author:         zhuxiankang
  parm:           task_id -> 任务序列号
*********************************/

void BaseApp_Init( uint8 task_id )
{ 
  /*变量初始化*/
  BaseApp_TaskID = task_id;         //任务ID
  BaseApp_NwkState = DEV_INIT;      //网络状态初始化为DEV_INIT
  BaseApp_TransID = 0;              //传输序列ID  
  
  //------------------------配置串口---------------------------------
  MT_UartInit();                    //串口初始化
  MT_UartRegisterTaskID(task_id);   //注册串口任务
  HalUARTWrite(0,"UartInit OK\n", sizeof("UartInit OK\n"));
  
  
  
  /*单播设置*/  
  BaseApp_Single_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;        //3:单播
  BaseApp_Single_DstAddr.endPoint = BASEAPP_ENDPOINT;               //端点号：20
  //BaseApp_Periodic_DstAddr.addr.shortAddr = 0x0000;               //协调器地址
  
  /*广播设置*/
  BaseApp_BroadCast_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;  //15:广播
  BaseApp_BroadCast_DstAddr.endPoint = BASEAPP_ENDPOINT;             //端点号：20
  BaseApp_BroadCast_DstAddr.addr.shortAddr = BROADCAST_ADDR;         //广播地址 
  
 
  /*定义本设备用来通信的APS层端点描述符*/
  BaseApp_epDesc.endPoint = BASEAPP_ENDPOINT;                     //端点号：20
  BaseApp_epDesc.task_id = &BaseApp_TaskID;                       //任务ID 
  BaseApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&BaseApp_SimpleDesc;   //Zigbee简单端点描述符
  BaseApp_epDesc.latencyReq = noLatencyReqs;                      //无延时策略
  
  /*在AF中注册端点20*/
  afRegister( &BaseApp_epDesc );

  /*注册按键事件*/
  //RegisterForKeys( BaseApp_TaskID );
  
  /*基站定时任务*/
  osal_start_timerEx( BaseApp_TaskID,BASEAPP_OFF_LINE_TASK_MSG_EVENT,BASEAPP_OFF_LINE_TASK_MSG_TIMEOUT );
  
}


//uint8 sendNum = 0;

/*********************************
  Funcname:       BaseApp_ProcessEvent
  Description:    任务事件处理函数
  Author:         zhuxiankang
  parm:           task_id -> 任务序列号 evnets -> 事件触发标志
*********************************/
uint16 BaseApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;     //接收消息结构体
  (void)task_id;  

  if ( events & SYS_EVENT_MSG )      //接收到系统消息
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( BaseApp_TaskID ); //调用系统信息，接收消息包
    
    while ( MSGpkt )
    {
      
      switch ( MSGpkt->hdr.event )  
      {
        /*消息到来响应事件*/
        case AF_INCOMING_MSG_CMD:
          BaseApp_MessageMSGCB( MSGpkt );   //调用回调函数对收到的数据进行处理
          break;

        /*设备网络状态变化事件，例如终端设备入网*/
        case ZDO_STATE_CHANGE:
          BaseApp_NwkState = (devStates_t)(MSGpkt->hdr.status);//获取设备当前状态
          
          if (BaseApp_NwkState == DEV_ZB_COORD)
              //|| (BaseApp_NwkState == DEV_ROUTER)
              //|| (BaseApp_NwkState == DEV_END_DEVICE) )
          {           
            //HAL_TOGGLE_LED1();   
            //HalUARTWrite(0,"State Change OK\n", sizeof("State Change OK\n"));
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
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( BaseApp_TaskID ); //在列表中检索下一条需要处理的消息事件
    }

    return (events ^ SYS_EVENT_MSG);                  //返回未处理的事件
  }

  /*定时事件*/
  if ( events & BASEAPP_OFF_LINE_TASK_MSG_EVENT )
  {
//    uint8 data[2] = {0x44,0x45};
//    uint8 data2[2] = {0x44,0x46};
//    
//    
//    //设备1先连上
//    if(AssociatedDevList[0].shortAddr != 0xFFFF) 
//    {
//      BaseApp_Single_DstAddr.addr.shortAddr = AssociatedDevList[0].shortAddr;
//      BaseApp_SendMessage(&BaseApp_Single_DstAddr,data,2,SINGLE_TEST_ID_3);
//    } 
//   
//    if(AssociatedDevList[1].shortAddr != 0xFFFF)
//    {
//      BaseApp_Single_DstAddr.addr.shortAddr = AssociatedDevList[1].shortAddr;
//      BaseApp_SendMessage(&BaseApp_Single_DstAddr,data2,2,SINGLE_TEST_ID_3);
//    }


    osal_start_timerEx( BaseApp_TaskID, BASEAPP_OFF_LINE_TASK_MSG_EVENT,  
        (BASEAPP_OFF_LINE_TASK_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );


    return (events ^ BASEAPP_OFF_LINE_TASK_MSG_EVENT);  //返回未处理的事件
  }

  return 0;
}

/*********************************************************************
 * 普通事件函数
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      BaseApp_MessageMSGCB
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
void BaseApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{


  //判断接收到的簇ID
  switch ( pkt->clusterId )
  {
    //广播测试命令,门锁1
    case BROADCAST_TEST_ID: 
      break;
      
    //单播测试命令,门锁1
    case SINGLE_TEST_ID:
      break;
      
    //单播测试命令,门锁2
    case SINGLE_TEST_ID_2:
      break;
  }
}





/*********************************************************************
*********************************************************************/

/*********************************
  Funcname:       BaseApp_ProcessEvent
  Description:    发送RF射频数据函数
  Author:         zhuxiankang
  parm:           SendData -> 发送的数据 
                  Cmd      -> 命令类型（或者也可以说串/簇ID/群集ID）
*********************************/
void BaseApp_SendMessage(afAddrType_t *DstAddr, uint8 *SendData, uint8 len, uint8 Cmd)
{
  
  if ( AF_DataRequest( //&BaseApp_Periodic_DstAddr,   //目的节点的地址信息
                       DstAddr,
                       &BaseApp_epDesc,               //端点信息
                       Cmd,                           //命令类型
                       len,                           //数据长度
                       SendData,                      //数据
                       &BaseApp_TransID,              //发送消息ID
                       AF_DISCV_ROUTE,                //发送选项
                       AF_DEFAULT_RADIUS )            //最大跳数半径
      == afStatus_SUCCESS )    
  {
    HAL_TOGGLE_LED2();
  }
  else
  {
    
  }
}