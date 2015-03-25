#ifndef ANDROID_AUDIO_MTK_VOLUME_INTERFACE_H
#define ANDROID_AUDIO_MTK_VOLUME_INTERFACE_H

#include "AudioType.h"
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include <utils/String8.h>

//!  AAudioMTKVolumeInterface interface
/*!
  this class is hold for volume controlb base on mode and device.
  need to take care both input and output volume.
*/

class AudioMTKVolumeInterface
{
    public:
        /**
        * virtual destrutor
        */
        virtual ~AudioMTKVolumeInterface() {};

        /**
        * check volume control init done.
        * @return status_t
        */
        virtual status_t    initCheck() = 0;

        /**
        *  volume controller init funciton
        * @return status_t
        */
        virtual status_t    initVolumeController() = 0;

        /**
        *  volume controller setMasterVolume, usually set master volume is by setting analog gain,
        * @param v
        * @param mode
        * @param routes
        * @return status_t
        */
        virtual status_t setMasterVolume(float v, audio_mode_t mode, uint32_t devices) = 0 ;

        /**
        *  volume controller GetMasterVolume
        * @return mastervolume
        */
        virtual float getMasterVolume() = 0 ;

        /**
        *  volume controller setVoiceVolume, usually set voice volume is use by incall mode
        * @param v
        * @param mode
        * @param routes
        * @return status_t
        */
        virtual status_t setVoiceVolume(float v, audio_mode_t mode, uint32_t devices) = 0 ;

        /**
        *  volume controller getVoiceVolume
        * @return VoiceVolume
        */
        virtual float getVoiceVolume(void) = 0;


        /**
        *  volume controller setStreamVolume, this function basicaaly use for FM or MATV
        * which need set volume by hardware, diogital is set by audiopolicymanager.
        * @param stream
        * @param v
        * @return status_t
        */
        virtual status_t setStreamVolume(int stream, float v) = 0 ;

        /**
        *  volume controller setStreamMute
        * @param stream
        * @param mute
        * @return status_t
        */
        virtual status_t setStreamMute(int stream, bool mute) = 0 ;

        /**
        *  volume controller getStreamVolume
        * @param stream
        * @return status_t
        */
        virtual float getStreamVolume(int stream) = 0;

        /**
        *  volume controller SetLineInPlaybackGain
        * should depend on different usage , FM ,MATV and output device to setline in gain
        * @param type
        * @return status_t
        */
        virtual status_t SetLineInPlaybackGain(int type) = 0;

        /**
        *  volume controller SetLineInRecordingGain
        * should depend on different usage , FM ,MATV and output device to setline in gain
        * @param type
        * @return status_t
        */
        virtual status_t SetLineInRecordingGain(int type) = 0;

        /**
        * volume controller SetSideTone
        * base on mode gain and route to decide sidetone gain
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t SetSideTone(uint32_t Mode, uint32_t gain) = 0 ;

        /**
          * volume controller setMatvVolume
          * set Matv Volume
          * @param volume
          * @return bool
          */
        virtual bool setMatvVolume(int volume) = 0 ;

        /**
          * volume controller SetMatvMute
          * Set Matv Mute
          * @param b_mute
          * @return bool
          */
        virtual bool SetMatvMute(bool b_mute) = 0 ;

        /**
          * volume controller GetSideToneGain
          * base on output device get sidetone gain
          * @param device
          * @return gain
          */
        virtual uint32_t GetSideToneGain(uint32_t device) = 0 ;

        /**
        * volume controller SetMicGain
        * base on mode gain and route to decide sidetone gain
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t SetMicGain(uint32_t Mode, uint32_t gain) = 0 ;

        /**
        * volume controller SetULTotalGain
        * base on mode and gain
        * @param Mode
        * @param Gain
        * @return status_t
        */
        virtual status_t SetULTotalGain(uint32_t Mode, unsigned char Volume) = 0 ;

        /**
        * volume controller SetMicGain
        * base on mode gain and route to decide sidetone gain
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t ApplyMicGain(uint32_t MicType, int mode) = 0 ;

        /**
        * volume controller SetDigitalHwGain
        * base on mode gain and route to decide sidetone GAIN_MUTE
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t SetDigitalHwGain(uint32_t Mode, uint32_t Gain , uint32_t devices) = 0 ;

        /**
        *  volume controller GetSWMICGain
        * get MIC software digital gain for HD record library
        */
        virtual uint8_t GetSWMICGain(void) = 0 ;

        /**
        *  volume controller GetULTotalGain
        * get MIC software digital gain for BesRecord library
        */
        virtual uint8_t GetULTotalGain(void) = 0 ;


        /**
        * volume controller Set modem DL gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdDlGain(int  Gain) = 0 ;


        /**
        * volume controller Set modem DL Ehn gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdDlEhn1Gain(int Gain) = 0 ;

        /**
        * volume controller Set modem Ul gain
        * @param Gain
        * @return status_t
        */
        virtual void ApplyMdUlGain(int  Gain) = 0 ;

        /**
        * volume controller map volume to digital gain
        * @param Gain
        * @return digital gain
        */
        virtual uint16_t MappingToDigitalGain(unsigned char Gain) = 0 ;

        /**
        * volume controller map volume to PGA gain
        * @param Gain
        * @return PGA gain
        */
        virtual uint16_t MappingToPGAGain(unsigned char Gain) = 0;

        /**
        * volume controller SetMicGainTuning
        * base on mode gain and route to decide sidetone gain
        * @param Mode
        * @param Gain
        * @param routes
        * @return status_t
        */
        virtual status_t SetMicGainTuning(uint32_t Mode, uint32_t gain) = 0 ;

        virtual status_t ApplySideTone(uint32_t Mode) = 0;
};

#endif
