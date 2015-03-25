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
#ifndef _CDP_DRV_IMP_H_
#define _CDP_DRV_IMP_H_
//-----------------------------------------------------------------------------
#include "cdp_drv.h"
//#include "imageio_log.h"


//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

//-----------------------------------------------------------------------------
#define CDP_DRV_ISP_DEV_NAME    "/dev/camera-isp"
#define CDP_DRV_INIT_MAX        5
#define CDP_DRV_BUF_PMEM        0
#define CDP_DRV_BUF_SYSRAM      1
#define CDP_DRV_BUF_SYSRAM_BANK0_SIZE       32768   // Bytes. 32KB. i.e. 0x8000.    // NEED_TUNING_BY_CHIP.
#define CDP_DRV_BUF_SYSRAM_BANK1_SIZE       49152   // Bytes. 48KB. i.e. 0xC000.    // NEED_TUNING_BY_CHIP.
#define CDP_DRV_BUF_SYSRAM_SIZE             (CDP_DRV_BUF_SYSRAM_BANK0_SIZE + CDP_DRV_BUF_SYSRAM_BANK1_SIZE)

#define CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START (0)
#define CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_END   (CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START + CDP_DRV_BUF_SYSRAM_BANK0_SIZE - 1)   // 0x7FFF.
#define CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_START (CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START + CDP_DRV_BUF_SYSRAM_BANK0_SIZE)       // 0x8000.
#define CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END   (CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK0_ADDR_START + CDP_DRV_BUF_SYSRAM_BANK0_SIZE + CDP_DRV_BUF_SYSRAM_BANK1_SIZE - 1) // 0x13FFF.

//-----------------------------------------------------------------------------
#define CDP_DRV_DEFAULT_CRSP_CTRL                   (0x00000003)
#define CDP_DRV_DEFAULT_DIT_INIT_X                  (0x0)
#define CDP_DRV_DEFAULT_DIT_INIT_Y                  (0x0)
#define CDP_DRV_DEFAULT_DIT_XRGB_DUMMY              (0xFF)
#define CDP_DRV_DEFAULT_DIT_SEED_R                  (0x3E8)
#define CDP_DRV_DEFAULT_DIT_SEED_G                  (0x7D0)
#define CDP_DRV_DEFAULT_DIT_SEED_B                  (0xBB8)
#define CDP_DRV_DEFAULT_DERING_THRESHOLD_1H         (0x4)
#define CDP_DRV_DEFAULT_DERING_THRESHOLD_1V         (0x6)
#define CDP_DRV_DEFAULT_DERING_THRESHOLD_2H         (0x8)
#define CDP_DRV_DEFAULT_DERING_THRESHOLD_2V         (0x100)
#define CDP_DRV_DEFAULT_TRUNC_BIT                   (0x0)
#define CDP_DRV_DEFAULT_SOF_RESET                   (0x0)
#define CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_Y     (0x0)
#define CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_U     (0x0)
#define CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_V     (0x0)
#define CDP_DRV_DEFAULT_FIFO_PRI_LOW_THR            (0x0)
#define CDP_DRV_DEFAULT_FIFO_PRI_THR                (0x0)
#define CDP_DRV_DEFAULT_ROTATION_BUF_LINE_NUM       (64)
//-----------------------------------------------------------------------------
#define CDP_DRV_MASK_TRUNC_BIT              (0x3)
#define CDP_DRV_MASK_TABLE_SELECT           (0x1F)
#define CDP_DRV_MASK_IMAGE_SIZE             (0x1FFF)
#define CDP_DRV_MASK_COEFF_STEP             (0x7FFFFF)
#define CDP_DRV_MASK_INT_OFFSET             (0x1FFF)
#define CDP_DRV_MASK_SUB_OFFSET             (0xFFFFF)
#define CDP_DRV_MASK_DERING_THRESHOLD_1     (0xF)
#define CDP_DRV_MASK_DERING_THRESHOLD_2     (0x1FF)
#define CDP_DRV_MASK_FIFO_PRI_LOW_THR       (0xFFF)
#define CDP_DRV_MASK_FIFO_PRI_THR           (0xFFF)
#define CDP_DRV_MASK_MAX_BURST_LEN          (0x1F)
#define CDP_DRV_MASK_BUF_LINE_NUM           (0x7F)
#define CDP_DRV_MASK_BUF_WIDTH              (0x1FFF)
#define CDP_DRV_MASK_BUF_SIZE               (0x1FFF)
#define CDP_DRV_MASK_ADDR_OFFSET            (0xFFFFFFF)
#define CDP_DRV_MASK_ADDR_STRIDE            (0xFFFF)
#define CDP_DRV_MASK_PADDING                (0xFF)
#define CDP_DRV_MASK_DIT_INIT               (0xF)
#define CDP_DRV_MASK_DIT_XRGB_DUMMY         (0xFF)
#define CDP_DRV_MASK_DIT_SEED               (0xFFFFF)
#define CDP_DRV_MASK_RSP_COEFF_STEP         (0x7)
#define CDP_DRV_MASK_RSP_OFFSET             (0x3)
//-----------------------------------------------------------------------------
#define CDP_DRV_YUV420_PLANE_2              (0)
#define CDP_DRV_YUV420_PLANE_3              (2)
//-----------------------------------------------------------------------------
#define CDP_DRV_RZ_4N_TAP_TABLE             (15)
#define CDP_DRV_RZ_N_TAP_TABLE              (0)
#define CDP_DRV_RZ_TABLE_AMOUNT             (6)
//
#define CDP_DRV_RZ_4_TAP_RATIO_MAX          (32)
#define CDP_DRV_RZ_4_TAP_RATIO_MIN          (2)     // For this definition, it really means the denominator of MinRatio (1/2).
//
#define CDP_DRV_RZ_4N_TAP_RATIO_MAX         (2)     // For this definition, it really means the denominator of MaxRatio (1/2).
#define CDP_DRV_RZ_4N_TAP_RATIO_MIN         (64)    // For this definition, it really means the denominator of MinRatio (1/64).
//
#define CDP_DRV_RZ_N_TAP_RATIO_MAX          (64)    // For this definition, it really means the denominator of MaxRatio (1/64).
#define CDP_DRV_RZ_N_TAP_RATIO_MIX          (256)   // For this definition, it really means the denominator of MinRatio (1/256).    //Vent@20120808: ~1/128 has no quality loss. 1/128~1/256 has quality loss (Confirmed by Joseph Lai).
//-----------------------------------------------------------------------------
// < Supporting Width >
//     CDRZ
#define CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_4_TAP  (2304)
#define CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_4N_TAP (1152)
#define CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_N_TAP  (2304)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_CDRZ_4_TAP  (768)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_CDRZ_4N_TAP (384)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_CDRZ_N_TAP  (768)
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_CDRZ_4_TAP  (65534)     // There is no supporting width limit for VR MRG mode CDRZ 4-tap/4n-tap/n-tap, so use a very large number to represent it.
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_CDRZ_4N_TAP (65534)
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_CDRZ_N_TAP  (65534)
//     CURZ
#define CDP_DRV_SUPPORT_WIDTH_FRAME_CURZ_4_TAP  (1920)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_CURZ_4_TAP  (65534)     // There is no supporting width limit for TPIPE mode CURZ 4-tap, so use a very large number to represent it.
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_CURZ_4_TAP  (65534)     // There is no supporting width limit for VR MRG mode CURZ 4-tap, so use a very large number to represent it.
//     PRZ
#define CDP_DRV_SUPPORT_WIDTH_FRAME_PRZ_4_TAP   (1280)
#define CDP_DRV_SUPPORT_WIDTH_FRAME_PRZ_4N_TAP  (640)
#define CDP_DRV_SUPPORT_WIDTH_FRAME_PRZ_N_TAP   (1280)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_PRZ_4_TAP   (768)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_PRZ_4N_TAP  (384)
#define CDP_DRV_SUPPORT_WIDTH_TPIPE_PRZ_N_TAP   (768)
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_PRZ_4_TAP   (4096)
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_PRZ_4N_TAP  (2048)
#define CDP_DRV_SUPPORT_WIDTH_VRMRG_PRZ_N_TAP   (4096)

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum
{
    CDP_DRV_BLOCK_SCAN_LINE,
    CDP_DRV_BLOCK_MTK
} CDP_DRV_BLOCK_ENUM;

typedef enum
{
    CDP_DRV_PADDING_MODE_8,
    CDP_DRV_PADDING_MODE_16
} CDP_DRV_PADDING_MODE_ENUM;

typedef enum
{
    CDP_DRV_UV_SELECT_EVEN,
    CDP_DRV_UV_SELECT_ODD,
    CDP_DRV_UV_SELECT_EVERY,
} CDP_DRV_UV_SELECT_ENUM;

typedef enum
{
    CDP_DRV_BURST_LEN_1 = 1,
    CDP_DRV_BURST_LEN_2 = 2,
    CDP_DRV_BURST_LEN_4 = 4,
    CDP_DRV_BURST_LEN_8 = 8
} CDP_DRV_BURST_LEN_ENUM;

typedef enum
{
    CDP_DRV_DIT_MODE_16X16,
    CDP_DRV_DIT_MODE_LFSR,
    CDP_DRV_DIT_MODE_16X16_LFSR
} CDP_DRV_DIT_MODE_ENUM;

typedef enum
{
    CDP_DRV_UV_FORMAT_YUV422,
    CDP_DRV_UV_FORMAT_YUV420,
    CDP_DRV_UV_FORMAT_YUV422V,
    CDP_DRV_UV_FORMAT_YUV444
} CDP_DRV_UV_FORMAT_ENUM;

typedef enum
{
    CDP_DRV_LC_LUMA,
    CDP_DRV_LC_CHROMA,
    CDP_DRV_LC_AMOUNT
} CDP_DRV_LC_ENUM;

typedef struct
{
    MINT32      Fd;
    MUINT32     VirAddr;
    MUINT32     PhyAddr;
    MUINT32     Size;
} CDP_DRV_BUF_STRUCT;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class CdpDrvImp : public CdpDrv
{
    protected:
        CdpDrvImp();
        ~CdpDrvImp();
    //
    public:
        static CdpDrv *GetInstance(MBOOL fgIsGdmaMode = MFALSE);
        virtual MVOID   DestroyInstance();
        virtual MBOOL  Init();
        virtual MBOOL  Uninit();
        virtual MBOOL  SetIspReg(isp_reg_t *pIspReg);
        virtual MBOOL  SetPhyIspDrv(IspDrv *pPhyIspDrv);
        virtual MBOOL  CheckReady();
//        virtual MBOOL   CalAlgoAndCStep(
//            CDP_DRV_RZ_ENUM         eRzName,
//            MUINT32                 SizeIn,
//            MUINT32                 u4CroppedSize,
//            MUINT32                 SizeOut,
//            CDP_DRV_ALGO_ENUM       *pAlgo,
//            MUINT32                 *pTable,
//            MUINT32                 *pCoeffStep);
        virtual MBOOL   CalAlgoAndCStep(
            CDP_DRV_MODE_ENUM eFrameOrTpipeOrVrmrg,
            CDP_DRV_RZ_ENUM   eRzName,
            MUINT32           SizeIn_H,
            MUINT32           SizeIn_V,
            MUINT32           u4CroppedSize_H,
            MUINT32           u4CroppedSize_V,
            MUINT32           SizeOut_H,
            MUINT32           SizeOut_V,
            CDP_DRV_ALGO_ENUM *pAlgo_H,
            CDP_DRV_ALGO_ENUM *pAlgo_V,
            MUINT32           *pTable_H,
            MUINT32           *pTable_V,
            MUINT32           *pCoeffStep_H,
            MUINT32           *pCoeffStep_V);
        virtual MBOOL   CalOffset(
            CDP_DRV_ALGO_ENUM Algo,
            MBOOL   IsWidth,
            MUINT32 CoeffStep,
            MFLOAT  Offset,
            MUINT32 *pLumaInt,
            MUINT32 *pLumaSub,
            MUINT32 *pChromaInt,
            MUINT32 *pChromaSub);
        virtual MBOOL   AllocateRotationBuf();
        virtual MBOOL   FreeRotationBuf();
        virtual MBOOL   RecalculateRotationBufAddr(
            CDP_DRV_ROTDMA_ENUM     RotDma,
            MUINT32                 LumaBufSize,
            MUINT32                 ChromaBufSize);
        virtual MBOOL   Reset();
        virtual MBOOL   ResetDefault();
        virtual MBOOL   DumpReg();
        virtual MBOOL   RotDmaEnumRemapping(
            CDP_DRV_FORMAT_ENUM eInFormat,
            CDP_DRV_PLANE_ENUM eInPlane,
            MUINT32 *pu4OutPlane);
        virtual MBOOL   InputImgFormatCheck(
            CDP_DRV_FORMAT_ENUM     eInFormat,
            CDP_DRV_PLANE_ENUM      eInPlane,
            CDP_DRV_SEQUENCE_ENUM   eInSequence);
        //CDRZ
        virtual MBOOL   CDRZ_Enable(MBOOL En);
        virtual MBOOL   CDRZ_ResetDefault();
        virtual MBOOL   CDRZ_DumpReg();
        virtual MBOOL   CDRZ_H_EnableScale(MBOOL En);
        virtual MBOOL   CDRZ_V_EnableScale(MBOOL En);
        virtual MBOOL   CDRZ_V_EnableFirst(MBOOL En);
        virtual MBOOL   CDRZ_H_SetAlgo(CDP_DRV_ALGO_ENUM Algo);
        virtual MBOOL   CDRZ_V_SetAlgo(CDP_DRV_ALGO_ENUM Algo);
#if 0   //js_test remove below later
        virtual MBOOL   CDRZ_EnableDering(MBOOL En);
        virtual MBOOL   CDRZ_H_SetDeringThreshold(
            MUINT32     Threshold1,
            MUINT32     Threshold2);
        virtual MBOOL   CDRZ_V_SetDeringThreshold(
            MUINT32     Threshold1,
            MUINT32     Threshold2);
#endif  //js_test remove below later
        virtual MBOOL   CDRZ_H_SetTruncBit(MUINT32 Bit);
        virtual MBOOL   CDRZ_V_SetTruncBit(MUINT32 Bit);
        virtual MBOOL   CDRZ_H_SetTable(MUINT32 Table);
        virtual MBOOL   CDRZ_V_SetTable(MUINT32 Table);
        virtual MBOOL   CDRZ_H_SetInputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_V_SetInputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   CDRZ_H_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   CDRZ_V_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   CDRZ_H_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   CDRZ_V_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   CDRZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop);
        virtual MBOOL   CDRZ_Unconfig();
        //CURZ
        virtual MBOOL   CURZ_Enable(MBOOL En);
        virtual MBOOL   CURZ_ResetDefault();
        virtual MBOOL   CURZ_DumpReg();
        virtual MBOOL   CURZ_H_EnableScale(MBOOL En);
        virtual MBOOL   CURZ_V_EnableScale(MBOOL En);
#if 0   //js_test remove below later
        virtual MBOOL   CURZ_EnableDering(MBOOL En);
        virtual MBOOL   CURZ_H_SetDeringThreshold(
            MUINT32     Threshold1,
            MUINT32     Threshold2);
        virtual MBOOL   CURZ_V_SetDeringThreshold(
            MUINT32     Threshold1,
            MUINT32     Threshold2);
#endif  //js_test remove below later
        virtual MBOOL   CURZ_H_SetTable(MUINT32 Table);
        virtual MBOOL   CURZ_V_SetTable(MUINT32 Table);
        virtual MBOOL   CURZ_H_SetInputSize(MUINT32 Size);
        virtual MBOOL   CURZ_V_SetInputSize(MUINT32 Size);
        virtual MBOOL   CURZ_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   CURZ_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   CURZ_H_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   CURZ_V_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   CURZ_H_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   CURZ_V_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   CURZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop);
        virtual MBOOL   CURZ_Unconfig();

#if 0 //js_test remove below later
        //VRZ
        virtual MBOOL   VRZ_Enable(MBOOL En);
        virtual MBOOL   VRZ_ResetDefault(void);
        virtual MBOOL   VRZ_DumpReg(void);
        virtual MBOOL   VRZ_H_EnableScale(MBOOL En);
        virtual MBOOL   VRZ_V_EnableScale(MBOOL En);
        virtual MBOOL   VRZ_H_SetInputSize(MUINT32 Size);
        virtual MBOOL   VRZ_V_SetInputSize(MUINT32 Size);
        virtual MBOOL   VRZ_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   VRZ_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   VRZ_H_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   VRZ_V_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   VRZ_H_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   VRZ_V_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   VRZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop);
        virtual MBOOL   VRZ_Unconfig(void);
#endif //js_test remove below later

#if 0 //js_test remove below later
        //RSP
        virtual MBOOL   RSP_Enable(MBOOL En);
        virtual MBOOL   RSP_ResetDefault(void);
        virtual MBOOL   RSP_DumpReg(void);
        virtual MBOOL   RSP_SetFormat(CDP_DRV_UV_FORMAT_ENUM Format);
        virtual MBOOL   RSP_SetInterrupt(
            MBOOL       En,
            MBOOL       WriteClear);
        virtual MBOOL   RSP_GetInterruptStatus(void);
        virtual MBOOL   RSP_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   RSP_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   RSP_H_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   RSP_V_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   RSP_H_SetOffset(MUINT32 Offset);
        virtual MBOOL   RSP_V_SetOffset(MUINT32 Offset);

#endif //js_test remove below later

        //PRZ
        virtual MBOOL   PRZ_Enable(MBOOL En);
        virtual MBOOL   PRZ_SetSource(CDP_DRV_PRZ_SRC_ENUM Source);
        virtual MBOOL   PRZ_ResetDefault();
        virtual MBOOL   PRZ_DumpReg();
        virtual MBOOL   PRZ_H_EnableScale(MBOOL En);
        virtual MBOOL   PRZ_V_EnableScale(MBOOL En);
        virtual MBOOL   PRZ_V_EnableFirst(MBOOL En);
        virtual MBOOL   PRZ_H_SetAlgo(CDP_DRV_ALGO_ENUM Algo);
        virtual MBOOL   PRZ_V_SetAlgo(CDP_DRV_ALGO_ENUM Algo);
#if 0   //js_test remove below later
        virtual MBOOL   PRZ_EnableDering(MBOOL En);
        virtual MBOOL   PRZ_H_SetDeringThreshold(
            MUINT32     Threshold1,
            MUINT32     Threshold2);
        virtual MBOOL   PRZ_V_SetDeringThreshold(
            MUINT32     Threshold1,
            MUINT32     Threshold2);
#endif  //js_test remove below later
        virtual MBOOL   PRZ_H_SetTruncBit(MUINT32 Bit);
        virtual MBOOL   PRZ_V_SetTruncBit(MUINT32 Bit);
        virtual MBOOL   PRZ_H_SetTable(MUINT32 Table);
        virtual MBOOL   PRZ_V_SetTable(MUINT32 Table);
        virtual MBOOL   PRZ_H_SetInputSize(MUINT32 Size);
        virtual MBOOL   PRZ_V_SetInputSize(MUINT32 Size);
        virtual MBOOL   PRZ_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   PRZ_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   PRZ_H_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   PRZ_V_SetCoeffStep(MUINT32 CoeffStep);
        virtual MBOOL   PRZ_H_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   PRZ_V_SetOffset(
            MUINT32     LumaInt,
            MUINT32     LumaSub,
            MUINT32     ChromaInt,
            MUINT32     ChromaSub);
        virtual MBOOL   PRZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop);
        virtual MBOOL   PRZ_Unconfig();

#if 0 //js_test remove below later
        //VRZO
        virtual MBOOL   VRZO_Enable(MBOOL En);
        virtual MBOOL   VRZO_ResetDefault(void);
        virtual MBOOL   VRZO_DumpReg(void);
        virtual MBOOL   VRZO_SetFormat(
            CDP_DRV_FORMAT_ENUM     Format,
            CDP_DRV_BLOCK_ENUM      Block,
            CDP_DRV_PLANE_ENUM      Plane,
            CDP_DRV_SEQUENCE_ENUM   Sequence);
        virtual MBOOL   VRZO_EnableCrop(MBOOL En);
        virtual MBOOL   VRZO_H_SetCropOffset(MUINT32 Offset);
        virtual MBOOL   VRZO_V_SetCropOffset(MUINT32 Offset);
        virtual MBOOL   VRZO_SetPaddingMode(CDP_DRV_PADDING_MODE_ENUM PaddingMode);
        virtual MBOOL   VRZO_SetRotation(CDP_DRV_ROTATION_ENUM Rotation);
        virtual MBOOL   VRZO_SetRotationBuf(
            MBOOL       FIFO,
            MBOOL       DoubleBuf,
            MUINT32     LineNum,
            MUINT32     Width,
            MUINT32     Size);
        virtual MBOOL   VRZO_SetRotationBufAddr(
            MUINT32     LumaAddr0,
            MUINT32     LumaAddr1,
            MUINT32     ChromaAddr0,
            MUINT32     ChromaAddr1);
        virtual MBOOL   VRZO_EnableFlip(MBOOL En);
        virtual MBOOL   VRZO_EnableSOFReset(MBOOL En);
        virtual MBOOL   VRZO_H_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV);
        virtual MBOOL   VRZO_V_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV);
        virtual MBOOL   VRZO_SetDMAPerf(
            MUINT32                 PriLowThr,
            MUINT32                 PriThr,
            CDP_DRV_BURST_LEN_ENUM  BurstLen);
        virtual MBOOL   VRZO_EnableReset(MBOOL En);
        virtual MBOOL   VRZO_GetResetStatus(void);
        virtual MBOOL   VRZO_SetInterrupt(
            MBOOL       En,
            MBOOL       WriteClear);
        virtual MBOOL   VRZO_GetInterruptStatus(void);
        virtual MBOOL   VRZO_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   VRZO_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   VRZO_SetOutputAddr(
            MUINT32     PhyAddr,
            MUINT32     Offset,
            MUINT32     Stride,
            MUINT32     PhyAddrC,
            MUINT32     OffsetC,
            MUINT32     StrideC);
        virtual MBOOL   VRZO_SetCamera2DispPadding(
            MUINT32     Y,
            MUINT32     U,
            MUINT32     V);
        virtual MBOOL   VRZO_SetDithering(
            MBOOL                   En,
            CDP_DRV_DIT_MODE_ENUM   Mode,
            MUINT32                 InitX,
            MUINT32                 InitY,
            MUINT32                 XRGBDummy);
        virtual MBOOL   VRZO_SetDitheringSeed(
            MUINT32     R,
            MUINT32     G,
            MUINT32     B);
        virtual MBOOL   VRZO_Config(
            CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
            CDP_DRV_IMG_CROP_STRUCT     Crop,
            CDP_DRV_FORMAT_ENUM         Format,
            CDP_DRV_PLANE_ENUM          Plane,
            CDP_DRV_SEQUENCE_ENUM       Sequence,
            CDP_DRV_ROTATION_ENUM       Rotation,
            MBOOL                       Flip);
        virtual MBOOL   VRZO_Unconfig(void);
#endif //js_test remove below later

        //VIDO
        virtual MBOOL   VIDO_Enable(MBOOL En);
        virtual MBOOL   VIDO_ResetDefault();
        virtual MBOOL   VIDO_DumpReg();
        virtual MBOOL   VIDO_SetFormat(
            CDP_DRV_FORMAT_ENUM     Format,
            CDP_DRV_BLOCK_ENUM      Block,
            CDP_DRV_PLANE_ENUM      Plane,
            CDP_DRV_SEQUENCE_ENUM   Sequence);
        virtual MBOOL   VIDO_EnableCrop(MBOOL En);
        virtual MBOOL   VIDO_H_SetCropOffset(MUINT32 Offset);
        virtual MBOOL   VIDO_V_SetCropOffset(MUINT32 Offset);
        virtual MBOOL   VIDO_SetPaddingMode(CDP_DRV_PADDING_MODE_ENUM PaddingMode);
        virtual MBOOL   VIDO_SetRotation(CDP_DRV_ROTATION_ENUM Rotation);

        virtual MBOOL   VIDO_RotationBufConfig(
            CDP_DRV_FORMAT_ENUM eFormat,
            CDP_DRV_PLANE_ENUM ePlane,
            MUINT32 u4TpipeWidth,
            MBOOL   fgFifoMode,
            MBOOL   fgDoubleBufMode,
            MUINT32 *pu4LumaBufSize,
            MUINT32 *pu4ChromaBufSize);
        virtual MBOOL   VIDO_SetRotationBufAddr(
            MUINT32     LumaAddr0,
            MUINT32     LumaAddr1,
            MUINT32     ChromaAddr0,
            MUINT32     ChromaAddr1);
        virtual MBOOL   VIDO_EnableFlip(MBOOL En);
        virtual MBOOL   VIDO_EnableSOFReset(MBOOL En);
        virtual MBOOL   VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV);
        virtual MBOOL   VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV);
        virtual MBOOL   VIDO_SetDMAPerf(
            MUINT32                 PriLowThr,
            MUINT32                 PriThr,
            CDP_DRV_BURST_LEN_ENUM  BurstLen);
        virtual MBOOL   VIDO_EnableReset(MBOOL En);
        virtual MBOOL   VIDO_GetResetStatus();
        virtual MBOOL   VIDO_SetInterrupt(
            MBOOL       En,
            MBOOL       WriteClear);
        virtual MBOOL   VIDO_GetInterruptStatus();
        virtual MBOOL   VIDO_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   VIDO_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   VIDO_SetOutputAddr(
            MUINT32     PhyAddr,
            MUINT32     Offset,
            MUINT32     Stride,
            MUINT32     PhyAddrC,
            MUINT32     OffsetC,
            MUINT32     StrideC,
            MUINT32     PhyAddrV,
            MUINT32     OffsetV,
            MUINT32     StrideV);
        virtual MBOOL   VIDO_SetCamera2DispPadding(
            MUINT32     Y,
            MUINT32     U,
            MUINT32     V);
        virtual MBOOL   VIDO_SetDithering(
            MBOOL                   En,
            CDP_DRV_DIT_MODE_ENUM   Mode,
            MUINT32                 InitX,
            MUINT32                 InitY,
            MUINT32                 XRGBDummy);
        virtual MBOOL   VIDO_SetDitheringSeed(
            MUINT32     R,
            MUINT32     G,
            MUINT32     B);
        virtual MBOOL   VIDO_Config(
            CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
            CDP_DRV_IMG_CROP_STRUCT     Crop,
            CDP_DRV_FORMAT_ENUM         Format,
            CDP_DRV_PLANE_ENUM          Plane,
            CDP_DRV_SEQUENCE_ENUM       Sequence,
            CDP_DRV_ROTATION_ENUM       Rotation,
            MBOOL                       Flip,
            MUINT32                     u4TpipeWidth,
            MBOOL                       fgDitherEnable);
        virtual MBOOL   VIDO_Unconfig();
        //DISPO
        virtual MBOOL   DISPO_Enable(MBOOL En);
        virtual MBOOL   DISPO_SetSource(CDP_DRV_DISPO_SRC_ENUM Source);
        virtual MBOOL   DISPO_ResetDefault();
        virtual MBOOL   DISPO_DumpReg();
        virtual MBOOL   DISPO_SetFormat(
            CDP_DRV_FORMAT_ENUM     Format,
            CDP_DRV_BLOCK_ENUM      Block,
            CDP_DRV_PLANE_ENUM      Plane,
            CDP_DRV_SEQUENCE_ENUM   Sequence);
        virtual MBOOL   DISPO_EnableCrop(MBOOL En);
        virtual MBOOL   DISPO_H_SetCropOffset(MUINT32 Offset);
        virtual MBOOL   DISPO_V_SetCropOffset(MUINT32 Offset);
        virtual MBOOL   DISPO_SetPaddingMode(CDP_DRV_PADDING_MODE_ENUM PaddingMode);
        virtual MBOOL   DISPO_SetRotation(CDP_DRV_ROTATION_ENUM Rotation);
        virtual MBOOL   DISPO_RotationBufConfig(
            CDP_DRV_FORMAT_ENUM eFormat,
            CDP_DRV_PLANE_ENUM ePlane,
            MUINT32 u4TpipeWidth,
            MBOOL   fgFifoMode,
            MBOOL   fgDoubleBufMode,
            MUINT32 *pu4LumaBufSize,
            MUINT32 *pu4ChromaBufSize);
        virtual MBOOL   DISPO_SetRotationBufAddr(
            MUINT32     LumaAddr0,
            MUINT32     LumaAddr1,
            MUINT32     ChromaAddr0,
            MUINT32     ChromaAddr1);
        virtual MBOOL   DISPO_EnableFlip(MBOOL En);
        virtual MBOOL   DISPO_EnableSOFReset(MBOOL En);
        virtual MBOOL   DISPO_H_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV);
        virtual MBOOL   DISPO_V_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV);
        virtual MBOOL   DISPO_SetDMAPerf(
            MUINT32                 PriLowThr,
            MUINT32                 PriThr,
            CDP_DRV_BURST_LEN_ENUM  BurstLen);
        virtual MBOOL   DISPO_EnableReset(MBOOL En);
        virtual MBOOL   DISPO_GetResetStatus();
        virtual MBOOL   DISPO_SetInterrupt(
            MBOOL       En,
            MBOOL       WriteClear);
        virtual MBOOL   DISPO_GetInterruptStatus();
        virtual MBOOL   DISPO_H_SetOutputSize(MUINT32 Size);
        virtual MBOOL   DISPO_V_SetOutputSize(MUINT32 Size);
        virtual MBOOL   DISPO_SetOutputAddr(
            MUINT32     PhyAddr,
            MUINT32     Offset,
            MUINT32     Stride,
            MUINT32     PhyAddrC,
            MUINT32     OffsetC,
            MUINT32     StrideC,
            MUINT32     PhyAddrV,
            MUINT32     OffsetV,
            MUINT32     StrideV);
        virtual MBOOL   DISPO_SetCamera2DispPadding(
            MUINT32     Y,
            MUINT32     U,
            MUINT32     V);
        virtual MBOOL   DISPO_SetDithering(
            MBOOL                   En,
            CDP_DRV_DIT_MODE_ENUM   Mode,
            MUINT32                 InitX,
            MUINT32                 InitY,
            MUINT32                 XRGBDummy);
        virtual MBOOL   DISPO_SetDitheringSeed(
            MUINT32     R,
            MUINT32     G,
            MUINT32     B);
        virtual MBOOL   DISPO_Config(
            CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
            CDP_DRV_IMG_CROP_STRUCT     Crop,
            CDP_DRV_FORMAT_ENUM         Format,
            CDP_DRV_PLANE_ENUM          Plane,
            CDP_DRV_SEQUENCE_ENUM       Sequence,
            CDP_DRV_ROTATION_ENUM       Rotation,
            MBOOL                       Flip,
            MBOOL                       fgDitherEnable);
        virtual MBOOL   DISPO_Unconfig();
    //
    private:
        mutable Mutex       mLock;
        volatile MINT32     mInitCount; // CDP Drv instance count.
        volatile MINT32     mSysramUsageCount; // SYSRAM Buffer usage count. Record how many users are using SYSRAM.
        SYSRAM_ALLOC_STRUCT SysramAlloc;
        CDP_DRV_BUF_STRUCT  mRotationBuf[CDP_DRV_ROTDMA_AMOUNT][CDP_DRV_LC_AMOUNT];
        isp_reg_t *mpIspReg;
        IspDrv *m_pPhyIspDrv;
        isp_reg_t *m_pPhyIspReg;

        MINT32 mFdSysram;
        MBOOL m_fgIsGdmaMode;


};

//-----------------------------------------------------------------------------
#endif  // _CDP_DRV_IMP_H_

