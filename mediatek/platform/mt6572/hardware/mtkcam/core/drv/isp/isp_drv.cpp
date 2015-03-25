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
#define LOG_TAG "IspDrv"
//
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/Errors.h>
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/threads.h>                  // For android::Mutex.
#else
#include <utils/Mutex.h>                    // For android::Mutex.
#endif
#include <cutils/atomic.h>
#include <cutils/properties.h>              // For property_get().
//#include <asm/arch/mt6589_sync_write.h>     // For dsb() in isp_drv.h.
#include <cutils/log.h>
//
#include "camera_isp.h"
#include "isp_drv.h"

//-----------------------------------------------------------------------------
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG    LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv);
EXTERN_DBG_LOG_VARIABLE(isp_drv);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
//-----------------------------------------------------------------------------

pthread_mutex_t IspRegMutex = PTHREAD_MUTEX_INITIALIZER;    // used in isp_drv.h

//-----------------------------------------------------------------------------
MUINT32 CEILING(MUINT32 A, MUINT32 B)
{
    if(B == 0)
        return A;
    if(A%B == 0)
        return A/B;
    else
        return A/B + 1;
}
MUINT32 ROUND(MUINT32 A, MUINT32 B)
{
    if(B == 0)
        return A;
    else
        return (2*A+B)/(2*B);
}
//----------------------------------------------------------------------------

// IspDrv
//----------------------------------------------------------------------------
IspDrv::IspDrv()
{
    //LOG_DBG("");
    mUsers = 0;
    mFd = -1;
    
    mpIspDrvRegMap = NULL;    
    mpIspHwRegAddr = NULL;

    mpRTBufTbl = NULL;

    //mHwBufList.bufList.clear();
    mSwBufList.bufList.clear();
}
//----------------------------------------------------------------------------
IspDrv::~IspDrv()
{
    //LOG_DBG("");
}

// static function
//----------------------------------------------------------------------------
IspDrv* IspDrv::createInstance(void)
{
    DBG_LOG_CONFIG(drv, isp_drv);
    return IspDrv::getInstance();
}
//----------------------------------------------------------------------------
IspDrv* IspDrv::getInstance(void)
{
    static IspDrv singleton;
    return &singleton;
}

// vitrual functions start
//----------------------------------------------------------------------------
void IspDrv::destroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MUINT32* mpSeninfHwRegAddr = NULL;
MBOOL IspDrv::init(void)
{
    MBOOL Result = MTRUE;
    isp_reg_t *pisp;
                   
    //
    Mutex::Autolock lock(mLock);        

    LOG_DBG(" +");

    //     
    LOG_DBG("mUsers(%d)", mUsers);
    if(mUsers > 0)    
    {        
        LOG_DBG("has inited");        
        android_atomic_inc(&mUsers);        
        LOG_DBG(" -");
        return Result;    
    }
    
    // isp reg map
    mpIspDrvRegMap = (MUINT32*)malloc(sizeof(isp_reg_t));
     
    // Open isp driver            
    mFd = open(ISP_DRV_DEV_NAME, O_RDWR);
    if(mFd < 0)
    {
        LOG_ERR("error open kernel driver, errno(%d):%s", errno, strerror(errno));
        Result = MFALSE;
        goto EXIT;
    }           
    
    // reset isp
#if !defined(_use_kernel_ref_cnt_)
    LOG_DBG("not use kernel ref cnt; DO ISP HW RESET");

    reset(); // Do IMGSYS SW RST, which will also enable CAM/SEN/JPGENC/JPGDEC clock.
#else   // _use_kernel_ref_cnt_
    LOG_DBG("use kernel ref cnt, mFd(%d)", mFd);

    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;
    ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC;
    ref_cnt.data_ptr = (MUINT32)&count;    
    ref_cnt.ctrl = ISP_REF_CNT_GET;    
    if ( MTRUE == kRefCntCtrl(&ref_cnt) ) 
    {
        if (0==count) 
        {
            LOG_DBG("first user at kernel space, do isp rest & clear kernel ring buff");
            reset(); // Do IMGSYS SW RST, which will also enable CAM/SEN/JPGENC/JPGDEC clock.
            freeAllPhyBuf();
        }
        ref_cnt.ctrl = ISP_REF_CNT_INC;
        if ( MFALSE == kRefCntCtrl(&ref_cnt) ) 
        {
            LOG_ERR("ISP_REF_CNT_INC fail, errno(%d):%s.", errno, strerror(errno));
        }
    }
    else 
    {
        LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
    }    
#endif

    // mmap seninf reg for debug
    mpSeninfHwRegAddr = (MUINT32 *) mmap(0, 0x1000, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mFd, 0x14014000);

    // mmap isp reg
    mpIspHwRegAddr = (MUINT32 *) mmap(0, ISP_BASE_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, 
                                    mFd, ISP_BASE_HW);
    if(mpIspHwRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap fail, errno(%d):%s", errno, strerror(errno));
        Result = MFALSE;
        goto EXIT;
    } 
        
    // Init setting         
    pisp = (isp_reg_t *) mpIspHwRegAddr;      
    ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL, 0x01010011);
    ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, 0x00000000);    
    // Jason TODO Reg   
    #if 0
    ISP_REG(pisp, CAM_PHSCNT) = 0x01010011;
    ISP_REG(pisp, CAM_TG2_PH_CNT) = 0x00000000;
    ISP_REG(pisp, CAM_PATH) = 0x30;
    ISP_BITS(pisp, CAM_INTEN, VSYNC_INT_EN) = 1; //always enable VD for bridge 3D application.    
    // Disable Camera PLL     
    (*mpIPllCon0RegAddr) |= 0x01; //Power Down
    (*mpIPllCon1RegAddr) &= (~0x30);
    #endif

    // Open mipi power driver
    #if ISP_DRV_CAM_MIPI_API
    mFd1 = open(ISP_DRV_DEV_NAME_MIPI, O_RDWR);
    if(mFd1 < 0)
    {
        LOG_ERR("error open kernel driver, errno(%d):%s", errno, strerror(errno));
        Result = MFALSE;
        goto EXIT;
    }
    #endif

    //pass1 buffer control shared mem.
    mRTBufTblSize = ISP_RT_BUF_TBL_NPAGES * getpagesize();
    mpRTBufTbl = (MUINT32 *)mmap(0, mRTBufTblSize, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_SHARED| MAP_LOCKED, 
                                mFd, mRTBufTblSize);
    LOG_DBG("mRTBufTblSize(0x%x),mpRTBufTbl(0x%x)",mRTBufTblSize,(MUINT32)mpRTBufTbl);
    if (mpRTBufTbl == MAP_FAILED)
    {
        LOG_ERR("mpRTBufTbl mmap FAIL");
        Result = MFALSE;
        goto EXIT;
    }    

    //
    //mHwBufList.bufList.clear();
    mSwBufList.bufList.clear(); 
    
    //
    mpCallback = NULL;
    
    // 
    android_atomic_inc(&mUsers);

EXIT:
    if(!Result)
    {
        if(mFd >= 0)
        {
            //munmap all items before close mfd
            if(NULL != mpRTBufTbl)
            {
               munmap(mpRTBufTbl, mRTBufTblSize);
               mpRTBufTbl = NULL; 
            }
            //
            if(NULL != mpIspHwRegAddr)                
            {
               munmap(mpIspHwRegAddr, ISP_BASE_RANGE);
               mpIspHwRegAddr = NULL;
            }                            
            //
            if(NULL != mpSeninfHwRegAddr)
            {
               munmap(mpSeninfHwRegAddr, ISP_BASE_RANGE);
               mpSeninfHwRegAddr = NULL;                
            }            
            close(mFd);
            mFd = -1;
        }
    }

    LOG_DBG(" -");    
    return Result;
}
//----------------------------------------------------------------------------
MBOOL IspDrv::uninit(void)
{
    MBOOL Result = MTRUE;    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    
    //
    Mutex::Autolock lock(mLock);

    LOG_DBG(" +");    

    //
    LOG_DBG("mUsers(%d)", mUsers);    
    if(mUsers <= 0)
    {   
        LOG_DBG("no more user");
        LOG_DBG(" -"); 
        return MTRUE;
    }
    
    // More than one user
    android_atomic_dec(&mUsers);    
    if(mUsers == 0)
    {   // Last user     
        // Jason TODO Reg                
        #if 0   
        ISP_WRITE_REG(pisp, CAM_CTL_INT_EN, 0);
        setCSI2(0, 0, 0, 0, 0, 0, 0, 0); // disable CSI2 
        setTgPhaseCounter(0, 0, 0, 0, 0, 0, 0);
        #endif
                        
        //
        if(mFd >= 0)
        {           
            #if defined(_use_kernel_ref_cnt_)
            ISP_REF_CNT_CTRL_STRUCT ref_cnt;
            MINT32 count;
            ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC;
            ref_cnt.data_ptr = (MUINT32)&count;
            ref_cnt.ctrl = ISP_REF_CNT_DEC;
            if ( MTRUE == kRefCntCtrl(&ref_cnt) ) 
            {
                if (0==count) 
                {
                    LOG_DBG("last user at kernel space, do clear kernel ring buff");
                    freeAllPhyBuf();
                }
            }
            else
            {
                LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
            }
        #endif
          
        //munmap all items before close mfd
        if(NULL != mpRTBufTbl)
        {
            munmap(mpRTBufTbl, mRTBufTblSize);
            mpRTBufTbl = NULL; 
        }
        //
        if(NULL != mpIspHwRegAddr)                
        {
            munmap(mpIspHwRegAddr, ISP_BASE_RANGE);
            mpIspHwRegAddr = NULL;
        }                
        //
            if(NULL != mpSeninfHwRegAddr)
            {
               munmap(mpSeninfHwRegAddr, ISP_BASE_RANGE);
               mpSeninfHwRegAddr = NULL;                
            }             
            close(mFd);            
            mFd = -1;                       
        }

        //
        if(NULL != mpIspDrvRegMap)
        {
            free((MUINT32*)mpIspDrvRegMap);
            mpIspDrvRegMap = NULL;
        }        
        
        //
        #if ISP_DRV_CAM_MIPI_API
        if(mFd1 > 0) 
        {
            close(mFd1);
            mFd1 = -1;
        }
        #endif        
    }
    else
    {
        LOG_DBG("still users");
    }

    LOG_DBG(" -");        
    return Result;
}
//----------------------------------------------------------------------------
MBOOL IspDrv::waitIrq(ISP_DRV_WAIT_IRQ_STRUCT waitIrq)
{
    MINT32 Ret;

    LOG_DBG(" +"); 
    
    //
    LOG_DBG("Clear(%d), Status(0x%08x), Timeout(%d).", waitIrq.clear, waitIrq.status, waitIrq.timeout);
    //
    Ret = ioctl(mFd,ISP_IOC_WAIT_IRQ,&waitIrq);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_WAIT_IRQ fail(%d)", Ret);

        *(mpSeninfHwRegAddr + 0x014/4) &= 0x7FFFFFFF;
        Ret = *(mpSeninfHwRegAddr + 0x018/4);
        usleep(1*1000);
        Ret = *(mpSeninfHwRegAddr + 0x018/4);        
        LOG_ERR("SENINF reg 0x14014018=0x%08X", Ret);

        *(mpSeninfHwRegAddr + 0x614/4) &= 0x7FFFFFFF;
        Ret = *(mpSeninfHwRegAddr + 0x618/4);
        usleep(1*1000);
        Ret = *(mpSeninfHwRegAddr + 0x618/4);        
        LOG_ERR("SENINF reg 0x14014618=0x%08X", Ret);

        Ret = *(mpSeninfHwRegAddr + 0x108/4);        
        LOG_ERR("SENINF reg 0x14014108=0x%08X", Ret);
        
        LOG_DBG(" -");
        return MFALSE;
    }

    LOG_DBG(" -");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::readIrq(MUINT32* pReadIrqStatus)
{
    MINT32 Ret;
    
    LOG_DBG(" +");
    
    //
    LOG_DBG("Status(%d)", *pReadIrqStatus);
    //
    *pReadIrqStatus = 0;
    //
    Ret = ioctl(mFd,ISP_IOC_READ_IRQ, pReadIrqStatus);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_READ_IRQ fail(%d)", Ret);
        LOG_DBG(" -");
        return MFALSE;
    }
    
    LOG_DBG(" -");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::checkIrq(MUINT32 checkIrqStatus)
{
    MINT32 Ret;
    MUINT32 readIrqStatus;
    
    LOG_DBG(" +");

    //
    LOG_DBG("Status(%d)", checkIrqStatus);
    //
    checkIrqStatus = 0;
    if(!readIrq(&readIrqStatus))
    {
        LOG_DBG(" -");
        return MFALSE;
    }
    //
    if((checkIrqStatus & readIrqStatus) != checkIrqStatus)
    {
        LOG_ERR("Status:Check(0x%08X), Read(0x%08X)", checkIrqStatus, readIrqStatus);
        LOG_DBG(" -");
        return MFALSE;
    }

    LOG_DBG(" -");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::clearIrq(MUINT32 clearIrqStatus)
{
    MINT32 Ret;

    LOG_DBG(" +");
    
    //
    LOG_DBG("Status(%d)", clearIrqStatus);
    //
    Ret = ioctl(mFd,ISP_IOC_CLEAR_IRQ, clearIrqStatus);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_CLEAR_IRQ fail(%d)", Ret);
        LOG_DBG(" -");
        return MFALSE;
    }

    LOG_DBG(" -");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::reset(void)
{
    MINT32 Ret;

    LOG_DBG(" +");
    
    //
    LOG_DBG("ISP HW RESET, mfd(0x%08x)", mFd);
    //
    Ret = ioctl(mFd,ISP_IOC_RESET,NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_RESET fail(%d)", Ret);
        LOG_DBG(" -");
        return MFALSE;
    }

    LOG_DBG(" -");
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::resetBuf(void)
{
    MINT32 Ret;

    LOG_DBG(" +");
    
    //
    Ret = ioctl(mFd,ISP_IOC_RESET_BUF,NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_RESET_BUF fail(%d)", Ret);
        LOG_DBG(" -");
        return MFALSE;
    }
    
    LOG_DBG(" -");    
    return MTRUE;
}
//
//--------------------------------------------------------------------------
MUINT32 IspDrv::getRegAddr(void)
{
    LOG_DBG(" +");
    
    LOG_DBG("mpIspHwRegAddr(0x%08X)", (MUINT32)mpIspHwRegAddr);

    LOG_DBG(" -");
    return (MUINT32)mpIspHwRegAddr;
}
//-----------------------------------------------------------------------------
isp_reg_t* IspDrv::getRegAddrMap(void)
{
    LOG_DBG(" +");
    
    LOG_DBG("mpIspDrvRegMap(0x%08X)", (MUINT32)mpIspDrvRegMap);

    LOG_DBG(" -");
    return (isp_reg_t*)mpIspDrvRegMap;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::readRegs(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 count)
{
    MINT32 Ret;
    ISP_REG_IO_STRUCT ispRegIo;
    //
    //LOG_DBG("count(%d)",count);
    //
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    //
    ispRegIo.Data = (MINT32)pRegIo;
    ispRegIo.Count = count;
    //
    Ret = ioctl(mFd, ISP_IOC_READ_REG, &ispRegIo);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_READ_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MUINT32 IspDrv::readReg(MUINT32 addr)
{
    ISP_DRV_REG_IO_STRUCT regIo;
    //
    //LOG_DBG("addr(0x%08X)",addr);
    //
    regIo.addr = addr;
    //
    if(!readRegs(&regIo, 1))
    {
        return 0;
    }
    return (regIo.data);
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::writeRegs(
    ISP_DRV_REG_IO_STRUCT*  pRegIo,
    MUINT32                 count)
{
    MINT32 Ret;
    ISP_REG_IO_STRUCT ispRegIo;
    //
    //LOG_DBG("count(%d)\n",count);
    //
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    //
    ispRegIo.Data = (MINT32)pRegIo;
    ispRegIo.Count = count;
    //
    Ret = ioctl(mFd, ISP_IOC_WRITE_REG, &ispRegIo);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_WRITE_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::writeReg(
    MUINT32     addr,
    MUINT32     data)
{
    ISP_DRV_REG_IO_STRUCT regIo;
    //
    //LOG_DBG("addr(0x%08X),data(0x%08X)",addr,data);
    //
    regIo.addr = addr;
    regIo.data = data;
    //
    if(!writeRegs(&regIo, 1))
    {
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::holdReg(MBOOL en)
{
    MINT32 Ret;
    //
    //LOG_DBG("en(%d)",en);
    //
    Ret = ioctl(mFd, ISP_IOC_HOLD_REG, &en);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_HOLD_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDrv::dumpReg(void)
{
    MINT32 Ret;
    //
    //LOG_DBG("");
    //
    Ret = ioctl(mFd, ISP_IOC_DUMP_REG, NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_DUMP_REG fail(%d)",Ret);
        return MFALSE;
    }
    return MTRUE;
}
//
//----------------------------------------------------------------------------
MINT32 IspDrv::sendCommand(
    MINT32      cmd,
    MINT32      arg1,
    MINT32      arg2,
    MINT32      arg3)
{
    MINT32 ret = 0;

    LOG_DBG(" +");
    
    #if 0
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    switch(cmd)
    {
        case ISP_DRV_CMD_RESET_BUF:
        {
            LOG_DBG("ISP_DRV_CMD_RESET_BUF");
            ret = ioctl(mFd, ISP_IOC_RESET_BUF);
            if(ret < 0)
            {
                LOG_ERR("ISP_DRV_CMD_RESET_BUF err(%d)", ret);
            }
            break;
        }
        case ISP_DRV_CMD_CLEAR_INT_STATUS : // cotta-- added for clean all interrupt
        {
            MUINT32 cleanISPVDSignal;
            cleanISPVDSignal = ISP_REG(pisp, CAM_INTSTA);           
            break;  
        }
        case ISP_DRV_CMD_RESET_VD_COUNT:
        {
            ret = ioctl(mFd, MT_ISP_IOC_T_RESET_VD_COUNT, NULL);
            if(ret < 0)
            {
                LOG_ERR("MT_ISP_IOC_T_RESET_VD_COUNT err");
            }          
            break;
        }
        //
        case ISP_DRV_CMD_SET_PIXEL_TYPE:
        {
            LOG_DBG("ISP_DRV_CMD_SET_PIXEL_TYPE(%d)", arg1);
            ISP_BITS(pisp, CAM_CTRL1, GPID) = arg1 & 0x01;
            ISP_BITS(pisp, CAM_CTRL1, GLID) = (arg1 >> 1) & 0x01;
            LOG_DBG("CAM_CTRL1(0x%08x)", ISP_REG(pisp, CAM_CTRL1));
            break;
        }
        case ISP_DRV_CMD_SET_KDBG_FLAG:
        {
            LOG_DBG("ISP_DRV_CMD_SET_KDBG_FLAG(%d)", arg1);
            ret = ioctl(mFd, ISP_IOC_DEBUG_FLAG, &arg1);
            if(ret < 0)
            {
                LOG_ERR("ISP_DRV_CMD_SET_KDBG_FLAG err");
            }
            break;
        }
        case ISP_DRV_CMD_SET_CAPTURE_DELAY:
        {
            ISP_BITS(pisp, CAM_VFCON, SP_DELAY) = arg1;
            break;
        }
        case ISP_DRV_CMD_SET_VD_PROC : //cotta-- added for force turn on VD signal
        {
            ioctl(mFd, MT_ISP_IOC_T_ENABLE_VD_PROC, (MBOOL*)arg1);          
            break;
        }
        //
        case ISP_DRV_CMD_GET_ADDR:
        {
            //LOG_DBG("ISP_DRV_CMD_GET_ADDR(0x%x)", (MINT32) mpIspHwRegAddr);
            *(MINT32 *) arg1 = (MINT32) mpIspHwRegAddr;
            break;
        }
        case ISP_DRV_CMD_GET_LINE_ID:
        {
            MUINT32 const u4LineID = ISP_BITS(pisp, CAM_CTRL1, GLID);
            *reinterpret_cast<MUINT32*>(arg1) = u4LineID;
            LOG_DBG("<ISP_DRV_CMD_GET_LINE_ID> u4LineID(%d)", u4LineID);
            break;
        }
        case ISP_DRV_CMD_GET_CAPTURE_DELAY : //cotta-- added for get ISP register capture delay value
        {
            *(MINT32 *) arg1 = (MINT32) ISP_BITS(pisp, CAM_VFCON, SP_DELAY); // get ISP capture delay value            
            break;
        }
        case ISP_DRV_CMD_GET_VD_COUNT:
        {
            ret = ioctl(mFd, MT_ISP_IOC_G_GET_VD_COUNT, (MUINT32*)arg1);
            if(ret < 0)
            {
                LOG_ERR("MT_ISP_IOC_G_GET_VD_COUNT err");
                *(MUINT32*)arg1 = 0;
            }        
            break;
        }        
        default:
        {
            LOG_DBG("Unknow cmd(0x%X)",cmd);
            ret = -1;
            break;
        }
    }        
    #endif
    
    LOG_DBG("Unknow cmd(0x%X)",cmd);

    LOG_DBG(" -");
    return ret;
}

//----------------------------------------------------------------------------
void IspDrv::regCallback(pIspDrvCallback pFunc)
{
    if(pFunc == NULL)
    {
        LOG_ERR("pFunc is NULL");
        return;
    }
    //
    mpCallback = pFunc;
}

MBOOL IspDrv::setDevID(MINT32 devid)
{
    MINT32 Ret;

    LOG_DBG(" +"); 
    
    //
    LOG_DBG("ISP_IOC_SET_DEVICE_ID ID(%d)", devid);
    Ret = ioctl(mFd,ISP_IOC_SET_DEVICE_ID,&devid);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_SET_DEVICE_ID fail(%d)", Ret);
        LOG_DBG(" -");
        return MFALSE;
    }

    LOG_DBG(" -");
    return MTRUE;

}

//
//----------------------------------------------------------------------------
MINT32 IspDrv::setCamModule(
    MBOOL       isEn,
    MUINT32     imgDataFormat,
    MUINT32     swapY,
    MUINT32     swapCbCr)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_DBG(" +");

    LOG_DBG("isEn(%1d), imgDataFormat(%ld), swapY(%ld), swapCbCr(%ld)",
                isEn, imgDataFormat, swapY, swapCbCr);

    // start set register
    ISP_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK) = 1;
    //
    if(MTRUE == isEn)
    {
        //
        //ISP_WRITE_REG(pisp, CAM_CTL_MODULE_EN, 0x00000009);
        ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_EN, 1);
        ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, TG1_EN, 1);        
        ISP_WRITE_REG(pisp, CAM_CTL_CLK_EN, 0x00008005);
        //
        ISP_WRITE_REG(pisp, CAM_CTL_INT_EN, 0);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, VS1_INT_EN, 1);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, TG1_SOF_INT_EN, 1);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, PASS1_DON_INT_EN, 1);
        //ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, IMGO_DROP_INT_EN, 1);  
        //
        ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, IMGO_BUS_SIZE, 0x1);    
        switch(imgDataFormat)
        {
            case ISP_DRV_IMG_FORMAT_YUV422:
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, IMGO_FORMAT, 0x1);
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, TG1_FMT, 0x3);            
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, TG1_SW, (swapCbCr<<1)+swapY);
                break;
            case ISP_DRV_IMG_FORMAT_JPEG:
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, IMGO_FORMAT, 0x2);
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, TG1_FMT, 0x7);
                // disable cdrz for jpeg case
                ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, CDRZ_EN, 0);
                break;
            default:
                LOG_ERR("invalid img data format(%d)", imgDataFormat);
                break;
        }                  
    }
    else
    {
        //
        ISP_WRITE_REG(pisp, CAM_CTL_MODULE_EN, 0);
        ISP_WRITE_REG(pisp, CAM_CTL_CLK_EN, 0);
        //
        ISP_WRITE_REG(pisp, CAM_CTL_INT_EN, 0);
    }   
    //
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 0);
    // end set register

    LOG_DBG(" -");
    return ret;   
}
//----------------------------------------------------------------------------
MINT32 IspDrv::setCdrzCtrl(
    MBOOL       isEn,
    MUINT32     inputPixel, 
    MUINT32     inputLine,     
    MUINT32     outputPixel, 
    MUINT32     outputLine)
{
    MINT32 ret = 0;  
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    
    MINT32 coefVertical, coefHorizontal; 

    LOG_DBG(" +");

    LOG_DBG("isEn(%d), inputPixel(%ld), inputLine(%ld), outputPixel(%ld), outputLine(%ld)"
                , isEn, inputPixel, inputLine, outputPixel, outputLine);

    // start set register
    ISP_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK) = 1;
    //
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, CDRZ_EN, isEn);    
    ISP_WRITE_BITS(pisp, CAM_CDRZ_CONTROL, CDRZ_HORIZONTAL_EN, isEn);   
    ISP_WRITE_BITS(pisp, CAM_CDRZ_CONTROL, CDRZ_Vertical_EN, isEn);
    if(isEn)
    {
        ISP_WRITE_REG(pisp, CAM_CDRZ_INPUT_IMAGE, (inputLine<<16)+inputPixel);
        ISP_WRITE_REG(pisp, CAM_CDRZ_OUTPUT_IMAGE, (outputLine<<16)+outputPixel);
        //
        ISP_WRITE_BITS(pisp, CAM_CDRZ_CONTROL, CDRZ_Vertical_First, 0);   
        //
        ISP_WRITE_BITS(pisp, CAM_CDRZ_CONTROL, CDRZ_Vertical_Algorithm, 0); 
        coefVertical = ROUND( (inputLine-1)*32768, (outputLine-1) ); // ROUND(a,b) => ROUND(a/b)    
        ISP_WRITE_REG(pisp, CAM_CDRZ_VERTICAL_COEFF_STEP, coefVertical);      
        //
        if( outputPixel >= (inputPixel>>1) )
        {   // 1X ~ 1/2X            
            ISP_WRITE_BITS(pisp, CAM_CDRZ_CONTROL, CDRZ_Horizontal_Algorithm, 0);            
            coefHorizontal = CEILING( (inputPixel-1)*32768, (outputPixel-1) ); // CEILING(a,b) => CEILING(a/b)            
        }   
        else
        { 
            ISP_WRITE_BITS(pisp, CAM_CDRZ_CONTROL, CDRZ_Horizontal_Algorithm, 2);
            coefHorizontal = CEILING( (outputPixel-1)*1048576, (inputPixel-1) ); // CEILING(a,b) => CEILING(a/b)
        }
        ISP_WRITE_REG(pisp, CAM_CDRZ_HORIZONTAL_COEFF_STEP, coefHorizontal);           
        
        LOG_DBG("coefHorizontal(%x), coefVertical(%x)", coefHorizontal, coefVertical);
    }
    //
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 0);
    // end set register

    LOG_DBG(" -");
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrv::setImgoAddr(    
    MUINT32     imgoAddr)
{
    MINT32 ret = 0;   
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_DBG(" +");

    LOG_DBG("imgoAddr(0x%08lX)", imgoAddr);

    // start set register
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 1);
    //
    ISP_WRITE_REG(pisp, CAM_IMGO_BASE_ADDR, imgoAddr);    
    //
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 0);
    // end set register

    LOG_DBG(" -");
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrv::setImgoSize(
    MUINT32     imgoPixel,
    MUINT32     imgoLine)
{
    MINT32 ret = 0;       
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_DBG(" +");

    LOG_DBG("imgoPixel(%ld), imgoLine(%ld)", imgoPixel, imgoLine);

    // start set register
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 1);

    // set CAM_WDMA ultra for drop frame issue
    LOG_DBG("before set: CAM_IMGO_CON=0x%08x", ISP_READ_REG(pisp, CAM_IMGO_CON));
    ISP_WRITE_REG(pisp, CAM_IMGO_CON, 0x08100850);
    LOG_DBG("after set: CAM_IMGO_CON=0x%08x", ISP_READ_REG(pisp, CAM_IMGO_CON));

    ISP_WRITE_REG(pisp, CAM_IMGO_XSIZE, (imgoPixel<<1)-1);    
    ISP_WRITE_REG(pisp, CAM_IMGO_YSIZE, imgoLine-1);    
    ISP_WRITE_REG(pisp, CAM_IMGO_STRIDE, (imgoPixel<<1));        
    //
    ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 0);
    // end set register

    LOG_DBG(" -");
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrv::control(MUINT32 isEn)
{
    MINT32 ret = 0;    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_DBG(" +");

    LOG_DBG("isEn(%ld)", isEn);

    //
    if(mpCallback != NULL)
    {
        LOG_DBG("Callback(%d)", isEn);
        if(isEn)
        {
            (*mpCallback)(true);
        }
        else
        {
            (*mpCallback)(false);
            mpCallback = NULL;
        }
        LOG_DBG("Callback End");
    }

    // start set register
    //ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 1);
    //    

    ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_DONE = {ISP_DRV_IRQ_CLEAR_WAIT,                                         
                                                ISP_DRV_INT_VS1_ST,
                                                CAM_INT_WAIT_TIMEOUT_MS};
    if(isEn)
    {
        // enable irq
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, VS1_INT_EN, isEn);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, TG1_SOF_INT_EN, isEn);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, PASS1_DON_INT_EN, isEn);
        //ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, IMGO_DROP_INT_EN, isEn); 
        // Jason TODO Reg
        //ioctl(mFd, MT_ISP_IOC_T_ENABLE_VD_PROC, (MBOOL*)&isEn);

        ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, IMGO_EN, isEn);    
        ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, VFDATA_EN, isEn);    

        // wait 1 vsync for VF active
        LOG_DBG("wait for vsync");
        ret = this->waitIrq( irq_TG1_DONE );
        if( MFALSE == ret )
        {
            LOG_ERR("waitIrq( irq_TG1_DONE ) fail");
            LOG_DBG(" -");
            return ret;
        }      
    }
    else
    {
        ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, VFDATA_EN, isEn);  
        
        // wait 1 vsync for VF active
        LOG_DBG("wait for vsync");        
        ret = this->waitIrq( irq_TG1_DONE );
        if( MFALSE == ret )
        {
            LOG_ERR("waitIrq( irq_TG1_DONE ) fail");
            LOG_DBG(" -");
            //return ret;   still doing close 
        }           
        
        if( 0x1 != ISP_READ_BITS(pisp, CAM_TG_INTER_ST, TG_CAM_CS) )
        {
            LOG_DBG("waitIrq OK but ISP not idle!!! Wait one more tg1");    
            ret = this->waitIrq( irq_TG1_DONE );
        }        
                                
        // stop DMA, clear IMGO addr and driver buff
        ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, IMGO_EN, isEn);        
        ISP_WRITE_REG(pisp, CAM_IMGO_BASE_ADDR, 0x00);
        mSwBufList.bufList.clear(); 
        //mHwBufList.bufList.clear();
        freeAllPhyBuf();        // for continuous shot, to clear kernel bufList

        // disable irq
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, VS1_INT_EN, isEn);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, TG1_SOF_INT_EN, isEn);
        ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, PASS1_DON_INT_EN, isEn);
        //ISP_WRITE_BITS(pisp, CAM_CTL_INT_EN, IMGO_DROP_INT_EN, isEn);  
        // Jason TODO Reg
        //ioctl(mFd, MT_ISP_IOC_T_ENABLE_VD_PROC, (MBOOL*)&isEn);        
    }                    
    
    //
    //ISP_WRITE_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK, 0);
    // end set register
    
    //if( ! isEn )
    //    usleep(200*1000);

    LOG_DBG(" -");
    return ret;  
}
#if 0
//----------------------------------------------------------------------------
MINT32 IspDrv::setImgoFBC(
    MBOOL       isEn,
    MUINT32     buffNum)
{
    MINT32 ret = 0;       
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_DBG("isEn(%ld), buffNum(%ld)", isEn, buffNum);

    // start set register
    ISP_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK) = 1;
    //
    ISP_BITS(pisp, CAM_CTL_IMGO_FBC, FBC_EN) = isEn;
    ISP_BITS(pisp, CAM_CTL_IMGO_FBC, FB_NUM) = buffNum;
    //
    ISP_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK) = 0;
    // end set register

    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrv::setImgoBuffRead(void)
{
    MINT32 ret = 0;       
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_DBG("imgo buff read");

    // start set register
    ISP_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK) = 1;
    //
    ISP_BITS(pisp, CAM_CTL_IMGO_FBC, RCNT_INC) = 1;
    //
    ISP_BITS(pisp, CAM_CTL_MODULE_EN, DB_LOCK) = 0;
    // end set register

    return ret;
}
#endif
//----------------------------------------------------------------------------
MBOOL IspDrv::waitBufReady(void)        
{   // wait pass1 done irq for buff ready
    MBOOL ret = MTRUE;
    
    ISP_RT_BUF_CTRL_STRUCT rt_buf_ctrl;
    MUINT32 bWaitBufRdy;

    LOG_DBG(" +");

    //check if there is already filled buffer
    rt_buf_ctrl.ctrl = ISP_RT_BUF_CTRL_IS_RDY;
    rt_buf_ctrl.buf_id = ISP_RT_BUF_IMGO;
    rt_buf_ctrl.data_ptr = (MUINT32)&bWaitBufRdy;
    LOG_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)", rt_buf_ctrl.ctrl, rt_buf_ctrl.buf_id, rt_buf_ctrl.data_ptr);        
    ret = this->rtBufCtrl((void*)&rt_buf_ctrl);    
    if( MFALSE == ret )
    {
        LOG_ERR("rtBufCtrl fail: ISP_RT_BUF_CTRL_IS_RDY");
        LOG_DBG(" -");
        return ret;
    }

    //*(mpSeninfHwRegAddr + 0x014/4) &= 0x7FFFFFFF;       
    //LOG_ERR("Jason test 0x14014018=0x%08X", *(mpSeninfHwRegAddr + 0x018/4));

    if( bWaitBufRdy )
    {
        LOG_DBG("wait for pass1 done");
        ISP_DRV_WAIT_IRQ_STRUCT irq_PASS1_DONE = {ISP_DRV_IRQ_CLEAR_WAIT,                                         
                                                    ISP_DRV_INT_PASS1_TG1_DON_ST,
                                                    CAM_INT_WAIT_TIMEOUT_MS};
        ret = this->waitIrq( irq_PASS1_DONE );
        if( MFALSE == ret )
        {
            LOG_ERR("waitIrq( ISP_DRV_INT_PASS1_TG1_DON_ST ) fail");
            LOG_DBG(" -");
            return ret;
        }        
    }        
        
    LOG_DBG(" -");
    return ret;
}
//----------------------------------------------------------------------------
MBOOL IspDrv::enqueueHwBuf(ISP_DRV_BUF_STRUCT buff)            
{   // append new buffer to the end of hwBuf list
    MBOOL ret = MTRUE;

    ISP_RT_BUF_CTRL_STRUCT  rt_buf_ctrl;
    ISP_RT_BUF_STRUCT       rt_buf;
    ISP_RT_BUF_STRUCT       ex_rt_buf; 
    MUINT32 size;

    LOG_DBG(" +");

    // check rt_buf size
    rt_buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
    rt_buf_ctrl.buf_id = ISP_RT_BUF_IMGO;
    rt_buf_ctrl.data_ptr = (MUINT32)&size;
    LOG_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)", rt_buf_ctrl.ctrl, rt_buf_ctrl.buf_id, rt_buf_ctrl.data_ptr);        
    ret = this->rtBufCtrl((void*)&rt_buf_ctrl);    
    if( (MFALSE==ret) || (ISP_RT_BUF_SIZE==size) )
    {
        LOG_ERR("ERROR: get rt buf size");
        LOG_DBG(" -");
        return MFALSE;
    }

    // push to isp drv mHwBufList
    buff.status = ISP_DRV_BUF_EMPTY;
    //mHwBufList.bufList.push_back(buff);

    // set rt_buf & ex_rt_buf
    rt_buf.memID = buff.memID;    
    rt_buf.size = buff.size;
    rt_buf.base_vAddr = buff.base_vAddr;
    rt_buf.base_pAddr = buff.base_pAddr;          
    LOG_DBG("rt_buf.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",
                rt_buf.memID, rt_buf.size, rt_buf.base_vAddr, rt_buf.base_pAddr);    
    if ( NULL != buff.next ) 
    {
        ex_rt_buf.memID = buff.next->memID;
        ex_rt_buf.size = buff.next->size;
        ex_rt_buf.base_vAddr = buff.next->base_vAddr;
        ex_rt_buf.base_pAddr = buff.next->base_pAddr;
        LOG_DBG("exchange 1st buf. by 2nd buf. and enque it.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",
                    ex_rt_buf.memID, ex_rt_buf.size, ex_rt_buf.base_vAddr, ex_rt_buf.base_pAddr);
    }
    
    // enque to kernel ring buffer
    rt_buf_ctrl.ctrl = ISP_RT_BUF_CTRL_ENQUE;
    rt_buf_ctrl.buf_id = ISP_RT_BUF_IMGO;
    rt_buf_ctrl.data_ptr = (MUINT32)&rt_buf;
    rt_buf_ctrl.ex_data_ptr = (MUINT32)0;    
    if ( NULL != buff.next ) 
    {   //enque exchanged buffer
        rt_buf_ctrl.ctrl = ISP_RT_BUF_CTRL_EXCHANGE_ENQUE;
        rt_buf_ctrl.ex_data_ptr = (MUINT32)&ex_rt_buf;        
    }
    LOG_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)", rt_buf_ctrl.ctrl,rt_buf_ctrl.buf_id,rt_buf_ctrl.data_ptr);        
    ret = this->rtBufCtrl((void*)&rt_buf_ctrl);
    if( MFALSE == ret )
    {
        LOG_ERR("ERROR: enque buff");
        LOG_DBG(" -");
        return MFALSE;
    }

    LOG_DBG(" ]-");
    return MTRUE;
}
MBOOL IspDrv::dequeueHwBuf(void)
{   //move filled Hw buffer to sw the end of list
    MBOOL ret = MTRUE;
    MUINT32 count;

    LOG_DBG(" +");

    Mutex::Autolock lock(mQueHwLock);
    
    ISP_RT_BUF_CTRL_STRUCT      rt_buf_ctrl;
    ISP_RT_DEQUE_BUF_STRUCT     rt_deque_buf;
    ISP_DRV_BUF_STRUCT          buff;

    //deque filled buffer from kernel ring buffer
    rt_buf_ctrl.ctrl = ISP_RT_BUF_CTRL_DEQUE;
    rt_buf_ctrl.buf_id = ISP_RT_BUF_IMGO;
    rt_buf_ctrl.data_ptr = (MUINT32)&rt_deque_buf;
    LOG_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)", rt_buf_ctrl.ctrl, rt_buf_ctrl.buf_id, rt_buf_ctrl.data_ptr);        
    ret = this->rtBufCtrl((void*)&rt_buf_ctrl);
    if( MFALSE == ret )
    {
        LOG_ERR("ERROR: deque buff");
        LOG_DBG(" -");
        return MFALSE;
    }
    
    count = rt_deque_buf.count;
    LOG_DBG("deque_buf.count(%d)",count);
    if ( ISP_RT_BUF_SIZE < count ) 
    {
        LOG_ERR("ERROR: rt_deque_buf.count");
        LOG_DBG(" -");
        return MFALSE;
    }
    
    // push deque buffs to isp drv mSwBufList
    mSwBufList.bufList.clear();       
    for(MUINT32 i=0;i<count;i++) 
    {
        LOG_DBG("i(%d):ID(0x%x)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)/swbufL.size(%d)",
                i, rt_deque_buf.data[i].memID, rt_deque_buf.data[i].size, 
                rt_deque_buf.data[i].base_vAddr, rt_deque_buf.data[i].base_pAddr,
                mSwBufList.bufList.size());
        buff.status = ISP_DRV_BUF_FILLED;
        buff.memID = rt_deque_buf.data[i].memID;
        buff.size = rt_deque_buf.data[i].size;
        buff.base_vAddr = rt_deque_buf.data[i].base_vAddr;
        buff.base_pAddr = rt_deque_buf.data[i].base_pAddr;
        buff.timeStampS = rt_deque_buf.data[i].timeStampS;
        buff.timeStampUs = rt_deque_buf.data[i].timeStampUs;
        mSwBufList.bufList.push_back(buff);
    }

    LOG_DBG(" -");      
    return MTRUE;
}
MBOOL IspDrv::dequeueSwBuf(ISP_DRV_FILLED_BUF_LIST bufList)
{   // delete all swBuf list after inform caller    
    MBOOL ret = MTRUE;    

    LOG_DBG(" +");
    
    Mutex::Autolock lock(mQueSwLock);

    if ( 0 == mSwBufList.bufList.size() ) 
    {
        //wait semephore till
        LOG_ERR("empty SW buffer");
        LOG_DBG(" -");
        return MFALSE;
    }
    while ( mSwBufList.bufList.size() ) 
    {
        //all element at the end
        bufList.pBufList->push_back(mSwBufList.bufList.front());
        //delete first element
        mSwBufList.bufList.pop_front();
        //
        LOG_DBG("memID(0x%x)/size(%d)/vaddr(0x%x)/paddr(0x%x)",
                                    bufList.pBufList->back().memID,
                                    bufList.pBufList->back().memID,                                    
                                    bufList.pBufList->back().base_vAddr,
                                    bufList.pBufList->back().base_pAddr);
    }

    LOG_DBG(" -");
    return MTRUE;
}
MUINT32 IspDrv::getCurrHwBuf(void)
{   //get 1st NOT filled HW buffer address    
    return 0;
}
MUINT32 IspDrv::getNextHwBuf(void)
{   // get 2nd NOT filled HW buffer address
    return 0;
}
MUINT32 IspDrv::freeSinglePhyBuf(ISP_DRV_BUF_STRUCT buff)
{   // free single physical buffer
    return 0;
}
MUINT32 IspDrv::freeAllPhyBuf(void)
{   //free all physical buffer (clear kernel ring buffer)
    MBOOL ret = MTRUE;
    
    ISP_RT_BUF_CTRL_STRUCT  rt_buf_ctrl;
    MUINT32 dummy;
    
    LOG_DBG(" +");

    rt_buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
    rt_buf_ctrl.buf_id = ISP_RT_BUF_IMGO;
    rt_buf_ctrl.data_ptr = (MUINT32)&dummy;
    LOG_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)", rt_buf_ctrl.ctrl,rt_buf_ctrl.buf_id,rt_buf_ctrl.data_ptr);        
    ret = this->rtBufCtrl((void*)&rt_buf_ctrl);    
    if( (MFALSE==ret) )
    {
        LOG_ERR("ERROR:clear kernel ring buff failed");
        return -1;
    }

    LOG_DBG(" -");
    return 0;;
}
//----------------------------------------------------------------------------
MBOOL IspDrv::rtBufCtrl(void *pBuf_ctrl)
{
    MINT32 Ret;
    //
    Ret = ioctl(mFd,ISP_IOC_BUFFER_CTRL,pBuf_ctrl);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_BUFFER_CTRL(%d) \n",Ret);
        return MFALSE;
    }
    //
    return MTRUE;
}
#if defined(_use_kernel_ref_cnt_)
MBOOL IspDrv::kRefCntCtrl(ISP_REF_CNT_CTRL_STRUCT* pCtrl)
{
    MINT32 Ret;
    //
    Ret = ioctl(mFd,ISP_IOC_REF_CNT_CTRL,pCtrl);
    if(Ret < 0)
    {
        LOG_ERR("ISP_IOC_REF_CNT_CTRL fail(%d)[errno(%d):%s] \n",Ret, errno, strerror(errno));
        return MFALSE;
    }
    //
    return MTRUE;
}
#endif
