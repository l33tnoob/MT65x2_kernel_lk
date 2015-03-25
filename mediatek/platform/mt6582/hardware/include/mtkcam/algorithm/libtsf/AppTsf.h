//MTK_SWIP_PROJECT_START
#ifndef __APP_TSF_H__
#define __APP_TSF_H__

#include "MTKTsfType.h"
#include "MTKTsf.h"

/*****************************************************************************
	Class Define
******************************************************************************/
class AppTsf : public MTKTsf {
public:    
    static MTKTsf* getInstance();
    virtual void destroyInstance();
    
    AppTsf();
    virtual ~AppTsf();   
    // Process Control
    MRESULT TsfInit(void *InitInData, void *InitOutData);
    MRESULT TsfMain(void);					// START
	MRESULT TsfExit(void);					// EXIT
	MRESULT TsfReset(void);                 // RESET for each image

	// Feature Control        
	MRESULT TsfFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);	
private:
    MTKTSF_STATE_ENUM			m_TsfState;
};

#endif //__APP_TSF_H__
//MTK_SWIP_PROJECT_END