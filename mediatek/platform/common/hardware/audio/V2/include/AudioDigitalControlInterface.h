#ifndef _AUDIO_DIGITAL_CONTROL_INTERFACE_H
#define _AUDIO_DIGITAL_CONTROL_INTERFACE_H

#include "AudioStreamAttribute.h"
#include "AudioDigitalType.h"
#include "AudioType.h"
#include "SpeechType.h"

//!A digital control interface
/*!
  this class is define digital part control interface
*/
namespace android
{

class AudioDigitalControlInterface
{
    public:

        /**
        * a destuctor for AudioDigitalControlInterface
        */

        virtual ~AudioDigitalControlInterface() {};
        /**
        * a destuctor for AudioAnalogControl, do nothing.
        * @return status_t
        */
        virtual status_t InitCheck() = 0;

        /**
        * a function of set several memory interface of buffersize
        * this function only set buffersize but not allocate in practice
        * @param Memory_Interface
        * @param BufferSzie
        * @see AudioDigitalType::Memory_Interface
        * @return status_t
        */
        virtual status_t SetMemBufferSize(uint32 Memory_Interface, uint32 BufferSzie) = 0;

        /**
        * a function of Get several memory interface of buffersize
        * @param Memory_Interface
        * @see AudioDigitalType::Memory_Interface
        * @return status_t
        */
        virtual uint32 GetMemBufferSize(uint32 Memory_Interface) = 0;

        /**
        * a function allocate memory for memory interface
        * @param Memory_Interface
        * @see AudioDigitalType::Memory_Interface
        * @return status_t
        */
        virtual status_t AllocateMemBufferSize(uint32 Memory_Interface) = 0;

        /**
        * a function free memory for memory interface
        * @param Memory_Interface
        * @see AudioDigitalType::Memory_Interface
        * @return status_t
        */
        virtual status_t FreeMemBufferSize(uint32 Memory_Interface) = 0;


        /**
        * a function to set samplerate of memory interface
        * @param Memory_Interface
        * @param SampleRate
        * @see AudioDigitalType::Memory_Interface
        * @return status_t
        */
        virtual status_t SetMemIfSampleRate(uint32 Memory_Interface, uint32 SampleRate) = 0;

        /**
        * a function to GetMemIfSampleRate
        * @param Memory_Interface
        * @return status_t
        */
        virtual uint32 GetMemIfSampleRate(uint32 Memory_Interface) = 0;

        /**
        * a function to SetMemIfChannelCount
        * @param Memory_Interface
        * @param Channels
        * @return status_t
        */
        virtual status_t SetMemIfChannelCount(uint32 Memory_Interface, uint32 Channels) = 0;

        /**
        * a function to GetMemIfChannelCount
        * @param Memory_Interface
        * @return status_t
        */
        virtual uint32 GetMemIfChannelCount(uint32 Memory_Interface) = 0;

        /**
        * a function to SetMemIfInterruptSample
        * @param Memory_Interface
        * @param SampleCount
        * @return status_t
        */
        virtual status_t SetMemIfInterruptSample(uint32 Memory_Interface, uint32 SampleCount) = 0;

        /**
        * a function to GetMemIfInterruptSample
        * @param Memory_Interface
        * @return status_t
        */
        virtual uint32 GetMemIfInterruptSample(uint32 Memory_Interface) = 0;

        /**
        * a function to SetMemIfEnable , this will start memif and interrupt if
        * @param Memory_Interface
        * @param State
        * @return status_t
        */
        virtual status_t SetMemIfEnable(uint32 Memory_Interface, uint32 State) = 0;

        /**
        * a function to SetinputConnection
        * @param ConnectionState
        * @see AudioDigitalType::InterConnectionInput
        * @param Input
        * @see AudioDigitalType::InterConnectionInput
        * @param Output
        * @see AudioDigitalType::InterConnectionOutput
        * @return status_t
        */
        virtual status_t SetinputConnection(uint32 ConnectionState, uint32 Input , uint32 Output) = 0;

        /**
        * a function to setmic inverse , only use for stereo input
        * @param enable
        * @return status_t
        */
        virtual status_t SetMicinputInverse(bool bEnable) = 0;

        /**
        * a function to SetIrqmcuEnable
        * @param Irqmode
        * @see AudioDigitalType::IRQ_MCU_MODE
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetIrqMcuEnable(AudioDigitalType::IRQ_MCU_MODE Irqmode, bool bEnable) = 0;

        /**
        * a function to SetIrqMcuSampleRate
        * @param Irqmode
        * @see AudioDigitalType::IRQ_MCU_MODE
        * @param SampleRate
        * @return status_t
        */
        virtual status_t SetIrqMcuSampleRate(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 SampleRate) = 0;

        /**
        * a function to SetIrqMcuCounter
        * @param Irqmode
        * @see AudioDigitalType::IRQ_MCU_MODE
        * @param Counter
        * @return status_t
        */
        virtual status_t SetIrqMcuCounter(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 Counter) = 0;

        /**
        * a function to Set MrgI2S Enable
        * @param bEnable
        * @see true or false
        * @param sampleRate
        * @return status_t
        */
        virtual status_t SetMrgI2SEnable(bool bEnable, unsigned int sampleRate) = 0;

        /**
        * a function to Reset MrgIf of FM Chip
        * @return status_t
        */
        virtual status_t ResetFmChipMrgIf(void) = 0;

        /**
        * a function to GetIrqStatus
        * @param Irqmode
        * @see AudioDigitalType::IRQ_MCU_MODE
        * @return status_t
        */
        virtual status_t GetIrqStatus(AudioDigitalType::IRQ_MCU_MODE Irqmode, AudioIrqMcuMode *Mcumode) = 0;

        /**
        * a function to SetAfeOn , this is a afe bit , turn on will caise digital part start.
        * @param Irqmo
        * @return status_t
        */
        virtual status_t SetAfeEnable(bool  bEnable) = 0;

        /**
        * a function to GetAfeOn , this is a afe bit , turn on will caise digital part start.
        * @param Irqmo
        * @return status_t
        */
        virtual bool GetAfeEnable(bool  bEnable) = 0;

        /**
        * a function fo get digital block
        * @param block
        * @param bEnable
        * @return status_t
        */
        virtual int GetDigitalBlockState(int block) = 0;

        /**
        * a function fo Set Attribute of I2SDacOut
        * @param sampleRate
        * @return status_t
        */
        virtual status_t SetI2SDacOutAttribute(uint32_t sampleRate) = 0;

        /**
        * a function fo SetI2SDacOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t SetI2SDacOut(AudioDigtalI2S *mDigitalI2S) = 0;

        /**
        * a function fo GetI2SDacOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t GetI2SDacOut(AudioDigtalI2S *mDigitalI2S) = 0;

        /**
        * a function fo Set2ndI2SOutAttribute
        * @param sampleRate
        * @return status_t
        */
        virtual status_t Set2ndI2SOutAttribute(uint32_t sampleRate) = 0;

        /**
        * a function fo Set2ndI2SOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t Set2ndI2SOut(AudioDigtalI2S *mDigitalI2S) = 0;

        /**
        * a function fo Get2ndI2SOut
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t Get2ndI2SOut(AudioDigtalI2S *mDigitalI2S) = 0;

        /**
        * a function fo Set2ndI2SEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t Set2ndI2SEnable(bool bEnable) = 0;

        /**
        * a function fo SetI2SSoftReset
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SSoftReset(bool bEnable) = 0;
        
        /**
        * a function fo SetI2SDacEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SDacEnable(bool bEnable) = 0;

        /**
        * a function fo SetI2SAdcIn
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t SetI2SAdcIn(AudioDigtalI2S *mDigitalI2S) = 0;

        /**
        * a function fo GetI2SAdcIn
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t GetI2SAdcIn(AudioDigtalI2S *mDigitalI2S) = 0;

        /**
        * a function fo SetI2SAdcEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SAdcEnable(bool bEnable) = 0;


        /**
        * a function fo SetDAIBBT attribute
        * @param DAIBT
        * @return status_t
        */
        virtual status_t SetDAIBBT(AudioDigitalDAIBT *DAIBT) = 0;

        /**
        * a function fo GetDAIBTOut attribute
        * @param DAIBT
        * @return status_t
        */
        virtual status_t GetDAIBTOut(AudioDigitalDAIBT *DAIBT) = 0;

        /**
        * a function fo SetDAIBTEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetDAIBTEnable(bool bEnable) = 0;

        virtual void BT_SCO_SetMode(uint32_t mode) = 0;
		  
        virtual uint32_t BT_SCO_GetMode(void) = 0;

        /**
        * this function will  tansform SampleRate into hardware samplerate format
        * @return AudioMEMIFAttribute::SAMPLINGRATE
        */
        virtual AudioMEMIFAttribute::SAMPLINGRATE SampleRateTransform(uint32 SampleRate) = 0;

        /**
        * this functionis to set HwDigitalGainMode with sampleRate and step with how many samples
        * @param GainType
        * @param SampleRate
        * @param SamplePerStep
        * @return status_t
        */
        virtual status_t SetHwDigitalGainMode(AudioDigitalType::Hw_Digital_Gain GainType, AudioMEMIFAttribute::SAMPLINGRATE SampleRate,  uint32 SamplePerStep) = 0;

        /**
        * this functionis to set HwDigitalGainMode with sampleRate and step with how many samples
        * @param GainType
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHwDigitalGainEnable(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable) = 0;

        /**
        * this functionis to set HwDigitalGainMode with sampleRate and step with how many samples, w/o Ramp Up
        * @param GainType
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHwDigitalGainEnableByPassRampUp(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable) = 0;

        /**
        * this functionis to set Hw digital gain
        * @return status_t
        */
        virtual status_t SetHwDigitalGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType) = 0;

        /**
        * this functionis to set Hw digital current gain
        * @return status_t
        */
        virtual status_t SetHwDigitalCurrentGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType) = 0;

        /**
        * this functionis to set modem pcm attribute.
        * @param modem_index
        * @param p_modem_pcm_attribute
        * @return status_t
        */
        virtual status_t SetModemPcmConfig(modem_index_t modem_index, AudioDigitalPCM *p_modem_pcm_attribute) = 0;

        /**
        * this functionis to set modem pcm enable/disable
        * @param modem_index
        * @param modem_pcm_on
        * @return status_t
        */
        virtual status_t SetModemPcmEnable(modem_index_t modem_index, bool modem_pcm_on) = 0;

        /**
        * this functionis to enable/disable side tone filter
        * @param stf_on
        * @return status_t
        */
        virtual status_t EnableSideToneFilter(bool stf_on) = 0;

        /**
          * this functionis to set Sinetone Ouput LR
        * @return status_t
        */
        virtual status_t SetSinetoneOutputLR(bool bLR) = 0;

        /**
        * this functionis to check if digital dl pth is change ex: Dac to BT....
        * @param PreDevice
        * @param NewDevice
        * @return bool
        */
        virtual bool CheckDlDigitalChange(uint32_t PreDevice, uint32_t NewDevice) = 0;

        /**
        * this functionis to get device by device
        * @param Device
        * @return uint32
        */
        virtual uint32 DlPolicyByDevice(uint32_t Device) = 0;

        /**
        * this functionis to set Hw digital gain
        * @param connection input or output
        * @param direction 1: input 0:output
        * @param Enable 1: enable 0:disable
        * @return status_t
        */
        virtual status_t EnableSideToneHw(uint32 connection , bool direction, bool  Enable) = 0;

        /**
        * a function to Set I2SASRC Config
        * @param bIsUseASRC
        * @param dToSampleRate
        * @return status_t
        */
        virtual status_t SetI2SASRCConfig(bool bIsUseASRC, unsigned int dToSampleRate) = 0;
        /**
        * a function fo SetI2SASRCEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t SetI2SASRCEnable(bool bEnable) = 0;
        /**
        * a function to Set Set2ndI2SIn Config
        * @param sampleRate
        * @param bIsSlaveMode
        * @return status_t
        */
        virtual status_t Set2ndI2SInConfig(unsigned int sampleRate, bool bIsSlaveMode) = 0;
        /**
        * a function fo Set2ndI2SIn
        * @param mDigitalI2S
        * @return status_t
        */
        virtual status_t Set2ndI2SIn(AudioDigtalI2S *mDigitalI2S) = 0;
        /**
        * a function fo Set2ndI2SInEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t Set2ndI2SInEnable(bool bEnable) = 0;
        /**
        * a function fo Set2ndI2SOutEnable
        * @param bEnable
        * @return status_t
        */
        virtual status_t Set2ndI2SOutEnable(bool bEnable) = 0;

        virtual status_t SetMemIfFetchFormatPerSample(uint32 InterfaceType, AudioMEMIFAttribute::FETCHFORMATPERSAMPLE eFetchFormat) = 0;
        virtual AudioMEMIFAttribute::FETCHFORMATPERSAMPLE GetMemIfFetchFormatPerSample(uint32 InterfaceType) = 0;
        virtual status_t SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT ConnectionFormat, AudioDigitalType::InterConnectionOutput Output) = 0;
        virtual AudioDigitalType::OUTPUT_DATA_FORMAT GetoutputConnectionFormat(AudioDigitalType::InterConnectionOutput Output) = 0;
};

}
#endif
