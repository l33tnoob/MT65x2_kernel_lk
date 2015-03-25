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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "show_animation_common.h"
#include "show_logo_log.h"
  
static int charging_low_index = 0;
static int charging_animation_index = 0;


#define CHECK_LOGO_BIN_OK  0
#define CHECK_LOGO_BIN_ERROR  -1


static unsigned short  number_pic_addr[(NUMBER_RIGHT - NUMBER_LEFT)*(NUMBER_BOTTOM - NUMBER_TOP)*2] = {0x0}; //addr
static unsigned short  line_pic_addr[(TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*2] = {0x0};
static unsigned short  percent_pic_addr[(PERCENT_RIGHT - PERCENT_LEFT)*(PERCENT_BOTTOM - PERCENT_TOP)*2] = {0x0};
static unsigned short  top_animation_addr[(TOP_ANIMATION_RIGHT - TOP_ANIMATION_LEFT)*(TOP_ANIMATION_BOTTOM - TOP_ANIMATION_TOP)*2] = {0x0};



/*
 * Check logo.bin address if valid, and get logo related info
 *
 */ 
int check_logo_index_valid(unsigned int index, void * logo_addr, LOGO_PARA_T * logo_info)
{  
    unsigned int *pinfo = (unsigned int*)logo_addr;
    logo_info->logonum = pinfo[0];
    
    LOG_ANIM("[show_animation_common: %s %d]logonum =%d, index =%d\n", __FUNCTION__,__LINE__,logo_info->logonum, index);
    if (index >= logo_info->logonum)
    {
        LOG_ANIM("[show_animation_common: %s %d]unsupported logo, index =%d\n", __FUNCTION__,__LINE__, index);
        return CHECK_LOGO_BIN_ERROR;
    
    }   
   // LOG_ANIM("show_animation_common, pinfo[1]=%d,  pinfo[2+index]=%d, pinfo[3+index]=%d, pinfo[3+index] - pinfo[2+index]= %d, pinfo[1] - pinfo[2+index] =%d \n", 
   //            pinfo[1],pinfo[2+index],pinfo[3+index], pinfo[3+index] - pinfo[2+index],pinfo[1] - pinfo[2+index]); 
    
    if(index < logo_info->logonum - 1)
		logo_info->logolen = pinfo[3+index] - pinfo[2+index];
	else
		logo_info->logolen = pinfo[1] - pinfo[2+index];

    logo_info->inaddr = (unsigned int)logo_addr + pinfo[2+index];
    LOG_ANIM("show_animation_common, in_addr=0x%08x,  logolen=%d\n", 
                logo_info->inaddr,  logo_info->logolen);

    return CHECK_LOGO_BIN_OK;                
}


/*
 * Fill a screen size buffer with logo content 
 *
 */ 
void fill_animation_logo(unsigned int index, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOGO_PARA_T logo_info;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return; 
              
    decompress_logo((void*)logo_info.inaddr, dec_logo_addr, logo_info.logolen, phical_screen.fb_size);
    RECT_REGION_T rect = {0, 0, phical_screen.width, phical_screen.height};   
    
    fill_rect_with_content(fill_addr, rect, (unsigned short *)dec_logo_addr, phical_screen);

    
}

/*
 * Fill a rectangle size address with special color 
 *
 */ 
void fill_animation_prog_bar(RECT_REGION_T rect_bar,
                       unsigned int fgColor, 
                       unsigned int start_div, unsigned int occupied_div,
                       void *fill_addr, LCM_SCREEN_T phical_screen)
{
    unsigned int div_size  = (rect_bar.bottom - rect_bar.top) / (ANIM_V0_REGIONS);
    unsigned int draw_size = div_size - (ANIM_V0_SPACE_AFTER_REGION);
    
    unsigned int i;
    
    for (i = start_div; i < start_div + occupied_div; ++ i)
    {
        unsigned int draw_bottom = rect_bar.bottom - div_size * i - (ANIM_V0_SPACE_AFTER_REGION);
        unsigned int draw_top    = draw_bottom - draw_size;
        
        RECT_REGION_T rect = {rect_bar.left, draw_top, rect_bar.right, draw_bottom};

        fill_rect_with_color(fill_addr, rect, fgColor, phical_screen);

    }
}


/*
 * Fill a rectangle with logo content 
 *
 */ 
void fill_animation_dynamic(unsigned int index, RECT_REGION_T rect, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOGO_PARA_T logo_info;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return; 
                    
    decompress_logo((void*)logo_info.inaddr, (void*)dec_logo_addr, logo_info.logolen, (rect.right-rect.left)*(rect.bottom-rect.top)*2);
    fill_rect_with_content(fill_addr, rect, (unsigned short *)dec_logo_addr, phical_screen);            
}


/*
 * Fill a rectangle  with number logo content 
 *
 * number_position: 0~1st number, 1~2nd number 
 */
void fill_animation_number(unsigned int index, unsigned int number_position, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOG_ANIM("[show_animation_common: %s %d]index= %d, number_position = %d\n",__FUNCTION__,__LINE__, index, number_position);

    LOGO_PARA_T logo_info;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return;                 

    // draw default number rect,
    decompress_logo((void*)logo_info.inaddr, (void*)number_pic_addr, logo_info.logolen, number_pic_size);

    //static RECT_REGION_T number_location_rect = {NUMBER_LEFT,NUMBER_TOP,NUMBER_RIGHT,NUMBER_BOTTOM};    
    RECT_REGION_T battery_number_rect = {NUMBER_LEFT + (NUMBER_RIGHT - NUMBER_LEFT)*number_position,
                            NUMBER_TOP,
                            NUMBER_RIGHT + (NUMBER_RIGHT - NUMBER_LEFT)*number_position,
                            NUMBER_BOTTOM};   
                                                     
    fill_rect_with_content(fill_addr, battery_number_rect, (unsigned short *)number_pic_addr,phical_screen);            
}

/*
 * Fill a line with special color 
 *
 */ 
void fill_animation_line(unsigned int index, unsigned int capacity_grids, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen)
{    
    LOGO_PARA_T logo_info;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return; 
                
    decompress_logo((void*)logo_info.inaddr, (void*)line_pic_addr, logo_info.logolen, line_pic_size);

    RECT_REGION_T rect = {CAPACITY_LEFT, CAPACITY_TOP, CAPACITY_RIGHT, CAPACITY_BOTTOM}; 
    int i = capacity_grids;
    for(; i < CAPACITY_BOTTOM; i++)
    {      
        rect.top = i;
        rect.bottom = i+1;                                             
        fill_rect_with_content(fill_addr, rect, (unsigned short *)line_pic_addr, phical_screen);

    }
}



/*
 * Show charging animation version 0
 *
 */
void fill_animation_battery_ver_0(unsigned int capacity,  void *fill_addr, void * dec_logo_addr, void * logo_addr,
                       LCM_SCREEN_T phical_screen)
{

    unsigned int capacity_grids = 0;

    if (capacity > 100) capacity = 100;

    capacity_grids = (capacity * (ANIM_V0_REGIONS)) / 100;
    
    LOG_ANIM("[show_animation_common: %s %d]capacity =%d, capacity_grids = %d\n",__FUNCTION__,__LINE__, capacity, capacity_grids);
    

    //show_logo(1);
    fill_animation_logo(ANIM_V0_BACKGROUND_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
    // Fill Occupied Color

    //RECT_REGION_T bar_rect = {BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM};    
    RECT_REGION_T rect_bar = {bar_rect.left + 1, bar_rect.top + 1, bar_rect.right, bar_rect.bottom};
    
//    RECT_REGION_T rect_bar = {BAR_LEFT + 1, BAR_TOP + 1,BAR_RIGHT, BAR_BOTTOM};
    
    fill_animation_prog_bar(rect_bar,
                       (unsigned int)(BAR_OCCUPIED_COLOR), 
                       0,  capacity_grids,
                       fill_addr, phical_screen);                              

    fill_animation_prog_bar(rect_bar,
                      (unsigned int)(BAR_EMPTY_COLOR),
                      capacity_grids, ANIM_V0_REGIONS - capacity_grids,
                      fill_addr, phical_screen); 

}

/*
 * Show charging animation version 1
 *
 */
void fill_animation_battery_ver_1(unsigned int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOG_ANIM("[show_animation_common: %s %d]capacity : %d\n",__FUNCTION__,__LINE__, capacity);
    
    if (capacity >= 100) {
        //show_logo(37); // battery 100
        fill_animation_logo(FULL_BATTERY_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
     
    } else if (capacity < 10) {
        LOG_ANIM("[show_animation_common: %s %d]charging_low_index = %d\n",__FUNCTION__,__LINE__, charging_low_index);  
        charging_low_index ++ ;
                
        fill_animation_logo(LOW_BAT_ANIM_START_0 + charging_low_index, fill_addr, dec_logo_addr, logo_addr,phical_screen);
        fill_animation_number(NUMBER_PIC_START_0 + capacity, 1, fill_addr, logo_addr, phical_screen);
        fill_animation_dynamic(NUMBER_PIC_PERCENT, percent_location_rect, fill_addr, percent_pic_addr, logo_addr, phical_screen);
        
        if (charging_low_index >= 9) charging_low_index = 0;

    } else {

        unsigned int capacity_grids = 0;
        //static RECT_REGION_T battery_rect = {CAPACITY_LEFT,CAPACITY_TOP,CAPACITY_RIGHT,CAPACITY_BOTTOM};
        capacity_grids = CAPACITY_BOTTOM - (CAPACITY_BOTTOM - CAPACITY_TOP) * (capacity - 10) / 90;
        LOG_ANIM("[show_animation_common: %s %d]capacity_grids : %d,charging_animation_index = %d\n",__FUNCTION__,__LINE__, capacity_grids,charging_animation_index);   

        //background 
        fill_animation_logo(ANIM_V1_BACKGROUND_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
        
        fill_animation_line(ANIM_LINE_INDEX, capacity_grids, fill_addr,  logo_addr, phical_screen);
        fill_animation_number(NUMBER_PIC_START_0 + (capacity/10), 0, fill_addr, logo_addr, phical_screen);
        fill_animation_number(NUMBER_PIC_START_0 + (capacity%10), 1, fill_addr, logo_addr, phical_screen);
        fill_animation_dynamic(NUMBER_PIC_PERCENT, percent_location_rect, fill_addr, percent_pic_addr, logo_addr, phical_screen);                
        
        
         if (capacity <= 90)
         {
            RECT_REGION_T top_animation_rect = {TOP_ANIMATION_LEFT, capacity_grids - (TOP_ANIMATION_BOTTOM - TOP_ANIMATION_TOP), TOP_ANIMATION_RIGHT, capacity_grids};
            //top_animation_rect.bottom = capacity_grids;
            //top_animation_rect.top = capacity_grids - top_animation_height;
            charging_animation_index++;        
            //show_animation_dynamic(15 + charging_animation_index, top_animation_rect, top_animation_addr);
            fill_animation_dynamic(BAT_ANIM_START_0 + charging_animation_index, top_animation_rect, fill_addr, top_animation_addr, logo_addr, phical_screen);  
            
            if (charging_animation_index >= 9) charging_animation_index = 0;
         }
    }

}

/*
 * Show charging animation version 2:wireless ui
 * total 29 logo:from 39 ~ 68 
 * less(0<10): 50-53 , low(<30):54-57 ,middle(<60):58-61 , high():62-75 , o:66, full:67,num (0-9):39-48, %:49
 *
 */

 void fill_animation_battery_ver_2(unsigned int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOG_ANIM("[show_animation_common: %s %d]capacity : %d\n",__FUNCTION__,__LINE__, capacity);
//    RECT_REGION_T wireless_bgd_rect = {0, 0, phical_screen.width, phical_screen.height};
    
    charging_low_index >= 3? charging_low_index = 0:charging_low_index++;
    LOG_ANIM("[show_animation_common: %s %d]charging_low_index = %d\n",__FUNCTION__,__LINE__, charging_low_index); 
    
    if (capacity >= 100) {
         // battery 100
        fill_animation_logo(V2_BAT_100_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
    } else if (capacity <= 0) {
        fill_animation_logo(V2_BAT_0_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
    } else {
        int bg_index = V2_BAT_0_10_START_INDEX; //capacity > 0 && capacity < 10
        if (capacity >= 10 && capacity < 40) {
            bg_index = V2_BAT_10_40_START_INDEX;               
        } else if (capacity >= 40 && capacity < 80) {            
            bg_index = V2_BAT_40_80_START_INDEX;
        } else if (capacity >= 80 && capacity < 100) {      
            bg_index = V2_BAT_80_100_START_NDEX;
        }        
        fill_animation_logo(bg_index + charging_low_index, fill_addr, dec_logo_addr, logo_addr,phical_screen);    
        RECT_REGION_T tmp_rect = {(int)phical_screen.width * 4/10, 
                        (int) phical_screen.height * 1/6,
                        (int)phical_screen.width* 5/10,
                        (int)phical_screen.height*16/60};  
        unsigned short tmp_num_addr[(int)phical_screen.width * phical_screen.height/100]; //addr
                                     
        if (capacity >= 10) {
            LOG_ANIM("[show_animation_common: %s %d]tmp_rect left = %d, right = %d,top = %d,bottom = %d,\n",__FUNCTION__,__LINE__, 
                        tmp_rect.left,tmp_rect.right,tmp_rect.top,tmp_rect.bottom);                         
            fill_animation_dynamic(V2_NUM_START_0_INDEX + (capacity/10), tmp_rect, fill_addr, tmp_num_addr, logo_addr, phical_screen);
            tmp_rect.left += (int)phical_screen.width /10;
            tmp_rect.right += (int)phical_screen.width /10;
        }
        
        LOG_ANIM("[show_animation_common: %s %d]tmp_rect left = %d, right = %d,top = %d,bottom = %d,\n",__FUNCTION__,__LINE__, 
                tmp_rect.left,tmp_rect.right,tmp_rect.top,tmp_rect.bottom);                  
        fill_animation_dynamic(V2_NUM_START_0_INDEX + (capacity%10), tmp_rect, fill_addr, tmp_num_addr, logo_addr, phical_screen);                   
        
        tmp_rect.left += (int)phical_screen.width /10;
        tmp_rect.right += (int)phical_screen.width /10;

        LOG_ANIM("[show_animation_common: %s %d]tmp_rect left = %d, right = %d,top = %d,bottom = %d,\n",__FUNCTION__,__LINE__, 
                        tmp_rect.left,tmp_rect.right,tmp_rect.top,tmp_rect.bottom);                                         
        fill_animation_dynamic(V2_NUM_PERCENT_INDEX, tmp_rect, fill_addr, tmp_num_addr, logo_addr, phical_screen);  
        
    }
}

/*
 * Show charging animation by version
 *
 */
void fill_animation_battery_by_ver(unsigned int capacity,void *fill_addr, void * dec_logo_addr, void * logo_addr,
                        LCM_SCREEN_T phical_screen, int version)
{
    LOG_ANIM("[show_animation_common: %s %d]version : %d\n",__FUNCTION__,__LINE__, version);
    switch (version)
    {
        case 0:
            fill_animation_battery_ver_0(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;
        case 1:
            fill_animation_battery_ver_1(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;
        case 2:
            fill_animation_battery_ver_2(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;            
        default:
            fill_animation_battery_ver_0(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;   
    }                     
}                       
