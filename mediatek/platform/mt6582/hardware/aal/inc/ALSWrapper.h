#ifndef __ALS_WRAPPER_H__
#define __ALS_WRAPPER_H__

#include <utils/KeyedVector.h>

namespace android {

class ALSWrapper {
public:
    ALSWrapper() {
        mInit = false;
        mRatio = 0.1f;
        mCurLux = 0;
        mCurThd = 0;
        mALSMap.clear();
    }
    
    virtual ~ALSWrapper() {
    }
    
    virtual void reset() {
        mCurLux = 0;
        mCurThd = 0;
    }

    virtual void setRatio(float ratio) {
        mRatio = ratio;
        reset();
    }
   
    virtual bool init(float *als, float *lux, int size);   
    virtual bool init(KeyedVector<float, float> &map);
    virtual float remap(int als);
    virtual float debounce(float lux);
    virtual void dump();

private:
    bool mInit;
    KeyedVector<float, float> mALSMap;

    float mRatio;
    float mCurLux;
    float mCurThd;
};
};
#endif 

