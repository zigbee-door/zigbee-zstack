#ifndef DEV_BUZZER_H_
#define DEV_BUZZER_H_


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dri_delay.h"
#include "dri_buzzer.h"
#include "hal_led.h"  

/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */
/*音阶和频率*/
#define     GamutC         0x3c         
#define     GamutD         0x35
#define     GamutDp        0x32
#define     GamutE         0x30
#define     GamutF         0x2d
#define     GamutG         0x28
#define     GamutA         0x23
#define     GamutB         0x20

/*LED*/
#define LED_ON() HAL_TURN_ON_LED1()
#define LED_OFF() HAL_TURN_OFF_LED1()

/*LED灯亮灭标志*/
#define LedOn   1
#define LedOff  2


/* ------------------------------------------------------------------------------------------------
 *                                          Prototype
 * ------------------------------------------------------------------------------------------------
 */
void Dev_Buzzer_System_Start(void);

void Dev_Buzzer_One_Buzzer(void);
void Dev_Buzzer_OneBuzzer_Led(uint8 Led);
void Dev_Buzzer_Two_Buzzer(void);
void Dev_Buzzer_Three_Buzzer(void);

void Dev_Buzzer_Door_Open(void);
void Dev_Buzzer_Door_Close(void);
void Dev_Buzzer_Door_Lock(void);
void Dev_Buzzer_Key_Open(void);
void Dev_Buzzer_Key_Close(void);

void Dev_Buzzer_Card_Full(void);
void Dev_Buzzer_Card_Success(void);
void Dev_Buzzer_Card_Fail(void);
void Dev_Buzzer_Door_Init(void);


#endif


