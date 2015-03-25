#include <linux/mu3d/hal/mu3d_hal_comm.h>
#include <linux/mu3d/hal/mu3d_hal_osal.h>

DEV_INT32 wait_for_value(DEV_INT32 addr, DEV_INT32 msk, DEV_INT32 value, DEV_INT32 ms_intvl, DEV_INT32 count)
{
	DEV_UINT32 i;

	for (i = 0; i < count; i++)
	{
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		os_ms_delay(ms_intvl);
	}

	return RET_FAIL;
}

#ifdef NEVER

DEV_INT32 wait_for_value_tmout(DEV_INT32 addr, DEV_INT32 msk, DEV_INT32 value, DEV_INT32 ms_intvl, DEV_INT32 count)
{
	DEV_UINT32 i;

	for (i = 0; i < count; i++)
	{
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		os_ms_sleep(ms_intvl);
	}

	return RET_FAIL;
}

DEV_INT32 wait_until_true(DEV_INT32 addr, DEV_INT32 msk, DEV_INT32 value, DEV_INT32 ms_intvl, DEV_INT32 count)
{
	DEV_UINT32 i;

	for (i = 0; i < count; i++)
	{
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		os_ms_sleep(ms_intvl);
	}

	return RET_FAIL;
}
#endif
