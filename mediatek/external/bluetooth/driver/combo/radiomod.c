/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

/* use nvram */
#include "CFG_BT_File.h"
#include "CFG_BT_Default.h"
#include "CFG_file_lid.h"
#include "libnvram.h"

#include "bt_kal.h"


/**************************************************************************
 *                       D E F I N I T I O N S                            *
***************************************************************************/

#define BT_NVRAM_DATA_CLONE_FILE_NAME    "/data/BT_Addr"

typedef BtStatus (*HCI_CMD_FUNC_T)(void);
typedef BtStatus (*HCI_EVT_FUNC_T)(HCI_EVENT_T *);

typedef union {
  ap_nvram_btradio_struct fields;
  unsigned char raw[sizeof(ap_nvram_btradio_struct)];
} BT_NVRAM_DATA_T;

typedef struct {
  HCI_CMD_FUNC_T command_func;
} HCI_SEQ_T;

typedef struct {
  short tagidx;
  short rawidx;
  BT_NVRAM_DATA_T bt_nvram;
  HCI_SEQ_T *cur_script;
} BT_INIT_VAR_T;

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static HCI_CMD_T hciCommand;
static HANDLE  bt_com_port;
static BT_INIT_VAR_T btinit[1];
extern unsigned long g_chipId;

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static BtStatus GORMcmd_HCC_Set_Local_BD_Addr(void);
static BtStatus GORMcmd_HCC_Set_LinkKeyType(void);
static BtStatus GORMcmd_HCC_Set_UnitKey(void);
static BtStatus GORMcmd_HCC_Set_Encryption(void);
static BtStatus GORMcmd_HCC_Set_PinCodeType(void);
static BtStatus GORMcmd_HCC_Set_Voice(void);
static BtStatus GORMcmd_HCC_Set_PCM(void);
static BtStatus GORMcmd_HCC_Set_Radio(void);
static BtStatus GORMcmd_HCC_Set_Sleep_Timeout(void);
static BtStatus GORMcmd_HCC_Set_BT_FTR(void);
static BtStatus GORMcmd_HCC_Set_TX_Power_Offset(void);
static BtStatus GORMcmd_HCC_Coex_Performance_Adjust(void);


static BtStatus GORMcmd_HCC_Set_OSC_Info(void);
static BtStatus GORMcmd_HCC_Set_LPO_Info(void);
static BtStatus GORMcmd_HCC_Set_PTA(void);
static BtStatus GORMcmd_HCC_Set_BLEPTA(void);
static BtStatus GORMcmd_HCC_Set_RF_Desense(void);
static BtStatus GORMcmd_HCC_RESET(void);
static BtStatus GORMcmd_HCC_Set_Internal_PTA_1(void);
static BtStatus GORMcmd_HCC_Set_Internal_PTA_2(void);
static BtStatus GORMcmd_HCC_Set_Sleep_Control_Reg(void);
static BtStatus GORMcmd_HCC_Set_SLP_LDOD_VCTRL_Reg(void);
static BtStatus GORMcmd_HCC_Set_RF_Reg_100(void);


static BOOL BT_Get_Local_BD_Addr(UCHAR *);
static void GetRandomValue(UCHAR *);
static int WriteBDAddrToNvram(UCHAR *);


//===================================================================
// Combo chip
#ifdef MTK_MT6620
HCI_SEQ_T bt_init_script_6620[] = 
{    
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_LinkKeyType         }, /*0xFC1B*/
    {  GORMcmd_HCC_Set_UnitKey             }, /*0xFC75*/
    {  GORMcmd_HCC_Set_Encryption          }, /*0xFC76*/
    {  GORMcmd_HCC_Set_PinCodeType         }, /*0x0C0A*/
    {  GORMcmd_HCC_Set_Voice               }, /*0x0C26*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    /* MT6620 sleep mode doesn't support E1/E2  */
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Set_BT_FTR              }, /*0xFC7D*/
    {  GORMcmd_HCC_Set_OSC_Info            }, /*0xFC7B*/
    {  GORMcmd_HCC_Set_LPO_Info            }, /*0xFC7C*/
    {  GORMcmd_HCC_Set_PTA                 }, /*0xFC74*/
    {  GORMcmd_HCC_Set_BLEPTA              }, /*0xFCFC*/
    {  GORMcmd_HCC_Set_RF_Desense          }, /*0xFC20*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_Internal_PTA_1      }, /*0xFCFB*/
    {  GORMcmd_HCC_Set_Internal_PTA_2      }, /*0xFCFB*/
    {  GORMcmd_HCC_Set_Sleep_Control_Reg   }, /*0xFCD0*/
    {  GORMcmd_HCC_Set_SLP_LDOD_VCTRL_Reg  }, /*0xFCD0*/
    {  GORMcmd_HCC_Set_RF_Reg_100          }, /*0xFCB0*/
    {  0  },
};
#endif

#ifdef MTK_MT6628
HCI_SEQ_T bt_init_script_6628[] = 
{    
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_LinkKeyType         }, /*0xFC1B*/
    {  GORMcmd_HCC_Set_UnitKey             }, /*0xFC75*/
    {  GORMcmd_HCC_Set_Encryption          }, /*0xFC76*/
    {  GORMcmd_HCC_Set_PinCodeType         }, /*0x0C0A*/
    {  GORMcmd_HCC_Set_Voice               }, /*0x0C26*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Set_BT_FTR              }, /*0xFC7D*/
    {  GORMcmd_HCC_Set_OSC_Info            }, /*0xFC7B*/
    {  GORMcmd_HCC_Set_LPO_Info            }, /*0xFC7C*/
    {  GORMcmd_HCC_Set_PTA                 }, /*0xFC74*/
    {  GORMcmd_HCC_Set_BLEPTA              }, /*0xFCFC*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_Internal_PTA_1      }, /*0xFCFB*/
    {  GORMcmd_HCC_Set_Internal_PTA_2      }, /*0xFCFB*/
    {  GORMcmd_HCC_Set_RF_Reg_100          }, /*0xFCB0*/
    {  0  },
};
#endif

#if defined(MTK_CONSYS_MT6572) || defined(MTK_CONSYS_MT6582) || \
defined(MTK_CONSYS_MT6592) || defined(MTK_CONSYS_MT6571)
HCI_SEQ_T bt_init_script_consys[] = 
{    
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  0  },
};
#endif

#ifdef MTK_MT6630
HCI_SEQ_T bt_init_script_6630[] = 
{    
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Coex_Performance_Adjust }, /*0xFC22*/
    {  0  },
};
#endif

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static BtStatus GORMcmd_HCC_Set_Local_BD_Addr(void)
{
    unsigned char ucDefaultAddr[6] = {0};
    
    hciCommand.opCode = 0xFC1A;
    hciCommand.len = 6;
    
    LOG_DBG("GORMcmd_HCC_Set_Local_BD_Addr\n");
    
    switch(g_chipId)
    {
      case 0x6620:
        memcpy(ucDefaultAddr, stBtDefault_6620.addr, 6);
        break;
      case 0x6628:
        memcpy(ucDefaultAddr, stBtDefault_6628.addr, 6);
        break;
      case 0x6572:
        memcpy(ucDefaultAddr, stBtDefault_6572.addr, 6);
        break;
      case 0x6582:
        memcpy(ucDefaultAddr, stBtDefault_6582.addr, 6);
        break;
      case 0x6592:
        memcpy(ucDefaultAddr, stBtDefault_6592.addr, 6);
        break;
      case 0x6571:
        memcpy(ucDefaultAddr, stBtDefault_6571.addr, 6);
        break;
      case 0x6630:
        memcpy(ucDefaultAddr, stBtDefault_6630.addr, 6);
        break;
      default:
        LOG_ERR("Unknown combo chip id\n");
        goto Set_BD_Addr;
    }
    
    if (0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6))
    {
        LOG_WAN("Nvram BD address default value\n");
        
        /* Want to retrieve module eFUSE address on combo chip */
        BT_Get_Local_BD_Addr(btinit->bt_nvram.fields.addr);
        
        if (0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)){
            LOG_WAN("eFUSE address default value\n");
            #ifdef BD_ADDR_AUTOGEN 
            GetRandomValue(btinit->bt_nvram.fields.addr);
            #endif
        }
        else {
            LOG_WAN("eFUSE address has valid value\n");
        }
        
        // Save BD address to Nvram and /data/BD_Addr
        WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
    }
    else {
        LOG_WAN("Nvram BD address has valid value\n");
    }

Set_BD_Addr:
    hciCommand.parms[0] = btinit->bt_nvram.fields.addr[5];
    hciCommand.parms[1] = btinit->bt_nvram.fields.addr[4];
    hciCommand.parms[2] = btinit->bt_nvram.fields.addr[3];
    hciCommand.parms[3] = btinit->bt_nvram.fields.addr[2];
    hciCommand.parms[4] = btinit->bt_nvram.fields.addr[1];
    hciCommand.parms[5] = btinit->bt_nvram.fields.addr[0];
    
    LOG_WAN("Write BD address: %02x-%02x-%02x-%02x-%02x-%02x\n",
        hciCommand.parms[5], hciCommand.parms[4], hciCommand.parms[3],
        hciCommand.parms[2], hciCommand.parms[1], hciCommand.parms[0]);
    
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

#if defined(MTK_MT6620) || defined(MTK_MT6628)
static BtStatus GORMcmd_HCC_Set_LinkKeyType(void)
{
    hciCommand.opCode = 0xFC1B;
    hciCommand.len = 1;
    hciCommand.parms[0] = 0x01;	  /*00: unit key; 01: combination key*/
    
    LOG_DBG("GORMcmd_HCC_Set_LinkKeyType\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_UnitKey(void)
{
    hciCommand.opCode = 0xFC75;
    hciCommand.len = 16;
    
    hciCommand.parms[0] = 0x00;
    hciCommand.parms[1] = 0x00;
    hciCommand.parms[2] = 0x00;
    hciCommand.parms[3] = 0x00;
    hciCommand.parms[4] = 0x00;
    hciCommand.parms[5] = 0x00;
    hciCommand.parms[6] = 0x00;
    hciCommand.parms[7] = 0x00;
    hciCommand.parms[8] = 0x00;
    hciCommand.parms[9] = 0x00;
    hciCommand.parms[10] = 0x00;
    hciCommand.parms[11] = 0x00;
    hciCommand.parms[12] = 0x00;
    hciCommand.parms[13] = 0x00;
    hciCommand.parms[14] = 0x00;
    hciCommand.parms[15] = 0x00;
    
    LOG_DBG("GORMcmd_HCC_Set_UnitKey\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_Encryption(void)
{
    hciCommand.opCode = 0xFC76;
    hciCommand.len = 3;
    
    hciCommand.parms[0] = 0x00;
    hciCommand.parms[1] = 0x02;
    hciCommand.parms[2] = 0x10;
    
    LOG_DBG("GORMcmd_HCC_Set_Encryption\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_PinCodeType(void)
{
    hciCommand.opCode = 0x0C0A;
    hciCommand.len = 1;
    hciCommand.parms[0] = 0x00;	  /*00: variable PIN; 01: Fixed PIN*/
    
    LOG_DBG("GORMcmd_HCC_Set_PinCodeType\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_Voice(void)
{
    hciCommand.opCode = 0x0C26;
    hciCommand.len = 2;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.Voice[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.Voice[1];
    
    LOG_DBG("GORMcmd_HCC_Set_Voice\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif


static BtStatus GORMcmd_HCC_Set_PCM(void)
{
    hciCommand.opCode = 0xFC72;
    hciCommand.len = 4;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.Codec[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.Codec[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.Codec[2];
    hciCommand.parms[3] = btinit->bt_nvram.fields.Codec[3];	
    
    LOG_DBG("GORMcmd_HCC_Set_PCM\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_Radio(void)
{
    hciCommand.opCode = 0xFC79;
    hciCommand.len = 6;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.Radio[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.Radio[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.Radio[2];
    hciCommand.parms[3] = btinit->bt_nvram.fields.Radio[3];
    hciCommand.parms[4] = btinit->bt_nvram.fields.Radio[4];
    hciCommand.parms[5] = btinit->bt_nvram.fields.Radio[5];
    
    LOG_DBG("GORMcmd_HCC_Set_Radio\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_TX_Power_Offset(void)
{
    hciCommand.opCode = 0xFC93;
    hciCommand.len = 3;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.TxPWOffset[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.TxPWOffset[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.TxPWOffset[2];
    
    LOG_DBG("GORMcmd_HCC_Set_TX_Power_Offset\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_Sleep_Timeout(void)
{
    hciCommand.opCode = 0xFC7A;
    hciCommand.len = 7;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.Sleep[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.Sleep[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.Sleep[2];
    hciCommand.parms[3] = btinit->bt_nvram.fields.Sleep[3];
    hciCommand.parms[4] = btinit->bt_nvram.fields.Sleep[4];
    hciCommand.parms[5] = btinit->bt_nvram.fields.Sleep[5];
    hciCommand.parms[6] = btinit->bt_nvram.fields.Sleep[6];
    
    LOG_DBG("GORMcmd_HCC_Set_Sleep_Timeout\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

#if defined(MTK_MT6630)
static BtStatus GORMcmd_HCC_Coex_Performance_Adjust(void)
{
    hciCommand.opCode = 0xFC22;
    hciCommand.len = 6;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.CoexAdjust[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.CoexAdjust[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.CoexAdjust[2];
    hciCommand.parms[3] = btinit->bt_nvram.fields.CoexAdjust[3];
    hciCommand.parms[4] = btinit->bt_nvram.fields.CoexAdjust[4];
    hciCommand.parms[5] = btinit->bt_nvram.fields.CoexAdjust[5];
    
    LOG_DBG("GORMcmd_HCC_Coex_Performance_Adjust\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif

#if defined(MTK_MT6620) || defined(MTK_MT6628)
static BtStatus GORMcmd_HCC_Set_BT_FTR(void)
{
    hciCommand.opCode = 0xFC7D;
    hciCommand.len = 2;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.BtFTR[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.BtFTR[1];
    
    LOG_DBG("GORMcmd_HCC_Set_BT_FTR\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_OSC_Info(void)
{
    hciCommand.opCode = 0xFC7B;
    hciCommand.len = 5;
    
    hciCommand.parms[0] = 0x01;     /* do not care on 6620 */
    hciCommand.parms[1] = 0x01;     /* do not care on 6620 */
    hciCommand.parms[2] = 0x14;     /* clock drift */
    hciCommand.parms[3] = 0x0A;     /* clock jitter */
    
    if(g_chipId == 0x6620){
        hciCommand.parms[4] = 0x05; /* OSC stable time */
    }
    
    if(g_chipId == 0x6628){
        hciCommand.parms[4] = 0x08; /* OSC stable time */
    }
    
    LOG_DBG("GORMcmd_HCC_Set_OSC_Info\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_LPO_Info(void)
{
    hciCommand.opCode = 0xFC7C;
    hciCommand.len = 10;
    
    hciCommand.parms[0] = 0x01;     /* LPO source = external */
    hciCommand.parms[1] = 0xFA;     /* LPO clock drift = 250ppm */
    hciCommand.parms[2] = 0x0A;     /* LPO clock jitter = 10us */
    hciCommand.parms[3] = 0x02;     /* LPO calibration mode = manual mode */
    hciCommand.parms[4] = 0x00;     /* LPO calibration interval = 10 mins */
    hciCommand.parms[5] = 0xA6;
    hciCommand.parms[6] = 0x0E;
    hciCommand.parms[7] = 0x00;
    hciCommand.parms[8] = 0x40;     /* LPO calibration cycles = 64 */
    hciCommand.parms[9] = 0x00;
    
    LOG_DBG("GORMcmd_HCC_Set_LPO_Info\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_PTA(void)
{
    hciCommand.opCode = 0xFC74;
    hciCommand.len = 10;
    
    hciCommand.parms[0] = 0xC9;
    hciCommand.parms[1] = 0x8B;
    hciCommand.parms[2] = 0xBF;
    hciCommand.parms[3] = 0x00;
    hciCommand.parms[4] = 0x00;
    hciCommand.parms[5] = 0x52;
    hciCommand.parms[6] = 0x0E;
    hciCommand.parms[7] = 0x0E;
    hciCommand.parms[8] = 0x1F;
    hciCommand.parms[9] = 0x1B;
    
    LOG_DBG("GORMcmd_HCC_Set_PTA\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_BLEPTA(void)
{
    hciCommand.opCode = 0xFCFC;
    hciCommand.len = 5;
    
    hciCommand.parms[0] = 0x16;	
    hciCommand.parms[1] = 0x0E;	
    hciCommand.parms[2] = 0x0E;	
    hciCommand.parms[3] = 0x00; 
    hciCommand.parms[4] = 0x07; 
    
    LOG_DBG("GORMcmd_HCC_Set_BLEPTA\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

#ifdef MTK_MT6620
static BtStatus GORMcmd_HCC_Set_RF_Desense(void)
{
    hciCommand.opCode = 0xFC20;
    hciCommand.len = 6;
    
    hciCommand.parms[0] = 0x01;    /* DWA is off at Channel 30, others still ON */
    hciCommand.parms[1] = 0x00;    /* Turn Off RX Current Boost Function */
    hciCommand.parms[2] = 0x00;    /* Turn Off 0.8889 interface switch Function */
    hciCommand.parms[3] = 0x00;    /* Turn Off MPLL Hopping Function */
    hciCommand.parms[4] = 0x00;    /* Turn Off DC_Notch Function */
    hciCommand.parms[5] = 0x00;    /* Turn Off EMI_Hopping Function */
    
    LOG_DBG("GORMcmd_HCC_Set_RF_Desense\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif
#endif

static BtStatus GORMcmd_HCC_RESET(void)
{
    hciCommand.opCode = 0x0C03;
    hciCommand.len = 0;
    
    LOG_DBG("GORMcmd_HCC_RESET\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

#if defined(MTK_MT6620) || defined(MTK_MT6628)
static BtStatus GORMcmd_HCC_Set_Internal_PTA_1(void)
{
    hciCommand.opCode = 0xFCFB;
    hciCommand.len = 15;
    
    hciCommand.parms[0] = 0x00;
    hciCommand.parms[1] = 0x01;	    /* bt_pta_high_level_tx */
    hciCommand.parms[2] = 0x0F;	    /* bt_pta_mid_level_tx */
    hciCommand.parms[3] = 0x0F;	    /* bt_pta_low_level_tx */
    hciCommand.parms[4] = 0x01;	    /* bt_pta_high_level_rx */
    hciCommand.parms[5] = 0x0F;	    /* bt_pta_mid_level_rx */
    hciCommand.parms[6] = 0x0F;	    /* bt_pta_low_level_rx */
    hciCommand.parms[7] = 0x01;	    /* ble_pta_high_level_tx */
    hciCommand.parms[8] = 0x0F;	    /* ble_pta_mid_level_tx */
    hciCommand.parms[9] = 0x0F;	    /* ble_pta_low_level_tx */
    hciCommand.parms[10] = 0x01;    /* ble_pta_high_level_rx */
    hciCommand.parms[11] = 0x0F;    /* ble_pta_mid_level_rx */
    hciCommand.parms[12] = 0x0F;    /* ble_pta_low_level_rx */
    hciCommand.parms[13] = 0x02;    /* time_r2g */
    hciCommand.parms[14] = 0x01;    /* always  0x1 */
    
    LOG_DBG("GORMcmd_HCC_Set_Internal_PTA_1\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_Internal_PTA_2(void)
{
    hciCommand.opCode = 0xFCFB;
    hciCommand.len = 7;
     
    hciCommand.parms[0] = 0x01;		
    hciCommand.parms[1] = 0x19;     /* wifi20_hb */
    hciCommand.parms[2] = 0x19;     /* wifi20_hb */
    hciCommand.parms[3] = 0x07;     /* next_rssi_update_bt_slots */
    hciCommand.parms[4] = 0xD0;     /* next_rssi_update_bt_slots */
    hciCommand.parms[5] = 0x00;     /* stream_identify_by_host */
    hciCommand.parms[6] = 0x01;     /* enable auto AFH */
    
    LOG_DBG("GORMcmd_HCC_Set_Internal_PTA_2\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

#ifdef MTK_MT6620
static BtStatus GORMcmd_HCC_Set_Sleep_Control_Reg(void)
{
    DWORD addr, val;
    
    hciCommand.opCode = 0xFCD0;
    hciCommand.len = 8;
    
    addr = 0x81010074;
    val = 0x000029E2;
    hciCommand.parms[0] = (UCHAR)((addr) & 0xFF);		
    hciCommand.parms[1] = (UCHAR)((addr >> 8) & 0xFF);		
    hciCommand.parms[2] = (UCHAR)((addr >> 16) & 0xFF);
    hciCommand.parms[3] = (UCHAR)((addr >> 24) & 0xFF);		
    hciCommand.parms[4] = (UCHAR)((val) & 0xFF);		
    hciCommand.parms[5] = (UCHAR)((val >> 8) & 0xFF);		
    hciCommand.parms[6] = (UCHAR)((val >> 16) & 0xFF);
    hciCommand.parms[7] = (UCHAR)((val >> 24) & 0xFF);
    
    LOG_DBG("GORMcmd_HCC_Set_Sleep_Control_Reg\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_SLP_LDOD_VCTRL_Reg(void)
{
    DWORD addr, val;
    
    hciCommand.opCode = 0xFCD0;
    hciCommand.len = 8;
    
    addr = 0x8102001C;
    val = 0x00000879;
    hciCommand.parms[0] = (UCHAR)((addr) & 0xFF);		
    hciCommand.parms[1] = (UCHAR)((addr >> 8) & 0xFF);		
    hciCommand.parms[2] = (UCHAR)((addr >> 16) & 0xFF);
    hciCommand.parms[3] = (UCHAR)((addr >> 24) & 0xFF);		
    hciCommand.parms[4] = (UCHAR)((val) & 0xFF);		
    hciCommand.parms[5] = (UCHAR)((val >> 8) & 0xFF);		
    hciCommand.parms[6] = (UCHAR)((val >> 16) & 0xFF);
    hciCommand.parms[7] = (UCHAR)((val >> 24) & 0xFF);
    
    LOG_DBG("GORMcmd_HCC_Set_SLP_LDOD_VCTRL_Reg\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif

static BtStatus GORMcmd_HCC_Set_RF_Reg_100(void)
{
    hciCommand.opCode = 0xFCB0;
    hciCommand.len = 6;
    
    hciCommand.parms[0] = 0x64;	
    hciCommand.parms[1] = 0x01;
    hciCommand.parms[2] = 0x02;
    hciCommand.parms[3] = 0x00;
    hciCommand.parms[4] = 0x00;
    hciCommand.parms[5] = 0x00;
    
    LOG_DBG("GORMcmd_HCC_Set_RF_Reg_100\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif


DWORD GORM_Init(
    HANDLE  hComPort,
    PUCHAR  pucPatchExtData,
    DWORD   dwPatchExtLen,
    PUCHAR  pucPatchData,
    DWORD   dwPatchLen,
    PBYTE   ucNvRamData,
    DWORD   dwBaud,
    DWORD   dwHostBaud,
    DWORD   dwFlowControl
    )
{
    int      i = 0;
    BtStatus btStatus = BT_STATUS_SUCCESS;
    UCHAR    ucEventBuf[MAX_EVENT_SIZE];
    DWORD    dwEventLen;
    
    LOG_DBG("GORM_Init\n");
    
    // Save com port fd for GORMcmd use
    bt_com_port = hComPort;
    
    // Copy Nvram data
    memcpy(btinit->bt_nvram.raw, ucNvRamData, sizeof(ap_nvram_btradio_struct));
    
    // General init script
    switch(g_chipId)
    {
#ifdef MTK_MT6620
      case 0x6620:
        btinit->cur_script = bt_init_script_6620;
        break;
#endif
#ifdef MTK_MT6628
      case 0x6628:
        btinit->cur_script = bt_init_script_6628;
        break;
#endif
#ifdef MTK_CONSYS_MT6572
      case 0x6572:
        btinit->cur_script = bt_init_script_consys;
        break;
#endif
#ifdef MTK_CONSYS_MT6582
      case 0x6582:
        btinit->cur_script = bt_init_script_consys;
        break;
#endif
#ifdef MTK_CONSYS_MT6592
      case 0x6592:
        btinit->cur_script = bt_init_script_consys;
        break;
#endif
#ifdef MTK_CONSYS_MT6571
      case 0x6571:
        btinit->cur_script = bt_init_script_consys;
        break;
#endif
#ifdef MTK_MT6630
      case 0x6630:
        btinit->cur_script = bt_init_script_6630;
        break;
#endif
      default:
        LOG_ERR("Unknown combo chip id\n");
        break;
    }

    /* Can not find matching script, simply skip */
    if((btinit->cur_script) == NULL){
        LOG_ERR("No matching init script\n");
        btStatus = BT_STATUS_FAILED;
        return btStatus;
    }
    
    i = 0;
    while(btinit->cur_script[i].command_func){
        
        btStatus = btinit->cur_script[i].command_func();
        
        if(btStatus == BT_STATUS_CANCELLED){
            i ++;
            continue;//skip
        }
        
        if(btStatus == BT_STATUS_FAILED){
            LOG_ERR("Command %d fails\n", i);
            return btStatus;
        }
        
        if(BT_ReadExpectedEvent(
            hComPort, 
            ucEventBuf, 
            MAX_EVENT_SIZE, 
            0x0E, 
            &dwEventLen, 
            TRUE, 
            hciCommand.opCode, 
            TRUE, 
            0x00) == FALSE){
            
            LOG_ERR("Read event of command %d fails\n", i);
            btStatus = BT_STATUS_FAILED;
            return btStatus;
        }
        
        i ++;
    }
    
    return btStatus;
}


static BOOL BT_Get_Local_BD_Addr(UCHAR *pBDAddr)
{
    LOG_DBG("BT_Get_Local_BD_Addr\n");
    
    HCI_CMD_T hciCmd;
    DWORD dwReadLen = 0;
    UCHAR pAckEvent[20];
    
    hciCmd.opCode = 0x1009;
    hciCmd.len = 0;
    
    if(BT_SendHciCommand(bt_com_port, &hciCmd) == FALSE){
        LOG_ERR("Write get BD address command fails\n");
        return FALSE;
    }
    
    // Read local BD addr in firmware
    if(BT_ReadExpectedEvent(
        bt_com_port, 
        pAckEvent, 
        sizeof(pAckEvent),
        0x0E,
        &dwReadLen,
        TRUE,
        0x1009,
        TRUE,
        0x0) == FALSE){
        
        LOG_ERR("Read local BD address fails\n");
        return FALSE;
    }
    
    LOG_WAN("Local BD address: %02x-%02x-%02x-%02x-%02x-%02x\n",
        pAckEvent[12], pAckEvent[11], pAckEvent[10], pAckEvent[9], pAckEvent[8], pAckEvent[7]);
    
    pBDAddr[0] = pAckEvent[12];
    pBDAddr[1] = pAckEvent[11];
    pBDAddr[2] = pAckEvent[10];
    pBDAddr[3] = pAckEvent[9];
    pBDAddr[4] = pAckEvent[8];
    pBDAddr[5] = pAckEvent[7];
    
    return TRUE;
}

static void GetRandomValue(UCHAR string[6])
{
    int iRandom = 0;
    int fd = 0;
    unsigned long seed;
    
    LOG_WAN("Enable random generation\n");
    
    /* initialize random seed */
    srand (time(NULL));
    iRandom = rand();
    LOG_WAN("iRandom = [%d]", iRandom);
    string[0] = (((iRandom>>24|iRandom>>16) & (0xFE)) | (0x02)); /* must use private bit(1) and no BCMC bit 0 */
    
    /* second seed */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand (tv.tv_usec);
    iRandom = rand();
    LOG_WAN("iRandom = [%d]", iRandom);
    string[1] = ((iRandom>>8) & 0xFF);
    
    /* third seed */
    fd = open("/dev/urandom", O_RDONLY);
    if (fd > 0){
        if (read(fd, &seed, sizeof(unsigned long)) > 0){
            srand (seed);
            iRandom = rand();
        }
        close(fd);
    }
    
    LOG_WAN("iRandom = [%d]", iRandom);
    string[5] = (iRandom & 0xFF);
    
    return;
}

static int WriteBDAddrToNvram(UCHAR *pBDAddr)
{
    F_ID bt_nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    int bt_cfgfile_fd = -1;
    
    bt_cfgfile_fd = open(BT_NVRAM_DATA_CLONE_FILE_NAME, O_WRONLY);
    if(bt_cfgfile_fd < 0){
        LOG_ERR("Can't open config file %s\n", BT_NVRAM_DATA_CLONE_FILE_NAME);
    }
    else{
        lseek(bt_cfgfile_fd, 0, 0);
        write(bt_cfgfile_fd, pBDAddr, 6);
        close(bt_cfgfile_fd);
    }
    
    bt_nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
    if(bt_nvram_fd.iFileDesc < 0){
        LOG_ERR("Open BT NVRAM fails errno %d\n", errno);
        return -1;
    }
    
    if(rec_num != 1){
        LOG_ERR("Unexpected record num %d\n", rec_num);
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }
    
    if(rec_size != sizeof(ap_nvram_btradio_struct)){
        LOG_ERR("Unexpected record size %d ap_nvram_btradio_struct %d\n",
                 rec_size, sizeof(ap_nvram_btradio_struct));
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }
    
    lseek(bt_nvram_fd.iFileDesc, 0, 0);
    
    /* Update BD address */
    if (write(bt_nvram_fd.iFileDesc, pBDAddr, 6) < 0){
        LOG_ERR("Write BT NVRAM fails errno %d\n", errno);
        NVM_CloseFileDesc(bt_nvram_fd);
        return -1;
    }
    
    NVM_CloseFileDesc(bt_nvram_fd);
    return  0;
}
