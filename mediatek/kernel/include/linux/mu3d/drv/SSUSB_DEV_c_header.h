#ifndef __SSUSB_DEV_REGS_H__
#define __SSUSB_DEV_REGS_H__


#include <linux/mu3d/hal/mu3d_hal_hw.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifndef REG_BASE_C_MODULE
// ----------------- SSUSB_DEV Bit Field Definitions -------------------

#define PACKING
typedef unsigned int FIELD;
typedef u32 UINT32;

/*TODO: Can i remove these 2 define???*/
#define REPRO_TD925_ISSUE 0
#define USBHSET 0



#if REPRO_TD925_ISSUE
extern u32 exit_u12;
#define U12_DELAY 100
#endif


#define U3D_FIFO_START_ADDRESS 0



#endif //#ifndef REG_BASE_C_MODULE

#ifdef __cplusplus
}
#endif

#endif // __SSUSB_DEV_REGS_H__
