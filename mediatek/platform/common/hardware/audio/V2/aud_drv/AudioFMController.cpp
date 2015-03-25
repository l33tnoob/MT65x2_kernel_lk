#include "AudioFMController.h"

#include <utils/threads.h>

#include <cutils/properties.h>

#include "WCNChipController.h"
#include "AudioIoctl.h"
#include "AudioFMResourceManager.h"
#include "AudioMTKStreamManager.h"

#define LOG_TAG "AudioFMController"

namespace android
{

/*==============================================================================
 *                     Property keys
 *============================================================================*/

const char PROPERTY_KEY_FM_FORCE_DIRECT_MODE_TYPE[PROPERTY_KEY_MAX]  = "af.fm.force_direct_mode_type";

/*==============================================================================
 *                     Const Value
 *============================================================================*/

/*==============================================================================
 *                     Enumerator
 *============================================================================*/

enum fm_force_direct_mode_t
{
    FM_FORCE_NONE           = 0x0,
    FM_FORCE_DIRECT_MODE    = 0x1,
    FM_FORCE_INDIRECT_MODE  = 0x2,
};

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioFMController *AudioFMController::mAudioFMController = NULL;

AudioFMController *AudioFMController::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mAudioFMController == NULL)
    {
        mAudioFMController = new AudioFMController();
    }
    ASSERT(mAudioFMController != NULL);
    return mAudioFMController;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioFMController::AudioFMController()
{
    ALOGD("%s()", __FUNCTION__);

    mFmEnable = false;
    mIsFmDirectConnectionMode = true;

    mFmVolume = -1.0; // valid volume value: 0.0 ~ 1.0

    mFmDeviceCallback = NULL;

    mAudioFMResourceManager = new AudioFMResourceManager();
    mAudioMTKStreamManager = AudioMTKStreamManager::getInstance();
}

AudioFMController::~AudioFMController()
{
    ALOGD("%s()", __FUNCTION__);

    delete mAudioFMResourceManager;
}

/*==============================================================================
 *                     FM Control
 *============================================================================*/

bool AudioFMController::CheckFmNeedUseDirectConnectionMode()
{
    // FM force direct/indirect mode
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_FM_FORCE_DIRECT_MODE_TYPE, property_value, "0"); //"0": default not force any mode
    fm_force_direct_mode_t fm_force_direct_mode = (fm_force_direct_mode_t)atoi(property_value);

    bool retval = false;
    switch (fm_force_direct_mode)
    {
        case FM_FORCE_NONE:   // Decided by whether earphone is inserted or not
        {
            const audio_devices_t output_device = (audio_devices_t)mAudioFMResourceManager->getDlOutputDevice();
            retval = (output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
                      output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) ? true : false; // only earphone use direct mode, else, ex: earphone + wify, use indirect mode
            ALOGD("%s(), FM_FORCE_NONE, retval = %d", __FUNCTION__, retval);
            break;
        }
        case FM_FORCE_DIRECT_MODE:   // Force to direct mode
        {
            ALOGW("%s(), FM_FORCE_DIRECT_MODE", __FUNCTION__);
            retval = true;
            break;
        }
        case FM_FORCE_INDIRECT_MODE:   // Force to indirect mode
        {
            ALOGW("%s(), FM_FORCE_INDIRECT_MODE", __FUNCTION__);
            retval = false;
            break;
        }
        default:
        {
            WARNING("No such fm_force_direct_mode!!");
        }
    }

    return retval;
}


bool AudioFMController::GetFmEnable()
{
    Mutex::Autolock _l(mLock);
    ALOGV("%s(), mFmEnable = %d", __FUNCTION__, mFmEnable);
    return mFmEnable;
}

status_t AudioFMController::SetFmEnable(const bool enable)
{
    // Lock to Protect HW Registers & AudioMode
    mAudioFMResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, 3000);
    mAudioFMResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK,     3000);

    Mutex::Autolock _l(mLock);

    ALOGD("+%s(), mFmEnable = %d => enable = %d", __FUNCTION__, mFmEnable, enable);

    // Check Current Status
    if (enable == mFmEnable)
    {
        ALOGW("-%s(), enable == mFmEnable, return.", __FUNCTION__);
        mAudioFMResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
        mAudioFMResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        return INVALID_OPERATION;
    }

    // Check Audio Mode is Normal
    const audio_mode_t audio_mode = mAudioFMResourceManager->GetAudioMode();
    if (audio_mode != AUDIO_MODE_NORMAL)
    {
        ALOGW("%s(), Current AudioMode(%d) is not AUDIO_MODE_NORMAL(%d), return.", __FUNCTION__, audio_mode, AUDIO_MODE_NORMAL);
        mAudioFMResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
        mAudioFMResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        return INVALID_OPERATION;
    }


    // Update Enable Status
    mFmEnable = enable;

    // get current device
    const audio_devices_t output_device = (audio_devices_t)mAudioFMResourceManager->getDlOutputDevice();
    ALOGD("%s(), output_device = 0x%x", __FUNCTION__, output_device);

    if (mFmEnable == true) // Open
    {
        // Clock
        mAudioFMResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
        mAudioFMResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

        // set sampling rate
        mAudioFMResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, GetFmDownlinkSamplingRate());

        // AFE ON
        mAudioFMResourceManager->SetAfeEnable(true);

        // Set FM chip initialization: Reset FM I2S FIFO & Config GPIO
        SetFmChipInitialization();

        // Set FM source module enable
        mAudioFMResourceManager->SetFmSourceModuleEnable(true);

        // Set Audio Digital/Analog HW Register
        if (CheckFmNeedUseDirectConnectionMode() == true)
        {
            SetFmDirectConnection(true, true);
            if (mAudioMTKStreamManager->IsOutPutStreamActive() == false)
            {
                mAudioFMResourceManager->StartOutputDevice();
            }
        }
        else
        {
            mIsFmDirectConnectionMode = false;
        }

        // Set Direct/Indirect Mode to FMAudioPlayer
        DoDeviceChangeCallback();
    }
    else // Close
    {
        // Disable Audio Digital/Analog HW Register
        if (mAudioMTKStreamManager->IsOutPutStreamActive() == false)
        {
            mAudioFMResourceManager->StopOutputDevice();
        }
        SetFmDirectConnection(false, true);

        // Set FM source module disable
        mAudioFMResourceManager->SetFmSourceModuleEnable(false);

        // AFE OFF
        mAudioFMResourceManager->SetAfeEnable(false);

        // Clock
        mAudioFMResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
        mAudioFMResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    }

    // Unlock
    mAudioFMResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
    mAudioFMResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioFMController::ChangeDevice(const audio_devices_t new_device)
{
    Mutex::Autolock _l(mLock);

    ASSERT(mFmEnable == true);

    const audio_devices_t pre_device = (audio_devices_t)mAudioFMResourceManager->getDlOutputDevice();
    ALOGD("+%s(), pre_device = 0x%x, new_device = 0x%x", __FUNCTION__, pre_device, new_device);

    const uint32_t kAudioDeviceSpeakerAndHeadset   = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET;
    const uint32_t kAudioDeviceSpeakerAndHeadphone = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
    if (new_device == pre_device)
    {
        ALOGE("-%s(), pre_device = 0x%x, new_device = 0x%x", __FUNCTION__, pre_device, new_device);
        return INVALID_OPERATION;
    }
    else if (new_device == kAudioDeviceSpeakerAndHeadset || new_device == kAudioDeviceSpeakerAndHeadphone)
    {
        ALOGD("%s(), entering Warning Tone, only config analog part", __FUNCTION__);

        // wait until HW Gain stable
        if (mIsFmDirectConnectionMode == true)
        {
            usleep(430000); // -74.5/0.25 * 64 / 44100 = 430 ms
        }

        if (pre_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||pre_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
        {
            //Add SPK
            mAudioFMResourceManager->setDlOutputDevice(new_device);
            mAudioFMResourceManager->AddSubSPKToOutputDevice();
        }
        else
        {
            mAudioFMResourceManager->StopOutputDevice();
            mAudioFMResourceManager->setDlOutputDevice(new_device);
            usleep(50000);						
            mAudioFMResourceManager->StartOutputDevice();
        }
        ALOGD("-%s()", __FUNCTION__);
        return NO_ERROR;
    }
    else if (pre_device == kAudioDeviceSpeakerAndHeadset || pre_device == kAudioDeviceSpeakerAndHeadphone)
    {
        ALOGD("%s(), leaving Warning Tone, only config analog part", __FUNCTION__);
        if (new_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||new_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
        {
            //Sub SPK
            mAudioFMResourceManager->setDlOutputDevice(new_device);
            mAudioFMResourceManager->AddSubSPKToOutputDevice();
        }
        else
        {
            mAudioFMResourceManager->StopOutputDevice();
            mAudioFMResourceManager->setDlOutputDevice(new_device);
            mAudioFMResourceManager->StartOutputDevice();
        }
        ALOGD("-%s()", __FUNCTION__);
        return NO_ERROR;
    }


    // Close
    mAudioFMResourceManager->StopOutputDevice();

    // Update
    mAudioFMResourceManager->setDlOutputDevice(new_device);

    // Open
    SetFmDirectConnection(CheckFmNeedUseDirectConnectionMode(), false);

    // Set Direct/Indirect Mode for FM Chip
    DoDeviceChangeCallback();

    // Enable PMIC Analog Part
    if (mIsFmDirectConnectionMode == true || // Direct mode, open it directly
        mAudioMTKStreamManager->IsOutPutStreamActive() == true) // When speaker -> earphone -> speaker quickly, streamout might not do standby(), so would not do write() either. Hence open analog part here
    {
        mAudioFMResourceManager->StartOutputDevice();
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void AudioFMController::SetFmDeviceCallback(const AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *callback_data)
{
    if (callback_data == NULL)
    {
        mFmDeviceCallback = NULL;
    }
    else
    {
        mFmDeviceCallback = callback_data->callback;
        ASSERT(mFmDeviceCallback != NULL);
    }
}

status_t AudioFMController::DoDeviceChangeCallback()
{
    ALOGD("+%s(), mIsFmDirectConnectionMode = %d, callback = 0x%x", __FUNCTION__, mIsFmDirectConnectionMode, mFmDeviceCallback);

    ASSERT(mFmEnable == true);

    if (mFmDeviceCallback == NULL) // factory mode might not set mFmDeviceCallback
    {
        ALOGE("-%s(), mFmDeviceCallback == NULL", __FUNCTION__);
        return NO_INIT;
    }


    if (mIsFmDirectConnectionMode == true)
    {
        mFmDeviceCallback((void *)false); // Direct Mode, No need to create in/out stream
        ALOGD("-%s(), mFmDeviceCallback(false)", __FUNCTION__);
    }
    else
    {
        mFmDeviceCallback((void *)true);  // Indirect Mode, Need to create in/out stream
        ALOGD("-%s(), mFmDeviceCallback(true)", __FUNCTION__);
    }

    return NO_ERROR;
}

/*==============================================================================
 *                     Audio HW Control
 *============================================================================*/

uint32_t AudioFMController::GetFmUplinkSamplingRate() const
{
    return mAudioFMResourceManager->GetFmUplinkSamplingRate();
}

uint32_t AudioFMController::GetFmDownlinkSamplingRate() const
{
    return mAudioFMResourceManager->GetFmDownlinkSamplingRate();
}

status_t AudioFMController::SetFmDirectConnection(const bool enable, const bool bforce)
{
    ALOGD("+%s(), enable = %d, bforce = %d", __FUNCTION__, enable, bforce);

    // Check Current Status
    if (mIsFmDirectConnectionMode == enable && bforce == false)
    {
        ALOGW("-%s(), enable = %d, bforce = %d", __FUNCTION__, enable, bforce);
        return INVALID_OPERATION;
    }


    // Update Direct Mode Status
    mIsFmDirectConnectionMode = enable;

    // Apply
    mAudioFMResourceManager->SetFmDirectConnection(mIsFmDirectConnectionMode);

    // Update (HW_GAIN2) Volume for Direct Mode Only
    if (mIsFmDirectConnectionMode == true)
    {
        SetFmVolume(mFmVolume);
    }


    ALOGD("-%s(), enable = %d, bforce = %d", __FUNCTION__, enable, bforce);
    return NO_ERROR;
}

status_t AudioFMController::SetFmVolume(const float fm_volume)
{
    ALOGD("+%s(), mFmVolume = %f => fm_volume = %f", __FUNCTION__, mFmVolume, fm_volume);

    const float kMaxFmVolume = 1.0;
    ASSERT(0 <= fm_volume && fm_volume <= kMaxFmVolume); // valid volume value: 0.0 ~ 1.0

    mFmVolume = fm_volume;

    // Set HW Gain for Direct Mode
    if (mFmEnable == true && mIsFmDirectConnectionMode == true)
    {
        mAudioFMResourceManager->SetFmVolume(mFmVolume);
    }
    else
    {
        ALOGD("%s(), Do nothing. mFMEnable = %d, mIsFmDirectConnectionMode = %d", __FUNCTION__, mFmEnable, mIsFmDirectConnectionMode);
    }

    ALOGD("-%s(), mFmVolume = %f", __FUNCTION__, mFmVolume);
    return NO_ERROR;
}

/*==============================================================================
 *                     WCN FM Chip Control
 *============================================================================*/

bool AudioFMController::GetFmChipPowerInfo()
{
    return WCNChipController::GetInstance()->GetFmChipPowerInfo();
}


status_t AudioFMController::SetFmChipInitialization()
{
    ALOGD("+%s()", __FUNCTION__);

    int ret = 0;

    // File descriptor
    int fd_audio = ::open(kAudioDeviceName, O_RDWR);
    ALOGD("%s(), open(%s), fd_audio = %d", __FUNCTION__, kAudioDeviceName, fd_audio);
    ASSERT(fd_audio >= 0);

    // Reset FM chip merge interface I2S FIFO
    if (WCNChipController::GetInstance()->IsFMMergeInterfaceSupported() == true)
    {
        ret = ::ioctl(fd_audio, AUDDRV_RESET_FMCHIP_MERGEIF);
        ALOGD("%s(), ioctl: AUDDRV_RESET_FMCHIP_MERGEIF, ret = %d", __FUNCTION__, ret);
    }

    // Set GPIO // TODO(Harvey, Hongcheng): Really need to set FM chip GPIO in audio driver??
    ::ioctl(fd_audio, AUDDRV_SET_FM_I2S_GPIO);
    ALOGD("%s(), ioctl: AUDDRV_SET_FM_I2S_GPIO, ret = %d", __FUNCTION__, ret);

    close(fd_audio);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

} // end of namespace android
