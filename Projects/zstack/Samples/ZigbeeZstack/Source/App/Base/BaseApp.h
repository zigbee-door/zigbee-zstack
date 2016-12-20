/**************************************************************************************************
  Filename:       BaseApp.h
  Date:           2016-12-13 $
  Revision:       1.0
  Description:    门锁应用程序头文件
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
#define BASEAPP_MAX_CLUSTERS       2          //最大输入/输出簇个数(这里主要分为广播和组播)
#define BASEAPP_PERIODIC_CLUSTERID 1          //广播簇ID,包括输入和输出
#define BASEAPP_FLASH_CLUSTERID     2         //组播簇ID,包括输入和输出

  
/*门锁离线刷卡开门任务*/
#define BASEAPP_OFF_LINE_TASK_MSG_TIMEOUT   800    //门锁离线任务执行时间间隔0.8s
#define BASEAPP_OFF_LINE_TASK_MSG_EVENT     0x0001  //门锁离线任务定时事件标志
  

  
// Flash Command Duration - in milliseconds
#define BASEAPP_FLASH_DURATION               1000
  
  
  
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
