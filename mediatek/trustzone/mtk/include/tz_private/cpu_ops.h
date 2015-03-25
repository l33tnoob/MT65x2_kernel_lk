/*
 * Prototypes for cache/cpu related ops */


#ifndef __TZ_CPU_OPS_H__
#define __TZ_CPU_OPS_H__

void cache_flush_louis(void);
void cache_flush_all(void);
void __disable_dcache(void);
void __enable_dcache(void);
void __switch_to_amp(void);
void __switch_to_smp(void);

#endif /* __TZ_CPU_OPS_H__ */
