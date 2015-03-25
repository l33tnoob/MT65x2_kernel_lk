#ifndef ANDROID_AUDIO_MTK_STREAMMANAGER_INTERFACE_H
#define ANDROID_AUDIO_MTK_STREAMMANAGER_INTERFACE_H

#include <hardware_legacy/AudioHardwareInterface.h>

#include "AudioPolicyParameters.h"

namespace android
{



//!  AudioMTKStreamMAnagerInterface interface
/*!
this class is hold for StreamIn , need to concer multitple user and mode change.
need to take care both input and output volume.
*/

class AudioMTKStreamManagerInterface
{
    public:
        /**
        * virtual destrutor
        */
        virtual ~AudioMTKStreamManagerInterface() {};

        /**
        * check init done.
        * @return status_t*/
        virtual status_t  initCheck() = 0;

        /**
        * do StreamExist  check is any stream input and output exist
        * @return status_t*/
        virtual bool  StreamExist() = 0 ;

        /**
        * do StreamExist  check is any stream input  exist
        * @return bool*/
        virtual bool  InputStreamExist() = 0 ;

        /**
        * do StreamExist  check is any stream output exist
        * @return bool*/
        virtual bool  OutputStreamExist() = 0 ;


        /** This method creates and opens the audio hardware output stream */
        virtual android_audio_legacy::AudioStreamOut *openOutputStream(
            uint32_t devices,
            int *format = 0,
            uint32_t *channels = 0,
            uint32_t *sampleRate = 0,
            status_t *status = 0,
            uint32_t output_flag = 0) = 0;

        /** This method creates and opens the audio hardware input stream */
        virtual android_audio_legacy::AudioStreamIn *openInputStream(
            uint32_t devices,
            int *format,
            uint32_t *channels,
            uint32_t *sampleRate,
            status_t *status,
            android_audio_legacy::AudioSystem::audio_in_acoustics acoustics) = 0;
        /**
        * do closeInputStream
        * @return status_t*/
        virtual status_t  closeInputStream(android_audio_legacy::AudioStreamIn *in) = 0;

        /**
        * do closeOutputStream
        * @return status_t*/
        virtual status_t closeOutputStream(android_audio_legacy::AudioStreamOut *out) = 0;

        /**
        * IsOutPutStreamActive
        * @return bool */
        virtual bool IsOutPutStreamActive(void) = 0;

        /**
        * do IsInPutStreamActive
        * @return bool*/
        virtual bool IsInPutStreamActive(void) = 0;

        /**
        * do IsInPutStreamActive
        **/
        virtual void SetMusicPlusStatus(bool bEnable) = 0;
        virtual bool GetMusicPlusStatus() = 0;

        /**
        * get input size from streammanager
        **/
        virtual size_t getInputBufferSize(int32_t sampleRate, int format, int channelCount) = 0;

        /**
        * do IsInPutStreamActive
        **/
        virtual status_t UpdateACFHCF(int value) = 0;

        //forceall input and output standby
        virtual status_t ForceAllStandby() = 0;

        //suspend input and output standby
        virtual status_t SetOutputStreamSuspend(bool bEnable) = 0;
        virtual status_t SetInputStreamSuspend(bool bEnable) = 0;

        // ACF Preview parameter
        virtual status_t SetACFPreviewParameter(void *ptr , int len) = 0;
        virtual status_t SetHCFPreviewParameter(void *ptr , int len) = 0;

        // FSync flag
        virtual bool GetFSyncFlag(int streamType) = 0;
        virtual void ClearFSync(int streamType) = 0;

        virtual status_t setParameters(const String8 &keyValuePairs, int IOport) = 0;
        virtual String8     getParameters(const String8 &keys, int IOport) = 0;
        // Set Input Mute
        virtual void SetInputMute(bool bEnable) = 0;
        
        // Low latency
        virtual status_t setParametersToStreamOut(const String8 &keyValuePairs) = 0;


        // set besloudness to streamout
        virtual void SetBesLoudnessStatus(bool bEnable) = 0;
        virtual bool GetBesLoudnessStatus() = 0;
        virtual void SetBesLoudnessControlCallback(const BESLOUDNESS_CONTROL_CALLBACK_STRUCT *callback_data) = 0;


#if 1 //def HIFI_SWITCH_BY_AUDENH //HP switch use AudEnh setting
        virtual void SetHiFiDACStatus(bool bEnable) = 0;
        virtual bool GetHiFiDACStatus(void) = 0;
#endif

};

}

#endif

