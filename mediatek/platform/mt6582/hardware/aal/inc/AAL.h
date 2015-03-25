#ifndef __AAL_H__
#define __AAL_H__

#define DEFAULT_BACKLIGHT 102

// platform dependent
#include "ddp_drv.h"
#include <utils/String8.h>

using namespace android;

class AAL {
public:    
    enum Mode {
        AAL_MODE_DISABLE    = 0,
        AAL_MODE_AUTO       = 1,
        AAL_MODE_CABC       = 2,
        AAL_MODE_UT         = 4
    };

    enum Status {
        eDirtyNone          = 0x0,
        eDirtyBacklight     = 0x1,
        eDirtyLumaCurve     = 0x2,
        eDirtyFilter        = 0x4
    };

    enum ResetFlag {
        eResetParams        = 0x1,
        eResetCurve         = 0x2,
        eResetFilters       = 0x4,
        eResetAll           = 0x7,
    };

    AAL() : mFavorBacklight(DEFAULT_BACKLIGHT) {}
    virtual ~AAL() {}

    virtual void setMode(const unsigned int mode) = 0;
    virtual void setFavorBacklight(const unsigned int level) = 0;

    virtual unsigned int  getTargetBacklight() { return mFavorBacklight; }

    virtual void reset(unsigned int flag) = 0;

    virtual int calibrate(float *luminance) = 0;
    virtual int contentAnalysis(DISP_AAL_STATISTICS *statistics, int index, int num) = 0;
    virtual int ambientlightAnalysis(unsigned int als) = 0;
    virtual int calculateParams(DISP_AAL_PARAM *param) = 0;

    virtual int loadConfig() = 0;
    virtual int loadCustParams(void *lib) { return 0; };
    
    virtual void dump(String8& result, char* buffer, size_t SIZE) const = 0;
    virtual void dump() = 0;

    virtual void dumpDebugInfo(char* buffer, size_t SIZE) = 0;
protected:
    unsigned int mFavorBacklight;
};


#endif

