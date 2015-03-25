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
static MBOOL configDpBufInfo(PortInfo const &srcPortInfo, int const srcBufAddr, DpColorFormat &dstFmt, unsigned int dstAddrList[3], unsigned int dstSizeList[3], unsigned int &dstPlaneNum, unsigned int &dstYStride, unsigned int &dstUVStride)
{
    int srcYSize = srcPortInfo.u4Stride[0] * srcPortInfo.u4ImgHeight;  
    dstYStride = srcPortInfo.u4Stride[0];
    dstUVStride = srcPortInfo.u4Stride[1];
    
    switch (srcPortInfo.eImgFmt)
    { 
    case eImgFmt_YV12:    /*!< 420 format, 3 plane (Y),(V),(U) */
        dstFmt = eYV12;
        dstPlaneNum = 3;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstAddrList[2] = srcBufAddr + srcYSize + dstUVStride*srcPortInfo.u4ImgHeight/2;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight/2;
        dstSizeList[2] = dstUVStride*srcPortInfo.u4ImgHeight/2;
        break;
    case eImgFmt_NV21:    /*!< 420 format, 2 plane (Y),(VU) */ 
        dstFmt = eNV21;
        dstPlaneNum = 2;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight/2;
        break;
    case eImgFmt_NV12:    /*!< 420 format, 2 plane (Y),(UV) */ 
        dstFmt = eNV12;  
        dstPlaneNum = 2;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight/2;
        break;
    case eImgFmt_YUY2:    /*!< 422 format, 1 plane (YUYV) */ 
        dstFmt = eYUYV;    
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*2;
        dstYStride *= 2;
        break;
    case eImgFmt_UYVY:    /*!< 422 format, 1 plane (UYVY) */ 
        dstFmt = eUYVY; 
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*2;
        dstYStride *= 2;
        break;
    case eImgFmt_RGB565:    /*!< RGB 565 (16-bit), 1 plane */ 
        dstFmt = eRGB565;    
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*2;
        dstYStride *= 2;
        break;
    case eImgFmt_RGB888:    /*!< RGB 888 (24-bit), 1 plane (RGB) */ 
        dstFmt = eRGB888;
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*3;
        dstYStride *= 3;
        break;
    case eImgFmt_ARGB888:    /*!< ARGB (32-bit), 1 plane */ 
        dstFmt = eARGB8888;  	
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*4;
        dstYStride *= 4;
        break;
    case eImgFmt_YV16:    /*!< 422 format, 3 plane (Y),(U),(V)*/ 
        dstFmt = eYV16;    	
        dstPlaneNum = 3;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstAddrList[2] = srcBufAddr + srcYSize + dstUVStride*srcPortInfo.u4ImgHeight;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight;
        dstSizeList[2] = dstUVStride*srcPortInfo.u4ImgHeight;
        break;
    case eImgFmt_NV16:    /*!< 422 format, 2 plane (Y),(UV)*/ 
        dstFmt = eNV16;  
        dstPlaneNum = 2;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight;
        break;
    case eImgFmt_NV61:    /*!< 422 format, 2 plane (Y),(VU)*/ 
        dstFmt = eNV61;    	
        dstPlaneNum = 2;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight;
        break;
    case eImgFmt_I420:    /*!<420 format, 3 plane (Y),(U),(V) */ 
        dstFmt = eYV21;    	
        dstPlaneNum = 3;
        dstAddrList[0] = srcBufAddr;
        dstAddrList[1] = srcBufAddr + srcYSize;
        dstAddrList[2] = srcBufAddr + srcYSize + dstUVStride*srcPortInfo.u4ImgHeight/2;
        dstSizeList[0] = srcYSize;
        dstSizeList[1] = dstUVStride*srcPortInfo.u4ImgHeight/2;
        dstSizeList[2] = dstUVStride*srcPortInfo.u4ImgHeight/2;
        break;
    case eImgFmt_Y800:    /*!< Y plane only  */ 
        dstFmt = eGREY;   
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize;
        break;
    case eImgFmt_YVYU:    /*!< 422 format, 1 plane (YVYU)*/ 
        dstFmt = eYVYU;   
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*2;
        dstYStride *= 2;
        break;
    case eImgFmt_VYUY:    /*!< 422 format, 1 plane (VYUY)   */ 
        dstFmt = eVYUY;
        dstPlaneNum = 1;
        dstAddrList[0] = srcBufAddr;
        dstSizeList[0] = srcYSize*2;
        dstYStride *= 2;
        break;
    case eImgFmt_NV12_BLK:
    case eImgFmt_NV21_BLK:
    case eImgFmt_UNKNOWN:   /*!< unknow */
    case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
    case eImgFmt_BAYER10:    /*!< Bayer format, 10-bit */ 
    case eImgFmt_BAYER12:    /*!< Bayer format, 12-bit */
    case eImgFmt_JPEG:    /*!< JPEG format */
    default:
        return MFALSE;
    }
    return MTRUE;
}


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
      //m_pIspDrvShell(NULL),
      m_resMgr(NULL),
      m_pipePass(EPipePass_PASS2),
      //m_isPartialUpdate(MFALSE),
      m_isImgPlaneByImgi(MFALSE)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    //memset(&this->m_camPass2Param,0x00,sizeof(CamPathPass1Parameter));
    this->m_vBufImgi.resize(0);
    this->m_vBufDispo.resize(0);
    this->m_vBufVido.resize(0);

    /*** create isp driver ***/
    //m_pIspDrvShell = IspDrvShell::createInstance((eScenarioID == eScenarioID_GDMA) ? (MTRUE) : (MFALSE));   // When Scenario is GDMA, pass MTRUE.

    /* create resource manager */
    m_resMgr = ResMgrDrv::CreateInstance();

}

CdpPipe::
~CdpPipe()
{
    /*** release isp driver ***/
    //m_pIspDrvShell->destroyInstance();

}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
init()
{
    PIPE_INF("CDP :E");
    //
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    //
    /*if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init();
        m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountInc();
        m_CamPathPass2.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
    }*/
    //
    if ( m_resMgr ) {
        m_resMgr->Init();
    }
#if 0
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
#endif
    PIPE_INF("CDP :X");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
uninit()
{
    PIPE_INF("CDP :E");
    //IMEM_BUF_INFO buf_info;
    //
    Mutex::Autolock lock(mLock);
#if 0
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

    m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountDec();
    //
    m_pIspDrvShell->uninit();
#endif
    //
    //m_dpStream.stop();
    m_resMgr->Uninit();

    PIPE_DBG("CDP ::X");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
start()
{
//int path  = CAM_ISP_PASS2_START;

    PIPE_DBG(":E:pass[%d] +",this->m_pipePass);

    /*if ( EPipePass_PASS2 == this->m_pipePass ) {
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

    m_CamPathPass2.start((void*)&path);*/

    if (m_vBufImgi.size()>0)
    {
        unsigned int src_vir_addr_list[3];
        unsigned int src_phy_addr_list[3];
        unsigned int src_size_list[3];
        unsigned int src_plane_num;
        DpColorFormat src_fmt;
        DpRect src_crop;
        unsigned int src_y_stride;
        unsigned int src_uv_stride;

        configDpBufInfo(m_portInfo_imgi, m_vBufImgi[0].u4BufVA, src_fmt, src_vir_addr_list, src_size_list, src_plane_num, src_y_stride, src_uv_stride);
        configDpBufInfo(m_portInfo_imgi, m_vBufImgi[0].u4BufPA, src_fmt, src_phy_addr_list, src_size_list, src_plane_num, src_y_stride, src_uv_stride);
        src_crop.x = m_portInfo_imgi.crop.x;
        src_crop.y = m_portInfo_imgi.crop.y;
        src_crop.w = m_portInfo_imgi.crop.w;
        src_crop.h = m_portInfo_imgi.crop.h;   
	 
        if (m_vBufImgi[0].memID>0 && m_vBufImgi[0].u4BufPA==0)   // no mva address, but has ion id
        {
            m_dpStream.setSrcBuffer(m_vBufImgi[0].memID, src_size_list, src_plane_num);
        }
        else if (m_vBufImgi[0].memID>0 && (m_vBufImgi[0].u4BufPA == m_vBufImgi[0].u4BufVA))   // no mva address, but has ion id
        {
            m_dpStream.setSrcBuffer(m_vBufImgi[0].memID, src_size_list, src_plane_num);
        }
        else if (m_vBufImgi[0].u4BufPA == m_vBufImgi[0].u4BufVA)    // mav & va the same (record buffer)
        {
            m_dpStream.setSrcBuffer((void**)src_vir_addr_list, src_size_list, src_plane_num);
        } 
        else if (m_vBufImgi[0].u4BufPA!=0)    // has mva address
        {
            m_dpStream.setSrcBuffer((void**)src_vir_addr_list, (void**)src_phy_addr_list, src_size_list, src_plane_num);
        }
        else    // no mva address, no ion id
        {
            m_dpStream.setSrcBuffer((void**)src_vir_addr_list, src_size_list, src_plane_num);
        }
        m_dpStream.setSrcConfig(m_portInfo_imgi.u4ImgWidth, m_portInfo_imgi.u4ImgHeight, src_y_stride, src_uv_stride, src_fmt, DP_PROFILE_BT601, eInterlace_None, &src_crop);
        
        if (m_vBufDispo.size()>0)
        {
            unsigned int dst_vir_addr_list[3];
            unsigned int dst_phy_addr_list[3];
            unsigned int dst_size_list[3];
            unsigned int dst_plane_num;
            DpColorFormat dst_fmt;
            DpRect dst_crop;
            unsigned int dst_y_stride;
            unsigned int dst_uv_stride;

            configDpBufInfo(m_portInfo_dispo, m_vBufDispo[0].u4BufVA, dst_fmt, dst_vir_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
            configDpBufInfo(m_portInfo_dispo, m_vBufDispo[0].u4BufPA, dst_fmt, dst_phy_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
            dst_crop.x = m_portInfo_dispo.crop.x;
            dst_crop.y = m_portInfo_dispo.crop.y;
            dst_crop.w = m_portInfo_dispo.crop.w;
            dst_crop.h = m_portInfo_dispo.crop.h;

            if (m_vBufDispo[0].memID>0 && m_vBufDispo[0].u4BufPA==0)    // no mva address, but has ion id
            {
                m_dpStream.setDstBuffer(m_vBufDispo[0].memID, dst_size_list, dst_plane_num);
	     }
            else if (m_vBufDispo[0].memID>0 && (m_vBufDispo[0].u4BufPA == m_vBufDispo[0].u4BufVA))    // no mva address, but has ion id
            {
                m_dpStream.setDstBuffer(m_vBufDispo[0].memID, dst_size_list, dst_plane_num);
	     }
            else if (m_vBufDispo[0].u4BufPA == m_vBufDispo[0].u4BufVA)    // mav & va the same (record buffer)
            {
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num);
            } 
            else if (m_vBufDispo[0].u4BufPA!=0)    // has mva address
            {
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, (void**)dst_phy_addr_list, dst_size_list, dst_plane_num);
            }
            else    // no mva address, no ion id
            {
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num);
            }      	     
            m_dpStream.setDstConfig(m_portInfo_dispo.u4ImgWidth, m_portInfo_dispo.u4ImgHeight, dst_y_stride, dst_uv_stride, dst_fmt, DP_PROFILE_BT601, eInterlace_None, &dst_crop);
            m_dpStream.setRotate(0);
            m_dpStream.setFlip(0);
            m_dpStream.invalidate();    
        }
        
        if (m_vBufVido.size()>0)
        {
            unsigned int dst_vir_addr_list[3];
            unsigned int dst_phy_addr_list[3];
            unsigned int dst_size_list[3];
            unsigned int dst_plane_num;
            DpColorFormat dst_fmt;
            DpRect dst_crop;
            unsigned int dst_y_stride;
            unsigned int dst_uv_stride;

            if (m_vBufDispo.size()>0)
            {
                // reconfig src
                m_dpStream.setSrcBuffer((void**)src_vir_addr_list, (void**)src_phy_addr_list, src_size_list, src_plane_num);
                //m_dpStream.setSrcBuffer((void**)src_addr_list, src_size_list, src_plane_num);
                m_dpStream.setSrcConfig(m_portInfo_imgi.u4ImgWidth, m_portInfo_imgi.u4ImgHeight, src_y_stride, src_uv_stride, src_fmt, DP_PROFILE_BT601, eInterlace_None, &src_crop);
            }

            configDpBufInfo(m_portInfo_vido, m_vBufVido[0].u4BufVA, dst_fmt, dst_vir_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
            configDpBufInfo(m_portInfo_vido, m_vBufVido[0].u4BufPA, dst_fmt, dst_phy_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
            dst_crop.x = m_portInfo_vido.crop.x;
            dst_crop.y = m_portInfo_vido.crop.y;
            dst_crop.w = m_portInfo_vido.crop.w;
            dst_crop.h = m_portInfo_vido.crop.h;

            if (m_vBufVido[0].memID>0 && m_vBufVido[0].u4BufPA==0)    // no mva address, but has ion id
            {
                m_dpStream.setDstBuffer(m_vBufVido[0].memID, dst_size_list, dst_plane_num);
            }
            else if (m_vBufVido[0].memID>0 && (m_vBufVido[0].u4BufPA == m_vBufVido[0].u4BufVA))    // no mva address, but has ion id
            {
                m_dpStream.setDstBuffer(m_vBufVido[0].memID, dst_size_list, dst_plane_num);
            }
            else if (m_vBufVido[0].u4BufPA == m_vBufVido[0].u4BufVA)    // mav & va the same (record buffer)
            {
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num);
            }
            else if (m_vBufVido[0].u4BufPA!=0)    // has mva address
            {
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, (void**)dst_phy_addr_list, dst_size_list, dst_plane_num);
            }
            else    // no mva address, no ion id
            {
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num);
            }      	     	     
            m_dpStream.setDstConfig(m_portInfo_vido.u4ImgWidth, m_portInfo_vido.u4ImgHeight, dst_y_stride, dst_uv_stride, dst_fmt, DP_PROFILE_BT601, eInterlace_None, &dst_crop);
            m_dpStream.setRotate((int)m_portInfo_vido.eImgRot*90);
            m_dpStream.setFlip((bool)m_portInfo_vido.eImgFlip);
            m_dpStream.invalidate();
        }
    }

    if (MTRUE == m_isImgPlaneByImgi)
    {
        m_vBufImgi.clear();
        m_vBufDispo.clear();
        m_vBufVido.clear();
        m_isImgPlaneByImgi = MFALSE;
    }

    return  MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
syncJpegPass2C()
{
    PIPE_DBG("CDP :E");
#if 0
    m_resMgr->GetMode(&resMgrMode);
    if(resMgrMode.Dev==RES_MGR_DRV_DEV_CAM &&
        (resMgrMode.ScenHw==RES_MGR_DRV_SCEN_HW_VSS || resMgrMode.ScenHw==RES_MGR_DRV_SCEN_HW_ZSD)) {

        PIPE_DBG("EPIPEIRQ_SOF");

        irq(EPipePass_PASS1_TG1, EPIPEIRQ_SOF);
    }
#endif
    PIPE_DBG("CDP :X");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
startFmt()
{

//int path  = CAM_ISP_FMT_START;

    PIPE_DBG("CDP :E");
    #if 0
    m_CamPathPass2.ispTopCtrl.path = ISP_PASS2FMT;
    m_CamPathPass2.start((void*)&path);
    #endif
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
stop()
{
    PIPE_DBG("CDP :E");
#if 0
    m_CamPathPass2.stop(NULL);
#endif
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
    //stISP_BUF_INFO bufInfo;

    PIPE_DBG("CDP :E");
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    /*if (EPortIndex_IMGI == portID.index) {
        dmaChannel = ISP_DMA_IMGI;
    }
    else if (EPortIndex_VIPI == portID.index) {
        dmaChannel = ISP_DMA_VIPI;
    }
    else if (EPortIndex_VIP2I == portID.index) {
        dmaChannel = ISP_DMA_VIP2I;
    }*/
    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    //bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    //bufInfo.memID = rQBufInfo.vBufInfo[0].memID;
    //bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize;
    //bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu;
    //bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe;
    m_vBufImgi.push_back(rQBufInfo.vBufInfo[0]);

    /*
    if ( 0!= this->m_CamPathPass2.enqueueBuf( dmaChannel , bufInfo ) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }*/
    //


    PIPE_DBG("CDP :X");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    //MUINT32 dmaChannel = 0;
    //stISP_FILLED_BUF_LIST bufInfo;
    //ISP_BUF_INFO_L  bufList;

    PIPE_DBG("CDP :E ");
    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    //
    /*if (EPortIndex_IMGI == portID.index) {
        dmaChannel = ISP_DMA_IMGI;
    }
    else if (EPortIndex_VIPI == portID.index) {
        dmaChannel = ISP_DMA_VIPI;
    }
    else if (EPortIndex_VIP2I == portID.index) {
        dmaChannel = ISP_DMA_VIP2I;
    }*/
    //
    /*bufInfo.pBufList = &bufList;
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
    }*/
    rQBufInfo.vBufInfo.resize(m_vBufImgi.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID = m_vBufImgi[i].memID;
        rQBufInfo.vBufInfo[i].u4BufVA = m_vBufImgi[i].u4BufVA;
        rQBufInfo.vBufInfo[i].u4BufSize = m_vBufImgi[i].u4BufSize;
    }
    m_vBufImgi.resize(0);
#if 0
    vector< IhwScenario::PortQTBufInfo >::const_iterator it;
    bool isFindBuffer = false;
    for (it = in.begin(); it != in.end(); it++ ) {
        if ((*it).ePortIndex == eID_Pass1DispOut) {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[map] enque pass 1 out buffer");
            for (MUINT32 i = 0; i < (*it).bufInfo.vBufInfo.size(); i++)
            {
                IhwScenario::PortBufInfo one(eID_Pass1DispOut,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufVA,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufPA,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufSize,
                                  (*it).bufInfo.vBufInfo.at(i).memID);

                vEnBufPass1Out.push_back(one);
            };
#endif    
    //
    PIPE_DBG("CDP :X ");
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
    //stISP_BUF_INFO bufInfo;

    PIPE_DBG("CDP :E");
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    /*if (EPortIndex_DISPO == portID.index) {
        dmaChannel = ISP_DMA_DISPO;
    }
    else if (EPortIndex_VIDO == portID.index) {
        dmaChannel = ISP_DMA_VIDO;
    }
    else if (EPortIndex_FDO == portID.index) {
        dmaChannel = ISP_DMA_FDO;
    }*/
    if (EPortIndex_DISPO == portID.index)
    {
        m_vBufDispo.push_back(rQBufInfo.vBufInfo[0]);
    }
    else if (EPortIndex_VIDO == portID.index)
    {
        m_vBufVido.push_back(rQBufInfo.vBufInfo[0]);
    }
        
    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    //bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA;
    //bufInfo.memID = rQBufInfo.vBufInfo[0].memID;
    //bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize;
    //bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu;
    //bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe;
    //
    /*if ( 0 != this->m_CamPathPass2.enqueueBuf( dmaChannel, bufInfo ) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }*/

    PIPE_DBG("CDP :X");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    //MUINT32 dmaChannel = 0;
    //stISP_FILLED_BUF_LIST bufInfo;
    //ISP_BUF_INFO_L  bufList;

    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    //
    /*if (EPortIndex_DISPO == portID.index) {
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
    }*/
    //
    if (EPortIndex_DISPO == portID.index) {
        rQBufInfo.vBufInfo.resize(m_vBufDispo.size());
        for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
            rQBufInfo.vBufInfo[i].memID = m_vBufDispo[i].memID;
            rQBufInfo.vBufInfo[i].u4BufVA = m_vBufDispo[i].u4BufVA;
            rQBufInfo.vBufInfo[i].u4BufPA = m_vBufDispo[i].u4BufPA;
            rQBufInfo.vBufInfo[i].u4BufSize = m_vBufDispo[i].u4BufSize;
        }
        m_vBufDispo.resize(0);
    }
    else if (EPortIndex_VIDO == portID.index) {
        rQBufInfo.vBufInfo.resize(m_vBufVido.size());
        for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
            rQBufInfo.vBufInfo[i].memID = m_vBufVido[i].memID;
            rQBufInfo.vBufInfo[i].u4BufVA = m_vBufVido[i].u4BufVA;
            rQBufInfo.vBufInfo[i].u4BufPA = m_vBufVido[i].u4BufPA;
            rQBufInfo.vBufInfo[i].u4BufSize = m_vBufVido[i].u4BufSize;
        }
        m_vBufVido.resize(0);
    }

    return  MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
CdpPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
int ret = 0;
int idx_imgi = -1;
int idx_dispo = -1;
int idx_vido = -1;
    int imgi_format_en = 0, imgi_format = 0, imgi_bus_size_en=0, imgi_bus_size=0;

    EConfigSettingStage settingStage = m_settingStage;
    

    int pixel_byte_imgi = 1;
    //

    //
    PIPE_DBG("settingStage(%d) in(%d) / out(%d) ",settingStage, vInPorts.size(), vOutPorts.size());

    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) { continue; }
        //
        PIPE_INF("CDP : vInPorts:[%d]:(0x%x),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)",i,
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
            if (MTRUE == m_isImgPlaneByImgi)
            {
                m_vBufImgi.clear();
                mImgiBuf.memID = 0;//vInPorts[i]->memID;
                mImgiBuf.u4BufVA = vInPorts[i]->u4BufVA;
                mImgiBuf.u4BufPA = vInPorts[i]->u4BufPA;
                m_vBufImgi.push_back(mImgiBuf);
            }
            idx_imgi = i;
            m_portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];
        }
    }
    //
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) { continue; }
        //
        PIPE_INF("CDP : vOutPorts:[%d]:(0x%x),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)",i,
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
            if (MTRUE == m_isImgPlaneByImgi)
            {
                m_vBufDispo.clear();
                mDispoBuf.memID = 0;//vOutPorts[i]->memID;
                mDispoBuf.u4BufVA = vOutPorts[i]->u4BufVA;
                mDispoBuf.u4BufPA = vOutPorts[i]->u4BufPA;
                m_vBufDispo.push_back(mDispoBuf);
            }
            idx_dispo = i;
            m_portInfo_dispo =  (PortInfo)*vOutPorts[idx_dispo];
        }
        else if ( EPortIndex_VIDO == vOutPorts[i]->index ) {
            if (MTRUE == m_isImgPlaneByImgi)
            {
                m_vBufVido.clear();
                mVidoBuf.memID = 0;//vOutPorts[i]->memID;
                mVidoBuf.u4BufVA = vOutPorts[i]->u4BufVA;
                mVidoBuf.u4BufPA = vOutPorts[i]->u4BufPA;
                m_vBufVido.push_back(mVidoBuf);
            }
            idx_vido = i;
            m_portInfo_vido =  (PortInfo)*vOutPorts[idx_vido];
        }
    }
    //

    this->m_pipePass = (EPipePass)m_portInfo_imgi.pipePass;
    PIPE_DBG("CDP : this->m_pipePass:[%d]",this->m_pipePass);

    if (m_portInfo_imgi.u4IsRunSegment) {
    	MUINT32 *pSegNumVa = (MUINT32*)m_portInfo_imgi.u4SegNumVa;
    	*pSegNumVa = 1;	// not support tpipe in MT6572
    	PIPE_INF("CDP : m_portInfo_imgi.u4IsRunSegment:[%d]",m_portInfo_imgi.u4IsRunSegment);
    	PIPE_INF("CDP : Not support tpipe! Set m_portInfo_imgi.u4SegNumVa:[%d]",*((MUINT32*)m_portInfo_imgi.u4SegNumVa));
    }

    //
    PIPE_DBG("meScenarioFmt:[%d]",meScenarioFmt);

    //should be before scenario parsing
    PIPE_DBG("m_portInfo_imgi.eImgFmt:[0x%x]",m_portInfo_imgi.eImgFmt);
    switch( m_portInfo_imgi.eImgFmt ) {
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane(YVU)
        case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV16:      //= 0x08000,   //422 format, 2 plane (UV)
        case eImgFmt_NV61:      //= 0x10000,   //422 format, 2 plane (VU)
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
        case eImgFmt_ARGB888:   //= 0x1000,   //ARGB (32-bit), 1 plane
        case eImgFmt_Y800:		//= 0x040000, //Y plane only
            break;
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit
        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        default:
            PIPE_ERR("m_portInfo_imgi.eImgFmt:Format NOT Support");
            return MFALSE;
    }

    //
    PIPE_DBG("meScenarioID:[%d]",meScenarioID);

    switch (meScenarioID) {
        case eScenarioID_VR:         //  Video Recording/Preview
        case eScenarioID_GDMA:       // for GDMA only
        case eScenarioID_VEC:        //  Vector Generation
        case eScenarioID_ZSD:        //  Zero Shutter Delay
        case eScenarioID_VSS_CDP_CC:
        case eScenarioID_VSS:
        case eScenarioID_N3D_IC:
        case eScenarioID_ZSD_CDP_CC:
            break;
        default:
            PIPE_ERR("NOT Support scenario");
            return MFALSE;
    }

    if (-1 != idx_imgi ) {
        PIPE_DBG("config imgi[%d, %d] imgiCrop_f(0x%x, 0x%x)[%d, %d, %d, %d]\n",m_portInfo_imgi.u4ImgWidth,m_portInfo_imgi.u4ImgHeight, \
                m_portInfo_imgi.crop.floatX,m_portInfo_imgi.crop.floatY, m_portInfo_imgi.crop.x,m_portInfo_imgi.crop.y, \
                m_portInfo_imgi.crop.w,m_portInfo_imgi.crop.h);

            switch( m_portInfo_imgi.eImgFmt ) {
                case eImgFmt_NV16:      //= 0x8000,   //422 format, 2 plane
                case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
                case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
                case eImgFmt_YV12:      //= 0x00008,    //420 format, 3 plane (YVU)
                case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
                case eImgFmt_YV16:      //= 0x4000,   //422 format, 3 plane
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

    if ( -1 != idx_dispo) {
        PIPE_DBG("config dispo");
    }

    if ( -1 != idx_vido) {
        PIPE_DBG("config vido");
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
    //m_isPartialUpdate = MTRUE;
    //
    ret = configPipe(vInPorts,vOutPorts);
    //
    //m_isPartialUpdate = MFALSE;
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
//    MINT32 type = 0;
//    MUINT32 irq = 0;

    PIPE_DBG("tid(%d) (type,irq)=(0x%08x,0x%08x)", gettid(), pass, irq_int);
#if 0
    //irq_int
    if ( EPIPEIRQ_PATH_DONE != irq_int || EPIPEIRQ_VSYNC != irq_int) {
    }
    else {
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
    else if ( EPipePass_PASS1_TG1 == pass ) {  // for jpeg ring (jpeg will get isp information from pipe mgr)
        type = ISP_DRV_IRQ_TYPE_INT;
        irq = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    }
    else {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        return MFALSE;
    }
#endif
    //
    //PIPE_DBG("(type,irq)=(0x%08x,0x%08x)", type, irq);
    //
    //ret = m_CamPathPass2.waitIrq(type,irq);

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
            /*
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
            */
            //
            //m_CamPathPass2.setDMACurrBuf((MUINT32) dmaChannel);
            break;
        case EPIPECmd_SET_NEXT_BUFFER:
            /*
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
            */
            //
            //m_CamPathPass2.setDMANextBuf((MUINT32) dmaChannel);
            break;
        case EPIPECmd_SET_CQ_CHANNEL:
            //m_pass2_CQ = arg1;//CAM_ISP_CQ0
            //m_CamPathPass2.CQ = m_pass2_CQ;
            break;
        case EPIPECmd_SET_CONFIG_STAGE:
            m_settingStage = (EConfigSettingStage)arg1;
            break;
        case EPIPECmd_SET_FMT_START:
            /*if ( arg1 ) {
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
            }*/
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
            /*if ( arg1)  {
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
            }*/
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
            //Use this flag to do Video snapshop!!!
            m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
            //GDMA_EN
            /*if ( arg1)  {
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
            }*/
            break;
        case EPIPECmd_SET_CAM_CTL_DBG:
            /*if(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(    m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv(),
                                        m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->getRegAddrMap(),
                                        CAM_CTL_DBG_SET,arg1);
            }
            else
            {
                ISP_WRITE_REG(m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspReg(),CAM_CTL_DBG_SET,arg1);
            }*/
            break;
        case EPIPECmd_GET_CAM_CTL_DBG:
            /*if (arg1) {
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
            }*/
            break;
        case EPIPECmd_SET_IMG_PLANE_BY_IMGI:
            //m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_ISP_RESET:
            //PIPE_INF("EPIPECmd_ISP_RESET");
            //m_CamPathPass2.ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->reset();
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

