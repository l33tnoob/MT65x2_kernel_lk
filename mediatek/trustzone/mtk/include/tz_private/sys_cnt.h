
#ifndef __MTEE_SYS_CNT_H__
#define __MTEE_SYS_CNT_H__

/**
 * Return a 64-bit non-rewind counter.
 */
uint64_t get_incr_only_count(void);

/**
 * Return a 32-bit counter. Note that it may rewind.
 */
uint32_t get_raw_count(void);

/**
 * get gpt clock rate.
 */
uint32_t MTEE_get_gpt_clock_rate(void);

#endif
