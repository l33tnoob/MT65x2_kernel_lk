#include "AudioMachineDevice.h"
#include "AudioAnalogType.h"
#include "AudioIoctl.h"
#include "audio_custom_exp.h"
#include "AudioUtility.h"
#include <utils/Log.h>

#define LOG_TAG "AudioMachineDevice"
#ifndef ANDROID_DEFAULT_CODE
#include <cutils/xlog.h>
#ifdef ALOGE
#undef ALOGE
#endif
#ifdef ALOGW
#undef ALOGW
#endif ALOGI
#undef ALOGI
#ifdef ALOGD
#undef ALOGD
#endif
#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGE XLOGE
#define ALOGW XLOGW
#define ALOGI XLOGI
#define ALOGD XLOGD
#define ALOGV XLOGV
#else
#include <utils/Log.h>
#endif


#define PMIC_TRIM_ADDRESS1 (0x1c2)
#define PMIC_TRIM_ADDRESS2 (0x1c4)
#define PMIC_TRIM_REG1_DEFAULT (0x0220)
#define PMIC_TRIM_REG2_DEFAULT (0x0006)

#define PMIC_TRIM_SPK_ADDRESS (0x01CA)
#define PMIC_TRIM_SPKREG1_DEFAULT (0x0010)
#define PMIC_AUTO_TRIM_ADRESS (0x014E)

#define SPK_TRIM_INTERVAL   (1)

namespace android
{

status_t AudioMachineDevice::InitCheck()
{
    ALOGD("InitCheck");
    return NO_ERROR;
}

int AudioMachineDevice::GetChipVersion()
{
    int ret = ::ioctl(mFd, GET_PMIC_VERSION, 0);
    //ALOGD("GetChipVersion ret = %d",ret);
    return ret;
}

AudioMachineDevice::AudioMachineDevice()
{
    mAudioAnalogReg = NULL;
    mAudioAnalogReg = AudioAnalogReg::getInstance();
    mFd = 0;
#ifdef USING_EXTAMP_HP
    mCloseSpkOnly = mOpenSpkOnly = 0;
#endif

#ifdef USING_CLASSD_AMP
    mSpeakerClass = AudioAnalogType::CLASS_D;
#else
    mSpeakerClass = AudioAnalogType::CLASS_AB;
#endif
    mCurrentSensing = false;
    if (!mAudioAnalogReg)
    {
        ALOGW("AudioMachineDevice init mAudioAnalogReg fail =  %p", mAudioAnalogReg);
    }
    // init analog part.
    mFd = ::open(kAudioDeviceName, O_RDWR);
    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++)
    {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++)
    {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }
}
#if 0
status_t AudioMachineDevice::StartHeadphoneTrimFunction()
{
    GetTrimOffset();
    if (GetChipVersion() == CHIP_VERSION_E1)
    {
        mAudioAnalogReg->SetAnalogReg(0x071A, 0x1000, 0x1000);
    }
    return NO_ERROR;
}
#endif
status_t AudioMachineDevice::StartSpkTrimFunction()
{
#if 0
#ifdef USING_EXTAMP_HP
#else
    AnalogOpen(AudioAnalogType::DEVICE_OUT_SPEAKERL);
    SPKAutoTrimOffset(); // use spk auto trim
    GetSPKAutoTrimOffset();
#endif
#else
    ALOGD("Disable StartSpkTrimFunction");
#endif
    return NO_ERROR;
}
#if 0
status_t AudioMachineDevice::GetTrimOffset()
{
    uint32 reg1 = 0, reg2 = 0;
    bool trim_enable = 0;
    // get to check if trim happen
    reg1 = mAudioAnalogReg->GetAnalogReg(PMIC_TRIM_ADDRESS1);
    reg2 = mAudioAnalogReg->GetAnalogReg(PMIC_TRIM_ADDRESS2);
    ALOGD("reg1 = 0x%x reg2 = 0x%x", reg1, reg2);
    trim_enable = (reg1 >> 12) & 1;
    if (trim_enable == 0)
    {
        reg1 = (PMIC_TRIM_REG1_DEFAULT & 0xfff0);
        reg2 = (PMIC_TRIM_REG2_DEFAULT & 0x0fff);
    }
    ALOGD("trim_enable = %d reg1 = 0x%x reg2 = 0x%x", trim_enable, reg1, reg2);
    mHPRtrim = (reg1 >> 8) & 0x000f;
    mHPRfinetrim = ((reg1 >> 15) & 0x0001) | ((reg2 & 0x0001) << 1);
    mHPLtrim = (reg1 >> 4) & 0x000f;
    mHPLfinetrim = (reg1 >> 13) & 0x0003;
    mIVHPLtrim = (reg2 >> 1) & 0x000f;
    mIVHPLfinetrim  = (reg2 >> 9) & 0x0003;
    ALOGD("GetTrimOffset mHPRtrim = 0x%x mHPRfinetrim = 0x%x mHPLtrim = 0x%x mHPLfinetrim = 0x%x mIVHPLtrim = 0x%x mIVHPLfinetrim = 0x%x",
          mHPRtrim, mHPRfinetrim, mHPLtrim, mHPLfinetrim, mIVHPLtrim, mIVHPLfinetrim);

    return NO_ERROR;
}
#endif
status_t AudioMachineDevice::GetSPKTrimOffset()
{
    uint32 reg1 = 0;
    bool trim_enable = 0;
    // get to check if trim happen
    reg1 = mAudioAnalogReg->GetAnalogReg(PMIC_TRIM_SPK_ADDRESS);
    trim_enable = (reg1 >> 13) & 0x1;
    if (trim_enable == 0)
    {
        reg1 = PMIC_TRIM_SPKREG1_DEFAULT;
    }
    mSPKpolarity = (reg1 >> 12) & 0x1;
    mISPKtrim = (reg1 >> 7) & 0x1f;
    ALOGD("trim_enable = %d mSPKpolarity = %d mISPKtrim = 0x%x",
          trim_enable, mSPKpolarity, mISPKtrim);
    return NO_ERROR;
}

status_t AudioMachineDevice::SPKAutoTrimOffset()
{
    uint32_t WaitforReady = 0;
    int retyrcount = 10;
    ALOGD("Go StartSpkTrimFunction\n");
    mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x2018, 0xffff);  // Choose new mode for trim (E2 Trim)
    mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0008, 0xffff); // Enable auto trim
    mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3000, 0xf000); // set gain
    mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x0a00, 0x0f00); // set gain
    mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0001, 0x0001); // Enable amplifier & auto trim
    do
    {
        WaitforReady = mAudioAnalogReg->GetAnalogReg(SPK_CON1);
        WaitforReady = (WaitforReady & 0x8000) >> 14;
        usleep(10 * 1000);
        //ALOGD("SPKAutoTrimOffset sleep........");
    }
    while ((!WaitforReady) && retyrcount);
    ALOGD("SPKAutoTrimOffset done");
    mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x0, 0xffff);
    mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0000, 0x0001);
    return NO_ERROR;
}

status_t AudioMachineDevice::GetSPKAutoTrimOffset()
{
    uint32 reg1 = 0;
    ALOGD("GetSPKAutoTrimOffset ");
    mAudioAnalogReg->SetAnalogReg(0x013A, 0x0802, 0xffff); // totally hardcode....
    reg1 = mAudioAnalogReg->GetAnalogReg(PMIC_AUTO_TRIM_ADRESS);
    mSPKpolarity = (reg1 >> 9) & 0x1;
    mISPKtrim = (reg1 >> 10) & 0x1f;
    ALOGD("mSPKpolarity = %d mISPKtrim = 0x%x", mSPKpolarity, mISPKtrim);
    return NO_ERROR;
}

status_t AudioMachineDevice::SetSPKTrimOffset(void)
{
    uint32 AUDBUG_reg = 0;
    AUDBUG_reg |= 1 << 14; // enable trim function
    AUDBUG_reg |= mSPKpolarity << 13 ; // polarity
    AUDBUG_reg |= mISPKtrim << 8 ; // polarity
    ALOGD("SetSPKTrimOffset AUDBUG_reg = 0x%x", AUDBUG_reg);
    mAudioAnalogReg->SetAnalogReg(SPK_CON1, AUDBUG_reg, 0x7f00);
    return NO_ERROR;
}
#if 0
status_t AudioMachineDevice::SetHPTrimOffset(void)
{
    uint32 AUDBUG_reg = 0;
    ALOGD("SetHPTrimOffset");
    AUDBUG_reg |= 1 << 8; // enable trim function
    AUDBUG_reg |= mHPRfinetrim << 11 ;
    AUDBUG_reg |= mHPLfinetrim << 9;
    AUDBUG_reg |= mHPRtrim << 4;
    AUDBUG_reg |= mHPLtrim;
    mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG3, AUDBUG_reg, 0x1fff);
    return NO_ERROR;
}

status_t AudioMachineDevice::SetIVHPTrimOffset(void)
{
    uint32 AUDBUG_reg = 0;
    ALOGD("SetIVHPTrimOffset");
    AUDBUG_reg |= 1 << 8; // enable trim function
    AUDBUG_reg |= mHPRfinetrim << 11 ;
    AUDBUG_reg |= mIVHPLfinetrim << 9;
    AUDBUG_reg |= mHPRtrim << 4;
    AUDBUG_reg |= mIVHPLtrim;
    mAudioAnalogReg->SetAnalogReg(AUDBUF_CFG3, AUDBUG_reg, 0x1fff);
    return NO_ERROR;
}
#endif
bool AudioMachineDevice::GetDownLinkStatus(void)
{
    for (int i = 0; i <= AudioAnalogType::DEVICE_2IN1_SPK; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            return true;
        }
    }
    return false;
}

bool AudioMachineDevice::GetULinkStatus(void)
{
    for (int i = AudioAnalogType::DEVICE_2IN1_SPK; i <= AudioAnalogType::DEVICE_IN_DIGITAL_MIC; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            return true;
        }
    }
    return false;
}

status_t AudioMachineDevice::SetAmpGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec, use 15dB for
    uint32 index =  13;

    // condition for gainb not mute
    if (volume > 11)
    {
        volume = 11;
    }
    //const int HWgain[] =  {-60,0,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    index -= volume;
    if (index < 1)
    {
        index = 1; // min to 0dB
    }
    if (volume_Type == AudioAnalogType::VOLUME_SPKL || volume_Type == AudioAnalogType::VOLUME_SPKR)
    {
        mAudioAnalogReg->SetAnalogReg(SPK_CON9, index << 8, 0x00000f00);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetHandSetGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    // this will base on hw spec.
    uint32 index = 15;
    //const int HWgain[] =  {-21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9};
    volume = volume / 2;
    if (volume > index)
    {
        volume = index;
    }
    index -= volume;
    if (index < 8)
    {
        index = 8;    // Min bounded at -5 dB
    }
    if (volume_Type == AudioAnalogType::VOLUME_HSOUTL || volume_Type == AudioAnalogType::VOLUME_HSOUTR)
    {
        mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, index << 4, 0x000000f0);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetHeadPhoneGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    //return NO_ERROR;
    // this will base on hw spec.
    uint32 index = 7;
    //const int HWgain[] = {-5, -3, -1, 1, 3, 5, 7, 9};
    volume = volume / 2;
    if (volume > index)
    {
        volume = index;
    }
    index -= volume;
#if 1
    if (volume_Type == AudioAnalogType::VOLUME_HPOUTL)
    {
        mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, index << 12, 0x00007000);
    }
    else if (volume_Type == AudioAnalogType::VOLUME_HPOUTR)
    {
        mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, index << 8, 0x000000700);
    }
#else
    uint32 currentVol;
    int direction, i;
    // Do ramp up
    if (volume_Type == AudioAnalogType::VOLUME_HPOUTL)
    {
        currentVol = mAudioAnalogReg->GetAnalogReg(AUDTOP_CON5) >> 12;
        currentVol &= 0x7;
        if (index != currentVol)
        {
            direction = (currentVol < index) ? 1 : -1;
            for (i = currentVol; i != index ; i++)
            {
                currentVol += direction;
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, i << 12, 0x00007000);
                ALOGD("SetHeadPhoneGain  = %d ", i);
                usleep(1000);
            }
        }
    }
    else if (volume_Type == AudioAnalogType::VOLUME_HPOUTR)
    {
        currentVol = mAudioAnalogReg->GetAnalogReg(AUDTOP_CON5) >> 8;
        currentVol &= 0x7;
        if (index != currentVol)
        {
            direction = (currentVol < index) ? 1 : -1;
            for (i = currentVol; i != index ; i++)
            {
                currentVol += direction;
                //mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5,index<<12,0x00007000);
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, i << 8, 0x00000700);
                usleep(1000);
                ALOGD("SetHeadPhoneGain  = %d ", i);
            }
        }
    }
#endif
    return NO_ERROR;
}

status_t AudioMachineDevice::SetLineinGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    return NO_ERROR;
}

status_t AudioMachineDevice::SetPreampBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    ALOGD("SetPreampBufferGain volume_Type = %d volume = %d", volume_Type, volume);
    // this will base on hw spec.
    uint32 index =  5;
    // condifiton for gain.
    volume = volume / 6;
    if (volume > index)
    {
        volume = index;
    }
    //const int PreAmpGain[] = {-6, 0, 6, 12, 18, 24};
    index -= volume;
    if (volume_Type == AudioAnalogType::VOLUME_MICAMPR)
    {
        mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1, index << 8, 0x00000700);
    }
    else if (volume_Type == AudioAnalogType::VOLUME_MICAMPL)
    {
        mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, index << 4, 0x00000070);
    }
    return NO_ERROR;
}

status_t AudioMachineDevice::SetLevelShiftBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    return NO_ERROR;
}

status_t AudioMachineDevice::SetIVBufferGain(AudioAnalogType::VOLUME_TYPE volume_Type, int volume)
{
    return NO_ERROR;
}

/**
,* a basic function for SetAnalogGain for different Volume Type
* @param VoleumType value want to set to analog volume
* @param volume function of analog gain , value between 0 ~ 255
* @return status_t
*/
status_t AudioMachineDevice::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("SetAnalogGain VOLUME_TYPE = %d volume = %d ", VoleumType, volume);
    switch (VoleumType)
    {
        case AudioAnalogType::VOLUME_HSOUTL:
        case AudioAnalogType::VOLUME_HSOUTR:
            SetHandSetGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_HPOUTL:
        case AudioAnalogType::VOLUME_HPOUTR:
            SetHeadPhoneGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_SPKL:
        case AudioAnalogType::VOLUME_SPKR:
            SetAmpGain(VoleumType,  volume);
            break;
        case AudioAnalogType::VOLUME_LINEINL:
        case AudioAnalogType::VOLUME_LINEINR:
            SetLineinGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_MICAMPL:
        case AudioAnalogType::VOLUME_MICAMPR:
            SetPreampBufferGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_LEVELSHIFTL:
        case AudioAnalogType::VOLUME_LEVELSHIFTR:
            SetLevelShiftBufferGain(VoleumType, volume);
            break;
        case AudioAnalogType::VOLUME_IV_BUFFER:
            SetIVBufferGain(VoleumType, volume);
            break;
            // defdault no support device
        case AudioAnalogType::VOLUME_LINEOUTL:
        case AudioAnalogType::VOLUME_LINEOUTR:
        default:
            break;

    }
    return NO_ERROR;
}


/**
* a basic function for GetAnalogGain for different Volume Type
* @param VoleumType value want to get analog volume
* @return int
*/
int AudioMachineDevice::GetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType)
{
    int volume = 0;
    uint32 regvalue = 0;
    // here only implement DL gain .
    switch (VoleumType)
    {
        case AudioAnalogType::VOLUME_HSOUTL:
        case AudioAnalogType::VOLUME_HSOUTR:
            regvalue = mAudioAnalogReg->GetAnalogReg(AUDTOP_CON7);
            volume = (regvalue >> 4) & 0xf;
            volume = -21 + volume * 2;
            break;
        case AudioAnalogType::VOLUME_HPOUTL:
        case AudioAnalogType::VOLUME_HPOUTR:
            // here only use L as volume
            regvalue = mAudioAnalogReg->GetAnalogReg(AUDTOP_CON5);
            volume = (regvalue >> 12) & 0x7;
            volume =  -5 + volume * 2;
            break;
        case AudioAnalogType::VOLUME_SPKL:
        case AudioAnalogType::VOLUME_SPKR:
#ifdef USING_EXTAMP_HP
            volume = GetAnalogGain(AudioAnalogType::VOLUME_HPOUTL);
#else
            regvalue = mAudioAnalogReg->GetAnalogReg(SPK_CON9);
            volume = regvalue & 0xf00;
            volume = volume >> 8;
            if (volume == 0)
            {
                volume = -64;
            }
            else if (volume == 1)
            {
                volume = 0;
            }
            else
            {
                volume = volume + 2;
            }
#endif
            break;
        default:
            break;
    }
    ALOGD("GetAnalogGain regvalue = 0x%x volume = %d VoleumType = %d", regvalue, volume, VoleumType);
    return volume;
}

/**
* a basic function fo SetAnalogMute, if provide mute function of hardware.
* @param VoleumType value want to set to analog volume
* @param mute of volume type
* @return status_t
*/
status_t AudioMachineDevice::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("SetAnalogMute VOLUME_TYPE = %d mute = %d ", VoleumType, mute);
    return NO_ERROR;
}

/**
* a basic function fo AnalogOpen, open analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioMachineDevice::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioMachineDevice AnalogOpen DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();
    if (mBlockAttribute[DeviceType].mEnable == true)
    {
        ALOGW("AnalogOpen bypass with DeviceType = %d", DeviceType);
        mLock.unlock();
        return NO_ERROR;;
    }
    mBlockAttribute[DeviceType].mEnable = true;
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2430, 0xffff); // Set voice buffer to smallest -22dB.
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xB7F6, 0xffff); // enable input short of HP to prevent voice signal leakage . Enable 2.4V.
            // Depop. Enable audio clock
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x7000, 0xf000); // enable clean 1.35VCM buffer in audioUL
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0014, 0xffff); // enable audio bias. enable LCH DAC
            for (uint16_t i = 3; i < 11; i++)
            {
                usleep(5 * 1000);
                uint16_t rReg = 0x2500 + (i << 4);
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, rReg, 0xffff); // enable voice buffer and -1dB gain. ramp up volume from -21dB to -1dB here
            }
            // tell kernel to open device
            ioctl(mFd, SET_EARPIECE_ON, NULL);
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xF7F2, 0xffff); // Enable 2.4V. Enable HalfV buffer for HP VCM generation.Enable audio clock
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x7000, 0xf000); // enable clean 1.35VCM buffer in audioUL
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, 0x0014, 0xffff); // set RCH/LCH buffer gain to smallest -5dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x007C, 0xffff); // enable audio bias. enable audio DAC, HP buffers
            usleep(10 * 1000);
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xF5BA, 0xffff); // HP pre-charge function release, disable depop mux of HP drivers. Disable depop VCM gen.
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, 0x2214, 0xffff); // set RCH/LCH buffer gain to -1dB
            // tell kernel to open device
#ifdef USING_EXTAMP_HP
            if (!mOpenSpkOnly)
                 ioctl(mFd, SET_HEADPHONE_ON, NULL); // tell kernel to open device   
#else
            ioctl(mFd, SET_HEADPHONE_ON, NULL); // tell kernel to open device
#endif
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            mOpenSpkOnly = true;
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mOpenSpkOnly = false;
            mLock.lock();
#else
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Set voice buffer to smallest -22dB.
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xB7F6, 0xffff); // enable input short of HP to prevent voice signal leakage . Enable 2.4V.
            // Depop. Enable audio clock
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x7000, 0xf000); // enable clean 1.35VCM buffer in audioUL
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0014, 0xffff); // enable audio bias. enable LCH DAC
            usleep(10 * 1000);
            //mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x35B0, 0xffff); // enable voice buffer and +1dB gain. Inter-connect voice buffer to SPK AMP
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x3550, 0xffff); // enable voice buffer and -11dB gain. Inter-connect voice buffer to SPK AMP
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x000E, 0x000E); // Speaker clock
            mAudioAnalogReg->SetAnalogReg(SPK_CON2, 0x0214, 0xffff); // enable classAB OC function
            mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x0400, 0xffff); // Set Spk 6dB gain

            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3008, 0xffff); // enable SPK-Amp with 0dB gain, enable SPK amp offset triming, select class D mode
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3009, 0xffff); // Enable Class ABD
            usleep(5 * 1000);

            if(mSpeakerClass == AudioAnalogType::CLASS_AB || mCurrentSensing == true)
            {
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3000, 0xffff); // disable amp before switch to ClassAB
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3005, 0xffff); // enable SPK AMP with 0dB gain, select Class AB. enable Amp.
            }
            else
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x3001,0xffff); // enable SPK AMP with 0dB gain, select Class D. enable Amp.
            }

            mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0A00, 0xffff); // spk output stage enable and enable
            for(uint16_t i = 6; i <= 11; i++)
            {
                usleep(SPK_TRIM_INTERVAL * 1000);
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, (0x3500|(i<<4)), 0xffff); // enable voice buffer and +1dB gain. Inter-connect voice buffer to SPK AMP
            } 	
#endif //USING_EXTAMP_HP

            // tell kernel to open device
            ioctl(mFd, SET_SPEAKER_ON, NULL);
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
#else
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xF7F2, 0xffff); // enable input short of HP to prevent voice signal leakage . Enable 2.4V.
            // Depop. Enable audio clock
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x7000, 0xf000); // enable clean 1.35VCM buffer in audioUL
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, 0x0014, 0xffff); // set RCH/LCH buffer gain to smallest -5dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x007C, 0xffff); // enable audio bias. enable audio DAC, HP buffers
            usleep(10 * 1000);
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xF5BA, 0xffff); // HP pre-charge function release, disable depop mux of HP drivers. Disable depop VCM gen.
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, 0x2214, 0xffff); // set RCH/LCH buffer gain to -1dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x35B0, 0xffff); // enable voice buffer and -1dB gain. Inter-connect voice buffer to SPK AMP

            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x000E, 0x000E); // Speaker clock
            mAudioAnalogReg->SetAnalogReg(SPK_CON2, 0x0214, 0xffff); // enable classAB OC function
            mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x0400, 0xffff); // disable fast VCM function, set PGA gain 6dB
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3008, 0xffff); // enable SPK-Amp with 0dB gain, enable SPK amp offset triming, select class D mode
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3009, 0xffff); // Enable Class ABD
            usleep(5 * 1000);
            if(mSpeakerClass == AudioAnalogType::CLASS_AB || mCurrentSensing == true)
            {
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3000, 0xffff); // disable amp before switch to ClassAB
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3005, 0xffff); // enable SPK AMP with 0dB gain, select Class AB. enable Amp.
            }
            else
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x3001,0xffff); // enable SPK AMP with 0dB gain, select Class D. enable Amp.
            }

            mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0A00, 0xffff); // spk output stage enable and enable spk amp
#endif
            // tell kernel to open device
            ioctl(mFd, SET_SPEAKER_ON, NULL);
            ioctl(mFd, SET_HEADPHONE_ON, NULL);
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Set voice buffer to smallest -22dB.
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xB7F6, 0xffff); // enable input short of HP to prevent voice signal leakage . Enable 2.4V.
                // Depop. Enable audio clock
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x7000, 0xf000); // enable clean 1.35VCM buffer in audioUL
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0014, 0xffff); // enable audio bias. enable LCH DAC
                usleep(10 * 1000);
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x35B0, 0xffff); // enable voice buffer and -1dB gain. Inter-connect voice buffer to SPK AMP
                mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x000E, 0x000E); // Speaker clock
                mAudioAnalogReg->SetAnalogReg(SPK_CON2, 0x0614, 0xffff); // enable classAB OC function, enable speaker L receiver mode[6]
                mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x0100, 0xffff); // Set Spk 0dB gain for 2in1 speaker
                //Use Class AB as 2IN1 Speaker because it has lower noise flow
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x1005, 0xffff); // enable SPK AMP with -6dB gain for 2in1 speaker, select Class AB. enable Amp.
                mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0A00, 0xffff); // spk output stage enable and enable
            }
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            //ALOGD("AudioAnalogType::DEVICE_IN_ADC2:");

            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0 , 0x7800, 0x7f80);//Enable LCH 1.4v, 2.4V
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON2 , 0x00F0, 0x00ff);//Enable RCH 1.4V, 2.4V
            AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, (AudioAnalogType::MUX_TYPE)mBlockAttribute[AudioAnalogType::DEVICE_IN_PREAMP_L].mMuxSelect);
            AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, (AudioAnalogType::MUX_TYPE)mBlockAttribute[AudioAnalogType::DEVICE_IN_PREAMP_R].mMuxSelect);
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1 , 0x0200, 0x0700);//RCH PGA gain 6dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0 , 0x0020, 0x0070);//LCH PGA gain 6dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON8 , 0x0008, 0x0008);//MICBIAS
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0 , 0x0180, 0x0180);//Enable LCH ADC, PGA
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON2 , 0x0003, 0x0003);//Enable RCH ADC, PGA


            /* sine table
            mAudioAnalogReg->SetAnalogReg(0x0712 ,0x0002,0x0002);
            mAudioAnalogReg->SetAnalogReg(0x0714 ,0x0c90,0x0c90);
            mAudioAnalogReg->SetAnalogReg(0x071A ,0xf000,0xf000);
            mAudioAnalogReg->SetAnalogReg(0x071c ,0x0067,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x071E ,0x0093,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x072c ,0x0180,0xffff);
            mAudioAnalogReg->SetAnalogReg(0x0736 ,0x0022,0xffff);
            */
            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON8 , 0x020C, 0x03FF); //MICBIAS, digital mic enable
            break;
        case AudioAnalogType::DEVICE_OUT_LINEOUTR:
        case AudioAnalogType::DEVICE_OUT_LINEOUTL:
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            WARNING("AnalogOpen with not support device");
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

status_t AudioMachineDevice::AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioMachineDevice AnalogOpenForAddSPK DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();

    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETL].mEnable = false;
    mBlockAttribute[AudioAnalogType::VOLUME_SPEAKER_HEADSET_R].mEnable = true;

    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
#ifdef USING_EXTAMP_HP

#else
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Set voice buffer to smallest -22dB.

            //mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x35B0, 0xffff); // enable voice buffer and +1dB gain. Inter-connect voice buffer to SPK AMP
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x3550, 0xffff); // enable voice buffer and -11dB gain. Inter-connect voice buffer to SPK AMP
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_CLR, 0x000E, 0x000E); // Speaker clock
            mAudioAnalogReg->SetAnalogReg(SPK_CON2, 0x0214, 0xffff); // enable classAB OC function
            mAudioAnalogReg->SetAnalogReg(SPK_CON9, 0x0400, 0xffff); // Set Spk 6dB gain

            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3008, 0xffff); // enable SPK-Amp with 0dB gain, enable SPK amp offset triming, select class D mode
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3009, 0xffff); // Enable Class ABD
            usleep(5 * 1000);

            if (mSpeakerClass == AudioAnalogType::CLASS_AB || mCurrentSensing == true)
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3000, 0xffff); // disable amp before switch to ClassAB
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3005, 0xffff); // enable SPK AMP with 0dB gain, select Class AB. enable Amp.
            }
            else
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x3001, 0xffff); // enable SPK AMP with 0dB gain, select Class D. enable Amp.
            }

            mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0A00, 0xffff); // spk output stage enable and enable
            for(uint16_t i = 6; i <= 11; i++)
            {
                usleep(SPK_TRIM_INTERVAL * 1000);
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, (0x3500|(i<<4)), 0xffff); // enable voice buffer and +1dB gain. Inter-connect voice buffer to SPK AMP
            } 
#endif //USING_EXTAMP_HP

            // tell kernel to open device
            ioctl(mFd, SET_SPEAKER_ON, NULL);
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

status_t AudioMachineDevice::AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    uint16_t i;
    ALOGD("AnalogCloseForSubSPK DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();

    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = true;
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
            // tell kernel to open device
            ioctl(mFd, SET_SPEAKER_OFF, NULL);

#ifdef USING_EXTAMP_HP

#else
		 for(uint16_t i = 10; i >= 5; i--)
            {                
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, (0x3500|(i<<4)), 0xffff); // ramp to -11dB. Inter-connect voice buffer to SPK AMP
                usleep(SPK_TRIM_INTERVAL * 1000);
            }
            
            if (mSpeakerClass == AudioAnalogType::CLASS_AB || mCurrentSensing == true)
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0000, 0xffff); // Mute Spk amp, select to original class D mode. disable class-AB Amp
            }
            else
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0004, 0xffff); // Mute Spk amp, select to original class AB mode. disable class-D Amp
            }
            mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0000, 0xffff); // Disable SPK output stage, disable spk amp.
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_SET, 0x000E, 0x000E); // Disable Speaker clock
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2500, 0xffff); // set voice buffer gain as -22dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Disable voice buffer

#endif
            break;
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            WARNING("AnalogOpen with not support device");
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

status_t AudioMachineDevice::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("SetFrequency");
    mBlockSampleRate[DeviceType] = frequency;
    return NO_ERROR;
}

/**
* a basic function fo AnalogClose, ckose analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioMachineDevice::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    uint16_t i;
    ALOGD("AnalogClose DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();
    mBlockAttribute[DeviceType].mEnable = false;
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            // tell kernel to open device
            ioctl(mFd, SET_EARPIECE_OFF, NULL);
            i = (mAudioAnalogReg->GetAnalogReg(AUDTOP_CON7) & 0xf0) >> 4;
            i = (i < 4) ? 4 : i;
            i = (i > 16) ? 16 : i;
            for (i = i - 1; i >= 3; i--)
            {
                uint16_t rReg = 0x2500 + (i << 4);
                usleep(5 * 1000);
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, rReg, 0xffff); // disable voice buffer and -21dB gain. ramp down volume from current to -21dB here
            }
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2500, 0xffff); // set voice buffer gain as -22dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Disable voice buffer
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0000, 0xffff); // Disable audio bias and L-DAC
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x0000, 0x1000); // Disable 1.4v common mdoe
            }
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0x37E2, 0xffff); // Disable input short of HP drivers for voice signal leakage prevent and disable 2.4V reference buffer , audio DAC clock.
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            // tell kernel to open device
#ifdef USING_EXTAMP_HP
            if(!mCloseSpkOnly)
            ioctl(mFd, SET_HEADPHONE_OFF, NULL);
#else        
            ioctl(mFd, SET_HEADPHONE_OFF, NULL);
#endif
            //To Do : Need ramp down?
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, 0x0014, 0xffff); // Set RCH/LCH buffer to smallest gain -5dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xF7F2, 0xffff); // Reset pre-charge function, Enable depop mux of HP drivers, Enable depop VCM gen
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0000, 0xffff); // Disable audio bias, audio DAC, HP buffers
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x0000, 0x1000);    // Disable clean 1.35V VCM buffer in audio UL.
            }
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0x37E2, 0xffff); // Disable input short of HP drivers for voice signal leakage prevent and disable 2.4V reference buffer , audio DAC clock.
            //mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5,0x0001,0xffff); // Set the gain of RCH/LCH buffer to the normal gain -1dB
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
            // tell kernel to open device
            ioctl(mFd, SET_SPEAKER_OFF, NULL);

#ifdef USING_EXTAMP_HP
            mLock.unlock();
            mCloseSpkOnly = true;
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mCloseSpkOnly = false;
            mLock.lock();
#else
            for(uint16_t i = 10; i >= 5; i--)
            {                
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, (0x3500|(i<<4)), 0xffff); // ramp to -11dB. Inter-connect voice buffer to SPK AMP
                usleep(SPK_TRIM_INTERVAL * 1000);
            }
            		
            if(mSpeakerClass == AudioAnalogType::CLASS_AB || mCurrentSensing == true)
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x0000,0xffff); // Mute Spk amp, select to original class D mode. disable class-AB Amp
            }
            else
            {
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0004, 0xffff); // Mute Spk amp, select to original class AB mode. disable class-D Amp
            }
            mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0000, 0xffff); // Disable SPK output stage, disable spk amp.
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_SET, 0x000E, 0x000E); // Disable Speaker clock
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2500, 0xffff); // set voice buffer gain as -22dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Disable voice buffer
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0000, 0xffff); // Disable audio bias and L-DAC
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x0000, 0x1000);    // Disable 1.4v common mdoe
            }
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0x37E2, 0xffff); // Disable input short of HP drivers for voice signal leakage prevent and disable 2.4V reference buffer , audio DAC clock.

#endif
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:

            if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0000, 0xffff); // Mute Spk amp, select to original class D mode. disable class-AB Amp
                mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0000, 0xffff); // Disable SPK output stage, disable spk amp.
                mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_SET, 0x000E, 0x000E); // Disable Speaker clock
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2500, 0xffff); // set voice buffer gain as -22dB
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Disable voice buffer
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0000, 0xffff); // Disable audio bias and L-DAC
                if (GetULinkStatus() == false)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x0000, 0x1000);    // Disable 1.4v common mdoe
                }
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0x37E2, 0xffff); // Disable input short of HP drivers for voice signal leakage prevent and disable 2.4V reference buffer , audio DAC clock.
            }


            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            // tell kernel to open device
            ioctl(mFd, SET_HEADPHONE_OFF, NULL);
            ioctl(mFd, SET_SPEAKER_OFF, NULL);
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
#else
            if(mSpeakerClass == AudioAnalogType::CLASS_AB || mCurrentSensing == true)
            {
                mAudioAnalogReg->SetAnalogReg(SPK_CON0,0x0000,0xffff); // Mute Spk amp, select to original class D mode. disable class-AB Amp
            }
            else
            {
            mAudioAnalogReg->SetAnalogReg(SPK_CON0, 0x0004, 0xffff); // Mute Spk amp, select to original class AB mode. disable class-D Amp
            }
            mAudioAnalogReg->SetAnalogReg(SPK_CON12, 0x0000, 0xffff); // Disable SPK output stage, disable spk amp.
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN1_SET, 0x000E, 0x000E); // Disable Speaker clock
            //Voice buffer
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2500, 0xffff); // set voice buffer gain as -22dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON7, 0x2400, 0xffff); // Disable voice buffer
            //Audio buffer
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON5, 0x0014, 0xffff); // Set RCH/LCH buffer to smallest gain -5dB
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0xF7F2, 0xffff); // Reset pre-charge function, Enable depop mux of HP drivers, Enable depop VCM gen
            //audio bias
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON4, 0x0000, 0xffff); // Disable audio bias, audio DAC, HP buffers
            //common 1.35V
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x0000, 0x1000);    // Disable clean 1.35V VCM buffer in audio UL.
            }
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON6, 0x37E2, 0xffff); // Disable input short of HP drivers for voice signal leakage prevent and disable 2.4V reference buffer , audio DAC clock.

#endif
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0 , 0x0000, 0x0180);//Disable LCH ADC, PGA
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON2 , 0x0000, 0x0003);//Disable RCH ADC, PGA
            if (GetDownLinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0 , 0x6000, 0x7f80);//Disable LCH 1.4v, 2.4V                
            }
            else
            {
                mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0 , 0x7000, 0x7f80);//Disable LCH 2.4V, keep 1.4V
            }
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON2 , 0x00C0, 0x00ff);//Disable RCH 1.4V, 2.4V ALPS00824353 , always disable RG_AUDULR_VCMSEL
            AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L,  AudioAnalogType::MUX_OPEN);
            AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R,  AudioAnalogType::MUX_OPEN);
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON8 , 0x0000, 0x0008);//MICBIAS

            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(AUDTOP_CON8 , 0x0000, 0x000C); //MICBIAS, digital mic enable
            break;
        case AudioAnalogType::DEVICE_OUT_LINEOUTR:
        case AudioAnalogType::DEVICE_OUT_LINEOUTL:
        default:
            ALOGW("AnalogOpen with not support device = 0x%x", DeviceType);
            WARNING("AnalogOpen with not support device");
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

AudioAnalogType::MUX_TYPE AudioMachineDevice::AnalogGetMux(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    uint32 Reg_Value = 0;
    AudioAnalogType::MUX_TYPE MuxType = AudioAnalogType::MUX_AUDIO;
    //To Do
    return MuxType;
}

/**
* a basic function fo select mux of device type, not all device may have mux
* if select a device with no mux support , report error.
* @param DeviceType analog part
* @param MuxType analog mux selection
* @return status_t
*/
status_t AudioMachineDevice::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AnalogSetMux DeviceType = %s MuxType = %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
    mBlockAttribute[DeviceType].mMuxSelect = MuxType ;
    uint32 Reg_Value = 0;
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_IN_PREAMP_L:
        {
            if (mBlockAttribute[AudioAnalogType::DEVICE_IN_ADC1].mEnable == true || MuxType == AudioAnalogType::MUX_OPEN)
            {
                // Before microphone path is opened, do not set mic MUX as others except open, otherwise there will be micbias leakage when audio path open.
                if (MuxType != AudioAnalogType::MUX_IN_MIC3) // not R/L swap
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON3, 0x0000, 0x00000100);
                }
                if (MuxType == AudioAnalogType::MUX_OPEN)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x0003, 0x0000000f);
                }
                else if (MuxType == AudioAnalogType::MUX_IN_MIC1)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0, 0x0000000f);
                }
                else if (MuxType == AudioAnalogType::MUX_IN_MIC2)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 1, 0x0000000f);
                }
                else if (MuxType == AudioAnalogType::MUX_IN_MIC3)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON3, 0x0100, 0x00000100);
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0x4, 0x0000000f);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON0, 0, 0x0000000f);
                    ALOGW("AnalogSetMux warning");
                }
            }
            break;
        }
        case AudioAnalogType::DEVICE_IN_PREAMP_R:
        {
            if (mBlockAttribute[AudioAnalogType::DEVICE_IN_ADC1].mEnable == true || MuxType == AudioAnalogType::MUX_OPEN)
            {
                // Before microphone path is opened, do not set mic MUX as others except open, otherwise there will be micbias leakage when audio path open.
                if (MuxType != AudioAnalogType::MUX_IN_MIC1) // not R/L swap
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON3, 0x0000, 0x00000200);
                }
                if (MuxType == AudioAnalogType::MUX_OPEN)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1, 0x0020, 0x000000f0);
                }
                else if (MuxType == AudioAnalogType::MUX_IN_MIC1)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON3, 0x0200, 0x00000200);
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1, 0x0040, 0x000000f0);
                }
                else if (MuxType == AudioAnalogType::MUX_IN_MIC2)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1, 0x0010, 0x000000f0);
                }
                else if (MuxType == AudioAnalogType::MUX_IN_MIC3)
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1, 0x0000, 0x000000f0);
                }
                else
                {
                    mAudioAnalogReg->SetAnalogReg(AUDTOP_CON1, 0x0000, 0x000000f0);
                    ALOGW("AnalogSetMux warning");
                }
            }
            break;
        }

        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
        default:
            ALOGW("AnalogSetMux warning");
            break;
    }
    return NO_ERROR;
}

/**
* a  function for setParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return status_t
*/
status_t AudioMachineDevice::setParameters(int command1 , int command2 , unsigned int data)
{
    return NO_ERROR;
}

/**
* a function for setParameters , provide wide usage of analog control
* @param command1
* @param data
* @return status_t
*/
status_t AudioMachineDevice::setParameters(int command1 , void *data)
{
    switch(command1)
    {
        case AudioAnalogType::SET_CURRENT_SENSING:
        {            
            uint16_t temp;
            int par2 = (int)data;
            temp = 0x23 | (par2<<9);
            mAudioAnalogReg->SetAnalogReg(SPK_CON10, temp, 0xffff);
            mCurrentSensing = (bool)data;
            ALOGD("Update mCurrentSensing = [%d]",mCurrentSensing);
            ALOGD("Update SPK_CON10 = [0x%x]",temp);
            break;
        }
        case AudioAnalogType::SET_CURRENT_SENSING_PEAK_DETECTOR:
        {
            int par2 = (int)data;
            uint16_t temp;
            temp = par2<<8;
            mAudioAnalogReg->SetAnalogReg(SPK_CON10, temp, 0x0100);
            ALOGD("Update SPK_CON10 = [0x%x]",temp);
        }
        default:
            break;
    }
    return NO_ERROR;
}

/**
* a function fo getParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return copy_size
*/
int AudioMachineDevice::getParameters(int command1 , int command2 , void *data)
{
    return 0;
}

bool AudioMachineDevice::GetAnalogSpkOCState(void)
{
    uint32 regValue = mAudioAnalogReg->GetAnalogReg(SPK_CON6);
    ALOGD("[%s] regValue = 0x%x", __FUNCTION__, regValue);

#ifdef USING_CLASSD_AMP
    if (regValue & 0x4000)
    {
        return true;
    }
#else
    if (regValue & 0x8000)
    {
        return true;
    }
#endif
    return false;
}




}
