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
#define LOG_TAG "SeninfDrv"
//
#include <fcntl.h>
#include <sys/mman.h>
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/threads.h>                  // For android::Mutex.
#else
#include <utils/Mutex.h>                    // For android::Mutex.
#endif
#include <cutils/atomic.h>
#include <cutils/properties.h>              // For property_get().
//#include <mt6589_sync_write.h>
#include <cutils/xlog.h>
//
#include "drv_types.h"
#include "camera_isp.h"
#include "isp_reg.h"
#include "seninf_reg.h"
#include "seninf_drv.h"
#include "mtkcam/hal/sensor_hal.h"
//#include "gpio_const.h"
//#include "mt_gpio.h"
#define GPIO_MODE_00 0
#define GPIO_MODE_01 1
#define GPIO_MODE_02 2
#define GPIO62       62
#define GPIO61       61

//-----------------------------------------------------------------------------
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG    LOG_TAG
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(seninf_drv);
EXTERN_DBG_LOG_VARIABLE(seninf_drv);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (seninf_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (seninf_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (seninf_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (seninf_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (seninf_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (seninf_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
/******************************************************************************
*
*******************************************************************************/
#define SENINF_DEV_NAME         "/dev/mt6572-seninf"
#define ISP_DEV_NAME            "/dev/camera-isp"

#define SCAM_ENABLE             (1)     // 1: enable SCAM feature. 0. disable SCAM feature.
#define FPGA                    (0)     // Jason TODO remove
#define JASON_TMP               (0)     // Jason TODO remove, for rst/pdn sensor on FPGA

#define CAM_MMSYS_CONFIG_BASE       0x14000000  //MT6572
#define CAM_MMSYS_CONFIG_RANGE      0x100
#define CAM_MIPI_RX_CONFIG_BASE     0x14015000	//MT6572
#define CAM_MIPI_RX_CONFIG_RANGE    0x100
#define CAM_MIPI_CONFIG_BASE        0x10011000	//MT6572
#define CAM_MIPI_CONFIG_RANGE       0x1000
#define CAM_GPIO_CFG_BASE           0x1020B000	//MT6572
#define CAM_GPIO_CFG_RANGE          0x100
//#define CAM_GPIO_BASE               0x10005000	//MT6572
//#define CAM_GPIO_RANGE              0x1000
//#define CAM_PLL_BASE                0x10000000  //Jason TODO Reg
//#define CAM_PLL_RANGE               0x200
//#define CAM_APCONFIG_RANGE      0x1000
//#define CAM_MIPIRX_ANALOG_RANGE 0x1000
//#define CAM_MIPIPLL_RANGE       0x100

int SEN_CSI2_SETTING = 1;
int SEN_CSI2_ENABLE = 1;

// no need to do sync write after 89; this only for coding sync
#define mt65xx_reg_sync_writel(v, a)                \
        do {                                        \
            *(volatile unsigned int *)(a) = (v);    \
        } while (0)
/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::createInstance()
{
    DBG_LOG_CONFIG(drv, seninf_drv);
    return SeninfDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
static SeninfDrv singleton;
SeninfDrv*
SeninfDrv::
getInstance()
{
    LOG_MSG("[getInstance] \n");    
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
SeninfDrv::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv::SeninfDrv()
{
    LOG_MSG("[SeninfDrv] \n");

    mUsers = 0;
    mfd = 0;
    tg1GrabWidth = 0;
    tg1GrabHeight = 0;	
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv::~SeninfDrv()
{
    LOG_MSG("[~SeninfDrv] \n");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::init()  
{
    LOG_MSG("[init]: Entry count %d \n", mUsers);
    MBOOL result;
    //MINT32 imgsys_cg_clr0 = 0x15000000;
    //MINT32 pll_base_hw = 0x10000000;
    //MINT32 mipiRx_config = 0x1500C000;    
    //MINT32 mipiRx_analog = 0x10012000;  //MINT32 mipiRx_analog = 0x10012800;
    //MINT32 gpio_base_addr = 0x10001000;   

    Mutex::Autolock lock(mLock);
    //
    if (mUsers > 0) {
        LOG_MSG("  Has inited \n");
        android_atomic_inc(&mUsers);
        return 0;
    }

    // open isp driver
    mfd = open(ISP_DEV_NAME, O_RDWR);
    LOG_MSG("  Open ISP kernel done \n");
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }

    // access mpIspHwRegAddr
    m_pIspDrv = IspDrv::createInstance();
    LOG_MSG("  Create ISP Instance done \n");    
    if (!m_pIspDrv) {
        LOG_ERR("IspDrvImp::createInstance fail \n");
        return -2;
    }
    //
    result = m_pIspDrv->init();
    LOG_MSG("  IspDrv init done \n");
    if ( MFALSE == result ) {
        LOG_ERR("pIspDrv->init() fail \n");
        return -3;
    }

    //get isp reg for TG module use
    mpIspHwRegAddr = (unsigned long*)m_pIspDrv->getRegAddr();
    LOG_MSG("  IspDrv getRegAddr done \n");
    if ( NULL == mpIspHwRegAddr ) {
        LOG_ERR("getRegAddr fail \n");
        return -4;
    }

    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned long *) mmap(0, SENINF_BASE_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, SENINF_BASE_HW);
    LOG_MSG("  mmap 1 done \n");
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -5;
    }
    
#if (!FPGA)  // Jason TODO remove    
    // mmap gpio reg 
    /*  mt_gpio_set_mode at isp kernel driver  
    mpGpioHwRegAddr = (unsigned long *) mmap(0, CAM_GPIO_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, CAM_GPIO_BASE);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(6), %d, %s \n", errno, strerror(errno));
        return -10;
    }
    */        
    mpCAMIODrvRegAddr = (unsigned long *) mmap(0, CAM_GPIO_CFG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_GPIO_CFG_BASE);
    if(mpCAMIODrvRegAddr == MAP_FAILED)
    {
        return -10;
    }    
           
    // mmap pll reg
    /*  clkmux_sel at isp kernel driver  
    mpPLLHwRegAddr = (unsigned long *) mmap(0, CAM_PLL_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, CAM_PLL_BASE);
    if (mpPLLHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(2), %d, %s \n", errno, strerror(errno));
        return -6;
    }
    mpIPllCon0RegAddr = mpPLLHwRegAddr + (0x158 /4);
    */

    // mmap seninf clear gating reg
    mpCAMMMSYSRegAddr = (unsigned long *) mmap(0, CAM_MMSYS_CONFIG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_MMSYS_CONFIG_BASE);
    if (mpCAMMMSYSRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(3), %d, %s \n", errno, strerror(errno));
        return -7;
    }

    // mipi rx config address
    mpCSI2RxConfigRegAddr = (unsigned long *) mmap(0, CAM_MIPI_RX_CONFIG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_MIPI_RX_CONFIG_BASE);
    if (mpCSI2RxConfigRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(4), %d, %s \n", errno, strerror(errno));
        return -8;
    }

    // mipi rx analog address
    mpCSI2RxAnalogRegStartAddr = (unsigned long *) mmap(0, CAM_MIPI_CONFIG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_MIPI_CONFIG_BASE);
    if (mpCSI2RxAnalogRegStartAddr == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s \n", errno, strerror(errno));
        return -9;
    }
    mpCSI2RxAnalogRegAddr = mpCSI2RxAnalogRegStartAddr + (0x800/4);

    
    //set CMMCLK(gpio_62) mode 1 
    ISP_GPIO_SEL_STRUCT gpioCtrl;
    gpioCtrl.Pin = GPIO62;          // CMMCLK
    gpioCtrl.Mode = GPIO_MODE_01;
    ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);        

    //set CMPCLK(gpio_61) mode 1 
    gpioCtrl.Pin = GPIO61;	    // CMPCLK
    gpioCtrl.Mode = GPIO_MODE_01;
    ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);	
#endif

#if (FPGA)
    // For FPGA    
    LOG_WRN("Sensor init rst/pdn power up\n");     
    setPdnRst(1, 1);    
#endif

    android_atomic_inc(&mUsers);

    LOG_MSG("[init]: Exit count %d \n", mUsers);

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::uninit()
{
    LOG_MSG("[uninit]: %d \n", mUsers);
    MBOOL result;    

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

#if (FPGA)  // Jason TODO remove	
    LOG_WRN("Sensor uninit rst/pdn power down");     
    setPdnRst(1, 0);
#endif

#if (!FPGA)  // Jason TODO remove		        
        //set CMMCLK(gpio_62) mode 0         
        ISP_GPIO_SEL_STRUCT gpioCtrl;
        gpioCtrl.Pin = GPIO62;          // CMMCLK
        gpioCtrl.Mode = GPIO_MODE_00;
        ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);

        //set CMPCLK(gpio_61) mode 0         
        gpioCtrl.Pin = GPIO61;          // CMPCLK
        gpioCtrl.Mode = GPIO_MODE_00;
        ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);
		
        // Disable Camera PLL // Jason TODO PLL
        //(*mpIPllCon0RegAddr) |= 0x01; //Power Down        

        // Jason TODO Reg
        //disable MIPI RX analog
        *(mpCSI2RxAnalogRegAddr + (0x24/4)) &= 0xFFFFFFFE;//RG_CSI_BG_CORE_EN
        *(mpCSI2RxAnalogRegAddr + (0x20/4)) &= 0xFFFFFFFE;//RG_CSI0_LDO_CORE_EN        
        *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFFE;//RG_CSI0_LNRC_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFFE;//RG_CSI0_LNRD0_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFFE;//RG_CSI0_LNRD1_LDO_OUT_EN

        // munmap rx analog address
        if ( 0 != mpCSI2RxAnalogRegStartAddr ) {
            munmap(mpCSI2RxAnalogRegStartAddr, CAM_MIPI_CONFIG_RANGE);
            mpCSI2RxAnalogRegStartAddr = NULL;
        }
        // munmap rx config address
        if ( 0 != mpCSI2RxConfigRegAddr ) {
            munmap(mpCSI2RxConfigRegAddr, CAM_MIPI_RX_CONFIG_RANGE);
            mpCSI2RxConfigRegAddr = NULL;
        }
        // munmap seninf clear gating reg
        if ( 0 != mpCAMMMSYSRegAddr ) {
            munmap(mpCAMMMSYSRegAddr, CAM_MMSYS_CONFIG_RANGE);
            mpCAMMMSYSRegAddr = NULL;
        }                
        // munmap gpio reg 
        if ( 0 != mpCAMIODrvRegAddr ) {
            munmap(mpCAMIODrvRegAddr, CAM_GPIO_CFG_RANGE);
            mpCAMIODrvRegAddr = NULL;
        }
#endif
        // munmap seninf reg       
        if ( 0 != mpSeninfHwRegAddr ) {
            munmap(mpSeninfHwRegAddr, SENINF_BASE_RANGE);
            mpSeninfHwRegAddr = NULL;
        }
        // uninit isp
        mpIspHwRegAddr = NULL;        
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
int SeninfDrv::waitSeninf1Irq(int mode)
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
int SeninfDrv::setTg1PhaseCounter(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;    

    LOG_MSG("[setTg1PhaseCounter] pcEn(%d) clkPol(%d)\n",pcEn,clkPol);
#if (!FPGA)  // Jason TODO remove
    // Enable Camera PLL first // Jason TODO PLL
    ISP_PLL_SEL_STRUCT pllCtrl;

    if (mclkSel == CAM_PLL_48_GROUP) 
        pllCtrl.MclkSel = MCLK_USING_UNIV_48M;  // 48MHz
    else if (mclkSel == CAM_PLL_52_GROUP) 
        pllCtrl.MclkSel = MCLK_USING_UNIV_208M; // 208MHz

    ioctl(mfd, ISP_IOC_PLL_SEL_IRQ, &pllCtrl);
#endif    

#if (FPGA)  // Jason TODO remove
    // Jason TODO tmp, use 12M and not to 6Mhz
    clkCnt = 0; 
#endif
    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt         

    //Seninf Top pclk clear gating
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN, 1);
    SENINF_WRITE_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN, 1);
#if (!FPGA)  // Jason TODO remove
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKRS, clkRiseEdge);
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL, clkFallEdge);
    //SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = clkCnt - 1;
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT, clkCnt);
#else
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKRS, 0);
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL, 1);
    //SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = clkCnt - 1;
    // 12Mhz to 6Mhz
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT, 1);
    // Jason TODO tmp, use 12M and not to 6Mhz
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT, clkCnt);
#endif
    //TODO:remove later
    //SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = 0;  //FPGA
    //SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL) = 0;	//FPGA

    //SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL) = clkCnt >> 1;//fpga
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKFL_POL, (clkCnt & 0x1) ? 0 : 1);

    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKPOL, clkPol);
    // mclkSel, 0: 122.88MHz, (others: Camera PLL) 1: 120.3MHz, 2: 52MHz
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, TGCLK_SEL, 1);//force PLL due to ISP engine clock dynamic spread
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN, 1);//FPGA experiment
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, PCEN, pcEn);//FPGA experiment
    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, PAD_PCLK_INV, padPclkInv);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN, 1);
    // Wait 1ms for PLL stable
    usleep(1000);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::setTg1GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1GrabRange] \n");
    tg1GrabWidth = pixelEnd - pixelStart;
    tg1GrabHeight = lineEnd - lineStart;

    // TG Grab Win Setting
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_E, pixelEnd);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_S, pixelStart);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_E, lineEnd);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_S, lineStart);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::setTg1SensorModeCfg(
    unsigned long hsPol, unsigned long vsPol
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1SensorModeCfg] \n");

    // Sensor Mode Config
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, hsPol);
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, vsPol);

    //SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, VS_TYPE, vsPol);//VS_TYPE = 0 for 4T:vsPol(0)
    //if(SEN_CSI2_SETTING)
    //    SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_VSYNC_TYPE, !vsPol);//VSYNC_TYPE = 1 for 4T:vsPol(0)
       
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN, 1);
    ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOT_MODE, 1);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::setTg1InputCfg(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel,
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb
)
{
    int ret = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("[setTg1InputCfg] \n");

    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_EN, 0x1);
    SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL, padSel);
    //SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, inSrcTypeSel);
    if(SEN_CSI2_ENABLE && (inSrcTypeSel==MIPI_SENSOR))    
    {
        LOG_MSG("inSrcTypeSel = % d \n",0);       
        SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, 0);
    }
    else
    {
        LOG_MSG("inSrcTypeSel = % d \n",inSrcTypeSel);
        SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, inSrcTypeSel);
    }
    
    ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, SEN_IN_LSB, 0x0);//no matter what kind of format, set 0
    //ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TG1_FMT_CLR, 0x7);
    //ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TG1_FMT_SET, inDataType);
    ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, TG1_FMT, inDataType);


	if (MIPI_SENSOR == inSrcTypeSel) {
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC, 0x0);
		if (JPEG_FMT == inDataType) {
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x18);//0x1B;
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1E);//0x1F;
			SENINF_WRITE_BITS(pSeninf, SENINF1_SPARE, SENINF_FIFO_FULL_SEL, 0x1);
			SENINF_WRITE_BITS(pSeninf, SENINF1_SPARE, SENINF_VCNT_SEL, 0x1);
			SENINF_WRITE_BITS(pSeninf, SENINF1_SPARE, SENINF_CRC_SEL, 0x0);//0x2);
		}
		else {
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x1B);
			SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1F);
		}
	}
    else if (SERIAL_SENSOR == inSrcTypeSel) { // Jason TODO Serial       
	}
	else {
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC, 0x1);
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_FLUSH_EN, 0x1B);
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, FIFO_PUSH_EN, 0x1F);
	}

	//One-pixel mode
	if ( JPEG_FMT != inDataType) {
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_PIX_SEL, 0);
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS, 0);
		//ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TWO_PIX_CLR, 1);
		//ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TWO_PIX_SET, 0);
		ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN, 0);

                LOG_DBG("[JPEG_SENSOR]inDataType format(%d) is YUV\n", inDataType);
	}
	else {
		SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_PIX_SEL, 1);
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS, 1);
		//ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_CLR, TWO_PIX_CLR, 1);
		//ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL_SET, TWO_PIX_SET, 1);
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, IMGO_FORMAT, 0x2);
                ISP_WRITE_BITS(pisp, CAM_CTL_FMT_SEL, TG1_FMT, 0x7);
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC, 0x0);
		ISP_WRITE_BITS(pisp, CAM_TG_SEN_MODE, SOT_CLR_MODE, 1);        
                ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, JPG_LINEND_EN, 1);		
		ISP_WRITE_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN, 1);
                
                LOG_DBG("[JPEG_SENSOR]inDataType format(%d) is JPG\n", inDataType);
	}


	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_IRQ_SW_RST, 0x1);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_MUX_SW_RST, 0x0);
	SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_IRQ_SW_RST, 0x0);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::setTg1ViewFinderMode(
    unsigned long spMode, unsigned long spDelay
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1ViewFinderMode] \n");
    //
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SPDELAY_MODE, 1);
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SINGLE_MODE, spMode);
    ISP_WRITE_BITS(pisp, CAM_TG_VF_CON, SP_DELAY, spDelay);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::sendCommand(int cmd, int arg1, int arg2, int arg3)
{
    int ret = 0;

    LOG_MSG("[sendCommand] cmd: 0x%x \n", cmd);
    switch (cmd) {
    case CMD_SET_DEVICE:
        mDevice = arg1;
        break;
        
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
unsigned long SeninfDrv::readReg(unsigned long addr)
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
int SeninfDrv::writeReg(unsigned long addr, unsigned long val)
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
int SeninfDrv::readRegs(reg_t *pregs, int count)
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
int SeninfDrv::writeRegs(reg_t *pregs, int count)
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
int SeninfDrv::holdReg(bool isHold)
{
    int ret;
    int hold = isHold;

    //LOG_MSG("[holdReg]");

    ret = ioctl(mfd, ISP_IOC_HOLD_REG, &hold);
    if (ret < 0) {
        LOG_ERR("ISP_IOC_HOLD_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::dumpReg()
{
    int ret;

    LOG_MSG("[dumpReg] \n");

    ret = ioctl(mfd, ISP_IOC_DUMP_REG, NULL);
    if (ret < 0) {
        LOG_ERR("ISP_IOC_DUMP_REG err \n");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::initTg1CSI2(bool csi2_en)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int ret = 0;
    unsigned int temp = 0;

    LOG_MSG("[initCSI2]:enable = %d\n", (int) csi2_en);

    //--- add for efuse ---//
    unsigned long* efuseAddr = (unsigned long *) mmap(0, 0x200, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, 0x10009000);
    if (efuseAddr == MAP_FAILED)
    {
        LOG_ERR("efuse mmap err, %d, %s \n", errno, strerror(errno));
        return -1;
    }

    //unsigned int mipiVar = *(mpCSI2RxAnalogRegAddr + (0x24/4));
    unsigned int efuseVar = *(efuseAddr + (0x180/4));        
    unsigned long* mipiVar = mpCSI2RxAnalogRegAddr + (0x24/4);
    unsigned int tmpVar;
    LOG_MSG("efuseVar = 0x%08x", efuseVar);    
    LOG_MSG("MIPI_RX_ANA24 Var original = 0x%08x", *(mipiVar));    
    //MIPI_RX_ANA24[15:12] = MIPI_RX_ANA24[15:12] + Offset180 [7:4]
    if(efuseVar&0x000000F0)
    {   
        tmpVar = 0x8 + ((efuseVar&0x000000F0)>>4);
        *(mipiVar) &= 0xFFFF0FFF;
        *(mipiVar) |= ((tmpVar&0x0000000F)<<12);
    }
    //MIPI_RX_ANA24[11:8] = MIPI_RX_ANA24[11:8] + Offset180 [3:0]
    if(efuseVar&0x0000000F)
    {   
        tmpVar = 0x8 + ((efuseVar&0x0000000F));
        *(mipiVar) &= 0xFFFFF0FF;
        *(mipiVar) |= ((tmpVar&0x0000000F)<<8);
    }
    LOG_MSG("MIPI_RX_ANA24 Var after efuse = 0x%08x", *(mipiVar));
    
    if ( 0 != efuseAddr )
    {
        munmap(efuseAddr, 0x200);
        efuseAddr = NULL;
    }
    //--- add for efuse ---//

#if (!FPGA)  // Jason TODO remove
	if(csi2_en == 0) 
    {
        // disable mipi BG
        *(mpCSI2RxAnalogRegAddr + (0x24/4)) &= 0xFFFFFFFE;//RG_CSI_BG_CORE_EN

        // Jason TODO GPIO api
        // disable mipi gpio pin   
        //*(mpGpioHwRegAddr + (0x1C0/4)) |= 0xFFE0;//GPI*_IES = 1 for Parallel CAM
        //*(mpGpioHwRegAddr + (0x1D0/4)) |= 0x0001;//GPI*_IES = 1 for Parallel CAM

        // disable mipi input select
        *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFE7;//main & sub clock lane input select hi-Z
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFE7;//main & sub data lane 0 input select hi-Z
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFE7;//main & sub data lane 1 input select hi-Z
        
	}
	else
    {   // enable mipi input select              
        if(mDevice & SENSOR_DEV_MAIN)
        {        
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) |= 0x00000008;//main clock lane input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) |= 0x00000008;//main data lane 0 input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) |= 0x00000008;//main data lane 1 input select mipi            
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFEF;//sub clock lane input select mipi disable     
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFEF;//sub data lane 0 input select mipi disable    
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFEF;//sub data lane 1 input select mipi disable            
        }
        else if(mDevice & SENSOR_DEV_SUB)
        {
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) |= 0x00000010;//sub clock lane input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) |= 0x00000010;//sub data lane 0 input select mipi
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) |= 0x00000010;//sub data lane 1 input select mipi            
            *(mpCSI2RxAnalogRegAddr + (0x00/4)) &= 0xFFFFFFF7;//main clock lane input select mipi disable     
            *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFFFFFF7;//main data lane 0 input select mipi disable    
            *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFFFFFF7;//main data lane 1 input select mipi disable  
        }

        // enable mipi BG
        *(mpCSI2RxAnalogRegAddr + (0x24/4)) |= 0x00000001;//RG_CSI_BG_CORE_EN
        usleep(30);
        *(mpCSI2RxAnalogRegAddr + (0x20/4)) |= 0x00000001;//RG_CSI0_LDO_CORE_EN
        usleep(1);
        *(mpCSI2RxAnalogRegAddr + (0x00/4)) |= 0x00000001;//RG_CSI0_LNRC_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x04/4)) |= 0x00000001;//RG_CSI0_LNRD0_LDO_OUT_EN
        *(mpCSI2RxAnalogRegAddr + (0x08/4)) |= 0x00000001;//RG_CSI0_LNRD1_LDO_OUT_EN

       // CSI Offset calibration 
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LNC_HSRXDB_EN, 1);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN0_HSRXDB_EN, 1);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN1_HSRXDB_EN, 1);
       *(mpCSI2RxConfigRegAddr + (0x38/4)) |= 0x00000004;//MIPI_RX_HW_CAL_START
       LOG_MSG("[initCSI2]:CSI0 calibration do !\n");
       //usleep(1000);              
       //while( !(*(mpCSI2RxConfigRegAddr + (0x44/4)) & 0x10101) ){}// polling LNRC, LNRD0, LNRD1
       usleep(1000);
       if(!(*(mpCSI2RxConfigRegAddr + (0x44/4)) & 0x10101))// checking LNRC, LNRD0, LNRD1
       {
            LOG_ERR("[initCSI2]: CIS2Polling calibration failed: 0x%08x", *(mpCSI2RxConfigRegAddr + (0x44/4)));
            //ret = -1;
       }
       LOG_MSG("[initCSI2]:CSI0 calibration end !\n");
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LNC_HSRXDB_EN, 0);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN0_HSRXDB_EN, 0);
       SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_DBG, LN1_HSRXDB_EN, 0);

       *(mpCSI2RxAnalogRegAddr + (0x20/4)) &= 0xFFFFFFDF;//bit 5:RG_CSI0_4XCLK_INVERT = 0
       *(mpCSI2RxAnalogRegAddr + (0x04/4)) &= 0xFFBFFFFF;//bit 22:RG_CSI0_LNRD0_HSRX_BYPASS_SYNC = 0
       *(mpCSI2RxAnalogRegAddr + (0x08/4)) &= 0xFFBFFFFF;//bit 22:RG_CSI0_LNRD1_HSRX_BYPASS_SYNC = 0
       //if(SEN_CSI2_SETTING)
       if(SEN_CSI2_ENABLE)
       {
           *(mpCSI2RxAnalogRegAddr + (0x20/4)) &= 0xFFFFFFBF;//bit 6:RG_CSI0_4XCLK_DISABLE = 0
           SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_LNMUX,0xE4);         
       }
       else
       {
           *(mpCSI2RxAnalogRegAddr + (0x20/4)) |= 0x00000040;//bit 6:RG_CSI0_4XCLK_DISABLE = 1
       }
       *(mpCSI2RxAnalogRegAddr + (0x20/4)) |= 0x00000002;//bit 1:RG_CSI0_LNRD_HSRX_BCLK_INVERT = 1       

	}
#endif
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int SETTLE_TEST = 0x00001A00;
bool HW_MODE = 0;
int SeninfDrv::setTg1CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlane_num, unsigned long csi2_en,
    unsigned long dataheaderOrder, unsigned long dataFlow
)
{
    int ret = 0,temp = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_WRN("[configTg1CSI2]:DataTermDelay:%d SettleDelay:%d ClkTermDelay:%d VsyncType:%d dlane_num:%d CSI2 enable:%d HeaderOrder:%d DataFlow:%d\n",
      	(int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlane_num, (int) csi2_en, (int)dataheaderOrder, (int)dataFlow);

    if(csi2_en == 1) 
    {  
        if(0 == SEN_CSI2_SETTING) 
        {
            // disable NCSI2 first     
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_LANE_EN, 0);              
            
            // add src select earlier, for countinuous MIPI clk issue
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, 0x8);
            
	        #if (FPGA)
            //temp = (dataSettleDelay&0xFF)<<8;
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);        
            //SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_LNRD_TIMING, SETTLE_PARAMETER, (dataSettleDelay&0xFF));
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, 0x00000300);       
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, ED_SEL, 1);
            #else
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, 0x00003000);//set 0x30 for settle delay               
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_SPARE0, 0x80000000); 
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, ED_SEL, dataheaderOrder); 
            #endif
            
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, 0); //mipi hsync must be zero.
            //VS_TYPE = 0 for 4T:vsPol(0)
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, VS_TYPE, 0);
            
            //Use sw settle mode
            if(HW_MODE)        
            {
               SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, HSRX_DET_EN, 1);  
            }
            else
            {
               SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, HSRX_DET_EN, 0);  
               SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, SETTLE_TEST);        
            }
            
            // enable NCSI2
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_LANE_EN, csi2_en);   
            switch(dlane_num)
            {
                case 0://SENSOR_MIPI_1_LANE
                    SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE0_EN, 1);               
                    break;
                case 1://SENSOR_MIPI_2_LANE
                    SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE0_EN, 1);               
                    SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE1_EN, 1);               
                    break;
                default:
                    break;                
            }                            
            // turn on all interrupt
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_EN, 0x80007FFF);
        }
        else
        {
            //if(SEN_CSI2_SETTING)
            // disable CSI2 first     
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, 0);   

            // add src select earlier, for countinuous MIPI clk issue
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL, 0x0);

            temp = (dataSettleDelay&0xFF)<<16;
            //SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DELAY, 0x00300000);//set 0x30 for settle delay             
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DELAY, temp);//set 0x30 for settle delay             
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_ED_SEL, dataheaderOrder); 

            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_HSYNC_POL, 0); //mipi hsync must be zero.
            //VSYNC_TYPE = 1 for 4T:vsPol(0)
            SENINF_WRITE_BITS(pSeninf, SENINF1_CTRL, SENINF_VSYNC_POL, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_VSYNC_TYPE, 1);

            // enable CSI2            
            if(SEN_CSI2_ENABLE)
            {
                SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, csi2_en);   
                switch(dlane_num)
                {                
                    case 1://SENSOR_MIPI_2_LANE
                        SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, DLANE1_EN, 1);                   
                        break;
                        case 0://SENSOR_MIPI_1_LANE                           
                    default:
                        break;                
                }           
             }         
        }
    }
    else 
    {   
        if(SEN_CSI2_SETTING)
        {
            // disable CSI2        
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, CSI2_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, DLANE1_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_CSI2_CTRL, DLANE2_EN, 0);
        }
        else
        {
            // disable NCSI2
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_LANE_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE0_EN, 0);
            SENINF_WRITE_BITS(pSeninf, SENINF1_NCSI2_CTL, DATA_LANE1_EN, 0);
        }
    }

    LOG_WRN("end of setTg1CSI2\n");
    //int j;
    //for(j = 0x600; j < 0x648; j+=4)
    //    LOG_WRN("[%x]=0x%08x", j, *(mpSeninfHwRegAddr + (j/4)));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::initTg1Serial(bool serial_en)            // Jason TODO Serial
{
    int ret = 0;    
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SCAM1_CON, Enable, serial_en);

    return ret;
}
 
/*******************************************************************************
*
********************************************************************************/
// Jason TODO Serial
int SeninfDrv::setTg1Serial(
    unsigned long clk_inv, unsigned long width, unsigned long height,
    unsigned long conti_mode, unsigned long csd_num)        
{
    int ret = 0;    
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	//set CMPCLK(gpio_61) mode 2		 
	ISP_GPIO_SEL_STRUCT gpioCtrl;
	gpioCtrl.Pin = GPIO61;			// CMPCLK
	gpioCtrl.Mode = GPIO_MODE_02;
	ioctl(mfd, ISP_IOC_GPIO_SEL_IRQ, &gpioCtrl);
	
    SENINF_WRITE_BITS(pSeninf, SCAM1_SIZE, WIDTH, width);
    SENINF_WRITE_BITS(pSeninf, SCAM1_SIZE, HEIGHT, height);    
    
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Clock_inverse, clk_inv);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Continuous_mode, conti_mode);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, CSD_NUM, csd_num);  // 0(1-lane);1(2-line);2(4-lane)
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG, Cycle, 4);
    SENINF_WRITE_BITS(pSeninf, SCAM1_CFG2, DIS_GATED_CLK, 1);    
    
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::setTg1IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    /*
    [1: 0]: CMPCLK, CMMCLK , CMDAT0~3
    00: 2mA
    01: 4mA
    10: 6mA
    11: 8mA
    [3: 2]: CMPDN, CMRST
    [5: 4]: CMPDN2, CMRST2
    */

#if (!FPGA)  // Jason TODO remove
    unsigned long* pCAMIODrvCtl; 
    if(mpCAMIODrvRegAddr != NULL) 
    {        
        pCAMIODrvCtl = mpCAMIODrvRegAddr + (0x060/4);
        *(pCAMIODrvCtl) &= 0xFFFFFFCF;
        switch(ioDrivingCurrent)
        {
            case 0x00:  //2mA
                *(pCAMIODrvCtl) |= 0x00; 
                break;
            case 0x20:  //4mA
                *(pCAMIODrvCtl) |= 0x10; 
                break;
            case 0x40:  //6mA
                *(pCAMIODrvCtl) |= 0x20;                 
                break;
            case 0x60:  //8mA                
                *(pCAMIODrvCtl) |= 0x30;                 
                break;
            default:    //4mA
                *(pCAMIODrvCtl) |= 0x10; 
                break;
    }
    }
    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(pCAMIODrvCtl)));
#endif

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrv::setTg1MCLKEn(bool isEn)
{
    int ret = 0;

    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN, isEn);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int SeninfDrv::getRegAddr(void)
{
    LOG_MSG("mpSeninfHwRegAddr(0x%08X)",(unsigned int)mpSeninfHwRegAddr);
    //
    if(mpSeninfHwRegAddr != NULL)
    {
        return (unsigned int)mpSeninfHwRegAddr;
    }
    else
    {
        return 0;
    }
}

#if 0 // Jason TODO remove
//
/*******************************************************************************
*
********************************************************************************/

int SeninfDrv::setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount,
			unsigned long startLine, unsigned long startPixel, unsigned long  flashPol)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN, 0x0);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_POL, flashPol);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_END_FRM, endFrame);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_STARTPNT, startPoint);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT_NO, unitCount);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT, lineUnit);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_PXL, startPixel);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_LINE, startLine);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN, 0x1);

	return ret;

}


int SeninfDrv::setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN, 0x0);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_CONT_FRM, contiFrm);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_START_FRM, startFrame);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_STARTPNT, 0x0);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_TRIG_SRC, 0x0);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT_NO, unitCount);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT, lineUnit);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_PXL, startPixel);
	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_LINE, startLine);

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN, 0x1);

	return ret;
}

int SeninfDrv::setFlashEn(bool flashEn)
{
	int ret = 0;
	isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

	ISP_WRITE_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_EN, flashEn);

	return ret;

}



int SeninfDrv::setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	if ((hsStart > 4095) || (hsEnd > 4095))
	{
		LOG_ERR("CCIR656 HSTART or HEND value err \n");
		ret = -1;
	}

	SENINF_WRITE_BITS(pSeninf, CCIR656_CTL, CCIR656_VS_POL, vsPol);
	SENINF_WRITE_BITS(pSeninf, CCIR656_CTL, CCIR656_HS_POL, hsPol);
	SENINF_WRITE_BITS(pSeninf, CCIR656_H, CCIR656_HS_END, hsEnd);
	SENINF_WRITE_BITS(pSeninf, CCIR656_H, CCIR656_HS_START, hsStart);

	return ret;
}
#endif

#if (FPGA)  // Jason TODO remove
int SeninfDrv::setPdnRst(int camera, bool on)
{
    int tgsel;
    int ret = 0;
    //MINT32 imgsys_cg_clr0 = 0x15000000;    
    //MINT32 gpio_base_addr = 0x10001000;

    // mmap seninf clear gating reg
    mpCAMMMSYSRegAddr = (unsigned long *) mmap(0, CAM_MMSYS_CONFIG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, CAM_MMSYS_CONFIG_BASE);
    if (mpCAMMMSYSRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(3), %d, %s \n", errno, strerror(errno));
        return -7;
    }

    *(mpCAMMMSYSRegAddr + (0x8/4)) |= 0x03FF; //clear gate

#if (!JASON_TMP) 
    /*
    // mmap gpio reg
    mpGpioHwRegAddr = (unsigned long *) mmap(0, CAM_GPIO_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, CAM_GPIO_BASE);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -1;
    }

    // Jason TODO GPIO api
    // set RST/PDN/RST1/PDN1(gpio_142, 143, 89, 90)
    *(mpGpioHwRegAddr + (0xE84/4)) |= 0x000F;  
    *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFF0;  
    switch(camera) {
        case 1:
            tgsel = 0;
            break;
        case 2:
            tgsel = 1;
            break;

        default:
            tgsel = 0;
            break;
    }
    LOG_MSG("camera = %d tgsel = %d, On = %d \n",camera, tgsel,on);

    // Jason TODO GPIO api
    // set RST/PDN/RST1/PDN1(gpio_142, 143, 89, 90)
    if (0 == tgsel){//Tg1 
        if(1 == on){
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFFD;             
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0001; 
  
        }
        else {
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFFE; 
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0002;             
        }
    }
    else {
        if(1 == on){
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFF7; 
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0004; 
  
        }
        else {
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFFB; 
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0008;             
        }

    }
    */
#else 
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    if(1 == on)
    {    
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_PWRDN, 0);
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_RST, 0);
        
        //#if defined(OV5642_YUV)
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_PWRDN, 0);
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_RST, 1);
        //#if defined(S5K4ECGX_MIPI_YUV)
        /*
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_PWRDN, 1);
        {
            int i = 100;
            while(i--); 
        }
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_RST, 1);
        */
        //#else
        //#error
        //#endif
    }
    else
    {
        //#if defined(OV5642_YUV)
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_PWRDN, 1);
        SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_RST, 0);    
        //#if defined(S5K4ECGX_MIPI_YUV)
        //SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_PWRDN, 0);
        //SENINF_WRITE_BITS(pSeninf, SENINF_TG1_PH_CNT, EXT_RST, 0);
        //#else
        //#error
        //#endif
    }    
    LOG_WRN("SENINF_TG1_PH_CNT: 0x%08x", SENINF_READ_REG(pSeninf, SENINF_TG1_PH_CNT));
#endif       
    //seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    //LOG_WRN("SENINF_TG1_PH_CNT: 0x%08x", SENINF_READ_REG(pSeninf, SENINF_TG1_PH_CNT));

    return ret;

}
#endif


int SeninfDrv::checkSeninf1Input()
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

#if 0 // Jason TODO remove
int SeninfDrv::setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	SENINF_WRITE_BITS(pSeninf, N3D_CTL, N3D_EN, n3dEn);
	SENINF_WRITE_BITS(pSeninf, N3D_CTL, I2C1_EN, i2c1En);
	SENINF_WRITE_BITS(pSeninf, N3D_CTL, I2C2_EN, i2c2En);
	SENINF_WRITE_BITS(pSeninf, N3D_CTL, MODE, n3dMode);

	return ret;
}


int SeninfDrv::setN3DI2CPos(unsigned long n3dPos)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	SENINF_WRITE_BITS(pSeninf, N3D_POS, N3D_POS, n3dPos);

	return ret;
}


int SeninfDrv::setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn)
{
    int ret = 0;
	seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

	SENINF_WRITE_BITS(pSeninf, N3D_TRIG, I2CA_TRIG, i2c1TrigOn);
	SENINF_WRITE_BITS(pSeninf, N3D_TRIG, I2CB_TRIG, i2c2TrigOn);

	return ret;

}
#endif
