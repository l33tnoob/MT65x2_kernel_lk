//MTK_SWIP_PROJECT_START
#ifndef __MTK_TSF_H__
#define __MTK_TSF_H__

#include "MTKTsfType.h"
#include "MTKTsfErrCode.h"

typedef enum DRVTsfObject_s {
    DRV_TSF_OBJ_NONE = 0,
    DRV_TSF_OBJ_SW,
    DRV_TSF_OBJ_SW_NEON,    
    DRV_TSF_OBJ_UNKNOWN = 0xFF,
} DrvTsfObject_e;

typedef enum
{
	MTKTSF_STATE_STANDBY = 0,               // After create Obj or Reset
	MTKTSF_STATE_INIT,				// After init environment info
    MTKTSF_STATE_MAIN,				// After init tsf cb/cr gain
	MTKTSF_STATE_READY,			    // After tsf core optimization
	MTKTSF_STATE_ISP_DONE,			    // After tsf core optimization
	MTKTSF_STATE_MAX
} MTKTSF_STATE_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
	MTKTSF_FEATURE_BEGIN,				// minimum of feature id
	MTKTSF_FEATURE_SET_ENV_INFO,		// feature id to setup environment information
    MTKTSF_FEATURE_GET_ENV_INFO,		// feature id to retrieve environment information
    MTKTSF_FEATURE_SET_PROC_INFO,		// feature id to setup processing information
	MTKTSF_FEATURE_GET_PROC_INFO,		// feature id to retrieve environment information
    MTKTSF_FEATURE_GET_RESULT,			// feature id to get result
	MTKTSF_FEATURE_GEN_CAP_TBL,			// feature id to generate capture image table
	MTKTSF_FEATURE_SET_TBL_CHANGE,		// feature id to set table info if table change
	MTKTSF_FEATURE_FORWARD_CONVERT,
	MTKTSF_FEATURE_BACKWARD_CONVERT,
	MTKTSF_FEATURE_BATCH,
    MTKTSF_FEATURE_GET_LOG,				// feature id to get debugging log
    MTKTSF_FEATURE_MAX					// maximum of feature id
}	MTKTSF_FEATURE_ENUM;

typedef enum 
{ 
    MTK_BAYER_B = 0, 
    MTK_BAYER_Gb, 
    MTK_BAYER_Gr, 
    MTK_BAYER_R, 
    MTK_BAYER_NO   
} MTK_BAYER_ORDER_ENUM; 

typedef struct 
{
	int raw_wd;
	int raw_ht;
	int block_wd;
	int block_ht;
	int block_wd_last;
	int block_ht_last;
	int x_grid_num;
	int y_grid_num;
	int x_offset;
	int y_offset;
} MTK_TSF_LSC_PARAM_STRUCT, *P_MTK_TSF_LSC_PARAM_STRUCT;


typedef struct
{
    MUINT16						ImgWidth;                        // input image width
    MUINT16						ImgHeight;                       // input image height
	MTK_BAYER_ORDER_ENUM		BayerOrder;
    MTK_TSF_LSC_PARAM_STRUCT	*pLscConfig;
	MINT32						*ShadingTbl;	
	MUINT32						*Para;
	MINT32						*pTuningPara;
	MUINT32						*WorkingBufAddr;
	MUINT8						*ImgAddr;  
	MUINT8						TS_TS;			// 0(disable); 1(enable)
	MUINT8						MA_NUM;			// 3~7
	MUINT8						Raw16_9Mode;	// 0(4:3); 1(16:9)
	MUINT32						*DebugAddr;
	MUINT32						DebugFlag;
} MTK_TSF_ENV_INFO_STRUCT, *P_MTK_TSF_ENV_INFO_STRUCT;

typedef struct
{
	MTK_TSF_LSC_PARAM_STRUCT	*pLscConfig;
	MINT32						*ShadingTbl;	
} MTK_TSF_TBL_STRUCT, *P_MTK_TSF_TBL_STRUCT;

typedef struct
{
	MUINT32				WorkingBuffSize;	// for upper layer to query
	MUINT32				DebugBuffSize;		// for upper layer to query
} MTK_TSF_GET_ENV_INFO_STRUCT, *P_MTK_TSF_GET_ENV_INFO_STRUCT;

typedef struct 
{
    MTKTSF_STATE_ENUM	TsfState;
} MTK_TSF_GET_PROC_INFO_STRUCT, *P_MTK_TSF_GET_PROC_INFO_STRUCT;

typedef struct 
{
    MUINT32				LogBuffAddr; 
	MUINT32				LogBuffSize;	// for upper layer to query
    MUINT32				DebugBuffAddr; 
	MUINT32				DebugBuffSize;	// for upper layer to query
} MTK_TSF_GET_LOG_INFO_STRUCT, *P_MTK_TSF_GET_LOG_INFO_STRUCT;

//--------------Process info--------------------//

typedef struct
{
	MINT32 *ShadingTbl;	
	MINT32 ParaL;
	MINT32 ParaC;
} MTK_TSF_SET_PROC_INFO_STRUCT, *P_MTK_TSF_SET_PROC_INFO_STRUCT;

typedef struct
{
	MUINT32 *ShadingTbl;
} MTK_TSF_RESULT_INFO_STRUCT, *P_MTK_TSF_RESULT_INFO_STRUCT;

class MTKTsf
{
public:
    static MTKTsf* createInstance();
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKTsf(){}
    // Process Control
    virtual MRESULT TsfInit(void *InitInData, void *InitOutData);	
    virtual MRESULT TsfMain(void);
	virtual MRESULT TsfExit(void);
	virtual MRESULT TsfReset(void);                 // RESET for each image

	// Feature Control        
	virtual MRESULT TsfFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};


#endif  //__MTK_TSF_H__
//MTK_SWIP_PROJECT_END