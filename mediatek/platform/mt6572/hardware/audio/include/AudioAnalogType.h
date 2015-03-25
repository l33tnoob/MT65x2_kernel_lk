#ifndef _AUDIO_ANALOG_TYPE_H
#define _AUDIO_ANALOG_TYPE_H

/*!
 *     AudioAnalogType is a public class to let user to use of define enum
 */

class AudioAnalogType
{
    public:
        // gain control for different output
        enum VOLUME_TYPE
        {
            VOLUME_HSOUTL = 0 ,
            VOLUME_HSOUTR,
            VOLUME_HPOUTL,
            VOLUME_HPOUTR,
            VOLUME_SPKL,
            VOLUME_SPKR,
            VOLUME_SPEAKER_HEADSET_R,
            VOLUME_SPEAKER_HEADSET_L,
            VOLUME_IV_BUFFER,
            VOLUME_LINEOUTL,
            VOLUME_LINEOUTR,
            VOLUME_LINEINL,
            VOLUME_LINEINR,
            VOLUME_MICAMPL,
            VOLUME_MICAMPR,
            VOLUME_LEVELSHIFTL,
            VOLUME_LEVELSHIFTR,
            VOLUME_TYPE_MAX
        };

        // mux seleciotn
        enum MUX_TYPE
        {
            MUX_VOICE = 0,
            MUX_AUDIO = 1,
            MUX_IV_BUFFER = 2,
            MUX_LINEIN_STEREO = 3,
            MUX_LINEIN_L = 4,
            MUX_LINEIN_R = 5,
            MUX_LINEIN_AUDIO_MONO = 6,
            MUX_LINEIN_AUDIO_STEREO = 7,
            MUX_IN_MIC1 = 8,
            MUX_IN_MIC2 = 9,
            MUX_IN_MIC3 = 10,
            MUX_IN_LINE_IN = 11,
            MUX_IN_PREAMP_L = 12,
            MUX_IN_PREAMP_R = 13,
            MUX_IN_LEVEL_SHIFT_BUFFER = 14,
            MUX_MUTE = 15,
            MUX_OPEN = 16,
            MAX_MUX_TYPE
        };

        // device power
        enum DEVICE_TYPE
        {
            DEVICE_OUT_EARPIECER = 0,
            DEVICE_OUT_EARPIECEL = 1,
            DEVICE_OUT_HEADSETR = 2,
            DEVICE_OUT_HEADSETL = 3,
            DEVICE_OUT_SPEAKERR = 4,
            DEVICE_OUT_SPEAKERL = 5,
            DEVICE_OUT_SPEAKER_HEADSET_R = 6,
            DEVICE_OUT_SPEAKER_HEADSET_L = 7,
            DEVICE_OUT_LINEOUTR = 8,
            DEVICE_OUT_LINEOUTL = 9,
            DEVICE_2IN1_SPK = 10,
            //DEVICE_IN_LINEINR = 11,
            //DEVICE_IN_LINEINL = 12,
            DEVICE_IN_ADC1 = 13,
            DEVICE_IN_ADC2 = 14,
            DEVICE_IN_ADC3 = 15,
            DEVICE_IN_PREAMP_L = 16,
            DEVICE_IN_PREAMP_R = 17,
            DEVICE_IN_DIGITAL_MIC = 18,
            DEVICE_MAX
        };

        enum DEVICE_SAMPLERATE_TYPE
        {
            DEVICE_OUT_DAC,
            DEVICE_IN_ADC,
            DEVICE_INOUT_MAX
        };


        enum DEVICE_TYPE_SETTING
        {
            DEVICE_PLATFORM_MACHINE,
            DEVICE_PLATFORM,
            DEVICE_MACHINE,
            DEVICE_TYPE_SETTING_MAX
        };

        enum AUDIOANALOG_TYPE
        {
            AUDIOANALOG_DEVICE,
            AUDIOANALOG_VOLUME,
            AUDIOANALOG_MUX
        };

        enum AUDIOANALOGZCD_TYPE
        {
            AUDIOANALOGZCD_HEADPHONE = 1,
            AUDIOANALOGZCD_HANDSET = 2,
            AUDIOANALOGZCD_IVBUFFER = 3,
        };

        enum SPEAKER_CLASS
        {
            CLASS_AB =0,
            CLASS_D,
        };
        
        enum AUDIOANALOG_COMMAND
        {
            SET_SPEAKER_CLASS = 0,
            GET_SPEAKER_CLASS = 1,
            SET_CURRENT_SENSING = 2,
            SET_CURRENT_SENSING_PEAK_DETECTOR = 3,
        };

};

const char kAudioAnalogMuxTypeName[AudioAnalogType::MAX_MUX_TYPE][32] =
{
    "MUX_VOICE", "MUX_AUDIO", "MUX_IV_BUFFER", "MUX_LINEIN_STEREO",
    "MUX_LINEIN_L", "MUX_LINEIN_R", "MUX_LINEIN_AUDIO_MONO", "MUX_LINEIN_AUDIO_STEREO",
    "MUX_IN_MIC1", "MUX_IN_MIC2", "MUX_IN_MIC3", "MUX_IN_LINE_IN",
    "MUX_IN_PREAMP_L", "MUX_IN_PREAMP_R", "MUX_IN_LEVEL_SHIFT_BUFFER", "MUX_MUTE", "MUX_OPEN"
};

const char kAudioAnalogDeviceTypeName[AudioAnalogType::DEVICE_MAX][32] =
{
    "DEVICE_OUT_EARPIECER", "DEVICE_OUT_EARPIECEL", "DEVICE_OUT_HEADSETR", "DEVICE_OUT_HEADSETL",
    "DEVICE_OUT_SPEAKERR", "DEVICE_OUT_SPEAKERL", "DEVICE_OUT_SPEAKER_HEADSET_R", "DEVICE_OUT_SPEAKER_HEADSET_L",
    "DEVICE_OUT_LINEOUTR", "DEVICE_OUT_LINEOUTL", "DEVICE_2IN1_SPK", "DEVICE_IN_LINEINR",
    "DEVICE_IN_LINEINL", "DEVICE_IN_ADC1", "DEVICE_IN_ADC2", "DEVICE_IN_ADC3",
    "DEVICE_IN_PREAMP_L", "DEVICE_IN_PREAMP_R", "DEVICE_IN_DIGITAL_MIC"
};


/*!
 *     AnalogBlockAttribute is a public class to define device has mux and store mux selection
 */
class AnalogBlockAttribute
{
    public:
        bool mMuxExist;
        int mMuxSelect;
        int mEnable;
        int mFrequency;
};

/*!
 *     AnalogVolumeAttribute is a public class to define device has PGA or volume part  and store volume
 */
class AnalogVolumeAttribute
{
    public:
        bool mVolumeEnable;
        int mVolume;
};

#endif
