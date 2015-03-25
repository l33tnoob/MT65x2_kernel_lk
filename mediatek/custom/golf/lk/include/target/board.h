#ifndef __BOARD_H
#define __BOARD_H

#define CONFIG_CFB_CONSOLE 
#define CFB_CONSOLE_ON

#define CONFIG_SYS_PROMPT               "LK> "
#define CONFIG_SYS_CBSIZE               256  		 /* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE               (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

//Project specific header file
#define CFG_DISPLAY_WIDTH       (DISP_GetScreenWidth())
#define CFG_DISPLAY_HEIGHT      (DISP_GetScreenHeight())
#define CFG_DISPLAY_BPP         (16)

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#define CFG_POWER_CHARGING
#endif

#endif
