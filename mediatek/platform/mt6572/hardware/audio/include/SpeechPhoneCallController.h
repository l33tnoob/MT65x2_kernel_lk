#ifndef ANDROID_SPEECH_PHONE_CALL_CONTROLLER_H
#define ANDROID_SPEECH_PHONE_CALL_CONTROLLER_H

#include "AudioType.h"
#include "AudioDigitalType.h"

#include "AudioResourceManagerInterface.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"
#include "AudioVolumeFactory.h"
#include "SpeechDriverFactory.h"
#include "AudioBTCVSDControl.h"


enum tty_mode_t
{
    AUD_TTY_OFF  =  0,
    AUD_TTY_FULL =  1,
    AUD_TTY_VCO  =  2,
    AUD_TTY_HCO  =  4,
    AUD_TTY_ERR  = -1
};

namespace android
{

class SpeechPhoneCallController
{
    public:
        virtual ~SpeechPhoneCallController();

        static SpeechPhoneCallController *GetInstance();


        virtual status_t        OpenModemSpeechControlFlow(const audio_mode_t audio_mode);
        virtual status_t        CloseModemSpeechControlFlow(const audio_mode_t audio_mode);
        virtual status_t        ChangeDeviceForModemSpeechControlFlow(const audio_mode_t audio_mode, const audio_devices_t new_device);


        virtual bool            CheckTtyNeedOn() const;
        virtual bool            CheckSideToneFilterNeedOn(const audio_devices_t output_device) const;

        virtual tty_mode_t      GetTtyCtmMode() const { return mTty_Ctm; }
        virtual status_t        SetTtyCtmMode(const tty_mode_t tty_mode, const audio_mode_t audio_mode);

        virtual audio_devices_t GetRoutingForTty() const { return mRoutingForTty; }
        virtual void            SetRoutingForTty(const audio_devices_t new_device) { mRoutingForTty = new_device; }

        virtual void            SetVtNeedOn(const bool vt_on);
        virtual void            SetMicMute(const bool mute_on);
        virtual void            SetBTMode(const int mode);

    protected:
        SpeechPhoneCallController();

        virtual bool            IsModeIncall(const audio_mode_t audio_mode) const;

        virtual status_t        SetAfeAnalogClock(const bool clock_on);

        virtual status_t        OpenModemSpeechDigitalPart(const modem_index_t modem_index, const audio_devices_t output_device);
        virtual status_t        CloseModemSpeechDigitalPart(const modem_index_t modem_index, const audio_devices_t output_device);

        virtual status_t        SetModemSpeechInterConnection(AudioDigitalType::Digital_Block block, modem_index_t modem_index, AudioDigitalType::InterConnectionState Connection);

        virtual status_t        SetModemSpeechDACI2sOutAttribute(int sample_rate);
        virtual status_t        SetModemSpeechADCI2sInAttribute(int sample_rate);
        virtual status_t        SetModemSpeechDAIBTAttribute(int sample_rate);

        virtual status_t        SetModemPcmAttribute(const modem_index_t modem_index, int sample_rate);

        virtual void            SetTtyInOutDevice(audio_devices_t routing_device, tty_mode_t tty_mode, audio_mode_t audio_mode);

        Mutex mLock;

        AudioResourceManagerInterface *mAudioResourceManager;
        AudioAnalogControlInterface   *mAudioAnalogInstance;
        AudioDigitalControlInterface  *mAudioDigitalInstance;
        AudioMTKVolumeInterface       *mAudioVolumeInstance;
        SpeechDriverFactory           *mSpeechDriverFactory;

        audio_devices_t mRoutingForTty;
        tty_mode_t      mTty_Ctm;

    private:
        static SpeechPhoneCallController *mSpeechPhoneCallController; // singleton

        bool mVtNeedOn;

        bool mMicMute;
        int mBTMode;

#ifdef EXT_MODEM_BT_CVSD
        AudioBTCVSDControl *mAudioBTCVSDControl;
#endif

};

} // end namespace android

#endif // end of ANDROID_SPEECH_PHONE_CALL_CONTROLLER_H
