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
#define LOG_TAG "iio/ppp"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "PipeImp.h"
#include "PostProcPipe.h"
//
#include <cutils/properties.h>  // For property_get().


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""



#define SET_TCM_SETTING \
    tcm_en = 0x08000000; /* (reg_4054[27] */    \
    tcm_en |= 0x00000001; /* (reg_4054[0]) */   \
    tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMGI_EN)? 0x00000002:0);/*(reg_4054[1])*/     \
    tcm_en |= ((enable1&CAM_CTL_EN1_UNP_EN)?    0x00100000:0);/*(reg_4054[20])*/


//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);


/*******************************************************************************
*
********************************************************************************/
PostProcPipe::
PostProcPipe(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EScenarioID const eScenarioID,
    EScenarioFmt const eScenarioFmt
)
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt),
      m_pIspDrvShell(NULL),
      m_pipePass(EPipePass_PASS2),
      m_pass2_CQ(CAM_ISP_CQ_NONE),
      m_isImgPlaneByImgi(MFALSE)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    memset(&this->m_camPass2Param,0x00,sizeof(CamPathPass2Parameter));
    this->m_vBufImgi.resize(1);
    this->m_vBufVipi.resize(1);
    this->m_vBufVip2i.resize(1);
    this->m_vBufDispo.resize(1);
    this->m_vBufVido.resize(1);

    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance();

    // create MdpMgr object
    if(MFALSE == m_CamPathPass2.cdpPipe.createMdpMgr())
    {
        PIPE_ERR("createMdpMgr fail");
    }    
}

PostProcPipe::
~PostProcPipe()
{
    /*** release isp driver ***/
    m_pIspDrvShell->destroyInstance();
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
init()
{
    PIPE_DBG(":E");

    //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init();
    m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountInc();

//js_test, to reset register value
//m_pIspDrvShell->getPhyIspDrv()->reset();

        //
        m_CamPathPass2.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
    }

    // alloc tpipe table
    tdriSize = ISP_MAX_TDRI_HEX_SIZE;
    IMEM_BUF_INFO buf_info;
    buf_info.size = tdriSize;
    if ( m_pIspDrvShell->m_pIMemDrv->allocVirtBuf(&buf_info) ) {
        PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        return MFALSE;
    }
    tdriMemId = buf_info.memID;
    pTdriVir = (MUINT8*)buf_info.virtAddr;
    //
    if ( m_pIspDrvShell->m_pIMemDrv->mapPhyAddr(&buf_info) ) {
        PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        return MFALSE;
    }
    tdriPhy =  (MUINT32)buf_info.phyAddr;
    PIPE_DBG("tdriPhy(0x%x)\n",tdriPhy);
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
    segmSimpleConfIdxNum = 0;
    //
    PIPE_INF("m_pIspDrvShell(0x%x) pTdriVir(0x%x) tdriPhy(0x%x) configVa(0x%x)",
        m_pIspDrvShell,pTdriVir,tdriPhy,pTpipeConfigVa);

    PIPE_DBG("X");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
uninit()
{
    PIPE_DBG(":E");
    IMEM_BUF_INFO buf_info;

    // free tpipe table
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
    // free tpipe simple configure table
    buf_info.size = tpipe_config_size;
    buf_info.memID = tpipe_config_memId;
    buf_info.virtAddr = (MUINT32)pTpipeConfigVa;
    if ( m_pIspDrvShell->m_pIMemDrv->freeVirtBuf(&buf_info) ) {
        PIPE_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
        return MFALSE;
    }
    //
    m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountDec();
    //
    m_pIspDrvShell->uninit();

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
start()
{
int path  = CAM_ISP_PASS2_START;


    PIPE_DBG(":E:pass[%d] +",this->m_pipePass);

    if ( EPipePass_PASS2 == this->m_pipePass ) 
    {
        path  = CAM_ISP_PASS2_START;
    }
    else if ( EPipePass_PASS2B == this->m_pipePass ) 
    {
        path  = CAM_ISP_PASS2B_START;
    }
    else if ( EPipePass_PASS2C == this->m_pipePass ) 
    {
        path  = CAM_ISP_PASS2C_START;
    }

    m_CamPathPass2.start((void*)&path);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
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
PostProcPipe::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;

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
    else if (EPortIndex_IMGCI == portID.index) {
        dmaChannel = ISP_DMA_IMGCI;
    }
    else if (EPortIndex_LSCI == portID.index) {
        dmaChannel = ISP_DMA_LSCI;
    }
    else if (EPortIndex_LCEI == portID.index) {
        dmaChannel = ISP_DMA_LCEI;
    }

    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    bufInfo.memID = rQBufInfo.vBufInfo[0].memID;
    bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize;
    bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu;
    bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe;
    if ( 0 != this->m_CamPathPass2.enqueueBuf( dmaChannel , bufInfo ) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }
    //

    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;

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
    else if (EPortIndex_IMGCI == portID.index) {
        dmaChannel = ISP_DMA_IMGCI;
    }
    else if (EPortIndex_LSCI == portID.index) {
        dmaChannel = ISP_DMA_LSCI;
    }
    else if (EPortIndex_LCEI == portID.index) {
        dmaChannel = ISP_DMA_LCEI;
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
        rQBufInfo.vBufInfo.at(i).u4BufVA = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufSize = bufList.front().size;
        bufList.pop_front();
    }
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
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
    else if (EPortIndex_IMGO == portID.index) {
        dmaChannel = ISP_DMA_IMGO;
    }

    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    bufInfo.memID = rQBufInfo.vBufInfo[0].memID;
    bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize;
    bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu;
    bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe;

    if ( 0 != this->m_CamPathPass2.enqueueBuf( dmaChannel, bufInfo ) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }

    PIPE_DBG("[%d]:0x%08d,0x%08x,0x%08x ",portID.index,
                                        rQBufInfo.vBufInfo[0].u4BufSize,
                                        rQBufInfo.vBufInfo[0].u4BufVA,
                                        rQBufInfo.vBufInfo[0].u4BufPA);

    PIPE_DBG(":X");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 ret = 0;
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
    else if (EPortIndex_IMGO == portID.index) {
        dmaChannel = ISP_DMA_IMGO;
    }

    
    bufInfo.pBufList = &bufList;

    ret = this->m_CamPathPass2.dequeueBuf( dmaChannel,bufInfo);
    if(ret == -2)
    {
        return MFALSE;        
    }
    else if(ret != 0) 
    {
        PIPE_ERR("ERROR:dequeueBuf");
        return MFALSE;
    }
    //
    rQBufInfo.vBufInfo.resize(bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID = bufList.front().memID;
        rQBufInfo.vBufInfo.at(i).u4BufVA = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo.at(i).u4BufPA = bufList.front().base_pAddr;
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
MBOOL PostProcPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    int ret = 0;
    int idx_imgi = -1;
    int idx_vipi = -1;
    int idx_vip2i = -1;
    int idx_dispo = -1;
    int idx_vido = -1;
    int idx_imgo = -1;
    int idx_imgci = -1;
    int idx_lsci = -1;
    int idx_lcei = -1;

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
    int tpipe = 0;
    int isIspOn = 0;
    int tcm_en = 0;
    //
    int eis_raw_sel = 0;
    //
    int isEn1C24StatusFixed = 1;  /* 0:C24(en1) won't be clear */
    int isEn1C02StatusFixed = 1;  /* 0:C02(en1) won't be clear */
    int isEn1CfaStatusFixed = 1; //test Green 0;  /* 0:Cfa(en1) can be clear */
    int isEn1HrzStatusFixed = 0;  /* 0:hrz(en1) can be clear */
    int isEn1MfbStatusFixed = 1;  /* 0:mfb(en1) won't be clear */
    int isEn2CdrzStatusFixed = 0;  /* 0:Cdrz(en2) can be clear */
    int isEn2G2cStatusFixed = 0;  /* 0:G2c(en2) can be clear */
    int isEn2C42StatusFixed = 0;  /* 0:c42(en2) can be clear */
    int isImg2oStatusFixed = 1; /* 1:img2o won't be clear */
    int isImgoStatusFixed = 1;  /* 1:imgo won't be clear */
    int isAaoStatusFixed = 1;  /* 1:aao won't be clear */
    int isEsfkoStatusFixed = 1;  /* 1:Esfko won't be clear */
    int isFlkiStatusFixed = 1;  /* 1:Flki won't be clear */
    int isLcsoStatusFixed = 1;  /* 1:Lcso won't be clear */
    int isEn1AaaGropStatusFixed = 1;  /* 1:SGG_EN,AF_EN,FLK_EN,AA_EN,LCS_EN won't be clear */
    int isApplyTurn = 0;  /* 1:apply turning(FeatureIO) setting */
    int isShareDmaCtlByTurn =1; /* 1:share DMA(imgci,lsci and lcei) ctl by turning */

    //
    int ctl_mux_sel = 0;
    int ctl_mux_sel2 = 0;
    int ctl_sram_mux_cfg = 0;
    TPIPE_UPDATE_TYPE updaType = TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE;
    EConfigSettingStage settingStage = m_settingStage;

#if 0 // no use CQ
    int pass2_CQ = CAM_ISP_CQ_NONE;//
    int pass2_cq_en = 0;//
#else   //use CQ1
    int pass2_CQ = m_pass2_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//
    int pass2_cq_en = 0;// = (CAM_ISP_CQ0 == pass1_CQ)?CAM_CTL_EN2_CQ0_EN:((CAM_ISP_CQ0B == pass1_CQ)?CAM_CTL_EN2_CQ0B_EN:);//0; //
    if ( CAM_ISP_CQ_NONE != pass2_CQ) 
    {
        if( CAM_ISP_CQ1 == pass2_CQ )
        {
            pass2_cq_en = CAM_CTL_EN2_CQ1_EN;
        }
        
        if( CAM_ISP_CQ2 == pass2_CQ )
        {
            pass2_cq_en = CAM_CTL_EN2_CQ2_EN;
        }
        
        if( CAM_ISP_CQ3 == pass2_CQ )
        {
            pass2_cq_en = CAM_CTL_EN2_CQ3_EN;
        }
    }
#endif

    int pixel_byte_imgi = 1;
    int pixel_byte_vipi = 1;
    int pixel_byte_vip2i = 1;
    int pixel_byte_imgci = 1;
    int pixel_byte_lsci = 1;
    int pixel_byte_lcei = 1;
    int pixel_byte_imgo = 1;
    MUINT32 cq_size = 0,cq_phy = 0;
    MUINT8 *cq_vir = NULL;
    //
    PortInfo portInfo_imgi;
    PortInfo portInfo_vipi;
    PortInfo portInfo_vip2i;
    PortInfo portInfo_imgci;
    PortInfo portInfo_lsci;
    PortInfo portInfo_lcei;
    PortInfo portInfo_imgo;
    //

    PIPE_DBG("settingStage(%d) in[%d]/out[%d]", settingStage, vInPorts.size(), vOutPorts.size());

    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) 
    {
        if( 0 == vInPorts[i] )
        {
            continue;
        }
        
        PIPE_DBG("vInPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d)",
                                                        i,
                                                        vInPorts[i]->eImgFmt,
                                                        vInPorts[i]->u4ImgWidth,
                                                        vInPorts[i]->u4ImgHeight,
                                                        vInPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vInPorts[i]->type,
                                                        vInPorts[i]->index,
                                                        vInPorts[i]->inout);
        //
        if ( EPortIndex_IMGI == vInPorts[i]->index ) 
        {
            idx_imgi = i;
            dma_en |=  CAM_CTL_DMA_EN_IMGI_EN;
            portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];
        }
        else if ( EPortIndex_VIPI == vInPorts[i]->index ) 
        {
            idx_vipi = i;
            dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
            portInfo_vipi =  (PortInfo)*vInPorts[idx_vipi];
        }
        else if ( EPortIndex_VIP2I == vInPorts[i]->index ) 
        {
            idx_vip2i = i;
            dma_en |=  CAM_CTL_DMA_EN_VIP2I_EN;
            portInfo_vip2i =  (PortInfo)*vInPorts[idx_vip2i];
        }
        else if ( EPortIndex_IMGCI == vInPorts[i]->index ) 
        {
            idx_imgci = i;
            dma_en |=  CAM_CTL_DMA_EN_IMGCI_EN;
            portInfo_imgci =  (PortInfo)*vInPorts[idx_imgci];
        }
        else if ( EPortIndex_LSCI == vInPorts[i]->index ) 
        {
            idx_lsci = i;
            dma_en |=  CAM_CTL_DMA_EN_LSCI_EN;
            portInfo_lsci =  (PortInfo)*vInPorts[idx_lsci];
        }
        else if ( EPortIndex_LCEI == vInPorts[i]->index ) 
        {
            idx_lcei = i;
            dma_en |=  CAM_CTL_DMA_EN_LCEI_EN;
            portInfo_lcei =  (PortInfo)*vInPorts[idx_lcei];
        }
    }
    //
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) 
    {
        if ( 0 == vOutPorts[i] ) { continue; }
        //
        PIPE_DBG("vOutPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d)",i,
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
        else if ( EPortIndex_IMGO == vOutPorts[i]->index ) {
            idx_imgo = i;
            dma_en |= CAM_CTL_DMA_EN_IMGO_EN;
            portInfo_imgo =  (PortInfo)*vOutPorts[idx_imgo];
        }

    }
    //
    this->m_pipePass = (EPipePass)portInfo_imgi.pipePass;
    PIPE_DBG("this->m_pipePass:[%d]",this->m_pipePass);
    switch (this->m_pipePass) 
    {
        case EPipePass_PASS2:
            tpipe = 1;
            cq_size = cq1_size;
            cq_phy = cq1_phy;
            cq_vir = cq1_vir;
            updaType = TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE;
            break;
        case EPipePass_PASS2B:
            pass2_CQ = CAM_ISP_CQ2;
            pass2_cq_en = CAM_CTL_EN2_CQ2_EN;
            tpipe = 1;
            cq_size = cq2_size;
            cq_phy = cq2_phy;
            cq_vir = cq2_vir;
            intb_en = ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST;
            updaType = TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE;
            break;
        case EPipePass_PASS2C:
            pass2_CQ = CAM_ISP_CQ3;
            pass2_cq_en = CAM_CTL_EN2_CQ3_EN;
            tpipe = 1;
            cq_size = cq3_size;
            cq_phy = cq3_phy;
            cq_vir = cq3_vir;
            intc_en = ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST;
            updaType = TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE;
            break;
        default:
            PIPE_ERR("NOT Support concurrency");
            return MFALSE;
    }
    //
    PIPE_DBG("meScenarioFmt:[%d]",meScenarioFmt);
    switch (meScenarioFmt) {
        case eScenarioFmt_RAW:
            isApplyTurn = 1;            
            isEn1CfaStatusFixed = 1; //Test Green
            subMode = ISP_SUB_MODE_RAW;
            //eImgFmt_BAYER8 == portInfo_imgi.eImgFmt
            if ( dma_en & CAM_CTL_DMA_EN_VIDO_EN ) {
                enable2 |= CAM_CTL_EN2_UV_CRSA_EN;
            }
            enable1 = CAM_CTL_EN1_CAM_EN|CAM_CTL_EN1_CFA_EN|CAM_CTL_EN1_UNP_EN;//|CAM_CTL_EN1_GGM_EN;
            enable2 |= CAM_CTL_EN2_UV_CRSA_EN; //mdp direct-link
            enable2 |= CAM_CTL_EN2_CDRZ_EN|CAM_CTL_EN2_C42_EN|CAM_CTL_EN2_G2C_EN|CAM_CTL_EN2_PRZ_EN| \
                        ( pass2_cq_en ? (CAM_CTL_EN2_CQ2_EN|CAM_CTL_EN2_CQ3_EN|pass2_cq_en) : 0);
            enable2 &= ~CAM_CTL_EN2_CDRZ_EN; // TEST_MDP
            //CURZ should be off in ISP+CDP direct link mode
            if ( tpipe ) {PIPE_DBG("HRZ not support tpipe"); enable1 &= ~CAM_CTL_EN1_HRZ_EN;}
            if ( tpipe ) {PIPE_DBG("CURZ not support tpipe"); enable2 &= ~CAM_CTL_EN2_CURZ_EN;}
            int_en = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;
            break;
        case eScenarioFmt_YUV:
            isApplyTurn = 0;
            isEn1CfaStatusFixed = 0; //test green
            isEn1HrzStatusFixed = 1;
#if 0    // for tpipe main issue
            isEn1C02StatusFixed = 0;
#endif
            isEn1C24StatusFixed = 0;
            subMode = ISP_SUB_MODE_YUV;
            if ( dma_en & CAM_CTL_DMA_EN_VIDO_EN ) {
                enable2 |= CAM_CTL_EN2_UV_CRSA_EN;
            }
            enable1 = CAM_CTL_EN1_C24_EN;
            enable2 |= CAM_CTL_EN2_UV_CRSA_EN; //mdp direct-link
            enable2 |= CAM_CTL_EN2_CDRZ_EN|CAM_CTL_EN2_C42_EN|CAM_CTL_EN2_PRZ_EN| \
                        ( pass2_cq_en ? (CAM_CTL_EN2_CQ2_EN|CAM_CTL_EN2_CQ3_EN|pass2_cq_en) : 0);
            enable2 &= ~CAM_CTL_EN2_CDRZ_EN; //TEST_MDP
            if ( tpipe ) {PIPE_DBG("HRZ not support tpipe"); enable1 &= ~CAM_CTL_EN1_HRZ_EN;}
            if ( tpipe ) {PIPE_DBG("CURZ not support tpipe"); enable2 &= ~CAM_CTL_EN2_CURZ_EN;}
            int_en = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;

            //IP_YUV C02
            //420 in -> should be on, mode=0;
            //422 in -> on or off, mode SHOULD BE 1. otherwise pass2 malfunction.


            break;
        case eScenarioFmt_MFB:
            isShareDmaCtlByTurn = 0;
            isEn1MfbStatusFixed = 0;
#if 0  // for tpipe main issue
            isEn1C02StatusFixed = 0;
#endif
            isEn1C24StatusFixed = 0;
            isEn1CfaStatusFixed = 1;
            isEn1HrzStatusFixed = 1;
            subMode = ISP_SUB_MODE_MFB;
            //dma_en =
            enable1 = CAM_CTL_EN1_MFB_EN;
            //enable2 =
            int_en = ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST;
            //
            pixel_byte_imgci = 2<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_lsci = 2<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_lcei = 2<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_imgo = 1<<CAM_ISP_PIXEL_BYTE_FP;

            //tile = 0;

            break;

/*
        case eScenarioFmt_RGB:
            subMode = ISP_SUB_MODE_RGB;
            break;
        case eScenarioFmt_JPG:
            subMode = ISP_SUB_MODE_JPG;
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
    PIPE_DBG("portInfo_imgi.eImgFmt:[%d]/m_isImgPlaneByImgi(%d)",portInfo_imgi.eImgFmt,m_isImgPlaneByImgi);
    switch( portInfo_imgi.eImgFmt ) {
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_BAYER8;
            break;
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit
            pixel_byte_imgi = (5<<CAM_ISP_PIXEL_BYTE_FP)>>2; // 1.25
            cam_in_fmt = CAM_FMT_SEL_BAYER10;
            //
#if 0
            if ( portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE] % 8 ) {
                //RAW10 stride should be multiple of 8
                PIPE_ERR(" RAW10 STRIDE SHOULD BE MULTIPLE OF 8");
            }
#endif
            break;
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit
            pixel_byte_imgi = (3<<CAM_ISP_PIXEL_BYTE_FP)>>1; // 1.5
            cam_in_fmt = CAM_FMT_SEL_BAYER12;
#if 0
            if ( portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE] % 6 ) {
                //RAW12 stride should be multiple of 6
                PIPE_ERR(" RAW12 STRIDE SHOULD BE MULTIPLE OF 6");
            }
#endif
            break;
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vipi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_YUV420_2P;
            enable1 |= CAM_CTL_EN1_C02_EN;
            //
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
            }
            break;
        case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
            pixel_byte_imgi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vipi = 1<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_vip2i = 1<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_YUV420_3P;
            enable1 |= CAM_CTL_EN1_C02_EN;
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
                idx_vip2i = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIP2I_EN;
            }
            break;
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            pixel_byte_imgi = 2<<CAM_ISP_PIXEL_BYTE_FP;
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
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
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIPI_EN;
                idx_vip2i = idx_imgi;
                dma_en |=  CAM_CTL_DMA_EN_VIP2I_EN;
            }
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
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
        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        default:
            PIPE_ERR("portInfo_imgi.eImgFmt:Format NOT Support");
            return MFALSE;
    }


    //
    PIPE_DBG("meScenarioID:[%d], dma(0x%x) isApplyTurn(%d)",meScenarioID,dma_en,isApplyTurn);

    scenario = meScenarioID;

    switch (meScenarioID) 
    {
        case eScenarioID_N3D_VR:     //  Native Stereo Camera VR
            isImg2oStatusFixed = 0;
            isIspOn = 1;
            break;
        case eScenarioID_IC:         //  Image Capture
            isIspOn = 1;
            break;
        case eScenarioID_N3D_IC:     //  Native Stereo Camera IC
            isIspOn = 1;
            SET_TCM_SETTING;
            break;
        case eScenarioID_VSS:       //  video snap shot
            
            #if 0   //cc-only
            enable1 &= (~CAM_CTL_EN1_PAK2_EN);  //TEST_MDP
            isImg2oStatusFixed = 0;
            #else            
            isImg2oStatusFixed = 1;
            #endif
            
            scenario = eScenarioID_N3D_IC;
            isIspOn = 1;

            /*
                    -VSS ISP flow is as follows, pass1 is dump before or after HRZ. PASS2 is from after HRZ.
                    -Please set the following flexible setting
                        -BIN_OUT_SEL_EN =1(reg_4078,[19], set reg_40c8, clr reg_40cc)
                        -BIN_OUT_SEL = 2 or 3(reg_4078,[3:2], set reg_40c8, clr reg_40cc)
                            -2:before HRZ
                            -3:after  HRZ
                        -PREGAIN_SEL=1 (reg_407c,[24], set reg_40d0,, clr reg_40d4)
                        -SGG_HRZ_SEL=1 (reg_407c,[28], set reg_40d0,, clr reg_40d4)
                        -EIS_RAW_SEL=1 (reg_4018,[16], set reg_40a0,, clr reg_40a4)
                        -PASS1_DONE_MUX_EN = 1 (reg_4078,[30], set reg_40c8, clr reg_40cc)
                        -PASS1_DONE_MUX = 1 (reg_4078,[12:8], set reg_40c8, clr reg_40cc)
                        -BIN_EN = 0 (reg_4004,[2], set reg_4080, clr reg_4084)
                    */
            eis_raw_sel = 1;

#if 1

            ctl_mux_sel = 0x00100008;    // 4074. AAO_SEL
//            ctl_mux_sel2 = 0x4008010C;   // 4078.   //after HRZ // 0x40080108;//before HRZ //

#if 0   //cc-mode only
            ctl_mux_sel2 = 0xC028410C;//0x40380148;   //TEST_MDP            // 4078.   //after HRZ // 0x40080108;//before HRZ //
            ctl_sram_mux_cfg = 0x110054F7;  //SL TEST_MDP  0x11000000;  // 407C.
#else
            PIPE_INF("pass2:cc-mode only");
            isEn1HrzStatusFixed = 1;
            ctl_mux_sel2 = 0xC0280308;    // 4078. add set PASS1_DONE_MUX to 3, BIN_OUT_SEL to 8
            ctl_sram_mux_cfg = 0x510054F7; //SL TEST_MDP  // 407C.
#endif
            
            //
            if ( eScenarioFmt_YUV == meScenarioFmt ) {
                //vss_yuv pass2
                ctl_mux_sel = 0x06100148;  //SL TEST_YUV_VSS ctl_mux_sel = 0x00400040;   // 4074.    // SGG_SEL_EN = 1, SGG_SEL = 1.
                #if 0 //SL remove PASS1_DONE_MUX[bit4] FLKO
                ctl_mux_sel2 = 0xC0385148;//0x40081100;  // 4078.    // PASS1_DONE_MUX_EN = 1, PASS1_DONE_MUX = 0x11.
                #else
                ctl_mux_sel2 = 0xC0084108;  // 4078.    // PASS1_DONE_MUX_EN = 1, PASS1_DONE_MUX = 0x11.
                #endif
                ctl_sram_mux_cfg = 0x110054F7; //SL TEST_VSS_YUV ctl_sram_mux_cfg = 0x00021000;  // 407C.    // SGG_HRZ_SEL = 0, PREGAIN_SEL = 0, ESFKO_SOF_SEL_EN = 1, ESFKO_SOF_SEL = 1.
            }

#else


            ctl_mux_sel = 0;
            ctl_mux_sel2 = 0x4008010C;//after HRZ // 0x40080108;//before HRZ //
            ctl_sram_mux_cfg = 0x11000000;
            //
//workaround for pass1 path but need to set isIspOn = 1;
//enable1 |= CAM_CTL_EN1_PAK_EN;
//dma_en  |= CAM_CTL_DMA_EN_IMGO_EN;

            //vss_yuv pass2
            if ( eScenarioFmt_YUV == meScenarioFmt ) {
                ctl_mux_sel2 = 0x40080100;
//enable1 &= (~CAM_CTL_EN1_PAK_EN);
            }
#endif
            /*
                    -EIS didnot support pass2+tpipe.
                    -EIS_raw/EIS_yuv is in pass1. source is from diff. module.SGG for EIS_raw, CDRZ for EIS_yuv
                    -VSS_YUV_pass1
                        -BIN_OUT_SEL_EN =1(reg_4078,[19], set reg_40c8, clr reg_40cc)
                        -BIN_OUT_SEL = 0    (reg_4078,[3:2], set reg_40c8, clr reg_40cc)
                        -pak_en = 0             (reg_4004,[12], set reg_4080,, clr reg_4084)
                    -VSS_YUV_pass2
                        -C02_SEL_EN = 1 (reg_4074,[25], set reg_40c0,, clr reg_40c4)
                        -C02_SEL      = 0 (reg_4074,[11:10], set reg_40c0,, clr reg_40c4)
                        -G2G_SEL_EN = 1 (reg_4074,[26], set reg_40c0,, clr reg_40c4)
                        -G2G_SEL      = 0 (reg_4074,[12], set reg_40c0,, clr reg_40c4)
                    -if need to get EIS_yuv after SGG, try hidden path
                        -EIS_RAW_SEL=1 (reg_4018,[16], set reg_40a0,, clr reg_40a4)
                        -SGG_SEL_EN = 1 (reg_4074,[22], set reg_40c0,, clr reg_40c4)
                        -SGG_SEL      = 1 (reg_4074,[7:6], set reg_40c0,, clr reg_40c4)
                        -SGG_HRZ_SEL=0 (reg_407c,[28], set reg_40d0,, clr reg_40d4)
                        -SGG_EN        =1 (reg_4004,[15], set reg_4080,, clr reg_4084)
                        -PASS1_DONE_MUX_EN = 1 (reg_4078,[30], set reg_40c8, clr reg_40cc)
                        -PASS1_DONE_MUX = 0x11 (reg_4078,[12:8], set reg_40c8, clr reg_40cc)
                    */

            SET_TCM_SETTING;
//SL TEST_RAW_YUV            tcm_en = 0x8c100003;//SL TEST_MDP         tcm_en = 0x0C100003; //SL TEST_MDP // from IspStreamCase0() 0x15004054 8C100003
            break;
        case eScenarioID_IP:
            isIspOn = 1;
            isImgoStatusFixed = 0; // need to control IMGO
            isAaoStatusFixed = 0; // need to control AAO
            isEsfkoStatusFixed = 0;  // need to control Esfko
            isFlkiStatusFixed = 0;  // need to control Flki
            isLcsoStatusFixed = 0;  // need to control Lcso
            isEn1AaaGropStatusFixed = 0; // need to control SGG_EN,AF_EN,FLK_EN,AA_EN,LCS_EN            
            tcm_en &= ~0x08000000 ;// TEST_MDP
            
            break;
        default:
            PIPE_ERR("NOT Support scenario");
            return MFALSE;
    }
    /*-----------------------------------------------------------------------------
      m_camPass2Param
      -----------------------------------------------------------------------------*/
    //top
    //scenario/sub_mode
    //m_camPass2Param.scenario = ISP_SCENARIO_VR;
    //m_camPass2Param.subMode =  ISP_SUB_MODE_RAW;
    m_camPass2Param.en_Top.enable1 =  enable1;
    enable2 |= CAM_CTL_EN2_UV_CRSA_EN;
    m_camPass2Param.en_Top.enable2 =  enable2;
    m_camPass2Param.en_Top.dma = dma_en;
    m_camPass2Param.isIspOn = isIspOn;
    //ctl_int
    m_camPass2Param.ctl_int.int_en = int_en;
    m_camPass2Param.ctl_int.intb_en = intb_en;
    m_camPass2Param.ctl_int.intc_en = intc_en;

    //fmt_sel
    m_camPass2Param.fmt_sel.reg_val = 0x00; //reset fmt_sel
    m_camPass2Param.fmt_sel.bit_field.scenario = scenario;
    m_camPass2Param.fmt_sel.bit_field.sub_mode = subMode;
    m_camPass2Param.fmt_sel.bit_field.cam_in_fmt = cam_in_fmt;
    //ctl_sel
    //WORKAROUND: to fix CQ0B/CQ0C fail issue
    int DB_en = 1;
    m_camPass2Param.ctl_sel.reg_val = 0;
    m_camPass2Param.ctl_sel.bit_field.tdr_sel = 1;//DB_en?0:1;
    m_camPass2Param.ctl_sel.bit_field.pass2_db_en = 0;//DB_en?1:0;
    m_camPass2Param.ctl_sel.bit_field.pass1_db_en = 1;//DB_en?1:0;
    m_camPass2Param.ctl_sel.bit_field.eis_raw_sel = eis_raw_sel;

    if ( m_camPass2Param.ctl_sel.bit_field.tdr_sel == m_camPass2Param.ctl_sel.bit_field.pass2_db_en ) {
        PIPE_ERR("Error:TDR_SEL/PASS2_DB_EN conflict ");
        return MFALSE;
    }
    //mux_sel
    m_camPass2Param.ctl_mux_sel.reg_val = ctl_mux_sel;
    //mux_sel2
    m_camPass2Param.ctl_mux_sel2.reg_val = ctl_mux_sel2;
    //
    m_camPass2Param.ctl_sram_mux_cfg.reg_val = ctl_sram_mux_cfg;
    //
    m_camPass2Param.CQ = pass2_CQ;
    m_camPass2Param.scenario = meScenarioID;
    m_camPass2Param.isConcurrency = this->m_pipePass;
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
    m_camPass2Param.updateTdri.updateType = updaType;
    m_camPass2Param.tcm_en = tcm_en;


    m_camPass2Param.bypass_ispCdpPipe = 0;
    //
    m_camPass2Param.tdri.memBuf.base_vAddr = (MUINT32)pTdriVir;
    m_camPass2Param.tdri.memBuf.base_pAddr = tdriPhy;
    m_camPass2Param.tdri.memBuf.size = 0; //0-> means tpipemain data is filled by pass2, otherwise by pipe driver.

    PIPE_INF("Pass(%d),sceFmt(%d),imgi.fmt(%d),isByImgi(%d),sceID(%d),dma(0x%x) isTurn(%d),VA=0x%x PA=0x%x\n",
        this->m_pipePass, \
        meScenarioFmt, \
        portInfo_imgi.eImgFmt, \
        m_isImgPlaneByImgi, \
        meScenarioID, \
        dma_en,\
        isApplyTurn, \
        m_camPass2Param.tdri.memBuf.base_vAddr, \
        m_camPass2Param.tdri.memBuf.base_pAddr);
    //
    m_camPass2Param.cqi.memBuf.base_vAddr = (unsigned long)cq_vir;
    m_camPass2Param.cqi.memBuf.base_pAddr = cq_phy;
    m_camPass2Param.cqi.memBuf.size = cq_size;
    //
    m_camPass2Param.ringTdriCfg.ringBufferMcuRowNo = 0;
    m_camPass2Param.ringTdriCfg.ringBufferMcuHeight = 0;
    //
    if ( -1 != idx_imgi ) {
        PIPE_DBG("config imgi");
        this->configDmaPort(&portInfo_imgi,m_camPass2Param.imgi,(MUINT32)pixel_byte_imgi,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.imgi.ring_en = 0;
        m_camPass2Param.imgi.ring_size= 0;
        //
        m_camPass2Param.imgi.memBuf_c_ofst = 0;
        m_camPass2Param.imgi.memBuf_v_ofst = 0;
        //
        m_camPass2Param.src_img_size.h = vInPorts[idx_imgi]->u4ImgHeight;
        //
        // for digital zoom crop
        m_camPass2Param.imgi.crop.x = portInfo_imgi.crop.x;
        m_camPass2Param.imgi.crop.floatX = portInfo_imgi.crop.floatX;
        m_camPass2Param.imgi.crop.y = portInfo_imgi.crop.y;
        m_camPass2Param.imgi.crop.floatY = portInfo_imgi.crop.floatY;
        m_camPass2Param.imgi.crop.w = portInfo_imgi.crop.w;
        m_camPass2Param.imgi.crop.h = portInfo_imgi.crop.h;
        m_camPass2Param.imgi.lIspColorfmt = portInfo_imgi.eImgFmt;  //SL TEST_MDP_YUV
        //
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
                    m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.size.w * m_camPass2Param.imgi.size.h;
                    break;
                case eImgFmt_YV12:      //= 0x00008,    //420 format, 3 plane (YVU)
                case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.size.w * m_camPass2Param.imgi.size.h;;
                    //Y+U plane size (Y + 1/4Y)
                    m_camPass2Param.imgi.memBuf_v_ofst = (m_camPass2Param.imgi.memBuf_c_ofst*5)>>2;

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
                case eImgFmt_I422:         //422 format, 3 plane

                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.size.w * m_camPass2Param.imgi.size.h;;
                    //Y+U plane size (Y + 1/2Y)
                    m_camPass2Param.imgi.memBuf_v_ofst = (m_camPass2Param.imgi.memBuf_c_ofst*3)>>1;
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
        m_camPass2Param.vipi.ring_en = 0;
        m_camPass2Param.vipi.ring_size= 0;
        //
        //m_camPass2Param.vipi.size.stride = portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE];
    }
    //
    if (-1 != idx_vip2i ) {
        PIPE_DBG("config vip2i");
        this->configDmaPort(&portInfo_vip2i,m_camPass2Param.vip2i,(MUINT32)pixel_byte_vip2i,(MUINT32)0,(MUINT32)1,ESTRIDE_3RD_PLANE);
        m_camPass2Param.vip2i.ring_en = 0;
        m_camPass2Param.vip2i.ring_size= 0;
        //
        //m_camPass2Param.vip2i.size.stride = portInfo_vip2i.u4Stride[ESTRIDE_3RD_PLANE];
    }
    //for IP_MFB
    pixel_byte_imgci = pixel_byte_imgi;
    pixel_byte_lsci = pixel_byte_imgi;
    pixel_byte_lcei = pixel_byte_imgi;
    m_camPass2Param.mfb.bld_mode = 2;
    if (-1 != idx_imgci ) {
        PIPE_DBG("config imgci");
        this->configDmaPort(&portInfo_imgci,m_camPass2Param.imgci,(MUINT32)pixel_byte_imgci,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.imgci.ring_en = 0;
        m_camPass2Param.imgci.ring_size= 0;
        //
        m_camPass2Param.imgci.size.stride = portInfo_imgci.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_lsci ) {
        PIPE_DBG("config lsci");
        this->configDmaPort(&portInfo_lsci,m_camPass2Param.lsci,(MUINT32)pixel_byte_lsci,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.lsci.ring_en = 0;
        m_camPass2Param.lsci.ring_size= 0;
        //
        m_camPass2Param.lsci.size.stride = portInfo_lsci.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_lcei ) {
        PIPE_DBG("config lcei");
        this->configDmaPort(&portInfo_lcei,m_camPass2Param.lcei,(MUINT32)pixel_byte_lcei,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.lcei.ring_en = 0;
        m_camPass2Param.lcei.ring_size= 0;
        //
        m_camPass2Param.lcei.size.stride = portInfo_lcei.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_imgo ) {
        PIPE_DBG("config imgo");
        this->configDmaPort(&portInfo_imgo,m_camPass2Param.imgo_dma,(MUINT32)pixel_byte_imgo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        m_camPass2Param.imgo_dma.size.stride = portInfo_imgo.u4Stride[ESTRIDE_1ST_PLANE];
        m_camPass2Param.imgo.imgo_stride = portInfo_imgo.u4Stride[ESTRIDE_1ST_PLANE];
        //tile will update imgo crop ofst register if en.
        m_camPass2Param.imgo.imgo_crop_en = 1;
    }
    //

    if ( -1 != idx_dispo) {
        PIPE_DBG("config dispo");
        //
        this->configCdpOutPort(vOutPorts[idx_dispo],m_camPass2Param.dispo);
        //dispo not support rotation/flip
//SL TEST_MDP        m_camPass2Param.dispo.Rotation = CDP_DRV_ROTATION_0;
//SL TEST_MDP        m_camPass2Param.dispo.Flip = MFALSE;
        //
        //m_camPass2Param.dispo.memBuf.size = vOutPorts[idx_dispo]->u4BufSize;
        m_camPass2Param.dispo.memBuf.base_vAddr = vOutPorts[idx_dispo]->u4BufVA;
        //m_camPass2Param.dispo.memBuf.base_pAddr = vOutPorts[idx_dispo]->u4BufPA;
    }
    if ( -1 != idx_vido) {
        PIPE_DBG("config vido");
        //
        this->configCdpOutPort(vOutPorts[idx_vido],m_camPass2Param.vido);
        //
        //m_camPass2Param.vido.memBuf.size = vOutPorts[idx_vido]->u4BufSize;
        m_camPass2Param.vido.memBuf.base_vAddr = vOutPorts[idx_vido]->u4BufVA;//SL add back
        //m_camPass2Param.vido.memBuf.base_pAddr = vOutPorts[idx_vido]->u4BufPA;
    }


    PIPE_DBG("settingStage(%d)\n",settingStage);
    
    //config setting stage
    m_camPass2Param.capTdriCfg.isRunSegment = portInfo_imgi.u4IsRunSegment;
    if(settingStage == eConfigSettingStage_Init) 
    {
        //m_camPass2Param.bypass_ispRawPipe = 0;
        //m_camPass2Param.bypass_ispRgbPipe = 0;
        //m_camPass2Param.bypass_ispYuvPipe = 0;

        if(m_camPass2Param.capTdriCfg.isRunSegment) // for VSS capture
        {  
            m_camPass2Param.capTdriCfg.isCalculateTpipe = 1;
            m_camPass2Param.capTdriCfg.isRunSegment = portInfo_imgi.u4IsRunSegment;
            m_camPass2Param.capTdriCfg.setSimpleConfIdxNumVa = portInfo_imgi.u4SegNumVa;
            m_camPass2Param.capTdriCfg.segSimpleConfBufVa = (MUINT32)pTpipeConfigVa;
           m_camPass2Param.capTdriCfg.isCalculateTpipe = 1;
        }
    } 
    else if (settingStage == eConfigSettingStage_UpdateTrigger) 
    {
        //m_camPass2Param.bypass_ispRawPipe = 1;
        //m_camPass2Param.bypass_ispRgbPipe = 1;
        //m_camPass2Param.bypass_ispYuvPipe = 1;

        PIPE_DBG("u4SegTpipeSimpleConfigIdx(%d)\n",portInfo_imgi.u4SegTpipeSimpleConfigIdx);

        if(m_camPass2Param.capTdriCfg.isRunSegment) 
        {
            m_camPass2Param.capTdriCfg.isCalculateTpipe = 0;

            if(portInfo_imgi.u4SegTpipeSimpleConfigIdx < segmSimpleConfIdxNum) 
            {
                m_camPass2Param.tdri.memBuf.base_pAddr = tdriPhy + pTpipeConfigVa[portInfo_imgi.u4SegTpipeSimpleConfigIdx];
            } 
            else 
            {
                PIPE_ERR("u4SegTpipeSimpleConfigIdx(%d) over max value(%d) error!",portInfo_imgi.u4SegTpipeSimpleConfigIdx,segmSimpleConfIdxNum);
                return MFALSE;
            }

            #if 0
            int i;
            for(i=0; i<segmSimpleConfIdxNum;i++){
                LOG_VRB("i(%d) offset(%d)\n",i,pTpipeConfigVa[i]);
            }
            #endif
        }
    } 
    else 
    {
        PIPE_ERR("settingStage(%d) error!",settingStage);
        return MFALSE;
    }  
   
    ret = m_CamPathPass2.config( &m_camPass2Param );
    if( ret != 0 )
    {
        PIPE_ERR("Pass 2 config error!");
        return MFALSE;
    } 
    else
    {
        if(m_camPass2Param.capTdriCfg.isRunSegment && settingStage == eConfigSettingStage_Init) 
        {
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
PostProcPipe::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    PIPE_DBG("NOT SUPPORT for postproc pipe ");
    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
PostProcPipe::
onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2)
{
    PIPE_DBG("tid(%d) (u4Param1, u4Param2)=(%d, %d)", gettid(), u4Param1, u4Param2);
    return  MTRUE;
}


/*******************************************************************************
* Command
********************************************************************************/
MBOOL
PostProcPipe::
onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam)
{
    PIPE_DBG("tid(%d) (u4InParam)=(%d)",gettid(), u4InParam);
    *pu4OutParam = 0x12345678;
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
irq(EPipePass pass, EPipeIRQ irq_int)
{
int    ret = 0;
MINT32 type = 0;
MUINT32 irq = 0;

    PIPE_DBG("tid(%d) (type,irq)=(0x%08x,0x%08x)", gettid(), pass, irq_int);

    //irq_int
    if ( EPIPEIRQ_PATH_DONE != irq_int ) {
        PIPE_ERR("IRQ:NOT SUPPORT irq for PASS2");
        return MFALSE;
    }
    //pass
    if ( EPipePass_PASS2 == pass ) {
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
PostProcPipe::
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
        case EPIPECmd_SET_CURRENT_BUFFER:
            if ( EPortIndex_IMGI == arg1 ) 
            {
                dmaChannel = ISP_DMA_IMGI;
            }
            if ( EPortIndex_VIPI == arg1 ) 
            {
                dmaChannel = ISP_DMA_VIPI;
            }
            if ( EPortIndex_VIP2I == arg1 ) 
            {
                dmaChannel = ISP_DMA_VIP2I;
            }
            if ( EPortIndex_DISPO == arg1 )
            {
                dmaChannel = ISP_DMA_DISPO;
            }
            if ( EPortIndex_VIDO == arg1 ) 
            {
                dmaChannel = ISP_DMA_VIDO;
            }
            
            //for MFB
            if ( EPortIndex_IMGCI == arg1 )
            {
                dmaChannel = ISP_DMA_IMGCI;
            }
            if ( EPortIndex_LSCI == arg1 ) 
            {
                dmaChannel = ISP_DMA_LSCI;
            }
            if ( EPortIndex_LCEI == arg1 ) 
            {
                dmaChannel = ISP_DMA_LCEI;
            }
            if ( EPortIndex_IMGO == arg1 ) 
            {
                dmaChannel = ISP_DMA_IMGO;
            }
            
            m_CamPathPass2.setDMACurrBuf((MUINT32) dmaChannel);
            break;
        case EPIPECmd_SET_NEXT_BUFFER:
            if ( EPortIndex_IMGI == arg1 ) 
            {
                dmaChannel = ISP_DMA_IMGI;
            }
            if ( EPortIndex_VIPI == arg1 ) 
            {
                dmaChannel = ISP_DMA_VIPI;
            }
            if ( EPortIndex_VIP2I == arg1 ) 
            {
                dmaChannel = ISP_DMA_VIP2I;
            }
            if ( EPortIndex_DISPO == arg1 ) 
            {
                dmaChannel = ISP_DMA_DISPO;
            }
            if ( EPortIndex_VIDO == arg1 ) 
            {
                dmaChannel = ISP_DMA_VIDO;
            }
            
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
        case EPIPECmd_FREE_MAPPED_BUFFER:
            {
                stISP_BUF_INFO buf_info = (stISP_BUF_INFO)(*(stISP_BUF_INFO*)arg2);
                m_CamPathPass2.freePhyBuf(arg1,buf_info);
            }
            break;
        case EPIPECmd_SET_IMG_PLANE_BY_IMGI:
            m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
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

