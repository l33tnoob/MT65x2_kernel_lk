#ifndef ANDROID_AUDIO_MTK_HARDWARE_H
#define ANDROID_AUDIO_MTK_HARDWARE_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <utils/String8.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <media/AudioSystem.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include "AudioType.h"
#include "AudioMTKVolumeInterface.h"
#include "AudioDigitalControlInterface.h"
#include "AudioAnalogControlInterface.h"
#include "AudioDigitalType.h"

#include "AudioMTKStreamManager.h"
#include "AudioMTKStreamManagerInterface.h"

#include "AudioResourceManager.h"
#include "AudioResourceFactory.h"
#include "AudioFtm.h"

#include "SpeechDriverFactory.h"
#include "AudioSpeechEnhanceInfo.h"
#include "AudioBTCVSDControl.h"

#include "AudioParamTuning.h"

#include "AudioFMController.h"
#include "AudioMTKHardwareCommand.h"
//#if defined(MTK_VIBSPK_SUPPORT)
#include "AudioVIBSPKControl.h"
//#endif


namespace android
{

class AudioMTKHardware : public android_audio_legacy::AudioHardwareBase
{
    public:
        AudioMTKHardware();
        AudioMTKHardware(bool SpeechControlEnable);
        virtual             ~AudioMTKHardware();

        /**
         * check to see if the audio hardware interface has been initialized.
         * return status based on values defined in include/utils/Errors.h
         */
        virtual status_t    initCheck();

        /** set the audio volume of a voice call. Range is between 0.0 and 1.0 */
        virtual status_t    setVoiceVolume(float volume);

        /**
         * set the audio volume for all audio activities other than voice call.
         * Range between 0.0 and 1.0. If any value other than NO_ERROR is returned,
         * the software mixer will emulate this capability.
         */
        virtual status_t    setMasterVolume(float volume);

        /**
         * setMode is called when the audio mode changes. NORMAL mode is for
         * standard audio playback, RINGTONE when a ringtone is playing, and IN_CALL
         * when a call is in progress.
         */
        virtual status_t    setMode(int mode);

        // mic mute
        virtual status_t    setMicMute(bool state);
        virtual status_t    getMicMute(bool *state);

        // set/get global audio parameters
        virtual status_t    setParameters(const String8 &keyValuePairs);
        virtual String8     getParameters(const String8 &keys);

        // Returns audio input buffer size according to parameters passed or 0 if one of the
        // parameters is not supported
        virtual size_t    getInputBufferSize(uint32_t sampleRate, int format, int channelCount);

        /** This method creates and opens the audio hardware output stream */
        virtual android_audio_legacy::AudioStreamOut *openOutputStream(
            uint32_t devices,
            int *format = 0,
            uint32_t *channels = 0,
            uint32_t *sampleRate = 0,
            status_t *status = 0);
        virtual    void        closeOutputStream(android_audio_legacy::AudioStreamOut *out);
        /** This method creates and opens the audio hardware input stream */
        virtual android_audio_legacy::AudioStreamIn *openInputStream(
            uint32_t devices,
            int *format,
            uint32_t *channels,
            uint32_t *sampleRate,
            status_t *status,
            android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);
        virtual    void        closeInputStream(android_audio_legacy::AudioStreamIn *in);

        /**This method dumps the state of the audio hardware */
        virtual status_t dumpState(int fd, const Vector<String16> &args);

        // Interface of AduioMTKHardware.h
        // add EM parameter or general purpose commands
        virtual status_t SetEMParameter(void *ptr, int len) ;
        virtual status_t GetEMParameter(void *ptr, int len) ;
        virtual status_t SetAudioCommand(int par1, int par2);
        virtual status_t GetAudioCommand(int par1);
        virtual status_t SetAudioData(int par1, size_t len, void *ptr);
        virtual status_t GetAudioData(int par1, size_t len, void *ptr);

        // ACF Preview parameter
        virtual status_t SetACFPreviewParameter(void *ptr , int len);
        virtual status_t SetHCFPreviewParameter(void *ptr , int len);

        //for PCMxWay Interface API
        virtual int xWayPlay_Start(int sample_rate);
        virtual int xWayPlay_Stop(void);
        virtual int xWayPlay_Write(void *buffer, int size_bytes);
        virtual int xWayPlay_GetFreeBufferCount(void);
        virtual int xWayRec_Start(int sample_rate);
        virtual int xWayRec_Stop(void);
        virtual int xWayRec_Read(void *buffer, int size_bytes);
        //added by wendy
        virtual int ReadRefFromRing(void *buf, uint32_t datasz, void *DLtime);
        virtual int GetVoiceUnlockULTime(void *DLtime);
        virtual int SetVoiceUnlockSRC(uint outSR, uint outChannel);
        virtual bool startVoiceUnlockDL();
        virtual bool stopVoiceUnlockDL();
        virtual void freeVoiceUnlockDLInstance();
        virtual int GetVoiceUnlockDLLatency();
        virtual bool getVoiceUnlockDLInstance();



    protected:
        /** returns true if the given mode maps to a telephony or VoIP call is in progress */
        virtual bool     isModeInCall(int mode)
        {
            return ((mode == AUDIO_MODE_IN_CALL)
                    || (mode == AUDIO_MODE_IN_CALL_2)
                    || (mode == AUDIO_MODE_IN_CALL_EXTERNAL)
                    || (mode == AUDIO_MODE_IN_COMMUNICATION));
        }
        /** returns true if a telephony or VoIP call is in progress */
        virtual bool     isInCall() { return isModeInCall(mMode); }

    private:
        status_t ForceAllStandby(void);
        status_t SetOutputSuspend(bool bEnable);
        status_t SetInputSuspend(bool bEnable);
        bool ModeInCall(audio_mode_t mode);
        // this will base on Mode and Audiohardware's mMode to check
        bool ModeEnterCall(audio_mode_t Mode);
        bool ModeLeaveCall(audio_mode_t Mode);
        bool ModeEnterSipCall(audio_mode_t Mode);
        bool ModeLeaveSipCall(audio_mode_t Mode);
        bool ModeCallSwitch(audio_mode_t Mode);

        bool InputStreamExist();
        bool OutputStreamExist();
        bool StreamExist();

        bool IsOutPutStreamActive();
        bool IsInPutStreamActive();

        status_t HardwareInit(bool BenableSpeech);

        bool UpdateOutputFIR(int mode , int index);
        status_t GetDefaultDcCalibrationParam(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *cali_param);
        status_t DcCalibrationProcess(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *cali_param);
        status_t SetDcCalibration(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *cali_param);

        status_t dump(int fd, const Vector<String16> &args);

        void             UpdateKernelState();

        audio_mode_t     mMode;

        int              mFd;
        bool             mHardwareInit;

        bool             mMicMute;

        AudioMTKVolumeInterface *mAudioVolumeInstance;
        AudioAnalogControlInterface *mAudioAnalogInstance;
        AudioDigitalControlInterface *mAudioDigitalInstance;

        SpeechDriverFactory *mSpeechDriverFactory;

        AudioResourceManagerInterface *mAudioResourceManager;
        AudioMTKStreamManagerInterface *mAudioMTKStreamManager;
        AudioFtm *mAudioFtmInstance;
        AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;
        AudioBTCVSDControl *mAudioBTCVSDControl;

        SPH_Control     mAudio_Control_State;

        AudioParamTuning *mAudioTuningInstance;

        pthread_mutex_t setParametersMutex;  // use for setParameters
        AUDIO_BUFFER_DC_CALIBRATION_STRUCT mDcCaliParam;
        status_t SetAudioCommonCommand(int par1, int par2);
        status_t GetAudioCommonCommand(int parameters1);
        status_t SetAudioCommonData(int par1, size_t len, void *ptr);
        status_t GetAudioCommonData(int par1, size_t len, void *ptr);
        String8 getCommonParameters(AudioParameter &param, AudioParameter &returnParam);
        status_t setCommonParameters(const String8 &keyValuePairs);
};

}

#endif
