#ifndef DEV_AT24C256_H_
#define DEV_AT24C256_H_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dri_i2c.h"
#include <string.h>
/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */
/******************I2C总线从机地址*********************/
#define AT24C256_WRITE_ADDR      0xA0   //写器件地址
#define AT24C256_READ_ADDR       0xA1   //读器件地址，其实就是写地址+1嘛



/******************存储区域划分*********************/

/*1.门锁ID*/
#define DOORID_BASE_ADDR             0x0000     //门锁ID信息
  #define DOORID_MAX_ADDR            0x0003

#define DOORID_RESERVED_ADDR         0x000F     //保留

/*2.普通卡列表*/
#define COMMONCARD_BASE_ADDR         0x0010     //普通卡最多100张（4byte/张）
  #define COMMONCARD_MAX_ADDR        0x019F     
  #define COMMONCARD_MAX1_ADDR       0x0023     //测试用，首先测试5张普通卡

#define COMMONCARD_RESERVED_ADDR     0x01FF     //保留








/*最大存储地址*/
#define EEPROM_MAX_ADDR               0x7FFF



//#define ORGCARDID_BASE_ADDR           0x0000    //普通卡最多90张（4byte/张）
//  #define ORGCARDID_MAX_ADDR          0x0167
//
//
//#define AUTHCARDID_BASE_ADDR          0x0190    //授权卡最多3张（4byte/张），特权卡加入黑名单前需清空存储区域
//  #define AUTHCARDID_MAX_ADDR         0x019B
//
//#define UNAUTHCARDID_BASE_ADDR        0x019C    //删权卡最多3张（4byte/张），特权卡加入黑名单前需清空存储区域
//  #define UNAUTHCARDID_MAX_ADDR       0x01A7
//
//#define TOTALCARD_BASE_ADDR           0x01A8    //总卡最多10张（4byte/张），特权卡加入黑名单前需清空存储区域
//  #define TOTALCARD_MAX_ADDR          0x01CF
//
//#define FLOORCARD_BASE_ADDR           0x01D0    //楼层卡最多10张（4byte/张），特权卡加入黑名单前需清空存储区域
//  #define FLOORCARD_MAX_ADDR          0x01F7
//
//
//#define BLACKCARD_BASE_ADDR           0x0200    //黑名单最多200个，为丢失特权卡而用(不包括普通卡)（4byte/张）
//  #define BLACKCARD_MAX_ADDR          0x051F    
//
//#define CARD_RESERVED_ADDR            0x05FF    //卡类型区域保留
//
///*2.离线刷卡记录*/
//#define AUTHCARD_RECORD_BASE_ADDR     0x0600    //特权卡授权删权记录200条（16Byte/条 容量标志1Byte + 时间6byte +卡类型1byte + 特权卡4byte + 普通卡4byte,如果是清空普通卡操作，全是00）
//  #define AUTHCARD_RECORD_MAX_ADDR    0x127F
//
//#define AUTHCARD_P_BASE_ADDR          0x1300    //特权卡指针8个（2Byte/个 当前存储的地址1Byte + 存储位置1Byte）
//#define AUTHCARD_P_MAX_ADDR           0x130F
//  
//
//#define AUTHCARD_RECORD_RESERVED_ADDR 0x13FF
//
//#define ORGCARD_RECORD_BASE_ADDR      0x1400    //普通卡(包括总卡、楼层卡)刷卡记录20*30*2*12条，将近2个月（12Byte/条 容量标志1Byte + 时间6Byte+卡类型1Byte + 卡4Byte）
//  #define ORGCARD_RECORD_MAX_ADDR     0x4C3F    
//
//#define ORGCARD_P_BASE_ADDR           0x4D00    //普通卡指针256个（2Byte/个 当前存储的地址1Byte + 存储位置1Byte）
//#define ORGCARD_P_MAX_ADDR            0x4EFF
//
//
//#define RECORD_RESERVED_ADDR          0x4FFF    //记录区域保留







/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void AT24C256_DelayMs(uint16 n);
uint8 AT24C256_WriteByte(uint8 SlaveAddr,uint16 ByteAddr,uint8 Data);
uint8 AT24C256_ReadByte(uint8 SlaveAddr,uint16 ByteAddr,uint8 *Data);
uint8 AT24C256_WriteBuff(uint8 SlaveAddr, uint16 ByteAddr, uint8 *Data, uint8 Len);
uint8 AT24C256_ReadBuff(uint8 SlaveAddr, uint16 ByteAddr, uint8 *Data, uint8 Len);



#endif