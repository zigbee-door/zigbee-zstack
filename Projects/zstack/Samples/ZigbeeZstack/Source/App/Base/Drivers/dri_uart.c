#define DRI_UART_C

#include "dri_uart.h"


/************************************************
 *function :  Uart0_Init
 *describe :  Uart0初始化
 *input    :  None
 *return   :  None
 ***********************************************/
void Uart0_Init(void)
{
//   CLKCONCMD  &= ~0x40;         //Set system frequency as XOSC 32Mhz
//   while(!(SLEEPSTA & 0x40));   //Wait the 32Mhz OSC to stable 
//   //while(CLKCONSTA & 0x40);   //6bit 0:32Mhz 1:16Mhz 
//   CLKCONCMD  &= ~0x47;         //CLKSPD = 000 -> system speed 32Mhz TICKSPD =111 -> Timer out 250Khz = 32Mhz/128
//   SLEEPCMD   |=  0x04;         //Disable the RCOSC(16Mhz)
   
   PERCFG      =  0x00;         //Uart standby position 1 
   P0SEL      |=  0x0c;         //P0.2 P0.3 set as Peripheral IO
   P2DIR      &= ~0xc0;         //Uart0 first as uart
   
   U0CSR      |=  0x80;         //Bit7 Mode = 1 UART mode not SPI mode
   U0GCR      |=  10;           //BAUD_E = 10
   U0BAUD     |=  216;          //BAUD_M = 216 BAUD_RATE = 57600，后期可以换成115200
   
   UTX0IF      =  0;            //none interrupt none done, if 1, interrupt none done
   
   U0CSR      |=  0x40;         //Enable to receive RE = 1
   //IEN0       |=  0x84;         //EA = 1; URX0IE = 1;
   
   IEN0   |= 0x04;              //uart0中断使能
}



//重定义printf到串口
__near_func int putchar(int ch)  
{  
    Uart0_SendByte(ch);  
    return ch;  
}  





/************************************************
 *function :  Uart0_SendByte
 *describe :  Uart0 to Send One Byte
 *input    :  Data  ->  Send Data
 *return   :  None
 ***********************************************/
void Uart0_SendByte(uint8 Data)
{
  //Wdt_Feet();
  U0DBUF = Data;              //U0DBUF auto to Send Data, if send Data done, the UTX0IF = 1 (UTX0IF is a interrupt flag)
  while(UTX0IF == 0);         //wait to send data done
  UTX0IF = 0;
}

/************************************************
 *function :  Uart0_SendString
 *describe :  Uart0 to Send String
 *input    :  *Data -> Send Buff
              len   -> Buff length
 *return   :  None
 ***********************************************/
void Uart0_SendString(uint8 *Data, uint8 len)
{
  uint8 j;
  for(j=0; j<len; j++)
  {
    U0DBUF = *Data ++;
    while(UTX0IF == 0);    
    UTX0IF = 0;
  }
}