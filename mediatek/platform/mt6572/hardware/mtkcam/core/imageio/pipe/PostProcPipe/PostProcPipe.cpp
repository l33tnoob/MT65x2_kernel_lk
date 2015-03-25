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

#include <DpMultiStream.h> // Fro DpFramework


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""


//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

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
PostProcPipe::
PostProcPipe(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EScenarioID const eScenarioID,
    EScenarioFmt const eScenarioFmt
)
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt),
      //m_pIspDrvShell(NULL),
      m_pipePass(EPipePass_PASS2)
      //m_pass2_CQ(CAM_ISP_CQ_NONE),
      //m_isImgPlaneByImgi(MFALSE)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    //memset(&this->m_camPass2Param,0x00,sizeof(CamPathPass1Parameter));
    this->m_vBufImgi.resize(0);
    this->m_vBufDispo.resize(0);
    this->m_vBufVido.resize(0);

    /*** create isp driver ***/
    //m_pIspDrvShell = IspDrvShell::createInstance();
}

PostProcPipe::
~PostProcPipe()
{
    /*** release isp driver ***/
    //m_pIspDrvShell->destroyInstance();
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
    //if ( m_pIspDrvShell ) {
    //    m_pIspDrvShell->init();
    //m_pIspDrvShell->getPhyIspDrv()->GlobalPipeCountInc();

//js_test, to reset register value
//m_pIspDrvShell->getPhyIspDrv()->reset();

        //
     //   m_CamPathPass2.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
    //}

    // alloc tpipe table
#if 0
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
#endif
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
#if 0
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
#endif
    m_dpStream.stop();
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
start()
{
    //int path  = CAM_ISP_PASS2_START;


    PIPE_DBG(":E:pass[%d] +",this->m_pipePass);
#if 0
    if ( EPipePass_PASS2 == this->m_pipePass ) {
        path  = CAM_ISP_PASS2_START;
    }
    else if ( EPipePass_PASS2B == this->m_pipePass ) {
        path  = CAM_ISP_PASS2B_START;
    }
    else if ( EPipePass_PASS2C == this->m_pipePass ) {
        path  = CAM_ISP_PASS2C_START;
    }

    m_CamPathPass2.start((void*)&path);
 #endif
    if (m_vBufImgi.size()>0)
    {
        unsigned int src_vir_addr_list[3];
        unsigned int src_phy_addr_list[3];
        unsigned int src_size_list[3];
        unsigned int dst_vir_addr_list[3];
        unsigned int dst_phy_addr_list[3];
        unsigned int dst_size_list[3];
        unsigned int src_plane_num, dst_plane_num;
        DpColorFormat src_fmt, dst_fmt;
        DpRect src_crop, dst_crop;
        unsigned int port_id = 0;
        unsigned int src_y_stride;
        unsigned int src_uv_stride;
        unsigned int dst_y_stride;
        unsigned int dst_uv_stride;

        configDpBufInfo(m_portInfo_imgi, m_vBufImgi[0].u4BufVA, src_fmt, src_vir_addr_list, src_size_list, src_plane_num, src_y_stride, src_uv_stride);
        configDpBufInfo(m_portInfo_imgi, m_vBufImgi[0].u4BufPA, src_fmt, src_phy_addr_list, src_size_list, src_plane_num, src_y_stride, src_uv_stride);
        src_crop.x = m_portInfo_imgi.crop.x;
	 src_crop.y = m_portInfo_imgi.crop.y;
	 src_crop.w = m_portInfo_imgi.crop.w;
	 src_crop.h = m_portInfo_imgi.crop.h;

	 PIPE_INF("m_vBufImgi: memID=%d, u4BufPA=0x%x, u4BufVA=0x%x\n",  m_vBufImgi[0].memID, m_vBufImgi[0].u4BufPA, m_vBufImgi[0].u4BufVA);

	 if (m_vBufImgi[0].memID>0 && m_vBufImgi[0].u4BufPA==0)   // no mva address, but has ion id
	 {
	    PIPE_INF("m_vBufImgi: no mva address, but has ion id (use ion) \n");
	     m_dpStream.setSrcBuffer(m_vBufImgi[0].memID, src_size_list, src_plane_num);
        }
	 else if (m_vBufImgi[0].memID>0 && (m_vBufImgi[0].u4BufPA == m_vBufImgi[0].u4BufVA))   // no mva address, but has ion id
	 {
	    PIPE_INF("m_vBufImgi: has ion id, va & mva address the same (use ion) \n");
	     m_dpStream.setSrcBuffer(m_vBufImgi[0].memID, src_size_list, src_plane_num);
        }
	 else if (m_vBufImgi[0].u4BufPA == m_vBufImgi[0].u4BufVA) // va & mva the same (record buffer)
	 {
	     PIPE_INF("m_vBufImgi: no ion id, va & mva address the same (use va) \n");
            m_dpStream.setSrcBuffer((void**)src_vir_addr_list, src_size_list, src_plane_num);
	 }
	 else if (m_vBufImgi[0].u4BufPA!=0)    // has mva address
	 {
	     PIPE_INF("m_vBufImgi: has mva address (use mva) \n");
	     m_dpStream.setSrcBuffer((void**)src_vir_addr_list, (void**)src_phy_addr_list, src_size_list, src_plane_num);
        }
        else    // no mva address, no ion id
        {
            PIPE_INF("m_vBufImgi: no mva address, no ion id (use va) \n");
            m_dpStream.setSrcBuffer((void**)src_vir_addr_list, src_size_list, src_plane_num);
        }
        m_dpStream.setSrcConfig(m_portInfo_imgi.u4ImgWidth, m_portInfo_imgi.u4ImgHeight, src_y_stride, src_uv_stride, src_fmt, DP_PROFILE_BT601, &src_crop);
        
        if (m_vBufDispo.size()>0)
        {
            configDpBufInfo(m_portInfo_dispo, m_vBufDispo[0].u4BufVA, dst_fmt, dst_vir_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
	     configDpBufInfo(m_portInfo_dispo, m_vBufDispo[0].u4BufPA, dst_fmt, dst_phy_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
	     dst_crop.x = m_portInfo_dispo.crop.x;
	     dst_crop.y = m_portInfo_dispo.crop.y;
	     dst_crop.w = m_portInfo_dispo.crop.w;
	     dst_crop.h = m_portInfo_dispo.crop.h;

	     PIPE_INF("m_vBufDispo: memID=%d, u4BufPA=0x%x, u4BufVA=0x%x\n",  m_vBufDispo[0].memID, m_vBufDispo[0].u4BufPA, m_vBufDispo[0].u4BufVA);


            if (m_vBufDispo[0].memID>0 && m_vBufDispo[0].u4BufPA==0)    // no mva address, but has ion id
            {
                PIPE_INF("m_vBufDispo: no mva address, but has ion id (use ion) \n");
	         m_dpStream.setDstBuffer(m_vBufDispo[0].memID, dst_size_list, dst_plane_num, port_id);
	     }
            else if (m_vBufDispo[0].memID>0 && (m_vBufDispo[0].u4BufPA == m_vBufDispo[0].u4BufVA)) // va & mva the same (record buffer)
            {
                PIPE_INF("m_vBufDispo: has ion id, va & mva address the same (use ion) \n");
                m_dpStream.setDstBuffer(m_vBufDispo[0].memID, dst_size_list, dst_plane_num, port_id);
            }
            else if (m_vBufDispo[0].u4BufPA == m_vBufDispo[0].u4BufVA) // va & mva the same (record buffer)
            {
                PIPE_INF("m_vBufDispo: no ion id, va & mva address the same (use va) \n");
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num, port_id);
            }
            else if (m_vBufDispo[0].u4BufPA!=0)    // has mva address
            {
               PIPE_INF("m_vBufDispo: has mva address (use mva) \n");
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, (void**)dst_phy_addr_list, dst_size_list, dst_plane_num, port_id);
            }
            else    // no mva address, no ion id
            {
                PIPE_INF("m_vBufDispo: no mva address, no ion id (use va) \n");
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num, port_id);
            }
	     m_dpStream.setDstConfig(m_portInfo_dispo.u4ImgWidth, m_portInfo_dispo.u4ImgHeight, dst_y_stride, dst_uv_stride, dst_fmt, DP_PROFILE_BT601, &dst_crop, port_id);
#if (PLATFORM_VERSION_MAJOR == 2)
            m_dpStream.setRotate((int)m_portInfo_dispo.eImgRot*90, port_id);
            m_dpStream.setFlip((bool)m_portInfo_dispo.eImgFlip, port_id);
#else
            m_dpStream.setRotate(0, port_id);
            m_dpStream.setFlip(0, port_id);
#endif
            port_id++;
        }
        
        if (m_vBufVido.size()>0)
        {
            configDpBufInfo(m_portInfo_vido, m_vBufVido[0].u4BufVA, dst_fmt, dst_vir_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
	     configDpBufInfo(m_portInfo_vido, m_vBufVido[0].u4BufPA, dst_fmt, dst_phy_addr_list, dst_size_list, dst_plane_num, dst_y_stride, dst_uv_stride);
	     dst_crop.x = m_portInfo_vido.crop.x;
	     dst_crop.y = m_portInfo_vido.crop.y;
	     dst_crop.w = m_portInfo_vido.crop.w;
	     dst_crop.h = m_portInfo_vido.crop.h;

	     PIPE_INF("m_vBufVido: memID=%d, u4BufPA=0x%x, u4BufVA=0x%x\n",  m_vBufVido[0].memID, m_vBufVido[0].u4BufPA, m_vBufVido[0].u4BufVA);


            if (m_vBufVido[0].memID>0 && m_vBufVido[0].u4BufPA==0)    // no mva address, but has ion id
            {
                PIPE_INF("m_vBufVido: no mva address, but has ion id (use ion) \n");
                m_dpStream.setDstBuffer(m_vBufVido[0].memID, dst_size_list, dst_plane_num, port_id);
	     }
            else if (m_vBufVido[0].memID>0 && (m_vBufVido[0].u4BufPA == m_vBufVido[0].u4BufVA))    // no mva address, but has ion id
            {
                PIPE_INF("m_vBufVido: has ion id, va & mva address the same (use ion) \n");
                m_dpStream.setDstBuffer(m_vBufVido[0].memID, dst_size_list, dst_plane_num, port_id);
	     }
            else if (m_vBufVido[0].u4BufPA == m_vBufVido[0].u4BufVA) // va & mva the same (record buffer)
            {
                PIPE_INF("m_vBufVido: no ion id, va & mva address the same (use va) \n");
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num, port_id);
            }
            else if (m_vBufVido[0].u4BufPA!=0)    // has mva address
            {
                PIPE_INF("m_vBufVido: has mva address (use mva) \n");
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, (void**)dst_phy_addr_list, dst_size_list, dst_plane_num, port_id);
            }
            else    // no mva address, no ion id
            {
                PIPE_INF("m_vBufVido: no mva address, no ion id (use va) \n");
                m_dpStream.setDstBuffer((void**)dst_vir_addr_list, dst_size_list, dst_plane_num, port_id);
            }          
            m_dpStream.setDstConfig(m_portInfo_vido.u4ImgWidth, m_portInfo_vido.u4ImgHeight, dst_y_stride, dst_uv_stride, dst_fmt, DP_PROFILE_BT601, &dst_crop, port_id);
            m_dpStream.setRotate((int)m_portInfo_vido.eImgRot*90, port_id);
            m_dpStream.setFlip((bool)m_portInfo_vido.eImgFlip, port_id);
        }

	 if (m_vBufDispo.size()>0 || m_vBufVido.size()>0)
        {
            m_dpStream.invalidate();   
        }
    }
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
#if 0
    m_CamPathPass2.stop(NULL);
#endif
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
    //stISP_BUF_INFO bufInfo;

    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
#if 0
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
 #endif
    m_vBufImgi.push_back(rQBufInfo.vBufInfo[0]);

    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
#if 0
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
#endif

    rQBufInfo.vBufInfo.resize(m_vBufImgi.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID = m_vBufImgi[i].memID;
        rQBufInfo.vBufInfo[i].u4BufVA = m_vBufImgi[i].u4BufVA;
        rQBufInfo.vBufInfo[i].u4BufSize = m_vBufImgi[i].u4BufSize;
    }
    m_vBufImgi.resize(0);
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
    //MUINT32 dmaChannel = 0;
    //stISP_BUF_INFO bufInfo;

    PIPE_DBG(":E");
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
#if 0
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
#endif
    if (EPortIndex_DISPO == portID.index)
    {
        m_vBufDispo.push_back(rQBufInfo.vBufInfo[0]);
    }
    else if (EPortIndex_VIDO == portID.index)
    {
        m_vBufVido.push_back(rQBufInfo.vBufInfo[0]);
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

   // MUINT32 dmaChannel = 0;
   // stISP_FILLED_BUF_LIST bufInfo;
  //  ISP_BUF_INFO_L  bufList;

    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
#if 0
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
        rQBufInfo.vBufInfo.at(i).u4BufPA = bufList.front().base_pAddr;
        rQBufInfo.vBufInfo[i].u4BufSize = bufList.front().size;
        rQBufInfo.vBufInfo[i].i4TimeStamp_sec = bufList.front().timeStampS;
        rQBufInfo.vBufInfo[i].i4TimeStamp_us = bufList.front().timeStampUs;
        bufList.pop_front();
    }
#endif
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
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
int ret = 0;
int idx_imgi = -1;
int idx_dispo = -1;
int idx_vido = -1;
    EConfigSettingStage settingStage = m_settingStage;
    int pixel_byte_imgi = 1;



    PIPE_DBG("settingStage(%d) in[%d]/out[%d]", settingStage, vInPorts.size(), vOutPorts.size());

    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) { continue; }
        //
        PIPE_INF("vInPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d)",
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
        if ( EPortIndex_IMGI == vInPorts[i]->index ) {
            idx_imgi = i;
            m_portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];
        }

    }
    //
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) { continue; }
        //
        PIPE_INF("vOutPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d)",i,
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
            m_portInfo_dispo =  (PortInfo)*vOutPorts[idx_dispo];
        }
        else if ( EPortIndex_VIDO == vOutPorts[i]->index ) {
            idx_vido = i;
            m_portInfo_vido =  (PortInfo)*vOutPorts[idx_vido];
        }
    }
    //
    this->m_pipePass = (EPipePass)m_portInfo_imgi.pipePass;
    PIPE_DBG("this->m_pipePass:[%d]",this->m_pipePass);

    if (m_portInfo_imgi.u4IsRunSegment) {
    	MUINT32 *pSegNumVa = (MUINT32*)m_portInfo_imgi.u4SegNumVa;
    	*pSegNumVa = 1;	// not support tpipe in MT6572
    	PIPE_INF("m_portInfo_imgi.u4IsRunSegment:[%d]",m_portInfo_imgi.u4IsRunSegment);
    	PIPE_INF("Not support tpipe! Set m_portInfo_imgi.u4SegNumVa:[%d]",*((MUINT32*)m_portInfo_imgi.u4SegNumVa));
    }

    //
    PIPE_DBG("meScenarioFmt:[%d]",meScenarioFmt);

    //should be before scenario parsing
    PIPE_DBG("m_portInfo_imgi.eImgFmt:[%d]",m_portInfo_imgi.eImgFmt);
    switch( m_portInfo_imgi.eImgFmt ) {
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV16:      //422 format, 2 plane
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
        case eImgFmt_ARGB888:   //= 0x1000,   //ARGB (32-bit), 1 plane
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
        case eScenarioID_N3D_VR:     //  Native Stereo Camera VR
        case eScenarioID_IC:         //  Image Capture
        case eScenarioID_N3D_IC:     //  Native Stereo Camera IC
        case eScenarioID_VSS:       //  video snap shot
        case eScenarioID_IP:
            break;
        default:
            PIPE_ERR("NOT Support scenario");
            return MFALSE;
    }

    //
    if ( -1 != idx_imgi ) {
        PIPE_INF("config imgi[%d, %d] imgiCrop_f(0x%x, 0x%x)[%d, %d, %d, %d]\n",m_portInfo_imgi.u4ImgWidth,m_portInfo_imgi.u4ImgHeight, \
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
#if 0
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
#endif
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
            #if 0
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
            //for MFB
            if ( EPortIndex_IMGCI == arg1 ) {
                dmaChannel = ISP_DMA_IMGCI;
            }
            if ( EPortIndex_LSCI == arg1 ) {
                dmaChannel = ISP_DMA_LSCI;
            }
            if ( EPortIndex_LCEI == arg1 ) {
                dmaChannel = ISP_DMA_LCEI;
            }
            if ( EPortIndex_IMGO == arg1 ) {
                dmaChannel = ISP_DMA_IMGO;
            }

            //
            m_CamPathPass2.setDMACurrBuf((MUINT32) dmaChannel);
            #endif
            break;
        case EPIPECmd_SET_NEXT_BUFFER:
            #if 0
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
            #endif
            break;
        case EPIPECmd_SET_CQ_CHANNEL:
            #if 0
            m_pass2_CQ = arg1;//CAM_ISP_CQ0
            m_CamPathPass2.CQ = m_pass2_CQ;
            #endif
            break;
        case EPIPECmd_SET_CONFIG_STAGE:
            m_settingStage = (EConfigSettingStage)arg1;
            break;
        case EPIPECmd_FREE_MAPPED_BUFFER:
            #if 0
            {
                stISP_BUF_INFO buf_info = (stISP_BUF_INFO)(*(stISP_BUF_INFO*)arg2);
                m_CamPathPass2.freePhyBuf(arg1,buf_info);
            }
            #endif
            break;
        case EPIPECmd_SET_IMG_PLANE_BY_IMGI:
            //m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
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

