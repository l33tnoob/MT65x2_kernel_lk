#ifndef _MT_IDLE_H
#define _MT_IDLE_H

#include <mach/mt_spm_api.h>

#ifdef SPM_MCDI_FUNC
extern void enable_mcidle_by_bit(int id);
extern void disable_mcidle_by_bit(int id);
#else
static inline void enable_mcidle_by_bit(int id) { }
static inline void disable_mcidle_by_bit(int id) { }
#endif

extern void enable_dpidle_by_bit(int id);
extern void disable_dpidle_by_bit(int id);

extern bool idle_state_get(u8 idx);
extern bool idle_state_en(u8 idx, bool en);

enum {
    IDLE_TYPE_MC = 0,
    IDLE_TYPE_DP = 1,
    IDLE_TYPE_RG = 2,
    NR_TYPES = 3,
};



#endif
