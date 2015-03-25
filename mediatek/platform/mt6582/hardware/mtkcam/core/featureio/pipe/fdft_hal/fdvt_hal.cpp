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
 /*
** $Log: fdvt_hal.cpp $
 *
*/
#define LOG_TAG "mHalFDVT"

#define FDVT_DDP_SUPPORT

#include <utils/Errors.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/atomic.h>
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>
#include <mtkcam/common/faces.h>
//#include "MediaLog.h"
//#include "MediaAssert.h"
#include "fdvt_hal.h"
#include <mtkcam/Log.h>

#include <mtkcam/algorithm/libfdft/MTKDetection.h>
#include "camera_custom_fd.h"

#ifdef FDVT_DDP_SUPPORT
#include <DpBlitStream.h>
#endif

#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>

#define DUMP_IMAGE (0)

#define FD_SCALES 11
static const MUINT32 image_input_width_vga = 640;
static const MUINT32 image_input_height_vga = 480;
static const MUINT32 image_input_height_buffer = 640;
static MUINT32 image_width_array[FD_SCALES] = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 32};
static MUINT32 image_height_array[FD_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 24};
static MUINT32 ImageScaleTotalSize = 0;
static MUINT8 *ImageScaleBuffer;
static MBOOL EnableSWResizerFlag;
static MUINT8 *ImageVGABuffer;
static bool g_input_vga = 0;
static bool g_input_4_3 = 0;
static bool g_input_16_9 = 0; 
static void Create640WidthImage_Y(MUINT8* src_img, MUINT32 src_w, MUINT32 src_h, MUINT8* dst_img);
static void CreateScaleImagesFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray);
static void CreateScaleImagesFromPreview_Y(MUINT8* image_buffer_preview, MUINT8* image_buffer, MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray);

#define MHAL_NO_ERROR 0
#define MHAL_INPUT_SIZE_ERROR 1

#define MAX_FACE_NUM 15

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//
//
static halFDBase *pHalFD = NULL;

static MINT32 FdResult_Num;
static FD_RESULT FdResult[15];

MUINT32 g_Lcm_rotate = 0;
MUINT32 g_Sensor_rotate = 0;
MUINT32 g_CameraTYPE = 0;

MUINT32 g_FDW = 0;
MUINT32 g_FDH = 0;

volatile MINT32     mFDCount = 0;

unsigned char         g_BufferGroup = 0;
unsigned char         g_ucPlane=2;
unsigned short int  g_uwDst_width =320;
unsigned short int  g_uwDst_height = 240;


/*******************************************************************************
*
********************************************************************************/
halFDBase*
halFDVT::
getInstance()
{
    MY_LOGD("[Create] Before:%d,", mFDCount);
    //MHAL_LOG("[halFDVT] getInstance \n");
    if (pHalFD == NULL) {
        pHalFD = new halFDVT();
    }
    
    android_atomic_inc(&mFDCount);
    
    MY_LOGD("[Create] After:%d,", mFDCount);
    
    return pHalFD;
}

/*******************************************************************************
*
********************************************************************************/
void   
halFDVT::
destroyInstance() 
{
     MY_LOGD("[Delete] Before:%d,", mFDCount);
    
    // More than one user
    if (mFDCount > 0)
          android_atomic_dec(&mFDCount);
    
   if (mFDCount == 0) 
   {
        if (pHalFD) 
        {
            delete pHalFD;
        }
        pHalFD = NULL;
   } 
   
   MY_LOGD("[Delete] After:%d,", mFDCount);
}

/*******************************************************************************
*                                            
********************************************************************************/
halFDVT::halFDVT()
{
    m_pMTKFDVTObj = NULL;

    m_FDW = 0; 
    m_FDH = 0; 
    m_DispW = 0; 
    m_DispH = 0; 
    m_DispX = 0; 
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;
    mFDCount = 0;

    m_pMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_FDFT_SW);

    //MHAL_LOG("m_pMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_FDFT_SW)\n"); 	
}


halFDVT::~halFDVT()
{
    m_FDW = 0; 
    m_FDH = 0; 
    m_DispW = 0; 
    m_DispH = 0; 
    m_DispX = 0; 
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;   
    
    if (m_pMTKFDVTObj) {
        m_pMTKFDVTObj->destroyInstance();
    }
    m_pMTKFDVTObj = NULL;      
}


MINT32
halFDVT::halFDInit(
    MUINT32 fdW,
    MUINT32 fdH,
    MUINT32 WorkingBuffer,
    MUINT32 WorkingBufferSize,
    MBOOL   SWResizerEnable
)
{
    MUINT32 i;
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* Cdata=NULL;
    MTKFDFTInitInfo FDFTInitInfo;
    FD_Customize_PARA FDCustomData;
    //MHAL_LOG("[mHalFDInit] Start \n");
    g_FDW = fdW;
    g_FDH = fdH;
    m_FDW = fdW;
    m_FDH = fdH;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;
    
    //m_FDW = 640;
    //m_FDH = 480;
    
    if(m_FDW == image_input_width_vga && m_FDH == image_input_height_vga)
    {
        g_input_vga = 1;
        g_input_4_3 = 0;
        g_input_16_9 = 0;
    }
    else if( m_FDW*3 == m_FDH*4)
    {
        g_input_vga = 0;
        g_input_4_3 = 1;
        g_input_16_9 = 0;
    		
    }
    //else if( m_FDW*9 == m_FDH*16)
    else if( (m_FDW*9 == m_FDH*16) || (m_FDW*3 == m_FDH*5))
    {
        g_input_vga = 0;
        g_input_4_3 = 0;
        g_input_16_9 = 1;
    }
    else
    {
        g_input_vga = 0;
        g_input_4_3 = 0;
        g_input_16_9 = 1;
    }
    //MHAL_ASSERT(m_pMTKFDVTObj != NULL, "m_pMTKFDVTObj is NULL");

    get_fd_CustomizeData(&FDCustomData);

    FDFTInitInfo.WorkingBufAddr = WorkingBuffer;
    FDFTInitInfo.WorkingBufSize = WorkingBufferSize;
    FDFTInitInfo.FDThreadNum = FDCustomData.FDThreadNum;
    FDFTInitInfo.FDThreshold = FDCustomData.FDThreshold;
    FDFTInitInfo.MajorFaceDecision = FDCustomData.MajorFaceDecision;
    FDFTInitInfo.OTRatio = FDCustomData.OTRatio;
    FDFTInitInfo.SmoothLevel = FDCustomData.SmoothLevel;
    FDFTInitInfo.FDSkipStep = FDCustomData.FDSkipStep;
    FDFTInitInfo.FDRectify = FDCustomData.FDRectify;
    FDFTInitInfo.FDRefresh = FDCustomData.FDRefresh;
    FDFTInitInfo.FDBufWidth = fdW;
    FDFTInitInfo.FDBufHeight = fdH;
    FDFTInitInfo.SDThreshold = FDCustomData.SDThreshold;
    FDFTInitInfo.SDMainFaceMust = FDCustomData.SDMainFaceMust;
    FDFTInitInfo.GSensor = FDCustomData.GSensor;
    //MY_LOGD("Customer Data: 1:%d, 2:%d, 3:%d, 4:%d, 5:%d, 6:%d, 7:%d", FDFTInitInfo.FDThreadNum, FDFTInitInfo.FDThreshold,  FDFTInitInfo.MajorFaceDecision,
    //                                                        FDFTInitInfo.OTRatio, FDFTInitInfo.SmoothLevel, FDFTInitInfo.FDSkipStep, FDFTInitInfo.FDRefresh);
    
    m_pMTKFDVTObj->FDVTInit(&FDFTInitInfo);
    
    EnableSWResizerFlag = SWResizerEnable;
    if(EnableSWResizerFlag)
    {
        ImageScaleTotalSize = 0;
        for(i=0; i<FD_SCALES;i++)
        {
            ImageScaleTotalSize += image_width_array[i]*image_height_array[i];
        }
        ImageScaleBuffer = new unsigned char[ImageScaleTotalSize];
    }
    
    if(g_input_vga == 0)
    {
        ImageVGABuffer = new unsigned char[image_input_width_vga*image_input_height_buffer];
        memset(ImageVGABuffer, 0, image_input_width_vga*image_input_height_buffer);
    }
        
    return err;
}

bool doRGB565Buffer_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr)
{
     bool ret = true;
    
    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;
    
    unsigned char* pucYBuf;
    unsigned char* pucUVBuf;
    unsigned int i, j, k;
    int Y[4], U, V, R[4], G[4], B[4], r, g, b;
    int dImgW = 640;
    int dImgH = 480;
    
    pucYBuf = src_yp;
    pucUVBuf = src_yp+dImgW * dImgH;
    
    for(i=0;i<dImgH;i=i+2)
    {
	for(j=0;j<dImgW;j=j+2)
	{
			Y[0] = *(pucYBuf + ((i+0) * dImgW) + j);
			Y[1] = *(pucYBuf + ((i+0) * dImgW) + j+1) ;
			Y[2] = *(pucYBuf + ((i+1) * dImgW) + j) ;
			Y[3] = *(pucYBuf + ((i+1) * dImgW) + j+1) ;
			
			Y [0]= (Y[0]+Y[1]+Y[2]+Y[3]) >> 2;
			V  =  *(pucUVBuf + ((i >> 1) * dImgW) + j );
			U  =  *(pucUVBuf + ((i >> 1) * dImgW) + j +1);
	    
			for(k=0;k<1;k++)
			{
				r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
				g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
				b = (32 * Y[k] + 57 * (U-128) + 16) / 32;
				
				R[k] = (r<0) ? 0: (r>255) ? 255 : r;
				G[k]= (g<0) ? 0: (g>255) ? 255 : g;
				B[k] = (b<0) ? 0: (b>255) ? 255 : b;
			}
			
		          *(dst_rgb + ((i>>1)  * dImgW ) + j+0) = ((G[0] & 0x1C) <<3 ) + ((B[0] & 0xF8) >>3 );
			*(dst_rgb + ((i>>1)  * dImgW ) + j+1) = ((G[0] & 0xE0) >>5 ) + ((R[0] & 0xF8));
		}
	}

        return ret;
}

bool doRGB565Buffer_DDP(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr)
{
    bool ret = true;

#ifdef FDVT_DDP_SUPPORT
    DpBlitStream FDstream;
    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;
    unsigned int src_addr_list[3];
    unsigned int src_size_list[3];
    unsigned int dst_addr_list[3];
    unsigned int dst_size_list[3];
    
    int src_ysize = g_FDW * g_FDH;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;
  
    //*****************************************************************************// 
    //*******************src  NV21/YV12 **************************************//
    //*****************************************************************************//
    if(g_ucPlane ==2)
    {
        src_addr_list[0] = (unsigned int)src_yp;
        src_addr_list[1] = (unsigned int)(src_yp + src_ysize);
        src_size_list[0] = src_ysize;
        src_size_list[1] = src_usize + src_vsize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, g_ucPlane);
        //FDstream.setSrcConfig(g_FDW, g_FDH, eYUV_420_2P_YVYU); //89
        FDstream.setSrcConfig(g_FDW, g_FDH, DP_COLOR_NV21); //82&72
    }
     
    else if(g_ucPlane ==3)
    {
        src_addr_list[0] = (unsigned int)src_yp;
        src_addr_list[1] = (unsigned int)(src_yp + src_ysize);
        src_addr_list[2] = (unsigned int)(src_yp + src_ysize * 5/4);

        src_size_list[0] = src_ysize;
        src_size_list[1] = src_vsize;
        src_size_list[2] = src_usize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, g_ucPlane);
        //FDstream.setSrcConfig(g_FDW, g_FDH, eYUV_420_3P_YVU); //89
        FDstream.setSrcConfig(g_FDW, g_FDH, DP_COLOR_YV12); //82&72
    }   

    //***************************dst RGB565********************************//
    FDstream.setDstBuffer((void *)dst_rgb, g_uwDst_width*g_uwDst_height*2); // 320*240*2
    //FDstream.setDstConfig(g_uwDst_width, g_uwDst_height, g_uwDst_width * 2, eRGB565);
    FDstream.setDstConfig(g_uwDst_width, g_uwDst_height, eRGB565);
    FDstream.setRotate(0);

    //*****************************************************************************//    
    
    //MY_LOGD("Pipe_DDP_Performance_RGB565 Start");   
    
//***********************************************************************//
//Adjust FD thread priority to RR
//***********************************************************************//
    //int const policy    = SCHED_OTHER;
#if MTKCAM_HAVE_RR_PRIORITY
    int policy    = SCHED_RR;
    int priority  = PRIO_RT_FD_THREAD;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
//***********************************************************************//

    // set & add pipe to stream
    if (0>FDstream.invalidate())  //trigger HW
    {
          MY_LOGD("FDstream invalidate failed");
          //***********************************************************************//
          //Adjust FD thread priority to Normal and return false
          //***********************************************************************//
#if MTKCAM_HAVE_RR_PRIORITY
          policy    = SCHED_OTHER;
          priority  = 0;
          ::sched_getparam(0, &sched_p);
          sched_p.sched_priority = priority;  //  Note: "priority" is nice value
          sched_setscheduler(0, policy, &sched_p);
          setpriority(PRIO_PROCESS, 0, priority);
#endif          
          return false;
    }
 
 //***********************************************************************//
//Adjust FD thread priority to Normal
//***********************************************************************//
#if MTKCAM_HAVE_RR_PRIORITY
     policy    = SCHED_OTHER;
     priority  = 0;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
//***********************************************************************// 
    
    //MY_LOGD("Pipe_DDP_Performance_RGB565 End");
#endif    
     return ret;
}

/*******************************************************************************
* Create RGB565 QVGA for Tracking
********************************************************************************/
//0: 640x480 (NV21), 1: 4:3 (NV21), 2:  4:3 (YV12), 3: 16:9 (NV21),  4: 16:9(YV12), 5: Not Support!

MINT32
halFDVT::halFDBufferCreate(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8  ucBufferGroup   
)
{
    //MY_LOGD("halFDBufferCreate Start");
    
    g_BufferGroup = ucBufferGroup;
    
    switch(ucBufferGroup)
    {
        case 0:
                  g_uwDst_width = 320;
                  g_uwDst_height = 240;
                  doRGB565Buffer_SW(dstAddr, srcAddr);
                  break;
        case 1:
                  g_ucPlane = 2;
                  g_uwDst_width = 320;
                  g_uwDst_height = 240;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 2:
                  g_ucPlane = 3;
                  g_uwDst_width = 320;
                  g_uwDst_height = 240;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 3:
                  g_ucPlane = 2;
                  g_uwDst_width = 320;
                  g_uwDst_height = 180;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 4:
                  g_ucPlane = 3;
                  g_uwDst_width = 320;
                  g_uwDst_height = 180;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break; 
        case 5:
                  g_ucPlane = 2;
                  g_uwDst_width = 320;
                  g_uwDst_height = 192;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        case 6:
                  g_ucPlane = 3;
                  g_uwDst_width = 320;
                  g_uwDst_height = 192;
                  doRGB565Buffer_DDP(dstAddr, srcAddr);
                  break;
        default:
                 MY_LOGD("Unknow Group: %d,", ucBufferGroup);
                 break;
     }
     
     //MY_LOGD("halFDBufferCreate End");
     
     
#if (DUMP_IMAGE)
    char szFileName[100]={'\0'};
    char szFileName1[100]={'\0'};
    FILE * pRawFp;
    FILE * pRawFp1;
    int i4WriteCnt;
    
    MY_LOGD("ucBufferGroup: %d,", ucBufferGroup);
    
    sprintf(szFileName1, "/sdcard/srcOK_%04d_%04d_YCC420.raw", g_FDW,g_FDH);
    pRawFp1 = fopen(szFileName1, "wb");               
    if (NULL == pRawFp1 )
    {
        MY_LOGD("Can't open file to save RAW Image\n"); 
        while(1);
    }
    i4WriteCnt = fwrite((void *)srcAddr,1, (g_FDW * g_FDH * 1.5),pRawFp1);
    fflush(pRawFp1); 
    fclose(pRawFp1);    
        
    sprintf(szFileName, "/sdcard/dstOK_%04d_%04d_RGB565.raw", g_uwDst_width,g_uwDst_height);
    pRawFp = fopen(szFileName, "wb");               
    if (NULL == pRawFp )
    {
        MY_LOGD("Can't open file to save RAW Image\n"); 
        while(1);
    }
    i4WriteCnt = fwrite((void *)dstAddr,1, (g_uwDst_width * g_uwDst_height * 2),pRawFp);
    fflush(pRawFp); 
    fclose(pRawFp);
#endif     
     
     return MHAL_NO_ERROR;
     
}
/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDDo(
MUINT32 ScaleImages, 
MUINT32 ImageBuffer1, //RGB565 QVGA 
MUINT32 ImageBuffer2, //Y VGA
MBOOL   SDEnable,
MINT32 rRotation_Info
)
{
     //MY_LOGD("g_BufferGoup:%d,", g_BufferGroup);
    if(g_BufferGroup > 6)
    {
        MY_LOGD("Error BufferGoup:%d,", g_BufferGroup);
        return MHAL_INPUT_SIZE_ERROR;
    }
	
    MUINT8* y_vga;    
    
    if( g_input_vga == 0 && g_input_4_3 == 0 && g_input_16_9 == 0)
	      return MHAL_INPUT_SIZE_ERROR;

    //MY_LOGD("[halFDDo] IN,  %d", mFDCount);
    FACEDETECT_GSENSOR_DIRECTION direction;
    if( rRotation_Info == 0)
        direction = FACEDETECT_GSENSOR_DIRECTION_0;
    else if( rRotation_Info == 90)
        direction = FACEDETECT_GSENSOR_DIRECTION_270;	
    else if( rRotation_Info == 270)
        direction = FACEDETECT_GSENSOR_DIRECTION_90;	    
    else if( rRotation_Info == 180)
        direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;	
    else
        direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;	    
        
        
    //direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;
    int alreay_have_scale_images = 0;
    if(EnableSWResizerFlag)
    {
    	  FDVT_OPERATION_MODE_ENUM mode;
        m_pMTKFDVTObj->FDVTGetMode(&mode);
        

        if(g_input_vga == 1)
        {
    	      y_vga = (MUINT8*)ImageBuffer2;
    	      if( mode == FDVT_GFD_MODE)
    	      {
    	          CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
            		alreay_have_scale_images = 1;
            }
        }
        else if(g_input_4_3 == 1)
        {
        	  if(SDEnable)
        	  {
                Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_FDW, m_FDH, ImageVGABuffer);
                y_vga = ImageVGABuffer;
                CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
                alreay_have_scale_images = 1;
            }
            else
            {
            	  if( mode == FDVT_GFD_MODE)
    	          {
            	      CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array);
                    alreay_have_scale_images = 1;
                }
            }	
        }
        else if(g_input_16_9 == 1)
        {
        	  if(SDEnable || mode == FDVT_GFD_MODE)
        	  {
        	      Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_FDW, m_FDH, ImageVGABuffer);
                y_vga = ImageVGABuffer;
                CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
                alreay_have_scale_images = 1;
            }
        }
        
        m_pMTKFDVTObj->FDVTMain( (MUINT32)ImageScaleBuffer, (MUINT32)ImageBuffer1, FDVT_GFD_MODE, direction, 0);
        
        result  DetectResult[MAX_FACE_NUM];
        m_pMTKFDVTObj->FDVTGetResult( (MUINT32)&DetectResult, FACEDETECT_TRACKING_DISPLAY);
        
        static int last_face_num;
        if( last_face_num == 1 && DetectResult->face_num ==0 )
        {
            if(g_input_vga == 1)
            {
    	          y_vga = (MUINT8*)ImageBuffer2;
    	          if(alreay_have_scale_images == 0)
    	          {	
    	              CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
                }
            }
            else if(g_input_4_3 == 1)
            {
            	  if(alreay_have_scale_images == 0)
    	          {	
                    CreateScaleImagesFromPreview_Y( (MUINT8*)ImageBuffer2, ImageScaleBuffer, m_FDW, m_FDH, image_width_array, image_height_array);
                }
            }
            else if(g_input_16_9 == 1)
            {
        	      Create640WidthImage_Y((MUINT8*)ImageBuffer2, m_FDW, m_FDH, ImageVGABuffer);
                y_vga = ImageVGABuffer;
                
                if(alreay_have_scale_images == 0)
    	          {	
                    CreateScaleImagesFromVGA_Y( (MUINT8*)y_vga, ImageScaleBuffer, image_width_array, image_height_array);
                }
            }
            for(int i=0; i<2;i++)
            {
                m_pMTKFDVTObj->FDVTMain( (MUINT32)ImageScaleBuffer, (MUINT32)ImageBuffer1, FDVT_GFD_MODE, direction, i);
                m_pMTKFDVTObj->FDVTGetResult( (MUINT32)DetectResult, FACEDETECT_TRACKING_DISPLAY);
                if(DetectResult->face_num > 0 )
                	break;
            }
            
        }
        last_face_num = DetectResult->face_num;
        
        
        if(SDEnable)
        {
            m_pMTKFDVTObj->FDVTMain( (MUINT32)ImageScaleBuffer, (MUINT32)y_vga, FDVT_SD_MODE, direction, 0);
        }
    }
    else
    {
    		m_pMTKFDVTObj->FDVTMain( (MUINT32)ScaleImages, (MUINT32)ImageBuffer1, FDVT_GFD_MODE, direction, 0);
    		
    		if(SDEnable)
    		{
    		    m_pMTKFDVTObj->FDVTMain( (MUINT32)ScaleImages, (MUINT32)ImageBuffer2, FDVT_SD_MODE, direction, 0);
    	  }
    }
    
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDUninit(
)
{  
    //MHAL_LOG("[halFDUninit] IN \n");
    m_pMTKFDVTObj->FDVTReset();

    if(EnableSWResizerFlag)
    {
         delete[]ImageScaleBuffer;
    }    
    if(g_input_vga == 0)
    {
        delete[]ImageVGABuffer;
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;
     
    return err;
}

MINT32
halFDVT::halSDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/

MINT32
halFDVT::halFDGetFaceInfo(
    MtkCameraFaceMetadata *fd_info_result
)
{
    MUINT8 i;
    MY_LOGD("[GetFaceInfo] NUM_Face:%d,", FdResult_Num);
    
    if( (FdResult_Num < 0) || (FdResult_Num > 15) )
         FdResult_Num = 0;
  
    fd_info_result->number_of_faces =  FdResult_Num;
    
    for(i=0;i<FdResult_Num;i++)
    {
 	fd_info_result->faces[i].rect[0] = FdResult[i].rect[0];
	fd_info_result->faces[i].rect[1] = FdResult[i].rect[1];
	fd_info_result->faces[i].rect[2] = FdResult[i].rect[2];
	fd_info_result->faces[i].rect[3] = FdResult[i].rect[3];
	fd_info_result->faces[i].score = FdResult[i].score;
          fd_info_result->posInfo[i].rop_dir = FdResult[i].rop_dir;
          fd_info_result->posInfo[i].rip_dir  = FdResult[i].rip_dir;
    }    
    
	return MHAL_NO_ERROR;
}

MINT32
halFDVT::halFDGetFaceResult(
    MtkCameraFaceMetadata *fd_result
)
{
    MINT32 faceCnt = 0;
    MUINT8 pbuf[1024];   
    MUINT8 i;
    MINT8 DrawMode = 0;
    g_Lcm_rotate = g_Sensor_rotate =g_CameraTYPE = 0;
    
    faceCnt=m_pMTKFDVTObj->FDVTGetResult((MUINT32) pbuf, FACEDETECT_TRACKING_DISPLAY);
    
    m_DispW = image_width_array[0];
    m_DispH = image_height_array[0];;
    
    m_pMTKFDVTObj->FDVTGetICSResult((MUINT32) fd_result, (MUINT32) pbuf, m_DispW, m_DispH, g_Lcm_rotate, g_Sensor_rotate, g_CameraTYPE, DrawMode);
    
    FdResult_Num = fd_result->number_of_faces;
    
    for(i=0;i<MAX_FACE_NUM;i++)
    {          
          FdResult[i].rect[0] = fd_result->faces[i].rect[0];
          FdResult[i].rect[1] = fd_result->faces[i].rect[1];
          FdResult[i].rect[2] = fd_result->faces[i].rect[2];
          FdResult[i].rect[3] = fd_result->faces[i].rect[3];
          FdResult[i].score = fd_result->faces[i].score;
          FdResult[i].rop_dir = fd_result->posInfo[i].rop_dir;
          FdResult[i].rip_dir  = fd_result->posInfo[i].rip_dir;
    }
    
    
    
    return faceCnt;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::mHalFDSetDispInfo(
    MUINT32 x,
    MUINT32 y,
    MUINT32 w,
    MUINT32 h,
    MUINT32 Lcm_rotate,
    MUINT32 Sensor_rotate,
    MINT32 CameraTYPE
)
{
    MINT32 err = MHAL_NO_ERROR;

    // x,y is offset from left-top corner
    // w,h is preview frame width and height seen on LCD
    m_DispX = x;
    m_DispY = y;
    m_DispW = w;
    m_DispH = h;
    m_DispRoate = (Sensor_rotate << 5) | (Lcm_rotate <<2 ) | (CameraTYPE);
    
    g_Lcm_rotate = Lcm_rotate;
    g_Sensor_rotate = Sensor_rotate;
    g_CameraTYPE = CameraTYPE;
    
    //LOGE("[FDdraw_SetDisp] Lcm_rotate %d Sensor_rotate %d m_DispRoate %d \n",Lcm_rotate,Sensor_rotate,m_DispRoate); 

    return err;
}


/*******************************************************************************
*
********************************************************************************/

MINT32
halFDVT::halSetDetectPara(MUINT8 Para)
{
    MINT32 err = MHAL_NO_ERROR;
    
    m_DetectPara = Para;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halSDGetSmileResult()
{
    MINT32 SmileCnt = 0;
    MUINT8 pbuf1[8];
       
    SmileCnt = m_pMTKFDVTObj->FDVTGetSDResult((MUINT32) pbuf1);
    
    return SmileCnt;
}

typedef struct
{
    MUINT8 *srcAddr;
    MUINT32 srcWidth;
    MUINT32 srcHeight;
    MUINT8 *dstAddr;
    MUINT32 dstWidth;
    MUINT32 dstHeight;
} PIPE_BILINEAR_Y_RESIZER_STRUCT, *P_PIPE_BILINEAR_Y_RESIZER_STRUCT; 

#define PIPE_IUL_I_TO_X(i)				((i) << 16)              		  ///< Convert from integer to S15.16 fixed-point
#define PIPE_IUL_X_TO_I(x)         		(((x) + (1 << 15)) >> 16) 	 ///< Convert from S15.16 fixed-point to integer (round)
#define PIPE_IUL_X_TO_I_CHOP(x)		((x) >> 16)              		  ///< Convert from S15.16 fixed-point to integer (chop)
#define PIPE_IUL_X_TO_I_CARRY(x)		(((x) + 0x0000FFFF) >> 16) ///< Convert from S15.16 fixed-point to integer (carry)
#define PIPE_IUL_X_FRACTION(x)		((x) & 0x0000FFFF)

#define PIPE_LINEAR_INTERPOLATION(val1, val2, weighting2)   \
   PIPE_IUL_X_TO_I((val1) * (PIPE_IUL_I_TO_X(1) - (weighting2)) + (val2) * (weighting2))

static void PipeBilinearResizer(P_PIPE_BILINEAR_Y_RESIZER_STRUCT pUtlRisizerInfo)
{
	const MUINT32 srcPitch = pUtlRisizerInfo->srcWidth;
	const MUINT32 srcStepX = PIPE_IUL_I_TO_X(pUtlRisizerInfo->srcWidth) /pUtlRisizerInfo->dstWidth;
	const MUINT32 srcStepY = PIPE_IUL_I_TO_X(pUtlRisizerInfo->srcHeight) /pUtlRisizerInfo->dstHeight;
	const MUINT32 img_w = pUtlRisizerInfo->dstWidth;

	MUINT8 *const src_buffer = pUtlRisizerInfo->srcAddr;
	MUINT8 *dstAddr= pUtlRisizerInfo->dstAddr;
	MUINT32 srcCoordY = 0;
	MINT32 h = pUtlRisizerInfo->dstHeight;

	while (--h >= 0)
	{
		MINT32 w = img_w;
		MUINT32 srcCoordX = 0;

		MINT32 srcOffset_1;
		MINT32 srcOffset_2;
		MUINT8 *src_ptr_1;
		MUINT8 *src_ptr_2;

		MINT32 y_carry = PIPE_IUL_X_TO_I_CARRY(srcCoordY);
		MINT32 y_chop  = PIPE_IUL_X_TO_I_CHOP(srcCoordY);

        if(y_carry < 0 || y_carry >= (MINT32)pUtlRisizerInfo->srcHeight)
            return;
        if(y_chop < 0 || y_chop >= (MINT32)pUtlRisizerInfo->srcHeight)
            return;


		srcOffset_1 = y_chop * srcPitch;
		srcOffset_2 = y_carry * srcPitch;
		src_ptr_1 = src_buffer + srcOffset_1;
		src_ptr_2 = src_buffer + srcOffset_2;

		while (--w >= 0)
		{
			MUINT8 pixel_1, pixel_2;
			MINT32 y, y1;

			MINT32 x_carry = PIPE_IUL_X_TO_I_CARRY(srcCoordX);
			MINT32 x_chop = PIPE_IUL_X_TO_I_CHOP(srcCoordX);

			MINT32 weighting2;

			weighting2 = PIPE_IUL_X_FRACTION(srcCoordX);

			/// 1st horizontal interpolation.
			pixel_1 = *(src_ptr_1 + x_chop);
			pixel_2 = *(src_ptr_1 + x_carry);
			y = PIPE_LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

			/// 2nd horizontal interpolation.
			pixel_1 = *(src_ptr_2 + x_chop);
			pixel_2 = *(src_ptr_2 + x_carry);
			y1 = PIPE_LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

			/// Vertical interpolation.
			weighting2 = PIPE_IUL_X_FRACTION(srcCoordY);

			y = PIPE_LINEAR_INTERPOLATION(y, y1, weighting2);

			*dstAddr++ = (MUINT8)y;

			srcCoordX += srcStepX;
		}
		srcCoordY += srcStepY;
	}
}  
static void Create640WidthImage_Y(MUINT8* src_img, MUINT32 src_w, MUINT32 src_h, MUINT8* dst_img)
{
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;	
    UtlRisizerInfo.srcAddr = src_img;
    UtlRisizerInfo.srcWidth= src_w;
    UtlRisizerInfo.srcHeight= src_h;
    UtlRisizerInfo.dstAddr = dst_img;
    UtlRisizerInfo.dstWidth = image_input_width_vga;
    if(g_input_4_3 == 1)
    {
        UtlRisizerInfo.dstHeight = image_input_height_vga;
    }
    else if(g_input_16_9 == 1)
    {
        UtlRisizerInfo.dstHeight = src_h*image_input_width_vga/src_w;
    }
    PipeBilinearResizer(&UtlRisizerInfo);
}

static void CreateScaleImagesFromPreview_Y(MUINT8* image_buffer_preview, MUINT8* image_buffer, MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < FD_SCALES ; current_scale ++ )
    {
    	  UtlRisizerInfo.srcAddr = image_buffer_preview;
        UtlRisizerInfo.srcWidth= w;
        UtlRisizerInfo.srcHeight= h;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        PipeBilinearResizer(&UtlRisizerInfo);
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
    }

}

static void CreateScaleImagesFromVGA_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray)
{
    MINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < FD_SCALES ; current_scale ++ )
    {
    	  UtlRisizerInfo.srcAddr = image_buffer_vga;
        UtlRisizerInfo.srcWidth= image_input_width_vga;
        UtlRisizerInfo.srcHeight= image_input_height_vga;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        PipeBilinearResizer(&UtlRisizerInfo);
        /*
        FILE* fp;
        char filename[20];
        sprintf(filename, "%d_%dx%d.raw", current_scale, ImageWidthArray[current_scale], ImageHeightArray[current_scale]);
        fp =fopen(filename, "wb");
        fwrite( (void*)dst_ptr, ImageWidthArray[current_scale]*ImageHeightArray[current_scale], 1, fp);
        fclose(fp);
        */
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
    }

}


