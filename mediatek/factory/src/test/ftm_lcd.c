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
#include <errno.h>
#include <fcntl.h>

#include <common.h>
#include <miniui.h>
#include <ftm.h>
#include <sys/time.h>
#include <cust_lcd.h>
#include <linux/fb.h>
#include "mtkfb_info.h"
#include "mtkfb.h"
#ifdef FEATURE_FTM_LCD

#define BRIGHTNESS_FILE "/sys/class/leds/lcd-backlight/brightness"

typedef enum {
    Image_Red = 0,
    Image_Green,
    Image_Blue,
    Image_White,
    Image_Black,
    Image_Frame,
    Image_Crosstalk,
    Image_ColorBar_Gray,
    Image_ColorBar_Red,
    Image_ColorBar_Green,
    Image_ColorBar_Blue,
    Image_Fps,
    IMAGE_POWER,
    Image_Num
};

static unsigned int test_img_index = 0;

static int seq_index = 0;
static int brightness_seq[] = {
	50, 100, 150, 200, 255, -1
};

#define TAG	"[LCD] "
#define mod_to_lcd(p)	(lcd_module *)((char *)(p) + sizeof(struct ftm_module))

#define ARY_SIZE(x)     (sizeof((x)) / sizeof((x[0])))

enum {
	ITEM_SHOW_TEST_IMAGES,
	ITEM_CHANGE_CONTRAST,
	ITEM_PASS,
	ITEM_FAIL
};

static item_t lcd_item[] = {
	item(ITEM_SHOW_TEST_IMAGES, uistr_info_show_test_images),
	item(ITEM_CHANGE_CONTRAST, uistr_info_change_contrast),
	item(ITEM_PASS,             uistr_pass),
	item(ITEM_FAIL,             uistr_fail),
	item(-1, NULL),
};

DEFINE_TEST_IMAGE_FILENAMES(TEST_IMAGES);

typedef struct {
	struct ftm_module *module;

    /* image view */
    struct imageview img_view;
    char img_title[128];
    char img_filename[128];
    text_t img_title_ctext;
    unsigned int img_index;

    /* item view */
	struct itemview *itm_view;
    text_t itm_title_ctext;
} lcd_module;


static int
write_int(char const* path, int value)
{
	int fd;

	if (path == NULL)
		return -1;

	fd = open(path, O_RDWR);
	if (fd >= 0) {
		char buffer[20];
		int bytes = sprintf(buffer, "%d\n", value);
		int amt = write(fd, buffer, bytes);
		close(fd);
		return amt == -1 ? -errno : 0;
	}

	LOGE("write_int failed to open %s\n", path);
	return -errno;
}

static void change_contrast(int key, void *priv)
{
	lcd_module *lcd = (lcd_module *)priv;
	struct imageview *iv = &lcd->img_view;

	int blk_show_image_flag = 0;
	bool exit = false;

	switch (key)
    {
    case UI_KEY_UP:
    case UI_KEY_VOLUP:
		exit = true;
		break;
    case UI_KEY_DOWN:
    case UI_KEY_VOLDOWN:
   		write_int(BRIGHTNESS_FILE, brightness_seq[seq_index]);
		if (brightness_seq[++seq_index] == -1)
			seq_index = 0;
        break;

    case UI_KEY_BACK:
	case UI_KEY_CONFIRM:
        exit = true;
        break;

    default:
        break;
    }
	if((blk_show_image_flag == 0) && (!exit)) {			
		ui_color(255, 255, 255, 255);
		ui_fill(0, 0, ui_fb_width(), ui_fb_height());
		ui_flip();				
	    blk_show_image_flag = 1;
    }
	if (exit) {
        iv->exit(iv);
	}

}

static void update_imageview(lcd_module *lcd)
{
	struct imageview *iv = &lcd->img_view;
    unsigned int i = lcd->img_index;

    LOGD(TAG "update_imageview(%d)\n", i);

    sprintf(lcd->img_title, "LCD Test Image (%d/%d)", 
            i + 1, ARY_SIZE(TEST_IMAGES));

    sprintf(lcd->img_filename,
            "system/res/images/%s.png", TEST_IMAGES[i]);

    init_text(&lcd->img_title_ctext, lcd->img_title, COLOR_YELLOW);

    //iv->set_title(iv, &lcd->img_title_ctext);
    iv->set_title(iv, NULL);
    iv->set_image(iv, lcd->img_filename, 0, 0);
}

static long int get_current_time_us(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}

typedef enum errortype{
	FB_OPEN_FAILED,
	FB_GET_VINFO_FAILED,
	FB_PUT_VINFO_FAILED,
	FB_PAN_DISPLAY_FAILED,
	FB_POWER_ON_FAILED,
	FB_POWER_OFF_FAILED,
	FB_ERROR_TYPE_NUM
};

unsigned char spy_error_type[FB_ERROR_TYPE_NUM][32] = 
{
	"open fb0 device failed",
	"get vinfo failed",
	"put vinfo failed",
	"pan display failed"
};

static int lcd_imageview_key_handler(int key, void *priv) 
{
    lcd_module *lcd = (lcd_module *)priv;
	struct imageview *iv = &lcd->img_view;
    int handled = 0;
    bool exit = false;
	unsigned int w = ui_fb_width();
	unsigned int h = ui_fb_height();
	unsigned int i = 0;

    switch (key)
    {
    case UI_KEY_UP:
    case UI_KEY_VOLUP:
        if (test_img_index > 0)
        {
            -- test_img_index;
        }
        break;

    case UI_KEY_DOWN:
    case UI_KEY_VOLDOWN:
        if (test_img_index < (ARY_SIZE(TEST_IMAGES) + Image_Num - 1))
        {
		test_img_index++;
        }
	else
	{
		test_img_index = 0;
	}
        break;

    case UI_KEY_BACK:
	case UI_KEY_CONFIRM:
        exit = true;
        break;

    default:
        handled = -1;
        break;
    }

    if (exit) {
        iv->exit(iv);
    } else if (-1 != handled) {
        if (test_img_index < ARY_SIZE(TEST_IMAGES))
        {
            /* show test image */
        	lcd->img_index = test_img_index;
        	update_imageview(lcd);
        }
        else
        {
            /* draw pure color image */
        	switch (test_img_index - ARY_SIZE(TEST_IMAGES))
        	{
        	case Image_Red:
        		ui_color(255, 0, 0, 255);
	        	ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();
        		break;
        	case Image_Green:
        		ui_color(0, 255, 0, 255);
			ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();
        		break;
        	case Image_Blue:
        		ui_color(0, 0, 255, 255);
			ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();
        		break;
        	case Image_White:
        		ui_color(255, 255, 255, 255);
			ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();
        		break;
        	case Image_Black:
        		ui_color(0, 0, 0, 255);
			ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();
        		break;
		case  Image_Frame:
		{
			ui_color(0, 0, 0, 255);
	        	ui_fill(0, 0, w, h);

			ui_color(255, 255, 255, 255);
			ui_line(0, 0, w-1, h-1, 1);
			ui_line(w-1, 0, 0, h-1, 1);
			ui_line(0, 0, 0, h-1, 1);
			ui_line(0, 0, w-1, 0, 1);
			ui_line(w-1, 0, w-1, h-1, 1);
			ui_line(0, h-1, w-1, h-1, 1);

			ui_flip();
			break;
		}
    		case Image_Crosstalk:
		{
			ui_color(128, 128, 128, 255);
	        	ui_fill(0, 0, w, h);
			ui_color(255, 255, 255, 255);
			ui_fill(w/4, h/4, w/2, h/2);
			ui_fill(w/2, h/2, w/4*3, h/4*3);
			ui_flip();
			break;
		}
		case Image_ColorBar_Gray:
		{
			for(i=0;i<h;i++)
			{
				ui_color(0xff*i/h, 0xff*i/h, 0xff*i/h, 255);
				ui_line(0, i, w, i, 1);
			}
			ui_flip();
			break;
		}
		case Image_ColorBar_Red:
		{
			for(i=0;i<h;i++)
			{
				ui_color(0xff*i/h, 0, 0, 255);
				ui_line(0, i, w, i, 1);
			}
			ui_flip();
			break;
		}
		case Image_ColorBar_Green:
		{
			for(i=0;i<h;i++)
			{
				ui_color(0, 0xff*i/h, 0, 255);
				ui_line(0, i, w, i, 1);
			}
			ui_flip();
			break;
		}
		case Image_ColorBar_Blue:
		{
			for(i=0;i<h;i++)
			{
				ui_color(0, 0, 0xff*i/h, 255);
				ui_line(0, i, w, i, 1);
			}
			ui_flip();
			break;
		}
    		case Image_Fps:
		{
			
			int ret = 0;
			int test_count = 500;
			int fd= 0;			
			int max = 0;
			int min = 0xffffffff;
			struct fb_var_screeninfo vi;
			mtk_dispif_info_t dispif;
			memset(&dispif, 0, sizeof(mtk_dispif_info_t));
			
			fd = open("/dev/graphics/fb0", O_RDONLY);
			if(fd < 0) 
			{
				ret = FB_OPEN_FAILED;
				goto done;
			}

			if(ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) 
			{
				ret = FB_GET_VINFO_FAILED;
				goto done;
			}
			if(ioctl(fd, MTKFB_GET_DISPLAY_IF_INFORMATION, &dispif) < 0) 
			{
				ret = FB_PUT_VINFO_FAILED;
				goto done;
			}
			
			unsigned int fps_claimed = dispif.vsyncFPS;
			unsigned int fps = 0;
			unsigned int i = 0;
			unsigned int fliptime = 0;
			char string[128];
			unsigned int ttt = 0;
			unsigned int ttt2 = 0;

			ui_color(255, 255, 255, 255);
			ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();
        		
        		ui_color(0, 0, 0, 255);
			ui_fill(0, 0, ui_fb_width(), ui_fb_height());
			ui_flip();

			for(i=0;i<10;i++)
			{
  				vi.yoffset = (i%2) * vi.yres;

				if (ioctl(fd, FBIOPUT_VSCREENINFO, &vi) < 0) 
				{
					ret = FB_PAN_DISPLAY_FAILED;
					goto done;
				}
			}
			
			ttt = get_current_time_us();
			for(i=0;i<test_count;i++)
			{
				vi.yoffset = (i%2) * vi.yres;

				if (ioctl(fd, FBIOPUT_VSCREENINFO, &vi) < 0) 
				{
					ret = FB_PAN_DISPLAY_FAILED;
					goto done;
				}
			}
			
			fliptime = get_current_time_us() - ttt;
			done:
				ui_color(255, 255, 255, 255);
				ui_fill(0, 0, w, h);
				ui_flip();
				ui_color(0, 0, 0, 255);

				if(ret == 0)
				{
					if(!fliptime) fliptime = 1;
					fps = 1000*1000*100/(fliptime/test_count);
					//sprintf(string, "run %d times, fps=%d.%d, max=%d, min=%d, fps claimed=%d.%d\n", i, fps/100, fps%100, max, min, fps_claimed/100, fps_claimed%100);
					//ui_text(0, 0, string);
					ui_printf("run %d times\nfps=%d.%d\nfps claimed=%d.%d\n", i, fps/100, fps%100, fps_claimed/100, fps_claimed%100);
					ui_flip();
				}
				else
				{
					sprintf(string, "test failed, reason: %s\n",spy_error_type[ret]);
					ui_text(0, 0, string);
					ui_printf(string);
					ui_flip();
				}

				close(fd);
			break;
		}
		case IMAGE_POWER:
		{
			int ret = 0;
			int test_count = 100;
			int i = 0;
			int fd = open("/dev/graphics/fb0", O_RDONLY);
			if(fd < 0) 
			{
				ret = FB_OPEN_FAILED;
				goto done2;
			}
	
		
			for(i = 0;i < test_count;i++)
			{
				if(ioctl(fd, MTKFB_POWEROFF) < 0) 
				{
					ret = FB_POWER_OFF_FAILED;
					goto done2;
				}
				
				if(ioctl(fd, MTKFB_POWERON) < 0) 
				{
					ret = FB_POWER_ON_FAILED;
					goto done2;
				}
				
				ui_printf("%d times\n", i);
				ui_flip();	
			}

			ui_color(255, 255, 255, 255);
			ui_fill(0, 0, w, h);
			ui_flip();
			ui_color(0, 0, 0, 255);
			ui_printf("run %d times\ntest pass\n", i);
			ui_flip();

			done2:
			close(fd);
		}
        	default:
        		break;
        	}


        	//ui_flip();

        }

    }

    return handled;
}


static void display_test_images(lcd_module *lcd)
{
	struct imageview *iv = &lcd->img_view;

    ui_init_imageview(iv, lcd_imageview_key_handler, lcd);

    /* reset all the img index */
    test_img_index = 0;
    lcd->img_index = 0;

    update_imageview(lcd);
    iv->run(iv);
}


static void init_main_menu(struct ftm_param *param, lcd_module *lcd)
{
    struct itemview *iv = lcd->itm_view;

    init_text(&lcd->itm_title_ctext, param->name, COLOR_YELLOW);

    iv->set_title(iv, &lcd->itm_title_ctext);
    iv->set_items(iv, lcd_item, ITEM_SHOW_TEST_IMAGES);
}
//need this func to show white image for BLK test
static void blk_update_imageview(lcd_module *lcd)
{
	struct imageview *iv = &lcd->img_view;
	lcd->img_index = lcd->img_index + 1;
    unsigned int i = lcd->img_index;
	
    LOGD(TAG "blk_update_imageview(%d)\n", i);

    //sprintf(lcd->img_title, "LCD Test Image (%d/%d)", 
     //       i + 1, ARY_SIZE(TEST_IMAGES));

    //sprintf(lcd->img_filename,
     //       "/res/images/%s.png", TEST_IMAGES[i]);

    init_text(&lcd->img_title_ctext, lcd->img_title, COLOR_YELLOW);

    //iv->set_title(iv, &lcd->img_title_ctext);
    iv->set_title(iv, NULL);
   // iv->set_image(iv, lcd->img_filename, 0, 0);
}

static void blk_test_images(lcd_module *lcd)
{
	struct imageview *iv = &lcd->img_view;

    ui_init_imageview(iv, change_contrast, lcd);

    /* reset all the img index */
   // test_img_index = 0;
    lcd->img_index = 0;

    blk_update_imageview(lcd);
    iv->run(iv);
}
/////////////////////////////////////////////////
static int lcd_entry(struct ftm_param *param, void *priv)
{
	lcd_module *lcd = (lcd_module *)priv;

    bool exit = false;
    seq_index = 0;

    init_main_menu(param, lcd);
    while(!exit)
    {
        switch(lcd->itm_view->run(lcd->itm_view, NULL))
        {
        case ITEM_SHOW_TEST_IMAGES:
            display_test_images(lcd);
            init_main_menu(param, lcd);
            exit = false;
            break;
        case ITEM_CHANGE_CONTRAST:
			blk_test_images(lcd);//show white image and test BLK level
            init_main_menu(param, lcd);
            exit = false;
            break;
        case ITEM_PASS:
            lcd->module->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        case ITEM_FAIL:
            lcd->module->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case -1:
            exit = true;
            break;
        default:
            break;
        }
    }
    write_int(BRIGHTNESS_FILE, 255);

	return 0;
}


int lcd_init(void)
{
	int r;
	struct ftm_module *mod;
	lcd_module *lcd;

	write_int(BRIGHTNESS_FILE, 255);

	mod = ftm_alloc(ITEM_LCD, sizeof(lcd_module));
	if (!mod)
		return -ENOMEM;

	lcd = mod_to_lcd(mod);

	lcd->module = mod;
	lcd->itm_view = ui_new_itemview();
	if (!lcd->itm_view)
		return -ENOMEM;

	r = ftm_register(mod, lcd_entry, (void*)lcd);
	if (r) {
		LOGD(TAG "register LCD failed (%d)\n", r);
		return r;
	}

	return 0;
}

#endif
