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
  }
  
  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                     //普通卡内存清空验证
  {
    AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
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
    }
    
    for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                   //普通卡内存清空验证
    {
      AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
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
 * @brief       门锁ID信息初始化
 * @param       无
 * @return      无
 *******************************************
 */
//void Data_DoorID_Init(void)
//{
//  uint8 DoorId[4] = {0x00};
//  uint8 Data[4] = {DOOR_COMPANY,DOOR_BUILD,DOOR_FLOOR,DOOR_ID};
//  AT24C256_WriteBuff(AT24C256_WRITE_ADDR,DOORID_BASE_ADDR,Data,4);
//  
////  AT24C256_ReadBuff(AT24C256_WRITE_ADDR,DOORID_BASE_ADDR,DoorId,4);
//
//}




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
  
  for(i=COMMONCARD_BASE_ADDR; i<COMMONCARD_MAX1_ADDR; i+=4)                                                
  {
    AT24C256_ReadBuff(AT24C256_WRITE_ADDR,i,ReadData,4);
    if(strncmp((const char*)CardId,(const char*)ReadData,4)== 0)                //判断是否已经授权过该卡
    {
      DataResult = DATA_ERR;
      break;
    }
    
    else if((NullPosition == COMMONCARD_MAX1_ADDR) || (UsedPosition == COMMONCARD_MAX1_ADDR))
    {
      if(NullPosition == COMMONCARD_MAX1_ADDR)
      {
        if(strncmp((const char*)ReadData,(const char*)NullRom,4)== 0)           //寻找空内存
        {
          NullPosition = i;                                                     //(RomPosition + 1) % 4  == 0
        }
      }
      
      if(UsedPosition == COMMONCARD_MAX1_ADDR)
      {
        if(strncmp((const char*)ReadData,(const char*)UsedRom,4)== 0)           //寻找使用过的内存
        {
          UsedPosition = i;                                                     //(UsedPosition + 1) % 4  == 0
        }
      }
    }
  }
  
  if(DataResult == DATA_OK)
  {
    if((NullPosition != COMMONCARD_MAX1_ADDR) && ((NullPosition % 4) == 0))
    {
      DataResult = Data_Storage(CardId,NullPosition);                           //普通卡存储 
    }
    
    else if((UsedPosition != COMMONCARD_MAX1_ADDR) && ((UsedPosition % 4) == 0))
    {
      DataResult = Data_Storage(CardId,UsedPosition);                           //普通卡存储 
    }
    
    else if((NullPosition == COMMONCARD_MAX1_ADDR) && (UsedPosition == COMMONCARD_MAX1_ADDR))
    {
      DataResult = DATA_FULL;                                                   //存储容量已满
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
    
  AT24C256_ReadBuff(AT24C256_WRITE_ADDR,WriteAddr,ReadId,4);
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