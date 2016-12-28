#ifndef APP_TCP_H_
#define APP_TCP_H_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_types.h"
#include<ioCC2530.h>
#include <string.h>
#include <stdbool.h>
#include "OSAL.h"
#include "dri_uart.h"



/*数据帧最大长度*/
#define Rx_BUFF                 0xFF
#define Tx_BUFF                 0xFF


/* ------------------------------------------------------------------------------------------------
 *                                      variable
 * ------------------------------------------------------------------------------------------------
 */
#ifndef APP_TCP_C_
  #define TCP_C_Extern extern
#else
  #define TCP_C_Extern
#endif


/*全局变量*/
typedef struct{
  
  //和接收数据的处理程序有关
  uint8 RxData;           //接收到的数据
  uint8 RxStatus;         //从上位机服务器发送的数据帧包接收情况
  uint8 RxProgress;       //当前一帧数据的接收进程
  uint8 RxCount;          //计数值
  
  
  //接收数据的具体数据
  uint8 CMD;              //命令字
  uint8 LENG;             //帧长度
  uint8 DOOR_ID[2];       //门锁ID
  uint8 RESP;             //命令状态
  uint8 DATA[Rx_BUFF];    //数据，最多可以接收255字节
  uint8 LRC;              //校验
  
}Typedef_Frame;

TCP_C_Extern Typedef_Frame Frame;      //当前文件和BaseApp.c是全局变量




/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */

/*IPORT端口定义*/
#define RST         P0_1                    //低电平复位有效
#define FUNCTION    P0_4                    //默认情况下，TCP Server、TCP Client等在连接建立时输出低电平，连接断开时输出高电平               


/****************************************数据帧宏定义******************************************/

/*数据帧包接收情况*/
#define FRAME_RX_START          11          //开始准备接收数据或者空闲
#define FRAME_RX_OK             22          //完整接收了一帧数据命令
#define FRAME_RX_ERR            33          //数据帧有误

/*一帧数据的接收进程*/
#define FRAME_RX_SOP_START      1           //SOP 帧头1
#define FRAME_RX_SOP_CHECK      2           //SOP 帧头2
#define FRAME_RX_LENG           3           //LENG 帧数据长度
#define FRAME_RX_CMD            4           //CMD 传输的命令类型
#define FRAME_RX_DOOR_ID        5           //DOOR_ID门锁ID
#define FRAME_RX_DATA           6           //DATA 传输的数据
#define FRAME_RX_LRC            7           //LRC 数据的异或和
#define FRAME_RX_EOP            8           //EOP 帧尾
#define FRAME_RX_END            9           //数据帧如果接收错误则退出数据帧





/*数据帧字节*/
#define SOP                     0xAA
#define EOP                     0x0E      

/*数据帧的最小长度*/
#define MIN_LENG                0x03    //上位机发送数据帧的最小长度，从CMD ~ DATA[n]的长度
#define FPB_MIN_LENG            0x04    //上位机接收数据帧的最小长度

/*数据帧的当前最大命令数*/
#define MAX_CMD                 0x02    //命令逐1增加


/****************************************命令类型宏定义******************************************/

//命令类型
#define BASE_CMD_INIT                 0x00        //命令初始化，空命令
#define BASE_CMD_GET_ASSOCLIST        0x01        //获取基站的关联列表   
#define BASE_CMD_OPEN_DOOR            0x02        //远程开门



//反馈状态
#define BASE_RESP_OK            0xEE        //操作成功


//帧错误反馈
#define BASE_RESP_LENG_ERR      0x10        //接收数据帧不完整
#define BASE_RESP_CMD_ERR       0x11        //命令字未知
#define BASE_RESP_LRC_ERR       0x12        //LRC校验错误
#define BASE_RESP_EOP_ERR       0x13        //帧尾错误

//命令类型反馈
//#define BASE_RESP_NO_DOOR_LIST  0x20        //没有门锁关联列表信息





/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */

//数据帧接收处理
void Tcp_Init(void);
void Tcp_Receive(uint8 Rx_Data);
void Tcp_Process(uint8 Rx_Data);
void Tcp_Send(uint8 FPB_CMD, uint8* DoorId, uint8 RESP, uint8 *data, uint8 DataLength);













/**************************************************************************************************
 */
#endif