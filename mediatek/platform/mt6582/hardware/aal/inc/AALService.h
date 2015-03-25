#ifndef __AAL_SERVICE_H__
#define __AAL_SERVICE_H__

#include <utils/threads.h>
#include "IAALService.h"
#include "AAL.h"
#include "ALSWrapper.h"

// HAL
#include <hardware/hardware.h>
#include <hardware/lights.h>


#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>



#define AAL_STATISTICS_BUF_NUM 2
#define LED_RESERVEBIT_SHIFT 16
#define LED_RESERVEBIT_PATTERN 1

namespace android
{

class AALService : 
        public BinderService<AALService>, 
        public BnAALService,
        public Thread
{
    friend class BinderService<AALService>;
public:
    // screen brightenss mode copy from Settings.System
    enum BrightnessMode {
        /** SCREEN_BRIGHTNESS_MODE value for manual mode. */
        SCREEN_BRIGHTNESS_MODE_MANUAL = 0,
        /** SCREEN_BRIGHTNESS_MODE value for automatic mode. */
        SCREEN_BRIGHTNESS_MODE_AUTOMATIC = 1,
        /** SCREEN_ECO_BRIGHTNESS_MODE value for automatic eco backlight mode. */
        SCREEN_BRIGHTNESS_ECO_MODE_AUTOMATIC = 2,
    };
    
    AALService();
    ~AALService();
    
    static char const* getServiceName() { return "AAL"; }
    
    // IAALServic interface
    virtual status_t setMode(int32_t mode);
    virtual status_t setBacklightColor(int32_t color);
    virtual status_t setBacklightBrightness(int32_t brightness);
    virtual status_t setLightSensorValue(int32_t value);
    virtual status_t setScreenBrightness(int32_t brightness);
    virtual status_t setScreenState(int32_t state, int32_t brightness);
    
    virtual status_t dump(int fd, const Vector<String16>& args);
    
    status_t setBrightnessLevel(int32_t level);
    status_t setDarkeningSpeedLevel(int32_t level);
    status_t setBrighteningSpeedLevel(int32_t level);
    status_t setSmartBacklightLevel(int32_t level);
    status_t setToleranceRatioLevel(int32_t level);
    status_t setReadabilityLevel(int32_t level);
private:
    virtual void onFirstRef();
    virtual status_t readyToRun();
    virtual bool threadLoop();

    void reset();
    
    status_t enableAALEvent(int enable);

    status_t loadCalData();
    status_t loadCfgData();

    status_t loadConfig();
    
    status_t setBacklight(int32_t level);

    status_t debugDump();

    void updateDebugInfo(sp<SurfaceControl> surface);
    void clearDebugInfo(sp<SurfaceControl> surface);

    // hardware
    light_device_t *mLight;
    int mFd;

    mutable Mutex mLock;
    bool mIsCalibrated;
    int mEnableEvent;
    int mPendingState;
    int mPendingBacklight;
    int mState;
    int mBacklight;
    int mAnimation;
    int mMode;
    AAL *mAALFW;
    AAL *mBLSHW;
    
    // ALS
    ALSWrapper mALSWrapper;
    float mALS;
    float mALI;

    // platform dependent
    int mIdx;
    DISP_AAL_STATISTICS mAALStatistic[AAL_STATISTICS_BUF_NUM]; // ring buffer to keep history
    DISP_AAL_PARAM mAALParams;

    unsigned int mFramePixels;

    // for AAL debug information
    bool mAALDebugOn;
    bool mAALDebugOnScreen;
};
};
#endif
