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
#ifndef _ISP_DRV_H_
#define _ISP_DRV_H_

#include <vector>
#include <list>
using namespace std;

//#include <asm/arch/mt6589_sync_write.h> // only 6589 For dsb() in isp_drv.h.
#if (PLATFORM_VERSION_MAJOR == 2)
#include "utils/threads.h"  // For android::Mutex.
#else
#include "utils/Mutex.h"    // For android::Mutex.
#endif
#include "isp_reg.h"

using namespace android;

typedef unsigned long long  MUINT64;
typedef unsigned int        MUINT32;
typedef signed int          MINT32;
typedef void                MVOID;
typedef int                 MBOOL;
#ifndef MTRUE
    #define MTRUE           1
#endif
#ifndef MFALSE
    #define MFALSE          0
#endif

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
// New macro for read ISP registers.
#define ISP_READ_BITS(RegBase, RegName, FieldName)  	(RegBase->RegName.Bits.FieldName)
#define ISP_READ_REG(RegBase, RegName)              	(RegBase->RegName.Raw)
// for write ISP registers except enable register
#define ISP_WRITE_BITS(RegBase, RegName, FieldName, Value)              \
	    do {                                                           	\        
	        (RegBase->RegName.Bits.FieldName) = (Value);               	\
	    } while (0)
#define ISP_WRITE_REG(RegBase, RegName, Value)                          \
	    do {                                                           	\
	        (RegBase->RegName.Raw) = (Value);                          	\
	    } while (0)
    
// for write CAM_CTL_EN1/CAM_CTL_EN2/CAM_DMA_EN register.
#define ISP_WRITE_ENABLE_BITS(RegBase, RegName, FieldName, Value)   	\
		do {															\		 
			pthread_mutex_lock(&IspRegMutex);							\
			(RegBase->RegName.Bits.FieldName) = (Value);				\
			pthread_mutex_unlock(&IspRegMutex); 						\
		} while (0)
	
#define ISP_WRITE_ENABLE_REG(RegBase, RegName, Value)   				\
		do {															\
			pthread_mutex_lock(&IspRegMutex);							\
			(RegBase->RegName.Raw) = (Value);							\
			pthread_mutex_unlock(&IspRegMutex); 						\
		} while (0)
// ------------------------------------------------------------------------
#define ISP_DRV_INT_IMGO_DROP_ST       	((unsigned int)1 << 19)
#define ISP_DRV_INT_IMGO_OVERR_ST     	((unsigned int)1 << 17)
#define ISP_DRV_INT_IMGO_ERR_ST        	((unsigned int)1 << 16)
#define ISP_DRV_INT_PASS1_TG1_DON_ST   	((unsigned int)1 << 10)
#define ISP_DRV_INT_TG1_SOF_ST   		((unsigned int)1 << 7)
#define ISP_DRV_INT_TG1_DROP_ST   		((unsigned int)1 << 6)
#define ISP_DRV_INT_TG1_GBERR_ST   		((unsigned int)1 << 5)
#define ISP_DRV_INT_TG1_ERR_ST   		((unsigned int)1 << 4)
#define ISP_DRV_INT_EXPDON1_ST   		((unsigned int)1 << 3)
#define ISP_DRV_INT_TG1_INT2_ST   		((unsigned int)1 << 2)
#define ISP_DRV_INT_TG1_INT1_ST   		((unsigned int)1 << 1)
#define ISP_DRV_INT_VS1_ST   			((unsigned int)1 << 0)
// ------------------------------------------------------------------------
#define ISP_DRV_CAM_MIPI_API        	(0)
//
#define ISP_DRV_DEV_NAME            	"/dev/camera-isp"
#define ISP_DRV_DEV_NAME_MIPI       	"/proc/clkmgr/mipi_test"

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
#define CAM_INT_WAIT_TIMEOUT_MS 		(2000)	// 5000 -> 2000

typedef enum
{
    ISP_DRV_IRQ_CLEAR_NONE,
    ISP_DRV_IRQ_CLEAR_WAIT,
    ISP_DRV_IRQ_CLEAR_ALL
}ISP_DRV_IRQ_CLEAR_ENUM;
//
typedef enum
{
	ISP_DRV_IMG_FORMAT_YUV422,
	ISP_DRV_IMG_FORMAT_JPEG,
}ISP_DRV_IMG_FORMAT_ENUM;
//
typedef enum
{
    //No parameter set or get
    ISP_DRV_CMD_RESET_BUF       = 0x1001,
    ISP_DRV_CMD_CLEAR_INT_STATUS,           //cotta-- added for clean all interrupt 
    ISP_DRV_CMD_RESET_VD_COUNT,
    //Set by parameter
    ISP_DRV_CMD_SET_PIXEL_TYPE  = 0x2001,
    ISP_DRV_CMD_SET_KDBG_FLAG,
    ISP_DRV_CMD_SET_CAPTURE_DELAY,
    ISP_DRV_CMD_SET_VD_PROC,                //cotta-- added for force turn on VD signal
    ISP_DRV_CMD_SET_PQ_PARAM,
    //Get by parameter
    ISP_DRV_CMD_GET_ADDR        = 0x3001,
    ISP_DRV_CMD_GET_LINE_ID,
    ISP_DRV_CMD_GET_CAPTURE_DELAY,          //cotta-- added for get ISP register capture delay value
    ISP_DRV_CMD_GET_VD_COUNT,
    ISP_DRV_CMD_GET_PQ_PARAM,
    ISP_DRV_CMD_DRV_MAX         = 0xFFFF
}ISP_DRV_CMD_ENUM;
//
typedef struct
{
    ISP_DRV_IRQ_CLEAR_ENUM  clear;
    MUINT32                 status;
    MUINT32                 timeout;
}ISP_DRV_WAIT_IRQ_STRUCT;

typedef struct
{
    MUINT32     addr;
    MUINT32     data;
}ISP_DRV_REG_IO_STRUCT;
//
typedef enum
{
    ISP_DRV_BUF_EMPTY = 0,
    ISP_DRV_BUF_FILLED,
}ISP_DRV_BUF_STATUS;

typedef struct _ISP_DRV_BUF_STRUCT
{
    ISP_DRV_BUF_STATUS  		status;
    MUINT32         			base_vAddr;
    MUINT32         			base_pAddr;
    MUINT32         			size;
    MUINT32         			memID;
    MUINT64         			timeStampS;     // Time stamp
    MUINT32         			timeStampUs;
    MVOID           			*private_info;
    struct _ISP_DRV_BUF_STRUCT  *next;
}ISP_DRV_BUF_STRUCT;

typedef list<ISP_DRV_BUF_STRUCT> ISP_DRV_BUF_L;

typedef struct  
{
    MUINT32        	filledCnt; 	//  fill count
    ISP_DRV_BUF_L  	bufList;
}ISP_DRV_BUF_LIST;

typedef struct{
    ISP_DRV_BUF_L  *pBufList;
}ISP_DRV_FILLED_BUF_LIST;
//
typedef void (*pIspDrvCallback)(MBOOL);

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern pthread_mutex_t IspRegMutex; // Original IspRegMutex is defined in isp_drv.cpp.

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class IspDrv
{
protected:
	IspDrv();
	~IspDrv();   
public:
	// static function
    static IspDrv* 	createInstance(void);
	static IspDrv*	getInstance(void);
	
	// basic
    virtual void   	destroyInstance(void);        
    virtual MBOOL 	init(void);        
    virtual MBOOL 	uninit(void);        
    virtual MBOOL   waitIrq(ISP_DRV_WAIT_IRQ_STRUCT waitIrq);	
    virtual MBOOL   readIrq(MUINT32* irqStatus);
    virtual MBOOL   checkIrq(MUINT32 irqStatus);
    virtual MBOOL   clearIrq(MUINT32 irqStatus);
    virtual MBOOL   reset(void);
    virtual MBOOL   resetBuf(void);
	//
    virtual MUINT32 getRegAddr(void);
    virtual isp_reg_t* getRegAddrMap(void);		 
	virtual MBOOL 	readRegs(ISP_DRV_REG_IO_STRUCT* pRegIo, MUINT32 count);	
    virtual MUINT32 readReg(MUINT32 addr);
    virtual MBOOL 	writeRegs(ISP_DRV_REG_IO_STRUCT* pRegIo, MUINT32 count);    	
	virtual MBOOL	writeReg(MUINT32 addr,MUINT32 val); 	
    virtual MBOOL 	holdReg(MBOOL isHold);
    virtual MBOOL 	dumpReg(void);
	//
	virtual MINT32 	sendCommand(
        MINT32      cmd,
        MINT32      arg1 = 0,
        MINT32      arg2 = 0,
        MINT32      arg3 = 0);
    virtual void 	regCallback(pIspDrvCallback pFunc);

    virtual MBOOL 	setDevID(MINT32 devid);

	// cam & cdrz & wdma setting
	virtual MINT32 	setCamModule(
		MBOOL		isEn,
		MUINT32 	imgDataFormat,
		MUINT32 	swapY,
		MUINT32 	swapCbCr);
	virtual MINT32 	setCdrzCtrl(
		MBOOL		isEn,
		MUINT32     inputPixel, 
	    MUINT32     inputLine,     
	    MUINT32     outputPixel, 
	    MUINT32     outputLine);	
	virtual MINT32 	setImgoAddr(    
	    MUINT32     imgoAddr);
	virtual MINT32 	setImgoSize(
        MUINT32     imgoPixel,
        MUINT32     imgoLine);  
	/*	
	virtual MINT32	setImgoFBC(
    	MBOOL       isEn,
	    MUINT32     buffNum);
	virtual MINT32	setImgoBuffRead(void);
	*/	
    virtual MINT32 	control(MUINT32 isEn);  
	
	// enque/deque buff control
	virtual MBOOL waitBufReady(void);	// wait pass1 done irq for buff ready
	virtual MBOOL enqueueHwBuf(		// append new buffer to the end of hwBuf list
		ISP_DRV_BUF_STRUCT buff);				
	virtual MBOOL dequeueHwBuf(void);	//move filled Hw buffer to sw the end of list.
	virtual MBOOL dequeueSwBuf(		// delete all swBuf list after inform caller
		ISP_DRV_FILLED_BUF_LIST bufList);		
	virtual MUINT32 getCurrHwBuf(void);	//get 1st NOT filled HW buffer address	
	virtual MUINT32 getNextHwBuf(void);	//get 2nd NOT filled HW buffer address
	virtual MUINT32 freeSinglePhyBuf(	// free single physical buffer
		ISP_DRV_BUF_STRUCT buff); 				
	virtual MUINT32 freeAllPhyBuf(void);	//free all physical buffer
	
	// to isp kernel space function
private:
	virtual MBOOL 	rtBufCtrl(void *pBuf_ctrl);	
	#if defined(_use_kernel_ref_cnt_) 		// defined in kernel camera_isp.h   
	virtual MBOOL   kRefCntCtrl(ISP_REF_CNT_CTRL_STRUCT* pCtrl);
	#endif	

	
private:    
	mutable Mutex 		mLock;
    volatile MINT32 	mUsers;    
    MINT32 				mFd;
	#if ISP_DRV_CAM_MIPI_API
	MINT32				mFd1;
	#endif
    //
    MUINT32*        	mpIspDrvRegMap;
    MUINT32*			mpIspHwRegAddr;		
	// rtbc
	ISP_DRV_BUF_LIST 	mHwBufList;
    ISP_DRV_BUF_LIST 	mSwBufList;	
	mutable Mutex       mQueHwLock;
    mutable Mutex       mQueSwLock;
	MUINT32*			mpRTBufTbl;
	MUINT32 			mRTBufTblSize;
	//	
    pIspDrvCallback 	mpCallback;
};
//----------------------------------------------------------------------------
#endif  // _ISP_DRV_H_
