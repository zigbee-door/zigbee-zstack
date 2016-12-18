#ifndef DEV_MFRC522_H_
#define DEV_MFRC522_H_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dri_spi.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */

/*和MF522通讯时返回的状态标志*/
#define MFRC522_OK                 0		 //正确状态
#define MFRC522_ERR                1		 //其他错误
#define MFRC522_TIMEOUT            2             //寻卡超时


/*FIFO缓冲区最大的字节数*/
#define FIFOLEVEL_MAX              16




/*MFRC522引脚设定*/
#define MFRC522_RST     P1_7  //复位信号

/*MFRC522寄存器*/
//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg	          0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34	          0x3F

/*MFRC522命令字和错误代码*/   
//MF522命令字 PCD - MFRC522 
#define PCD_IDLE                  0x00               //NO action;取消当前命令
#define PCD_AUTHENT               0x0E               //验证密钥
#define PCD_RECEIVE               0x08               //接收数据
#define PCD_TRANSMIT              0x04               //发送数据
#define PCD_TRANSCEIVE            0x0C               //发送并接收数据
#define PCD_RESETPHASE            0x0F               //复位
#define PCD_CALCCRC               0x03               //CRC计算

//Mifare_One卡片命令字 PICC - M1CARD
#define PICC_REQIDL               0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL               0x52               //寻天线区内全部卡
#define PICC_ANTICOLL             0x93               //防冲撞
#define PICC_SElECTTAG            0x93               //选卡
#define PICC_AUTHENT1A            0x60               //验证A密钥
#define PICC_AUTHENT1B            0x61               //验证B密钥
#define PICC_READ                 0x30               //读块
#define PICC_WRITE                0xA0               //写块
#define PICC_DECREMENT            0xC0               //扣款
#define PICC_INCREMENT            0xC1               //充值
#define PICC_RESTORE              0xC2               //调块数据到缓冲区
#define PICC_TRANSFER             0xB0               //保存缓冲区中数据
#define PICC_HALT                 0x50               //休眠




/* ------------------------------------------------------------------------------------------------
 *                                          Applications
 * ------------------------------------------------------------------------------------------------
 */
void MFRC522_Init(void);
void MFRC522_Reset(void);
void MFRC522_AntennaOff(void);
void MFRC522_AntennaOn(void);
void MFRC522_ISO14443A(void);
void MFRC522_CrcCheck(uint8 *CrcData, uint8 CrcLen, uint8 *CheckData);

uint8 MFRC522_RequestCard(uint8 ReqMode,uint8 *CardType);
uint8 MFRC522_ToCard(uint8 Command, uint8 *SendData, uint8 SendLen, uint8 *ReadData, uint32 *ReveiveBits);
uint8 MFRC522_Anticoll(uint8 *CardData);
uint8 MFRC522_SelectTag(uint8 *CardData);
uint8 MFRC522_Auth(uint8 AuthMode, uint8 BlockAddr, uint8 *SectorKey, uint8 *CardData);
uint8 MFRC522_WriteToBlock(uint8 BlockAddr, uint8 *WriteData);
uint8 MFRC522_ReadFromBlock(uint8 BlockAddr, uint8 *ReadData);
uint8 MFRC522_Sleep(void);

/* ------------------------------------------------------------------------------------------------
 *                                          Middle
 * ------------------------------------------------------------------------------------------------
 */
void MFRC522_ClearBitMask(uint8 RegAddr,uint8 Mask);
void MFRC522_SetBitMask(uint8 RegAddr,uint8 Mask);
/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void MFRC522_Delay(uint8 n);
void MFRC522_Write(uint8 RegAddr,uint8 Data);
uint8 MFRC522_Read(uint8 RegAddr);




#endif