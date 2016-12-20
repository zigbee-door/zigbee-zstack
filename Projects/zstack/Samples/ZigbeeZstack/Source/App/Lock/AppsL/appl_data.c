#define APPL_DATA_C_

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "appl_data.h"

/***************************数据信息初始化*************************************/

/*******************************************
 * @fn          Data_Door_Init
 * @brief       门锁初始化
 * @param       无
 * @return      DataResult -> DATA_ERR  初始化失败
                DataResult -> DATA_OK   初始化成功
 *******************************************
 */
uint8 Data_Door_Init(void)
{
  uint8 DataResult = DATA_OK;                                                 //数据操作标识
  
  DataResult = Data_CommonCard_Init();                                          //普通卡列表初始化
  
  
  return DataResult;
}





/*******************************************
 * @fn          Data_CommonCard_Init
 * @brief       普通卡列表初始化
 * @param       无
 * @return      DataResult -> DATA_ERR  初始化失败
                           -> DATA_OK   初始化成功
 *******************************************
 */
uint8 Data_CommonCard_Init(void)
{
  uint8 NullCardRom[4] = {'=','=','=','='};                                   //普通卡空内存标识
  uint8 ReadData[4]    = {0x00};                                            
  uint8 DataResult = DATA_OK;                                                 //数据操作标识
  uint16 i;
  
  /*1.普通卡内存清空*/
  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                                        
  {
    AT24C256_WriteBuff(AT24C256_WRITE_ADDR,i,NullCardRom,4);
    Delay_Ms(5);                                                             //一定要给一个小延时，写和读要有时间差异
  }
  

  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                     //普通卡内存清空验证
  {
    AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
    Delay_Ms(5); 
    if(strncmp((const char*)ReadData,(const char*)NullCardRom,4) != 0)          //如果数据不相等
    {
      DataResult = DATA_ERR;
      break;
    }
  }
  
  
  /*2.如果写入读取不一致，再次清空*/
  if(DataResult == DATA_ERR)
  {
    for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                                     
    {
      AT24C256_WriteBuff(AT24C256_WRITE_ADDR,i,NullCardRom,4);
      Delay_Ms(5); 
    }
    
    for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                   //普通卡内存清空验证
    {
      AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
      Delay_Ms(5); 
      if(strncmp((const char*)ReadData,(const char*)NullCardRom,4) != 0)        //如果数据不相等
      {
        DataResult = DATA_ERR;
        break;
      }
    }
  }
  
  return DataResult;
  
}

/*******************************************
 * @fn          Data_DoorID_Init
 * @brief       门锁ID信息初始化,测试用
 * @param       无
 * @return      无
 *******************************************
 */


void Data_DoorID_Init(void)
{
  uint8 data[4] = {0x00};
  
  uint8 Data[4] = {DOOR_COMPANY,DOOR_BUILD,DOOR_FLOOR,DOOR_ID};
  AT24C256_WriteBuff(AT24C256_WRITE_ADDR,DOORID_BASE_ADDR,Data,4);
  Delay_Ms(100);
  AT24C256_ReadBuff(AT24C256_WRITE_ADDR,DOORID_BASE_ADDR,data,4);
  Delay_Ms(100);
}




/***************************门锁ID数据操作*************************************/

/*******************************************
 * @fn          Data_DoorID_Read
 * @brief       门锁ID信息读取
 * @param       DoorId -> 门锁ID
 * @return      无
 *******************************************
 */
void Data_DoorID_Read(uint8 *DoorId)
{
  AT24C256_ReadBuff(AT24C256_WRITE_ADDR,DOORID_BASE_ADDR,DoorId,4);
  Delay_Ms(5);
}


/***************************普通卡数据操作*************************************/

/*******************************************
 * @fn          Data_CommonCard_Auth
 * @brief       普通卡授权存储
 * @param       CardId -> 需要授权的卡号
 * @return      Result
                 -> DATA_OK      授权成功
                 -> DATA_FULL    卡号已满 100张
                 -> DATA_ERR     卡号已授权或者存储失败
 *******************************************
 */
uint8 Data_CommonCard_Auth(uint8 *CardId)
{
  uint8 DataResult    = DATA_OK;
  uint8 ReadData[4]   = {0x00};
  uint8 NullRom[4]    = {'=','=','=','='};                                    //空内存标识
  uint8 UsedRom[4]    = {'-','-','-','-'};                                    //使用过的内存标识
  uint16 NullPosition = COMMONCARD_MAX1_ADDR;                                 //空内存位置
  uint16 UsedPosition = COMMONCARD_MAX1_ADDR;                                 //使用过内存
  uint16 i;
  
  
  /*1. 轮询所有的普通卡列表*/
  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                                                
  {
    /*1.1 读取4字节的普通卡地址信息*/
    AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
    
    //这个很重要，轮询读取的时候
    Delay_Ms(5);                                                              //一定要给个小延时
    
    /*1.2 如果普通卡已经授权，则退出*/
    if(strncmp((const char*)CardId,(const char*)ReadData,4)== 0)                
    {
      DataResult = DATA_ERR;
      break;
    }
    
    /*1.3 如果两个位置都被记录，则不会进去判断了 */
    else if((NullPosition == COMMONCARD_MAX1_ADDR) || (UsedPosition == COMMONCARD_MAX1_ADDR))
    {
      
      /*1.3.1 第一次循环满足该条件，如果被记录新的位置则不会进来*/
      if(NullPosition == COMMONCARD_MAX1_ADDR)
      {
        /*1.3.1.1 如果是未使用过的内存，记录该内存位置 */
        if(strncmp((const char*)ReadData,(const char*)NullRom,4)== 0)           
        {
          NullPosition = i;                                                     //(RomPosition + 1) % 4  == 0
        }
      }
      
      /*1.3.2 第一次循环满足该条件，如果被记录新的位置则不会进来*/
      if(UsedPosition == COMMONCARD_MAX1_ADDR)
      {
         /*1.3.2.1 如果是使用过但是被擦除的内存，记录该使用过但被擦除的内存 */
        if(strncmp((const char*)ReadData,(const char*)UsedRom,4)== 0)           
        {
          UsedPosition = i;                                                     //(UsedPosition + 1) % 4  == 0
        }
      }
    }
  }
  
  /*2. 如果该普通卡没有被授过权*/
  if(DataResult == DATA_OK)
  {
    /*2.1 空内存有位置，则优先存储空内存，需要注意空内存如果有位置不可能是COMMONCARD_MAX1_ADDR，最大是需要注意空内存如果有位置不可能是COMMONCARD_MAX1_ADDR-4*/
    if((NullPosition != COMMONCARD_MAX1_ADDR) && ((NullPosition % 4) == 0))
    {
      DataResult = Data_Storage(CardId,NullPosition);                           
    }
    
    /*2.2 空内存没有位置，则考虑存储被使用过但是被擦除的内存，这样做是为了让存储更加均匀，简单考虑也可以没有被擦除的内存，而全部设置成空内存*/
    else if((UsedPosition != COMMONCARD_MAX1_ADDR) && ((UsedPosition % 4) == 0))
    {
      DataResult = Data_Storage(CardId,UsedPosition);                           
    }
    
    /*2.3 如果存储容量已满 */
    else if((NullPosition == COMMONCARD_MAX1_ADDR) && (UsedPosition == COMMONCARD_MAX1_ADDR))
    {
      DataResult = DATA_FULL;                                                 
    }
  }
  
  return DataResult;
}

/*******************************************
 * @fn          Data_CommonCard_UnAuth
 * @brief       普通卡删权存储
 * @param       CardId -> 需要删权的卡号
 * @return      Result
                 -> DATA_OK     删普通卡成功
                 -> DATA_ERR    删普通卡失败，没有这张卡,或者存储数据失败
 *******************************************
 */
uint8 Data_CommonCard_UnAuth(uint8 *CardId)
{
  uint8 DataResult  = DATA_ERR;
  uint8 ReadData[4] = {0x00};
  uint8 UsedRom[4]  = {'-','-','-','-'};                                      //使用过的内存标识
  uint16 i;
  
  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4) 
  {
    AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
    
    Delay_Ms(5);    //给个5ms的小延时
    
    if(strncmp((const char*)CardId,(const char*)ReadData,4) == 0)               //判断是否已授权过该卡
    {
      DataResult = Data_Storage(UsedRom,i);
      break;
    }
  }
  
  return DataResult;
}


/*******************************************
 * @fn          Data_CommonCard_Confirm
 * @brief       普通卡确认
 * @param       CardId -> 需要确认的卡号
 * @return      Result
                 -> DATA_OK     有该卡
                 -> DATA_ERR    无该卡
 *******************************************
 */
uint8 Data_CommonCard_Confirm(uint8 *CardId)
{
  uint8 DataResult = DATA_ERR;
  uint8 ReadData[4] = {0x00};
  uint16 i;
  
  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4) 
  {
    AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
    Delay_Ms(5);    
    if(strncmp((const char*)CardId,(const char*)ReadData,4)== 0)                //判断是否已授权过该卡
    {
      DataResult = DATA_OK;
      break;
    }
  }
  
  return DataResult;
}
















/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/*******************************************
 * @fn          Data_Storage
 * @brief       存储判断
 * @param       WriteId   -> 需要存储的卡号
                WriteAddr -> 存储的位置

 * @return      Result
                 -> AT24C256_OK     存储成功
                 -> AT24C256_ERR    存储失败
 *******************************************
 */
uint8 Data_Storage(uint8 *WriteId, uint16 WriteAddr)
{
  uint8 ReadId[4] = {0x00};
  uint8 DataResult = DATA_OK;
  
  AT24C256_WriteBuff(AT24C256_WRITE_ADDR,WriteAddr,WriteId,4);
  
  
  Delay_Ms(5);    //写完之后需要给个5ms的小延时
    
  AT24C256_ReadBuff(AT24C256_WRITE_ADDR,WriteAddr,ReadId,4);
  
  Delay_Ms(5);    
  
  if(strncmp((const char*)ReadId,(const char*)WriteId,4) != 0)                  //判断存储及是否成功
  {
    AT24C256_WriteBuff(AT24C256_WRITE_ADDR,WriteAddr,WriteId,4);
    if(strncmp((const char*)ReadId,(const char*)WriteId,4) != 0)                //再次判断存储及是否成功
    {
      DataResult = DATA_ERR;                                                    //存储失败
    }
  } 

  return DataResult;
}