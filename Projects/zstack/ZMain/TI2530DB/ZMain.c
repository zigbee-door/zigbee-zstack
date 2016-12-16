#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_Nv.h"
#include "OnBoard.h"
#include "ZMAC.h"
#ifndef NONWK
  #include "AF.h"
#endif
/* Hal */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_adc.h"
#include "hal_drivers.h"
#include "hal_assert.h"
#include "hal_flash.h"


// Maximun number of Vdd samples checked before go on
#define MAX_VDD_SAMPLES  3
#define ZMAIN_VDD_LIMIT  HAL_ADC_VDD_LIMIT_4

extern bool HalAdcCheckVdd (uint8 limit);


static void zmain_dev_info( void );
static void zmain_ext_addr( void );
static void zmain_vdd_check( void );

#ifdef LCD_SUPPORTED
static void zmain_lcd_init( void );
#endif


int main( void )
{
  
  /*1.系统初始化*/
  osal_int_disable( INTS_ALL );   //关操作系统全部中断
  HAL_BOARD_INIT();               //初始化硬件设备，例如CPU的晶振以及LED的引脚输入输出等
  zmain_vdd_check();              //电源检测,检测电源是否足够运行
  InitBoard( OB_COLD );           //初始化板子LED和按键等
  HalDriverInit();                //初始化硬件抽象层HAL驱动
  osal_nv_init( NULL );           //初始化非易失性单元Flash存储器
  ZMacInit();                     //初始化MAC层
  zmain_ext_addr();               //得到并显示设备的外部地址，确定IEEE 64位地址
  zgInit();                       //初始化非易失性变量
#ifndef NONWK                     //初始化AF层
  afInit();                       //Since the AF isn't a task, call it's initialization routine
#endif
  osal_init_system();             //初始化操作系统,包括了任务初始化
  osal_int_enable( INTS_ALL );    //使能操作系统全部中断
  InitBoard( OB_READY );          //板子最终初始化
  zmain_dev_info();               //显示设备信息
#ifdef LCD_SUPPORTED
  zmain_lcd_init();               //初始化LCD
#endif
#ifdef WDT_IN_PM1
  WatchDogEnable( WDTIMX );       //If WDT is used, this is a good place to enable it.
#endif
  
  /*2.操作系统任务*/
  osal_start_system();             //执行操作系统，进入主循环
  return 0;                        //这里执行不到
} 

/*********************************************************************
 * @fn      zmain_vdd_check
 * @brief   Check if the Vdd is OK to run the processor.
 * @return  Return if Vdd is ok; otherwise, flash LED, then reset
 *********************************************************************/
static void zmain_vdd_check( void )
{
  uint8 vdd_passed_count = 0;
  bool toggle = 0;

  // Repeat getting the sample until number of failures or successes hits MAX
  // then based on the count value, determine if the device is ready or not
  while ( vdd_passed_count < MAX_VDD_SAMPLES )
  {
    if ( HalAdcCheckVdd (ZMAIN_VDD_LIMIT) )
    {
      vdd_passed_count++;    // Keep track # times Vdd passes in a row
      MicroWait (10000);     // Wait 10ms to try again
    }
    else
    {
      vdd_passed_count = 0;  // Reset passed counter
      MicroWait (50000);     // Wait 50ms
      MicroWait (50000);     // Wait another 50ms to try again
    }

    /* toggle LED1 and LED2 */
    if (vdd_passed_count == 0)
    {
      if ((toggle = !(toggle)))
        HAL_TOGGLE_LED1();
      else
        HAL_TOGGLE_LED2();
    }
  }

  /* turn off LED1 */
  HAL_TURN_OFF_LED1();
  HAL_TURN_OFF_LED2();
}

/**************************************************************************************************
 * @fn          zmain_ext_addr
 *
 * @brief       Execute a prioritized search for a valid extended address and write the results
 *              into the OSAL NV system for use by the system. Temporary address not saved to NV.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void zmain_ext_addr(void)
{
  uint8 nullAddr[Z_EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8 writeNV = TRUE;

  // First check whether a non-erased extended address exists in the OSAL NV.
  if ((SUCCESS != osal_nv_item_init(ZCD_NV_EXTADDR, Z_EXTADDR_LEN, NULL))  ||
      (SUCCESS != osal_nv_read(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, aExtendedAddress)) ||
      (osal_memcmp(aExtendedAddress, nullAddr, Z_EXTADDR_LEN)))
  {
    // Attempt to read the extended address from the location on the lock bits page
    // where the programming tools know to reserve it.
    HalFlashRead(HAL_FLASH_IEEE_PAGE, HAL_FLASH_IEEE_OSET, aExtendedAddress, Z_EXTADDR_LEN);

    if (osal_memcmp(aExtendedAddress, nullAddr, Z_EXTADDR_LEN))
    {
      // Attempt to read the extended address from the designated location in the Info Page.
      if (!osal_memcmp((uint8 *)(P_INFOPAGE+HAL_INFOP_IEEE_OSET), nullAddr, Z_EXTADDR_LEN))
      {
        osal_memcpy(aExtendedAddress, (uint8 *)(P_INFOPAGE+HAL_INFOP_IEEE_OSET), Z_EXTADDR_LEN);
      }
      else  // No valid extended address was found.
      {
        uint8 idx;
        
#if !defined ( NV_RESTORE )
        writeNV = FALSE;  // Make this a temporary IEEE address
#endif

        /* Attempt to create a sufficiently random extended address for expediency.
         * Note: this is only valid/legal in a test environment and
         *       must never be used for a commercial product.
         */
        for (idx = 0; idx < (Z_EXTADDR_LEN - 2);)
        {
          uint16 randy = osal_rand();
          aExtendedAddress[idx++] = LO_UINT16(randy);
          aExtendedAddress[idx++] = HI_UINT16(randy);
        }
        // Next-to-MSB identifies ZigBee devicetype.
#if ZG_BUILD_COORDINATOR_TYPE && !ZG_BUILD_JOINING_TYPE
        aExtendedAddress[idx++] = 0x10;
#elif ZG_BUILD_RTRONLY_TYPE
        aExtendedAddress[idx++] = 0x20;
#else
        aExtendedAddress[idx++] = 0x30;
#endif
        // MSB has historical signficance.
        aExtendedAddress[idx] = 0xF8;
      }
    }

    if (writeNV)
    {
      (void)osal_nv_write(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, aExtendedAddress);
    }
  }

  // Set the MAC PIB extended address according to results from above.
  (void)ZMacSetReq(MAC_EXTENDED_ADDRESS, aExtendedAddress);
}

/**************************************************************************************************
 * @fn          zmain_dev_info
 *
 * @brief       This displays the IEEE (MSB to LSB) on the LCD.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void zmain_dev_info(void)
{
#ifdef LCD_SUPPORTED
  uint8 i;
  uint8 *xad;
  uint8 lcd_buf[Z_EXTADDR_LEN*2+1];

  // Display the extended address.
  xad = aExtendedAddress + Z_EXTADDR_LEN - 1;

  for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
  {
    uint8 ch;
    ch = (*xad >> 4) & 0x0F;
    lcd_buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    ch = *xad & 0x0F;
    lcd_buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
  }
  lcd_buf[Z_EXTADDR_LEN*2] = '\0';
  HalLcdWriteString( "IEEE: ", HAL_LCD_LINE_1 );
  HalLcdWriteString( (char*)lcd_buf, HAL_LCD_LINE_2 );
#endif
}

#ifdef LCD_SUPPORTED
/*********************************************************************
 * @fn      zmain_lcd_init
 * @brief   Initialize LCD at start up.
 * @return  none
 *********************************************************************/
static void zmain_lcd_init ( void )
{
#ifdef SERIAL_DEBUG_SUPPORTED
  {
    HalLcdWriteString( "TexasInstruments", HAL_LCD_LINE_1 );

#if defined( MT_MAC_FUNC )
#if defined( ZDO_COORDINATOR )
      HalLcdWriteString( "MAC-MT Coord", HAL_LCD_LINE_2 );
#else
      HalLcdWriteString( "MAC-MT Device", HAL_LCD_LINE_2 );
#endif // ZDO
#elif defined( MT_NWK_FUNC )
#if defined( ZDO_COORDINATOR )
      HalLcdWriteString( "NWK Coordinator", HAL_LCD_LINE_2 );
#else
      HalLcdWriteString( "NWK Device", HAL_LCD_LINE_2 );
#endif // ZDO
#endif // MT_FUNC
  }
#endif // SERIAL_DEBUG_SUPPORTED
}
#endif

/*********************************************************************
*********************************************************************/
