#define LOG_TAG "AudioSetParam"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryBase.h>
#include <media/AudioSystem.h>
#include <media/mediaplayer.h>
#include <system/audio_policy.h>
#include <hardware/audio_policy.h>
#include <hardware_legacy/AudioPolicyInterface.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <hardware/hardware.h>
#include <system/audio.h>

using namespace android;

int main()
{
    ProcessState::self()->startThreadPool();
    sp<ProcessState> proc(ProcessState::self());

    char cmd[1024];
    while (true)
    {

        printf("\nplease enter command, ex: 'GET_XXX_ENABLE', 'SET_XXX_ENABLE=0', 'SET_XXX_ENABLE=1', and '0' for exit\n\n");
        scanf("%s", cmd);

        if (cmd[0] == '0')   // exit
        {
            break;
        }
        else if (strrchr(cmd, '=') != NULL)   // has '=', it's set function
        {
            AudioSystem::setParameters(0, String8(cmd));
        }
        else   // get function
        {
            printf("%s\n", AudioSystem::getParameters(0, String8(cmd)).string());
        }
    }

    return 0;
}


