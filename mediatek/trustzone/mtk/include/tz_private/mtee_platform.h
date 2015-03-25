/*
 * MTEE Platform related init
 *
 * Function that platform can use, or function that platform should export
 *
 */

#ifndef __MTEE_PLATFORM_H__
#define __MTEE_PLATFORM_H__

/***************************************************************
 *  Platform provided functions
 ***************************************************************/

/**
 * Get platform SRAM size & reserved size.
 * For now, SRAM MUST start at 0x100000
 *
 * @param pSize return SRAM size
 * @param pReserved return SRAM reserved size at SRAM end.
 */
void platform_get_sram_size(unsigned int *pSize, unsigned int *pReserved);

/**
 * Platform early init
 * Do early init necessary for the platform.
 * (ex, interrupt/IO memory map...)
 */
void platform_early_init(void);

/**
 * Boot up secondary CPU
 * Called after platform_early_init.
 * Should wakeup all CPUs and let them enter MTEE initia
 */
void platform_boot_secondary(void);

/**
 * Init secondary CPU.
 * Will be run on secondary CPUs when first boot.
 */
void platform_secondary_init(void);

/**
 * Secondary CPU wait, wait for kernel.
 * Called after all init is done for secondary CPU. This should wait for
 * kernel to start secondary CPU.
 */
void platform_secondary_wait(void);

/**
 * Stop secondary CPUs.
 * Will be run when secondary CPUs about to be off-line.
 */
void platform_cpu_enter_lowpower(void);

/**
 * Secondary CPUs leave low power state.
 * Cancel previous enter lowpower.
 */
void platform_cpu_leave_lowpower(void);

/**
 * CPU dormant function
 */
int platform_cpu_dormant(int mode);

/**
 * Return from CPU dormant without power-down
 */
void platform_cpu_dormant_abort(void);

/**
 * Save platform context before dormant
 */
void platform_save_context(void);

/**
 * Restore platform context after dormant
 */
void platform_restore_context(void);


/***************************************************************
 *  System provided functions
 ***************************************************************/

/**
 * System entry function.
 * Wakeup core must jump to physical address of this function
 */
void secondary_startup(void);

/**
 * Setup return address after trustzone init complete.
 */
void start_helper(unsigned int addr);

/**
 * Check if an CPU is online */
int cpu_online(int cpuid);

/**
 * Mark an CPU as online or offline */
void mark_cpu_stats(int cpuid, int online);

#define mark_cpu_online(cpuid)    mark_cpu_stats(cpuid, 1);
#define mark_cpu_offline(cpuid)   mark_cpu_stats(cpuid, 0);

/*
 * Add current thread stack for tracking.
 * Used to track thread stack for CPU power-down.
 */
void MTEE_TrackCpuThreadStack(int cpuid);

/*
 * Untrack current thread stack for CPU power-down abort
 */
void MTEE_UntrackCpuThreadStack(int cpuid);

/*
 * Free tracked CPU thread stack(if any)
 */
void MTEE_FreeTrackedCpuThreadStack(int cpuid);

#endif /* __MTEE_PLATFORM_H__ */
