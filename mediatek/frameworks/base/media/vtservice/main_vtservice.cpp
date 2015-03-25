/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

// System headers required for setgroups, etc.
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <linux/rtpm_prio.h>
#include <sys/prctl.h>
#include <android_filesystem_config.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#ifdef MTK_VT3G324M_SUPPORT
#include "VTSServiceForMediaServer.h"
#endif

using namespace android;

int main(int argc, char** argv)
{
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    ALOGI("ServiceManager: %p", sm.get());
 
#ifdef MTK_VT3G324M_SUPPORT
    VTService::VTSService_instantiate();
#endif

    if (AID_ROOT == getuid())
    {
        //setuid(AID_MEDIA);
        // Change UID from MEDIA to RADIO for the reason of AppOPS on MR2.
        // Camera Service will check if the caller and callee are in the same uid
        // So, change to RADIO resulting phone & VT are in the same process.
        setuid(AID_RADIO);
    }
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
}
