#ifndef __DP_CONFIG_H__
#define __DP_CONFIG_H__

#define CONFIG_FOR_OS_WINDOWS       0

#define CONFIG_FOR_OS_ANDROID       1

#if (CONFIG_FOR_OS_WINDOWS && CONFIG_FOR_OS_ANDROID)
    #error "Please specify the correct platform"
#endif

#define CONFIG_FOR_TPIPE_FINFO      0

#define CONFIG_FOR_PROFILE_INFO     0

#define CONFIG_FOR_DUMP_COMMAND     0

#define CONFIG_FOR_DUMP_OUTPUT      0

#define CONFIG_FOR_FLUSH_RANGE      0

#define CONFIG_FOR_VERIFY_FPGA      0

#define CONFIG_FOR_6582_SERIES      0

#define CONFIG_FOR_SEC_VIDEO_PATH   defined(TRUSTONIC_TEE_SUPPORT) && defined(MTK_SEC_VIDEO_PATH_SUPPORT)

#endif  // __DP_CONFIG_H__
