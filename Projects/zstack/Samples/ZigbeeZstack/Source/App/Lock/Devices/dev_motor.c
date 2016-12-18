#define DOOR_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dev_motor.h"

/**************************************************************************************************
 * @fn          Door_Open
 * @brief       开门(常开)
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Door_Open(uint8_t Led)
{
  Warn_OneBuzzer_Led(Led);                             
  Motor_H1ON();                                                                 //防止锁难开先松动一下
  Motor_H1ON();
  Delay_Ms(10);       
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(10);
  Motor_H1ON();
  Motor_H1ON();
  Delay_Ms(10);       
  Delay_Ms(10);
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(10);
  Delay_Ms(10);
  Motor_H1ON();
  Motor_H1ON();
  Delay_Ms(10);       
  Delay_Ms(10);
  Delay_Ms(10);       
  Delay_Ms(10);
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(10);
  Delay_Ms(10);
  Delay_Ms(10);       
  Delay_Ms(10);
  Motor_H1H2OFF();
  Delay_Ms(100);      
  Motor_H1ON();
  Motor_H1ON();
  Delay_Ms(100);      
  Delay_Ms(100);
  Motor_H1H2OFF();
  Motor_H1H2OFF();
  Warn_Door_Open();   
}

/**************************************************************************************************
 * @fn          Door_Close
 * @brief       关门
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Door_Close(uint8_t Led)
{
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(10);   
  Motor_H1ON();       
  Motor_H1ON();
  Delay_Ms(10);
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(10);  
  Delay_Ms(10);
  Motor_H1ON();       
  Motor_H1ON();
  Delay_Ms(10);
  Delay_Ms(10);
  Motor_H1ON();
  Motor_H1ON();
  Delay_Ms(10);
  Delay_Ms(10);
  Delay_Ms(10);       
  Delay_Ms(10);
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(10);
  Delay_Ms(10);
  Delay_Ms(10);       
  Delay_Ms(10);
  Motor_H1H2OFF();
  Delay_Ms(100);
  Motor_H2ON();
  Motor_H2ON();
  Delay_Ms(100);  
  Delay_Ms(100);
  Motor_H1H2OFF();
  Motor_H1H2OFF();
  Warn_Door_Close(); 
  
  if(Led == LedOff)
  {
    LED_OFF();
  }
  
  else
  {
    LED_ON();
  }
  
}




/**************************************************************************************************
 * @fn          Door_OpenClose
 * @brief       开门并关门
 * @param       无
 * @return      无
 **************************************************************************************************
 */

void Door_Open_Close(void)
{  
  uint8_t count = 0;
  Door_Open(LedOn);
  
  Door_Int_Enable();            //P0.4中断使能
  IsDoorClose = CLOSE_NO;       //允许中断检测门状态 
  
  Timer_Int_Enable();           //允许定时中断
  T1_3S_Flag = TIMESTART;
  
  do
  {
    if(IsDoorClose == CLOSE_YES)
    {
      Delay_Ms(10);             //消除抖动
      if(DOOR == PUSHOUT)       //变成高电平的状态才允许再次中断
      {
        IsDoorClose = CLOSE_NO;
        count ++;
      }
    }
    
    if(count >= 1)              //可能立马电机关门也可能等门关上电机才关门
    {
      count = 0;
      T1_3S_Flag = TIMEEND;     //立即关门，否则3S关门       
      Delay_Ms(10);             //为了让T1_3S_Count清零而给出的延时
    }
    
    
    //Wdt_FeetDog();
    
  }while(T1_3S_Flag == TIMESTART);
  
  T1_3S_Flag = TIMEINIT;        //注意如果没有超时跳出，那么要记得清楚计数值count
  Timer_Int_Disable();          //关闭定时中断
  
  Door_Close(LedOn);
  
  KeyDoor_Int_Disable();        //设置P0.1、P0.4中断禁止
  IsDoorClose = CLOSE_INIT;     //禁止中断检测门状态，防止里面开门时按下把手跳进中断
  
}