#include "AudioSampleRateController.h"

#include "AudioFMController.h"
#include "AudioMATVController.h"

#define LOG_TAG  "AudioMTKStreamOut"

namespace android
{

/*==============================================================================
 *                     Property keys
 *============================================================================*/


/*==============================================================================
 *                     Const Value
 *============================================================================*/


/*==============================================================================
 *                     Enumerator
 *============================================================================*/


/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioSampleRateController *AudioSampleRateController::mAudioSampleRateController = NULL;

AudioSampleRateController *AudioSampleRateController::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mAudioSampleRateController == NULL)
    {
        mAudioSampleRateController = new AudioSampleRateController();
    }
    ASSERT(mAudioSampleRateController != NULL);
    return mAudioSampleRateController;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioSampleRateController::AudioSampleRateController()
{
    ALOGD("%s()", __FUNCTION__);
    mStreamOut= NULL;
}

AudioSampleRateController::~AudioSampleRateController()
{
    ALOGD("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     SampleRate Control
 *============================================================================*/

uint32_t AudioSampleRateController::RegistreStreamOut(AudioMTKStreamOut* streamout)
{
    ALOGD("%s()", __FUNCTION__);
    mStreamOut = streamout;
    return NO_ERROR;
}

uint32_t AudioSampleRateController::AddRecord(SampleRateScenario_T scenario, uint32_t samplerate, Setting_Result_T result)
{

    ALOGD("%s(scenario=%d,samplrate=%d,result=%d)", __FUNCTION__, scenario, samplerate, result);
    if(mRecord.size() >= MAX_RECORD)
    {    
        mRecord.removeAt(0);
    }

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    Setting_T tmp;
    tmp.Scenario = scenario;
    tmp.SampleRate = samplerate;
    tmp.Result = result;    
    tmp.Time = *tm;

    mRecord.push(tmp);

    ALOGD("localtime:%02d/%02d/%02d-%02d:%02d:%02d\n",
        tmp.Time.tm_mon + 1, tmp.Time.tm_mday, tmp.Time.tm_year + 1900,
           tmp.Time.tm_hour, tmp.Time.tm_min, tmp.Time.tm_sec);
    return NO_ERROR;
}

uint32_t AudioSampleRateController::ApplySampleRate(SampleRateScenario_T scenario, uint32_t samplerate)
{
    ALOGD("%s(scenario=%d,samplrate=%d)", __FUNCTION__, scenario, samplerate);
    Setting_Result_T result = FAILD_NOT_ALLOW;

    if(mStreamOut==NULL)
    {
        ALOGD("%s() Not support hal", __FUNCTION__);
        AddRecord(scenario, samplerate, FAILD_NOT_ALLOW);
        return INVALID_OPERATION;        
    }
    
    //reject request when playing
    if (mStreamOut->GetStreamRunning() || 
            AudioFMController::GetInstance()->GetFmEnable() == true ||
            AudioMATVController::GetInstance()->GetMatvEnable() == true )
    { 
        ALOGD("%s() Stream is running, reject reset sample rate", __FUNCTION__);
        AddRecord(scenario, samplerate, FAILD_NOT_ALLOW);
        return INVALID_OPERATION;
    }
    
    return INVALID_OPERATION; ////////////////for NXP debug

    // Change sample rate or do some statistic here here
    if(samplerate != 48000)
    {
        samplerate = 44100;
    }

    if(mStreamOut->sampleRate()==samplerate)
    {
        AddRecord(scenario, samplerate, SUCCESS_NO_CHANGE);
        return NO_ERROR;
    }

    if(mStreamOut->ResetSampleRate(samplerate))
    {
        AddRecord(scenario, samplerate, SUCCESS_DID_CHANGE);
        return NO_ERROR;
    }else{
        result = FAILD_NOT_ALLOW;
        AddRecord(scenario, samplerate, FAILD_NOT_ALLOW);
        return INVALID_OPERATION;        
    }
    
    return NO_ERROR;
}


bool AudioSampleRateController::Lock(void)
{
    ALOGD("%s()", __FUNCTION__);
    return mStreamOut->StreamOutLock();
}

bool AudioSampleRateController::Unlock(void)
{
    ALOGD("%s()", __FUNCTION__);
    return mStreamOut->StreamOutUnlock();
}


} // end of namespace android
