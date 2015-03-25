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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <linux/fb.h>
#include <linux/kd.h>

#include <pixelflinger/pixelflinger.h>

#include "common.h"
#include "graphics.h"

#include <math.h>
#include "ftm.h"

const int GGL_SUBPIXEL_BITS = 4;

#define  TRI_FRACTION_BITS  (GGL_SUBPIXEL_BITS)
#define  TRI_ONE            (1 << TRI_FRACTION_BITS)
#define  TRI_HALF           (1 << (TRI_FRACTION_BITS-1))
#define  TRI_FROM_INT(x)    ((x) << TRI_FRACTION_BITS)



#if defined(SUPPORT_GB2312)
	#if defined(FEATURE_FTM_FONT_72x72)
	#include "font_chn_72x72.h"
	#elif defined(FEATURE_FTM_FONT_64x64)
	#include "font_chn_64x64.h"
	#elif defined(FEATURE_FTM_FONT_48x48)
	#include "font_chn_48x48.h"
	#elif defined(FEATURE_FTM_FONT_32x32)
	#include "font_chn_32x32.h"
	#elif defined(FEATURE_FTM_FONT_28x28)
	#include "font_chn_28x28.h"
	#elif defined(FEATURE_FTM_FONT_26x26)
	#include "font_chn_26x26.h"
	#elif defined(FEATURE_FTM_FONT_24x24)
	#include "font_chn_24x24.h"
	#elif defined(FEATURE_FTM_FONT_20x20)
	#include "font_chn_20x20.h"
	#elif defined(FEATURE_FTM_FONT_18x18)
	#include "font_chn_18x18.h"
	#elif defined(FEATURE_FTM_FONT_16x16)
	#include "font_chn_16x16.h"
	#else
	#include "font_chn_26x26.h"
	#endif
#else
  #if defined(FEATURE_FTM_FONT_36x64)
	#include "font_36x64.h"
	#elif defined(FEATURE_FTM_FONT_32x60)
	#include "font_32x60.h"
	#elif defined(FEATURE_FTM_FONT_24x44)
	#include "font_24x44.h"
	#elif defined(FEATURE_FTM_FONT_16x30)
	#include "font_16x30.h"
	#elif defined(FEATURE_FTM_FONT_16x28)
	#include "font_16x28.h"
	#elif defined(FEATURE_FTM_FONT_12x22)
	#include "font_12x22.h"
	#elif defined(FEATURE_FTM_FONT_10x18)
	#include "font_10x18.h"
	#elif defined(FEATURE_FTM_FONT_8x14)
	#include "font_8x14.h"
	#elif defined(FEATURE_FTM_FONT_6x10)
	#include "font_6x10.h"
	#else
	#include "font_12x22.h"
	#endif
#endif


typedef struct {
#if defined(SUPPORT_GB2312)	
    GGLSurface* texture;
#else
		GGLSurface texture;
#endif
    unsigned cwidth;
    unsigned cheight;
    unsigned ascent;
} GRFont;

static GRFont *gr_font = 0;
static GGLContext *gr_context = 0;
static GGLSurface gr_font_texture;
static GGLSurface gr_framebuffer[2];
static GGLSurface gr_mem_surface;
static unsigned gr_active_fb = 0;

static int gr_fb_fd = -1;
static int gr_vt_fd = -1;

static struct fb_var_screeninfo vi;



#if defined(SUPPORT_GB2312)
// ============start===============       
#define GB2312_FIRST_MIN 0xB0
#define GB2312_FIRST_MAX 0xD7
/*0xF7*/
#define GB2312_SECOND_MIN 0xA1
#define GB2312_SECOND_MAX 0xFE

#define GB2312_AND_ENG_SPACE ((GB2312_FIRST_MAX - GB2312_FIRST_MIN+1)*(GB2312_SECOND_MAX - GB2312_SECOND_MIN+1)+96)
#define GB2312_AND_ENG_MAP(First, Second) ((First-GB2312_FIRST_MIN)*(GB2312_SECOND_MAX - GB2312_SECOND_MIN + 1) + (Second-GB2312_SECOND_MIN) +96)

static int gr_font_get_coord(char first, char second)
{
      if(first < GB2312_FIRST_MIN || first > GB2312_FIRST_MAX
        ||second <GB2312_SECOND_MIN || second>GB2312_SECOND_MAX)
      {
         //some char coding. like UTF8.
         return 0;
      }
      else
      {
            return GB2312_AND_ENG_MAP(first, second);
      }
}

static int gr_font_texture_split(int* block, int* residue)
{
      int pivotal;      
      pivotal = (8192+font.cwidth-1)/font.cwidth * font.cwidth;      
      *block = font.width/pivotal;
      *residue = font.width % pivotal;      
      //fprintf(stderr, "gr_font_texture_split FIN, block%d, residue%d, pivotal%d\n", *block, *residue, pivotal);
      return pivotal;      
}

static int gr_font_which_context(char* s, int* str_offset, int* off)
{
      int pivotal;
      int which = 0;
      int coord;      
      pivotal = (8192+font.cwidth-1)/font.cwidth * font.cwidth;
      
      if((*s & 0x80) != 0)// GB2312
      {
            *str_offset = 2;
            coord = gr_font_get_coord(*s, *(s+1));
            which = coord / (pivotal/font.cwidth);  //coord is 0 based.
            *off = coord % (pivotal/font.cwidth);  //coord is 0 based.
      }
      else
      {
            *str_offset = 1;
            which = 0;
            if(*s < 32 ||*s >128)
            {
                *off = 0;
            }
            else
            {
                 *off = *s - 32;
            }
      }      
      //fprintf(stderr, "gr_font_which_context FIN, %s, str_offset%d, off%d, which%d\n", s, *str_offset, *off, which);
      return which;
}

//================end================

static void gr_init_font(void)
{    
    unsigned char *bits, *rle, *org_bits;
    unsigned char *in, data; 
    int block, residue, pivotal, surface_num;
    unsigned int i,j;
    void* bits_split;
  
    gr_font = calloc(sizeof(*gr_font), 1);
    gr_font->cwidth = font.cwidth;
    gr_font->cheight = font.cheight;
    gr_font->ascent = font.cheight - 2;

    org_bits = bits = malloc(font.width * font.height);    

    in = font.rundata;   
   
    while((data = *in++)) {
        memset(bits, (data & 0x80) ? 255 : 0, data & 0x7f);
        bits += (data & 0x7f);
    } 
      
      pivotal = gr_font_texture_split(&block, &residue);
      
      if(residue != 0)
      {
        surface_num = block+1;
    }
    else
    {
      surface_num = block;
    }
      gr_font->texture = malloc(surface_num*sizeof(GGLSurface));
      
      for(i=0; i<block; i++)
      {
            bits_split = malloc(pivotal * font.height);
            gr_font->texture[i].version = sizeof(GGLSurface);
          gr_font->texture[i].width = pivotal;
          gr_font->texture[i].height = font.height;
          gr_font->texture[i].stride = pivotal;
          gr_font->texture[i].data = (void*) bits_split;
          gr_font->texture[i].format = GGL_PIXEL_FORMAT_A_8;
          
          //fprintf(stderr, "bits_split%d\n", bits_split);
          for(j=0; j<font.height; j++)
          {
            memcpy(bits_split+ j*pivotal, org_bits+i*pivotal+j*font.width, pivotal);
          }     
      }
   // fprintf(stderr, "block over\n");
      if(residue != 0)
      {
        bits_split = malloc(residue * font.height);
            gr_font->texture[i].version = sizeof(GGLSurface);
          gr_font->texture[i].width = residue;
          gr_font->texture[i].height = font.height;
          gr_font->texture[i].stride = residue;
          gr_font->texture[i].data = (void*) bits_split;
          gr_font->texture[i].format = GGL_PIXEL_FORMAT_A_8;
          
          for(j=0; j<font.height; j++)
          {
            memcpy(bits_split+ j*residue, org_bits+block*pivotal+j*font.width, residue);
          }
   }
   
   free(org_bits);   
   //perror("gr_init_font FIN");
}
/*
int gr_text_scale(int x, int y, const char *s, int scale)
{
    GGLContext *gl = gr_context;
    GRFont *font = gr_font;
    unsigned int off, str_offset;
    int which;
    int cwidth=0; //char width

    y -= font->ascent;
            
    gl->texEnvi(gl, GGL_TEXTURE_ENV, GGL_TEXTURE_ENV_MODE, GGL_REPLACE);
    gl->texGeni(gl, GGL_S, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->texGeni(gl, GGL_T, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->enable(gl, GGL_TEXTURE_2D); //open all the 2D functions
    
   while(*s != 0)
   {  
          str_offset=0;
          off =0;
                
          which = gr_font_which_context(s, &str_offset, &off);
          gl->bindTexture(gl, &font->texture[which]);                 

                 
          cwidth = (font->cwidth);
          if(*s < 0x80)  //if char is english char, the width should be narrow 
          {  
               cwidth = cwidth/2;
          }
        
          gl->texCoord2i(gl, (off * font->cwidth)-x , 0 -y);
          gl->recti(gl, x, y, x+cwidth, y+font->cheight); //show the char
          x += cwidth; 

      
          s += str_offset;

                  
   }
     
    return x;
}
*/

int gr_text(int x, int y, const char *s)
{
    GGLContext *gl = gr_context;
    GRFont *font = gr_font;
    unsigned int off, str_offset;
    int which;
    int cwidth=0; //char width

    y -= font->ascent;
            
    gl->texEnvi(gl, GGL_TEXTURE_ENV, GGL_TEXTURE_ENV_MODE, GGL_REPLACE);
    gl->texGeni(gl, GGL_S, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->texGeni(gl, GGL_T, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->enable(gl, GGL_TEXTURE_2D);
    
   // fprintf(stderr, "gr_text START, ");
   //LOGE("X" "Sgr-------------------------------");
   //LOGE("X" "%s", s);
   while(*s != 0)
   {
          /* //origin code:
          str_offset=0;
          off =0;
                  
          // LOGE("X" "[0x%X] %c", *s, *s);
          which = gr_font_which_context(s, &str_offset, &off);
          gl->bindTexture(gl, &font->texture[which]);                 
                 
          gl->texCoord2i(gl, (off * font->cwidth)-x , 0 -y);
          gl->recti(gl, x, y, x+font->cwidth, y+font->cheight);
          x += font->cwidth; 
      
          s += str_offset; */
 
          //code to revise wide char to narrow char:
          str_offset=0;
          off =0;
                
          which = gr_font_which_context(s, &str_offset, &off);
          gl->bindTexture(gl, &font->texture[which]);                 

                 
          cwidth = (font->cwidth);
          if(*s < 0x80)  //if char is english char, the width should be narrow 
          {  
               cwidth = cwidth/2;
          }
        
          gl->texCoord2i(gl, (off * font->cwidth)-x , 0 -y);
          gl->recti(gl, x, y, x+cwidth, y+font->cheight); //show the char
          x += cwidth; 

      
          s += str_offset;

                  
   }
     
    return x;
}

int gr_measure(const char *s)
{
      int count = 0;
      while(*s != 0)
      {
            if((*s & 0x80) != 0)
            {
                  s += 2;                 
            }
            else
            {
                  s += 1;
            }
            count += 1;
      }
  
  return gr_font->cwidth * count;
}

#else

static void gr_init_font(void)
{
    GGLSurface *ftex;
    unsigned char *bits, *rle;
    unsigned char *in, data;

    gr_font = calloc(sizeof(*gr_font), 1);
    ftex = &gr_font->texture;

    bits = malloc(font.width * font.height);

    ftex->version = sizeof(*ftex);
    ftex->width = font.width;
    ftex->height = font.height;
    ftex->stride = font.width;
    ftex->data = (void*) bits;
    ftex->format = GGL_PIXEL_FORMAT_A_8;

    in = font.rundata;
    while((data = *in++)) {
        memset(bits, (data & 0x80) ? 255 : 0, data & 0x7f);
        bits += (data & 0x7f);
    }

    gr_font->cwidth = font.cwidth;
    gr_font->cheight = font.cheight;
    gr_font->ascent = font.cheight - 2;
}

int gr_text(int x, int y, const char *s)
{
    GGLContext *gl = gr_context;
    GRFont *font = gr_font;
    unsigned off;

    y -= font->ascent;

    gl->bindTexture(gl, &font->texture);
    gl->texEnvi(gl, GGL_TEXTURE_ENV, GGL_TEXTURE_ENV_MODE, GGL_REPLACE);
    gl->texGeni(gl, GGL_S, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->texGeni(gl, GGL_T, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->enable(gl, GGL_TEXTURE_2D);

    while((off = *s++)) {
        off -= 32;
        if (off < 96) {
            gl->texCoord2i(gl, (off * font->cwidth) - x, 0 - y);
            gl->recti(gl, x, y, x + font->cwidth, y + font->cheight);
        }
        x += font->cwidth;
    }

    return x;
}

int gr_measure(const char *s)
{
    return gr_font->cwidth * strlen(s);
}

#endif

static int get_framebuffer(GGLSurface *fb)
{
    int fd;
    struct fb_fix_screeninfo fi;
    void *bits;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        perror("cannot open fb0");
        return -1;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) {
        perror("failed to get fb0 info");
        close(fd);
        return -1;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        perror("failed to get fb0 info");
        close(fd);
        return -1;
    }
#ifdef FACTORY_MODE_SUPPORT_RGBA_8888
    LOGD("FACTORY_MODE_SUPPORT_RGBA_8888\n");
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
	perror("failed to get fb0 vinfo");
	close(fd);
	return -1;
    }

    vi.bits_per_pixel = 32;

    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
	perror("failed to put fb0 vinfo");
	close(fd);
	return -1;
    }
#endif
    bits = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bits == MAP_FAILED) {
        perror("failed to mmap framebuffer");
        close(fd);
        return -1;
    }

    fb->version = sizeof(*fb);
	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3)
	||0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2)){
	    fb->width = vi.yres;
    	fb->height = vi.xres;
    	fb->stride = vi.yres;
	}
	else{
		fb->width = vi.xres;
    	fb->height = vi.yres;
    	fb->stride = vi.xres;
	}
    fb->data = bits;
#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    LOGD("fb->width=%d, fb->height=%d\n", fb->width, fb->height);
    LOGD("define FACTORY_MODE_SUPPORT_BGRA_8888\n");
    fb->format = GGL_PIXEL_FORMAT_BGRA_8888;
#else
    LOGD("not define FACTORY_MODE_SUPPORT_BGRA_8888\n");
    fb->format = GGL_PIXEL_FORMAT_RGB_565;
#endif

    fb++;

    fb->version = sizeof(*fb);
	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3)
	||0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2)){
	    fb->width = vi.yres;
    	fb->height = vi.xres;
    	fb->stride = vi.yres;
	}
	else{
		fb->width = vi.xres;
    	fb->height = vi.yres;
    	fb->stride = vi.xres;
	}
    LOGD("fb->width=%d, fb->height=%d\n", fb->width, fb->height);
#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    LOGD("vi.yres=%d, vi.xres_virtual=%d\n", vi.yres, vi.xres_virtual);
    fb->data = (void*) (((unsigned) bits) + vi.yres * vi.xres_virtual * 4);
#else
    fb->data = (void*) (((unsigned) bits) + vi.yres * vi.xres_virtual * 2);
#endif

#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    fb->format = GGL_PIXEL_FORMAT_BGRA_8888;
#else
    fb->format = GGL_PIXEL_FORMAT_RGB_565;
#endif

    return fd;
}

static void get_memory_surface(GGLSurface* ms) {
  ms->version = sizeof(*ms);
  if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3)
	||0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2)){
	ms->width = vi.yres;
  	ms->height = vi.xres;
  	ms->stride = vi.yres;
  }
  else{
	ms->width = vi.xres;
  	ms->height = vi.yres;
  	ms->stride = vi.xres;
  }
  
#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
  ms->data = malloc(vi.xres * vi.yres * 4);
#else
  ms->data = malloc(vi.xres * vi.yres * 2);
#endif

#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    ms->format = GGL_PIXEL_FORMAT_BGRA_8888;
#else
  ms->format = GGL_PIXEL_FORMAT_RGB_565;
#endif
}

static void set_active_framebuffer(unsigned n)
{
    if (n > 1) return;
    vi.yres_virtual = vi.yres * 2;
    vi.yoffset = n * vi.yres;
	
#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    vi.bits_per_pixel = 32;
#else
    vi.bits_per_pixel = 16;
#endif

    if (ioctl(gr_fb_fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
        perror("active fb swap failed");
    }
}

void gr_flip(void)
{
    GGLContext *gl = gr_context;
    int j,k;
	
#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    unsigned fb_lineLength = vi.xres_virtual * 4;
#else
    unsigned fb_lineLength = vi.xres_virtual * 2;
#endif

#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
    unsigned mem_surface_lineLength = vi.xres * 4;
#else
	unsigned mem_surface_lineLength = vi.xres * 2;
#endif
    void *d = NULL;
    void *s = gr_mem_surface.data;
	unsigned int width = vi.xres_virtual;
	unsigned int height = vi.yres;
    /* swap front and back buffers */
    gr_active_fb = (gr_active_fb + 1) & 1;
    d = gr_framebuffer[gr_active_fb].data;
    /* copy data from the in-memory surface to the buffer we're about
     * to make active. */

	if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3))
	{
		unsigned int l;
        #ifdef FACTORY_MODE_SUPPORT_RGBA_8888
    		unsigned int *s_temp;
    		unsigned int *d_temp;

    		s_temp = (unsigned int*)s;
        #else
		unsigned short *s_temp;
		unsigned short *d_temp;

		s_temp = (unsigned short*)s;
        #endif
		for (j=0; j<width; j++){
	  		for (k=0, l=height-1; k<height; k++, l--)
	    	{
				d_temp = d + ((width * l + j) * sizeof(d_temp));
				*d_temp = *s_temp++;
	    	}
		}
	}
	else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2))
	{
		unsigned int l;
        #ifdef FACTORY_MODE_SUPPORT_RGBA_8888
    		unsigned int *s_temp;
    		unsigned int *d_temp;

    		s_temp = (unsigned int*)s;
        #else
		unsigned short *s_temp;
		unsigned short *d_temp;

		s_temp = (unsigned short*)s;
        #endif

		for (j=width - 1; j>=0; j--){
	  		for (k=0, l=0; k<height; k++, l++)
	    	{
				d_temp = d + ((width * l + j) * sizeof(d_temp));
				*d_temp = *s_temp++;
	    	}
		}
	}
        else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3))
	{
		unsigned int l;
        #ifdef FACTORY_MODE_SUPPORT_RGBA_8888
    		unsigned int *s_temp;
    		unsigned int *d_temp;

    		s_temp = (unsigned int*)s;
        #else
		unsigned short *s_temp;
		unsigned short *d_temp;

		s_temp = (unsigned short*)s;
        #endif

		for (j=height - 1; j>=0; j--)
                {
         	    for (k=0, l=vi.xres-1; k<vi.xres; k++, l--)
	    	    {
			d_temp = d + ((width * j + l) * sizeof(d_temp));
			*d_temp = *s_temp++;
	    	    }
		}
	}
	else{
    	    for (j = 0; j < vi.yres; ++ j)
    	    {
        	memcpy(d, s, mem_surface_lineLength);
			d += fb_lineLength;
			s += mem_surface_lineLength;
    	    }
	}
/*
    memcpy(gr_framebuffer[gr_active_fb].data, gr_mem_surface.data,
           vi.xres * vi.yres * 2);
*/
    /* inform the display driver */
    set_active_framebuffer(gr_active_fb);
}

void gr_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    GGLContext *gl = gr_context;
    GGLint color[4];
    color[0] = ((r << 8) | r) + 1;
    color[1] = ((g << 8) | g) + 1;
    color[2] = ((b << 8) | b) + 1;
    color[3] = ((a << 8) | a) + 1;
    gl->color4xv(gl, color);
}





void gr_fill(int x, int y, int w, int h)
{
    GGLContext *gl = gr_context;
    gl->disable(gl, GGL_TEXTURE_2D);
    gl->recti(gl, x, y, w, h);
}

void gr_point(int x, int y, int radius)
{
    GGLContext *gl = gr_context;
    gl->disable(gl, GGL_TEXTURE_2D);
    
    GGLcoord point[2];
    point[0]= TRI_FROM_INT(x);
    point[1]= TRI_FROM_INT(y);
    GGLcoord gl_radius = TRI_FROM_INT(radius);
    const GGLcoord* v = point;

    gl->pointx(gl, v, gl_radius);
}

void gr_line(int sx, int sy, int dx, int dy, int width)
{
    GGLContext *gl = gr_context;
    gl->disable(gl, GGL_TEXTURE_2D);

    GGLcoord s[2];
    GGLcoord d[2];
    GGLcoord gl_width = TRI_FROM_INT(width);
    const GGLcoord* v0 = s;
    const GGLcoord* v1 = d;   
  
    s[0] = TRI_FROM_INT(sx);
    s[1] = TRI_FROM_INT(sy);
    d[0] = TRI_FROM_INT(dx);
    d[1] = TRI_FROM_INT(dy);
   
    gl->linex(gl, v0, v1, gl_width);
}

void gr_circle(int x, int y, int radius)
{
    GGLContext *gl = gr_context;
    gl->disable(gl, GGL_TEXTURE_2D);
#if 0    
    GGLcoord point[2];
    point[0]= TRI_FROM_INT(x);
    point[1]= TRI_FROM_INT(y);    

    const GGLcoord* v = &point;
#endif    

    GGLcoord s[2];
    GGLcoord d[2];
    const GGLcoord* v0 = s;
    const GGLcoord* v1 = d;    

    int i = 0, j = 0;
    int r = radius;
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0, x4 = 0, y4 = 0;
    
    for(i=x-r; i<=x+r; i++) 
    {
        x1=x2; y1=y2; x3=x4; y3=y4;
        x2 = i;
        y2 = (int)((float)y+r*sin(acos(((float)i-x)/r)));
        x4 = i;
        y4 = (int)((float)y-r*sin(acos(((float)i-x)/r)));      
        if(j==0) 
        { 
            j=1; x1=x2; y1=y2; x3=x4; y3=y4; 
        }
#if 0        
        point[0]= TRI_FROM_INT(x1);
        point[1]= TRI_FROM_INT(y1);
        gl->pointx(gl, v, TRI_ONE);
        point[0]= TRI_FROM_INT(x2);
        point[1]= TRI_FROM_INT(y2);
        gl->pointx(gl, v, TRI_ONE);
        point[0]= TRI_FROM_INT(x3);
        point[1]= TRI_FROM_INT(y3);
        gl->pointx(gl, v, TRI_ONE);
        point[0]= TRI_FROM_INT(x4);
        point[1]= TRI_FROM_INT(y4);
        gl->pointx(gl, v, TRI_ONE);        
#endif
        s[0] = TRI_FROM_INT(x1);
        s[1] = TRI_FROM_INT(y1);
        d[0] = TRI_FROM_INT(x2);
        d[1] = TRI_FROM_INT(y2);        
        gl->linex(gl, v0, v1, TRI_ONE);
        s[0] = TRI_FROM_INT(x3);
        s[1] = TRI_FROM_INT(y3);
        d[0] = TRI_FROM_INT(x4);
        d[1] = TRI_FROM_INT(y4);        
        gl->linex(gl, v0, v1, TRI_ONE);            
    }
}

void gr_blit(gr_surface source, int sx, int sy, int w, int h, int dx, int dy) {
    if (gr_context == NULL) {
        return;
    }
    GGLContext *gl = gr_context;

    gl->bindTexture(gl, (GGLSurface*) source);
    gl->texEnvi(gl, GGL_TEXTURE_ENV, GGL_TEXTURE_ENV_MODE, GGL_REPLACE);
    gl->texGeni(gl, GGL_S, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->texGeni(gl, GGL_T, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->enable(gl, GGL_TEXTURE_2D);
    gl->texCoord2i(gl, sx - dx, sy - dy);
    gl->recti(gl, dx, dy, dx + w, dy + h);
}

unsigned int gr_get_width(gr_surface surface) {
    if (surface == NULL) {
        return 0;
    }
    return ((GGLSurface*) surface)->width;
}

unsigned int gr_get_height(gr_surface surface) {
    if (surface == NULL) {
        return 0;
    }
    return ((GGLSurface*) surface)->height;
}



int gr_init(void)
{
    gglInit(&gr_context);
    GGLContext *gl = gr_context;

    gr_init_font();
    gr_vt_fd = open("/dev/tty0", O_RDWR | O_SYNC);
    if (gr_vt_fd < 0) {
        // This is non-fatal; post-Cupcake kernels don't have tty0.
        perror("can't open /dev/tty0");
    } else if (ioctl(gr_vt_fd, KDSETMODE, (void*) KD_GRAPHICS)) {
        // However, if we do open tty0, we expect the ioctl to work.
        perror("failed KDSETMODE to KD_GRAPHICS on tty0");
        gr_exit();
        return -1;
    }

    gr_fb_fd = get_framebuffer(gr_framebuffer);
    if (gr_fb_fd < 0) {
        gr_exit();
        return -1;
    }

    get_memory_surface(&gr_mem_surface);

    fprintf(stderr, "framebuffer: fd %d (%d x %d)\n",
            gr_fb_fd, gr_framebuffer[0].width, gr_framebuffer[0].height);

        /* start with 0 as front (displayed) and 1 as back (drawing) */
    gr_active_fb = 0;

    //set_active_framebuffer(0);
    { 
       vi.yres_virtual = vi.yres * 2;
       vi.yoffset = 0 * vi.yres;
	   
#if defined(FACTORY_MODE_SUPPORT_RGBA_8888)
       	vi.bits_per_pixel = 32;
#else
       vi.bits_per_pixel = 16;
#endif
    }

    gl->colorBuffer(gl, &gr_mem_surface);


    gl->activeTexture(gl, 0);
    gl->enable(gl, GGL_BLEND);
    gl->blendFunc(gl, GGL_SRC_ALPHA, GGL_ONE_MINUS_SRC_ALPHA);

    return 0;
}

void gr_exit(void)
{
    close(gr_fb_fd);
    gr_fb_fd = -1;

    free(gr_mem_surface.data);

    ioctl(gr_vt_fd, KDSETMODE, (void*) KD_TEXT);
    close(gr_vt_fd);
    gr_vt_fd = -1;
}

int gr_fb_width(void)
{
    return gr_framebuffer[0].width;
}

int gr_fb_height(void)
{
    return gr_framebuffer[0].height;
}

gr_pixel *gr_fb_data(void)
{
    return (unsigned short *) gr_mem_surface.data;
}
