#ifndef DRI_UART_H
#define DRI_UART_H



/*-Include----------------*/
#include<ioCC2530.h>
#include "stdio.h"
#include "hal_uart.h"



/*-function---------------*/
void Uart0_Init(void);
void Uart0_SendByte(uint8 Data);
void Uart0_SendString(uint8 *Data, uint8 len);








#endif