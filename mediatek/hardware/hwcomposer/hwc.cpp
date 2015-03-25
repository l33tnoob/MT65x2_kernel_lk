#include <cutils/properties.h>

#include <cutils/xlog.h>

#include "hwc.h"

// ---------------------------------------------------------------------------

void HWCMediator::initFeatures()
{
    memset(&m_features, 0, sizeof(hwc_feature_t));

#ifdef MTK_EXTERNAL_SUPPORT
    m_features.externals = 1;
#endif

#ifdef MTK_VIRTUAL_SUPPORT
    m_features.virtuals = 1;
#endif

#ifdef MTK_ENHAHCE_SUPPORT
    m_features.enhance = 1;
#endif

#ifdef MTK_STEREO_SUPPORT
    m_features.stereoscopic = 1;
#endif
}

// ---------------------------------------------------------------------------

static int hwc_device_open(
    const struct hw_module_t* module,
    const char* name,
    struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag:                HARDWARE_MODULE_TAG,
        module_api_version: HWC_MODULE_API_VERSION_0_1,
        hal_api_version:    HARDWARE_HAL_API_VERSION,
        id:                 HWC_HARDWARE_MODULE_ID,
        name:               "MediaTek Hardware Composer HAL",
        author:             "MediaTek Inc.",
        methods:            &hwc_module_methods,
    }
};

// ---------------------------------------------------------------------------

static int hwc_prepare(
    struct hwc_composer_device_1* dev, size_t num_display,
    hwc_display_contents_1_t** displays)
{
    return HWCMediator::getInstance().prepare(num_display, displays);
}

static int hwc_set(
    struct hwc_composer_device_1* dev, size_t num_display,
    hwc_display_contents_1_t** displays)
{
    return HWCMediator::getInstance().set(num_display, displays);
}

static int hwc_event_control(
    struct hwc_composer_device_1* dev,
    int disp, int event, int enabled)
{
    return HWCMediator::getInstance().eventControl(disp, event, enabled);
}

static int hwc_blank(
    struct hwc_composer_device_1* dev,
    int disp, int blank)
{
    return HWCMediator::getInstance().blank(disp, blank);
}

static int hwc_query(
    struct hwc_composer_device_1* dev,
    int what, int* value)
{
    return HWCMediator::getInstance().query(what, value);
}

static void hwc_register_procs(
    struct hwc_composer_device_1* dev,
    hwc_procs_t const* procs)
{
    hwc_private_device_t* hwc_dev = (hwc_private_device_t*) dev;
    hwc_dev->procs = (typeof(hwc_dev->procs)) procs;
}

static void hwc_dump(
    struct hwc_composer_device_1* dev,
    char* buff, int buff_len)
{
    HWCMediator::getInstance().dump(buff, buff_len);
}

static int hwc_get_display_configs(
    struct hwc_composer_device_1* dev, int disp,
    uint32_t* configs, size_t* numConfigs)
{
    return HWCMediator::getInstance().getConfigs(disp, configs, numConfigs);
}

static int hwc_get_display_attributes(
    struct hwc_composer_device_1* dev, int disp,
    uint32_t config, const uint32_t* attributes, int32_t* values)
{
    return HWCMediator::getInstance().getAttributes(disp, config,
                                                    attributes, values);
}

static int hwc_device_close(struct hw_device_t* device)
{
    hwc_private_device_t* dev = (hwc_private_device_t*) device;
    if (dev)
    {
        HWCMediator::getInstance().close(dev);

        free(dev);
    }
    return 0;
}

// ---------------------------------------------------------------------------

static int hwc_device_open(
    const struct hw_module_t* module,
    const char* name,
    struct hw_device_t** device)
{
    hwc_private_device_t* dev;

    if (strcmp(name, HWC_HARDWARE_COMPOSER))
        return -EINVAL;

    dev = (hwc_private_device_t*) malloc(sizeof(*dev));
    if (dev == NULL)
        return -ENOMEM;

    // initialize our state here
    memset(dev, 0, sizeof(*dev));

    // initialize the procs
    dev->base.common.tag            = HARDWARE_DEVICE_TAG;
#if defined(MTK_HWC_VER_1_3)
    dev->base.common.version        = HWC_DEVICE_API_VERSION_1_3;
#elif defined(MTK_HWC_VER_1_2)
    dev->base.common.version        = HWC_DEVICE_API_VERSION_1_2;
#else
    dev->base.common.version        = HWC_DEVICE_API_VERSION_1_0;
#endif
    dev->base.common.module         = const_cast<hw_module_t*>(module);
    dev->base.common.close          = hwc_device_close;

    dev->base.prepare               = hwc_prepare;
    dev->base.set                   = hwc_set;
    dev->base.eventControl          = hwc_event_control;
    dev->base.blank                 = hwc_blank;
    dev->base.query                 = hwc_query;
    dev->base.registerProcs         = hwc_register_procs;
    dev->base.dump                  = hwc_dump;
    dev->base.getDisplayConfigs     = hwc_get_display_configs;
    dev->base.getDisplayAttributes  = hwc_get_display_attributes;

    *device = &dev->base.common;

    HWCMediator::getInstance().open(dev);

    return 0;
}
