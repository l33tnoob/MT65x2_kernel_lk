#ifndef ANDROID_WCN_CHIP_CONTROLLER_H
#define ANDROID_WCN_CHIP_CONTROLLER_H

#include "AudioType.h"
#include <linux/fm.h>

#include <utils/threads.h>

extern "C" {
#include "bt_drv.h"
}

namespace android
{

class WCNChipController
{
    public:
        virtual ~WCNChipController();

        static WCNChipController *GetInstance();

        virtual bool     GetFmChipPowerInfo();

        virtual status_t SetFmChipVolume(const uint32_t fm_chip_volume);


        virtual status_t InitAudioFMInfo();
        virtual status_t InitAudioBTInfo();

        virtual bool     IsFMMergeInterfaceSupported();
        virtual bool     IsBTMergeInterfaceSupported();

        virtual bool     IsFmChipPadSelConnSys();
        virtual bool     IsFmChipUseSlaveMode();
        virtual uint32_t GetFmChipSamplingRate();
        virtual uint32_t BTChipHWInterface();
        virtual bool BTUseCVSDRemoval();
        virtual uint32_t BTChipSamplingRate();
        virtual uint32_t BTChipSyncFormat();
        virtual uint32_t BTChipSyncLength();
        virtual uint32_t BTChipSecurityHiLo();


    protected:
        WCNChipController();

        Mutex            mLock;

        bool             mInitAudioFMInfoFlag;
        bool             mInitAudioBTInfoFlag;
        fm_audio_info_t  mFmAudioInfo;
        AUDIO_CONFIG  mBTAudioInfo;


    private:
        static WCNChipController *mWCNChipController; // singleton
};

} // end namespace android

#endif // end of ANDROID_WCN_CHIP_CONTROLLER_H
