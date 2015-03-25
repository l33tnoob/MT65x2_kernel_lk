#ifndef __DISP_HAL_H__
#define __DISP_HAL_H__
#include "lcm_drv.h"
#include "fbconfig_kdebug.h"
// Forward declarations
struct resource;
struct mutex;
struct device;

typedef enum
{	
   DISP_STATUS_OK = 0,

   DISP_STATUS_NOT_IMPLEMENTED,
   DISP_STATUS_ALREADY_SET,
   DISP_STATUS_ERROR,
} DISP_STATUS;


typedef enum {
   DISP_STATE_IDLE = 0,
   DISP_STATE_BUSY,
} DISP_STATE;


#define MAKE_PANEL_COLOR_FORMAT(R, G, B) ((R << 16) | (G << 8) | B)
#define PANEL_COLOR_FORMAT_TO_BPP(x) ((x&0xff) + ((x>>8)&0xff) + ((x>>16)&0xff))

typedef enum {
    PANEL_COLOR_FORMAT_RGB332 = MAKE_PANEL_COLOR_FORMAT(3, 3, 2),
    PANEL_COLOR_FORMAT_RGB444 = MAKE_PANEL_COLOR_FORMAT(4, 4, 4),
    PANEL_COLOR_FORMAT_RGB565 = MAKE_PANEL_COLOR_FORMAT(5, 6, 5),
    PANEL_COLOR_FORMAT_RGB666 = MAKE_PANEL_COLOR_FORMAT(6, 6, 6),
    PANEL_COLOR_FORMAT_RGB888 = MAKE_PANEL_COLOR_FORMAT(8, 8, 8),
} PANEL_COLOR_FORMAT;

typedef struct
{
    DISP_STATUS (*init)(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited);
    DISP_STATUS (*enable_power)(BOOL enable);
    DISP_STATUS (*update_screen)(BOOL isMuextLocked);

    UINT32 (*get_working_buffer_size)(void);
    UINT32 (*get_working_buffer_bpp)(void);
    PANEL_COLOR_FORMAT (*get_panel_color_format)(void);
    void (*init_te_control)(void);
	UINT32 (*get_dithering_bpp)(void);

	DISP_STATUS (*capture_framebuffer)(unsigned int pvbuf, unsigned int bpp);

    void (*esd_reset)(void);
	BOOL (*esd_check)(void);
} DISP_IF_DRIVER;

typedef void (*DISPHAL_EVENT_HANDLER)(void *params);

int disphal_process_dbg_opt(const char *opt);
const DISP_IF_DRIVER* disphal_get_if_driver(void);
int disphal_init_ctrl_if(void);
int disphal_panel_enable(const LCM_DRIVER *lcm_drv, struct mutex* pLcmCmdMutex, BOOL enable);
int disphal_wait_not_busy(void);
int disphal_update_screen(const LCM_DRIVER *lcm_drv, struct mutex* pLcmCmdMutex, UINT32 x, UINT32 y, UINT32 width, UINT32 height);
int disphal_set_backlight(const LCM_DRIVER *lcm_drv, struct mutex* pLcmCmdMutex, UINT32 level);
int disphal_set_backlight_mode(const LCM_DRIVER *lcm_drv, struct mutex* pLcmCmdMutex, UINT32 mode);
int disphal_set_pwm(const LCM_DRIVER *lcm_drv, struct mutex* pLcmCmdMutex, UINT32 divider);
int disphal_get_pwm(const LCM_DRIVER *lcm_drv, struct mutex* pLcmCmdMutex, UINT32 divider, unsigned int *freq);
DISP_STATUS disphal_fm_desense_query(void);
DISP_STATUS disphal_fm_desense(unsigned long freq);
DISP_STATUS disphal_reset_update(void);
DISP_STATUS disphal_get_default_updatespeed(unsigned int *speed);
DISP_STATUS disphal_get_current_updatespeed(unsigned int *speed);
DISP_STATUS disphal_change_updatespeed(unsigned int speed);
int disphal_prepare_suspend(void);
const LCM_DRIVER *disphal_get_lcm_driver(const char *lcm_name, unsigned int *lcm_index);
int disphal_register_event(char* event_name, DISPHAL_EVENT_HANDLER event_handler);
int disphal_enable_te(BOOL enable);
int disphal_pm_restore_noirq(struct device *device);
int disphal_enable_mmu(BOOL enable);
int disphal_allocate_fb(struct resource* res, unsigned int* pa, unsigned int* va, unsigned int* dma_pa);
int disphal_map_overlay_out_buffer(unsigned int va, unsigned int size, unsigned int* dma_pa);
int disphal_unmap_overlay_out_buffer(unsigned int va, unsigned int size, unsigned int dma_pa);
int disphal_sync_overlay_out_buffer(unsigned int va, unsigned int size);
int disphal_dma_map_kernel(unsigned int dma_pa, unsigned int size, unsigned int* kva, unsigned int* mapsize);
int disphal_dma_unmap_kernel(unsigned int dma_pa, unsigned int size, unsigned int kva);
int disphal_init_overlay_to_memory(void);
int disphal_deinit_overlay_to_memory(void);
int disphal_get_fb_alignment(void);
unsigned int disphal_bls_query(void);
void disphal_bls_enable(bool enable);
unsigned int disphal_check_lcm(UINT32 color);
void fbconfig_disp_set_te_enable(char enable);
void fbconfig_disp_set_continuous_clock(int enable);
#endif
