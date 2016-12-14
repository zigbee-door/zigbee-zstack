/**************************************************************************************************
  Filename:       BaseApp.c
  Date:           2016-12-13 $
  Revision:       1.0
  Description:    基站应用程序
  Author:         zhuxiankang
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "BaseApp.h"
//#include "BaseAppHw.h"  这个暂时不用,这里没有添加Hw的文件

#include "OnBoard.h"


/* HAL */
#include "hal_lcd.h" //这里是硬件抽象层的头文件
#include "hal_led.h"
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs.
const cId_t BaseApp_ClusterList[BASEAPP_MAX_CLUSTERS] = //输入输出簇列表，这个是否也可以作为命令簇ID?
{
  BASEAPP_PERIODIC_CLUSTERID,    //广播的簇:0x01
  BASEAPP_FLASH_CLUSTERID        //组播的簇:0x02
};

const SimpleDescriptionFormat_t BaseApp_SimpleDesc = //Zigbee简单端点描述符定义
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

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in BaseApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t BaseApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

uint8 BaseApp_TaskID;   // 任务ID Task ID for internal task/event processing
                          // This variable will be received when
                          // BaseApp_Init() is called.
devStates_t BaseApp_NwkState;   //网络状态

uint8 BaseApp_TransID;  // 传输序列ID This is the unique message ID (counter)

afAddrType_t BaseApp_Periodic_DstAddr;  //发送数据的广播目的地址
afAddrType_t BaseApp_Flash_DstAddr;     //发送数据的组播目的地址

aps_Group_t BaseApp_Group;              //组信息定义

uint8 BaseAppPeriodicCounter = 0;       //发送次数统计
uint8 BaseAppFlashCounter = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void BaseApp_HandleKeys( uint8 shift, uint8 keys ); //函数声明
void BaseApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void BaseApp_SendPeriodicMessage( void );
void BaseApp_SendFlashMessage( uint16 flashTime );

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      BaseApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */



/*任务初始化函数！！！！！！！！！！！！！！！！！*/

void BaseApp_Init( uint8 task_id )
{ 
  BaseApp_TaskID = task_id;         //osal分配的任务ID，随着用户添加任务的增多而改变
  BaseApp_NwkState = DEV_INIT;      //网络状态初始化为DEV_INIT
  /*初始化应用设备的网络类型，设备类型的改变都要产生一个事件—ZDO_STATE_CHANGE，从字面理解为
//ZDO状态发生了改变。所以在设备初始化的时候一定要把它初始化为什么状态都没有。那么它就要去检测
//整个环境，看是否能重新建立或者加入存在的网络。但是有一种情况例外，就是当NV_RESTORE被设置的
//时候（NV_RESTORE是把信息保存在非易失存储器中），那么当设备断电或者某种意外重启时，由于网络
//状态存储在非易失存储器中，那么此时就只需要恢复其网络状态，而不需要重新建立或者加入网络了*/
  BaseApp_TransID = 0;              //传输序列号初始化0,消息发送ID（多消息时有顺序之分）
  
  //------------------------配置串口---------------------------------
  MT_UartInit();                    //串口初始化
  MT_UartRegisterTaskID(task_id);   //注册串口任务
  HalUARTWrite(0,"UartInit OK\n", sizeof("UartInit OK\n"));
  //-----------------------------------------------------------------
  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in BaseAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES
//该段的意思是，如果设置了 HOLD_AUTO_START 宏定义，
//将会在启动芯片的时候会暂停启动流程，
//只有外部触发以后才会启动芯片。其实就是需要一个按钮触发它的启动流程。
  
#if defined ( HOLD_AUTO_START ) 
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);   //ZDO初始化
#endif

  
  /* 设置发送数据的方式和目的地址寻址模式*/
  //---------------------------
  //周期消息，广播发送
  // Setup for the periodic message's destination address
  // Broadcast to everyone
  /*广播到所有设备，数据的广播地址初始化*/
  BaseApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;//15:广播
  BaseApp_Periodic_DstAddr.endPoint = BASEAPP_ENDPOINT;         //端点号：20
  BaseApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;               //16位网络地址:0xFFFF
  
  /*单播到一个设备*/
//  (WXL_BaseApp_Single_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;;
//  WXL_BaseApp_Single_DstAddr.endPoint = WXL_BASEAPP_ENDPOINT;
//  BaseApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;
 
//  BaseApp_Periodic_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;    //2:单播
//  BaseApp_Periodic_DstAddr.endPoint = BASEAPP_ENDPOINT;         //端点号：20
  //  BaseApp_Periodic_DstAddr.addr.shortAddr = 0x0000;               //16位网络地址:0x0000
  
  
  
  
  //--------------------------
  /*数组的组播信息初始化*/
  // Setup for the flash command's destination address - Group 1
  BaseApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;    //1:组播
  BaseApp_Flash_DstAddr.endPoint = BASEAPP_ENDPOINT;           //端点号：20
  BaseApp_Flash_DstAddr.addr.shortAddr = BASEAPP_FLASH_GROUP;  //16位网络地址:0x0001,同时设置为组号

  // Fill out the endpoint description.
  /*定义本设备用来通信的APS层端点描述符*/
  /*端点信息初始化*/
  BaseApp_epDesc.endPoint = BASEAPP_ENDPOINT;                   //BaseApp EP描述符的EP号：20
  BaseApp_epDesc.task_id = &BaseApp_TaskID;                     //BaseApp EP描述符的任务ID，详细见函数头部 
  BaseApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&BaseApp_SimpleDesc;   //BaseApp EP简单描述符
  BaseApp_epDesc.latencyReq = noLatencyReqs;                      //延时策略

  // Register the endpoint description with the AF
  /*向AF层登记EP描述符*/
  /*登记endpoint description 到AF,要对该应用进行初始化并在AF进行登记，告诉应用层有这么一个EP已
  经开通可以使用，那么下层要是有关于该应用的信息或者应用要对下层做哪些操作，就自动得到下层的配
  合。*/
  afRegister( &BaseApp_epDesc );

  // Register for all key events - This app will handle all key events
  /*注册按键事件*/
  RegisterForKeys( BaseApp_TaskID );

  // By default, all devices start out in Group 1 为闪烁消息配置的组
  /*组信息初始化*/
  BaseApp_Group.ID = 0x0001;                          //组号，和BASEAPP_FLASH_GROUP一样
  osal_memcpy( BaseApp_Group.name, "Group 1", 7  );   //设定组名
  aps_AddGroup( BASEAPP_ENDPOINT, &BaseApp_Group ); //把该组登记添加到APS中

  /*如果支持LCD，显示一串字符*/ 
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "BaseApp", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      BaseApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */

/*事件处理函数！！！！！！！！！！！！！！！！！*/
uint16 BaseApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;     //接收到的消息，消息包
  (void)task_id;  

  /*如果大事件是接收系统消息*/ //则接收系统消息再进行判断
  //提取系统类消息
  
  //系统消息可以分为按键事件，AF数据接收事件和网络状态改变事件
  if ( events & SYS_EVENT_MSG )
  {
    //这里是接收到的数据包，按照结构体afIncomingMSGPacket_t的格式接收信息
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( BaseApp_TaskID ); //调用系统信息，接收消息包
    
    //如果接收到信息
    while ( MSGpkt )//接收到
    {
      //先判断接收到的信息是哪个事件类型
      switch ( MSGpkt->hdr.event )
      {
      
        /*按键处理事件*/
        case KEY_CHANGE:
          BaseApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break; //执行具体的按键处理函数，定义在sampleAPP.c中

          
          
         /*消息到来响应事件*/
        // 接收数据事件,调用函数AF_DataRequest()接收数据
        // 这个是用的最多的接收信息事件
        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD:
          BaseApp_MessageMSGCB( MSGpkt );//调用回调函数对收到的数据进行处理
          break;

        //BaseApp.c BaseApp_ProcessEvent函数内
        /*设备网络状态变化事件*/
        // 只要网络状态发生改变，就通过ZDO_STATE_CHANGE事件通知所有的任务，注意，是所有任务都会收到这消息。
          
        //这里设置了一个定时函数osal_start_timerEx()，这个定时函数由网络状态变化这个事件来触发
        //这里旨在网络状态发生变化时通知所有的其他设备？？？？
        //也可以用作定时上传传感器数据？？？
        case ZDO_STATE_CHANGE:
          BaseApp_NwkState = (devStates_t)(MSGpkt->hdr.status);//获取设备当前状态
          
          //这里是设置要定时广播数据的设备
          //例如协调器一般是不用定时广播数据的,那么这里可以屏蔽
          if ( (BaseApp_NwkState == DEV_ZB_COORD)
              || (BaseApp_NwkState == DEV_ROUTER)
              || (BaseApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending the periodic message in a regular interval.
            /*按一定间隔启动定时器*/
            //这个定时器是为发送周期信息设置的，我觉得因为在这个例子中,用户自己添加的任务,
            //只有两个事件是用于向外发送消息的,一个是发送flash闪烁消息，属于组寻址，
            //而另一个是发送periodic周期消息，属于广播；这里是一个设备的网络状态发生了变化，
            //必须要告诉同一网络中的其它设备，因此要进行广播通知其它设备…发送的消息中应该会包括本设备的类型。
            //……不知道这样理解对不对～管它的，以后会明白的～
            //更新：这个定时器只是为发送周期信息开启的，设备启动初始化后从这里开始触发第一个周期信息的发送，
            //然后周而复始下去.
            //BaseApp.c BaseApp_ProcessEvent函数内
            osal_start_timerEx( BaseApp_TaskID,
                              BASEAPP_SEND_PERIODIC_MSG_EVT,
                              BASEAPP_SEND_PERIODIC_MSG_TIMEOUT );
          }
          else
          {
            // Device is no longer in the network
          }
          break;

        default:
          break;
      }

      // Release the memory
      //以上把收到系统消息这个大事件处理完了，释放消息占用的内存
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      /*指针指向下一个"已接收到的”［程序在while ( MSGpkt )内］放在缓冲区的待处理的事件，与
      // BaseApp_ProcessEvent处理多个事件相对应，返回while ( MSGpkt )重新处理事件，
      //直到缓冲区没有等待处理事件为止。*/
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( BaseApp_TaskID );
    }

    // return unprocessed events
     //两者相异或，返回未处理的事件，return到osal_start_system()下的events = (tasksArr[idx])( idx, events )语句中，重新在osal_start_system()下轮询再进入此函数进行处理。
    return (events ^ SYS_EVENT_MSG);  //清除系统类事件，返回未处理的任务
  }

  //BaseApp.c BaseApp_ProcessEvent函数内
  // Send a message out - This event is generated by a timer
  //  (setup in BaseApp_Init()).
  /* 如果大事件是向外发送信息，该事件由定时器产生*/
  //如果是定时事件，则进行定时事件处理
  if ( events & BASEAPP_SEND_PERIODIC_MSG_EVT )
  {
    // Send the periodic message
    
    //处理周期性事件，利用BaseApp_SendPeriodicMessage()处理完当前的周期性事件，
    //然后启动定时器开启下一个周期性事情，
    //这样一种循环下去，也即是上面说的周期性事件了，
    //可以做为传感器定时采集、上传任务
    
    BaseApp_SendPeriodicMessage();          //这里是发送函数，很重要！！！！！！！！！！！！！！！！！！！！

    // Setup to send message again in normal period (+ a little jitter)
    //这里为任务BaseApp_TaskID的事件BASEAPP_SEND_PERIODIC_MSG_EVT设定一个定时器，
    //定时时间为 (BASEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF))，
    //当时间一到，该运行的任务将被通报有事件发生。
    //循环调用定时器
    osal_start_timerEx( BaseApp_TaskID, BASEAPP_SEND_PERIODIC_MSG_EVT,
        (BASEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events
    return (events ^ BASEAPP_SEND_PERIODIC_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */
/*********************************************************************
 * @fn      BaseApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void BaseApp_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  
  //如果按键1被按下，则组播数据
  if ( keys & HAL_KEY_SW_1 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
    BaseApp_SendFlashMessage( BASEAPP_FLASH_DURATION );
  }
  
  //如果按键2被按下，则移除组
  if ( keys & HAL_KEY_SW_2 )
  {
    /* The Flashr Command is sent to Group 1.
     * This key toggles this device in and out of group 1.
     * If this device doesn't belong to group 1, this application
     * will not receive the Flash command sent to group 1.
     */
    aps_Group_t *grp;
    grp = aps_FindGroup( BASEAPP_ENDPOINT, BASEAPP_FLASH_GROUP );
    if ( grp )
    {
      // Remove from the group
      aps_RemoveGroup( BASEAPP_ENDPOINT, BASEAPP_FLASH_GROUP );
    }
    else
    {
      // Add to the flash group
      aps_AddGroup( BASEAPP_ENDPOINT, &BaseApp_Group );
    }
  }
}

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
  uint16 flashTime;
  byte buf[3];

  //判断接收到的簇ID
  switch ( pkt->clusterId )
  {
    //收到广播数据
    case BASEAPP_PERIODIC_CLUSTERID:  

      /*用户添加**/
      osal_memset(buf,0,3);                   //先初始化buf全为0,清零操作
      osal_memcpy(buf,pkt->cmd.Data,2);
      
      if((buf[0] == '1') && (buf[1] == '1'))  //判断接收的是否为D1
      {
        HalLedBlink(HAL_LED_2,0,50,500);      //LED2间隔500ms闪烁
        
        #if defined(ZDO_COORDINATOR)
        HalUARTWrite(0,"CoordinatorEB-Pro 57 Receive '11' OK\n", sizeof("CoordinatorEB-Pro 57 Receive 'D1' OK\n"));//串口发送
        HalLedBlink(HAL_LED_1,0,50,500);      //LED1间隔500ms闪烁
        BaseApp_SendPeriodicMessage();      //协调器接收到D1后返回D1给终端，让终端LED1也闪烁
        #endif
      }
      else
      {
        HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
      }
      break;

    //收到组播数据
    case BASEAPP_FLASH_CLUSTERID:
      flashTime = BUILD_UINT16(pkt->cmd.Data[1], pkt->cmd.Data[2] );
      HalLedBlink( HAL_LED_4, 4, 50, (flashTime / 4) );
      break;
  }
}

/*********************************************************************
 * @fn      BaseApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */

//发送周期信息
//在BaseApp_ProcessEvent中的osal_start_timerEx中设置了5s的定时器
//5s以后的定时事件就是调用了BaseApp_SendPeriodicMessage函数进行数据的发送
//所以每5s会发送一次数据，而且这个数据是广播数据
void BaseApp_SendPeriodicMessage( void )
{
  
  byte SendData[3] = "11";
  
  //调用AF_DataRequest将数据无线广播出去
  if ( AF_DataRequest( &BaseApp_Periodic_DstAddr,   //目的节点的地址信息
                       &BaseApp_epDesc,             //端点信息
                       BASEAPP_PERIODIC_CLUSTERID,  //这个也可以是命令ID? 簇ID信息，这里是发送的簇ID,与接收到的簇ID一一对应
                       //这个也是串ID，通过判断串ID来达到相应的作用，根据不同的串ID设置不同的功能，这样ID和功能形成了一一对应的关系
                       //在无线控制的过程中，不需要传输大量的命令，只需要传输串ID，然后通过串ID判断需要执行的命令就可以了，这样既保证了数据的安全性
                       //和通信的可靠性，又提高了通信的效率 ZigBee2007/PRO协议栈实验与实践 P160
                       2,                             //数据长度
                       SendData,                      //数据
                       &BaseApp_TransID,            //发送消息ID
                       AF_DISCV_ROUTE,                //发送选项
                       AF_DEFAULT_RADIUS )            //最大跳数半径
      == afStatus_SUCCESS )    
  {
  }
  else
  {
    HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      BaseApp_SendFlashMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */
void BaseApp_SendFlashMessage( uint16 flashTime )
{
  uint8 buffer[3];
  buffer[0] = (uint8)(BaseAppFlashCounter++);
  buffer[1] = LO_UINT16( flashTime );
  buffer[2] = HI_UINT16( flashTime );

  if ( AF_DataRequest( &BaseApp_Flash_DstAddr, &BaseApp_epDesc,
                       BASEAPP_FLASH_CLUSTERID,
                       3,
                       buffer,
                       &BaseApp_TransID,
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
