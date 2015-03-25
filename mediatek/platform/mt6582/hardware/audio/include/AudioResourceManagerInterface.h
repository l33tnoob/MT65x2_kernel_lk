#ifndef _AUDIO_RESOURCE_MANAGER_INTERFACE_H
#define _AUDIO_RESOURCE_MANAGER_INTERFACE_H

#include "AudioUtility.h"
#include "AudioType.h"

class AudioResourceManagerInterface
{
    public:

        enum AUDIO_LOCK_TYPE
        {
            AUDIO_HARDWARE_LOCK = 0,
            AUDIO_MODE_LOCK,
            AUDIO_VOLUME_LOCK,
            AUDIO_STREAMOUT_LOCK,
            AUDIO_STREAMINMANAGER_LOCK ,
            AUDIO_STREAMINMANAGERCLIENT_LOCK ,
            NUM_OF_AUDIO_LOCK
        };

        enum AUDIO_CLOCK_TYPE
        {
            CLOCK_AUD_CORE = 0,
            CLOCK_AUD_AFE ,
            CLOCK_AUD_ADC,
            CLOCK_AUD_I2S,
            CLOCK_AUD_ANA,
            CLOCK_AUD_LINEIN,
            CLOCK_AUD_HDMI,
            CLOCK_TYPE_MAX
        };

        enum DEVICE_SAMPLERATE_TYPE
        {
            DEVICE_OUT_DAC,
            DEVICE_IN_ADC,
            DEVICE_INOUT_MAX
        };

        /**
        * a function for ~AudioResourceManagerInterface destructor
        */
        virtual ~AudioResourceManagerInterface() {};

        /**
        * a function for tell AudioResourceManager output device, usually use for output routing
        * outdevice change may also effetc input device , take care of this.
        * @param new_device
        */
        virtual status_t setDlOutputDevice(uint32 new_device) = 0;

        /**
        * a function for tell AudioResourceManager input device, usually use for input routing
        * @param new_device
        */
        virtual status_t setUlInputDevice(uint32 new_device) = 0;

        /**
        * a function for tell AudioResourceManager ionput source, usually use for input routing
        * @param mDevice
        */
        virtual status_t setUlInputSource(uint32 Source) = 0;

        /**
         * a function for set AudioResourceManager AudioMode
         * @param mNewMode
         */
        virtual status_t SetAudioMode(audio_mode_t mNewMode) = 0;

        /**
         * a function for get AudioResourceManager AudioMode
         * @param mNewMode
         */
        virtual audio_mode_t GetAudioMode() = 0;

        /**
         * a function for tell AudioResourceManager select StartOutputDevice
         */
        virtual status_t StartOutputDevice() = 0;

        /**
         * a function for tell AudioResourceManager select StopOutputDevice
         */
        virtual status_t StopOutputDevice() = 0;

        /**
         * a function for tell AudioResourceManager select outputdevice
         */
        virtual status_t SelectOutputDevice(uint32_t new_device) = 0;

        /**
         * a function for tell AudioResourceManager select StartnputDevice
         */
        virtual status_t StartInputDevice() = 0;

        /**
         * a function for tell AudioResourceManager select StopInputDevice
         */
        virtual status_t StopInputDevice() = 0;

        /**
         * a function for tell AudioResourceManager select inputdevice
         */
        virtual status_t SelectInputDevice(uint32_t device)  = 0;

        /**
         * a function for tell AudioResourceManager get current output device
         */
        virtual uint32 getDlOutputDevice(void) = 0;

        /**
         * a function for tell AudioResourceManager get current input device
         */
        virtual uint32 getUlInputDevice(void) = 0;

        /**
         * a function for tell AudioResourceManager to aquire lock ,Timeout in milisecond
         * if Timeout is not set , it will wait until lock is acquired
         * @param AudioLockType
         * @param mTimeout
         */
        virtual status_t EnableAudioLock(int AudioLockType, int mTimeout = 0) = 0;

        /**
         * a function for tell AudioResourceManager to release lock
         * @param AudioLockType
         */
        virtual status_t DisableAudioLock(int AudioLockType) = 0;

        /**
         * a function for tell AudioResourceManager to requset clock
         * @param AudioLockType
         */
        virtual status_t EnableAudioClock(int AudioClockType, bool Enable) = 0;

        /**
        * a  function to setParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , int command2 , unsigned int data) = 0;

        /**
        * a function to setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , void *data) = 0;

        /**
        * a function to getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy size of data
        */
        virtual int getParameters(int command1 , int command2 , void *data) = 0;

        /**
        * a function to dump AudioResourceManager , can base on command1 to dump different information
        * @param command1
        * @param command2
        */
        virtual int dump(int command1) = 0;

        /**
        * a function to check if wired-headset on
        */
        virtual bool IsWiredHeadsetOn(void) = 0;

        /**
        * a function to return Mode Incall
        */
        virtual bool IsModeIncall(void) = 0;

        /**
        * a function for to get MIC digital gain for HD Record
        */
        virtual long GetSwMICDigitalGain(void) = 0;

        /**
        * a function for to get UL total gain for BesRecord
        */
        virtual uint8_t GetULTotalGainValue(void) = 0;

        /**
         * a function for set mic inverse
         */
        virtual status_t SetMicInvserse(bool bEnable) = 0;


        /**
         * a function for set AFE_ON
         */
        virtual status_t SetAfeEnable(const bool bEnable) = 0;

        /**
        * a function for get mic inverse
        */
        virtual bool GetMicInvserse(void) = 0;
        /*
         * a function for set Analog Frequence
         */
        virtual status_t SetFrequency(int DeviceType, unsigned int frequency) = 0;

        /*
         * a function for lock debug
         */
        virtual void SetDebugLine(int line) = 0;

        virtual status_t AddSubSPKToOutputDevice() = 0;
};

#endif
