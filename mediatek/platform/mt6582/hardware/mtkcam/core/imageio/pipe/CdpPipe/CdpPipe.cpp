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
#define LOG_TAG "iio/cdp"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_

//
#include "PipeImp.h"
#include "CdpPipe.h"
//
#include <cutils/properties.h>  // For property_get().

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""


DECLARE_DBG_LOG_VARIABLE(pipe);
//EXTERN_DBG_LOG_VARIABLE(pipe);


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
CdpPipe::
CdpPipe(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EScenarioID const eScenarioID,
    EScenarioFmt const eScenarioFmt
)
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt),
      m_pIspDrvShell(NULL),
      m_resMgr(NULL),
      m_pipePass(EPipePass_PASS2),
      m_isPartialUpdate(MFALSE),
      m_isImgPlaneByImgi(MFALSE)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    memset(&this->m_camPass2Param,0x00,sizeof(CamPathPass1Parameter));
    this->m_vBufImgi.resize(1);
    this->m_vBufVipi.resize(1);
    this->m_vBufVip2i.resize(1);
    this->m_vBufDispo.resize(1);
    this->m_vBufVido.resize(1);

    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance(eScenarioID);

    /* create resource manager */
    m_resMgr = ResMgrDrv::CreateInstance();

}

CdpPipe::
~CdpPipe()
{
    /*** release isp driver ***/
    m_pIspDrvShell->destroyInstance();

}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
init()
{
    PIPE_INF(":E. meScenarioID: %d.", meScenarioID);
    //
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init();
        m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountInc();
        m_CamPathPass2.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
    }
    //
    if ( m_resMgr ) {
        m_resMgr->Init();
    }

    if (m_pIspDrvShell->m_pIMemDrv)    // Only do following steps when m_pIspDrvShell->m_pIMemDrv is not NULL (which implicitly imply it's non-GDMA mode.).
    {
        // alloc tdri table
        tdriSize = ISP_MAX_TDRI_HEX_SIZE;
        IMEM_BUF_INFO buf_info;
        buf_info.size = tdriSize;
        if ( m_pIspDrvShell->m_pIMemDrv->allocVirtBuf(&buf_info) ) {
            PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
            return MFALSE;
        }
        tdriMemId = buf_info.memID;
        pTdriVir = (MUINT8*)buf_info.virtAddr;
        if ( m_pIspDrvShell->m_pIMemDrv->mapPhyAddr(&buf_info) ) {
            PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
            return MFALSE;
        }
        tdriPhy =  (MUINT32)buf_info.phyAddr;
        PIPE_DBG("ALLOC pTdriVir(0x%x) tdriPhy(0x%x)\n",pTdriVir,tdriPhy);
        //
        // alloc tPipe configure table
        tpipe_config_size = ISP_MAX_TPIPE_SIMPLE_CONF_SIZE;
        IMEM_BUF_INFO config_buf_info;
        config_buf_info.size = tpipe_config_size;
        if ( m_pIspDrvShell->m_pIMemDrv->allocVirtBuf(&config_buf_info) ) {
            PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
            return MFALSE;
        }
        tpipe_config_memId = config_buf_info.memID;
        pTpipeConfigVa = (MUINT32*)config_buf_info.virtAddr;
        PIPE_DBG("ALLOC pTpipeConfigVa(0x%x)\n",pTpipeConfigVa);
        //
        tdriRingPhy = 0;
        pTdriRingVir = NULL;
        //
        segmSimpleConfIdxNum = 0;
        //
        PIPE_DBG("m_pIspDrvShell(0x%x) m_resMgr(0x%x) pTdriVir(0x%x) tdriPhy(0x%x) configVa(0x%x)",
            m_pIspDrvShell,m_resMgr,pTdriVir,tdriPhy,pTpipeConfigVa);
        //
    }

    PIPE_INF("X");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
uninit()
{
    PIPE_INF(":E. meScenarioID: %d.", meScenarioID);
    IMEM_BUF_INFO buf_info;
    //
    Mutex::Autolock lock(mLock);

    if (m_pIspDrvShell->m_pIMemDrv)    // Only do following steps when m_pIspDrvShell->m_pIMemDrv is not NULL (which implicitly imply it's non-GDMA mode.).
    {
        // free tpip table
        buf_info.size = tdriSize;
        buf_info.memID = tdriMemId;
        buf_info.virtAddr = (MUINT32)pTdriVir;
        buf_info.phyAddr  = (MUINT32)tdriPhy;
        if ( m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) ) {
            PIPE_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
            return MFALSE;
        }
        if ( m_pIspDrvShell->m_pIMemDrv->freeVirtBuf(&buf_info) ) {
            PIPE_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
            return MFALSE;
        }
        // free tpip simpile configure table
        buf_info.size = tpipe_config_size;
        buf_info.memID = tpipe_config_memId;
        buf_info.virtAddr = (MUINT32)pTpipeConfigVa;
        if ( m_pIspDrvShell->m_pIMemDrv->freeVirtBuf(&buf_info) ) {
            PIPE_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
            return MFALSE;
        }
        //
        // free ring tpip table
        PIPE_DBG("tdriRingPhy(0x%x) pTdriRingVir(0x%x)\n",tdriRingPhy,pTdriRingVir);
        if(tdriRingPhy && pTdriRingVir) {
            buf_info.size = tdriRingSize;
            buf_info.memID = tdriRingMemId;
            buf_info.virtAddr = (MUINT32)pTdriRingVir;
            buf_info.phyAddr  = (MUINT32)tdriRingPhy;
            if ( m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) ) {
                PIPE_ERR("ERROR:m_pIMemDrv->unmapPhyAddr (tdriRing)");
                return MFALSE;
            }
            if ( m_pIspDrvShell->m_pIMemDrv->freeVirtBuf(&buf_info) ) {
                PIPE_ERR("ERROR:m_pIMemDrv->freeVirtBuf (tdriRing)");
                return MFALSE;
            }
            pTdriRingVir = NULL;
            tdriRingPhy = 0;
            tdriRingMemId = -1;
        }
    }

    m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountDec();
    //
    m_pIspDrvShell->uninit();
    //
    m_resMgr->Uninit();

    PIPE_DBG(":X");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
start()
{
int path  = CAM_ISP_PASS2_START;

    PIPE_DBG(":E:pass[%d] +",this->m_pipePass);

    if ( ( EPipePass_PASS2 == this->m_pipePass ) || ( EPipePass_PASS2_Phy == this->m_pipePass ) ){
        path  = CAM_ISP_PASS2_START;
        m_CamPathPass2.ispTopCtrl.path = ISP_PASS2;
    }
    else if ( EPipePass_PASS2B == this->m_pipePass ) {
        path  = CAM_ISP_PASS2B_START;
        m_CamPathPass2.ispTopCtrl.path = ISP_PASS2B;
    }
    else if ( EPipePass_PASS2C == this->m_pipePass ) {
        path  = CAM_ISP_PASS2C_START;
        m_CamPathPass2.ispTopCtrl.path = ISP_PASS2C;
    } else {
        PIPE_ERR("[Error] unknown path(%d)\n",this->m_pipePass);
        path  = CAM_ISP_PASS2_START;
        m_CamPathPass2.ispTopCtrl.path = ISP_PASS2;
        return MFALSE;
    }

    m_CamPathPass2.start((void*)&path);

    return  MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
syncJpegPass2C()
{
    PIPE_DBG(":E");

    m_resMgr->GetMode(&resMgrMode);
    if(resMgrMode.Dev==RES_MGR_DRV_DEV_CAM &&
        (resMgrMode.ScenHw==RES_MGR_DRV_SCEN_HW_VSS || resMgrMode.ScenHw==RES_MGR_DRV_SCEN_HW_ZSD)) {

        PIPE_DBG("EPIPEIRQ_SOF");

        irq(EPipePass_PASS1_TG1, EPIPEIRQ_SOF);
    }

    PIPE_DBG(":X");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
startFmt()
{
int path  = CAM_ISP_FMT_START;

    PIPE_DBG(":E");

    m_CamPathPass2.ispTopCtrl.path = ISP_PASS2FMT;
    m_CamPathPass2.start((void*)&path);
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
stop()
{
    PIPE_DBG(":E");

    m_CamPathPass2.stop(NULL);

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;

    PIPE_DBG(":E");
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    if (EPortIndex_IMGI == portID.index) {
        dmaChannel = ISP_DMA_IMGI;
    }
    else if (EPortIndex_VIPI == portID.index) {
        dmaChannel = ISP_DMA_VIPI;
    }
    else if (EPortIndex_VIP2I == portID.index) {
        dmaChannel = ISP_DMA_VIP2I;
    }
    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    bufInfo.memID = rQBufInfo.vBufInfo[0].memID;
    bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize;
    bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu;
    bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe;
    if ( 0!= this->m_CamPathPass2.enqueueBuf( dmaChannel , bufInfo ) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }
    //


    PIPE_DBG(":X");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;

    PIPE_DBG("E ");
    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    //
    if (EPortIndex_IMGI == portID.index) {
        dmaChannel = ISP_DMA_IMGI;
    }
    else if (EPortIndex_VIPI == portID.index) {
        dmaChannel = ISP_DMA_VIPI;
    }
    else if (EPortIndex_VIP2I == portID.index) {
        dmaChannel = ISP_DMA_VIP2I;
    }
    //
    bufInfo.pBufList = &bufList;
    if ( 0 != this->m_CamPathPass2.dequeueBuf( dmaChannel,bufInfo) ) {
        PIPE_ERR("ERROR:dequeueBuf");
        return MFALSE;
    }
    //
    rQBufInfo.vBufInfo.resize(bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID = bufList.front().memID;
        rQBufInfo.vBufInfo[i].u4BufVA = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufSize = bufList.front().size;
        bufList.pop_front();
    }
    //
    PIPE_DBG("X ");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;

    PIPE_DBG(":E");
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    if (EPortIndex_DISPO == portID.index) {
        dmaChannel = ISP_DMA_DISPO;
    }
    else if (EPortIndex_VIDO == portID.index) {
        dmaChannel = ISP_DMA_VIDO;
    }
    else if (EPortIndex_FDO == portID.index) {
        dmaChannel = ISP_DMA_FDO;
    }
    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    bufInfo.memID = rQBufInfo.vBufInfo[0].memID;
    bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize;
    bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu;
    bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe;
    //
    if ( 0 != this->m_CamPathPass2.enqueueBuf( dmaChannel, bufInfo ) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }

    PIPE_DBG(":X");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;

    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    //
    if (EPortIndex_DISPO == portID.index) {
        dmaChannel = ISP_DMA_DISPO;
    }
    else if (EPortIndex_VIDO == portID.index) {
        dmaChannel = ISP_DMA_VIDO;
    }
    else if (EPortIndex_FDO == portID.index) {
        dmaChannel = ISP_DMA_FDO;
    }
    //
    bufInfo.pBufList = &bufList;
    if ( 0 != this->m_CamPathPass2.dequeueBuf( dmaChannel,bufInfo) ) {
        PIPE_ERR("ERROR:dequeueBuf");
        return MFALSE;
    }
    //
    rQBufInfo.vBufInfo.resize(bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID = bufList.front().memID;
        rQBufInfo.vBufInfo[i].u4BufVA = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufSize = bufList.front().size;
        rQBufInfo.vBufInfo[i].i4TimeStamp_sec = bufList.front().timeStampS;
        rQBufInfo.vBufInfo[i].i4TimeStamp_us = bufList.front().timeStampUs;
        bufList.pop_front();
    }
    //

    return  MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    PIPE_INF("[CdpPipe::configPipe] m_pass2_CQ(%d) ",m_pass2_CQ);

int ret = 0;    // 0: Success. other: error code.
int idx_imgi = -1;
int idx_vipi = -1;
int idx_vip2i = -1;
int idx_dispo = -1;
int idx_vido = -1;

    int scenario = ISP_SCENARIO_MAX;
    int subMode = ISP_SUB_MODE_MAX;
    int cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
    int dmai_swap = 0;
    int dmai_swap_uv = 0;
    int enable1 = 0;
    int enable2 = 0;
    int dma_en = 0;
    int int_en = 0;
    int intb_en = 0;
    int intc_en = 0;
    int tpipe = CAM_MODE_FRAME;
    int tcm_en = 0;
    int isIspOn = 0;
    int gdma_link_crz_prz_mrg = 0;
    //
    int ctl_mux_sel = 0;  //4074
    int ctl_mux_sel2 = 0; //4078
    int ctl_sram_mux_cfg = 0;  //407C
    int isEn1C24StatusFixed = 1; /* 1:c24(en1) won't be clear */
    int isEn1C02StatusFixed = 1; /* 1:c02(en1) won't be clear */
    int isEn1CfaStatusFixed = 1; /* 1:cfa(en1) won't be clear */
    int isEn1HrzStatusFixed = 1; /* 1:hrz(en1) won't be clear */
    int isEn1MfbStatusFixed = 1; /* 1:mfb(en1) won't be clear */
    int isEn2CdrzStatusFixed = 1; /* 1:cdrz(en2) won't be clear */
    int isEn2G2cStatusFixed = 1; /* 1:g2c(en2) won't be clear */
    int isEn2C42StatusFixed = 1; /* 1:c42(en2) won't be clear */
    int isImg2oStatusFixed = 1; /* 1:img2o won't be clear */
    int isImgoStatusFixed = 1;  /* 1:imgo won't be clear */
    int isAaoStatusFixed = 1;  /* 1:aao won't be clear */
    int isEsfkoStatusFixed = 1;  /* 1:Esfko won't be clear */
    int isFlkiStatusFixed = 1;  /* 1:Flki won't be clear */
    int isLcsoStatusFixed = 1;  /* 1:Lcso won't be clear */
    int isEn1AaaGropStatusFixed = 1;  /* 1:SGG_EN,AF_EN,FLK_EN,AA_EN,LCS_EN won't be clear */
    int isShareDmaCtlByTurn =1; /* 1:share DMA(imgci,lsci and lcei) ctl by turning */
    int isApplyTurn = 0;  /* 1:apply turning(FeatureIO) setting */
    int imgi_format_en = 0, imgi_format = 0, imgi_bus_size_en=0, imgi_bus_size=0;
    int vipi_format_en = 0, vipi_format = 0, vipi_bus_size_en=0, vipi_bus_size=0;
    int vip2i_format_en = 0, vip2i_format = 0;
    int imgci_format_en = 0, imgci_format = 0, imgci_bus_size_en=0, imgci_bus_size=0;
    TPIPE_UPDATE_TYPE updaType = TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE;
    EConfigSettingStage settingStage = m_settingStage;

#if 0 // no use CQ
    int pass2_CQ = CAM_ISP_CQ_NONE;//
    int pass2_cq_en = 0;//
#else   //use CQ1

    //PIPE_ERR("[test_debuging] m_pass2_CQ(%d) ",m_pass2_CQ);

    int pass2_CQ = m_pass2_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//
    int pass2_cq_en = 0;// = (CAM_ISP_CQ0 == pass1_CQ)?CAM_CTL_EN2_CQ0_EN:((CAM_ISP_CQ0B == pass1_CQ)?CAM_CTL_EN2_CQ0B_EN:);//0; //
    if ( CAM_ISP_CQ_NONE != pass2_CQ) {
        if ( CAM_ISP_CQ1 == pass2_CQ ) { pass2_cq_en = CAM_CTL_EN2_CQ1_EN;}
        if ( CAM_ISP_CQ2 == pass2_CQ )  { pass2_cq_en = CAM_CTL_EN2_CQ2_EN;}
        if ( CAM_ISP_CQ3 == pass2_CQ )  { pass2_cq_en = CAM_CTL_EN2_CQ3_EN;}
    }
#endif
    int pixel_byte_imgi = 1;
    int pixel_byte_vipi = 1;
    int pixel_byte_vip2i = 1;
    MUINT32 cq_size = 0,cq_phy = 0;
    MUINT8 *cq_vir = NULL;
    //
    PortInfo portInfo_imgi;
    PortInfo portInfo_vipi;
    PortInfo portInfo_vip2i;
    //
    PIPE_DBG("settingStage(%d) in(%d) / out(%d) ",settingStage, vInPorts.size(), vOutPorts.size());
/*
    vector<PortInfo const*>::const_iterator iter;
    for ( iter = vInPorts.begin(); iter != vInPorts.end() ; ++iter ) {
        if ( 0 == (*iter) ) { continue; }
        PIPE_DBG(":%d,%d,%d,%d,%d,%d",
                                            (*iter)->eImgFmt,
                                            (*iter)->u4ImgWidth,
                                            (*iter)->u4ImgHeight,
                                            (*iter)->type,
                                            (*iter)->index,
                                            (*iter)->inout);

    }*/



    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) { continue; }
        //
        PIPE_INF("vInPorts:[%d]:(0x%x),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)",i,
                                                        vInPorts[i]->eImgFmt,
                                                        vInPorts[i]->u4ImgWidth,
                                                        vInPorts[i]->u4ImgHeight,
                                                        vInPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vInPorts[i]->type,
                                                        vInPorts[i]->index,
                                                        vInPorts[i]->inout);

        if ( EPortIndex_IMGI == vInPorts[i]->index ) {
            idx_imgi = i;
            dma_en |=  CAM_CTL_DMA_EN_IMGI_EN;
            //
            portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];
        }
        else if ( EPortIndex_VIPI == vInPorts[i]->index ) {
            idx_vipi = i;
            dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
            //
            portInfo_vipi =  (PortInfo)*vInPorts[idx_vipi];
        }
        else if ( EPortIndex_VIP2I == vInPorts[i]->index ) {
            idx_vip2i = i;
            dma_en |=  CAM_CTL_DMA_EN_VIP2I_EN;
            //
            portInfo_vip2i =  (PortInfo)*vInPorts[idx_vip2i];
        }
    }
    //
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) { continue; }
        //
        PIPE_INF("vOutPorts:[%d]:(0x%x),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)",i,
                                                        vOutPorts[i]->eImgFmt,
                                                        vOutPorts[i]->u4ImgWidth,
                                                        vOutPorts[i]->u4ImgHeight,
                                                        vOutPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vOutPorts[i]->type,
                                                        vOutPorts[i]->index,
                                                        vOutPorts[i]->inout);

        if ( EPortIndex_DISPO == vOutPorts[i]->index ) {
            idx_dispo = i;
            dma_en |= CAM_CTL_DMA_EN_DISPO_EN;
        }
        else if ( EPortIndex_VIDO == vOutPorts[i]->index ) {
            idx_vido = i;
            dma_en |= CAM_CTL_DMA_EN_VIDO_EN;
        }
    }
    //

    this->m_pipePass = (EPipePass)portInfo_imgi.pipePass;
    PIPE_DBG("this->m_pipePass:[%d]",this->m_pipePass);
    switch (portInfo_imgi.pipePass) {
        case EPipePass_PASS2:
            tpipe = CAM_MODE_TPIPE;
            cq_size = cq1_size;
            cq_phy = cq1_phy;
            cq_vir = cq1_vir;
            updaType = m_isPartialUpdate? TPIPE_UPDATE_TYPE_CQ1_PARTIAL_SAVE: TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE;
            break;
        case EPipePass_PASS2B:
            pass2_CQ = CAM_ISP_CQ2;
            pass2_cq_en = CAM_CTL_EN2_CQ2_EN;
            tpipe = CAM_MODE_TPIPE;
            cq_size = cq2_size;
            cq_phy = cq2_phy;
            cq_vir = cq2_vir;
            intb_en = ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST;
            updaType = m_isPartialUpdate? TPIPE_UPDATE_TYPE_CQ2_PARTIAL_SAVE: TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE;
            break;
        case EPipePass_PASS2C:
            pass2_CQ = CAM_ISP_CQ3;
            pass2_cq_en = CAM_CTL_EN2_CQ3_EN;
            tpipe = CAM_MODE_TPIPE;
            cq_size = cq3_size;
            cq_phy = cq3_phy;
            cq_vir = cq3_vir;
            intc_en = ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST;
            updaType = m_isPartialUpdate? TPIPE_UPDATE_TYPE_CQ3_PARTIAL_SAVE: TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE;
            break;
        case EPipePass_PASS2_Phy:   // For GDMA.
            pass2_CQ = CAM_ISP_CQ_NONE;
            tpipe = CAM_MODE_VRMRG;
            break;
        default:
            PIPE_ERR("NOT Support concurrency");
            return MFALSE;
    }

    //
    PIPE_DBG("meScenarioFmt:[%d]",meScenarioFmt);
    switch (meScenarioFmt) {
        case eScenarioFmt_RAW:
            subMode = ISP_SUB_MODE_RAW;
            //eImgFmt_BAYER8 == portInfo_imgi.eImgFmt
            break;
        case eScenarioFmt_YUV:
            subMode = ISP_SUB_MODE_YUV;
            break;
/*
        case eScenarioFmt_RGB:
            subMode = ISP_SUB_MODE_RGB;
            break;
        case eScenarioFmt_JPG:
            subMode = ISP_SUB_MODE_JPG;
            //cam_in_fmt = ;
            break;
        case eScenarioFmt_MFB:
            subMode = ISP_SUB_MODE_MFB;
            //cam_in_fmt = ;
            break;
        case eScenarioFmt_RGB_LOAD:
            subMode = ISP_SUB_MODE_RGB_LOAD;
            break;
*/
        default:
            PIPE_ERR("NOT Support submode");
            return MFALSE;

    }
    //should be before scenario parsing
    PIPE_DBG("portInfo_imgi.eImgFmt:[0x%x]/m_isImgPlaneByImgi(%d)",portInfo_imgi.eImgFmt,m_isImgPlaneByImgi);
    switch( portInfo_imgi.eImgFmt ) {
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
            cam_in_fmt = CAM_FMT_SEL_YUV420_2P;
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vipi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            dmai_swap_uv = (eImgFmt_NV12==portInfo_imgi.eImgFmt)?0:2;
            //
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
            }
            break;
        case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane(YVU)
        case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
            cam_in_fmt = CAM_FMT_SEL_YUV420_3P;
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vipi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vip2i = 1<<CAM_ISP_PIXEL_BYTE_FP;
            //
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
                idx_vip2i = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIP2I_EN;
                //
                dmai_swap_uv = (eImgFmt_I420==portInfo_imgi.eImgFmt)?0:1;
            }
            break;
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            dmai_swap = 1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            dmai_swap = 0;
            break;
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 3;
            break;
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 2;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_I422:      //422 format, 3 plane        
            cam_in_fmt = CAM_FMT_SEL_YUV422_3P;
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vipi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vip2i = 1<<CAM_ISP_PIXEL_BYTE_FP;
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
                idx_vip2i = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIP2I_EN;
            }
            break;
        case eImgFmt_NV16:      //= 0x08000,   //422 format, 2 plane (UV)
        case eImgFmt_NV61:      //= 0x10000,   //422 format, 2 plane (VU)
            cam_in_fmt = CAM_FMT_SEL_YUV422_2P;
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vipi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            dmai_swap_uv = (eImgFmt_NV16==portInfo_imgi.eImgFmt)?0:2;
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
            }
            break;

        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            pixel_byte_imgi = 3<<CAM_ISP_PIXEL_BYTE_FP;
            dmai_swap = 0; //0:RGB,2:BGR
            break;
        case eImgFmt_ARGB888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            pixel_byte_imgi = 4<<CAM_ISP_PIXEL_BYTE_FP;
            dmai_swap = 0; //0:ARGB,1:RGBA,2:ABGR,3:BGRA
            break;
        case eImgFmt_Y800:		//= 0x040000, //Y plane only
            cam_in_fmt = CAM_FMT_SEL_Y_ONLY;
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            break;
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit

        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        default:
            PIPE_ERR("portInfo_imgi.eImgFmt:Format NOT Support");
            return MFALSE;
    }

    //
    PIPE_DBG("meScenarioID:[%d], subMode[%d]",meScenarioID,subMode);

    scenario = meScenarioID;

    switch (meScenarioID) {
        case eScenarioID_CONFIG_FMT: //  Config FMT
        case eScenarioID_VR:         //  Video Recording/Preview
        case eScenarioID_GDMA:       // for GDMA only
            isImg2oStatusFixed = 0; /* for VR */
        case eScenarioID_VEC:        //  Vector Generation
        case eScenarioID_ZSD:        //  Zero Shutter Delay
            if ( dma_en & CAM_CTL_DMA_EN_VIDO_EN ) {
                
            }
            enable1 = 0;
            enable2 |= CAM_CTL_EN2_CURZ_EN|CAM_CTL_EN2_PRZ_EN| \
                        ( pass2_cq_en ? (CAM_CTL_EN2_CQ2_EN|CAM_CTL_EN2_CQ3_EN|pass2_cq_en) : 0);


            int_en = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;
            //
            isIspOn = 0;

            if ( eScenarioID_GDMA == scenario ) {
                scenario = eScenarioID_VR;
                PIPE_INF("GDMA scenario(%d)",scenario);
                //enable GDMA
                enable2 |= CAM_CTL_EN2_GDMA_EN;
                //enable GDMA link
                //enable CRZ_PRZ_MRG_SET
                gdma_link_crz_prz_mrg = 1;
                //disable CURZ, should be OFF once CRZ_PRZ_MRG_SET is on.
                enable2 &= (~CAM_CTL_EN2_CURZ_EN);
                //disable tile mode.
                tpipe = CAM_MODE_VRMRG;
                pass2_CQ = CAM_ISP_CQ_NONE;
            }
            break;
        case eScenarioID_VSS_CDP_CC:
        case eScenarioID_VSS:
            isImg2oStatusFixed = 0; /* for VSS_CSP_CC and VSS */

        case eScenarioID_N3D_IC:
        case eScenarioID_ZSD_CDP_CC:
            #if 0
                1.5.1.2	           CDP Concurrence notes
                        Only CDP can be used for concurrence.
                        //
                        Keep Scenario / Sub-mode, don't change it (ex: at VR_RAW concurrence, still keep VR_RAW)
                        //
                        Set CAM_CTL_SRAMMUX_CFG at IC_JPG and N3D_IC scenario
                            -SRAM_MUX_SET_EN=1
                                //(reg_407C[8],SET_40D0/CLR_40D4)
                            -SRAM_MUX_MODE and Scenario is VR_RAW
                                //(reg_407C[6:4] = 0/reg_407C[2:0] = 1)
                            -SRAM_MUX_TPIPE don't care
                         //
                        RAW_PASS1 always be 0
                            //(reg_4018[29],set reg_40a0, clr reg_40a4)

                        //******************************************************************************
                        //below is for simulation ONLY. no need to set.byTH.
                        Program spare3  register(reg_406c)
                            -CON_SCENARIO_EN=1(reg_406c[2:0]), CON_SUB_MODE_EN=1(reg_406c[10:8])
                            -Set CON_SCENARIO and CON_SUB_MODE be main  scenario and sub_mode
                        //******************************************************************************

                        Use flexible setting for CDP concurrence usage
                            -PASS2_DONE_MUX_EN=1
                                //(reg_4078,[31], set reg_40c8, clr reg_40cc)
                                    -Chose PASS2_DONE_MUX=0,
                                        //(reg_4078,[17:13], set reg_40c8, clr reg_40cc)
                                    -please note vido and dispo is dedicated for pass2
                            -CDP_SEL_EN=1, CDP_SEL=1
                                //(reg_4074[29]/reg_4074[15], set reg_40c0,, clr reg_40c4)
                            -IMGI_MUX_EN =1
                                //(reg_4078,[25], set reg_40c8, clr reg_40cc)
                                -IMGI_STRIDE.FORMAT_EN=1,
                                    //(reg_4240[23])
                                -IMGI_STRIDE.FORMAT=1
                                    //(reg_4240[21:20])
                                -IMGI_STRIDE.BUS_SIZE_EN =1
                                    //(reg_4240[19])
                                -IMGI_STRIDE.BUS_SIZE  = yuv422_1plane ? 1 : 0;
                                    //(reg_4240[17:16])
                             //*****below is MUST to bypass GDMA when using in-DMA.
                            -GDMA flexible
                                -GDMA_SEL_EN = 1 ,
                                    //(reg_407C[20]/SET_40D0/CLR_40D4)
                                -GDMA_SYNC_EN = 1 ,
                                    //(reg_407C[21]/SET_40D0/CLR_40D4)
                                -GDMA_IMGCI_SEL = 0 ,
                                    //(reg_407C[23]/SET_40D0/CLR_40D4)
                                -GDMA_REPEAT = yuv420 ? 1 : 0 ,
                                    //(reg_407C[22]/SET_40D0/CLR_40D4)
                            -CAM_IN_FMT depend on yuv422/yuv420  and plane number,
                                //(reg_4010[11:8],set reg_4098,, clr reg_409c)

                            //******************************************************************************
                            //no need anymore by TH 20120702
                            -RAW_PASS1 =1  for IC_RAW, N3D_IC_RAW and N3D_VR_RAW,      (reg_4018[29],set reg_40a0, clr reg_40a4)
                            //******************************************************************************

                            -Tpipe flexible setting
                                -CTL_TCM_EN. TCM_LOAD_EN =1 ,
                                    //(reg_4054[27])
                                -TCM_IMGI_EN = IMG_EN ,
                                    //(reg_4054[1])
                                -TCM_VIPI_EN   = VIPI_EN ,
                                    //(reg_4054[3])
                                -TCM_VIP2I_EN = VIP2I_EN ,
                                    //(reg_4054[4])
                                -TCM_CURZ_EN = CURZ_EN ,
                                    //(reg_4054[17])
                                -TCM_PRZ_EN = PRZ_EN ,
                                    //(reg_4054[15])
                                -TCM_VIDO_EN = VIDO_EN ,
                                    //(reg_4054[14])
                                -TCM_DISPO_EN = DISPO_EN ,
                                    //(reg_4054[13])
                                -Other TCM_XXX_EN should be 0                                          (&0x0102E01A)
                #endif
            //
            if ( eScenarioID_N3D_IC == meScenarioID || eScenarioID_VSS == meScenarioID) {
                scenario = eScenarioID_N3D_IC;
            }
            else { /* do not change scenario and subMode in eScenarioID_ZSD_CDP_CC & eScenarioID_VSS_CDP_CC */
                scenario = m_CamPathPass2.readReg(0x4010);
                subMode =  (scenario>>4)&0x07;
                scenario &= 0x07;
            }


            //
            if ( eScenarioID_N3D_IC == scenario ) {
                //reg_407C
                ctl_sram_mux_cfg |= 0x00000101;
            }
            //reg_4078[31] = 1
            ctl_mux_sel2 |= 0x80000000;
            //reg_4074[29][15] = 1
            ctl_mux_sel |= 0x20008000;
            //reg_4078[25] = 1
            ctl_mux_sel2 |= 0x02000000;


            #if 0
            //(reg_4240[23] = 1)
            imgi_stride_fmt |= 0x00800000;
            //(reg_4240[21:20])
            imgi_stride_fmt |= 0x00100000;
            //(reg_4240[19])
            imgi_stride_fmt |= 0x00080000;
            //-IMGI_STRIDE.BUS_SIZE  = yuv422_1plane ? 1 : 0;
            //(reg_4240[17:16])
            imgi_stride_fmt |= (eImgFmt_YUY2 == portInfo_imgi.eImgFmt)?0x00010000:0;
            imgi_stride_fmt |= (eImgFmt_UYVY == portInfo_imgi.eImgFmt)?0x00010000:0;
            #else
            imgi_format_en = 1;
            imgi_format = 1;
            imgi_bus_size_en = 1;
            imgi_bus_size = ((eImgFmt_YUY2 == portInfo_imgi.eImgFmt)?1:0) | ((eImgFmt_UYVY == portInfo_imgi.eImgFmt)?1:0);
            #endif

            //
            if ( dma_en & CAM_CTL_DMA_EN_VIDO_EN ) {
             enable2 |= CAM_CTL_EN2_UV_CRSA_EN;
            }
            enable1 = 0;
            enable2 |= CAM_CTL_EN2_UV_CRSA_EN; //mdp direct-link
            enable2 |= CAM_CTL_EN2_CURZ_EN|CAM_CTL_EN2_PRZ_EN| \
                     ( pass2_cq_en ? (CAM_CTL_EN2_CQ2_EN|CAM_CTL_EN2_CQ3_EN|pass2_cq_en) : 0);

            int_en = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;
            //

            //*****below is MUST to bypass GDMA when using in-DMA.
            //-GDMA flexible
            //-GDMA_SEL_EN = 1 ,
            //(reg_407C[20]/SET_40D0/CLR_40D4)
            ctl_sram_mux_cfg |= 0x00100000;
            //-GDMA_SYNC_EN = 1 ,
            //(reg_407C[21]/SET_40D0/CLR_40D4)
            ctl_sram_mux_cfg |= 0x00200000;
            //-GDMA_IMGCI_SEL = 0 ,
            //(reg_407C[23]/SET_40D0/CLR_40D4)
            //ctl_sram_mux_cfg |= 0x00800000;
            //-GDMA_REPEAT = yuv420 ? 1 : 0 ,
            //(reg_407C[22]/SET_40D0/CLR_40D4)
            if ( eImgFmt_NV21 == portInfo_imgi.eImgFmt || \
                 eImgFmt_NV12 == portInfo_imgi.eImgFmt || \
                 eImgFmt_YV12 == portInfo_imgi.eImgFmt || \
                 eImgFmt_I420 == portInfo_imgi.eImgFmt) {
                ctl_sram_mux_cfg |= 0x00400000;
            }
            //
            //-CAM_IN_FMT depend on yuv422/yuv420  and plane number,
            //(reg_4010[11:8],set reg_4098,, clr reg_409c)

            //-Tpipe flexible setting
            //-CTL_TCM_EN. TCM_LOAD_EN =1 ,
            //(reg_4054[27])
            tcm_en = 0x08000000;
            //-TCM_CTL_EN = 1,
            //(reg_4054[0])
            tcm_en |= 0x00000001; //org
            #if 0// 6582 VSS_raw
            tcm_en = 0x8c000003 ;//TEST_MDP         tcm_en = 0x08000000;//            disable IMGI_TCM_UNP_EN
            //SL TEST_RAW_YUV tcm_en = 0x8c100003 ;//TEST_MDP         tcm_en = 0x08000000;
            tcm_en |= ((enable1&&CAM_CTL_EN1_UNP_EN)?0x00100000:0);
            tcm_en |= ((enable1&&CAM_CTL_EN1_LSC_EN)?0x00800000:0); //TEST TCM
            #endif
            
            
            //-TCM_IMGI_EN = IMG_EN ,
            //(reg_4054[1])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMGI_EN)?0x00000002:0);
            //-TCM_IMGCI_EN = IMGCI_EN ,
            //(reg_4054[2])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMGCI_EN)?0x00000004:0);
            //-TCM_VIPI_EN   = VIPI_EN ,
            //(reg_4054[3])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_VIPI_EN)?0x00000008:0);
            //-TCM_VIP2I_EN = VIP2I_EN ,
            //(reg_4054[4])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_VIP2I_EN)?0x00000010:0);
            //(reg_4054[6])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_LCEI_EN)?0x00000040:0);
            //(reg_4054[7])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_LSCI_EN)?0x00000080:0);
            //(reg_4054[8])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMGO_EN)?0x00000100:0);
            //(reg_4054[9])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMG2O_EN)?0x00000200:0);
            //-TCM_DISPO_EN = DISPO_EN ,
            //(reg_4054[13])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_DISPO_EN)?0x00002000:0);
            //-TCM_VIDO_EN = VIDO_EN ,
            //(reg_4054[14])
            tcm_en |= ((dma_en&CAM_CTL_DMA_EN_VIDO_EN)?0x00004000:0);
            //-TCM_PRZ_EN = PRZ_EN ,
            //(reg_4054[15])
            tcm_en |= ((enable2&CAM_CTL_EN2_PRZ_EN)?0x00008000:0);
            //-TCM_CDRZ_EN = CDRZ_EN ,
            //(reg_4054[16])
            tcm_en |= ((enable2&CAM_CTL_EN2_CDRZ_EN)?0x00010000:0);
            //-TCM_CURZ_EN = CURZ_EN ,
            //(reg_4054[17])
            tcm_en |= ((enable2&CAM_CTL_EN2_CURZ_EN)?0x00020000:0);
            //(reg_4054[24])
            tcm_en |= ((enable2&CAM_CTL_EN2_NR3D_EN)?0x01000000:0);
            //-Other TCM_XXX_EN should be 0                                          (&0x0102E01A)

            isIspOn = 0;
            break;
        default:
            PIPE_ERR("NOT Support scenario");
            return MFALSE;
    }


    PIPE_INF("m_pipePass(%d),meScenarioFmt(%d),portInfo_imgi.eImgFmt(%d),m_isImgPlaneByImgi(%d),meScenarioID(%d), subMode(%d)", \
        this->m_pipePass, \
        meScenarioFmt, \
        portInfo_imgi.eImgFmt,\
        m_isImgPlaneByImgi, \
        meScenarioID, \
        subMode);

    /*-----------------------------------------------------------------------------
      m_camPass2Param
      -----------------------------------------------------------------------------*/
    //top
    //scenario/sub_mode
    //m_camPass2Param.scenario = ISP_SCENARIO_VR;
    //m_camPass2Param.subMode =  ISP_SUB_MODE_RAW;
    m_camPass2Param.en_Top.enable1 =  enable1;
    m_camPass2Param.en_Top.enable2 =  enable2;
    //if ( CAM_ISP_CQ1 == vr_param.pass2_CQ ) {
    //    vr_param.enFuncCfg.en_Top.enable2 |= CAM_CTL_EN2_CQ1_EN|CAM_CTL_EN2_CQ2_EN;
    //}
    m_camPass2Param.en_Top.dma = dma_en;
    //ctl_int
    m_camPass2Param.ctl_int.int_en = int_en;
    m_camPass2Param.ctl_int.intb_en = intb_en;
    m_camPass2Param.ctl_int.intc_en = intc_en;

    //fmt_sel
    m_camPass2Param.fmt_sel.reg_val = 0x00; //reset fmt_sel
    m_camPass2Param.fmt_sel.bit_field.scenario = scenario;
    m_camPass2Param.fmt_sel.bit_field.sub_mode = subMode;
    m_camPass2Param.fmt_sel.bit_field.cam_in_fmt = cam_in_fmt;
    //ctl_sel(0x4018)
    //WORKAROUND: to fix CQ0B/CQ0C fail issue
    int DB_en = 1;
    m_camPass2Param.ctl_sel.reg_val = 0;
    m_camPass2Param.ctl_sel.bit_field.CURZ_BORROW = 1; // lend sram to BNR
    m_camPass2Param.ctl_sel.bit_field.tdr_sel = 1;//DB_en?0:1;
    m_camPass2Param.ctl_sel.bit_field.pass2_db_en = 0;//DB_en?1:0;
    m_camPass2Param.ctl_sel.bit_field.pass1_db_en = 1;//DB_en?1:0;
    m_camPass2Param.ctl_sel.bit_field.gdma_link = gdma_link_crz_prz_mrg;
    m_camPass2Param.ctl_sel.bit_field.crz_prz_mrg = gdma_link_crz_prz_mrg;
    if ( m_camPass2Param.ctl_sel.bit_field.tdr_sel == m_camPass2Param.ctl_sel.bit_field.pass2_db_en ) {
        PIPE_DBG("Error:TDR_SEL/PASS2_DB_EN conflict ");
    }

    //mux_sel
    m_camPass2Param.ctl_mux_sel.reg_val = ctl_mux_sel;
    //mux_sel2
    m_camPass2Param.ctl_mux_sel2.reg_val = ctl_mux_sel2;
    //
    m_camPass2Param.ctl_sram_mux_cfg.reg_val = ctl_sram_mux_cfg;
    //
    m_camPass2Param.CQ= pass2_CQ;
    //m_camPass2Param.isConcurrency = portInfo_imgi.pipePass;
    m_camPass2Param.isConcurrency = ((meScenarioID==eScenarioID_ZSD_CDP_CC || meScenarioID==eScenarioID_VSS_CDP_CC)?(1):(0));
    m_camPass2Param.isEn1C24StatusFixed = isEn1C24StatusFixed;
    m_camPass2Param.isEn1C02StatusFixed = isEn1C02StatusFixed;
    m_camPass2Param.isEn1CfaStatusFixed = isEn1CfaStatusFixed;
    m_camPass2Param.isEn1HrzStatusFixed = isEn1HrzStatusFixed;
    m_camPass2Param.isEn1MfbStatusFixed = isEn1MfbStatusFixed;
    m_camPass2Param.isEn2CdrzStatusFixed = isEn2CdrzStatusFixed;
    m_camPass2Param.isEn2G2cStatusFixed = isEn2G2cStatusFixed;
    m_camPass2Param.isEn2C42StatusFixed = isEn2C42StatusFixed;
    m_camPass2Param.isImg2oStatusFixed = isImg2oStatusFixed;
    m_camPass2Param.isImgoStatusFixed = isImgoStatusFixed;
    m_camPass2Param.isAaoStatusFixed = isAaoStatusFixed;
    m_camPass2Param.isEsfkoStatusFixed = isEsfkoStatusFixed;
    m_camPass2Param.isFlkiStatusFixed = isFlkiStatusFixed;
    m_camPass2Param.isLcsoStatusFixed = isLcsoStatusFixed;
    m_camPass2Param.isEn1AaaGropStatusFixed = isEn1AaaGropStatusFixed;
    m_camPass2Param.isApplyTurn = isApplyTurn;
    m_camPass2Param.isShareDmaCtlByTurn = isShareDmaCtlByTurn;

    m_camPass2Param.tpipe = tpipe;
    m_camPass2Param.bypass_ispRawPipe = 1;
    m_camPass2Param.bypass_ispRgbPipe = 1;
    m_camPass2Param.bypass_ispYuvPipe = 1;
    //
    m_camPass2Param.tdri.memBuf.base_vAddr = (MUINT32)pTdriVir;
    m_camPass2Param.tdri.memBuf.base_pAddr = tdriPhy;
    m_camPass2Param.tdri.memBuf.size = 0; //0-> means tpipemain data is filled by pass2, otherwise by pipe driver.
    m_camPass2Param.tcm_en = tcm_en;
    //
    m_camPass2Param.cqi.memBuf.base_vAddr = (unsigned long)cq_vir;
    m_camPass2Param.cqi.memBuf.base_pAddr = cq_phy;
    m_camPass2Param.cqi.memBuf.size = cq_size;
    //
    m_camPass2Param.dispo.crop_en = 0;
    m_camPass2Param.vido.crop_en = 0;
    //

    m_camPass2Param.updateTdri.updateType = updaType;
    m_camPass2Param.updateTdri.partUpdateFlag = TPIPE_UPDATE_IMGI | \
                                                TPIPE_UPDATE_VIDO | \
                                                TPIPE_UPDATE_DISPO;
    //
    if (-1 != idx_imgi ) {
        PIPE_DBG("config imgi[%d, %d] imgiCrop_f(0x%x, 0x%x)[%d, %d, %d, %d]\n",portInfo_imgi.u4ImgWidth,portInfo_imgi.u4ImgHeight, \
                portInfo_imgi.crop.floatX,portInfo_imgi.crop.floatY, portInfo_imgi.crop.x,portInfo_imgi.crop.y, \
                portInfo_imgi.crop.w,portInfo_imgi.crop.h);

        this->configDmaPort(&portInfo_imgi,m_camPass2Param.imgi,(MUINT32)pixel_byte_imgi,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        m_camPass2Param.imgi.format_en = imgi_format_en;
        m_camPass2Param.imgi.format = imgi_format;
        m_camPass2Param.imgi.bus_size_en = imgi_bus_size_en;
        m_camPass2Param.imgi.bus_size = imgi_bus_size;
        m_camPass2Param.imgi.ring_en = portInfo_imgi.u4EnRingBuffer;
        m_camPass2Param.imgi.ring_size= portInfo_imgi.u4RingSize;
        m_camPass2Param.ringTdriCfg.ringBufferMcuRowNo = portInfo_imgi.u4RingBufferMcuRowNo;
        m_camPass2Param.ringTdriCfg.ringBufferMcuHeight = portInfo_imgi.u4RingBufferMcuHeight;
        //
        // for digital zoom crop
        m_camPass2Param.imgi.crop.x = portInfo_imgi.crop.x;
        m_camPass2Param.imgi.crop.floatX = portInfo_imgi.crop.floatX;
        m_camPass2Param.imgi.crop.y = portInfo_imgi.crop.y;
        m_camPass2Param.imgi.crop.floatY = portInfo_imgi.crop.floatY;
        m_camPass2Param.imgi.crop.w = portInfo_imgi.crop.w;
        m_camPass2Param.imgi.crop.h = portInfo_imgi.crop.h;
        //
        if(m_camPass2Param.imgi.ring_en) {
            // alloc big memory for ring tile table extra
            if(pTdriRingVir==NULL && tdriRingPhy==0) {
                tdriRingSize = ISP_MAX_RING_TDRI_SIZE;
                IMEM_BUF_INFO buf_info;
                buf_info.size = tdriRingSize;
                if ( m_pIspDrvShell->m_pIMemDrv->allocVirtBuf(&buf_info) ) {
                    PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                    return MFALSE;
                }
                tdriRingMemId = buf_info.memID;
                pTdriRingVir = (MUINT8*)buf_info.virtAddr;
                if ( m_pIspDrvShell->m_pIMemDrv->mapPhyAddr(&buf_info) ) {
                    PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                    return MFALSE;
                }
                tdriRingPhy =  (MUINT32)buf_info.phyAddr;
                PIPE_DBG("ALLOC pTdriRingVir(0x%x) tdriRingPhy(0x%x)\n",pTdriRingVir,tdriRingPhy);
            }

            //
            PIPE_DBG("[RingTpipeBuffer] tpipe_offset(0x%x)",portInfo_imgi.u4RingTdriBufOffset);
            m_camPass2Param.tdri.memBuf.base_vAddr = (MUINT32)pTdriRingVir + portInfo_imgi.u4RingTdriBufOffset;
            m_camPass2Param.tdri.memBuf.base_pAddr = tdriRingPhy + portInfo_imgi.u4RingTdriBufOffset;
            m_camPass2Param.tdri.memBuf.size = 0; //0-> means tpipemain data is filled by pass2, otherwise by pipe driver.
            m_camPass2Param.updateTdri.updateType = TPIPE_UPDATE_TYPE_FULL;
            //
            if(settingStage == eConfigSettingStage_Init) {
                m_camPass2Param.ringTdriCfg.isCalculateTdri = 1;
                m_camPass2Param.ringTdriCfg.ringConfNumVa = portInfo_imgi.u4RingConfNumVA;
                m_camPass2Param.ringTdriCfg.ringConfVerNumVa = portInfo_imgi.u4RingConfVerNumVA;
                m_camPass2Param.ringTdriCfg.ringErrorControlVa = portInfo_imgi.u4RingErrorControlVA;
                m_camPass2Param.ringTdriCfg.ringConfBufVa = portInfo_imgi.u4RingConfBufVA;
            } else if(settingStage == eConfigSettingStage_UpdateTrigger) {
                m_camPass2Param.ringTdriCfg.isCalculateTdri = 0;
            } else {
                PIPE_ERR("[RingTpipeBuffer]settingStage(%d) error!",settingStage);
                return MFALSE;
            }
        } else if (portInfo_imgi.u4IsRunSegment) { // for VSS tpipe capture
            PIPE_DBG("run vss tpipe capture\n");

            if(settingStage == eConfigSettingStage_Init) {
                m_camPass2Param.capTdriCfg.isCalculateTpipe = 1;
                m_camPass2Param.capTdriCfg.isRunSegment = portInfo_imgi.u4IsRunSegment;
                m_camPass2Param.capTdriCfg.setSimpleConfIdxNumVa = portInfo_imgi.u4SegNumVa;
                m_camPass2Param.capTdriCfg.segSimpleConfBufVa = (MUINT32)pTpipeConfigVa;

            } else if(settingStage == eConfigSettingStage_UpdateTrigger) {

                PIPE_DBG("u4SegTpipeSimpleConfigIdx(%d)\n",portInfo_imgi.u4SegTpipeSimpleConfigIdx);

                m_camPass2Param.capTdriCfg.isCalculateTpipe = 0;

                if(portInfo_imgi.u4SegTpipeSimpleConfigIdx < segmSimpleConfIdxNum) {
                    m_camPass2Param.tdri.memBuf.base_pAddr = tdriPhy + pTpipeConfigVa[portInfo_imgi.u4SegTpipeSimpleConfigIdx];
                } else {
                    PIPE_ERR("u4SegTpipeSimpleConfigIdx(%d) over max value(%d) error!",portInfo_imgi.u4SegTpipeSimpleConfigIdx,segmSimpleConfIdxNum);
                    return MFALSE;
                }

            } else {
                PIPE_ERR("[VssCapture]settingStage(%d) error!",settingStage);
                return MFALSE;
            }
        }
        //
        m_camPass2Param.imgi.memBuf_c_ofst = 0;
        m_camPass2Param.imgi.memBuf_v_ofst = 0;
        //config n-plane in by imgi only
        if (m_isImgPlaneByImgi) {
            portInfo_vipi = portInfo_imgi;
            portInfo_vipi.u4BufPA = 0x01;//for NOT bypass vipi config in pass2
            portInfo_vip2i = portInfo_imgi;
            portInfo_vip2i.u4BufPA = 0x01;//for NOT bypass vip2i config in pass2

            switch( portInfo_imgi.eImgFmt ) {
                case eImgFmt_NV16:      //= 0x8000,   //422 format, 2 plane
                case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
                case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE] * m_camPass2Param.imgi.size.h;
                    break;
                case eImgFmt_YV12:      //= 0x00008,    //420 format, 3 plane (YVU)
                case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
                    if (0==dmai_swap_uv) {
                        //Y plane size
                        m_camPass2Param.imgi.memBuf_c_ofst = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE] * m_camPass2Param.imgi.size.h;
                        //Y+U plane size (Y + 1/4Y)
                        //m_camPass2Param.imgi.memBuf_v_ofst = (m_camPass2Param.imgi.memBuf_c_ofst*5)>>2;
                        //Y+U plane size
                        m_camPass2Param.imgi.memBuf_v_ofst = m_camPass2Param.imgi.memBuf_c_ofst + portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE] * (m_camPass2Param.imgi.size.h >> 1);
                    }
                    else {
                        //Y plane size
                        m_camPass2Param.imgi.memBuf_v_ofst = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE] * m_camPass2Param.imgi.size.h;
                        m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.memBuf_v_ofst + portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE] * (m_camPass2Param.imgi.size.h >> 1);
                    }

                    //
                    portInfo_vipi.u4ImgWidth    >>= 1;
                    portInfo_vipi.u4ImgHeight   >>= 1;
                    portInfo_vipi.crop.x        >>= 1;
                    portInfo_vipi.crop.floatX   >>= 1;
                    portInfo_vipi.crop.y        >>= 1;
                    portInfo_vipi.crop.floatY   >>= 1;
                    portInfo_vipi.crop.w        >>= 1;
                    portInfo_vipi.crop.h        >>= 1;
                    //
                    portInfo_vip2i.u4ImgWidth    >>= 1;
                    portInfo_vip2i.u4ImgHeight   >>= 1;
                    portInfo_vip2i.crop.x        >>= 1;
                    portInfo_vip2i.crop.floatX   >>= 1;
                    portInfo_vip2i.crop.y        >>= 1;
                    portInfo_vip2i.crop.floatY   >>= 1;
                    portInfo_vip2i.crop.w        >>= 1;
                    portInfo_vip2i.crop.h        >>= 1;

                    break;
                case eImgFmt_YV16:      //= 0x4000,   //422 format, 3 plane
                case eImgFmt_I422:      //
                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE] * m_camPass2Param.imgi.size.h;;
                    //Y+U plane size (Y + 1/2Y)
                    //m_camPass2Param.imgi.memBuf_v_ofst = (m_camPass2Param.imgi.memBuf_c_ofst*3)>>1;
                    //Y+U plane size
                    m_camPass2Param.imgi.memBuf_v_ofst = m_camPass2Param.imgi.memBuf_c_ofst + portInfo_imgi.u4Stride[ESTRIDE_2ND_PLANE] * m_camPass2Param.imgi.size.h ;
                    //
                    portInfo_vipi.u4ImgWidth >>= 1;
                    //portInfo_vipi.u4ImgHeight;
                    portInfo_vipi.crop.x>>= 1;
                    portInfo_vipi.crop.floatX>>= 1;
                    //portInfo_vipi.crop.y;
                    portInfo_vipi.crop.w>>= 1;
                    //portInfo_vipi.crop.h;
                    //
                    portInfo_vip2i.u4ImgWidth >>= 1;
                    //portInfo_vip2i.u4ImgHeight;
                    portInfo_vip2i.crop.x>>= 1;
                    portInfo_vip2i.crop.floatX>>= 1;
                    //portInfo_vip2i.crop.y;
                    portInfo_vip2i.crop.w>>= 1;
                    //portInfo_vip2i.crop.h;

                    break;
                case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
                case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
                case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
                case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
                case eImgFmt_ARGB888:   //= 0x1000,   //ARGB (32-bit), 1 plane
                case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit
                case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit
                case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit
                case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
                case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
                case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
                default:
                    break;
            }
        }

    }
    //
    if (-1 != idx_vipi ) {
        PIPE_DBG("config vipi");
        this->configDmaPort(&portInfo_vipi,m_camPass2Param.vipi,(MUINT32)pixel_byte_vipi,(MUINT32)dmai_swap_uv,(MUINT32)1,ESTRIDE_2ND_PLANE);
        //
        m_camPass2Param.vipi.format_en = vipi_format_en;
        m_camPass2Param.vipi.format = vipi_format;
        m_camPass2Param.vipi.bus_size_en = vipi_bus_size_en;
        m_camPass2Param.vipi.bus_size = vipi_bus_size;
        m_camPass2Param.vipi.ring_en = portInfo_vipi.u4EnRingBuffer;
        m_camPass2Param.vipi.ring_size= portInfo_vipi.u4RingSize;
        //
    }
    //
    if (-1 != idx_vip2i ) {
        PIPE_DBG("config vip2i");
        this->configDmaPort(&portInfo_vip2i,m_camPass2Param.vip2i,(MUINT32)pixel_byte_vip2i,(MUINT32)0,(MUINT32)1,ESTRIDE_3RD_PLANE);
        //
        m_camPass2Param.vip2i.format_en = vip2i_format_en;
        m_camPass2Param.vip2i.format = vip2i_format;
        m_camPass2Param.vip2i.ring_en = portInfo_vip2i.u4EnRingBuffer;
        m_camPass2Param.vip2i.ring_size= portInfo_vip2i.u4RingSize;
        //
    }
    //
    if ( -1 != idx_dispo) {
        PIPE_DBG("config dispo");
        //
        configCdpOutPort(vOutPorts[idx_dispo],m_camPass2Param.dispo);
        //dispo not support rotation/flip
        //m_camPass2Param.dispo.Rotation = CDP_DRV_ROTATION_0;
        //m_camPass2Param.dispo.Flip = MFALSE;
        //
        //m_camPass2Param.dispo.memBuf.base_vAddr = vOutPorts[idx_dispo]->u4BufVA;
        //m_camPass2Param.dispo.memBuf.base_pAddr = vOutPorts[idx_dispo]->u4BufPA;
    }
    //
    if ( -1 != idx_vido) {
        PIPE_DBG("config vido");
        //
        configCdpOutPort(vOutPorts[idx_vido],m_camPass2Param.vido);
        //
        //m_camPass2Param.vido.memBuf.base_vAddr = vOutPorts[idx_vido]->u4BufVA;
        //m_camPass2Param.vido.memBuf.base_pAddr = vOutPorts[idx_vido]->u4BufPA;
    }
    //

    PIPE_DBG("ring_en(%d) tpipe(%d)\n",m_camPass2Param.imgi.ring_en,m_camPass2Param.tpipe);

    if(m_camPass2Param.imgi.ring_en && m_camPass2Param.tpipe) {
        ret = m_CamPathPass2.configRingTpipe( &m_camPass2Param);
    }
    else {
        ret = m_CamPathPass2.config( &m_camPass2Param);
    }
    if( ret != 0 ) {
        PIPE_ERR("Pass 2 config error ring_en(%d)!",m_camPass2Param.imgi.ring_en);
        return MFALSE;
    } else {
        if(m_camPass2Param.capTdriCfg.isRunSegment && settingStage == eConfigSettingStage_Init) {
            MUINT32 *pSetSimpleConfIdxNumVa = (MUINT32 *)portInfo_imgi.u4SegNumVa;
            segmSimpleConfIdxNum = *pSetSimpleConfIdxNumVa;
        }
    }

    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    MBOOL ret;
    //
    m_isPartialUpdate = MTRUE;
    //
    ret = configPipe(vInPorts,vOutPorts);
    //
    m_isPartialUpdate = MFALSE;
    //
    if (MFALSE == ret)  {
        PIPE_ERR("Error:configPipeUpdate ");
        return MFALSE;
    }

    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CdpPipe::
onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2)
{
int ret = 0;

    PIPE_DBG("tid(%d) (u4Param1, u4Param2)=(0x%08x, 0x%08x)", gettid(), u4Param1, u4Param2);

    switch ( u4Param1 ) {
/*
        case EPIPECmd_SET_ZOOM_RATIO:
        ret = m_CamPathPass2.setZoom( u4Param2 );
        break;
*/
        default:
            PIPE_ERR("NOT support command!");
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("onSet2Params error!");
        return MFALSE;
    }

    return  MTRUE;
}


/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CdpPipe::
onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam)
{
    PIPE_DBG("tid(%d) (u4InParam)=(%d)", gettid(), u4InParam);
    *pu4OutParam = 0x12345678;
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
irq(EPipePass pass, EPipeIRQ irq_int)
{
int    ret = 0;
MINT32 type = 0;
MUINT32 irq = 0;

    PIPE_DBG("tid(%d) (type,irq)=(0x%08x,0x%08x)", gettid(), pass, irq_int);

    //irq_int
    if ( EPIPEIRQ_PATH_DONE != irq_int || EPIPEIRQ_VSYNC != irq_int) {
    }
    else {
        PIPE_ERR("IRQ:NOT SUPPORT irq for PASS2");
        return MFALSE;
    }
    //pass
    if ( ( EPipePass_PASS2 == pass ) || ( EPipePass_PASS2_Phy == pass ) ){
        type = ISP_DRV_IRQ_TYPE_INT;
        irq = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;
    }
    else if ( EPipePass_PASS2B == pass ) {
        type = ISP_DRV_IRQ_TYPE_INTB;
        irq = ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST;
    }
    else if ( EPipePass_PASS2C == pass ) {
        type = ISP_DRV_IRQ_TYPE_INTC;
        irq = ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST;
    }
    else if ( EPipePass_PASS1_TG1 == pass ) {  // for jpeg ring (jpeg will get isp information from pipe mgr)
        type = ISP_DRV_IRQ_TYPE_INT;
        irq = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    }
    else {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        return MFALSE;
    }
    //
    PIPE_DBG("(type,irq)=(0x%08x,0x%08x)", type, irq);
    //
    ret = m_CamPathPass2.waitIrq(type,irq);

    if( ret != 0 )
    {
        PIPE_ERR("waitIrq error!");
        return  MFALSE;
    }
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    int    ret = 0;
    MUINT32 dmaChannel = 0;
    PIPE_DBG("tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

    switch ( cmd ) {
/*
        case EPIPECmd_SET_ZOOM_RATIO:
        ret = m_CamPathPass2.setZoom( arg1 );
        break;
*/
        case EPIPECmd_SET_BASE_ADDR:

            break;
        case EPIPECmd_SET_CURRENT_BUFFER:
            if ( EPortIndex_IMGI == arg1 ) {
                dmaChannel = ISP_DMA_IMGI;
            }
            if ( EPortIndex_VIPI == arg1 ) {
                dmaChannel = ISP_DMA_VIPI;
            }
            if ( EPortIndex_VIP2I == arg1 ) {
                dmaChannel = ISP_DMA_VIP2I;
            }
            if ( EPortIndex_DISPO == arg1 ) {
                dmaChannel = ISP_DMA_DISPO;
            }
            if ( EPortIndex_VIDO == arg1 ) {
                dmaChannel = ISP_DMA_VIDO;
            }
            //
            m_CamPathPass2.setDMACurrBuf((MUINT32) dmaChannel);
            break;
        case EPIPECmd_SET_NEXT_BUFFER:
            if ( EPortIndex_IMGI == arg1 ) {
                dmaChannel = ISP_DMA_IMGI;
            }
            if ( EPortIndex_VIPI == arg1 ) {
                dmaChannel = ISP_DMA_VIPI;
            }
            if ( EPortIndex_VIP2I == arg1 ) {
                dmaChannel = ISP_DMA_VIP2I;
            }
            if ( EPortIndex_DISPO == arg1 ) {
                dmaChannel = ISP_DMA_DISPO;
            }
            if ( EPortIndex_VIDO == arg1 ) {
                dmaChannel = ISP_DMA_VIDO;
            }
            //
            m_CamPathPass2.setDMANextBuf((MUINT32) dmaChannel);
            break;
        case EPIPECmd_SET_CQ_CHANNEL:
            m_pass2_CQ = arg1;//CAM_ISP_CQ0
            m_CamPathPass2.CQ = m_pass2_CQ;
            m_CamPathPass2.flushCqDescriptor((MUINT32) m_pass2_CQ);
            break;
        case EPIPECmd_SET_CONFIG_STAGE:
            m_settingStage = (EConfigSettingStage)arg1;
            break;
        case EPIPECmd_SET_FMT_START:
            if ( arg1 ) {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_BITS(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                            CAM_CTL_START,FMT_START,
                                            1);
                }
                else
                {
                    ISP_WRITE_BITS(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_START,FMT_START, 1);
                }
            }
            break;
        case EPIPECmd_SET_FMT_EN:
            /*if ( arg1 ) {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_BITS(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                            CAM_CTL_EN2_SET,FMT_EN_SET,
                                            1);
                }
                else
                {
                    ISP_WRITE_BITS(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2_SET,FMT_EN_SET, 1);
                }
            }
            else {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_BITS(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                            CAM_CTL_EN2_CLR,FMT_EN_CLR,
                                            1);
                }
                else
                {
                    ISP_WRITE_BITS(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2_CLR,FMT_EN_CLR, 1);
                }
            }*/
            break;
        case EPIPECmd_GET_FMT:
            if ( arg1)  {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    *(MUINT32*)arg1 = ISP_IOCTL_READ_REG(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                                            CAM_CTL_EN2)& CAM_CTL_EN2_FMT_EN;
                }
                else
                {
                    *(MUINT32*)arg1 = ISP_READ_REG(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2)& CAM_CTL_EN2_FMT_EN;
                }
            }
            else {
                PIPE_ERR("EPIPECmd_GET_FMT:NULL PARAM BUFFER");
                return MFALSE;
            }
            break;
/*
        case EPIPECmd_SET_GDMA_LINK_EN:
            if ( CAM_ISP_CQ_NONE != m_pass2_CQ ) {
                m_CamPathPass2.ispTopCtrl.ispDrvSwitch2Virtual(m_pass2_CQ);
            }
            //
            if ( arg1 ) {
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_EN2_SET,GDMA_EN_SET) = 1;    //0x15004088[25]
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_EN2_CLR,GDMA_EN_CLR) = 0;    //0x1500408C[25]
                //
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_SEL_SET,GDMA_LINK_SET) = 1;  //0x150040A0[1]
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_SEL_SET,CRZ_PRZ_MRG_SET) = 1;//0x150040A0[0]
                //disbale tile
                ISP_REG(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_TCM_EN) = 0;
            }
            else {
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_EN2_SET,GDMA_EN_SET) = 0;
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_EN2_CLR,GDMA_EN_CLR) = 1;

                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_SEL_CLR,GDMA_LINK_CLR) = 1;
                ISP_BITS(m_CamPathPass2.ispTopCtrl.getPhyIspReg(),CAM_CTL_SEL_CLR,CRZ_PRZ_MRG_CLR) = 1;
            }
            //
            if ( CAM_ISP_CQ_NONE != m_pass2_CQ ) {
                m_CamPathPass2.ispTopCtrl.ispDrvSwitch2Phy();
            }

            break;
*/
        case EPIPECmd_GET_GDMA:
            //GDMA_EN
            if ( arg1)  {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    *(MUINT32*)arg1 = ISP_IOCTL_READ_REG(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                                            CAM_CTL_EN2)& CAM_CTL_EN2_GDMA_EN;
                }
                else
                {
                    *(MUINT32*)arg1 = ISP_READ_REG(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2)& CAM_CTL_EN2_GDMA_EN;
                }
            }
            else {
                PIPE_ERR("EPIPECmd_GET_FMT:NULL PARAM BUFFER");
                return MFALSE;
            }
            //GDMA_LINK
            if ( arg2)  {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    *(MUINT32*)arg2 = ISP_IOCTL_READ_REG(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                                            CAM_CTL_SEL)& 0x00000002;
                }
                else
                {
                    *(MUINT32*)arg2 = ISP_READ_REG(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_SEL)& 0x00000002;
                }
            }
            else {
                PIPE_ERR("EPIPECmd_GET_FMT:NULL PARAM BUFFER");
                return MFALSE;
            }
            break;
        case EPIPECmd_SET_CAM_CTL_DBG:
            if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(    m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                        m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                        CAM_CTL_DBG_SET,arg1);
            }
            else
            {
                ISP_WRITE_REG(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_DBG_SET,arg1);
            }
            break;
        case EPIPECmd_GET_CAM_CTL_DBG:
            if (arg1) {
                if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
                {
                    *(MUINT32*)arg1 = ISP_IOCTL_READ_REG(   m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                                            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                                            CAM_CTL_DBG_PORT);
                }
                else
                {
                    *(MUINT32*)arg1 = ISP_READ_REG(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_DBG_PORT);
                }
            }
            break;
        case EPIPECmd_SET_IMG_PLANE_BY_IMGI:
            m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_ISP_RESET:
            PIPE_INF("EPIPECmd_ISP_RESET");
            m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->reset();
            break;
        default:
            PIPE_ERR("NOT support command!");
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("sendCommand error!");
        return MFALSE;
    }
    return  MTRUE;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

