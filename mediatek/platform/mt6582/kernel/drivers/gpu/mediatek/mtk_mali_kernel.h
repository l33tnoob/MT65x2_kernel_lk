#ifndef __MTK_MALI_KERNEL_H__
#define __MTK_MALI_KERNEL_H__

#include "mt_reg_base.h"
#include <asm/io.h> /*For ioread/iowrite*/



/*-----------------------------------------------------------------------------
    MTK Local Config Flag
  -----------------------------------------------------------------------------*/
//#define MTKMALI_SWITCH_CG_AT_CORE_START_STOP
//#define MTKMALI_SWITCH_CG_AT_SUSPEND_RESUME

  



/*-----------------------------------------------------------------------------
    Register Map
  -----------------------------------------------------------------------------*/

#define REG_MFG_CG_CON      ( G3D_CONFIG_BASE + 0x0 )
#define REG_MFG_RESET       ( G3D_CONFIG_BASE + 0xC )
#define REG_MFG_DEBUG_SEL   ( G3D_CONFIG_BASE + 0xF0 )

#define REG_SMI_CG_TEMP     ( 0xF4000000 + 0x100 )

/*-----------------------------------------------------------------------------
    Print Macro
  -----------------------------------------------------------------------------*/
#define MALIK_PRINTF             printk
#define MALIK_MSG(fmt, arg...)   { MALIK_PRINTF("Mali: %s(): "fmt,__FUNCTION__, ##arg); }
#define MALIK_ERRMSG(fmt, arg...)   { MALIK_PRINTF("Mali Error: %s()@%s:line %d: "fmt,__FUNCTION__, __FILE__,__LINE__, ##arg);  MTKMALI_DumpRegister(); }
#define MALIK_WARNMSG(fmt, arg...)  { MALIK_PRINTF("Mali Warn: %s(): "fmt,__FUNCTION__, ##arg); }



/*-----------------------------------------------------------------------------
    Macro
  -----------------------------------------------------------------------------*/
#define M_READ32( addr, offset )          ioread32( ((unsigned char*)addr) + offset )
#define M_WRITE32( addr, offset, val )    iowrite32( val, ((unsigned char*)addr) + offset )


/*-----------------------------------------------------------------------------
    Function
  -----------------------------------------------------------------------------*/
void MTKMALI_DumpRegister( void );

#endif /*__MTK_MALI_H__*/

