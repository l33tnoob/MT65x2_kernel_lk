#define LOG_TAG "AAL"

#include <cutils/xlog.h>
#include <binder/BinderService.h>
#include <AALService.h>

using namespace android;

int main(int argc, char** argv) 
{
    XLOGD("AAL service start...");

    AALService::publishAndJoinThreadPool(true);
    // When AAL is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    XLOGD("AAL service exit...");
    return 0;
}
