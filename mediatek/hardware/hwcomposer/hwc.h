#ifndef _HWCOMPOSER_
#define _HWCOMPOSER_

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <utils/Singleton.h>

using namespace android;

// ---------------------------------------------------------------------------

typedef struct hwc_private_device
{
    hwc_composer_device_1_t base;

    /* our private state goes below here */
    uint32_t tag;
    hwc_procs_t* procs;
} hwc_private_device_t;

// ---------------------------------------------------------------------------

class HWCMediator : public Singleton<HWCMediator>
{
public:
    HWCMediator();

    ~HWCMediator();

    void open(hwc_private_device_t* device);

    void close(hwc_private_device_t* device);

    int prepare(size_t num_display, hwc_display_contents_1_t** displays);

    int set(size_t num_display, hwc_display_contents_1_t** displays);

    int eventControl(int dpy, int event, int enabled);

    int blank(int dpy, int blank);

    int query(int what, int* value);

    void dump(char* buff, int buff_len);

    int getConfigs(int dpy, uint32_t* configs, size_t* numConfigs);

    int getAttributes(int dpy, uint32_t config,
            const uint32_t* attributes, int32_t* values);

    hwc_feature_t m_features;

    void initFeatures();
};

#endif // _HWCOMPOSER_
