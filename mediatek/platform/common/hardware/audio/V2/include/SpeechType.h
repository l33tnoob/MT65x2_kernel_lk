#ifndef ANDROID_SPEECH_TYPE_H
#define ANDROID_SPEECH_TYPE_H

#include "AudioType.h"

namespace android
{

#ifdef SLOGV
#undef SLOGV
#if 0 // for speech debug usage
#define SLOGV(...) ALOGD(__VA_ARGS__)
#else
#define SLOGV(...) ALOGV(__VA_ARGS__)
#endif
#endif
enum modem_index_t
{
    MODEM_1   = 0,
    MODEM_2   = 1,
    MODEM_EXTERNAL   = 2,
    NUM_MODEM
};

enum modem_status_mask_t
{
    SPEECH_STATUS_MASK   = (1 << 0),
    RECORD_STATUS_MASK   = (1 << 1),
    BGS_STATUS_MASK      = (1 << 2),
    P2W_STATUS_MASK      = (1 << 3),
    TTY_STATUS_MASK      = (1 << 4),
    VT_STATUS_MASK       = (1 << 5),
    LOOPBACK_STATUS_MASK = (1 << 6),
    VM_RECORD_STATUS_MASK   = (1 << 7),
};

enum speech_mode_t
{
    SPEECH_MODE_NORMAL          = 0,
    SPEECH_MODE_EARPHONE        = 1,
    SPEECH_MODE_LOUD_SPEAKER    = 2,
    SPEECH_MODE_BT_EARPHONE     = 3,
    SPEECH_MODE_BT_CORDLESS     = 4,
    SPEECH_MODE_BT_CARKIT       = 5,
    SPEECH_MODE_MAGIC_CON_CALL  = 6,
    SPEECH_MODE_PRESERVED_2     = 7,
    SPEECH_MODE_NO_CONNECT      = 8
};

enum phone_call_mode_t
{
    RAT_2G_MODE     = 0, // 2G phone call
    RAT_3G_MODE     = 1, // 3G phone call // for both 2G/3G phone call, set mode as 2G. Modem side can query 2G/3G phone call.
    RAT_3G324M_MODE = 2, // VT phone call
};

/*enum tty_mode_t {
    TTY_ERR  = -1,
    TTY_OFF  = 0,
    TTY_FULL = 1,
    TTY_VCO  = 2,
    TTY_HCO  = 4
};*/

enum ctm_interface_t   // L1 CTM Interface
{
    DIRECT_MODE = 0,
    BAUDOT_MODE = 1
};


enum record_format_t
{
    RECORD_FORMAT_PCM         = 0,
    RECORD_FORMAT_VM          = 1,
    RECORD_FORMAT_DUAL_MIC_VM = 2,
    RECORD_FORMAT_CTM_4WAY    = 3,
};

enum record_sample_rate_t
{
    RECORD_SAMPLE_RATE_08K = 0,
    RECORD_SAMPLE_RATE_16K = 1,
    RECORD_SAMPLE_RATE_32K = 2,
    RECORD_SAMPLE_RATE_48K = 3
};

enum record_channel_t
{
    RECORD_CHANNEL_MONO = 0,
    RECORD_CHANNEL_STEREO = 1
};

enum vm_record_format_t
{
    VM_RECORD_VM_MASK      = 0x0001,
    VM_RECORD_CTM4WAY_MASK = 0x0002,
};


// define for dual mic pcm2way format
enum dualmic_pcm2way_format_t
{
    P2W_FORMAT_NORMAL = 0,
    P2W_FORMAT_VOIP   = 1,
    P2W_FORMAT_NB_CAL = 2, // NB calibration
    P2W_FORMAT_WB_CAL = 3, // WB calibration
};

enum pcmnway_format_t
{
    SPC_PNW_MSG_BUFFER_SE  = (1 << 0), // Bit 0, PCM4WAY_PutToSE
    SPC_PNW_MSG_BUFFER_SPK = (1 << 1), // Bit 1, PCM4WAY_PutToSpk
    SPC_PNW_MSG_BUFFER_MIC = (1 << 2), // Bit 2, PCM4WAY_GetFromMic
    SPC_PNW_MSG_BUFFER_SD  = (1 << 3), // Bit 3, PCM4WAY_GetFromSD
};

// speech enhancement function mask
// This is the power on/off setting of enhancement. Most of the case, it should be totally on.
enum sph_enh_main_mask_t
{
    //SPH_ENH_MAIN_MASK_ES     = (1 << 0),
    SPH_ENH_MAIN_MASK_AEC      = (1 << 1),
    //SPH_ENH_MAIN_MASK_EES    = (1 << 2),
    SPH_ENH_MAIN_MASK_ULNR     = (1 << 3), // VCE depends on this
    SPH_ENH_MAIN_MASK_DLNR     = (1 << 4), // VCE depends on this
    SPH_ENH_MAIN_MASK_TDNC     = (1 << 5),
    SPH_ENH_MAIN_MASK_DMNR     = (1 << 6), // Enable only when phone with dual mic
    SPH_ENH_MAIN_MASK_SIDETONE = (1 << 7),
#if defined(MTK_DUAL_MIC_SUPPORT)
    SPH_ENH_MAIN_MASK_ALL      = 0xFFFF,
#else
    SPH_ENH_MAIN_MASK_ALL      = (0xFFFF & (~SPH_ENH_MAIN_MASK_DMNR)),
#endif
};

// speech enhancement function dynamic mask
// This is the dynamic switch to decided the enhancment output.
enum sph_enh_dynamic_mask_t
{
    SPH_ENH_DYNAMIC_MASK_DMNR      = (1 << 0), // for receiver
    SPH_ENH_DYNAMIC_MASK_VCE       = (1 << 1),
    SPH_ENH_DYNAMIC_MASK_BWE       = (1 << 2),
    SPH_ENH_DYNAMIC_MASK_LSPK_DMNR = (1 << 5), // for loud SPEAKER_AMP
    SPH_ENH_DYNAMIC_MASK_SIDEKEY_DGAIN = (1 << 6) ,//digital gain control
#if defined(MTK_DUAL_MIC_SUPPORT)
    SPH_ENH_DYNAMIC_MASK_ALL       = 0xFFFFFFFF,
#else
    SPH_ENH_DYNAMIC_MASK_ALL       = (0xFFFFFFFF & (~SPH_ENH_DYNAMIC_MASK_DMNR)),
#endif
};

typedef struct
{
    uint16_t main_func;    // ES,AEC,EES,ULNR,DLNR,TDNC,DMNR,SIDETONE, ...
    uint32_t dynamic_func; // DMNR,VCE,BWE,
} sph_enh_mask_struct_t;


} // end namespace android

#endif
