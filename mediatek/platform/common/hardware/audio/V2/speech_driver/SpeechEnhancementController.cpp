#include "SpeechEnhancementController.h"
#include "SpeechDriverFactory.h"
#include "SpeechDriverInterface.h"
#include "SpeechType.h"

#include <cutils/properties.h>

#include <utils/threads.h>

#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"

#define LOG_TAG "SpeechEnhancementController"

static const char PROPERTY_KEY_SPH_ENH_MASKS[PROPERTY_KEY_MAX] = "persist.af.modem.sph_enh_mask";
static const char PROPERTY_KEY_MAGIC_CON_CALL_ON[PROPERTY_KEY_MAX] = "persist.af.magic_con_call_on";
static const char PROPERTY_KEY_BT_HEADSET_NREC_ON[PROPERTY_KEY_MAX] = "persist.af.bt_headset_nrec_on";


namespace android
{

SpeechEnhancementController *SpeechEnhancementController::mSpeechEnhancementController = NULL;
SpeechEnhancementController *SpeechEnhancementController::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechEnhancementController == NULL)
    {
        mSpeechEnhancementController = new SpeechEnhancementController();
    }
    ASSERT(mSpeechEnhancementController != NULL);
    return mSpeechEnhancementController;
}


SpeechEnhancementController::SpeechEnhancementController()
{
    // default value (all enhancement on)
    char property_default_value[PROPERTY_VALUE_MAX];
    sprintf(property_default_value, "0x%x 0x%x", SPH_ENH_MAIN_MASK_ALL, SPH_ENH_DYNAMIC_MASK_ALL);

    // get sph_enh_mask_struct from property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_SPH_ENH_MASKS, property_value, property_default_value);

    // parse mask info from property_value
    sscanf(property_value, "0x%x 0x%x",
           &mSpeechEnhancementMask.main_func,
           &mSpeechEnhancementMask.dynamic_func);

    ALOGD("mSpeechEnhancementMask: main_func = 0x%x, sub_func = 0x%x",
          mSpeechEnhancementMask.main_func,
          mSpeechEnhancementMask.dynamic_func);

    // Magic conference call
    char magic_conference_call_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_MAGIC_CON_CALL_ON, magic_conference_call_on, "0"); //"0": default off
    mMagicConferenceCallOn = (magic_conference_call_on[0] == '0') ? false : true;

    // BT Headset NREC
    char bt_headset_nrec_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_BT_HEADSET_NREC_ON, bt_headset_nrec_on, "1"); //"1": default on
    mBtHeadsetNrecOn = (bt_headset_nrec_on[0] == '0') ? false : true;
}

SpeechEnhancementController::~SpeechEnhancementController()
{

}

status_t SpeechEnhancementController::SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetNBSpeechParameters(pSphParamNB);
        }
    }

    return NO_ERROR;
}


#if defined(MTK_DUAL_MIC_SUPPORT)
status_t SpeechEnhancementController::SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetDualMicSpeechParameters(pSphParamDualMic);
        }
    }

    return NO_ERROR;
}
#endif


#if defined(MTK_WB_SPEECH_SUPPORT)
status_t SpeechEnhancementController::SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetWBSpeechParameters(pSphParamWB);
        }
    }

    return NO_ERROR;
}
#endif


status_t SpeechEnhancementController::SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask)
{
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "0x%x 0x%x", mask.main_func, mask.dynamic_func);
    property_set(PROPERTY_KEY_SPH_ENH_MASKS, property_value);

    mSpeechEnhancementMask = mask;

    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetSpeechEnhancementMask(mSpeechEnhancementMask);
        }
    }

    return NO_ERROR;
}


status_t SpeechEnhancementController::SetDynamicMaskOnToAllModem(const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on)
{
    sph_enh_mask_struct_t mask = GetSpeechEnhancementMask();

    const bool current_flag_on = ((mask.dynamic_func & dynamic_mask_type) > 0);
    if (new_flag_on == current_flag_on)
    {
        ALOGW("%s(), dynamic_mask_type(%x), new_flag_on(%d) == current_flag_on(%d), return",
              __FUNCTION__, dynamic_mask_type, new_flag_on, current_flag_on);
        return NO_ERROR;
    }

    if (new_flag_on == false)
    {
        mask.dynamic_func &= (~dynamic_mask_type);
    }
    else
    {
        mask.dynamic_func |= dynamic_mask_type;
    }

    return SetSpeechEnhancementMaskToAllModem(mask);
}

void SpeechEnhancementController::SetMagicConferenceCallOn(const bool magic_conference_call_on)
{
    ALOGD("%s(), mMagicConferenceCallOn = %d, new magic_conference_call_on = %d",
          __FUNCTION__, mMagicConferenceCallOn, magic_conference_call_on);

    property_set(PROPERTY_KEY_MAGIC_CON_CALL_ON, (magic_conference_call_on == false) ? "0" : "1");
    mMagicConferenceCallOn = magic_conference_call_on;
}

void SpeechEnhancementController::SetBtHeadsetNrecOnToAllModem(const bool bt_headset_nrec_on)
{
    SpeechDriverFactory *pSpeechDriverFactory = SpeechDriverFactory::GetInstance();
    SpeechDriverInterface *pSpeechDriver = NULL;

    property_set(PROPERTY_KEY_BT_HEADSET_NREC_ON, (bt_headset_nrec_on == false) ? "0" : "1");
    mBtHeadsetNrecOn = bt_headset_nrec_on;

    for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
    {
        pSpeechDriver = pSpeechDriverFactory->GetSpeechDriverByIndex((modem_index_t)modem_index);
        if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
        {
            pSpeechDriver->SetBtHeadsetNrecOn(mBtHeadsetNrecOn);
        }
    }
}


}
