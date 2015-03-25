#ifndef ANDROID_AUDIO_MTK_HARDWARE_INTERFACE_H
#define ANDROID_AUDIO_MTK_HARDWARE_INTERFACE_H

//#include <SpeechControlInterface.h>

//!  AudioMTKHardwareInterface interface
/*!
  this class is hold extension of android default hardwareinterface
*/
class AudioMTKHardwareInterface
{
    public:
        /**
        *  SetEMParamete, set em parameters to audioahrdware
        * @param ptr
        * @param len
        * @return status
        */
        virtual status_t SetEMParameter(void *ptr , int len) = 0;

        /**
        *  GetEMParameter, get em parameters to audioahrdware
        * @param ptr
        * @param len
        * @return status
        */
        virtual status_t GetEMParameter(void *ptr , int len) = 0;

        /**
        *  SetAudioCommand, base on par1 and par2
        * @param par1
        * @param par2
        * @return status
        */
        virtual status_t SetAudioCommand(int par1, int par2) = 0;

        /**
        *  GetAudioCommand, base on par1
        * @param par1
        * @return status
        */
        virtual status_t GetAudioCommand(int par1) = 0;

        /**
        *  SetAudioData
        * @param par1
        * @param len
        * @param ptr
        * @return status
        */
        virtual status_t SetAudioData(int par1, size_t len, void *ptr) = 0;

        /**
        *  GetAudioData
        * @param par1
        * @param len
        * @param ptr
        * @return status
        */
        virtual status_t GetAudioData(int par1, size_t len, void *ptr) = 0;

        /**
        *  set ACF Preview parameter , thoiis function only temporary replace coefficient
        * @param ptr
        * @param len
        * @return status
        */
        virtual status_t SetACFPreviewParameter(void *ptr , int len) = 0;

        /**
        *  set HCF Preview parameter , thoiis function only temporary replace coefficient
        * @param ptr
        * @param len
        * @return status
        */
        virtual status_t SetHCFPreviewParameter(void *ptr , int len) = 0;

        /////////////////////////////////////////////////////////////////////////
        //    for PCMxWay Interface API
        /////////////////////////////////////////////////////////////////////////
        virtual int xWayPlay_Start(int sample_rate) = 0;
        virtual int xWayPlay_Stop(void) = 0;
        virtual int xWayPlay_Write(void *buffer, int size_bytes) = 0;
        virtual int xWayPlay_GetFreeBufferCount(void) = 0;
        virtual int xWayRec_Start(int sample_rate) = 0;
        virtual int xWayRec_Stop(void) = 0;
        virtual int xWayRec_Read(void *buffer, int size_bytes) = 0;

        //added by wendy
        virtual int ReadRefFromRing(void *buf, uint32_t datasz, void *DLtime) = 0;
        virtual int GetVoiceUnlockULTime(void *DLtime) = 0;
        virtual int SetVoiceUnlockSRC(uint outSR, uint outChannel) = 0;
        virtual bool startVoiceUnlockDL() = 0;
        virtual bool stopVoiceUnlockDL() = 0;
        virtual void freeVoiceUnlockDLInstance() = 0;
        virtual int GetVoiceUnlockDLLatency() = 0;
        virtual bool getVoiceUnlockDLInstance() = 0;
};

#endif