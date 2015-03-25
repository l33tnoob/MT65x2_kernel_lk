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
#define LOG_TAG "iio/camio"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//

#include "PipeImp.h"
#include "CamIOPipe.h"
//
#include <cutils/properties.h>  // For property_get().

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""

//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);


/*******************************************************************************
*
********************************************************************************/
CamIOPipe::
CamIOPipe(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EScenarioID const eScenarioID,
    EScenarioFmt const eScenarioFmt
)
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt),
      m_pIspDrvShell(NULL),
      m_pass1_CQ(CAM_ISP_CQ_NONE),
      m_RawType(0)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    PIPE_INF(":E");
    //
    memset(&this->m_camPass1Param,0x00,sizeof(CamPathPass1Parameter));
    this->m_vBufImgo.resize(1);
    this->m_vBufImg2o.resize(1);

    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance();
    PIPE_INF(":X");
}

CamIOPipe::
~CamIOPipe()
{
    PIPE_INF(":E");
    //
    m_pIspDrvShell->destroyInstance();
    PIPE_INF(":X");
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::init()
{
    MBOOL result = MTRUE;
    MUINT32 reg_val;

    PIPE_DBG(":E");

    if( m_pIspDrvShell ) 
    {
        m_pIspDrvShell->init();        
        m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountInc();
        m_CamPathPass1.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
#if 1
        //clear TOP enable register at every time pass1 init.
        //"NO OLD DATA" should keep once every time pass1 init exception FMT_EN
        PIPE_INF("reset TOP registers(0x%x)",m_pIspDrvShell->getPhyIspReg());
        //
        if(m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
            reg_val = ISP_IOCTL_READ_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_EN2);
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_DMA_EN, 0);
            //disable GGM
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_GGM_CTRL, 0);

            //reset GDMA relative setting
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_SEL, 0);
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN1, 0x40000000);
            reg_val = ISP_READ_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2);
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_DMA_EN, 0);
            //disable GGM
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_GGM_CTRL, 0);
            //reset GDMA relative setting
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_SEL, 0);
        }
        //
        PIPE_INF("EN2(0x%x)",reg_val);
#endif
        //

    }

    //
    m_CamPathPass1.ispBufCtrl.path = ISP_PASS1;
	////////////////////////////////////////////////////////////////////////////////
    ///--adding clear buf information at the beginnning to avoid some problems
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
	MUINT32 dummy;
	int rt_dma;
	//imgo
	rt_dma=_imgo_;
    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
    buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
    buf_ctrl.data_ptr = (MUINT32)&dummy;
    if ( MTRUE != m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) )
	{
       PIPE_ERR("ERROR:rtBufCtrl_clear imgo fail");
       return -1;
    }
	//img2o
	rt_dma=_img2o_;
    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
    buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
    buf_ctrl.data_ptr = (MUINT32)&dummy;
    if ( MTRUE != m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) )
	{
       PIPE_ERR("ERROR:rtBufCtrl_clear img2o fail");
       return -1;
    }
    ///////////////////////////////////////////////////////////////////////////////////

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
uninit()
{
MINT32 ret = 0;
    PIPE_DBG(":E");

    m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountDec();
    //
    m_pIspDrvShell->uninit();

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::start()
{        
    int path  = CAM_ISP_PASS1_START;

    PIPE_DBG(":E");
    
    path  = CAM_ISP_PASS1_START;
    m_CamPathPass1.start((void*)&path);
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::startCQ0()
{
    int path  = CAM_ISP_PASS1_START;

    PIPE_DBG(":E");
    //
    path  = CAM_ISP_PASS1_CQ0_START;
    m_CamPathPass1.start((void*)&path);

/*
    if ( CQ_CONTINUOUS_EVENT_TRIGGER != m_CQ0TrigMode ) {
        m_camPass1Param.CQ = CAM_ISP_CQ_NONE;
        m_CamPathPass1.CQ = CAM_ISP_CQ_NONE;
    }
*/

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/

MBOOL
CamIOPipe::
startCQ0B()
{
int path  = CAM_ISP_PASS1_START;

    PIPE_DBG(":E");
    //
    path  = CAM_ISP_PASS1_CQ0B_START;
    m_CamPathPass1.start((void*)&path);

/*
    if ( CQ_CONTINUOUS_EVENT_TRIGGER != m_CQ0BTrigMode ) {
        m_camPass1Param.CQ = CAM_ISP_CQ_NONE;
        m_CamPathPass1.CQ = CAM_ISP_CQ_NONE;
    }
*/

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
stop()
{
int path  = CAM_ISP_PASS1_START;

    PIPE_DBG(":E");
    //
    m_CamPathPass1.stop((void*)&path);
    //
//    m_pIspDrvShell->getPhyIspDrv()->reset();
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
    //
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    PIPE_DBG("+ tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;
    stISP_BUF_INFO exbufInfo;
    PIPE_DBG(":E:tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    if (EPortIndex_IMGO == portID.index) {
        dmaChannel = ISP_DMA_IMGO;
    }
    else if (EPortIndex_IMG2O == portID.index) {
        dmaChannel = ISP_DMA_IMG2O;
    }
    //enque buffer
    bufInfo.memID       = rQBufInfo.vBufInfo[0].memID;
    bufInfo.size        = rQBufInfo.vBufInfo[0].u4BufSize;
    bufInfo.base_vAddr  = rQBufInfo.vBufInfo[0].u4BufVA;
    bufInfo.base_pAddr  = rQBufInfo.vBufInfo[0].u4BufPA;
    bufInfo.bufSecu     = rQBufInfo.vBufInfo[0].bufSecu;
    bufInfo.bufCohe     = rQBufInfo.vBufInfo[0].bufCohe;
    bufInfo.next = (stISP_BUF_INFO*)NULL;
    //
    PIPE_DBG("bufInfo,(%d),(0x%08x),(0x%08x),(0x%08x),(%d/%d)",
            bufInfo.memID,
            bufInfo.size,
            bufInfo.base_vAddr,
            bufInfo.base_pAddr,
            bufInfo.bufSecu,
            bufInfo.bufCohe);

    //buffer wanna exchange. to replace original buffer.
    if ( 2 <= rQBufInfo.vBufInfo.size() ) {
        PIPE_WRN("exchange 1st buf. by 2nd buf. and enque it.:size(%d)",rQBufInfo.vBufInfo.size());
        exbufInfo.memID       = rQBufInfo.vBufInfo[1].memID;
        exbufInfo.size        = rQBufInfo.vBufInfo[1].u4BufSize;
        exbufInfo.base_vAddr  = rQBufInfo.vBufInfo[1].u4BufVA;
        exbufInfo.base_pAddr  = rQBufInfo.vBufInfo[1].u4BufPA;
        exbufInfo.bufSecu     = rQBufInfo.vBufInfo[1].bufSecu;
        exbufInfo.bufCohe     = rQBufInfo.vBufInfo[1].bufCohe;
        exbufInfo.next = (stISP_BUF_INFO*)NULL;
        //set for original buffer info.
        bufInfo.next = (stISP_BUF_INFO*)&exbufInfo;
        //
        PIPE_DBG("exbufInfo,(%d),(0x%08x),(0x%08x),(0x%08x),(%d/%d)",
                exbufInfo.memID,
                exbufInfo.size,
                exbufInfo.base_vAddr,
                exbufInfo.base_pAddr,
                exbufInfo.bufSecu,
                exbufInfo.bufCohe);
    }
    //
    if ( 0 != this->m_CamPathPass1.enqueueBuf( dmaChannel, bufInfo ) ) {
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
CamIOPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;

    PIPE_DBG("E ");
    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);

    if (EPortIndex_IMGO == portID.index) {
        dmaChannel = ISP_DMA_IMGO;
    }
    else if (EPortIndex_IMG2O == portID.index) {
        dmaChannel = ISP_DMA_IMG2O;
    }
    //
    bufInfo.pBufList = &bufList;
    if ( 0 != this->m_CamPathPass1.dequeueBuf( dmaChannel,bufInfo) ) {
        PIPE_ERR("ERROR:dequeueBuf");
        return MFALSE;
    }
    //
    PIPE_DBG("dma:[0x%x]/size:[0x%x]/addr:[0x%x] ",dmaChannel,
                                                bufList.size(),
                                                bufList.front().base_vAddr);

    //
    rQBufInfo.vBufInfo.resize(bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID             = bufList.front().memID;
        rQBufInfo.vBufInfo[i].u4BufSize         = bufList.front().size;
        rQBufInfo.vBufInfo[i].u4BufVA           = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufPA           = bufList.front().base_pAddr;
        rQBufInfo.vBufInfo[i].i4TimeStamp_sec   = bufList.front().timeStampS;
        rQBufInfo.vBufInfo[i].i4TimeStamp_us    = bufList.front().timeStampUs;
        //
        bufList.pop_front();
        //
        PIPE_DBG("dma:[0x%x]/size:[0x%x]/addr:[0x%x]/buf size:[%d]/memid:[%d]",   dmaChannel,
                                                                                rQBufInfo.vBufInfo[i].memID,
                                                                                rQBufInfo.vBufInfo.size(),
                                                                                rQBufInfo.vBufInfo[i].u4BufVA,
                                                                                rQBufInfo.vBufInfo[i].u4BufSize);
    }

    //
    PIPE_DBG("X ");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    int ret = 0;
    MBOOL result = MTRUE;
    int idx_imgo = -1;
    int idx_img2o = -1;
    int idx_tgi = -1;
    int idx_tg1i = -1;
    int idx_tg2i = -1;

    /***  ***/
    //  ctor input meScenarioID
    int scenario = ISP_SCENARIO_MAX;
    // dependent on scenario
    //int sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    // dependent on meScenarioFmt which is ctor input
    int subMode = ISP_SUB_MODE_MAX;
    // pass1 out is fixed format.
    int cam_out_fmt = CAM_FMT_SEL_YUV422_1P;
    // dependent on scenario
    int continuous = 0;
    // config setting stage
    EConfigSettingStage settingStage = m_settingStage;
    // fixed CQ resource
    int pass1_CQ = m_pass1_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//
    int pass1_cq_en = 0;// = (CAM_ISP_CQ0 == pass1_CQ)?CAM_CTL_EN2_CQ0_EN:((CAM_ISP_CQ0B == pass1_CQ)?CAM_CTL_EN2_CQ0B_EN:);//0; //
    if ( CAM_ISP_CQ_NONE != pass1_CQ) {
        if ( CAM_ISP_CQ0  == pass1_CQ ) { pass1_cq_en = CAM_CTL_EN2_CQ0_EN;}
        if ( CAM_ISP_CQ0B == pass1_CQ ) { pass1_cq_en = CAM_CTL_EN2_CQ0B_EN;}
        if ( CAM_ISP_CQ0C == pass1_CQ ) { pass1_cq_en = CAM_CTL_EN2_CQ0C_EN;}
    }
    // default MUST ON module
    int enable1 = 0;
    int enable2 = 0;
    int dma_en = 0;
    int dma_int = CAM_CTL_DMA_INT_SOF2_INT_EN;
    int int_en = ISP_DRV_IRQ_INT_STATUS_TG1_ERR_ST| \
                 ISP_DRV_IRQ_INT_STATUS_TG2_ERR_ST| \
                 ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST | \
                 ISP_DRV_IRQ_INT_STATUS_VS1_ST | \
                 ISP_DRV_IRQ_INT_STATUS_VS2_ST | \
                 ISP_DRV_IRQ_INT_STATUS_PASS1_TG2_DON_ST | \
                 ISP_DRV_IRQ_INT_STATUS_IMGO_ERR_ST | \
                 ISP_DRV_IRQ_INT_STATUS_IMG2O_ERR_ST | \
                 ISP_DRV_IRQ_INT_STATUS_ESFKO_ERR_ST | \
                 ISP_DRV_IRQ_INT_STATUS_AAO_ERR_ST;
                //ISP_DRV_IRQ_INT_STATUS_DMA_ERR_ST;
    /*
        *   top_pass1 relative registers
        *   DONOT set/clr if isIspOn == 0
        */
    int isIspOn = 0;
    //
    int eis_raw_sel = 0;
    //
    int ctl_mux_sel = 0;
    int ctl_mux_sel2 = 0;
    int ctl_sram_mux_cfg = 0;
    //
    int isEn1AaaGropStatusFixed = 1;  /* 1:SGG_EN,AF_EN,FLK_EN,AA_EN,LCS_EN won't be clear */
    int isEn1C24StatusFixed = 1; /* 1:c24(en1) won't be clear */
    int isEn1C02StatusFixed = 1; /* 1:c02(en1) won't be clear */
    int isEn1CfaStatusFixed = 1; /* 1:cfa(en1) won't be clear */
    int isEn1HrzStatusFixed = 1; /* 1:hrz(en1) won't be clear */
    int isEn1MfbStatusFixed = 1; /* 1:mfb(en1) won't be clear */
    int isEn2CdrzStatusFixed = 1; /* 1:cdrz(en2) won't be clear */
    int isEn2G2cStatusFixed = 1; /* 1:G2c(en2) won't be clear */
    int isEn2C42StatusFixed = 1; /* 1:c42(en2) won't be clear */
    int isImg2oStatusFixed = 0; /* 0:img2o can be clear */
    int isAaoStatusFixed = 1;  /* 1:aao won't be clear */
    int isEsfkoStatusFixed = 1;  /* 1:Esfko won't be clear */
    int isFlkiStatusFixed = 1;  /* 1:Flki won't be clear */
    int isLcsoStatusFixed = 1;  /* 1:Lcso won't be clear */
    int isShareDmaCtlByTurn =1; /* 1:share DMA(imgci,lsci and lcei) ctl by turning */


    //TODO:filled by sensor info
    int pix_id = -1;

    //TODO: removed and modified by TG?
    //int tg1_fmt = CAM_FMT_SEL_TG_FMT_RAW8;
    //int tg1_sw = CAM_FMT_SEL_TG_SW_RGB;
    int tg_sel = 0;
    int curz_borrow = 0;
    //
    int cdrz_in_size_w = 0;
    //
    int pixel_byte_imgo = 1;
    int pixel_byte_img2o = 1;
    //
    PortInfo portInfo_tg1i;
    PortInfo portInfo_tg2i;
    PortInfo portInfo_imgo;
    PortInfo portInfo_img2o;

    //====== Reset in NCC Mode ====== >>

#if 1     
    //"NO OLD DATA" should keep once every time pass1 in NCC mode
    if(meScenarioID == eScenarioID_ZSD)
    {
        PIPE_DBG("reset TOP for NCC(0x%x)",m_pIspDrvShell->getPhyIspReg());
        PIPE_DBG("sceID(%d)",meScenarioID);
        
        if(m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_EN2, 0);
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_CTL_DMA_EN, 0);

            //disable GGM
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspDrv(),m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),CAM_GGM_CTRL, 0);
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN1, 0x40000000);
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_EN2, 0);
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_CTL_DMA_EN, 0);

            //disable GGM
            ISP_WRITE_ENABLE_REG(m_pIspDrvShell->getPhyIspReg(),CAM_GGM_CTRL, 0);
        }
    }     
    
#endif

    //====== Reset in NCC Mode ======= <<

    PIPE_DBG("in[%d]/out[%d]", vInPorts.size(), vOutPorts.size());
    vector<PortInfo const*>::const_iterator iter;
/*
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

    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) 
    {
        if(0 == vInPorts[i])
        { 
            continue;
        }
        
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
        if( EPortIndex_TG1I == vInPorts[i]->index ) 
        {
            idx_tgi = i;
            idx_tg1i = i;
            enable1 |= CAM_CTL_EN1_TG1_EN;
            portInfo_tg1i = (PortInfo)*vInPorts[idx_tg1i];
        }
        else if( EPortIndex_TG2I == vInPorts[i]->index ) 
        {
            idx_tgi = i;
            idx_tg2i = i;
            enable1 |= CAM_CTL_EN1_TG2_EN;
            portInfo_tg2i = (PortInfo)*vInPorts[idx_tg2i];
        }
        
        if( -1 != idx_tg1i && -1 != idx_tg2i ) 
        {
            if( eScenarioID_N3D_IC != meScenarioID && eScenarioID_N3D_VR != meScenarioID ) 
            {
                PIPE_ERR("NOT SUPPORT TG1/TG2 simultaneously for scenario(%d)",meScenarioID);
                return MFALSE;
            }
        }
        else if( -1 != idx_tg2i ) 
        {
            //TG2 ONLY
            tg_sel = 1;
        }
    }
    //
    for(MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) 
    {     
        if( 0 == vOutPorts[i] )
        { 
            continue; 
        }
        
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
        
        if ( EPortIndex_IMGO == vOutPorts[i]->index ) 
        {
            idx_imgo = i;
            dma_en |= CAM_CTL_DMA_EN_IMGO_EN;
            portInfo_imgo =  (PortInfo)*vOutPorts[idx_imgo];
        }
        else if ( EPortIndex_IMG2O == vOutPorts[i]->index ) 
        {
            idx_img2o = i;
            dma_en |= CAM_CTL_DMA_EN_IMG2O_EN;
            portInfo_img2o =  (PortInfo)*vOutPorts[idx_img2o];
        }
    }
    //
    pix_id = vInPorts[idx_tgi]->eRawPxlID;
    //for ISP pass1 case
    cdrz_in_size_w = vInPorts[idx_tgi]->u4ImgWidth;
    //
    PIPE_INF("meScenarioFmt:[%d]",meScenarioFmt);
    switch (meScenarioFmt) {
        case eScenarioFmt_RAW:
            subMode = ISP_SUB_MODE_RAW;
            //eImgFmt_BAYER8 == vInPorts[idx_imgi]->eImgFmt
            //cam_out_fmt = CAM_FMT_SEL_BAYER8;
            //tg1_fmt = CAM_FMT_SEL_TG_FMT_RAW8;
            //tg1_sw = CAM_FMT_SEL_TG_SW_RGB;
            //enable table
            #if 1 //org
            enable1 |= CAM_CTL_EN1_CAM_EN;
            #else
            enable1 |= CAM_CTL_EN1_CAM_EN|CAM_CTL_EN1_GGM_EN|CAM_CTL_EN1_CFA_EN|CAM_CTL_EN1_UNP_EN; //TEST_MDP
            #endif
            #if 1 //org
            enable2 |= pass1_cq_en;
            #else
            enable2 |= CAM_CTL_EN2_C42_EN|CAM_CTL_EN2_G2C_EN|pass1_cq_en;
            #endif
            //js_test
            //dma_en = CAM_CTL_DMA_EN_IMGO_EN | ( ( eScenarioID_ZSD == meScenarioID )?CAM_CTL_DMA_EN_IMG2O_EN:0 );

            int_en |= ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
            break;
        case eScenarioFmt_YUV:
            subMode = ISP_SUB_MODE_YUV;
            //cam_out_fmt = CAM_FMT_SEL_YUV422_1P;
            //tg1_fmt = CAM_FMT_SEL_TG_FMT_YUV422;
            //js_test parameter decided by sensor driver??
            //tg1_sw = CAM_FMT_SEL_TG_SW_UYVY;
            //enable table

            enable1 |= CAM_CTL_EN1_C02_EN;  // for tpipe main issue
//            enable1 |= CAM_CTL_EN1_C24_EN|CAM_CTL_EN1_C02_EN;//remove YUV sensor abnormal image|CAM_CTL_EN1_GGM_EN;
            enable2 |= pass1_cq_en; //89
//            enable2 |= CAM_CTL_EN2_C42_EN|pass1_cq_en;    

            //js_test
            //dma_en = CAM_CTL_DMA_EN_IMGO_EN | ( ( eScenarioID_ZSD == meScenarioID )?CAM_CTL_DMA_EN_IMG2O_EN:0 );

            int_en |= ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
            break;
/*
        case eScenarioFmt_RGB:
            subMode = ISP_SUB_MODE_RGB;
            cam_out_fmt = CAM_FMT_SEL_RGB565;
            tg1_fmt = CAM_FMT_SEL_TG_FMT_RGB565;
            tg1_sw = CAM_FMT_SEL_TG_SW_RGB;//1; 0,1 is same thing;
            break;
        case eScenarioFmt_JPG:
            subMode = ISP_SUB_MODE_JPG;
            //cam_out_fmt = ;
            //tg1_fmt = CAM_FMT_SEL_TG_FMT_RAW8;
            //tg1_sw = CAM_FMT_SEL_TG_SW_RGB;
            break;
        case eScenarioFmt_MFB:
            subMode = ISP_SUB_MODE_MFB;
            //cam_out_fmt = ;
            //tg1_fmt = CAM_FMT_SEL_TG_FMT_RAW8;
            //tg1_sw = CAM_FMT_SEL_TG_SW_RGB;
            break;
        case eScenarioFmt_RGB_LOAD:
            subMode = ISP_SUB_MODE_RGB_LOAD;
            cam_out_fmt = CAM_FMT_SEL_RGB888;
            //tg1_fmt = CAM_FMT_SEL_TG_FMT_RAW8;
            //tg1_sw = CAM_FMT_SEL_TG_SW_RGB;
            break;
*/
        default:
            PIPE_ERR("NOT Support submode");
            return MFALSE;
    }
    //
    PIPE_INF("meScenarioID:[%d]",meScenarioID);
    scenario = meScenarioID;
    switch (meScenarioID) {
        case eScenarioID_CONFIG_FMT: //  Config FMT
        case eScenarioID_VR:         //  Video Recording/Preview
            isImg2oStatusFixed = 1;
        case eScenarioID_VEC:        //  Vector Generation
            //sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            continuous = 1;
            isIspOn = 1;
            break;
        case eScenarioID_ZSD:   //  Zero Shutter Delay
#if 1  //kk test default:0
            if(meScenarioFmt == eScenarioFmt_RAW) 
            {
                isEn1CfaStatusFixed = 1; //Test Green Screen 0;
                isEn1HrzStatusFixed = 0;
                isEn2CdrzStatusFixed = 0;
                isEn2G2cStatusFixed = 0;
                isEn2C42StatusFixed = 0;
                enable1 |= CAM_CTL_EN1_CFA_EN;
                enable2 |= CAM_CTL_EN2_CDRZ_EN|CAM_CTL_EN2_C42_EN|CAM_CTL_EN2_G2C_EN;
            } 
            else if (meScenarioFmt == eScenarioFmt_YUV) 
            {
                isEn1C24StatusFixed = 0;
                isEn1C02StatusFixed = 0;
                isEn2CdrzStatusFixed = 0;
                isEn2C42StatusFixed = 0;
                enable1 |= CAM_CTL_EN1_C24_EN|CAM_CTL_EN1_C02_EN;
                enable2 |= CAM_CTL_EN2_CDRZ_EN|CAM_CTL_EN2_C42_EN;
            }
#endif
            //js_test
            //sensorScenarioId = ( eScenarioID_ZSD == meScenarioID )?ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            //sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            continuous = 1;
            enable1 |=CAM_CTL_EN1_PAK_EN | CAM_CTL_EN1_PAK2_EN;
            isIspOn = 1;
            //!!MUST.to extend BNR line buffer.
            curz_borrow = 1;
            //check HRZ
            PIPE_DBG("ZSD-vInPorts[idx_tgi]->u4ImgWidth:%d ",vInPorts[idx_tgi]->u4ImgWidth);
            if ( CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL < vInPorts[idx_tgi]->u4ImgWidth ) 
            {
                //if ( CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL < ( vInPorts[idx_tgi]->u4ImgWidth >> 1 )) {
                    enable1 |= CAM_CTL_EN1_HRZ_EN;
                    cdrz_in_size_w = CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL;
                //}
                //else {
                //    enable1 |= CAM_CTL_EN1_BIN_EN;
                //    cdrz_in_size_w >>= 1;
                //}
            }
            //check zsd img2o
            if ( !(dma_en & CAM_CTL_DMA_EN_IMG2O_EN) ) {
                enable1 &= (~CAM_CTL_EN1_HRZ_EN);
                enable2 &= (~CAM_CTL_EN2_CDRZ_EN);
            }

            // Zero Shutter Delay,  Pre-processed Raw
            // The raw type, 0: pure raw, 1: pre-process raw
            if(m_RawType == 1) 
            {
                ctl_mux_sel2 = 0x40080108;   // 0x15004078, Bin out after LSC //org
//                ctl_mux_sel2 = 0xC0084108;//0x40380148;   //TEST_MDP ctl_mux_sel2 = 0x40080108;   // 0x15004078, Bin out after LSC
            }
            else
            {
                ctl_mux_sel2 =0x40080104;
            }
            //
            ctl_mux_sel |= 0x00100008;   // 0x15004078, set AA_SEL_EN/AA_SEL

            ctl_sram_mux_cfg = 0x000054F7; //SL TEST_MDP  // 407C.
            

            break;
        case eScenarioID_IC:         //  Image Capture
        case eScenarioID_N3D_VR:     //  Native Stereo Camera VR
        case eScenarioID_N3D_IC:     //  Native Stereo Camera IC
            //sensorScenarioId = ( eScenarioID_N3D_VR == meScenarioID )?ACDK_SCENARIO_ID_CAMERA_PREVIEW:ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
            continuous = 0;
            enable1 =CAM_CTL_EN1_PAK_EN;// remove PAK2 | CAM_CTL_EN1_PAK2_EN; ;
            enable2 = 0;
            dma_en = CAM_CTL_DMA_EN_IMGO_EN;
            int_en |= ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
            isIspOn = 0;
            break;
        case eScenarioID_VSS:       //video snap shot
            //
            isEn1CfaStatusFixed = 1; //Test Green Screen 0;
            isImg2oStatusFixed = 1;
            //a special scenario
            scenario = eScenarioID_N3D_IC;
            continuous = 1;
            //dma_en |= CAM_CTL_DMA_EN_ESFKO_EN;  // reg_400C.  ESFKO_EN[3].
            dma_int |= CAM_CTL_DMA_INT_ESFKO_DONE_EN;    // reg_4028.    ESFKO_DONE_EN[20].
            
            //check img2o
            if(dma_en & CAM_CTL_DMA_EN_IMG2O_EN)    //cc-mode only
            {
                enable1 |= CAM_CTL_EN1_PAK_EN | CAM_CTL_EN1_PAK2_EN;

                isEn1HrzStatusFixed = 0;
                enable1 |= CAM_CTL_EN1_HRZ_EN;
                
                PIPE_INF("VSS-vInPorts[idx_tgi]->u4ImgWidth:%d",vInPorts[idx_tgi]->u4ImgWidth);                

                #if 0   // VSS mode - HRZ has no limitation
                if( CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL < vInPorts[idx_tgi]->u4ImgWidth ) 
                {
                    enable1 |= CAM_CTL_EN1_HRZ_EN;
                    cdrz_in_size_w = CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL;
                }
                #endif
            }
            else
            {
                PIPE_INF("no img2o");
                enable1 |= CAM_CTL_EN1_PAK_EN ;  //SL TEST_MDP               
            }

    
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

            if(dma_en & CAM_CTL_DMA_EN_IMG2O_EN)    //cc-mode only
            {
                PIPE_INF("cc-mode test");
                ctl_mux_sel2 = 0xC0280308;    // 4078. add set PASS1_DONE_MUX to 3, BIN_OUT_SEL to 8
                ctl_sram_mux_cfg = 0x510054F7; //SL TEST_MDP  // 407C.
            }
            else
            {
                //SL            ctl_mux_sel2 = 0x4008010C;   // 4078.   //after HRZ // 0x40080108;//before HRZ // //org
                ctl_mux_sel2 = 0xC028410C;// 0x40380148;   //TEST_MDP ctl_mux_sel2 = 0x4008010C;   // 4078.   //after HRZ // 0x40080108;//before HRZ 
                ctl_sram_mux_cfg = 0x110054F7; //SL TEST_MDP  // 407C.
            }
         
           
            enable1 &= (~CAM_CTL_EN1_BIN_EN);

            //vss_yuv pass1
            if ( eScenarioFmt_YUV == meScenarioFmt ) 
            {
                enable1 &= (~CAM_CTL_EN1_PAK_EN);
                enable1 &= (~CAM_CTL_EN1_PAK2_EN);
                ctl_mux_sel = 0x00400040;   // 4074.    // SGG_SEL_EN = 1, SGG_SEL = 1. //org
                //ctl_mux_sel = 0x06100148; //TEST_VSS_YUV ctl_mux_sel = 0x00400040;   // 4074.    // SGG_SEL_EN = 1, SGG_SEL = 1.

                if(dma_en & CAM_CTL_DMA_EN_IMG2O_EN)    //cc-mode only
                {
                    ctl_mux_sel2 = 0xC0281308;  //4078 add set IMG2O_MUX_EN 1, PASS1_DON_MUX to 19
                }
                else
                {
                    ctl_mux_sel2 = 0xC0081108;  // 4078.    // PASS1_DONE_MUX_EN = 1, PASS1_DONE_MUX = 0x11.
                }

                //ctl_sram_mux_cfg = 0x00021000;  // 407C.    // SGG_HRZ_SEL = 0, PREGAIN_SEL = 0, ESFKO_SOF_SEL_EN = 1, ESFKO_SOF_SEL = 1. //org
                ctl_sram_mux_cfg = 0x110054F7; //TEST_VSS_YUV ctl_sram_mux_cfg = 0x00021000;  // 407C.    // SGG_HRZ_SEL = 0, PREGAIN_SEL = 0, ESFKO_SOF_SEL_EN = 1, ESFKO_SOF_SEL = 1.
            }

            /*
                    -EIS didnot support pass2+tpipe.
                    -EIS_raw/EIS_yuv is in pass1. source is from diff. module.SGG for EIS_raw, CDRZ for EIS_yuv
                    -VSS_YUV_pass1
                        -BIN_OUT_SEL_EN = 1 (reg_4078,[19], set reg_40c8, clr reg_40cc)
                        -BIN_OUT_SEL    = 0 (reg_4078,[3:2], set reg_40c8, clr reg_40cc)
                        -pak_en         = 0 (reg_4004,[12], set reg_4080, clr reg_4084)
                    -VSS_YUV_pass2
                        -C02_SEL_EN     = 1 (reg_4074,[25], set reg_40c0, clr reg_40c4)
                        -C02_SEL        = 0 (reg_4074,[11:10], set reg_40c0,, clr reg_40c4)
                        -G2G_SEL_EN     = 1 (reg_4074,[26], set reg_40c0, clr reg_40c4)
                        -G2G_SEL        = 0 (reg_4074,[12], set reg_40c0, clr reg_40c4)
                    -if need to get EIS_yuv after SGG, try hidden path
                        -EIS_RAW_SEL    = 1 (reg_4018,[16], set reg_40a0, clr reg_40a4)
                        -SGG_SEL_EN     = 1 (reg_4074,[22], set reg_40c0, clr reg_40c4)
                        -SGG_SEL        = 1 (reg_4074,[7:6], set reg_40c0, clr reg_40c4)
                        -SGG_EN         = 1 (reg_4004,[15], set reg_4080, clr reg_4084)
                        -PASS1_DONE_MUX_EN = 1  (reg_4078,[30], set reg_40c8, clr reg_40cc)
                        -PASS1_DONE_MUX = 0x11  (reg_4078,[12:8], set reg_40c8, clr reg_40cc)
                        -SGG_HRZ_SEL    = 0 (reg_407c,[28], set reg_40d0, clr reg_40d4)
                        -PREGAIN_SEL    = 0 (reg_407c,[24], set reg_40d0, clr reg_40d4)
                        -ESFKO_SOF_SEL_EN = 1   (reg_407c,[17], set reg_40d0, clr reg_40d4)
                        -ESFKO_SOF_SEL = 1  (reg_407c,[12], set reg_40d0, clr reg_40d4)
                    */
            break;
        case eScenarioID_IP:         //  Image Playback
        default:
            PIPE_ERR("NOT Support scenario");
            return MFALSE;
    }
    //
    MUINT32 imgFmt = 0;
    MINT32* pPixel_byte = NULL;
    MINT32* pPass1_out_fmt = NULL;
    MINT32  cam_out_fmt_tmp;
    //
    for (MUINT32 i = 0 ; i < 2 ; i++ ) {
        //
        if ( 0 == i ) {
            if (-1 == idx_imgo ) {
                continue;
            }
            //
            imgFmt = (MUINT32)portInfo_imgo.eImgFmt;
            pPixel_byte = (MINT32*)&pixel_byte_imgo;
            pPass1_out_fmt = (MINT32*)&cam_out_fmt;
        }
        else if ( 1 == i ) {
            if (-1 == idx_img2o ) {
                continue;
            }
            //
            imgFmt = (MUINT32)portInfo_img2o.eImgFmt;
            pPixel_byte = (MINT32*)&pixel_byte_img2o;
            pPass1_out_fmt = (MINT32*)&cam_out_fmt_tmp;
        }
        //
        if ( NULL == pPixel_byte ) {
            PIPE_ERR("ERROR:NULL pPixel_byte");
            return MFALSE;
        }
        //
        switch (imgFmt) {
            case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
                *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
                *pPass1_out_fmt = CAM_FMT_SEL_BAYER8;
                break;
            case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
                *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
                *pPass1_out_fmt = CAM_FMT_SEL_BAYER10;
                break;
            case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
                *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
                *pPass1_out_fmt = CAM_FMT_SEL_BAYER12;
                break;
            case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
            case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
            case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
            case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
                *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
                *pPass1_out_fmt = CAM_FMT_SEL_YUV422_1P;
                break;
            default:
                PIPE_ERR("eImgFmt:[%d]NOT Support yet",imgFmt);
                return MFALSE;
        }
        //
        PIPE_INF("i(%d),imgFmt(%d),*pPixel_byte(%d),pPass1_out_fmt(%d)",i,imgFmt,*pPixel_byte,*pPass1_out_fmt);
    }
    //

    //
    /*-----------------------------------------------------------------------------
      m_camPass1Param
      -----------------------------------------------------------------------------*/
    //scenario/sub_mode
    //m_camPass1Param.scenario = ISP_SCENARIO_VR;
    //m_camPass1Param.subMode =  ISP_SUB_MODE_RAW;

    /* 2-pixel/2 pixel mode
            -reg_4010[24] ->TG1       TWO_PIX
            -reg_4410[1]  ->TG        DBL_DATA_BUS
            -reg_8010[8]  ->seninf    SENINF_PIX_SEL
           //
           if ( 2-pixel_mode ) {
                CAM_CTL_EN1_BIN_EN
           }
           else {
           }

     */

    //single/continuous mode
    m_camPass1Param.b_continuous = continuous; //
    //enable table
    m_camPass1Param.en_Top.enable1 = enable1;
    m_camPass1Param.en_Top.enable2 = enable2;
    m_camPass1Param.en_Top.dma = dma_en;
    m_camPass1Param.ctl_int.dma_int = dma_int;
    m_camPass1Param.ctl_int.int_en = int_en;
    m_camPass1Param.isIspOn = isIspOn;
    //
    m_camPass1Param.CQ= pass1_CQ;
    //fmt_sel
    m_camPass1Param.fmt_sel.reg_val = 0x00; //reset fmt_sel
    m_camPass1Param.fmt_sel.bit_field.scenario = scenario;
    m_camPass1Param.fmt_sel.bit_field.sub_mode = subMode;
    m_camPass1Param.fmt_sel.bit_field.cam_out_fmt = cam_out_fmt;
    //TODO:remove later. should be set by frontend
    //m_camPass1Param.fmt_sel.bit_field.tg1_fmt = tg1_fmt;
    //m_camPass1Param.fmt_sel.bit_field.tg1_sw = tg1_sw;
    //ctl_sel
    //WORKAROUND: to fix CQ0B/CQ0C fail issue
    int DB_en = 1;
    m_camPass1Param.ctl_sel.reg_val = 0;
    m_camPass1Param.ctl_sel.bit_field.tdr_sel = 1;//DB_en?0:1;
    m_camPass1Param.ctl_sel.bit_field.pass2_db_en = 0;//DB_en?1:0;
    m_camPass1Param.ctl_sel.bit_field.pass1_db_en = 1;//DB_en?1:0;
    m_camPass1Param.ctl_sel.bit_field.eis_raw_sel = eis_raw_sel;
    m_camPass1Param.ctl_sel.bit_field.tg_sel = tg_sel;
    m_camPass1Param.ctl_sel.bit_field.CURZ_BORROW = curz_borrow;
    if ( m_camPass1Param.ctl_sel.bit_field.tdr_sel == m_camPass1Param.ctl_sel.bit_field.pass2_db_en ) {
        PIPE_ERR("Error:TDR_SEL/PASS2_DB_EN conflict ");
        return MFALSE;
    }
    //mux_sel
    m_camPass1Param.ctl_mux_sel.reg_val = ctl_mux_sel;
    //mux_sel2
    m_camPass1Param.ctl_mux_sel2.reg_val = ctl_mux_sel2;
    //
    m_camPass1Param.ctl_sram_mux_cfg.reg_val = ctl_sram_mux_cfg;
    //
    m_camPass1Param.isEn1C24StatusFixed = isEn1C24StatusFixed;
    m_camPass1Param.isEn1C02StatusFixed = isEn1C02StatusFixed;
    m_camPass1Param.isEn1CfaStatusFixed = isEn1CfaStatusFixed;
    m_camPass1Param.isEn1HrzStatusFixed = isEn1HrzStatusFixed;
    m_camPass1Param.isEn1MfbStatusFixed = isEn1MfbStatusFixed;
    m_camPass1Param.isEn2CdrzStatusFixed = isEn2CdrzStatusFixed;
    m_camPass1Param.isEn2G2cStatusFixed = isEn2G2cStatusFixed;
    m_camPass1Param.isEn2C42StatusFixed = isEn2C42StatusFixed;
    m_camPass1Param.isImg2oStatusFixed = isImg2oStatusFixed;
    m_camPass1Param.isAaoStatusFixed = isAaoStatusFixed;
    m_camPass1Param.isEsfkoStatusFixed = isEsfkoStatusFixed;
    m_camPass1Param.isFlkiStatusFixed = isFlkiStatusFixed;
    m_camPass1Param.isLcsoStatusFixed = isLcsoStatusFixed;
    m_camPass1Param.isEn1AaaGropStatusFixed = isEn1AaaGropStatusFixed;
    m_camPass1Param.isShareDmaCtlByTurn = isShareDmaCtlByTurn;
    //
    //pix_id
    m_camPass1Param.pix_id = pix_id;



    //source -> from TG
    m_camPass1Param.src_img_size.w = vInPorts[idx_tgi]->u4ImgWidth;
    m_camPass1Param.src_img_size.h = vInPorts[idx_tgi]->u4ImgHeight;
    m_camPass1Param.src_img_size.stride = m_camPass1Param.src_img_size.stride;
    m_camPass1Param.src_img_roi.w = m_camPass1Param.src_img_size.w;
    m_camPass1Param.src_img_roi.h = m_camPass1Param.src_img_size.h;
    m_camPass1Param.src_img_roi.x = 0;
    m_camPass1Param.src_img_roi.y = 0;
    //m_camPass1Param.src_color_format = srcColorFormat;
    //config cdrz
    m_camPass1Param.cdrz_in_size.w = cdrz_in_size_w;
    m_camPass1Param.cdrz_in_size.h = vInPorts[idx_tgi]->u4ImgHeight;
    
    //imgo
    if (-1 != idx_imgo ) 
    {
        // use output dma crop
        PIPE_INF("config imgo");

        // not support x crop
        portInfo_imgo.crop.x = 0;
        portInfo_imgo.crop.floatX = 0;
        portInfo_imgo.crop.w = portInfo_imgo.u4ImgWidth;

        if((portInfo_imgo.crop.y!= 0)|| (portInfo_imgo.crop.floatX!= 0) || (portInfo_imgo.crop.h != portInfo_imgo.u4ImgHeight))
        {
            PIPE_DBG("[warning] crop_y=%d,crop_floatY(%d),crop_h=%d\n", portInfo_imgo.crop.y, portInfo_imgo.crop.floatY, portInfo_imgo.crop.h);
        }

         // use output dma crop
        this->configDmaPort(&portInfo_imgo,m_camPass1Param.imgo,(MUINT32)pixel_byte_imgo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);

        PIPE_INF("imgo_crop [%d, %d, %d, %d]_f(0x%x,0x%x)\n", \
                m_camPass1Param.imgo.crop.x,m_camPass1Param.imgo.crop.y, \
                m_camPass1Param.imgo.crop.w,m_camPass1Param.imgo.crop.h, \
                m_camPass1Param.imgo.crop.floatX,m_camPass1Param.imgo.crop.floatY);
    }
    //img2o
    if (-1 != idx_img2o ) 
    {
        // use output dma crop
        PIPE_INF("config img2o + crop");
        // not support x crop
        //portInfo_img2o.crop.x = 0;
        //portInfo_img2o.crop.floatX = 0;
        //portInfo_img2o.crop.w = portInfo_img2o.u4ImgWidth;

        if((portInfo_img2o.crop.y!=0) || (portInfo_img2o.crop.h == portInfo_img2o.u4ImgHeight))
            PIPE_DBG("[warning] crop_y=%d crop_floatY(%d) crop_h=%d\n", \
                portInfo_img2o.crop.y,portInfo_img2o.crop.floatY,portInfo_img2o.crop.h);

        this->configDmaPort(&portInfo_img2o,m_camPass1Param.img2o,(MUINT32)pixel_byte_img2o,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
    }
    
    ret = m_CamPathPass1.config( &m_camPass1Param );
    if( ret != 0 )
    {
        PIPE_ERR("Pass 1 config error!");
        return MFALSE;
    }

    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    PIPE_DBG("NOT SUPPORT for camio pipe ");
    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CamIOPipe::
onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2)
{
int ret = 0;

    PIPE_DBG("+ tid(%d) (u4Param1, u4Param2)=(%d, %d)", gettid(), u4Param1, u4Param2);

    switch ( u4Param1 ) {
/*
        case EPIPECmd_SET_ZOOM_RATIO:
        ret = m_CamPathPass1.setZoom( u4Param2 );
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
CamIOPipe::
onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam)
{
    PIPE_DBG("+ tid(%d) (u4InParam)=(%d)", gettid(), u4InParam);
    *pu4OutParam = 0x12345678;
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
irq(EPipePass pass, EPipeIRQ irq_int)
{
int    ret = 0;
MINT32 type = 0;
MUINT32 irq = 0;

    PIPE_DBG("+ tid(%d) (pass,irq_int)=(0x%08x,0x%08x)", gettid(), pass, irq_int);

    //pass
    if ( EPipePass_PASS1_TG1 != pass && EPipePass_PASS1_TG2 != pass ) {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        return MFALSE;
    }
    //irq_int
    if ( EPIPEIRQ_VSYNC == irq_int ) {
        type = ISP_DRV_IRQ_TYPE_INT;
        irq = (EPipePass_PASS1_TG1 == pass)?ISP_DRV_IRQ_INT_STATUS_VS1_ST:ISP_DRV_IRQ_INT_STATUS_VS2_ST;
    }
    else if ( EPIPEIRQ_PATH_DONE == irq_int ) {
        type = ISP_DRV_IRQ_TYPE_INT;
        irq = (EPipePass_PASS1_TG1 == pass)?ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST:ISP_DRV_IRQ_INT_STATUS_PASS1_TG2_DON_ST;
    }
/*
    else if ( EPIPEIRQ_SOF == irq_int ) {
        type = (EPipePass_PASS1_TG1 == pass)?ISP_DRV_IRQ_TYPE_INT:ISP_DRV_IRQ_TYPE_DMA;
        irq = (EPipePass_PASS1_TG1 == pass)?ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST:ISP_DRV_IRQ_DMA_INT_SOF2_INT_ST;
    }
*/
    else {
        PIPE_ERR("IRQ:NOT SUPPORT irq");
        return  MFALSE;
    }
    //
    PIPE_DBG("(type,irq)=(0x%08x,0x%08x)", type, irq);
    //
    ret = m_CamPathPass1.waitIrq(type,irq);

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
CamIOPipe::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
int    ret = 0;
IspSize out_size;
IspDMACfg out_dma;
MUINT32 dmaChannel = 0;

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

    switch ( cmd ) {
        case EPIPECmd_SET_ISP_CDRZ:
            ::memcpy((char*)&out_size,(char*)arg1,sizeof(IspSize));
            m_CamPathPass1.setCdrz( out_size );
            break;
        case EPIPECmd_SET_ISP_IMGO:
            ::memcpy((char*)&out_dma,(char*)arg1,sizeof(IspDMACfg));
            m_CamPathPass1.setDMAImgo( out_dma );
            break;
        case EPIPECmd_SET_CURRENT_BUFFER:
            #if 0
            if ( EPortIndex_IMGO == arg1 ) {
                dmaChannel = ISP_DMA_IMGO;
            }
            else if ( EPortIndex_IMG2O == arg1 ) {
                dmaChannel = ISP_DMA_IMG2O;
            }
            //
            m_CamPathPass1.setDMACurrBuf((MUINT32) dmaChannel);
            #endif
            PIPE_INF("No need anymore for Pass1");
            break;
        case EPIPECmd_SET_NEXT_BUFFER:
            #if 0
            if ( EPortIndex_IMGO == arg1 ) {
                dmaChannel = ISP_DMA_IMGO;
            }
            else if ( EPortIndex_IMG2O == arg1 ) {
                dmaChannel = ISP_DMA_IMG2O;
            }
            #endif
            PIPE_INF("No need anymore for Pass1");
            //
            m_CamPathPass1.setDMANextBuf((MUINT32) dmaChannel);
            break;
        case EPIPECmd_SET_CQ_CHANNEL:
            m_pass1_CQ = arg1;//CAM_ISP_CQ0
            m_CamPathPass1.CQ = m_pass1_CQ;
            m_CamPathPass1.flushCqDescriptor((MUINT32) m_pass1_CQ);
            break;
        case EPIPECmd_SET_CONFIG_STAGE:
            m_settingStage = (EConfigSettingStage)arg1;
            break;
        case EPIPECmd_SET_CQ_TRIGGER_MODE:
            //TO Physical Reg.
            m_CamPathPass1.setCQTriggerMode(arg1,arg2,arg3);
            m_CQ0TrigMode  = (ISP_DRV_CQ0 == (ISP_DRV_CQ_ENUM)arg1)?arg2:0;
            m_CQ0BTrigMode = (ISP_DRV_CQ0B == (ISP_DRV_CQ_ENUM)arg1)?arg2:0;
            break;
        case EPIPECmd_FREE_MAPPED_BUFFER:
            {
                stISP_BUF_INFO buf_info = (stISP_BUF_INFO)(*((stISP_BUF_INFO*)arg2));
                m_CamPathPass1.freePhyBuf(arg1,buf_info);
            }
            break;
        case EPIPECmd_SET_IMGO_RAW_TYPE:
            {
                if ( eRawImageType_PreProc == arg1 ) {
                    m_RawType = 1;
                }
            }
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

