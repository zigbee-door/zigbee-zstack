#define MFRC522_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "dev_mfrc522.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Applications
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          MFRC522_Init
 * @brief       MFRC522初始化
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void MFRC522_Init(void)
{
  //uint8 tmp;
  MFRC522_Reset();
  MFRC522_AntennaOff();	
  //MFRC522_AntennaOn();
  //tmp = MFRC522_Read(TxControlReg);测试
  MFRC522_ISO14443A();	
}

/**************************************************************************************************
 * @fn          MFRC522_Reset
 * @brief       MFRC522复位阶段设置
 * @param       n
 * @return      无
 **************************************************************************************************
 */

uint8 k;

void MFRC522_Reset(void)
{
  MFRC522_RST = 1;    
  MFRC522_Delay(1);
  MFRC522_RST = 0;    //低电平时切断内部电流吸收，关闭振荡器，断开输入管脚与外部电路的连接
  MFRC522_Delay(1);   
  MFRC522_RST = 1;    //上升沿来启动内部复位阶段
  MFRC522_Delay(1);
  
  MFRC522_Write(CommandReg, PCD_RESETPHASE);  //BIT4=PowerDown(0):启动唤醒过程 BIT3-BIT0(1111):复位MFRC522
  

  /*TPrescaler 12位预分频器 110100111110(3390)**/
  MFRC522_Write(TModeReg,0x8D);               //BIT7=TAuto(1):定时器启动  BIT3-BIT0=TPrescaler_Hi(1101):预分频器高4位
  MFRC522_Write(TPrescalerReg,0x3E);          //BIT7-BIT0(00111110)=TPrescaler_Lo:定时器预分频器低8位
  /*TreloadVal 16位定时器重装值 30*/
  MFRC522_Write(TReloadRegL,0x30);            //定时器频率f = 6.78Mhz/TPrescaler  T = 1/f = TPrescaler/6.78Mhz
  MFRC522_Write(TReloadRegH,0);               //t = TreloadVal * T =  15ms   如果要为24ms，TReloadRegL = 0x30
  
  MFRC522_Write(ModeReg, 0x3D);               //BIT1-BIT0=CRCPreset(01):6363
  MFRC522_Write(TxAutoReg, 0x40);	      //100%ASK
  
  k = MFRC522_Read(TxAutoReg);
  MFRC522_Write(TxAutoReg, 0x40);
  
}


/**************************************************************************************************
 * @fn          MFRC522_AntennaOff
 * @brief       关闭天线
 * @param       n
 * @return      无
 **************************************************************************************************
 */
void MFRC522_AntennaOff(void)
{
  MFRC522_ClearBitMask(TxControlReg, 0x03); //TX2 TX1清0,，关闭了天线
}


/**************************************************************************************************
 * @fn          MFRC522_AntennaOn
 * @brief       开启天线
 * @param       n
 * @return      无
 **************************************************************************************************
 */


void MFRC522_AntennaOn(void)
{
  uint8 tmp;
  tmp = MFRC522_Read(TxControlReg);
  if(!(tmp & 0x03))
  {
    MFRC522_SetBitMask(TxControlReg,0x03);
  }
}

/**************************************************************************************************
 * @fn          MFRC522_ConfigISOType
 * @brief       设置RC632的工作方式,ISO14443A协议标准
 * @param       无
 * @return      无
 **************************************************************************************************
 */
void MFRC522_ISO14443A(void)
{
   MFRC522_SetBitMask(Status2Reg, 0x08); //该位用来指示MIFARE Cypto1单元接通和因此所有卡的数据通信被加密的情况，MFCrypto1On=0
   MFRC522_Write(RxSelReg , 0x86);       //84 非接触式UART的输入选择内部模拟部分的调制信号
   MFRC522_Write(RFCfgReg , 0x7F);       //4F 接收器信号电压的增益因子48dB
   MFRC522_Delay(1);
   MFRC522_AntennaOn();
}


/**************************************************************************************************
 * @fn          MFRC522_CrcCheck
 * @brief       用MF522计算CRC校验
 * @param       CrcData   -> 要校验CRC的数据
                CrcLen    -> 数据长度
                CheckData -> CRC校验结果  
 * @return      Size -> 卡号容量 
 **************************************************************************************************
 */
void MFRC522_CrcCheck(uint8 *CrcData, uint8 CrcLen, uint8 *CheckData)
{
  uint8 i;
  uint8 CrcIrq;
  
  MFRC522_ClearBitMask(DivIrqReg,0x04);     //BIT2=CRCIrq(0)
  MFRC522_SetBitMask(FIFOLevelReg,0x80);    //BIT7=FlushBuffer(1), FIFO初始化
  
  /*1.向FIFO中写入数据*/
  for(i=0; i<CrcLen; i++)
  {
    MFRC522_Write(FIFODataReg,CrcData[i]);
  }
  MFRC522_Write(CommandReg,PCD_CALCCRC);    //激活CRC协处理器或执行自测试
  
  /*2.等待CRC校验完成*/
  i = 0xFF;
  do
  {
    CrcIrq = MFRC522_Read(DivIrqReg);       //BIT2=CRCIrq,CRC协处理器在处理完FIFO缓冲区的所有数据后置位CRCIRq标志
    i--;
  }
  while((i!=0)&&!(CrcIrq & 0x04));          //1)i=0超时跳出 2)处理完FIFO缓冲区的所有数据跳出
  
  /*3.读取CRC校验结果*/
  CheckData[0] = MFRC522_Read(CRCResultRegL); 
  CheckData[1] = MFRC522_Read(CRCResultRegM);
  
}



/**************************************************************************************************
 * @fn          MFRC522_RequestCard
 * @brief       寻卡，读取卡类型号
 * @param       ReqMode   ->  寻卡方式
                CardType  ->  返回卡片类型
                    	        0x4400 = Mifare_UltraLight
 *				0x0400 = Mifare_One(S50)  //Mifare_One(S50) 校园卡
 *				0x0200 = Mifare_One(S70)
 *				0x0800 = Mifare_Pro(X)
 *				0x4403 = Mifare_DESFire
 * @return      ReqStatus ->  寻卡结果  
 **************************************************************************************************
 */

uint8 MFRC522_RequestCard(uint8 ReqMode,uint8 *CardType)
{   
  uint8 ReqStatus = MFRC522_ERR;  
  uint32 ReqBits;   //接收到的数据有效位数
  
  MFRC522_Write(BitFramingReg,0x07);  //BIT2-BIT0=TxLastBists(111)?
  //ReqStatus = MFRC522_Read(BitFramingReg);
  
  CardType[0] = ReqMode;
  
  ReqStatus = MFRC522_ToCard(PCD_TRANSCEIVE, CardType, 1, CardType, &ReqBits);
  
  if((ReqStatus != MFRC522_OK)||(ReqBits != 0x10))       //如果返回的不是16bit 0x0400正好是16bit
  {
    ReqStatus = MFRC522_ERR;
  }
  
  return ReqStatus; 
}


/**************************************************************************************************
 * @fn          MFRC522_Anticoll
 * @brief       RC522和ISO14443卡通讯
 * @param       CardData  -> 卡号数据
 * @return      ReqStatus -> 寻卡结果 
 **************************************************************************************************
 */
uint8 MFRC522_Anticoll(uint8 *CardData)
{
  uint8 i;
  uint8 AntiStatus = MFRC522_ERR;
  uint32 AntiBits;    //接收到的数据有效位数
  uint8 CardDataCheck = 0;

  MFRC522_ClearBitMask(Status2Reg,0x08);  //关闭内部温度传感器
  MFRC522_Write(BitFramingReg,0x00);      //BIT2-BIT0=TxLastBists(000)  定义发送的最后一个字节的位数，000表示最后一个字节的所有位都应发送
  MFRC522_ClearBitMask(CollReg,0x80);     //BIT7=ValuesAfterColl(1) 
  
  CardData[0] = PICC_ANTICOLL;
  CardData[1] = 0x20;
  
  AntiStatus = MFRC522_ToCard(PCD_TRANSCEIVE,CardData,2,CardData,&AntiBits);
  
  if(AntiStatus == MFRC522_OK)
  {
    /*校验卡号*/
    for(i=0; i<4; i++)
    {
      CardDataCheck ^= CardData[i];      //1.datasheet/RFID/MifareCard/1.M1简单介绍，4byte卡号 1byte校验字
    }
    
    if(CardDataCheck != CardData[i])
    {
      AntiStatus = MFRC522_ERR;
    }
  }
  
  MFRC522_SetBitMask(CollReg,0x80);      //BIT7=ValuesAfterColl(1) 
  return AntiStatus;
}

/**************************************************************************************************
 * @fn          MFRC522_SelectTag
 * @brief       选卡，读取卡存储器容量
 * @param       CardData  -> 卡号数据
 * @return      Size -> 卡号容量 
 **************************************************************************************************
 */
uint8 MFRC522_SelectTag(uint8 *CardData)
{
  uint8 Buff[9];
  uint8 i;
  uint32 Bits;
  uint8 Size;
  uint8 Status  = MFRC522_ERR; 
  
  MFRC522_ClearBitMask(Status2Reg,0x08);    //BIT3=MFCrypto1On(1)
  Buff[0] = PICC_SElECTTAG;
  Buff[1] = 0x70;                           //?
  for(i=0; i<5; i++)
  {
    Buff[i+2] = CardData[i];
  }
  
  MFRC522_CrcCheck(Buff,7,&Buff[7]);        //buff[7] buff[8]表明CRC校验的低八位和高八位
    
  Status = MFRC522_ToCard(PCD_TRANSCEIVE, Buff, 9, Buff, &Bits);
  
  if((Status == MFRC522_OK) && (Bits == 0x18)) //0x18 = 24 24/8 =3Byte
  {
    Size = Buff[0];
  }
  
  else
  {
    Size = 0;
  }
  
  return Size;
}


/**************************************************************************************************
 * @fn          MFRC522_Auth
 * @brief       验证卡片密码
 * @param       authMode  -> 密码验证模式
                                          0x60 = 验证A密钥
                                          0x61 = 验证B密钥 
                BlockAddr -> 块地址(共0-63块)，其中块0放了厂商的初始数据，包括卡号等。
                SectorKey -> 扇区密码 6byte
                CardData  -> 卡号，4Byte
 * @return      成功返回  -> MI_OK 
 **************************************************************************************************
 */
uint8 MFRC522_Auth(uint8 AuthMode, uint8 BlockAddr, uint8 *SectorKey, uint8 *CardData)
{
  uint8 Buff[12];
  uint8 i;
  uint32 Bits;
  uint8 Status  = MFRC522_ERR; 
  
  /*验证指令1byte+块地址1byte＋扇区密码6byte＋卡序列号4byte*/
  Buff[0] = AuthMode;
  Buff[1] = BlockAddr;
  
  for(i=0; i<6; i++)
  {
    Buff[i+2] = SectorKey[i];
  }
  
  for(i=0; i<4; i++)
  {
    Buff[i+8] = CardData[i];
  }
  
  Status = MFRC522_ToCard(PCD_AUTHENT, Buff, 12, Buff, &Bits);
  
  if ((Status != MFRC522_OK) || (!(MFRC522_Read(Status2Reg) & 0x08)))    //BIT3=MFCryptolOn(1) 成功执行Auth后才会置1，读写模式时使用
  {   
    Status = MFRC522_ERR;   
  }
  return Status;
}


/**************************************************************************************************
 * @fn          MFRC522_WriteToBlock
 * @brief       写块数据
 * @param       BlockAddr -> 块地址(共0-63块)，其中块0放了厂商的初始数据，包括卡号等。
                WriteData -> 向块写16字节数据
 * @return      成功返回  -> MI_OK 
 **************************************************************************************************
 */
uint8 MFRC522_WriteToBlock(uint8 BlockAddr, uint8 *WriteData)
{
  uint8 Buff[18];
  uint8 i;
  uint8 Status;
  uint32 Bits;
  
  Buff[0] = PICC_WRITE;
  Buff[1] = BlockAddr;
  
  MFRC522_CrcCheck(Buff,2,&Buff[2]);
  Status = MFRC522_ToCard(PCD_TRANSCEIVE, Buff, 4, Buff, &Bits);
  
  if((Status != MFRC522_OK) || (Bits != 4) || ((Buff[0] & 0x0F) != 0x0A))
  {
     Status = MFRC522_ERR;
  }
  
  if(Status == MFRC522_OK)
  {
    for(i=0; i<16; i++)
    {
      Buff[i] = WriteData[i];
    }
    MFRC522_CrcCheck(Buff,16,&Buff[16]);
    Status = MFRC522_ToCard(PCD_TRANSCEIVE, Buff, 18, Buff, &Bits);
    if((Status != MFRC522_OK) || (Bits != 4) || ((Buff[0] & 0x0F) != 0x0A))
    {
       Status = MFRC522_ERR;
    }
  }
  
  return Status;
}


/**************************************************************************************************
 * @fn          MFRC522_ReadFromBlock
 * @brief       读块数据
 * @param       BlockAddr -> 块地址(共0-63块)，其中块0放了厂商的初始数据，包括卡号等。
                ReadData  -> 向块写16字节数据
 * @return      成功返回  -> MI_OK 
 **************************************************************************************************
 */
uint8 MFRC522_ReadFromBlock(uint8 BlockAddr, uint8 *ReadData)
{
  uint8 Status;
  uint32 Bits;
  ReadData[0] = PICC_READ;
  ReadData[1] = BlockAddr;
  MFRC522_CrcCheck(ReadData, 2, &ReadData[2]);
  
  Status = MFRC522_ToCard(PCD_TRANSCEIVE, ReadData, 4, ReadData, &Bits);
  if((Status != MFRC522_OK) || (Bits != 0x90))    //18*8 = 144 
  {
     Status = MFRC522_ERR;
  }
  return Status;
}


/**************************************************************************************************
 * @fn          MFRC522_Sleep
 * @brief       命令卡片进入休眠状态
 * @param       无
 * @return      无
 **************************************************************************************************
 */
uint8 MFRC522_Sleep(void)
{
  uint32 Bits;
  uint8 Buff[4];
  
  Buff[0] = PICC_HALT;
  Buff[1] = 0;
  
  MFRC522_CrcCheck(Buff, 2, &Buff[2]);
  
  MFRC522_ToCard(PCD_TRANSCEIVE, Buff, 4, Buff, &Bits);
  
  return MFRC522_OK;
  
}






/**************************************************************************************************
 * @fn          MFRC522_ToCard
 * @brief       RC522和ISO14443卡通讯
 * @param       Command       ->  MF522命令字
                SendData      ->  过RC522发送到卡片的数据,
                SendLen       ->  发送的数据长度
                ReceiveData   ->  接收到的卡片返回数据
                ReveiveLen    ->  返回数据的位长度
 * @return      ReqStatus     ->  寻卡结果 
 **************************************************************************************************
 */
uint8 MFRC522_ToCard(uint8 Command, uint8 *SendData, uint8 SendLen, uint8 *ReceiveData, uint32 *ReveiveBits)
{
  uint8 Status  = MFRC522_ERR; 
  uint8 IrqEn   = 0x00;
  uint8 WaitIrq = 0x00; 
  uint32 i;
  uint8 IRqStatus;
  uint8 FIFOLevel  = 0x00;       //FIFOLevelReg BIT6-BIT0 FIFO缓冲区中保存的字节数，BIT7被读时返回为0无须担心
  uint8 RxLastBits = 0x00;        //ControlReg   BIT2-BIT0 显示最后接收到的字节的有效位数目，如果该位为0，整个字节有效 
    
  switch(Command)
  {
    case PCD_AUTHENT:     IrqEn   = 0x12;    //认证卡密
                          WaitIrq = 0x10;
                          break;
                      
    case PCD_TRANSCEIVE:  IrqEn   = 0x77;    //发送FIFO中数据
                          WaitIrq = 0x30;
                          break;
                        
    default:              break;
  }
  
  MFRC522_Write(CommIEnReg, IrqEn|0x80);     //允许中断请求
  MFRC522_ClearBitMask(CommIrqReg, 0x80);    //清除所有中断请求位
  MFRC522_SetBitMask(FIFOLevelReg, 0x80);    //BIT7=FlushBuffer(1), FIFO初始化
  MFRC522_Write(CommandReg, PCD_IDLE);       //无动作，取消当前动作的执行
  
  /*1.向FIFO中写入数据*/
  for(i=0; i<SendLen; i++)
  {
    MFRC522_Write(FIFODataReg,SendData[i]);
  }
  
  /*2.执行发送命令，将FIFO缓冲区的数据发送到天线，发送完成后自动激活接收器*/
  MFRC522_Write(CommandReg,Command);
  if((Command == PCD_TRANSCEIVE) ||(Command == PCD_AUTHENT))               
  { 
    MFRC522_SetBitMask(BitFramingReg,0x80);   //BIT7=StartSend(1):启动数据发送
  }
  
  /*3.等待数据接收完成*/
  i = 2000;                                   //i根据时钟频率调整，操作M1卡最大等待时间15ms?注意上面MFRC522_Reset函数中定时器的设置时间是15ms
  do
  {
    IRqStatus = MFRC522_Read(CommIrqReg);
    i--;
  }
  while((i!=0)&& !(IRqStatus&0x01) && !(IRqStatus&WaitIrq)); //1)i=0超时跳出 2)定时器(15ms)减到0时跳出 3)命令终止或者接收完成跳出
  MFRC522_ClearBitMask(BitFramingReg,0x80);   //停止数据发送
  
  
  /*4.检测寻卡状态*/
  if(i != 0)
  {
    /*4.1.寻卡未发生错误*/
    if(!(MFRC522_Read(ErrorReg) & 0x1B))      //未发生以下错误：BufferOvfl Collerr CRCErr ProtecolErr
    {
      Status = MFRC522_OK;                    //这里可以设一个断点查看,表明未发生错误
      
      if(IRqStatus & IrqEn & 0x01)            //定时器递减到零,寻卡超时
      {
        Status = MFRC522_TIMEOUT;              
      }
      
       /*4.1.1 接收到的数据处理*/
      if(Command == PCD_TRANSCEIVE)                     
      {
         FIFOLevel  = MFRC522_Read(FIFOLevelReg);       //接收到的字节数
         RxLastBits = MFRC522_Read(ControlReg) & 0x07;  //最后一个字节的有效数
         
         if(RxLastBits)                                 //如果不是0，最后一个字节并不是整个字节有效
         {
           *ReveiveBits = (FIFOLevel-1)*8 + RxLastBits;  
         }
         else                                           //如果是0，整个字节有效
         {
           *ReveiveBits = FIFOLevel * 8;
         }
         if(FIFOLevel == 0)
         {
           FIFOLevel = 1;
         }
         if(FIFOLevel > FIFOLEVEL_MAX)
         {
           FIFOLevel = FIFOLEVEL_MAX;
         }
         
         /*4.1.2 读取接收到的数据*/
         for(i=0; i<FIFOLevel; i++)
         {
           ReceiveData[i] = MFRC522_Read(FIFODataReg);    //读取FIFO缓冲区的数据
         }
      }
    }
    
    /*4.2.寻卡发生错误，错误类型未知*/
    else                                                  
    {
      Status = MFRC522_ERR;
    }
  }
  
  /*5.清除*/
  MFRC522_SetBitMask(ControlReg,0x80);  //停止定时器
  MFRC522_Write(CommandReg,PCD_IDLE);   //无动作，取消当前动作的执行
  
  return Status;
}


/* ------------------------------------------------------------------------------------------------
 *                                          Middle
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          MFRC522_ClearBitMask
 * @brief       清RC522寄存器位
 * @param       RegAddr   -> 寄存器地址
                Mask      -> 清位值
 * @return      无
 **************************************************************************************************
 */
void MFRC522_ClearBitMask(uint8 RegAddr,uint8 Mask)
{
  uint8 tmp;
  tmp = MFRC522_Read(RegAddr);
  MFRC522_Write(RegAddr, tmp&(~Mask));
}

/**************************************************************************************************
 * @fn          MFRC522_SetBitMask
 * @brief       清RC522寄存器位
 * @param       RegAddr   -> 寄存器地址
                Mask      -> 置位值
 * @return      无
 **************************************************************************************************
 */
void MFRC522_SetBitMask(uint8 RegAddr,uint8 Mask)
{
  uint8 tmp;
  tmp = MFRC522_Read(RegAddr);
  MFRC522_Write(RegAddr, tmp | Mask);
}





/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/**************************************************************************************************
 * @fn          MFRC522_Delay
 * @brief       1us左右的延时
 * @param       n
 * @return      无
 **************************************************************************************************
 */
void MFRC522_Delay(uint8 n)
{
  uint8 i;
  for (i=0; i<n; i++)
  {
    asm("nop");
  }
}


/**************************************************************************************************
 * @fn          MFRC522_Write
 * @brief       CC2530发送数据
 * @param       RegAddr   -> 寄存器地址
                Data      -> 待发送的数据
 * @return      无
 **************************************************************************************************
 */
void MFRC522_Write(uint8 RegAddr,uint8 Data)
{
  uint8 Addr;
  CS  = 0;  //片选信号低电平才能处理数据
  SCK = 0;  //时钟低电平空闲
  
/*|----------------------------------------------|
  |      |       BIT7       | BIT6-BIT1 |  BIT0  |
  | MOSI |---------------------------------------|
  |      | Read(1) Write(0) |    Addr   | RFU(0) |
  |----------------------------------------------|*/
  
  Addr = ((RegAddr << 1)&0x7E);   //左移1位，并把头尾清0， BIT7(0):写入操作，BIT0(0) 
  
  Spi_WriteByte(Addr);              
  Spi_WriteByte(Data);  
  
  CS  = 1;  //关闭片选
  SCK = 1; 

}

/**************************************************************************************************
 * @fn          MFRC522_Read
 * @brief       CC2530读取数据
 * @param       RegAddr   -> 寄存器地址
 * @return      无
 **************************************************************************************************
 */
uint8 MFRC522_Read(uint8 RegAddr)
{
  uint8 Data,Addr;
  
  CS  = 0; 
  SCK = 0; 
  
  Addr = ((RegAddr << 1)&0x7E)|0x80; //左移1位，并把头置1，尾清0，BIT7(1):读取操作，BIT0(0) 
  
  Spi_WriteByte(Addr); 
  Data = Spi_ReadByte();
  
  CS  = 1;  //关闭片选
  SCK = 1; 
  return Data;
}