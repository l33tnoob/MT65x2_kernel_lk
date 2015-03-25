#define LOG_TAG "iio/pathp1"
//
//
#include "cam_path.h"
//



#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(path);
EXTERN_DBG_LOG_VARIABLE(path);


/*/////////////////////////////////////////////////////////////////////////////
  CamPathPass1
  /////////////////////////////////////////////////////////////////////////////*/
int CamPathPass1::config( struct CamPathPass1Parameter* p_parameter )
{
    ISP_PATH_DBG("CamPathPass1::config E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    /*function List*/
    m_isp_function_list[0 ] = (IspFunction_B*)&DMACQ;
    m_isp_function_list[1 ] = (IspFunction_B*)&ispTopCtrl;
    m_isp_function_list[2 ] = (IspFunction_B*)&ispRawPipe;
    m_isp_function_list[3 ] = (IspFunction_B*)&ispRgbPipe;
    m_isp_function_list[4 ] = (IspFunction_B*)&ispYuvPipe;
    m_isp_function_list[5 ] = (IspFunction_B*)&cdpPipe;
    m_isp_function_list[6 ] = (IspFunction_B*)&DMAImgo;
    m_isp_function_list[7 ] = (IspFunction_B*)&DMAImg2o;
    m_isp_function_count = 8;

    /***Setup isp function parameter***/
    //top
    ispTopCtrl.path = ISP_PASS1;
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
    ispTopCtrl.isAaoStatusFixed = p_parameter->isAaoStatusFixed;
    ispTopCtrl.isEsfkoStatusFixed = p_parameter->isEsfkoStatusFixed;
    ispTopCtrl.isFlkiStatusFixed = p_parameter->isFlkiStatusFixed;
    ispTopCtrl.isLcsoStatusFixed = p_parameter->isLcsoStatusFixed;
    ispTopCtrl.isEn1AaaGropStatusFixed = p_parameter->isEn1AaaGropStatusFixed;
    ispTopCtrl.isShareDmaCtlByTurn = p_parameter->isShareDmaCtlByTurn;
    ispTopCtrl.isConcurrency = 0;
    ispTopCtrl.pix_id = p_parameter->pix_id;
    ispTopCtrl.b_continuous = p_parameter->b_continuous;
    ispTopCtrl.isIspOn = p_parameter->isIspOn;
    //isp pipe
    ispRawPipe.bypass = p_parameter->bypass_ispRawPipe;
    ispRawPipe.enable1 = p_parameter->en_Top.enable1;
    ispRawPipe.src_img_w = p_parameter->src_img_size.w;
    ispRawPipe.src_img_h = p_parameter->src_img_size.h;
    ispRawPipe.cdrz_in_w = p_parameter->cdrz_in_size.w; //for HRZ
    //
    ispRgbPipe.bypass = p_parameter->bypass_ispRgbPipe;
    ispRgbPipe.enable1 = p_parameter->en_Top.enable1;
    ispRgbPipe.src_img_h = p_parameter->src_img_size.h;
    //
    ispYuvPipe.bypass = p_parameter->bypass_ispYuvPipe;
    ispYuvPipe.enable2 = p_parameter->en_Top.enable2;

    //cdp cdrz
    cdpPipe.tpipeMode = CDP_DRV_MODE_TPIPE;  // for algo use
    cdpPipe.path = ispTopCtrl.path; //for CDP free buffer
    cdpPipe.bypass = p_parameter->bypass_ispCdpPipe;
    cdpPipe.cdrz_in = p_parameter->cdrz_in_size;//p_parameter->src_img_size;
    cdpPipe.cdrz_out = ( ISP_SCENARIO_ZSD == p_parameter->fmt_sel.bit_field.scenario)? \
                        p_parameter->img2o.size:p_parameter->imgo.size;
    cdpPipe.conf_cdrz = ( ISP_SCENARIO_VR == p_parameter->fmt_sel.bit_field.scenario || \
                            ISP_SCENARIO_ZSD == p_parameter->fmt_sel.bit_field.scenario ) ? \
                            ( (ispTopCtrl.en_Top.enable2 & CAM_CTL_EN2_CDRZ_EN)?1:0 ) : 0;

    //should set with no crop setting
    ISP_PATH_DBG("pass1 cdrz crop setting");
    cdpPipe.cdrz_crop.x = 0;
    cdpPipe.cdrz_crop.floatX = 0;
    cdpPipe.cdrz_crop.y = 0;
    cdpPipe.cdrz_crop.floatY = 0;
    cdpPipe.cdrz_crop.w =  cdpPipe.cdrz_in.w;
    cdpPipe.cdrz_crop.h =  cdpPipe.cdrz_in.h;

    //imgo
    DMAImgo.dma_cfg = p_parameter->imgo;
    //img2o
    DMAImg2o.dma_cfg = p_parameter->img2o;
    //buffer control path
    ispBufCtrl.path = ispTopCtrl.path;
    //
    //pass1 commandQ
    if ( CAM_ISP_CQ_NONE != p_parameter->CQ ) {
        DMACQ.bypass = 0; //
        DMACQ.CQ = p_parameter->CQ;
        ispTopCtrl.CQ = p_parameter->CQ;
        ispRawPipe.CQ = p_parameter->CQ;
        ispRgbPipe.CQ = p_parameter->CQ;
        ispYuvPipe.CQ = p_parameter->CQ;
        cdpPipe.CQ = p_parameter->CQ;
        DMAImgo.CQ = p_parameter->CQ;
        DMAImg2o.CQ = p_parameter->CQ;
        this->CQ = p_parameter->CQ;//for path config
    }
    else {
        DMACQ.bypass = 1;
    }


#if 0
    //real time buffer control
    if ( 0 < p_parameter->imgo.memBuf.cnt) {
        ispBufCtrl.init(ISP_DMA_IMGO);
    }
    //
    if ( 0 < p_parameter->img2o.memBuf.cnt) {
        ispBufCtrl.init(ISP_DMA_IMG2O);
    }
#endif

    //
    this->_config(NULL);

    ISP_PATH_DBG("X");

    return 0;
}

int CamPathPass1::_waitIrq( int type, unsigned int irq )
{
int ret = 0;
ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)type;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = irq;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.Timeout = CAM_INT_WAIT_TIMEOUT_MS;//ms ,0 means pass through.

    if ( MFALSE == (MBOOL)ispTopCtrl.waitIrq(WaitIrq) ) {
        ret = -1;
    }

    ISP_PATH_DBG("ret(%d)",ret);

    return ret;
}
//
int CamPathPass1::setCdrz( IspSize out_size )
{
int ret = 0;
    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    this->cdpPipe.cdrz_out = out_size;
    this->cdpPipe.conf_cdrz = 1;
    this->cdpPipe.conf_rotDMA = 0;

    ret = this->cdpPipe.config();
    if ( 0 != ret ) {
        ISP_PATH_ERR("ERROR config cdrz ");
    }
    ISP_PATH_DBG("X");

    return ret;
}
//
int CamPathPass1::setDMAImgo( IspDMACfg const out_dma )
{
int ret = 0;

    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);


    this->DMAImgo.dma_cfg = out_dma;
    //
    ret = this->DMAImgo.config();
    //
    if ( 0 != ret ) {
        ISP_PATH_ERR("ERROR config imgo ");
    }
    ISP_PATH_DBG("X");
    return ret;
}

//
int CamPathPass1::dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo )
{
    int ret = 0;

    ISP_PATH_DBG("E");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);




/*
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    //
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    WaitIrq.Type = (ISP_DRV_IRQ_TYPE_ENUM)ISP_DRV_IRQ_TYPE_INT;//ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;//ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    WaitIrq.Timeout = CAM_INT_WAIT_TIMEOUT_MS;//ms ,0 means pass through.
    //
    //check if there is already filled buffer
    if ( MFALSE == this->ispBufCtrl.waitBufReady(dmaChannel) ) {
        this->ispTopCtrl.waitIrq(WaitIrq);
        //set next buffer FIRST
        setDMANextBuf((MUINT32) dmaChannel);
    }
*/

    //check if there is already filled buffer
    if ( MFALSE == this->ispBufCtrl.waitBufReady(dmaChannel) ) {
        ISP_PATH_ERR("waitBufReady fail");
        return -1;
    }
    //move FILLED buffer from hw to sw list
    if ( 0 != this->ispBufCtrl.dequeueHwBuf( dmaChannel ) ) {
        ISP_PATH_ERR("ERROR:dequeueHwBuf");
        return -1;
    }
    //delete all after move sw list to bufInfo.
    if ( 0 != this->ispBufCtrl.dequeueSwBuf( dmaChannel, bufInfo ) ) {
        ISP_PATH_ERR("ERROR:dequeueSwBuf");
        return -1;
    }
    //
    //ISP_PATH_DBG("[0x%x] ",bufInfo.pBufList->front().base_vAddr);
    //
    ISP_PATH_DBG("X");
    return ret;
}
//
int CamPathPass1::setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src)
{
    int ret = 0;
    ISP_PATH_DBG("E:%d/%d/%d ",cq,mode,trig_src);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    this->ispTopCtrl.setCQTriggerMode(cq,mode,trig_src);
    ISP_PATH_DBG("X");
    return ret;
}



