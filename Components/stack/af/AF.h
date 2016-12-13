/**************************************************************************************************
  Filename:       AF.h
  Revised:        $Date: 2009-07-09 17:11:17 -0700 (Thu, 09 Jul 2009) $
  Revision:       $Revision: 20288 $

  Description:    This file contains the Application Framework definitions.


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

#ifndef AF_H
#define AF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "nwk.h"
#include "APSMEDE.h"

/*********************************************************************
 * CONSTANTS
 */

#define AF_BROADCAST_ENDPOINT              0xFF
  
#define AF_ACK_REQUEST                     0x10
#define AF_DISCV_ROUTE                     0x20
#define AF_EN_SECURITY                     0x40
#define AF_SKIP_ROUTING                    0x80

// Backwards support for afAddOrSendMessage / afFillAndSendMessage.
#define AF_TX_OPTIONS_NONE                 0
#define AF_MSG_ACK_REQUEST                 AF_ACK_REQUEST

// Default Radius Count value
#define AF_DEFAULT_RADIUS                  DEF_NWK_RADIUS

/*********************************************************************
 * Node Descriptor
 */

#define AF_MAX_USER_DESCRIPTOR_LEN         16
#define AF_USER_DESCRIPTOR_FILL          0x20
typedef struct
{
  byte len;     // Length of string descriptor
  byte desc[AF_MAX_USER_DESCRIPTOR_LEN];
} UserDescriptorFormat_t;

// Node Logical Types
#define NODETYPE_COORDINATOR    0x00
#define NODETYPE_ROUTER         0x01
#define NODETYPE_DEVICE         0x02

// Node Frequency Band - bit map
#define NODEFREQ_800            0x01    // 868 - 868.6 MHz
#define NODEFREQ_900            0x04    // 902 - 928 MHz
#define NODEFREQ_2400           0x08    // 2400 - 2483.5 MHz

// Node MAC Capabilities - bit map
//   Use CAPINFO_ALTPANCOORD, CAPINFO_DEVICETYPE_FFD,
//       CAPINFO_DEVICETYPE_RFD, CAPINFO_POWER_AC,
//       and CAPINFO_RCVR_ON_IDLE from NLMEDE.h

// Node Descriptor format structure
typedef struct
{
  uint8 LogicalType:3;
  uint8 ComplexDescAvail:1;  /* AF_V1_SUPPORT - reserved bit. */
  uint8 UserDescAvail:1;     /* AF_V1_SUPPORT - reserved bit. */
  uint8 Reserved:3;
  uint8 APSFlags:3;
  uint8 FrequencyBand:5;
  uint8 CapabilityFlags;
  uint8 ManufacturerCode[2];
  uint8 MaxBufferSize;
  uint8 MaxInTransferSize[2];
  uint16 ServerMask;
  uint8 MaxOutTransferSize[2];
  uint8 DescriptorCapability; 
} NodeDescriptorFormat_t;

// Bit masks for the ServerMask.
#define PRIM_TRUST_CENTER  0x01
#define BKUP_TRUST_CENTER  0x02
#define PRIM_BIND_TABLE    0x04
#define BKUP_BIND_TABLE    0x08
#define PRIM_DISC_TABLE    0x10
#define BKUP_DISC_TABLE    0x20
#define NETWORK_MANAGER    0x40

/*********************************************************************
 * Node Power Descriptor
 */

// Node Current Power Modes (CURPWR)
// Receiver permanently on or sync with coordinator beacon.
#define NODECURPWR_RCVR_ALWAYS_ON   0x00
// Receiver automatically comes on periodically as defined by the
// Node Power Descriptor.
#define NODECURPWR_RCVR_AUTO        0x01
// Receiver comes on when simulated, eg by a user pressing a button.
#define NODECURPWR_RCVR_STIM        0x02

// Node Available Power Sources (AVAILPWR) - bit map
//   Can be used for AvailablePowerSources or CurrentPowerSource
#define NODEAVAILPWR_MAINS          0x01  // Constant (Mains) power
#define NODEAVAILPWR_RECHARGE       0x02  // Rechargeable Battery
#define NODEAVAILPWR_DISPOSE        0x04  // Disposable Battery

// Power Level
#define NODEPOWER_LEVEL_CRITICAL    0x00  // Critical
#define NODEPOWER_LEVEL_33          0x04  // 33%
#define NODEPOWER_LEVEL_66          0x08  // 66%
#define NODEPOWER_LEVEL_100         0x0C  // 100%

// Node Power Descriptor format structure
typedef struct
{
  unsigned int PowerMode:4;
  unsigned int AvailablePowerSources:4;
  unsigned int CurrentPowerSource:4;
  unsigned int CurrentPowerSourceLevel:4;
} NodePowerDescriptorFormat_t;

/*********************************************************************
 * Simple Descriptor
 */

// AppDevVer values
#define APPDEVVER_1               0x01

// AF_V1_SUPPORT AppFlags - bit map
#define APPFLAG_NONE                0x00  // Backwards compatibility to AF_V1.

// AF-AppFlags - bit map
#define AF_APPFLAG_NONE             0x00
#define AF_APPFLAG_COMPLEXDESC      0x01  // Complex Descriptor Available
#define AF_APPFLAG_USERDESC         0x02  // User Descriptor Available

typedef uint16  cId_t;
// Simple Description Format Structure
typedef struct
{
  byte          EndPoint;               //用户定义端点号(1~240)
  uint16        AppProfId;              //端点的簇ID(0x0000~0xFFFF)
  uint16        AppDeviceId;            //端点支持的设备ID(0x0000~0xFFFF)
  byte          AppDevVer:4;            //端点上设备执行的设备描述版本,用户定义
  byte          Reserved:4;             //保留
  byte          AppNumInClusters;       //端点支持的输入簇个数
  cId_t         *pAppInClusterList;     //指向输入簇列表的指针
  byte          AppNumOutClusters;      //端点支持的输出簇个数
  cId_t         *pAppOutClusterList;    //指向输出簇列表的指针
} SimpleDescriptionFormat_t;

/*********************************************************************
 * AF Message Format
 */

// Frame Types
#define FRAMETYPE_KVP          0x01     // 0001
#define FRAMETYPE_MSG          0x02     // 0010

#define ERRORCODE_SUCCESS               0x00

#define AF_HDR_KVP_MAX_LEN   0x08  // Max possible AF KVP header.
#define AF_HDR_V1_0_MAX_LEN  0x03  // Max possible AF Ver 1.0 header.
#define AF_HDR_V1_1_MAX_LEN  0x00  // Max possible AF Ver 1.1 header.

// Generalized MSG Command Format
typedef struct
{
  byte   TransSeqNumber;           //传输数据的序列号     
  uint16 DataLength;               //数据的长度
  byte  *Data;                     //应用层接收的用户数据
} afMSGCommandFormat_t;

typedef enum
{
  noLatencyReqs,
  fastBeacons,
  slowBeacons
} afNetworkLatencyReq_t;

/*********************************************************************
 * Endpoint  Descriptions
 */

typedef enum
{
  afAddrNotPresent = AddrNotPresent,  //用于设备的绑定或匹配      0
  afAddr16Bit      = Addr16Bit,       //数据发送以单播的形式发送  2
  afAddr64Bit      = Addr64Bit,       //数据发送以单播的形式发送  3  
  afAddrGroup      = AddrGroup,       //数据发送以组播的形式发送  1
  afAddrBroadcast  = AddrBroadcast    //数据发送以广播的形式发送  15
} afAddrMode_t;

typedef struct
{
  union
  {
    uint16      shortAddr;    //16位网络地址信息
    ZLongAddr_t extAddr;      //64位IEEE地址信息
  } addr;                     //目的节点的地址信息
  afAddrMode_t addrMode;      //枚举类型，数据发送形式[广播、组播、单播]
  byte endPoint;              //端点号
  uint16 panId;               //PANID信息
} afAddrType_t;


typedef struct
{
  osal_event_hdr_t hdr;     //OSAL消息事件
  uint16 groupId;           //消息的组ID
  uint16 clusterId;         //消息的蔟ID，这里可以判断是广播还是组播或单播
  afAddrType_t srcAddr;     //源节点地址信息
                               
  uint16 macDestAddr;       //MAC帧头的目的网络短地址
  uint8 endPoint;           //端点号
  uint8 wasBroadcast;       //是否是广播
  uint8 LinkQuality;        //接收数据帧的链路质量
  uint8 correlation;        //结构数据帧的原始相关值
  int8  rssi;               //RSSI信号强度指示，单位为dBm
  uint8 SecurityUse;        //数据安全设置
  uint32 timestamp;         //MAC层中收到的时间戳
  afMSGCommandFormat_t cmd; //应用层数据
} afIncomingMSGPacket_t;

typedef struct
{
  osal_event_hdr_t hdr;
  byte endpoint;
  byte transID;
} afDataConfirm_t;

// Endpoint Table - this table is the device description
// or application registration.
// There will be one entry in this table for every
// endpoint defined.
typedef struct
{
  byte endPoint;                          //端点号，用户定义，取值1~240
  byte *task_id;                          //指向任务ID的指针
  SimpleDescriptionFormat_t *simpleDesc;  //指向Zigbee端点简单描述符结构体
  afNetworkLatencyReq_t latencyReq;       //延时策略
} endPointDesc_t;

// Typedef for callback function to retrieve an endpoints
//   descriptors, contained in the endpoint descriptor.
//   This will allow an application to dynamically change
//   the descriptor and not use the RAM/ROM.
typedef void *(*pDescCB)( uint8 type, uint8 endpoint );

// Descriptor types used in the above callback
#define AF_DESCRIPTOR_SIMPLE            1
#define AF_DESCRIPTOR_PROFILE_ID        2

// Bit definitions for epList_t flags.
typedef enum
{
  eEP_AllowMatch = 1,
  eEP_NotUsed
} eEP_Flags;

typedef struct
{
  endPointDesc_t *epDesc;
  eEP_Flags flags;
  pDescCB  pfnDescCB;     // Don't use if this function pointer is NULL.
  void *nextDesc;
} epList_t;

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

#define afStatus_SUCCESS            ZSuccess
#define afStatus_FAILED             ZFailure
#define afStatus_MEM_FAIL           ZMemError
#define afStatus_INVALID_PARAMETER  ZInvalidParameter

typedef ZStatus_t afStatus_t;

typedef struct
{
  uint8              kvp;
  APSDE_DataReqMTU_t aps;
} afDataReqMTU_t;


/*********************************************************************
 * Globals
 */

extern epList_t *epList;

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * afInit - Initialize the AF.
  */
  extern void afInit( void );

 /*
  * afRegisterExtended - Register an Application's EndPoint description
  *           with a callback function for descriptors.
  *
  */
  extern epList_t *afRegisterExtended( endPointDesc_t *epDesc, pDescCB descFn );

 /*
  * afRegister - Register an Application's EndPoint description.
  *
  */
  extern afStatus_t afRegister( endPointDesc_t *epDesc );

 /*
  * afDataConfirm - APS will call this function after a data message
  *                 has been sent.
  */
  extern void afDataConfirm( uint8 endPoint, uint8 transID, ZStatus_t status );

 /*
  * afIncomingData - APS will call this function when an incoming
  *                   message is received.
  */
  extern void afIncomingData( aps_FrameFormat_t *aff, zAddrType_t *SrcAddress, uint16 SrcPanId,
                       NLDE_Signal_t *sig, byte SecurityUse, uint32 timestamp );

  afStatus_t AF_DataRequest( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                             uint16 cID, uint16 len, uint8 *buf, uint8 *transID,
                             uint8 options, uint8 radius );

  
/*********************************************************************
 * @fn      AF_DataRequestSrcRtg
 *
 * @brief   Common functionality for invoking APSDE_DataReq() for both
 *          SendMulti and MSG-Send.
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param  *srcEP - Origination (i.e. respond to or ack to) End Point Descr.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   len - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 * @param   options - Valid bit mask of Tx options.
 * @param   radius - Normally set to AF_DEFAULT_RADIUS.
 * @param   relayCnt - Number of devices in the relay list
 * @param   pRelayList - Pointer to the relay list
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */

afStatus_t AF_DataRequestSrcRtg( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                           uint16 cID, uint16 len, uint8 *buf, uint8 *transID,
                           uint8 options, uint8 radius, uint8 relayCnt, 
                           uint16* pRelayList );

/*********************************************************************
 * Direct Access Functions - ZigBee Device Object
 */

 /*
  *	afFindEndPointDesc - Find the endpoint description entry from the
  *                      endpoint number.
  */
  extern endPointDesc_t *afFindEndPointDesc( byte endPoint );

 /*
  *	afFindSimpleDesc - Find the Simple Descriptor from the endpoint number.
  *   	  If return value is not zero, the descriptor memory must be freed.
  */
  extern byte afFindSimpleDesc( SimpleDescriptionFormat_t **ppDesc, byte EP );
  
 /*
  *	afDataReqMTU - Get the Data Request MTU(Max Transport Unit)
  */
  extern uint8 afDataReqMTU( afDataReqMTU_t* fields );

 /*
  *	afGetMatch - Get the action for the Match Descriptor Response
  *             TRUE allow match descriptor response
  */
  extern uint8 afGetMatch( uint8 ep );

 /*
  *	afSetMatch - Set the action for the Match Descriptor Response
  *             TRUE allow match descriptor response
  */
  extern uint8 afSetMatch( uint8 ep, uint8 action );

 /*
  *	afNumEndPoints - returns the number of endpoints defined.
  */
  extern byte afNumEndPoints( void );

 /*
  *	afEndPoints - builds an array of endpoints.
  */
  extern void afEndPoints( byte *epBuf, byte skipZDO );

 /*
  *     afCopyAddress
  *
  */
extern void afCopyAddress (afAddrType_t *afAddr, zAddrType_t *zAddr);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
