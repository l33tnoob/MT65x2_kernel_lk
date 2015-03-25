/*This file implements MTK boot mode.*/

#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>

#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <platform/mtk_key.h>

#include <target/cust_key.h>

#define MODULE_NAME "[FACTORY]"

BOOL factory_check_key_trigger(void)
{
#if 1
	//wait
	ulong begin = get_timer(0);
	printf("\n%s Check factory boot\n",MODULE_NAME);
	printf("%s Wait 50ms for special keys\n",MODULE_NAME);
	
	/* If the boot reason is RESET, than we will NOT enter factory mode. */
	if(mtk_detect_pmic_just_rst())
	{
	  return false;
	}
	
    while(get_timer(begin)<50)
    {    
		if(mtk_detect_key(MT65XX_FACTORY_KEY))
		{	
			printf("%s Detect key\n",MODULE_NAME);
			printf("%s Enable factory mode\n",MODULE_NAME);		
			g_boot_mode = FACTORY_BOOT;
			//video_printf("%s : detect factory mode !\n",MODULE_NAME);
			return TRUE;
		}
	}
#endif		
	return FALSE;		
}
extern BOOT_ARGUMENT *g_boot_arg;
BOOL factory_detection(void)
{
    int forbid_mode = 0;
    
    if(g_boot_arg->sec_limit.magic_num == 0x4C4C4C4C)
    {
        if(g_boot_arg->sec_limit.forbid_mode == F_FACTORY_MODE)
        {
            //Forbid to enter factory mode
            printf("%s Forbidden\n",MODULE_NAME);
            return FALSE;
        }
    }

    forbid_mode = g_boot_arg->boot_mode &= 0x000000FF;
    if(factory_check_key_trigger())
    {
    	return TRUE;
    }
	
	return FALSE;
}


