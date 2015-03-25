/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_GUI_FPSCOUNTER_H
#define ANDROID_GUI_FPSCOUNTER_H

namespace android {
// ----------------------------------------------------------------------------

// tool class for FPS statistics, provide AVG, MAX, MIN message
// * AVG for FPS in a given duration
// * MAX and MIN for stability reference
class FpsCounter {
private:
    // for AVG
    float       mFps;

    // for MAX, MIN
    nsecs_t     mMaxDuration;
    nsecs_t     mMinDuration;
    nsecs_t     mMaxDurationCounting;
    nsecs_t     mMinDurationCounting;

    // per interval result
    uint32_t    mFrames;
    nsecs_t     mLastLogTime;
    nsecs_t     mLastLogDuration;

    // per update result
    nsecs_t     mLastTime;
    nsecs_t     mLastDuration;

public:
    // the given counting interval, read system property by default
    nsecs_t     mCountInterval;

    FpsCounter() { reset(); }
    ~FpsCounter() {}

    // main control
    bool reset();
    bool update(nsecs_t time);
    bool update();

    // get result
    inline float   getFps()             { return mFps;             }
    inline nsecs_t getMaxDuration()     { return mMaxDuration;     }
    inline nsecs_t getMinDuration()     { return mMinDuration;     }
    inline nsecs_t getLastLogDuration() { return mLastLogDuration; }
    inline nsecs_t getLastDuration()    { return mLastDuration;    }
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_FPSCOUNTER_H
