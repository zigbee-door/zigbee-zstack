/**************************************************************************************************
  Filename:       LockApp.h
  Date:           2016-12-13 $
  Revision:       1.0
  Description:    门锁应用程序头文件
  Author:         zhuxiankang
**************************************************************************************************/

#ifndef LOCKAPP_H
#define LOCKAPP_H

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
#define LOCKAPP_ENDPOINT           20         //端点号定义为20
#define LOCKAPP_PROFID             0x0F08     //簇ID
#define LOCKAPP_DEVICEID           0x0001     //设备ID
#define LOCKAPP_DEVICE_VERSION     0          //设备版本号
#define LOCKAPP_FLAGS              0          //保留  
#define LOCKAPP_MAX_CLUSTERS       4          //最大输入/输出命令（簇）个数
  
  
  



/*命令类型（簇ID）*/
#define BROADCAST_TEST_ID   0x01          //协调器广播测试命令
#define SINGLE_TEST_ID      0x02          //终端单播数据测试,门锁1
#define SINGLE_TEST_ID_2    0x03          //终端单播数据测试,门锁2
#define SINGLE_TEST_ID_3    0x04          //协调器单播数据给基站测试
  




  
  
  
/*门锁离线刷卡开门任务*/
#define LOCKAPP_OFF_LINE_TASK_MSG_TIMEOUT   3000    //门锁离线任务执行时间间隔0.8s
#define LOCKAPP_OFF_LINE_TASK_MSG_EVENT     0x0001  //门锁离线任务定时事件标志
  
  
  
/*协调器地址*/
#define COORDINATOR_ADDR                    0x0000  
  
  
  

  
  
  
/*********************************************************************
 * 函数声明
 */
extern void LockApp_Init( uint8 task_id );
extern UINT16 LockApp_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LOCKAPP_H */
