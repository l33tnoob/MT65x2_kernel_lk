/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

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
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief
 
#define LOG_TAG "CamShotTest"


#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>

//
#include <mtkcam/camshot/ISImager.h>

extern "C" {
#include <pthread.h>
}

//
#include <mtkcam/drv/imem_drv.h>

using namespace NSCamShot; 
using namespace NSCamHW; 

/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)

#define TARGET_WIDTH    640
#define TARGET_HEIGHT   480 

static  IMemDrv *g_pIMemDrv;

static pthread_t g_CliKeyThreadHandle; 
static MBOOL g_bIsCLITest = MTRUE;

static ISImager *g_pISImager = NULL; 

static IMEM_BUF_INFO g_rOutMem; 

static EImageFormat g_eImgFmt[] = {eImgFmt_YUY2, eImgFmt_NV21, eImgFmt_I420, eImgFmt_YV16, eImgFmt_JPEG} ; 

//
static MUINT32 g_u4TargetFmt = 0; 
static MUINT32 g_u4TargetWidth = 0, g_u4TargetHeight = 0; 
static MUINT32 g_u4TargetRotation = 0;
//
static MUINT32 g_u4SrcWidth = 0, g_u4SrcHeight = 0; 


/******************************************************************************
* 
*******************************************************************************/
static void allocMem(IMEM_BUF_INFO &memBuf) 
{
    if (g_pIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (g_pIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
}

/******************************************************************************
* 
*******************************************************************************/
static void deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (g_pIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if (g_pIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }        
}



/******************************************************************************
* save the buffer to the file 
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size); 
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true; 
}


/////////////////////////////////////////////////////////////////////////
//! Nucamera commands
/////////////////////////////////////////////////////////////////////////
typedef struct CLICmd_t
{
    //! Command string, include shortcut key
    const char *pucCmdStr;

    //! Help string, include functionality and parameter description
    const char *pucHelpStr;

    //! Handling function
    //! \param a_u4Argc  [IN] Number of arguments plus 1
    //! \param a_pprArgv [IN] Array of command and arguments, element 0 is
    //!                       command string
    //! \return error code
    //FIXME: return MRESULT is good?
    MUINT32 (*handleCmd)(const int argc, char** argv);

} CLICmd;

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4SetFormat_Cmd(int argc, char** argv)
{
    if (argc != 1)
    {
        printf("setFormat <format:0:YUY2, 1:NV21, 2:I420, 3:YV16, 4:JPEG>\n"); 
        return 1; 
    }
    g_u4TargetFmt = atoi(argv[0]); 
    if (g_u4TargetFmt > 4) 
    {
        g_u4TargetFmt = 4; 
    }
    g_pISImager->setFormat(g_eImgFmt[g_u4TargetFmt]); 
    return 0; 
} 

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4SetRotation_Cmd(int argc, char** argv)
{
    if (argc != 1)
    {
        printf("setRotation <rotation:0:0, 1:90, 2:180, 3:270>\n"); 
        return 1; 
    }
    
    g_u4TargetRotation = atoi(argv[0]) * 90;
    g_pISImager->setRotation( g_u4TargetRotation ); 
    return 0; 
} 

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4SetFlip_Cmd(int argc, char** argv)
{
    if (argc != 1)
    {
        printf("setFlip <Flip:0,1>\n"); 
        return 1; 
    }
    
    g_pISImager->setFlip(atoi(argv[0])); 

    return 0; 
} 

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4SetEnc_Cmd(int argc, char** argv)
{
    if (argc != 2) 
    {
        printf("setEnc <SOI> <Quality:0~100>\n"); 
        return 1; 
    }

    g_pISImager->setEncodeParam(atoi(argv[0]), atoi(argv[1])); 
    return 0; 
} 

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4SetSize_Cmd(int argc, char** argv)
{
    if (argc != 2) 
    {
        printf("setSize <width> <height>\n"); 
        return 1; 
    }
    g_u4TargetWidth = atoi(argv[0]); 
    g_u4TargetHeight = atoi(argv[1]); 

    deallocMem(g_rOutMem); 
    //    
    g_rOutMem.size = (g_u4TargetWidth * g_u4TargetHeight * 2 + L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1); 
    allocMem(g_rOutMem);     

    g_pISImager->setResize(g_u4TargetWidth, g_u4TargetHeight); 

    BufInfo rBufInfo(g_rOutMem.size, g_rOutMem.virtAddr, g_rOutMem.phyAddr, g_rOutMem.memID); 
    //
    g_pISImager->setTargetBufInfo(rBufInfo); 
   
    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4SetROI_Cmd(int argc, char** argv)
{
    if (argc != 4) 
    {
        printf("setROI <x> <y> <w> <h>\n"); 
        return 1; 
    }
    Rect rROI;

    rROI.x = atoi(argv[0]); 
    rROI.y = atoi(argv[1]); 
    rROI.w = atoi(argv[2]); 
    rROI.h = atoi(argv[3]); 
 
    if ((rROI.x + rROI.w) > g_u4SrcWidth || (rROI.y + rROI.h) > g_u4SrcHeight) 
    {
        printf("error ROI=(%d,%d,%d,%d)\n", rROI.x, rROI.y, rROI.w, rROI.h); 
        return 1; 
    }

    g_pISImager->setROI(rROI); 

    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MUINT32 u4Execute_Cmd(int argc, char** argv)
{
    g_pISImager->execute(); 
    //
    if (eImgFmt_JPEG == g_eImgFmt[g_u4TargetFmt])
    {        
        saveBufToFile("/data/result.jpg", reinterpret_cast<MUINT8*>(g_rOutMem.virtAddr), g_pISImager->getJpegSize()); 
    }
    else 
    {
        char filename[256];      
        if( g_u4TargetRotation == 90 || g_u4TargetRotation == 270 )
        {
            sprintf(filename, "/data/result%dx%d.yuv", 
                                g_u4TargetHeight, g_u4TargetWidth);      
        }
        else {
            sprintf(filename, "/data/result%dx%d.yuv", 
                                g_u4TargetWidth, g_u4TargetHeight);      
        }
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(g_rOutMem.virtAddr), g_rOutMem.size);     
    }

    return 0; 
}


/////////////////////////////////////////////////////////////////////////
//
//!  The cli command for the manucalibration
//!
/////////////////////////////////////////////////////////////////////////
static CLICmd g_pSImager_Cmds[] =
{
    {"setFormat", "setFormat <format:0:YUY2, 1:NV21, 2:I420, 3:YV16, 4:JPEG>", u4SetFormat_Cmd},
    {"setRotation", "setRotation <rotation:0:0, 1:90, 2:180, 3:270>", u4SetRotation_Cmd},
    {"setFlip", "setFlip <Flip:0,1>", u4SetFlip_Cmd},
    {"setEnc", "setEnc <SOI> <Quality:0~100>", u4SetEnc_Cmd},
    {"setSize", "setSize <width> <height>", u4SetSize_Cmd},
    {"setROI",  "setROI <x> <y> <w> <h>", u4SetROI_Cmd}, 
    {"do", "do", u4Execute_Cmd},
    {NULL, NULL, NULL}
};


/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param sig: The input arguments 
/////////////////////////////////////////////////////////////////////////
static void thread_exit_handler(int sig)
{ 
    printf("This signal is %d \n", sig);
    pthread_exit(0);
}    

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
static void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;
    
    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


//  vHelp () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
static void vHelp()
{
    printf("\n***********************************************************\n");
    printf("* SImager CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n"); 
    printf("===========================================================\n");    

    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");

    int i = 0; 
    for (i = 0; ; i++)
    {
        if (NULL == g_pSImager_Cmds[i].pucCmdStr) 
        {
            break; 
        } 
        printf("%s    [%s]\n", g_pSImager_Cmds[i].pucCmdStr, 
                               g_pSImager_Cmds[i].pucHelpStr);        
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param a_pArg: The input arguments 
/////////////////////////////////////////////////////////////////////////
static void* cliKeyThread (void *a_pArg)
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
        char  *pucStrToken, *pucCmdToken;
        char  *pucArgValues[25];
        
        pucStrToken = (char *)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] =(char*) pucStrToken;
            pucStrToken = (char*)strtok (NULL, " ");                
        }

        if (u4ArgCount == 0)
        {
            continue; 
        }
        
        pucCmdToken = (char*) pucArgValues[0]; 

        //parse the command 
        if ((strcmp((char *)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp(); 
        }
        else if ((strcmp((char *)pucCmdToken, "exit") == 0) || 
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            printf("Exit From CLI\n"); 
            g_bIsCLITest = MFALSE; 
            break; 
        }
        else
        {
            MBOOL bIsFoundCmd = MFALSE;
            for (MUINT32 u4CmdIndex = 0; ; u4CmdIndex++)
            {
                if(NULL == g_pSImager_Cmds[u4CmdIndex].pucCmdStr)
                {
                    break; 
                }  
                if (strcmp((char *)pucCmdToken, g_pSImager_Cmds[u4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE; 
                    g_pSImager_Cmds[u4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);                     
                    break;
                }                
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n"); 
            }            
        }        
            
    }

    return 0; 
}


/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    MY_LOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            MY_LOGE("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        readCnt += nr;
        cnt++;
    }
    MY_LOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}

/*******************************************************************************
*  Test simager 
********************************************************************************/
int test_simger(ImgBufInfo & rSrcImgBufInfo, ImgBufInfo & rTargetImgBufInfo, MUINT32 u4Rotation, MUINT32 u4Flip) 
{
    MY_LOGD("+"); 

    return 0; 
}

/*******************************************************************************
*  Main Function 
********************************************************************************/
int main_simager(int argc, char** argv)
{
    printf("SImager Test \n");     
    char *filename; 
    int srcFmt = 0; 
    //
    if (argc != 5) 
    {
        printf("Usage: camshottest <filename> <width> <height> <format> \n"); 
        return 0; 
    }
    filename = argv[1]; 
    g_u4SrcWidth = g_u4TargetWidth = atoi(argv[2]); 
    g_u4SrcHeight = g_u4TargetHeight = atoi(argv[3]); 
    srcFmt = atoi(argv[4]); 
    if (srcFmt > 4) 
    {
        srcFmt = 0; 
    }
    g_pIMemDrv =  IMemDrv::createInstance(); 
    g_pIMemDrv->init(); 
    if (NULL == g_pIMemDrv)
    {
        MY_LOGE("g_pIMemDrv is NULL"); 
        return 0; 
    }
    if (g_u4SrcWidth > 5000 || g_u4SrcWidth < 0)
    {
        g_u4SrcWidth = 0; 
    }
    if (g_u4SrcHeight > 5000 || g_u4SrcHeight < 0) 
    {
        g_u4SrcHeight = 0; 
    }
    // 
    IMEM_BUF_INFO rInMem; 
    rInMem.size = (g_u4SrcWidth * g_u4SrcHeight * 2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);     
    allocMem(rInMem); 
    loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), g_u4SrcWidth * g_u4SrcHeight * 2); 
    printf("load image:%s\n", filename); 

    //    
    g_rOutMem.size = (g_u4SrcWidth * g_u4SrcHeight * 2 + L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1); 
    allocMem(g_rOutMem); 
    //
    MUINT32 u4InStride[3] = {0, 0, 0}; 
    if (g_eImgFmt[srcFmt] == eImgFmt_YUY2)
    {
        u4InStride[0] = g_u4SrcWidth * 2; 
        u4InStride[1] = u4InStride[2] = 0; 
    } 
    else if (eImgFmt_NV21 == g_eImgFmt[srcFmt])
    {
        u4InStride[0] = g_u4SrcWidth; 
        u4InStride[1] = g_u4SrcWidth; 
        u4InStride[2] = 0; 
    }
    else if ((eImgFmt_I420 == g_eImgFmt[srcFmt]) || (eImgFmt_YV16 == g_eImgFmt[srcFmt]))
    {
        u4InStride[0] = g_u4SrcWidth; 
        u4InStride[1] = g_u4SrcWidth/2; 
        u4InStride[2] = g_u4SrcWidth/2; 
    }
    else 
    {
        u4InStride[0] = g_u4SrcWidth; 
        u4InStride[1] = g_u4SrcWidth; 
        u4InStride[2] = g_u4SrcWidth; 
    }

    ImgBufInfo rSrcImgInfo(ImgInfo(g_eImgFmt[srcFmt], g_u4SrcWidth, g_u4SrcHeight), 
                           BufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID), u4InStride); 

    // (1). Create Instance 
    g_pISImager = ISImager::createInstance(rSrcImgInfo); 
    if (g_pISImager == NULL) 
    {
        MY_LOGE("Null ISImager Obj \n"); 
        return 0; 
    }

    // init setting     
    BufInfo rBufInfo(g_rOutMem.size, g_rOutMem.virtAddr, g_rOutMem.phyAddr, g_rOutMem.memID); 
    //
    g_pISImager->setTargetBufInfo(rBufInfo); 
    //
    g_pISImager->setFormat(g_eImgFmt[srcFmt]); 
    //
    g_pISImager->setRotation(0); 
    //
    g_pISImager->setFlip(0); 
    // 
    g_pISImager->setResize(g_u4SrcWidth, g_u4SrcHeight); 
    //
    g_pISImager->setEncodeParam(1, 90); 
    //
    g_pISImager->setROI(Rect(0, 0, g_u4SrcWidth, g_u4SrcHeight)); 
    //

    vHelp(); 
    
    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL); 

    //!***************************************************
    //! Main thread wait for exit 
    //!***************************************************    
    while (g_bIsCLITest== MTRUE)
    {
        usleep(100000); 
    }

    //
    deallocMem(rInMem); 
    deallocMem(g_rOutMem); 
    //
    g_pIMemDrv->uninit(); 
    g_pIMemDrv->destroyInstance(); 
    g_pISImager->destroyInstance(); 
    return 0; 
}
