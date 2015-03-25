/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "show_logo_log.h"
#include "show_logo_common.h"

/*
#ifdef  BUILD_LK

#ifndef LOG_ANIM
#define  LOG_ANIM     printf
#endif 

#else 

#include <cutils/xlog.h>

#ifndef  LOG_ANIM
#define  LOG_ANIM     XLOGD
#endif 

#endif 

*/


#define CHECK_RECT_OK  0
#define CHECK_RECT_SIZE_ERROR  -1


/*
 * Fill one point address with  source color and color pixel bits
 *
 * @parameter
 *      
 */
void fill_point_buffer(unsigned int *fill_addr, unsigned int src_color, unsigned int bits_per_pixel)
{    
    if (32 == bits_per_pixel) {
        *fill_addr = RGB565_TO_ARGB8888(src_color);
    } else if (16 == bits_per_pixel) {
        *fill_addr = src_color;
    } else {
        LOG_ANIM("[show_logo_common %s %d]not support bits_per_pixel = %d \n",__FUNCTION__,__LINE__,(int)bits_per_pixel);   
    }       
       
}



/*
 * Check the rectangle if valid
 *
 * 
 *      
 */
int check_rect_valid(RECT_REGION_T rect)
{

    int draw_width = rect.right - rect.left;
    int draw_height = rect.bottom - rect.top;

 
    if (draw_width < 0 || draw_height < 0)
    {
       LOG_ANIM("[show_logo_common %s %d]drawing width or height is error\n",__FUNCTION__,__LINE__); 
       LOG_ANIM("[show_logo_common %s %d]rect:left= %d ,right= %d,top= %d,bottom= %d\n",__FUNCTION__,__LINE__,rect.left,rect.right,rect.top,rect.bottom); 
                
       return CHECK_RECT_SIZE_ERROR;
    }        
    
    return CHECK_RECT_OK;
}   

/*
 * Draw a rectangle region (32 bits perpixel) with logo content 
 *
 * 
 *
 */
void fill_rect_with_content_by_32bit(unsigned int *fill_addr, RECT_REGION_T rect, unsigned short *src_addr, LCM_SCREEN_T phical_screen)
{
    int draw_height = rect.bottom - rect.top;
    
    int virtual_width = phical_screen.needAllign == 0? phical_screen.width:phical_screen.allignWidth;
    int virtual_height = phical_screen.height;     
    
    int i = 0;
    int j = 0; 
    
    unsigned int * dst_addr = fill_addr;
    unsigned int * color_addr = (unsigned int *)src_addr;   
    
    for(i = rect.top; i < rect.bottom; i++)
    {   
        for(j = rect.left; j < rect.right; j++) 
        {
            switch (phical_screen.rotation)
            {
                case 90:
                    color_addr = (unsigned int *) (src_addr + draw_height*j + i);
                    dst_addr = fill_addr + ((virtual_width * i + virtual_width - j));
                    break;                    
                case 270:
                    color_addr = (unsigned int *) (src_addr + draw_height*j + i);
                    dst_addr = fill_addr + ((virtual_width * (virtual_height - i - 1)+ j));
                    break;
                case 180:
                    // adjust fill in address
                    color_addr = (unsigned int *)src_addr++;
                    dst_addr = fill_addr + virtual_width * (virtual_height - i)- j-1;
                    break;                    
                default:
                    color_addr = (unsigned int *)src_addr++;
                    dst_addr = fill_addr + virtual_width * i + j;

            }
              
            //if(((i>rect.bottom-2) || ( i%400==0)) && ((j>rect.right-2) || (j%400 == 0)))
            //{
            //   LOG_ANIM("[show_logo_common]dst_addr = %d,color_addr= %d,i= %d,j=%d \n",(int)dst_addr, (int)color_addr, i , j);      
            //}            
            //fill_point_buffer(dst_addr, 0xf800, phical_screen.bits_per_pixel);      // fill in red color for test
            fill_point_buffer(dst_addr, *color_addr, phical_screen.bits_per_pixel);           
        }
    }
}





/*
 * Draw a rectangle region (16 bits per pixel) with logo content 
 *
 * 
 *
 */
void fill_rect_with_content_by_16bit(unsigned short *fill_addr, RECT_REGION_T rect, unsigned short *src_addr, LCM_SCREEN_T phical_screen)
{
    int draw_height = rect.bottom - rect.top;
    
    int virtual_width = phical_screen.needAllign == 0? phical_screen.width:phical_screen.allignWidth;
    int virtual_height = phical_screen.height;     
    
    int i = 0;
    int j = 0; 
    
    unsigned short * dst_addr = fill_addr;
    unsigned short * color_addr = src_addr;   
    
    for(i = rect.top; i < rect.bottom; i++)
    {   
        for(j = rect.left; j < rect.right; j++) 
        {
            switch (phical_screen.rotation)
            {
                case 90:
                    color_addr = src_addr + draw_height*j + i;
                    dst_addr = fill_addr + ((virtual_width * i + virtual_width - j));
                    break;                    
                case 270:
                    color_addr = src_addr + draw_height*j + i;
                    dst_addr = fill_addr + ((virtual_width * (virtual_height - i - 1)+ j));
                    break;
                case 180:
                    // adjust fill in address
                    color_addr = src_addr++;
                    dst_addr = fill_addr + virtual_width * (virtual_height - i)- j-1;
                    break;                    
                default:
                    color_addr = src_addr++;
                    dst_addr = fill_addr + virtual_width * i + j;

            }
              
            //if(((i > rect.bottom-2) || ( i%400 == 0)) && ((j > rect.right-2) || (j%400 == 0)))
            //{
            //   LOG_ANIM("[show_logo_common]dst_addr = %d,color_addr= %d,i= %d,j=%d \n",(int)dst_addr, (int)color_addr, i , j);      
            //}            
            *dst_addr = *color_addr;          
        }
    }
}

/*
 * Draw aa rectangle region (32 bits per pixel)with spcial color 
 *
 * 
 *
 */
void fill_rect_with_color_by_32bit(unsigned int *fill_addr, RECT_REGION_T rect, unsigned int src_color, LCM_SCREEN_T phical_screen)
{

    //int virtual_width = phical_screen.width;
    int virtual_width = phical_screen.needAllign == 0? phical_screen.width:phical_screen.allignWidth;
    int virtual_height = phical_screen.height;
    
               
    int i = 0;
    int j = 0; 
    
    unsigned int * dst_addr = (unsigned int *)fill_addr;
 
    
    for(i = rect.top; i < rect.bottom; i++)
    {   
        for(j = rect.left; j < rect.right; j++) 
        {
            switch (phical_screen.rotation)
            {
                case 90:
                    dst_addr = fill_addr + ((virtual_width * i + virtual_width - j));
                    break;                    
                case 270:
                    dst_addr = fill_addr + ((virtual_width * (virtual_height - i - 1)+ j));
                    break;
                case 180:
                    dst_addr = fill_addr + virtual_width * (virtual_height - i)- j-1;
                    break;                    
                default:
                    dst_addr = fill_addr + virtual_width * i + j;

            }

            fill_point_buffer(dst_addr, src_color, phical_screen.bits_per_pixel);           
        }
    }
}


/*
 * Draw a rectangle region (16 bits per pixel)  with spcial color 
 *
 * 
 *
 */
void fill_rect_with_color_by_16bit(unsigned short *fill_addr, RECT_REGION_T rect, unsigned int src_color, LCM_SCREEN_T phical_screen)
{


    //int virtual_width = phical_screen.width;
    int virtual_width = phical_screen.needAllign == 0? phical_screen.width:phical_screen.allignWidth;
    int virtual_height = phical_screen.height;
    
               
    int i = 0;
    int j = 0; 
    
    unsigned short * dst_addr = fill_addr;
 
    
    for(i = rect.top; i < rect.bottom; i++)
    {   
        for(j = rect.left; j < rect.right; j++) 
        {
            switch (phical_screen.rotation)
            {
                case 90:                    
                    dst_addr = fill_addr + ((virtual_width * i + virtual_width - j));
                    break;                    
                case 270:                    
                    dst_addr = fill_addr + ((virtual_width * (virtual_height - i - 1) + j));
                    break;
                case 180:
                    dst_addr = fill_addr + virtual_width * (virtual_height - i)- j - 1;
                    break;                    
                default:
                    dst_addr = fill_addr + virtual_width * i + j;

            }
            
            *dst_addr = (unsigned short)src_color;
           
        }
    }
}


/*
 * Draw a rectangle region  with logo content 
 *
 * 
 *
 */
 
void fill_rect_with_content(void * fill_addr, RECT_REGION_T rect, unsigned short *src_addr, LCM_SCREEN_T phical_screen)

{
    if (check_rect_valid(rect) != CHECK_RECT_OK)
        return;
                    
    if (phical_screen.fill_dst_bits == 16) {
        fill_rect_with_content_by_16bit((unsigned short *)fill_addr,  rect, src_addr, phical_screen);
    } else if (phical_screen.fill_dst_bits == 32){
        fill_rect_with_content_by_32bit((unsigned int *)fill_addr,  rect, src_addr, phical_screen);
    } else {
        LOG_ANIM("[show_logo_common %s %d]unsupported phical_screen.fill_dst_bits =%d\n",__FUNCTION__,__LINE__, phical_screen.fill_dst_bits ); 
    }
}  

/*
 * Draw a rectangle region  with spcial color 
 *
 * 
 *
 */
void fill_rect_with_color(void *fill_addr, RECT_REGION_T rect, unsigned int src_color, LCM_SCREEN_T phical_screen)

{
    if (check_rect_valid(rect) != CHECK_RECT_OK)
        return;
                    
    if (phical_screen.fill_dst_bits == 16) {
        fill_rect_with_color_by_16bit((unsigned short *)fill_addr,  rect, src_color, phical_screen);
    } else if (phical_screen.fill_dst_bits == 32) {
        fill_rect_with_color_by_32bit((unsigned int *)fill_addr,  rect, src_color, phical_screen);
    } else {
        LOG_ANIM("[show_logo_common %s %d]unsupported phical_screen.fill_dst_bits =%d\n",__FUNCTION__,__LINE__, phical_screen.fill_dst_bits ); 
    }
}  
