/*
 */

#ifndef _MT_SECURE_API_H_
#define _MT_SECURE_API_H_

/* Use the arch_extension sec pseudo op before switching to secure world */
#if defined(__GNUC__) && \
	defined(__GNUC_MINOR__) && \
	defined(__GNUC_PATCHLEVEL__) && \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)) \
	>= 40502
#define MC_ARCH_EXTENSION_SEC
#endif


/*
 * return code for fastcalls
 */
#define MC_FC_RET_OK					0
#define MC_FC_RET_ERR_INVALID			1



/* t-base fastcall  */
/* 
 * Command to inform SW that we will enter in Power Sleep (Dormant).
 * Parameters:
 *   - param0, the NW wakeup addr (phys).
 *   - param1, the cpuID who will sleep
 *   - param2, unused.
 * Return:
 *   - Cannot fail, always MC_FC_RET_OK.
 */
#define MC_FC_SLEEP                     -3
/* 
 * Command to write NW reset addr for Slave cpus.
 * Parameters:
 *   - param0, the NW reset addr (phys).
 *   - param1, the cpuID.
 *   - param2, unused.
 * Return:
 *   - Cannot fail, always MC_FC_RET_OK.
 */
#define MC_FC_SET_RESET_VECTOR		-301
/* 
 * Command to switch off BootRom.
 * Parameters:
 *   - param0, unused.
 *   - param1, unsued.
 *   - param2, unused.
 * Return:
 *   - Cannot fail, always MC_FC_RET_OK.
 */
#define MC_FC_TURN_OFF_BOOTROM		-302
/* 
 * Command to cancel a sleep command in case of Dormant abort.
 * Parameters:
 *   - param0, unused.
 *   - param1, unsued.
 *   - param2, unused.
 * Return:
 *   - Cannot fail, always MC_FC_RET_OK.
 */
#define MC_FC_SLEEP_CANCELLED		-303



/*
 * mt_secure_call
 * Parameters:
 *   - cmd, the command we want to execute.
 *   - param0, param1, param2: the parameters used for this command.
 * Return:
 *   - Error code of the command (MC_FC_RET_OK in case of success).
 */
static inline int mt_secure_call(uint32_t cmd, uint32_t param0, uint32_t param1, uint32_t param2)
{
	/* SMC expect values in r0-r3 */
	register u32 reg0 __asm__("r0") = cmd;
	register u32 reg1 __asm__("r1") = param0;
	register u32 reg2 __asm__("r2") = param1;
	register u32 reg3 __asm__("r3") = param2;
	int ret = 0;

	__asm__ volatile (
#ifdef MC_ARCH_EXTENSION_SEC
		/* This pseudo op is supported and required from
		 * binutils 2.21 on */
		".arch_extension sec\n"
#endif
		"smc 0\n"
		: "+r"(reg0), "+r"(reg1), "+r"(reg2), "+r"(reg3)
	);

	/* set response */
	ret = reg0;
	return ret;
}


#endif /* _MT_SECURE_API_H_ */
