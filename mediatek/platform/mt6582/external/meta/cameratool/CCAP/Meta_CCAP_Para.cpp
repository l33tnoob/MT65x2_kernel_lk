/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define LOG_TAG "CCAP"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sched.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

extern "C" {
#include <pthread.h>
}

#include "meta_ccap_para.h"
#include "AcdkLog.h"
//#include "cct_if.h"

#define CCAP_DEBUG 1


#define FT_FAT_MAX_PEER_SIZE		2048*8
#define FT_FAT_MAX_FRAME_SIZE		FT_FAT_MAX_PEER_SIZE/64*56

//static bool IsSupport(ACDK_CCT_IS_SUPPORTED_ENUM enmID,bool* pbResult);

static unsigned short nScreenLeft			= 0;
static unsigned short nScreenRight			= 240;
static unsigned short nScreenTop				= 0;
static unsigned short nScreenBottom			= 320;
static const int nBitDepth				= 16;


static META_BOOL bCapDone = FALSE;
static const UINT32				MAX_ACDK_IOCTL_PARA_LENGTH	= 1024;
static  UINT16				PREVIEW_WIDTH				= 240;
static  UINT16				PREVIEW_HEIGHT				= 320;
//static  UINT32				BUFFER_SIZE					= (UINT32)(PREVIEW_WIDTH * PREVIEW_HEIGHT * 2);
static  UINT32				BUFFER_SIZE           = (UINT32)(240*320*2);
static const UINT32				LOG_BUFFER_SIZE				= 1024 * 4;
static const UINT32				BUFFER_NUMBER				= 3;
static const int				nBufferSize					= 4096;

static		 META_BOOL				bACDKOpenFlag				= FALSE;
static       UINT32				g_nJPGSize					= 0;
static		 UINT32				g_iFullSizeWidth			= 0;
static		 UINT32				g_iFullSizeHeight			= 0;
static       UINT32				g_RealRead					= 0;
static       int				g_PeerBufferLen				= 0;
static       char*				g_pPeerBuffer				= NULL;

static       unsigned int				g_CaptureBufferLen				= 0;
static       char*				g_pCapturerBuffer				= NULL;


static       unsigned int				g_NvramBufferLen				= 0;
static       char*				g_pNvramBuffer				= NULL;

static      int g_iCapSize = 0;
static      int g_iCapWidth = 0;
static      int g_iCapHeight = 0;

static ACDK_FEATURE_INFO_STRUCT		g_ACDKCCTFeatureInfo;

static UINT8 g_TEST = 123;
static META_BOOL g_bAcdkOpend = FALSE;

static META_BOOL bGetImgDimensionDone = FALSE;
static int g_iGetImgDimensionWidth = 0;
static int g_iGetImgDimensionHeight = 0;

static int g_acdkState = -1;
static META_BOOL g_init_flag = FALSE;
static FT_CCT_STATE_MACHINE	g_FT_CCT_StateMachine =
{
	// is init
	KAL_FALSE,
	// is fb init
	KAL_FALSE,
	FT_CNF_FAIL,
	// memory management
	0, 0,
	0, FT_CCT_INTERNAL_BUF_SIZE,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	// sensor info
	{
		0,
		{
			{ ACDK_CCT_REG_ISP, 0, 0, 0, 0, 0, AP_SENSOR_OUTPUT_FORMAT_RAW_Gr, 0, 0 },
			{ ACDK_CCT_REG_ISP, 0, 0, 0, 0, 0, AP_SENSOR_OUTPUT_FORMAT_RAW_Gr, 0, 0 },
			{ ACDK_CCT_REG_ISP, 0, 0, 0, 0, 0, AP_SENSOR_OUTPUT_FORMAT_RAW_Gr, 0, 0 },
			{ ACDK_CCT_REG_ISP, 0, 0, 0, 0, 0, AP_SENSOR_OUTPUT_FORMAT_RAW_Gr, 0, 0 }
		}
	},
		// sensor engineer mode state
		-1, -1, -1, -1, -1,
		// preview state
		NULL,
		// AE state
		KAL_FALSE,
		// WB state
		FT_CCT_WB_RESET,
		// AF state
		KAL_FALSE,
		KAL_TRUE,
		KAL_FALSE,
		// compensation mode state
		CAMERA_TUNING_PREVIEW_SET, // WARNING!! MUST set default mode to capture normal for MT6219 old chip
		// defect table calibration state
		KAL_FALSE,
		KAL_FALSE,
		// strobe calibration state
		KAL_FALSE,
		{ 0, 0 },
	// capture jpeg state
	CAPTURE_JPEG_IDLE,
	// USB data tunnel state
	KAL_FALSE,
	DATA_TUNNEL_RS232,
	USB_TUNNEL_IDLE,
	0,
	OUTPUT_PURE_RAW10,   //dana check later
	1,
	// dev tool scene mode
	//{ CAM_BANDING_60HZ, KAL_FALSE, CAM_AUTO_DSC_MODE },
	// debug
	//{0},
	//{0},
};


typedef META_BOOL (*FT_Process)(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff);	//define function point

typedef struct
{
	FT_CCT_OP	OP_ID;
	FT_Process	call;
	const char*      pchCommand;
}MsgFunction;

/************************************************************************/
/* ACDK Common Function                                                 */
/************************************************************************/
int WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);


/************************************************************************/
/* Function Pointer Structure Init                                      */
/************************************************************************/




const FT_CCT_SENSOR_EX * get_sensor_by_id(const ACDK_CCT_REG_TYPE_ENUM  type, const kal_uint32  device_id) {
		kal_uint8	i;
		for(i=0; i<g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor_count; i++) {
#if 0
			if( type==g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].type && device_id==g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].device_id ) {
				return &(g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i]);
			}
#else
			return &(g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0]);
#endif
		}
		return NULL;
	}


static META_BOOL bSendDataToACDK(/*ACDK_CCT_FEATURE_ENUM*/int	FeatureID,
						   UINT8*					pInAddr,
						   UINT32					nInBufferSize,
						   UINT8*					pOutAddr,
						   UINT32					nOutBufferSize,
						   UINT32*					pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (Mdk_IOControl(FeatureID, &rAcdkFeatureInfo));
}


static VOID vPrvCb(VOID *a_pParam)
{
//    ACDK_LOGD("Preview Callback ");

    //ageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    //ACDK_LOGD("Buffer Type:%d",  pImgBufInfo->eImgType);
    //ACDK_LOGD("Size:%d", pImgBufInfo->BufInfoUnion.rPrvVDOBufInfo.u4ImgSize);
    //ACDK_LOGD("Width:%d", pImgBufInfo->BufInfoUnion.rPrvVDOBufInfo.u2ImgXRes);
    //ACDK_LOGD("Height:%d", pImgBufInfo->BufInfoUnion.rPrvVDOBufInfo.u2ImgYRes);
}

static META_BOOL bSendDataToCCT(/*ACDK_CCT_FEATURE_ENUM*/int    FeatureID,
                                               UINT8*                                 pInAddr,
                                               UINT32                                 nInBufferSize,
                                               UINT8*                                 pOutAddr,
                                               UINT32                                 nOutBufferSize,
                                               UINT32*                                       pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (CctIF_IOControl(FeatureID, &rAcdkFeatureInfo));
}


META_BOOL FT_ACDK_CCT_OP_PREVIEW_LCD_START(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	if(!g_init_flag)
	{
		if(! Set_srcDev(g_FT_CCT_StateMachine.src_device_mode))
		{
		ACDK_LOGD("Set_srcDev fail"); 
		return FALSE;
		}
	}
      if (!g_bAcdkOpend )
      {
         return FALSE;
      }

      ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_START");

      //ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;
      ACDK_PREVIEW_STRUCT rCCTPreviewConfig;

      rCCTPreviewConfig.fpPrvCB = vPrvCb;
      rCCTPreviewConfig.u4PrvW = 320;
      rCCTPreviewConfig.u4PrvH = 240;
      rCCTPreviewConfig.u16PreviewTestPatEn = 0;

      UINT32 u4RetLen = 0;

/*
      if(!g_FT_CCT_StateMachine.is_fb_init)
      {
		  ACDK_LOGE("[CCAP]:  Begin clean cct fb!");
		  // clean cct frame buffer
		  if(ft_fb_init())
		  {
			  g_FT_CCT_StateMachine.is_fb_init = KAL_TRUE;
		  }
      }
      */

      META_BOOL bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START, (UINT8 *)&rCCTPreviewConfig,sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
      if (!bRet)
      {
      	pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
          return FALSE;
      }
      else
      {
        g_acdkState = 0;    // preview start
        ACDK_LOGD("g_acdkState(%d)",g_acdkState);
      }


	  // Do ft_cct_init after ACDK_CCT_OP_PREVIEW_LCD_START
	  if(!g_FT_CCT_StateMachine.is_init)
	  {
		  ACDK_LOGE("[CCAP]:  Begin initialize ft cct!");
		  // init isp, sensor and firmware
		  if(ft_cct_init())
			  g_FT_CCT_StateMachine.is_init = KAL_TRUE;
	  }

      pCNF->status = FT_CCT_ERR_PREVIEW_ALREADY_STARTED;
			g_FT_CCT_StateMachine.p_preview_sensor = get_sensor_by_id((ACDK_CCT_REG_TYPE_ENUM)NULL, NULL);

			return TRUE;

}


META_BOOL FT_ACDK_CCT_OP_PREVIEW_LCD_STOP(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
       if (!g_bAcdkOpend )
       {
           return FALSE;
       }

       ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_STOP");

       UINT32 u4RetLen = 0;
       META_BOOL bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &u4RetLen);
       if (!bRet)
       {
       	pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
           return FALSE;
    	 }
       else
       {
            g_acdkState = -1;    // preview stop
            ACDK_LOGD("g_acdkState(%d)",g_acdkState);
       }
    	 pCNF->status = FT_CCT_ERR_PREVIEW_ALREADY_STOPPED;
	     g_FT_CCT_StateMachine.p_preview_sensor = NULL;
	     
	        
		 bRet = bSendDataToACDK (ACDK_CMD_RESET_LAYER_BUFFER, NULL,0, NULL, 0, &u4RetLen);

		 if( !bRet)
		 {
		     ACDK_LOGE("[CCAP]: ACDK_CMD_RESET_LAYER_BUFFER fail !");
		     return FALSE;
		 }

	     
	     
	     
		 CctIF_DeInit();
		 CctIF_Close();
		Mdk_DeInit();
		Mdk_Close();
        //Temp. Mark
    //    CctIF_DeInit();
    //    CctIF_Close();
		g_bAcdkOpend = FALSE;

	     return TRUE;

}
BOOL Set_srcDev(UINT32 srcDev)
{
		
		//UINT32 srcDev = 2; //sub sensor [MT6589 sub = 0x2]
		ACDK_LOGD("[Set_srcDev] +\n"); 
    //====== Check If Change Device Without Preview Start/Stop Process ======
    if(g_acdkState == -1 && g_bAcdkOpend == TRUE)
    {
    	 CctIF_DeInit();
        CctIF_Close();
	 ACDK_LOGD("CctIF_DeInit() in  Set_srcDev()1\n"); 	
        Mdk_DeInit();
        Mdk_Close();
	 ACDK_LOGD("Mdk_DeInit() in  Set_srcDev()1\n"); 
	 			
        
        g_bAcdkOpend = FALSE;
    }
		if(!g_bAcdkOpend)
		{
			 	if (Mdk_Open() == FALSE)
  			{
      		ACDK_LOGE("Mdk_Open() Fail \n"); 
      		return FALSE;
  			}
  			else if (CctIF_Open() == FALSE)
  			{
      		ACDK_LOGE("CctIF_Open() Fail \n"); 
      		return FALSE;
  			}
  			else
  			{
            g_bAcdkOpend = TRUE;
  			}
			
		}
  
		//select camera sensor
		LOGD("Set main/sub sensor for isp object in AppCamCtrl::mrInitCamCtrl():%d \n",srcDev); 
		ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo; 
		bool bRet;
		unsigned int u4RetLen;
		rAcdkFeatureInfo.puParaIn = (UINT8*)&srcDev; 
		rAcdkFeatureInfo.u4ParaInLen = sizeof(UINT32); 
		rAcdkFeatureInfo.puParaOut = NULL; 
		rAcdkFeatureInfo.u4ParaOutLen = 0; 
		rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen; 
	
		LOGD("Set main/sub sensor for sensorInit() in IspHal::init():%d \n",srcDev); 
		bRet = Mdk_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
		if (!bRet) {
			LOGD("ACDK_FEATURE_SET_SRC_DEV Fail: %d \n",srcDev); 
			return E_CCT_CCAP_API_FAIL; 
		}

		LOGE("lln::init ACDK\n");
		if(Mdk_Init()==false)
		{
			LOGE("Mdk_Init fail\n");
			ACDK_LOGD("Mdk_DeInit() in  Set_srcDev()2\n"); 
			CctIF_DeInit();
			CctIF_Close();
			Mdk_DeInit();
			Mdk_Close();
    	g_bAcdkOpend = FALSE;
			
			return E_CCT_CCAP_API_FAIL; 
			
			
		}	
		else if(CctIF_Init(g_FT_CCT_StateMachine.src_device_mode)==false)
		{
			LOGE("CctIF_Init fail\n");
			ACDK_LOGD("CctIF_DeInit() in  Set_srcDev()2\n"); 
			CctIF_DeInit();
			CctIF_Close();
    	g_bAcdkOpend = FALSE;
			
			return E_CCT_CCAP_API_FAIL; 		
			
		}
		ACDK_LOGD("Mdk_Init() in  Set_srcDev()3\n"); 
		
    g_bAcdkOpend = TRUE; 
		g_init_flag =TRUE;
		ACDK_LOGD("[Set_srcDev] -\n"); 
    return S_CCT_CCAP_OK; 
		
}
BOOL Set_SubCamera()
{

		UINT32 srcDev = 2; //sub sensor [MT6589 sub = 0x2]

    //====== Check If Change Device Without Preview Start/Stop Process ======
    if(g_acdkState == -1 && g_bAcdkOpend == TRUE)
    {
		ACDK_LOGD("CctIF_DeInit() = %d\n", CctIF_DeInit());//MFALSE is 0 
		ACDK_LOGD("CctIF_Close() = %d\n", CctIF_Close());
		ACDK_LOGD("Mdk_DeInit() = %d\n", Mdk_DeInit());
		ACDK_LOGD("Mdk_Close() = %d\n", Mdk_Close());
        //Temp. Mark

        g_bAcdkOpend = FALSE;
    }
		if(!g_bAcdkOpend)
		{
			 	if (Mdk_Open() == FALSE)
  			{
      		ACDK_LOGE("Mdk_Open() Fail \n");
      		return FALSE;
  			}
  			else
  			{
            g_bAcdkOpend = TRUE;
				ACDK_LOGD("CctIF_Open() = %d\n", CctIF_Open());
  			}

		}

		//select camera sensor
		LOGD("Set main/sub sensor for isp object in AppCamCtrl::mrInitCamCtrl():%d \n",srcDev);
		ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
		bool bRet;
		unsigned int u4RetLen;
		rAcdkFeatureInfo.puParaIn = (UINT8*)&srcDev;
		rAcdkFeatureInfo.u4ParaInLen = sizeof(UINT32);
		rAcdkFeatureInfo.puParaOut = NULL;
		rAcdkFeatureInfo.u4ParaOutLen = 0;
		rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

		LOGD("Set main/sub sensor for sensorInit() in IspHal::init():%d \n",srcDev);
		bRet = Mdk_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
		if (!bRet) {
			LOGD("ACDK_FEATURE_SET_SRC_DEV Fail: %d \n",srcDev);
			return E_CCT_CCAP_API_FAIL;
		}

		LOGE("lln::init ACDK\n");
        //Temp. Mark
        //if((Mdk_Init()==false) || (CctIF_Init() == false))
		if(Mdk_Init()==false)
		{
			LOGE("Mdk_Init fail\n");
			ACDK_LOGD("Mdk_Init()==false\n");
			ACDK_LOGD("CctIF_DeInit() = %d\n", CctIF_DeInit());
			ACDK_LOGD("CctIF_Close() = %d\n", CctIF_Close());
			ACDK_LOGD("Mdk_DeInit() = %d\n", Mdk_DeInit());
			ACDK_LOGD("Mdk_Close() = %d\n", Mdk_Close());
            //Temp. Mark
            //CctIF_DeInit();
            //CctIF_Close();
    	g_bAcdkOpend = FALSE;

			return E_CCT_CCAP_API_FAIL;


		}
		ACDK_LOGD("CctIF_Init() = %d\n", CctIF_Init(srcDev));
		ACDK_LOGD("[Set_SubCamera] END\n");

    	g_bAcdkOpend = TRUE;

    	return S_CCT_CCAP_OK;

}


BOOL Set_Main2Camera()
{

		UINT32 srcDev = 8; //main2 sensor
    //====== Check If Change Device Without Preview Start/Stop Process ======
    if(g_acdkState == -1 && g_bAcdkOpend == TRUE)
    {
        Mdk_DeInit();
        Mdk_Close();

        //Temp. Mark
        //CctIF_DeInit();
        //CctIF_Close();
        g_bAcdkOpend = FALSE;
    }
		if(!g_bAcdkOpend)
		{
		        //Temp. Mark
		        //if ((Mdk_Open() == FALSE) || (CctIF_Open() == FALSE))
			 	if (Mdk_Open() == FALSE)
  			{
      		ACDK_LOGE("Mdk_Open() Fail \n");
      		return FALSE;
  			}
  			else
  			{
  				g_bAcdkOpend = TRUE;
  			}

		}

		//select camera sensor
		ACDK_LOGD("Set main/sub sensor for isp object in AppCamCtrl::mrInitCamCtrl():%d \n",srcDev);
		ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
		bool bRet;
		unsigned int u4RetLen;
		rAcdkFeatureInfo.puParaIn = (UINT8*)&srcDev;
		rAcdkFeatureInfo.u4ParaInLen = sizeof(UINT32);
		rAcdkFeatureInfo.puParaOut = NULL;
		rAcdkFeatureInfo.u4ParaOutLen = 0;
		rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

		ACDK_LOGD("Set main/sub sensor for sensorInit() in IspHal::init():%d \n",srcDev);
		bRet = Mdk_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
		if (!bRet) {
			ACDK_LOGD("ACDK_FEATURE_SET_SRC_DEV Fail: %d \n",srcDev);
			return E_CCT_CCAP_API_FAIL;
		}

		ACDK_LOGE("lln::init ACDK\n");

        //Temp. Mark
        //if((Mdk_Init()==false) || (CctIF_Init() == false))
		if(Mdk_Init()==false)
		{
			ACDK_LOGE("Mdk_Init fail\n");
			Mdk_DeInit();
			Mdk_Close();
            //Temp. Mark
            //CctIF_DeInit();
            //CctIF_Close();
			g_bAcdkOpend = FALSE;

			return E_CCT_CCAP_API_FAIL;


		}

    	g_bAcdkOpend = TRUE;

    	return S_CCT_CCAP_OK;

}

BOOL FT_ACDK_CCT_OP_SUBPREVIEW_LCD_START(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	  Set_SubCamera();
      if (!g_bAcdkOpend )
      {
         return FALSE;
      }


      ACDK_LOGD("FT_CCT_OP_SUBPREVIEW_LCD_START\n");

      //ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;
      ACDK_PREVIEW_STRUCT rCCTPreviewConfig;

      rCCTPreviewConfig.fpPrvCB = vPrvCb;
      rCCTPreviewConfig.u4PrvW = 320;
      rCCTPreviewConfig.u4PrvH = 240;
      rCCTPreviewConfig.u16PreviewTestPatEn = 0;

      UINT32 u4RetLen = 0;


      BOOL bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START, (UINT8 *)&rCCTPreviewConfig,sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
      if (!bRet)
      {
      	pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
          return FALSE;
      }
      pCNF->status = FT_CCT_ERR_PREVIEW_ALREADY_STARTED;
			g_FT_CCT_StateMachine.p_preview_sensor = get_sensor_by_id((ACDK_CCT_REG_TYPE_ENUM)NULL, NULL);



	     return TRUE;

}

BOOL FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
  ACDK_LOGD("FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL JB2-82\n");
	if (!g_bAcdkOpend )
	{
    ACDK_LOGD("FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL fail -- g_bAcdkOpend not open\n");
    return FALSE;
	}


#ifdef DUMMY_FLASHLIGHT
  ACDK_LOGD("FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL DUMMY_FLASHLIGHT defined\n");
  pCNF->status = FT_CNF_FAIL;
	if(1)
	{
	    ACDK_LOGD("FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL fail -- Dummy flash\n");
    	return FALSE;
  }
#else
  ACDK_LOGD("FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL DUMMY_FLASHLIGHT NOT Defined\n");

#endif

  UINT32 u4RetLen = 0;
	unsigned int inData = 500000;

	BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_FLASH_CONTROL,
														   (UINT8*)&inData,
														   (UINT32)sizeof(int),
														   NULL,
														   0,
														   &u4RetLen);
	if (bRet<0)
  {
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL fail -- acdk return error:");
    pCNF->status = FT_CNF_FAIL;
    return FALSE;
  }

	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL pass!");
	pCNF->status = FT_CNF_OK;
	return true;

}



BOOL FT_ACDK_CCT_OP_SUBPREVIEW_LCD_STOP(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
       if (!g_bAcdkOpend )
       {
           return FALSE;
       }

       ACDK_LOGD("FT_CCT_OP_SUBPREVIEW_LCD_STOP\n");

       UINT32 u4RetLen = 0;
       BOOL bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &u4RetLen);
       if (!bRet)
       {
       		pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
           	return FALSE;
    	}
	   pCNF->status = FT_CCT_ERR_PREVIEW_ALREADY_STOPPED;
	   g_FT_CCT_StateMachine.p_preview_sensor = NULL;

		CctIF_DeInit();
              CctIF_Close();
	   	Mdk_DeInit();
		Mdk_Close();
        //Temp. Mark
        //CctIF_DeInit();
        //CctIF_Close();
		g_bAcdkOpend = FALSE;

	     return TRUE;

}


BOOL FT_ACDK_CCT_OP_MAIN2PREVIEW_LCD_START(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	  Set_Main2Camera();
      if (!g_bAcdkOpend )
      {
         return FALSE;
      }


      ACDK_LOGD("FT_CCT_OP_MAIN2PREVIEW_LCD_START\n");

	ACDK_PREVIEW_STRUCT rCCTPreviewConfig;
      //ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;

      rCCTPreviewConfig.fpPrvCB = vPrvCb;
      rCCTPreviewConfig.u4PrvW= 320;
      rCCTPreviewConfig.u4PrvH= 240;
      rCCTPreviewConfig.u16PreviewTestPatEn = 0;

      UINT32 u4RetLen = 0;


      BOOL bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START, (UINT8 *)&rCCTPreviewConfig,sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
      if (!bRet)
      {
      	pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
          return FALSE;
      }
      pCNF->status = FT_CCT_ERR_PREVIEW_ALREADY_STARTED;
			g_FT_CCT_StateMachine.p_preview_sensor = get_sensor_by_id((ACDK_CCT_REG_TYPE_ENUM)NULL, NULL);



	     return TRUE;

}


BOOL FT_ACDK_CCT_OP_MAIN2PREVIEW_LCD_STOP(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
       if (!g_bAcdkOpend )
       {
           return FALSE;
       }

       ACDK_LOGD("FT_CCT_OP_MAIN2PREVIEW_LCD_STOP\n");

       UINT32 u4RetLen = 0;
       BOOL bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &u4RetLen);
       if (!bRet)
       {
       		pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
           	return FALSE;
    	}
	   pCNF->status = FT_CCT_ERR_PREVIEW_ALREADY_STOPPED;
	   g_FT_CCT_StateMachine.p_preview_sensor = NULL;

	   	Mdk_DeInit();
		Mdk_Close();
        //Temp. Mark
        //CctIF_DeInit();
        //CctIF_Close();
		g_bAcdkOpend = FALSE;

	     return TRUE;

}

bool FT_ACDK_CCT_OP_QUERY_ISP_ID(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	//pCNF->result.isp_id = 0x62388a00;
	//return true;

	UINT32 id = 0;
	UINT32 nRealRead = 0;

	bSendDataToCCT(ACDK_CCT_OP_QUERY_ISP_ID,
							   NULL,
							   0,
							   (UINT8*)&id,
							   4,
							   &nRealRead
                  );


	ACDK_LOGD("[CCAP]:  Query ISP ID OK! ID:%u RealReadCnt:%u",id,nRealRead);
	pCNF->result.isp_id = id;
	return true;


}



bool FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_ISP_SET_TUNING_PARAS param;
	memcpy(&param,(ACDK_CCT_ISP_SET_TUNING_PARAS*)(*pBuff),sizeof(ACDK_CCT_ISP_SET_TUNING_PARAS));

	UINT32 nRealReadByteCnt = 0;


	ACDK_LOGD("[CCAP]:========== FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS ==========");
	ACDK_LOGD("Index=%u, Category=%d", param.u4Index, param.eCategory);
	ACDK_LOGD("-------------------------");
	int i,j;
	for (i = 0; i < NVRAM_OBC_TBL_NUM; i++)
	{
		ACDK_LOGD("+ OBC[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.OBC[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.OBC[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_NR1_TBL_NUM; i++)
	{
		ACDK_LOGD("+ NR1[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.NR1[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.NR1[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_CFA_TBL_NUM; i++)
	{
		ACDK_LOGD("+ CFA[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.CFA[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.CFA[i].set[j]);
		}
	}


	for (i = 0; i < NVRAM_ANR_TBL_NUM; i++)
	{
		ACDK_LOGD("+ ANR[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.ANR[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.ANR[i].set[j]);
		}
	}


	for (i = 0; i < NVRAM_CCR_TBL_NUM; i++)
	{
		ACDK_LOGD("+ CCR[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.CCR[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.CCR[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_EE_TBL_NUM; i++)
	{
		ACDK_LOGD("+ EE[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.EE[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.EE[i].set[j]);
		}
	}

/*
	for (i = 0; i < NVRAM_NR3D_TBL_NUM; i++)
	{
		ACDK_LOGD("+ NR3D[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.NR3D[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.NR3D[i].set[j]);
		}
	}


	for (i = 0; i < NVRAM_MFB_TBL_NUM; i++)
	{
		ACDK_LOGD("+ MFB[%u]", i);
		for (j = 0; j < param.stIspNvramRegs.MFB[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, param.stIspNvramRegs.MFB[i].set[j]);
		}
	}
*/
	ACDK_LOGD("-------------------------");

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS,
						(UINT8*)&param,
						sizeof(ACDK_CCT_ISP_SET_TUNING_PARAS),
						NULL,
						0,
						&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS pass!");
	ACDK_LOGD("[CCAP]:======================================================");
	pCNF->status = FT_CNF_OK;
	return true;
}





bool FT_ACDK_CCT_OP_REG_READ(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
	int nSize = sizeof(ACDK_CCT_REG_RW_STRUCT);
	memset(&ACDK_reg_read,0,nSize);

	ACDK_reg_read.RegAddr	= pREQ->cmd.reg_read.reg_addr;
	ACDK_reg_read.Type		= pREQ->type;


	UINT32 nRealReadByteCnt = 0;

	if(false == bSendDataToCCT(ACDK_CCT_OP_REG_READ,
								(UINT8*)&ACDK_reg_read,
								sizeof(ACDK_CCT_REG_RW_STRUCT),
								(UINT8*)&ACDK_reg_read,
								sizeof(ACDK_CCT_REG_RW_STRUCT),
								&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_REG_READ driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_REG_READ pass!");
	pCNF->result.reg_read.value = ACDK_reg_read.RegData;
	return true;
}


bool FT_ACDK_CCT_OP_REG_WRITE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
	memset(&ACDK_reg_write,0,sizeof(ACDK_CCT_REG_RW_STRUCT));

	ACDK_reg_write.RegAddr = pREQ->cmd.reg_write.reg_addr;
	ACDK_reg_write.RegData = pREQ->cmd.reg_write.value;
	ACDK_reg_write.Type = pREQ->type;
	UINT32 nRealReadByteCnt = 0;

	if(false == bSendDataToCCT(ACDK_CCT_OP_REG_WRITE,
	                            (UINT8*)&ACDK_reg_write,
	                             sizeof(ACDK_CCT_REG_RW_STRUCT),
	                             NULL,
                               	 0,
	                             &nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_REG_WRITE driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_REG_WRITE pass!");
	return true;
}


#if 0 // Ignored on 6573
bool Load_from_navram(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    if (!g_bAcdkOpend )
    {
        return false;
    }

	ACDK_LOGD("FT_MSDK_CCT_OP_LOAD_FROM_NVRAM");

    UINT32 u4RetLen = 0;

    META_BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_LOAD_FROM_NVRAM, NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);

    if (!bRet)
    {
    		ACDK_LOGD("[CCAP]:Load_from_navram driver failed!");
        ACDK_LOGE("ACDK_CCT_OP_LOAD_FROM_NVRAM Fail");
        return false;
    }
	ACDK_LOGD("[CCAP]:Load_from_navram pass!");
    return true;

}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
bool Save_to_navram(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    if (!g_bAcdkOpend )
    {
        return false;
    }

    ACDK_LOGD("FT_MSDK_CCT_OP_SAVE_TO_NVRAM");

    UINT32 u4RetLen = 0;

    META_BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_SAVE_TO_NVRAM,
    														NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_SAVE_TO_NVRAM Fail");
        return false;
    }
		ACDK_LOGD("[CCAP]:Save_to_navram pass!");
    return true;
}



bool FT_ACDK_CCT_OP_LOAD_FROM_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	if(Load_from_navram(NULL,NULL,NULL)){
		pCNF->result.nvram.status = 1;
		pCNF->status = FT_CNF_OK;
		return true;

	}
	pCNF->result.nvram.status = 0;
	pCNF->status = FT_CNF_FAIL;
	return false;
}


bool FT_ACDK_CCT_OP_SAVE_TO_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    if (!g_bAcdkOpend )
    {
        return false;
    }

	UINT32 u4RetLen = 0;

    META_BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_SAVE_TO_NVRAM, NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);

	if (!bRet)
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_SAVE_TO_NVRAM driver failed!");
		pCNF->result.nvram.status = 0;
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_SAVE_TO_NVRAM pass!");
	pCNF->result.nvram.status = 1;
	pCNF->status = FT_CNF_OK;
	return true;
}
#endif




static bool GetSensorSesolutionInfo(ACDK_CCT_SENSOR_RESOLUTION_STRUCT* pSensor)
{
	if(NULL == pSensor)
		return false;

  UINT32 u4RetLen = 0;
	ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

	if (false == bSendDataToCCT(ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION,
                                 NULL,
                                 0,
                                 (UINT8 *)&SensorResolution,
                                 sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                 &u4RetLen))
	{
		ACDK_LOGD("[CCAP]:GetSensorSesolutionInfo driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:GetSensorSesolutionInfo pass!");
	memcpy(pSensor,&SensorResolution,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));
	return true;
}


static bool GetSersonInfo(ACDK_CCT_SENSOR_INFO_STRUCT* pData)
{
	ACDK_LOGE("[CCAP]: Begin get senor!");

	if(NULL == pData)
		return false;

	ACDK_CCT_SENSOR_INFO_STRUCT ACDK_Sensor;

	UINT32 ACDKCCTParaLen = 0;

	ACDK_LOGE("[CCAP]: Begin IOCtl!");
	if (false == bSendDataToCCT(ACDK_CCT_OP_QUERY_SENSOR,
															NULL,
															0,
															(UINT8 *)&ACDK_Sensor,
															sizeof(ACDK_CCT_SENSOR_INFO_STRUCT),
															&ACDKCCTParaLen))
	{
		ACDK_LOGD("[CCAP]:GetSersonInfo driver failed!");
		ACDK_LOGE("[CCAP]: Query sensor Error!");
		return false;
	}
	ACDK_LOGD("[CCAP]:sensor info first pixel is :%d",ACDK_Sensor.StartPixelBayerPtn);
	ACDK_LOGD("[CCAP]:GetSersonInfo pass");
	ACDK_LOGE("[CCAP]: End query sensor!");
	memcpy(pData,&ACDK_Sensor,sizeof(ACDK_CCT_SENSOR_INFO_STRUCT));
	return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ispTuningSettingIndexValue *index_value = (ispTuningSettingIndexValue *)&pREQ->cmd.dev_6238_isp_tuning_setting_index_value;
    ACDK_CCT_ISP_REG_CATEGORY const aryCategory[EIsp_Num_Of_Category] =
	{
         EIsp_Category_OBC,
         EIsp_Category_NR1,
         EIsp_Category_CFA,
         EIsp_Category_ANR,
         EIsp_Category_CCR,
         EIsp_Category_EE,
         //EIsp_Category_NR3D,
         //EIsp_Category_MFB
    };

	ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX acdk_cct_reg_idx;
	memset(&acdk_cct_reg_idx, 0, sizeof(acdk_cct_reg_idx));

	MUINT32 nRealReadByteCnt = 0;
	for(int i = 0; i < EIsp_Num_Of_Category; i++)
	{
        acdk_cct_reg_idx.eCategory  = aryCategory[i];
		acdk_cct_reg_idx.u4Index = index_value->value[i];

		ACDK_LOGD("[CCAP]:category=%d, index value=%d", acdk_cct_reg_idx.eCategory, acdk_cct_reg_idx.u4Index);
		if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX,
			(UINT8*)&acdk_cct_reg_idx,
			sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX),
			NULL,
			0,
			&nRealReadByteCnt))
			{
				ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX driver failed!");
				pCNF->status = FT_CNF_FAIL;
				break;
			}
			ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX pass!");
			pCNF->status = FT_CNF_OK;
	}
	return true;
}

bool FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_CCT_ISP_REG_CATEGORY const aryCategory[EIsp_Num_Of_Category] =
	{
        EIsp_Category_OBC,
        EIsp_Category_NR1,
        EIsp_Category_CFA,
        EIsp_Category_ANR,
        EIsp_Category_CCR,
        EIsp_Category_EE,
        //EIsp_Category_NR3D,
        //EIsp_Category_MFB
    };


    MUINT32 nRealReadByteCnt = 0;
    MUINT32 u4Index = 0xFFFFFFFF;

	pCNF->status = FT_CNF_OK;
	for(int i = 0; i < EIsp_Num_Of_Category; i++)
	{
        ACDK_CCT_ISP_REG_CATEGORY eCategory = aryCategory[i];
        u4Index = 0xFFFFFFFF;
		ACDK_LOGD("[CCAP]:category=%d", eCategory);
		if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX,
            (UINT8*)&eCategory,
            sizeof(eCategory),
            (UINT8*)&u4Index,
            sizeof(u4Index),
			&nRealReadByteCnt))
			{
				ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX driver failed!");
				pCNF->status = FT_CNF_FAIL;
				break;
			}
			ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX pass!");
		ACDK_LOGD("[CCAP]:index value=%d", u4Index);
		pCNF->result.get_6238_isp_tuning_setting_index_value.value[i] = u4Index;
	}
	return true;
}



/////////////////////////////////////////////////////////////////////////
//
//   mrSaveRAWImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
META_BOOL bRAW10To8(MUINT8 *a_pInBuf,  UINT32 a_u4Width, UINT32 a_u4Height, UINT32 a_u4Size, UINT8 a_uBitDepth,  UCHAR *a_pOutBuf)
{
    if (a_uBitDepth != 10)
    {
        ACDK_LOGE("Not support bitdepth");
        return FALSE;
    }

    UINT32 *pu4SrcBuf = (UINT32 *) a_pInBuf;
    //UCHAR *pucBuf = (UCHAR *) malloc (a_u4Width * a_u4Height  * 2 * sizeof(UCHAR));

    UCHAR *puDestBuf = (UCHAR *)a_pOutBuf;

    while (puDestBuf < (UCHAR *)a_pOutBuf + a_u4Width * a_u4Height)
    {
        UINT32 u4Pixel = *(pu4SrcBuf++);
        *(puDestBuf++) = (UCHAR)((u4Pixel & 0x03FF) >> 2);
        *(puDestBuf++) = (UCHAR)(((u4Pixel >> 10) & 0x03FF) >> 2);
        *(puDestBuf++) = (UCHAR)(((u4Pixel >> 20) & 0x03FF) >> 2);
    }


    return TRUE;
}




static VOID vCapCb(VOID *a_pParam)
{
    ACDK_LOGD("Capture Callback ");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    ACDK_LOGD("Buffer Type:%d",  pImgBufInfo->eImgType);
    ACDK_LOGD("output_format:%d",  g_FT_CCT_StateMachine.output_format);


		/*g_pCapturerBuffer = (char*)malloc(g_CaptureBufferLen);
		if(g_pCapturerBuffer == NULL)
			return;
		memset(g_pCapturerBuffer, 0, sizeof(g_CaptureBufferLen));*/


    if (pImgBufInfo->eImgType == PURE_RAW8_TYPE || pImgBufInfo->eImgType == PROCESSED_RAW8_TYPE
	|| pImgBufInfo->eImgType == PURE_RAW10_TYPE || 	pImgBufInfo->eImgType == PROCESSED_RAW10_TYPE)
    {
        //! currently the RAW buffer type is packed buffer
        //! The packed format is the same as MT6516 ISP format <00 Pixel1, Pixel2, Pixel3 > in 4bytes


        if(g_FT_CCT_StateMachine.output_format == OUTPUT_PURE_RAW8)
        {
        	ACDK_LOGD("g_FT_CCT_StateMachine.output_format : PURE_RAW8_TYPE");

		g_iCapSize = pImgBufInfo->RAWImgBufInfo.imgWidth * pImgBufInfo->RAWImgBufInfo.imgHeight * 1;
    		g_pCapturerBuffer = (UCHAR *) malloc (g_iCapSize);
		if(g_pCapturerBuffer == NULL) return;
		META_BOOL bRet = TRUE;
		/*
        	bRet = bRAW10To8(pImgBufInfo->RAWImgBufInfo.bufAddr,
              	                  pImgBufInfo->RAWImgBufInfo.imgWidth,
                                       pImgBufInfo->RAWImgBufInfo.imgHeight,
                                       pImgBufInfo->RAWImgBufInfo.imgSize,
                                       pImgBufInfo->RAWImgBufInfo.bitDepth,
                                       g_pCapturerBuffer);

        	if(!bRet)
        	{
        		ACDK_LOGD("bRAW10To8 error");
        		return;
        	}
        	*/
        	memcpy(g_pCapturerBuffer, (const void *)pImgBufInfo->RAWImgBufInfo.bufAddr, g_iCapSize);
		ACDK_LOGE("FT_ACDK_CCT_OP_GET_CAPTURE_BUF: the buf size is:%d",sizeof(g_pCapturerBuffer));
        }
	 else if(g_FT_CCT_StateMachine.output_format == OUTPUT_PROCESSED_RAW8)
        {
        	ACDK_LOGD("g_FT_CCT_StateMachine.output_format : PROCESSED_RAW8_TYPE");

		g_iCapSize = pImgBufInfo->RAWImgBufInfo.imgWidth * pImgBufInfo->RAWImgBufInfo.imgHeight * 1;
    		g_pCapturerBuffer = (UCHAR *) malloc (g_iCapSize);
		if(g_pCapturerBuffer == NULL) return;
		META_BOOL bRet = TRUE;
		memcpy(g_pCapturerBuffer, (const void *)pImgBufInfo->RAWImgBufInfo.bufAddr, g_iCapSize);
		ACDK_LOGE("FT_ACDK_CCT_OP_GET_CAPTURE_BUF: the buf size is:%d",sizeof(g_pCapturerBuffer));
       }
	else if(g_FT_CCT_StateMachine.output_format == OUTPUT_PURE_RAW10)
	{
		ACDK_LOGD("g_FT_CCT_StateMachine.output_format : OUTPUT_PURE_RAW10");

		g_iCapSize = pImgBufInfo->RAWImgBufInfo.imgWidth * pImgBufInfo->RAWImgBufInfo.imgHeight * 5 / 4;
		ACDK_LOGD("g_iCapSize:%d",  g_iCapSize);
    		g_pCapturerBuffer = (UCHAR *) malloc (g_iCapSize);
		if(g_pCapturerBuffer == NULL) return;
		META_BOOL bRet = TRUE;
		//memset(&g_pCapturerBuffer,0,sizeof(g_pCapturerBuffer));
		memcpy(g_pCapturerBuffer, (const void *)pImgBufInfo->RAWImgBufInfo.bufAddr, g_iCapSize);
		ACDK_LOGE("FT_ACDK_CCT_OP_GET_CAPTURE_BUF: the buf size is:%d",sizeof(g_pCapturerBuffer));
	}
	else if(g_FT_CCT_StateMachine.output_format == OUTPUT_PROCESSED_RAW10)
	{
		ACDK_LOGD("g_FT_CCT_StateMachine.output_format : PROCESSED_RAW10_TYPE");

		g_iCapSize = pImgBufInfo->RAWImgBufInfo.imgWidth * pImgBufInfo->RAWImgBufInfo.imgHeight * 5 / 4;
		ACDK_LOGD("g_iCapSize:%d",  g_iCapSize);
    		g_pCapturerBuffer = (UCHAR *) malloc (g_iCapSize);
		if(g_pCapturerBuffer == NULL) return;
		META_BOOL bRet = TRUE;
		//memset(&g_pCapturerBuffer,0,sizeof(g_pCapturerBuffer));
		memcpy(g_pCapturerBuffer, (const void *)pImgBufInfo->RAWImgBufInfo.bufAddr, g_iCapSize);
		ACDK_LOGE("FT_ACDK_CCT_OP_GET_CAPTURE_BUF: the buf size is:%d",sizeof(g_pCapturerBuffer));
	}
        ACDK_LOGD("Size:%d", g_iCapSize);
        ACDK_LOGD("Width:%d", pImgBufInfo->RAWImgBufInfo.imgWidth);
        ACDK_LOGD("Height:%d", pImgBufInfo->RAWImgBufInfo.imgHeight);
        ACDK_LOGD("BitDepth:%d", pImgBufInfo->RAWImgBufInfo.bitDepth);
        ACDK_LOGD("Bayer Start:%d", pImgBufInfo->RAWImgBufInfo.eColorOrder);
        g_iCapWidth = pImgBufInfo->RAWImgBufInfo.imgWidth;
        g_iCapHeight = pImgBufInfo->RAWImgBufInfo.imgHeight;
    	g_CaptureBufferLen = g_iCapSize;
    	bCapDone = TRUE;
    	return;
    }
    else if (pImgBufInfo->eImgType == JPEG_TYPE)
    {
        ACDK_LOGD("Size:%d", pImgBufInfo->imgBufInfo.imgSize);
        ACDK_LOGD("Width:%d", pImgBufInfo->imgBufInfo.imgWidth);
        ACDK_LOGD("Height:%d", pImgBufInfo->imgBufInfo.imgHeight);
        g_iCapSize = pImgBufInfo->imgBufInfo.imgSize;
        g_iCapWidth = pImgBufInfo->imgBufInfo.imgWidth;
        g_iCapHeight = pImgBufInfo->imgBufInfo.imgHeight;
        g_CaptureBufferLen = g_iCapSize;
    }
    else
    {
        ACDK_LOGD("UnKnow Format ");
        return;
    }

		g_pCapturerBuffer = (char*)malloc(g_CaptureBufferLen);
		if(g_pCapturerBuffer == NULL) return;
		memset(g_pCapturerBuffer, 0, sizeof(g_CaptureBufferLen));
		if (pImgBufInfo->eImgType == JPEG_TYPE)
			memcpy(g_pCapturerBuffer,(const void *)pImgBufInfo->RAWImgBufInfo.bufAddr,g_CaptureBufferLen);
    bCapDone = TRUE;
}








bool FT_ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX	(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{


	ACDK_CCT_CAP_OUTPUT_FORMAT ACDK_capture_type;
	//eACDK_CAP_FORMAT	ACDK_capture_type;
	unsigned short int width=g_iFullSizeWidth;
	unsigned short int height=g_iFullSizeHeight;
	static const FT_CCT_SENSOR_EX  	*s_sensor=NULL;
        //ACDK_CCT_STILL_CAPTURE_STRUCT StillCaptureConfigPara;
        ACDK_CAPTURE_STRUCT_S StillCaptureConfigPara;
        memset(&StillCaptureConfigPara, 0, sizeof(ACDK_CCT_STILL_CAPTURE_STRUCT));

	StillCaptureConfigPara.eOperaMode = ACDK_OPT_META_MODE;
	StillCaptureConfigPara.i4IsSave = 0;

	if( NULL == (s_sensor=get_sensor_by_id(pREQ->type, pREQ->device_id))) {
			pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
			ACDK_LOGD("Capture error!  FT_CCT_ERR_INVALID_SENSOR_ID ");
			return false;
	}
	if(CAMERA_TUNING_CAPTURE_SET==g_FT_CCT_StateMachine.comp_mode) {
		width = s_sensor->width;
		height = s_sensor->height;
		StillCaptureConfigPara.eCameraMode = CAPTURE_MODE;
		ACDK_LOGD("In CAPTURE MODE, eCameraMode=%d", StillCaptureConfigPara.eCameraMode);
	}
	else if(CAMERA_TUNING_PREVIEW_SET==g_FT_CCT_StateMachine.comp_mode) {
		width = s_sensor->preview_width;
		height = s_sensor->preview_height;
		StillCaptureConfigPara.eCameraMode = PREVIEW_MODE;
		ACDK_LOGD("In PREVIEW MODE, eCameraMode=%d", StillCaptureConfigPara.eCameraMode);
	}
	else if(CAMERA_TUNING_VIDEO_SET==g_FT_CCT_StateMachine.comp_mode) {
		width = s_sensor->preview_width;
		height = s_sensor->preview_height;
		StillCaptureConfigPara.eCameraMode = VIDEO_MODE;
		ACDK_LOGD("InVIDEO MODE, eCameraMode=%d", StillCaptureConfigPara.eCameraMode);
	}


	if(pREQ->cmd.capture_ex.output_format == OUTPUT_JPEG)
	{

		width = width / pREQ->cmd.capture_ex.sub_sample;
		height = height / pREQ->cmd.capture_ex.sub_sample;
		g_nJPGSize = height*width*3/5;
		ACDK_capture_type = OUTPUT_JPEG;
		StillCaptureConfigPara.eOutputFormat = JPEG_TYPE;
		g_FT_CCT_StateMachine.output_format = OUTPUT_JPEG;
		ACDK_LOGD("Capture: capture type is JPEG");
	}
	else if(pREQ->cmd.capture_ex.output_format == OUTPUT_PURE_RAW8)
	{
		g_nJPGSize = height*width;
		ACDK_capture_type = OUTPUT_PURE_RAW8;
    		StillCaptureConfigPara.eOutputFormat = PURE_RAW8_TYPE;
		g_FT_CCT_StateMachine.output_format = OUTPUT_PURE_RAW8;
   		ACDK_LOGD("Capture: capture type is 8 bit pure Raw data");
	}
	else if(pREQ->cmd.capture_ex.output_format == OUTPUT_PROCESSED_RAW8)
	{
		g_nJPGSize = height*width;
		ACDK_capture_type = OUTPUT_PROCESSED_RAW8;
    		StillCaptureConfigPara.eOutputFormat = PROCESSED_RAW8_TYPE;
		g_FT_CCT_StateMachine.output_format = OUTPUT_PROCESSED_RAW8;
   		ACDK_LOGD("Capture: capture type is 8 bit processed Raw data");
	}
	else if(pREQ->cmd.capture_ex.output_format == OUTPUT_PURE_RAW10)
	{
		g_nJPGSize = height*width * 5 / 4;
		ACDK_capture_type = OUTPUT_PURE_RAW10;
    		StillCaptureConfigPara.eOutputFormat = PURE_RAW10_TYPE;
		g_FT_CCT_StateMachine.output_format = OUTPUT_PURE_RAW10;
    		ACDK_LOGD("Capture: capture type is 10 bit pure Raw data");
	}
	else if(pREQ->cmd.capture_ex.output_format == OUTPUT_PROCESSED_RAW10)
	{
		g_nJPGSize = height*width * 5 / 4;
		ACDK_capture_type = OUTPUT_PROCESSED_RAW10;
    		StillCaptureConfigPara.eOutputFormat = PROCESSED_RAW10_TYPE;
		g_FT_CCT_StateMachine.output_format = OUTPUT_PROCESSED_RAW10;
    		ACDK_LOGD("Capture: capture type is 10 bit processed Raw data");
	}
	else
	{
		ACDK_LOGD("Capture error!  invalidate capture type");
		return false;
	}
	ACDK_LOGD("Capture: the orginal<latest> width:%d, height:%d",width,height);
	width = width & 0xFFF0;
	height = height & 0xFFF0;
	if(width < 320 || height < 240)
	{
		width = 320;
		height = 240;
	}
	ACDK_LOGD("Capture: the after<latest> width:%d, height:%d",width,height);
	StillCaptureConfigPara.u2JPEGEncWidth = width;
	StillCaptureConfigPara.u2JPEGEncHeight = height;
	StillCaptureConfigPara.fpCapCB = vCapCb;
	UINT32 ACDKCCTParaLen = 0;
	bCapDone = FALSE;

	g_FT_CCT_StateMachine.capture_jpeg_state = CAPTURE_JPEG_PROCESS;
	if (!bSendDataToACDK(ACDK_CMD_CAPTURE/*ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX*/,
		                           (UINT8 *)&StillCaptureConfigPara,
                               sizeof(ACDK_CCT_STILL_CAPTURE_STRUCT),
                               NULL,
                               0,
                               &ACDKCCTParaLen))
	{
		ACDK_LOGD("Capture: driver error");
		return false;
	}

	if(!bCapDone)
		return false;

	else
	{

		pCNF->result.capture_ex.sub_sample = pREQ->cmd.capture_ex.sub_sample;
		if (ACDK_capture_type == OUTPUT_JPEG)
		{
			pCNF->result.capture_ex.output_width = g_iCapWidth;
			pCNF->result.capture_ex.output_height = g_iCapHeight;
			pCNF->result.capture_ex.width = g_iCapWidth;
			pCNF->result.capture_ex.height =g_iCapHeight;
			pCNF->result.capture_ex.capture_size = g_iCapSize;
		}
		else if (ACDK_capture_type == OUTPUT_PURE_RAW8 ||
			ACDK_capture_type == OUTPUT_PROCESSED_RAW8)
		{
			pCNF->result.capture_ex.output_width = g_iCapWidth;
			pCNF->result.capture_ex.output_height = g_iCapHeight;
			pCNF->result.capture_ex.width = g_iCapWidth;
			pCNF->result.capture_ex.height = g_iCapHeight;
			pCNF->result.capture_ex.capture_size = g_iCapSize;
		}
		else if (ACDK_capture_type == OUTPUT_PURE_RAW10 ||
			ACDK_capture_type == OUTPUT_PROCESSED_RAW10)
		{
			pCNF->result.capture_ex.output_width = g_iCapWidth;
			pCNF->result.capture_ex.output_height = g_iCapHeight;
			pCNF->result.capture_ex.width = g_iCapWidth;
			pCNF->result.capture_ex.height = g_iCapHeight;
			pCNF->result.capture_ex.capture_size = g_iCapSize;
		}
		else
			return false;


		pCNF->result.capture_ex.output_format = pREQ->cmd.capture_ex.output_format;
		ACDK_LOGD("CCAP date length:%d",g_CaptureBufferLen);

		return true;
	}
}








bool FT_ACDK_CCT_OP_GET_CAPTURE_BUF	(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGE("FT_ACDK_CCT_OP_GET_CAPTURE_BUF: the buf size is:%d",sizeof(g_pCapturerBuffer));
  if(!g_pCapturerBuffer)
  {
  	ACDK_LOGE("error FT_ACDK_CCT_OP_GET_CAPTURE_BUF g_pCapturerBuffer is null");
     return false;
  }
	ACDK_LOGE("Enter FT_ACDK_CCT_OP_GET_CAPTURE_BUF");
	kal_uint32	request_length;
	request_length = pREQ->cmd.get_capture_buf.length;

		// check request offset
		if( pREQ->cmd.get_capture_buf.offset >= g_CaptureBufferLen )
		{
			ACDK_LOGE("Error FT_ACDK_CCT_OP_GET_CAPTURE_BUF<pREQ->cmd.get_capture_buf.offset >= g_CaptureBufferLen>");
			if(g_pCapturerBuffer)
			{
				free(g_pCapturerBuffer);
				g_pCapturerBuffer = NULL;
		        g_CaptureBufferLen = 0;
			}
			return false;
		}

		// check request length
		if( (pREQ->cmd.get_capture_buf.offset+request_length) > g_CaptureBufferLen ) {
			request_length = g_CaptureBufferLen-pREQ->cmd.get_capture_buf.offset;
		}

		// narrow down length to the max peer buffer size
		if( FT_FAT_MAX_FRAME_SIZE < request_length ) {
			request_length = FT_FAT_MAX_FRAME_SIZE;
		}
		ACDK_LOGE("offset:%d get_capture_buf.length:%d request_length:%d",pREQ->cmd.get_capture_buf.offset,pREQ->cmd.get_capture_buf.length,request_length);


		g_pPeerBuffer = (char*)malloc(request_length);
		if(g_pPeerBuffer == NULL) return false;
		g_PeerBufferLen = request_length;
		memset(g_pPeerBuffer, 0, request_length);
		memcpy(g_pPeerBuffer,g_pCapturerBuffer+pREQ->cmd.get_capture_buf.offset,request_length);


		if(pREQ->cmd.get_capture_buf.offset + request_length>=g_CaptureBufferLen)
		{
			free(g_pCapturerBuffer);
			g_pCapturerBuffer = NULL;
			g_CaptureBufferLen = 0;

			//Msdk6238IspDisableBinningMode(); //????????????????????
			ACDK_LOGE("FT_MSDK_CCT_OP_GET_CAPTURE_BUF over");
			g_FT_CCT_StateMachine.capture_jpeg_state = CAPTURE_JPEG_IDLE;
		}
	return true;
}


#if 0
//TODO
bool FT_ACDK_CCT_OP_SET_ENG_SENSOR_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	UINT32 uGroupCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT,NULL,0,(UINT8*)&uGroupCnt,sizeof(UINT32),&nRealReadByteCnt))//TODO
		return false;

	for(; 0 < uGroupCnt; uGroupCnt--)
	{
		UINT8 group_name[64] = {0};
		ACDK_SENSOR_GROUP_INFO_STRUCT ACDK_GroupName;
		memset(&ACDK_GroupName,0,sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT));
		ACDK_GroupName.GroupNamePtr = group_name;

		UINT32 nIndate = uGroupCnt - 1;

		if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA,
		(UINT8*)&nIndate,
		sizeof(UINT32),
		(UINT8*)&ACDK_GroupName,
		sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT),
		&nRealReadByteCnt))
		return false;

		int item_count = ACDK_GroupName.ItemCount;

		// find CCT group index
		if(!strcmp(((FT_CCT_SENSOR_ENG_KEY *)(*pBuff))->group_name, (kal_char*)group_name))
		{
		// found CCT group index
			//g_FT_CCT_StateMachine.sensor_eng_group_idx = MSDK_GroupName.GroupIdx;
			for(; 0 < item_count; item_count--)
			{
				ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Item;
				memset(&ACDK_Item,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

				ACDK_Item.GroupIdx = nIndate;
				ACDK_Item.ItemIdx  = item_count - 1;

				if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
				(UINT8*)&ACDK_Item,
				sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
				(UINT8*)&ACDK_Item,
				sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
				&nRealReadByteCnt))
				return false;

				if(!strcmp(((FT_CCT_SENSOR_ENG_KEY *)(*pBuff))->item_name, (kal_char*)ACDK_Item.ItemNamePtr)) {
					// found item
					ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Item_set;
					memset(&ACDK_Item_set,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));
					ACDK_Item_set.GroupIdx = nIndate;
					ACDK_Item_set.ItemIdx = item_count - 1;
					ACDK_Item_set.ItemValue = pREQ->cmd.set_eng_sensor_para;

					if(bSendDataToCCT(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
					(UINT8*)&ACDK_Item_set,
					sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
					NULL,
					0,
					&nRealReadByteCnt))
					{

						pCNF->status = FT_CNF_OK;
						ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_SET_ENG_SENSOR_PARA pas!");
						return true;
					}
					pCNF->status =  FT_CCT_ERR_SENSOR_ENG_SET_INVALID_VALUE;
					ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_SET_ENG_SENSOR_PARA driver failed!");
					return false;
				}

			}
			pCNF->status = FT_CCT_ERR_SENSOR_ENG_ITEM_NOT_EXIST;
			ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_SET_ENG_SENSOR_PARA driver failed!");
			return false;

		}
	}
	pCNF->status = FT_CCT_ERR_SENSOR_ENG_ITEM_NOT_EXIST;
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_SET_ENG_SENSOR_PARA driver failed!");
	return false;
}

// TODO
bool FT_ACDK_CCT_OP_GET_ENG_SENSOR_PARA		(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	memset(&(pCNF->result.get_eng_sensor_para), 0, sizeof(pCNF->result.get_eng_sensor_para));

	UINT32 nRealReadByteCnt = 0;
	UINT32 uGroupCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT,NULL,0,(UINT8*)&uGroupCnt,sizeof(UINT32),&nRealReadByteCnt))//TODO
		return false;

	for(; 0 < uGroupCnt; uGroupCnt--)
	{
		UINT8 group_name[64] = {0};
		ACDK_SENSOR_GROUP_INFO_STRUCT ACDK_GroupName;
		memset(&ACDK_GroupName,0,sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT));
		ACDK_GroupName.GroupNamePtr = group_name;

		UINT32 nIndate = uGroupCnt - 1;

		if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA,
		(UINT8*)&nIndate,
		sizeof(UINT32),
		(UINT8*)&ACDK_GroupName,
		sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT),
		&nRealReadByteCnt))
		return false;

		int item_count = ACDK_GroupName.ItemCount;

		// find CCT group index
		if(!strcmp(((FT_CCT_SENSOR_ENG_KEY *)(*pBuff))->group_name, (kal_char*)group_name))
		{
		// found CCT group index
			//g_FT_CCT_StateMachine.sensor_eng_group_idx = MSDK_GroupName.GroupIdx;
			for(; 0 < item_count; item_count--)
			{
				ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Item;
				memset(&ACDK_Item,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

				ACDK_Item.GroupIdx = nIndate;
				ACDK_Item.ItemIdx  = item_count - 1;

				if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
				(UINT8*)&ACDK_Item,
				sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
				(UINT8*)&ACDK_Item,
				sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
				&nRealReadByteCnt))
				return false;

				if(!strcmp(((FT_CCT_SENSOR_ENG_KEY *)(*pBuff))->item_name, (kal_char*)ACDK_Item.ItemNamePtr)) {
					// found item
					pCNF->result.get_eng_sensor_para.value = ACDK_Item.ItemValue;
					pCNF->result.get_eng_sensor_para.min = ACDK_Item.Min;
					pCNF->result.get_eng_sensor_para.max = ACDK_Item.Max;
					pCNF->result.get_eng_sensor_para.exist = KAL_TRUE;
					pCNF->status = FT_CNF_OK;
					ACDK_LOGD("[CCAP]: get_eng_sensor_para %d,%d,%d!",ACDK_Item.ItemValue,ACDK_Item.Min,ACDK_Item.Max);
					ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_GET_ENG_SENSOR_PARA pass!");
					return TRUE;
				}
			}
			ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_GET_ENG_SENSOR_PARA driver failed!");
			pCNF->status = FT_CCT_ERR_SENSOR_ENG_ITEM_NOT_EXIST;
			return FALSE;
		}
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_GET_ENG_SENSOR_PARA driver failed!");
	pCNF->status = FT_CCT_ERR_SENSOR_ENG_ITEM_NOT_EXIST;
	return FALSE;
}
#endif



bool FT_ACDK_CCT_OP_AE_DISABLE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_AE_DISABLE driver failed!");
		return false;
	}

	if(!bSendDataToCCT(ACDK_CCT_OP_AE_DISABLE,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_AE_DISABLE driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_AE_DISABLE pass!");
	g_FT_CCT_StateMachine.ae_enable = KAL_FALSE;
	return true;
}


bool FT_ACDK_CCT_OP_AE_ENABLE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		//pCNF->status = FT_CNF_FAIL;
		return false;
	}
	if(!bSendDataToCCT(ACDK_CCT_OP_AE_ENABLE,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		//pCNF->status = FT_CNF_FAIL;
		return false;
	}
	//pCNF->status = FT_CNF_OK;
	g_FT_CCT_StateMachine.ae_enable = KAL_TRUE;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN,
	NULL,
	0,
	NULL,
	0,
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN pass!");
	g_FT_CCT_StateMachine.wb_activated_idx = FT_CCT_WB_OFF;

	return true;
}

bool FT_ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	//void* pTemp = NULL;
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN pass!");
	g_FT_CCT_StateMachine.wb_activated_idx = FT_CCT_WB_AUTO;
	return true;
}



bool FT_ACDK_CCT_OP_WB_ACTIVATE_BY_INDEX		(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
		if(pREQ->cmd.wb_activate.index == FT_CCT_WB_AUTO)
		{

			FT_ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN(NULL,NULL,NULL);
			g_FT_CCT_StateMachine.wb_activated_idx = FT_CCT_WB_AUTO;
		}
		if(pREQ->cmd.wb_activate.index == FT_CCT_WB_RESET
			|| pREQ->cmd.wb_activate.index == FT_CCT_WB_OFF)
		{

			FT_ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN(NULL,NULL,NULL);
			g_FT_CCT_StateMachine.wb_activated_idx = FT_CCT_WB_OFF;
		}
		return true;
}


bool FT_ACDK_CCT_OP_QUERY_SENSOR				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("[CCAP]:  Begin QUERY_SENSOR!");
	FT_CCT_ON_BOARD_SENSOR* p_on_board_sensors = NULL;
	g_PeerBufferLen = sizeof(FT_CCT_ON_BOARD_SENSOR);
	ACDK_LOGD("[CCAP]:  Buffer Len:%d!",g_PeerBufferLen);

	// get peer buf
	g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
	if(g_pPeerBuffer == NULL) return false;

	p_on_board_sensors = (FT_CCT_ON_BOARD_SENSOR*)g_pPeerBuffer;

	memset(p_on_board_sensors, 0, sizeof(FT_CCT_ON_BOARD_SENSOR));
	p_on_board_sensors->sensor_count = g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor_count;
	ACDK_LOGD("[CCAP]:  Sensor Count:%d!",p_on_board_sensors->sensor_count);
	for(unsigned int i = 0; i < g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor_count; i++)
	{
		p_on_board_sensors->sensor[i].type		= g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].type;
		ACDK_LOGD("[CCAP]:  Sensor type:%d!",p_on_board_sensors->sensor[i].type);

		p_on_board_sensors->sensor[i].device_id	= g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].device_id;
		ACDK_LOGD("[CCAP]:  Sensor id:%d!",p_on_board_sensors->sensor[i].device_id);

		p_on_board_sensors->sensor[i].width		= g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].width;
		ACDK_LOGD("[CCAP]:  Sensor width:%d!",p_on_board_sensors->sensor[i].width);

		p_on_board_sensors->sensor[i].height	= g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].height;
		ACDK_LOGD("[CCAP]:  Sensor height:%d!",p_on_board_sensors->sensor[i].height);

		p_on_board_sensors->sensor[i].start_pixel_bayer_ptn = g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].start_pixel_bayer_ptn;
		ACDK_LOGD("[CCAP]:  Sensor ptn:%d!",p_on_board_sensors->sensor[i].start_pixel_bayer_ptn);

		p_on_board_sensors->sensor[i].grab_x_offset = g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].grab_x_offset;
		ACDK_LOGD("[CCAP]:  Sensor x:%d!",p_on_board_sensors->sensor[i].grab_x_offset);

		p_on_board_sensors->sensor[i].grab_y_offset = g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[i].grab_y_offset;
		ACDK_LOGD("[CCAP]:  Sensor y:%d!",p_on_board_sensors->sensor[i].grab_y_offset);
	}
	return true;
}




bool FT_ACDK_CCT_V2_OP_AWB_GET_GAIN				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_GAIN!");
	AWB_GAIN_T ACDK_AWB_Gain_struct;
	memset(&ACDK_AWB_Gain_struct,0,sizeof(AWB_GAIN_T));

	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_GAIN,
	NULL,
	0,
	(UINT8*)&ACDK_AWB_Gain_struct,
	sizeof(AWB_GAIN_T),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_GAIN driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_GAIN pass!");

	pCNF->result.get_6238_awb_gain.i4R = ACDK_AWB_Gain_struct.i4R;
	pCNF->result.get_6238_awb_gain.i4G = ACDK_AWB_Gain_struct.i4G;
	pCNF->result.get_6238_awb_gain.i4B = ACDK_AWB_Gain_struct.i4B;
	ACDK_LOGD("AWB GAIN is<%d><%d><%d>!",ACDK_AWB_Gain_struct.i4R,ACDK_AWB_Gain_struct.i4G,ACDK_AWB_Gain_struct.i4B);

	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AWB_SET_GAIN(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_SET_GAIN!");
	AWB_GAIN_T ACDK_AWB_Gain_struct;
	memset(&ACDK_AWB_Gain_struct,0,sizeof(AWB_GAIN_T));
	ACDK_AWB_Gain_struct.i4R = pREQ->cmd.dev_6238_awb_set_gain.i4R;
	ACDK_AWB_Gain_struct.i4G = pREQ->cmd.dev_6238_awb_set_gain.i4G;
	ACDK_AWB_Gain_struct.i4B = pREQ->cmd.dev_6238_awb_set_gain.i4B;
	ACDK_LOGD("AWB GAIN is<%d><%d><%d>!",ACDK_AWB_Gain_struct.i4R,ACDK_AWB_Gain_struct.i4G,ACDK_AWB_Gain_struct.i4B);
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_SET_GAIN,
	(UINT8 *)&ACDK_AWB_Gain_struct,
	sizeof(AWB_GAIN_T),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_SET_GAIN driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_SET_GAIN pass!");
	pCNF->status = FT_CNF_OK;

	return true;
}



bool FT_ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM		(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_CCM_STRUCT ccm;
	memset(&ccm,0,sizeof(ccm));
	memcpy(&ccm, &(pREQ->cmd.dev_6238_awb_current_ccm), sizeof(ccm));

	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM,
	(UINT8*)&ccm,
	sizeof(ccm),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM pass!");
	pCNF->status = FT_CNF_OK;
	return true;
}


/************************************************************************/
bool FT_ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM		(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_CCT_CCM_STRUCT ccm;
	memset(&ccm, 0, sizeof(ccm));

	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM,
	NULL,
	0,
	(UINT8*)&ccm,
	sizeof(ccm),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM pass!");
	ACDK_LOGD("M11 M12 M13 : 0x%03X 0x%03X 0x%03X", ccm.M11, ccm.M12, ccm.M13);
       ACDK_LOGD("M21 M22 M23 : 0x%03X 0x%03X 0x%03X", ccm.M21, ccm.M22, ccm.M23);
       ACDK_LOGD("M31 M32 M33 : 0x%03X 0x%03X 0x%03X", ccm.M31, ccm.M32, ccm.M33);
	memcpy(&(pCNF->result.get_6238_awb_current_ccm), &ccm, sizeof(ccm));
	pCNF->status = FT_CNF_OK;

	return true;
}




bool FT_ACDK_CCT_V2_OP_AE_GET_SCENE_MODE				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	INT32 mode =0;
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_SCENE_MODE,
	NULL,
	0,
	(UINT8*)&mode,
	sizeof(UINT8),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_SCENE_MODE driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_SCENE_MODE pass!");
	pCNF->result.get_6238_ae_scene_mode = (UINT8)mode;
	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_AE_GET_BAND				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	INT32 mode =0;
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_BAND,
	NULL,
	0,
	(UINT8*)&mode,
	sizeof(INT32),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_BAND driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_BAND pass!");
	pCNF->result.get_6238_ae_band = (UINT8)mode;
	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_FUNCTION_ENABLE_STRUCT ACDK_CCT_enable;
	memset(&ACDK_CCT_enable, 0, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG,
	NULL,
	0,
	(UINT8*)&ACDK_CCT_enable,
	sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG pass!");
	pCNF->result.get_6283_ae_gamma_bypass_flag = ACDK_CCT_enable.Enable;
	ACDK_LOGD("[CCAP]:ACDK_CCT_enable.Enable is %d!",ACDK_CCT_enable.Enable);
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AE_GET_METERING_MODE				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	INT32 mode =0;
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_METERING_RESULT,
	NULL,
	0,
	(UINT8*)&mode,
	sizeof(UINT8),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_METERING_MODE driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_METERING_MODE pass!");
	pCNF->result.get_6238_ae_metering_mode = (UINT8)mode;
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AWB_GET_CCM_PARA				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_NVRAM_CCM_PARA CCM_para;
	memset(&CCM_para, 0, sizeof(CCM_para));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_CCM_PARA,
			NULL,
			0,
			(UINT8*)&CCM_para,
			sizeof(CCM_para),
			&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_CCM_PARA driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AWB_GET_CCM_PARA pass!");
	memcpy(&(pCNF->result.get_6238_awb_cmm_para), &CCM_para, sizeof(ACDK_CCT_NVRAM_CCM_PARA));
	pCNF->status = FT_CNF_OK;
	return true;
}

static bool GetItemInfo(ACDK_SENSOR_ITEM_INFO_STRUCT* pMSDK_Data)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
						(UINT8*)pMSDK_Data,
						sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
						(UINT8*)pMSDK_Data,
						sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
						&nRealReadByteCnt)){
		ACDK_LOGD("[CCAP]:GetItemInfo driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:GetItemInfo pass!");
	return true;
}



bool FT_ACDK_CCT_OP_GET_SENSOR_PREGAIN				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	if( -1 == g_FT_CCT_StateMachine.sensor_eng_group_idx )
	{
		ACDK_LOGD("[CCAP] error! -1 == g_FT_CCT_StateMachine.sensor_eng_group_idx");
		return false;
	}

	//memset((void*)pCNF,0,sizeof(pCNF->result.get_sensor_pregain));

	ACDK_SENSOR_ITEM_INFO_STRUCT MSDK_Data;
	memset(&MSDK_Data,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));
	MSDK_Data.GroupIdx = g_FT_CCT_StateMachine.sensor_eng_group_idx;
	MSDK_Data.ItemIdx = g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R;

	if( -1 != g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R && GetItemInfo(&MSDK_Data))
	{
		pCNF->result.get_sensor_pregain.pregain_r.value = MSDK_Data.ItemValue;
		pCNF->result.get_sensor_pregain.pregain_r.min = MSDK_Data.Min;// item_info.min;
		pCNF->result.get_sensor_pregain.pregain_r.max = MSDK_Data.Max;// item_info.max;
		pCNF->result.get_sensor_pregain.pregain_r.exist = KAL_TRUE;
		ACDK_LOGD("[CCAP]: GET_SENSOR_PREGAIN%d%d%d%d!",g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R,MSDK_Data.ItemValue,MSDK_Data.Min,MSDK_Data.Max);
	}

	memset(&MSDK_Data,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));
	MSDK_Data.GroupIdx = g_FT_CCT_StateMachine.sensor_eng_group_idx;
	MSDK_Data.ItemIdx = g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr;

	if( -1 != g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr && GetItemInfo(&MSDK_Data))
	{
		pCNF->result.get_sensor_pregain.pregain_gr.value = MSDK_Data.ItemValue;//item_info.item_value;
		pCNF->result.get_sensor_pregain.pregain_gr.min = MSDK_Data.Min;//item_info.min;
		pCNF->result.get_sensor_pregain.pregain_gr.max = MSDK_Data.Max;//.max;
		pCNF->result.get_sensor_pregain.pregain_gr.exist = KAL_TRUE;
		ACDK_LOGD("[CCAP]: GET_SENSOR_PREGAIN%d%d%d%d!",g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr,MSDK_Data.ItemValue,MSDK_Data.Min,MSDK_Data.Max);
	}

	memset(&MSDK_Data,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));
	MSDK_Data.GroupIdx = g_FT_CCT_StateMachine.sensor_eng_group_idx;
	MSDK_Data.ItemIdx = g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gb;

	if( -1 != g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gb && GetItemInfo(&MSDK_Data))
	{
		//get_sensor_item_info(g_FT_CCT_StateMachine.sensor_eng_group_idx, g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gb, &item_info);
		pCNF->result.get_sensor_pregain.pregain_gb.value = MSDK_Data.ItemValue;//.item_value;
		pCNF->result.get_sensor_pregain.pregain_gb.min = MSDK_Data.Min;//.min;
		pCNF->result.get_sensor_pregain.pregain_gb.max = MSDK_Data.Max;//.max;
		pCNF->result.get_sensor_pregain.pregain_gb.exist = KAL_TRUE;
		ACDK_LOGD("[CCAP]: GET_SENSOR_PREGAIN%d%d%d%d!",g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gb,MSDK_Data.ItemValue,MSDK_Data.Min,MSDK_Data.Max);
	}

	memset(&MSDK_Data,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));
	MSDK_Data.GroupIdx = g_FT_CCT_StateMachine.sensor_eng_group_idx;
	MSDK_Data.ItemIdx = g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_B;

	if( -1 != g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_B && GetItemInfo(&MSDK_Data))
	{
		//get_sensor_item_info(g_FT_CCT_StateMachine.sensor_eng_group_idx, g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_B, &item_info);
		pCNF->result.get_sensor_pregain.pregain_b.value = MSDK_Data.ItemValue;//.item_value;
		pCNF->result.get_sensor_pregain.pregain_b.min = MSDK_Data.Min;//.min;
		pCNF->result.get_sensor_pregain.pregain_b.max = MSDK_Data.Max;//.max;
		pCNF->result.get_sensor_pregain.pregain_b.exist = KAL_TRUE;
		ACDK_LOGD("[CCAP]: GET_SENSOR_PREGAIN%d%d%d%d!",g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_B,MSDK_Data.ItemValue,MSDK_Data.Min,MSDK_Data.Max);
	}

	//return FT_CNF_OK;*/
	return true;
}



bool FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF		(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	//const ispShadingStatusMsg* ptr = (const ispShadingStatusMsg*)&(pREQ->cmd.dev_6238_isp_shading_status);
	ACDK_CCT_MODULE_CTRL_STRUCT ACDK_Data;
	memset(&ACDK_Data,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));


	//MSDK_Data.Enable = (UINT8)ptr->m_switch;
	ACDK_Data.Mode   = (CAMERA_TUNING_SET_ENUM)(pREQ->cmd.dev_6238_isp_shading_status.mode);
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
						(UINT8*)&ACDK_Data,
						sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
						(UINT8*)&ACDK_Data,
						sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
						&nRealReadByteCnt))
						{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF driver failed!");
		return false;
	}
	//ispShadingStatusMsg *ptr = (ispShadingStatusMsg *)&(req->cmd.dev_6238_isp_shading_status);
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF pass!");
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF enable is %d",ACDK_Data.Enable);
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF mode is %d",ACDK_Data.Mode);
	pCNF->result.get_6238_isp_shading_status.m_switch =  ACDK_Data.Enable;
	return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA			(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	winmo_cct_shading_comp_struct CCT_shading;
	memset(&CCT_shading,0,sizeof(winmo_cct_shading_comp_struct));

	//CAMERA_TUNING_SET_ENUM


	ACDK_CCT_SHADING_COMP_STRUCT ACDK_Output;
	memset(&ACDK_Output,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));


	ACDK_Output.pShadingComp = &CCT_shading;
	ACDK_LOGD("Comp mode=%d", g_FT_CCT_StateMachine.comp_mode);
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
						(UINT8*)&(g_FT_CCT_StateMachine.comp_mode),
						sizeof(g_FT_CCT_StateMachine.comp_mode),
						(UINT8*)&ACDK_Output,
						sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
						NULL))
						{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA pass!");
	ACDK_LOGD("Shading mode=%d", ACDK_Output.SHADING_MODE);
	ACDK_LOGD("  - SDBLK_TRIG=%d", ACDK_Output.pShadingComp->SDBLK_TRIG);
	ACDK_LOGD("  - SHADING_EN=%d", ACDK_Output.pShadingComp->SHADING_EN);
	ACDK_LOGD("  - SHADINGBLK_XOFFSET=%d", ACDK_Output.pShadingComp->SHADINGBLK_XOFFSET);
	ACDK_LOGD("  - SHADINGBLK_YOFFSET=%d", ACDK_Output.pShadingComp->SHADINGBLK_YOFFSET);
	ACDK_LOGD("  - SHADINGBLK_XNUM=%d", ACDK_Output.pShadingComp->SHADINGBLK_XNUM);
	ACDK_LOGD("  - SHADINGBLK_YNUM=%d", ACDK_Output.pShadingComp->SHADINGBLK_YNUM);
	ACDK_LOGD("  - SHADINGBLK_WIDTH=%d", ACDK_Output.pShadingComp->SHADINGBLK_WIDTH);
	ACDK_LOGD("  - SHADINGBLK_HEIGHT=%d", ACDK_Output.pShadingComp->SHADINGBLK_HEIGHT);
	ACDK_LOGD("  - SHADING_RADDR=%d", ACDK_Output.pShadingComp->SHADING_RADDR);
	ACDK_LOGD("  - SD_LWIDTH=%d", ACDK_Output.pShadingComp->SD_LWIDTH);
	ACDK_LOGD("  - SD_LHEIGHT=%d", ACDK_Output.pShadingComp->SD_LHEIGHT);
	ACDK_LOGD("  - SDBLK_RATIO00=%d", ACDK_Output.pShadingComp->SDBLK_RATIO00);
	ACDK_LOGD("  - SDBLK_RATIO01=%d", ACDK_Output.pShadingComp->SDBLK_RATIO01);
	ACDK_LOGD("  - SDBLK_RATIO10=%d", ACDK_Output.pShadingComp->SDBLK_RATIO10);
	ACDK_LOGD("  - SDBLK_RATIO11=%d", ACDK_Output.pShadingComp->SDBLK_RATIO11);
	ACDK_LOGD("  - SD_TABLE_SIZE=%d", ACDK_Output.pShadingComp->SD_TABLE_SIZE);
	memcpy((winmo_cct_shading_comp_struct*)&(pCNF->result.get_shading_para),
			ACDK_Output.pShadingComp,
			sizeof(winmo_cct_shading_comp_struct));

	return true;
}


bool FT_ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_AE_MODE_CFG_T MSDK_AE_get_expo;
	memset(&MSDK_AE_get_expo, 0, sizeof(ACDK_AE_MODE_CFG_T));
	memcpy(&MSDK_AE_get_expo, (ACDK_AE_MODE_CFG_T *)&(pREQ->cmd.dev_ae_mode_cfg), sizeof(ACDK_AE_MODE_CFG_T));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
	(UINT8*)&MSDK_AE_get_expo,
	sizeof(ACDK_AE_MODE_CFG_T),
	(UINT8*)&MSDK_AE_get_expo,
	sizeof(ACDK_AE_MODE_CFG_T),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:u4Eposuretime:%d!",MSDK_AE_get_expo.u4Eposuretime);
	ACDK_LOGD("[CCAP]:u4AfeGain:%d! u4IspGain:%d",MSDK_AE_get_expo.u4AfeGain, MSDK_AE_get_expo.u4IspGain);
	ACDK_LOGD("[CCAP]:uFrameRate:%d!",MSDK_AE_get_expo.u2FrameRate);
	ACDK_LOGD("[CCAP]:uFlareGain:%d!",MSDK_AE_get_expo.u2FlareGain);
	ACDK_LOGD("[CCAP]:uFlareValue:%d!",MSDK_AE_get_expo.u2FlareValue);
	memcpy(&(pCNF->result.get_acd_ae_mode_cfg),&MSDK_AE_get_expo,sizeof(ACDK_AE_MODE_CFG_T));
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_OP_AE_GET_CAPTURE_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_AE_MODE_CFG_T MSDK_AE_get_expo;
	memset(&MSDK_AE_get_expo, 0, sizeof(ACDK_AE_MODE_CFG_T));
	memcpy(&MSDK_AE_get_expo, (ACDK_AE_MODE_CFG_T *)&(pREQ->cmd.dev_ae_mode_cfg), sizeof(ACDK_AE_MODE_CFG_T));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
	(UINT8*)&MSDK_AE_get_expo,
	sizeof(ACDK_AE_MODE_CFG_T),
	(UINT8*)&MSDK_AE_get_expo,
	sizeof(ACDK_AE_MODE_CFG_T),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_AE_GET_CAPTURE_PARA driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:u4Eposuretime:%d!",MSDK_AE_get_expo.u4Eposuretime);
	ACDK_LOGD("[CCAP]:u4AfeGain:%d! u4IspGain:%d",MSDK_AE_get_expo.u4AfeGain, MSDK_AE_get_expo.u4IspGain);
	ACDK_LOGD("[CCAP]:uFrameRate:%d!",MSDK_AE_get_expo.u2FrameRate);
	ACDK_LOGD("[CCAP]:uFlareGain:%d!",MSDK_AE_get_expo.u2FlareGain);
	ACDK_LOGD("[CCAP]:uFlareValue:%d!",MSDK_AE_get_expo.u2FlareValue);
	memcpy(&(pCNF->result.get_acd_ae_mode_cfg),&MSDK_AE_get_expo,sizeof(ACDK_AE_MODE_CFG_T));
	pCNF->status = FT_CNF_OK;
	return true;
}



bool FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_CCT_ISP_GET_TUNING_PARAS* pParam = NULL;
	g_PeerBufferLen = sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS);
	g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
	if(g_pPeerBuffer == NULL) return false;
	pParam = (ACDK_CCT_ISP_GET_TUNING_PARAS*)g_pPeerBuffer;
	memset(pParam, 0, sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS));

	UINT32 nRealReadByteCnt = 0;
	ACDK_LOGD("[CCAP]:========== FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS ==========");
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS,
	NULL,
	0,
	(UINT8*)pParam,
	sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS),
	&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS pass!");
	ACDK_LOGD("-------------------------");
	int i,j;
	for (i = 0; i < NVRAM_OBC_TBL_NUM; i++)
	{
		ACDK_LOGD("+ OB[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.OBC[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.OBC[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_NR1_TBL_NUM; i++)
	{
		ACDK_LOGD("+ NR1[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.NR1[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.NR1[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_CFA_TBL_NUM; i++)
	{
		ACDK_LOGD("+ CFA[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.CFA[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.CFA[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_ANR_TBL_NUM; i++)
	{
		ACDK_LOGD("+ ANR[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.ANR[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.ANR[i].set[j]);
		}
	}

	for (i = 0; i < NVRAM_CCR_TBL_NUM; i++)
	{
		ACDK_LOGD("+ CCR[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.CCR[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.CCR[i].set[j]);
		}
	}


	for (i = 0; i < NVRAM_EE_TBL_NUM; i++)
	{
		ACDK_LOGD("+ EE[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.EE[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.EE[i].set[j]);
		}
	}
/*

	for (i = 0; i < NVRAM_NR3D_TBL_NUM; i++)
	{
		ACDK_LOGD("+ NR3D[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.NR3D[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.NR3D[i].set[j]);
		}
	}


	for (i = 0; i < NVRAM_MFB_TBL_NUM; i++)
	{
		ACDK_LOGD("+ MFB[%u]", i);
		for (j = 0; j < pParam->stIspNvramRegs.MFB[i].COUNT; j++)
		{
			ACDK_LOGD("  - set[%u]=%u", j, pParam->stIspNvramRegs.MFB[i].set[j]);
		}
	}
*/
	ACDK_LOGD("-------------------------");
	ACDK_LOGD("[CCAP]:======================================================");

	pCNF->status = FT_CNF_OK;

	return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF		(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	const ispShadingStatusMsg* ptr = (const ispShadingStatusMsg*)&(pREQ->cmd.dev_6238_isp_shading_status);
	ACDK_CCT_MODULE_CTRL_STRUCT MSDK_Data;
	memset(&MSDK_Data,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

	MSDK_Data.Enable = (UINT8)ptr->m_switch;
	MSDK_Data.Mode   = (CAMERA_TUNING_SET_ENUM)ptr->mode;
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF enable is %d",MSDK_Data.Enable);
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF mode is %d",MSDK_Data.Mode);

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF,
						(UINT8*)&MSDK_Data,
						sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
						NULL,
						0,
						NULL))
						{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF pass!");
	return true;
}

bool FT_ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE pass!");
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE pass!");
	pCNF->status = FT_CNF_OK;
	return true;
}



bool FT_ACDK_CCT_V2_OP_SET_OB_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	const ACDK_CCT_FUNCTION_ENABLE_STRUCT* OBCtrl = (const ACDK_CCT_FUNCTION_ENABLE_STRUCT*)&(pREQ->cmd.func_enable);
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_OB_ON_OFF :the Enable = %d",OBCtrl->Enable);
    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
   	if(!bSendDataToCCT(ACDK_CCT_V2_OP_SET_OB_ON_OFF,
                        (UINT8 *)OBCtrl,
                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                        NULL,
                        0,
                        &u4RetLen))
    {
      ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_OB_ON_OFF driver failed!");
			pCNF->status = FT_CNF_FAIL;
			return false;
    }
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_OB_ON_OFF pass!");
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_V2_OP_GET_OB_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_FUNCTION_ENABLE_STRUCT OBCtrlOut;
    memset(&OBCtrlOut,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
   	if(!bSendDataToCCT(ACDK_CCT_V2_OP_GET_OB_ON_OFF,
                       NULL,
                       0,
                       (UINT8*)&OBCtrlOut,
                       sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                       &u4RetLen))
    {
      ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_GET_OB_ON_OFF driver failed!");
			pCNF->status = FT_CNF_FAIL;
			return false;
    }
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_GET_OB_ON_OFF pass!");
    memcpy(&(pCNF->result.get_func_enable),&OBCtrlOut,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_V2_OP_SET_NR_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	const ACDK_CCT_FUNCTION_ENABLE_STRUCT* NRCtrl = (const ACDK_CCT_FUNCTION_ENABLE_STRUCT*)&(pREQ->cmd.func_enable);
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_NR_ON_OFF :Enable = %d",NRCtrl->Enable);
    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
   	if(!bSendDataToCCT(ACDK_CCT_V2_OP_SET_NR_ON_OFF,
                        (UINT8 *)NRCtrl,
                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                        NULL,
                        0,
                        &u4RetLen))
    {
     	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_NR_ON_OFF driver failed!");
			pCNF->status = FT_CNF_FAIL;
			return false;
    }
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_NR_ON_OFF pass!");
		pCNF->status = FT_CNF_OK;
		return true;
}


bool FT_ACDK_CCT_V2_OP_GET_NR_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_FUNCTION_ENABLE_STRUCT NRCtrlOut;
    memset(&NRCtrlOut,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
   	if(!bSendDataToCCT(ACDK_CCT_V2_OP_GET_NR_ON_OFF,
                       NULL,
                       0,
                       (UINT8*)&NRCtrlOut,
                       sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                       &u4RetLen))
    {
    	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_GET_NR_ON_OFF driver failed!");
			pCNF->status = FT_CNF_FAIL;
			return false;
    }
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_GET_NR_ON_OFF pass!");
    memcpy(&(pCNF->result.get_func_enable),&NRCtrlOut,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_V2_OP_SET_EE_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	const ACDK_CCT_FUNCTION_ENABLE_STRUCT* EECtrl = (const ACDK_CCT_FUNCTION_ENABLE_STRUCT*)&(pREQ->cmd.func_enable);
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_EE_ON_OFF :Enable = %d",EECtrl->Enable);
    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
   	if(!bSendDataToCCT(ACDK_CCT_V2_OP_SET_EE_ON_OFF,
                        (UINT8 *)EECtrl,
                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                        NULL,
                        0,
                        &u4RetLen))
    {
    	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_EE_ON_OFF driver failed!");
			pCNF->status = FT_CNF_FAIL;
			return false;
    }
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_SET_EE_ON_OFF pass!");
		pCNF->status = FT_CNF_OK;
		return true;
}


bool FT_ACDK_CCT_V2_OP_GET_EE_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_FUNCTION_ENABLE_STRUCT EECtrlOut;
    memset(&EECtrlOut,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
   	if(!bSendDataToCCT(ACDK_CCT_V2_OP_GET_EE_ON_OFF,
                       NULL,
                       0,
                       (UINT8*)&EECtrlOut,
                       sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                       &u4RetLen))
    {
    	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_GET_EE_ON_OFF driver failed!");
			pCNF->status = FT_CNF_FAIL;
			return false;
    }
    ACDK_LOGD("[CCAP]:FT_ACDK_CCT_V2_OP_GET_EE_ON_OFF pass!");
    memcpy(&(pCNF->result.get_func_enable),&EECtrlOut,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));
		pCNF->status = FT_CNF_OK;
		return true;
}

bool FT_ACDK_CCT_V2_ISP_DEFECT_TABLE_ON(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_CCT_MODULE_CTRL_STRUCT ACDK_MODULE_ctrl_struct;
	memset(&ACDK_MODULE_ctrl_struct,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

	ACDK_MODULE_ctrl_struct.Enable = TRUE;
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_ISP_DEFECT_TABLE_ON,
	(UINT8*)&ACDK_MODULE_ctrl_struct,
	sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_CCT_MODULE_CTRL_STRUCT ACDK_MODULE_ctrl_struct;
	memset(&ACDK_MODULE_ctrl_struct,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

	ACDK_MODULE_ctrl_struct.Enable = FALSE;
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF,
	(UINT8*)&ACDK_MODULE_ctrl_struct,
	sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_CCT_FUNCTION_ENABLE_STRUCT ACDK_get_bypass_onoff;
	memset(&ACDK_get_bypass_onoff,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF,
	NULL,
	0,
	(UINT8*)&ACDK_get_bypass_onoff,
	sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
	&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->result.get_6238_isp_dynamic_bypass_mode_on_off = ACDK_get_bypass_onoff.Enable;
	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM	(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 u4RetLen = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM,NULL,0,NULL,0,&u4RetLen))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}




bool FT_ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
        ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS");
	ACDK_CCT_FUNCTION_ENABLE_STRUCT ccmStatus;
    memset(&ccmStatus, 0, sizeof(ccmStatus));

    UINT32 u4RetLen = 0;
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS,
			NULL,
			0,
			(UINT8 *)&ccmStatus,
			sizeof(ccmStatus),
			&u4RetLen))
	{
		pCNF->status = FT_CNF_FAIL;
		ACDK_LOGE("ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS driver error!");
		return false;
	}
    memcpy(&(pCNF->result.get_func_enable), &ccmStatus, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));
    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS pass:CCM status is %d", pCNF->result.get_func_enable.Enable);
    pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
//@ 89 tmp mark
/*
	UINT32 u4Index = (UINT32)(pREQ->cmd.awb_get_nvram_ccm_index);
    ACDK_CCT_CCM_STRUCT ccm;
    memset(&ccm, 0, sizeof(ccm));
    UINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM:index=%u", u4Index);
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM,
			(UINT8*)&u4Index,
		   	sizeof(u4Index),
		   	(UINT8*)&ccm,
		   	sizeof(ccm),
		   	&u4RetLen))
	{
		pCNF->status = FT_CNF_FAIL;
		ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM driver error!");
		return false;
	}

    memcpy(&(pCNF->result.get_6238_awb_nvram_ccm), &(ccm), sizeof(ACDK_CCT_CCM_STRUCT));
    pCNF->status = FT_CNF_OK;
	ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM pass!");
    ACDK_LOGD("Get NVram CCM");
    ACDK_LOGD("Light Mode:%u", u4Index);
    ACDK_LOGD("CCM Matrix");
    ACDK_LOGD("M11 M12 M13 : 0x%03X 0x%03X 0x%03X", ccm.M11, ccm.M12, ccm.M13);
    ACDK_LOGD("M21 M22 M23 : 0x%03X 0x%03X 0x%03X", ccm.M21, ccm.M22, ccm.M23)
    ACDK_LOGD("M31 M32 M33 : 0x%03X 0x%03X 0x%03X", ccm.M31, ccm.M32, ccm.M33);
    */
    return true;
}

bool FT_ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM");
	ACDK_CCT_SET_NVRAM_CCM inCCM;
	memcpy(&inCCM, &pREQ->cmd.awb_set_nvram_ccm_para, sizeof(ACDK_CCT_SET_NVRAM_CCM));
    UINT32 u4RetLen = 0;

	ACDK_LOGD("0x%03x, 0x%03x, 0x%03x", inCCM.ccm.M11, inCCM.ccm.M12, inCCM.ccm.M13);
    ACDK_LOGD("0x%03x, 0x%03x, 0x%03x", inCCM.ccm.M21, inCCM.ccm.M22, inCCM.ccm.M23);
    ACDK_LOGD("0x%03x, 0x%03x, 0x%03x", inCCM.ccm.M31, inCCM.ccm.M32, inCCM.ccm.M33);

    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM,
			(UINT8*)&inCCM,
			sizeof(inCCM),
			NULL,
			0,
 			&u4RetLen))
	{
		ACDK_LOGD("FT_ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM driver error!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM pass!");
    pCNF->status = FT_CNF_OK;
    return true;
}


bool FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA");
	ACDK_CCT_NVRAM_CCM_PARA ccm_para;
    memset(&ccm_para, 0, sizeof(ccm_para));
	memcpy(&ccm_para, &(pREQ->cmd.dev_6238_awb_cmm_para), sizeof(ACDK_CCT_NVRAM_CCM_PARA));
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA,
			(UINT8*)&ccm_para,
			sizeof(ccm_para),
			NULL,
			0,
			&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		ACDK_LOGE("FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA driver error!");
		return false;
	}

	pCNF->status = FT_CNF_OK;
        ACDK_LOGD("FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA pass!");
	return true;
}


bool FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS");
    ACDK_CCT_FUNCTION_ENABLE_STRUCT ccmStatus;
    memset(&ccmStatus, 0, sizeof(ccmStatus));
	memcpy(&ccmStatus, &(pREQ->cmd.func_enable), sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    UINT32 u4RetLen = 0;
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS,
			(UINT8*)&ccmStatus,
			sizeof(ccmStatus),
			NULL,
			0,
			&u4RetLen))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
    pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS");
	ACDK_CCT_FUNCTION_ENABLE_STRUCT MSDK_enable_struct;
	memset(&MSDK_enable_struct,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

	MSDK_enable_struct.Enable = (META_BOOL)pREQ->cmd.dev_6238_ae_gamma_bypass;
	UINT32 nRealReadByteCnt = 0;
	ACDK_LOGD("MSDK_enable_struct.Enable is %d",MSDK_enable_struct.Enable);
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS,
	(UINT8*)&MSDK_enable_struct,
	sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}

	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE	(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_LOGD("FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE");
	ACDK_CCT_GAMMA_TABLE_STRUCT MSDK_get_gamma_table;
	memset(&MSDK_get_gamma_table,0,sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE,
											NULL,
											0,
											(UINT8*)&MSDK_get_gamma_table,
											sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT),
											&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	memcpy(&(pCNF->result.get_6238_ae_gamma_table),&MSDK_get_gamma_table,sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));
	pCNF->status = FT_CNF_OK;
	return true;
}



bool FT_ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE	(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_LOGD("FT_ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE");
	ACDK_CCT_GAMMA_TABLE_STRUCT MSDK_get_gamma_table;
	memset(&MSDK_get_gamma_table,0,sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));
	memcpy(&MSDK_get_gamma_table,&(pREQ->cmd.dev_ae_gamma_table),sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE,
											(UINT8 *)&MSDK_get_gamma_table,
											sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT),
											NULL,
											0,
											&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}




bool FT_ACDK_CCT_OP_AE_GET_ENABLE_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_OP_AE_GET_ENABLE_INFO");

    UINT32 u4RetLen = 0;
    UINT32 u4AEEnableInfo = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_AE_GET_ENABLE_INFO,
                        NULL,
                        0,
                        (UINT8 *)&u4AEEnableInfo,
                        sizeof(u4AEEnableInfo),
                        &u4RetLen))
		{
			pCNF->status = FT_CNF_FAIL;
			return false;
		}
	ACDK_LOGD("AE Status=%d", u4AEEnableInfo);
		pCNF->result.get_ae_enable_info = u4AEEnableInfo;
		pCNF->status = FT_CNF_OK;
		return true;

}



bool FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

	ACDK_CCT_SHADING_COMP_STRUCT ACDK_data;
	memset(&ACDK_data,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));
	ACDK_data.SHADING_MODE = g_FT_CCT_StateMachine.comp_mode;

	winmo_cct_shading_comp_struct Cct_shading;
	memset(&Cct_shading,0,sizeof(winmo_cct_shading_comp_struct));
	memcpy(&Cct_shading,(const winmo_cct_shading_comp_struct *)&pREQ->cmd.set_shading_para,sizeof(winmo_cct_shading_comp_struct));

	ACDK_data.pShadingComp = &Cct_shading;
	UINT32 nRealReadByteCnt = 0;

	ACDK_LOGD("Shading mode=%d", ACDK_data.SHADING_MODE);
	ACDK_LOGD("  - SDBLK_TRIG=%d", ACDK_data.pShadingComp->SDBLK_TRIG);
	ACDK_LOGD("  - SHADING_EN=%d", ACDK_data.pShadingComp->SHADING_EN);
	ACDK_LOGD("  - SHADINGBLK_XOFFSET=%d", ACDK_data.pShadingComp->SHADINGBLK_XOFFSET);
	ACDK_LOGD("  - SHADINGBLK_YOFFSET=%d", ACDK_data.pShadingComp->SHADINGBLK_YOFFSET);
	ACDK_LOGD("  - SHADINGBLK_XNUM=%d", ACDK_data.pShadingComp->SHADINGBLK_XNUM);
	ACDK_LOGD("  - SHADINGBLK_YNUM=%d", ACDK_data.pShadingComp->SHADINGBLK_YNUM);
	ACDK_LOGD("  - SHADINGBLK_WIDTH=%d", ACDK_data.pShadingComp->SHADINGBLK_WIDTH);
	ACDK_LOGD("  - SHADINGBLK_HEIGHT=%d", ACDK_data.pShadingComp->SHADINGBLK_HEIGHT);
	ACDK_LOGD("  - SHADING_RADDR=%d", ACDK_data.pShadingComp->SHADING_RADDR);
	ACDK_LOGD("  - SD_LWIDTH=%d", ACDK_data.pShadingComp->SD_LWIDTH);
	ACDK_LOGD("  - SD_LHEIGHT=%d", ACDK_data.pShadingComp->SD_LHEIGHT);
	ACDK_LOGD("  - SDBLK_RATIO00=%d", ACDK_data.pShadingComp->SDBLK_RATIO00);
	ACDK_LOGD("  - SDBLK_RATIO01=%d", ACDK_data.pShadingComp->SDBLK_RATIO01);
	ACDK_LOGD("  - SDBLK_RATIO10=%d", ACDK_data.pShadingComp->SDBLK_RATIO10);
	ACDK_LOGD("  - SDBLK_RATIO11=%d", ACDK_data.pShadingComp->SDBLK_RATIO11);
	ACDK_LOGD("  - SD_TABLE_SIZE=%d", ACDK_data.pShadingComp->SD_TABLE_SIZE);
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA,
						(UINT8*)&ACDK_data,
						sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
						NULL,
						0,
						&nRealReadByteCnt))
						{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}



bool FT_ACDK_CCT_V2_OP_AE_SET_SCENE_MODE				(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	#if 0
	kal_bool current_ae_enable = g_FT_CCT_StateMachine.ae_enable;
	if(!g_FT_CCT_StateMachine.ae_enable) {
		FT_MSDK_CCT_V2_OP_AE_ENABLE(pREQ,pCNF,pBuff);
	}
	#endif
	INT32  scene_mode = (INT32)(pREQ->cmd.dev_6238_ae_scene_mode);

	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE,(UINT8*)&scene_mode,sizeof(INT32),NULL,0,&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AE_SET_METERING_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	INT32 MeteringMode = 0;
	MeteringMode = (INT32)(pREQ->cmd.dev_6238_ae_metering_mode);

	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_SET_METERING_MODE,
	(UINT8*)&MeteringMode,
	sizeof(UINT32),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}

	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO");
	UINT32 u4RetLen = 0;
	ACDK_AE_MODE_CFG_T rAEExpPara;

	memset(&rAEExpPara,0,sizeof(ACDK_AE_MODE_CFG_T));
	memcpy(&rAEExpPara,(ACDK_AE_MODE_CFG_T *)&(pREQ->cmd.dev_ae_mode_cfg),sizeof(ACDK_AE_MODE_CFG_T));

  ACDK_LOGD("Expoure time:%d", rAEExpPara.u4Eposuretime);
  ACDK_LOGD("AFE Gain:%d Isp Gain:%d", rAEExpPara.u4AfeGain, rAEExpPara.u4IspGain);
  ACDK_LOGD("Flare:%d Flare Gain:%d",rAEExpPara.u2FlareValue, rAEExpPara.u2FlareGain);
    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
  if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,
                      (UINT8 *)&rAEExpPara,
                      sizeof(ACDK_AE_MODE_CFG_T),
                      NULL,
                      0,
                      &u4RetLen))
  {
		pCNF->status = FT_CNF_FAIL;
		return false;
  }
  pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_OP_AE_SET_CAPTURE_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_AE_SET_CAPTURE_PARA");
	UINT32 u4RetLen = 0;
	ACDK_AE_MODE_CFG_T rAEExpPara;

	memset(&rAEExpPara,0,sizeof(ACDK_AE_MODE_CFG_T));
	memcpy(&rAEExpPara,(ACDK_AE_MODE_CFG_T *)&(pREQ->cmd.dev_ae_mode_cfg),sizeof(ACDK_AE_MODE_CFG_T));

  ACDK_LOGD("Expoure time:%d", rAEExpPara.u4Eposuretime);
  ACDK_LOGD("AFE Gain:%d Isp Gain:%d", rAEExpPara.u4AfeGain, rAEExpPara.u4IspGain);
  ACDK_LOGD("Flare:%d Flare Gain:%d",rAEExpPara.u2FlareValue, rAEExpPara.u2FlareGain);
    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
  if(!bSendDataToCCT(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                      (UINT8 *)&rAEExpPara,
                      sizeof(ACDK_AE_MODE_CFG_T),
                      NULL,
                      0,
                      &u4RetLen))
  {
		pCNF->status = FT_CNF_FAIL;
		return false;
  }
  pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_AE_SELECT_BAND(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_SELECT_BAND");
	kal_bool current_ae_enable = g_FT_CCT_StateMachine.ae_enable;

	// should enable AE
	if(!g_FT_CCT_StateMachine.ae_enable) {
		FT_ACDK_CCT_OP_AE_ENABLE(NULL,NULL,NULL);
	}
	INT32 Band = 0;
	Band = (INT32)(pREQ->cmd.dev_6238_ae_select_band);
	ACDK_LOGD("BAND is %d",Band);
	UINT32 nRealReadByteCnt = 0;

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AE_SELECT_BAND,
	(UINT8*)&Band,
	sizeof(INT32),
	NULL,
	0,
	&nRealReadByteCnt))
	{
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	if(current_ae_enable !=  g_FT_CCT_StateMachine.ae_enable){
			FT_ACDK_CCT_OP_AE_DISABLE(NULL,NULL,NULL);
	}
	pCNF->status = FT_CNF_OK;
	return true;
}



/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AWB_SET_AWB_MODE
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_OP_AWB_SET_AWB_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_OP_AWB_SET_AWB_MODE");

    UINT32 u4RetLen = 0;


    UINT32 u4AWBMode = (UINT32)(pREQ->cmd.dev_set_awb_mode);

    ACDK_LOGD("AWBMode = %d", u4AWBMode);


    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_AWB_SET_AWB_MODE,
                        (UINT8 *)&u4AWBMode,
                        sizeof(UINT32),
                        NULL,
                        0,
                        &u4RetLen))
    {
 			pCNF->status = FT_CNF_FAIL;
			return false;
    }

		pCNF->status = FT_CNF_OK;
		return true;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AWB_GET_AWB_MODE
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_OP_AWB_GET_AWB_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_OP_AWB_GET_AWB_MODE");

    UINT32 u4RetLen = 0;


    UINT32 u4AWBMode;
    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_AWB_GET_AWB_MODE,
                        NULL,
                        0,
                        (UINT8 *)&u4AWBMode,
                        sizeof(UINT32),
                        &u4RetLen))
    {
 			pCNF->status = FT_CNF_FAIL;
			return false;
    }
		ACDK_LOGD("AWBMode = %d", u4AWBMode);
		pCNF->result.get_awb_mode = u4AWBMode;
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO");

    UINT32 u4RetLen = 0;
    INT32 i4AWBEnable;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,
                        NULL,
                        0,
                        (UINT8 *)&i4AWBEnable,
                        sizeof(INT32),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    ACDK_LOGD("AUTO RUN INFO = %d", i4AWBEnable);
    pCNF->result.get_auto_run_info = i4AWBEnable;
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_V2_OP_AF_OPERATION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_AF_OPERATION");

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_OPERATION,
                        NULL,
                        0,
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		return true;
}


bool FT_ACDK_CCT_V2_OP_MF_OPERATION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_MF_OPERATION");
		INT32 i4MFPos = (INT32)(pREQ->cmd.dev_mf_operation);
    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_MF_OPERATION,
                        (UINT8 *)&i4MFPos,
                        sizeof(i4MFPos),
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_V2_OP_AF_GET_BEST_POS(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_BEST_POS");

		INT32 i4AFBestPos;

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_GET_BEST_POS,
                        NULL,
                        0,
                        (UINT8*)&i4AFBestPos,
                        sizeof(i4AFBestPos),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		ACDK_LOGD("The best pos is: = %d", i4AFBestPos);
		pCNF->result.get_af_best_pos = i4AFBestPos;
		return true;
}


bool FT_ACDK_CCT_V2_OP_AF_GET_RANGE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_AF_GET_RANGE");

		FOCUS_RANGE_T sFocusRange;

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_GET_RANGE,
                        NULL,
                        0,
                        (UINT8*)&sFocusRange,
                        sizeof(sFocusRange),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		ACDK_LOGD("The macro poa is: = %d", sFocusRange.i4MacroPos);
		ACDK_LOGD("The inf pos is: = %d", sFocusRange.i4InfPos);
		memcpy(&(pCNF->result.get_focus_range),&sFocusRange,sizeof(FOCUS_RANGE_T));
		return true;
}


bool FT_ACDK_CCT_OP_FLASH_ENABLE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_OP_FLASH_ENABLE");

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_FLASH_ENABLE,
                        NULL,
                        0,
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		return true;
}


bool FT_ACDK_CCT_OP_FLASH_DISABLE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_OP_FLASH_DISABLE");

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_FLASH_DISABLE,
                        NULL,
                        0,
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		return true;
}



bool FT_ACDK_CCT_OP_FLASH_GET_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("FT_ACDK_CCT_OP_FLASH_GET_INFO");

		INT32 i4FlashEnable;

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_FLASH_GET_INFO,
                        NULL,
                        0,
                        (UINT8*)&i4FlashEnable,
                        sizeof(INT32),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		pCNF->status = FT_CNF_OK;
		ACDK_LOGD("The flash info is: = %d", i4FlashEnable);
		pCNF->result.get_flash_enable = i4FlashEnable;
		return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3");
	ispTable* p_ShadingTable = NULL;
	g_PeerBufferLen = sizeof(ispTable);
	g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
	if(g_pPeerBuffer == NULL) return false;
	p_ShadingTable = (ispTable*)g_pPeerBuffer;
	memset(p_ShadingTable, 0, sizeof(ispTable));


	ispShadingParaV3 *request = (ispShadingParaV3 *)(&pREQ->cmd.dev_6238_isp_shading_para_v3);
	ispShadingParaV3 *result = (ispShadingParaV3 *)(&pCNF->result.get_6238_isp_shading_table_para_v3);
	result->mode = request->mode;
	result->offset = request->offset;
	result->length = request->length;
	result->color_temperature = request->color_temperature;


	ACDK_CCT_TABLE_SET_STRUCT MSDK_Table;
	memset(&MSDK_Table,0,sizeof(ACDK_CCT_TABLE_SET_STRUCT));
	MSDK_Table.Length = request->length;
	MSDK_Table.Mode = (CAMERA_TUNING_SET_ENUM)request->mode;
	MSDK_Table.pBuffer = p_ShadingTable->data;
	MSDK_Table.Offset = request->offset;
	MSDK_Table.ColorTemp = request->color_temperature;


	UINT32 nRealReadByteCnt = 0;
	ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3 enter");
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,
											(UINT8*)&MSDK_Table,
                       sizeof(ACDK_CCT_TABLE_SET_STRUCT),
											(UINT8*)&MSDK_Table,
                       sizeof(ACDK_CCT_TABLE_SET_STRUCT),
											&nRealReadByteCnt))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }

	 for (MINT32 j = 0; j  < MSDK_Table.Length; j=j+4)
        	{
            		ACDK_LOGD("0x%08x 0x%08x 0x%08x 0x%08x \n", MSDK_Table.pBuffer [j],MSDK_Table.pBuffer [j+1],MSDK_Table.pBuffer [j+2],MSDK_Table.pBuffer [j+3]);
        	}




    ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3 exit");
    ACDK_LOGE("[CCAP]out buffer size:%d",sizeof(ACDK_CCT_TABLE_SET_STRUCT));

		pCNF->status = FT_CNF_OK;
		return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
		ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3");
		if(NULL == *pBuff)
		{
			pCNF->status = FT_CNF_FAIL;
			return false;
		}

		ispShadingParaV3 cmd = pREQ->cmd.dev_6238_isp_shading_para_v3;

		ispTable IspTableData;
		memset(&IspTableData,0,sizeof(ispTable));
		memcpy(&IspTableData,(ispTable*)(*pBuff),sizeof(ispTable));

		ACDK_CCT_TABLE_SET_STRUCT MSDK_able_set;
		memset(&MSDK_able_set,0,sizeof(ACDK_CCT_TABLE_SET_STRUCT));
		MSDK_able_set.Length = cmd.length;
		MSDK_able_set.Mode = (CAMERA_TUNING_SET_ENUM)cmd.mode;
		MSDK_able_set.pBuffer = IspTableData.data;
		MSDK_able_set.Offset = cmd.offset;
		MSDK_able_set.ColorTemp = cmd.color_temperature;

		for (MINT32 j = 0; j  < MSDK_able_set.Length; j=j+4)
        	{
            		ACDK_LOGD("0x%08x 0x%08x 0x%08x 0x%08x \n", MSDK_able_set.pBuffer [j],MSDK_able_set.pBuffer [j+1],MSDK_able_set.pBuffer [j+2],MSDK_able_set.pBuffer [j+3]);
        	}

		UINT32 nRealReadByteCnt = 0;
		ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3 enter");
		if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
												(UINT8*)&MSDK_able_set,
												sizeof(ACDK_CCT_TABLE_SET_STRUCT),
												NULL,
												0,
												&nRealReadByteCnt))
		{
			pCNF->status = FT_CNF_FAIL;
			return false;
		}

		ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3 exit");
		ACDK_LOGE("[CCAP]Real out byte Cnt :%d",nRealReadByteCnt);


	pCNF->status = FT_CNF_OK;
	return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF");
	ispTable* p_ShadingTable = NULL;
	g_PeerBufferLen = sizeof(ispTable);
	g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
	if(g_pPeerBuffer == NULL) return false;
	p_ShadingTable = (ispTable*)g_pPeerBuffer;
	memset(p_ShadingTable, 0, sizeof(ispTable));


	ispShadingParaV3 *request = (ispShadingParaV3 *)(&pREQ->cmd.dev_6238_isp_shading_para_v3);
	ispShadingParaV3 *result = (ispShadingParaV3 *)(&pCNF->result.get_6238_isp_shading_table_para_v3);
	result->mode = request->mode;
	result->offset = request->offset;
	result->length = request->length;
	result->color_temperature = request->color_temperature;


	ACDK_CCT_TABLE_SET_STRUCT MSDK_Table;
	memset(&MSDK_Table,0,sizeof(ACDK_CCT_TABLE_SET_STRUCT));
	MSDK_Table.Length = request->length;
	MSDK_Table.Mode = (CAMERA_TUNING_SET_ENUM)request->mode;
	MSDK_Table.pBuffer = p_ShadingTable->data;
	MSDK_Table.Offset = request->offset;
	MSDK_Table.ColorTemp = request->color_temperature;


	UINT32 nRealReadByteCnt = 0;
	ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF enter");
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,
											(UINT8*)&MSDK_Table,
                       sizeof(ACDK_CCT_TABLE_SET_STRUCT),
											(UINT8*)&MSDK_Table,
                       sizeof(ACDK_CCT_TABLE_SET_STRUCT),
											&nRealReadByteCnt))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF exit");
    ACDK_LOGE("[CCAP]out buffer size :%d",sizeof(ACDK_CCT_TABLE_SET_STRUCT));

		pCNF->status = FT_CNF_OK;
		return true;
}


bool FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
		ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF");
		if(NULL == *pBuff)
		{
			pCNF->status = FT_CNF_FAIL;
			return false;
		}

		ispShadingParaV3 cmd = pREQ->cmd.dev_6238_isp_shading_para_v3;

		ispTable IspTableData;
		memset(&IspTableData,0,sizeof(ispTable));
		memcpy(&IspTableData,(ispTable*)(*pBuff),sizeof(ispTable));

		ACDK_CCT_TABLE_SET_STRUCT MSDK_able_set;
		memset(&MSDK_able_set,0,sizeof(ACDK_CCT_TABLE_SET_STRUCT));
		MSDK_able_set.Length = cmd.length;
		MSDK_able_set.Mode = (CAMERA_TUNING_SET_ENUM)cmd.mode;
		MSDK_able_set.pBuffer = IspTableData.data;
		MSDK_able_set.Offset = cmd.offset;
		MSDK_able_set.ColorTemp = cmd.color_temperature;

		for (MINT32 j = 0; j  < MSDK_able_set.Length; j=j+4)
        	{
            		ACDK_LOGD("0x%08x 0x%08x 0x%08x 0x%08x \n", MSDK_able_set.pBuffer [j],MSDK_able_set.pBuffer [j+1],MSDK_able_set.pBuffer [j+2],MSDK_able_set.pBuffer [j+3]);
        	}

		UINT32 nRealReadByteCnt = 0;
		ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF enter");
		if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
												(UINT8*)&MSDK_able_set,
												sizeof(ACDK_CCT_TABLE_SET_STRUCT),
												NULL,
												0,
												&nRealReadByteCnt))
		{
			pCNF->status = FT_CNF_FAIL;
			return false;
		}
		ACDK_LOGD("[CCAP] ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF exit");
    		ACDK_LOGE("[CCAP]Real out byte Cnt  :%d",nRealReadByteCnt);

	pCNF->status = FT_CNF_OK;
	return true;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_AF_INFO
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_GET_AF_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_GET_AF_INFO");

		ACDK_AF_INFO_T sAFInfo;
		memset(&sAFInfo,0,sizeof(ACDK_AF_INFO_T));

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_GET_AF_INFO,
														NULL,
														0,
														(UINT8*)&sAFInfo,
														sizeof(sAFInfo),
                            &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Info : [AFMode] %d, [AFMeter] %d, [Curr Pos] %d", sAFInfo.i4AFMode, sAFInfo.i4AFMeter, sAFInfo.i4CurrPos);
		pCNF->status = FT_CNF_OK;
		memcpy(&(pCNF->result.get_af_info),&sAFInfo,sizeof(ACDK_AF_INFO_T));
		return true;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_CALI_OPERATION
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_AF_CALI_OPERATION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_CALI_OPERATION");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_AF_CALI_DATA_T *pCaliData = NULL;
		g_PeerBufferLen = sizeof(ACDK_AF_CALI_DATA_T);
		g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
		if(g_pPeerBuffer == NULL) return false;
		pCaliData = (ACDK_AF_CALI_DATA_T*)g_pPeerBuffer;

    memset (pCaliData, 0, sizeof(ACDK_AF_CALI_DATA_T));


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_CALI_OPERATION,
                        NULL,
                        0,
							         (UINT8*)pCaliData,
							          sizeof(ACDK_AF_CALI_DATA_T),
                        &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Best Pos = %d", pCaliData->i4BestPos);
		pCNF->status = FT_CNF_OK;

		return true;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_SET_RANGE
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_AF_SET_RANGE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_AF_SET_RANGE");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

		FOCUS_RANGE_T sFocusRange;
		memset(&sFocusRange,0,sizeof(FOCUS_RANGE_T));
		memcpy(&sFocusRange,&(pREQ->cmd.dev_focus_range),sizeof(FOCUS_RANGE_T));

    ACDK_LOGD("Focus Range = %d to %d", sFocusRange.i4InfPos, sFocusRange.i4MacroPos);


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_SET_RANGE,
                        (UINT8*)&sFocusRange,
                        sizeof(sFocusRange),
                        NULL,
                        0,
                        &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }

		pCNF->status = FT_CNF_OK;
		return true;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_FV
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_AF_GET_FV(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_FV");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	ACDK_AF_POS_T sAFPos;
	memset(&sAFPos,0,sizeof(ACDK_AF_POS_T));
	memcpy(&sAFPos,&(pREQ->cmd.dev_af_pos),sizeof(ACDK_AF_POS_T));


	ACDK_AF_VLU_T sAFVlu;
	memset(&sAFVlu,0,sizeof(ACDK_AF_VLU_T));
	ACDK_AF_VLU_32T sAFVlu32;
	memset(&sAFVlu32,0,sizeof(ACDK_AF_VLU_32T));


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_GET_FV,
														(UINT8*)&sAFPos,
														sizeof(sAFPos),
														(UINT8*)&sAFVlu,
														sizeof(sAFVlu),
                            &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
	ACDK_LOGD("[Num] %4d", sAFPos.i4Num);
		for (INT32 i=0; i<sAFVlu.i4Num; i++)
		{
    	ACDK_LOGD("[Pos] %4d, [Vlu] %lld", sAFPos.i4Pos[i], sAFVlu.i8Vlu[i]);
		}
    pCNF->status = FT_CNF_OK;

	sAFVlu32.i4Num = sAFVlu.i4Num;
	for (INT32 i=0; i<sAFVlu.i4Num; i++)
	{
		sAFVlu32.i4Vlu[i*2] = sAFVlu.i8Vlu[i] & 0x0000FFFF;
		sAFVlu32.i4Vlu[i*2+1] = (sAFVlu.i8Vlu[i] & 0xFFFF0000) >> 16;
	}

	for (INT32 i=0; i<sAFVlu.i4Num; i++)
	{
    	ACDK_LOGD("[Pos] %4d, [Vlu32] %12d, [Vlu32] %l2d", sAFPos.i4Pos[i], sAFVlu32.i4Vlu[i*2], sAFVlu32.i4Vlu[i*2+1]);
	}
		memcpy(&(pCNF->result.get_af_pos),&sAFVlu32,sizeof(ACDK_AF_VLU_32T));
		return true;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_READ
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_AF_READ(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_READ");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

		NVRAM_LENS_PARA_STRUCT sLensPara;
		memset(&sLensPara,0,sizeof(NVRAM_LENS_PARA_STRUCT));


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_READ,
                        NULL,
                        0,
												(UINT8*)&sLensPara,
												sizeof(sLensPara),
                        &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("[Read AF Para]");
    ACDK_LOGD("[Version]%d", sLensPara.Version);
    //ACDK_LOGD("[sZoomDest.i4NormalNum]%d", sLensPara.rAFNVRAM.sZoomDest[0].sExactSrch.i4NormalNum);
    //ACDK_LOGD("[i4TUNE_PARA1]%d", sLensPara.rAFNVRAM.i4TUNE_PARA1);
    //ACDK_LOGD("[i4TUNE_PARA2]%d", sLensPara.rAFNVRAM.i4TUNE_PARA2);
    //ACDK_LOGD("[i4TUNE_PARA3]%d", sLensPara.rAFNVRAM.i4TUNE_PARA3);
    //ACDK_LOGD("[Thres Sub]%d", sLensPara.rAFNVRAM.i4AF_THRES_SUB);
    //ACDK_LOGD("[Thres Offset]%d", sLensPara.rAFNVRAM.i4AF_THRES_OFFSET);
    ACDK_LOGD("[i4CHANGE_CNT_DELTA]%d", sLensPara.rAFNVRAM.i4CHANGE_CNT_DELTA);
    ACDK_LOGD("[i4LV_THRES]%d", sLensPara.rAFNVRAM.i4LV_THRES);
    ACDK_LOGD("[i4SPOT_PERCENT_W]%d", sLensPara.rAFNVRAM.i4SPOT_PERCENT_W);
    ACDK_LOGD("[i4SPOT_PERCENT_H]%d", sLensPara.rAFNVRAM.i4SPOT_PERCENT_H);
    ACDK_LOGD("[i4InfPos]%d", sLensPara.rAFNVRAM.i4InfPos);
    ACDK_LOGD("[i4AFC_STEP_SIZE]%d", sLensPara.rAFNVRAM.i4AFC_STEP_SIZE);
    ACDK_LOGD("[i4BackJumpPos]%d", sLensPara.rAFNVRAM.i4BackJumpPos);
    ACDK_LOGD("[Inf Pos]%d", sLensPara.rFocusRange.i4InfPos);
    ACDK_LOGD("[Macro Pos]%d", sLensPara.rFocusRange.i4MacroPos);
    pCNF->status = FT_CNF_OK;
		memcpy(&(pCNF->result.get_af_lens_para),&sLensPara,sizeof(NVRAM_LENS_PARA_STRUCT));
		return true;


}



/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_APPLY
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_AF_APPLY(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_APPLY");
    NVRAM_LENS_PARA_STRUCT sLensPara;


    ACDK_LOGD("[real Version]%d", pREQ->cmd.dev_af_lens_para.Version);
    ACDK_LOGD("[real Inf Pos]%d", pREQ->cmd.dev_af_lens_para.rFocusRange.i4InfPos);
    ACDK_LOGD("[real Macro Pos]%d", pREQ->cmd.dev_af_lens_para.rFocusRange.i4MacroPos);
		//memset(&sLensPara,0,sizeof(NVRAM_LENS_PARA_STRUCT));
		memcpy(&sLensPara,&(pREQ->cmd.dev_af_lens_para),sizeof(NVRAM_LENS_PARA_STRUCT));
		ACDK_LOGD("[Read AF size:%d]",sizeof(NVRAM_LENS_PARA_STRUCT));

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_LOGD("[Read AF Para]");
    ACDK_LOGD("[Version]%d", sLensPara.Version);
    ACDK_LOGD("[Inf Pos]%d", sLensPara.rFocusRange.i4InfPos);
    ACDK_LOGD("[Macro Pos]%d", sLensPara.rFocusRange.i4MacroPos);

    ACDK_LOGD("[i4VAFC_FAIL_CNT]%d", sLensPara.rAFNVRAM.i4VAFC_FAIL_CNT);
    ACDK_LOGD("[i4CHANGE_CNT_DELTA]%d", sLensPara.rAFNVRAM.i4CHANGE_CNT_DELTA);
    ACDK_LOGD("[i4LV_THRES]%d", sLensPara.rAFNVRAM.i4LV_THRES);
    ACDK_LOGD("[i4SPOT_PERCENT_W]%d", sLensPara.rAFNVRAM.i4SPOT_PERCENT_W);
    ACDK_LOGD("[i4SPOT_PERCENT_H]%d", sLensPara.rAFNVRAM.i4SPOT_PERCENT_H);
    ACDK_LOGD("[i4InfPos]%d", sLensPara.rAFNVRAM.i4InfPos);
    ACDK_LOGD("[i4AFC_STEP_SIZE]%d", sLensPara.rAFNVRAM.i4AFC_STEP_SIZE);
    ACDK_LOGD("[i4BackJumpPos]%d", sLensPara.rAFNVRAM.i4BackJumpPos);
    //ACDK_LOGD("[sZoomDest.i4NormalNum]%d", sLensPara.rAFNVRAM.sZoomDest[0].sExactSrch.i4NormalNum);
    //ACDK_LOGD("[i4TUNE_PARA1]%d", sLensPara.rAFNVRAM.i4TUNE_PARA1);
    //ACDK_LOGD("[i4TUNE_PARA2]%d", sLensPara.rAFNVRAM.i4TUNE_PARA2);
    //ACDK_LOGD("[i4TUNE_PARA3]%d", sLensPara.rAFNVRAM.i4TUNE_PARA3);
    //ACDK_LOGD("[Thres Sub]%d", sLensPara.rAFNVRAM.i4AF_THRES_SUB);
    //ACDK_LOGD("[Thres Offset]%d", sLensPara.rAFNVRAM.i4AF_THRES_OFFSET);


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_APPLY,
														(UINT8*)&sLensPara,
														sizeof(sLensPara),
                            NULL,
                            0,
                            &u4RetLen))
     {
     	pCNF->status = FT_CNF_FAIL;
		 	return false;
     }

    pCNF->status = FT_CNF_OK;
		return true;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SHADING_CAL
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_SHADING_CAL(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_SHADING_CAL");


    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    UINT32 u4RetLen = 0;
    ACDK_CCT_LSC_CAL_SET_STRUCT rLSCCalSet;
    /*
    rLSCCalSet.mode = (CAMERA_TUNING_SET_ENUM)(UINT8)atoi(a_pprArgv[0]);
    rLSCCalSet.colorTemp = (UINT8) atoi(a_pprArgv[1]);
    rLSCCalSet.boundaryEndX = 20;
    rLSCCalSet.boundaryEndY = 20;
    rLSCCalSet.boundaryStartX = 20;
    rLSCCalSet.boundaryStartY = 20;
    rLSCCalSet.attnRatio = 1;
    */
		memcpy(&rLSCCalSet,&(pREQ->cmd.dev_lsc_cal_set),sizeof(ACDK_CCT_LSC_CAL_SET_STRUCT));

    ACDK_LOGD("rLSCCalSet.mode :%d", rLSCCalSet.mode);
    ACDK_LOGD("rLSCCalSet.colorTemp  :%d", rLSCCalSet.colorTemp);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_SHADING_CAL,
                        (UINT8 *)&rLSCCalSet,
                        sizeof(ACDK_CCT_LSC_CAL_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
		 	return false;
    }
    pCNF->status = FT_CNF_OK;
		return true;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_INFO
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_OP_DEV_AE_GET_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    AE_NVRAM_T rAENVRAM;

    memset(&rAENVRAM,0, sizeof(AE_NVRAM_T));

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_INFO");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_GET_INFO,
                        NULL,
                        0,
                        (UINT8 *)&rAENVRAM,
                        sizeof(AE_NVRAM_T),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================



    // TEST ONLY (check AE parameter)
    ACDK_LOGD("u4MinGain = %d", rAENVRAM.rDevicesInfo.u4MinGain);
    ACDK_LOGD("u4MaxGain = %d", rAENVRAM.rDevicesInfo.u4MaxGain);
    ACDK_LOGD("u4MiniISOGain = %d", rAENVRAM.rDevicesInfo.u4MiniISOGain);
    ACDK_LOGD("u4GainStepUnit = %d", rAENVRAM.rDevicesInfo.u4GainStepUnit);
    ACDK_LOGD("u4PreExpUnit = %d", rAENVRAM.rDevicesInfo.u4PreExpUnit);
    ACDK_LOGD("u4PreMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4PreMaxFrameRate);
    ACDK_LOGD("u4VideoExpUnit = %d", rAENVRAM.rDevicesInfo.u4VideoExpUnit);
    ACDK_LOGD("u4VideoMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4VideoMaxFrameRate);
    ACDK_LOGD("u4Video2PreRatio = %d", rAENVRAM.rDevicesInfo.u4Video2PreRatio);
    ACDK_LOGD("u4CapExpUnit = %d", rAENVRAM.rDevicesInfo.u4CapExpUnit);
    ACDK_LOGD("u4CapMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4CapMaxFrameRate);
    ACDK_LOGD("u4Cap2PreRatio = %d", rAENVRAM.rDevicesInfo.u4Cap2PreRatio);
    ACDK_LOGD("u4LensFno = %d", rAENVRAM.rDevicesInfo.u4LensFno);

    ACDK_LOGD("u4HistHighThres = %d", rAENVRAM.rHistConfig.u4HistHighThres);
    ACDK_LOGD("u4HistLowThres = %d", rAENVRAM.rHistConfig.u4HistLowThres);
    ACDK_LOGD("u4MostBrightRatio = %d", rAENVRAM.rHistConfig.u4MostBrightRatio);
    ACDK_LOGD("u4MostDarkRatio = %d", rAENVRAM.rHistConfig.u4MostDarkRatio);
    ACDK_LOGD("u4CentralHighBound = %d", rAENVRAM.rHistConfig.u4CentralHighBound);
    ACDK_LOGD("u4CentralLowBound = %d", rAENVRAM.rHistConfig.u4CentralLowBound);
    ACDK_LOGD("u4OverExpThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4OverExpThres[0], rAENVRAM.rHistConfig.u4OverExpThres[1],
                                     rAENVRAM.rHistConfig.u4OverExpThres[2], rAENVRAM.rHistConfig.u4OverExpThres[3], rAENVRAM.rHistConfig.u4OverExpThres[4]);
    ACDK_LOGD("u4HistStretchThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4HistStretchThres[0], rAENVRAM.rHistConfig.u4HistStretchThres[1],
                                     rAENVRAM.rHistConfig.u4HistStretchThres[2], rAENVRAM.rHistConfig.u4HistStretchThres[3], rAENVRAM.rHistConfig.u4HistStretchThres[4]);
    ACDK_LOGD("u4BlackLightThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4BlackLightThres[0], rAENVRAM.rHistConfig.u4BlackLightThres[1],
                                     rAENVRAM.rHistConfig.u4BlackLightThres[2], rAENVRAM.rHistConfig.u4BlackLightThres[3], rAENVRAM.rHistConfig.u4BlackLightThres[4]);
    ACDK_LOGD("bEnableBlackLight = %d", rAENVRAM.rCCTConfig.bEnableBlackLight);
    ACDK_LOGD("bEnableHistStretch = %d", rAENVRAM.rCCTConfig.bEnableHistStretch);
    ACDK_LOGD("bEnableAntiOverExposure = %d", rAENVRAM.rCCTConfig.bEnableAntiOverExposure);
    ACDK_LOGD("bEnableTimeLPF = %d", rAENVRAM.rCCTConfig.bEnableTimeLPF);
    ACDK_LOGD("bEnableCaptureThres = %d", rAENVRAM.rCCTConfig.bEnableCaptureThres);
    ACDK_LOGD("u4AETarget = %d", rAENVRAM.rCCTConfig.u4AETarget);
    ACDK_LOGD("u4InitIndex = %d", rAENVRAM.rCCTConfig.u4InitIndex);
    ACDK_LOGD("u4BackLightWeight = %d", rAENVRAM.rCCTConfig.u4BackLightWeight);
    ACDK_LOGD("u4HistStretchWeight = %d", rAENVRAM.rCCTConfig.u4HistStretchWeight);
    ACDK_LOGD("u4AntiOverExpWeight = %d", rAENVRAM.rCCTConfig.u4AntiOverExpWeight);
    ACDK_LOGD("u4BlackLightStrengthIndex = %d", rAENVRAM.rCCTConfig.u4BlackLightStrengthIndex);
    ACDK_LOGD("u4HistStretchStrengthIndex = %d", rAENVRAM.rCCTConfig.u4HistStretchStrengthIndex);
    ACDK_LOGD("u4AntiOverExpStrengthIndex = %d", rAENVRAM.rCCTConfig.u4AntiOverExpStrengthIndex);
    ACDK_LOGD("u4TimeLPFStrengthIndex = %d", rAENVRAM.rCCTConfig.u4TimeLPFStrengthIndex);
    ACDK_LOGD("u4LPFConvergeLevel[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rCCTConfig.u4LPFConvergeLevel[0], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[1],
                                     rAENVRAM.rCCTConfig.u4LPFConvergeLevel[2], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[3], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[4]);
    ACDK_LOGD("u4InDoorEV = %d", rAENVRAM.rCCTConfig.u4InDoorEV);
    ACDK_LOGD("i4BVOffset = %d", rAENVRAM.rCCTConfig.i4BVOffset);
    ACDK_LOGD("u4PreviewFlareOffset = %d", rAENVRAM.rCCTConfig.u4PreviewFlareOffset);
    ACDK_LOGD("u4CaptureFlareOffset = %d", rAENVRAM.rCCTConfig.u4CaptureFlareOffset);
    ACDK_LOGD("u4CaptureFlareThres = %d", rAENVRAM.rCCTConfig.u4CaptureFlareThres);
    memcpy(&(pCNF->result.get_ae_nvram_info),&rAENVRAM,sizeof(AE_NVRAM_T));
    pCNF->status = FT_CNF_OK;
    return true;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_APPLY_INFO
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_OP_DEV_AE_APPLY_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    // AE NVRAM test data
    AE_NVRAM_T rAENVRAM;

    memcpy(&rAENVRAM,&(pREQ->cmd.dev_ae_nvram_info),sizeof(AE_NVRAM_T));
        // TEST ONLY (check AE parameter)
    ACDK_LOGD("u4MinGain = %d", rAENVRAM.rDevicesInfo.u4MinGain);
    ACDK_LOGD("u4MaxGain = %d", rAENVRAM.rDevicesInfo.u4MaxGain);
    ACDK_LOGD("u4MiniISOGain = %d", rAENVRAM.rDevicesInfo.u4MiniISOGain);
    ACDK_LOGD("u4GainStepUnit = %d", rAENVRAM.rDevicesInfo.u4GainStepUnit);
    ACDK_LOGD("u4PreExpUnit = %d", rAENVRAM.rDevicesInfo.u4PreExpUnit);
    ACDK_LOGD("u4PreMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4PreMaxFrameRate);
    ACDK_LOGD("u4VideoExpUnit = %d", rAENVRAM.rDevicesInfo.u4VideoExpUnit);
    ACDK_LOGD("u4VideoMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4VideoMaxFrameRate);
    ACDK_LOGD("u4Video2PreRatio = %d", rAENVRAM.rDevicesInfo.u4Video2PreRatio);
    ACDK_LOGD("u4CapExpUnit = %d", rAENVRAM.rDevicesInfo.u4CapExpUnit);
    ACDK_LOGD("u4CapMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4CapMaxFrameRate);
    ACDK_LOGD("u4Cap2PreRatio = %d", rAENVRAM.rDevicesInfo.u4Cap2PreRatio);
    ACDK_LOGD("u4LensFno = %d", rAENVRAM.rDevicesInfo.u4LensFno);

    ACDK_LOGD("u4HistHighThres = %d", rAENVRAM.rHistConfig.u4HistHighThres);
    ACDK_LOGD("u4HistLowThres = %d", rAENVRAM.rHistConfig.u4HistLowThres);
    ACDK_LOGD("u4MostBrightRatio = %d", rAENVRAM.rHistConfig.u4MostBrightRatio);
    ACDK_LOGD("u4MostDarkRatio = %d", rAENVRAM.rHistConfig.u4MostDarkRatio);
    ACDK_LOGD("u4CentralHighBound = %d", rAENVRAM.rHistConfig.u4CentralHighBound);
    ACDK_LOGD("u4CentralLowBound = %d", rAENVRAM.rHistConfig.u4CentralLowBound);
    ACDK_LOGD("u4OverExpThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4OverExpThres[0], rAENVRAM.rHistConfig.u4OverExpThres[1],
                                     rAENVRAM.rHistConfig.u4OverExpThres[2], rAENVRAM.rHistConfig.u4OverExpThres[3], rAENVRAM.rHistConfig.u4OverExpThres[4]);
    ACDK_LOGD("u4HistStretchThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4HistStretchThres[0], rAENVRAM.rHistConfig.u4HistStretchThres[1],
                                     rAENVRAM.rHistConfig.u4HistStretchThres[2], rAENVRAM.rHistConfig.u4HistStretchThres[3], rAENVRAM.rHistConfig.u4HistStretchThres[4]);
    ACDK_LOGD("u4BlackLightThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4BlackLightThres[0], rAENVRAM.rHistConfig.u4BlackLightThres[1],
                                     rAENVRAM.rHistConfig.u4BlackLightThres[2], rAENVRAM.rHistConfig.u4BlackLightThres[3], rAENVRAM.rHistConfig.u4BlackLightThres[4]);
    ACDK_LOGD("bEnableBlackLight = %d", rAENVRAM.rCCTConfig.bEnableBlackLight);
    ACDK_LOGD("bEnableHistStretch = %d", rAENVRAM.rCCTConfig.bEnableHistStretch);
    ACDK_LOGD("bEnableAntiOverExposure = %d", rAENVRAM.rCCTConfig.bEnableAntiOverExposure);
    ACDK_LOGD("bEnableTimeLPF = %d", rAENVRAM.rCCTConfig.bEnableTimeLPF);
    ACDK_LOGD("bEnableCaptureThres = %d", rAENVRAM.rCCTConfig.bEnableCaptureThres);
    ACDK_LOGD("u4AETarget = %d", rAENVRAM.rCCTConfig.u4AETarget);
    ACDK_LOGD("u4InitIndex = %d", rAENVRAM.rCCTConfig.u4InitIndex);
    ACDK_LOGD("u4BackLightWeight = %d", rAENVRAM.rCCTConfig.u4BackLightWeight);
    ACDK_LOGD("u4HistStretchWeight = %d", rAENVRAM.rCCTConfig.u4HistStretchWeight);
    ACDK_LOGD("u4AntiOverExpWeight = %d", rAENVRAM.rCCTConfig.u4AntiOverExpWeight);
    ACDK_LOGD("u4BlackLightStrengthIndex = %d", rAENVRAM.rCCTConfig.u4BlackLightStrengthIndex);
    ACDK_LOGD("u4HistStretchStrengthIndex = %d", rAENVRAM.rCCTConfig.u4HistStretchStrengthIndex);
    ACDK_LOGD("u4AntiOverExpStrengthIndex = %d", rAENVRAM.rCCTConfig.u4AntiOverExpStrengthIndex);
    ACDK_LOGD("u4TimeLPFStrengthIndex = %d", rAENVRAM.rCCTConfig.u4TimeLPFStrengthIndex);
    ACDK_LOGD("u4LPFConvergeLevel[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rCCTConfig.u4LPFConvergeLevel[0], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[1],
                                     rAENVRAM.rCCTConfig.u4LPFConvergeLevel[2], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[3], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[4]);
    ACDK_LOGD("u4InDoorEV = %d", rAENVRAM.rCCTConfig.u4InDoorEV);
    ACDK_LOGD("i4BVOffset = %d", rAENVRAM.rCCTConfig.i4BVOffset);
    ACDK_LOGD("u4PreviewFlareOffset = %d", rAENVRAM.rCCTConfig.u4PreviewFlareOffset);
    ACDK_LOGD("u4CaptureFlareOffset = %d", rAENVRAM.rCCTConfig.u4CaptureFlareOffset);
    ACDK_LOGD("u4CaptureFlareThres = %d", rAENVRAM.rCCTConfig.u4CaptureFlareThres);

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_APPLY_INFO");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_APPLY_INFO,
                        (UINT8 *)&rAENVRAM,
                        sizeof(AE_NVRAM_T),
                        NULL,
                        0,
                        &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    pCNF->status = FT_CNF_OK;
    return true;
}



#if 0
bool FT_ACDK_CCT_OP_DEV_AE_APPLY_MANUAL_INFO(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    // AE NVRAM test data
    AE_NVRAM_T rAENVRAM;

    memcpy(&rAENVRAM,&(pREQ->cmd.dev_ae_nvram_info),sizeof(AE_NVRAM_T));
        // TEST ONLY (check AE parameter)
    ACDK_LOGD("u4MinGain = %d", rAENVRAM.rDevicesInfo.u4MinGain);
    ACDK_LOGD("u4MaxGain = %d", rAENVRAM.rDevicesInfo.u4MaxGain);
    ACDK_LOGD("u4MiniISOGain = %d", rAENVRAM.rDevicesInfo.u4MiniISOGain);
    ACDK_LOGD("u4GainStepUnit = %d", rAENVRAM.rDevicesInfo.u4GainStepUnit);
    ACDK_LOGD("u4PreExpUnit = %d", rAENVRAM.rDevicesInfo.u4PreExpUnit);
    ACDK_LOGD("u4PreMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4PreMaxFrameRate);
    ACDK_LOGD("u4VideoExpUnit = %d", rAENVRAM.rDevicesInfo.u4VideoExpUnit);
    ACDK_LOGD("u4VideoMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4VideoMaxFrameRate);
    ACDK_LOGD("u4Video2PreRatio = %d", rAENVRAM.rDevicesInfo.u4Video2PreRatio);
    ACDK_LOGD("u4CapExpUnit = %d", rAENVRAM.rDevicesInfo.u4CapExpUnit);
    ACDK_LOGD("u4CapMaxFrameRate = %d", rAENVRAM.rDevicesInfo.u4CapMaxFrameRate);
    ACDK_LOGD("u4Cap2PreRatio = %d", rAENVRAM.rDevicesInfo.u4Cap2PreRatio);
    ACDK_LOGD("u4LensFno = %d", rAENVRAM.rDevicesInfo.u4LensFno);

    ACDK_LOGD("u4HistHighThres = %d", rAENVRAM.rHistConfig.u4HistHighThres);
    ACDK_LOGD("u4HistLowThres = %d", rAENVRAM.rHistConfig.u4HistLowThres);
    ACDK_LOGD("u4MostBrightRatio = %d", rAENVRAM.rHistConfig.u4MostBrightRatio);
    ACDK_LOGD("u4MostDarkRatio = %d", rAENVRAM.rHistConfig.u4MostDarkRatio);
    ACDK_LOGD("u4CentralHighBound = %d", rAENVRAM.rHistConfig.u4CentralHighBound);
    ACDK_LOGD("u4CentralLowBound = %d", rAENVRAM.rHistConfig.u4CentralLowBound);
    ACDK_LOGD("u4OverExpThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4OverExpThres[0], rAENVRAM.rHistConfig.u4OverExpThres[1],
                                     rAENVRAM.rHistConfig.u4OverExpThres[2], rAENVRAM.rHistConfig.u4OverExpThres[3], rAENVRAM.rHistConfig.u4OverExpThres[4]);
    ACDK_LOGD("u4HistStretchThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4HistStretchThres[0], rAENVRAM.rHistConfig.u4HistStretchThres[1],
                                     rAENVRAM.rHistConfig.u4HistStretchThres[2], rAENVRAM.rHistConfig.u4HistStretchThres[3], rAENVRAM.rHistConfig.u4HistStretchThres[4]);
    ACDK_LOGD("u4BlackLightThres[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rHistConfig.u4BlackLightThres[0], rAENVRAM.rHistConfig.u4BlackLightThres[1],
                                     rAENVRAM.rHistConfig.u4BlackLightThres[2], rAENVRAM.rHistConfig.u4BlackLightThres[3], rAENVRAM.rHistConfig.u4BlackLightThres[4]);
    ACDK_LOGD("bEnableBlackLight = %d", rAENVRAM.rCCTConfig.bEnableBlackLight);
    ACDK_LOGD("bEnableHistStretch = %d", rAENVRAM.rCCTConfig.bEnableHistStretch);
    ACDK_LOGD("bEnableAntiOverExposure = %d", rAENVRAM.rCCTConfig.bEnableAntiOverExposure);
    ACDK_LOGD("bEnableTimeLPF = %d", rAENVRAM.rCCTConfig.bEnableTimeLPF);
    ACDK_LOGD("bEnableCaptureThres = %d", rAENVRAM.rCCTConfig.bEnableCaptureThres);
    ACDK_LOGD("u4AETarget = %d", rAENVRAM.rCCTConfig.u4AETarget);
    ACDK_LOGD("u4InitIndex = %d", rAENVRAM.rCCTConfig.u4InitIndex);
    ACDK_LOGD("u4BackLightWeight = %d", rAENVRAM.rCCTConfig.u4BackLightWeight);
    ACDK_LOGD("u4HistStretchWeight = %d", rAENVRAM.rCCTConfig.u4HistStretchWeight);
    ACDK_LOGD("u4AntiOverExpWeight = %d", rAENVRAM.rCCTConfig.u4AntiOverExpWeight);
    ACDK_LOGD("u4BlackLightStrengthIndex = %d", rAENVRAM.rCCTConfig.u4BlackLightStrengthIndex);
    ACDK_LOGD("u4HistStretchStrengthIndex = %d", rAENVRAM.rCCTConfig.u4HistStretchStrengthIndex);
    ACDK_LOGD("u4AntiOverExpStrengthIndex = %d", rAENVRAM.rCCTConfig.u4AntiOverExpStrengthIndex);
    ACDK_LOGD("u4TimeLPFStrengthIndex = %d", rAENVRAM.rCCTConfig.u4TimeLPFStrengthIndex);
    ACDK_LOGD("u4LPFConvergeLevel[5] = {%d, %d, %d, %d, %d}", rAENVRAM.rCCTConfig.u4LPFConvergeLevel[0], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[1],
                                     rAENVRAM.rCCTConfig.u4LPFConvergeLevel[2], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[3], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[4]);
    ACDK_LOGD("u4InDoorEV = %d", rAENVRAM.rCCTConfig.u4InDoorEV);
    ACDK_LOGD("u4BVOffset = %d", rAENVRAM.rCCTConfig.u4BVOffset);
    ACDK_LOGD("u4PreviewFlareOffset = %d", rAENVRAM.rCCTConfig.u4PreviewFlareOffset);
    ACDK_LOGD("u4CaptureFlareOffset = %d", rAENVRAM.rCCTConfig.u4CaptureFlareOffset);
    ACDK_LOGD("u4CaptureFlareThres = %d", rAENVRAM.rCCTConfig.u4CaptureFlareThres);

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_APPLY_MANUAL_INFO");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_APPLY_MANUAL_INFO,
                        (UINT8 *)&rAENVRAM,
                        sizeof(AE_NVRAM_T),
                        NULL,
                        0,
                        &u4RetLen))
    {
     	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    pCNF->status = FT_CNF_OK;
    return true;
}
#endif

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    UINT32 u4RetLen = 0;
    INT32 i4AEcurrentEVValue = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION,
                        NULL,
                        0,
                        (UINT8 *)&i4AEcurrentEVValue,
                        sizeof(INT32),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		ACDK_LOGD("AE current EV value = %d", i4AEcurrentEVValue);
		pCNF->result.get_ae_current_ev_value = i4AEcurrentEVValue;
    pCNF->status = FT_CNF_OK;
    return true;

}


bool FT_ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	// AWB NVRAM test data
		AWB_NVRAM_T rAWBNVRAMTestData;

    memcpy(&rAWBNVRAMTestData,&(pREQ->cmd.dev_awb_nvram_info),sizeof(AWB_NVRAM_T));
    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2,
                        (UINT8 *)&rAWBNVRAMTestData,
                        sizeof(AWB_NVRAM_T),
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    pCNF->status = FT_CNF_OK;
    return true;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_AWB_PARA
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_AWB_GET_AWB_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    AWB_NVRAM_T rAWBNVRAM; // TEST ONLY

    memset(&rAWBNVRAM,0, sizeof(rAWBNVRAM)); // TEST ONLY


    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_AWB_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_AWB_PARA,
                                NULL,
                                0,
                                (UINT8 *)&rAWBNVRAM,
                                sizeof(AWB_NVRAM_T),
                                &u4RetLen))
    	{
    		pCNF->status = FT_CNF_FAIL;
		return false;
    	}

    // TEST ONLY (check AWB parameter)
    ACDK_LOGD("rUnitGain.i4R = %d\n", rAWBNVRAM.rCalData.rUnitGain.i4R);
    ACDK_LOGD("rUnitGain.i4G = %d\n", rAWBNVRAM.rCalData.rUnitGain.i4G);
    ACDK_LOGD("rUnitGain.i4B = %d\n", rAWBNVRAM.rCalData.rUnitGain.i4B);
    ACDK_LOGD("rGoldenGain.i4R = %d\n", rAWBNVRAM.rCalData.rGoldenGain.i4R);
    ACDK_LOGD("rGoldenGain.i4G = %d\n", rAWBNVRAM.rCalData.rGoldenGain.i4G);
    ACDK_LOGD("rGoldenGain.i4B = %d\n", rAWBNVRAM.rCalData.rGoldenGain.i4B);
    ACDK_LOGD("rTuningUnitGain.i4R = %d\n", rAWBNVRAM.rCalData.rTuningUnitGain.i4R);
    ACDK_LOGD("rTuningUnitGain.i4G = %d\n", rAWBNVRAM.rCalData.rTuningUnitGain.i4G);
    ACDK_LOGD("rTuningUnitGain.i4B = %d\n", rAWBNVRAM.rCalData.rTuningUnitGain.i4B);
    ACDK_LOGD("rD65Gain.i4R = %d\n", rAWBNVRAM.rCalData.rD65Gain.i4R);
    ACDK_LOGD("rD65Gain.i4G = %d\n", rAWBNVRAM.rCalData.rD65Gain.i4G);
    ACDK_LOGD("rD65Gain.i4B = %d\n", rAWBNVRAM.rCalData.rD65Gain.i4B);


    ACDK_LOGD("rOriginalXY.rStrobe.i4X = %d\n", rAWBNVRAM.rOriginalXY.rStrobe.i4X);
    ACDK_LOGD("rOriginalXY.rStrobe.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rStrobe.i4Y);
    ACDK_LOGD("rOriginalXY.rHorizon.i4X = %d\n", rAWBNVRAM.rOriginalXY.rHorizon.i4X);
    ACDK_LOGD("rOriginalXY.rHorizon.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rHorizon.i4Y);
    ACDK_LOGD("rOriginalXY.rA.i4X = %d\n", rAWBNVRAM.rOriginalXY.rA.i4X);
    ACDK_LOGD("rOriginalXY.rA.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rA.i4Y);
    ACDK_LOGD("rOriginalXY.rTL84.i4X = %d\n", rAWBNVRAM.rOriginalXY.rTL84.i4X);
    ACDK_LOGD("rOriginalXY.rTL84.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rTL84.i4Y);
    ACDK_LOGD("rOriginalXY.rCWF.i4X = %d\n", rAWBNVRAM.rOriginalXY.rCWF.i4X);
    ACDK_LOGD("rOriginalXY.rCWF.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rCWF.i4Y);
    ACDK_LOGD("rOriginalXY.rDNP.i4X = %d\n", rAWBNVRAM.rOriginalXY.rDNP.i4X);
    ACDK_LOGD("rOriginalXY.rDNP.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rDNP.i4Y);
    ACDK_LOGD("rOriginalXY.rD65.i4X = %d\n", rAWBNVRAM.rOriginalXY.rD65.i4X);
    ACDK_LOGD("rOriginalXY.rD65.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rD65.i4Y);
    ACDK_LOGD("rOriginalXY.rDF.i4X = %d\n", rAWBNVRAM.rOriginalXY.rDF.i4X);
    ACDK_LOGD("rOriginalXY.rDF.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rDF.i4Y);

    ACDK_LOGD("rRotatedXY.rStrobe.i4X = %d\n", rAWBNVRAM.rRotatedXY.rStrobe.i4X);
    ACDK_LOGD("rRotatedXY.rStrobe.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rStrobe.i4Y);
    ACDK_LOGD("rRotatedXY.rHorizon.i4X = %d\n", rAWBNVRAM.rRotatedXY.rHorizon.i4X);
    ACDK_LOGD("rRotatedXY.rHorizon.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rHorizon.i4Y);
    ACDK_LOGD("rRotatedXY.rA.i4X = %d\n", rAWBNVRAM.rRotatedXY.rA.i4X);
    ACDK_LOGD("rRotatedXY.rA.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rA.i4Y);
    ACDK_LOGD("rRotatedXY.rTL84.i4X = %d\n", rAWBNVRAM.rRotatedXY.rTL84.i4X);
    ACDK_LOGD("rRotatedXY.rTL84.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rTL84.i4Y);
    ACDK_LOGD("rRotatedXY.rCWF.i4X = %d\n", rAWBNVRAM.rRotatedXY.rCWF.i4X);
    ACDK_LOGD("rRotatedXY.rCWF.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rCWF.i4Y);
    ACDK_LOGD("rRotatedXY.rDNP.i4X = %d\n", rAWBNVRAM.rRotatedXY.rDNP.i4X);
    ACDK_LOGD("rRotatedXY.rDNP.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rDNP.i4Y);
    ACDK_LOGD("rRotatedXY.rD65.i4X = %d\n", rAWBNVRAM.rRotatedXY.rD65.i4X);
    ACDK_LOGD("rRotatedXY.rD65.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rD65.i4Y);
    ACDK_LOGD("rRotatedXY.rDF.i4X = %d\n", rAWBNVRAM.rRotatedXY.rDF.i4X);
    ACDK_LOGD("rRotatedXY.rDF.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rDF.i4Y);

    ACDK_LOGD("rStrobe.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rStrobe.i4R);
    ACDK_LOGD("rStrobe.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rStrobe.i4G);
    ACDK_LOGD("rStrobe.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rStrobe.i4B);
    ACDK_LOGD("rHorizon.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rHorizon.i4R);
    ACDK_LOGD("rHorizon.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rHorizon.i4G);
    ACDK_LOGD("rHorizon.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rHorizon.i4B);
    ACDK_LOGD("rA.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rA.i4R);
    ACDK_LOGD("rA.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rA.i4G);
    ACDK_LOGD("rA.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rA.i4B);
    ACDK_LOGD("rTL84.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rTL84.i4R);
    ACDK_LOGD("rTL84.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rTL84.i4G);
    ACDK_LOGD("rTL84.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rTL84.i4B);
    ACDK_LOGD("rCWF.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rCWF.i4R);
    ACDK_LOGD("rCWF.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rCWF.i4G);
    ACDK_LOGD("rCWF.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rCWF.i4B);
    ACDK_LOGD("rDNP.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rDNP.i4R);
    ACDK_LOGD("rDNP.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rDNP.i4G);
    ACDK_LOGD("rDNP.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rDNP.i4B);
    ACDK_LOGD("rD65.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rD65.i4R);
    ACDK_LOGD("rD65.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rD65.i4G);
    ACDK_LOGD("rD65.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rD65.i4B);
    ACDK_LOGD("rDF.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rDF.i4R);
    ACDK_LOGD("rDF.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rDF.i4G);
    ACDK_LOGD("rDF.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rDF.i4B);

    ACDK_LOGD("rRotationMatrix.i4RotationAngle = %d\n", rAWBNVRAM.rRotationMatrix.i4RotationAngle);
    ACDK_LOGD("rRotationMatrix.i4Cos = %d\n", rAWBNVRAM.rRotationMatrix.i4Cos);
    ACDK_LOGD("rRotationMatrix.i4Sin = %d\n", rAWBNVRAM.rRotationMatrix.i4Sin);

    ACDK_LOGD("rDaylightLocus.i4SlopeNumerator = %d\n", rAWBNVRAM.rDaylightLocus.i4SlopeNumerator);
    ACDK_LOGD("rDaylightLocus.i4SlopeDenominator = %d\n", rAWBNVRAM.rDaylightLocus.i4SlopeDenominator);

    ACDK_LOGD("rAWBLightArea.rStrobe.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rStrobe.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rStrobe.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rStrobe.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LowerBound);

    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4LowerBound);

    ACDK_LOGD("rPWBDefaultGain.rDaylight.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.i4R);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.i4G);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.i4B);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.i4R);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.i4G);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.i4B);
    ACDK_LOGD("rPWBDefaultGain.rShade.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.i4R);
    ACDK_LOGD("rPWBDefaultGain.rShade.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.i4G);
    ACDK_LOGD("rPWBDefaultGain.rShade.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.i4B);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.i4R);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.i4G);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.i4B);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.i4R);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.i4G);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.i4B);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.i4R);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.i4G);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.i4B);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.i4R);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.i4G);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.i4B);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.i4R);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.i4G);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.i4B);

    ACDK_LOGD("rPreferenceColor.rTungsten.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rTungsten.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rTungsten.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rTungsten.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rShade.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rShade.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rShade.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rShade.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.i4R);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.i4G);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Strobe.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Strobe.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Strobe.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Strobe.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Strobe.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Strobe.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4B);

    ACDK_LOGD("rCCTEstimation.i4CCT[0] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[0]);
    ACDK_LOGD("rCCTEstimation.i4CCT[1] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[1]);
    ACDK_LOGD("rCCTEstimation.i4CCT[2] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[2]);
    ACDK_LOGD("rCCTEstimation.i4CCT[3] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[3]);
    ACDK_LOGD("rCCTEstimation.i4CCT[4] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[4]);

    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[0] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[0]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[1] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[1]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[2] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[2]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[3] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[3]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[4] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[4]);

    memcpy(&(pCNF->result.get_awb_nvram_info),&rAWBNVRAM,sizeof(AWB_NVRAM_T));
    pCNF->status = FT_CNF_OK;
    return true;



//@ 89 tmp mark
/*
    AWB_NVRAM_T rAWBNVRAM;

    memset(&rAWBNVRAM,0, sizeof(rAWBNVRAM));

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_AWB_PARA");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_AWB_PARA,
                        NULL,
                        0,
                        (UINT8 *)&rAWBNVRAM,
                        sizeof(AWB_NVRAM_T),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
		memcpy(&(pCNF->result.get_awb_nvram_info),&rAWBNVRAM,sizeof(AWB_NVRAM_T));
    pCNF->status = FT_CNF_OK;
    // TEST ONLY (check AWB parameter)
    ACDK_LOGD("rCalGain.u4R = %d", rAWBNVRAM.rCalData.rCalGain.u4R);
    ACDK_LOGD("rCalGain.u4G = %d", rAWBNVRAM.rCalData.rCalGain.u4G);
    ACDK_LOGD("rCalGain.u4B = %d", rAWBNVRAM.rCalData.rCalGain.u4B);
    ACDK_LOGD("rDefGain.u4R = %d", rAWBNVRAM.rCalData.rDefGain.u4R);
    ACDK_LOGD("rDefGain.u4G = %d", rAWBNVRAM.rCalData.rDefGain.u4G);
    ACDK_LOGD("rDefGain.u4B = %d", rAWBNVRAM.rCalData.rDefGain.u4B);
    ACDK_LOGD("rD65Gain.u4R = %d", rAWBNVRAM.rCalData.rD65Gain.u4R);
    ACDK_LOGD("rD65Gain.u4G = %d", rAWBNVRAM.rCalData.rD65Gain.u4G);
    ACDK_LOGD("rD65Gain.u4B = %d", rAWBNVRAM.rCalData.rD65Gain.u4B);

    ACDK_LOGD("rOriginalXY.rHorizon.i4X = %d", rAWBNVRAM.rOriginalXY.rHorizon.i4X);
    ACDK_LOGD("rOriginalXY.rHorizon.i4Y = %d", rAWBNVRAM.rOriginalXY.rHorizon.i4Y);
    ACDK_LOGD("rOriginalXY.rA.i4X = %d", rAWBNVRAM.rOriginalXY.rA.i4X);
    ACDK_LOGD("rOriginalXY.rA.i4Y = %d", rAWBNVRAM.rOriginalXY.rA.i4Y);
    ACDK_LOGD("rOriginalXY.rTL84.i4X = %d", rAWBNVRAM.rOriginalXY.rTL84.i4X);
    ACDK_LOGD("rOriginalXY.rTL84.i4Y = %d", rAWBNVRAM.rOriginalXY.rTL84.i4Y);
    ACDK_LOGD("rOriginalXY.rCWF.i4X = %d", rAWBNVRAM.rOriginalXY.rCWF.i4X);
    ACDK_LOGD("rOriginalXY.rCWF.i4Y = %d", rAWBNVRAM.rOriginalXY.rCWF.i4Y);
    ACDK_LOGD("rOriginalXY.rDNP.i4X = %d", rAWBNVRAM.rOriginalXY.rDNP.i4X);
    ACDK_LOGD("rOriginalXY.rDNP.i4Y = %d", rAWBNVRAM.rOriginalXY.rDNP.i4Y);
    ACDK_LOGD("rOriginalXY.rD65.i4X = %d", rAWBNVRAM.rOriginalXY.rD65.i4X);
    ACDK_LOGD("rOriginalXY.rD65.i4Y = %d", rAWBNVRAM.rOriginalXY.rD65.i4Y);
    ACDK_LOGD("rOriginalXY.rD75.i4X = %d", rAWBNVRAM.rOriginalXY.rD75.i4X);
    ACDK_LOGD("rOriginalXY.rD75.i4Y = %d", rAWBNVRAM.rOriginalXY.rD75.i4Y);
    ACDK_LOGD("rOriginalXY.rDF.i4X = %d\n", rAWBNVRAM.rOriginalXY.rDF.i4X);
    ACDK_LOGD("rOriginalXY.rDF.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rDF.i4Y);

    ACDK_LOGD("rRotatedXY.rHorizon.i4X = %d", rAWBNVRAM.rRotatedXY.rHorizon.i4X);
    ACDK_LOGD("rRotatedXY.rHorizon.i4Y = %d", rAWBNVRAM.rRotatedXY.rHorizon.i4Y);
    ACDK_LOGD("rRotatedXY.rA.i4X = %d", rAWBNVRAM.rRotatedXY.rA.i4X);
    ACDK_LOGD("rRotatedXY.rA.i4Y = %d", rAWBNVRAM.rRotatedXY.rA.i4Y);
    ACDK_LOGD("rRotatedXY.rTL84.i4X = %d", rAWBNVRAM.rRotatedXY.rTL84.i4X);
    ACDK_LOGD("rRotatedXY.rTL84.i4Y = %d", rAWBNVRAM.rRotatedXY.rTL84.i4Y);
    ACDK_LOGD("rRotatedXY.rCWF.i4X = %d", rAWBNVRAM.rRotatedXY.rCWF.i4X);
    ACDK_LOGD("rRotatedXY.rCWF.i4Y = %d", rAWBNVRAM.rRotatedXY.rCWF.i4Y);
    ACDK_LOGD("rRotatedXY.rDNP.i4X = %d", rAWBNVRAM.rRotatedXY.rDNP.i4X);
    ACDK_LOGD("rRotatedXY.rDNP.i4Y = %d", rAWBNVRAM.rRotatedXY.rDNP.i4Y);
    ACDK_LOGD("rRotatedXY.rD65.i4X = %d", rAWBNVRAM.rRotatedXY.rD65.i4X);
    ACDK_LOGD("rRotatedXY.rD65.i4Y = %d", rAWBNVRAM.rRotatedXY.rD65.i4Y);
    ACDK_LOGD("rRotatedXY.rD75.i4X = %d", rAWBNVRAM.rRotatedXY.rD75.i4X);
    ACDK_LOGD("rRotatedXY.rD75.i4Y = %d", rAWBNVRAM.rRotatedXY.rD75.i4Y);
    ACDK_LOGD("rRotatedXY.rDF.i4X = %d\n", rAWBNVRAM.rRotatedXY.rDF.i4X);
    ACDK_LOGD("rRotatedXY.rDF.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rDF.i4Y);

    ACDK_LOGD("rRotationMatrix.i4RotationAngle = %d", rAWBNVRAM.rRotationMatrix.i4RotationAngle);
    ACDK_LOGD("rRotationMatrix.i4H11 = %d", rAWBNVRAM.rRotationMatrix.i4H11);
    ACDK_LOGD("rRotationMatrix.i4H12 = %d", rAWBNVRAM.rRotationMatrix.i4H12);
    ACDK_LOGD("rRotationMatrix.i4H21 = %d", rAWBNVRAM.rRotationMatrix.i4H21);
    ACDK_LOGD("rRotationMatrix.i4H22 = %d", rAWBNVRAM.rRotationMatrix.i4H22);

    ACDK_LOGD("rDaylightLocus.i4SlopeNumerator = %d", rAWBNVRAM.rDaylightLocus.i4SlopeNumerator);
    ACDK_LOGD("rDaylightLocus.i4SlopeDenominator = %d", rAWBNVRAM.rDaylightLocus.i4SlopeDenominator);

    ACDK_LOGD("rAWBLightArea.rTungsten.i4RightBound = %d", rAWBNVRAM.rAWBLightArea.rTungsten.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LeftBound = %d", rAWBNVRAM.rAWBLightArea.rTungsten.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4UpperBound = %d", rAWBNVRAM.rAWBLightArea.rTungsten.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LowerBound = %d", rAWBNVRAM.rAWBLightArea.rTungsten.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4RightBound = %d", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LeftBound = %d", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4UpperBound = %d", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LowerBound = %d", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4RightBound = %d", rAWBNVRAM.rAWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LeftBound = %d", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4UpperBound = %d", rAWBNVRAM.rAWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LowerBound = %d", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4RightBound = %d", rAWBNVRAM.rAWBLightArea.rCWF.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LeftBound = %d", rAWBNVRAM.rAWBLightArea.rCWF.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4UpperBound = %d", rAWBNVRAM.rAWBLightArea.rCWF.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LowerBound = %d", rAWBNVRAM.rAWBLightArea.rCWF.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4RightBound = %d", rAWBNVRAM.rAWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LeftBound = %d", rAWBNVRAM.rAWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4UpperBound = %d", rAWBNVRAM.rAWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LowerBound = %d", rAWBNVRAM.rAWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4RightBound = %d", rAWBNVRAM.rAWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LeftBound = %d", rAWBNVRAM.rAWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4UpperBound = %d", rAWBNVRAM.rAWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LowerBound = %d", rAWBNVRAM.rAWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LowerBound);

    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rTwilight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rTwilight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rTwilight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rTwilight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4RightBound = %d", rAWBNVRAM.rPWBLightArea.rIncandescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LeftBound = %d", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4UpperBound = %d", rAWBNVRAM.rPWBLightArea.rIncandescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LowerBound = %d", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LowerBound);

    ACDK_LOGD("rPWBDefaultGain.rDaylight.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rDaylight.u4R);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rDaylight.u4G);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rDaylight.u4B);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.u4R);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.u4G);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.u4B);
    ACDK_LOGD("rPWBDefaultGain.rShade.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rShade.u4R);
    ACDK_LOGD("rPWBDefaultGain.rShade.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rShade.u4G);
    ACDK_LOGD("rPWBDefaultGain.rShade.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rShade.u4B);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rTwilight.u4R);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rTwilight.u4G);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rTwilight.u4B);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rFluorescent.u4R);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rFluorescent.u4G);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rFluorescent.u4B);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.u4R);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.u4G);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.u4B);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.u4R = %d", rAWBNVRAM.rPWBDefaultGain.rIncandescent.u4R);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.u4G = %d", rAWBNVRAM.rPWBDefaultGain.rIncandescent.u4G);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.u4B = %d", rAWBNVRAM.rPWBDefaultGain.rIncandescent.u4B);

    ACDK_LOGD("rPreferenceColor.rTungsten.i4SliderValue = %d", rAWBNVRAM.rPreferenceColor.rTungsten.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rTungsten.i4OffsetThr = %d", rAWBNVRAM.rPreferenceColor.rTungsten.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4SliderValue = %d", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4OffsetThr = %d", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rShade.i4SliderValue = %d", rAWBNVRAM.rPreferenceColor.rShade.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rShade.i4OffsetThr = %d", rAWBNVRAM.rPreferenceColor.rShade.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.u4R = %d", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.u4R);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.u4G = %d", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.u4G);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.u4B = %d", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.u4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.u4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.u4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.u4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.u4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.u4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.u4B);

    ACDK_LOGD("rCCTEstimation.i4CCT[0] = %d", rAWBNVRAM.rCCTEstimation.i4CCT[0]);
    ACDK_LOGD("rCCTEstimation.i4CCT[1] = %d", rAWBNVRAM.rCCTEstimation.i4CCT[1]);
    ACDK_LOGD("rCCTEstimation.i4CCT[2] = %d", rAWBNVRAM.rCCTEstimation.i4CCT[2]);
    ACDK_LOGD("rCCTEstimation.i4CCT[3] = %d", rAWBNVRAM.rCCTEstimation.i4CCT[3]);
    ACDK_LOGD("rCCTEstimation.i4CCT[4] = %d", rAWBNVRAM.rCCTEstimation.i4CCT[4]);
    ACDK_LOGD("rCCTEstimation.i4CCT[5] = %d", rAWBNVRAM.rCCTEstimation.i4CCT[5]);

    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[0] = %d", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[0]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[1] = %d", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[1]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[2] = %d", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[2]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[3] = %d", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[3]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[4] = %d", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[4]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[5] = %d", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[5]);
*/

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    UINT8 index = (UINT8)pREQ->cmd.dev_isp_shading_index;

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,
                        (UINT8*)&index,
                        sizeof(UINT8),
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }
    pCNF->status = FT_CNF_OK;
    return true;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    UINT8 index = 0;

    UINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX,
                        NULL,
                        0,
                        (UINT8*)&index,
                        sizeof(UINT8),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
			return false;
    }

    ACDK_LOGD("Shading Index:%d", index);
    pCNF->result.get_isp_shading_index = index;
    pCNF->status = FT_CNF_OK;
    return true;
}

bool FT_ACDK_CCT_OP_SET_COMPENSATION_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	if(CAMERA_TUNING_BINNING_SET < pREQ->cmd.set_compensation_mode)
	{
		pCNF->status = FT_CCT_ERR_INVALID_COMPENSATION_MODE;
		return false;
	}
	g_FT_CCT_StateMachine.comp_mode = pREQ->cmd.set_compensation_mode;
	ACDK_LOGD("Set compensation mode to %d", g_FT_CCT_StateMachine.comp_mode);
	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_OP_GET_COMPENSATION_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	pCNF->result.get_compensation_mode = g_FT_CCT_StateMachine.comp_mode;
	pCNF->status = FT_CNF_OK;
	return true;
}


#if 0 // removed in 6573
/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SAVE_OB_ON_OFF
/////////////////////////////////////////////////////////////////////////
bool FT_ACDK_CCT_V2_OP_SAVE_OB_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("ACDK_CCT_V2_OP_SAVE_OB_ON_OFF");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    UINT32 u4RetLen = 0;
    UINT32 u4SaveOBValue = 0;

    u4SaveOBValue = (UINT32)pREQ->cmd.dev_save_ob_value;
    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_SAVE_OB_ON_OFF,
                        (UINT8 *)&u4SaveOBValue,
                        sizeof(UINT32),
                        NULL,
                        0,
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
    	ACDK_LOGD("Save OB Value failed!");
			return false;
    }
    ACDK_LOGD("Save OB Value:%d", u4SaveOBValue);
    pCNF->status = FT_CNF_OK;
    return true;
}
#endif

bool FT_ACDK_CCT_OP_CDVT_SENSOR_TEST(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
		ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_TEST");
		UINT32 u4RetLen = 0;
		ACDK_CDVT_SENSOR_TEST_INPUT_T rSensorTestInput;
		memcpy(&rSensorTestInput,(ACDK_CDVT_SENSOR_TEST_INPUT_T*)(*pBuff),sizeof(ACDK_CDVT_SENSOR_TEST_INPUT_T));

		ACDK_LOGD("rSensorTestInput.rExpLinearity.eExpMode : %d",rSensorTestInput.rExpLinearity.eExpMode);
		ACDK_LOGD("rSensorTestInput.rExpLinearity.i4Gain : %d",rSensorTestInput.rExpLinearity.i4Gain);
		ACDK_LOGD("rSensorTestInput.rGainLinearityOBStability.i4GainTableSize : %d",rSensorTestInput.rGainLinearityOBStability.i4GainTableSize);

		ACDK_CDVT_SENSOR_TEST_OUTPUT_T rSensorTestOutput;
    memset (&rSensorTestOutput, 0, sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T));
    if(!bSendDataToCCT(ACDK_CCT_OP_CDVT_SENSOR_TEST,
                        (UINT8*)&rSensorTestInput,
                        sizeof(ACDK_CDVT_SENSOR_TEST_INPUT_T),
                        (UINT8*)&rSensorTestOutput,
                        sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T),
                        &u4RetLen))
    {
    	pCNF->status = FT_CNF_FAIL;
    	ACDK_LOGD("FT_ACDK_CCT_OP_CDVT_SENSOR_TEST failed!");
			return false;
    }

    AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T ap_output;
    memset (&ap_output, 0, sizeof(AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T));
    ap_output.i4ErrorCode = rSensorTestOutput.i4ErrorCode;
    ap_output.i4TestCount = rSensorTestOutput.i4TestCount;
    int cnt = 0;
    for(cnt = 0; cnt < ACDK_CDVT_MAX_TEST_COUNT; cnt ++){
    	ap_output.rRAWAnalysisResult[cnt].fRAvg = (int)(rSensorTestOutput.rRAWAnalysisResult[cnt].fRAvg * 1000);
    	ap_output.rRAWAnalysisResult[cnt].fGrAvg = (int)(rSensorTestOutput.rRAWAnalysisResult[cnt].fGrAvg * 1000);
    	ap_output.rRAWAnalysisResult[cnt].fGbAvg = (int)(rSensorTestOutput.rRAWAnalysisResult[cnt].fGbAvg * 1000);
    	ap_output.rRAWAnalysisResult[cnt].fBAvg = (int)(rSensorTestOutput.rRAWAnalysisResult[cnt].fBAvg * 1000);
    	ap_output.rRAWAnalysisResult[cnt].u4Median = rSensorTestOutput.rRAWAnalysisResult[cnt].u4Median;
    }
    g_PeerBufferLen = sizeof(AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T);
    g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
    memcpy(g_pPeerBuffer,&ap_output,sizeof(AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T));
    if(g_pPeerBuffer == NULL) return false;

    ACDK_LOGD("FT_ACDK_CCT_OP_CDVT_SENSOR_TEST pass");
    pCNF->status = FT_CNF_OK;
    return true;
}


bool FT_ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION");
	UINT32 u4RetLen = 0;
	ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T rSensorCalibrationInput;
	memcpy(&rSensorCalibrationInput,&(pREQ->cmd.dev_cdvt_sensor_cal_input),sizeof(ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T));
	ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T rSensorCalibrationOutput;
  memset (&rSensorCalibrationOutput, 0, sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
  if(!bSendDataToCCT(ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION,
                     (UINT8*)&rSensorCalibrationInput,
                     sizeof(ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T),
                     (UINT8*)&rSensorCalibrationOutput,
                     sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T),
                     &u4RetLen))
  {
  	  pCNF->status = FT_CNF_FAIL;
    	ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION failed!");
			return false;
  }
  	memcpy(&(pCNF->result.get_cdvt_sensor_cal_output),&rSensorCalibrationOutput,sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T));
    ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION pass");
    pCNF->status = FT_CNF_OK;
    return true;
}

static VOID vGetImageDimensionCb(VOID *a_pParam)
{
    ACDK_LOGD("GetImageDimension Callback ");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    ACDK_LOGD("Image Type:%d",  pImgBufInfo->eImgType);
    if (pImgBufInfo->eImgType == PURE_RAW10_TYPE)    //dana check later
    {
        ACDK_LOGD("Width:%d", pImgBufInfo->RAWImgBufInfo.imgWidth);
        ACDK_LOGD("Height:%d", pImgBufInfo->RAWImgBufInfo.imgHeight);
        g_iGetImgDimensionWidth = pImgBufInfo->RAWImgBufInfo.imgWidth;
        g_iGetImgDimensionHeight = pImgBufInfo->RAWImgBufInfo.imgHeight;
		bGetImgDimensionDone = TRUE;
    }
    else if (pImgBufInfo->eImgType == JPEG_TYPE)
    {
        ACDK_LOGD("Width:%d", pImgBufInfo->imgBufInfo.imgWidth);
        ACDK_LOGD("Height:%d", pImgBufInfo->imgBufInfo.imgHeight);
        g_iGetImgDimensionWidth = pImgBufInfo->imgBufInfo.imgWidth;
        g_iGetImgDimensionHeight = pImgBufInfo->imgBufInfo.imgHeight;
		bGetImgDimensionDone = TRUE;
    }
    else
    {
        ACDK_LOGD("Unknown Format ");
    }
	return;
}

bool FT_ACDK_CCT_OP_GET_IMAGE_DIMENSION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_CAP_OUTPUT_FORMAT ACDK_capture_type;
	unsigned short int width=g_iFullSizeWidth;
	unsigned short int height=g_iFullSizeHeight;
	static const FT_CCT_SENSOR_EX  	*s_sensor=NULL;
	ACDK_CCT_STILL_CAPTURE_STRUCT StillCaptureConfigPara;
	memset(&StillCaptureConfigPara, 0, sizeof(ACDK_CCT_STILL_CAPTURE_STRUCT));
	StillCaptureConfigPara.eOperaMode = ACDK_OPT_META_MODE;

	if( NULL == (s_sensor=get_sensor_by_id(pREQ->type, pREQ->device_id)))
	{
			pCNF->status = FT_CCT_ERR_INVALID_SENSOR_ID;
			ACDK_LOGD("Get image dimension error!  FT_CCT_ERR_INVALID_SENSOR_ID ");
			return false;
	}

	if(CAMERA_TUNING_CAPTURE_SET==g_FT_CCT_StateMachine.comp_mode)
	{
		width = s_sensor->width;
		height = s_sensor->height;
		StillCaptureConfigPara.eCameraMode = CAPTURE_MODE;
	}
	else if(CAMERA_TUNING_PREVIEW_SET==g_FT_CCT_StateMachine.comp_mode)
	{
		width = s_sensor->preview_width;
		height = s_sensor->preview_height;
		StillCaptureConfigPara.eCameraMode = PREVIEW_MODE;
	}

	if(pREQ->cmd.get_image_dimension.output_format == OUTPUT_JPEG)
	{
		width = width / pREQ->cmd.get_image_dimension.sub_sample;
		height = height / pREQ->cmd.get_image_dimension.sub_sample;
		//g_nJPGSize = height*width*3/5;
		ACDK_capture_type = OUTPUT_JPEG;
		StillCaptureConfigPara.eOutputFormat = OUTPUT_JPEG;
		ACDK_LOGD("Get image dimension: capture type is JPEG");
	}
	else if(pREQ->cmd.get_image_dimension.output_format == OUTPUT_PURE_RAW10 //dana check later
		|| pREQ->cmd.get_image_dimension.output_format == OUTPUT_PURE_RAW10)
	{
		//g_nJPGSize = height*width;
		ACDK_capture_type = OUTPUT_PURE_RAW10;
    	StillCaptureConfigPara.eOutputFormat = OUTPUT_PURE_RAW10;
    	ACDK_LOGD("Get image dimension: capture type is Raw data");
	}
	else
	{
		ACDK_LOGD("Get image dimension error!  invalidate capture type");
		return false;
	}
	ACDK_LOGD("Get image dimension: the orginal<latest> width:%d, height:%d",width,height);
	width = width & 0xFFF0;
	height = height & 0xFFF0;
	if(width < 320 || height < 240)
	{
		width = 320;
		height = 240;
	}
	ACDK_LOGD("Get image dimension: the after<latest> width:%d, height:%d",width,height);
	StillCaptureConfigPara.u2JPEGEncWidth = width;
	StillCaptureConfigPara.u2JPEGEncHeight = height;
	StillCaptureConfigPara.fpCapCB = vGetImageDimensionCb;
	UINT32 ACDKCCTParaLen = 0;
	bGetImgDimensionDone = FALSE;

	ACDK_LOGD("Get image dimension: Send capture req to ACDK...");
	//g_FT_CCT_StateMachine.capture_jpeg_state = CAPTURE_JPEG_PROCESS;
	if (!bSendDataToCCT(ACDK_CMD_CAPTURE/*ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX*/,
		                           (UINT8 *)&StillCaptureConfigPara,
                               sizeof(ACDK_CCT_STILL_CAPTURE_STRUCT),
                               NULL,
                               0,
                               &ACDKCCTParaLen))
	{
		ACDK_LOGD("GetImgDimension: driver error");
		return false;
	}

	ACDK_LOGD("Get image dimension: ACDK capture done, dimension got.");

	if(!bGetImgDimensionDone)
		return false;

	else
	{
		pCNF->result.get_image_dimension.sub_sample = pREQ->cmd.get_image_dimension.sub_sample;
		if (ACDK_capture_type == OUTPUT_JPEG)
		{
			pCNF->result.get_image_dimension.output_width = g_iGetImgDimensionWidth;
			pCNF->result.get_image_dimension.output_height = g_iGetImgDimensionHeight;
			pCNF->result.get_image_dimension.width = g_iGetImgDimensionWidth;
			pCNF->result.get_image_dimension.height =g_iGetImgDimensionHeight;
			ACDK_LOGD("GetImgDimension: Output format: JPEG");
			ACDK_LOGD("GetImgDimension: Width: %d", pCNF->result.get_image_dimension.width);
			ACDK_LOGD("GetImgDimension: Height: %d", pCNF->result.get_image_dimension.height);
		}
		else if (ACDK_capture_type == OUTPUT_PURE_RAW10) //dana check later
		{
			pCNF->result.get_image_dimension.output_width = g_iGetImgDimensionWidth;
			pCNF->result.get_image_dimension.output_height = g_iGetImgDimensionHeight;
			pCNF->result.get_image_dimension.width = g_iGetImgDimensionWidth;
			pCNF->result.get_image_dimension.height = g_iGetImgDimensionHeight;
			ACDK_LOGD("GetImgDimension: Output format: RAW8");
			ACDK_LOGD("GetImgDimension: Width: %d", pCNF->result.get_image_dimension.width);
			ACDK_LOGD("GetImgDimension: Height: %d", pCNF->result.get_image_dimension.height);
		}
		else
		{
			ACDK_LOGD("GetImgDimension: Output format: Invalid. Failed to get dimension.");
			return false;
		}
		pCNF->result.get_image_dimension.output_format = pREQ->cmd.get_image_dimension.output_format;
		return true;
	}
}

//Start of 6573 CCT feature ===========================================================
bool FT_ACDK_CCT_OP_ISP_READ_REG(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
	int nSize = sizeof(ACDK_CCT_REG_RW_STRUCT);
	memset(&ACDK_reg_read,0,nSize);

	ACDK_reg_read.RegAddr	= pREQ->cmd.reg_read.reg_addr;
	ACDK_reg_read.Type		= pREQ->type;


	UINT32 nRealReadByteCnt = 0;

	if(false == bSendDataToCCT(ACDK_CCT_OP_ISP_READ_REG,
								(UINT8*)&ACDK_reg_read,
								sizeof(ACDK_CCT_REG_RW_STRUCT),
								(UINT8*)&ACDK_reg_read,
								sizeof(ACDK_CCT_REG_RW_STRUCT),
								&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_ISP_READ_REG driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_ISP_READ_REG pass!");
	pCNF->result.reg_read.value = ACDK_reg_read.RegData;
	return true;
}

bool FT_ACDK_CCT_OP_ISP_WRITE_REG(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
	memset(&ACDK_reg_write,0,sizeof(ACDK_CCT_REG_RW_STRUCT));

	ACDK_reg_write.RegAddr = pREQ->cmd.reg_write.reg_addr;
	ACDK_reg_write.RegData = pREQ->cmd.reg_write.value;
	ACDK_reg_write.Type = pREQ->type;
	UINT32 nRealReadByteCnt = 0;

	if(false == bSendDataToCCT(ACDK_CCT_OP_ISP_WRITE_REG,
	                            (UINT8*)&ACDK_reg_write,
	                             sizeof(ACDK_CCT_REG_RW_STRUCT),
	                             NULL,
                               	 0,
	                             &nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_ISP_WRITE_REG driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_ISP_WRITE_REG pass!");
	return true;
}

bool FT_ACDK_CCT_OP_READ_SENSOR_REG(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
	int nSize = sizeof(ACDK_CCT_REG_RW_STRUCT);
	memset(&ACDK_reg_read,0,nSize);

	ACDK_reg_read.RegAddr	= pREQ->cmd.reg_read.reg_addr;
	ACDK_reg_read.Type		= pREQ->type;


	UINT32 nRealReadByteCnt = 0;

	if(false == bSendDataToCCT(ACDK_CCT_OP_READ_SENSOR_REG,
								(UINT8*)&ACDK_reg_read,
								sizeof(ACDK_CCT_REG_RW_STRUCT),
								(UINT8*)&ACDK_reg_read,
								sizeof(ACDK_CCT_REG_RW_STRUCT),
								&nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_READ_SENSOR_REG driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_READ_SENSOR_REG pass!");
	pCNF->result.reg_read.value = ACDK_reg_read.RegData;
	return true;
}

bool FT_ACDK_CCT_OP_WRITE_SENSOR_REG(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
	memset(&ACDK_reg_write,0,sizeof(ACDK_CCT_REG_RW_STRUCT));

	ACDK_reg_write.RegAddr = pREQ->cmd.reg_write.reg_addr;
	ACDK_reg_write.RegData = pREQ->cmd.reg_write.value;
	ACDK_reg_write.Type = pREQ->type;
	UINT32 nRealReadByteCnt = 0;

	if(false == bSendDataToCCT(ACDK_CCT_OP_WRITE_SENSOR_REG,
	                            (UINT8*)&ACDK_reg_write,
	                             sizeof(ACDK_CCT_REG_RW_STRUCT),
	                             NULL,
                               	 0,
	                             &nRealReadByteCnt))
	{
		ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_WRITE_SENSOR_REG driver failed!");
		pCNF->status = FT_CNF_FAIL;
		return false;
	}
	ACDK_LOGD("[CCAP]:FT_ACDK_CCT_OP_WRITE_SENSOR_REG pass!");
	return true;
}

bool FT_ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM pass!");
	return true;
}

bool FT_ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;

	ACDK_LOGD("FT_ACDK_CCT_OP_AWB_SAVE_AWB_PARA\n");

	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_AWB_SAVE_AWB_PARA driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_AWB_SAVE_AWB_PARA pass!");
	return true;
}

bool FT_ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM pass!");
	return true;
}

bool FT_ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM pass!");
	return true;
}

bool FT_ACDK_CCT_OP_ISP_SAVE_TO_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_ISP_SAVE_TO_NVRAM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_ISP_SAVE_TO_NVRAM driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_ISP_SAVE_TO_NVRAM pass!");
	return true;
}

bool FT_ACDK_CCT_OP_ISP_SET_PCA_TABLE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE accessSet;
	memset(&accessSet, 0, sizeof(accessSet));
	memcpy(&accessSet, &(pREQ->cmd.isp_set_pca_table), sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE));
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_ISP_SET_PCA_TABLE,
		(UINT8*)&accessSet,
		sizeof(accessSet),
		NULL,
		0,
		&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_ISP_SET_PCA_TABLE driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_ISP_SET_PCA_TABLE pass!");
	return true;
}

bool FT_ACDK_CCT_OP_ISP_GET_PCA_TABLE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_ACCESS_NVRAM_PCA_TABLE accessSet;
	memset(&accessSet, 0, sizeof(accessSet));
	accessSet.u4Offset = pREQ->cmd.isp_get_pca_table.u4Offset;
	accessSet.u4Count = pREQ->cmd.isp_get_pca_table.u4Count;
	accessSet.u8ColorTemperature = pREQ->cmd.isp_get_pca_table.u8ColorTemperature;
    UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_ISP_GET_PCA_TABLE,
		NULL,
		0,
		(UINT8*)&accessSet,
		sizeof(accessSet),
		&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_ISP_GET_PCA_TABLE driver failed!");
		return false;
	}
	memcpy(&(pCNF->result.isp_get_pca_table), &accessSet, sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE));
	ACDK_LOGD("FT_ACDK_CCT_OP_ISP_GET_PCA_TABLE pass!");
	return true;
}
bool FT_ACDK_CCT_OP_ISP_SET_PCA_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_ACCESS_PCA_CONFIG access;
	memset(&access, 0, sizeof(access));
	memcpy(&access, &(pREQ->cmd.isp_set_pca_para), sizeof(ACDK_CCT_ACCESS_PCA_CONFIG));
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_ISP_SET_PCA_PARA,
		(UINT8*)&access,
		sizeof(access),
		NULL,
		0,
		&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_ISP_SET_PCA_PARA driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_ISP_SET_PCA_PARA pass!");
	return true;
}
bool FT_ACDK_CCT_OP_ISP_GET_PCA_PARA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_CCT_ACCESS_PCA_CONFIG access;
	memset(&access, 0, sizeof(access));
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_ISP_GET_PCA_PARA,
		NULL,
		0,
		(UINT8*)&access,
		sizeof(access),
		&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_ISP_GET_PCA_PARA driver failed!");
		return false;
	}
	memcpy(&(pCNF->result.isp_get_pca_para), &access, sizeof(ACDK_CCT_ACCESS_PCA_CONFIG));
	ACDK_LOGD("FT_ACDK_CCT_OP_ISP_GET_PCA_PARA pass!");
	return true;
}

bool FT_ACDK_CCT_OP_SET_CCM_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_SET_CCM_MODE");

	UINT32 u4Index = (UINT8)pREQ->cmd.dev_ccm_mode;
	UINT32 u4RetLen = 0;

	if(!bSendDataToCCT(ACDK_CCT_OP_SET_CCM_MODE,
                        (UINT8*)&u4Index,
                        sizeof(u4Index),
                        NULL,
                        0,
                        &u4RetLen))
    	{
    		pCNF->status = FT_CNF_FAIL;
			return false;
    	}
    	pCNF->status = FT_CNF_OK;
    	return true;
}

bool FT_ACDK_CCT_OP_GET_CCM_MODE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_GET_CCM_MODE");

	UINT32 u4Index = 0;
    	UINT32 u4RetLen = 0;

    	if(!bSendDataToCCT(ACDK_CCT_OP_GET_CCM_MODE,
                        NULL,
                        0,
                        (UINT8*)&u4Index,
                        sizeof(u4Index),
                        &u4RetLen))
    	{
    		pCNF->status = FT_CNF_FAIL;
			return false;
    	}

    	ACDK_LOGD("CCM mode:%d", u4Index);
    	pCNF->result.get_ccm_mode = u4Index;
    	pCNF->status = FT_CNF_OK;
    	return true;

}

bool FT_ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_CCT_V2_OP_ISP_GET_NVRAM_DATA\n");

	ACDK_CCT_NVRAM_SET_STRUCT  NVRAMData;
    	memset (&NVRAMData, 0, sizeof(ACDK_CCT_NVRAM_SET_STRUCT));
    	NVRAMData.Mode = pREQ->cmd.dev_nvram_mode;
    	ACDK_LOGD("NVRAMData.Mode=%d", NVRAMData.Mode);

	//Reset global buffer
	if (NULL != g_pNvramBuffer)
	{
		free(g_pNvramBuffer);
		g_pNvramBuffer = NULL;
	}
	g_NvramBufferLen = 0;
	// Malloc corresponding size according to requested struct
	switch (pREQ->cmd.dev_nvram_mode)
    	{
	case CAMERA_NVRAM_DEFECT_STRUCT:
       	break;
       case CAMERA_NVRAM_SHADING_STRUCT:
		g_NvramBufferLen = 	sizeof(ISP_SHADING_STRUCT);
            	break;
        case CAMERA_NVRAM_3A_STRUCT:
		g_NvramBufferLen = 	sizeof(NVRAM_CAMERA_3A_STRUCT);
            	break;
        case CAMERA_NVRAM_ISP_PARAM_STRUCT:
		g_NvramBufferLen = 	sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT);
            	break;
				
	case CAMERA_NVRAM_LENS_STRUCT:
		g_NvramBufferLen = 	sizeof(NVRAM_LENS_PARA_STRUCT);
            	break;
            	
        default:
            	ACDK_LOGD("[Get Camera NVRAM data] Unsupported NVRAM structure");
            	pCNF->status = FT_CNF_FAIL;
	     	return false;
    	}
    	g_pNvramBuffer = (char*) malloc (g_NvramBufferLen);
	if (NULL == g_pNvramBuffer)
	{
		// Malloc fail
        	pCNF->status = FT_CNF_FAIL;
		return false;
	}
	memset(g_pNvramBuffer, 0, g_NvramBufferLen);
	// Assign global buffer to the struct to be passed to ACDK
    	NVRAMData.pBuffer = (UINT32 *)g_pNvramBuffer;
    	ACDK_LOGD("g_NvramBufferLen=%u", g_NvramBufferLen);

    	UINT32 u4RetLen = 0;
    	ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA");
    	BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&NVRAMData,
                                                        sizeof(ACDK_CCT_NVRAM_SET_STRUCT),
                                                        &u4RetLen);

    	if (!bRet)
    	{
		ACDK_LOGD("Get NVRAM data Fail");
		pCNF->status = FT_CNF_FAIL;
		return false;
    	}

    	ACDK_LOGD("Get NVRAM data OK, u4RetLen=%u", u4RetLen);

	// This segment is just for logging ------------------------------
	ISP_SHADING_STRUCT * pshadistr;
	switch (pREQ->cmd.dev_nvram_mode)
    	{
	case CAMERA_NVRAM_DEFECT_STRUCT:
       	break;
       case CAMERA_NVRAM_SHADING_STRUCT:
    		pshadistr =  reinterpret_cast<ISP_SHADING_STRUCT*> (NVRAMData.pBuffer) ;
    		ACDK_LOGD("PreviewSize :%d\n", pshadistr->LSCSize[0]);
    		ACDK_LOGD("CaptureSize :%d\n", pshadistr->LSCSize[3]);
    		ACDK_LOGD("Pre SVD Size :%d\n", pshadistr->PreviewSVDSize);
    		ACDK_LOGD("Cap SVD Size :%d\n", pshadistr->CaptureSVDSize);
    		ACDK_LOGD("Data Size  :%d\n", u4RetLen);
    		ACDK_LOGD("NVRAM Data :%d\n", pshadistr->PreviewFrmTable[0][0]);

            	break;
        case CAMERA_NVRAM_3A_STRUCT:
            	break;
        case CAMERA_NVRAM_ISP_PARAM_STRUCT:
            	break;
        default:
            	break;
    	}
	// End segment ---------------------------------------------

	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA_BUF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA_BUF");
    	ACDK_LOGD("Request offset=%u", pREQ->cmd.get_buf.offset);
    	ACDK_LOGD("Request length=%u", pREQ->cmd.get_buf.length);
	unsigned int reqOffset = pREQ->cmd.get_buf.offset;
	unsigned int reqLen = pREQ->cmd.get_buf.length;
	pCNF->status = FT_CNF_FAIL;

	if (NULL == g_pNvramBuffer)
	{
		ACDK_LOGD("NVRAM data buffer is NULL yet!");
		return false;
	}
	// check request offset
	if (reqOffset >= g_NvramBufferLen)
	{
		ACDK_LOGE("Error: Request offset >= g_NvramBufferLen");
		if (NULL != g_pNvramBuffer)
		{
			free(g_pNvramBuffer);
			g_pNvramBuffer = NULL;
	        	g_NvramBufferLen = 0;
		}
		return false;
	}
	// check request length
	if (reqOffset + reqLen > g_NvramBufferLen)
	{
		reqLen = g_NvramBufferLen - reqOffset;
	}
	// narrow down length to the max peer buffer size
	if (reqLen > FT_FAT_MAX_FRAME_SIZE)
	{
		reqLen = FT_FAT_MAX_FRAME_SIZE;
	}
	ACDK_LOGE("Revised length=%u", reqLen);

	g_pPeerBuffer = (char*)malloc(reqLen);
	if (NULL == g_pPeerBuffer)
		return false;
	g_PeerBufferLen = reqLen;
	memcpy(g_pPeerBuffer, g_pNvramBuffer + reqOffset, reqLen);

	if (reqOffset + reqLen >= g_NvramBufferLen)
	{
		ACDK_LOGD("Reach the end of the buffer!");
		free(g_pNvramBuffer);
		g_pNvramBuffer = NULL;
		g_NvramBufferLen = 0;
	}
	pCNF->status = FT_CNF_OK;
	return true;
}

bool FT_ACDK_CCT_OP_SET_ISP_ON(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_SET_ISP_ON");
    	UINT32 u4RetLen = 0;
    	ACDK_CCT_ISP_REG_CATEGORY eCategory;
	memcpy(&eCategory, &(pREQ->cmd.set_isp_on_off), sizeof(eCategory));
	ACDK_LOGD("ISP reg category=%d", eCategory);

    	if(!bSendDataToCCT(ACDK_CCT_OP_SET_ISP_ON,
                        (UINT8*)&eCategory,
                        sizeof(eCategory),
                        NULL,
                        0,
                        &u4RetLen))
    	{
    		ACDK_LOGE("Failed to set ISP on!");
    		pCNF->status = FT_CNF_FAIL;
		return false;
    	}
    	pCNF->status = FT_CNF_OK;
    	return true;
}

bool FT_ACDK_CCT_OP_SET_ISP_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_SET_ISP_OFF");
    	UINT32 u4RetLen = 0;
    	ACDK_CCT_ISP_REG_CATEGORY eCategory;
	memcpy(&eCategory, &(pREQ->cmd.set_isp_on_off), sizeof(eCategory));
	ACDK_LOGD("ISP reg category=%d", eCategory);

    	if(!bSendDataToCCT(ACDK_CCT_OP_SET_ISP_OFF,
                        (UINT8*)&eCategory,
                        sizeof(eCategory),
                        NULL,
                        0,
                        &u4RetLen))
    	{
    		ACDK_LOGE("Failed to set ISP off!");
    		pCNF->status = FT_CNF_FAIL;
		return false;
    	}
    	pCNF->status = FT_CNF_OK;
    	return true;
}

bool FT_ACDK_CCT_OP_GET_ISP_ON_OFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("ACDK_CCT_OP_GET_ISP_ON_OFF");
    	UINT32 u4RetLen = 0;
    	ACDK_CCT_ISP_REG_CATEGORY eCategory;
	ACDK_CCT_FUNCTION_ENABLE_STRUCT ctrl;
	memcpy(&eCategory, &(pREQ->cmd.get_isp_on_off), sizeof(eCategory));
    	memset(&ctrl, 0, sizeof(ctrl));
	ACDK_LOGD("ISP reg category=%d", eCategory);

    	if(!bSendDataToCCT(ACDK_CCT_OP_GET_ISP_ON_OFF,
                        (UINT8*)&eCategory,
                        sizeof(eCategory),
        		   (UINT8*)&ctrl,
        		   sizeof(ctrl),
                        &u4RetLen))
    	{
    		ACDK_LOGE("Failed to get ISP on/off!");
    		pCNF->status = FT_CNF_FAIL;
		return false;
    	}

   	ACDK_LOGD("(u4RetLen, ctrl.Enable)=(%u, %d)", u4RetLen, ctrl.Enable);
	memcpy(&(pCNF->result.get_func_enable), &ctrl, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));
    	pCNF->status = FT_CNF_OK;
    	return true;

}

bool FT_ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION");
	//if(NULL == pSensor)
	//	return false;

  	UINT32 u4RetLen = 0;
	ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

	ACDK_LOGD("FT_ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION enter");
	if (false == bSendDataToCCT(ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION,
                                 NULL,
                                 0,
                                 (UINT8 *)&SensorResolution,
                                 sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                 &u4RetLen))
	{
		ACDK_LOGD("[CCAP]:GetLscSensorSesolutionInfo driver failed!");
		return false;
	}
	ACDK_LOGD("[CCAP]:GetLscSensorSesolutionInfo pass!");
	ACDK_LOGD("SensorPreviewWidth=%d", SensorResolution.SensorPreviewWidth);
	ACDK_LOGD("SensorPreviewHeight=%d", SensorResolution.SensorPreviewHeight);
	ACDK_LOGD("SensorFullWidth=%d", SensorResolution.SensorFullWidth);
	ACDK_LOGD("SensorFullHeight=%d", SensorResolution.SensorFullHeight);
	ACDK_LOGD("SensorVideoWidth=%d", SensorResolution.SensorVideoWidth);
	ACDK_LOGD("SensorVideoHeight=%d", SensorResolution.SensorVideoHeight);
	memcpy(&(pCNF->result.get_lsc_sensor_res),&SensorResolution,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));
	return true;
}


bool FT_ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM pass!");
	return true;
}

bool FT_ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	UINT32 nRealReadByteCnt = 0;
	if(!bSendDataToCCT(ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM,NULL,0,NULL,0,&nRealReadByteCnt))
	{
		ACDK_LOGE("FT_ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM pass!");
	return true;
}

bool FT_ACDK_CCT_OP_AWB_GET_LIGHT_PROB(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_OP_AWB_GET_LIGHT_PROB");

	UINT32 u4RetLen = 0;
	ACDK_AWB_LIGHT_PROBABILITY_T pAWBLightProb;
	if(!bSendDataToCCT(ACDK_CCT_OP_AWB_GET_LIGHT_PROB,
                        NULL,
                        0,
                        (UINT8*)&pAWBLightProb,
                        sizeof(pAWBLightProb),
                        &u4RetLen))
    	{
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_AWB_GET_LIGHT_PROB driver failed!");
		return false;
    	}
	pCNF->result.get_awb_prob = pAWBLightProb;
	ACDK_LOGD("FT_ACDK_CCT_OP_AWB_GET_LIGHT_PROB pass!");

	int i = 0;
	for( i = 0 ; i < ACDK_AWB_LIGHT_NUM; i++)
	{
		ACDK_LOGD("u4P0 :%d\n", pAWBLightProb.u4P0[i]);
	}
	for( i = 0 ; i < ACDK_AWB_LIGHT_NUM; i++)
	{
		ACDK_LOGD("u4P1 :%d\n", pAWBLightProb.u4P1[i]);
	}
	for( i = 0 ; i < ACDK_AWB_LIGHT_NUM; i++)
	{
		ACDK_LOGD("u4P2 :%d\n", pAWBLightProb.u4P2[i]);
	}
	for( i = 0 ; i < ACDK_AWB_LIGHT_NUM; i++)
	{
		ACDK_LOGD("u4P :%d\n", pAWBLightProb.u4P[i]);
	}
	return true;

}
bool FT_ACDK_CCT_OP_STROBE_RATIO_TUNING(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
	ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_RATIO_TUNING");

	MUINT32 u4RetLen = 0;
    	MUINT32 AE5_5Data[50] = {0};
    	MUINT32 strobeLevel = 0;

    	memcpy(&strobeLevel, &(pREQ->cmd.dev_strobe_level), sizeof(strobeLevel));

    	if(strobeLevel < 1 || strobeLevel > 32)
        {
        	pCNF->status = FT_CNF_FAIL;
            	ACDK_LOGD("preflash level : %d\n it should be between 1 ~ 32", strobeLevel);
            	return false;
        }

	if (false == bSendDataToCCT(ACDK_CCT_OP_STROBE_RATIO_TUNING,
                                 (UINT8 *)&strobeLevel,
                                 sizeof(MUINT32),
                                 (UINT8 *)&AE5_5Data,
                                 sizeof(FT_CCT_STROBE_CAL_STRUCT),
                                 &u4RetLen))
	{
		ACDK_LOGD("[CCAP]:CCTOPStrobeRatioTuning driver failed!");
		return false;
	}
	ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_RATIO_TUNING pass!");

	int i = 0;
	for( i = 0 ; i < 50; i++)
	{
		ACDK_LOGD("[%4d] AE5_5Data:%d\n", i, AE5_5Data[i]);
	}

    	memcpy(&(pCNF->result.get_AE_win),&AE5_5Data,sizeof(FT_CCT_STROBE_CAL_STRUCT));

	for( i = 0 ; i < 50; i++)
	{
		ACDK_LOGD("[%4d]  get_AE_win:%d\n", i , pCNF->result.get_AE_win.AE_win[i]);
	}

	pCNF->status = FT_CNF_OK;
	return true;
}


MRESULT FT_ACDK_CCT_OP_SWITCH_CAMERA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{


    ACDK_LOGD("FT_ACDK_CCT_OP_SWITCH_CAMERA");
	
    MUINT32 u4CameraType = (UINT32)(pREQ->cmd.dev_src_device);
    MUINT32 u4RetLen = 0;

    ACDK_LOGD("u4CameraType: %d\n", u4CameraType);

    g_FT_CCT_StateMachine.src_device_mode = u4CameraType;


    return true;
}

MRESULT FT_ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{


    ACDK_LOGD("FT_ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF");

    UINT32 u4RetLen = 0;
    ISP_NVRAM_CCM_POLY22_STRUCT CCMPoly22;
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF,
                        NULL,
                        0,
                        (UINT8*)&CCMPoly22,
                        sizeof(CCMPoly22),
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF driver failed!");
		return false;
    }
    pCNF->result.get_ccm_poly22 = CCMPoly22;
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF pass!");
    pCNF->status = FT_CNF_OK;

    //debug log
    ACDK_LOGD("i4R_AVG:%d\n", CCMPoly22.i4R_AVG);
    ACDK_LOGD("i4R_STD:%d\n", CCMPoly22.i4R_STD);
    ACDK_LOGD("i4B_AVG:%d\n", CCMPoly22.i4B_AVG);
    ACDK_LOGD("i4B_STD:%d\n", CCMPoly22.i4B_STD);
	int i = 0;
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p00:%d\n", i, CCMPoly22.i4P00[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p10:%d\n", i, CCMPoly22.i4P10[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p01:%d\n", i, CCMPoly22.i4P01[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p20:%d\n", i, CCMPoly22.i4P20[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p11:%d\n", i, CCMPoly22.i4P11[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p02:%d\n", i, CCMPoly22.i4P02[i]);
	}
    return true;
}



MRESULT FT_ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF");

    UINT32 u4RetLen = 0;
    ISP_NVRAM_CCM_POLY22_STRUCT CCMPoly22;
    memcpy(&CCMPoly22, &(pREQ->cmd.set_ccm_poly22), sizeof(CCMPoly22));

	//debug log
    ACDK_LOGD("i4R_AVG:%d\n", CCMPoly22.i4R_AVG);
    ACDK_LOGD("i4R_STD:%d\n", CCMPoly22.i4R_STD);
    ACDK_LOGD("i4B_AVG:%d\n", CCMPoly22.i4B_AVG);
    ACDK_LOGD("i4B_STD:%d\n", CCMPoly22.i4B_STD);
	int i = 0;
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p00:%d\n", i, CCMPoly22.i4P00[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p10:%d\n", i, CCMPoly22.i4P10[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p01:%d\n", i, CCMPoly22.i4P01[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p20:%d\n", i, CCMPoly22.i4P20[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p11:%d\n", i, CCMPoly22.i4P11[i]);
	}
	for( i = 0 ; i < 9; i++)
	{
		ACDK_LOGD("[%4d] i4p02:%d\n", i, CCMPoly22.i4P02[i]);
	}

    if(!bSendDataToCCT(ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF,
                       (UINT8*)&CCMPoly22,
                        sizeof(CCMPoly22),
                        NULL,
                        0,
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF driver failed!");
		return false;
    }

    ACDK_LOGD("FT_ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF pass!");
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{


    ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM");

    UINT32 u4RetLen = 0;
    ISP_NVRAM_MFB_MIXER_STRUCT MFBMixer;
    if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM,
                        NULL,
                        0,
                        (UINT8*)&MFBMixer,
                        sizeof(MFBMixer),
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM driver failed!");
		return false;
    }
    pCNF->result.get_MFB_mixer = MFBMixer;
    ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAMpass!");
    pCNF->status = FT_CNF_OK;
	//debug log
	int i = 0;
	for( i = 0 ; i < 7; i++)
	{
    		ACDK_LOGD("[%4d] i4M0:%d\n", i,MFBMixer.param[i].i4M0);
    		ACDK_LOGD("[%4d] i4M1:%d\n", i,MFBMixer.param[i].i4M1);
	}

    return true;
}



MRESULT FT_ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM");

    UINT32 u4RetLen = 0;
    ISP_NVRAM_MFB_MIXER_STRUCT MFBMixer;
    memcpy(&MFBMixer, &(pREQ->cmd.set_MFB_mixer), sizeof(MFBMixer));

	//debug log
	int i = 0;
	for( i = 0 ; i < 7; i++)
	{
    		ACDK_LOGD("[%4d] i4M0:%d\n", i,MFBMixer.param[i].i4M0);
    		ACDK_LOGD("[%4d] i4M1:%d\n", i,MFBMixer.param[i].i4M1);
	}

    if(!bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM,
                       (UINT8*)&MFBMixer,
                        sizeof(MFBMixer),
                        NULL,
                        0,
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM driver failed!");
		return false;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM pass!");
    pCNF->status = FT_CNF_OK;
    return true;
}
MRESULT FT_ACDK_CCT_OP_ISP_GET_PCA_SLIDER(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{


    ACDK_LOGD("FT_ACDK_CCT_OP_ISP_GET_PCA_SLIDER");

    UINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_PCA_SLIDER PCASlider;
    if(!bSendDataToCCT(ACDK_CCT_OP_ISP_GET_PCA_SLIDER,
                        NULL,
                        0,
                        (UINT8*)&PCASlider,
                        sizeof(PCASlider),
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_ISP_GET_PCA_SLIDER driver failed!");
		return false;
    }
    pCNF->result.get_pca_slider = PCASlider;
    ACDK_LOGD("FT_ACDK_CCT_OP_ISP_GET_PCA_SLIDER!");
    pCNF->status = FT_CNF_OK;
	//debug log
	int i = 0;
	for( i = 0 ; i < 3; i++)
	{
    		ACDK_LOGD("[%4d] slider:%d\n", i,PCASlider.slider.value[i]);
	}

    return true;
}



MRESULT FT_ACDK_CCT_OP_ISP_SET_PCA_SLIDER(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_OP_ISP_SET_PCA_SLIDER");

    UINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_PCA_SLIDER PCASlider;
    memcpy(&PCASlider, &(pREQ->cmd.set_pca_slider), sizeof(PCASlider));

	//debug log
	int i = 0;
	for( i = 0 ; i < 3; i++)
	{
    		ACDK_LOGD("[%4d] slider:%d\n", i,PCASlider.slider.value[i]);
	}

    if(!bSendDataToCCT(ACDK_CCT_OP_ISP_SET_PCA_SLIDER,
                       (UINT8*)&PCASlider,
                        sizeof(PCASlider),
                        NULL,
                        0,
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_ISP_SET_PCA_SLIDER driver failed!");
		return false;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SET_PCA_SLIDER pass!");
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC");

    UINT32 u4RetLen = 0;
    ACDK_STROBE_STRUCT Strobe_nvram;
    

    if(!bSendDataToCCT(ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META,
                        NULL,
                        0,
                        (UINT8*)&Strobe_nvram,
                        sizeof(Strobe_nvram),
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META driver failed!");
		return false;
    }

    ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC pass!");

    //log
 
    	ACDK_LOGD("[engtab] exp:%d\n", Strobe_nvram.engTab.exp);
	ACDK_LOGD("[engtab] afe_gain:%d\n", Strobe_nvram.engTab.afe_gain);
	ACDK_LOGD("[engtab] isp_gain:%d\n", Strobe_nvram.engTab.isp_gain);
	ACDK_LOGD("[engtab] distance:%d\n", Strobe_nvram.engTab.distance);

	for( int i = 0 ; i < 6 ; i++)
	{
		ACDK_LOGD("[tuningpara] [%4d] yTar:%d\n", i,Strobe_nvram.tuningPara[i].yTar);
		ACDK_LOGD("[tuningpara] [%4d] antiIsoLevel:%d\n", i,Strobe_nvram.tuningPara[i].antiIsoLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiExpLevel:%d\n", i,Strobe_nvram.tuningPara[i].antiExpLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiStrobeLevel:%d\n", i,Strobe_nvram.tuningPara[i].antiStrobeLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiUnderLevel:%d\n", i,Strobe_nvram.tuningPara[i].antiUnderLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiOverLevel:%d\n", i,Strobe_nvram.tuningPara[i].antiOverLevel);
		ACDK_LOGD("[tuningpara] [%4d] foregroundLevel:%d\n", i,Strobe_nvram.tuningPara[i].foregroundLevel);
		ACDK_LOGD("[tuningpara] [%4d] isRefAfDistance:%d\n", i,Strobe_nvram.tuningPara[i].isRefAfDistance);
		ACDK_LOGD("[tuningpara] [%4d] accuracyLevel:%d\n", i,Strobe_nvram.tuningPara[i].accuracyLevel);
	}

	ACDK_LOGD("isTorchEngUpdate:%d\n", Strobe_nvram.isTorchEngUpdate);
	ACDK_LOGD("isAfEngUpdate:%d\n", Strobe_nvram.isAfEngUpdate);
	ACDK_LOGD("isNormaEnglUpdate:%d\n", Strobe_nvram.isNormaEnglUpdate);
	ACDK_LOGD("isLowBatEngUpdate:%d\n", Strobe_nvram.isLowBatEngUpdate);
	ACDK_LOGD("isBurstEngUpdate:%d\n", Strobe_nvram.isBurstEngUpdate);
	/*
	ACDK_LOGD("[engLevel]torchDuty:%d\n", Strobe_nvram.engLevel.torchDuty);
	ACDK_LOGD("[engLevel]torchStep:%d\n", Strobe_nvram.engLevel.torchStep);
	ACDK_LOGD("[engLevel]afDuty:%d\n", Strobe_nvram.engLevel.afDuty);
	ACDK_LOGD("[engLevel]afStep:%d\n", Strobe_nvram.engLevel.afStep);
	ACDK_LOGD("[engLevel]pfDuty:%d\n", Strobe_nvram.engLevel.pfDuty);
	ACDK_LOGD("[engLevel]mfDutyMax:%d\n", Strobe_nvram.engLevel.mfDutyMax);
	ACDK_LOGD("[engLevel]mfDutyMin:%d\n", Strobe_nvram.engLevel.mfDutyMin);
	ACDK_LOGD("[engLevel]pmfStep:%d\n", Strobe_nvram.engLevel.pmfStep);
	ACDK_LOGD("[engLevel]IChangeByVBatEn:%d\n", Strobe_nvram.engLevel.IChangeByVBatEn);
	ACDK_LOGD("[engLevel]vBatL:%d\n", Strobe_nvram.engLevel.vBatL);
	ACDK_LOGD("[engLevel]pfDutyL:%d\n", Strobe_nvram.engLevel.pfDutyL);
	ACDK_LOGD("[engLevel]mfDutyMaxL:%d\n", Strobe_nvram.engLevel.mfDutyMaxL);
	ACDK_LOGD("[engLevel]mfDutyMinL:%d\n", Strobe_nvram.engLevel.mfDutyMinL);
	ACDK_LOGD("[engLevel]pmfStepL:%d\n", Strobe_nvram.engLevel.pmfStepL);
	ACDK_LOGD("[engLevel]IChangeByBurstEn:%d\n", Strobe_nvram.engLevel.IChangeByBurstEn);
	ACDK_LOGD("[engLevel]pfDutyB:%d\n", Strobe_nvram.engLevel.pfDutyB);
	ACDK_LOGD("[engLevel]mfDutyMaxB:%d\n", Strobe_nvram.engLevel.mfDutyMaxB);
	ACDK_LOGD("[engLevel]mfDutyMinB:%d\n", Strobe_nvram.engLevel.mfDutyMinB);
	ACDK_LOGD("[engLevel]pmfStepB:%d\n", Strobe_nvram.engLevel.pmfStepB);
	*/
    pCNF->result.get_flash_nvram = Strobe_nvram;
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_OP_STROBE_SET_NVDATA(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_SET_NVDATA");

    UINT32 u4RetLen = 0;
    ACDK_STROBE_STRUCT Strobe_nvram_set;
    memcpy(&Strobe_nvram_set, &(pREQ->cmd.set_flash_nvram), sizeof(Strobe_nvram_set));

	//debug log
	ACDK_LOGD("[engtab] exp:%d\n", Strobe_nvram_set.engTab.exp);
	ACDK_LOGD("[engtab] afe_gain:%d\n", Strobe_nvram_set.engTab.afe_gain);
	ACDK_LOGD("[engtab] isp_gain:%d\n", Strobe_nvram_set.engTab.isp_gain);
	ACDK_LOGD("[engtab] distance:%d\n", Strobe_nvram_set.engTab.distance);

	for( int i = 0 ; i < 6 ; i++)
	{
		ACDK_LOGD("[tuningpara] [%4d] yTar:%d\n", i,Strobe_nvram_set.tuningPara[i].yTar);
		ACDK_LOGD("[tuningpara] [%4d] antiIsoLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].antiIsoLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiExpLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].antiExpLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiStrobeLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].antiStrobeLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiUnderLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].antiUnderLevel);
		ACDK_LOGD("[tuningpara] [%4d] antiOverLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].antiOverLevel);
		ACDK_LOGD("[tuningpara] [%4d] foregroundLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].foregroundLevel);
		ACDK_LOGD("[tuningpara] [%4d] isRefAfDistance:%d\n", i,Strobe_nvram_set.tuningPara[i].isRefAfDistance);
		ACDK_LOGD("[tuningpara] [%4d] accuracyLevel:%d\n", i,Strobe_nvram_set.tuningPara[i].accuracyLevel);
	}

	ACDK_LOGD("isTorchEngUpdate:%d\n", Strobe_nvram_set.isTorchEngUpdate);
	ACDK_LOGD("isAfEngUpdate:%d\n", Strobe_nvram_set.isAfEngUpdate);
	ACDK_LOGD("isNormaEnglUpdate:%d\n", Strobe_nvram_set.isNormaEnglUpdate);
	ACDK_LOGD("isLowBatEngUpdate:%d\n", Strobe_nvram_set.isLowBatEngUpdate);
	ACDK_LOGD("isBurstEngUpdate:%d\n", Strobe_nvram_set.isBurstEngUpdate);
	/*
	ACDK_LOGD("[engLevel]torchDuty:%d\n", Strobe_nvram_set.engLevel.torchDuty);
	ACDK_LOGD("[engLevel]torchStep:%d\n", Strobe_nvram_set.engLevel.torchStep);
	ACDK_LOGD("[engLevel]afDuty:%d\n", Strobe_nvram_set.engLevel.afDuty);
	ACDK_LOGD("[engLevel]afStep:%d\n", Strobe_nvram_set.engLevel.afStep);
	ACDK_LOGD("[engLevel]pfDuty:%d\n", Strobe_nvram_set.engLevel.pfDuty);
	ACDK_LOGD("[engLevel]mfDutyMax:%d\n", Strobe_nvram_set.engLevel.mfDutyMax);
	ACDK_LOGD("[engLevel]mfDutyMin:%d\n", Strobe_nvram_set.engLevel.mfDutyMin);
	ACDK_LOGD("[engLevel]pmfStep:%d\n", Strobe_nvram_set.engLevel.pmfStep);
	ACDK_LOGD("[engLevel]IChangeByVBatEn:%d\n", Strobe_nvram_set.engLevel.IChangeByVBatEn);
	ACDK_LOGD("[engLevel]vBatL:%d\n", Strobe_nvram_set.engLevel.vBatL);
	ACDK_LOGD("[engLevel]pfDutyL:%d\n", Strobe_nvram_set.engLevel.pfDutyL);
	ACDK_LOGD("[engLevel]mfDutyMaxL:%d\n", Strobe_nvram_set.engLevel.mfDutyMaxL);
	ACDK_LOGD("[engLevel]mfDutyMinL:%d\n", Strobe_nvram_set.engLevel.mfDutyMinL);
	ACDK_LOGD("[engLevel]pmfStepL:%d\n", Strobe_nvram_set.engLevel.pmfStepL);
	ACDK_LOGD("[engLevel]IChangeByBurstEn:%d\n", Strobe_nvram_set.engLevel.IChangeByBurstEn);
	ACDK_LOGD("[engLevel]pfDutyB:%d\n", Strobe_nvram_set.engLevel.pfDutyB);
	ACDK_LOGD("[engLevel]mfDutyMaxB:%d\n", Strobe_nvram_set.engLevel.mfDutyMaxB);
	ACDK_LOGD("[engLevel]mfDutyMinB:%d\n", Strobe_nvram_set.engLevel.mfDutyMinB);
	ACDK_LOGD("[engLevel]pmfStepB:%d\n", Strobe_nvram_set.engLevel.pmfStepB);
*/
    if(!bSendDataToCCT(ACDK_CCT_OP_STROBE_SET_NVDATA_META,
                       (UINT8*)&Strobe_nvram_set,
                        sizeof(Strobe_nvram_set),
                        NULL,
                        0,
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_STROBE_SET_NVDATA_META driver failed!");
		return false;
    }

    ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_SET_NVDATA pass!");
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_OP_STROBE_WRITE_NVRAM(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_WRITE_NVRAM");

    UINT32 u4RetLen = 0;


    if(!bSendDataToCCT(ACDK_CCT_OP_STROBE_WRITE_NVRAM,
                        NULL,
                        0,
                        NULL,
                        0,
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_STROBE_WRITE_NVRAM driver failed!");
		return false;
    }

    ACDK_LOGD("FT_ACDK_CCT_OP_STROBE_WRITE_NVRAM pass!");
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_OP_AE_GET_FALRE_CALIBRATION(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_OP_AE_GET_FALRE_CALIBRATION");

    UINT32 u4RetLen = 0;
    UINT32 Flare_Thres_set;
    UINT32 Flare_Offset_out;
	
	Flare_Thres_set = (UINT8)pREQ->cmd.set_Flare_Thres;

	//debug log
    	ACDK_LOGD("set_Flare_Thres:%d\n",Flare_Thres_set);
	

    if(!bSendDataToCCT(ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION,
                       (UINT8*)&Flare_Thres_set,
                        sizeof(Flare_Thres_set),
                        (UINT8*)&Flare_Offset_out,
                        sizeof(Flare_Offset_out),
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_ISP_SET_PCA_SLIDER driver failed!");
		return false;
    }

    //debug log
    ACDK_LOGD("get_Flare_Offset:%d\n",Flare_Offset_out);
	
    ACDK_LOGD("FT_ACDK_CCT_OP_AE_GET_FALRE_CALIBRATION pass!");
    pCNF->result.get_Flare_Offset = Flare_Offset_out;
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_OP_AE_PLINE_TABLE_TEST(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{

    ACDK_LOGD("FT_ACDK_CCT_OP_AE_PLINE_TABLE_TEST");

    UINT32 u4RetLen = 0;
    ACDK_CDVT_AE_PLINE_TEST_INPUT_T	pline_test_set;
    ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T pline_test_result;
	
	memcpy(&pline_test_set, &(pREQ->cmd.dev_pline_test_input), sizeof(ACDK_CDVT_AE_PLINE_TEST_INPUT_T));

	//debug log
    ACDK_LOGD("eSensorMode:%d\n",pline_test_set.eSensorMode);
    ACDK_LOGD("rAEPlinetableInfo:%d\n",pline_test_set.rAEPlinetableInfo.i4ShutterDelayFrame);
    ACDK_LOGD("i4SensorGainDelayFrame:%d\n",pline_test_set.rAEPlinetableInfo.i4SensorGainDelayFrame);
    ACDK_LOGD("i4ISPGainDelayFrame:%d\n",pline_test_set.rAEPlinetableInfo.i4ISPGainDelayFrame);
    ACDK_LOGD("i4TestSteps:%d\n",pline_test_set.rAEPlinetableInfo.i4TestSteps);
    ACDK_LOGD("i4RepeatTime:%d\n",pline_test_set.rAEPlinetableInfo.i4RepeatTime);
    ACDK_LOGD("i4IndexScanStart:%d\n",pline_test_set.rAEPlinetableInfo.i4IndexScanStart);
    ACDK_LOGD("i4IndexScanEnd:%d\n",pline_test_set.rAEPlinetableInfo.i4IndexScanEnd);
    
    for( int i = 0 ; i < 100 ; i++)
	{
		ACDK_LOGD("[i4PlineTable] [%4d] i4Index:%d\n", i,pline_test_set.rAEPlinetableInfo.i4PlineTable[i].i4Index);
		ACDK_LOGD("[i4PlineTable] [%4d] i4ShutterTime:%d\n", i,pline_test_set.rAEPlinetableInfo.i4PlineTable[i].i4ShutterTime);
		ACDK_LOGD("[i4PlineTable] [%4d] i4SensorGain:%d\n", i,pline_test_set.rAEPlinetableInfo.i4PlineTable[i].i4SensorGain);
		ACDK_LOGD("[i4PlineTable] [%4d] i4ISPGain:%d\n", i,pline_test_set.rAEPlinetableInfo.i4PlineTable[i].i4ISPGain);
		//ACDK_LOGD("[i4PlineTable] [%4d] i4Yvalue:%d\n", i,pline_test_set.rAEPlinetableInfo[i].i4Yvalue);
	}
	

    if(!bSendDataToCCT(ACDK_CCT_OP_AE_PLINE_TABLE_TEST,
                       (UINT8*)&pline_test_set,
                        sizeof(pline_test_set),
                        (UINT8*)&pline_test_result,
                        sizeof(pline_test_result),
                        &u4RetLen))
    {
    		pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_OP_AE_PLINE_TABLE_TEST driver failed!");
		return false;
    }

    //debug log
    ACDK_LOGD("i4ErrorCode:%d\n",pline_test_result.i4ErrorCode);
    ACDK_LOGD("i4TestCount:%d\n",pline_test_result.i4TestCount);
	for( int i = 0 ; i < 100 ; i++)
	{
		ACDK_LOGD("[i4PlineTable] [%4d] i4Index:%d\n", i,pline_test_result.rYAnalysisResult[i].i4Index);
		ACDK_LOGD("[i4PlineTable] [%4d] i4ShutterTime:%d\n", i,pline_test_result.rYAnalysisResult[i].i4ShutterTime);
		ACDK_LOGD("[i4PlineTable] [%4d] i4SensorGain:%d\n", i,pline_test_result.rYAnalysisResult[i].i4SensorGain);
		ACDK_LOGD("[i4PlineTable] [%4d] i4ISPGain:%d\n", i,pline_test_result.rYAnalysisResult[i].i4ISPGain);
		ACDK_LOGD("[i4PlineTable] [%4d] i4Yvalue:%d\n", i,pline_test_result.rYAnalysisResult[i].i4Yvalue);
	}
	g_PeerBufferLen = sizeof(ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T);
    g_pPeerBuffer = (char*)malloc(g_PeerBufferLen);
    memcpy(g_pPeerBuffer,&pline_test_result,sizeof(ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T));
    if(g_pPeerBuffer == NULL) return false;

    ACDK_LOGD("FT_ACDK_CCT_OP_AE_PLINE_TABLE_TEST pass!");
    //memcpy(&(pCNF->result.get_pline_test_result),&pline_test_result,sizeof(ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T));
    
    pCNF->status = FT_CNF_OK;
    return true;
}

MRESULT FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE(const FT_CCT_REQ* pREQ,FT_CCT_CNF* pCNF,char** pBuff)
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE\n");
    

    ACDK_CCT_FUNCTION_ENABLE_STRUCT cctOnOff;

    cctOnOff.Enable = (UINT8)pREQ->cmd.on_off;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE,
                                                       (UINT8 *)&cctOnOff,
                                                       sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        	pCNF->status = FT_CNF_FAIL;
    		ACDK_LOGD("[CCAP]:ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE driver failed!");
		return false;
    }
    ACDK_LOGD("CCAP Set TSF AWB Force On/Off Success, Enable:%d\n", cctOnOff.Enable);
    
    pCNF->status = FT_CNF_OK;
    return true;
}


//End of 6573 CCT feature ============================================================
static bool ft_fb_init()
{
    int fd=0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    unsigned int fb_base = NULL;
    unsigned int fb_size = 0;
    int y = 0;

    fd = open ("/dev/graphics/fb0",O_RDWR);
    if(fd < 0)
    {
        printf("open dev file fail\n");
        return false;
    }

    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);

	ACDK_LOGD("finfo.line_length : %d", finfo.line_length);
	ACDK_LOGD("finfo.smem_len : %d", finfo.smem_len);
	ACDK_LOGD("finfo.ypanstep : %d", finfo.ypanstep);

	ACDK_LOGD("vinfo.xres : %d", vinfo.xres);
	ACDK_LOGD("vinfo.yres : %d", vinfo.yres);
	

	
    // Map whole frame buffer.
    fb_base = (unsigned int) mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE| PROT_NOCACHE, MAP_SHARED, fd, 0);

    fb_size = vinfo.yres * finfo.line_length;
    // Calculate new y position.
    y = (vinfo.yoffset + vinfo.yres * finfo.ypanstep) % vinfo.yres_virtual;
	if( y + vinfo.yres > vinfo.yres_virtual)
	{	
		y = 0;
	}
    // Clear frame buffer.
    memset((void*)(fb_base + y * finfo.line_length), 0, fb_size);
    vinfo.yoffset = y;
    // Flip to new buffer.
    ioctl(fd, FBIOPAN_DISPLAY, &vinfo);

    munmap((void*)fb_base, finfo.smem_len);
    close (fd);

	return true;

}
static bool ft_cct_init()
{
	//get sensor resolution data
	ACDK_CCT_SENSOR_RESOLUTION_STRUCT  ACDK_Sensor_Info;
	if(!GetSensorSesolutionInfo(&ACDK_Sensor_Info))
		return false;
	ACDK_LOGD("[CCAP]: Get Sensor resolution Info OK!");

	//query sensor data
	ACDK_CCT_SENSOR_INFO_STRUCT ACDK_Sensor;
	if(!GetSersonInfo(&ACDK_Sensor))
		return false;
	ACDK_LOGD("[CCAP]: query Sensor OK!");

	//init statemachine
	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].type						= ACDK_Sensor.Type;
	ACDK_LOGD("[CCAP]: Get Sensor Type:%d",(int)ACDK_Sensor.Type);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].device_id				= ACDK_Sensor.DeviceId;
	ACDK_LOGD("[CCAP]: Get Sensor ID:%u",ACDK_Sensor.DeviceId);

	switch(ACDK_Sensor.StartPixelBayerPtn)
	{
		case SENSOR_OUTPUT_FORMAT_RAW_R:
		g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].start_pixel_bayer_ptn = AP_SENSOR_OUTPUT_FORMAT_RAW_R;
		break;
		case SENSOR_OUTPUT_FORMAT_RAW_Gb:
		g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].start_pixel_bayer_ptn = AP_SENSOR_OUTPUT_FORMAT_RAW_Gb;
		break;
		case SENSOR_OUTPUT_FORMAT_RAW_B:
		g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].start_pixel_bayer_ptn = AP_SENSOR_OUTPUT_FORMAT_RAW_B;
		break;
		case SENSOR_OUTPUT_FORMAT_RAW_Gr:
		default:
		g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].start_pixel_bayer_ptn = AP_SENSOR_OUTPUT_FORMAT_RAW_Gr;
		break;
	}
	ACDK_LOGD("[CCAP]: Get Sensor ptn:%d",(int)g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].start_pixel_bayer_ptn);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].width					= ACDK_Sensor_Info.SensorFullWidth;
	ACDK_LOGD("[CCAP]: Get Sensor width:%u",ACDK_Sensor_Info.SensorFullWidth);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].height					= ACDK_Sensor_Info.SensorFullHeight;
	ACDK_LOGD("[CCAP]: Get Sensor height:%u",ACDK_Sensor_Info.SensorFullHeight);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].preview_width			= ACDK_Sensor_Info.SensorPreviewWidth;
	ACDK_LOGD("[CCAP]: Get Sensor preview_width:%u",ACDK_Sensor_Info.SensorPreviewWidth);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].preview_height			= ACDK_Sensor_Info.SensorPreviewHeight;
	ACDK_LOGD("[CCAP]: Get Sensor preview_height:%u",ACDK_Sensor_Info.SensorPreviewHeight);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].grab_x_offset			= ACDK_Sensor.GrabXOffset;
	ACDK_LOGD("[CCAP]: Get Sensor grab_x_offset:%u",ACDK_Sensor.GrabXOffset);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].grab_y_offset			= ACDK_Sensor.GrabYOffset;
	ACDK_LOGD("[CCAP]: Get Sensor grab_y_offset:%u",ACDK_Sensor.GrabYOffset);

	g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor_count						= 1;


	g_iFullSizeWidth = g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].width;
	g_iFullSizeHeight = g_FT_CCT_StateMachine.sensor_onboard_sensors.sensor[0].height;
	g_nJPGSize = g_iFullSizeWidth * g_iFullSizeHeight / 2;

	g_FT_CCT_StateMachine.p_preview_sensor = NULL;
	bool bEnable = FALSE;

	UINT32 uGroupCnt = 0;
	UINT32 nRealReadByteCnt = 0;

	// TODO
	bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT,NULL,0,(UINT8*)&uGroupCnt,sizeof(UINT32),&nRealReadByteCnt);

	for(; 0 < uGroupCnt; uGroupCnt--)
	{
	UINT8 group_name[128] = {0};
	ACDK_SENSOR_GROUP_INFO_STRUCT ACDK_GroupName;
	memset(&ACDK_GroupName,0,sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT));
	ACDK_GroupName.GroupNamePtr = group_name;

	UINT32 nIndate = uGroupCnt - 1;

	// TODO
	bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA,
	(UINT8*)&nIndate,
	sizeof(UINT32),
	(UINT8*)&ACDK_GroupName,
	sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT),
		&nRealReadByteCnt);

	int item_count = ACDK_GroupName.ItemCount;

	// find CCT group index
	if(!strcmp("CCT", (kal_char*)group_name))
	{
	// found CCT group index
	g_FT_CCT_StateMachine.sensor_eng_group_idx = ACDK_GroupName.GroupIdx;
	ACDK_LOGE("[!!!]ACDK_GroupName.GroupIdx:%d",ACDK_GroupName.GroupIdx);

	for(; 0 < item_count; item_count--)
	{
	ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Item;
	memset(&ACDK_Item,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

	ACDK_Item.GroupIdx = nIndate;
	ACDK_Item.ItemIdx  = item_count - 1;

	bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
	(UINT8*)&ACDK_Item,
	sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
	(UINT8*)&ACDK_Item,
	sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
				&nRealReadByteCnt);
				//return false;

	// find pregain index
	if(!strcmp("Pregain-R", (kal_char*)ACDK_Item.ItemNamePtr))
	{
	g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R = item_count-1;
	}
	else if(!strcmp("Pregain-Gr", (kal_char*)ACDK_Item.ItemNamePtr))
	{
	g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr = item_count-1;
	}
	else if(!strcmp("Pregain-Gb", (kal_char*)ACDK_Item.ItemNamePtr))
	{
	g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gb = item_count-1;
	}
	else if(!strcmp("Pregain-B", (kal_char*)ACDK_Item.ItemNamePtr))
	{
	g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_B = item_count-1;
	}
	}
	}
	}
	ACDK_LOGD("[CCAP]:  Initialize ft cct successful!");


//	InitPreview();
	return true;
}


static MsgFunction g_ACDK_FunctionMap[] =
{
	{FT_CCT_OP_PREVIEW_LCD_STOP,						&FT_ACDK_CCT_OP_PREVIEW_LCD_STOP,					"FT_CCT_OP_PREVIEW_LCD_STOP"},
	{FT_CCT_OP_PREVIEW_LCD_START,					&FT_ACDK_CCT_OP_PREVIEW_LCD_START,					"FT_CCT_OP_PREVIEW_LCD_START"},
//	{FT_CCT_OP_REG_READ,								&FT_ACDK_CCT_OP_REG_READ,							"FT_CCT_OP_REG_READ"},
//	{FT_CCT_OP_REG_WRITE,							&FT_ACDK_CCT_OP_REG_WRITE,							"FT_CCT_OP_REG_WRITE"},
	{FT_CCT_6238_OP_ISP_SET_TUNING_PARAS,			&FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS,				"FT_CCT_V2_OP_ISP_SET_TUNING_PARAS"},
	{FT_CCT_6238_OP_ISP_GET_TUNING_PARAS,			&FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS,				"FT_CCT_V2_OP_ISP_GET_TUNING_PARAS"},
//	{FT_CCT_OP_LOAD_FROM_NVRAM,						&FT_ACDK_CCT_OP_LOAD_FROM_NVRAM,					"FT_CCT_OP_LOAD_FROM_NVRAM"},
//	{FT_CCT_OP_SAVE_TO_NVRAM,						&FT_ACDK_CCT_OP_SAVE_TO_NVRAM,						"FT_CCT_OP_SAVE_TO_NVRAM"},
	{FT_CCT_6238_OP_ISP_SET_TUNING_INDEX,			&FT_ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX,				"FT_CCT_V2_OP_ISP_SET_TUNING_INDEX"},
	{FT_CCT_6238_OP_ISP_GET_TUNING_INDEX,			&FT_ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX,				"FT_CCT_V2_OP_ISP_GET_TUNING_INDEX"},
	{FT_CCT_OP_SINGLE_SHOT_CAPTURE_EX,				&FT_ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX,				"FT_CCT_OP_SINGLE_SHOT_CAPTURE_EX"},
	{FT_CCT_OP_GET_CAPTURE_BUF,						&FT_ACDK_CCT_OP_GET_CAPTURE_BUF,					"FT_CCT_OP_GET_CAPTURE_BUF"},
//	{FT_CCT_OP_SET_ENG_SENSOR_PARA,					&FT_ACDK_CCT_OP_SET_ENG_SENSOR_PARA,				"FT_CCT_OP_SET_ENG_SENSOR_PARA"},
//	{FT_CCT_OP_GET_ENG_SENSOR_PARA,					&FT_ACDK_CCT_OP_GET_ENG_SENSOR_PARA,				"FT_CCT_OP_GET_ENG_SENSOR_PARA"},
	{FT_CCT_OP_QUERY_ISP_ID,							&FT_ACDK_CCT_OP_QUERY_ISP_ID,					"FT_CCT_OP_QUERY_ISP_ID"},
	{FT_CCT_OP_AE_DISABLE,							&FT_ACDK_CCT_OP_AE_DISABLE,							"FT_CCT_OP_AE_DISABLE"},
	{FT_CCT_6238_OP_AWB_DISABLE_AUTO_RUN,				&FT_ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN,					"FT_CCT_6238_OP_AWB_DISABLE_AUTO_RUN"},
	{FT_CCT_6238_OP_AWB_ENABLE_AUTO_RUN,				&FT_ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN,				"FT_CCT_6238_OP_AWB_ENABLE_AUTO_RUN"},
	{FT_CCT_OP_WB_ACTIVATE_BY_INDEX,					&FT_ACDK_CCT_OP_WB_ACTIVATE_BY_INDEX,				"FT_CCT_OP_WB_ACTIVATE_BY_INDEX"},
	{FT_CCT_OP_QUERY_SENSOR,							&FT_ACDK_CCT_OP_QUERY_SENSOR,						"FT_CCT_OP_QUERY_SENSOR"},
	{FT_CCT_6238_OP_AWB_GET_GAIN,					&FT_ACDK_CCT_V2_OP_AWB_GET_GAIN,					"FT_CCT_6238_OP_AWB_GET_GAIN"},
	{FT_CCT_6238_OP_AWB_SET_CURRENT_CCM,				&FT_ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM,				"FT_CCT_6238_OP_AWB_SET_CURRENT_CCM"},
	{FT_CCT_6238_OP_AWB_GET_CURRENT_CCM,				&FT_ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM,				"FT_CCT_6238_OP_AWB_GET_CURRENT_CCM"},
	{FT_CCT_6238_OP_AE_GET_SCENE_MODE,				&FT_ACDK_CCT_V2_OP_AE_GET_SCENE_MODE,								"FT_CCT_6238_OP_AE_GET_SCENE_MODE"},
	{FT_CCT_6238_OP_AE_GET_BAND,						&FT_ACDK_CCT_V2_OP_AE_GET_BAND,							"FT_CCT_6238_OP_AE_GET_BAND"},
	{FT_CCT_6238_OP_AE_GET_GAMMA_BYPASS_FLAG,		&FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG,							"FT_CCT_6238_OP_AE_GET_GAMMA_BYPASS_FLAG"},
	{FT_CCT_6238_OP_AE_GET_METERING_MODE,			&FT_ACDK_CCT_V2_OP_AE_GET_METERING_MODE,						"FT_CCT_6238_OP_AE_GET_METERING_MODE"},
	{FT_CCT_6238_OP_AWB_GET_CCM_PARA,				&FT_ACDK_CCT_V2_OP_AWB_GET_CCM_PARA,						"FT_CCT_6238_OP_AWB_GET_CCM_PARA"},
	{FT_CCT_6238_OP_ISP_GET_SHADING_ON_OFF,			&FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,					"FT_CCT_6238_OP_ISP_GET_SHADING_ON_OFF"},
//	{FT_CCT_OP_GET_SENSOR_PREGAIN,					&FT_ACDK_CCT_OP_GET_SENSOR_PREGAIN,					"FT_CCT_OP_GET_SENSOR_PREGAIN"},
	{FT_CCT_6238_OP_ISP_GET_SHADING_PARA,			&FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,			"FT_CCT_6238_OP_ISP_GET_SHADING_PARA"},
	{FT_CCT_OP_AE_GET_AUTO_EXPO_PARA,			&FT_ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,					"FT_CCT_OP_AE_GET_AUTO_EXPO_PARA"},
	{FT_CCT_6238_OP_ISP_SET_SHADING_ON_OFF,			&FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF,			"FT_CCT_6238_OP_ISP_SET_SHADING_ON_OFF"},
	{FT_CCT_6238_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE,	&FT_ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE,					"FT_CCT_6238_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE"},
	{FT_CCT_6238_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE,	&FT_ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE,					"FT_CCT_6238_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE"},
//	{FT_CCT_V2_OP_SET_OB_ON_OFF,  &FT_ACDK_CCT_V2_OP_SET_OB_ON_OFF,   "FT_CCT_V2_OP_SET_OB_ON_OFF"},
//	{FT_CCT_V2_OP_GET_OB_ON_OFF,  &FT_ACDK_CCT_V2_OP_GET_OB_ON_OFF,   "FT_CCT_V2_OP_GET_OB_ON_OFF"},
//	{FT_CCT_V2_OP_SET_NR_ON_OFF,  &FT_ACDK_CCT_V2_OP_SET_NR_ON_OFF,   "FT_CCT_V2_OP_SET_NR_ON_OFF"},
//	{FT_CCT_V2_OP_GET_NR_ON_OFF,  &FT_ACDK_CCT_V2_OP_GET_NR_ON_OFF,   "FT_CCT_V2_OP_GET_NR_ON_OFF"},
//	{FT_CCT_V2_OP_SET_EE_ON_OFF,  &FT_ACDK_CCT_V2_OP_SET_EE_ON_OFF,   "FT_CCT_V2_OP_SET_EE_ON_OFF"},
//	{FT_CCT_V2_OP_GET_EE_ON_OFF,  &FT_ACDK_CCT_V2_OP_GET_EE_ON_OFF,   "FT_CCT_V2_OP_GET_EE_ON_OFF"},
//	{FT_CCT_6238_ISP_DEFECT_TABLE_ON,				&FT_ACDK_CCT_V2_ISP_DEFECT_TABLE_ON,					"FT_CCT_V2_ISP_DEFECT_TABLE_ON"},
//	{FT_CCT_6238_ISP_DEFECT_TABLE_OFF,				&FT_ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF,					"FT_CCT_V2_ISP_DEFECT_TABLE_OFF"},
	{FT_CCT_6238_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF,			&FT_ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF,					"FT_CCT_6238_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF"},
	{FT_CCT_6238_OP_AWB_ENABLE_DYNAMIC_CCM,			&FT_ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM,			"FT_CCT_6238_OP_AWB_ENABLE_DYNAMIC_CCM"},
	{FT_CCT_6238_OP_AWB_DISABLE_DYNAMIC_CCM,			&FT_ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM,					"FT_CCT_6238_OP_AWB_DISABLE_DYNAMIC_CCM"},
	{FT_CCT_V2_OP_AWB_GET_CCM_STATUS,        &FT_ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS,          "FT_CCT_V2_OP_AWB_GET_CCM_STATUS"},
	{FT_CCT_6238_OP_AWB_GET_NVRAM_CCM,       &FT_ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM,           "FT_CCT_6238_OP_AWB_GET_NVRAM_CCM"},
	{FT_CCT_6238_OP_AWB_SET_NVRAM_CCM,       &FT_ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM,           "FT_CCT_6238_OP_AWB_SET_NVRAM_CCM"},
	{FT_CCT_6238_OP_AWB_UPDATE_CCM_PARA,				&FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA,				"FT_CCT_6238_OP_AWB_UPDATE_CCM_PARA"},
	{FT_CCT_6238_OP_AWB_UPDATE_CCM_STATUS,       &FT_ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS,     "FT_CCT_6238_OP_AWB_UPDATE_CCM_STATUS"},
	{FT_CCT_6238_OP_AE_SET_GAMMA_BYPASS,				&FT_ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS,				"FT_CCT_6238_OP_AE_SET_GAMMA_BYPASS"},
	{FT_CCT_6238_OP_AE_GET_GAMMA_TABLE,				&FT_ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE,					"FT_CCT_6238_OP_AE_GET_GAMMA_TABLE"},
	{FT_CCT_OP_AE_ENABLE,							&FT_ACDK_CCT_OP_AE_ENABLE,							"FT_CCT_OP_AE_ENABLE"},
	{FT_CCT_OP_AE_GET_ENABLE_INFO,     &FT_ACDK_CCT_OP_AE_GET_ENABLE_INFO,     "FT_CCT_OP_AE_GET_ENABLE_INFO"},
	{FT_CCT_6238_OP_ISP_SET_SHADING_PARA,			&FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA,			"FT_CCT_6238_OP_ISP_SET_SHADING_PARA"},
	{FT_CCT_6238_OP_AE_SET_SCENE_MODE,				&FT_ACDK_CCT_V2_OP_AE_SET_SCENE_MODE,					"FT_CCT_6238_OP_AE_SET_SCENE_MODE"},
	{FT_CCT_6238_OP_AE_SET_METERING_MODE,			&FT_ACDK_CCT_V2_OP_AE_SET_METERING_MODE,			"FT_CCT_6238_OP_AE_SET_METERING_MODE"},
	{FT_CCT_V2_OP_AE_APPLY_EXPO_INFO,         &FT_ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,        "FT_CCT_V2_OP_AE_APPLY_EXPO_INFO"},
	{FT_CCT_6238_OP_AE_SELECT_BAND,					&FT_ACDK_CCT_V2_OP_AE_SELECT_BAND,					"FT_CCT_6238_OP_AE_SELECT_BAND"},
	{FT_CCT_OP_AWB_SET_AWB_MODE,            &FT_ACDK_CCT_OP_AWB_SET_AWB_MODE,           "FT_CCT_OP_AWB_SET_AWB_MODE"},
	{FT_CCT_OP_AWB_GET_AWB_MODE,            &FT_ACDK_CCT_OP_AWB_GET_AWB_MODE,           "FT_CCT_OP_AWB_GET_AWB_MODE"},
	{FT_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,    &FT_ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,   "FT_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO"},
	{FT_CCT_V2_OP_AE_SET_GAMMA_TABLE,       &FT_ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE,      "FT_CCT_V2_OP_AE_SET_GAMMA_TABLE"},
	{FT_CCT_6238_OP_AWB_SET_GAIN,           &FT_ACDK_CCT_V2_OP_AWB_SET_GAIN,            "FT_CCT_6238_OP_AWB_SET_GAIN"},
	{FT_CCT_V2_OP_AF_OPERATION,             &FT_ACDK_CCT_V2_OP_AF_OPERATION,            "FT_CCT_V2_OP_AF_OPERATION"},
	{FT_CCT_V2_OP_MF_OPERATION,             &FT_ACDK_CCT_V2_OP_MF_OPERATION,            "FT_CCT_V2_OP_MF_OPERATION"},
	{FT_CCT_V2_OP_AF_GET_BEST_POS,          &FT_ACDK_CCT_V2_OP_AF_GET_BEST_POS,         "FT_CCT_V2_OP_AF_GET_BEST_POS"},
	{FT_CCT_V2_OP_AF_GET_RANGE,             &FT_ACDK_CCT_V2_OP_AF_GET_RANGE,            "FT_CCT_V2_OP_AF_GET_RANGE"},
	{FT_CCT_OP_FLASH_ENABLE,                &FT_ACDK_CCT_OP_FLASH_ENABLE,               "FT_CCT_OP_FLASH_ENABLE"},
	{FT_CCT_OP_FLASH_DISABLE,               &FT_ACDK_CCT_OP_FLASH_DISABLE,              "FT_CCT_OP_FLASH_DISABLE"},
	{FT_CCT_OP_FLASH_GET_INFO,              &FT_ACDK_CCT_OP_FLASH_GET_INFO,             "FT_CCT_OP_FLASH_GET_INFO"},
	{FT_CCT_V2_OP_GET_AF_INFO,              &FT_ACDK_CCT_V2_OP_GET_AF_INFO,             "FT_CCT_V2_OP_GET_AF_INFO"},
	{FT_CCT_V2_OP_AF_CALI_OPERATION,        &FT_ACDK_CCT_V2_OP_AF_CALI_OPERATION,       "FT_CCT_V2_OP_AF_CALI_OPERATION"},
	{FT_CCT_V2_OP_AF_SET_RANGE,             &FT_ACDK_CCT_V2_OP_AF_SET_RANGE,            "FT_CCT_V2_OP_AF_SET_RANGE"},
	{FT_CCT_V2_OP_AF_GET_FV,                &FT_ACDK_CCT_V2_OP_AF_GET_FV,               "FT_CCT_V2_OP_AF_GET_FV"},
	{FT_CCT_V2_OP_AF_READ,                  &FT_ACDK_CCT_V2_OP_AF_READ,                 "FT_CCT_V2_OP_AF_READ"},
	{FT_CCT_V2_OP_AF_APPLY,                 &FT_ACDK_CCT_V2_OP_AF_APPLY,                "FT_CCT_V2_OP_AF_APPLY"},
	{FT_CCT_V2_OP_SHADING_CAL,              &FT_ACDK_CCT_V2_OP_SHADING_CAL,             "FT_CCT_V2_OP_SHADING_CAL"},
	{FT_CCT_V2_OP_DEV_AE_GET_INFO,          &FT_ACDK_CCT_OP_DEV_AE_GET_INFO,            "FT_CCT_V2_OP_DEV_AE_GET_INFO"},
	{FT_CCT_OP_DEV_AE_APPLY_INFO,           &FT_ACDK_CCT_OP_DEV_AE_APPLY_INFO,          "FT_CCT_OP_DEV_AE_APPLY_INFO"},
//	{FT_CCT_OP_DEV_AE_APPLY_MANUAL_INFO,    &FT_ACDK_CCT_OP_DEV_AE_APPLY_MANUAL_INFO,   "FT_CCT_OP_DEV_AE_APPLY_MANUAL_INFO"},
	{FT_CCT_OP_DEV_AE_GET_EV_CALIBRATION,   &FT_ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION,  "FT_CCT_OP_DEV_AE_GET_EV_CALIBRATION"},
	{FT_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2,   &FT_ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2,  "FT_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2"},
	{FT_CCT_V2_OP_AWB_GET_AWB_PARA,         &FT_ACDK_CCT_V2_OP_AWB_GET_AWB_PARA,        "FT_CCT_V2_OP_AWB_GET_AWB_PARA"},
	{FT_CCT_6238_OP_ISP_GET_SHADING_TABLE_V3,  &FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,"FT_CCT_6238_OP_ISP_GET_SHADING_TABLE_V3"},
	{FT_CCT_6238_OP_ISP_SET_SHADING_TABLE_V3,  &FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,"FT_CCT_6238_OP_ISP_SET_SHADING_TABLE_V3"},
	{FT_CCT_V2_OP_ISP_SET_SHADING_INDEX,    &FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,   "FT_CCT_V2_OP_ISP_SET_SHADING_INDEX"},
	{FT_CCT_V2_OP_ISP_GET_SHADING_INDEX,    &FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX,   "FT_CCT_V2_OP_ISP_GET_SHADING_INDEX"},
	{FT_CCT_OP_SET_COMPENSATION_MODE,       &FT_ACDK_CCT_OP_SET_COMPENSATION_MODE,      "FT_CCT_OP_SET_COMPENSATION_MODE"},
	{FT_CCT_OP_GET_COMPENSATION_MODE,       &FT_ACDK_CCT_OP_GET_COMPENSATION_MODE,      "FT_CCT_OP_GET_COMPENSATION_MODE"},
//	{FT_CCT_V2_OP_SAVE_OB_ON_OFF,           &FT_ACDK_CCT_V2_OP_SAVE_OB_ON_OFF,          "FT_CCT_V2_OP_SAVE_OB_ON_OFF"},
	{FT_CCT_OP_CDVT_SENSOR_TEST,             &FT_ACDK_CCT_OP_CDVT_SENSOR_TEST,           "FT_CCT_OP_CDVT_SENSOR_TEST"},
	{FT_CCT_OP_CDVT_SENSOR_CALIBRATION,     &FT_ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION,   "FT_CCT_OP_CDVT_SENSOR_CALIBRATION"},
	{FT_CCT_OP_GET_IMAGE_DIMENSION,     &FT_ACDK_CCT_OP_GET_IMAGE_DIMENSION,   "FT_CCT_OP_GET_IMAGE_DIMENSION"},
	{FT_CCT_OP_SUBPREVIEW_LCD_START,     &FT_ACDK_CCT_OP_SUBPREVIEW_LCD_START,   "FT_CCT_OP_SUBPREVIEW_LCD_START"},
	{FT_CCT_OP_SUBPREVIEW_LCD_STOP,     &FT_ACDK_CCT_OP_SUBPREVIEW_LCD_STOP,   "FT_CCT_OP_SUBPREVIEW_LCD_STOP"},
	{FT_CCT_OP_PHOTOFLASH_CONTROL,     &FT_ACDK_CCT_OP_PHOTOFLASH_CONTROL,   	"FT_CCT_OP_PHOTOFLASH_CONTROL"},
	//6573 CCT new feature ===============================================================
	{FT_CCT_OP_ISP_READ_REG,     &FT_ACDK_CCT_OP_ISP_READ_REG,   "FT_CCT_OP_ISP_READ_REG"},
	{FT_CCT_OP_ISP_WRITE_REG,     &FT_ACDK_CCT_OP_ISP_WRITE_REG,   "FT_CCT_OP_ISP_WRITE_REG"},
	{FT_CCT_OP_READ_SENSOR_REG,     &FT_ACDK_CCT_OP_READ_SENSOR_REG,   "FT_CCT_OP_READ_SENSOR_REG"},
	{FT_CCT_OP_WRITE_SENSOR_REG,     &FT_ACDK_CCT_OP_WRITE_SENSOR_REG,   "FT_CCT_OP_WRITE_SENSOR_REG"},
	{FT_CCT_OP_DEV_AE_SAVE_INFO_NVRAM,     &FT_ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM,   "FT_CCT_OP_DEV_AE_SAVE_INFO_NVRAM"},
	{FT_CCT_V2_OP_AWB_SAVE_AWB_PARA,     &FT_ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA,   "FT_CCT_V2_OP_AWB_SAVE_AWB_PARA"},
	{FT_CCT_V2_OP_AF_SAVE_TO_NVRAM,     &FT_ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM,   "FT_CCT_V2_OP_AF_SAVE_TO_NVRAM"},
	{FT_CCT_OP_ISP_LOAD_FROM_NVRAM,     &FT_ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM,   "FT_CCT_OP_ISP_LOAD_FROM_NVRAM"},
	{FT_CCT_OP_ISP_SAVE_TO_NVRAM,     &FT_ACDK_CCT_OP_ISP_SAVE_TO_NVRAM,   "FT_CCT_OP_ISP_SAVE_TO_NVRAM"},
	{FT_CCT_OP_ISP_SET_PCA_TABLE,     &FT_ACDK_CCT_OP_ISP_SET_PCA_TABLE,   "FT_CCT_OP_ISP_SET_PCA_TABLE"},
	{FT_CCT_OP_ISP_GET_PCA_TABLE,     &FT_ACDK_CCT_OP_ISP_GET_PCA_TABLE,   "FT_CCT_OP_ISP_GET_PCA_TABLE"},
	{FT_CCT_OP_ISP_SET_PCA_PARA,     &FT_ACDK_CCT_OP_ISP_SET_PCA_PARA,   "FT_CCT_OP_ISP_SET_PCA_PARA"},
	{FT_CCT_OP_ISP_GET_PCA_PARA,     &FT_ACDK_CCT_OP_ISP_GET_PCA_PARA,   "FT_CCT_OP_ISP_GET_PCA_PARA"},
	{FT_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,     &FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,     "FT_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF"},
	{FT_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,     &FT_ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,     "FT_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF"},
	{FT_CCT_OP_SET_CCM_MODE,     &FT_ACDK_CCT_OP_SET_CCM_MODE,     "FT_CCT_OP_SET_CCM_MODE"},
	{FT_CCT_OP_GET_CCM_MODE,     &FT_ACDK_CCT_OP_GET_CCM_MODE,     "FT_CCT_OP_GET_CCM_MODE"},
	{FT_CCT_V2_OP_ISP_GET_NVRAM_DATA,	&FT_ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA,	"FT_CCT_V2_OP_ISP_GET_NVRAM_DATA"},
	{FT_CCT_V2_OP_ISP_GET_NVRAM_DATA_BUF,	&FT_ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA_BUF,	"FT_CCT_V2_OP_ISP_GET_NVRAM_DATA_BUF"},
	{FT_CCT_OP_SET_ISP_ON,	&FT_ACDK_CCT_OP_SET_ISP_ON,	"FT_CCT_OP_SET_ISP_ON"},
	{FT_CCT_OP_SET_ISP_OFF,	&FT_ACDK_CCT_OP_SET_ISP_OFF,	"FT_CCT_OP_SET_ISP_OFF"},
	{FT_CCT_OP_GET_ISP_ON_OFF,	&FT_ACDK_CCT_OP_GET_ISP_ON_OFF,	"FT_CCT_OP_GET_ISP_ON_OFF"},
	{FT_CCT_OP_GET_LSC_SENSOR_RESOLUTION, &FT_ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION,"FT_CCT_OP_GET_LSC_SENSOR_RESOLUTION"},
	{FT_CCT_OP_SDTBL_LOAD_FROM_NVRAM, & FT_ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM,"FT_CCT_OP_SDTBL_LOAD_FROM_NVRAM"},
	{FT_CCT_OP_SDTBL_SAVE_TO_NVRAM, &FT_ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM,"FT_CCT_OP_SDTBL_SAVE_TO_NVRAM"},
	{FT_CCT_OP_AWB_GET_LIGHT_PROB, &FT_ACDK_CCT_OP_AWB_GET_LIGHT_PROB, "FT_CCT_OP_AWB_GET_LIGHT_PROB"},
	{FT_CCT_OP_STROBE_RATIO_TUNING, &FT_ACDK_CCT_OP_STROBE_RATIO_TUNING, "FT_CCT_OP_STROBE_RATIO_TUNING"},
	{FT_CCT_OP_SWITCH_CAMERA, &FT_ACDK_CCT_OP_SWITCH_CAMERA,"FT_CCT_OP_SWITCH_CAMERA"},
	{FT_CCT_OP_MAIN2PREVIEW_LCD_START, &FT_ACDK_CCT_OP_MAIN2PREVIEW_LCD_START,"FT_CCT_OP_MAIN2PREVIEW_LCD_START"},
	{FT_CCT_OP_MAIN2PREVIEW_LCD_STOP, &FT_ACDK_CCT_OP_MAIN2PREVIEW_LCD_STOP,"FT_CCT_OP_MAIN2PREVIEW_LCD_STOP"},
	{FT_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF, &FT_ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF,"FT_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF"},
	{FT_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF, &FT_ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF,"FT_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF"},
	{FT_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM, &FT_ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM,"FT_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM"},
	{FT_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM, &FT_ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM,"FT_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM"},
	{FT_CCT_OP_ISP_GET_PCA_SLIDER, &FT_ACDK_CCT_OP_ISP_GET_PCA_SLIDER,"FT_CCT_OP_ISP_GET_PCA_SLIDER"},
	{FT_CCT_OP_ISP_SET_PCA_SLIDER, &FT_ACDK_CCT_OP_ISP_SET_PCA_SLIDER,"FT_CCT_OP_ISP_SET_PCA_SLIDER"},
	{FT_CCT_OP_AE_GET_CAPTURE_PARA, &FT_ACDK_CCT_OP_AE_GET_CAPTURE_PARA,"FT_CCT_OP_AE_GET_CAPTURE_PARA"},
	{FT_CCT_OP_AE_SET_CAPTURE_PARA, &FT_ACDK_CCT_OP_AE_SET_CAPTURE_PARA,"FT_CCT_OP_AE_SET_CAPTURE_PARA"},
	//6582
	{FT_CCT_OP_STROBE_READ_NVRAM_TO_PC, &FT_ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC,"FT_CCT_OP_STROBE_READ_NVRAM_TO_PC"},
	{FT_CCT_OP_STROBE_SET_NVDATA, &FT_ACDK_CCT_OP_STROBE_SET_NVDATA,"FT_CCT_OP_STROBE_SET_NVDATA"},
	{FT_CCT_OP_STROBE_WRITE_NVRAM, &FT_ACDK_CCT_OP_STROBE_WRITE_NVRAM,"FT_CCT_OP_STROBE_WRITE_NVRAM"},
	{FT_CCT_OP_AE_GET_FALRE_CALIBRATION, &FT_ACDK_CCT_OP_AE_GET_FALRE_CALIBRATION,"FT_CCT_OP_AE_GET_FALRE_CALIBRATION"},
	{FT_CCT_OP_AE_PLINE_TABLE_TEST, &FT_ACDK_CCT_OP_AE_PLINE_TABLE_TEST,"FT_CCT_OP_AE_PLINE_TABLE_TEST"},
	{FT_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE, &FT_ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE,"FT_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE"},
	{FT_CCT_OP_END,NULL,NULL}
};


META_BOOL ProcessFT_Command(const FT_CCT_REQ* pREQ,char* pBuff,FT_CCT_CNF* pCNF_Struct)
{
	if(!pREQ ||!pCNF_Struct)
		return FALSE;
	int i = 0;
	while(1)
	{
		if(g_ACDK_FunctionMap[i].OP_ID == FT_CCT_OP_END)
			break;
		if(pREQ->op == g_ACDK_FunctionMap[i].OP_ID)
		{
			ACDK_LOGD("OP=%d, Command=%s", pREQ->op, g_ACDK_FunctionMap[i].pchCommand);
			if((*g_ACDK_FunctionMap[i].call)(pREQ,pCNF_Struct,&pBuff))
				return TRUE;
			return FALSE;
		}
		i++;
	}
	ACDK_LOGE("OP=%d, Command not found!", pREQ->op);
	return FALSE;
}
/************************************************************************/
/* CCAP Interface                                                       */
/************************************************************************/
META_BOOL META_CCAP_init()
{
	if(g_bAcdkOpend)
		return TRUE;
	ACDK_LOGE("[META_CCAP_init] +"); 
    //Temp. Mark
    //if ((MDK_Open() == FALSE) || (CctIF_Open() == FALSE))
  if (Mdk_Open() == FALSE )
  {
      ACDK_LOGE("Mdk_Open() Fail ");
      return FALSE;
  }
  ACDK_LOGD("Mdk_Open() in  META_CCAP_init 1"); 
  if (CctIF_Open() == FALSE)
  {
      ACDK_LOGE("CctIF_Open() Fail ");
      return FALSE;
  }
    //Temp. Mark
    //if ((Mdk_Init() == FALSE) || (CctIF_Init() == FALSE))
	if (Mdk_Init() == FALSE)
       {
           ACDK_LOGE("Mdk_Init() Fail ");
           goto Exit;
       }
  ACDK_LOGD("Mdk_Init() in  META_CCAP_init 2"); 
  if (CctIF_Init(g_FT_CCT_StateMachine.src_device_mode) == FALSE)
  {
           ACDK_LOGE("CctIF_Init() Fail ");
           goto Exit;
  }
	

  g_bAcdkOpend = TRUE;
    g_acdkState = -1;

	ACDK_LOGD("Initialize ACDK device OK!");



	bACDKOpenFlag = TRUE;
	ACDK_LOGE("[META_CCAP_init] -"); 
	return TRUE;
Exit:
	CctIF_DeInit();
    	CctIF_Close();
	Mdk_DeInit();
    	Mdk_Close();
    
  
	ACDK_LOGD("Mdk_DeInit() in  META_CCAP_init 3"); 

    	ACDK_LOGD("umount SDCard file system");
	ACDK_LOGE("[META_CCAP_init] -"); 
    return FALSE;
}


void META_CCAP_deinit()
{
    ACDK_LOGD("META_CCAP_deinit");

    g_acdkState = -1;
	CctIF_DeInit();
       CctIF_Close();
	Mdk_DeInit();
       Mdk_Close();
     
       
	ACDK_LOGD("Mdk_DeInit() in  META_CCAP_deinit"); 
}


void META_CCAP_OP(const FT_CCT_REQ *req, char *peer_buff_in)
{
	if(NULL == req)
		return;

	FT_CCT_CNF	cnf;
	ACDK_LOGE("cnfSize:%d H:%d OP:%d Status:%d Result:%d",sizeof(FT_CCT_CNF),sizeof(FT_H),sizeof(FT_CCT_OP),sizeof(unsigned char),sizeof(FT_CCT_RESULT));
	memset(&cnf,0,sizeof(FT_CCT_CNF));
	g_PeerBufferLen = 0;

	cnf.op				= req->op;
	cnf.status			= FT_CNF_FAIL;
	cnf.header.id		= req->header.id + 1;
	cnf.header.token	= req->header.token;

	if(!g_FT_CCT_StateMachine.is_fb_init)
	  {
		  ACDK_LOGD("[CCAP]:  Begin clean cct fb!"); 
		  // clean cct frame buffer
		  if(ft_fb_init())
			  g_FT_CCT_StateMachine.is_fb_init = KAL_TRUE;
      }
      

	ACDK_LOGD("clean cct fb done"); 
/*
	if(!g_FT_CCT_StateMachine.is_init)
	{
		ACDK_LOGE("[CCAP]:  Begin initialize ft cct!");
		// init isp, sensor and firmware
		if(ft_cct_init())
			g_FT_CCT_StateMachine.is_init = KAL_TRUE;
	}
*/

	//get input data
	if(FALSE == ProcessFT_Command(req,peer_buff_in,&cnf))
	{
		ACDK_LOGE("[CCAP]:  ProcessFT_Command Error!");
		goto EXIT;
	}
	cnf.status			= FT_CNF_OK;
EXIT:

	ACDK_LOGE("[CCAP]:  Begin Send Data to PC g_PeerBufferLen:%d!",g_PeerBufferLen);

	WriteDataToPC(&cnf,sizeof(FT_CCT_CNF),g_pPeerBuffer,g_PeerBufferLen);
	if(g_pPeerBuffer)
	{
		free(g_pPeerBuffer);
		g_pPeerBuffer = NULL;
	}

	ACDK_LOGE("[CCAP]:  End Send Data to PC !");
	return;

}


