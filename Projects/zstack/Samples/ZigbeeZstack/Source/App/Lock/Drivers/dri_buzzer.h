#ifndef DRI_BUZZER_H_
#define DRI_BUZZER_H_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include<ioCC2530.h>


/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */

/*蜂鸣器端口宏定义*/
#define DRI_BUZZER          P2_0
#define DRI_BUZZER_IN()     P2DIR &= ~0x01
#define DRI_BUZZER_OUT()    P2DIR |=  0x01
#define DRI_BUZZER_IO()     P2SEL &= ~0x01              //普通IO口
#define DRI_BUZZER_T4()     P2SEL |=  0x01              //T4用作PWM输出    
#define DRI_BUZZER_T4ON()   T4CTL |=  0x10;             //启动定时器
#define DRI_BUZZER_T4OFF()  T4CTL &= ~0x10;             //关闭定时器
#define DRI_BUZZER_OFF()    DRI_BUZZER_IN();DRI_BUZZER_IO();DRI_BUZZER_T4OFF()
#define DRI_BUZZER_ON()     DRI_BUZZER_OUT();DRI_BUZZER_T4();DRI_BUZZER_T4ON()


 

/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void Dri_Buzzer_Init(void);
void Dri_Buzzer_Timer4_Init(void);

/**************************************************************************************************
 */
#endif