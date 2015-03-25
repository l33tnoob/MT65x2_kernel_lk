/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


#include <dlfcn.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>

#include <utils/RefBase.h>
#include <utils/StrongPointer.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>


/* include linux framebuffer header */
#include <linux/fb.h>

/* include surface flinger header */
#include <private/gui/LayerState.h>

#include <binder/IPCThreadState.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <android/native_window.h>
#include <android/rect.h>

//#include <ui/Rect.h>
//#include <ui/Region.h>
#include <ui/DisplayInfo.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h> 



#include "charging_animation.h"
#include "show_logo_log.h"

using namespace android;

sp<SurfaceComposerClient> client;
sp<SurfaceControl> surfaceControl;

// data structure to access surface content
// AOSP API change: remove surfaceInfo
//Surface::SurfaceInfo info;

ANativeWindow_Buffer outBuffer;

sp<Surface>          surface;
DisplayInfo          dinfo;

// int dinfo_width,dinfo_height;
// int old_dinfo_Orientation;

static LCM_SCREEN_T phical_screen;

static int fb_fd = 0;

// logo.bin
static unsigned int *logo_addr = NULL;
// use for decompress logo resource
static void *dec_logo_addr = NULL;  
// use for nmap framebuffer 
static unsigned int *fb_addr = NULL;


// use double fb address
static unsigned int *front_fb_addr = NULL;
static unsigned int *back_fb_addr = NULL;
static unsigned int use_double_addr = 0;


static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;

static unsigned int fb_size = 0;

static int show_animationm_ver = 0;
static int draw_anim_mode = DRAW_ANIM_MODE_SURFACE;

// kernel logo index may be different for different logo.bin
static int kernel_logo_position = KERNEL_LOGO_INDEX ;

// add a flag for exiting on abnormal case
static int error_flag = 0;
/*
 * Set charging animation version
 *
 */
void set_anim_version(int version)
{
    LOG_ANIM("[charging_animation: %s %d]set animationm_version =  :%d, MAX_SUPPORT_ANIM_VER = %d",__FUNCTION__,__LINE__ ,version, MAX_SUPPORT_ANIM_VER);
    if ((version > (MAX_SUPPORT_ANIM_VER)) || (version < 0)) {
         show_animationm_ver = 0;
    } else {
         show_animationm_ver = version; 
    }
    LOG_ANIM("[charging_animation: %s %d]show_animationm_ver =  :%d",__FUNCTION__,__LINE__ ,show_animationm_ver);
}


/*
 * Set charging animation drawing method
 *
 */
void set_draw_mode(int draw_mode)
{
    LOG_ANIM("[charging_animation: %s %d]0 --use framebuffer, 1--use surface flinger ,draw_anim_mode =  :%d",__FUNCTION__,__LINE__ ,draw_mode);
    if ((draw_mode != (DRAW_ANIM_MODE_FB)) && (draw_mode != (DRAW_ANIM_MODE_SURFACE))) {
        draw_anim_mode = DRAW_ANIM_MODE_SURFACE;
    } else {
        draw_anim_mode = draw_mode;
    }       
}

/*
 * Charging animation init
 *
 */
void anim_init()
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    anim_logo_init();
    
    if (draw_anim_mode == (DRAW_ANIM_MODE_FB)) {
        anim_fb_init();
    } else {
        anim_surface_init();
    }
}


/*
 * Charging animation deinit
 *
 */
void anim_deinit()
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    anim_logo_deinit();    
    
    if (draw_anim_mode == (DRAW_ANIM_MODE_FB)) {
        anim_fb_deinit();
    } else {
        anim_surface_deinit();
    }
}


/*
 * Charging animation logo.bin related init
 *
 */
void anim_logo_init(void)
{
    // read and de-compress logo data here
    int fd = 0;
    int len = 0;
    int mtdid = 0;

    fd = open("/dev/logo", O_RDONLY);
    if(fd < 0)
    {
        LOG_ANIM("[charging_animation: %s %d]open logo partition device file fail, fd = %d \n",__FUNCTION__,__LINE__ , fd);
        error_flag = 1;
        return;
    }

    logo_addr = (unsigned int*)malloc(LOGO_BUFFER_SIZE);
    if(logo_addr == NULL)
    {
        LOG_ANIM("[charging_animation: %s %d]allocate logo buffer fail, size=0x%08x \n",__FUNCTION__,__LINE__ , LOGO_BUFFER_SIZE);
        goto error_return;
    }

    // (1) skip the image header
    len = read(fd, logo_addr, 512);
    if (len < 0)
    {
        LOG_ANIM("[charging_animation: %s %d]read from logo addr for 512B is failed! \n",__FUNCTION__,__LINE__);
        goto error_return;
    }
	// get the image 
    len = read(fd, logo_addr, LOGO_BUFFER_SIZE - 512);
    if (len < 0)
    {
        LOG_ANIM("[charging_animation: %s %d]read from logo addr for buffer is failed! \n",__FUNCTION__,__LINE__);
        goto error_return;
    }
    close(fd);

    if (show_animationm_ver > 0)
    {
        unsigned int *pinfo = (unsigned int*)logo_addr;         
        LOG_ANIM("[charging_animation: %s %d]pinfo[0]=0x%08x, pinfo[1]=0x%08x, pinfo[2]=%d\n", __FUNCTION__,__LINE__,
                    pinfo[0], pinfo[1], pinfo[2]);
                    
        if ((show_animationm_ver == WIRELESS_CHARGING_ANIM_VER) && (pinfo[0] < ANIM_V2_LOGO_NUM))
        {
            set_anim_version(1);
        }
        if (pinfo[0] < ANIM_V1_LOGO_NUM)
        {
            kernel_logo_position = ANIM_V0_LOGO_NUM - 1;
            set_anim_version(0);
        }
    }
    LOG_ANIM("[charging_animation: %s %d]show_animationm_ver =  :%d",__FUNCTION__,__LINE__ ,show_animationm_ver);    
    return;

error_return:
    close(fd);
    sleep(3);
    error_flag = 1;
    LOG_ANIM("[charging_animation: %s %d] error return !!!\n",__FUNCTION__,__LINE__); // to prevent interlace operation with MD reset
}

/*
 * Charging animation logo.bin related deinit
 *
 */
void anim_logo_deinit(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    free(logo_addr);
    logo_addr = NULL;
}


/*
 * Charging animation framebuffer related init
 *
 */
int anim_fb_init(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);

    fb_fd = open(FB_NODE_PATH, O_RDWR);
    if(fb_fd < 0)
    {
        LOG_ANIM("[charging_animation: %s %d]open dev file fail, fb_fd = %d \n",__FUNCTION__,__LINE__ , fb_fd);
        goto error;
    }

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);

    fb_size  = finfo.line_length * vinfo.yres;
    dec_logo_addr = malloc(fb_size);
    
    front_fb_addr =(unsigned int*)mmap(0, fb_size*2, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    back_fb_addr = (unsigned int*)((unsigned int)front_fb_addr + fb_size);
    fb_addr = front_fb_addr;

    LOG_ANIM("[charging_animation: %s %d]vinfo.xres  = %d, vinfo.yres = %d, vinfo.xres_virtual =%d, fb_size =%d, fb_addr = %d,back_fb_addr=%d\n"
            ,__FUNCTION__, __LINE__, vinfo.xres,vinfo.yres, vinfo.xres_virtual, fb_size,( int)fb_addr, (int)back_fb_addr);

    if(fb_addr == NULL || back_fb_addr == NULL)
    {
        LOG_ANIM("ChargingAnimation mmap fail\n");
        goto error;
    }

    phical_screen.bits_per_pixel = vinfo.bits_per_pixel;
    phical_screen.fill_dst_bits = vinfo.bits_per_pixel;
    
    phical_screen.width = vinfo.xres;
    phical_screen.height = vinfo.yres;
        
    // in JB2.MP need to allign width and height to 32 ,but jb5.mp needn't   
    phical_screen.needAllign = 1;
    phical_screen.allignWidth = vinfo.xres_virtual;
    
    // in GB3, no need to adjust 180 showing logo for fb driver does the change
    // but in JB5.MP, boot_logo_updater need adjust it for screen 180 roration
    phical_screen.need180Adjust = 1;
    phical_screen.fb_size = fb_size;
    
    LOG_ANIM("[charging_animation: %s %d]MTK_LCM_PHYSICAL_ROTATION = %s\n",__FUNCTION__,__LINE__, MTK_LCM_PHYSICAL_ROTATION);

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

    // test code
//    phical_screen.rotation = 0;      
//    phical_screen.needAllign = 0;
    
    LOG_ANIM("[show_logo_common]phical_screen: width= %d,height= %d,bits_per_pixel =%d,needAllign = %d,allignWidth=%d rotation =%d ,need180Adjust = %d\n",
            phical_screen.width, phical_screen.height,
            phical_screen.bits_per_pixel, phical_screen.needAllign,
            phical_screen.allignWidth, phical_screen.rotation, phical_screen.need180Adjust);  
    LOG_ANIM("[charging_animation: %s %d]show old animtion= 1, running show_animationm_ver %d\n",__FUNCTION__,__LINE__, show_animationm_ver);
    LOG_ANIM("[charging_animation: %s %d]draw_anim_mode = 1, running mode %d\n",__FUNCTION__,__LINE__, draw_anim_mode);
     
    return 0;
error:
    munmap(front_fb_addr, fb_size*2);
    close(fb_fd);
    error_flag = 1;
    
    return -1;
}

/*
 * Charging animation framebuffer related deinit
 *
 */
void anim_fb_deinit(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    free(dec_logo_addr);
    LOG_ANIM("[charging_animation: %s %d]free dec_logo_addr  \n",__FUNCTION__,__LINE__);
    munmap(front_fb_addr, fb_size*2);
    close(fb_fd);
}

/*
 * Charging animation framebuffer switch buffer
 *
 */
void anim_fb_addr_switch(void)
{
    LOG_ANIM("[charging_animation: %s %d]use_double_addr =%d \n",__FUNCTION__,__LINE__,use_double_addr);
    if(use_double_addr == 0) {
        use_double_addr++;
        fb_addr = front_fb_addr;
    } else {
        use_double_addr = 0;
        fb_addr = back_fb_addr;
    }
    LOG_ANIM("[charging_animation: %s %d]fb_addr =%d \n",__FUNCTION__,__LINE__, (int)fb_addr);
}

/*
 * Charging animation framebuffer update 
 *
 */
void anim_fb_disp_update(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    
    if (fb_addr == back_fb_addr) {
        vinfo.yoffset = vinfo.yres;
    } else {
        vinfo.yoffset = 0;
    }
    
    vinfo.activate |= (FB_ACTIVATE_FORCE | FB_ACTIVATE_NOW);
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
    {
        LOG_ANIM("ioctl FBIOPUT_VSCREENINFO flip failed\n");
    }
}



/*
 * Charging animation surface flinger related init
 *
 */
void anim_surface_init(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    client = new SurfaceComposerClient();  

    sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));  
    LOG_ANIM("[charging_animation: %s %d]ChargingAnimation getDisplayInfo()...\n",__FUNCTION__,__LINE__);
    status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
    if (status)
        LOG_ANIM("[charging_animation: %s %d]error=%x %d",__FUNCTION__,__LINE__,status,status);


    LOG_ANIM("[charging_animation: %s %d]dinfo.w=%d,dinfo.h=%d,dinfo.orientation=%d\n",__FUNCTION__,__LINE__,dinfo.w, dinfo.h, dinfo.orientation);
    LOG_ANIM("[charging_animation: %s %d]set default orientation\n",__FUNCTION__,__LINE__);

    SurfaceComposerClient::setDisplayProjection(dtoken, DisplayState::eOrientationDefault, Rect(dinfo.w, dinfo.h), Rect(dinfo.w, dinfo.h));          
 
    int dinfo_width = dinfo.w;
    int dinfo_height = dinfo.h;
  

    LOG_ANIM("[charging_animation: %s %d]dinfo_width=%d, dinfo_height=%d \n",__FUNCTION__,__LINE__,dinfo_width, dinfo_height);

    surfaceControl = client->createSurface(String8("charging-surface"), dinfo_width,  dinfo_height, PIXEL_FORMAT_RGB_565);
    LOG_ANIM("[charging_animation: %s %d]set layer geometry\n",__FUNCTION__,__LINE__);
    // set layer geometry
    SurfaceComposerClient::openGlobalTransaction();
    {  
        surfaceControl->setLayer(2000000);  
    }
    SurfaceComposerClient::closeGlobalTransaction();

    // data structure to access surface content 
    surface = surfaceControl->getSurface();
    
    phical_screen.width = dinfo.w;
    phical_screen.height = dinfo.h;
    // for we adjust the roration avove, so no need to consider rotation
    phical_screen.rotation = 0;   
    phical_screen.need180Adjust = 0;
    
    
    // use PIXEL_FORMAT_RGB_565 for surface flinger
    phical_screen.bits_per_pixel = 16;
    phical_screen.fill_dst_bits = 16;
    fb_size  = dinfo.w * dinfo.h* 4;
    
    dec_logo_addr = malloc(fb_size);
    LOG_ANIM("[charging_animation: %s %d]fb_size =%d\n" ,__FUNCTION__,__LINE__, fb_size);
    
    phical_screen.fb_size = fb_size;
    
    /* phical_screen: needAllign and  need180Adjust need confirm */
    phical_screen.needAllign = 1;
    phical_screen.allignWidth = ALIGN_TO(phical_screen.width, 32);
  

    
    LOG_ANIM("[show_logo_common] phical_screen: width= %d,height= %d,bits_per_pixel =%d,needAllign = %d,allignWidth=%d rotation =%d ,need180Adjust = %d\n",
        phical_screen.width, phical_screen.height,phical_screen.bits_per_pixel,phical_screen.needAllign,phical_screen.allignWidth,phical_screen.rotation,phical_screen.need180Adjust);  

}

/*
 * Charging animation surface flinger related deinit
 *
 */
void anim_surface_deinit(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);
    surfaceControl->clear();
    client->dispose();    
}


/*
 * Show special logo with the index in logo.bin
 *
 */ 
void anim_show_logo(int index)
{ 
     LOG_ANIM("[charging_animation: %s %d]draw_anim_mode=%d, show  index =  %d\n",__FUNCTION__,__LINE__,draw_anim_mode,index);
 
     if (draw_anim_mode == (DRAW_ANIM_MODE_FB)) {
        anim_fb_addr_switch();
        fill_animation_logo(index, fb_addr, dec_logo_addr, logo_addr,phical_screen);
        anim_fb_disp_update();
     } else {
        ARect tmpRect;
        tmpRect.left = 0;
        tmpRect.top = 0;
        tmpRect.right = phical_screen.width;
        tmpRect.bottom = phical_screen.height;
        
        status_t  lockResult = surface->lock(&outBuffer, &tmpRect); 
    
    
        LOG_ANIM("[charging_animation: %s %d]outBuffer.bits = %d\n",__FUNCTION__,__LINE__, (int)outBuffer.bits);
        LOG_ANIM("[charging_animation: %s %d]surface->lock return =  0x%08x,  %d\n",__FUNCTION__,__LINE__,lockResult,lockResult);
        if (0 == lockResult)
        {
            fill_animation_logo(index,outBuffer.bits, dec_logo_addr, logo_addr,phical_screen);
            surface->unlockAndPost();
        } 
     }     
 }
 
/*
 * Show first boot logo when phone boot up
 *
 */ 
void show_boot_logo(void)
{
    LOG_ANIM("[charging_animation: %s %d]show boot logo, index = 0 \n",__FUNCTION__,__LINE__);
    if (error_flag == 0) 
    {
        anim_show_logo(BOOT_LOGO_INDEX);
    }    
}

/*
 * Show kernel logo when phone boot up
 *
 */
void show_kernel_logo()
{
    LOG_ANIM("[charging_animation: %s %d]show kernel logo, index = 38 \n",__FUNCTION__,__LINE__);
    if (error_flag == 0) 
    {
        anim_show_logo(kernel_logo_position); 
    }    
}    

/*
 * Show low battery logo 
 *
 */
void show_low_battery(void){
    LOG_ANIM("[charging_animation: %s %d]show low battery logo, index = 2 \n",__FUNCTION__,__LINE__);
    if (error_flag == 0) 
    {
        anim_show_logo(LOW_BATTERY_INDEX);  
    }  
}


/*
 * Show charging over logo
 *
 */
void show_charger_ov_logo(void)
{
    LOG_ANIM("[charging_animation: %s %d]show charger_ov logo, index = 3 \n",__FUNCTION__,__LINE__);
    if (error_flag == 0) 
    {
        anim_show_logo(CHARGER_OV_INDEX);
    }
}


/*
 * Draw black screen
 *
 */
 void show_black_logo(void)
{
    LOG_ANIM("[charging_animation: %s %d]\n",__FUNCTION__,__LINE__);   
       
    if (draw_anim_mode == (DRAW_ANIM_MODE_FB)) {        
        anim_fb_addr_switch();
        memset((unsigned short *)fb_addr, 0x00, fb_size);
        anim_fb_disp_update();
    } else {
        ARect tmpRect;
        tmpRect.left = 0;
        tmpRect.top = 0;
        tmpRect.right = phical_screen.width;
        tmpRect.bottom = phical_screen.height;
        
        status_t  lockResult = surface->lock(&outBuffer, &tmpRect);         
        LOG_ANIM("[charging_animation: %s %d]outBuffer.bits = %d, surface->lock return =  0x%08x,\n",__FUNCTION__,__LINE__, (int)outBuffer.bits,lockResult);
        
        if (0 == lockResult)
        {
            ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
            LOG_ANIM("[charging_animation: %s %d]bpr = %d\n",__FUNCTION__,__LINE__, (int)bpr);
            
            memset((unsigned short *)outBuffer.bits, 0x00, bpr*outBuffer.height);
            surface->unlockAndPost();
        }
    }
}
 
/*
 * Show charging animation with battery capacity
 *
 */
void show_battery_capacity(unsigned int capacity)
{ 
    LOG_ANIM("[charging_animation: %s %d]capacity =%d\n",__FUNCTION__,__LINE__, capacity);
    
    if (draw_anim_mode == (DRAW_ANIM_MODE_FB)) {
        anim_fb_addr_switch();        
        fill_animation_battery_by_ver(capacity, (void *)fb_addr, dec_logo_addr, logo_addr, phical_screen, show_animationm_ver);           
        anim_fb_disp_update();
    } else {
        ARect tmpRect;
        tmpRect.left = 0;
        tmpRect.top = 0;
        tmpRect.right = phical_screen.width;
        tmpRect.bottom = phical_screen.height;
        
        status_t  lockResult = surface->lock(&outBuffer, &tmpRect); 
        LOG_ANIM("[charging_animation: %s %d]outBuffer.bits = %d, surface->lock return =  0x%08x,\n",__FUNCTION__,__LINE__, (int)outBuffer.bits,lockResult);
        
        if (0 == lockResult)
        {
            fill_animation_battery_by_ver(capacity, (void *)outBuffer.bits, dec_logo_addr, logo_addr, phical_screen, show_animationm_ver);        
            surface->unlockAndPost();
        }
    }
}