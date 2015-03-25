/*
 * (C) Copyright 2008
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


/* HEADER FILES   */

#include <debug.h>

#include <platform/mt_typedefs.h>
#include <platform/mt_disp_drv.h>
#include <platform/disp_drv.h>
#include <platform/lcd_drv.h>
#include <platform/mt_logo.h>
#include <platform/disp_drv_platform.h>

#include <target/board.h>
#include "lcm_drv.h"

#include <string.h>

/* show logo header file */
#include <show_logo_common.h>
#include <decompress_common.h>
#include <show_animation_common.h>


LCM_SCREEN_T phical_screen;
void  *logo_addr = NULL;

static LOGO_CUST_IF *logo_cust_if = NULL;

/********** show_animationm_ver:  charging animation version  ************/
/*                                                                       */ 
/* version 0: show 4 recatangle growing animation without battery number */
/* version 1: show wave animation with  battery number                   */
/*                                                                       */ 
/***                                                                   ***/

int show_animationm_ver = 0;


/*
 * Get the defined charging animation version 
 *
 */
void sync_anim_version()
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);
    
#ifdef ANIMATION_NEW
    show_animationm_ver = 1 ;     
#else
    show_animationm_ver = 0 ;
    dprintf(INFO, "[lk logo %s %d]not define ANIMATION_NEW:show old animation \n",__FUNCTION__,__LINE__); 
#endif

}

/*
 * Initliaze charging animation parameters 
 *
 */
void init_fb_screen()
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);
    unsigned int fb_size = mt_get_fb_size();
    logo_addr = mt_get_logo_db_addr();

    phical_screen.width = CFG_DISPLAY_WIDTH;
    phical_screen.height = CFG_DISPLAY_HEIGHT;
    phical_screen.fb_size = fb_size;   
    phical_screen.fill_dst_bits = MTK_FB_ALIGNMENT;   
        
    // in JB2.MP need to allign width and height to 32 ,but jb5.mp needn't   
    phical_screen.needAllign = 1;
    phical_screen.allignWidth = ALIGN_TO(CFG_DISPLAY_WIDTH, MTK_FB_ALIGNMENT);
    
    /* In GB, no need to adjust 180 showing logo ,for fb driver dealing the change */
    /* but in JB, need adjust it for screen 180 roration           */
    phical_screen.need180Adjust = 0;   // need sync with chip driver 
    
    dprintf(INFO, "[lk logo: %s %d]MTK_LCM_PHYSICAL_ROTATION = %s\n",__FUNCTION__,__LINE__, MTK_LCM_PHYSICAL_ROTATION);

    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
    { 
        phical_screen.rotation = 270;
    } else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2)){ 
        phical_screen.rotation = 90;
    } else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3) && (phical_screen.need180Adjust == 1)){ 
        phical_screen.rotation = 180;   
    } else {
        phical_screen.rotation = 0;   
    }
    
    sync_anim_version();
    if (show_animationm_ver == 1)
    {
        unsigned int logonum;
        unsigned int *db_addr = logo_addr;
    
        unsigned int *pinfo = (unsigned int*)db_addr;
        
        logonum = pinfo[0];
        dprintf(INFO, "[lk logo: %s %d]pinfo[0]=0x%08x, pinfo[1]=0x%08x, pinfo[2]=%d\n", __FUNCTION__,__LINE__,
                    pinfo[0], pinfo[1], pinfo[2]);
    
        dprintf(INFO, "[lk logo: %s %d]define ANIMATION_NEW:show new animation with capacity num\n",__FUNCTION__,__LINE__); 
        dprintf(INFO, "[lk logo: %s %d]CAPACITY_LEFT =%d, CAPACITY_TOP =%d \n",__FUNCTION__,__LINE__,(CAPACITY_LEFT) ,(CAPACITY_TOP) ); 
        dprintf(INFO, "[lk logo: %s %d]LCM_HEIGHT=%d, LCM_WIDTH=%d \n",__FUNCTION__,__LINE__,(CAPACITY_RIGHT),(CAPACITY_BOTTOM)); 
        if(logonum < 6)
        {
            show_animationm_ver = 0 ;
        } else {
            show_animationm_ver = 1 ;   
        }
    }
    
}


/*
 * Custom interface 
 *
 */
void mt_logo_get_custom_if(void)
{
    if(logo_cust_if == NULL)
    {
        logo_cust_if = (LOGO_CUST_IF *)LOGO_GetCustomIF();
    }
}


/*
 * Show first boot logo when phone boot up
 *
 */
void mt_disp_show_boot_logo(void)
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);    
    mt_logo_get_custom_if();

    if(logo_cust_if->show_boot_logo)
    {
        logo_cust_if->show_boot_logo();
    }
    else
    {
        ///show_logo(0);
        init_fb_screen();
        fill_animation_logo(BOOT_LOGO_INDEX, mt_get_fb_addr(), mt_get_tempfb_addr(), logo_addr, phical_screen);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

    return;
}

/*
 * Custom interface : charging state
 *
 */
void mt_disp_enter_charging_state(void)
{
    mt_logo_get_custom_if();

    if(logo_cust_if->enter_charging_state)
    {
        logo_cust_if->enter_charging_state();
    }
    else
    {
        dprintf(INFO, "[lk logo: %s %d]do nothing \n",__FUNCTION__,__LINE__);
    }

    return;
}


/*
 * Show full battery when poweroff charging
 *
 */
void mt_disp_show_battery_full(void)
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);
    mt_disp_show_battery_capacity(100);
}


/*
 * Show animation when poweroff charging
 *
 */
void mt_disp_show_battery_capacity(UINT32 capacity)
{
        dprintf(INFO, "[lk logo: %s %d]capacity =%d\n",__FUNCTION__,__LINE__, capacity);
    mt_logo_get_custom_if();

    if(logo_cust_if->show_battery_capacity)
    {
        logo_cust_if->show_battery_capacity(capacity);
    }
    else
    {     
        init_fb_screen();
        
        fill_animation_battery_by_ver(capacity, mt_get_fb_addr(), mt_get_tempfb_addr(), logo_addr, phical_screen, show_animationm_ver);            
                  
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

}

/*
 * Show charging over logo
 *
 */
void mt_disp_show_charger_ov_logo(void)
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);
    mt_logo_get_custom_if();

    if(logo_cust_if->show_boot_logo)
    {
        logo_cust_if->show_boot_logo();
    }
    else
    {
        init_fb_screen();
        fill_animation_logo(CHARGER_OV_INDEX, mt_get_fb_addr(), mt_get_tempfb_addr(), logo_addr, phical_screen);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

    return;
}

/*
 * Show low battery logo 
 *
 */
void mt_disp_show_low_battery(void)
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);
    mt_logo_get_custom_if();

    if(logo_cust_if->show_boot_logo)
    {
        logo_cust_if->show_boot_logo();
    }
    else
    {
        init_fb_screen();
        //show_logo(2);
        fill_animation_logo(LOW_BATTERY_INDEX, mt_get_fb_addr(), mt_get_tempfb_addr(), logo_addr, phical_screen);
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    }

    return;
}


/*
 * Fill rectangle region for with black  or other color  
 *
 */
void mt_disp_fill_rect(UINT32 left, UINT32 top,
                           UINT32 right, UINT32 bottom,
                           UINT32 color)
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);   
    init_fb_screen();
    RECT_REGION_T rect = {left, top, right, bottom};
    
    fill_rect_with_color(mt_get_fb_addr(), rect, color, phical_screen);     
}

