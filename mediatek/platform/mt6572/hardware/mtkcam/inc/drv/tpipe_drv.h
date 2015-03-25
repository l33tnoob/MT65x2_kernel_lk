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

/**
* @file tpipe_drv.h
* @brief ISP tnuing 
*/
 
#ifndef _TPIPE_DRV_H_
#define _TPIPE_DRV_H_
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define TPIPE_DRV_MAX_TPIPE_TOT_NO              (128)   // copy from tpipe main
#define TPIPE_DRV_MAX_DUMP_HEX_PER_TPIPE        (72)    // copy from tpipe main
#define TPIPE_DRV_MAX_TPIPE_HEX_SIZE            (sizeof(int) * TPIPE_DRV_MAX_DUMP_HEX_PER_TPIPE * TPIPE_DRV_MAX_TPIPE_TOT_NO)   // copy from tpipe main
//
#define TPIPE_INFORMATION_STRUCT_SIZE       16 // ISP_TPIPE_INFORMATION_STRUCT
#define TPIPE_DRV_MAX_TPIPE_NUM               (TPIPE_DRV_MAX_TPIPE_TOT_NO)
#define TPIPE_DRV_MAX_TPIPE_HEX_SIZE          (sizeof(int) * TPIPE_DRV_MAX_DUMP_HEX_PER_TPIPE * TPIPE_DRV_MAX_TPIPE_TOT_NO)
#define TPIPE_DRV_MAX_TPIPE_CONF_SIZE         (TPIPE_INFORMATION_STRUCT_SIZE  * TPIPE_DRV_MAX_TPIPE_TOT_NO)


/////update for turning path
#define TPIPE_DRV_UPDATE_BNR        ( 1u << 0 )
#define TPIPE_DRV_UPDATE_LSC        ( 1u << 1 )
#define TPIPE_DRV_UPDATE_MFB        ( 1u << 2 )
#define TPIPE_DRV_UPDATE_CFA        ( 1u << 3 )
#define TPIPE_DRV_UPDATE_NBC        ( 1u << 4 )
#define TPIPE_DRV_UPDATE_SEEE       ( 1u << 5 )
#define TPIPE_DRV_UPDATE_LCE        ( 1u << 6 )
#define TPIPE_DRV_UPDATE_NR3D       ( 1u << 7 )
#define TPIPE_DRV_UPDATE_TURNING_MAX_NUM    (8)
/////update for cdp pipe
#define TPIPE_DRV_UPDATE_IMGI       ( 1u <<  8)
#define TPIPE_DRV_UPDATE_IMGCI      ( 1u <<  9)
#define TPIPE_DRV_UPDATE_VIPI       ( 1u << 10)
#define TPIPE_DRV_UPDATE_VIP2I      ( 1u << 11)
#define TPIPE_DRV_UPDATE_FLKI       ( 1u << 12)
#define TPIPE_DRV_UPDATE_LCEI       ( 1u << 13)
#define TPIPE_DRV_UPDATE_LSCI       ( 1u << 14)
#define TPIPE_DRV_UPDATE_IMGO       ( 1u << 15)
#define TPIPE_DRV_UPDATE_IMG2O      ( 1u << 16)
#define TPIPE_DRV_UPDATE_ESFKO      ( 1u << 17)
#define TPIPE_DRV_UPDATE_AAO        ( 1u << 18)
#define TPIPE_DRV_UPDATE_LCSO       ( 1u << 19)
#define TPIPE_DRV_UPDATE_VIDO       ( 1u << 20)
#define TPIPE_DRV_UPDATE_DISPO      ( 1u << 21)
#define TPIPE_DRV_UPDATE_MAX_NUM    (22)




/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

typedef enum
{
    TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE = 0, // complete update and save tdri parameter (CQ1)
    TPIPE_DRV_UPDATE_TYPE_CQ1_PARTIAL_SAVE,  // partial update and save tdri parameter (CQ1)
    TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE,  // partial update for turning path (CQ1)
    TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE,     // complete update and save tdri parameter (CQ2)
    TPIPE_DRV_UPDATE_TYPE_CQ2_PARTIAL_SAVE,  // partial update and save tdri parameter (CQ2)
    TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE,  // partial update for turning path (CQ2)
    TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE,     // complete update and save tdri parameter (CQ3)
    TPIPE_DRV_UPDATE_TYPE_CQ3_PARTIAL_SAVE,  // partial update and save tdri parameter (CQ3)
    TPIPE_DRV_UPDATE_TYPE_CQ3_TURNING_SAVE,  // partial update for turning path (CQ3)
    TPIPE_DRV_UPDATE_TYPE_FULL,              // complete update and do no save tdri parameter (for jpeg)
    TPIPE_DRV_UPDATE_TYPE_ALL
} TPIPE_DRV_UPDATE_TYPE;


typedef enum
{
    TPIPE_DRV_CQ01 = 0,
    TPIPE_DRV_CQ02,
    TPIPE_DRV_CQ03,
    TPIPE_DRV_CQ_NUM
}TPIPE_DRV_CQ_ENUM;


class TdriMemBuffer
{
public:
    unsigned int size;
    unsigned int cnt;
    unsigned int base_vAddr;
    unsigned int base_pAddr;
    unsigned int ofst_addr;
    unsigned int alignment;
public:
    TdriMemBuffer():
        size(0),cnt(0),base_vAddr(0),base_pAddr(0),ofst_addr(0), alignment(16)
        {};
};


class TdriRect
{
public:
    long            x;
    long            y;
    unsigned long   w;
    unsigned long   h;

public:
    TdriRect():
        x(0),y(0),w(0),h(0)
        {};

   TdriRect(long _x, long _y, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; w = _w; h = _h;
        };

};



class TdriBnrCfg
{
public:
    int bpc_en;
	int bpc_tbl_en;
    int bpc_tbl_size;/* bad pixel table width */
};

class TdriLscCfg
{
public:
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
};
class TdriLceCfg
{
public:
    int lce_bc_mag_kubnx;
    int lce_offset_x;
    int lce_bias_x;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_offset_y;
    int lce_bias_y;
    int lce_slm_height;
};
class TdriNbcCfg
{
public:
	int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
};
class TdriSeeeCfg
{
public:
    int se_edge;
    int usm_over_shrink_en;
};
class TdriImgoCfg
{
public:
    int imgo_stride;
    int imgo_crop_en;
};
class TdriEsfkoCfg
{
public:
    int esfko_stride;
};
class TdriAaoCfg
{
public:
    int aao_stride;
};
class TdriLcsoCfg
{
public:
    int lcso_stride;
    int lcso_crop_en;
};
class TdriCdrzCfg
{
public:
    int cdrz_input_crop_width;
    int cdrz_input_crop_height;
    int cdrz_output_width;
    int cdrz_output_height;
    int cdrz_horizontal_integer_offset;/* pixel base */
    int cdrz_horizontal_subpixel_offset;/* 20 bits base */
    int cdrz_vertical_integer_offset;/* pixel base */
    int cdrz_vertical_subpixel_offset;/* 20 bits base */
    int cdrz_horizontal_luma_algorithm;
    int cdrz_vertical_luma_algorithm;
    int cdrz_horizontal_coeff_step;
    int cdrz_vertical_coeff_step;
};
class TdriCurzCfg
{
public:
    int curz_input_crop_width;
    int curz_input_crop_height;
    int curz_output_width;
    int curz_output_height;
    int curz_horizontal_integer_offset;/* pixel base */
    int curz_horizontal_subpixel_offset;/* 20 bits base */
    int curz_vertical_integer_offset;/* pixel base */
    int curz_vertical_subpixel_offset;/* 20 bits base */
    int curz_horizontal_coeff_step;
    int curz_vertical_coeff_step;
};
class TdriFeCfg
{
public:
    int fem_harris_tpipe_mode;
};
class TdriImg2oCfg
{
public:
    int img2o_stride;
    int img2o_crop_en;
};
class TdriPrzCfg
{
public:
    int prz_output_width;
    int prz_output_height;
    int prz_horizontal_integer_offset;/* pixel base */
    int prz_horizontal_subpixel_offset;/* 20 bits base */
    int prz_vertical_integer_offset;/* pixel base */
    int prz_vertical_subpixel_offset;/* 20 bits base */
    int prz_horizontal_luma_algorithm;
    int prz_vertical_luma_algorithm;
    int prz_horizontal_coeff_step;
    int prz_vertical_coeff_step;
};

class TdriMfbCfg
{
public:
    int bld_mode;
    int bld_deblock_en;
};


class TdriFlkiCfg
{
public:
    int flki_stride;
};
class TdriCfaCfg
{
public:
    int bayer_bypass;
};

class TdriTopCfg
{
public:
    int scenario;
    int mode;
    int debug_sel;
    int pixel_id;
    int cam_in_fmt;
    int imgi_en;
    int imgci_en;
    int vipi_en;
    int vip2i_en;
    int flki_en;
    int lce_en;
    int lcei_en;
    int lsci_en;
    int unp_en;
    int bnr_en;
    int lsc_en;
    int mfb_en;
    int c02_en;
    int c24_en;
    int cfa_en;
    int c42_en;
    int nbc_en;
    int seee_en;
    int imgo_en;
    int img2o_en;
    int esfko_en;
    int aao_en;
    int lcso_en;
    int cdrz_en;
    int curz_en;
    int fe_sel;
    int fe_en;
    int prz_en;
    int disp_vid_sel;
    int g2g2_en;
    int vido_en;
    int dispo_en;
    int nr3d_en;
};

class TdriUpdateCfg
{
public:
    TPIPE_DRV_UPDATE_TYPE updateType;  //TPIPE_DRV_UPDATE_TYPE
    int partUpdateFlag;
};

class TdriPerformCfg
{
public:
    int tpipeWidth;
    int tpipeHeight;
    int irqMode;
};

class TdriDMACfg
{
public:
    int srcWidth;
    int srcHeight;
    int tpipeTabSize;
    int baseVa;
    int ringConfNumVa;
    int ringConfVerNumVa;
    int ringErrorControlVa;
    int ringConfBufVa;
    int ringBufferMcuRowNo;
    int ringBufferMcuHeight;
    //
    int isRunSegment; // for vss capture
    int setSimpleConfIdxNumVa;
    int segSimpleConfBufVa;
};

class TdriRingInDMACfg
{
public:
    int stride;
    int ring_en;
    int ring_size;
};

class TdriRingOutDMACfg
{
public:

    int  stride;
    int  stride_c;
    int  stride_v;
    int  format_1; /* DISPO_FORMAT_1 */
    int  format_3;  /* DISPO_FORMAT_3 */
    int  rotation;
    int  flip;
    int  crop_en;
};


class TdriTuningCfg
{
public:
    TdriBnrCfg bnr;
    TdriLscCfg lsc;
    TdriLceCfg lce;
    TdriNbcCfg nbc;
    TdriSeeeCfg seee;
    TdriMfbCfg mfb;
    TdriCfaCfg cfa;
    //nr3d
};

class TdriDrvCfg
{
public:
    //enable table
    TdriUpdateCfg updateTdri;
    TdriTopCfg top;
    TdriDMACfg tdriCfg;
    TdriPerformCfg  tdriPerformCfg;

    TdriRingInDMACfg imgi;
    TdriRingInDMACfg vipi;
    TdriRingInDMACfg vip2i;
    int imgci_stride;
    int lcei_stride;
    int lsci_stride;
    TdriRingOutDMACfg dispo;
    TdriRingOutDMACfg vido;
    TdriCdrzCfg cdrz;
    TdriCurzCfg curz;
    TdriPrzCfg prz;
    TdriImg2oCfg img2o;
    TdriImgoCfg imgo;
    TdriLcsoCfg lcso;
    TdriEsfkoCfg esfko;
    TdriAaoCfg aao;
    TdriFlkiCfg flki;
    TdriFeCfg fe;

    TdriTuningCfg tuningFunc;
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/


/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class TpipeDrv
{
    public:


    protected:
        TpipeDrv() {};
        virtual ~TpipeDrv() {};
    //
    public:
        static TpipeDrv*  createInstance(void);
        virtual void    destroyInstance(void) = 0;
        virtual int   init(void) = 0;
        virtual int   uninit(void) = 0;
        virtual int   configTdriPara(TdriDrvCfg* pTdriInfo) = 0;
        virtual MBOOL getNr3dTop(TPIPE_DRV_CQ_ENUM cq,MUINT32 *pEn) = 0;

};

//-----------------------------------------------------------------------------
#endif  // _TPIPE_DRV_H_

