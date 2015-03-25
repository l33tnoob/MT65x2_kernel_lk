/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_PERFSURFACENATIVE_H
#define ANDROID_PERFSURFACENATIVE_H

__BEGIN_DECLS

enum {
    SCN_NONE           = 0,
    SCN_APP_SWITCH     = 1,
    SCN_APP_ROTATE     = 2,
    SCN_SW_CODEC       = 3,
    SCN_SW_CODEC_BOOST = 4,
    SCN_APP_TOUCH      = 5,
} PERFSURFACENATIVE_SCN_T;

enum {
    STATE_PAUSED    = 0,
    STATE_RESUMED   = 1,
    STATE_DESTROYED = 2,
    STATE_DEAD      = 3,
} PERFSURFACENATIVE_STATE_T;

extern void PerfServiceNative_boostEnable(int scenario);
extern void PerfServiceNative_boostDisable(int scenario);
extern void PerfServiceNative_boostEnableTimeout(int scenario, int timeout);
extern int  PerfServiceNative_userReg(int scn_core, int scn_freq);
extern void PerfServiceNative_userUnreg(int handle);
extern void PerfServiceNative_userEnable(int handle);
extern void PerfServiceNative_userDisable(int handle);
extern void PerfServiceNative_userEnableTimeout(int handle, int timeout);
extern void PerfServiceNative_userResetAll(void);
extern void PerfServiceNative_userDisableAll(void);

__END_DECLS

#endif // ANDROID_PERFSURFACENATIVE_H
