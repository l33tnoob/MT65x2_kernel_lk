#define LOG_TAG "BWManager" 

#include <sys/types.h>
#include <cutils/xlog.h>
#include "BWManager.h"

namespace android {


    ANDROID_SINGLETON_STATIC_INSTANCE(BWManager);

    BWManager::BWManager()
    {

    }


    // Set the BWC profile with BWC binder service
    status_t BWManager::setProfile(int32_t profile, bool isEnable)
    {    
	     return 0;
    }


};

