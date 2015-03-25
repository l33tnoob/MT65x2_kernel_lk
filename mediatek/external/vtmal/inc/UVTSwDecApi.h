/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   UVTSwDecApi.h
 *
 * Project:
 * --------
 *	MTK
 *
 * Description:
 * ------------
 *   The API module user interface of the Decoder.
 *
 * Author:
 * -------
 *   Wen-Jun Liu
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log:   $
 *
 * 05 24 2011 aby.chang
 * [ALPS00048915] [ALPS] New codebase lib
 * .
 *
 * 05 05 2011 aby.chang
 * [ALPS00045858] VT Codec New Interface
 * .
 *
 * 02 23 2011 isaac.lee
 * [WCPSP00000572] SWIP Check-in
 * .
 *
 * 02 21 2011 isaac.lee
 * [WCPSP00000572] SWIP Check-in
 * Add adaptor and related changes.
 *
 * 12 07 2010 eason.chen
 * [WCPSP00000553] VIDEO SWIP creation
 * .
 * First time check in mpeg4dec SWIP
 *
 * 10 14 2010 isaac.lee
 * [MAUI_02825696] Sync code from MAUI to 6276_QC
 * Sync with DEV and change to Pure ARM.
 *
 * 10 08 2010 yuta.chen
 * [MAUI_02825203] [SW Video] merge ARM and Duo MPEG-4 decoder
 * include kal_non_specific_general_types.h.
 *
 * 09 17 2010 yuta.chen
 * [MAUI_02563956] [V2 Development] MPEG-4 SW decoder adaptor warning
 * .
 *
 * 08 20 2010 isaac.lee
 * [MAUI_02608734] Check in EOF in Duo MP4 decoder
 * Add EOF in DecNextPicture.
 *
 * 08 05 2010 yuta.chen
 * [MAUI_02602513] [V2 Development] Modify EOF processing
 * .
 *
 * Jun 21 2010 mtk02893
 * [MAUI_02563956] [V2 Development] MPEG-4 SW decoder adaptor warning
 * 
 *
 * Jun 18 2010 mtk03105
 * [MAUI_02563774] [6276 HQA] Check in MAUI
 * Add Duo
 *
 * Jun 17 2010 mtk02893
 * [MAUI_02563956] [V2 Development] MPEG-4 SW decoder adaptor warning
 * 
 *
 * May 25 2010 mtk02893
 * [MAUI_02422577] [SW Video] check in VT related code
 * pack parameters of MP4SwDecInit 
 *
 * May 11 2010 mtk02184
 * [MAUI_02419683] [Low complexity decoder] New feature check-in (MPEG-4, H.264)
 * 
 *
 * Apr 29 2010 mtk02184
 * [MAUI_02408167] [V2 Development] Seperate constraint of width and resolution
 * 
 *
 * Mar 30 2010 mtk02893
 * [MAUI_02385163] [SW Video] augment MPEG-4 decoder
 * add new members in UVTSwDecOutput (for debug)
 *
 * Feb 22 2010 mtk02893
 * [MAUI_02355914] [V2_Integration] Update player to flexible architecture and all the customization
 * add prototype of callback functions
 *
 * Feb 11 2010 mtk02893
 * [MAUI_02355914] [V2_Integration] Update player to flexible architecture and all the customization
 * add a new API prototype: MP4SwDecReturnBitstream (implemented by driver)
 *
 * Feb 11 2010 mtk02893
 * [MAUI_02355914] [V2_Integration] Update player to flexible architecture and all the customization
 * add new APIs for V2 integration: MP4SwDecQueryCapability, MP4SwDecNextPicture 
 * modify MP4SwDecInit, MP4SwDecDecode for V2 integration
 *
 * Dec 23 2009 MTK02184
 * [MAUI_02020588] [V2 check in] check in video v2 with w09.49 temp load
 * 
 *
 * Sep 19 2008 mtk01297
 * [MAUI_01237656] Video Streaming_The player show abnormal.
 * 
 *
 * Sep 3 2008 mtk01297
 * [MAUI_01230365] Slide show_There are some messing text in 1st frame.
 * 
 *
 * Aug 6 2008 mtk01297
 * [MAUI_00816406] [SW Video] Check in sw video feature
 * 
 *
 * Aug 6 2008 mtk01297
 * [MAUI_00816406] [SW Video] Check in sw video feature
 * 
 * 
 ****************************************************************************/

/*------------------------------------------------------------------------------

    Table of contents

    1. Include Headers

    2. Enumerations used as a return value or a parameter.
        2.1. API's return value enumerations.

    3. User Structures
        3.1. Structures for UVTSwDecDecode() parameters.
        3.2. Structures for information interchange with 
             DEC API and user application.

    4. Common defines

    5. Prototypes of Decoder API functions

------------------------------------------------------------------------------*/

#ifndef UVTSWDECAPI_H
#define UVTSWDECAPI_H

#ifdef __cplusplus
extern "C"
    {
#endif

#include "drv_comm.h"

/*------------------------------------------------------------------------------
    1. Include Headers
------------------------------------------------------------------------------*/

#include "basetype.h"

#include "dec_enc_common.h"

#include "sw_types.h"

/*------------------------------------------------------------------------------
    2.1. API's return value enumerations.
------------------------------------------------------------------------------*/

    /* API return values */
    typedef enum
    {
        UVTSWDEC_OK = 0,
        UVTSWDEC_STRM_PROCESSED = 1,
        UVTSWDEC_VOP_RDY,
        UVTSWDEC_VOP_RDY_BUFF_NOT_EMPTY,
        UVTSWDEC_VOS_END,
        UVTSWDEC_HDRS_RDY,
        UVTSWDEC_HDRS_RDY_BUFF_NOT_EMPTY, 
        UVTSWDEC_PARAM_ERR = -1,
        UVTSWDEC_STRM_ERR = -2,
        UVTSWDEC_STRM_ERR_BUFF_NOT_EMPTY = -3,
        UVTSWDEC_NOT_INITIALIZED = -4, 
        UVTSWDEC_MEMFAIL = -5,
        UVTSWDEC_EVALUATION_LIMIT_EXCEEDED = -6,
        UVTSWDEC_RESOLUTION_ERR = -7,
        UVTSWDEC_BSQUEUEFULL_ERR = -8
    } UVTSwDecRet;



/*------------------------------------------------------------------------------
    3.1. Structures for UVTSwDecDecode() parameters.
------------------------------------------------------------------------------*/

    /* typedef of the Decoder instance */
    typedef void *UVTSwDecInst; 

    /* Input structure */
    typedef struct
    {
        u8  *pStream;            /* Pointer to stream to be decoded       */
        u32  dataLen;            /* Number of bytes to be decoded         */
        u32 *pExtOutputPic;      /* User allocated picture buffer pointer.
                                    Decoder will write output picture into 
                                    this pointer */
#ifdef FP_VT_DEC_IF
        u8  *pBuffer;	         /* Points to stream buffer to be decoded, including VA,PA and buffer status */
        u32 buffSize;            /* Buffer length */
#endif
    } UVTSwDecInput;

    /* Timecode structure */
    typedef struct 
    {
        u32 hours;
        u32 minutes;
        u32 seconds;
        u32 timeIncr;
        u32 timeRes;
    } UVTSwTimeCode;

    /* Output structure */
    typedef struct
    {
        u32 *pOutputPicture;     /* Pointer to the recent decoded picture
                                    decoder output, YUV format		 */
        u32 *pRefPicture; /* latest reference frame (for debug) */
        u32 nbrOfErrMBs;        /* Number of concealed MB's in last VOP  */
        u32 vopCodingType; /* vop type (for debug) */
        UVTSwTimeCode timeCode;
        u32 displayWidth;
        u32 displayHeight;
        u32 workingWidth;
        u32 workingHeight;
        u32 *pRefPictureU;
        u32 *pRefPictureV;
        u32 isEOF; /* add for driver, 1 when all frames are finished */
    } UVTSwDecOutput;

/*------------------------------------------------------------------------------
    3.2. Structures for information interchange with DEC API 
         and user application.
------------------------------------------------------------------------------*/

    /* QP info structure */
    typedef struct
    {
        u8  vopQp;
        u32 *pQp;  
    } UVTSwDecQpInfo;

    /* Deblocking info structure */
    typedef struct
    {
        u8  vopQP;
        u8  *pYuvBuffer;
    } UVTSwDecDeblockingInfo;

    /* Stream information */
    typedef struct
    {
        u32 frameWidth;
        u32 frameHeight;
        u32 isShortVideo;
        u32 profileAndLevelIndication;
        u32 videoFormat;
        u32 videoRange;
        u32 vopNumber;
        u32 vopCodingType;
        u32 resyncMarkerDisable;
        u32 vopQp;
        u32 videoObjectLayerWidth;
        u32 videoObjectLayerHeight;
        u32 userDataVOSLen;
        u32 userDataVISOLen;
        u32 userDataVOLLen;
        u32 userDataGOVLen;
        u32 parWidth;
        u32 parHeight;
        u32 streamFormat;
        u32 deblockingFilter;
    } UVTSwDecInfo;
 
    /* User data type */
    typedef enum {
        UVT_USER_DATA_VOS = 0,
        UVT_USER_DATA_VISO,
        UVT_USER_DATA_VOL,
        UVT_USER_DATA_GOV
    } UVTSwDecUserDataType;
    
    /* User data configuration */
    typedef struct {
        UVTSwDecUserDataType userDataType;
        u8  *pUserDataVOS; 
        u32  userDataVOSMaxLen;
        u8  *pUserDataVISO;
        u32  userDataVISOMaxLen;
        u8  *pUserDataVOL;
        u32  userDataVOLMaxLen;
        u8  *pUserDataGOV;
        u32  userDataGOVMaxLen;
    } UVTSwDecUserConf;
    
    /* Version information */
    typedef struct 
    {
        u32 major;    /* Decoder API major version */
        u32 minor;    /* Dncoder API minor version */
    } UVTSwDecApiVersion;

    typedef enum
    {
        UVTSWDEC_MPEG4_H263 = 0,
        UVTSWDEC_SORENSON_SPARK
    } UVTSwDecStreamFormat;

    typedef struct 
    {
        i32 width;        /* Maximum support width */
        i32 height;       /* Maximum support height */
        i32 size;         /* Maximum support size */
        u32 profile;      /* Maximum support profile */
        u32 intMemSize;   /* Required internal memory size */
        u32 extMemSize;   /* Required internal memory size */
        u32 isBsCacheable;/* is bitstream cacheable */
        u32 isFrameCacheable;/* is frame buffer cacheable */
        u32 supportMaxWidth; /* codec support max width on each platform */
    } UVTSwDecCapability;

    /* Profile */
    typedef enum
    {
        UVT_BASELINE = 0
    } UVTProfile;
    
    /* Scenario */
    typedef enum
    {
        VP = 0,
        VT = 1
    } UVTSwDecScenario;

#if 1 //#ifdef __VIDEO_ARCHI_V2__    
    /* Initial param */
    typedef struct 
    {
        u8 *intMemAddr; 
        u32 intMemSize;
        u32 sorensonSpark;
        UVTSwDecScenario scenario;
    #ifdef MP4_SHARED
        void (*funcReturnBitstream)(unsigned char *, unsigned int);
        unsigned char * (*funcMallocYUVBuffer)(void);
        int (*funcFreeYUVBuffer)(unsigned char *);
        void * (*funcMalloc)(unsigned int);
        void (*funcFree)(void *);
        void * (*funcMemset)(void *, int, unsigned int);
        void (*funcMemcpy)(void *, void *, unsigned int);
        int (*funcGetYUVStatus)(unsigned int *, unsigned int *);
        int (*funcGetBitRate)(unsigned int *);
        int (*funcGetFrameRate)(unsigned int *);
    #endif /* SHARED */
		void *hAdp;
        u32 u4IntraModeMaxNormalResolution;
        //u8  YUVFormat;                /* 0: I420, 1: YV12 */
        //u32 YStride;                  /* Y stride (in pixel) */
        //u32 UVStride;                 /* UV stride (in pixel) */
    } UVTSwDecInitParam;
    
    /* Return decode status */
    typedef struct
    {
        u8 *pOut;
        UVTSwDecRet status;
    } UVTSwDecReturnStatus;

    /* VOP type */
    typedef enum
    {
        UVTSWDEC_VOP_TYPE_I = 0, 
        UVTSWDEC_VOP_TYPE_P = 1
#ifndef MP4DEC_SORENSON_OFF
        ,UVTSWDEC_VOP_TYPE_D = 2 /* Disposable */
#endif
    } UVTSwDecVopType;
#endif /* VIDEO_ARCHI_V2 */
/*------------------------------------------------------------------------------
    4. Definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    5. Prototypes of Decoder API functions
------------------------------------------------------------------------------*/

    UVTSwDecRet UVTSwDecGetVopWidthHeight(UVTSwDecInst pDecInst, u32 *width, u32 *height);

    UVTSwDecRet UVTSwDecSetRefFrameBuffer(UVTSwDecInst pDecInst, u32 addr);

#if 1 //#ifdef __VIDEO_ARCHI_V2__
    UVTSwDecRet UVTSwDecInit(UVTSwDecInst *decInst, 
                             UVTSwDecInitParam *pInitParam);
    UVTSwDecRet UVTSwDecDecode(UVTSwDecInst     decInst, 
                               UVTSwDecInput    *pInput,
                               u32 isEOF);
    UVTSwDecRet UVTSwDecNextPicture(UVTSwDecInst decInst, 
                                    UVTSwDecOutput *pOutput);    
    UVTSwDecRet UVTSwDecQueryCapability(UVTSwDecCapability *pDecCap, 
                                        //UVTSwDecScenario scenario,
                                        u32 resolution);
	UVTSwDecRet UVTSwDecGetHeaderInfo(u8 *pStream, 
										u32 u4Length, 
										UVTSwDecInfo *prHeaderInfo);
    #ifndef MP4_SHARED
	extern void MP4VTSwDecReturnBitstream(void* hAdp, u8* pu1Addr, unsigned int u4Length);
    //extern void MP4SwDecReturnYUVBuffer(UVTSwDecReturnStatus *pStatus);
    extern int MP4VTSwDecGetYUVStatus(void* hAdp, unsigned int *pu4TotalYUVQueueNum, unsigned int *pu4WaitingForDisplayNum);
    extern int MP4VTSwDecGetBitRate(void* hAdp, unsigned int* pu4BitRate);
	extern int MP4VTSwDecGetFrameRate(void* hAdp, unsigned int* pu4FrameRate);

    extern unsigned char *MP4VTSwDecMallocYUVBuffer(void **hAdp);
    extern void MP4VTSwDecFreeYUVBuffer(void* hAdp, unsigned char* pu1Addr);
    extern void MP4VTSwDecGetCtrlVOS(void* hAdp, u32 *pu4CtrlVOSAddr, u32 *pu4CtrlVOSLength);
    #endif /* SHARED */
#else
    UVTSwDecRet UVTSwDecDecode(UVTSwDecInst     decInst, 
                               UVTSwDecInput    *pInput, 
                               UVTSwDecOutput   *pOutput);

    UVTSwDecRet UVTSwDecInit(UVTSwDecInst *decInst, u32 sorensonSpark);
#endif 

    UVTSwDecRet UVTSwDecGetInfo(UVTSwDecInst    decInst,
                                UVTSwDecInfo    *pDecInfo);

    void  UVTSwDecRelease(UVTSwDecInst decInst);

    UVTSwDecApiVersion UVTSwDecGetAPIVersion( void );

    UVTSwDecRet UVTSwDecGetQPs(UVTSwDecInst     pDecInst, 
                               UVTSwDecQpInfo   *pQpInfo);

    UVTSwDecRet UVTSwDecGetDeblockingInfo(UVTSwDecInst           pDecInst,
                                          UVTSwDecDeblockingInfo *pInfo);

    UVTSwDecRet UVTSwDecGetUserData(UVTSwDecInst        pDecInst,
                                    UVTSwDecUserConf    *pUserDataConfig);

    /* function prototype for API trace */
    void UVTSwDecTrace(char *);

    #ifndef MP4_SHARED
    /* function prototype for memory allocation */
    extern void *MP4VTSwDecMalloc(void* hAdp, unsigned int u4Size);

    /* function prototype for memory free*/
    extern void MP4VTSwDecFree(void *hAdp, void *pAddr);

    /* function prototype for memset */
    extern void *MP4VTSwDecMemset(void *ptr, int c, unsigned int);
    
    /* function prototype for memset */
    extern void MP4VTSwDecMemcpy(void *dest, void *src, unsigned int count);
    #endif /* SHARED */   
#ifdef __cplusplus
    }
#endif
    
#endif /* MP4SWDECAPI_H */

