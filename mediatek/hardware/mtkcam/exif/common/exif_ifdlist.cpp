/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/
//
#include <string.h>
//
#include "IBaseExif.h"
#include "Exif.h"
#include "exif_type.h"
#include "exif_errcode.h"
//


/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListInit (
)
{
    unsigned int err = EXIF_NO_ERROR;
    unsigned int i = 0;

    mpzeroList = new zeroIFDList_t();
    for (i = 0; i < IFD_MAX_ZEROIFD_CNT; ++i) {
        mpzeroList->ifdNodePool[i].ifd.tag = INVALID_TAG;
    }

    mpexifList = new exifIFDList_t();
    for (i = 0; i < IFD_MAX_EXIFIFD_CNT; ++i) {
        mpexifList->ifdNodePool[i].ifd.tag = INVALID_TAG;
    }

    mpgpsList = new gpsIFDList_t();
    for (i = 0; i < IFD_MAX_GPSIFD_CNT; ++i) {
        mpgpsList->ifdNodePool[i].ifd.tag = INVALID_TAG;
    }

    mpfirstList = new firstIFDList_t();
    for (i = 0; i < IFD_MAX_FIRSTIFD_CNT; ++i) {
        mpfirstList->ifdNodePool[i].ifd.tag = INVALID_TAG;
    }

    mpitopList = new itopIFDList_t();
    for (i = 0; i < IFD_MAX_ITOPIFD_CNT; ++i) {
        mpitopList->ifdNodePool[i].ifd.tag = INVALID_TAG;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListUninit (
)
{
    unsigned int err = EXIF_NO_ERROR;

    if ( mpzeroList != NULL)
    {
        delete mpzeroList;
        mpzeroList = NULL;
    }

    if ( mpexifList != NULL)
    {
        delete mpexifList;
        mpexifList = NULL;
    }

    if ( mpgpsList != NULL)
    {
        delete mpgpsList;
        mpgpsList = NULL;
    }

    if ( mpfirstList != NULL)
    {
        delete mpfirstList;
        mpfirstList = NULL;
    }

    if ( mpitopList != NULL)
    {
        delete mpitopList;
        mpitopList = NULL;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
ifdNode_t* 
ExifUtils::ifdListNodeAlloc (
    unsigned int ifdType
)
{
    unsigned int err = EXIF_NO_ERROR;
    
    ifdNode_t *pnode = NULL;
    unsigned int maxCnt = 0, idx;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        pnode = mpzeroList->ifdNodePool;
        maxCnt = IFD_MAX_ZEROIFD_CNT;
        break;
    case IFD_TYPE_EXIFIFD:
        pnode = mpexifList->ifdNodePool;
        maxCnt = IFD_MAX_EXIFIFD_CNT;
        break;
    case IFD_TYPE_GPSIFD:
        pnode = mpgpsList->ifdNodePool;
        maxCnt = IFD_MAX_GPSIFD_CNT;
        break;    
    case IFD_TYPE_FIRSTIFD:
        pnode = mpfirstList->ifdNodePool;
        maxCnt = IFD_MAX_FIRSTIFD_CNT;
        break;
    case IFD_TYPE_ITOPIFD:
        pnode = mpitopList->ifdNodePool;
        maxCnt = IFD_MAX_ITOPIFD_CNT;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }
    
    /* find a empty node */
    if (!err) {
        idx = 0;
        while (pnode->ifd.tag != INVALID_TAG && idx < maxCnt) {
            idx++;
            pnode++;
        }
    }

    exifErrPrint((unsigned char* ) "ifdListNodeAlloc", err);
    
    return pnode;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListNodeInsert (
    unsigned int ifdType, 
    ifdNode_t *pnode,
    void *pdata
)
{
    unsigned int err = EXIF_NO_ERROR;
    ifdNode_t *pheadNode = 0;
    ifdNode_t *pcurNode;
    ifdNode_t * pprevNode;
    
    unsigned char *pvalbuf = 0;
    unsigned int size;
    unsigned int *pbufPos = 0;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        pheadNode = mpzeroList->pheadNode;
        mpzeroList->nodeCnt++;
        pvalbuf = mpzeroList->valBuf;
        pbufPos = &mpzeroList->valBufPos;
        break;
    case IFD_TYPE_EXIFIFD:
        pheadNode = mpexifList->pheadNode;
        mpexifList->nodeCnt++;
        pvalbuf = mpexifList->valBuf;
        pbufPos = &mpexifList->valBufPos;
        break;
    case IFD_TYPE_GPSIFD:
        pheadNode = mpgpsList->pheadNode;
        mpgpsList->nodeCnt++;
        pvalbuf = mpgpsList->valBuf;
        pbufPos = &mpgpsList->valBufPos;
        break;        
    case IFD_TYPE_FIRSTIFD:
        pheadNode = mpfirstList->pheadNode;
        mpfirstList->nodeCnt++;
        pvalbuf = mpfirstList->valBuf;
        pbufPos = &mpfirstList->valBufPos;
        break;
    case IFD_TYPE_ITOPIFD:
        pheadNode = mpitopList->pheadNode;
        mpitopList->nodeCnt++;
        pvalbuf = mpitopList->valBuf;
        pbufPos = &mpitopList->valBufPos;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }

    if (!err) {
        pcurNode = pheadNode;
        
        if (pcurNode == NULL) { /* the only node */
            pheadNode = pnode;
            pnode->next = NULL;
        }
        else {
            pprevNode = NULL;
            
            while (pcurNode && (pcurNode->ifd.tag < pnode->ifd.tag)) {/* what if tag no are equal */
                pprevNode = pcurNode;
                pcurNode = pcurNode->next;
            }
                        
            if (pcurNode == NULL) { /* this is biggest tag number, append to tail */
                pprevNode->next = pnode;
                pnode->next = NULL;
            } 
            else { /* insert the node, sort by number */
                if (pcurNode == pheadNode) { /* this is smallest tag number */
                    pnode->next = pheadNode;
                    pheadNode = pnode;
                }
                else { /* insert to middle */
                    pprevNode->next = pnode;
                    pnode->next = pcurNode;
                }
            }
        }

        if (pnode && pdata) {
            size = exifIFDValueSizeof(pnode->ifd.type, pnode->ifd.count);
            if (size <= 4)
                memcpy(&pnode->ifd.valoff, pdata, size);
            else {  
                memcpy(pvalbuf + *pbufPos, pdata, size);
                pnode->ifd.valoff = *pbufPos;
                *pbufPos += size;
            }
        }

        err = ifdListHeadNodeSet(ifdType, pheadNode);
    }
    
    exifErrPrint((unsigned char* ) "ifdListNodeInsert", err);
    
    return err;
}


/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListNodeModify (
    unsigned short ifdType, 
    unsigned short tagId, 
    void *pdata
)
{
    unsigned int err = EXIF_NO_ERROR;
    
    unsigned int size = 0;
    unsigned int bufAddr = 0;
    ifdNode_t *pnode = 0;

    err = ifdListNodeInfoGet(ifdType, tagId, &pnode, &bufAddr);
    //MY_LOGD_IF("[ifdListNodeModify]err: 0x%x\n", err);
    //MY_LOGD_IF("ifd tag/type/count/valoff: 0x%x/0x%x/0x%x/0x%x\n", pnode->ifd.tag, pnode->ifd.type, pnode->ifd.count, pnode->ifd.valoff);
    //MY_LOGD_IF("bufAddr: 0x%x\n", bufAddr);   
    if ((!err) && (pnode->ifd.tag != INVALID_TAG)) {
        size = exifIFDValueSizeof(pnode->ifd.type, pnode->ifd.count);
        memcpy((unsigned char*)bufAddr, (unsigned char*)pdata, size);
    }
    
    exifErrPrint((unsigned char* ) "ifdListNodeModify", err);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListNodeDelete (
    unsigned int ifdType, 
    unsigned short tagId
)
{
    unsigned int err = EXIF_NO_ERROR;
    ifdNode_t *pheadNode = 0;
    ifdNode_t *pcurNode;
    ifdNode_t *pprevNode = 0;

    pheadNode = idfListHeadNodeGet(ifdType);
        
    if (pheadNode) {
        pcurNode = pheadNode;
        pprevNode = NULL;
        
        while (pcurNode && (pcurNode->ifd.tag != tagId)) {/* what if tag no are equal */
            pprevNode = pcurNode;
            pcurNode = pcurNode->next;
        }
        
        if (pcurNode != NULL) { /* node found and delete it */  
            if (pprevNode) { /* not head node */
                pprevNode->next = pcurNode->next;
                pcurNode->next = 0;             
                switch (ifdType) {
                case IFD_TYPE_ZEROIFD:
                    mpzeroList->nodeCnt--;
                    break;
                case IFD_TYPE_EXIFIFD:
                    mpexifList->nodeCnt--;
                    break;
                case IFD_TYPE_GPSIFD:
                    mpgpsList->nodeCnt--;
                    break;                    
                case IFD_TYPE_FIRSTIFD:
                    mpfirstList->nodeCnt--;
                    break;
                case IFD_TYPE_ITOPIFD:
                    mpitopList->nodeCnt--;
                    break;
                }
            }
            else { /* head node */
                pheadNode = pcurNode->next;
                err = ifdListHeadNodeSet(ifdType, pheadNode);
            }
            memset(pcurNode, 0x00, sizeof(ifdNode_t));/* clear node content */
        } 

    }
    
    exifErrPrint((unsigned char* ) "ifdListNodeDelete", err);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
ExifUtils::ifdListNodeInfoGet (
    unsigned short ifdType, 
    unsigned short tagId, 
    ifdNode_t **pnode, 
    unsigned int *pbufAddr
)
{
    unsigned int err = EXIF_NO_ERROR;
    ifdNode_t *pcurNode = 0;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        pcurNode = mpzeroList->pheadNode;
        break;
    case IFD_TYPE_EXIFIFD:
        pcurNode = mpexifList->pheadNode;
        break;
    case IFD_TYPE_GPSIFD:
        pcurNode = mpgpsList->pheadNode;
        break;        
    case IFD_TYPE_FIRSTIFD:
        pcurNode = mpfirstList->pheadNode;
        break;
    case IFD_TYPE_ITOPIFD:
        pcurNode = mpitopList->pheadNode;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }

    if (!err) {
        while (pcurNode != NULL && (pcurNode->ifd.tag != tagId))
            pcurNode = pcurNode->next;

        if (pcurNode) {
            *pbufAddr = (unsigned int)(exifHdrTmplAddrGet() + pcurNode->ifd.valoff);
            *pnode = pcurNode;
        }
        else
            err = LIBEXIF_IFD_ERR0003;
    }   
    
    exifErrPrint((unsigned char* ) "ifdListNodeInfoGet", err);
    
    return err;
}


/*******************************************************************************
*
********************************************************************************/
ifdNode_t*
ExifUtils::idfListHeadNodeGet (
    unsigned int ifdType
)
{
    unsigned int err = EXIF_NO_ERROR;
    ifdNode_t *pnode = 0;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        pnode = mpzeroList->pheadNode;
        break;
    case IFD_TYPE_EXIFIFD:
        pnode = mpexifList->pheadNode;
        break;
    case IFD_TYPE_GPSIFD:
        pnode = mpgpsList->pheadNode;
        break;        
    case IFD_TYPE_FIRSTIFD:
        pnode = mpfirstList->pheadNode;
        break;
    case IFD_TYPE_ITOPIFD:
        pnode = mpitopList->pheadNode;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }
    
    exifErrPrint((unsigned char* ) "idfListHeadNodeGet", err);
    
    return pnode;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
ExifUtils::ifdListHeadNodeSet (
    unsigned int ifdType, 
    ifdNode_t *pheadNode
)
{
    unsigned int err = EXIF_NO_ERROR;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        mpzeroList->pheadNode = pheadNode;
        break;
    case IFD_TYPE_EXIFIFD:
        mpexifList->pheadNode = pheadNode;
        break;
    case IFD_TYPE_GPSIFD:
        mpgpsList->pheadNode = pheadNode;
        break;        
    case IFD_TYPE_FIRSTIFD:
        mpfirstList->pheadNode = pheadNode;
        break;
    case IFD_TYPE_ITOPIFD:
        mpitopList->pheadNode = pheadNode;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }
    
    exifErrPrint((unsigned char* ) "ifdListHeadNodeSet", err);
    
    return err;
}

