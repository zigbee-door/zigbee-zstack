#define APPH_DOOR_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "apph_door.h"

/**************************************************************************************************
 * @fn          Door_Open
 * @brief       开门(常开)
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Door_Open(uint8 Led)
{
  Buzzer_One_Led(Led);                             
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
  Buzzer_Door_Open();   
}

/**************************************************************************************************
 * @fn          Door_Close
 * @brief       关门
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Door_Close(uint8 Led)
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
  Buzzer_Door_Close(); 
  
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
  uint16 i = 60;                 //最大允许延时3s
  DoorStatus = PUSHOUT;           //锁扣默认在外面，没有压入
  
  Door_Open(LedOn);
  
  //默认延时3s关门
  while(i--) 
  {
    Delay_Ms(50);
    
    if((DOOR == PUSHOUT) && (DoorStatus == PUSHIN))   //锁扣已经向内压过了，此时如果锁扣又弹出，
                                                      //模拟了一次开门的动作，门开的时候如果放手就算一次开门
    {
      break;
    }
  }
  
  DoorStatus = PUSHOUT;
  Door_Close(LedOff);
}