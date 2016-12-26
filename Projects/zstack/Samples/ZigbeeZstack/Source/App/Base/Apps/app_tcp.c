#define APP_TCP_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "app_tcp.h"
#include "hal_mcu.h"      //这里要调用中断的宏定义

/* ------------------------------------------------------------------------------------------------
 *                                          初始化函数
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          Tcp_Init
 * @brief       数据帧参数的初始化
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Tcp_Init(void)
{
  Frame.RxData     =  0x00;
  Frame.RxStatus   =  FRAME_RX_START;  
  Frame.RxProgress =  FRAME_RX_SOP_START;
  Frame.RxCount   =  0x00;
  
  Frame.CMD        =  BASE_CMD_INIT;  //空命令
  Frame.LENG       =  0x00;
  memset(Frame.DOOR_ID,0x00,2);
  Frame.RESP       =  0x00;
  memset(Frame.DATA,0x00,Rx_BUFF);
  Frame.LRC        =  0x00;
 
}








/* ------------------------------------------------------------------------------------------------
 *                                          中断函数
 * ------------------------------------------------------------------------------------------------
 */


HAL_ISR_FUNCTION( UART0_ISR, URX0_VECTOR )
{
  URX0IF    = 0;  
  Frame.RxData  = U0DBUF;    
  Tcp_Receive(Frame.RxData);
}


/* ------------------------------------------------------------------------------------------------
 *                                          应用函数
 * ------------------------------------------------------------------------------------------------
 */




/**************************************************************************************************
 * @fn          Tcp_Receive
 * @brief       接受服务器发送的TCP数据字节
 * @param       Rx_Data -> 串口接收的数据
 * @return      无
 **************************************************************************************************
 */
void Tcp_Receive(uint8 Rx_Data)
{
  if(Frame.RxStatus == FRAME_RX_START)     //这个条件暂时不去掉，接收数据的实时性可能会受影响
  {
    Tcp_Process(Rx_Data);
  }
}






/**************************************************************************************************
 * @fn          Tcp_Process
 * @brief       接受数据帧解析
 * @param       Rx_Data -> 串口接收的数据
 * @return      无
 **************************************************************************************************
 */
void Tcp_Process(uint8 Rx_Data)
{
  /*-----------------------------------------------------------------------------*/
  /* Command    0      2     1        3             4            5      6        */
  /*-----------------------------------------------------------------------------*/
  /* Name      SOP    CMD   LENG   DOOR_ID   Data[0]_Data[n-1]  LRC    EOP       */
  /*-----------------------------------------------------------------------------*/
  /* Values    0xAA                                                    0x0E      */
  /*-----------------------------------------------------------------------------*/
  /* No.Byte   2byte  1byte 1byte   2byte         nbyte        1byte   1byte     */
  /*-----------------------------------------------------------------------------*/
  switch(Frame.RxProgress)
  {
    /*1.接收到帧头SOP -> 0xAA(1)*/
    case FRAME_RX_SOP_START:          if(Rx_Data != SOP)
                                      {
                                        break;
                                      }
                                      
                                      Frame.RxProgress = FRAME_RX_SOP_CHECK;
                                      break;
                                
                                
    /*2.确认接收到的帧头SOP -> 0xAA(2)*/ 
    case FRAME_RX_SOP_CHECK:          if(Rx_Data == SOP)
                                      {
                                        Frame.LRC        = 0x00;
                                        Frame.RxProgress = FRAME_RX_LENG;
                                      }
                                      else
                                      {
                                        Frame.RxProgress = FRAME_RX_SOP_START;    //如果错误，重新开始接收          
                                      }
                                      break;   
                                      
    /*3.接收到数据长度 -> LENG*/                               
    case  FRAME_RX_LENG:              if(Rx_Data >= MIN_LENG)                     //如果大于等于最小数据帧长度
                                      {
                                        Frame.LENG = Rx_Data;                     //LRC校验开始
                                        Frame.LRC = Frame.LRC ^ Rx_Data;
                                        Frame.RxProgress = FRAME_RX_CMD;
                                        break;                                    //如果接收正确，跳出，等待下一个数据
                                      } 
                                      else 
                                      {
                                        Frame.RESP = BASE_RESP_LENG_ERR;          //长度缺失
                                        Frame.RxStatus = FRAME_RX_ERR;            //一帧数据错误,如果不初始化，不会再对接收数据进行处理  
                                        Frame.RxProgress = FRAME_RX_END;          //结束这帧数据的接收
                                      }
      
   /*4.接收到命令字 -> CMD*/   
   case  FRAME_RX_CMD:                if(Frame.RxProgress != FRAME_RX_END)        //如果上一个接收错误,这个不处理
                                      {
                                        if(Rx_Data <= MAX_CMD)                    //如果是命令字
                                        {
                                          Frame.CMD = Rx_Data;
                                          Frame.LRC = Frame.LRC ^ Rx_Data;
                                          Frame.RxProgress = FRAME_RX_DOOR_ID;   
                                          Frame.RxCount = 0;
                                          break;
                                        }
                                        else
                                        {
                                          Frame.CMD =  BASE_CMD_INIT;;
                                          Frame.RESP = BASE_RESP_CMD_ERR;           //长度缺失
                                          Frame.RxStatus = FRAME_RX_ERR;            //一帧数据错误,如果不初始化，不会再对接收数据进行处理  
                                          Frame.RxProgress = FRAME_RX_END;          //结束这帧数据的接收
                                        }
                                      }
                                  
                                      
   /*5.接收到门锁ID -> DOOR_ID*/   
   case FRAME_RX_DOOR_ID:             if(Frame.RxProgress != FRAME_RX_END)          //如果上一个接收错误,这个不处理
                                      {
                                        Frame.DOOR_ID[Frame.RxCount++] = Rx_Data;
                                        Frame.LRC = Frame.LRC ^ Rx_Data;
                                        if(Frame.RxCount == 2)
                                        {
                                          Frame.RxCount = 0;
                                          Frame.RxProgress = FRAME_RX_DATA;
                                        } 
                                        
                                        break;
                                      }
                                      
   /*6.接收数据 -> Data[0]_Data[n-1]*/
   case FRAME_RX_DATA:                if(Frame.RxProgress != FRAME_RX_END)
                                      {
                                        if(Frame.LENG == MIN_LENG)          //没有数据，就直接进入LRC，而不是跳出
                                        {
                                          Frame.RxProgress = FRAME_RX_LRC;
                                          //break;
                                        }
                                        
                                        else                             //有数据，暂时不处理
                                        {
                                          //Frame.LENG - MIN_LENG 这个有关系
                                          
                                          break;
                                        }
                                        
                                      }
                                      
   /*7.接收校验 -> LRC*/              
   case  FRAME_RX_LRC:                if(Frame.RxProgress != FRAME_RX_END)
                                      {
                                        if(Frame.LRC == Rx_Data)
                                        {
                                          Frame.RxProgress = FRAME_RX_EOP;
                                          break;
                                        }       
                                        else
                                        {
                                          Frame.RESP = BASE_RESP_LRC_ERR;
                                          Frame.RxStatus = FRAME_RX_ERR;
                                          Frame.RxProgress = FRAME_RX_END;        //强行结束接收数据
                                        }
                                       
                                      }
                            
   /*7.接收帧尾 -> EOP*/              
   case  FRAME_RX_EOP:                if(Frame.RxProgress != FRAME_RX_END)
                                      {
                                        if(Rx_Data == EOP)
                                        {
                                          Frame.RxStatus = FRAME_RX_OK;           //一帧数据正确 
                                          osal_set_event(0x07,0x04);              //(BaseApp_TaskID,BASEAPP_TCP_RECEIVE_TRUE_MSG_EVENT)
                                          break;
                                        }
                                        else
                                        {
                                          Frame.RESP = BASE_RESP_EOP_ERR;
                                          Frame.RxStatus = FRAME_RX_ERR;
                                          Frame.RxProgress = FRAME_RX_END;        //强行结束接收数据
                                        }     
                                       
                                      }                                   
                                      
                                      

   default:                           /* 接收到错误的数据帧 */
                                      if((Frame.RxStatus == FRAME_RX_ERR) && (Frame.RxProgress == FRAME_RX_END))
                                      {
                                        Tcp_Send(Frame.CMD,Frame.DOOR_ID,Frame.RESP,Frame.DATA,0); //反馈错误数据帧情况
                                        Tcp_Init();
                                        //osal_set_event(0x07,0x02);  //(BaseApp_TaskID,BASEAPP_TCP_RECEIVE_ERR_MSG_EVENT)
                                      }
                                      break;  
    
  }
}  
  
  
  


/**************************************************************************************************
 * @fn          Tcp_Send
 * @brief       发送反馈数据给上位机
 * @param       FPB_CMD -> 命令类型
                DoorId  -> 发送命令的目的门锁
                RESP    -> 反馈类型
                data    -> 数据
                DataLength -> 数据长度
 * @return      无
 **************************************************************************************************
 */
void Tcp_Send(uint8 FPB_CMD, uint8* DoorId, uint8 RESP, uint8 *data, uint8 DataLength)
{
    /*---------------------------------------------------------------------------------------------*/
    /* Command    0       1        2         3       4         5               6      7            */
    /*---------------------------------------------------------------------------------------------*/
    /* Name      SOP     LENG    FPB_CMD    DOOR_ID  RESP  Data[0]_Data[n-1]  LRC    EOP           */
    /*---------------------------------------------------------------------------------------------*/
    /* Values    0xAA                                                                0x0E          */
    /*---------------------------------------------------------------------------------------------*/
    /* No.Byte   2byte   1byte   1byte       2byte   1byte    nbyte    byte   1byte  1-byte        */
    /*---------------------------------------------------------------------------------------------*/
  
    uint8 i;
    uint8 LRC;
    uint8 TxCount = 0;
    uint8 frame[Tx_BUFF] = {0x00};
    
    /*SOP*/
    frame[TxCount++] = SOP;
    frame[TxCount++] = SOP;
    
    /*LENG*/
    frame[TxCount++] = DataLength + FPB_MIN_LENG;   //FPB_CMD ~ Data[n-1]
    
    
    /*FPB_CMD*/
    frame[TxCount++] = FPB_CMD;
   
    
    /*DOOR_ID*/
    for(i=0; i<2; i++)        //DOOR_ID
    {
      frame[TxCount++] = DoorId[i];
    }
    
    /*RESP*/
    frame[TxCount++] = RESP;
    
    /*Data[0]_Data[n-1]*/ 
    for(i=0; i<DataLength;i++)
    {
      frame[TxCount++] = data[i];
    }  
    
    
    /*LRC*/
    LRC = frame[2];     //LENG ~ Data[n-1]
    
    
    for(i=3;i<TxCount; i++)
    {
      LRC = LRC ^ frame[i];
    }
    
    frame[TxCount++] = LRC;
    
    /*EOP*/
    frame[TxCount++] = EOP;
    
    Uart0_SendString(frame,TxCount);
    
}



