/**************************************************************************************************
  Filename:       BaseApp.h
  Date:           2016-12-13 $
  Revision:       1.0
  Description:    基站应用程序头文件
  Author:         zhuxiankang
**************************************************************************************************/

#ifndef BASEAPP_H
#define BASEAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * 常量
 */

/*Zigbee设备节点简单端点描述符常量*/
#define BASEAPP_ENDPOINT           20         //端点号定义为20
#define BASEAPP_PROFID             0x0F08     //簇ID
#define BASEAPP_DEVICEID           0x0001     //设备ID
#define BASEAPP_DEVICE_VERSION     0          //设备版本号
#define BASEAPP_FLAGS              0          //保留
#define BASEAPP_MAX_CLUSTERS       1          //最大输入/输出命令（簇）个数
  
  
  



/*命令类型（簇ID）*/
//#define BROADCAST_TEST_ID   0x01          //协调器广播测试命令
//#define SINGLE_TEST_ID      0x02          //终端单播数据测试,门锁1
//#define SINGLE_TEST_ID_2    0x03          //终端单播数据测试,门锁2
//#define SINGLE_TEST_ID_3    0x04          //协调器单播数据给门锁测试
#define OPEN_DOOR_CMD_ID         0x01       //远程开门  
  
  
  
/*基站定时任务（暂时没用）*/
#define BASEAPP_OFF_LINE_TASK_MSG_TIMEOUT   2000    //基站离线任务
  
#define BASEAPP_OFF_LINE_TASK_MSG_EVENT     0x0001  //基站离线任务定时事件标志
#define BASEAPP_TCP_RECEIVE_ERR_MSG_EVENT   0x0002  //基站接收上位机数据帧错误,暂时没用
#define BASEAPP_TCP_RECEIVE_TRUE_MSG_EVENT  0x0004  //基站接收上位机数据帧正确  
//#define BASEAPP_TCP_RECEIVE_ERR_MSG_EVENT   0x0004  //事件编号在一个任务中必须是唯一的，而且必须是通过左移1位的方式来实现  

/*广播地址*/
#define BROADCAST_ADDR            0xFFFF  
  
  
// Flash Command Duration - in milliseconds
#define BASEAPP_FLASH_DURATION               1000
  
/*射频最大发送和接收数据量*/
#define RF_MAX_BUFF         0xFF   
  
/*********************************************************************
 * 函数声明
 */
extern void BaseApp_Init( uint8 task_id );
extern UINT16 BaseApp_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BASEAPP_H */
