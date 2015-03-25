#ifndef _AUDIO_ANALOG_CONTROL_EXT_H
#define _AUDIO_ANALOG_CONTROL_EXT_H

#include "AudioAnalogControlInterface.h"
#include "AudioAnalogType.h"
#include "AudioAnalogControl.h"

namespace android
{

//!  A Analog control class
/*!
  this class is hold  the operation of analog domain control , incluide power on/off
  and external device power and mux is included.
*/

class AudioAnalogControlExt : public AudioAnalogControlInterface
{
    public:
        /**
        * a basic function to Get AudioAnalogControlExt Instance
        * @return Pointer of AudioAnalog
        */
        static AudioAnalogControlExt *getInstance();

        /**
        * a destuctor for AudioAnalogControlExt, do nothing.
        */
        virtual ~AudioAnalogControlExt();

        /**
        * a destuctor for AudioAnalogControlExt, do nothing.
        * @return status_t
        */
        virtual status_t InitCheck();

        /**
        * a basic function for SetAnalogGain for different Volume Type
        * @param VoleumType value want to set to analog volume
        * @param volume function of analog gain , value between 0 ~ 255
        * @return status_t
        */
        virtual status_t SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume);

        /**
        * a basic function for GetAnalogGain for different Volume Type
        * @param VoleumType value want to get analog volume
        * @return int
        */
        virtual int GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType);

        /**
        * a basic function for SetFrequency for different Device Type , like input and output
        * @param DeviceType
        * @param frequency
        * @return status_t
        */
        virtual status_t SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency);

        /**
        * a basic function for SetAnalogMute, if provide mute function of hardware.
        * @param VoleumType value want to set to analog volume
        * @param mute of volume type
        * @return status_t
        */
        virtual status_t SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute);

        /**
        * a basic function for AnalogOpen, open analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        virtual status_t AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting);

        /**
        * a basic function for AnalogClose, ckose analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        virtual status_t AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting);

        /**
        * a basic function for getting select mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @param MuxType analog mux selection
        * @return status_t
        */
        virtual AudioAnalogType::MUX_TYPE AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType);

        /**
        * a basic function for select mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @param MuxType analog mux selection
        * @return status_t
        */
        virtual status_t AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType);

        /**
        * a basic function for get analog state m
        * if device is power on , return true, else return false.
        * @param DeviceType analog part
        * @return bool
        */
        virtual bool GetAnalogState(AudioAnalogType::DEVICE_TYPE DeviceType);

        /**
        * a  function for setParameters , provide wide usage of analog control
        * @param command1
            * @param command2
            * @param data
            * @return status_t
            */
        virtual status_t setParameters(int command1 , int command2 , unsigned int data);

        /**
        * a function for setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , void *data);

        /**
        * a function for getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy size of data
        */
        virtual int getParameters(int command1 , int command2 , void *data);

        /**
        * a function for get uplink status
        * @return true for uplink enable
        */
        virtual bool AnalogUplinkEnable(void);

        /**
        * a function for get DLink status
        * @return true for DLink enable
        */
        virtual bool AnalogDLlinkEnable(void);

        /**
        * a function to Set mic1 and mic3 inverse , only work when dual mic
        * @return bool
        */
        virtual status_t SetAnalogPinmuxInverse(bool bEnable);

        /**
        * a function to GetPinmuxinverse
        * @return bool
        */
        virtual bool GetAnalogPinmuxInverse(void);

        /**
        * a function to setmode to tell analogcontrol
        * @return bool
        */
        virtual status_t setmode(audio_mode_t mode);

        /**
        * a function for fade out / fade in
        * @param sample rate
        * @return status_t
        */
        virtual status_t FadeOutDownlink(uint16_t sample_rate);
        virtual status_t FadeInDownlink(uint16_t sample_rate);

        virtual status_t SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value);
        virtual bool GetAnalogSpkOCState(void);
        virtual status_t AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting);
        virtual status_t AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting);    
    private:

        /**
        * single instance of AudioAnalogControlExt
        */
        static AudioAnalogControlExt *UniqueAnalogExtInstance;

        /**
        * single instance of AudioAnalogContro
        */
        static AudioAnalogControlInterface *UniqueAnalogInstance;

        /**
        *  private constructor of AudioAnalogControlExt
        */
        AudioAnalogControlExt();

        /**
        * intentionally undefined
        */
        AudioAnalogControlExt(const AudioAnalogControlExt &);

        /**
        * intentionally undefined = operator
        */
        AudioAnalogControlExt &operator=(const AudioAnalogControlExt &);

        /**
        * AnalogBlockAttribute to telling Device Type has mux and now mux selection
        * @see AudioAnalogType::ANA_DEVICE_MAX
        */
        AnalogBlockAttribute mBlockAttribute[AudioAnalogType::DEVICE_MAX];

        /**
        * AnalogVolumeAttribute to telling Device volume and can be mute
        * @see AudioAnalogType::VOLUME_TYPE_MAX
        */
        AnalogVolumeAttribute mVolumeAttribute[AudioAnalogType::VOLUME_TYPE_MAX];
};

}

#endif
