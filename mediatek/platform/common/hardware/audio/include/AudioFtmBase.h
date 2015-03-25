#ifndef ANDROID_AUDIO_FTM_BASE_H
#define ANDROID_AUDIO_FTM_BASE_H

#include <sys/types.h>

namespace android
{

enum FMTX_Command
{
    FREQ_NONE = 0,
    FREQ_1K_HZ,
    FREQ_2K_HZ,
    FREQ_3K_HZ,
    FREQ_4K_HZ,
    FREQ_5K_HZ,
    FREQ_6K_HZ,
    FREQ_7K_HZ,
    FREQ_8K_HZ,
    FREQ_9K_HZ,
    FREQ_10K_HZ,
    FREQ_11K_HZ,
    FREQ_12K_HZ,
    FREQ_13K_HZ,
    FREQ_14K_HZ,
    FREQ_15K_HZ
};


enum UL_SAMPLERATE_INDEX
{
    UPLINK8K = 0,
    UPLINK16K,
    UPLINK32K,
    UPLINK48K,
    UPLINK_UNDEF
};


// for afe loopback
#define MIC1_OFF  0
#define MIC1_ON   1
#define MIC2_OFF  2
#define MIC2_ON   3


// for acoustic loopback
#define ACOUSTIC_STATUS   -1
#define DUAL_MIC_WITHOUT_DMNR_ACS_OFF 0
#define DUAL_MIC_WITHOUT_DMNR_ACS_ON  1
#define DUAL_MIC_WITH_DMNR_ACS_OFF   2
#define DUAL_MIC_WITH_DMNR_ACS_ON    3



class AudioFtmBase
{
    public:
        virtual ~AudioFtmBase();
        static AudioFtmBase *createAudioFtmInstance();

        /// Codec
        virtual void Audio_Set_Speaker_Vol(int level);
        virtual void Audio_Set_Speaker_On(int Channel);
        virtual void Audio_Set_Speaker_Off(int Channel);
        virtual void Audio_Set_HeadPhone_On(int Channel);
        virtual void Audio_Set_HeadPhone_Off(int Channel);
        virtual void Audio_Set_Earpiece_On();
        virtual void Audio_Set_Earpiece_Off();


        /// for factory mode & Meta mode (Analog part)
        virtual void FTM_AnaLpk_on(void);
        virtual void FTM_AnaLpk_off(void);


        /// Output device test
        virtual int RecieverTest(char receiver_test);
        virtual int LouderSPKTest(char left_channel, char right_channel);
        virtual int EarphoneTest(char bEnable);
        virtual int EarphoneTestLR(char bLR);


        /// Speaker over current test
        virtual int Audio_READ_SPK_OC_STA(void);
        virtual int LouderSPKOCTest(char left_channel, char right_channel);

        virtual int HDMI_SineGenPlayback(bool bEnable, int dSamplingRate);

        /// Loopback // TODO: Add in platform!!!
        virtual int PhoneMic_Receiver_Loopback(char echoflag);
        virtual int PhoneMic_EarphoneLR_Loopback(char echoflag);
        virtual int PhoneMic_SpkLR_Loopback(char echoflag);
        virtual int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic);
        virtual int HeadsetMic_SpkLR_Loopback(char echoflag);

        virtual int PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output);


        /// FM / mATV
        virtual int FMLoopbackTest(char bEnable);

        virtual int Audio_FM_I2S_Play(char bEnable);
        virtual int Audio_MATV_I2S_Play(int enable_flag);
        virtual int Audio_FMTX_Play(bool Enable, unsigned int Freq);

        virtual int ATV_AudPlay_On(void);
        virtual int ATV_AudPlay_Off(void);
        virtual unsigned int ATV_AudioWrite(void *buffer, unsigned int bytes);


        /// HDMI
        int Audio_HDMI_Play(bool Enable, unsigned int Freq);


        /// Vibration Speaker // MTK_VIBSPK_SUPPORT??
        virtual int      SetVibSpkCalibrationParam(void *cali_param);
        virtual uint32_t GetVibSpkCalibrationStatus();
        virtual void     SetVibSpkEnable(bool enable, uint32_t freq);
        virtual void     SetVibSpkRampControl(uint8_t rampcontrol);

        virtual bool     ReadAuxadcData(int channel, int *value);



    protected:
        AudioFtmBase();

};

} // end namespace android

#endif // end of ANDROID_AUDIO_FTM_BASE_H
