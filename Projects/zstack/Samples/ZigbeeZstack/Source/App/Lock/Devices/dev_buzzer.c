#define DEV_BUZZER_C_


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dev_buzzer.h"



/* ------------------------------------------------------------------------------------------------
 *                                          系统提示声
 * ------------------------------------------------------------------------------------------------
 */
/****************************************
 * @fn          Buzzer_System_Start
 * @brief       系统启动提示声
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_System_Start(void)
{
  LED_ON();
  BUZZER_ON();
  T4CC0   = GamutF;     
  Delay_Ms(100);
  LED_OFF();
  BUZZER_OFF();
}

/* ------------------------------------------------------------------------------------------------
 *                                          普通提示声
 * ------------------------------------------------------------------------------------------------
 */
/**************************************
 * @fn          Buzzer_One
 * @brief       普通提示声一声
 * @param       无
 * @return      无
 **************************************
 */
void Buzzer_One(void)
{
  LED_ON();
  BUZZER_ON();
  T4CC0   = GamutB;     
  Delay_Ms(100);
  LED_OFF();
  BUZZER_OFF();
  Delay_Ms(100);
}

/***************************************
 * @fn          Buzzer_One_Led
 * @brief       普通提示声一声
 * @param       无
 * @return      无
 ***************************************
 */
void Buzzer_One_Led(uint8 Led)
{
  if(Led == LedOn)
  {
    LED_ON();
  }
  
  else
  {
    LED_OFF();
  }
  
  BUZZER_ON();
  T4CC0   = GamutB;     
  Delay_Ms(100);
  BUZZER_OFF();
  Delay_Ms(100);
  
}

/***************************************
 * @fn          Buzzer_Two
 * @brief       普通提示声两声
 * @param       无
 * @return      无
 ***************************************
 */
void Buzzer_Two(void)
{
  Buzzer_One();
  Buzzer_One();
}

/***************************************
 * @fn          Buzzer_Three
 * @brief       普通提示声三声
 * @param       无
 * @return      无
 ***************************************
 */
void Buzzer_Three(void)
{
  Buzzer_One();
  Buzzer_One();
  Buzzer_One();
}

/* ------------------------------------------------------------------------------------------------
 *                                          开关门提示声
 * ------------------------------------------------------------------------------------------------
 */
/****************************************
 * @fn          Buzzer_Door_Open
 * @brief       开门提示声
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Door_Open(void)
{
  
  T4CC0   = GamutC; 
  BUZZER_ON();       
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
       
  T4CC0   = GamutDp;    
  BUZZER_ON();   
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
      
  T4CC0   = GamutG;    
  BUZZER_ON();   
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
}


/****************************************
 * @fn          Buzzer_Door_Close
 * @brief       关门提示声
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Door_Close(void)
{       
  T4CC0   = GamutG;   
  BUZZER_ON();
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
  
      
  T4CC0   = GamutDp;        
  BUZZER_ON();  
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
  
  
  T4CC0   = GamutC; 
  BUZZER_ON();
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
}


/****************************************
 * @fn          Buzzer_Door_Lock
 * @brief       门反锁LED提示
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Door_Lock(void)
{
  LED_ON();
  Delay_Ms(100);
  LED_OFF();
  Delay_Ms(100);
  LED_ON();
  Delay_Ms(100);
  LED_OFF();
  Delay_Ms(100);
  LED_ON();
  Delay_Ms(100);
  LED_OFF();
  Delay_Ms(100);
}

/****************************************
 * @fn          Buzzer_Key_Open
 * @brief       钥匙开门提示
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Key_Open(void)
{
  LED_ON();
  T4CC0   = GamutC; 
  BUZZER_ON();       
  Delay_Ms(100);
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
       
  T4CC0   = GamutB;    
  BUZZER_ON();   
  Delay_Ms(100);
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
  LED_OFF();
  
  
}


/* ------------------------------------------------------------------------------------------------
 *                                          授权状态提示声
 * ------------------------------------------------------------------------------------------------
 */
/****************************************
 * @fn          Buzzer_Card_Full
 * @brief       普通卡列表已满
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Card_Full(void)
{
  LED_OFF();
  T4CC0   = GamutB;   
  BUZZER_ON();
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
  
      
  T4CC0   = GamutC;        
  BUZZER_ON();  
  Delay_Ms(100);
  Delay_Ms(100);
  BUZZER_OFF();
  LED_ON();
}

/****************************************
 * @fn          Buzzer_Card_Success
 * @brief       普通卡列表授权删权成功
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Card_Success(void)
{
  LED_OFF();
  BUZZER_ON();
  T4CC0   = GamutC;     
  Delay_Ms(100);
  BUZZER_OFF();
  Delay_Ms(100);
  BUZZER_ON();
  T4CC0   = GamutB;     
  Delay_Ms(100);
  BUZZER_OFF();
  LED_ON();
}

/****************************************
 * @fn          Buzzer_Card_Fail
 * @brief       普通卡列表授权删权失败
 * @param       无
 * @return      无
 ****************************************
 */
void Buzzer_Card_Fail(void)
{
  Buzzer_One();
  Buzzer_One();
}



/**************************************************************************************************
 * @fn          Buzzer_Door_Init
 * @brief       门锁初始化提示
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void Buzzer_Door_Init(void)
{
  T4CC0   = GamutB;   
  BUZZER_ON();
  Delay_Ms(100);
  BUZZER_OFF();
  
      
  T4CC0   = GamutG;        
  BUZZER_ON();  
  Delay_Ms(100);
  BUZZER_OFF();
  
  
  T4CC0   = GamutE; 
  BUZZER_ON();
  Delay_Ms(100);
  BUZZER_OFF();
  
  
  T4CC0   = GamutA;   
  BUZZER_ON();
  Delay_Ms(100);
  BUZZER_OFF();
  
      
  T4CC0   = GamutF;        
  BUZZER_ON();  
  Delay_Ms(100);
  BUZZER_OFF();
  
  
  T4CC0   = GamutDp; 
  BUZZER_ON();
  Delay_Ms(100);
  BUZZER_OFF();
}