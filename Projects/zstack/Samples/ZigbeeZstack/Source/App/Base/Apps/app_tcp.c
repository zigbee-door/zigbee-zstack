#define APP_DELAY_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "app_tcp.h"
#include "hal_mcu.h"      //这里要调用中断的宏定义



/*全局变量*/
typedef struct{
  uint8 RxStatus;         //从上位机服务器发送的数据帧包接收情况
  uint8 RxProgress;       //当前一帧数据的接收进程
  uint8 RxCount;          //计数值
  uint8 CMD;              //命令字
  uint8 LENG;             //帧长度
  uint8 SEQ;              //帧序列
  uint8 BASE_ID[2];       //基站ID
  uint8 DOOR_ID[2];       //门锁ID
  uint8 RESP;             //命令状态
  uint8 DATA[Rx_BUFF];    //数据
  uint8 LRC;              //校验
  
}Typedef_Frame;

Typedef_Frame Frame; 






/* ------------------------------------------------------------------------------------------------
 *                                          中断函数
 * ------------------------------------------------------------------------------------------------
 */
uint8 Uart0_Rx;


HAL_ISR_FUNCTION( UART0_ISR, URX0_VECTOR )
{
  URX0IF    = 0;  
  Uart0_Rx  = U0DBUF;    
  Tcp_Receive(Uart0_Rx);
}


/* ------------------------------------------------------------------------------------------------
 *                                          应用函数
 * ------------------------------------------------------------------------------------------------
 */



/**************************************************************************************************
 * @fn          Tcp_Init
 * @brief       数据帧参数初始化
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Tcp_Init(void)
{
  Frame.RxStatus   =  FRAME_RX_START;  
  Frame.RxProgress =  FRAME_RX_SOP_START;
  Frame.RxCount   =  0x00;
  
  Frame.CMD        =  0x00;
  Frame.LENG       =  0x00;
  Frame.SEQ        =  0x00;
  memset(Frame.BASE_ID,0x00,2);
  memset(Frame.DOOR_ID,0x00,2);
  Frame.RESP       =  0x00;
  memset(Frame.DATA,0x00,Rx_BUFF);
  Frame.LRC        =  0x00;
 
}


/**************************************************************************************************
 * @fn          Tcp_Receive
 * @brief       接受服务器发送的TCP数据字节
 * @param       Rx_Data -> 串口接收的数据
 * @return      无
 **************************************************************************************************
 */
void Tcp_Receive(uint8 Rx_Data)
{
  uint8 data[2] = {0x44,0x45};
  
  
  if(Frame.RxStatus == FRAME_RX_START)     //这个条件暂时不去掉，接收数据的实时性可能会受影响
  {
    //TcpClient_Process(Rx_Data);
  }
}






/**************************************************************************************************
 * @fn          Tcp_SendtoPC
 * @brief       SendType -> 发送的类型（寻求命令或是反馈命令）
                Id       -> BASE_ID + DOOR_ID
 * @param       无
 * @return      无
 **************************************************************************************************
 */

void Tcp_SendtoPC(uint8 CMD, uint8* Id, uint8 RESP, uint8 *data, uint8 DataLength)
{
    /*------------------------------------------------------------------------------------------------------------*/
    /* Command    0       1        2       3       4         5       6           7            8     9             */
    /*------------------------------------------------------------------------------------------------------------*/
    /* Name      SOP       CMD   LENG    SEQ    BASE_ID   DOOR_ID  RESP  Data[0]_Data[n-1]  LRC    EOP           */
    /*------------------------------------------------------------------------------------------------------------*/
    /* Values    0xAA                                                                               0x0E          */
    /*------------------------------------------------------------------------------------------------------------*/
    /* No.Byte   2byte   1byte    1byte   1byte   2byte    2byte    1byte     n-byte        1-byte  1-byte        */
    /*------------------------------------------------------------------------------------------------------------*/
    /*1.寻求命令 FPB_CMD:0x00 RESP:0xEE*/
    /*2.反馈命令 FPB_CMD:命令字  RESP:反馈状态*/
  
    uint8 i;
    uint8 LRC;
    uint8 TxCount = 0;
    uint8 frame[Tx_BUFF] = {0x00};
    
    /*SOP*/
    frame[TxCount++] = SOP;
    frame[TxCount++] = SOP;
    
    /*CMD*/
    frame[TxCount++] = CMD;
    
    /*LENG*/
    frame[TxCount++] = DataLength + 6;
    
    /*SEQ*/
    frame[TxCount++] = SEq;   //目前固定0x01
    
    /*ID*/
    for(i=0; i<4; i++)        //BASE_ID + DOOR_ID
    {
      frame[TxCount++] = Id[i];
    }
    
    /*RESP*/
    frame[TxCount++] = RESP;
    
    /*Data[0]_Data[n-1]*/
    if(DataLength != 0x00)
    {
      for(i=0; i<DataLength;i++)
      {
        frame[TxCount++] = data[i];
      }  
    }
    
    LRC = frame[2];
    
    /*LRC*/
    for(i=3;i<TxCount; i++)
    {
      LRC = LRC ^ frame[i];
    }
    frame[TxCount++] = LRC;
    
    /*EOP*/
    frame[TxCount++] = EOP;
    
    Uart0_SendString(frame,TxCount);
}
