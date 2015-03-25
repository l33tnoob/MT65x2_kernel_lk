#ifndef ANDROID_AUDIO_PLATFORM_DEVICE_H
#define ANDROID_AUDIO_PLATFORM_DEVICE_H

#include "AudioAnalogType.h"
#include "AudioAnalogControlInterface.h"
#include "AudioAnalogReg.h"

namespace android
{

//!  A AudioPlatformDevice
/*!
  this class is use for control DAC , buffer power control and mux , volume setting
*/

class AudioPlatformDevice
{
    public:

        AudioPlatformDevice();

        /**
        * AudioMachineDevice InitCheck
        * @return status_t
        */
        status_t InitCheck();

        /**
        * a basic function for SetAnalogGain for different Volume Type
        * @param VoleumType value want to set to analog volume
        * @param volume function of analog gain , value between 0 ~ 255
        * @return status_t
        */
        status_t SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume);

        /**
        * a basic function for SetFrequency for different Device Type , like input and output
        * @param DeviceType
        * @param frequency
        * @return status_t
        */
        status_t SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency);

        /**
        * a basic function fo SetAnalogMute, if provide mute function of hardware.
        * @param VoleumType value want to set to analog volume
        * @param mute of volume type
        * @return status_t
        */
        status_t SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute);

        /**
        * a basic function fo AnalogOpen, open analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        status_t AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType);

        /**
        * a basic function fo AnalogClose, ckose analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        status_t AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType);

        /**
        * a basic function fo select mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @param MuxType analog mux selection
        * @return status_t
        */
        status_t AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType);

        /**
        * a  function for setParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return status_t
        */
        status_t setParameters(int command1 , int command2 , unsigned int data);

        /**
        * a function for setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        status_t setParameters(int command1 , void *data);

        /**
        * a function fo getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy_size
        */
        int getParameters(int command1 , int command2 , void *data);

        /**
        * a function for fade out / fade in
        * @param sample rate
        * @return status_t
        */
        status_t FadeOutDownlink(uint16_t sample_rate);
        status_t FadeInDownlink(uint16_t sample_rate);
        status_t SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value);
        status_t AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType);
        status_t AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType);
    private:
        uint32 GetDLFrequency(unsigned int frequency);
        uint32 GetULFrequency(unsigned int frequency);
        uint32 GetULFrequencyGroup(unsigned int frequency);
        bool GetDownLinkStatus(void);
        bool GetULinkStatus(void);
        status_t TopCtlChangeTrigger(void);
        status_t DCChangeTrigger(void);
        uint32 GetDLNewIFFrequency(unsigned int frequency);
        uint32 GetULNewIFFrequency(unsigned int frequency);
        /**
        * AnalogBlockAttribute to telling Device Type has mux and now mux selection
        * @see AudioAnalogType::DEVICE_MAX
        */
        AnalogBlockAttribute mBlockAttribute[AudioAnalogType::DEVICE_MAX];
        uint32 mBlockSampleRate[AudioAnalogType::DEVICE_INOUT_MAX];

        /**
        * AnalogBlockAttribute to telling Device Type has mux and now mux selection
        * @see AudioAnalogType::VOLUME_TYPE_MAX
        */
        AnalogVolumeAttribute mVolumeAttribute[AudioAnalogType::VOLUME_TYPE_MAX];
        int mHpLeftDcCalibration, mHpRightDcCalibration;
        AudioAnalogReg *mAudioAnalogReg;
        AudioLock mLock;

};

}

#endif
