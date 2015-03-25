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
#define LOG_TAG "SeninfDrvImp"
//
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//#include <asm/arch/sync_write.h> 

//
#include "drv_types.h"
#include <mtkcam/drv/isp_drv.h>
#include "camera_isp.h"
#include <mtkcam/drv/isp_reg.h>
#include "seninf_reg.h"
#include "seninf_drv_imp.h"
#include "mtkcam/hal/sensor_hal.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
//
/******************************************************************************
*
*******************************************************************************/
#define SENINF_DEV_NAME     "/dev/mt6582-seninf"
#define ISP_DEV_NAME     	"/dev/camera-isp"
#define SENSOR_DRV_NAME     "/dev/kd_camera_hw"

#define CAM_PLL_RANGE   (0x200)
#define SCAM_ENABLE     1   // 1: enable SCAM feature. 0. disable SCAM feature.

//#define FPGA 0 //for FPGA

#define CAM_APCONFIG_RANGE 0x1000
#define CAM_MMSYS_RANGE 0x100
#define CAM_MIPIRX_CONFIG_RANGE 0x100
#define CAM_MIPIRX_ANALOG_RANGE 0x1000
#define CAM_MIPIPLL_RANGE 0x100
#define CAM_GPIO_RANGE 0x1000

MUINT32 efuse_count = 0;
/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::createInstance()
{
    return SeninfDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrvImp::
getInstance()
{
    LOG_MSG("[getInstance] \n");
    static SeninfDrvImp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
SeninfDrvImp::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::SeninfDrvImp() :
    SeninfDrv()
{
    LOG_MSG("[SeninfDrvImp] \n");

    mUsers = 0;
    mfd = 0;
    m_fdSensor = -1;
//    mMipiType = 0;

}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::~SeninfDrvImp()
{
    LOG_MSG("[~SeninfDrvImp] \n");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()
{
    LOG_MSG("[init]: %d \n", mUsers);
    MBOOL result;
    unsigned int temp = 0,temp1 = 0, temp2 = 0;
    //MINT32 imgsys_cg_clr0 = 0x15000000;
    MINT32 pll_base_hw = 0x10000000;
    MINT32 mipiRx_config = 0x1500C000;
    MINT32 mipiRx_analog = 0x10010000;
    MINT32 gpio_base_addr = 0x10005000;
    MINT32 efuse_base_addr = 0x10206000;
#ifdef USING_MTK_LDVT
    MINT32 imgsys_cg_clr0 = 0x15000000;
    MINT32 mipi_analog_cg_clr0 = 0x14000000;
    unsigned long *pImgsysAddr;
    unsigned long *pMipiAnalogClockAddr;
#endif
    Mutex::Autolock lock(mLock);
    //
    if (mUsers > 0) {
        LOG_MSG("  Has inited \n");
        android_atomic_inc(&mUsers);
        return 0;
    }
    // Open isp driver
    mfd = open(ISP_DEV_NAME, O_RDWR);
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }

	// Access mpIspHwRegAddr
	m_pIspDrv = IspDrv::createInstance();

    if (!m_pIspDrv) {
        LOG_ERR("IspDrvImp::createInstance fail \n");
        return -2;
    }
    //
    result = m_pIspDrv->init();
    if ( MFALSE == result ) {
        LOG_ERR("pIspDrv->init() fail \n");
        return -3;
    }

	//get isp reg for TG module use
    mpIspHwRegAddr = (unsigned long*)m_pIspDrv->getRegAddr();
    if ( NULL == mpIspHwRegAddr ) {
        LOG_ERR("getRegAddr fail \n");
        return -4;
    }

    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned long *) mmap(0, SENINF_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, SENINF_BASE_HW);
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -5;
    }

    // mmap pll reg
    mpPLLHwRegAddr = (unsigned long *) mmap(0, CAM_PLL_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, pll_base_hw);
    if (mpPLLHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(2), %d, %s \n", errno, strerror(errno));
        return -6;
    }



    // mmap seninf clear gating reg   
    /*mpCAMMMSYSRegAddr = (unsigned long *) mmap(0, CAM_MMSYS_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, imgsys_cg_clr0);
    if (mpCAMMMSYSRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(3), %d, %s \n", errno, strerror(errno));
        return -7;
    }*/


    // mipi rx config address
    mpCSI2RxConfigRegAddr = (unsigned long *) mmap(0, CAM_MIPIRX_CONFIG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, mipiRx_config);
    if (mpCSI2RxConfigRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(4), %d, %s \n", errno, strerror(errno));
        return -8;
    }


    // mipi rx analog address
    mpCSI2RxAnalogRegStartAddr = (unsigned long *) mmap(0, CAM_MIPIRX_ANALOG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, mipiRx_analog);
    if (mpCSI2RxAnalogRegStartAddr == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s \n", errno, strerror(errno));
        return -9;
    }


    //gpio
    mpGpioHwRegAddr = (unsigned long *) mmap(0, CAM_GPIO_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, gpio_base_addr);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(6), %d, %s \n", errno, strerror(errno));
        return -10;
    }

    //efuse
    mpEfuseRegAddr = (unsigned long *) mmap(0, 0x400, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, efuse_base_addr);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(6), %d, %s \n", errno, strerror(errno));
        return -14;
    }


    mpIPllCon0RegAddr = mpPLLHwRegAddr + (0x50 /4);

    mpCAMIODrvRegAddr = mpGpioHwRegAddr + (0xB60 / 4);//GPIO122 -> CMMCLK

#ifdef USING_MTK_LDVT
    pImgsysAddr = (unsigned long *) mmap(0, 0x100, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, imgsys_cg_clr0);
    if (pImgsysAddr == MAP_FAILED) {
        LOG_ERR("mmap err(7), %d, %s \n", errno, strerror(errno));
        return -11;
    }

    pMipiAnalogClockAddr = (unsigned long *) mmap(0, 0x200, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mfd, mipi_analog_cg_clr0);
    if (pImgsysAddr == MAP_FAILED) {
        LOG_ERR("mmap err(8), %d, %s \n", errno, strerror(errno));
        return -12;
    }


    mt65xx_reg_writel(0xFFFFFFFF, pImgsysAddr + (0x08));
    mt65xx_reg_writel(0xFFFFFFFF, pMipiAnalogClockAddr + (0x108));
    mt65xx_reg_writel(0xFFFFFFFF, pMipiAnalogClockAddr + (0x118));

    //temp = *(mpGpioHwRegAddr + (0x00));
    //mt65xx_reg_writel(temp|0x600, mpGpioHwRegAddr + (0x0));
    //temp = *(mpGpioHwRegAddr + (0x400/4));
    //mt65xx_reg_writel(temp|0x600, mpGpioHwRegAddr + (0x400/4));
    
     LOG_MSG(" LDVT: turn on imgsys and mipi rx analog power \n");
#endif



#ifdef USING_MTK_LDVT
//        unsigned long *pCMMCLKReg = mpGpioHwRegAddr + (0x780 / 4);
//        *pCMMCLKReg = ( (*pCMMCLKReg)&(~0x1C0) ) | 0x40;
#endif


    mpCSI2RxAnalogRegAddr = mpCSI2RxAnalogRegStartAddr + (0x800/4);

   //set CMMCLK mode 1
    temp = *(mpGpioHwRegAddr + (0x780/4));
    mt65xx_reg_writel(((temp&0xFE3F)|0x0040), mpGpioHwRegAddr + (0x780/4));       

      //turn on nCSI2 first
    temp = *(mpSeninfHwRegAddr + (0x10/4) );
    mt65xx_reg_writel(((temp&0xFFFF0FFF)|0x8000), mpSeninfHwRegAddr + (0x10/4));       


    /* ----------------------------------- 
     Read Efuse data                   
    If M_HW_RES3 [30:27] !=0
    MIPI_RX_ANA24[15:12] = MIPI_RX_ANA24[15:12] + M_HW_RES3 [30:27]
    If M_HW_RES3 [26:23] !=0
    MIPI_RX_ANA24[11:8] = MIPI_RX_ANA24[11:8] + M_HW_RES3 [26:23]    
     ----------------------------------- */ 
    if(efuse_count == 0) {
        temp = *(mpEfuseRegAddr + (0x170/4) );
        if ((temp & 0x78000000) != 0x0) {
            temp2 = *(mpCSI2RxAnalogRegAddr + (0x24/4));
            temp1 = (temp2>>12)&0xF;
            temp1 = ((((temp>>27)&0xF) + temp1)&0xF)<<12;
             LOG_MSG("MIPI_RX_ANA24[15:12]:%d\n", temp1);
            mt65xx_reg_writel((temp1&0xF000)|(temp2&0xFFFF0FFF), mpCSI2RxAnalogRegAddr + (0x24/4));
        }
  
        if ((temp & 0x07800000) != 0x0) {
            temp2 = *(mpCSI2RxAnalogRegAddr + (0x24/4));
            temp1 = (temp2>>8)&0xF;
            temp1 = ((((temp>>23)&0xF) + temp1)&0xF)<<8;
            LOG_MSG("MIPI_RX_ANA24[11:8]:%d\n", temp1);
            mt65xx_reg_writel((temp1&0xF00)|(temp2&0xFFFFF0FF), mpCSI2RxAnalogRegAddr + (0x24/4));
        }
        efuse_count++;
    }   

    android_atomic_inc(&mUsers);

    LOG_MSG("[init]: X \n");


    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    LOG_MSG("[uninit]: %d \n", mUsers);
    MBOOL result;
    unsigned int temp = 0;

    Mutex::Autolock lock(mLock);
    //
    if (mUsers <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);

    
    if (mUsers == 0) {
        // Last user
        setTg1CSI2(0, 0, 0, 0, 0, 0, 0, 0);   // disable CSI2
		setTg1PhaseCounter(0, 0, 0, 0, 0, 0, 0);
         
         //disable MIPI RX analog
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_writel(temp&0xFFFFFFFC, mpCSI2RxAnalogRegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN
        mt65xx_reg_writel(temp&0xFFFFFFBC, mpCSI2RxAnalogRegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalogRegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFE0, mpCSI2RxAnalogRegAddr);

        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x10/4));  
        //set CMMCLK mode 0
        *(mpGpioHwRegAddr + (0x780/4)) &= 0xFE3F;   


        //
        mpIspHwRegAddr = NULL;
        if ( 0 != mpSeninfHwRegAddr ) {
            munmap(mpSeninfHwRegAddr, SENINF_BASE_RANGE);
            mpSeninfHwRegAddr = NULL;
        }
        // Disable Camera PLL
        //if ( mpIPllCon0RegAddr ) {
        //    (*mpIPllCon0RegAddr) |= 0x80000000; //Power Down
        //}
        if ( 0 != mpPLLHwRegAddr ) {
            munmap(mpPLLHwRegAddr, CAM_PLL_RANGE);
            mpPLLHwRegAddr = NULL;
        }
        if ( 0 != mpCAMAPConRegAddr ) {
            munmap(mpCAMAPConRegAddr, CAM_APCONFIG_RANGE);
            mpCAMAPConRegAddr = NULL;
        }
        /*if ( 0 != mpCAMMMSYSRegAddr ) {
            munmap(mpCAMMMSYSRegAddr, CAM_MMSYS_RANGE);
            mpCAMMMSYSRegAddr = NULL;
        }*/
        if ( 0 != mpCSI2RxConfigRegAddr ) {
            munmap(mpCSI2RxConfigRegAddr, CAM_MIPIRX_CONFIG_RANGE);
            mpCSI2RxConfigRegAddr = NULL;
        }
        if ( 0 != mpCSI2RxAnalogRegStartAddr ) {
            munmap(mpCSI2RxAnalogRegStartAddr, CAM_MIPIRX_ANALOG_RANGE);
            mpCSI2RxAnalogRegStartAddr = NULL;
        }
        if ( 0 != mpGpioHwRegAddr ) {
            munmap(mpGpioHwRegAddr, CAM_GPIO_RANGE);
            mpGpioHwRegAddr = NULL;
        }
        if ( 0 != mpEfuseRegAddr ) {
            munmap(mpEfuseRegAddr, 0x400);
            mpEfuseRegAddr = NULL;
        }             
        //
        result = m_pIspDrv->uninit();
        if ( MFALSE == result ) {
            LOG_ERR("pIspDrv->uninit() fail \n");
            return -3;
        }
        
        //
        if (mfd > 0) {
            close(mfd);
            mfd = -1;
        }
    }
    else {
        LOG_MSG("  Still users \n");
    }

    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::waitSeninf1Irq(int mode)
{
    int ret = 0;

    LOG_MSG("[waitIrq polling] 0x%x \n", mode);
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int sleepCount = 40;
    int sts;
    ret = -1;
    while (sleepCount-- > 0) {
        sts = SENINF_READ_REG(pSeninf, SENINF1_INTSTA);  // Not sure CTL_INT_STATUS or CTL_INT_EN
        if (sts & mode) {
            LOG_MSG("[waitIrq polling] Done: 0x%x \n", sts);
            ret = 0;
            break;
        }
        LOG_MSG("[waitIrq polling] Sleep... %d, 0x%x \n", sleepCount, sts);
        usleep(100 * 1000);
    }
    return ret;
}




/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1PhaseCounter(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    unsigned int temp = 0;
    bool clkfl_pol = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned long senFreq=0;

    LOG_MSG("[setTg1PhaseCounter] pcEn(%d) clkPol(%d)\n",pcEn,clkPol);
     
    if (mclkSel == CAM_PLL_48_GROUP) {
        //48MHz
        //(*mpIPllCon0RegAddr) &= 0x78FFFFFF;
        //(*mpIPllCon0RegAddr) |= 0x1000000;
        senFreq=1;
        ret = ioctl(mfd, ISP_SENSOR_FREQ_CTRL,&senFreq);
        if (ret < 0) {
           LOG_ERR("ERROR:ISP_SENSOR_FREQ_CTRL\n");
        }            
    }
    else if (mclkSel == CAM_PLL_52_GROUP) {
        //208MHz
        //(*mpIPllCon0RegAddr) &= 0x78FFFFFF;
        //(*mpIPllCon0RegAddr) |= 0x2000000;
        senFreq=2;
        ret = ioctl(mfd, ISP_SENSOR_FREQ_CTRL,&senFreq);
        if (ret < 0) {
           LOG_ERR("ERROR:ISP_SENSOR_FREQ_CTRL\n");
        }            
    }
    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt         

    //Seninf Top pclk clear gating
    temp = SENINF_READ_REG(pSeninf, SENINF_TOP_CTRL);
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_CTRL, temp|0x400);

    

    temp = ((clkCnt&0x3F)<<16)|((clkRiseEdge&0x3F)<<8)|(clkFallEdge&0x3F);
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_SEN_CK, temp);



    clkfl_pol = (clkCnt & 0x1) ? 0 : 1;
    temp = SENINF_READ_REG(pSeninf, SENINF_TG1_PH_CNT);
    temp &= 0x4FFFFFB8;
    temp |= (((pcEn&0x1)<<31)|(0x20000000)|((clkPol&0x1)<<28)|((padPclkInv&0x1)<<6)|(clkfl_pol<<2)|(0x1));//force PLL due to ISP engine clock dynamic spread
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_PH_CNT, temp);    // mclkSel, 0: 122.88MHz, (others: Camera PLL) 1: 48MHz, 2: 208MHz
    
    temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
    ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x1);

    // Wait 1ms for PLL stable
    usleep(1000);

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1GrabRange] \n");

    // TG Grab Win Setting
    ISP_WRITE_REG(pisp, CAM_TG_SEN_GRAB_PXL, ((pixelEnd&0x7FFF)<<16)|(pixelStart&0x7FFF));
    ISP_WRITE_REG(pisp, CAM_TG_SEN_GRAB_LIN, ((lineEnd&0x7FFF)<<16)|(lineStart&0x7FFF));
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1SensorModeCfg(
    unsigned long hsPol, unsigned long vsPol
)
{
    int ret = 0;
    unsigned int temp = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1SensorModeCfg] \n");

    // Sensor Mode Config
    temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
    temp &= 0xFFFFF9FF;
    temp |= (((hsPol&0x1)<<10)|((vsPol&0x1)<<9));
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, temp);

    temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
    ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x5);

    return ret;
}




/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1InputCfg(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel,
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb
)
{
    int ret = 0;
    unsigned int temp = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1InputCfg] \n");
    LOG_MSG("inSrcTypeSel = % d \n",inSrcTypeSel);
    temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
    temp &= 0xFFF0FFF;
    
    if(inSrcTypeSel == MIPI_SENSOR ) {
        //if(mMipiType == 0) { //MIPI_OPHY_NCSI2
        //    temp |= ((0x80000000)|((padSel&0x7)<<28)|((0x8)<<12));
        //}
        //else if(mMipiType == 1) {
            temp |= ((0x80000000)|((padSel&0x7)<<28));    
        //}
    }
    else {
    temp |= ((0x80000000)|((padSel&0x7)<<28)|((inSrcTypeSel&0xF)<<12));
    }
    
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);

    temp = ISP_READ_REG(pisp, CAM_TG_PATH_CFG);
    ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, temp&0xFFFFFFFC);//no matter what kind of format, set 0

    temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_CLR);
    ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_CLR, temp|0x70000);

    temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_SET);
    ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_SET, temp|((inDataType&0x7)<<16));


	if (MIPI_SENSOR == inSrcTypeSel) {
        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp&0xFFFFFCFF);//SOF_SRC = 0

		if (JPEG_FMT == inDataType) {
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            temp &= 0xF000FFFF;
            temp |= ((0x18<<22)|(0x1E<<16));
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);            

            temp = SENINF_READ_REG(pSeninf, SENINF1_SPARE);
            temp &= 0xFFFFC1FF;
            temp |= 0x2C00;//SENINF_FIFO_FULL_SEL=1, SENINF_VCNT_SEL=1, SENINF_CRC_SEL=2;
            SENINF_WRITE_REG(pSeninf, SENINF1_SPARE,temp); 
		}
		else {
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            temp &= 0xF000FFFF;
            temp |= ((0x1B<<22)|(0x1F<<16));
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);            

		}

	}
	else {
        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x100);//SOF_SRC = 0        

        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        temp &= 0xF000FFFF;
        temp |= ((0x1B<<22)|(0x1F<<16));
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);            
        
	}

	//One-pixel mode
	if ( JPEG_FMT != inDataType) {
        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        temp &= 0xFFFFFEFF;
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);         

        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp&0xFFFFFFFD);//DBL_DATA_BUS = 0   

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_CLR);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_CLR, temp|0x1000000);

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_SET);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_SET, temp&0xFEFFFFFF);

        temp = ISP_READ_REG(pisp, CAM_TG_PATH_CFG);
        ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, temp&0xFFFFFFEF);//JPGINF_EN=0


	}
	else {
        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp|0x100);         
        
        temp = ISP_READ_REG(pisp, CAM_TG_SEN_MODE);
        ISP_WRITE_REG(pisp, CAM_TG_SEN_MODE, temp|0x2);//DBL_DATA_BUS = 1   

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_CLR);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_CLR, temp|0x1000000);

        temp = ISP_READ_REG(pisp, CAM_CTL_FMT_SEL_SET);
        ISP_WRITE_REG(pisp, CAM_CTL_FMT_SEL_SET, temp|0x1000000);

        temp = ISP_READ_REG(pisp, CAM_TG_PATH_CFG);
        ISP_WRITE_REG(pisp, CAM_TG_PATH_CFG, temp|0x10);//JPGINF_EN=1

	}

    #if 0
    temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
    temp &= 0xFFFFFFFC;
    temp |= 0x3;
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);  
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp&0xFFFFFFFC);  
    #endif

    return ret;
}



/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1ViewFinderMode(
    unsigned long spMode, unsigned long spDelay
)
{
    int ret = 0;
    unsigned int temp = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1ViewFinderMode] \n");
    //
    temp = ISP_READ_REG(pisp, CAM_TG_VF_CON);
    temp &= 0xFFFFE0FD;
    temp |= (0x1000|((spMode&0x1)<<1)|((spDelay&0x7)<<8));
    ISP_WRITE_REG(pisp, CAM_TG_VF_CON, temp);

    return ret;
}



/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::sendCommand(int cmd, int arg1, int arg2, int arg3)
{
    int ret = 0;

    LOG_MSG("[sendCommand] cmd: 0x%x \n", cmd);
    switch (cmd) {
    case CMD_SET_DEVICE:
        mDevice = arg1;
        break;
        
    //case CMD_SET_MIPI_TYPE:
    //    mMipiType = arg1;
    //    LOG_MSG("[sendCommand] mMipiType = %d \n", mMipiType);
    //    break;
        
    case CMD_GET_SENINF_ADDR:
        //LOG_MSG("  CMD_GET_ISP_ADDR: 0x%x \n", (int) mpIspHwRegAddr);
        *(int *) arg1 = (int) mpSeninfHwRegAddr;
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned long SeninfDrvImp::readReg(unsigned long addr)
{
    int ret;
    reg_t reg[2];
    int val = 0xFFFFFFFF;

    LOG_MSG("[readReg] addr: 0x%08x \n", (int) addr);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = readRegs(reg, 1);
    if (ret < 0) {
    }
    else {
        val = reg[0].val;
    }

    return val;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::writeReg(unsigned long addr, unsigned long val)
{
    int ret;
    reg_t reg[2];

    LOG_MSG("[writeReg] addr/val: 0x%08x/0x%08x \n", (int) addr, (int) val);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = writeRegs(reg, 1);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::readRegs(reg_t *pregs, int count)
{
    MBOOL result = MTRUE;
    result = m_pIspDrv->readRegs( (ISP_DRV_REG_IO_STRUCT*) pregs, count);
    if ( MFALSE == result ) {
        LOG_ERR("MT_ISP_IOC_G_READ_REG err \n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::writeRegs(reg_t *pregs, int count)
{
    MBOOL result = MTRUE;
    result = m_pIspDrv->writeRegs( (ISP_DRV_REG_IO_STRUCT*) pregs, count);
    if ( MFALSE == result ) {
        LOG_ERR("MT_ISP_IOC_S_WRITE_REG err \n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::holdReg(bool isHold)
{
    int ret;
    int hold = isHold;

    //LOG_MSG("[holdReg]");

    ret = ioctl(mfd, ISP_HOLD_REG, &hold);
    if (ret < 0) {
        LOG_ERR("ISP_HOLD_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::dumpReg()
{
    int ret;

    LOG_MSG("[dumpReg] \n");

    ret = ioctl(mfd, ISP_DUMP_REG, NULL);
    if (ret < 0) {
        LOG_ERR("ISP_DUMP_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::initTg1CSI2(bool csi2_en)
{
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int ret = 0;
    unsigned int temp = 0;


   LOG_MSG("[initCSI2]:enable = %d\n", (int) csi2_en);

	if(csi2_en == 0) {
        //disable mipi BG
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN       
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalogRegAddr + (0x24/4));       
          
        // disable mipi pin
        temp = *(mpGpioHwRegAddr + (0x910/4));//GPI*_IES = 1 for Parallel CAM        
        mt65xx_reg_writel(temp|0x80, mpGpioHwRegAddr + (0x910/4));        

        //mode 2 for parallel pin  (GPIO141~146, 153~156)
        temp = *(mpGpioHwRegAddr + (0x7C0/4));     
        mt65xx_reg_writel(((temp&0x7)|0x2490), mpGpioHwRegAddr + (0x7C0/4));        
        temp = *(mpGpioHwRegAddr + (0x7D0/4));
        mt65xx_reg_writel(((temp&0xFFC0)|0x12), mpGpioHwRegAddr + (0x7D0/4)); 
        temp = *(mpGpioHwRegAddr + (0x7E0/4));
        mt65xx_reg_writel(((temp&0x1FF)|0x2400), mpGpioHwRegAddr + (0x7E0/4)); 
        temp = *(mpGpioHwRegAddr + (0x7F0/4));
        mt65xx_reg_writel(((temp&0xFFC0)|0x12), mpGpioHwRegAddr + (0x7F0/4)); 
        //mode 1 for parallel pin (GPIO120~123)
        temp = *(mpGpioHwRegAddr + (0x780/4));
#if 0
        mt65xx_reg_writel(((temp&0xF000)|0x249), mpGpioHwRegAddr + (0x780/4)); 
#else
        LOG_MSG("Don't control GPIO123(CMPCLK)\n");
        mt65xx_reg_writel(((temp&0xFE00)|0x49), mpGpioHwRegAddr + (0x780/4));
#endif

        //GPI setting
        temp = *(mpCSI2RxAnalogRegAddr + (0x48/4));
        mt65xx_reg_writel(((temp&0xFFFFFC3F)|0x3C0), mpCSI2RxAnalogRegAddr + (0x48/4));        
        temp = *(mpCSI2RxAnalogRegAddr + (0x4C/4));
        mt65xx_reg_writel(((temp&0xFEFBEFBE)|0x1041041), mpCSI2RxAnalogRegAddr + (0x4C/4));        
        temp = *(mpCSI2RxAnalogRegAddr + (0x50/4));
        mt65xx_reg_writel(((temp&0xFFFFFFFE)|0x1), mpCSI2RxAnalogRegAddr + (0x50/4));        


       temp = *(mpCSI2RxAnalogRegAddr + (0x00));//main & sub clock lane input select hi-Z
       mt65xx_reg_writel(temp&0xFFFFFFE7, mpCSI2RxAnalogRegAddr + (0x00));        
       temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//main data lane 0 input select hi-Z    
       mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x04/4));        
       temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//main data lane 1 input select hi-Z
       mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x08/4));        
       temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//main data lane 2 & sub data lane 0 input select hi-Z         
       mt65xx_reg_writel(temp&0xFFFFFFE7, mpCSI2RxAnalogRegAddr + (0x0C/4));        
       temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//main data lane 3 & sub data lane 1 input select hi-Z       
       mt65xx_reg_writel(temp&0xFFFFFFE7, mpCSI2RxAnalogRegAddr + (0x10/4));

	}
	else {
        //GPI
        //GPI setting
        temp = *(mpCSI2RxAnalogRegAddr + (0x48/4));
        mt65xx_reg_writel((temp&0xFFFFFC3F), mpCSI2RxAnalogRegAddr + (0x48/4));        
        temp = *(mpCSI2RxAnalogRegAddr + (0x4C/4));
        mt65xx_reg_writel((temp&0xFEFBEFBE), mpCSI2RxAnalogRegAddr + (0x4C/4));        
        temp = *(mpCSI2RxAnalogRegAddr + (0x50/4));
        mt65xx_reg_writel((temp&0xFFFFFFFE), mpCSI2RxAnalogRegAddr + (0x50/4));  
        
         // enable mipi lane
        if(mDevice & SENSOR_DEV_MAIN) {
            //disable GPI        
            temp = *(mpGpioHwRegAddr + (0x910/4));//GPI*_IES = 0 
            mt65xx_reg_writel(temp&0xFF7F, mpGpioHwRegAddr + (0x910/4));        
             //mode 1   (GPIO147~156)
            temp = *(mpGpioHwRegAddr + (0x7D0/4)); 
            mt65xx_reg_writel(((temp&0x3F)|0x1240), mpGpioHwRegAddr + (0x7D0/4));        
            temp = *(mpGpioHwRegAddr + (0x7E0/4));
            mt65xx_reg_writel(((temp&0x0)|0x1249), mpGpioHwRegAddr + (0x7E0/4));                   
            temp = *(mpGpioHwRegAddr + (0x7F0/4));
            mt65xx_reg_writel(((temp&0xFFC0)|0x9), mpGpioHwRegAddr + (0x7F0/4));                   

           temp = *(mpCSI2RxAnalogRegAddr + (0x00));//main clock lane input select mipi enable
           mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x00));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//main data lane 0 input select mipi enable
           mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x04/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//main data lane 1 input select mipi enable
           mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x08/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//main data lane 2 input select mipi enable       
           mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x0C/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//main data lane 3 input select mipi enable      
           mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalogRegAddr + (0x10/4));            
           temp = *(mpCSI2RxAnalogRegAddr + (0x00));//sub clock lane input select mipi disable
           mt65xx_reg_writel(temp&0xFFFFFFEF, mpCSI2RxAnalogRegAddr + (0x00));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//sub data lane 0 input select mipi disable         
           mt65xx_reg_writel(temp&0xFFFFFFEF, mpCSI2RxAnalogRegAddr + (0x0C/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//sub data lane 1 input select mipi disable      
           mt65xx_reg_writel(temp&0xFFFFFFEF, mpCSI2RxAnalogRegAddr + (0x10/4));      

           temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN        
           mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x04/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN        
           mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x08/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN        
           mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x0C/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN        
           mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x10/4));  
       
           temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));        
           mt65xx_reg_writel((temp&0xFFFFFFF3), mpCSI2RxAnalogRegAddr + (0x20/4));    

           temp = *(mpCSI2RxConfigRegAddr + (0x24/4));//       
           mt65xx_reg_writel(((temp&0xFFFFFF)|0xE4000000), mpCSI2RxConfigRegAddr + (0x24/4));             


        }
        else if (mDevice & SENSOR_DEV_SUB) {
            //disable GPI        
            temp = *(mpGpioHwRegAddr + (0x910/4));//GPI*_IES = 0 
            mt65xx_reg_writel(temp&0xFF7F, mpGpioHwRegAddr + (0x910/4));                
            //mode 1   (GPIO141~146)
            temp = *(mpGpioHwRegAddr + (0x7C0/4));     
            mt65xx_reg_writel(((temp&0x7)|0x1248), mpGpioHwRegAddr + (0x7C0/4));        
            temp = *(mpGpioHwRegAddr + (0x7D0/4));
            mt65xx_reg_writel(((temp&0xFFC0)|0x9), mpGpioHwRegAddr + (0x7D0/4));                   
            
           temp = *(mpCSI2RxAnalogRegAddr + (0x00));//sub clock lane input select mipi enable
           mt65xx_reg_writel(temp|0x00000010, mpCSI2RxAnalogRegAddr + (0x00));            
           temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//sub data lane 0 input select mipi enable       
           mt65xx_reg_writel(temp|0x00000010, mpCSI2RxAnalogRegAddr + (0x0C/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//sub data lane 1 input select mipi enable       
           mt65xx_reg_writel(temp|0x00000010, mpCSI2RxAnalogRegAddr + (0x10/4));    
           temp = *(mpCSI2RxAnalogRegAddr + (0x00));//main clock lane input select mipi disable
           mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x00));       
           temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//main data lane 0 input select mipi disable         
           mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x04/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//main data lane 1 input select mipi disable     
           mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x08/4));            
           temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//main data lane 2 input select mipi disable         
           mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x0C/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//main data lane 3 input select mipi disable      
           mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalogRegAddr + (0x10/4));   
      
           temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN        
           mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x0C/4));        
           temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN        
           mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x10/4));  
           
           temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));        
           mt65xx_reg_writel(((temp&0xFFFFFFF3)|0xC), mpCSI2RxAnalogRegAddr + (0x20/4));       
           usleep(1);           
           temp = *(mpCSI2RxConfigRegAddr + (0x24/4));//       
           mt65xx_reg_writel(((temp&0xFFFFFF)|0x1B000000), mpCSI2RxConfigRegAddr + (0x24/4)); 

        }

        // enable mipi ap
        temp = *(mpCSI2RxAnalogRegAddr + (0x24/4));//RG_CSI_BG_CORE_EN       
        //mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x24/4));  //JH test      
        mt65xx_reg_writel(temp|0x00000003, mpCSI2RxAnalogRegAddr + (0x24/4));                
       usleep(30);
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN        
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr + (0x20/4));       
       usleep(1);

       temp = *(mpCSI2RxAnalogRegAddr); //RG_CSI0_LNRC_LDO_OUT_EN        
       mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalogRegAddr);



        //JH test add
       temp = *(mpCSI2RxConfigRegAddr + (0x24/4));//MIPI_RX_HW_CAL_START       
       mt65xx_reg_writel(temp|0x00080000, mpCSI2RxConfigRegAddr + (0x24/4));   


       //Offset calibration
       temp = *(mpCSI2RxConfigRegAddr + (0x38/4));//1.	MIPI_RX_SW_CTRL_MODE        
       mt65xx_reg_writel(temp|0x1, mpCSI2RxConfigRegAddr + (0x38/4));        

            
       mt65xx_reg_writel(0x1541, mpCSI2RxConfigRegAddr + (0x3C/4)); //bit 0,6,8,10,12        

        //JH test mark
       temp = *(mpCSI2RxConfigRegAddr + (0x38/4));//MIPI_RX_HW_CAL_START       
       mt65xx_reg_writel(temp|0x00000004, mpCSI2RxConfigRegAddr + (0x38/4));   

       LOG_MSG("[initCSI2]:CSI0 calibration start !\n");
       usleep(500);
       //JH test mark
       if(!((*(mpCSI2RxConfigRegAddr + (0x44/4)) & 0x10101) && (*(mpCSI2RxConfigRegAddr + (0x48/4)) & 0x101))){
            LOG_ERR("[initCSI2]:CSI0 calibration failed!, CSI2Config Reg 0x44=0x%x, 0x48=0x%x\n",*(mpCSI2RxConfigRegAddr + (0x44/4)),*(mpCSI2RxConfigRegAddr + (0x48/4))); 
        //    ret = -1;       
       }
       LOG_MSG("[initCSI2]:CSI0 calibration end !\n");

       temp = *(mpCSI2RxConfigRegAddr + (0x38/4));//1.	MIPI_RX_SW_CTRL_MODE        
       mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxConfigRegAddr + (0x38/4)); 


        //apply old analog design default setting
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));//RG_CSI0_4XCLK_INVERT = 0      
        mt65xx_reg_writel(temp&0xFFFFFFDF, mpCSI2RxAnalogRegAddr + (0x20/4));             
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//RG_CSI0_LNRD0_HSRX_BYPASS_SYNC = 0      
        mt65xx_reg_writel(temp&0xFFBFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4)); 
        //mt65xx_reg_writel(temp|0x400000, mpCSI2RxAnalogRegAddr + (0x04/4)); 
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//RG_CSI0_LNRD1_HSRX_BYPASS_SYNC = 0       
        mt65xx_reg_writel(temp&0xFFBFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4)); 
        //mt65xx_reg_writel(temp|0x400000, mpCSI2RxAnalogRegAddr + (0x08/4)); 
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//RG_CSI0_LNRD2_HSRX_BYPASS_SYNC = 0     
        mt65xx_reg_writel(temp&0xFFBFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4)); 
        //mt65xx_reg_writel(temp|0x400000, mpCSI2RxAnalogRegAddr + (0x0C/4)); 
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//RG_CSI0_LNRD3_HSRX_BYPASS_SYNC = 0      
        mt65xx_reg_writel(temp&0xFFBFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4)); 
        //mt65xx_reg_writel(temp|0x400000, mpCSI2RxAnalogRegAddr + (0x10/4)); 
        temp = *(mpCSI2RxAnalogRegAddr + (0x20/4));//RG_CSI0_4XCLK_DISABLE = 1,RG_CSI0_LNRD_HSRX_BCLK_INVERT = 1        
        #if 0
        if(mMipiType == 0) { //MIPI_OPHY_NCSI2
            //mt65xx_reg_writel(temp|0x00000042, mpCSI2RxAnalogRegAddr + (0x20/4));  
            mt65xx_reg_writel((temp&0xFFFFFFFD)|0x41, mpCSI2RxAnalogRegAddr + (0x20/4));     
        temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_SPARE0);
        SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_SPARE0,(temp | 0x80000000));

	}    
        else if(mMipiType == 1) {//MIPI_OPHY_CSI2
        #endif
            mt65xx_reg_writel(temp|0x00000002, mpCSI2RxAnalogRegAddr + (0x20/4));//JH debug for ncsi2     
            if(mDevice & SENSOR_DEV_MAIN) {
                SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_LNMUX,0xE4);         
            }
            else if (mDevice & SENSOR_DEV_SUB) {
                SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_LNMUX,0x1B);                     
            }               
        #if 0               
        }
        #endif

	}    

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlane_num, unsigned long csi2_en,
    unsigned long dataheaderOrder, unsigned long dataFlow
)
{
    int ret = 0;
    unsigned int temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
	#if 0
    if(mMipiType == 0) {
    if(csi2_en == 1) {  // enable NCSI2
        LOG_MSG("[configTg1CSI2]:DataTermDelay:%d SettleDelay:%d ClkTermDelay:%d VsyncType:%d dlane_num:%d CSI2 enable:%d HeaderOrder:%d DataFlow:%d\n",
        	(int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlane_num, (int) csi2_en, (int)dataheaderOrder, (int)dataFlow);

        temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,((temp&0xFFFF0FFF)|0x8000));//nCSI2 must be slected when sensor clk output


        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFEEF); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)


		#if (defined(FPGA))
            temp = (dataSettleDelay&0xFF)<<8;  
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);        

            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL,(temp|(1<<16)|(csi2_en<<4)|(((1<<(dlane_num+1))-1)))) ;
            
        #else
                temp = ((dataSettleDelay&0xFF)<<8) | ((dataTermDelay&0xff));
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);   
                    
            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        	SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL,(temp|(dataheaderOrder<<16)|(csi2_en<<4)|(((1<<(dlane_num+1))-1)))) ;
		#endif
        //Turn on NCIS2 IRQ
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_EN,0x000000F8);//turn on all interrupt
        //Turn Off CSI2 IRQ
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTEN); 
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTEN, temp&(~0x7));
        //Read clear
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTSTA); 
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTEN, temp&(~0x10));

            #if 1
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            temp &= 0xFFFFFFFC;
            temp |= 0x3;
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);  
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp&0xFFFFFFFC); 
            #endif
        
    }
    else {   // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2 first    
    }

    }
    else if (mMipiType == 1) {
    #endif
        if(csi2_en == 1) {  // enable CSI2
            LOG_MSG("[configTg1CSI2]:DataTermDelay:%d SettleDelay:%d ClkTermDelay:%d VsyncType:%d dlane_num:%d CSI2 enable:%d HeaderOrder:%d DataFlow:%d\n",
            	(int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlane_num, (int) csi2_en, (int)dataheaderOrder, (int)dataFlow);
    
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,(temp&0xFFFF0FFF));//nCSI2 must be slected when sensor clk output
            
            //temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);        
            //SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFFE);    // disable CSI2 first
            
            temp = (dataSettleDelay&0xFF)<<16;
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DELAY, temp); 
            
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTEN); 
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTEN, temp|0x7);
    
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTSTA); 
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTEN, temp|0x10);
    
    
           	SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL,(dataFlow<<17) | (vsyncType <<13) | (1 << 10) | (dataheaderOrder<<5) | (1<<4) | (((1<<dlane_num)-1)<<1) | (csi2_en<<0));

            #if 1
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            temp &= 0xFFFFFBFC;
            temp |= 0x3;
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp);  
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,temp&0xFFFFFFFC); 
            #endif    
        }
        else {   // disable CSI2
            //temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
            //SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFFE);
        }
    #if 0
    }
    #endif
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
#ifdef ATV_SUPPORT
int SeninfDrvImp::initTg1Serial(bool serial_en)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SCAM1_CON, Enable, serial_en);
    LOG_MSG("[initTg1Serial]:test_atv   serial_en(%d)", (int)serial_en);
    return ret;
}
 
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1Serial(
    unsigned long clk_inv, unsigned long width, unsigned long height,
    unsigned long conti_mode, unsigned long csd_num)        
{
    int ret = 0;    
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    //set CMDAT0(gpio_120) mode 2
    //set CMPCLK(gpio_123) mode 2
    unsigned int temp = 0;
    temp = *(mpGpioHwRegAddr + (0x780/4));
    mt65xx_reg_writel(((temp&0xF1F8)|0x402), mpGpioHwRegAddr + (0x780/4));


    SENINF_WRITE_BITS(pSeninf, SCAM1_SIZE, WIDTH, width);
    SENINF_WRITE_BITS(pSeninf, SCAM1_SIZE, HEIGHT, height);    

    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Clock_inverse, clk_inv);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Continuous_mode, conti_mode);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, CSD_NUM, csd_num);  // 0(1-lane);1(2-line);2(4-lane)
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Cycle, 4);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG2, DIS_GATED_CLK, 1);    
    LOG_MSG("[setTg1Serial] test_atv");
    return ret;
}
#endif
int SeninfDrvImp::resetSeninf()
{
    int ret = 0;
    unsigned int temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);	
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, (temp|0x3));    
    usleep(10);
    LOG_MSG("[resetSeninf] reset done\n");
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, (temp&0xFFFFFFFC));    
    
    return ret;
    
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;
    unsigned int temp = 0;

    if(mpCAMIODrvRegAddr != NULL) {
        temp = *(mpCAMIODrvRegAddr);
        mt65xx_reg_writel(temp&0xFFFF0FFF, mpCAMIODrvRegAddr);        
        temp = *(mpCAMIODrvRegAddr);// CLK  CAM1  CAM
        mt65xx_reg_writel(temp|ioDrivingCurrent, mpCAMIODrvRegAddr);        

    }
//    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(mpCAMIODrvRegAddr)));

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1MCLKEn(bool isEn)
{
    int ret = 0;
    unsigned int temp = 0;   
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    temp = SENINF_READ_REG(pSeninf, SENINF_TG1_PH_CNT);
    temp &= 0xDFFFFFFF;
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_PH_CNT, temp|(isEn<<29));


    return ret;
}



int SeninfDrvImp::setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount,
			unsigned long startLine, unsigned long startPixel, unsigned long  flashPol)
{
    int ret = 0;
    unsigned int temp = 0;    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;


    temp = ISP_READ_REG(pisp, CAM_TG_FLASHA_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_CTL, temp&0xFFFFFFFE);//disable flasha_en    

    temp &= 0x2;
    temp |= ((flashPol&0x1)<<12)|((endFrame&0x7)<<8)|((startPoint&0x3)<<4);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_CTL, temp);

    temp = ((lineUnit&0xF)<<20)|(unitCount&0xFFFFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_LINE_CNT, temp);    

    temp = ((startLine&0x1FFF)<<16)|(startPixel&0x7FFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_POS, temp);    

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHB_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp|0x1);//enable flasha_en


	return ret;

}


int SeninfDrvImp::setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel)
{
    int ret = 0;
    unsigned int temp = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHB_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp&0xFFFFFFFE);//disable flashb_en

    temp = ((contiFrm&0x7)<<12)|((startFrame&0xF)<<8);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp);//disable flashb_en    

    temp = ((lineUnit&0xF)<<20)|(unitCount&0xFFFFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_LINE_CNT, temp);

    temp = ((startLine&0x1FFF)<<16)|(startPixel&0x7FFF);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_POS, temp);

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHB_CTL);
    ISP_WRITE_REG(pisp, CAM_TG_FLASHB_CTL, temp|0x1);//enable flashb_en

	return ret;
}

int SeninfDrvImp::setFlashEn(bool flashEn)
{
	int ret = 0;
    unsigned int temp = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    temp = ISP_READ_REG(pisp, CAM_TG_FLASHA_CTL);
    temp &= 0xFFFFFFFD;
    ISP_WRITE_REG(pisp, CAM_TG_FLASHA_CTL, temp|(flashEn<<1));

	return ret;

}



int SeninfDrvImp::setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd)
{
    int ret = 0;
    unsigned int temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	if ((hsStart > 4095) || (hsEnd > 4095))
	{
		LOG_ERR("CCIR656 HSTART or HEND value err \n");
		ret = -1;
	}
    temp = SENINF_READ_REG(pSeninf,CCIR656_CTL);
    temp &= 0xFFFFFFF3;
    temp |= (((vsPol&0x1)<<3)|((hsPol&0x1)<<1));
    SENINF_WRITE_REG(pSeninf,CCIR656_CTL, temp);

    temp = ((hsEnd&0xFFF)<<16)|(hsStart&0xFFF);
    SENINF_WRITE_REG(pSeninf, CCIR656_H, temp);

	return ret;
}

int SeninfDrvImp::checkSeninf1Input()
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;          

    temp = SENINF_READ_REG(pSeninf,SENINF1_DEBUG_4);
    LOG_MSG("[checkSeninf1Input]:size = 0x%x",temp);        
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;
    
    if( (tempW >= tg1GrabWidth) && (tempH >= tg1GrabHeight)  ) {
        ret = 0;
    }
    else {           
        ret = 1;
    }

    return ret;

}


int SeninfDrvImp::autoDeskewCalibration()
{
    int ret = 0;
    unsigned int temp = 0;
    MUINT32 lane_num=0,min_lane_code=0;
    MUINT8 lane0_code=0,lane1_code=0,lane2_code=0,lane3_code=0,i=0;
    MUINT8 clk_code=0;
    MUINT32 currPacket = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    
    LOG_MSG("autoDeskewCalibration start \n");

    temp = *(mpCSI2RxAnalogRegAddr + (0x00));//disable clock lane delay
    mt65xx_reg_writel(temp&0xFFEFFFFF, mpCSI2RxAnalogRegAddr + (0x00));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//disable data lane 0 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//disable data lane 2 delay         
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));        
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//disable data lane 3 delay       
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));


    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DGB_SEL,0x12);//set debug port to output packet number
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_EN,0x80007FFF);//set interrupt enable //@write clear?

    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration 
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line %d, default input has error, please check it \n",__LINE__);
    }
    LOG_MSG("autoDeskewCalibration start interupt status = 0x%x\n",SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS));
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
    
    //Fix Clock lane
    //set lane 0
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//enable data lane 0 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x04/4));     

    for(i=0; i<=0xF; i++) {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        //@add read interrupt status to clear
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));    
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x04/4));   
        //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//disable data lane 0 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));    
    usleep(5);
    
    //set lane 1
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//enable data lane 1 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x08/4));      

    for(i=0; i<=0xF; i++) {
        lane1_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));    
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x08/4));         
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//disable data lane 1 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));      
    usleep(5);
    
    //set lane 2
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//enable data lane 2 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x0C/4));      
  
    for(i=0; i<=0xF; i++) {
        lane2_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));    
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x0C/4));           
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//disable data lane 2 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));     
    usleep(5);

    //set lane 3    
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//enable data lane 3 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x10/4));      

    for(i=0; i<=0xF; i++) {
        lane3_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));           
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));    
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x10/4));                   
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//disable data lane 3 delay    
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));     
  
  LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d \n",lane0_code,lane1_code,lane2_code,lane3_code);

    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }
    if(min_lane_code > lane2_code) {
        min_lane_code = lane2_code;
    }
    if(min_lane_code > lane3_code) {
        min_lane_code = lane3_code;
    }
    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d \n",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code);


    //Fix Data lane
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//set to 0 first    
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));    
    mt65xx_reg_writel(temp|(((lane0_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x04/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));    
    mt65xx_reg_writel(temp|(((lane1_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x08/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));   
    mt65xx_reg_writel(temp|(((lane2_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x0C/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));   
    mt65xx_reg_writel(temp|(((lane3_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x10/4));      
    temp = *(mpCSI2RxAnalogRegAddr);//enable clock lane delay    
    mt65xx_reg_writel(temp|0x100000, mpCSI2RxAnalogRegAddr );
    temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//enable data lane 0 delay    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x04/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//enable data lane 1 dela   
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x08/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//enable data lane 2 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x0C/4));      
    temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//enable data lane 3 dela    
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalogRegAddr + (0x10/4)); 

    LOG_MSG("autoDeskewCalibration start test 5 \n");    

    for(i=0; i<=0xF; i++) {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalogRegAddr);//set to 0 first    
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalogRegAddr );
        temp = *(mpCSI2RxAnalogRegAddr);   
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalogRegAddr );
        while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         //usleep(5);
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);
        lane2_code = lane2_code -((min_lane_code+clk_code)>>1);
        lane3_code = lane3_code -((min_lane_code+clk_code)>>1); 
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x04/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x08/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x0C/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));//set to 0 first    
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalogRegAddr + (0x10/4));          
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x04/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x08/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x0C/4));      
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalogRegAddr + (0x10/4));          
        temp = *(mpCSI2RxAnalogRegAddr);//clk code = 0   
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalogRegAddr );
    }
    else {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        temp = *(mpCSI2RxAnalogRegAddr);//set to 0 first   
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalogRegAddr );
        temp = *(mpCSI2RxAnalogRegAddr); 
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalogRegAddr );               
    }
    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d\n",clk_code,min_lane_code);


    
    LOG_MSG("autoDeskewCalibration end \n");
    return ret;
}

void SeninfDrvImp::resetCSI2()
{
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    
    unsigned int temp = 0;     
 
    
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTSTA, temp & 0xFFFFFFEF);//read clear 
    temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTSTA); 
    LOG_MSG("[resetCSI2] CSI2 Interrupt Status = 0x%x\n", temp);
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTSTA, temp | 0x10);
    
    temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);        
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp|0x1000); //CSI2 SW reset
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFEFFF); //clear CSI2 SW reset    
    
}





