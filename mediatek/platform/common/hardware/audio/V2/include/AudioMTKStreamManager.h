#ifndef AUDIO_MTK_STREAM_MANAGER_H
#define AUDIO_MTK_STREAM_MANAGER_H

#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioMTKStreamManagerInterface.h"
#include "AudioMTKStreamManagerBase.h"
#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <media/AudioSystem.h>
#include "AudioUtility.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include "SpeechVMRecorder.h"
#include "AudioCustParam.h"

namespace android
{

class AudioMTKStreamManager : public AudioMTKStreamManagerBase
{
    public:

        /**
        * virtual destrutor
        */
        virtual ~AudioMTKStreamManager() {};

        /**
        * check init done.
        * @return status_t*/
        virtual status_t  initCheck();

        /**
        * do StreamExist  check is any stream input and output exist
        * @return status_t*/
        virtual bool  StreamExist() ;

        /**
        * do StreamExist  check is any stream input  exist
        * @return bool*/
        virtual bool  InputStreamExist() ;

        /**
        * do StreamExist  check is any stream output exist
        * @return bool*/
        virtual bool  OutputStreamExist() ;

        /** This method creates and opens the audio hardware output stream */
        virtual android_audio_legacy::AudioStreamOut *openOutputStream(
            uint32_t devices,
            int *format = 0,
            uint32_t *channels = 0,
            uint32_t *sampleRate = 0,
            status_t *status = 0,
            uint32_t output_flag = 0);

        /** This method creates and opens the audio hardware input stream */
        virtual android_audio_legacy::AudioStreamIn *openInputStream(
            uint32_t devices,
            int *format,
            uint32_t *channels,
            uint32_t *sampleRate,
            status_t *status,
            android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

        /**
        * do closeInputStream
        * @return status_t*/
        virtual status_t  closeInputStream(android_audio_legacy::AudioStreamIn *in);

        /**
        * do closeOutputStream
        * @return status_t*/
        virtual status_t closeOutputStream(android_audio_legacy::AudioStreamOut *out);

        /**
        * IsOutPutStreamActive
        * @return bool */
        virtual bool IsOutPutStreamActive(void);

        /**
        * do IsInPutStreamActive
        * @return bool*/
        virtual bool IsInPutStreamActive(void);

        // set musicplus to streamout
        virtual void SetMusicPlusStatus(bool bEnable);
        virtual bool GetMusicPlusStatus();

        /**
        * get input size from streammanager
        **/
        virtual size_t getInputBufferSize(int32_t sampleRate, int format, int channelCount);

        /**
        * do IsInPutStreamActive
        **/
        virtual status_t UpdateACFHCF(int value);

        virtual status_t ForceAllStandby();

        //suspend input and output
        virtual status_t SetOutputStreamSuspend(bool bEnable);
        virtual status_t SetInputStreamSuspend(bool bEnable);

        // ACF Preview parameter
        virtual status_t SetACFPreviewParameter(void *ptr , int len);
        virtual status_t SetHCFPreviewParameter(void *ptr , int len);

        // Set Input Mute
        virtual void SetInputMute(bool bEnable);

        // setParametersToStreamOut
        virtual status_t setParametersToStreamOut(const String8 &keyValuePairs);

        static AudioMTKStreamManager *getInstance();

        // set BesLoudness to streamout
        virtual void SetBesLoudnessStatus(bool bEnable);
        virtual bool GetBesLoudnessStatus();
        bool mBesLoudnessStatus;
#if 1 //def HIFI_SWITCH_BY_AUDENH //HP switch use AudEnh setting
        virtual void SetHiFiDACStatus(bool bEnable);
        virtual bool GetHiFiDACStatus(void);
        bool mHiFiDACStatus;
#endif

        virtual void SetBesLoudnessControlCallback(const BESLOUDNESS_CONTROL_CALLBACK_STRUCT *callback_data);
    protected:
        /**
        *  protected constructor of AudioMTKStreamManager
        */
        AudioMTKStreamManager();

    private:
        void (*mBesLoudnessControlCallback)(void *data);

        /**
        * single instance of AudioMTKStreamManager
        */
        static AudioMTKStreamManager *UniqueStreamManagerInstance;

        /**
        * intentionally undefined
        */
        AudioMTKStreamManager(const AudioMTKStreamManager &);

        /**
        * intentionally undefined = operator
        */
        AudioMTKStreamManager &operator=(const AudioMTKStreamManager &);

        uint32_t mStreamInNumber ;
        KeyedVector<uint32_t, android_audio_legacy::AudioStreamIn *> mStreamInVector; // vector to save current recording client
        uint32_t mStreamOutNumber ;
        KeyedVector<uint32_t, android_audio_legacy::AudioStreamOut *> mStreamOutVector; // vector to save current recording client

};

}

#endif
