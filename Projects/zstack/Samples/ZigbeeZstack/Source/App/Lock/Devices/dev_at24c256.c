#define DEV_AT24C256_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dev_at24c256.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          AT24C256_DelayMs
 * @brief       AT24C256延时函数
 * @param       n -> 延时毫秒数
 * @return      无
 **************************************************************************************************
 */
void AT24C256_DelayMs(uint16 n)     //需要注意这个延时函数和之前的延时函数不同
{
  uint16 i,j;
  for(i=0;i<n;i++)
  for(j=0;j<200;j++);
  for(j=0;j<200;j++);
  for(j=0;j<187;j++);
}

//之前的延时函数
//void AT24C256_DelayMs(uint16_t n)
//{
//  uint16_t i,j;
//  for(i=0;i<n;i++)
//  for(j=0;j<587;j++);
//}


/**************************************************************************************************
 * @fn          AT24C256_WriteByte
 * @brief       EEPROM写一字节数据
 * @param       SlaveAddr -> 从机地址
                ByteAddr  -> 数据地址(16bit需要拆分)（MAX 0X4000）
                Data      -> 待写入的数据
 * @return      AckFlag   -> 0 无应答 1 有应答 
 **************************************************************************************************
 */
uint8 AT24C256_WriteByte(uint8 SlaveAddr,uint16 ByteAddr,uint8 Data)
{
  uint8 AckFlag;
  
  I2C_Start();                                  //启动总线
  AckFlag = I2C_SendByte(SlaveAddr);            //发送从机地址（器件地址）
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte((ByteAddr>>8)&0xFF);   //发送地址高八位      
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte(ByteAddr&0xFF);        //发送地址低八位
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte(Data);                 //发送要存储的数据
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  I2C_Stop();                                   //结束总线
  AT24C256_DelayMs(12);                         //需要个小延时
  return 1;
}

/**************************************************************************************************
 * @fn          AT24C256_WriteBuff
 * @brief       EEPROM写多字节数据
 * @param       SlaveAddr -> 从机地址
                ByteAddr  -> 数据地址(16bit需要拆分)（MAX 0X4000）
                Data      -> 待写入的数据
                Len       -> 写入的数据数            (MAX 64)
 * @return      AckFlag   -> 0 无应答 1 有应答 
 **************************************************************************************************
 */
uint8 AT24C256_WriteBuff(uint8 SlaveAddr, uint16 ByteAddr, uint8 *Data, uint8 Len)
{
  uint8 AckFlag;
  uint8 i;
  
  I2C_Start();                                  //启动总线
  AckFlag = I2C_SendByte(SlaveAddr);            //发送从机地址（器件地址）
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte((ByteAddr>>8)&0xFF);   //发送地址高八位      
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte(ByteAddr&0xFF);        //发送地址低八位
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  for(i=0; i<Len; i++)
  {
    AckFlag = I2C_SendByte(Data[i]);            //发送要存储的数据
    if(AckFlag == 1)                            //若没有应答
    {
      return 0;
    }
  }
  
  I2C_Stop();                                   //结束总线
  AT24C256_DelayMs(12);                         //需要个小延时
  return 1;
}

/**************************************************************************************************
 * @fn          AT24C256_ReadByte
 * @brief       EEPROM读一字节数据
 * @param       SlaveAddr -> 从机地址
                ByteAddr  -> 数据地址(16bit需要拆分)（MAX 0X4000）
                Data      -> 待读取的数据
 * @return      AckFlag   -> 0 无应答 1 有应答 
 **************************************************************************************************
 */
uint8 AT24C256_ReadByte(uint8 SlaveAddr,uint16 ByteAddr,uint8 *Data)
{
  uint8 AckFlag;
  
  I2C_Start();                                  //启动总线
  AckFlag = I2C_SendByte(SlaveAddr);            //发送从机地址（器件地址）
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte((ByteAddr>>8)&0xFF);   //发送地址高八位      
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte(ByteAddr&0xFF);        //发送地址低八位
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  I2C_Start();     
  AckFlag = I2C_SendByte(SlaveAddr + 1);        //表明是读数据，注意默认最低位是0，表明写数据，+1变成1
  if(AckFlag == 1)                        
  {
    return 0;
  }
  
  *Data = I2C_ReceiveByte();                    //读取数据
  I2C_SendAck(1);                               //最后一个字节 No Acknowledge 所以需要使SDA = 1 
                           
  I2C_Stop();                                   //结束总线
  return 1;
}



/**************************************************************************************************
 * @fn          AT24C256_ReadBuff
 * @brief       EEPROM读多字节数据
 * @param       SlaveAddr -> 从机地址
                ByteAddr  -> 数据地址(16bit需要拆分)（MAX 0X4000）
                Data      -> 读取的数据
                Len       -> 读取的数据数           
 * @return      AckFlag   -> 0 无应答 1 有应答 
 **************************************************************************************************
 */
uint8 AT24C256_ReadBuff(uint8 SlaveAddr, uint16 ByteAddr, uint8 *Data, uint8 Len)
{
  uint8 AckFlag;
  uint8 i;
  
  I2C_Start();                                  //启动总线
  AckFlag = I2C_SendByte(SlaveAddr);            //发送从机地址（器件地址）
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte((ByteAddr>>8)&0xFF);   //发送地址高八位      
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  AckFlag = I2C_SendByte(ByteAddr&0xFF);        //发送地址低八位
  if(AckFlag == 1)                              //若没有应答
  {
    return 0;
  }
  
  I2C_Start();     
  AckFlag = I2C_SendByte(SlaveAddr + 1);        //表明是读数据，注意默认最低位是0，表明写数据，+1变成1
  if(AckFlag == 1)                        
  {
    return 0;
  }
  
  for(i=0; i<Len-1; i++)
  {
    Data[i] = I2C_ReceiveByte();                //读取存储的数据
    I2C_SendAck(0);  
  }
  
  Data[Len-1] = I2C_ReceiveByte();
  I2C_SendAck(1);
 
  I2C_Stop();                                   //结束总线
  return 1;
}