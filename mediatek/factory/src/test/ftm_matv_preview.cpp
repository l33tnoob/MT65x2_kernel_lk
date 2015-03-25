/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

extern "C" {
#include "common.h"
#include "miniui.h"
#include "ftm.h"
}
#include <mtkcam/acdk/MdkIF.h>

#include <stdlib.h>

///#define FOR_FACOTRY_MODE

#include <stdio.h>
extern "C" {
#include "jpeglib.h"
#include <setjmp.h>
}
#define TAG                  "[MATV_PREV] "
#define atv_abs(a) (((a) < 0) ? -(a) : (a)) 

extern const char * CfgFileNamesdcard ;
extern const char * CfgFileName   ;
extern bool sdcard_insert ;


static int bSendDataToACDK(MUINT32  FeatureID,
                                   MUINT8   *pInAddr,
                                   MUINT32  nInBufferSize,
                                   MUINT8   *pOutAddr,
                                   MUINT32  nOutBufferSize,
                                   MUINT32  *pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo; 

    rAcdkFeatureInfo.puParaIn = pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (Mdk_IOControl(FeatureID, &rAcdkFeatureInfo));
}

#ifdef __cplusplus
extern "C" {
#endif
static volatile int atvcapture_done = FALSE;
static char szCapFileName[256];
static char szRawFileName[256];

static unsigned int g_ImgCnt = 1;

struct my_error_mgr
{
    struct jpeg_error_mgr pub;    /* "public" fields */

    jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;


METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#define MAX(a,b)    ((a) > (b) ? (a) : (b))

double RGB2HSLH(int red,int green, int blue, double &H, double &S)
{
    double R,G,B,Max,Min,del_R,del_G,del_B,del_Max;    
    double L=0;
    bool enablelog = false;
    if((red == 241)&&(green==62)&&(blue ==58))
        enablelog = false;
    
    R = (double)red / 255.0;
    G = (double)green / 255.0;
    B = (double)blue / 255.0;

    if(enablelog)
        LOGD("RGB2HSLH1:%4.3f, %4.3f, %4.3f", R, G, B);
    Min = MIN(R, MIN(G, B));    
    Max = MAX(R, MAX(G, B));    
    del_Max = Max - Min;        

    L = (Max + Min) / 2.0;

    if(enablelog)
        LOGD("RGB2HSLH2:%4.3f %4.3f, %4.3f", Min, Max, del_Max);

    if (del_Max == 0)          
    {
        H = 0;                 
        S = 0;
    }
    else                        //Chromatic data...
    {
        if (L < 0.5) 
            S = del_Max / (Max + Min);
        else         
            S = del_Max / (2 - Max - Min);

        del_R = (((Max - R) / 6.0) + (del_Max / 2.0)) / del_Max;
        del_G = (((Max - G) / 6.0) + (del_Max / 2.0)) / del_Max;
        del_B = (((Max - B) / 6.0) + (del_Max / 2.0)) / del_Max;
        if(enablelog)
            LOGD("RGB2HSLH3:%4.3f %4.3f, %4.3f", del_R, del_G, del_B);

        if(R == Max) 
            H = del_B - del_G;
        else if (G == Max) 
            H = (1.0 / 3.0) + del_R - del_B;
        else if (B == Max) 
            H = (2.0 / 3.0) + del_G - del_R;

        if (H < 0)  H += 1;
        if (H > 1)  H -= 1;
    }
    if(enablelog)
        LOGD("RGB2HSLH4:%4.3f", H);
    return L;
}

bool MATVCheckPinToggle(char *frame_buffer,int frame_width,int frame_height)
{
    char *pSrc;
    char MATVPinToggle = 0;
    char Golden_Byte; 
    int i,j;
    bool ret = false;
    bool bfirst_check = true;

    pSrc = (char *) frame_buffer;
    if (bfirst_check)
    {
        Golden_Byte = *pSrc;
        bfirst_check = false;
    }

    for (j=1;j<frame_width*frame_height*3;j++)
    {
        for (i=0;i<=7;i++)
        {
            if((*(pSrc+j) & (0x1<<i)) != (Golden_Byte & (0x1<<i)))
            {
                MATVPinToggle = MATVPinToggle | (0x1<<i);
            }
        }
        
        if (MATVPinToggle == 0xff)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

int parse_JPEG_file(char *jpgfilename, char *rawfilename)
{
    /* This struct contains the JPEG decompression parameters and pointers to
    * working space (which is allocated as needed by the JPEG library).
    */
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE *infile;
    JSAMPARRAY buffer;
    int row_stride;

    if ((infile = fopen(jpgfilename, "rb")) == NULL)
    {
        LOGD("can't open %s", jpgfilename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }

    FILE *pFp = fopen(rawfilename, "wb");

    if (NULL == pFp)
    {
        LOGD("Can't open file to save raw Image");
        return 0;
    }
    int i4WriteCnt = 0;

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);


    (void) jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB;

    (void) jpeg_start_decompress(&cinfo);
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo.mem->alloc_sarray)
             ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);

        i4WriteCnt = fwrite(buffer[0], 1, row_stride , pFp);
        ///LOGD("saving file to save raw Image w%d wd%d ", row_stride, i4WriteCnt);
        ///put_scanline_someplace(buffer[0], row_stride);
    }


    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);
    fclose(pFp);

    return 1;
}


int matv_preview_init()
{
    g_ImgCnt = 1;
    LOGD(TAG"matv_preview_init + ");

    if (Mdk_Open() == 0)
    {
        LOGE(TAG"Mdk_Open() Fail ");
        return -1;
    }

    //set ATV mode
    LOGD(TAG"Set ATV Mode ");
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
    MINT32 srcDev = 4;//2; //ATV
    rAcdkFeatureInfo.puParaIn = (MUINT8 *)&srcDev;
    rAcdkFeatureInfo.u4ParaInLen = sizeof(MINT32);
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = NULL;
    Mdk_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);

    LOGD(TAG"Init ACDK ");

    if (Mdk_Init() == 0)
    {
        LOGE(TAG"Mdk_Init() Fail ");
        return -1;
    }
    
    LOGD(TAG"matv_preview_init - ");

    return 0;
}

int matv_preview_deinit()
{
    Mdk_DeInit();
    Mdk_Close();
    return 0;
}

int matv_preview_start()
{
    LOGD(TAG"ACDK_CCT_OP_PREVIEW_LCD_START");

    ACDK_PREVIEW_STRUCT rCCTPreviewConfig;
    rCCTPreviewConfig.fpPrvCB = NULL;
    rCCTPreviewConfig.u4PrvW = 320;
    rCCTPreviewConfig.u4PrvH = 240;

    unsigned int u4RetLen = 0;


    bool bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_START, (unsigned char *)&rCCTPreviewConfig,
                                sizeof(ACDK_PREVIEW_STRUCT),
                                NULL,
                                0,
                                &u4RetLen);

    if (!bRet)
    {
        LOGE(TAG"ACDK_CCT_OP_PREVIEW_LCD_START Fail");
    }

    //camera_state = CAMERA_STATE_PREVIEW;

    return 0;
}

int matv_preview_stop()
{
    LOGD(TAG"ACDK_CCT_OP_PREVIEW_LCD_STOP ");

    unsigned int u4RetLen = 0;
    bool bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &u4RetLen);

    if (!bRet)
    {
        return 1;
    }

    //camera_state = CAMERA_STATE_IDLE;
    return 0;
}

int matv_preview_reset_layer_buffer(void)
{
    unsigned int u4RetLen = 0;
    bool bRet = 0;
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = NULL;
    rAcdkFeatureInfo.u4ParaInLen = 0;
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

    bRet = Mdk_IOControl(ACDK_CMD_RESET_LAYER_BUFFER, &rAcdkFeatureInfo);

    if (!bRet)
    {
        return 1;
    }

    return 0;
}

bool mATVSaveJPEGImg(char *a_pBuf,  unsigned int a_u4Size)
{
    if(sdcard_insert)
        sprintf(szCapFileName, "/sdcard/matv_%04d.jpg" , g_ImgCnt);
    else
        sprintf(szCapFileName, "/data/matv_%04d.jpg" , g_ImgCnt);
    
    LOGD("Save image file name:%s", szCapFileName);

    FILE *pFp = fopen(szCapFileName, "wb");

    if (NULL == pFp)
    {
        LOGD("Can't open file to save Image");
        return FALSE;
    }

    int i4WriteCnt = fwrite(a_pBuf, 1, a_u4Size , pFp);   
    fclose(pFp);
    
    sprintf(szRawFileName, "/data/matv_%04d.raw" , g_ImgCnt);

    return TRUE;
}

void vATVCapCb(void *a_pParam)
{
    LOGD("matv Capture Callback ");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    if (pImgBufInfo->eImgType == JPEG_TYPE)
    {
        LOGD("Size:%d", pImgBufInfo->imgBufInfo.imgSize);
        LOGD("Width:%d", pImgBufInfo->imgBufInfo.imgWidth);
        LOGD("Height:%d", pImgBufInfo->imgBufInfo.imgHeight);

        mATVSaveJPEGImg((char *)pImgBufInfo->imgBufInfo.bufAddr,
                        pImgBufInfo->imgBufInfo.imgSize);
    }
    else
    {
        LOGD("UnKnow Format ");
        atvcapture_done = FALSE;
        return;
    }

    atvcapture_done = TRUE;
}


/////////////////////////////////////////////////////////////////////////
//
//   camera_capture_test () -
//!
//!  brief for camera capture test
//
/////////////////////////////////////////////////////////////////////////
#define  ATV_PATTERN_THRESHOLD 15 // 15%

bool matv_capture_check()
{
    
    bool return_value = true; 
    char *srcraw = NULL;
    char *dstraw = NULL;
    FILE *psrcfp = NULL;
    FILE *pdstfp = NULL;
    unsigned int err_cnt = 0;
	unsigned int pattern_count = 0;
    int srcbytes=0;
    int dstbytes=0;
    
    int readRedY = 0;
    int readGreenY = 0;
    int readBlueY = 0;
    int readRedOldY = 0;
    int readGreenOldY = 0;
    int readBlueOldY = 0;
    int LastHue = 0;

    LOGD("matv_capture-");

    LOGD("matv Get Sensor Resolution Info");

    unsigned int u4RetLen = 0;
    MINT32 sensorType = -1;
    bool bRet = false;

    //====== Get Sensor Resolution Info ======
#if 0
    ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;
    memset(&SensorResolution, 0, sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));
    bRet = bSendDataToACDK(ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION,
                                NULL,
                                0,
                                (UINT8 *)&SensorResolution,
                                sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                &u4RetLen);


    if (!bRet)
    {
        LOGD("[matvcapture] Get Sensor Resolution Fail");
        return_value = false;
        goto capture_exit;
    }
    else
    {
        LOGD("[matvcapture] SensorPreviewWidth  = %u", SensorResolution.SensorPreviewWidth);
        LOGD("[matvcapture] SensorPreviewHeight = %u", SensorResolution.SensorPreviewHeight);
        LOGD("[matvcapture] SensorFullWidht     = %u", SensorResolution.SensorFullWidth);
        LOGD("[matvcapture] SensorFullHeight    = %u", SensorResolution.SensorFullHeight);
    }

    //====== Get Sensor Type Info ======
    bRet = bSendDataToACDK(ACDK_CCT_OP_GET_SENSOR_TYPE,
                           NULL,
                           0,
                           (UINT8 *)&sensorType,
                           sizeof(MINT32),
                           &u4RetLen);

    if (!bRet)
    {
        LOGD("[matvcapture] Get Sensor Resolution Fail");
        return_value = false;
        goto capture_exit;
    }
    else
    {
        LOGD("[matvcapture] sensorType = %d", sensorType);
    }
#endif

      

#if 1  ///for 72 should enable
	ACDK_CAPTURE_STRUCT rCCTStillCapConfig;//gellmann

    rCCTStillCapConfig.eCameraMode = CAPTURE_MODE;//gellmann
    
    LOGD("[matvcapture]normal");

	rCCTStillCapConfig.eOutputFormat = JPEG_TYPE;

        //align to 16x
//        rCCTStillCapConfig.u2JPEGEncWidth =  SensorResolution.SensorFullWidht & (~0xF);
//        rCCTStillCapConfig.u2JPEGEncHeight =  SensorResolution.SensorFullHeight & (~0xF);
	rCCTStillCapConfig.u2JPEGEncWidth =  312 & (~0xF);     //will be alignd to 16x
	rCCTStillCapConfig.u2JPEGEncHeight =	238 & (~0xF);    //will be alignd to 16x

    rCCTStillCapConfig.fpCapCB = vATVCapCb;
    
#endif

#ifdef new_preview_cct

        rCCTStillCapConfig.eCameraMode   = CAPTURE_MODE;
        rCCTStillCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;
        rCCTStillCapConfig.eOutputFormat = OUTPUT_JPEG;
        rCCTStillCapConfig.u2JPEGEncWidth =  312 & (~0xF);     //will be alignd to 16x
        rCCTStillCapConfig.u2JPEGEncHeight =  238 & (~0xF);    //will be alignd to 16x
        rCCTStillCapConfig.fpCapCB = vATVCapCb;
        rCCTStillCapConfig.i4IsSave = 1;    // 0-no save, 1-save   
    
#endif

    atvcapture_done = FALSE;

#if 1  ///for 72 should enable
		bRet = bSendDataToACDK(ACDK_CMD_CAPTURE,
                           (unsigned char *)&rCCTStillCapConfig,
						   sizeof(ACDK_CAPTURE_STRUCT),
                           NULL,
                           0,
                           &u4RetLen);

#endif

    //wait JPEG Done;
    while (!atvcapture_done)
    {
        usleep(5000);
    }
    
    g_ImgCnt++;
    
    LOGD("[matvcapture] - done");

    if (!bRet)
    {
        return_value = false;
        goto capture_exit;
    }

    //check pattern.
    parse_JPEG_file(szCapFileName, szRawFileName);
    if(fopen("/data/matv_pattern.raw", "r") == NULL)
    {
        LOGD("[matvcapture] parser matv_patter to raw");
        if(sdcard_insert)
            parse_JPEG_file("/sdcard/matv_pattern.jpg", "/data/matv_pattern.raw");
        else
            parse_JPEG_file("/system/res/matv/matv_pattern.jpg", "/data/matv_pattern.raw");
    }

    srcraw = new char[3* 304 *224];
    dstraw = new char[3* 304 *224];

    if((srcraw == NULL) || (dstraw ==NULL))
    {
        LOGD("[matvcapture] allocate memory fail");
        return_value = false;
        goto capture_exit;
    }
    psrcfp = fopen(szRawFileName,"r");
    pdstfp = fopen("/data/matv_pattern.raw","r");  ///"/res/images/matv_pattern.raw"

    if((psrcfp == NULL) || (pdstfp == NULL))
    {
        LOGD("[matvcapture] open compare file fail");
        return_value = false;
        goto capture_exit;
    }

    srcbytes=fread(srcraw, 1, 3* 304 *224, psrcfp);
    dstbytes=fread(dstraw, 1, 3* 304 *224, pdstfp);

#if 0   
//    for(int i=0; i< 3* 304 *224;i++)
	for(int i=3* 304 *20; i< 3* 304 *(224-70);i++)
    {
		pattern_count = dstraw[i]+pattern_count;

		if(atv_abs(srcraw[i] - dstraw[i]) > 10)
		{
           err_cnt = atv_abs(srcraw[i] - dstraw[i])+err_cnt;
		}

	
//        if(atv_abs(srcraw[i] - dstraw[i]) > 10)
//            err_cnt++;
    }

    if ((pattern_count*ATV_PATTERN_THRESHOLD/100) <= err_cnt)
        return_value = false;
	
    LOGD("Preview check i: %d pattern_count: %d, err_count: %d, ret %d", g_ImgCnt-1, pattern_count, err_cnt, return_value);
		
#else
  	if(MATVCheckPinToggle(srcraw, 304, 224)==false)
    {
        LOGD("MATVCheckPinToggle fail!!!!");
        return false;
    }
    LOGD("MATVCheckPinToggle ok!!!!");
    
    for(int i=0; i< 3* 304 *224;i +=3)
    {
        int Sue = 0;
        double Dsue = 0.0;
        int Hue = 0;
        double DHue = 0.0 ;
        RGB2HSLH(srcraw[i], srcraw[i+1], srcraw[i+2], DHue, Dsue);
        ///Sue = (int)(Dsue* 240+0.5);
        Hue = (int)(DHue* 240+0.5) ;

	    ///LOGD("red pixel %d, U %d--%d, %d", i/3, Hue, LastHue, Sue);
        if(Hue <= 20 || Hue >= 230 )
        {
            readRedOldY++;
        }
        if(Hue<= 100 && Hue >= 60	)
        {
    	    readGreenOldY++;
    	}
        if(Hue<= 180 && Hue >= 140)
        {
    	    readBlueOldY++;
    	}
        
        if((atv_abs(Hue-LastHue) > 30)
            && (atv_abs(Hue-LastHue) < 235))
    	{
    	    LOGD("Preview reset %d, U %d--%d off %d, %d, %d, %d", i/3, Hue, LastHue, atv_abs(Hue-LastHue), readRedOldY, readGreenOldY, readBlueOldY);
            readRedOldY = 0;
     	    readGreenOldY = 0;
    	    readBlueOldY = 0;
            
    	}
        
        LastHue = Hue;

        if(readRedOldY > readRedY)
        {    
            readRedY = readRedOldY;
    	}        
        if(readGreenOldY > readGreenY)
        {    
            readGreenY = readGreenOldY;
    	}
        if(readBlueOldY > readBlueY)
        {    
            readBlueY = readBlueOldY;
        }
        ///if((i%30)==0)
        ///    usleep(5*1000);
        
        
    }

//#endif

    ///LOGD("[matvcapture] srcbytes %d, dstbytes %d, err_cnt is %d", srcbytes, dstbytes, err_cnt);
        
    if((readRedY < 304 *10)
        || (readRedY < 304 *10)
        || (readRedY < 304 *10))
        return_value = false;
    
    LOGD("Preview check i: %d red: %d, Green: %d, Blue: %d, ret %d", g_ImgCnt-1, readRedY, readGreenY, readBlueY, return_value);
#endif
	

capture_exit:
    
    if(srcraw != NULL) 
        delete [] srcraw;
    if(dstraw != NULL) 
        delete [] dstraw;

    if((psrcfp != NULL))
        fclose(psrcfp);
    if((pdstfp != NULL))
        fclose(pdstfp); 

    //remove temp file
    char buf[256];
    ///sprintf(buf, "rm %s",szCapFileName);    
    ///system(buf);
    memset(buf, 0, 256);
    ///sprintf(buf, "rm %s",szRawFileName);    
    ///system(buf);

    return return_value;
}


#ifdef __cplusplus
};
#endif

