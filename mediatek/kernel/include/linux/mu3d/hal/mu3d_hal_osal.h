#ifndef	_USB_OSAI_H_
#define	_USB_OSAI_H_
#include <linux/delay.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/mu3d/hal/mu3d_hal_comm.h>
#include <linux/mu3d/hal/mu3d_hal_hw.h>


#undef EXTERN

#ifdef _USB_OSAI_EXT_
#define EXTERN
#else
#define EXTERN extern
#endif


#define K_EMERG		1
#define K_ALET		2
#define K_CRIT		3
#define K_ERR		4
#define K_WARNIN	5
#define K_NOTICE	6
#define K_INFO		7
#define K_DEBUG		8

//static int debug_level = 8;
extern int debug_level;

//#define MGC_DebugLevel K_ERR
//#define MGC_DebugDisable 0
//#define MGC_GetDebugLevel()	(MGC_DebugLevel)
//#define _dbg_level(level)  ( !MGC_DebugDisable && ( level>=-1 && MGC_GetDebugLevel()>=level  ))
#define xprintk(level, format, args...) do { \
		if ( debug_level >= level ) { \
			printk("[U3D]"); \
			printk(format, ## args); \
		} \
	} while (0)

#define os_printk(level,fmt,args...) xprintk(level,fmt, ## args)
#ifdef NEVER
#define os_ASSERT  while(1)
#endif /* NEVER */
#define OS_R_OK                    ((DEV_INT32)   0)

EXTERN spinlock_t	_lock;
EXTERN DEV_INT32  os_reg_isr(DEV_UINT32 irq,irq_handler_t handler,void *isrbuffer);
EXTERN void os_ms_delay (DEV_UINT32  ui4_delay);
EXTERN void os_us_delay (DEV_UINT32  ui4_delay);
//EXTERN void os_ms_sleep (DEV_UINT32  ui4_sleep);

#ifdef NEVER
EXTERN void os_spin_lock(spinlock_t *lock);
EXTERN void os_spin_unlock(spinlock_t *lock);
#endif /* NEVER */

void os_memcpy(DEV_INT8 *pv_to, DEV_INT8 *pv_from, size_t z_l);
EXTERN void *os_memset(void *pv_to, DEV_UINT8 ui1_c, size_t z_l);
EXTERN void *os_mem_alloc(size_t z_size);

#ifdef NEVER
EXTERN void *os_virt_to_phys(void *vaddr);
#endif /* NEVER */

EXTERN void *os_phys_to_virt(void *paddr);

#ifdef NEVER
EXTERN void *os_ioremap(void *paddr,DEV_UINT32 t_size);
#endif /* NEVER */

EXTERN void os_mem_free(void *pv_mem);
EXTERN void os_disableIrq(DEV_UINT32 irq);
EXTERN void os_disableIrq(DEV_UINT32 irq);
EXTERN void os_enableIrq(DEV_UINT32 irq);
EXTERN void os_clearIrq(DEV_UINT32 irq);
EXTERN void os_get_random_bytes(void *buf,DEV_INT32 nbytes);
EXTERN void os_disableDcache(void);
EXTERN void os_flushinvalidateDcache(void);


#undef EXTERN


#endif
