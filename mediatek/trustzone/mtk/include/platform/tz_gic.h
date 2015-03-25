#include <stddef.h>

#ifndef _TZ_GIC_H_
#define _TZ_GIC_H_

/* GIC Public function prototype */
void tz_gic_init(unsigned int dist_base, unsigned int cpu_base, 
                 const unsigned int *allowed_fiq_list, int fiq_number);
void tz_gic_cpu_init(void);
void gic_raise_softirq(unsigned int cpumask, unsigned int irq);
void gic_ack_softirq(unsigned int irq);
void gic_save_context(void);
void gic_restore_context(void);

/**
 * TZ FIQ set.
 */
TZ_RESULT gic_fiq_set(int irq, unsigned long irq_flags);

/**
 * TZ GIC FIQ enable.
 */
TZ_RESULT gic_fiq_enable(int irq, int enable);

/**
 * TZ GIC FIQ query
 */
TZ_RESULT gic_fiq_query(int irq, unsigned int *pstate);

/**
 * Get GIC INTACT register
 */
TZ_RESULT gic_fiq_get_intack(unsigned int *iar);

/**
 * AcK GIC EOI
 */
TZ_RESULT gic_fiq_eoi(unsigned int iar);

/**
 * Trigger FIQ SGI.
 *
 * @param mask The CPU to send this irq.
 * @param irq The irq number to send. Must be FIQ.
 * @return return value.
 */
TZ_RESULT gic_trigger_softfiq(unsigned int mask, int irq);

/**
 * Mask all irq
 */
TZ_RESULT gic_irq_mask_all(unsigned int *pmask, unsigned int size);

/**
 * Restore all irq mask
 */
TZ_RESULT gic_irq_mask_restore(unsigned int *pmask, unsigned int size);


#endif
