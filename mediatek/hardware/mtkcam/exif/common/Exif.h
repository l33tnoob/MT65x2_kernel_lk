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
#ifndef _EXIF_UTILS_H
#define _EXIF_UTILS_H
//

/*******************************************************************************
*
********************************************************************************/

struct ifdNode_t;
struct zeroIFDList_t;
struct exifIFDList_t;
struct gpsIFDList_t;
struct firstIFDList_t;
struct itopIFDList_t;

/*******************************************************************************
*
********************************************************************************/


/******************************************************************************
 *  Exif Interface
 ******************************************************************************/
class  ExifUtils : public IBaseExif
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Instantiation.
    virtual                             ~ExifUtils();
                                        ExifUtils();

    virtual bool                        init();
    virtual bool                        uninit();

public:     ////                        Operations.
    virtual unsigned int exifApp1Make(exifImageInfo_t *pexifImgInfo, exifAPP1Info_t *pexifAPP1Info, unsigned int *pretSize);
    virtual unsigned int exifAppnMake(unsigned int appn, unsigned char *paddr, unsigned char *pdata, unsigned int dataSize, unsigned int *pretSize); 

public:     //// exif_ifdinit.cpp
    virtual unsigned int ifdValueInit();
    virtual unsigned int ifdZeroIFDValInit(ifdNode_t *pnode, struct zeroIFDList_t *plist);
    virtual unsigned int ifdExifIFDValInit(ifdNode_t *pnode, struct exifIFDList_t *plist);
    virtual unsigned int ifdGpsIFDValInit(ifdNode_t *pnode, struct gpsIFDList_t *plist);
    virtual unsigned int ifdFirstIFDValInit(ifdNode_t *pnode, struct firstIFDList_t *plist);
    virtual unsigned int ifdItopIFDValInit(ifdNode_t *pnode, struct itopIFDList_t *plist);

public:     //// exif_ifdlist.cpp
    virtual unsigned int ifdListInit();
    virtual unsigned int ifdListUninit();
    virtual ifdNode_t* ifdListNodeAlloc(unsigned int ifdType);
    virtual unsigned int ifdListNodeInsert(unsigned int ifdType, ifdNode_t *pnode, void *pdata);
    virtual unsigned int ifdListNodeModify(unsigned short ifdType, unsigned short tagId, void *pdata);
    virtual unsigned int ifdListNodeDelete(unsigned int ifdType, unsigned short tagId);
    virtual unsigned int ifdListNodeInfoGet(unsigned short ifdType, unsigned short tagId, ifdNode_t **pnode, unsigned int *pbufAddr);
    virtual ifdNode_t* idfListHeadNodeGet(unsigned int ifdType);
    virtual unsigned int ifdListHeadNodeSet(unsigned int ifdType, ifdNode_t *pheadNode);
    virtual zeroIFDList_t*  ifdZeroListGet()          { return mpzeroList; }
    virtual exifIFDList_t*  ifdExifListGet()          { return mpexifList; }
    virtual gpsIFDList_t*   ifdGpsListGet()           { return mpgpsList; }
    virtual firstIFDList_t* ifdFirstListGet()         { return mpfirstList; }
    virtual itopIFDList_t*  ifdItopListGet()          { return mpitopList; }


public:     //// exif_ifdmisc.cpp
    virtual unsigned int ifdListSizeof();
    virtual unsigned char* ifdListValBufGet(unsigned int ifdType);
    virtual unsigned int ifdListValBufSizeof(unsigned int ifdType);
    virtual unsigned int ifdListNodeCntGet(unsigned int ifdType);

public:     //// exif_hdr.cpp
    virtual unsigned int exifAPP1Write(unsigned char *pdata, unsigned int *pretSize);
    virtual unsigned int exifSOIWrite(unsigned char *pdata, unsigned int *pretSize);
    virtual unsigned char* exifHdrTmplAddrGet()             { return mpexifHdrTmplBuf; }
    virtual void exifHdrTmplAddrSet(unsigned char *paddr)   { mpexifHdrTmplBuf = paddr; }

public:     //// exif_misc.cpp
    virtual unsigned short  swap16(unsigned short x);
    virtual unsigned int    swap32(unsigned int x);
    virtual unsigned short  swap16ByOrder(unsigned short order, unsigned short x);
    virtual unsigned int    swap32ByOrder(unsigned short order, unsigned int x);
    virtual unsigned short  read16(void *psrc);
    virtual unsigned int    read32(void *psrc);
    virtual void write16(void *pdst, unsigned short src);
    virtual void write32(void *pdst, unsigned int src);
    virtual unsigned int exifMemcmp(unsigned char *pdst, unsigned char *psrc, unsigned int size);
    virtual unsigned int exifApp1Sizeof();
    virtual unsigned int exifIFDValueSizeof(unsigned short type, unsigned int count);
    virtual void exifErrPrint(unsigned char *pname, unsigned int err);

public:     //// exif_make.cpp
    virtual unsigned int exifIsGpsOnFlag()          { return exifGpsEnFlag; }
    virtual unsigned int exifTagUpdate(exifImageInfo_t *pexifImgInfo, exifAPP1Info_t *pexifAPP1Info);

private:  ////    Data Members.
    zeroIFDList_t*  mpzeroList;
    exifIFDList_t*  mpexifList;
    gpsIFDList_t*   mpgpsList;
    firstIFDList_t* mpfirstList;
    itopIFDList_t*  mpitopList;
    //
    unsigned int    exifGpsEnFlag;
    unsigned char*  mpexifHdrTmplBuf;
    //
    signed int      miLogLevel;

};

/******************************************************************************
 *
 ******************************************************************************/


#endif // _EXIF_UTILS_H

