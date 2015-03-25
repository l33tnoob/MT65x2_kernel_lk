#ifndef _AUDIO_DIGITAL_CONTROL_H
#define _AUDIO_DIGITAL_CONTROL_H

#include "AudioUtility.h"
#include "AudioDigitalControlInterface.h"
#include "AudioStreamAttribute.h"
#include "AudioInterConnection.h"
#include "AudioDigitalType.h"
#include "AudioAfeReg.h"

namespace android
{
//this class only control digital part  , interconnection is not incldue.
class AudioDigitalControl : public AudioDigitalControlInterface
{
    public:
        static AudioDigitalControl *getInstance();
        ~AudioDigitalControl() {};
        // virtual function implementation
        virtual status_t InitCheck();

        virtual status_t SetMemBufferSize(uint32 Memory_Interface, uint32 BufferSzie);
        virtual uint32 GetMemBufferSize(uint32 Memory_Interface);

        virtual status_t AllocateMemBufferSize(uint32 Memory_Interface);
        virtual status_t FreeMemBufferSize(uint32 Memory_Interface);

        virtual status_t SetMemIfSampleRate(uint32 Memory_Interface, uint32 SampleRate);
        virtual uint32 GetMemIfSampleRate(uint32 Memory_Interface);

        virtual status_t SetMemIfChannelCount(uint32 Memory_Interface, uint32 Channels);
        virtual uint32 GetMemIfChannelCount(uint32 Memory_Interface);

        virtual status_t SetMemIfEnable(uint32 Memory_Interface, uint32 State);

        virtual status_t SetMemIfInterruptSample(uint32 Memory_Interface, uint32 SampleCount);
        virtual uint32 GetMemIfInterruptSample(uint32 Memory_Interface) ;

        virtual status_t SetinputConnection(uint32 ConnectionState, uint32 Input , uint32 Output);

        virtual status_t SetIrqMcuEnable(AudioDigitalType::IRQ_MCU_MODE Irqmode, bool bEnable);
        virtual status_t SetIrqMcuSampleRate(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 SampleRate);
        virtual status_t SetIrqMcuCounter(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 Counter);

        virtual status_t SetMicinputInverse(bool bEnable);

        /**
        * a function to GetIrqStatus
        * @param Irqmode
        * @see AudioDigitalType::IRQ_MCU_MODE
        * @return status_t
        */
        virtual status_t GetIrqStatus(AudioDigitalType::IRQ_MCU_MODE Irqmode, AudioIrqMcuMode *Mcumode);

        /**
        * a function fo Set Attribute of I2SDacOut
        * @param sampleRate
        * @return status_t
        */
        virtual status_t SetI2SDacOutAttribute(uint32_t sampleRate);

        /**
        * a function fo SetI2SDacOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t SetI2SDacOut(AudioDigtalI2S *mDigitalI2S);

        /**
        * a function fo GetI2SDacOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t GetI2SDacOut(AudioDigtalI2S *mDigitalI2S);

        /**
        * a function fo SetI2SDacEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SDacEnable(bool bEnable);

        /**
        * a function fo SetI2SAdcIn
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t SetI2SAdcIn(AudioDigtalI2S *mDigitalI2S);

        /**
        * a function fo GetI2SAdcIn
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t GetI2SAdcIn(AudioDigtalI2S *mDigitalI2S);

        /**
        * a function fo SetI2SAdcEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SAdcEnable(bool bEnable);

        /**
        * a function fo Set2ndI2SOutAttribute
        * @param sampleRate
        * @return status_t
        */
        virtual status_t Set2ndI2SOutAttribute(uint32_t sampleRate);

        /**
        * a function fo Set2ndI2SOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t Set2ndI2SOut(AudioDigtalI2S *mDigitalI2S);

        /**
        * a function fo Get2ndI2SOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t Get2ndI2SOut(AudioDigtalI2S *mDigitalI2S);

        /**
        * a function fo Set2ndI2SEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t Set2ndI2SEnable(bool bEnable);

        /**
        * a function fo SetI2SSoftReset
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SSoftReset(bool bEnable);

        /**
        * a function to GetAfeOn , this is a afe bit , turn on will caise digital part start.
        * @param Irqmo
        * @return status_t
        */

        virtual status_t SetAfeEnable(bool  bEnable);

        /**
        * a function to GetAfeOn , this is a afe bit , turn on will caise digital part start.
        * @param Irqmo
        * @return status_t
        */
        virtual bool GetAfeEnable(bool  bEnable);

        /**
        * a function fo get digital block
        * @param block
        * @param bEnable
        * @return status_t
        */
        virtual int GetDigitalBlockState(int block);

        /**
        * a function fo SetDAIBBT attribute
        * @param DAIBT
        * @return status_t
        */
        virtual status_t SetDAIBBT(AudioDigitalDAIBT *DAIBT);

        /**
        * a function fo GetDAIBTOut attribute
        * @param DAIBT
        * @return status_t
        */
        virtual status_t GetDAIBTOut(AudioDigitalDAIBT *DAIBT);

        /**
        * a function fo SetDAIBTEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetDAIBTEnable(bool bEnable);

        virtual void BT_SCO_SetMode(uint32_t mode);

        virtual uint32_t BT_SCO_GetMode(void);

        /**
        * this function will  tansform SampleRate into hardware samplerate format
        * @return AudioMEMIFAttribute::SAMPLINGRATE
        */
        virtual AudioMEMIFAttribute::SAMPLINGRATE SampleRateTransform(uint32 SampleRate);
        /**
        * this function will  tansform SampleRate into hardware samplerate format for I2S
        * @return AudioDigtalI2S::I2S_SAMPLERATE
        */
        virtual AudioDigtalI2S::I2S_SAMPLERATE I2SSampleRateTransform(unsigned int sampleRate);

        /**
        * this functionis to set Hw digital gain
        * @return status_t
        */
        virtual status_t SetHwDigitalGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType);

        /**
        * this functionis to set Hw digital current gain
        * @return status_t
        */
        virtual status_t SetHwDigitalCurrentGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType);

        /**
        * this functionis to set HwDigitalGainMode with sampleRate and step with how many samples
        * @param GainType
        * @param SampleRate
        * @param SamplePerStep
        * @return status_t
        */
        virtual status_t SetHwDigitalGainMode(AudioDigitalType::Hw_Digital_Gain GainType, AudioMEMIFAttribute::SAMPLINGRATE SampleRate,  uint32 SamplePerStep);


        /**
        * this functionis to set HwDigitalGainMode with sampleRate and step with how many samples
        * @param GainType
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHwDigitalGainEnable(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable);
        /**
        * this functionis to set HwDigitalGainMode with sampleRate and step with how many samples, w/o Ramp Up
        * @param GainType
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHwDigitalGainEnableByPassRampUp(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable);

        /**
        * this functionis to set modem pcm attribute.
        * @param modem_index
        * @param p_modem_pcm_attribute
        * @return status_t
        */
        virtual status_t SetModemPcmConfig(modem_index_t modem_index, AudioDigitalPCM *p_modem_pcm_attribute);

        /**
        * this functionis to set modem pcm enable/disable
        * @param modem_index
        * @param modem_pcm_on
        * @return status_t
        */
        virtual status_t SetModemPcmEnable(modem_index_t modem_index, bool modem_pcm_on);

        /**
        * this functionis to enable/disable side tone filter
        * @param stf_on
        * @return status_t
        */
        virtual status_t EnableSideToneFilter(bool stf_on);

        /**
        * this functionis to check if digital dl pth is change ex: Dac to BT....
        * @param PreDevice
        * @param NewDevice
        * @return bool
        */
        virtual bool CheckDlDigitalChange(uint32_t PreDevice, uint32_t NewDevice);

        /**
        * this functionis to set Hw digital gain
        * @return status_t
        */
        virtual status_t EnableSideToneHw(uint32 connection , bool direction, bool  Enable);

        
        /**
	 * this functionis to set Sinetone Ouput LR
        * @return status_t
        */
        virtual status_t SetSinetoneOutputLR(bool bLR);

        /**
        * this functionis to get device by device
        * @param Device
        * @return uint32
        */
        virtual uint32 DlPolicyByDevice(uint32_t Device);

        /**
        * a function to Set MrgI2S Enable
        * @param bEnable
        * @see true or false
        * @param sampleRate
        * @return status_t
        */
        virtual status_t SetMrgI2SEnable(bool bEnable, unsigned int sampleRate);

        /**
        * a function to Reset MrgIf of FM Chip
        * @return status_t
        */
        virtual status_t ResetFmChipMrgIf(void);

        /**
        * a function to Set I2SASRC Config
        * @param bIsUseASRC
        * @param dToSampleRate
        * @return status_t
        */
        virtual status_t SetI2SASRCConfig(bool bIsUseASRC, unsigned int dToSampleRate);
        /**
        * a function fo SetI2SASRCEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SASRCEnable(bool bEnable);
        /**
        * a function to Set Set2ndI2SIn Config
        * @param sampleRate
        * @param bIsSlaveMode
        * @return status_t
        */
        virtual status_t Set2ndI2SInConfig(unsigned int sampleRate, bool bIsSlaveMode);
        /**
        * a function fo Set2ndI2SIn
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t Set2ndI2SIn(AudioDigtalI2S *mDigitalI2S);
        /**
        * a function fo Set2ndI2SInEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t Set2ndI2SInEnable(bool bEnable);
        /**
        * a function fo Set2ndI2SOutEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t Set2ndI2SOutEnable(bool bEnable);

        virtual status_t SetMemIfFetchFormatPerSample(uint32 InterfaceType, AudioMEMIFAttribute::FETCHFORMATPERSAMPLE eFetchFormat);
        virtual AudioMEMIFAttribute::FETCHFORMATPERSAMPLE GetMemIfFetchFormatPerSample(uint32 InterfaceType);
        virtual status_t SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT ConnectionFormat,AudioDigitalType::InterConnectionOutput Output);
        virtual AudioDigitalType::OUTPUT_DATA_FORMAT GetoutputConnectionFormat(AudioDigitalType::InterConnectionOutput Output);

    private:

        /**
        * this function will check all afe digital block return true if any digital block is running
        * @return bool
        */
        bool GetAfeDigitalStatus(void);
        // function implememnt
        status_t SetMemoryPathEnable(uint32 Memory_Interface, bool bEnable);
        status_t SetMemDuplicateWrite(uint32 Memory_Interface, int dupwrite);
        status_t SetMemMonoChannel(uint32 Memory_Interface, bool channel);

        static AudioDigitalControl *UniqueDigitalInstance;
        AudioDigitalControl();
        AudioDigitalControl(const AudioDigitalControl &);             // intentionally undefined
        AudioDigitalControl &operator=(const AudioDigitalControl &);  // intentionally undefined

        // mAudioMEMIF information
        AudioAfeReg *mAfeReg;

        uint32 mFd;
        AudioInterConnection *mInterConnectionInstance;
        AudioIrqMcuMode mAudioMcuMode[AudioDigitalType::NUM_OF_IRQ_MODE];
        AudioMEMIFAttribute mAudioMEMIF[AudioDigitalType::NUM_OF_DIGITAL_BLOCK];

        AudioDigtalI2S mDacI2SOut;
        AudioDigtalI2S mAdcI2SIn;
        AudioDigtalI2S m2ndI2S;
        AudioDigitalPCM mModemPcm1;  // slave only pcm
        AudioDigitalPCM mModemPcm2;  // slave,master pcm
        AudioDigitalDAIBT mDaiBt;
        AudioMrgIf mMrgIf;
        Mutex mLock;

        int mAudioDigitalBlock[AudioDigitalType::NUM_OF_DIGITAL_BLOCK];
        bool mMicInverse;
        bool mSideToneFilterOn;
        bool mUseI2SADCInStatus;
        uint32 BT_mode;
};

}

#endif
