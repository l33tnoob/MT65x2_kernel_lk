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
#define TPIPE_DRV_MAX_TPIPE_TOT_NO              (128)  ///< N.A.
#define TPIPE_DRV_MAX_DUMP_HEX_PER_TPIPE        (72)   ///< N.A.
#define TPIPE_DRV_MAX_TPIPE_HEX_SIZE            (sizeof(int) * TPIPE_DRV_MAX_DUMP_HEX_PER_TPIPE * TPIPE_DRV_MAX_TPIPE_TOT_NO)   // N.A.
//
#define TPIPE_INFORMATION_STRUCT_SIZE       16 ///< N.A.
#define TPIPE_DRV_MAX_TPIPE_NUM               (TPIPE_DRV_MAX_TPIPE_TOT_NO)///< N.A.
#define TPIPE_DRV_MAX_TPIPE_HEX_SIZE          (sizeof(int) * TPIPE_DRV_MAX_DUMP_HEX_PER_TPIPE * TPIPE_DRV_MAX_TPIPE_TOT_NO)///< N.A.
#define TPIPE_DRV_MAX_TPIPE_CONF_SIZE         (TPIPE_INFORMATION_STRUCT_SIZE  * TPIPE_DRV_MAX_TPIPE_TOT_NO)///< N.A.


/////update for turning path
#define TPIPE_DRV_UPDATE_BNR        ( 1u << 0 ) ///< update BNR  tuning table to CQ from featureio
#define TPIPE_DRV_UPDATE_LSC        ( 1u << 1 )///< update LSC  tuning table to CQ  
#define TPIPE_DRV_UPDATE_MFB        ( 1u << 2 ) ///< No function
#define TPIPE_DRV_UPDATE_CFA        ( 1u << 3 ) ///< update CFA  tuning table to CQ
#define TPIPE_DRV_UPDATE_NBC        ( 1u << 4 ) ///< update NBC  tuning table to CQ
#define TPIPE_DRV_UPDATE_SEEE       ( 1u << 5 ) ///< update SEEE  tuning table to CQ 
#define TPIPE_DRV_UPDATE_LCE        ( 1u << 6 ) ///< No function
#define TPIPE_DRV_UPDATE_NR3D       ( 1u << 7 ) ///< No function
#define TPIPE_DRV_UPDATE_SL2        ( 1u << 8 ) ///< update SL2  tuning table to CQ 
#define TPIPE_DRV_UPDATE_TURNING_MAX_NUM    (9) ///< Maximum tuning function number 
/////update for cdp pipe
#define TPIPE_DRV_UPDATE_IMGI       ( 1u <<  9) ///< update IMGI settings to CQ from Imageio
#define TPIPE_DRV_UPDATE_IMGCI      ( 1u << 10) ///< No function
#define TPIPE_DRV_UPDATE_VIPI       ( 1u << 11) ///< No function
#define TPIPE_DRV_UPDATE_VIP2I      ( 1u << 12) ///< No function
#define TPIPE_DRV_UPDATE_FLKI       ( 1u << 13) ///< No function
#define TPIPE_DRV_UPDATE_LCEI       ( 1u << 14) ///< No function
#define TPIPE_DRV_UPDATE_LSCI       ( 1u << 15) ///< update LSCI settings to CQ from Imageio
#define TPIPE_DRV_UPDATE_IMGO       ( 1u << 16) ///< update IMGO settings to CQ from Imageio
#define TPIPE_DRV_UPDATE_IMG2O      ( 1u << 17) ///< update IMG2O settings to CQ from Imageio
#define TPIPE_DRV_UPDATE_ESFKO      ( 1u << 18)///<  No function
#define TPIPE_DRV_UPDATE_AAO        ( 1u << 19) ///< No function
#define TPIPE_DRV_UPDATE_LCSO       ( 1u << 20) ///< No function
#define TPIPE_DRV_UPDATE_VIDO       ( 1u << 21) ///< No function
#define TPIPE_DRV_UPDATE_DISPO      ( 1u << 22) ///< No function
#define TPIPE_DRV_UPDATE_MAX_NUM    (23)        ///< Maximum tpipe function number  




/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
/**  
 * @brief update type for isp tuning and tdri parameter
 */

typedef enum
{
    TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE = 0, ///<  complete update and save tdri parameter (CQ1)
    TPIPE_DRV_UPDATE_TYPE_CQ1_PARTIAL_SAVE,  ///< partial update and save tdri parameter (CQ1)
    TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE,  ///< partial update for turning path (CQ1)
    TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE,     ///< complete update and save tdri parameter (CQ2)
    TPIPE_DRV_UPDATE_TYPE_CQ2_PARTIAL_SAVE,  ///< partial update and save tdri parameter (CQ2)
    TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE,  ///< partial update for turning path (CQ2)
    TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE,     ///< complete update and save tdri parameter (CQ3)
    TPIPE_DRV_UPDATE_TYPE_CQ3_PARTIAL_SAVE,  ///< partial update and save tdri parameter (CQ3)
    TPIPE_DRV_UPDATE_TYPE_CQ3_TURNING_SAVE,  ///< partial update for turning path (CQ3)
    TPIPE_DRV_UPDATE_TYPE_FULL,              ///< complete update and do no save tdri parameter (for jpeg)
    TPIPE_DRV_UPDATE_TYPE_ALL                ///< Maximum number of enum 
} TPIPE_DRV_UPDATE_TYPE;

/**  
 * @brief enum of CQ for tdri
 */

typedef enum
{
    TPIPE_DRV_CQ01 = 0,///< CQ1 for tdri 
    TPIPE_DRV_CQ02,    //< CQ2 for tdri  
    TPIPE_DRV_CQ03,    //< CQ3 for tdri
    TPIPE_DRV_CQ_NUM   //< CQ number for tdri
}TPIPE_DRV_CQ_ENUM;

/*
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
*/
/*
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
*/

/**  
 * @brief BNR configuration for tdri
 */

class TdriBnrCfg
{
public:
    int bpc_en; ///< bpc enable
	//int bpc_tbl_en;
    //int bpc_tbl_size;/* bad pixel table width */
};

/**  
 * @brief LSC configuration for tdri
 */

class TdriLscCfg
{
public:
    int sdblk_width;       ///< sdblk_width block width in x direction
    int sdblk_xnum;        ///< sdblk_xnum x directional number of block
    int sdblk_last_width;  ///< sdblk_last_width last block width
    int sdblk_height;      ///< sdblk_height block height in y direction
    int sdblk_ynum;        ///< sdblk_ynum block width in x direction
    int sdblk_last_height;///< sdblk_last_height last block height
};
/*class TdriLceCfg
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
};*/

/**  
 * @brief NBC configuration for tdri
 */
class TdriNbcCfg
{	
public:
	//int anr_en;
	int anr_eny;       ///< anr_eny Enable Y ANR
    int anr_enc;       ///< anr_enc Enable C ANR 
    int anr_iir_mode;  ///< anr_iir_mode In tdri mode, no IIR
    int anr_scale_mode;///< anr_scale_mode Different in tdri mode and frame mode
};

/**  
 * @brief SEEE configuration for tdri
 */

class TdriSeeeCfg
{
public:
    int se_edge;  ///< se_edge Select source of edge enhancement
    //int usm_over_shrink_en;
};

/**  
 * @brief IMGO configuration for tdri
 */

class TdriImgoCfg
{	
public:
    int imgo_stride;  ///< imgo stride
    int imgo_crop_en; ///< imgo crop enable
	int imgo_xoffset; ///< imgo offset in x direction
	int imgo_yoffset; ///< imgo offset in y direction
	int imgo_xsize;   ///< imgo x size
	int imgo_ysize;   ///< imgo y size
	int imgo_mux_en;  ///< imgo mux enable
	int imgo_mux;     ///< imgo mux
};
/*class TdriEsfkoCfg
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
};*/

/**  
 * @brief CDRZ configuration for tdri
 */

class TdriCdrzCfg
{
public:
    int cdrz_input_crop_width;           ///< cdrz input cropping width
    int cdrz_input_crop_height;          ///< cdrz input cropping height
    int cdrz_output_width;               ///< cdrz output width
    int cdrz_output_height;              ///< cdrz output height
    int cdrz_horizontal_integer_offset;  ///< integer offset in horizontal direction for luma 
    int cdrz_horizontal_subpixel_offset;///< sub-pixel offset in horizontal direction for luma
    int cdrz_vertical_integer_offset;    ///<  integer offset in vertical direction for luma 
    int cdrz_vertical_subpixel_offset;   ///< sub-pixel offset in vertical direction for luma 
    int cdrz_horizontal_luma_algorithm;  ///< horizontal luma algorithm
    int cdrz_vertical_luma_algorithm;    ///< vertical luma algorithm 
    int cdrz_horizontal_coeff_step;      ///< horizontal coefficience step
    int cdrz_vertical_coeff_step;        ///< vertical coefficience step
};
/*class TdriCurzCfg
{
public:
    int curz_input_crop_width;
    int curz_input_crop_height;
    int curz_output_width;
    int curz_output_height;
    int curz_horizontal_integer_offset;
    int curz_horizontal_subpixel_offset;
    int curz_vertical_integer_offset;
    int curz_vertical_subpixel_offset;
    int curz_horizontal_coeff_step;
    int curz_vertical_coeff_step;
};
class TdriFeCfg
{
public:
    int fem_harris_tpipe_mode;
};*/

/**  
 * @brief IMG2O configuration for tdri
 */
class TdriImg2oCfg
{
public:
    int img2o_stride;  ///< img2o stride
    int img2o_crop_en; ///< img2o crop enable	
    int img2o_xoffset; ///< img2o offset in x direction
    int img2o_yoffset; ///< img2o offset in y direction
    int img2o_xsize;   ///< img2o x size
    int img2o_ysize;   ///< img2o y size
    int img2o_mux_en;  ///< img2o mux enable
    int img2o_mux;     ///< img2o mux
};
/*class TdriPrzCfg
{
public:
    int prz_output_width;
    int prz_output_height;
    int prz_horizontal_integer_offset;
    int prz_horizontal_subpixel_offset;
    int prz_vertical_integer_offset;
    int prz_vertical_subpixel_offset;
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
};*/

/**  
 * @brief CFA configuration for tdri
 */
class TdriCfaCfg
{
public:
    int bayer_bypass; ///< Bypass bayer to rgb process 
}; 

/**  
 * @brief top configuration for tdri
 */

class TdriTopCfg
{
public:
    int scenario;    ///< Isp pipeline scenario
    int mode;        ///< Isp pipeline sub mode
    int debug_sel;   ///< Debug mode selection for all scenarios 
    int pixel_id;    ///< Bayer sequence , it is for RAW and RGB module
    int cam_in_fmt;  ///< Pass2 path input format  
	int tcm_load_en; ///< TDR manual setting enable
    int ctl_extension_en;///< 0: normal,suggested
    int rsp_en;       ///< Resample tile enable, for direct link MDP use
    int mdp_crop_en;  ///< MDP corp enable
    int imgi_en;      ///< enable switch of IMGI
 //   int imgci_en;
  //  int vipi_en;
  //  int vip2i_en;
  //  int flki_en;
   // int lce_en;
   // int lcei_en;
    int lsci_en;     ///< enable switch of LSCI
    int unp_en;      ///< enable switch of unpacked
    int bnr_en;      ///< enable switch of BNR
    int lsc_en;      ///< enable switch of LSC    
    int sl2_en;      ///< enable switch of SL2
   // int mfb_en;
   // int c02_en;
    int c24_en;     ///< enable switch of c24
    int cfa_en;     ///< enable switch of cfa
    int c42_en;     ///< enable switch of c42
    int nbc_en;     ///< enable switch of nbc
    int seee_en;    ///< enable switch of SEEE
    int imgo_en;    ///< enable switch of IMGO
    int img2o_en;   ///< enable switch of IMG2O
  //  int esfko_en;
  //  int aao_en;
  //  int lcso_en;
    int cdrz_en;   ///< enable switch of CDRZ
  //  int curz_en;
  //  int fe_sel;
  //  int fe_en;
  //  int prz_en;
  //  int disp_vid_sel;
  //  int g2g2_en;
  //  int vido_en;
  //  int dispo_en;
  //  int nr3d_en;
    int mdp_sel;   ///< 0 : from CDRZ 1: from YUV
    int interlace_mode;///< doesn't use it
};

/**  
 * @brief update type for TDRI
 */

class TdriUpdateCfg
{
public:
    TPIPE_DRV_UPDATE_TYPE updateType;  ///< update type for isp tuning and tdri parameter
    int partUpdateFlag;///< partial update
};

/**  
 * @brief software perform configuration for TDRI
 */

class TdriPerformCfg //TdriswCfg
{
public:
	int log_en;     ///< doesn't use it
    int src_width;  ///< width of source image
    int src_height;///< height of source image
    int tpipeWidth; ///< width of tpipe
    int tpipeHeight;///< height of tpipe
    int irqMode;    ///< interrupt mode
};

/**  
 * @brief software DMA configuration for tDRI
 */

class TdriDMACfg
{
public:
    int srcWidth;  ///< width of source image
    int srcHeight; ///< height of source image
    int tpipeTabSize; ///< doesn't use it
    int baseVa;       ///< base virtual address
   /* int ringConfNumVa;
    int ringConfVerNumVa;
    int ringErrorControlVa;
    int ringConfBufVa;
    int ringBufferMcuRowNo;
    int ringBufferMcuHeight;
    //
    int isRunSegment; // for vss capture
    int setSimpleConfIdxNumVa;
    int segSimpleConfBufVa;*/
};

/**  
 * @brief IMGI configuration
 */

class TdriRingInDMACfg
{
public:
    int stride;  ///< image stride of IMGI
   // int ring_en;//need to remove
   // int ring_size;//need to remode
};
/*
class TdriRingOutDMACfg
{
public:

    int  stride;
    int  stride_c;
    int  stride_v;
    int  format_1;
    int  format_3;  
    int  rotation;
    int  flip;
    int  crop_en;
};
*/

/**  
 * @brief SL2 configuration for tdri
 */

class Tdrisl2Cfg //TdriswCfg
{
public:
	  int sl2_hrz_comp; ///< SL2_HRZ_COMP = f = 1/HRZ_Scaling, Q0.3.11
};

/**  
 * @brief isp tuning configuration for tdri
 */

class TdriTuningCfg
{
public:
    TdriBnrCfg bnr; ///< BNR configuration for tdri
    TdriLscCfg lsc; ///< LSC configuration for tdri
   // TdriLceCfg lce;
    TdriNbcCfg nbc; ///< NBC configuration for tdri
    TdriSeeeCfg seee;///< SEEE configuration for tdri
   // TdriMfbCfg mfb;
    TdriCfaCfg cfa; ///< CFA configuration for tdri
    Tdrisl2Cfg sl2; ///< SL2 configuration for tdri
   
};

/**  
 * @brief tdri driver configuration
 */

class TdriDrvCfg
{
public:
    //enable table
    TdriUpdateCfg updateTdri;       ///< update type for TDRI
    TdriTopCfg top;                 ///< top configuration for tdri
    TdriDMACfg tdriCfg;             ///< software DMA configuration for TDRI
    TdriPerformCfg  tdriPerformCfg;///< software perform configuration for TDRI

    TdriRingInDMACfg imgi;          ///< IMGI configuration for tdri
    //TdriRingInDMACfg vipi;
    //TdriRingInDMACfg vip2i;
    //int imgci_stride;
    //int lcei_stride;
    int lsci_stride;                ///< LSCI stride for tdri
    //TdriRingOutDMACfg dispo;
    //TdriRingOutDMACfg vido;
    TdriCdrzCfg cdrz;              ///< CDRZ configuration for tdri
    //TdriCurzCfg curz;
    //TdriPrzCfg prz;
    TdriImg2oCfg img2o;            ///< IMG2O configuration for tdri
    TdriImgoCfg imgo;              ///< IMGO configuration for tdri
    //TdriLcsoCfg lcso;
    //TdriEsfkoCfg esfko;
    //TdriAaoCfg aao;
    //TdriFlkiCfg flki;
    //TdriFeCfg fe;   

    TdriTuningCfg tuningFunc;      ///< isp tuning configuration for tdri
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
/**  
 * @brief tdri parameter configuration for tpipe driver
 */

class TpipeDrv
{
    public:


    protected:
        TpipeDrv() {};
        virtual ~TpipeDrv() {};
    //
    public:
        /**  
            * @brief Create TpipeDrv instance.
            */
        static TpipeDrv*  createInstance(void);
		/**  
            * @brief destory TpipeDrv instance.
            */
        virtual void    destroyInstance(void) = 0;
		/**
            *@brief Initialize function
            *@note Must call this function right after createInstance and before other functions
            *@return
            *-0 indicates success, otherwise indicates fail
            */ 
        virtual int   init(void) = 0;

		/**
            *@brief Uninitialize function
            *@note Must call this function before destroyInstance
            *@return
            *-0 indicates success, otherwise indicates fail
            */
        virtual int   uninit(void) = 0;
		/**  
            * @brief tdri parameter configuration 
            */
        virtual int   configTdriPara(TdriDrvCfg* pTdriInfo) = 0;
       // virtual MBOOL getNr3dTop(TPIPE_DRV_CQ_ENUM cq,MUINT32 *pEn) = 0;
        /**  
            * @brief get tdri parameter configuration 
            */
        virtual   void*   GetpConfigTpipeStruct(void) =0; //SL
};

//-----------------------------------------------------------------------------
#endif  // _TPIPE_DRV_H_

