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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>

#include "bt_kal.h"

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

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
);

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

BOOL BT_InitDevice(
    HANDLE  hComPortFile,
    PBYTE   ucNvRamData,
    DWORD   dwBaud,
    DWORD   dwHostBaud,
    DWORD   dwFlowControl,
    SETUP_UART_PARAM_T setup_uart_param
    )
{
    DWORD   dwStatus;
    LPBYTE  pbPatchExtBin = NULL;
    DWORD   dwPatchExtLen = 0;
    FILE*   pPatchExtFile = NULL;
    LPBYTE  pbPatchBin = NULL;
    DWORD   dwPatchLen = 0;
    FILE*   pPatchFile = NULL;
    
    LOG_DBG("BT_InitDevice\n");
    
    
    /* Invoke HCI transport entrance */
    dwStatus = GORM_Init(
        hComPortFile,
        pbPatchExtBin,   //patch ext 
        dwPatchExtLen,   
        pbPatchBin,      //patch
        dwPatchLen,
        ucNvRamData,
        dwBaud,
        dwHostBaud,
        dwFlowControl
        );

    if(dwStatus != 0){
        LOG_ERR("GORM fails return code %d\n", (int)dwStatus);
        return FALSE;
    }
    
    return TRUE;
}

BOOL BT_DeinitDevice(
    HANDLE  hComPortFile
    )
{
    LOG_DBG("BT_DeinitDevice\n");
    return TRUE;
}

BOOL BT_SendHciCommand(
    HANDLE  hComPortFile,
    HCI_CMD_T *pHciCommand 
    )
{
    UCHAR ucHciCmd[256+4] = {0x01, 0x00, 0x00, 0x00};
    
    ucHciCmd[1] = (UCHAR)pHciCommand->opCode;
    ucHciCmd[2] = (UCHAR)(pHciCommand->opCode >> 8);
    ucHciCmd[3] = pHciCommand->len;
    
    LOG_DBG("OpCode 0x%04x len %d\n", pHciCommand->opCode, (int)pHciCommand->len);
    
    if(pHciCommand->len){
        memcpy(&ucHciCmd[4], pHciCommand->parms, pHciCommand->len);
    }
    
    if(bt_send_data(hComPortFile, ucHciCmd, pHciCommand->len + 4) < 0){
        LOG_ERR("Write HCI command fails errno %d\n", errno);
        return FALSE;
    }
    
    return TRUE;
}

static BOOL BT_ReadPacketHeader(
    HANDLE  hComPortFile,
    UCHAR*  pPacketType,//cmd, event, ACL data, SCO data
    PDWORD  pRemainLen, //remaining len for variable part
    USHORT* pusOpCode,  //cmd OpCode
    USHORT* pusConnHandle, //connect handle, flags
    UCHAR*  pEventCode  //event code
    )
{
    UCHAR   pCmdHdr[3];
    UCHAR   pAclHdr[4];
    UCHAR   pScoHdr[3];
    UCHAR   pEventHdr[2];
    UCHAR   type = 0;
    
    /* Read UART header */
    if(bt_receive_data(hComPortFile, &type, 1) < 0){
        LOG_ERR("Read packet header fails\n");
        return FALSE;
    }
    
    *pPacketType = type;
    
    switch(type)
    {
      case 1: //cmd
        if(bt_receive_data(hComPortFile, pCmdHdr, 3) < 0){
            LOG_ERR("Read command header fails %d\n", errno);
            return FALSE;
        }
        
        *pusOpCode = (((USHORT)pCmdHdr[0]) | (((USHORT)pCmdHdr[1]) << 8));
        *pRemainLen = pCmdHdr[2];
        break;
        
      case 2: //ACL data
        if(bt_receive_data(hComPortFile, pAclHdr, 4) < 0){
            LOG_ERR("Read ACL header fails %d\n", errno);
            return FALSE;
        }
        
        *pusConnHandle = (((USHORT)pAclHdr[0]) | (((USHORT)pAclHdr[1]) << 8));
        *pRemainLen = (((USHORT)pAclHdr[2]) | (((USHORT)pAclHdr[3]) << 8));
        break;
        
      case 3: //SCO data
        if(bt_receive_data(hComPortFile, pScoHdr, 3) < 0){
            LOG_ERR("Read SCO header fails %d\n", errno);
            return FALSE;
        }
        
        *pusConnHandle = (((USHORT)pScoHdr[0]) | (((USHORT)pScoHdr[1]) << 8));
        *pRemainLen = pScoHdr[2];
        break;
        
      case 4: //event
        if(bt_receive_data(hComPortFile, pEventHdr, 2) < 0){
            LOG_ERR("Read event header fails %d\n", errno);
            return FALSE;
        }
        
        *pEventCode = pEventHdr[0];
        *pRemainLen = pEventHdr[1];
        break;
        
      default: //other
        LOG_ERR("Unknown packet type %02x\n", type);
        return FALSE;
        break;
    }
    
    return TRUE;
}

static BOOL BT_ReadPacket(
    HANDLE  hComPortFile,
    PUCHAR  pPacket,
    DWORD   dwMaxBufSz,
    PDWORD  pdwPacketLen
    )
{
    UCHAR   packetType;
    DWORD   remainLen;
    USHORT  usOpCode, usConnHandle;
    UCHAR   ucEventCode;
    DWORD   dwPktLen = 0;
    
    if(dwMaxBufSz == 0){
        LOG_ERR("Read packet buffer too short\n");
        return FALSE;
    }
    
    if(BT_ReadPacketHeader(
        hComPortFile,
        &packetType,
        &remainLen,
        &usOpCode,
        &usConnHandle,
        &ucEventCode) == FALSE){
        
        LOG_ERR("Read packet header fails\n");
        return FALSE;
    }
    
    pPacket[0] = packetType;
    dwPktLen ++;
    
    /* Command packet */
    if(packetType == 1){
        if(dwMaxBufSz < (4 + remainLen)){
            LOG_ERR("Read command packet buffer too short\n");
            return FALSE;
        }
        
        pPacket[dwPktLen] = (UCHAR)usOpCode;
        pPacket[dwPktLen + 1] = (UCHAR)(usOpCode >> 8);
        dwPktLen += 2;
        
        pPacket[dwPktLen] = (UCHAR)remainLen;
        dwPktLen ++;
        
        if(bt_receive_data(hComPortFile, pPacket + dwPktLen, remainLen) < 0)
        {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }    
        
        dwPktLen += remainLen;
        *pdwPacketLen = dwPktLen;
        
        return TRUE;
    }
    
    /* ACL data */
    if(packetType == 2){
        if(dwMaxBufSz < (5 + remainLen)){
            LOG_ERR("Read ACL packet buffer too short\n");
            return FALSE;
        }
        
        pPacket[dwPktLen] = (UCHAR)usConnHandle;
        pPacket[dwPktLen + 1] = (UCHAR)(usConnHandle >> 8);
        dwPktLen += 2;

        pPacket[dwPktLen] = (UCHAR)remainLen;
        pPacket[dwPktLen + 1] = (UCHAR)(remainLen >> 8);
        dwPktLen += 2;
        
        if(bt_receive_data(hComPortFile, pPacket + dwPktLen, remainLen) < 0)
        {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }    
        
        dwPktLen += remainLen;
        *pdwPacketLen = dwPktLen;
        
        return TRUE;
    }
    
    /* SCO data */
    if(packetType == 3){
        if(dwMaxBufSz < (4 + remainLen)){
            LOG_ERR("Read SCO packet buffer too short\n");
            return FALSE;
        }
        
        pPacket[dwPktLen] = (UCHAR)usConnHandle;
        pPacket[dwPktLen + 1] = (UCHAR)(usConnHandle >> 8);
        dwPktLen += 2;
        
        pPacket[dwPktLen] = (UCHAR)remainLen;
        dwPktLen ++;
        
        if(bt_receive_data(hComPortFile, pPacket + dwPktLen, remainLen) < 0)
        {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }    
        
        
        dwPktLen += remainLen;
        *pdwPacketLen = dwPktLen;
        
        return TRUE;
    }
    
    /* Event packet */
    if(packetType == 4){
        if(dwMaxBufSz < (3 + remainLen)){
            LOG_ERR("Read event packet buffer too short\n");
            return FALSE;
        }
        
        pPacket[dwPktLen] = ucEventCode;
        pPacket[dwPktLen + 1] = (UCHAR)remainLen;
        dwPktLen += 2;
        
        if(bt_receive_data(hComPortFile, pPacket + dwPktLen, remainLen) < 0)
        {
            LOG_ERR("Read remain packet fails %d\n", errno);
            return FALSE;
        }    
        
        dwPktLen += remainLen;
        *pdwPacketLen = dwPktLen;
        
        return TRUE;
    }
    
    LOG_ERR("Unknown packet type\n");
    
    return FALSE;
}

BOOL BT_ReadExpectedEvent(
    HANDLE  hComPortFile,
    PUCHAR  pEventPacket,
    DWORD   dwMaxBufSz,
    UCHAR   ucExpectedEventCode,
    PDWORD  pdwPacketLen,
    BOOLEAN fCheckCompleteOpCode,//if event code is Command Complete Event, whether to check OpCode
    USHORT  usExpectedOpCode,
    BOOLEAN fCheckCommandStatus,//if event code is Command Status Event, whether to check status
    UCHAR   ucExpectedStatus
    )
{
    USHORT  usEventOpCode;
    UCHAR   ucEventCode, ucCommandStatus;
    
    if(BT_ReadPacket(hComPortFile, 
        pEventPacket,
        dwMaxBufSz,
        pdwPacketLen) == FALSE){

        LOG_ERR("Read packet fails\n");
        return FALSE;
    }
    
    /* Expect Event only */
    if(pEventPacket[0] != 4){
        LOG_ERR("Unexpected packet type\n");
        return FALSE;
    }
    
    ucEventCode = pEventPacket[1];
    
    if(ucEventCode != ucExpectedEventCode){
        LOG_ERR("Unexpected event code\n");
        return FALSE;
    }
    
    if(ucEventCode == 0x0E){
        if(fCheckCompleteOpCode){
            usEventOpCode = ((USHORT)pEventPacket[4]) | (((USHORT)pEventPacket[5]) << 8);

            if(usEventOpCode != usExpectedOpCode){
                LOG_ERR("Unexpected OpCode\n");
                return FALSE;
            }
        }
        if(fCheckCommandStatus){
            ucCommandStatus = pEventPacket[6];

            if(ucCommandStatus != ucExpectedStatus){
                LOG_ERR("Unexpected status %02x\n", ucCommandStatus);
                return FALSE;
            }            
        }
    }
    
    if(ucEventCode == 0x0F){
        if(fCheckCompleteOpCode){
            usEventOpCode = ((USHORT)pEventPacket[5]) | (((USHORT)pEventPacket[6]) << 8);

            if(usEventOpCode != usExpectedOpCode){
                LOG_ERR("Unexpected OpCode\n");
                return FALSE;
            }
        }
        
        if(fCheckCommandStatus){
            ucCommandStatus = pEventPacket[3];

            if(ucCommandStatus != ucExpectedStatus){
                LOG_ERR("Unexpected status %02x\n", ucCommandStatus);
                return FALSE;
            }
        }
    }
    
    return TRUE;
}
