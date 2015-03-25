#include "SpeechDriverFactory.h"
#include "SpeechType.h"
#include "SpeechDriverInterface.h"
#include "SpeechDriverDummy.h"
#include "SpeechDriverLAD.h"
#include <DfoDefines.h>

#include <utils/threads.h>

#ifdef EVDO_DT_SUPPORT  //evod modem speech driver
#include "SpeechDriverEVDO.h"
#endif
#define LOG_TAG "SpeechDriverFactory"

namespace android
{

SpeechDriverFactory *SpeechDriverFactory::mSpeechDriverFactory = NULL;
SpeechDriverFactory *SpeechDriverFactory::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);
    ALOGV("%s()", __FUNCTION__);

    if (mSpeechDriverFactory == NULL)
    {
        mSpeechDriverFactory = new SpeechDriverFactory();
    }
    ASSERT(mSpeechDriverFactory != NULL);
    return mSpeechDriverFactory;
}

SpeechDriverFactory::SpeechDriverFactory()
{
    ALOGV("%s()", __FUNCTION__);
    mSpeechDriver1 = NULL;
    mSpeechDriver2 = NULL;
    mSpeechDriverExternal = NULL;

    if (MTK_ENABLE_MD5 == true)
    {
        mActiveModemIndex = MODEM_EXTERNAL; // MTK_ENABLE_MD5: only MODEM_EXTERNAL exists
    }
    else if (MTK_ENABLE_MD1 == true)
    {
        mActiveModemIndex = MODEM_1; // default use modem 1
    }
    else if (MTK_ENABLE_MD2 == true)
    {
        mActiveModemIndex = MODEM_2; // if modem 1 not enabled, default use modem 2
    }
    else
    {
        ALOGE("Somebody forgot to define MTK_ENABLE_MD1/MTK_ENABLE_MD2 for this project");
        mActiveModemIndex = MODEM_1; // default use modem 1
#if defined(EVDO_DT_SUPPORT)
        mActiveModemIndex = MODEM_EXTERNAL; // if modem evdo,default use modem external
#endif
    }

    CreateSpeechDriverInstances();
    ALOGV("-%s(), mActiveModemIndex=%d", __FUNCTION__, mActiveModemIndex);
}
status_t SpeechDriverFactory::CreateSpeechDriverInstances()
{

    //for external modem LTE: use EEMCS
    if (MTK_ENABLE_MD5 == true)
    {
        // for MODEM_EXTERNAL, might use external modem

        if (mSpeechDriverExternal == NULL)
        {
            mSpeechDriverExternal = SpeechDriverLAD::GetInstance(MODEM_EXTERNAL);
        }
    }
    else
    {
        /// Create mSpeechDriver for modem 1
        if (MTK_ENABLE_MD1 == true)
        {
            // for internal modem_1, always return LAD
            mSpeechDriver1 = SpeechDriverLAD::GetInstance(MODEM_1);
        }
        else
        {
            ALOGW("Create SpeechDriverDummy for MODEM_1");
            mSpeechDriver1 = new SpeechDriverDummy(MODEM_1);
        }

        /// Create mSpeechDriver for modem 2
        if (MTK_ENABLE_MD2 == true)
        {
            // for modem_2, might use internal/external modem
            mSpeechDriver2 = SpeechDriverLAD::GetInstance(MODEM_2);
        }
        else
        {
            ALOGW("Create SpeechDriverDummy for MODEM_2");
            mSpeechDriver2 = new SpeechDriverDummy(MODEM_2);
        }

        /// Create mSpeechDriver for modem external
#ifdef EVDO_DT_SUPPORT  //evdo modem speech driver
        ALOGD("Create SpeechDriver for MODEM_EXTERNAL");
        mSpeechDriverExternal = SpeechDriverEVDO::GetInstance(MODEM_EXTERNAL);
#else
        ALOGW("Create SpeechDriverDummy for MODEM_EXTERNAL");
        mSpeechDriverExternal = new SpeechDriverDummy(MODEM_EXTERNAL);
#endif
    }

    return NO_ERROR;
}

status_t SpeechDriverFactory::DestroySpeechDriverInstances()
{
    if (mSpeechDriver1 != NULL)
    {
        delete mSpeechDriver1;
        mSpeechDriver1 = NULL;
    }

    if (mSpeechDriver2 != NULL)
    {
        delete mSpeechDriver2;
        mSpeechDriver2 = NULL;
    }

    if (mSpeechDriverExternal != NULL)
    {
        delete mSpeechDriverExternal;
        mSpeechDriverExternal = NULL;
    }
    return NO_ERROR;
}

SpeechDriverFactory::~SpeechDriverFactory()
{
    DestroySpeechDriverInstances();
}

SpeechDriverInterface *SpeechDriverFactory::GetSpeechDriver()
{
    SpeechDriverInterface *pSpeechDriver = NULL;
    ALOGD("%s(), mActiveModemIndex=%d", __FUNCTION__, mActiveModemIndex);

    switch (mActiveModemIndex)
    {
        case MODEM_1:
            pSpeechDriver = mSpeechDriver1;
            break;
        case MODEM_2:
            pSpeechDriver = mSpeechDriver2;
            break;
        case MODEM_EXTERNAL:
            pSpeechDriver = mSpeechDriverExternal;
            break;
        default:
            ALOGE("%s: no such modem index %d", __FUNCTION__, mActiveModemIndex);
            break;
    }

    ASSERT(pSpeechDriver != NULL);
    return pSpeechDriver;
}

/**
 * NO GUARANTEE that the returned pointer is not NULL!!
 * Be careful to use this function!!
 */
SpeechDriverInterface *SpeechDriverFactory::GetSpeechDriverByIndex(const modem_index_t modem_index)
{
    SpeechDriverInterface *pSpeechDriver = NULL;
    ALOGD("%s(), modem_index=%d", __FUNCTION__, modem_index);

    switch (modem_index)
    {
        case MODEM_1:
            pSpeechDriver = mSpeechDriver1;
            break;
        case MODEM_2:
            pSpeechDriver = mSpeechDriver2;
            break;
        case MODEM_EXTERNAL:
            pSpeechDriver = mSpeechDriverExternal;
            break;
        default:
            ALOGE("%s: no such modem index %d", __FUNCTION__, modem_index);
            break;
    }

    return pSpeechDriver;
}


modem_index_t SpeechDriverFactory::GetActiveModemIndex() const
{
    ALOGD("%s(), active modem index = %d", __FUNCTION__, mActiveModemIndex);
    return mActiveModemIndex;
}

status_t SpeechDriverFactory::SetActiveModemIndex(const modem_index_t modem_index)
{
    ALOGD("%s(), old modem index = %d, new modem index = %d", __FUNCTION__, mActiveModemIndex, modem_index);
    mActiveModemIndex = modem_index;
    return NO_ERROR;
}


status_t SpeechDriverFactory::SetActiveModemIndexByAudioMode(const audio_mode_t audio_mode)
{
    status_t return_status = NO_ERROR;

    switch (audio_mode)
    {
        case AUDIO_MODE_IN_CALL:
            return_status = SetActiveModemIndex(MODEM_1);
            break;
        case AUDIO_MODE_IN_CALL_2:
            return_status = SetActiveModemIndex(MODEM_2);
            break;
        case AUDIO_MODE_IN_CALL_EXTERNAL:
            return_status = SetActiveModemIndex(MODEM_EXTERNAL);
            break;
        default:
            ALOGE("%s() mode(%d) is neither MODE_IN_CALL nor MODE_IN_CALL_2!!", __FUNCTION__, audio_mode);
            return_status = INVALID_OPERATION;
            break;
    }
    return return_status;
}


} // end of namespace android

