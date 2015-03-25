#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <utils/CallStack.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/threads.h>

#include <gui/mediatek/FpsCounter.h>

#include <cutils/properties.h>

namespace android {

//--------------------------------------------------------------------------------------------------
bool FpsCounter::reset() {
    mFps = 0.0;

    mMaxDuration = -1;
    mMinDuration = -1;
    mMaxDurationCounting = -1;
    mMinDurationCounting = -1;

    mFrames = 0;
    mLastLogTime = -1;
    mLastLogDuration = -1;

    mLastTime = -1;
    mLastDuration = -1;

    // read property as default log interval setting
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.sf.stc_interval", value, "1000");
    mCountInterval = ms2ns(atoi(value));

    return true;
}

bool FpsCounter::update() {
    return update(systemTime(SYSTEM_TIME_MONOTONIC));
}

bool FpsCounter::update(nsecs_t timestamp) {
    if ((-1 == mLastLogTime) || (-1 == mLastTime) || (mLastTime >= timestamp)) {
        mLastLogTime = mLastTime = timestamp;
        return false;
    }

    mFrames++;

    // count duration from last time update
    mLastDuration = timestamp - mLastTime;
    mLastTime = timestamp;
    if ((-1 == mMaxDurationCounting) || (mLastDuration > mMaxDurationCounting)) {
        mMaxDurationCounting = mLastDuration;
    }
    if ((-1 == mMinDurationCounting) || (mLastDuration < mMinDurationCounting)) {
        mMinDurationCounting = mLastDuration;
    }

    // check if reach statistics interval, print result and reset for next
    nsecs_t duration = timestamp - mLastLogTime;
    if (duration > mCountInterval) {

        // update data for FPS result
        mFps = mFrames * 1e9 / duration;
        mLastLogDuration = duration;
        mMaxDuration = mMaxDurationCounting;
        mMinDuration = mMinDurationCounting;

        // reset counting data for next
        mFrames = 0;
        mLastLogTime = timestamp;
        mMaxDurationCounting = -1;
        mMinDurationCounting = -1;

        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
}; // namespace android
