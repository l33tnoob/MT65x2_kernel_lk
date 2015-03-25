#define LOG_TAG "iio/pathp2"
//
//
#include "cam_path.h"
//

/*******************************************************************************
*
********************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);


// the size only affect tpipe table
#define MAX_TPIPE_WIDTH                  (768)
#define MAX_TPIPE_HEIGHT                 (8192)
#define CONCURRENCY_RING_TPIPE_HEIGHT    (384)
#define CONCURRENCY_CAP_TPIPE_HEIGHT     (960)


// tpipe irq mode
#define TPIPE_IRQ_FRAME     (0)
#define TPIPE_IRQ_LINE      (1)
#define TPIPE_IRQ_TPIPE     (2)


// tpipe perform information
#define TPIPE_MAX_THROUGHPUT_PER_VSYNC      (6990506)  // 200M/30fps
#define TPIPE_THROUGHPUT_FOR_JPEG           ((1024*1024)>>1)   // 0.5M


// digital zoom setting
#define DIGITAL_ZOOM_FP_NUM     (10)
#define CDRZ_MIN_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)/128)
#define CDRZ_MAX_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define CURZ_MIN_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)/2)
#define CURZ_MAX_SCALE_RATIO    ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define PRZ_MIN_SCALE_RATIO     ((1 << DIGITAL_ZOOM_FP_NUM)/128)
#define PRZ_MAX_SCALE_RATIO     ((1 << DIGITAL_ZOOM_FP_NUM)*32)

#define BASIC_SCALE_RATIO       (1 << DIGITAL_ZOOM_FP_NUM)

#define CROP_MAX_RATIO      (1000 << DIGITAL_ZOOM_FP_NUM)


#define IS_ERROR_BLOCKING   (0)   // 1:blocking the current thread

#if IS_ERROR_BLOCKING
#define FUNCTION_BLOCKING   do{ISP_PATH_ERR("[Error]blocking\n");}while(1);
#else
#define FUNCTION_BLOCKING
#endif


/*******************************************************************************
*
********************************************************************************/
/*/////////////////////////////////////////////////////////////////////////////
  CamPathPass2
  /////////////////////////////////////////////////////////////////////////////*/
int CamPathPass2::config( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;    // 0: success. -1: error.
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 hAlgo,vAlgo;
    MUINT32 hTable, vTable;
    MUINT32 hCoeffStep,vCoeffStep;
    MUINT32 vidoXCropRatio, vidoYCropRatio, vidoCropRatio;
    MUINT32 dispoXCropRatio, dispoYCropRatio, dispoCropRatio;
    MUINT32 en3DNR = 0; //default=0

    ISP_PATH_DBG("CamPathPass2::config tdri(%d) E",p_parameter->tpipe);

    ISP_PATH_DBG("isRunSegment(%d) isCalculateTpipe(%d) \n",p_parameter->capTdriCfg.isRunSegment,p_parameter->capTdriCfg.isCalculateTpipe);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    /*function List*/
    if(p_parameter->capTdriCfg.isRunSegment) {
        if(p_parameter->capTdriCfg.isCalculateTpipe) {
            m_isp_function_list[0 ] = (IspFunction_B*)&DMACQ;
            m_isp_function_list[1 ] = (IspFunction_B*)&ispTurningCtrl;
            m_isp_function_list[2 ] = (IspFunction_B*)&tdriPipe;
            m_isp_function_count = 3;
        } else {
            m_isp_function_list[0 ] = (IspFunction_B*)&DMACQ;
            m_isp_function_list[1 ] = (IspFunction_B*)&ispTopCtrl;
            m_isp_function_list[2 ] = (IspFunction_B*)&ispRawPipe;
            m_isp_function_list[3 ] = (IspFunction_B*)&ispRgbPipe;
            m_isp_function_list[4 ] = (IspFunction_B*)&ispYuvPipe;
            m_isp_function_list[5 ] = (IspFunction_B*)&cdpPipe;
            m_isp_function_list[6 ] = (IspFunction_B*)&DMAImgi;
            m_isp_function_list[7 ] = (IspFunction_B*)&DMAVipi;
            m_isp_function_list[8 ] = (IspFunction_B*)&DMAVip2i;
            m_isp_function_list[9 ] = (IspFunction_B*)&DMATdri;
            m_isp_function_list[10 ] = (IspFunction_B*)&DMAImgci;
            m_isp_function_list[11 ] = (IspFunction_B*)&DMALsci;
            m_isp_function_list[12 ] = (IspFunction_B*)&DMALcei;
            m_isp_function_list[13 ] = (IspFunction_B*)&DMAImgo;
            m_isp_function_count = 14;
        }
    } else {
        m_isp_function_list[0 ] = (IspFunction_B*)&DMACQ;
        m_isp_function_list[1 ] = (IspFunction_B*)&ispTurningCtrl;
        m_isp_function_list[2 ] = (IspFunction_B*)&ispTopCtrl;
        m_isp_function_list[3 ] = (IspFunction_B*)&ispRawPipe;
        m_isp_function_list[4 ] = (IspFunction_B*)&ispRgbPipe;
        m_isp_function_list[5 ] = (IspFunction_B*)&ispYuvPipe;
        m_isp_function_list[6 ] = (IspFunction_B*)&cdpPipe;
        m_isp_function_list[7 ] = (IspFunction_B*)&DMAImgi;
        m_isp_function_list[8 ] = (IspFunction_B*)&DMAVipi;
        m_isp_function_list[9 ] = (IspFunction_B*)&DMAVip2i;
        m_isp_function_list[10 ] = (IspFunction_B*)&DMATdri;
        m_isp_function_list[11 ] = (IspFunction_B*)&tdriPipe;
    	m_isp_function_list[12 ] = (IspFunction_B*)&DMAImgci;
    	m_isp_function_list[13 ] = (IspFunction_B*)&DMALsci;
    	m_isp_function_list[14 ] = (IspFunction_B*)&DMALcei;
    	m_isp_function_list[15 ] = (IspFunction_B*)&DMAImgo;
    	m_isp_function_count = 16;
    }
    //turning
if (p_parameter->tpipe != CAM_MODE_VRMRG)   // Only run this when non-GDMA mode.
{
    ispTurningCtrl.bypass = 0;//ic.hsu
}
else    // GDMA Mode.
{
    ispTurningCtrl.bypass = 1;
}
    ispTurningCtrl.isApplyTurn = p_parameter->isApplyTurn;

    //top
    ispTopCtrl.path = ISP_PASS2;
    if ( CAM_ISP_CQ2 == p_parameter->CQ) {
        ispTopCtrl.path = ISP_PASS2B;
    }
    else if ( CAM_ISP_CQ3 == p_parameter->CQ ) {
        ispTopCtrl.path = ISP_PASS2C;
    }
    ispTopCtrl.en_Top  = p_parameter->en_Top;
    ispTopCtrl.ctl_int = p_parameter->ctl_int;
    ispTopCtrl.fmt_sel = p_parameter->fmt_sel;
    ispTopCtrl.ctl_sel = p_parameter->ctl_sel;
    ispTopCtrl.ctl_mux_sel = p_parameter->ctl_mux_sel;
    ispTopCtrl.ctl_mux_sel2 = p_parameter->ctl_mux_sel2;
    ispTopCtrl.ctl_sram_mux_cfg = p_parameter->ctl_sram_mux_cfg;
    ispTopCtrl.isEn1C24StatusFixed = p_parameter->isEn1C24StatusFixed;
    ispTopCtrl.isEn1C02StatusFixed = p_parameter->isEn1C02StatusFixed;
    ispTopCtrl.isEn1CfaStatusFixed = p_parameter->isEn1CfaStatusFixed;
    ispTopCtrl.isEn1HrzStatusFixed = p_parameter->isEn1HrzStatusFixed;
    ispTopCtrl.isEn1MfbStatusFixed = p_parameter->isEn1MfbStatusFixed;
    ispTopCtrl.isEn2CdrzStatusFixed = p_parameter->isEn2CdrzStatusFixed;
    ispTopCtrl.isEn2G2cStatusFixed = p_parameter->isEn2G2cStatusFixed;
    ispTopCtrl.isEn2C42StatusFixed = p_parameter->isEn2C42StatusFixed;
    ispTopCtrl.isImg2oStatusFixed = p_parameter->isImg2oStatusFixed;
    ispTopCtrl.isImgoStatusFixed = p_parameter->isImgoStatusFixed;
    ispTopCtrl.isAaoStatusFixed = p_parameter->isAaoStatusFixed;
    ispTopCtrl.isEsfkoStatusFixed = p_parameter->isEsfkoStatusFixed;
    ispTopCtrl.isFlkiStatusFixed = p_parameter->isFlkiStatusFixed;
    ispTopCtrl.isLcsoStatusFixed = p_parameter->isLcsoStatusFixed;
    ispTopCtrl.isEn1AaaGropStatusFixed = p_parameter->isEn1AaaGropStatusFixed;
    ispTopCtrl.isShareDmaCtlByTurn = p_parameter->isShareDmaCtlByTurn;
    ispTopCtrl.isApplyTurn = p_parameter->isApplyTurn;


    ispTopCtrl.isConcurrency = p_parameter->isConcurrency;
    p_parameter->pix_id = ispTopCtrl.pix_id; /* get pix_id from pass1 */
    ispTopCtrl.isIspOn = p_parameter->isIspOn;
    //isp_raw
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    ispRawPipe.enable1 = p_parameter->en_Top.enable1;
    ispRawPipe.src_img_w = p_parameter->src_img_size.w;
    ispRawPipe.src_img_h = p_parameter->src_img_size.h;
    //isp_rgb
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    ispRgbPipe.enable1 = p_parameter->en_Top.enable1;
    ispRgbPipe.src_img_h = p_parameter->src_img_size.h;

    //isp_yuv
    ispYuvPipe.bypass = p_parameter->bypass_ispYuvPipe;
    ispYuvPipe.enable2 = p_parameter->en_Top.enable2;
    //cdpPipe
    cdpPipe.disp_vid_sel = CAM_CDP_PRZ_CONN_TO_DISPO;
    cdpPipe.enable2 = p_parameter->en_Top.enable2;
    cdpPipe.dma_enable = p_parameter->en_Top.dma;
    cdpPipe.bypass = p_parameter->bypass_ispCdpPipe;
    cdpPipe.dispo_out = p_parameter->dispo; //SL add first
    cdpPipe.vido_out = p_parameter->vido; //SL add first vido
    cdpPipe.conf_rotDMA = 1;

    ISP_PATH_DBG("enable1(0x%x) enable2(0x%x) dma(0x%x),cq(%d)\n",p_parameter->en_Top.enable1,p_parameter->en_Top.enable2,cdpPipe.dma_enable,p_parameter->CQ);


    /*
     * RESIZER
     */
    switch (p_parameter->tpipe)
    {
        case CAM_MODE_FRAME:
        cdpPipe.tpipeMode = CDP_DRV_MODE_FRAME;
        break;

        case CAM_MODE_TPIPE:
        cdpPipe.tpipeMode = CDP_DRV_MODE_TPIPE;
        break;

        case CAM_MODE_VRMRG:
        cdpPipe.tpipeMode = CDP_DRV_MODE_VRMRG;
        break;
    }

    if (p_parameter->tpipe != CAM_MODE_VRMRG)   // Only run this when non-GDMA mode.
    {
        //SL tdriPipe.getNr3dTop(p_parameter->CQ, &en3DNR);
    }

    //
    cdpPipe.conf_cdrz = 0;
    //
    #if 0 //82 no need to take care if PRZ neect to VIDO or DISPO //     CAM_CDP_PRZ_CONN_TO_DISPO or CAM_CDP_PRZ_CONN_TO_VIDO
    getCdpMuxSetting(*p_parameter, &cdpPipe.disp_vid_sel); // get mux "disp_vid_sel" value
    #endif
    ISP_PATH_DBG("disp_vid_sel(%d) en3DNR(%d)\n",cdpPipe.disp_vid_sel,en3DNR);
    ISP_PATH_DBG("[imgi] in[%d, %d] imgiCrop[%d, %d, %d, %d]_f(0x%x, 0x%x)\n",p_parameter->imgi.size.w,p_parameter->imgi.size.h, \
                p_parameter->imgi.crop.x,p_parameter->imgi.crop.y,p_parameter->imgi.crop.w,p_parameter->imgi.crop.h, \
                p_parameter->imgi.crop.floatX,p_parameter->imgi.crop.floatY);
#if 0//to remove in   6582
    if ( CAM_CTL_EN2_CDRZ_EN & p_parameter->en_Top.enable2 ) {
        //isp_cdp pass2
        cdpPipe.conf_cdrz = 1;
        cdpPipe.cdrz_filter = CAM_CDP_CDRZ_8_TAP;
        //
        if(cdpPipe.disp_vid_sel==CAM_CDP_PRZ_CONN_TO_DISPO) {
            cdpPipe.prz_out = p_parameter->dispo.size;
            //
            if(CAM_CTL_DMA_EN_VIDO_EN & p_parameter->en_Top.dma) { // 2 out
                cdpPipe.cdrz_out = p_parameter->vido.size;
            } else { // 1 out (bypass cdrz)
                cdpPipe.cdrz_out.w = p_parameter->imgi.crop.w;
                cdpPipe.cdrz_out.h = p_parameter->imgi.crop.h;
                cdpPipe.cdrz_out.stride = p_parameter->imgi.size.stride * p_parameter->imgi.crop.w / p_parameter->imgi.size.w;
            }
        } else if(cdpPipe.disp_vid_sel==CAM_CDP_PRZ_CONN_TO_VIDO) {
            cdpPipe.prz_out = p_parameter->vido.size;
            //
            if(CAM_CTL_DMA_EN_DISPO_EN & p_parameter->en_Top.dma) { // 2 out, cdrz run crop
                cdpPipe.cdrz_out = p_parameter->dispo.size;
            } else {  // 1 out (bypass cdrz), Prz run crop
                cdpPipe.cdrz_out.w = p_parameter->imgi.crop.w;
                cdpPipe.cdrz_out.h = p_parameter->imgi.crop.h;
                cdpPipe.cdrz_out.stride = p_parameter->imgi.size.stride * p_parameter->imgi.crop.w / p_parameter->imgi.size.w;
            }
        } else {
            ISP_PATH_ERR("[Error]Not support this case disp_vid_sel(%d)\n",cdpPipe.disp_vid_sel);
            FUNCTION_BLOCKING;
        }
        //
        if(en3DNR) { // 3dnr enable

        } else {
            cdpPipe.cdrz_in = p_parameter->imgi.size;
            cdpPipe.cdrz_crop.x = p_parameter->imgi.crop.x;
            cdpPipe.cdrz_crop.floatX = p_parameter->imgi.crop.floatX;
            cdpPipe.cdrz_crop.y = p_parameter->imgi.crop.y;
            cdpPipe.cdrz_crop.floatY = p_parameter->imgi.crop.floatY;
            cdpPipe.cdrz_crop.w = p_parameter->imgi.crop.w;
            cdpPipe.cdrz_crop.h = p_parameter->imgi.crop.h;
            //
            cdpPipe.prz_in = cdpPipe.cdrz_out;
            cdpPipe.prz_crop.x = 0;
            cdpPipe.prz_crop.floatX = 0;
            cdpPipe.prz_crop.y = 0;
            cdpPipe.prz_crop.floatY = 0;
            cdpPipe.prz_crop.w = cdpPipe.prz_in.w;
            cdpPipe.prz_crop.h = cdpPipe.prz_in.h;

            ISP_PATH_DBG("cdpPipe.cdrz_crop.x(%u),cdpPipe.cdrz_crop.floatX(%u)\n",cdpPipe.cdrz_crop.x,cdpPipe.cdrz_crop.floatX);
            ISP_PATH_DBG("cdpPipe.cdrz_crop.y(%u),cdpPipe.cdrz_crop.floatY(%u)\n",cdpPipe.cdrz_crop.y,cdpPipe.cdrz_crop.floatY);
            ISP_PATH_DBG("cdpPipe.cdrz_crop.w(%u),cdpPipe.cdrz_crop.h(%u)\n",cdpPipe.cdrz_crop.w,cdpPipe.cdrz_crop.h);
        }
        //
        ispTopCtrl.ctl_sel.bit_field.disp_vid_sel = cdpPipe.disp_vid_sel;
        ispTopCtrl.ctl_sel.bit_field.prz_opt_sel = 0; //prz in mux,0:frm prz,1:frm DIPI,2:frm before CURZ

        //cdrz set for tpipe input
        p_parameter->cdrz.cdrz_input_crop_width = cdpPipe.cdrz_crop.w;
        p_parameter->cdrz.cdrz_input_crop_height = cdpPipe.cdrz_crop.h;
        p_parameter->cdrz.cdrz_output_width = cdpPipe.cdrz_out.w;
        p_parameter->cdrz.cdrz_output_height = cdpPipe.cdrz_out.h;
        p_parameter->cdrz.cdrz_horizontal_integer_offset = cdpPipe.cdrz_crop.x;/* pixel base */
        p_parameter->cdrz.cdrz_horizontal_subpixel_offset = \
            ((cdpPipe.cdrz_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        p_parameter->cdrz.cdrz_vertical_integer_offset = cdpPipe.cdrz_crop.y;/* pixel base */
        p_parameter->cdrz.cdrz_vertical_subpixel_offset = \
            ((cdpPipe.cdrz_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */

        ISP_PATH_DBG("CDRZ:cdrz.cdrz_vertical_integer_offset(%u),cdrz.cdrz_vertical_subpixel_offset(%u)\n",p_parameter->cdrz.cdrz_vertical_integer_offset,p_parameter->cdrz.cdrz_vertical_subpixel_offset);
        ISP_PATH_DBG("CDRZ:cdrz.cdrz_horizontal_integer_offset(%u),cdrz.cdrz_horizontal_subpixel_offset(%u)\n",p_parameter->cdrz.cdrz_horizontal_integer_offset,p_parameter->cdrz.cdrz_horizontal_subpixel_offset);

        //
        Result = ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->CalAlgoAndCStep(cdpPipe.tpipeMode,
                                                              CDP_DRV_RZ_CDRZ,
                                                              cdpPipe.cdrz_in.w,
                                                              cdpPipe.cdrz_in.h,
                                                              cdpPipe.cdrz_crop.w,
                                                              cdpPipe.cdrz_crop.h,
                                                              cdpPipe.cdrz_out.w,
                                                              cdpPipe.cdrz_out.h,
                                                              (CDP_DRV_ALGO_ENUM*)&hAlgo,
                                                              (CDP_DRV_ALGO_ENUM*)&vAlgo,
                                                              &hTable,
                                                              &vTable,
                                                              &hCoeffStep,
                                                              &vCoeffStep);
        if (!Result)
        {
            ret = -1;
            goto EXIT;
        }
                
        //
        p_parameter->cdrz.cdrz_horizontal_luma_algorithm = hAlgo;
        p_parameter->cdrz.cdrz_vertical_luma_algorithm = vAlgo;
        p_parameter->cdrz.cdrz_horizontal_coeff_step = hCoeffStep;
        p_parameter->cdrz.cdrz_vertical_coeff_step = vCoeffStep;


        ISP_PATH_DBG("CDRZ:in[%d, %d] crop[%d %d %d %d]_f(0x%x, 0x%x) out[%d,%d]\n",cdpPipe.cdrz_in.w,cdpPipe.cdrz_in.h, \
                cdpPipe.cdrz_crop.x,cdpPipe.cdrz_crop.y, cdpPipe.cdrz_crop.w,cdpPipe.cdrz_crop.h, \
                cdpPipe.cdrz_crop.floatX,cdpPipe.cdrz_crop.floatY,cdpPipe.cdrz_out.w,cdpPipe.cdrz_out.h);
        ISP_PATH_DBG("hCoeffStep(%d) vCoeffStep(%d)\n",hCoeffStep,vCoeffStep);

    } else if ( CAM_CTL_EN2_CURZ_EN & p_parameter->en_Top.enable2 ) { //cdp pass2
        //
        if(cdpPipe.disp_vid_sel==CAM_CDP_PRZ_CONN_TO_DISPO) {
            cdpPipe.prz_out = p_parameter->dispo.size;
            //
            if(CAM_CTL_DMA_EN_VIDO_EN & p_parameter->en_Top.dma) { // 2 out
                cdpPipe.curz_out = p_parameter->vido.size;
            } else { // 1 out (bypass curz)
                cdpPipe.curz_out.w = p_parameter->imgi.crop.w;
                cdpPipe.curz_out.h = p_parameter->imgi.crop.h;
                cdpPipe.curz_out.stride = p_parameter->imgi.size.stride * p_parameter->imgi.crop.w / p_parameter->imgi.size.w;
            }
         } else if(cdpPipe.disp_vid_sel==CAM_CDP_PRZ_CONN_TO_VIDO) {
            cdpPipe.prz_out = p_parameter->vido.size;
            if(CAM_CTL_DMA_EN_DISPO_EN & p_parameter->en_Top.dma) { // 2 out, Curz run crop
                cdpPipe.curz_out = p_parameter->dispo.size;
            } else {  // 1 out (bypass curz), Prz run crop
                cdpPipe.curz_out.w = p_parameter->imgi.crop.w;
                cdpPipe.curz_out.h = p_parameter->imgi.crop.h;
                cdpPipe.curz_out.stride = p_parameter->imgi.size.stride * p_parameter->imgi.crop.w / p_parameter->imgi.size.w;
            }
        } else {
            ISP_PATH_ERR("[Error]Not support this case disp_vid_sel(%d)\n",cdpPipe.disp_vid_sel);
            FUNCTION_BLOCKING;
        }

        //
        if(en3DNR) {
            // 3dnr enable
        } else {
            cdpPipe.curz_in = p_parameter->imgi.size;
            cdpPipe.curz_crop.x = p_parameter->imgi.crop.x;
            cdpPipe.curz_crop.floatX = p_parameter->imgi.crop.floatX;
            cdpPipe.curz_crop.y = p_parameter->imgi.crop.y;
            cdpPipe.curz_crop.floatY = p_parameter->imgi.crop.floatY;
            cdpPipe.curz_crop.w = p_parameter->imgi.crop.w;
            cdpPipe.curz_crop.h = p_parameter->imgi.crop.h;
            //
            cdpPipe.prz_in = cdpPipe.curz_out;
            cdpPipe.prz_crop.x = 0;
            cdpPipe.prz_crop.floatX = 0;
            cdpPipe.prz_crop.y = 0;
            cdpPipe.prz_crop.floatY = 0;
            cdpPipe.prz_crop.w = cdpPipe.prz_in.w;
            cdpPipe.prz_crop.h = cdpPipe.prz_in.h;
        }
        //
        ispTopCtrl.ctl_sel.bit_field.disp_vid_sel = cdpPipe.disp_vid_sel;
        ispTopCtrl.ctl_sel.bit_field.prz_opt_sel = 0; //prz in mux,0:frm prz,1:frm DIPI,2:frm before CURZ


        //curz set for tpipe
        p_parameter->curz.curz_input_crop_width = cdpPipe.curz_crop.w;
        p_parameter->curz.curz_input_crop_height = cdpPipe.curz_crop.h;
        p_parameter->curz.curz_output_width = cdpPipe.curz_out.w;
        p_parameter->curz.curz_output_height = cdpPipe.curz_out.h;
        //
        p_parameter->curz.curz_horizontal_integer_offset = cdpPipe.curz_crop.x;/* pixel base */
        p_parameter->curz.curz_horizontal_subpixel_offset = \
            ((cdpPipe.curz_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        p_parameter->curz.curz_vertical_integer_offset = cdpPipe.curz_crop.y;/* pixel base */
        p_parameter->curz.curz_vertical_subpixel_offset = \
            ((cdpPipe.curz_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        //

        Result = ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->CalAlgoAndCStep(cdpPipe.tpipeMode,
                                                              CDP_DRV_RZ_CURZ,
                                                              cdpPipe.curz_in.w,
                                                              cdpPipe.curz_in.h,
                                                              cdpPipe.curz_crop.w,
                                                              cdpPipe.curz_crop.h,
                                                              cdpPipe.curz_out.w,
                                                              cdpPipe.curz_out.h,
                                                              (CDP_DRV_ALGO_ENUM*)&hAlgo,
                                                              (CDP_DRV_ALGO_ENUM*)&vAlgo,
                                                              &hTable,
                                                              &vTable,
                                                              &hCoeffStep,
                                                              &vCoeffStep);
        if (!Result)
        {
            ret = -1;
            goto EXIT;
        }

        p_parameter->curz.curz_horizontal_coeff_step = hCoeffStep;
        p_parameter->curz.curz_vertical_coeff_step = vCoeffStep;

        ISP_PATH_DBG("CURZ:in[%d, %d] crop[%d %d %d %d]_f(0x%x, 0x%x) out[%d, %d]\n",cdpPipe.curz_in.w,cdpPipe.curz_in.h,
                cdpPipe.curz_crop.x,cdpPipe.curz_crop.y,cdpPipe.curz_crop.w,cdpPipe.curz_crop.h,
                cdpPipe.curz_crop.floatX,cdpPipe.curz_crop.floatY,cdpPipe.curz_out.w,cdpPipe.curz_out.h);
        ISP_PATH_DBG("hCoeffStep(%d) vCoeffStep(%d)\n",hCoeffStep,vCoeffStep);

    } else if ( CAM_CTL_EN2_PRZ_EN & p_parameter->en_Top.enable2 ) {  // CDRZ(0), CURZ(0), PRZ(1)
        if(cdpPipe.disp_vid_sel==CAM_CDP_PRZ_CONN_TO_DISPO) {
            cdpPipe.prz_out = p_parameter->dispo.size;
        } else if(cdpPipe.disp_vid_sel==CAM_CDP_PRZ_CONN_TO_VIDO) {
            cdpPipe.prz_out = p_parameter->vido.size;
        } else {
            ISP_PATH_ERR("[Error]Not support this case disp_vid_sel(%d)\n",cdpPipe.disp_vid_sel);
            FUNCTION_BLOCKING;
        }
        //
        if(en3DNR) {
            ISP_PATH_ERR("[Error]Not support this case en3DNR(%d)\n",en3DNR);
            FUNCTION_BLOCKING;
        } else {
            cdpPipe.prz_in = p_parameter->imgi.size;
            cdpPipe.prz_crop.x = p_parameter->imgi.crop.x;
            cdpPipe.prz_crop.floatX = p_parameter->imgi.crop.floatX;
            cdpPipe.prz_crop.y = p_parameter->imgi.crop.y;
            cdpPipe.prz_crop.floatY = p_parameter->imgi.crop.floatY;
            cdpPipe.prz_crop.w = p_parameter->imgi.crop.w;
            cdpPipe.prz_crop.h = p_parameter->imgi.crop.h;
        }
        //
        ispTopCtrl.ctl_sel.bit_field.disp_vid_sel = cdpPipe.disp_vid_sel;
        ispTopCtrl.ctl_sel.bit_field.prz_opt_sel = 0; //prz in mux,0:frm prz,1:frm DIPI,2:frm before CURZ

        ISP_PATH_DBG("PRZ:in[%d, %d] crop[%d %d %d %d]_f(0x%x,0x%x)\n", cdpPipe.prz_in.w, cdpPipe.prz_in.h, \
                cdpPipe.prz_crop.x, cdpPipe.prz_crop.y, cdpPipe.prz_crop.w,cdpPipe.prz_crop.h, \
                cdpPipe.prz_crop.floatX, cdpPipe.prz_crop.floatY);
    } else {
        ISP_PATH_DBG("CDRZ(%d) CURZ(%d) PRZ(%d)\n",CAM_CTL_EN2_CDRZ_EN & p_parameter->en_Top.enable2,
                CAM_CTL_EN2_CURZ_EN & p_parameter->en_Top.enable2, CAM_CTL_EN2_PRZ_EN & p_parameter->en_Top.enable2);
    }
#else   // [mdpmgr] collect crop information

    if(CAM_CTL_EN2_CDRZ_EN & p_parameter->en_Top.enable2 ) 
    {        
        ISP_PATH_DBG("[imgi] in(%d,%d),imgiCrop(%d,%d,%d,%d),CropFloat(0x%x,0x%x)\n", p_parameter->imgi.size.w,
                                                                                       p_parameter->imgi.size.h,
                                                                                       p_parameter->imgi.crop.x,
                                                                                       p_parameter->imgi.crop.y,
                                                                                       p_parameter->imgi.crop.w,
                                                                                       p_parameter->imgi.crop.h,
                                                                                       p_parameter->imgi.crop.floatX,
                                                                                       p_parameter->imgi.crop.floatY);

        cdpPipe.conf_cdrz = 1;
        
        cdpPipe.cdrz_in   = p_parameter->imgi.size;
        cdpPipe.cdrz_out  = p_parameter->imgi.size;  // just default value

        cdpPipe.cdrz_crop.x      = p_parameter->imgi.crop.x;
        cdpPipe.cdrz_crop.floatX = p_parameter->imgi.crop.floatX;
        cdpPipe.cdrz_crop.y      = p_parameter->imgi.crop.y;
        cdpPipe.cdrz_crop.floatY = p_parameter->imgi.crop.floatY;
        cdpPipe.cdrz_crop.w      = p_parameter->imgi.crop.w;
        cdpPipe.cdrz_crop.h      = p_parameter->imgi.crop.h;
    }

#endif

    //disp_vid_sel set for tpipe
    p_parameter->ctl_sel.bit_field.disp_vid_sel = ispTopCtrl.ctl_sel.bit_field.disp_vid_sel;

#if 0//to remove in   6582

    if ( CAM_CTL_EN2_PRZ_EN & p_parameter->en_Top.enable2 ) 
    {
        //prz set for tpipe
        p_parameter->prz.prz_output_width = cdpPipe.prz_out.w;
        p_parameter->prz.prz_output_height = cdpPipe.prz_out.h;

        p_parameter->prz.prz_horizontal_integer_offset = cdpPipe.prz_crop.x;/* pixel base */
        p_parameter->prz.prz_horizontal_subpixel_offset = \
            ((cdpPipe.prz_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        p_parameter->prz.prz_vertical_integer_offset = cdpPipe.prz_crop.y;/* pixel base */
        p_parameter->prz.prz_vertical_subpixel_offset = \
            ((cdpPipe.prz_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        //

        Result = ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->CalAlgoAndCStep(cdpPipe.tpipeMode,
                                                              CDP_DRV_RZ_PRZ,
                                                              cdpPipe.prz_in.w,
                                                              cdpPipe.prz_in.h,
                                                              cdpPipe.prz_crop.w,
                                                              cdpPipe.prz_crop.h,
                                                              cdpPipe.prz_out.w,
                                                              cdpPipe.prz_out.h,
                                                              (CDP_DRV_ALGO_ENUM*)&hAlgo,
                                                              (CDP_DRV_ALGO_ENUM*)&vAlgo,
                                                              &hTable,
                                                              &vTable,
                                                              &hCoeffStep,
                                                              &vCoeffStep);
        if (!Result)
        {
            ret = -1;
            goto EXIT;
        }

        p_parameter->prz.prz_horizontal_luma_algorithm = hAlgo;
        p_parameter->prz.prz_vertical_luma_algorithm = vAlgo;
        p_parameter->prz.prz_horizontal_coeff_step = hCoeffStep;
        p_parameter->prz.prz_vertical_coeff_step = vCoeffStep;

        ISP_PATH_DBG("PRZ:in[%d, %d] crop[%d %d %d %d]_f(0x%x,0x%x) out[%d, %d]\n",cdpPipe.prz_in.w,cdpPipe.prz_in.h, \
                cdpPipe.prz_crop.x,cdpPipe.prz_crop.y,cdpPipe.prz_crop.w,cdpPipe.prz_crop.h, \
                cdpPipe.prz_crop.floatX,cdpPipe.prz_crop.floatY,cdpPipe.prz_out.w,cdpPipe.prz_out.h);
        ISP_PATH_DBG("hAlgo(%d) vAlgo(%d) hCoeffStep(%d) vCoeffStep(%d)\n",hAlgo,vAlgo,hCoeffStep,vCoeffStep);
      
    }
#endif  

    //dispo
    cdpPipe.dispo_out = p_parameter->dispo;
    //vido
    cdpPipe.vido_out = p_parameter->vido;
    //imgi
    DMAImgi.dma_cfg = p_parameter->imgi;


    ISP_PATH_DBG("[imgi](%d , %d , %d) vidoRotate(%d)\n", p_parameter->imgi.size.w, p_parameter->imgi.size.h, p_parameter->imgi.size.stride,cdpPipe.vido_out.Rotation);

    ISP_PATH_DBG("isRunSegment(%d) [Pa]dispo(0x%x) vido(0x%x) imgi(0x%x) \n", \
        p_parameter->capTdriCfg.isRunSegment, \
        p_parameter->dispo.memBuf.base_pAddr,p_parameter->vido.memBuf.base_pAddr,p_parameter->imgi.memBuf.base_pAddr);

    //vipi
    DMAVipi.dma_cfg = p_parameter->vipi;
    DMAVipi.bypass =( 0 == DMAVipi.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //vip2i
    DMAVip2i.dma_cfg = p_parameter->vip2i;
    DMAVip2i.bypass =( 0 == DMAVip2i.dma_cfg.memBuf.base_pAddr )? 1 : 0;

    //imgci
    DMAImgci.bypass = 1;
    DMALsci.bypass = 1;
    DMALcei.bypass = 1;
    DMAImgo.bypass = 1;
    if ( CAM_CTL_DMA_EN_IMGCI_EN & ispTopCtrl.en_Top.dma ) {
        DMAImgci.bypass = 0;
        DMAImgci.dma_cfg = p_parameter->imgci;
    }
    //lsci
    if ( CAM_CTL_DMA_EN_LSCI_EN & ispTopCtrl.en_Top.dma ) {
        DMALsci.bypass = 0;
        DMALsci.dma_cfg = p_parameter->lsci;
    }
    //lcei
    if ( CAM_CTL_DMA_EN_LCEI_EN & ispTopCtrl.en_Top.dma ) {
        DMALcei.bypass = 0;
        DMALcei.dma_cfg = p_parameter->lcei;
    }
    //imgo
    if ( CAM_CTL_DMA_EN_IMGO_EN & ispTopCtrl.en_Top.dma ) {
        DMAImgo.bypass = 0;
        DMAImgo.dma_cfg = p_parameter->imgo_dma;
    }


    // tpipemain
    tdriPipe.enTdri = p_parameter->tpipe;
    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        DMATdri.bypass = 0;
        tdriPipe.bypass = 0;
        //n-tap NOT support tpipe mode
        cdpPipe.cdrz_filter = CAM_CDP_CDRZ_8_TAP;

        ISP_PATH_DBG("p_parameter->tcm_en = (0x%8x)\n", p_parameter->tcm_en);
        this->configTpipeData(p_parameter);

        DMATdri.dma_cfg = p_parameter->tdri;
        #if 1 //6582 VSS RAW
        ispTopCtrl.tcm_en = 0x90000000 | p_parameter->tcm_en; //ORG
        #else
        ispTopCtrl.tcm_en = 0x8c000003 | p_parameter->tcm_en;// TEST_MDP  : disable IMGI_TCM_UNP_EN        ispTopCtrl.tcm_en = 0x90000000 | p_parameter->tcm_en;
        #endif
        ISP_PATH_DBG("ispTopCtrl.en_Top.enable1 = (0x%8x)\n", ispTopCtrl.en_Top.enable1);
#if 0        
        if(ispTopCtrl.en_Top.enable1&CAM_CTL_EN1_UNP_EN) // TEST_RAW_YUV
        {
            ispTopCtrl.tcm_en |= 0x00100000 ;
            ISP_PATH_INF("ispTopCtrl.tcm_en |= 0x00100000(CAM_CTL_EN1_UNP_EN)");
            ISP_PATH_DBG("ispTopCtrl.tcm_en = (0x%8x)", ispTopCtrl.tcm_en);


        }
        else
        {            
            ISP_PATH_INF("ispTopCtrl.tcm_en NO |= 0x00100000");
            ISP_PATH_DBG("ispTopCtrl.tcm_en = (0x%8x)", ispTopCtrl.tcm_en);
        }
        if(ispTopCtrl.en_Top.enable1&CAM_CTL_EN1_LSC_EN) // TEST_RAW_YUV
        {
            ispTopCtrl.tcm_en |= 0x00800000 ;
            ISP_PATH_INF("ispTopCtrl.tcm_en |= 0x00800000");
            ISP_PATH_DBG("ispTopCtrl.tcm_en = (0x%8x)", ispTopCtrl.tcm_en);


        }
        else
        {            
            ISP_PATH_INF("ispTopCtrl.tcm_en NO |= 0x00800000(CAM_CTL_EN1_LSC_EN)");
            ISP_PATH_DBG("ispTopCtrl.tcm_en = (0x%8x)", ispTopCtrl.tcm_en);
        }
#endif        
        // update tpipe width/height
        cdpPipe.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_h = tdriPipe.tdri.tdriPerformCfg.tpipeHeight;
    }
    else {  // CAM_MODE_FRAME or CAM_MODE_VRMRG
        ispTopCtrl.tcm_en = 0x0;
        DMATdri.bypass = 1;
        tdriPipe.bypass = 1;
    }

    //buffer control path
    ispBufCtrl.path = ispTopCtrl.path;

    //pass2 commandQ
    if ( CAM_ISP_CQ_NONE != p_parameter->CQ ) 
    {
        //CAM_ISP_CQ0 is illegal
        DMACQ.bypass = 0; //
        DMACQ.dma_cfg = p_parameter->cqi;
        ispTurningCtrl.CQ = p_parameter->CQ;
        ispTopCtrl.CQ = p_parameter->CQ;
        ispRawPipe.CQ = p_parameter->CQ;
        ispRgbPipe.CQ = p_parameter->CQ;
        ispYuvPipe.CQ = p_parameter->CQ;
        cdpPipe.CQ = p_parameter->CQ;
        DMAImgi.CQ = p_parameter->CQ;
        DMAVipi.CQ = p_parameter->CQ;
        DMAVip2i.CQ = p_parameter->CQ;
        DMATdri.CQ = p_parameter->CQ;
        DMACQ.CQ = p_parameter->CQ;
        
        //for MFB only
        DMAImgci.CQ = p_parameter->CQ;
        DMALsci.CQ = p_parameter->CQ;
        DMALcei.CQ = p_parameter->CQ;
        DMAImgo.CQ = p_parameter->CQ;
        this->CQ = p_parameter->CQ;//for path config
    }
    else 
    {
        DMACQ.bypass = 1;
    }

    // set scenario ID
    DMACQ.DMAI_B::sceID    = p_parameter->scenario;
    DMAImgi.DMAI_B::sceID  = p_parameter->scenario;
    DMAImgci.DMAI_B::sceID = p_parameter->scenario;
    DMAVipi.DMAI_B::sceID  = p_parameter->scenario;
    DMAVip2i.DMAI_B::sceID = p_parameter->scenario;
    DMATdri.DMAI_B::sceID  = p_parameter->scenario;    
    DMALsci.DMAI_B::sceID  = p_parameter->scenario;
    DMALcei.DMAI_B::sceID  = p_parameter->scenario;

    // config
    this->_config((void*)p_parameter);

EXIT:

    ISP_PATH_DBG("X");
    return ret;
}


//
int CamPathPass2::configRingTpipe( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;    // 0: success. -1: error.
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 hAlgo,vAlgo;
    MUINT32 hTable,vTable;
    MUINT32 hCoeffStep,vCoeffStep;


    ISP_PATH_DBG("CamPathPass2::configRingTpipe tdri(%d) tdri_cal(%d)",p_parameter->tpipe,p_parameter->ringTdriCfg.isCalculateTdri);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    /*function List*/
    if(p_parameter->ringTdriCfg.isCalculateTdri) {
        m_isp_function_list[0 ] = (IspFunction_B*)&tdriPipe;
        m_isp_function_count = 1;
    } else {
        m_isp_function_list[0 ] = (IspFunction_B*)&DMACQ;
        m_isp_function_list[1 ] = (IspFunction_B*)&ispTopCtrl;
        m_isp_function_list[2 ] = (IspFunction_B*)&cdpPipe;
        m_isp_function_list[3 ] = (IspFunction_B*)&DMAImgi;
        m_isp_function_list[4 ] = (IspFunction_B*)&DMAVipi;
        m_isp_function_list[5 ] = (IspFunction_B*)&DMAVip2i;
        m_isp_function_list[6 ] = (IspFunction_B*)&DMATdri;
        m_isp_function_count = 7;
    }
    //turning
    ispTurningCtrl.bypass = 1;
    ispTurningCtrl.isApplyTurn = 0;

    //top
    ispTopCtrl.path = ISP_PASS2;
    if ( CAM_ISP_CQ2 == p_parameter->CQ) {
        ispTopCtrl.path = ISP_PASS2B;
    }
    else if ( CAM_ISP_CQ3 == p_parameter->CQ ) {
        ispTopCtrl.path = ISP_PASS2C;
    }
    ispTopCtrl.en_Top  = p_parameter->en_Top;
    ispTopCtrl.ctl_int = p_parameter->ctl_int;
    ispTopCtrl.fmt_sel = p_parameter->fmt_sel;
    ispTopCtrl.ctl_sel = p_parameter->ctl_sel;
    ispTopCtrl.ctl_mux_sel = p_parameter->ctl_mux_sel;
    ispTopCtrl.ctl_mux_sel2 = p_parameter->ctl_mux_sel2;
    ispTopCtrl.ctl_sram_mux_cfg = p_parameter->ctl_sram_mux_cfg;
    ispTopCtrl.isEn1C24StatusFixed = p_parameter->isEn1C24StatusFixed;
    ispTopCtrl.isEn1C02StatusFixed = p_parameter->isEn1C02StatusFixed;
    ispTopCtrl.isEn1CfaStatusFixed = p_parameter->isEn1CfaStatusFixed;
    ispTopCtrl.isEn1HrzStatusFixed = p_parameter->isEn1HrzStatusFixed;
    ispTopCtrl.isEn1MfbStatusFixed = p_parameter->isEn1MfbStatusFixed;
    ispTopCtrl.isEn2CdrzStatusFixed = p_parameter->isEn2CdrzStatusFixed;
    ispTopCtrl.isEn2G2cStatusFixed = p_parameter->isEn2G2cStatusFixed;
    ispTopCtrl.isEn2C42StatusFixed = p_parameter->isEn2C42StatusFixed;
    ispTopCtrl.isImg2oStatusFixed = p_parameter->isImg2oStatusFixed;
    ispTopCtrl.isImgoStatusFixed = p_parameter->isImgoStatusFixed;
    ispTopCtrl.isAaoStatusFixed = p_parameter->isAaoStatusFixed;
    ispTopCtrl.isEsfkoStatusFixed = p_parameter->isEsfkoStatusFixed;
    ispTopCtrl.isFlkiStatusFixed = p_parameter->isFlkiStatusFixed;
    ispTopCtrl.isLcsoStatusFixed = p_parameter->isLcsoStatusFixed;
    ispTopCtrl.isEn1AaaGropStatusFixed = p_parameter->isEn1AaaGropStatusFixed;
    ispTopCtrl.isShareDmaCtlByTurn = p_parameter->isShareDmaCtlByTurn;
    ispTopCtrl.isApplyTurn = p_parameter->isApplyTurn;
    ispTopCtrl.isConcurrency = p_parameter->isConcurrency;
    p_parameter->pix_id = ispTopCtrl.pix_id; /* get pix_id from pass1 */
    ispTopCtrl.isIspOn = p_parameter->isIspOn;
    //isp_raw
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    ispRawPipe.enable1 = p_parameter->en_Top.enable1;
    ispRawPipe.src_img_w = p_parameter->src_img_size.w;
    ispRawPipe.src_img_h = p_parameter->src_img_size.h;
    //isp_rgb
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    ispRgbPipe.enable1 = p_parameter->en_Top.enable1;
    //isp_yuv
    ispYuvPipe.bypass = p_parameter->bypass_ispRgbPipe;
    //cdpPipe

    cdpPipe.disp_vid_sel = CAM_CDP_PRZ_CONN_TO_DISPO; /* jpeg always use disp as output */

    cdpPipe.enable2 = p_parameter->en_Top.enable2;
    cdpPipe.dma_enable = p_parameter->en_Top.dma;
    cdpPipe.bypass = p_parameter->bypass_ispCdpPipe;
    cdpPipe.conf_rotDMA = 1;
    /*
    * RESIZER
    */
    cdpPipe.tpipeMode = CDP_DRV_MODE_TPIPE;
    cdpPipe.conf_cdrz = 0;
    if ( CAM_CTL_EN2_CURZ_EN & p_parameter->en_Top.enable2 &&
        CAM_CTL_EN2_PRZ_EN & p_parameter->en_Top.enable2) {
        //curz (only for cropping)
        cdpPipe.curz_in.w = p_parameter->imgi.size.w;
        cdpPipe.curz_in.h = p_parameter->imgi.size.h;
        cdpPipe.curz_in.stride = (p_parameter->imgi.size.stride*p_parameter->imgi.pixel_byte)>>CAM_ISP_PIXEL_BYTE_FP;;
        //
        cdpPipe.curz_crop.x = p_parameter->imgi.crop.x;
        cdpPipe.curz_crop.floatX = p_parameter->imgi.crop.floatX;
        cdpPipe.curz_crop.y = p_parameter->imgi.crop.y;
        cdpPipe.curz_crop.floatY = p_parameter->imgi.crop.floatY;
        cdpPipe.curz_crop.w = p_parameter->imgi.crop.w;
        cdpPipe.curz_crop.h = p_parameter->imgi.crop.h;
        //
        cdpPipe.curz_out.w = p_parameter->imgi.crop.w;
        cdpPipe.curz_out.h = p_parameter->imgi.crop.h;
        cdpPipe.curz_out.stride = cdpPipe.curz_in.stride * cdpPipe.curz_out.w / cdpPipe.curz_in.w; // according ratio
        //
        ISP_PATH_DBG("[CURZ_in] w(%d) h(%d) stride(%d)\n",cdpPipe.curz_in.w,cdpPipe.curz_in.h,cdpPipe.curz_in.stride);
        ISP_PATH_DBG("[CURZ_out] w(%d) h(%d) stride(%d)\n",cdpPipe.curz_out.w,cdpPipe.curz_out.h,cdpPipe.curz_out.stride);
        //
        cdpPipe.prz_in = cdpPipe.curz_out;
        //
        cdpPipe.prz_crop.x = 0;
        cdpPipe.prz_crop.floatX = 0;
        cdpPipe.prz_crop.y = 0;
        cdpPipe.prz_crop.floatY = 0;
        cdpPipe.prz_crop.w = cdpPipe.prz_in.w;
        cdpPipe.prz_crop.h = cdpPipe.prz_in.h;
        //
        cdpPipe.prz_out = p_parameter->dispo.size;
        //
        ispTopCtrl.ctl_sel.bit_field.prz_opt_sel = 0; //prz in mux,0:frm prz,1:frm DIPI,2:frm before CURZ
        //
        if ( CAM_CTL_DMA_EN_DISPO_EN & p_parameter->en_Top.dma &&
            ((CAM_CTL_DMA_EN_VIDO_EN & p_parameter->en_Top.dma)==0)) {    // 1 outs
            cdpPipe.prz_out = p_parameter->dispo.size;
            cdpPipe.disp_vid_sel = CAM_CDP_PRZ_CONN_TO_DISPO;

        } else {
            ISP_PATH_ERR("Setting Error(DMA out)");
            goto EXIT;
        }
        ispTopCtrl.ctl_sel.bit_field.disp_vid_sel = cdpPipe.disp_vid_sel;

        //curz set for tpipe
        p_parameter->curz.curz_input_crop_width = cdpPipe.curz_out.w;
        p_parameter->curz.curz_input_crop_height = cdpPipe.curz_out.h;
        p_parameter->curz.curz_output_width = cdpPipe.curz_out.w;
        p_parameter->curz.curz_output_height = cdpPipe.curz_out.h;
        p_parameter->curz.curz_horizontal_integer_offset = cdpPipe.curz_crop.x;/* pixel base */
        p_parameter->curz.curz_horizontal_subpixel_offset = \
            ((cdpPipe.curz_crop.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
        p_parameter->curz.curz_vertical_integer_offset = cdpPipe.curz_crop.y;/* pixel base */
        p_parameter->curz.curz_vertical_subpixel_offset = \
            ((cdpPipe.curz_crop.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */


        Result = ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->CalAlgoAndCStep(cdpPipe.tpipeMode,
                                                              CDP_DRV_RZ_CURZ,
                                                              cdpPipe.curz_in.w,
                                                              cdpPipe.curz_in.h,
                                                              cdpPipe.curz_crop.w,
                                                              cdpPipe.curz_crop.h,
                                                              cdpPipe.curz_out.w,
                                                              cdpPipe.curz_out.h,
                                                              (CDP_DRV_ALGO_ENUM*)&hAlgo,
                                                              (CDP_DRV_ALGO_ENUM*)&vAlgo,
                                                              &hTable,
                                                              &vTable,
                                                              &hCoeffStep,
                                                              &vCoeffStep);
        if (!Result)
        {
            ret = -1;
            goto EXIT;
        }

        p_parameter->curz.curz_horizontal_coeff_step = hCoeffStep;
        p_parameter->curz.curz_vertical_coeff_step = vCoeffStep;

        ISP_PATH_DBG("CURZ:in[%d, %d] out[%d, %d] hCoeffStep(%d) vCoeffStep(%d)\n",
            cdpPipe.curz_in.w,cdpPipe.curz_in.h,cdpPipe.curz_out.w,cdpPipe.curz_out.h,
            hCoeffStep,vCoeffStep);


        //prz (only for scale down)
        p_parameter->prz.prz_output_width = cdpPipe.prz_out.w;
        p_parameter->prz.prz_output_height = cdpPipe.prz_out.h;

        p_parameter->prz.prz_horizontal_integer_offset = 0; /* pixel base */
        p_parameter->prz.prz_horizontal_subpixel_offset = 0;/* 20 bits base */
        p_parameter->prz.prz_vertical_integer_offset = 0;   /* pixel base */
        p_parameter->prz.prz_vertical_subpixel_offset = 0;  /* 20 bits base */
        //
        Result = ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->CalAlgoAndCStep(cdpPipe.tpipeMode,
                                                              CDP_DRV_RZ_PRZ,
                                                              cdpPipe.prz_in.w,
                                                              cdpPipe.prz_in.h,
                                                              cdpPipe.prz_crop.w,
                                                              cdpPipe.prz_crop.h,
                                                              cdpPipe.prz_out.w,
                                                              cdpPipe.prz_out.h,
                                                              (CDP_DRV_ALGO_ENUM*)&hAlgo,
                                                              (CDP_DRV_ALGO_ENUM*)&vAlgo,
                                                              &hTable,
                                                              &vTable,
                                                              &hCoeffStep,
                                                              &vCoeffStep);
        if (!Result)
        {
            ret = -1;
            goto EXIT;
        }

        p_parameter->prz.prz_horizontal_luma_algorithm = hAlgo;
        p_parameter->prz.prz_vertical_luma_algorithm = vAlgo;
        p_parameter->prz.prz_horizontal_coeff_step = hCoeffStep;
        p_parameter->prz.prz_vertical_coeff_step = vCoeffStep;

        ISP_PATH_DBG("PRZ:in[%d, %d] out[%d, %d] hAlgo(%d) vAlgo(%d) hCoeffStep(%d) vCoeffStep(%d)\n",
            cdpPipe.prz_in.w,cdpPipe.prz_in.h,cdpPipe.prz_out.w,cdpPipe.prz_out.h,
            hAlgo,vAlgo,hCoeffStep,vCoeffStep);
        ISP_PATH_DBG("PRZ:hori int(%d) sub(%d), ver int(%d) sub(%d)\n",p_parameter->prz.prz_horizontal_integer_offset,
            p_parameter->prz.prz_horizontal_subpixel_offset,p_parameter->prz.prz_vertical_integer_offset,
            p_parameter->prz.prz_vertical_subpixel_offset);


    } else {
        ISP_PATH_ERR("Setting Error(CURZ not be enabled)");
        goto EXIT;
    }


    //disp_vid_sel set for tpipe
    p_parameter->ctl_sel.bit_field.disp_vid_sel = ispTopCtrl.ctl_sel.bit_field.disp_vid_sel;
    //dispo
    cdpPipe.dispo_out = p_parameter->dispo;
    //vido
    cdpPipe.vido_out = p_parameter->vido;
    //imgi
    DMAImgi.dma_cfg = p_parameter->imgi;


    ISP_PATH_DBG("[imgi](%d , %d , %d)\n", p_parameter->imgi.size.w, p_parameter->imgi.size.h, p_parameter->imgi.size.stride);


    //vipi
    DMAVipi.dma_cfg = p_parameter->vipi;
    DMAVipi.bypass =( 0 == DMAVipi.dma_cfg.memBuf.base_pAddr )? 1 : 0;
    //vip2i
    DMAVip2i.dma_cfg = p_parameter->vip2i;
    DMAVip2i.bypass =( 0 == DMAVip2i.dma_cfg.memBuf.base_pAddr )? 1 : 0;

    ISP_PATH_DBG("dispo:sequence(in):%d sequence(out):%d",p_parameter->dispo.Sequence,cdpPipe.dispo_out.Sequence);
    ISP_PATH_DBG("Imgi:[%d, %d, %d]",DMAImgi.dma_cfg.size.w,DMAImgi.dma_cfg.size.h,DMAImgi.dma_cfg.size.stride);
    ISP_PATH_DBG("Vipi:[%d, %d, %d]",DMAVipi.dma_cfg.size.w,DMAVipi.dma_cfg.size.h,DMAVipi.dma_cfg.size.stride);
    ISP_PATH_DBG("Vip2i:[%d, %d, %d]",DMAVip2i.dma_cfg.size.w,DMAVip2i.dma_cfg.size.h,DMAVip2i.dma_cfg.size.stride);

    ISP_PATH_DBG("en1(0x%x) en2(0x%x) dma(0x%x)\n",p_parameter->en_Top.enable1,p_parameter->en_Top.enable2,p_parameter->en_Top.dma);






    // tpipemain
    tdriPipe.enTdri = p_parameter->tpipe;
    if ( p_parameter->tpipe == CAM_MODE_TPIPE ) {
        DMATdri.bypass = 0;
        tdriPipe.bypass = 0;
        //n-tap NOT support tpipe mode
        cdpPipe.cdrz_filter = CAM_CDP_CDRZ_8_TAP;

        if(p_parameter->ringTdriCfg.isCalculateTdri) {
            this->configTpipeData(p_parameter);
        } else {
            this->getTpipePerform(p_parameter);
        }

        DMATdri.dma_cfg = p_parameter->tdri;
//SL        ispTopCtrl.tcm_en = 0x90000000 | p_parameter->tcm_en;//org
        ispTopCtrl.tcm_en = 0x8c100003 | p_parameter->tcm_en;//SL TEST_MDP         ispTopCtrl.tcm_en = 0x90000000 | p_parameter->tcm_en;        
        // update tpipe width/height
        cdpPipe.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_w = tdriPipe.tdri.tdriPerformCfg.tpipeWidth;
        ispTopCtrl.tpipe_h = tdriPipe.tdri.tdriPerformCfg.tpipeHeight;
    }
    else {  // CAM_MODE_FRAME or CAM_MODE_VRMRG
        ispTopCtrl.tcm_en = 0x0;
        DMATdri.bypass = 1;
        tdriPipe.bypass = 1;
    }
    ISP_PATH_DBG("AppTur(%d) tcm_en(0x%x) tpipe(%d, %d)\n",ispTopCtrl.isApplyTurn,ispTopCtrl.tcm_en,ispTopCtrl.tpipe_w,ispTopCtrl.tpipe_h);


    //pass2 commandQ
    if ( CAM_ISP_CQ_NONE != p_parameter->CQ ) {
        //CAM_ISP_CQ0 is illegal
        DMACQ.bypass = 0; //
        DMACQ.dma_cfg = p_parameter->cqi;
        ispTurningCtrl.CQ = p_parameter->CQ;
        ispTopCtrl.CQ = p_parameter->CQ;
        ispRawPipe.CQ = p_parameter->CQ;
        ispRgbPipe.CQ = p_parameter->CQ;
        ispYuvPipe.CQ = p_parameter->CQ;
        cdpPipe.CQ = p_parameter->CQ;
        DMAImgi.CQ = p_parameter->CQ;
        DMAVipi.CQ = p_parameter->CQ;
        DMAVip2i.CQ = p_parameter->CQ;
        DMATdri.CQ = p_parameter->CQ;
        DMACQ.CQ = p_parameter->CQ;
        //for MFB only
        DMAImgci.CQ = p_parameter->CQ;
        DMALsci.CQ = p_parameter->CQ;
        DMALcei.CQ = p_parameter->CQ;
        DMAImgo.CQ = p_parameter->CQ;
        this->CQ = p_parameter->CQ;//for path config
    }
    else {
        DMACQ.bypass = 1;
    }
    //
    this->_config((void*)p_parameter);

    //
EXIT:
    return ret;
}



int CamPathPass2::getTpipePerform( struct CamPathPass2Parameter* p_parameter )
{
    CAM_REG_CTL_DMA_EN *pEnDMA;
    pEnDMA = (CAM_REG_CTL_DMA_EN*)&(p_parameter->en_Top.dma);

   /* if(p_parameter->imgi.ring_en) {
        ISP_PATH_INF("ring_en(%d) isConcurrency(%d)\n",p_parameter->imgi.ring_en,p_parameter->isConcurrency);

        if(p_parameter->isConcurrency) {
            MUINT32 inputWFix, ringProcLine, maxTpipeH;
            MUINT32 remainWThroup;

            inputWFix = p_parameter->imgi.size.w * 12/10; // for tpipe loss buffer

            ringProcLine = \
                ((p_parameter->ringTdriCfg.ringBufferMcuRowNo * p_parameter->ringTdriCfg.ringBufferMcuHeight) >> 1) & (~0x01);

            maxTpipeH = (TPIPE_THROUGHPUT_FOR_JPEG / inputWFix) & (~0x01);

            if(maxTpipeH >= ringProcLine) {
                tdriPipe.tdri.tdriPerformCfg.tpipeHeight = ringProcLine;
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;
                tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_LINE;
            } else {
                tdriPipe.tdri.tdriPerformCfg.tpipeHeight = maxTpipeH;
                tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_TPIPE;
                remainWThroup = (TPIPE_THROUGHPUT_FOR_JPEG / tdriPipe.tdri.tdriPerformCfg.tpipeHeight) & (~0x01);
                if(remainWThroup > MAX_TPIPE_WIDTH)
                    tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;
                else
                    tdriPipe.tdri.tdriPerformCfg.tpipeWidth = remainWThroup;
            }
            ISP_PATH_INF("inputWFix(%d) ringProcLine(%d) maxTpipeH(%d)\n",inputWFix,ringProcLine,maxTpipeH);
        } else {
            tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;

            tdriPipe.tdri.tdriPerformCfg.tpipeHeight = MAX_TPIPE_HEIGHT;

            tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_LINE;
        }
    } else {
        // check and set tpipe width
        //To Do : Need modify on MT6582
        /*if(pEnDMA->Bits.VIDO_EN && (p_parameter->vido.Rotation==CDP_DRV_ROTATION_90 || p_parameter->vido.Rotation==CDP_DRV_ROTATION_270)) {
            if((p_parameter->vido.Format==CDP_DRV_FORMAT_YUV422 && p_parameter->vido.Plane==CDP_DRV_PLANE_2) ||
               (p_parameter->vido.Format==CDP_DRV_FORMAT_YUV420 && p_parameter->vido.Plane==CDP_DRV_PLANE_3) ) {
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth = 512;
            } else if (p_parameter->vido.Format==CDP_DRV_FORMAT_YUV422 && p_parameter->vido.Plane==CDP_DRV_PLANE_3) {
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth = 384;
            } else {
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;
            }
        } else {
            tdriPipe.tdri.tdriPerformCfg.tpipeWidth = MAX_TPIPE_WIDTH;
        }*/
        //
        // check and set tpipe height
        if (p_parameter->capTdriCfg.isRunSegment) {
            tdriPipe.tdri.tdriPerformCfg.tpipeHeight = CONCURRENCY_CAP_TPIPE_HEIGHT;
        } else {
            tdriPipe.tdri.tdriPerformCfg.tpipeHeight = MAX_TPIPE_HEIGHT;
        }

        // check and set irq mode
        if (p_parameter->capTdriCfg.isRunSegment) {
            tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_TPIPE;
        } else {
            tdriPipe.tdri.tdriPerformCfg.irqMode = TPIPE_IRQ_FRAME;
        }
   // }

    ISP_PATH_DBG("tpipeWidth(%d) tpipeHeight(%d) irqMode(%d)\n", \
                tdriPipe.tdri.tdriPerformCfg.tpipeWidth,tdriPipe.tdri.tdriPerformCfg.tpipeHeight, \
                tdriPipe.tdri.tdriPerformCfg.irqMode);

    return MTRUE;
}

int CamPathPass2::getCdpMuxSetting(struct CamPathPass2Parameter pass2Parameter, MINT32 *pDispVidSel)
{
    MINT32 ret = 0;
    MUINT32 vidoXCropRatio, vidoYCropRatio, vidoCropRatio;
    MUINT32 dispoXCropRatio, dispoYCropRatio, dispoCropRatio;
    MUINT32 vidoAbsRatio, dispoAbsRatio;
    MUINT32 xAbsRatio, yAbsRatio;

    MBOOL enVido = CAM_CTL_DMA_EN_VIDO_EN & pass2Parameter.en_Top.dma;
    MBOOL endispo = CAM_CTL_DMA_EN_DISPO_EN & pass2Parameter.en_Top.dma;
    MBOOL enCurz = CAM_CTL_EN2_CURZ_EN & pass2Parameter.en_Top.enable2;
    MBOOL enCdrz = CAM_CTL_EN2_CDRZ_EN & pass2Parameter.en_Top.enable2;
    MBOOL enPrz = CAM_CTL_EN2_PRZ_EN & pass2Parameter.en_Top.enable2;


    *pDispVidSel = -1; // set wrong initial value for debug

    /* check prz, cdrz and curz enable setting */
    if((!enPrz) && (!enCdrz) && (!enCurz) ) {
        *pDispVidSel = 0;
        ISP_PATH_WRN("[warning]enPrz(%d) enCdrz(%d) enCurz(%d)\n",enPrz,enCdrz,enCurz);
        return ret;
    } else if((!enPrz) || (enCdrz && enCurz) ) {
        ISP_PATH_ERR("[error]enPrz(%d) enCdrz(%d) enCurz(%d)\n",enPrz,enCdrz,enCurz);
        FUNCTION_BLOCKING;
    }


    if(enVido && endispo) { // vido:(1), dispo(1)

        /* get and check crop ratio of dispo */
        dispoXCropRatio = (pass2Parameter.dispo.size.w << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.w;
        dispoYCropRatio = (pass2Parameter.dispo.size.h << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.h;
        if(BASIC_SCALE_RATIO <= dispoXCropRatio && BASIC_SCALE_RATIO <= dispoYCropRatio) { // scale up

            dispoCropRatio = (dispoXCropRatio >= dispoYCropRatio) ? (dispoXCropRatio) : (dispoYCropRatio);

            dispoAbsRatio = dispoCropRatio - BASIC_SCALE_RATIO;

        } else if (BASIC_SCALE_RATIO >= dispoXCropRatio && BASIC_SCALE_RATIO >= dispoYCropRatio) { // scale down

            dispoCropRatio = (dispoXCropRatio <= dispoYCropRatio) ? (dispoXCropRatio) : (dispoYCropRatio);

            dispoAbsRatio = BASIC_SCALE_RATIO - dispoCropRatio;

        } else {  // x and y are not the same direction
            if(dispoXCropRatio > dispoYCropRatio) {  // x(scale up), y(scale down)
                xAbsRatio = dispoXCropRatio - BASIC_SCALE_RATIO;
                yAbsRatio = BASIC_SCALE_RATIO - dispoYCropRatio;

                if(xAbsRatio >= yAbsRatio) {
                    dispoCropRatio = dispoXCropRatio;
                    dispoAbsRatio = dispoCropRatio - BASIC_SCALE_RATIO;
                } else {
                    dispoCropRatio = dispoYCropRatio;
                    dispoAbsRatio = BASIC_SCALE_RATIO - dispoCropRatio;
                }
            } else { // x(scale down), y(scale up)
                xAbsRatio = BASIC_SCALE_RATIO - dispoXCropRatio;
                yAbsRatio = dispoYCropRatio - BASIC_SCALE_RATIO;

                if(xAbsRatio >= yAbsRatio) {
                    dispoCropRatio = dispoXCropRatio;
                    dispoAbsRatio = BASIC_SCALE_RATIO - dispoCropRatio;
                } else {
                    dispoCropRatio = dispoYCropRatio;
                    dispoAbsRatio = dispoCropRatio - BASIC_SCALE_RATIO;
                }
            }
            //
            ISP_PATH_WRN("[warning]x,y direction is not the same[Imgi]=[%d, %d], [dispo]=[%d, %d]\n",
                    pass2Parameter.imgi.size.w,pass2Parameter.imgi.size.h,pass2Parameter.dispo.size.w,pass2Parameter.dispo.size.h);

            ISP_PATH_DBG("[dispo]xAbsRatio(%d) yAbsRatio(%d) AbsRatio(%d) CropRatio(%d)\n",xAbsRatio,yAbsRatio,dispoAbsRatio,dispoCropRatio);
        }

        ISP_PATH_DBG("[Imgi]=[%d,%d],[Imgi_crop]=[%d,%d,%d,%d],[dispo]=[%d,%d]\n",pass2Parameter.imgi.size.w,pass2Parameter.imgi.size.h,
            pass2Parameter.imgi.crop.x,pass2Parameter.imgi.crop.y,pass2Parameter.imgi.crop.w,pass2Parameter.imgi.crop.h,
            pass2Parameter.dispo.size.w,pass2Parameter.dispo.size.h);

        ISP_PATH_DBG("[dispo]cropRatiox(0x%x), cropRatioy(0x%x), cropRatio(0x%x)\n",
                    dispoXCropRatio,dispoYCropRatio,dispoCropRatio);

        /* get and check crop ratio of vido */
        vidoXCropRatio = (pass2Parameter.vido.size.w << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.w;
        vidoYCropRatio = (pass2Parameter.vido.size.h << DIGITAL_ZOOM_FP_NUM) / pass2Parameter.imgi.crop.h;
        if(BASIC_SCALE_RATIO <= vidoXCropRatio && BASIC_SCALE_RATIO <= vidoYCropRatio) { // scale up

            vidoCropRatio = (vidoXCropRatio >= vidoYCropRatio) ? (vidoXCropRatio) : (vidoYCropRatio);
            vidoAbsRatio = vidoCropRatio - BASIC_SCALE_RATIO;

        } else if (BASIC_SCALE_RATIO >= vidoXCropRatio && BASIC_SCALE_RATIO >= vidoYCropRatio) { // scale down

            vidoCropRatio = (vidoXCropRatio <= vidoYCropRatio) ? (vidoXCropRatio) : (vidoYCropRatio);
            vidoAbsRatio = BASIC_SCALE_RATIO - vidoCropRatio;

        } else {
            if(vidoXCropRatio > vidoYCropRatio) {  // x(scale up), y(scale down)
                xAbsRatio = vidoXCropRatio - BASIC_SCALE_RATIO;
                yAbsRatio = BASIC_SCALE_RATIO - vidoYCropRatio;

                if(xAbsRatio >= yAbsRatio) {
                    vidoCropRatio = vidoXCropRatio;
                    vidoAbsRatio = vidoCropRatio - BASIC_SCALE_RATIO;
                } else {
                    vidoCropRatio = vidoYCropRatio;
                    vidoAbsRatio = BASIC_SCALE_RATIO - vidoCropRatio;
                }
            } else { // x(scale down), y(scale up)
                xAbsRatio = BASIC_SCALE_RATIO - vidoXCropRatio;
                yAbsRatio = vidoYCropRatio - BASIC_SCALE_RATIO;

                if(xAbsRatio >= yAbsRatio) {
                    vidoCropRatio = vidoXCropRatio;
                    vidoAbsRatio = BASIC_SCALE_RATIO - vidoCropRatio;
                } else {
                    vidoCropRatio = vidoYCropRatio;
                    vidoAbsRatio = vidoCropRatio - BASIC_SCALE_RATIO;
                }
            }
            //

            ISP_PATH_WRN("[warning]x,y direction is not the same[ImgiCrop]=[%d, %d],[vido]=[%d, %d]\n",
                    pass2Parameter.imgi.crop.w,pass2Parameter.imgi.crop.h,pass2Parameter.vido.size.w,pass2Parameter.vido.size.h);
            ISP_PATH_DBG("[vido]xAbsRatio(%d) yAbsRatio(%d) AbsRatio(%d) CropRatio(%d)\n",xAbsRatio,yAbsRatio,vidoAbsRatio,vidoCropRatio);
        }

        ISP_PATH_DBG("[Imgi]=[%d,%d],[Imgi_crop]=[%d,%d,%d,%d],[vido]=[%d,%d]\n",pass2Parameter.imgi.size.w,pass2Parameter.imgi.size.h,
            pass2Parameter.imgi.crop.x,pass2Parameter.imgi.crop.y,pass2Parameter.imgi.crop.w,pass2Parameter.imgi.crop.h,
            pass2Parameter.vido.size.w,pass2Parameter.vido.size.h);

        ISP_PATH_DBG("[vido]cropRatiox(0x%x), cropRatioy(0x%x), cropRatio(0x%x)\n",
                    vidoXCropRatio,vidoYCropRatio,vidoCropRatio);

        /* check scale ratio first due to CURZ & CDRZ limitation */
        if(enCurz) {
            if(dispoAbsRatio>=vidoAbsRatio && dispoCropRatio>=PRZ_MIN_SCALE_RATIO && dispoCropRatio<=PRZ_MAX_SCALE_RATIO
                            && vidoCropRatio>=CURZ_MIN_SCALE_RATIO && vidoCropRatio<=CURZ_MAX_SCALE_RATIO) {
                *pDispVidSel = CAM_CDP_PRZ_CONN_TO_DISPO; // source ratio is approch 1
            }else if(dispoAbsRatio<vidoAbsRatio && dispoCropRatio>=CURZ_MIN_SCALE_RATIO && dispoCropRatio<=CURZ_MAX_SCALE_RATIO
                                && vidoCropRatio>=PRZ_MIN_SCALE_RATIO && vidoCropRatio<=PRZ_MAX_SCALE_RATIO) {
                *pDispVidSel = CAM_CDP_PRZ_CONN_TO_VIDO; // source ratio is approch 1
            }else {
                ISP_PATH_ERR("[Error]scale ratio(1) out of range dispoAbsRatio(0x%x), vidoAbsRatio(0x%x)\n",dispoAbsRatio,vidoAbsRatio);
                FUNCTION_BLOCKING;
            }
        } else if(enCdrz) {
            if(dispoAbsRatio>=vidoAbsRatio && dispoCropRatio>=PRZ_MIN_SCALE_RATIO && dispoCropRatio<=PRZ_MAX_SCALE_RATIO
                            && vidoCropRatio>=CDRZ_MIN_SCALE_RATIO && vidoCropRatio<=CDRZ_MAX_SCALE_RATIO) {
                *pDispVidSel = CAM_CDP_PRZ_CONN_TO_DISPO; // source ratio is approch 1
            }else if(dispoAbsRatio<vidoAbsRatio && dispoCropRatio>=CDRZ_MIN_SCALE_RATIO && dispoCropRatio<=CDRZ_MAX_SCALE_RATIO
                                && vidoCropRatio>=PRZ_MIN_SCALE_RATIO && vidoCropRatio<=PRZ_MAX_SCALE_RATIO) {
                *pDispVidSel = CAM_CDP_PRZ_CONN_TO_VIDO; // source ratio is approch 1
            }else {
                ISP_PATH_ERR("[Error]scale ratio(2) out of range dispoAbsRatio(0x%x), vidoAbsRatio(0x%x)\n",dispoAbsRatio,vidoAbsRatio);
                FUNCTION_BLOCKING;
            }
        } else {
            ISP_PATH_ERR("[ERROR]enCurz(%d) enCdrz(%d)\n",enCurz,enCdrz);
            FUNCTION_BLOCKING;
        }
    } else if(enVido==0 && endispo) { // vido:(0), dispo(1)
        *pDispVidSel = CAM_CDP_PRZ_CONN_TO_DISPO;

    } else if(endispo==0 && enVido) { // vido:(1), dispo(0)
        *pDispVidSel = CAM_CDP_PRZ_CONN_TO_VIDO;

    } else {
        ISP_PATH_ERR("[ERROR]en_Top.dma(0x%x)\n",pass2Parameter.en_Top.dma);
        FUNCTION_BLOCKING;
    }


    if(*pDispVidSel!=CAM_CDP_PRZ_CONN_TO_DISPO && *pDispVidSel!=CAM_CDP_PRZ_CONN_TO_VIDO) {
        ISP_PATH_ERR("[ERROR]*pDispVidSel(%d)\n",*pDispVidSel);
        FUNCTION_BLOCKING;
    }


    return ret;
}



int CamPathPass2::configTpipeData( struct CamPathPass2Parameter* p_parameter )
{
    int ret = 0;
    CAM_REG_CTL_EN1 *pEn1;
    CAM_REG_CTL_EN2 *pEn2;
    CAM_REG_CTL_DMA_EN *pEnDMA;
   // CAM_REG_CTL_TCM_EN *pEnTCM;
   // CAM_REG_CTL_PIX_ID *pPixid;
   // CAM_REG_CTL_CROP_X *pMdpcrop;
   // CAM_REG_IMGI_SLOW_DOWN *pInterlace;

    pEn1 = (CAM_REG_CTL_EN1*)&(p_parameter->en_Top.enable1);
    pEn2 = (CAM_REG_CTL_EN2*)&(p_parameter->en_Top.enable2);
    pEnDMA = (CAM_REG_CTL_DMA_EN*)&(p_parameter->en_Top.dma);
	//pEnTCM = (CAM_REG_CTL_TCM_EN*)&(p_parameter->en_Top.tcm); 
	//pPixid=  (CAM_REG_CTL_PIX_ID*)&(p_parameter->en_Top.pixid);
	//pMdpcrop=(CAM_REG_CTL_CROP_X*)&(p_parameter->en_Top.mdpcrop);
	//pInterlace=(CAM_REG_IMGI_SLOW_DOWN*)&(p_parameter->en_Top.interlace);
    tdriPipe.enTdri = p_parameter->tpipe;

    /* config top_en */
    if(p_parameter->isConcurrency &&
            (CAM_CTL_EN2_CURZ_EN & p_parameter->en_Top.enable2) &&
            ((CAM_CTL_EN2_CDRZ_EN & p_parameter->en_Top.enable2) == 0x00)) {
        /* Due to hardware flexible setting for scenario and mode,
           so we need a corrected setting for tpipe main (only for cdp yuv input) */
        tdriPipe.tdri.top.scenario = 2; // ZSD:2
        tdriPipe.tdri.top.mode = 1; // ZSD_YUV:1
        ISP_PATH_WRN("[Warning]run scenario ZSD tpipe to repace scenario VSS");
    } else {
        tdriPipe.tdri.top.scenario = p_parameter->fmt_sel.bit_field.scenario;
        tdriPipe.tdri.top.mode = p_parameter->fmt_sel.bit_field.sub_mode;
    }


    tdriPipe.tdri.top.debug_sel = p_parameter->ctl_sel.bit_field.dbg_sel;
    tdriPipe.tdri.top.pixel_id = p_parameter->pix_id;
    tdriPipe.tdri.top.cam_in_fmt = p_parameter->fmt_sel.bit_field.cam_in_fmt;
   // tdriPipe.tdri.top.tcm_load_en =p_parameter->fmt_sel.bit_field.cam_in_fmt; 
    ISP_PATH_DBG("[Top]scenario(%d) mode(%d) debug_sel(%d) pixel_id(%d) cam_in_fmt(%d)",
        tdriPipe.tdri.top.scenario,tdriPipe.tdri.top.mode,tdriPipe.tdri.top.debug_sel,
        tdriPipe.tdri.top.pixel_id,tdriPipe.tdri.top.cam_in_fmt);

  
	//tdriPipe.tdri.top.tcm_load_en = pEnTCM->Bits.TCM_LOAD_EN;	
	//tdriPipe.tdri.top.ctl_extension_en = pPixid->Bits.CTL_EXTENSION_EN;
	//tdriPipe.tdri.top.mdp_crop_en = pMdpcrop->Bits.MDP_CROP_EN;
    tdriPipe.tdri.top.imgi_en = pEnDMA->Bits.IMGI_EN;
    //To Do : Need modify on MT6582
    //tdriPipe.tdri.top.imgci_en = pEnDMA->Bits.IMGCI_EN;
    //tdriPipe.tdri.top.vipi_en = pEnDMA->Bits.VIPI_EN;
    //tdriPipe.tdri.top.vip2i_en = pEnDMA->Bits.VIP2I_EN;
    //tdriPipe.tdri.top.flki_en = pEnDMA->Bits.FLKI_EN;
    //tdriPipe.tdri.top.lce_en = pEn1->Bits.LCE_EN;
    //tdriPipe.tdri.top.lcei_en = pEnDMA->Bits.LCEI_EN;
    tdriPipe.tdri.top.lsci_en = pEnDMA->Bits.LSCI_EN;
    tdriPipe.tdri.top.unp_en = pEn1->Bits.UNP_EN;
    tdriPipe.tdri.top.bnr_en = pEn1->Bits.BNR_EN;        

    tdriPipe.tdri.top.lsc_en = pEn1->Bits.LSC_EN;
	tdriPipe.tdri.top.sl2_en = pEn1->Bits.SL2_EN;
    //To Do : Need modify on MT6582
    //tdriPipe.tdri.top.mfb_en = pEn1->Bits.MFB_EN;
    //tdriPipe.tdri.top.c02_en = pEn1->Bits.C02_EN;
    tdriPipe.tdri.top.c24_en = pEn1->Bits.C24_EN;
    tdriPipe.tdri.top.cfa_en = pEn1->Bits.CFA_EN;
    tdriPipe.tdri.top.c42_en = pEn2->Bits.C42_EN;

    tdriPipe.tdri.top.nbc_en = pEn2->Bits.NBC_EN;
    tdriPipe.tdri.top.seee_en = pEn2->Bits.SEEE_EN;

    tdriPipe.tdri.top.imgo_en = pEnDMA->Bits.IMGO_EN;
    tdriPipe.tdri.top.img2o_en = pEnDMA->Bits.IMG2O_EN;
    //tdriPipe.tdri.top.esfko_en = pEnDMA->Bits.ESFKO_EN;
    //tdriPipe.tdri.top.aao_en = pEnDMA->Bits.AAO_EN;
    //To Do : Need modify on MT6582
    //tdriPipe.tdri.top.lcso_en = pEnDMA->Bits.LCSO_EN;

    tdriPipe.tdri.top.cdrz_en = pEn2->Bits.CDRZ_EN;
    //To Do : Need modify on MT6582
    //tdriPipe.tdri.top.curz_en = pEn2->Bits.CURZ_EN;

   // tdriPipe.tdri.top.fe_sel = p_parameter->ctl_sel.bit_field.fe_sel;
    //To Do : Need modify on MT6582
    //tdriPipe.tdri.top.fe_en = pEn2->Bits.FE_EN;
    //tdriPipe.tdri.top.prz_en = pEn2->Bits.PRZ_EN;

    //tdriPipe.tdri.top.disp_vid_sel = p_parameter->ctl_sel.bit_field.disp_vid_sel;
    //To Do : Need modify on MT6582
    //tdriPipe.tdri.top.g2g2_en = pEn2->Bits.G2G2_EN;
    //tdriPipe.tdri.top.vido_en = pEnDMA->Bits.VIDO_EN;
    //tdriPipe.tdri.top.dispo_en = pEnDMA->Bits.DISPO_EN;
    //tdriPipe.tdri.top.nr3d_en = pEn2->Bits.NR3D_EN;

   // ISP_PATH_DBG("cfa(%d) dispo_en(%d)\n",tdriPipe.tdri.top.cfa_en,tdriPipe.tdri.top.dispo_en);

	//tdriPipe.tdri.top.interlace_mode = pInterlace->Bits.INTERLACE_MODE;
    /* config dma */
    tdriPipe.tdri.imgi.stride = (p_parameter->imgi.size.stride*p_parameter->imgi.pixel_byte) >>CAM_ISP_PIXEL_BYTE_FP;
   // tdriPipe.tdri.imgi.ring_en = p_parameter->imgi.ring_en;
   // tdriPipe.tdri.imgi.ring_size = p_parameter->imgi.ring_size;
   // tdriPipe.tdri.imgci_stride = (p_parameter->imgci.size.stride*p_parameter->imgci.pixel_byte) >>CAM_ISP_PIXEL_BYTE_FP;;
  //  tdriPipe.tdri.vipi.stride = (p_parameter->vipi.size.stride*p_parameter->vipi.pixel_byte) >>CAM_ISP_PIXEL_BYTE_FP;;
  //  tdriPipe.tdri.vipi.ring_en = p_parameter->vipi.ring_en;
  //  tdriPipe.tdri.vipi.ring_size = p_parameter->vipi.ring_size;
  //  tdriPipe.tdri.vip2i.stride = (p_parameter->vip2i.size.stride*p_parameter->vip2i.pixel_byte) >>CAM_ISP_PIXEL_BYTE_FP;;
  //  tdriPipe.tdri.vip2i.ring_en = p_parameter->vip2i.ring_en;
  //  tdriPipe.tdri.vip2i.ring_size = p_parameter->vip2i.ring_size;

  //  tdriPipe.tdri.lcei_stride = (p_parameter->lcei.size.stride*p_parameter->lcei.pixel_byte) >>CAM_ISP_PIXEL_BYTE_FP;;
  //  tdriPipe.tdri.lsci_stride = (p_parameter->lsci.size.stride*p_parameter->lsci.pixel_byte) >>CAM_ISP_PIXEL_BYTE_FP;;

   // ISP_PATH_DBG("Pass2[Imgi]stride(%d) ring_en(%d) ring_size(%d)",
   //     tdriPipe.tdri.imgi.stride,tdriPipe.tdri.imgi.ring_en,tdriPipe.tdri.imgi.ring_size);
    /* vido */
	  // MUINT32 u4OutSequence = 0;
/*    tdriPipe.tdri.vido.rotation = p_parameter->vido.Rotation;
    tdriPipe.tdri.vido.flip = p_parameter->vido.Flip?1:0;;
    tdriPipe.tdri.vido.format_1 = p_parameter->vido.Format;
    tdriPipe.tdri.vido.format_3 = p_parameter->vido.Plane;
 
    ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->RotDmaEnumRemapping(p_parameter->vido.Format,
                                                              p_parameter->vido.Plane,
                                                              (MUINT32*)&tdriPipe.tdri.vido.format_3);
    tdriPipe.tdri.vido.stride = p_parameter->vido.size.stride * p_parameter->vido.pixel_byte;
    tdriPipe.tdri.vido.stride_c = p_parameter->vido.size_c.stride * p_parameter->vido.pixel_byte;
    tdriPipe.tdri.vido.stride_v = p_parameter->vido.size_v.stride * p_parameter->vido.pixel_byte;
    tdriPipe.tdri.vido.crop_en = p_parameter->vido.crop_en;

    ISP_PATH_DBG("[Vido]rotation(%d) flip(%d) format_1(%d) format_3(%d) seq(%d) stride(%d) stride_c(%d) stride_v(%d) crop_en(%d)",
        tdriPipe.tdri.vido.rotation, tdriPipe.tdri.vido.flip, tdriPipe.tdri.vido.format_1,
        tdriPipe.tdri.vido.format_3,u4OutSequence,tdriPipe.tdri.vido.stride,tdriPipe.tdri.vido.stride_c,
        tdriPipe.tdri.vido.stride_v,tdriPipe.tdri.vido.crop_en);
*/

    /* dispo */
   //tdriPipe.tdri.dispo.rotation = p_parameter->dispo.Rotation;
   // tdriPipe.tdri.dispo.flip = p_parameter->dispo.Flip?1:0;
   // tdriPipe.tdri.dispo.format_1 = p_parameter->dispo.Format;
   // tdriPipe.tdri.dispo.format_3 = p_parameter->dispo.Plane;
   // ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->RotDmaEnumRemapping(p_parameter->dispo.Format,
   //                                                           p_parameter->dispo.Plane,
  //                                                            (MUINT32*)&tdriPipe.tdri.dispo.format_3);
   // tdriPipe.tdri.dispo.stride = p_parameter->dispo.size.stride * p_parameter->dispo.pixel_byte;
   // tdriPipe.tdri.dispo.stride_c = p_parameter->dispo.size_c.stride * p_parameter->dispo.pixel_byte;
   // tdriPipe.tdri.dispo.stride_v = p_parameter->dispo.size_v.stride * p_parameter->dispo.pixel_byte;
   // tdriPipe.tdri.dispo.crop_en = p_parameter->dispo.crop_en;


    /* curz */
   /* tdriPipe.tdri.curz.curz_input_crop_width = p_parameter->curz.curz_input_crop_width;
    tdriPipe.tdri.curz.curz_input_crop_height = p_parameter->curz.curz_input_crop_height;
    tdriPipe.tdri.curz.curz_output_width = p_parameter->curz.curz_output_width;
    tdriPipe.tdri.curz.curz_output_height = p_parameter->curz.curz_output_height;
    tdriPipe.tdri.curz.curz_horizontal_integer_offset = p_parameter->curz.curz_horizontal_integer_offset;
    tdriPipe.tdri.curz.curz_horizontal_subpixel_offset = p_parameter->curz.curz_horizontal_subpixel_offset;
    tdriPipe.tdri.curz.curz_vertical_integer_offset = p_parameter->curz.curz_vertical_integer_offset;
    tdriPipe.tdri.curz.curz_vertical_subpixel_offset = p_parameter->curz.curz_vertical_subpixel_offset;
    tdriPipe.tdri.curz.curz_horizontal_coeff_step = p_parameter->curz.curz_horizontal_coeff_step;
    tdriPipe.tdri.curz.curz_vertical_coeff_step = p_parameter->curz.curz_vertical_coeff_step;*/


    //ISP_PATH_DBG("[Dispo]rotation(%d) flip(%d) format_1(%d) format_3(%d) seq(%d) stride(%d) stride_c(%d) stride_v(%d) crop_en(%d)",
      //  tdriPipe.tdri.dispo.rotation, tdriPipe.tdri.dispo.flip, tdriPipe.tdri.dispo.format_1,
      //  tdriPipe.tdri.dispo.format_3,u4OutSequence,tdriPipe.tdri.dispo.stride,tdriPipe.tdri.dispo.stride_c,
      //  tdriPipe.tdri.dispo.stride_v, tdriPipe.tdri.dispo.crop_en);

    ::memcpy( (char*)&tdriPipe.tdri.imgo, (char*)&p_parameter->imgo, sizeof(IspImgoCfg)); /* IMGO */
    ::memcpy( (char*)&tdriPipe.tdri.cdrz, (char*)&p_parameter->cdrz, sizeof(IspCdrzCfg)); /* CDRZ */
    //::memcpy( (char*)&tdriPipe.tdri.prz, (char*)&p_parameter->prz, sizeof(IspPrzCfg)); /* PRZ */
    ::memcpy( (char*)&tdriPipe.tdri.img2o, (char*)&p_parameter->img2o, sizeof(IspImg2oCfg)); /* IMG2O */
   // ::memcpy( (char*)&tdriPipe.tdri.esfko, (char*)&p_parameter->esfko, sizeof(TdriEsfkoCfg)); /* ESFKO */
   // ::memcpy( (char*)&tdriPipe.tdri.aao, (char*)&p_parameter->aao, sizeof(TdriAaoCfg)); /* AAO */
    //::memcpy( (char*)&tdriPipe.tdri.lcso, (char*)&p_parameter->lcso, sizeof(TdriLcsoCfg)); /* LCSO */
    //::memcpy( (char*)&tdriPipe.tdri.flki, (char*)&p_parameter->flki, sizeof(TdriFlkiCfg)); /* FLKI */
    //::memcpy( (char*)&tdriPipe.tdri.fe, (char*)&p_parameter->fe, sizeof(TdriFeCfg)); /* FE */

    ::memcpy( (char*)&tdriPipe.tdri.tuningFunc.bnr, (char*)&p_parameter->bnr, sizeof(TdriBnrCfg)); /* BNR */
    ::memcpy( (char*)&tdriPipe.tdri.tuningFunc.lsc, (char*)&p_parameter->lsc, sizeof(TdriLscCfg)); /* LSC */
   // ::memcpy( (char*)&tdriPipe.tdri.tuningFunc.lce, (char*)&p_parameter->lce, sizeof(TdriLceCfg)); /* LCE */
    ::memcpy( (char*)&tdriPipe.tdri.tuningFunc.nbc, (char*)&p_parameter->nbc, sizeof(TdriNbcCfg)); /* NBC */
    ::memcpy( (char*)&tdriPipe.tdri.tuningFunc.seee, (char*)&p_parameter->seee, sizeof(TdriSeeeCfg)); /* SEEE */
    //::memcpy( (char*)&tdriPipe.tdri.tuningFunc.mfb, (char*)&p_parameter->mfb, sizeof(TdriMfbCfg)); /* MFB */
    ::memcpy( (char*)&tdriPipe.tdri.tuningFunc.cfa, (char*)&p_parameter->cfa, sizeof(TdriCfaCfg)); /* CFA */
	::memcpy( (char*)&tdriPipe.tdri.tuningFunc.sl2, (char*)&p_parameter->sl2, sizeof(Tdrisl2Cfg)); /* SL2 */

    ::memcpy( (char*)&tdriPipe.tdri.updateTdri, (char*)&p_parameter->updateTdri, sizeof(IspTdriUpdateCfg)); /* TDRI update */

    ISP_PATH_DBG("updaType(%d)\n",(int)tdriPipe.tdri.updateTdri.updateType);

    /* software tpipe setting */
    tdriPipe.tdri.tdriCfg.srcWidth = p_parameter->imgi.size.w;
    tdriPipe.tdri.tdriCfg.srcHeight = p_parameter->imgi.size.h;
  //  tdriPipe.tdri.tdriCfg.ringBufferMcuRowNo = p_parameter->ringTdriCfg.ringBufferMcuRowNo;
  //  tdriPipe.tdri.tdriCfg.ringBufferMcuHeight = p_parameter->ringTdriCfg.ringBufferMcuHeight;
    tdriPipe.tdri.tdriCfg.baseVa = p_parameter->tdri.memBuf.base_vAddr;
  //  tdriPipe.tdri.tdriCfg.ringConfNumVa = p_parameter->ringTdriCfg.ringConfNumVa;
  //  tdriPipe.tdri.tdriCfg.ringConfVerNumVa = p_parameter->ringTdriCfg.ringConfVerNumVa;
  //  tdriPipe.tdri.tdriCfg.ringErrorControlVa = p_parameter->ringTdriCfg.ringErrorControlVa;
  //  tdriPipe.tdri.tdriCfg.ringConfBufVa = p_parameter->ringTdriCfg.ringConfBufVa;
   // tdriPipe.tdri.tdriCfg.isRunSegment = p_parameter->capTdriCfg.isRunSegment;
   // tdriPipe.tdri.tdriCfg.setSimpleConfIdxNumVa = p_parameter->capTdriCfg.setSimpleConfIdxNumVa;
   // tdriPipe.tdri.tdriCfg.segSimpleConfBufVa = p_parameter->capTdriCfg.segSimpleConfBufVa;
    //
    // get tpipe perform information
    getTpipePerform(p_parameter);

   // ISP_PATH_ERR("setSimpleConfIdxNumVa(0x%x) segSimpleConfBufVa(0x%x)\n",tdriPipe.tdri.tdriCfg.setSimpleConfIdxNumVa,
     //       tdriPipe.tdri.tdriCfg.segSimpleConfBufVa);

    ISP_PATH_DBG("[Tdri]srcWidth(%d) srcHeight(%d)",tdriPipe.tdri.tdriCfg.srcWidth,tdriPipe.tdri.tdriCfg.srcHeight);

EXIT:
    return ret;

}

int CamPathPass2::setZoom( MUINT32 zoomRatio )
{
int ret = 0;

    ISP_PATH_DBG(":E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);


    /*function List*/
     m_isp_function_list[0 ] = (IspFunction_B*)&cdpPipe;
     m_isp_function_list[1 ] = (IspFunction_B*)&DMAImgi;
     m_isp_function_list[2 ] = (IspFunction_B*)&DMAVipi;
     m_isp_function_list[3 ] = (IspFunction_B*)&DMAVip2i;
     m_isp_function_list[4 ] = (IspFunction_B*)&DMATdri;
    m_isp_function_count=5;

    //cdp
    cdpPipe.CQ = CAM_ISP_CQ1;

    this->_setZoom(NULL);

    ISP_PATH_DBG(":X");

    return ret;

}

int CamPathPass2::_waitIrq(int type, unsigned int irq )
{
int ret = 0;
ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    ISP_PATH_DBG(":E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)type;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = irq;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.Timeout = CAM_INT_PASS2_WAIT_TIMEOUT_MS;//ms ,0 means pass through.
#if 0 //SL skip this irQ to wait MDP done by dequeueDstBuffer
    if (0 == ispTopCtrl.waitIrq(WaitIrq) ) {
        ISP_PATH_ERR("waitIrq fail");
        ret = -1;
    }
#else
    ISP_PATH_DBG("No Pass2 _waitIrq()\n");
    ISP_PATH_DBG("return true, immediately\n");
    ISP_PATH_DBG("Let dequeueDstBuffer() to wait\n");    

#endif
    ISP_PATH_DBG(":X");

    return ret;
}


//
int CamPathPass2::dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo )
{
    int ret = 0;

    ISP_PATH_DBG(":E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

#if 0
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    //
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)ISP_DRV_IRQ_TYPE_INT;;
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;;
    WaitIrq.Timeout = CAM_INT_WAIT_TIMEOUT_MS;//ms ,0 means pass through.
    //
    //check if there is already filled buffer
    if ( MFALSE == this->ispBufCtrl.waitBufReady(dmaChannel) ) {
        this->ispTopCtrl.waitIrq(WaitIrq);
        //set next buffer FIRST
        setDMANextBuf((MUINT32) dmaChannel);
    }
#endif

    //move FILLED buffer from hw to sw list
    ret = this->ispBufCtrl.dequeueHwBuf( dmaChannel );
    if(ret == -2) 
    {
        return -2;        
    }
    else if(ret != 0)
    {
        ISP_PATH_ERR("ERROR:dequeueHwBuf(%d)",dmaChannel);
        return -1;
    }
    
    //delete all after move sw list to bufInfo.
    if ( 0!= this->ispBufCtrl.dequeueSwBuf( dmaChannel, bufInfo ) )
    {
        ISP_PATH_ERR("ERROR:dequeueSwBuf");
        return -1;
    }
    ISP_PATH_DBG(":X");
    //
    return ret;
}



