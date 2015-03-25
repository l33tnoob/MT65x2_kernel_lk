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
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG    LOG_TAG
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);
// Clear previous define, use our own define.
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (pipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (pipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (pipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (pipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (pipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (pipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
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
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    m_vBufImgo.resize(1);    

    // create isp driver
    m_pIspDrv = IspDrv::createInstance();
}
/*******************************************************************************
*
********************************************************************************/
CamIOPipe::
~CamIOPipe()
{   
    m_pIspDrv->destroyInstance();    
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
init()
{
    MBOOL ret = MTRUE;
    MUINT32 reg_val;
    isp_reg_t *pisp;

    PIPE_DBG(" +");
    
    //
    if ( m_pIspDrv )
    {
        m_pIspDrv->init();
        pisp =(isp_reg_t *)(m_pIspDrv->getRegAddr());

        ISP_WRITE_REG(pisp, CAM_CTL_MODULE_EN, 0);
        ISP_WRITE_REG(pisp, CAM_CTL_CLK_EN, 0);
        ISP_WRITE_REG(pisp, CAM_CTL_INT_EN, 0);
    }
    m_pIspDrv->freeAllPhyBuf(); // clear isp kernel ring buff

    PIPE_DBG(" -");
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
uninit()
{
    MBOOL ret = MTRUE;
    
    PIPE_DBG(" +");
    
    //
    m_pIspDrv->uninit();

    PIPE_DBG(" -");
    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
start()
{
    MBOOL ret = MTRUE;
    
    PIPE_DBG(" +");
    
    //m_CamPathPass1.start((void*)&path);
    m_pIspDrv->control(MTRUE);

    PIPE_DBG(" -");
    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
startCQ0()
{
#if 0  
    /*
    int path  = CAM_ISP_PASS1_START;

        PIPE_DBG(":E");
        //
    path  = CAM_ISP_PASS1_CQ0_START;
    m_CamPathPass1.start((void*)&path);
    */
    /*
        if ( CQ_CONTINUOUS_EVENT_TRIGGER != m_CQ0TrigMode ) {
            m_camPass1Param.CQ = CAM_ISP_CQ_NONE;
            m_CamPathPass1.CQ = CAM_ISP_CQ_NONE;
        }
    */
#endif
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
startCQ0B()
{
#if 0  
    /*
    int path  = CAM_ISP_PASS1_START;

        PIPE_DBG(":E");
        //
        path  = CAM_ISP_PASS1_CQ0B_START;
        m_CamPathPass1.start((void*)&path);
    */
    /*
        if ( CQ_CONTINUOUS_EVENT_TRIGGER != m_CQ0BTrigMode ) {
            m_camPass1Param.CQ = CAM_ISP_CQ_NONE;
            m_CamPathPass1.CQ = CAM_ISP_CQ_NONE;
        }
    */
#endif
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
stop()
{
    MBOOL ret = MTRUE;
    
    PIPE_DBG(" +");
       
    //m_CamPathPass1.stop((void*)&path);
    m_pIspDrv->control(MFALSE);

    PIPE_DBG(" -");
    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
#if 0  
    MUINT32 dmaChannel = 0;
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
    //
#endif
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
#if 0  
    PIPE_DBG("+ tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
#endif   
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MBOOL ret = MTRUE;
    //
    ISP_DRV_BUF_STRUCT buff;
    ISP_DRV_BUF_STRUCT ex_buff;    

    PIPE_DBG(" +");
    
    //
    Mutex::Autolock lock(m_queLock);
    //
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", 
                    gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", 
                    rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    buff.memID = rQBufInfo.vBufInfo[0].memID;    
    buff.size = rQBufInfo.vBufInfo[0].u4BufSize;
    buff.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    buff.base_pAddr = rQBufInfo.vBufInfo[0].u4BufPA;
    buff.next = (ISP_DRV_BUF_STRUCT*)NULL;
    PIPE_DBG("buff,(%d),(0x%08x),(0x%08x),(0x%08x)", buff.memID, buff.size
                                                   , buff.base_vAddr, buff.base_pAddr);
    if ( 2 <= rQBufInfo.vBufInfo.size() )
    {   //enque exchanged buffer
        ex_buff.memID = rQBufInfo.vBufInfo[1].memID;
        ex_buff.size = rQBufInfo.vBufInfo[1].u4BufSize;
        ex_buff.base_vAddr = rQBufInfo.vBufInfo[1].u4BufVA;
        ex_buff.base_pAddr = rQBufInfo.vBufInfo[1].u4BufPA;
        ex_buff.next = (ISP_DRV_BUF_STRUCT*)NULL;
        PIPE_DBG("ex_buff,(%d),(0x%08x),(0x%08x),(0x%08x)", ex_buff.memID, ex_buff.size
                                                          , ex_buff.base_vAddr, ex_buff.base_pAddr);
        //set for original buffer info.
        buff.next = (ISP_DRV_BUF_STRUCT*)&ex_buff;
    }
    
    ret = m_pIspDrv->enqueueHwBuf(buff);
    if( MTRUE != ret )
    {
        PIPE_ERR("IspDrv enqueueHwBuf failed");
        PIPE_DBG(" -");
        return MFALSE;
    }

    PIPE_DBG(" -");
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MBOOL ret = MTRUE;
    //
    ISP_DRV_FILLED_BUF_LIST filledBufList;
    ISP_DRV_BUF_L bufList;

    PIPE_DBG(" +");
    
    //
    Mutex::Autolock lock(m_queLock);
    //
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", 
                        gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", 
                        rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    filledBufList.pBufList = &bufList;

    //check if there is already filled buffer
    ret = m_pIspDrv->waitBufReady();
    if(MFALSE == ret)
    {
        PIPE_ERR("waitBufReady failed");
        PIPE_DBG(" -");
        return ret;
    }

    //move FILLED buffer from hw to sw list
    ret = m_pIspDrv->dequeueHwBuf();
    if(MFALSE == ret)
    {    
        PIPE_ERR("dequeueHwBuf failed");
        PIPE_DBG(" -");
        return ret;
    }

    //delete all after move sw list to bufInfo.
    ret = m_pIspDrv->dequeueSwBuf(filledBufList);
    if(MFALSE == ret)
    {
        PIPE_ERR("dequeueSwBuf failed");
        PIPE_DBG(" -");
        return ret;
    }
            
    // fill to output
    rQBufInfo.vBufInfo.resize(bufList.size());
    PIPE_DBG("bufList size:[0x%x]", bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) 
    {
        rQBufInfo.vBufInfo[i].memID             = bufList.front().memID;
        rQBufInfo.vBufInfo[i].u4BufSize         = bufList.front().size;
        rQBufInfo.vBufInfo[i].u4BufVA           = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufPA           = bufList.front().base_pAddr;
        rQBufInfo.vBufInfo[i].i4TimeStamp_sec   = bufList.front().timeStampS;
        rQBufInfo.vBufInfo[i].i4TimeStamp_us    = bufList.front().timeStampUs;
        //
        bufList.pop_front();
        //
        PIPE_DBG("size:[0x%x]/vAddr:[0x%x]/memid:[%d]/buf size:[%d]",
                                        rQBufInfo.vBufInfo[i].u4BufSize,
                                        rQBufInfo.vBufInfo[i].u4BufVA,
                                        rQBufInfo.vBufInfo[i].memID,
                                        rQBufInfo.vBufInfo.size());        
    }
    
    PIPE_DBG(" -");
    return ret;   
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    MBOOL ret = MTRUE;
    //
    isp_reg_t *pisp;

    PortInfo    portInfo_tgi;    
    PortInfo portInfo_imgo;
    MINT32      idx_tgi = -1;    
    MINT32      idx_imgo = -1;    
    MUINT32     imgInFmt = 0;
    MUINT32     imgOutFmt = 0;
    MUINT32     imgInW, imgInH, imgOutW, imgOutH;
    MUINT32     swapY, swapCbCr;
    MBOOL       cdrzEn = MFALSE;
    MBOOL       singleMode = MFALSE;
    
    PIPE_DBG(" +");
    
    PIPE_DBG("inPort size[%d]/outPort size[%d]", vInPorts.size(), vOutPorts.size());

    // InPorts info
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) 
    {
        if ( 0 == vInPorts[i] ) { continue; }
        
        PIPE_INF("vInPorts:[%d]: fmt(0x%x),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)",
                        i, vInPorts[i]->eImgFmt,
                        vInPorts[i]->u4ImgWidth, vInPorts[i]->u4ImgHeight,
                        vInPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                        vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                        vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                        vInPorts[i]->type, vInPorts[i]->index, vInPorts[i]->inout);

        if(EPortIndex_TG1I == vInPorts[i]->index)
        {            
            idx_tgi = i;
            portInfo_tgi = (PortInfo)*vInPorts[idx_tgi];
            //enable1 |= CAM_CTL_EN1_TG1_EN;            
        }
        else
        {
            PIPE_ERR("InPort error: should be TG1I");
            PIPE_DBG(" -");
            return MFALSE;
        }
    }
    
    // OutPorts info
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) 
    {
        if ( 0 == vOutPorts[i] ) { continue; }
        
        PIPE_INF("vOutPorts:[%d]:(0x%x),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)",
                        i, vOutPorts[i]->eImgFmt,
                        vOutPorts[i]->u4ImgWidth, vOutPorts[i]->u4ImgHeight,
                        vOutPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                        vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                        vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                        vOutPorts[i]->type, vOutPorts[i]->index, vOutPorts[i]->inout);
        
        if (EPortIndex_IMGO == vOutPorts[i]->index) 
        {
            idx_imgo = i;            
            portInfo_imgo =  (PortInfo)*vOutPorts[idx_imgo];
            //dma_en |= CAM_CTL_DMA_EN_IMGO_EN;            
        }
        else
        {
            PIPE_ERR("OutPort error: should be IMGO");
            PIPE_DBG(" -");
            return MFALSE;
        }
    }   

    // check scenario and fmt
    PIPE_INF("meScenarioID:[%d]", meScenarioID);
    switch(meScenarioID)
    {
        case eScenarioID_VSS:                        
        case eScenarioID_ZSD:
            break;
        default:
            PIPE_ERR("NOT Support scenario[%d]", meScenarioID);
            PIPE_DBG(" -");
            return MFALSE;
    }    

    PIPE_INF("meScenarioFmt:[%d]", meScenarioFmt);
    switch(meScenarioFmt)
    {
        case eScenarioFmt_YUV:
            imgInFmt = ISP_DRV_IMG_FORMAT_YUV422;
            break;
        case eScenarioFmt_JPG:            
            imgInFmt = ISP_DRV_IMG_FORMAT_JPEG;
            break;        
        default:
            PIPE_ERR("NOT Support submode[$d]", meScenarioFmt);
            PIPE_DBG(" -");
            return MFALSE;
    }

    PIPE_INF("portInfo_imgo.eImgFmt:[%d]", (MUINT32)portInfo_imgo.eImgFmt);
    switch((MUINT32)portInfo_imgo.eImgFmt)
    {
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            imgOutFmt = ISP_DRV_IMG_FORMAT_YUV422;
            break;
        case eImgFmt_JPEG:  
            imgOutFmt = ISP_DRV_IMG_FORMAT_JPEG;
            break;
        default:
            PIPE_ERR("NOT Support eImgFmt[%d]", imgOutFmt);
            PIPE_DBG(" -");
            return MFALSE;
    }       

    // retrieve info for pipe usage
    if(imgInFmt != imgOutFmt)
    {
        PIPE_ERR("inFmt(%d)/outFmt(%d) miss match");
        PIPE_DBG(" -");
        //return MFALSE;
    }
    
    swapY = 0;
    swapCbCr = 0;
    if(ISP_DRV_IMG_FORMAT_JPEG == imgOutFmt)
    {
        singleMode = MTRUE;        
    }
    else
    {
    singleMode = MFALSE;
    }
    PIPE_INF("swap(%d, %d) singleMode(%d)", swapY, swapCbCr, singleMode);

    imgInW = portInfo_tgi.u4ImgWidth;
    imgInH = portInfo_tgi.u4ImgHeight;
    imgOutW = portInfo_imgo.u4ImgWidth;
    imgOutH = portInfo_imgo.u4ImgHeight;  
    cdrzEn = (imgInW != imgOutW)? MTRUE : MFALSE;
    PIPE_INF("in(%d, %d)->out(%d, %d)", imgInW, imgInH, imgOutW, imgOutH);

    // config isp registers
    pisp =(isp_reg_t *)(m_pIspDrv->getRegAddr());

    m_pIspDrv->setImgoSize(imgOutW, imgOutH);       
    m_pIspDrv->setCdrzCtrl(cdrzEn, imgInW, imgInH, imgOutW, imgOutH);    
    m_pIspDrv->setCamModule(MTRUE, imgOutFmt, swapY, swapCbCr);    
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SINGLE_MODE, singleMode);

    PIPE_DBG(" -");
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{    
    PIPE_ERR("CamIOPipe configPipeUpdate NOT SUPPORT!"); 
    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CamIOPipe::
onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2)
{
#if 0    
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
#endif 
    PIPE_ERR("CamIOPipe onSet2Params NOT SUPPORT!");
    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CamIOPipe::
onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam)
{
#if 0  
    PIPE_DBG("+ tid(%d) (u4InParam)=(%d)", gettid(), u4InParam);
    *pu4OutParam = 0x12345678;
#endif    
    PIPE_ERR("CamIOPipe onGet1ParamBasedOn1Input NOT SUPPORT!");
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
irq(EPipePass pass, EPipeIRQ irq_int)
{
    MBOOL ret = MTRUE;
    //
    ISP_DRV_WAIT_IRQ_STRUCT irq;
    //

    PIPE_DBG(" +");
    PIPE_DBG("tid(%d) (pass,irq_int)=(0x%08x,0x%08x)", gettid(), pass, irq_int);

    //pass
    if ( EPipePass_PASS1_TG1 != pass ) 
    {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        PIPE_DBG(" -");
        return MFALSE;
    }
    
    //irq_int
    switch(irq_int)
    {
        case EPIPEIRQ_VSYNC:
            irq.status = ISP_DRV_INT_VS1_ST;
            break;
        case EPIPEIRQ_PATH_DONE:
            irq.status = ISP_DRV_INT_PASS1_TG1_DON_ST;
            break;
        default:
            PIPE_ERR("IRQ:NOT SUPPORT irq type");
            PIPE_DBG(" -");
            return  MFALSE;
    }
    irq.clear = ISP_DRV_IRQ_CLEAR_WAIT;
    irq.timeout = CAM_INT_WAIT_TIMEOUT_MS; // tmp
    ret = m_pIspDrv->waitIrq(irq);

    if( MFALSE == ret )
    {
        PIPE_ERR("waitIrq error!");
        PIPE_DBG(" -");
        return  MFALSE;
    }

    PIPE_DBG(" -");
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    MBOOL ret = MTRUE;
	MUINT32 DevId = 1;


	PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

	switch ( cmd ) {
		case EPIPECmd_SET_SENSOR_DEV:
			DevId = (MUINT32) arg1;
			m_pIspDrv->setDevID(DevId);
			break;
		default:
			PIPE_ERR("NOT support command!");
			return MFALSE;
	}


#if 0   
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

    if( ret != MTRUE )
    {
        PIPE_ERR("sendCommand error!");
        return MFALSE;
    }
#endif  
    //
    PIPE_ERR("CamIOPipe sendCommand NOT SUPPORT!");
    return  ret;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

