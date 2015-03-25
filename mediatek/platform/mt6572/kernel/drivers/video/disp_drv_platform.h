#ifndef __DISP_DRV_PLATFORM_H__
#define __DISP_DRV_PLATFORM_H__

#include <linux/dma-mapping.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/m4u.h>
//#include <mach/mt6585_pwm.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
//#include <mach/boot.h>
#include <board-custom.h>
#include <linux/disp_assert_layer.h>
#include "ddp_reg.h"
#include "ddp_path.h"

///LCD HW feature options for MT6575
#define MTK_LCD_HW_SIF_VERSION      2       ///for MT6575, we naming it is V2 because MT6516/73 is V1...
//#define MTK_LCD_HW_3D_SUPPORT
#define MTKFB_NO_M4U
#define MT65XX_NEW_DISP

#define ALIGN_TO(x, n)  \
           (((x) + ((n) - 1)) & ~((n) - 1))
#define MTK_FB_ALIGNMENT 32
#define MTK_FB_SYNC_SUPPORT
#if !defined(MTK_LCA_RAM_OPTIMIZE)
    #define MTK_OVL_DECOUPLE_SUPPORT
#endif

#endif //__DISP_DRV_PLATFORM_H__
