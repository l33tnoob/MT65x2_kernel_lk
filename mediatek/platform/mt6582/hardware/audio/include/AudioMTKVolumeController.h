#ifndef ANDROID_AUDIO_MTK_VOLUME_CONTROLLER_H
#define ANDROID_AUDIO_MTK_VOLUME_CONTROLLER_H

#include "AudioMTKVolumeInterface.h"
#include "AudioDigitalControlFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioType.h"
#include "AudioCustParam.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include <cutils/properties.h>

#include "audio_custom_exp.h"

#include <IATVCtrlService.h>
#include <binder/IServiceManager.h>
namespace android
{
#define NUM_AUDIO_ROUTE  (17)

/****************************************************
* Define Volume Range of  sound & Voice.
*****************************************************/
#define DEVICE_AUDIO_BUFFER_MAX_VOLUME           (9)
#define DEVICE_AUDIO_BUFFER_MIN_VOLUME           (-5)
#define DEVICE_AMP_MAX_VOLUME     (15)
#ifndef DEVICE_AMP_MIN_VOLUME   //reDefine @Audio_custom_exp.h 
#define DEVICE_AMP_MIN_VOLUME     (6)
#endif
#define DEVICE_IV_BUFFER_MAX_VOLUME     (5)
#define DEVICE_IV_BUFFER_MIN_VOLUME     (-2)
#define DEVICE_LINEIN_PLAY_MAX_VOLUME  (20)

#define AUDIO_VOLUME_MAX       (152)
#define AUDIO_ONEDB_STEP         (4)
#define AUDIO_TWODB_STEP         (8)
#define VOICE_VOLUME_MAX       (160)
#define VOICE_ONEDB_STEP         (4)
#define AMP_VOLUME_MAX           (172)
#define AMP_ONEDB_STEP             (4)
#define UPLINK_GAIN_MAX            (252)
#define UPLINK_ONEDB_STEP         (4)
#define LEVEL_SHIFT_BUFFER_GAIN_MAX   (240)
#define LEVEL_SHIFT_THREEDB_STEP              (32)
#define LEVEL_SHIFT_BUFFER_MAXDB        (21)
#define LINEIN_GAIN_MAX                       (240)
#define LINEIN_GAIN_2DB_STEP                (16)
#define SIDETONE_GAIN_MAX            (240)
#define SIDETONE_ONEDB_STEP         (4)

#define HW_DIGITAL_GAIN_MAX (252)
#define HW_DIGITAL_GAIN_STEP (4)   // 2==> 0.5 dB ,4==>1dB

#define AUDIO_SYSTEM_UL_GAIN_MAX            (45)
#define MIN_PGA_GAIN                                         (2)
#define MAX_PGA_GAIN_RANGE                          (30)
#define AUDIO_UL_PGA_STEP                               (6)
#define SW_AGC_GAIN_MAX                                (17)
#define TRANSPARENT_AGC_GAIN_OFFSET       (33)                        //for digital microphone


typedef enum
{
    Idle_Normal_Record = 0,
    Idle_Headset_Record,
    Voice_Rec_Mic_Handset,
    Voice_Rec_Mic_Headset,
    Idle_Video_Record_Handset,
    Idle_Video_Record_Headset,
    Normal_Mic,
    Headset_Mic,
    Handfree_Mic,
    VOIP_Normal_Mic,
    VOIP_Headset_Mic,
    VOIP_Handfree_Mic,
    TTY_CTM_Mic,
    Level_Shift_Buffer_Gain,
    Analog_PLay_Gain,
    Voice_UnLock_Mic_Handset,
    Voice_UnLock_Mic_Headset,
    Customization1_Mic_Handset,
    Customization1_Mic_Headset,
    Customization2_Mic_Handset,
    Customization2_Mic_Headset,
    Customization3_Mic_Handset,
    Customization3_Mic_Headset,
    Num_Mic_Gain
} MIC_GAIN_MODE;

typedef enum
{
    EarPiece_SideTone_Gain = 0,
    Headset_SideTone_Gain ,
    LoudSpk_SideTone_Gain ,
    Num_Side_Tone_Gain
} SIDETOEN_GAIN_MODE;

typedef enum
{
    Audio_Earpiece = 0,
    Audio_Headset,
    Audio_Headphone,
    Audio_Speaker,
    Audio_DualMode_Earpiece,
    Audio_DualMode_Headset,
    Audio_DualMode_Headphone,
    Audio_DualMode_speaker,
    Ringtone_Earpiece,
    Ringtone_Headset,
    Ringtone_Headphone,
    Ringtone_Speaker,
    Sipcall_Earpiece,
    Sipcall_Headset,
    Sipcall_Headphone,
    Sipcall_Speaker,
    Num_of_Audio_gain
} AUDIO_GAIN_MODE;

typedef enum
{
    DRC_VERSION_1  = 0,
    DRC_VERSION_2 = 1,
} DRC_VERSION;


class AudioMTKVolumeController : public AudioMTKVolumeInterface
{
    public:
        static float linearToLog(int volume);
        static int logToLinear(float volume);

        static AudioMTKVolumeController *getInstance();
        ~AudioMTKVolumeController() {};
        /**
         * check to see if the audio hardware interface has been initialized.
         */
        virtual status_t    initCheck();
        virtual status_t    initVolumeController();

        virtual status_t setMasterVolume(float v, audio_mode_t mode, uint32_t devices);
        virtual float getMasterVolume();
        virtual status_t setVoiceVolume(float v, audio_mode_t mode, uint32_t devices);
        virtual float getVoiceVolume(void);


        // here only valid stream can be set .
        virtual status_t setStreamVolume(int stream, float v);
        virtual status_t setStreamMute(int stream, bool mute);
        virtual float getStreamVolume(int stream);

        // should depend on different usage , FM ,MATV and output device to setline in gain
        virtual status_t SetLineInPlaybackGain(int type);
        virtual status_t SetLineInRecordingGain(int type);

        /**
          * volume controller setMatvVolume
          * set Matv Volume
          * @param volume
          * @return bool
          */
        virtual bool setMatvVolume(int volume);

        /**
          * volume controller SetMatvMute
          * Set Matv Mute
          * @param b_mute
          * @return bool
          */
        virtual bool SetMatvMute(bool b_mute);

        virtual status_t SetMicGain(uint32_t Mode, uint32_t devices);
        virtual status_t SetULTotalGain(uint32_t Mode, unsigned char Volume);

        virtual status_t SetSideTone(uint32_t Mode, uint32_t devices);
        virtual status_t SetMicGainTuning(uint32_t Mode, uint32_t gain);

        /**
        * volume controller GetSideToneGain
        * base on output device get sidetone gain
        * @param device
        * @return gain
        */
        virtual uint32_t GetSideToneGain(uint32_t device);

        virtual status_t ApplySideTone(uint32_t Mode);

        /**
        * volume controller ApplyMicGain
        * base on mode gain and route to decide sidetone gain
        * @param Mode
        * @param Gain
        * @return status_t
        */
        virtual status_t ApplyMicGain(uint32_t MicType, int mode);


        /**
        * volume controller SetDigitalHwGain
        * base on mode gain and route to set digital HW gain
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t SetDigitalHwGain(uint32_t Mode, uint32_t Gain , uint32_t routes);

        /**
        * volume controller Set modem DL gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdDlGain(int  Gain);

        /**
        * volume controller Set modem DL Ehn gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdDlEhn1Gain(int32 Gain);

        /**
        * volume controller Set modem Ul gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdUlGain(int  Gain);

        /**
        * volume controller map volume to digital gain
        * @param Gain
        * @return digital gain
        */
        virtual uint16_t MappingToDigitalGain(unsigned char Gain);

        /**
        * volume controller map volume to PGA gain
        * @param Gain
        * @return PGA gain
        */
        virtual uint16_t MappingToPGAGain(unsigned char Gain);

        /**
        *  volume controller GetSWMICGain
        * get MIC software digital gain for HD record library
        */
        virtual uint8_t GetSWMICGain() {return mSwAgcGain;}

        /**
        *  volume controller GetULTotalGain
        * get MIC software digital gain for BesRecord library
        */
        virtual uint8_t GetULTotalGain() {return mULTotalGain;}

    private:
        static AudioMTKVolumeController *UniqueVolumeInstance;
        AudioMTKVolumeController();
        AudioMTKVolumeController(const AudioMTKVolumeController &);             // intentionally undefined
        AudioMTKVolumeController &operator=(const AudioMTKVolumeController &);  // intentionally undefined
        void GetDefaultVolumeParameters(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *volume_param);

        // cal and set and set analog gainQuant
        void ApplyAudioGain(int Gain, uint32 mode, uint32 device);
        void ApplyAmpGain(int Gain, uint32 mode, uint32 device);
        void ApplyExtAmpHeadPhoneGain(int Gain, uint32 mode, uint32 device);
        void ApplyDualmodeGain(int Gain, uint32 mode, uint32 device);
        bool SetVolumeRange(uint32 mode, int32 MaxVolume, int32 MinVolume, int32 VolumeRange);

        bool IsHeadsetMicInput(uint32 device);
        uint16_t UpdateSidetone(int DL_PGA_Gain, int  Sidetone_Volume , uint8_t SW_AGC_Ul_Gain);

        bool CheckMicUsageWithMode(uint32_t MicType, int mode);
        uint32_t GetSideToneGainType(uint32 devices);
        int GetMatvVolume(void);
        void GetMatvService(void);
        bool SetLevelShiftBufferGain(uint32 MicMode, uint32 Gain);
        bool ApplyLevelShiftBufferGain(uint32 MicMode);
        bool ModeSetVoiceVolume(int mode);
        uint32_t GetDRCVersion(uint32 device);
        /**
        * volume controller mapping gain into DB
        * base on Gain and return Degrade in DB
        * @param Gain
        * @return uint32_t
        */
        uint32_t MapDigitalHwGain(uint32_t Gain);

        AudioAnalogControlInterface *mAudioAnalogControl;
        AudioDigitalControlInterface *mAudioDigitalControl;

        // Keeping pointer to ATVCtrlService
        sp<IATVCtrlService> spATVCtrlService;

        bool mMatvMute;
        float mVoiceVolume;
        uint8_t mSwAgcGain;
        uint8_t mULTotalGain;
        float mMasterVolume;
        float mStreamVolume[android_audio_legacy::AudioSystem::NUM_STREAM_TYPES];

        int mMatvVolume;
        uint32_t mSpeechDrcType ;
        bool mInitDone;

        // data strcutre for volume
        AUDIO_VER1_CUSTOM_VOLUME_STRUCT mVolumeParam;
        AUDIO_CUSTOM_PARAM_STRUCT mSphParamNB;
        AUDIO_CUSTOM_WB_PARAM_STRUCT mSphParamWB;
        int32 mVolumeMax[Num_of_Audio_gain];
        int32 mVolumeMin[Num_of_Audio_gain];
        int32 mVolumeRange[Num_of_Audio_gain];
        int mMicGain[Num_Mic_Gain];
        uint8_t mULTotalGainTable[Num_Mic_Gain];
        int mSideTone[Num_Side_Tone_Gain];
};

}

#endif

