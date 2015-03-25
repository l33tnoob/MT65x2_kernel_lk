///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkTest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkTest.cpp
//! \brief

#define LOG_TAG "ACDKTest"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <unistd.h>

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkErrCode.h"
#include "AcdkLog.h"
#include "mtkcam/acdk/AcdkCommon.h"
//#include "cct_feature.h"

extern "C" {
#include <pthread.h>
}


#include "camera_custom_eeprom.h"
#include "mtkcam/acdk/AcdkIF.h"

#define MEDIA_PATH "//data"
#define PROJECT_NAME "Yusu"

typedef int                MBOOL;
typedef void               MVOID; 
#define CHAR                signed char
#define UCHAR               char
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef signed short        INT16;
//typedef signed int          BOOL;
#define BOOL                signed int
//typedef signed int          INT32;
#define INT32               signed int
typedef unsigned int        UINT32;
typedef long long           INT64;
typedef unsigned long long  UINT64;
typedef float               FLOAT;
typedef double              DOUBLE;
typedef void                VOID;

typedef INT32 MRESULT;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif

/////////////////////////////////////////////////////////////////////////
//
//! @brief insert the PMEM driver module shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char*  const g_mkPMEMNod_arg_list[] = {
    "mknod",
    "/dev/pmem_multimedia",
    "c",
    "10",
    "0",
    NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief mount SD shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char* const g_mountSD_arg_list[]  = {
       "mount",
       "-t",
       "vfat",
       "/dev/mmcblk0p1",
       "/sdcard",
       NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief unmount SD shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char*  const g_unMountSD_arg_list[] = {
    "umount",
    "/sdcard",
    NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief sync shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char* const g_sync_arg_list[]  = {
    "sync",
    NULL
};


/////////////////////////////////////////////////////////////////////////
//  Global Variable for the the thread
/////////////////////////////////////////////////////////////////////////
static BOOL g_bIsCLITest = MTRUE;
static pthread_t g_CliKeyThreadHandle;



/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () -
//! @brief the CLI key input thread, wait for CLI command
//! @param sig: The input arguments
/////////////////////////////////////////////////////////////////////////
void thread_exit_handler(MINT32 a_u4Sig)
{
    ACDK_LOGD("This signal is %d \n", a_u4Sig);
    pthread_exit(0);
}


/////////////////////////////////////////////////////////////////////////
//
//  vExecProgram () -
//! @brief execute the external program
//! @param pProgram: program name
//! @param ppArgList: Arguments
/////////////////////////////////////////////////////////////////////////
VOID vExecProgram(const char *pProgram, const char * const ppArgList[])
{
    pid_t childPid;

    //Duplicate this process
    childPid = fork ();

    if (childPid != 0)
    {
        return;
    }
    else
    {
        //execute the program, searching for it in the path
        execvp(pProgram, (char **)ppArgList);
        abort();
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;

    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


/////////////////////////////////////////////////////////////////////////
//
//  getHexToken () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
char* getHexToken(char *inStr, MUINT32 *outVal)
{
    MUINT32 thisVal, tVal;
    char x;
    char *thisStr = inStr;

    thisVal = 0;

    // If first character is ';', we have a comment, so
    // get out of here.

    if (*thisStr == ';')
    {
        return (thisStr);
    }
        // Process hex characters.

    while (*thisStr)
    {
        // Do uppercase conversion if necessary.

        x = *thisStr;
        if ((x >= 'a') && (x <= 'f'))
        {
            x &= ~0x20;
        }
        // Check for valid digits.

        if ( !(((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'F'))))
        {
            break;
        }
        // Hex ASCII to binary conversion.

        tVal = (MUINT32)(x - '0');
        if (tVal > 9)
        {
            tVal -= 7;
        }

        thisVal = (thisVal * 16) + tVal;

        thisStr++;
    }

        // Return updated pointer and decoded value.

    *outVal = thisVal;
    return (thisStr);
}

static MUINT32 g_u4ImgCnt = 0;

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveRAWImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bSaveRAWImg(char *a_pBuf,  MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT32 a_u4Size, UINT8 a_uBitDepth, UINT8 a_uBayerStart)
{
    char szFileName[256];

    UINT8 uBayerStart = 0;

    //convert the bayerstart, //TODO
    switch (a_uBayerStart)
    {
        case 0xB4:
            uBayerStart = 3;
    	    break;
    }

    sprintf(szFileName, "%s//%04d_%s_%dx%d_%d_%d.raw" , MEDIA_PATH,
                                                          g_u4ImgCnt,
                                                          PROJECT_NAME,
                                                          a_u4Width,
                                                          a_u4Height,
                                                          a_uBitDepth,
                                                          uBayerStart);

    MUINT32 *pu4SrcBuf = (MUINT32 *) a_pBuf;
    char *pucBuf = (char *) malloc (a_u4Width * a_u4Height  * 2 * sizeof(char));

    UINT16 *pu2DestBuf = (UINT16 *)pucBuf;

    while (pu2DestBuf < (UINT16 *)pucBuf + a_u4Width * a_u4Height)
    {
        MUINT32 u4Pixel = *(pu4SrcBuf++);
        *(pu2DestBuf++) = (UINT16)(u4Pixel & 0x03FF);
        *(pu2DestBuf++) = (UINT16)((u4Pixel >> 10) & 0x03FF);
        *(pu2DestBuf++) = (UINT16)((u4Pixel >> 20) & 0x03FF);
    }


    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        ACDK_LOGE("Can't open file to save Image\n");
        return MFALSE;
    }

    MINT32 i4WriteCnt = fwrite(pucBuf, 1, a_u4Width * a_u4Height  * 2  , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    free(pucBuf);
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveJPEGImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bSaveJPEGImg(char *a_pBuf,  MUINT32 a_u4Size)
{
    char szFileName[256];

    sprintf(szFileName, "%s//%04d_%s.jpg" , MEDIA_PATH, g_u4ImgCnt, PROJECT_NAME);

    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        ACDK_LOGE("Can't open file to save Image\n");
        return MFALSE;
    }

	MINT32 i4WriteCnt = fwrite(a_pBuf, 1, a_u4Size , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    return MTRUE;
}


/////////////////////////////////////////////////////////////////////////
//!  CLI Test Command
//! For test  -> ACDK interface
/////////////////////////////////////////////////////////////////////////
static BOOL g_bAcdkOpend = MFALSE;
static bool bSendDataToACDK(MINT32   FeatureID,
						    MVOID*					pInAddr,
						    MUINT32					nInBufferSize,
                            MVOID*                  pOutAddr,
						    MUINT32					nOutBufferSize,
						    MUINT32*				pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = (MUINT8*)pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = (MUINT8*)pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (MDK_IOControl(FeatureID, &rAcdkFeatureInfo));
}

static BOOL bCapDone = MFALSE;


/////////////////////////////////////////////////////////////////////////
// vPrvCb
//! @brief capture callback function for ACDK to callback the capture buffer
//! @param a_pParam: the callback image buffer info
//!                              the buffer info will be
//!                              if the buffer type is JPEG, it use CapBufInfo
//!                              if the buffer type is RAW, it use RAWBufInfo
//!
/////////////////////////////////////////////////////////////////////////
static VOID vCapCb(VOID *a_pParam)
{
#if 0
    ACDK_LOGD("Capture Callback \n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    ACDK_LOGD("Buffer Type:%d\n",  pImgBufInfo->eImgType);

    BOOL bRet = MTRUE;

    if (pImgBufInfo->eImgType == RAW_TYPE)
    {
        //! currently the RAW buffer type is packed buffer
        //! The packed format is the same as MT6516 ISP format <00 Pixel1, Pixel2, Pixel3 > in 4bytes
        ACDK_LOGD("Size:%d\n", pImgBufInfo->RAWImgBufInfo.imgSize);
        ACDK_LOGD("Width:%d\n", pImgBufInfo->RAWImgBufInfo.imgWidth);
        ACDK_LOGD("Height:%d\n", pImgBufInfo->RAWImgBufInfo.imgHeight);
        ACDK_LOGD("BitDepth:%d\n", pImgBufInfo->RAWImgBufInfo.bitDepth);
        ACDK_LOGD("Bayer Start:%d\n", pImgBufInfo->RAWImgBufInfo.eColorOrder);

#if 1
        bRet = bSaveRAWImg((char *)pImgBufInfo->RAWImgBufInfo.bufAddr,
                           pImgBufInfo->RAWImgBufInfo.imgWidth,
                           pImgBufInfo->RAWImgBufInfo.imgHeight,
                           pImgBufInfo->RAWImgBufInfo.imgSize,
                           pImgBufInfo->RAWImgBufInfo.bitDepth,
                           pImgBufInfo->RAWImgBufInfo.eColorOrder);
#else  //RAW8 Save
        UCHAR *pBuf = (UCHAR *) malloc (pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width * pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height * 1);

        bRet = bRAW10To8(pImgBufInfo->rRawBufInfo.pucRawBuf,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height,
                                       pImgBufInfo->rRawBufInfo.u4ImgSize,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.uBitDepth,
                                       pBuf);
        FILE *pFp = fopen("/data/test8.raw", "wb");

        if (NULL == pFp )
        {
            ACDK_LOGE("Can't open file to save Image\n");
        }

        MINT32 i4WriteCnt = fwrite(pBuf, 1, pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width * pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height  * 1  , pFp);

        fclose(pFp);
        sync();
        free(pBuf);
#endif

    }
    else if (pImgBufInfo->eImgType == JPEG_TYPE)
   {
        ACDK_LOGD("Size:%d\n", pImgBufInfo->rCapBufInfo.u4ImgSize);
        ACDK_LOGD("Width:%d\n", pImgBufInfo->rCapBufInfo.u2ImgXRes);
        ACDK_LOGD("Height:%d\n", pImgBufInfo->rCapBufInfo.u2ImgYRes)

        bRet = bSaveJPEGImg((char *)pImgBufInfo->rCapBufInfo.pucImgBuf,
                                             pImgBufInfo->rCapBufInfo.u4ImgSize);
   }
   else
   {
        ACDK_LOGD("UnKnow Format \n");
   }

    bCapDone = MTRUE;
    g_u4ImgCnt ++;
#endif

    ACDK_LOGD("[AcdkTest]::Capture Callback\n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;
    bCapDone = MTRUE;

}

/////////////////////////////////////////////////////////////////////////
// vPrvCb
//! @brief preview callback function for ACDK to callback the preview buffer
//! @param a_pParam: the callback image buffer info
//!                              the buffer info will be PrvVDOBufInfo structure
//!
/////////////////////////////////////////////////////////////////////////
static VOID vPrvCb(VOID *a_pParam)
{
    //ACDK_LOGD("Preview Callback \n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    //ACDK_LOGD("Buffer Type:%d\n",  pImgBufInfo->eImgType);
    //ACDK_LOGD("Size:%d\n", pImgBufInfo->imgBufInfo.imgSize);
    //ACDK_LOGD("Width:%d\n", pImgBufInfo->imgBufInfo.imgWidth);
    //ACDK_LOGD("Height:%d\n", pImgBufInfo->imgBufInfo.imgHeight);
}




/////////////////////////////////////////////////////////////////////////
// ACDK_CMD_PREVIEW_START
/////////////////////////////////////////////////////////////////////////
MRESULT mrPreviewStart_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("ACDK_CMD_PREVIEW_START\n");

    ACDK_PREVIEW_STRUCT rPreviewConfig;

    rPreviewConfig.fpPrvCB = vPrvCb;
    rPreviewConfig.u4PrvW = 320;
    rPreviewConfig.u4PrvH = 240;

    MUINT32 u4RetLen = 0;


    BOOL bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START, (UINT8 *)&rPreviewConfig,
                                                                                                                sizeof(ACDK_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
    if (!bRet)
    {
        return ACDK_RETURN_API_FAIL;
    }

    return ACDK_RETURN_NO_ERROR;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CMD_PREVIEW_STOP
/////////////////////////////////////////////////////////////////////////
MRESULT mrPreviewStop_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("ACDK_CMD_PREVIEW_STOP\n");

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &u4RetLen);

    if (!bRet)
    {
        return ACDK_RETURN_API_FAIL;
    }
    return ACDK_RETURN_NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_SINGLE_SHOT_CAPTURE
/////////////////////////////////////////////////////////////////////////
MRESULT mrSingleShot_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    int format;

    if (!g_bAcdkOpend )
    {
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("ACDK_CMD_CAPTURE\n");
    if (a_u4Argc != 2 && a_u4Argc != 4)
    {
        ACDK_LOGD("Usage: cap <mode, prv:0, cap:1> <format, 1:raw, 0:jpg> <width (Option)> <height (Option)>\n");
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_CAPTURE_STRUCT rStillCapConfig;

    MUINT32 u4CapCount;
    MINT32 i4IsSave; //0-don't save, 1-save
    MBOOL bUnPack;

    rStillCapConfig.eCameraMode = (eACDK_CAMERA_MODE)atoi((char *)a_pprArgv[0]);
    rStillCapConfig.u4CapCount = 1;
    rStillCapConfig.i4IsSave = 1;
    format = atoi((char *)a_pprArgv[1]);

    switch(format) {
        case 0:
            rStillCapConfig.eOutputFormat = JPEG_TYPE;
            ACDK_LOGD("[mrSingleShot_Cmd] OUTPUT_JPEG\n");
            break;

        case 1:
            rStillCapConfig.eOutputFormat = PURE_RAW8_TYPE;
            ACDK_LOGD("[mrSingleShot_Cmd] OUTPUT_PURE_RAW8\n");
            break;

        case 2:
            rStillCapConfig.eOutputFormat = PURE_RAW10_TYPE;
            ACDK_LOGD("[mrSingleShot_Cmd] OUTPUT_PURE_RAW10\n");
            break;

        case 3:
            rStillCapConfig.eOutputFormat = PROCESSED_RAW8_TYPE;
            ACDK_LOGD("[mrSingleShot_Cmd] OUTPUT_PROCESSED_RAW8\n");
            break;
        case 4:
            rStillCapConfig.eOutputFormat = PROCESSED_RAW10_TYPE;
            ACDK_LOGD("[mrSingleShot_Cmd] OUTPUT_PROCESSED_RAW10\n");
            break;

        default:
            rStillCapConfig.eOutputFormat = PURE_RAW8_TYPE;
            ACDK_LOGD("[mrSingleShot_Cmd] OUTPUT_PURE_RAW8\n");
            break;
    }


    if  (a_u4Argc == 4)
    {
        rStillCapConfig.u2JPEGEncWidth = atoi((char *)a_pprArgv[2]);
        rStillCapConfig.u2JPEGEncHeight = atoi((char *)a_pprArgv[3]);
    }
    else
    {
        rStillCapConfig.u2JPEGEncWidth = 0;
        rStillCapConfig.u2JPEGEncHeight = 0;
    }
    rStillCapConfig.fpCapCB = vCapCb;
    MUINT32 u4RetLen = 0;

    bCapDone = MFALSE;
    BOOL bRet = bSendDataToACDK(ACDK_CMD_CAPTURE, (UINT8 *)&rStillCapConfig,
                                                                                                                     sizeof(ACDK_CAPTURE_STRUCT),
                                                                                                                     NULL,
                                                                                                                     0,
                                                                                                                     &u4RetLen);

    //wait JPEG Done;
    while (!bCapDone)
    {
        ACDK_LOGD("Capture Waiting...\n");
        usleep(1000);
    }

    ACDK_LOGD("ACDK_CMD_PREVIEW_START\n");

    ACDK_PREVIEW_STRUCT rPreviewConfig;


    rPreviewConfig.fpPrvCB = vPrvCb;
    rPreviewConfig.u4PrvW = 320;
    rPreviewConfig.u4PrvH = 240;

    bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START, (UINT8 *)&rPreviewConfig,
                                                                                                                sizeof(ACDK_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);

    if (!bRet)
    {
        return ACDK_RETURN_API_FAIL;
    }

    return ACDK_RETURN_NO_ERROR;
}


/////////////////////////////////////////////////////////////////////////
//
//  ACDK  CLI Command List  () -
//! @brief
//! @param
/////////////////////////////////////////////////////////////////////////
static const Acdk_CLICmd g_prAcdkCLICmds [] =
{
    //Camera Control
    {"prvstart",                      "ACDK_CMD_PREVIEW_START",       mrPreviewStart_Cmd},
    {"prvstop",                      "ACDK_CMD_PREVIEW_STOP",        mrPreviewStop_Cmd},
    {"cap",                            "ACDK_CMD_CAPTURE, (cap <mode> <type>  <width (Option)> <height (Option)>)",   mrSingleShot_Cmd},

    NULL_CLI_CMD,
};

/////////////////////////////////////////////////////////////////////////
//
//  vHelp () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
VOID vHelp()
{
    if (g_prAcdkCLICmds == NULL)
    {
        ACDK_LOGE("Null Acdk Support Cmds \n");
        return;
    }

    printf("\n***********************************************************\n");
    printf("* ACDK CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n");
    printf("===========================================================\n");
    MUINT32 i = 0;
    while(g_prAcdkCLICmds[ i].pucCmdStr != NULL)
    {
        printf("%s    [%s]\n", g_prAcdkCLICmds[ i].pucCmdStr,
                                g_prAcdkCLICmds[ i].pucHelpStr);
        i++;
    }
    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () -
//! @brief the CLI key input thread, wait for CLI command
//! @param a_pArg: The input arguments
/////////////////////////////////////////////////////////////////////////
VOID* cliKeyThread (VOID *a_pArg)
{
    char urCmds[256] = {0};

    //! ************************************************
    //! Set the signal for kill self thread
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal
    //! to kill self
    //! ************************************************
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);

    while (1)
    {

        printf("Input Cmd#");
        fgets(urCmds, 256, stdin);

        //remove the '\n'
        urCmds[strlen(urCmds)-1] = '\0';
        char *pCmds = &urCmds[0];
        //remove the space in the front of the string
        vSkipSpace(&pCmds);

        //Ignore blank command
        if (*pCmds == '\0')
        {
            continue;
        }

        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        MUINT8  *pucStrToken, *pucCmdToken;
        MUINT8  *pucArgValues[MAX_CLI_CMD_ARGS];

        pucStrToken = (MUINT8*)strtok(pCmds, " ");

        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] = pucStrToken;
            pucStrToken = (MUINT8*)strtok (NULL, " ");
        }

        if (u4ArgCount == 0)
        {
            continue;
        }

        pucCmdToken = pucArgValues[0];

        //parse the command
        if ((strcmp((char*)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp();
        }
        else if ((strcmp((char*)pucCmdToken, "exit") == 0) ||
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            ACDK_LOGD("Exit From CLI\n");
            g_bIsCLITest = MFALSE;
        }
        else
        {
            MINT32 i4CmdIndex = 0;
            BOOL bIsFoundCmd = MFALSE;
            while (g_prAcdkCLICmds[i4CmdIndex].pucCmdStr != NULL)
            {
                if (strcmp((char*)pucCmdToken, g_prAcdkCLICmds[i4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE;
                    g_prAcdkCLICmds[i4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);
                    break;
                }
                i4CmdIndex++;
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n");
            }
        }

    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//  main () -
//! @brief The main function for sensor test, it will create two thread
//!        one is for CLI command
//!        the other is for keypad input command
//! @param argc: The number of the input argument
//! @param argv: The input arguments
/////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    MUINT32 u4RealOutLen = 0;

    ACDK_LOGD(" Acdk CLI Test\n");

    //! *************************************************
    //! Create the related object and init/enable it
    //! *************************************************
    if (MDK_Open() == MFALSE)
    {
        ACDK_LOGE("MDK_Open() Fail \n");
        goto Exit;
    }


    if (MDK_Init() == MFALSE)
    {
        ACDK_LOGE("MDK_Init() Fail \n");
        goto Exit;
    }

    g_bAcdkOpend = MTRUE;

    //! *************************************************
    //! Create the CLI command thread to listen input CLI command
    //! *************************************************
    ACDK_LOGD(" Create the CLI thread \n");

    vHelp();
    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest== MTRUE)
    {
        sleep(1);
    }

    //!***************************************************
    //! Receive the exit command, cancel the two thread
    //!***************************************************
    int status;
    ACDK_LOGD("Cancel cli key thread\n");
    if ((status = pthread_kill(g_CliKeyThreadHandle, SIGUSR1)) != 0)
    {
         ACDK_LOGE("Error cancelling thread %d, error = %d (%s)\n", (int)g_CliKeyThreadHandle,
               status, (char*)strerror);
    }


Exit:

    //!***************************************************
    //! Exit
    //! 1. DeInit ACDK device and close it
    //! 2. Umount the SD card and sync the file to ensure
    //!    all files are written to SD card
    //! 3. Sync all file to SD card to ensure the files are saving in SD
    //!***************************************************
    MDK_DeInit();
    MDK_Close();
    ACDK_LOGD("umount SDCard file system\n");

    return 0;
}


