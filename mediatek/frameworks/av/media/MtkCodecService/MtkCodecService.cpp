#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <sys/prctl.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <linux/rtpm_prio.h>
#include <sys/capability.h>
#include <private/android_filesystem_config.h>
#include "../BnMtkCodec/BnMtkCodec.h"

using namespace android;

int main(int argc, char **argv)
{
    struct sched_param param;
    param.sched_priority = RTPM_PRIO_OMX_AUDIO;
    sched_setscheduler(0, SCHED_RR, &param);
    
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    BnMtkCodec::instantiate();
    //
    if (AID_ROOT == getuid())
    {
        //re-adjust caps for its thread, and set uid to media
        if (-1 == prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0))
        {
            ALOGW("mtkcodecservice prctl for set caps failed: %s", strerror(errno));
        } 
        else
        {
            __user_cap_header_struct hdr;
            __user_cap_data_struct data;

            setuid(AID_MEDIA);         // change user to media
    
            hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps again
            hdr.pid = 0;
            data.effective = (1 << CAP_SYS_NICE);
            data.permitted = (1 << CAP_SYS_NICE);
            data.inheritable = 0xffffffff;
            if (-1 == capset(&hdr, &data))
            {
                ALOGW("mediaserver cap re-setting failed, %s", strerror(errno));
            }
        }

    }
    else
    {
        ALOGD("mtkcodecservice re-adjust caps is not in root user");
    }
    //
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
    return 0;
}
