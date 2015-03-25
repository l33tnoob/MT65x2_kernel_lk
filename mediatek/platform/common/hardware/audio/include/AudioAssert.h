#ifndef ANDROID_AUDIO_ASSERT_H
#define ANDROID_AUDIO_ASSERT_H

#ifdef HAVE_AEE_FEATURE

#include <aee.h>

#else // no HAVE_AEE_FEATURE

#ifndef DB_OPT_DEFAULT
#define DB_OPT_DEFAULT (0)
#endif

static int aee_system_exception(const char *module, const char* path, unsigned int flags, const char *msg,...)
{
    return 0;
}

static int aee_system_warning(const char *module, const char* path, unsigned int flags, const char *msg,...)
{
    return 0;
}
#endif // end of HAVE_AEE_FEATURE


#define ASSERT(exp) \
    do { \
        if (!(exp)) { \
            ALOGE("ASSERT("#exp") fail: \""  __FILE__ "\", %uL", __LINE__); \
            aee_system_exception("[Audio]", NULL, DB_OPT_DEFAULT, " %s, %uL", strrchr(__FILE__, '/') + 1, __LINE__); \
        } \
    } while(0)

#define WARNING(string) \
    do { \
        ALOGW("WARNING("string") fail: \""  __FILE__ "\", %uL", __LINE__); \
        aee_system_warning("[Audio]", NULL, DB_OPT_DEFAULT, string); \
    } while(0)



#endif // end of ANDROID_AUDIO_ASSERT_H
