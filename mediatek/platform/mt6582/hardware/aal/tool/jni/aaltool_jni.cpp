#define LOG_TAG "AALTOOL_JNI"

#include <jni.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <linux/sensors_io.h>
#include "AALClient.h"

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jint JNICALL Java_com_mediatek_aaltool_AALALSCalibration_nGetALSRawData(JNIEnv * env, jobject jobj)
{
    int err = 0;
    int als = 0;
    int fd = open("/dev/als_ps", O_RDONLY);
    
    if (fd < 0) 
    {
        ALOGE("Fail to open alsps device (error: %s)\n", strerror(errno));
        return -1;
    }
    
    if ((err = ioctl(fd, ALSPS_GET_ALS_RAW_DATA, &als)))
    {
        ALOGE("ioctl ALSPS_GET_ALS_RAW_DATA error: %d\n", err);
        close(fd);
        return -1;
    }
    
    ALOGD("als = %d\n", als);
    close(fd);
    return als;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBrightnessLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Brightness level = %d", level);
    if (AALClient::getInstance().setBrightnessLevel(level) != 0)
    {
        ALOGE("fail to set Brightness level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetDarkeningSpeedLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("DarkeningSpeed level = %d", level);
    if (AALClient::getInstance().setDarkeningSpeedLevel(level) != 0)
    {
        ALOGE("fail to set Darkening Speed level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBrighteningSpeedLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("BrighteningSpeed level = %d", level);
    if (AALClient::getInstance().setBrighteningSpeedLevel(level) != 0)
    {
        ALOGE("fail to set Brightening Speed level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetSmartBacklightLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("SmartBacklight level = %d", level);
    if (AALClient::getInstance().setSmartBacklightLevel(level) != 0)
    {
        ALOGE("fail to set SmartBacklight level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetToleranceRatioLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Tolerance Ratio level = %d", level);
    if (AALClient::getInstance().setToleranceRatioLevel(level) != 0)
    {
        ALOGE("fail to set Tolerance Ratio level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetReadabilityLevel(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Readability level = %d", level);
    if (AALClient::getInstance().setReadabilityLevel(level) != 0)
    {
        ALOGE("fail to set Readability level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


JNIEXPORT jboolean JNICALL Java_com_mediatek_aaltool_AALTuning_nSetBacklight(JNIEnv * env, jobject jobj, jint level)
{
    ALOGD("Backlight level = %d", level);
    if (AALClient::getInstance().setBacklightBrightness(level) != 0)
    {
        ALOGE("fail to set backlight level");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

#ifdef __cplusplus
}
#endif
