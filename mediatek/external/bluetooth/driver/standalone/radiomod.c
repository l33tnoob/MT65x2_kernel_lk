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

//--------------------------------6622------------------------------------                                     
#if defined MTK_MT6622
#if BT_PATCH_EXT_ENABLE
static PATCH_SETTING_T patch_setting = {0x0, 0x0, 0x0, 
                                      0x0, 0x0}; //patch related setting if MT6622 also need patch ext RAM
#endif
//--------------------------------6626------------------------------------
#elif defined MTK_MT6626
#if BT_PATCH_EXT_ENABLE
static PATCH_SETTING_T patch_setting = {0x0, 0x0, 0x0, 
                                      0x0, 0x0}; //patch related setting if MT6626 also need patch ext RAM
#endif
#endif

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static BtStatus GORMcmd_HCC_Set_Local_BD_Addr(void);
static BtStatus GORMcmd_HCC_Set_CapID(void);
static BtStatus GORMcmd_HCC_Set_LinkKeyType(void);
static BtStatus GORMcmd_HCC_Set_UnitKey(void);
static BtStatus GORMcmd_HCC_Set_Encryption(void);
static BtStatus GORMcmd_HCC_Set_PinCodeType(void);
static BtStatus GORMcmd_HCC_Set_Voice(void);
static BtStatus GORMcmd_HCC_Set_PCM(void);
static BtStatus GORMcmd_HCC_Set_Radio(void);
static BtStatus GORMcmd_HCC_Set_Sleep_Timeout(void);
static BtStatus GORMcmd_HCC_Set_BT_FTR(void);
static BtStatus GORMcmd_HCC_Set_ECLK(void);


static BtStatus GORMcmd_HCC_WakeUpChip(void);
static BtStatus GORMcmd_HCC_Read_Local_Version(void);
static BtStatus GORMcmd_HCC_Simulate_MT6612(void);
static BtStatus GORMcmd_HCC_Fix_UART_Escape(void);
static BtStatus GORMcmd_HCC_GetHwVersion(void);
static BtStatus GORMcmd_HCC_GetGormVersion(void);
static BtStatus GORMcmd_HCC_ChangeBaudRate(void);
#if BT_PATCH_EXT_ENABLE
static BtStatus GORMcmd_HCC_Set_Patch_Base_Ext(void);
static BtStatus GORMcmd_HCC_Set_Patch_Base(void);
static BtStatus GORMcmd_HCC_Reset_Patch_Len(void);
static BtStatus GORMcmd_HCC_WritePatch_ext(void);
#endif
static BtStatus GORMcmd_HCC_WritePatch(void);
static BtStatus GORMcmd_HCC_Set_Chip_Feature(void);
static BtStatus GORMcmd_HCC_Set_OSC_Info(void);
static BtStatus GORMcmd_HCC_Set_LPO_Info(void);
static BtStatus GORMcmd_HCC_Set_RF_Desense(void);
static BtStatus GORMcmd_HCC_Set_PTA(void);
static BtStatus GORMcmd_HCC_RESET(void);
static BtStatus GORMcmd_HCC_Enable_PTA(void);
static BtStatus GORMcmd_HCC_Set_WiFi_Ch(void);
static BtStatus GORMcmd_HCC_Set_AFH_Mask(void);
static BtStatus GORMcmd_HCC_Set_Sleep_Control_Reg(void);
static BtStatus GORMcmd_HCC_I2S_Switch(void);
static BtStatus GORMcmd_HCC_JTAG_PCM_Switch(void);


static BtStatus GORMcmd_HCC_Set_FW_Reg(DWORD, DWORD);


static void GetRandomValue(UCHAR *);
static int WriteBDAddrToNvram(UCHAR *);


//===================================================================
// Standalone chip, MT661x is phased out
HCI_SEQ_T bt_init_script[] = 
{
#ifdef MTK_MT6626
    {  GORMcmd_HCC_WakeUpChip              }, /*0xFF*/
#endif
    {  GORMcmd_HCC_Read_Local_Version      }, /*0x1001*/
#ifdef MTK_MT6622
    {  GORMcmd_HCC_Simulate_MT6612         }, /*0xFCCC*/
#endif
    {  GORMcmd_HCC_Fix_UART_Escape         }, /*0xFCD0*/
    {  GORMcmd_HCC_GetHwVersion            }, /*0xFCD1*/
    {  GORMcmd_HCC_GetGormVersion          }, /*0xFCD1*/
    {  GORMcmd_HCC_ChangeBaudRate          }, /*0xFC77*/
    {  GORMcmd_HCC_WritePatch              },
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
#ifdef MTK_MT6622
    {  GORMcmd_HCC_Set_CapID               }, /*0xFC7F*/
#endif
    {  GORMcmd_HCC_Set_LinkKeyType         }, /*0xFC1B*/
    {  GORMcmd_HCC_Set_UnitKey             }, /*0xFC75*/
    {  GORMcmd_HCC_Set_Encryption          }, /*0xFC76*/
    {  GORMcmd_HCC_Set_PinCodeType         }, /*0x0C0A*/
    {  GORMcmd_HCC_Set_Voice               }, /*0x0C26*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Set_BT_FTR              }, /*0xFC7D*/
#ifdef MTK_MT6626
    {  GORMcmd_HCC_Set_ECLK                }, /*0xFCD0*/
#endif
    {  GORMcmd_HCC_Set_Chip_Feature        }, /*0xFC1E*/
    {  GORMcmd_HCC_Set_OSC_Info            }, /*0xFC7B*/
    {  GORMcmd_HCC_Set_LPO_Info            }, /*0xFC7C*/
    {  GORMcmd_HCC_Set_RF_Desense          }, /*0xFC20*/
    {  GORMcmd_HCC_Set_PTA                 }, /*0xFC74*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
#ifdef MTK_MT6626
    {  GORMcmd_HCC_Set_CapID               }, /*0xFC1B*/
#endif
    {  GORMcmd_HCC_Enable_PTA              }, /*0xFCD2*/
    {  GORMcmd_HCC_Set_WiFi_Ch             }, /*0xFCD3*/
    {  GORMcmd_HCC_Set_AFH_Mask            }, /*0x0C3F*/
    {  GORMcmd_HCC_Set_Sleep_Control_Reg   }, /*0xFCD0*/
#ifdef MTK_MT6626
    {  GORMcmd_HCC_I2S_Switch              }, /*0xFC85*/
    {  GORMcmd_HCC_JTAG_PCM_Switch         }, /*0xFCD6*/
#endif
    {  0  },
};
#endif

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static BtStatus GORMcmd_HCC_Set_Local_BD_Addr(void)
{
    hciCommand.opCode = 0xFC1A;
    hciCommand.len = 6;
    
    LOG_DBG("GORMcmd_HCC_Set_Local_BD_Addr\n");
    
    if (0 == memcmp(btinit->bt_nvram.fields.addr, stBtDefault.addr, 6))
    {
        LOG_WAN("Nvram BD address default value\n");
        /* Standalone chip has no eFUSE address */
        #ifdef BD_ADDR_AUTOGEN
        GetRandomValue(btinit->bt_nvram.fields.addr);
        #endif
        
        // Save BD address to Nvram and /data/BD_Addr
        WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
    }
    else {
        LOG_WAN("Nvram BD address has valid value\n");
    }
    
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

static BtStatus GORMcmd_HCC_Set_CapID(void)
{
    hciCommand.opCode = 0xFC7F;
    hciCommand.len = 1;
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.CapId[0];
#ifdef MTK_MT6626
    if(0xFF == btinit->bt_nvram.fields.CapId[0]){
        hciCommand.parms[0] = 0x40;
    }
#endif

    LOG_DBG("GORMcmd_HCC_Set_CapID\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_LinkKeyType(void)
{
    hciCommand.opCode = 0xFC1B;
    hciCommand.len = 1;
    hciCommand.parms[0] = btinit->bt_nvram.fields.LinkKeyType[0];
    
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
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.UintKey[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.UintKey[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.UintKey[2];
    hciCommand.parms[3] = btinit->bt_nvram.fields.UintKey[3];
    hciCommand.parms[4] = btinit->bt_nvram.fields.UintKey[4];
    hciCommand.parms[5] = btinit->bt_nvram.fields.UintKey[5];
    hciCommand.parms[6] = btinit->bt_nvram.fields.UintKey[6];
    hciCommand.parms[7] = btinit->bt_nvram.fields.UintKey[7];
    hciCommand.parms[8] = btinit->bt_nvram.fields.UintKey[8];
    hciCommand.parms[9] = btinit->bt_nvram.fields.UintKey[9];
    hciCommand.parms[10] = btinit->bt_nvram.fields.UintKey[10];
    hciCommand.parms[11] = btinit->bt_nvram.fields.UintKey[11];
    hciCommand.parms[12] = btinit->bt_nvram.fields.UintKey[12];
    hciCommand.parms[13] = btinit->bt_nvram.fields.UintKey[13];
    hciCommand.parms[14] = btinit->bt_nvram.fields.UintKey[14];
    hciCommand.parms[15] = btinit->bt_nvram.fields.UintKey[15];
    
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
    
    hciCommand.parms[0] = btinit->bt_nvram.fields.Encryption[0];
    hciCommand.parms[1] = btinit->bt_nvram.fields.Encryption[1];
    hciCommand.parms[2] = btinit->bt_nvram.fields.Encryption[2];
    
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
    hciCommand.parms[0] = btinit->bt_nvram.fields.PinCodeType[0];
    
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

#ifdef MTK_MT6626
static BtStatus GORMcmd_HCC_Set_ECLK(void)
{
    hciCommand.opCode = 0xFCD0;
    hciCommand.len = 8;
    
    hciCommand.parms[0] = 0x04;
    hciCommand.parms[1] = 0x07;
    hciCommand.parms[2] = 0x00;
    hciCommand.parms[3] = 0x80;
    hciCommand.parms[4] = ((btinit->bt_nvram.raw[52] & 0x1) << 0x1) | 0x1;   /*(nvram value ECLK_SEL & 0x1<<0x1)|0x1*/
    hciCommand.parms[5] = 0x00;
    hciCommand.parms[6] = 0x00;
    hciCommand.parms[7] = 0x00;
    
    LOG_DBG("GORMcmd_HCC_Set_ECLK [%02X]\n", hciCommand.parms[4]);
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif

/* Implement in BT_WakeMagic */
static BtStatus GORMcmd_HCC_WakeUpChip(void)
{
    return BT_STATUS_NOT_SUPPORTED;
}

static BtStatus GORMcmd_HCC_Read_Local_Version(void)
{
    hciCommand.opCode = 0x1001;
    hciCommand.len = 0;
    
    LOG_DBG("GORMcmd_HCC_Read_Local_Version\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

#ifdef MTK_MT6622
static BtStatus GORMcmd_HCC_Simulate_MT6612(void)
{
    hciCommand.opCode = 0xFCCC;
    hciCommand.len = 1;
    hciCommand.parms[0] = 0x00; //disable
    
    LOG_DBG("GORMcmd_HCC_Simulate_MT6612\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif

static BtStatus GORMcmd_HCC_Fix_UART_Escape(void)
{
    hciCommand.opCode = 0xFCD0;
    hciCommand.len = 8;
    
    hciCommand.parms[0] = 0x40;
    hciCommand.parms[1] = 0x00;
    hciCommand.parms[2] = 0x06;
    hciCommand.parms[3] = 0x80;
    hciCommand.parms[4] = 0x77;
    hciCommand.parms[5] = 0x00;
    hciCommand.parms[6] = 0x00;
    hciCommand.parms[7] = 0x00;
    
    LOG_DBG("GORMcmd_HCC_Fix_UART_Escape\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_GetHwVersion(void)
{
    hciCommand.opCode = 0xFCD1;
    hciCommand.len = 4;
    
    hciCommand.parms[0] = 0x00;
    hciCommand.parms[1] = 0x00;
    hciCommand.parms[2] = 0x00;
    hciCommand.parms[3] = 0x80;
    
    LOG_DBG("GORMcmd_HCC_GetHwVersion\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_GetGormVersion(void)
{
    hciCommand.opCode = 0xFCD1;
    hciCommand.len = 4;
    
    hciCommand.parms[0] = 0x04;
    hciCommand.parms[1] = 0x00;
    hciCommand.parms[2] = 0x00;
    hciCommand.parms[3] = 0x80;
    
    LOG_DBG("GORMcmd_HCC_GetGormVersion\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

/* Implement in BT_SetBaudRate */
static BtStatus GORMcmd_HCC_ChangeBaudRate(void)
{
    return BT_STATUS_NOT_SUPPORTED;
}

#if BT_PATCH_EXT_ENABLE
static BtStatus GORMcmd_HCC_Set_Patch_Base_Ext(void)
{
    LOG_DBG("GORMcmd_HCC_Set_Patch_Base_Ext\n");
    return GORMcmd_HCC_Set_FW_Reg(patch_setting.dwPatchAddr, patch_setting.dwPatchExtVal);
}

static BtStatus GORMcmd_HCC_Set_Patch_Base(void)
{
    LOG_DBG("GORMcmd_HCC_Set_Patch_Base\n");
    return GORMcmd_HCC_Set_FW_Reg(patch_setting.dwPatchAddr, patch_setting.dwPatchBaseVal);
}

static BtStatus GORMcmd_HCC_Reset_Patch_Len(void)
{
    LOG_DBG("GORMcmd_HCC_Reset_Patch_Len\n");
    return GORMcmd_HCC_Set_FW_Reg(patch_setting.dwPatchLenResetAddr, patch_setting.dwPatchLenResetVal);
}

/* Implement in BT_DownPatch */
static BtStatus GORMcmd_HCC_WritePatch_ext(void)
{
    return BT_STATUS_NOT_SUPPORTED;    
}
#endif

/* Implement in BT_DownPatch */
static BtStatus GORMcmd_HCC_WritePatch(void)
{
    return BT_STATUS_NOT_SUPPORTED;
}

static BtStatus GORMcmd_HCC_Set_Chip_Feature(void)
{
    hciCommand.opCode = 0xFC1E;
    hciCommand.len = 8;
    
    hciCommand.parms[0] = 0xBF;
    hciCommand.parms[1] = 0xFE;
    hciCommand.parms[2] = 0x8D;
    hciCommand.parms[3] = 0xFE;
    hciCommand.parms[4] = 0x98;
    hciCommand.parms[5] = 0x1F;
    hciCommand.parms[6] = 0x59;
    hciCommand.parms[7] = 0x87;
    
    LOG_DBG("GORMcmd_HCC_Set_Chip_Feature\n");
    
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
    
    hciCommand.parms[0] = 0x01;
    hciCommand.parms[1] = 0x01;
    hciCommand.parms[2] = 0x14;     /* clock drift */
    hciCommand.parms[3] = 0x0A;     /* clock jitter */
    hciCommand.parms[4] = 0x06;     /* OSC stable time */
    
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
    
#ifdef MTK_MT6626
    hciCommand.parms[0] = 0x00;     /* LPO source = internal */
#else
    hciCommand.parms[0] = 0x01;     /* LPO source = external */
#endif
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

static BtStatus GORMcmd_HCC_Set_RF_Desense(void)
{
    hciCommand.opCode = 0xFC20;
    hciCommand.len = 6;
    
    hciCommand.parms[0] = 0x00;    /* DWA is off at Channel 30, others still ON */
    hciCommand.parms[1] = 0x00;    /* Turn On RX Current Boost Function */
    hciCommand.parms[2] = 0x01;    /* Turn Off 0.8889 interface switch Function */
    hciCommand.parms[3] = 0x00;    /* Turn Off MPLL Hopping Function */
    hciCommand.parms[4] = 0x01;    /* Turn Off DC_Notch Function */
    hciCommand.parms[5] = 0x00;    /* Turn Off EMI_Hopping Function */
    
    LOG_DBG("GORMcmd_HCC_Set_RF_Desense\n");
    
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
    
    hciCommand.parms[0] = 0xCF;
    hciCommand.parms[1] = 0x8B;
    hciCommand.parms[2] = 0x1F;
    hciCommand.parms[3] = 0x04;
    hciCommand.parms[4] = 0x08;
    hciCommand.parms[5] = 0xA2;
    hciCommand.parms[6] = 0x62;
    hciCommand.parms[7] = 0x56;
    hciCommand.parms[8] = 0x07;
    hciCommand.parms[9] = 0x1B;
    
    LOG_DBG("GORMcmd_HCC_Set_PTA\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

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

static BtStatus GORMcmd_HCC_Enable_PTA(void)
{
    hciCommand.opCode = 0xFCD2;
    hciCommand.len = 1;
    hciCommand.parms[0] = 0x00;
    
    LOG_DBG("GORMcmd_HCC_Enable_PTA\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_WiFi_Ch(void)
{
    hciCommand.opCode = 0xFCD3;
    hciCommand.len = 2;
    
    hciCommand.parms[0] = 0x00;
    hciCommand.parms[1] = 0x0A;   // HB = 0x3c
    
    LOG_DBG("GORMcmd_HCC_Set_WiFi_Ch\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_AFH_Mask(void)
{
    hciCommand.opCode = 0x0C3F;
    hciCommand.len = 10;
    
    hciCommand.parms[0] = 0xFF;
    hciCommand.parms[1] = 0xFF;
    hciCommand.parms[2] = 0xFF;
    hciCommand.parms[3] = 0xFF;
    hciCommand.parms[4] = 0xFF;
    hciCommand.parms[5] = 0xFF;
    hciCommand.parms[6] = 0xFF;
    hciCommand.parms[7] = 0xFF;
    hciCommand.parms[8] = 0xFF;
    hciCommand.parms[9] = 0x7F;
    
    LOG_DBG("GORMcmd_HCC_Set_AFH_Mask\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_Set_Sleep_Control_Reg(void)
{
    DWORD addr, val;
        
    addr = 0x80090074;
#if defined MTK_MT6622
    val = 0x00000A82;
#elif defined MTK_MT6626
    val = 0x00002AE2;
#endif

    LOG_DBG("GORMcmd_HCC_Set_Sleep_Control_Reg\n");
    
    return GORMcmd_HCC_Set_FW_Reg(addr, val);
}

#ifdef MTK_MT6626
static BtStatus GORMcmd_HCC_I2S_Switch(void)
{
    hciCommand.opCode = 0xFC85;
    hciCommand.len = 1;
    hciCommand.parms[0] = 0x02;
    
    LOG_DBG("GORMcmd_HCC_I2S_Switch\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}

static BtStatus GORMcmd_HCC_JTAG_PCM_Switch(void)
{
    hciCommand.opCode = 0xFCD6;
    hciCommand.len = 1;
    hciCommand.parms[0] = 0x01;
    
    LOG_DBG("GORMcmd_HCC_JTAG_PCM_Switch\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif

static BtStatus GORMcmd_HCC_Set_FW_Reg(DWORD dwAddr, DWORD dwVal)
{
    hciCommand.opCode = 0xFCD0;
    hciCommand.len = 8;
    
    hciCommand.parms[0] = (UCHAR)((dwAddr) & 0xFF);		
    hciCommand.parms[1] = (UCHAR)((dwAddr >> 8) & 0xFF);		
    hciCommand.parms[2] = (UCHAR)((dwAddr >> 16) & 0xFF);
    hciCommand.parms[3] = (UCHAR)((dwAddr >> 24) & 0xFF);		
    hciCommand.parms[4] = (UCHAR)((dwVal) & 0xFF);		
    hciCommand.parms[5] = (UCHAR)((dwVal >> 8) & 0xFF);		
    hciCommand.parms[6] = (UCHAR)((dwVal >> 16) & 0xFF);
    hciCommand.parms[7] = (UCHAR)((dwVal >> 24) & 0xFF);		
    
    LOG_DBG("GORMcmd_HCC_Set_FW_Reg\n");
    
    if(BT_SendHciCommand(bt_com_port, &hciCommand) == TRUE){
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}


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
    
    // Copy nvram data
    memcpy(btinit->bt_nvram.raw, ucNvRamData, sizeof(ap_nvram_btradio_struct));
    
    // General init script
    btinit->cur_script = bt_init_script;
    
    /* Can not find matching script, simply skip */
    if((btinit->cur_script) == NULL){
        LOG_ERR("No matching init script\n");
        btStatus = BT_STATUS_FAILED;
        return btStatus;
    }
    
    i = 0;
    while(btinit->cur_script[i].command_func){
        
        if(GORMcmd_HCC_ChangeBaudRate == btinit->cur_script[i].command_func)
        {
            if(BT_SetBaudRate(hComPort, dwBaud, dwHostBaud, dwFlowControl) == FALSE){
                LOG_ERR("Change baud rate fails\n");
                btStatus = BT_STATUS_FAILED;
                return btStatus;
            }            
            i ++;
            continue;
        }
		
    #if BT_PATCH_EXT_ENABLE
        if(GORMcmd_HCC_WritePatch_ext == btinit->cur_script[i].command_func)
        {
            if((pucPatchExtData == NULL) || (dwPatchExtLen == 0)){
                LOG_ERR("No valid patch ext data!\n");
            }
            else{
                if(BT_DownPatch(hComPort, pucPatchExtData, dwPatchExtLen) == FALSE){
                    LOG_ERR("Download patch ext fails\n");
                    btStatus = BT_STATUS_FAILED;
                    return btStatus;
                }
            }
            i ++;
            continue;
        }
    #endif

        if(GORMcmd_HCC_WritePatch == btinit->cur_script[i].command_func)
        {
            if((pucPatchData == NULL) || (dwPatchLen == 0)){
                LOG_ERR("No valid patch data\n");
            }
            else{
                if(BT_DownPatch(hComPort, pucPatchData, dwPatchLen) == FALSE){
                    LOG_ERR("Download patch fails\n");
                    btStatus = BT_STATUS_FAILED;
                    return btStatus;
                }
            }
            i ++;
            continue;
        }
        
        if(GORMcmd_HCC_WakeUpChip == btinit->cur_script[i].command_func)
        {
            if(BT_WakeMagic(hComPort, TRUE) == FALSE){		         
                btStatus = BT_STATUS_FAILED;
                return btStatus;
            }
            i++;
            continue;
        }
        
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
