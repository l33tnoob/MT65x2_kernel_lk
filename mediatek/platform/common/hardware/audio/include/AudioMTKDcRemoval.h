#ifndef AUDIO_MTK_DCREMOVE_PROCESS_H
#define AUDIO_MTK_DCREMOVE_PROCESS_H
#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <utils/threads.h>
#ifndef uint32
typedef unsigned int        uint32;
#endif

extern "C" {
#include <dc_removal_flt.h>
}
namespace android
{

class DcRemove
{
    public:
        enum {
            DCR_MODE_1 = 0,
            DCR_MODE_2,
            DCR_MODE_3
        };
        DcRemove();
        ~DcRemove();
        status_t init(uint32 channel, uint32 samplerate, uint32 drcMode);
        size_t process(const void *inbuffer, size_t bytes, void *outbuffer);
        status_t close();
    private:
        DCRemove_Handle *mHandle;
        DcRemove(const DcRemove &);
        DcRemove &operator=(const DcRemove &);
        mutable Mutex  mLock;
};
}
#endif
