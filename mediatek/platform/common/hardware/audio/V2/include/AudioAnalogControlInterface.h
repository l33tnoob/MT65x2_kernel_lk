#ifndef _AUDIO_ANALOG_CONTROL_INTERFACE_H
#define _AUDIO_ANALOG_CONTROL_INTERFACE_H

#include "AudioStreamAttribute.h"
#include "AudioAnalogType.h"
#include <hardware_legacy/AudioSystemLegacy.h>
#include "AudioType.h"


class AudioAnalogControlInterface
{
    public:
        /**
        * a destuctor for AudioAnalogControl, do nothing.
        */
        virtual ~AudioAnalogControlInterface() {};

        /**
        * a destuctor for AudioAnalogControl, do nothing.
        * @return status_t
        */
        virtual status_t InitCheck() = 0;

        /**
        * a basic function for SetAnalogGain for different Volume Type
        * @param VoleumType value want to set to analog volume
        * @param volume function of analog gain , value between 0 ~ 255 , means degrade gain in db
        * @return status_t
        */
        virtual status_t SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume) = 0;

        /**
        * a basic function for GetAnalogGain for different Volume Type
        * @param VoleumType value want to get analog volume
        * @return int
        */
        virtual int GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType) = 0;

        /**
        * a basic function for SetFrequency for different Device Type , like input and output
        * @param DeviceType
        * @param frequency
        * @return status_t
        */
        virtual status_t SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency) = 0;

        /**
        * a basic function fo SetAnalogMute, if provide mute function of hardware.
        * @param VoleumType value want to set to analog volume
        * @param mute of volume type
        * @return status_t
        */
        virtual status_t SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute) = 0;

        /**
        * a basic function fo AnalogOpen, open analog power , could seperate with DAC Buffer and external device
        * @param DeviceType analog part power
        * @return status_t
        */
        virtual status_t AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting) = 0;

        /**
        * a basic function fo AnalogClose, ckose analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        virtual status_t AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting) = 0;

        /**
        * a basic function fo getting select mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @param MuxType analog mux selection
        * @return status_t
        */
        virtual AudioAnalogType::MUX_TYPE AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType) = 0;

        /**
        * a basic function fo select mux of device type, not all device may have mux
        * if select a device with no mux support , report error.
        * @param DeviceType analog part
        * @param MuxType analog mux selection
        * @return status_t
        */
        virtual status_t AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType) = 0;

        /**
        * a basic function fo get analog state m
        * if device is power on , return true, else return false.
        * @param DeviceType analog part
        * @return bool
        */
        virtual bool GetAnalogState(AudioAnalogType::DEVICE_TYPE DeviceType) = 0;
        /**
        * a  function for setParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , int command2 , unsigned int data) = 0 ;

        /**
        * a function for setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , void *data) = 0;

        /**
        * a function fo getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy_size
        */
        virtual int getParameters(int command1 , int command2 , void *data) = 0;

        /**
        * a function to getUplinState
        * @return bool
        */
        virtual bool AnalogUplinkEnable(void) = 0 ;

        /**
        * a function to getDLlinState
        * @return bool
        */
        virtual bool AnalogDLlinkEnable(void) = 0;

        /**
        * a function to Set mic1 and mic3 inverse , only work when dual mic
        * @return bool
        */
        virtual status_t SetAnalogPinmuxInverse(bool bEnable) = 0 ;

        /**
        * a function to GetPinmuxinverse
        * @return bool
        */
        virtual bool GetAnalogPinmuxInverse(void) = 0;

        /**
        * a function to setmode to tell analogcontrol
        * @return bool
        */
        virtual status_t setmode(audio_mode_t mode) = 0;

        /**
        * a function for fade out / fade in
        * @param sample rate
        * @return status_t
        */
        virtual status_t FadeOutDownlink(uint16_t sample_rate) = 0;
        virtual status_t FadeInDownlink(uint16_t sample_rate) = 0;
        virtual status_t SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value) = 0;

        virtual bool GetAnalogSpkOCState(void) = 0;
        /**
        * a basic function fo AnalogOpenForAddSPK, open analog power , could seperate with DAC Buffer and external device
        * @param DeviceType analog part power
        * @return status_t
        */
        virtual status_t AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting) = 0;

        /**
        * a basic function fo AnalogCloseForSubSPK, ckose analog power
        * @param DeviceType analog part power
        * @return status_t
        */
        virtual status_t AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::DEVICE_TYPE_SETTING Type_setting) = 0;
};

#endif
