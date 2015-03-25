/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <linux/ioctl.h>
#include <linux/fb.h>
#include "meta_lcd.h"

// ---------------------------------------------------------------------------
//
// FIXME: should include mtkfb.h instead
//
#define MTK_IOW(num, dtype)         _IOW('O', num, dtype)
#define MTKFB_META_RESTORE_SCREEN   MTK_IOW(101, unsigned long)

// ---------------------------------------------------------------------------

static int _LCDFactoryModeTest(unsigned long testDuration);

// ---------------------------------------------------------------------------

static struct fb_var_screeninfo vinfo;
static int fbfd = -1;
static int fbsize = 0;
static int vfbsize = 0;
static unsigned char *fbbuf = NULL;

// ---------------------------------------------------------------------------

bool Meta_LCDFt_Init(void)
{
    /* Open video memory */
    if ((fbfd = open("/dev/graphics/fb0", O_RDWR)) < 0) {
        return false;
    }

    /* Get variable display parameters */
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        return false;
    }

    fbsize = vinfo.xres_virtual* vinfo.yres * (vinfo.bits_per_pixel / 8);
    vfbsize = vinfo.xres_virtual * vinfo.yres_virtual * (vinfo.bits_per_pixel / 8);

    /* Map video memory */
    if ((fbbuf = mmap(0, vfbsize, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fbfd, 0)) == (void *)-1) {
        return false;
    }

    return true;
}

bool Meta_LCDFt_Deinit(void)
{
    bool ret = true;
    
    if (fbbuf) {
        if (munmap(fbbuf, vfbsize)) {
            ret = false;
        }
        fbbuf = NULL;
    }
    if (fbfd >= 0) {
        if (close(fbfd) == -1) {
            ret = false;
        }
        fbfd = -1;
        fbsize = vfbsize = 0;
    }

    return ret;
}

LCDFt_CNF Meta_LCDFt_OP(LCDFt_REQ req)
{
    LCDFt_CNF rtn;
    rtn.status = true;

    if (_LCDFactoryModeTest(req.time_duration) == -1) {
        rtn.status = false;
    }

    return rtn;
}

// ---------------------------------------------------------------------------

#define ARGB8888_TO_RGB565(x)           \
    ((((x >>  0) & 0xFF) >> 3 <<  0) |  \
     (((x >>  8) & 0xFF) >> 2 <<  5) |  \
     (((x >> 16) & 0xFF) >> 3 << 11))

static void _FillColor(unsigned char *buf,
                       unsigned int width,
                       unsigned int height,
                       unsigned int bits_per_pixel,
                       unsigned int ARGB8888)
{
    unsigned int bytes_per_pixel = bits_per_pixel / 8;
    int i = width * height;

    if (16 == bits_per_pixel)
    {
        unsigned short RGB565 = ARGB8888_TO_RGB565(ARGB8888);
        while (--i >= 0)
        {
            *(unsigned short *)buf = RGB565;
            buf += bytes_per_pixel;
        }
    }
    else if (32 == bits_per_pixel)
    {
        while (--i >= 0)
        {
            *(unsigned int *)buf = ARGB8888;
            buf += bytes_per_pixel;
        }
    }
    else
    {
        assert(0);
    }
}


static void _FillTestPattern(unsigned char *buf,
                             unsigned int width,
                             unsigned int height,
                             unsigned int bits_per_pixel)
{
    unsigned int RED   = (16 == bits_per_pixel) ? 0xF800 : 0xFF0000;
    unsigned int GREEN = (16 == bits_per_pixel) ? 0x07E0 : 0x00FF00;
    unsigned int BLUE  = (16 == bits_per_pixel) ? 0x001F : 0x0000FF;

    unsigned int RED_SHIFT   = (16 == bits_per_pixel) ? 11 : 16;
    unsigned int GREEN_SHIFT = (16 == bits_per_pixel) ?  6 :  8;
    unsigned int BLUE_SHIFT  = 0;

    unsigned int GRADIENTCOLOR_BEGIN_LINE = height / 5;

    unsigned int pixelColor;
    unsigned int pixelColor_column;
    unsigned int bitShift;
    unsigned int x, y;

    assert(16 == bits_per_pixel || 32 == bits_per_pixel);

    unsigned int bytes_per_pixel = bits_per_pixel / 8;

    // Draw a white bar on the Top

    _FillColor(buf, width, GRADIENTCOLOR_BEGIN_LINE, bits_per_pixel, 0xFFFFFFFF);

    buf += width * bytes_per_pixel * GRADIENTCOLOR_BEGIN_LINE;
    height -= GRADIENTCOLOR_BEGIN_LINE;

    // Draw 3 gradient vertical color bar
    
    for (x = 0; x < width; ++ x) {
        bool resetColor = false;
        
        unsigned short *usPtr = (unsigned short *)buf;
        unsigned int   *uiPtr = (unsigned int *)buf;

        if (x < width / 3) {
            pixelColor = RED;
            pixelColor_column = RED;
            bitShift = RED_SHIFT;
        }
        else if (x < width * 2 / 3) {
            pixelColor = GREEN;
            pixelColor_column = GREEN;
            bitShift = GREEN_SHIFT;
        }
        else {
            pixelColor = BLUE;
            pixelColor_column = BLUE;
            bitShift = BLUE_SHIFT;
        }

        for (y = 0; y < height; ++ y) {
            if (y > 0) {
                if (resetColor) {
                    pixelColor = pixelColor_column;
                    resetColor = false;
                }
                pixelColor -= (1 << bitShift);
                if ((pixelColor >> bitShift) == 0) {
                    resetColor = true;
                }
            }
            if (16 == bits_per_pixel) {
                usPtr[x + width * y] = (unsigned short)pixelColor;
            } else if (32 == bits_per_pixel) {
                uiPtr[x + width * y] = 0xFF000000 | pixelColor; // add opaque alpha
            } else {
                assert(0);
            }
        }
    }
}

#if 0
static unsigned char *backupScreenBuffer = NULL;

static int _BackupScreen()
{
    if (NULL == backupScreenBuffer)
    {
        backupScreenBuffer = malloc(fbsize);

        if (NULL == backupScreenBuffer)
        {
            return -1;
        }
    }

    assert(vinfo.yoffset + vinfo.yres <= vinfo.yres_virtual);

    memcpy(backupScreenBuffer, 
           fbbuf + vinfo.yoffset * vinfo.xres * (vinfo.bits_per_pixel / 8),
           fbsize);

    return 0;
}


static int _RestoreScreen()
{
    int ret = 0;
    
    if (NULL == backupScreenBuffer) return -1;

    memcpy(fbbuf, backupScreenBuffer, fbsize);

    vinfo.activate = FB_ACTIVATE_VBL;
    vinfo.yoffset = 0;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        ret = -1;
        goto End;
    }

End:
    free(backupScreenBuffer);
    backupScreenBuffer = NULL;

    return ret;
}
#else
static int _BackupScreen()
{
    return 0;
}

static int _RestoreScreen()
{
    vinfo.yoffset = 0;

    if (ioctl(fbfd, MTKFB_META_RESTORE_SCREEN, &vinfo) == -1) {
        return -1;
    }
    return 0;
}
#endif


static int _LCDFactoryModeTest(unsigned long testDuration) // ms
{
    if (fbfd == -1 || fbbuf == NULL || fbsize == 0) {
        return -1;
    }

    _BackupScreen();

    /* Fill test pattern */
    _FillTestPattern(fbbuf, vinfo.xres_virtual, vinfo.yres, vinfo.bits_per_pixel);

    /* Flip to frame buffer 0 */
    vinfo.activate = FB_ACTIVATE_VBL;
    vinfo.yoffset = 0;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        return -1;
    }

    /* Sleep for a moment */
    usleep(testDuration * 1000);

    /* Clear the screen as Gray */
    _FillColor(fbbuf + fbsize,
               vinfo.xres_virtual, vinfo.yres,
               vinfo.bits_per_pixel, 0xFF808080);

    /* Flip to frame buffer 1 */
    vinfo.activate = FB_ACTIVATE_VBL;
    vinfo.yoffset = vinfo.yres;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        return -1;
    }

    /* Sleep for a moment */
    usleep(testDuration * 1000);

    _RestoreScreen();

    return 0;
}

// ---------------------------------------------------------------------------

