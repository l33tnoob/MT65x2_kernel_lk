#ifndef ANDROID_ITER_SERVICE_H
#define ANDROID_ITER_SERVICE_H

#include <stdint.h>
#include <sys/types.h>
//#include <utils/threads.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
//#include <binder/Parcel.h>
namespace android
{
class ITerService : public IInterface
{
    virtual status_t getSimMccMncGemini(uint32_t simId, String8* outStr) = 0;
public:
    DECLARE_META_INTERFACE(TerService);

    virtual bool isEarlyReadServiceEnabled() = 0;
    virtual void setEarlyReadServiceEnable(bool onoff) = 0;

    virtual bool isEarlyDataReady() = 0;
//    virtual char* getSimMccMnc() = 0;
//    virtual char* getSimMccMncGemini(uint32_t simId) = 0;
    virtual status_t getSimMccMnc(String8* outStr) = 0;
};


// ------------------------------------------------------------------------- //
class BnTerService : public BnInterface<ITerService>
{
    String8 mMccMnc;

public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

// ------------------------------------------------------------------------- //

}
#endif
