#ifndef __AAL_SERVICE_CLIENT_H__
#define __AAL_SERVICE_CLIENT_H__

#include <stdint.h>
#include <sys/types.h>

#include <binder/Binder.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>

namespace android {

class IAALService;
class AALClient : public Singleton<AALClient>
{
    friend class Singleton<AALClient>;
    
public:
    status_t setMode(int32_t mode);
    status_t setBacklightColor(int32_t color);    
    status_t setBacklightBrightness(int32_t level);
    status_t setLightSensorValue(int32_t value);
    status_t setScreenBrightness(int32_t brightness);
    status_t setScreenState(int32_t state, int32_t brightness);

    status_t setBrightnessLevel(int32_t level);
    status_t setDarkeningSpeedLevel(int32_t level);
    status_t setBrighteningSpeedLevel(int32_t level);
    status_t setSmartBacklightLevel(int32_t level);
    status_t setToleranceRatioLevel(int32_t level);
    status_t setReadabilityLevel(int32_t level);
private:    
    AALClient();
    
    // DeathRecipient interface
    void serviceDied();
    
    status_t assertStateLocked() const;

    mutable Mutex mLock;
    mutable sp<IAALService> mAALService;
    mutable sp<IBinder::DeathRecipient> mDeathObserver;
};

};

#endif
