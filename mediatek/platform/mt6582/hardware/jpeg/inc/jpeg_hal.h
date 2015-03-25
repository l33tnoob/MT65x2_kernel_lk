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

#ifndef __JPEG_HAL_H__
#define __JPEG_HAL_H__

#ifndef MTK_M4U_SUPPORT
  #define JPEG_ENC_USE_PMEM
  #define USE_PMEM
#else
  #include "m4u_lib.h"
#endif

#include "m4u_lib.h"

#if 0 //def JENC_LOCK_VIDEO    
#include <val_types.h>
#include <val_api.h>
#endif
/*******************************************************************************
*
********************************************************************************/
#ifndef JUINT32
typedef unsigned int JUINT32;
#endif

#ifndef JINT32
typedef int JINT32;
#endif
 
#define JPEG_MAX_ENC_SIZE (128*1024*1024)




/*******************************************************************************
* class JpgEncHal
********************************************************************************/
class JpgEncHal {
public:
    JpgEncHal();
    virtual ~JpgEncHal();

    //enum SrcFormat {
    //    kRGB_565_Format,
    //    kRGB_888_Format,
    //    kARGB_8888_Format,
    //    kABGR_8888_Format,
    //    kYUY2_Pack_Format,      // YUYV
    //    kUYVY_Pack_Format,      // UYVY
    //    kYVU9_Planar_Format,    // YUV411, 4x4 sub sample U/V plane
    //    kYV16_Planar_Format,    // YUV422, 2x1 subsampled U/V planes
    //    kYV12_Planar_Format,    // YUV420, 2x2 subsampled U/V planes
    //    kNV12_Format,           // YUV420, 2x2 subsampled , interleaved U/V plane
    //    kNV21_Format,           // YUV420, 2x2 subsampled , interleaved V/U plane
    //    
    //    kSrcFormatCount
    //};

    //enum JPEG_ENC_RESULT {
    //  JPEG_ENC_RST_CFG_ERR,
    //  JPEG_ENC_RST_DONE,      
    //  JPEG_ENC_RST_ROW_DONE,
    //  JPEG_ENC_RST_HUFF_ERROR,
    //  JPEG_ENC_RST_DECODE_FAIL,
    //  JPEG_ENC_RST_BS_UNDERFLOW
    //  
    //};
    

    enum EncFormat {
        //kYUV_444_Format,
        //kYUV_422_Format,
        //kYUV_411_Format,
        //kYUV_420_Format,
        //kYUV_400_Format,
        
        kENC_YUY2_Format,           // YUYV
        kENC_UYVY_Format,           // UYVY
        kENC_NV12_Format,           // YUV420, 2x2 subsampled , interleaved U/V plane
        kENC_NV21_Format,           // YUV420, 2x2 subsampled , interleaved V/U plane
        kENC_YV12_Format,           /// YUV420, 2x2 subsampled, 3 plan 

        
        kEncFormatCount
    };
    
    enum {
      JPEG_ENC_MEM_PHY,
      JPEG_ENC_MEM_PMEM,
      JPEG_ENC_MEM_M4U,
      JPEG_ENC_MEM_ION
      
    };
    
    enum {
      JPEG_ENC_HW,
      JPEG_ENC_SW      
    };

    enum EncLockType{
      JPEG_ENC_LOCK_HW_FIRST,
      JPEG_ENC_LOCK_SW_ONLY,      
      JPEG_ENC_LOCK_HW_ONLY 
    };    
    
    
    // lock with enum
    bool LevelLock(EncLockType type); 
    
    //lock hw first
    bool lock();
    bool unlock();
    bool start(JUINT32 *encSize);
    
    /* set image actual width, height and encode format */ 
    bool setEncSize(JUINT32 width, JUINT32 height, EncFormat encformat) ;
    
    /* get requirement of minimum source buffer size and stride after setEncSize */
    JUINT32 getSrcBufMinSize()      { return fSrcMinBufferSize  ; };  
    JUINT32 getSrcCbCrBufMinSize()  { return fSrcMinCbCrSize ; };
    JUINT32 getSrcBufMinStride()    { return fSrcMinBufferStride  ; };    
    
    /* Set source buffer virtual address.
       The srcChromaAddr should be NULL in YUV422.
    */
    bool setSrcAddr(void *srcAddr, void *srcChromaAddr); 
    /* Set source buffer virtual address.
       The srcChromaAddr should be NULL in YUV422.
       For YUV420(3P), the Y, U, V can be different plan and non-continuous physically
    */
    bool setSrcAddr(void *srcAddr, void *srcCr, void *srcCb);
    
    /* Set source size of buffer1(srcSize) and buffer2(srcSize2) and stride. 
       The buffer size and stride should be at least minimum buffer size and stride.
       The buffer1 and buffer2 share the buffer stride.
       Stride should be align to 32(YUV422) or 16 (YUV420).       
       */
    bool setSrcBufSize(JUINT32 srcStride,JUINT32 srcSize, JUINT32 srcSize2); 
    
    bool setSrcBufSize(JUINT32 srcStride,JUINT32 srcSize, JUINT32 srcSize2, JUINT32 srcSize3);        
    /* set encoding quality , range should be [100:1] */
    bool setQuality(JUINT32 quality) { if( quality > 100) return false ; else fQuality = quality; return true ;}
    
    /* set distination buffer virtual address and size */
    bool setDstAddr(void *dstAddr) { if(dstAddr == NULL) return false; 
                                     else fDstAddr = dstAddr; return true;}
                                     
    /* set bitstream buffer size , should at least 624 bytes */
    bool setDstSize(JUINT32 size) { if(size<624)return false;
                                   else fDstSize = size; return true ;}

    /* set Normal/Exif mode, 1:Normal,0:Exif, default is Normal mode */
    void enableSOI(bool b) { fIsAddSOI = b; }
    
    
    void setIonMode(bool ionEn) { if( ionEn ) fMemType = JPEG_ENC_MEM_ION; 
                                         else fMemType = fMemTypeDefault ;      }
    
    void setSrcFD( JINT32 srcFD, JINT32 srcFD2 ) { fSrcFD = srcFD; fSrcFD2 = srcFD2; }
    
    void setDstFD( JINT32 dstFD ) { fDstFD = dstFD ; }
    
    //bool setSrcAddrPA( JUINT32 srcAddrPA, JUINT32 srcChromaAddrPA); 
    
    //bool setDstAddrPA( JUINT32 dstAddrPA){ if(dstAddrPA == NULL) return false; 
    //                                      else fDstAddrPA = dstAddrPA; return true; 
    //                                   } 
     void setDRI( JINT32 dri ) { fDRI = dri ; } 

     
    
private:
  
    bool allocPMEM();  
    bool alloc_m4u();

    bool free_m4u();
    
    bool alloc_ion();
    bool free_ion();
    bool islock;
    bool onSwEncode(JUINT32 *encSize);
    
    bool fEncoderType;  /// to identify current HAL use HW or SW
    
    bool lockVideo() ;
    bool unlockVideo();

  

    MTKM4UDrv *pM4uDrv ;
    M4U_MODULE_ID_ENUM fm4uJpegInputID ; 
    M4U_MODULE_ID_ENUM fm4uJpegOutputID ; 
    
    JUINT32 fMemType ;
    JUINT32 fMemTypeDefault ;
    
    JUINT32 fSrcWidth;
    JUINT32 fSrcHeight;
    JUINT32 fDstWidth;
    JUINT32 fDstHeight;
    JUINT32 fQuality;
    JUINT32 fROIX;
    JUINT32 fROIY;
    JUINT32 fROIWidth;
    JUINT32 fROIHeight;
    
    JUINT32 fSrcMinBufferSize ;
    JUINT32 fSrcMinCbCrSize ;        
    JUINT32 fSrcMinBufferStride;
    JUINT32 fSrcMinCbCrStride;
    
    JUINT32 fEncSrcBufSize  ;
    JUINT32 fSrcBufStride;
    JUINT32 fSrcBufHeight;
    
    JUINT32 fEncCbCrBufSize ;
    JUINT32 fSrcCbCrBufStride;
    JUINT32 fSrcCbCrBufHeight;
    
    //SrcFormat fSrcFormat;
    EncFormat fEncFormat;
    
    void *fSrcAddr;
    void *fSrcChromaAddr;

    //JUINT32 fEncDstBufSize  ;

    
    void *fSrcCb;
    void *fSrcCr;
    void *fDstAddr;
    int fDstSize;
    bool fIsAddSOI;
    
    

    JUINT32 fSrcAddrPA ;
    JUINT32 fSrcChromaAddrPA;
    JUINT32 fDstAddrPA ;
    
    JUINT32 fDstM4uPA;    
    JUINT32 fSrcM4uPA;    
    JUINT32 fSrcChromaM4uPA;    
    JUINT32 fIsSrc2p;
    JINT32 fSrcPlaneNumber;
    
    //ION
    
    bool fIonEn ;
    //bool fSrcIonEn ;
    //bool fDstIonEn ;
    JINT32 fSrcFD;
    JINT32 fSrcFD2;
    JINT32 fDstFD ;
    
    JUINT32 fSrcIonPA       ;
    JUINT32 fSrcChromaIonPA ;
    JUINT32 fDstIonPA       ;

    void* fSrcIonVA       ;
    void* fSrcChromaIonVA ;
    void* fDstIonVA       ;

    void* fSrcIonHdle       ;
    void* fSrcChromaIonHdle ;
    void* fDstIonHdle       ;


    
    JINT32 fIonDevFD ;
    JUINT32 fDRI ;
    
    

    

#if 1 //def JPEG_ENC_USE_PMEM
    
    unsigned char *fEncSrcPmemVA      ;
    unsigned char *fEncSrcCbCrPmemVA  ;
    unsigned char *fEncDstPmemVA      ;
    
    JUINT32 fEncSrcPmemPA      ;
    JUINT32 fEncSrcCbCrPmemPA  ;
    JUINT32 fEncDstPmemPA      ;
    
    int fEncSrcPmemFD      ;
    int fEncSrcCbCrPmemFD  ;
    int fEncDstPmemFD      ;
#endif
    int encID;
    unsigned long fResTable;

#if 0 //def JENC_LOCK_VIDEO    
    VAL_UINT32_T fVDriverType ;    
    VAL_HW_LOCK_T fVLock;
#endif  
    
    
};

#endif 