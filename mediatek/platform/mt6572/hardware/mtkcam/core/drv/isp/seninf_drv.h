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
#ifndef _SENINF_DRV_H_
#define _SENINF_DRV_H_

#include <utils/Errors.h>
#include <cutils/log.h>

#include "isp_drv.h"

using namespace android;

/*******************************************************************************
*
********************************************************************************/
#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    ALOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    ALOGD("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    ALOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#else
#include "uvvf.h"
#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_ERRMSG("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)    
#define LOG_WRN(fmt, arg...)    
#define LOG_ERR(fmt, arg...)    
#endif    
#endif

/*******************************************************************************
*
********************************************************************************/
typedef enum {
	PAD_10BIT		= 0x0,
	PAD_8BIT_7_0	= 0x3,
	PAD_8BIT_9_2	= 0x4,
}PAD2CAM_DATA_ENUM;

typedef enum { //0:CSI2, 3: parallel, 8:nCSI2        
	TEST_MODEL		= 0x1,
	CCIR656			= 0x2,
	PARALLEL_SENSOR	= 0x3,
	SERIAL_SENSOR	= 0x4,
	HD_TV			= 0x5,
	EXT_CSI2_OUT1	= 0x6,
	EXT_CSI2_OUT2	= 0x7,
	MIPI_SENSOR     = 0x8
}SENINF_SOURCE_ENUM;

typedef enum {
	TG_12BIT	= 0x0,
	TG_10BIT	= 0x1,
	TG_8BIT		= 0x2
}SENSOR_DATA_BITS_ENUM;

typedef enum {
	RAW_8BIT_FMT		= 0x0,
	RAW_10BIT_FMT		= 0x1,
	RAW_12BIT_FMT		= 0x2,
	YUV422_FMT			= 0x3,
	CCIR656_FMT			= 0x4,
	RGB565_MIPI_FMT		= 0x5,
	RGB888_MIPI_FMT		= 0x6,
	JPEG_FMT			= 0x7
}TG_FORMAT_ENUM;

typedef enum {
	ACTIVE_HIGH		= 0x0,
	ACTIVE_LOW		= 0x1,		
}CCIR656_OUTPUT_POLARITY_ENUM;

typedef enum {
	IMMIDIANT_TRIGGER	= 0x0,
	REFERENCE_VS1		= 0x1,
	I2C1_BEFORE_I2C2	= 0x2,
	I2C2_BEFORE_I2C1	= 0x3
}N3D_I2C_TRIGGER_MODE_ENUM;

typedef enum drvSeninfCmd_s {
	CMD_SET_DEVICE				= 0x1000,
    CMD_GET_SENINF_ADDR         = 0x2001,
    CMD_DRV_SENINF_MAX             = 0xFFFF
} drvSeninfCmd_e;


#define CAM_PLL_48_GROUP        (1)
#define CAM_PLL_52_GROUP        (2)

/*******************************************************************************
*
********************************************************************************/
class SeninfDrv {
public:
    //
    static SeninfDrv* createInstance();
    static SeninfDrv* getInstance();
    virtual void   destroyInstance();

public:
    SeninfDrv();
    virtual ~SeninfDrv();
    
public:
    virtual int init();
    virtual int uninit();
    virtual int waitSeninf1Irq(int mode);
    typedef struct reg_s {
        unsigned long addr;
        unsigned long val;
    } reg_t;
    virtual unsigned long readReg(unsigned long addr);
    virtual int writeReg(unsigned long addr, unsigned long val);
    virtual int readRegs(reg_t *pregs, int count);
    virtual int writeRegs(reg_t *pregs, int count);
    virtual int holdReg(bool isHold);
    virtual int dumpReg();
    //
    virtual int setTg1PhaseCounter(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv);
    virtual int setTg1GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd);
    virtual int setTg1SensorModeCfg(unsigned long hsPol, unsigned long vsPol);
    virtual int setTg1ViewFinderMode(unsigned long spMode, unsigned long spDelay);
    virtual int setTg1InputCfg(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel,
		TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb);
    virtual int sendCommand(int cmd, int arg1 = 0, int arg2 = 0, int arg3 = 0);
    virtual int initTg1CSI2(bool csi2_en);
    virtual int setTg1CSI2(unsigned long dataTermDelay, 
                        unsigned long dataSettleDelay, 
                        unsigned long clkTermDelay, 
                        unsigned long vsyncType, 
                        unsigned long dlane_num, 
                        unsigned long csi2_en,
                        unsigned long dataheaderOrder,
                        unsigned long dataFlow);
    virtual int initTg1Serial(bool serial_en);			// Jason TODO Serial
    virtual int setTg1Serial(unsigned long clk_inv, unsigned long width, unsigned long height,
	    unsigned long conti_mode, unsigned long csd_num);	// Jason TODO Serial
    virtual int setTg1IODrivingCurrent(unsigned long ioDrivingCurrent);
    virtual int setTg1MCLKEn(bool isEn);
    virtual unsigned int getRegAddr(void);
    //
    /*
    virtual int setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel, unsigned long  flashPol) = 0;
    virtual int setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel) = 0;
    virtual int setFlashEn(bool flashEn) = 0;
    virtual int setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd) = 0;
    virtual int setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode) = 0;
    virtual int setN3DI2CPos(unsigned long n3dPos) = 0;
    virtual int setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn) = 0;
    */
    // Jason TODO remove
    //virtual int setPdnRst(int camera, bool on);
	virtual int checkSeninf1Input();
	
private:	
    IspDrv *m_pIspDrv;
    volatile int mUsers;
    mutable Mutex mLock;
    int mfd;
	
    unsigned long *mpIspHwRegAddr;
    unsigned long *mpSeninfHwRegAddr;
	
    unsigned long *mpPLLHwRegAddr;				// pll base
    unsigned long *mpIPllCon0RegAddr;			// ->	mpPLLHwRegAddr + (0x158 /4)
    unsigned long *mpGpioHwRegAddr;				// gpio base
    unsigned long *mpCAMIODrvRegAddr;			// ->	mpGpioHwRegAddr + (0x5B0 / 4);
	unsigned long *mpCAMMMSYSRegAddr;			// mmsys base    
    unsigned long *mpCSI2RxConfigRegAddr;		// mipi rx config base
	unsigned long *mpCSI2RxAnalogRegAddr;		// mipi config base
    unsigned long *mpCSI2RxAnalogRegStartAddr;	// ->	mpCSI2RxAnalogRegStartAddr + (0x800/4);	
    //unsigned long *mpCAMAPConRegAddr;	 
    //unsigned long *mpSENINFClearGateRegAddr;
    //unsigned long *mpSENINFSetGateRegAddr;	
	//unsigned long *mpTg2IOPinMuxCfgAddr;
    unsigned long tg1GrabWidth;
    unsigned long tg1GrabHeight;
	
    MINT32 mDevice;
};

#endif // _SENINF_DRV_H_