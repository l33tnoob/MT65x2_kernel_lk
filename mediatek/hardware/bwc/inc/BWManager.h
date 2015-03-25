#ifndef __BWC_SERVICE_MANAGER_H__
#define __BWC_SERVICE_MANAGER_H__

#include <stdint.h>
#include <sys/types.h>
#include <utils/Singleton.h>

namespace android {

    // Bandwidth Manager class
    // API layer of BWC service
    class BWManager : public Singleton<BWManager>
    {
        friend class Singleton<BWManager>;

    public:
        status_t setProfile(int32_t profile, bool isEnable);

    private:    
        BWManager();

    };

};

#endif
