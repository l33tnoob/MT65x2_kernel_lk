/*
 * cfb_console.c
 *
 * Color-Framebuffer-Console-driver-for-8/15/16/24/32-bits-per-pixel.
 *
 * At-the-moment-only-the-8x16-font-is-tested-and-the-font-fore- and
 * background-color-is-limited-to-black/white/gray-colors. The-Linux
 * logo-can-be-placed-in-the-upper-left-corner-and-additional-board
 * information-strings-(that-normaly-goes-to-serial-port)-can-be-drawed.
 *
 * The-console-driver-can-use-the-standard-PC-keyboard-interface-(i8042)
 * for-character-input.-Character-output-goes-to-a-memory-mapped-video
 * framebuffer-with-little-or-big-endian-organisation.
 * With-environment-setting-'console=serial'-the-console-i/o-can-be
 * forced-to-serial-port.

 The-driver-uses-graphic-specific-defines/parameters/functions:

 (for-SMI-LynxE-graphic-chip)

 VIDEO_HW_RECTFILL	 - graphic driver supports hardware rectangle fill
 VIDEO_HW_BITBLT	 - graphic driver supports hardware bit blt
 CONFIG_VIDEO_SMI_LYNXEM - use graphic driver for SMI 710,712,810
 VIDEO_FB_LITTLE_ENDIAN	 - framebuffer organisation default: big endian


 Console Parameters are set by graphic drivers global struct:
 VIDEO_VISIBLE_ROWS	     - y resolution
 VIDEO_VISIBLE_COLS	     - x resolution
 VIDEO_DATA_FORMAT		  - graphical data format GDF
 VIDEO_PIXEL_SIZE	     - storage size in byte per pixel
 VIDEO_FB_ADRS		     - start of video memory

 VIDEO_TSTC_FCT		     - keyboard_tstc function
 VIDEO_GETC_FCT		     - keyboard_getc function
 CONFIG_I8042_KBD	     - AT Keyboard driver for i8042
 VIDEO_KBD_INIT_FCT	     - init function for keyboard


 CONFIG_CONSOLE_CURSOR	     - on/off-drawing-cursor-is-done-with-delay
			                   loop-in-VIDEO_TSTC_FCT (i8042)
 CONFIG_SYS_CONSOLE_BLINK_COUNT     - value-for-delay-loop - blink rate
 CONFIG_CONSOLE_TIME	     - display-time/date-in-upper-right-corner,
			       needs-CONFIG_CMD_DATE-and-CONFIG_CONSOLE_CURSOR
 CONFIG_VIDEO_BMP_LOGO	     - use-bmp_logo-instead-of-linux_logo
 CONFIG_VIDEO_LOGO	     - display-Linux-Logo-in-upper-left-corner
 CONFIG_CONSOLE_EXTRA_INFO   - display-additional-board-information-strings
			                   -that-normaly-goes-to-serial-port.-
			       This-define-requires-a-board-specific-function:
			       video_lk_drawstring (VIDEO_INFO_X,
						 VIDEO_INFO_Y + i*VIDEO_FONT_HEIGHT,
						 info);
			       that-fills-a-info-buffer-at i=row.
			       s.a:-board/-eltec/-bab7xx.

CONFIG_VIDEO_SW_CURSOR:	     - Draws-a-cursor-after-the-last-character.-No
			       blinking-is-provided. Uses-the-macros-CURSOR_SET-
			       and-CURSOR_OFF.
CONFIG_VIDEO_HW_CURSOR:	     - Uses-the-hardware-cursor-capability-of-the
			       graphic-chip. Uses-the-macro-CURSOR_SET.
			       ATTENTION: If-booting-an-OS, the-display-driver
			       must-disable-the-hardware-register-of-the-graphic
			       chip. Otherwise-a-blinking-field-is-displayed
CONFIG_VGA_AS_SINGLE_DEVICE  - If-set-the-framebuffer-device-will-be-initialised
			       as-an-output-only-device. The-Keyboard-driver
			       will-not-be-set-up. This-may-be-used, if-you
			       have-none-or-more-than-one-Keyboard-devices
			       (USB-Keyboard, -AT-Keyboard).
*/

#include <string.h>
#include <printf.h>
/*****************************************************************************/
/* COLOR Format				                        						 */
/*****************************************************************************/
#define FG_COL_5B         (CONSOLE_FG_COL >> 3)
#define FG_COL_6B         (CONSOLE_FG_COL >> 2)
#define BG_COL_5B         (CONSOLE_BG_COL >> 3)
#define BG_COL_6B         (CONSOLE_BG_COL >> 2)


/*****************************************************************************/
/* Defines-for-the-CT69000-driver                   					     */
/*****************************************************************************/
#ifdef	CONFIG_VIDEO_CT69000
#define NEED_BITBLT
#define NEED_LITTLE
#define NEED_RECTFILL
#ifdef NEED_LITTLE
#define VIDEO_FB_LITTLE_ENDIAN
#endif
#ifdef NEED_RECTFILL
#define VIDEO_HW_RECTFILL
#endif
#ifdef NEED_BITBLT
#define VIDEO_HW_BITBLT
#endif
#endif

#define DRAW_TABLE_0 0x00000000
#define DRAW_TABLE_1 0x000000ff
#define DRAW_TABLE_3 0x0000ffff
#define DRAW_TABLE_7 0x00ffffff
#define DRAW_TABLE_8 0xff000000
#define DRAW_TABLE_C 0xffff0000
#define DRAW_TABLE_E 0xffffff00
#define DRAW_TABLE_F 0xffffffff

/* Disrupted defined sequence */
static const int lk_video_font_draw_table32[16][4] = {
	     {DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_0},
	     {DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_7},
	     {DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_0},
	     {DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_7},
	     {DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_0},
	     {DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_7},
	     {DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_0},
	     {DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_7},
	     {DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_0},
	     {DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_7},
	     {DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_0},
	     {DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_7, DRAW_TABLE_7},
	     {DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_0},
	     {DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_0, DRAW_TABLE_7},
	     {DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_0},
	     {DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_7, DRAW_TABLE_7}};


/*****************************************************************************/
/* Console-device-defines-with-SMI-graphic			                	     */
/* Any-other-graphic-must-change-this-section	            			     */
/*****************************************************************************/
#ifdef	CONFIG_VIDEO_SMI_LYNXEM
#define NEED_BITBLT
#define NEED_LITTLE
#define NEED_RECTFILL
#ifdef NEED_LITTLE
#define VIDEO_FB_LITTLE_ENDIAN
#endif
#ifdef NEED_BITBLT
#define VIDEO_HW_BITBLT
#endif
#ifdef NEED_RECTFILL
#define VIDEO_HW_RECTFILL
#endif
#endif


/*****************************************************************************/
/* Defines-for-the-SED13806-driver					                         */
/*****************************************************************************/
#if defined(CONFIG_VIDEO_SM501) && defined(CONFIG_HH405)
#define VIDEO_FB_LITTLE_ENDIAN
#endif

/*****************************************************************************/
/* Defines-for-the-SED13806-driver                   					     */
/*****************************************************************************/
#ifdef CONFIG_VIDEO_SED13806
#define VIDEO_HW_BITBLT
#endif

#ifdef CONFIG_VIDEO_SED13806
#define VIDEO_HW_RECTFILL
#endif

#if defined(CONFIG_VIDEO_SED13806) && !defined(CONFIG_TOTAL5200)
#define VIDEO_FB_LITTLE_ENDIAN
#endif


/*****************************************************************************/
/* Defines-for-the-MB862xx-driver                   					     */
/*****************************************************************************/
#if defined(CONFIG_VIDEO_MB862xx) && defined(CONFIG_VIDEO_MB862xx_ACCEL)
#define VIDEO_HW_BITBLT
#endif

#if defined(CONFIG_VIDEO_MB862xx) && defined(CONFIG_VIDEO_MB862xx_ACCEL)
#define VIDEO_HW_RECTFILL
#endif

#if defined(CONFIG_VIDEO_MB862xx) && defined(CONFIG_VIDEO_CORALP)
#define VIDEO_FB_LITTLE_ENDIAN
#endif

/*****************************************************************************/
/* Include-video_fb.h-after-definitions-of-VIDEO_HW_RECTFILL-etc	         */
/*****************************************************************************/
#include <video_font.h>
#include <video_fb.h>
#include <target/board.h>
#include <stdarg.h>

/*****************************************************************************/
/* some-Macros					                            			     */
/*****************************************************************************/
#define WINSIZE_X           (pGD->winSizeX)
#define WINSIZE_Y           (pGD->winSizeY)
#define VIDEO_VISIBLE_ROWS	(WINSIZE_Y)
#define VIDEO_VISIBLE_COLS	(WINSIZE_X)
#define DATA_FORMAT         (pGD->gdfIndex)
#define VIDEO_DATA_FORMAT	(DATA_FORMAT)
#define PIXEL_SIZE          (pGD->gdfBytesPP)
#define VIDEO_PIXEL_SIZE	(PIXEL_SIZE)
#define VIDEO_FB_SIZE       (pGD->memSize)
#define FB_ADDRESS          (pGD->frameAdrs)
#define VIDEO_FB_ADRS		(FB_ADDRESS)

/* Disrupted defined sequence */
static const int lk_video_font_draw_table24[16][3] = {
	    {DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_0},
	    {DRAW_TABLE_0, DRAW_TABLE_0, DRAW_TABLE_7},
	    {DRAW_TABLE_0, DRAW_TABLE_3, DRAW_TABLE_8},
	    {DRAW_TABLE_0, DRAW_TABLE_3, DRAW_TABLE_F},
	    {DRAW_TABLE_1, DRAW_TABLE_C, DRAW_TABLE_0},
	    {DRAW_TABLE_1, DRAW_TABLE_C, DRAW_TABLE_7},
	    {DRAW_TABLE_1, DRAW_TABLE_F, DRAW_TABLE_8},
	    {DRAW_TABLE_1, DRAW_TABLE_F, DRAW_TABLE_F},
	    {DRAW_TABLE_E, DRAW_TABLE_0, DRAW_TABLE_0},
	    {DRAW_TABLE_E, DRAW_TABLE_0, DRAW_TABLE_7},
	    {DRAW_TABLE_E, DRAW_TABLE_3, DRAW_TABLE_8},
	    {DRAW_TABLE_E, DRAW_TABLE_3, DRAW_TABLE_F},
	    {DRAW_TABLE_F, DRAW_TABLE_C, DRAW_TABLE_0},
	    {DRAW_TABLE_F, DRAW_TABLE_C, DRAW_TABLE_7},
	    {DRAW_TABLE_F, DRAW_TABLE_F, DRAW_TABLE_8},
	    {DRAW_TABLE_F, DRAW_TABLE_F, DRAW_TABLE_F}};
	    

#if defined(DEBUG) || defined(DEBUG_CFB_CONSOLE)
#define PRINTD(x)	  printf(x)
#else
#define PRINTD(x)
#endif

/*****************************************************************************/
/* Console-device-defines-with-i8042-keyboard-controller        		     */
/* Any-other-keyboard-controller-must-change-this-section		             */
/*****************************************************************************/
#include <platform/mt_disp_drv.h>
/*****************************************************************************/
/* Cursor-definition:						                        	     */
/* CONFIG_VIDEO_HW_CURSOR: Uses the hardware cursor capability of the	     */
/*			   graphic chip. Uses-the macro CURSOR_SET.	                     */
/*			   ATTENTION: If booting an-OS, the display driver               */
/*			   must disable the hardware register of the graphic             */
/*			   chip. Otherwise a blinking field is displayed                 */
/* CONFIG_CONSOLE_CURSOR:  Uses-a timer function (see drivers/input/i8042.c) */
/*                         to let the-cursor blink. Uses the macros	         */
/*                         CURSOR_OFF and CURSOR_ON.			             */
/* CONFIG_VIDEO_SW_CURSOR: Draws a cursor-after the last character. No	     */
/*			   blinking is provided. Uses the macros CURSOR_SET              */
/*			   and CURSOR_OFF.				                                 */
/*****************************************************************************/
#if !defined(CONFIG_VIDEO_SW_CURSOR) && \
    !defined(CONFIG_CONSOLE_CURSOR) && \
    !defined(CONFIG_VIDEO_HW_CURSOR)
/* no-Cursor-defined */
#define CURSOR_OFF
#define CURSOR_ON
#define CURSOR_SET
#endif

#ifdef	CONFIG_CONSOLE_CURSOR
#ifdef	CURSOR_ON
#error	information only one of CONFIG_CONSOLE_CURSOR,CONFIG_VIDEO_SW_CURSOR,CONFIG_VIDEO_HW_CURSOR can be defined
#endif
void	console_cursor (int state);
#define CURSOR_ON  console_cursor(1);
#define CURSOR_SET
#define CURSOR_OFF console_cursor(0);

#ifndef CONFIG_I8042_KBD
#warning information Cursor drawing on/off needs timer function s.a. drivers/input/i8042.c
#endif
#else
#ifdef	CONFIG_CONSOLE_TIME
#error	information CONFIG_CONSOLE_CURSOR must be defined for CONFIG_CONSOLE_TIME
#endif
#endif /*-CONFIG_CONSOLE_CURSOR- */

#ifdef CONFIG_VIDEO_HW_CURSOR
#ifdef	CURSOR_ON
#error	information only one of CONFIG_CONSOLE_CURSOR,CONFIG_VIDEO_SW_CURSOR,CONFIG_VIDEO_HW_CURSOR can be defined
#endif
#define CURSOR_ON
#define CURSOR_OFF
#define CURSOR_SET video_set_hw_cursor(console_col * VIDEO_FONT_WIDTH, \
		  (console_row * VIDEO_FONT_HEIGHT) + video_logo_height);
#endif	/* -CONFIG_VIDEO_HW_CURSOR- */

#ifdef	CONFIG_VIDEO_SW_CURSOR
#ifdef	CURSOR_ON
#error	information only one of CONFIG_CONSOLE_CURSOR,CONFIG_VIDEO_SW_CURSOR,CONFIG_VIDEO_HW_CURSOR can be defined
#endif
#define CURSOR_ON
#define CURSOR_SET video_set_cursor();
#define CURSOR_OFF video_lk_putchar(console_col * VIDEO_FONT_WIDTH,\
				                    console_row * VIDEO_FONT_HEIGHT, ' ');

#endif /*-CONFIG_VIDEO_SW_CURSOR- */


#ifdef	CONFIG_VIDEO_LOGO
#define VIDEO_INFO_Y		(VIDEO_FONT_HEIGHT/2)
#define VIDEO_INFO_X		(VIDEO_LOGO_WIDTH)
#else	/* -CONFIG_VIDEO_LOGO- */
#define VIDEO_LOGO_HEIGHT	0
#define VIDEO_LOGO_WIDTH	0
#endif	/* -CONFIG_VIDEO_LOGO- */


#define VIDEO_COLS		    WINSIZE_X
#define VIDEO_BURST_LEN		(VIDEO_COLS/8)
#define VIDEO_ROWS		    WINSIZE_Y
#define VIDEO_SIZE		    (VIDEO_PIXEL_SIZE*VIDEO_COLS*VIDEO_ROWS)
#define VIDEO_PIX_BLOCKS	(VIDEO_SIZE >> 2)
#define VIDEO_LINE_LEN		(VIDEO_PIXEL_SIZE*VIDEO_COLS)

#ifdef	CONFIG_VIDEO_LOGO
#define FONT_HEIGHT         (VIDEO_FONT_HEIGHT)
#define CONSOLE_ROWS		((WINSIZE_Y - video_logo_height) / FONT_HEIGHT)
#else
#define FONT_HEIGHT         (VIDEO_FONT_HEIGHT)
#define CONSOLE_ROWS		(WINSIZE_Y / FONT_HEIGHT)
#endif

#define FONT_WIDTH          (VIDEO_FONT_WIDTH)
#define CONSOLE_COLS		(WINSIZE_X / FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(FONT_HEIGHT * VIDEO_LINE_LEN)

#define CONSOLE_ADDR        (video_console_address)
#define CONSOLE_ROW_FIRST	(CONSOLE_ADDR)
#define CONSOLE_ROW_SECOND	(CONSOLE_ADDR + CONSOLE_ROW_SIZE)
#define CONSOLE_ROW_LAST	(CONSOLE_ADDR - CONSOLE_ROW_SIZE + CONSOLE_SIZE)
#define CONSOLE_SIZE		(CONSOLE_ROWS * CONSOLE_ROW_SIZE)
#define CONSOLE_SCROLL_SIZE	(CONSOLE_SIZE - CONSOLE_ROW_SIZE)


/* -Macros- */
#ifdef	VIDEO_FB_LITTLE_ENDIAN
#define SWAP16(x)	 ((((x) & 0x00ff) << 8) | ( (x) >> 8))
#else
#define SWAP16(x)	 (x)
#endif

#ifdef	VIDEO_FB_LITTLE_ENDIAN
#define SHORTSWAP32(x)	 ((((x) & 0x000000ff) <<  8) | (((x) & 0x0000ff00) >> 8)|\
			  (((x) & 0x00ff0000) <<  8) | (((x) & 0xff000000) >> 8) )
#else
#if defined(VIDEO_FB_16BPP_WORD_SWAP)
#define SHORTSWAP32(x)	 ( ((x) >> 16) | ((x) << 16) )
#else
#define SHORTSWAP32(x)	 (x)
#endif
#endif

#ifdef VIDEO_FB_LITTLE_ENDIAN
#define SWAP32(x)	 ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8)|\
			  (((x) & 0x00ff0000) >>  8) | (((x) & 0xff000000) >> 24) )
#else
#define SWAP32(x)	 (x)
#endif


#ifdef CONFIG_CONSOLE_EXTRA_INFO
extern void video_get_info_str (/* setup-a-board-string:-type,-speed,-etc. */
    int line_number,	        /* location-to-place-info-string-beside-logo */
    char *info		            /* buffer-for-info-string */
    );

#endif

/* -Locals- */
static GraphicDevice *pGD;	/* Pointer-to-Graphic-array */

static void *video_console_address;	/* console-buffer-start-address */
static void *video_fb_address;		/* frame-buffer-address */

static int console_row = 0; /* cursor-row */
static int console_col = 0; /* cursor-col */

/* color pats */
static u32 eorx;
static u32 fgx;
static u32 bgx;

static int video_logo_height = VIDEO_LOGO_HEIGHT;


static const int lk_video_font_draw_table16[] = {
#if 0
	    0x00000000, 0x0000ffff, 0xffff0000, 0xffffffff };
#else   // Jett Patch: RGB565 with Little Endian Table
        0x00000000, 0xffff0000, 0x0000ffff, 0xffffffff };
#endif

static const int lk_video_font_draw_table15[] = {
	    0x00000000, 0x00007fff, 0x7fff0000, 0x7fff7fff };


static const int lk_video_font_draw_table8[] = {
	    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
	    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
	    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
	    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff };

/******************************************************************************/

static int get_console_col(void)
{
    return console_col;
}

static void set_console_col(int value)
{
    console_col = value;
}

static void increase_console_col(int value)
{
    console_col += value;
}

static int get_console_row(void)
{
    return console_row;
}

static void set_console_row(int value)
{
    console_row = value;
}

static void increase_console_row(int value)
{
    console_row += value;
}

/******************************************************************************/

static void video_lk_drawchars (int xx, int yy, unsigned char *s, int count)
{
	u8 *pc         = NULL;
	u8 *tempdest   = NULL;
	u8 *pdest      = NULL;
	int rownum     = 0;
	int offset     = 0;
	unsigned int data_format = 0;

	pdest = video_fb_address + yy * VIDEO_LINE_LEN + xx * VIDEO_PIXEL_SIZE;
	data_format = VIDEO_DATA_FORMAT;

	switch (data_format) {
	case GDF_15BIT_555RGB:
		while (count--) {
			offset = (*s++) * VIDEO_FONT_HEIGHT;
			pc = video_fontdata + offset;
			for (rownum = VIDEO_FONT_HEIGHT, tempdest = pdest;
			     rownum--;
			     tempdest += VIDEO_LINE_LEN) {
				u8 bits = *pc++;

				((u32 *) tempdest)[0] = SHORTSWAP32 ((lk_video_font_draw_table15 [bits >> 6] & eorx) ^ bgx);
				((u32 *) tempdest)[1] = SHORTSWAP32 ((lk_video_font_draw_table15 [bits >> 4 & 3] & eorx) ^ bgx);
				((u32 *) tempdest)[2] = SHORTSWAP32 ((lk_video_font_draw_table15 [bits >> 2 & 3] & eorx) ^ bgx);
				((u32 *) tempdest)[3] = SHORTSWAP32 ((lk_video_font_draw_table15 [bits & 3] & eorx) ^ bgx);
			}
			pdest = pdest + VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
		}
		break;

	case GDF__8BIT_332RGB:
	case GDF__8BIT_INDEX:
		while (count--) {
		    offset = (*s++) * VIDEO_FONT_HEIGHT;
			pc = video_fontdata + offset;
			for (rownum = VIDEO_FONT_HEIGHT, tempdest = pdest;
			     rownum--;
			     tempdest += VIDEO_LINE_LEN) {
				u8 bits = *pc++;

				((u32 *) tempdest)[0] = (lk_video_font_draw_table8[bits >> 4] & eorx) ^ bgx;
				((u32 *) tempdest)[1] = (lk_video_font_draw_table8[bits & 15] & eorx) ^ bgx;
			}
			pdest = pdest + VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
		}
		break;

	case GDF_16BIT_565RGB:
		while (count--) {
		    offset = (*s++) * VIDEO_FONT_HEIGHT;
			pc = video_fontdata + offset;
			for (rownum = VIDEO_FONT_HEIGHT, tempdest = pdest;
			     rownum--;
			     tempdest += VIDEO_LINE_LEN) {
				u8 bits = *pc++;

				((u32 *) tempdest)[0] = SHORTSWAP32 ((lk_video_font_draw_table16 [bits >> 6] & eorx) ^ bgx);
				((u32 *) tempdest)[1] = SHORTSWAP32 ((lk_video_font_draw_table16 [bits >> 4 & 3] & eorx) ^ bgx);
				((u32 *) tempdest)[2] = SHORTSWAP32 ((lk_video_font_draw_table16 [bits >> 2 & 3] & eorx) ^ bgx);
				((u32 *) tempdest)[3] = SHORTSWAP32 ((lk_video_font_draw_table16 [bits & 3] & eorx) ^ bgx);
			}
			pdest = pdest + VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
		}
		break;

	case GDF_24BIT_888RGB:
		while (count--) {
		    offset = (*s++) * VIDEO_FONT_HEIGHT;
			pc = video_fontdata + offset;
			for (rownum = VIDEO_FONT_HEIGHT, tempdest = pdest;
			     rownum--;
			     tempdest += VIDEO_LINE_LEN) {
				u8 bits = *pc++;

				((u32 *) tempdest)[0] = (lk_video_font_draw_table24[bits >> 4][0] & eorx) ^ bgx;
				((u32 *) tempdest)[1] = (lk_video_font_draw_table24[bits >> 4][1] & eorx) ^ bgx;
				((u32 *) tempdest)[2] = (lk_video_font_draw_table24[bits >> 4][2] & eorx) ^ bgx;
				((u32 *) tempdest)[3] = (lk_video_font_draw_table24[bits & 15][0] & eorx) ^ bgx;
				((u32 *) tempdest)[4] = (lk_video_font_draw_table24[bits & 15][1] & eorx) ^ bgx;
				((u32 *) tempdest)[5] = (lk_video_font_draw_table24[bits & 15][2] & eorx) ^ bgx;
			}
			pdest = pdest + VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
		}
		break;

	case GDF_32BIT_X888RGB:
		while (count--) {
		    offset = (*s++) * VIDEO_FONT_HEIGHT;
			pc = video_fontdata + offset;
			for (rownum = VIDEO_FONT_HEIGHT, tempdest = pdest;
			     rownum--;
			     tempdest += VIDEO_LINE_LEN) {
				u8 bits = *pc++;

				((u32 *) tempdest)[0] = SWAP32 ((lk_video_font_draw_table32 [bits >> 4][0] & eorx) ^ bgx);
				((u32 *) tempdest)[1] = SWAP32 ((lk_video_font_draw_table32 [bits >> 4][1] & eorx) ^ bgx);
				((u32 *) tempdest)[2] = SWAP32 ((lk_video_font_draw_table32 [bits >> 4][2] & eorx) ^ bgx);
				((u32 *) tempdest)[3] = SWAP32 ((lk_video_font_draw_table32 [bits >> 4][3] & eorx) ^ bgx);
				((u32 *) tempdest)[4] = SWAP32 ((lk_video_font_draw_table32 [bits & 15][0] & eorx) ^ bgx);
				((u32 *) tempdest)[5] = SWAP32 ((lk_video_font_draw_table32 [bits & 15][1] & eorx) ^ bgx);
				((u32 *) tempdest)[6] = SWAP32 ((lk_video_font_draw_table32 [bits & 15][2] & eorx) ^ bgx);
				((u32 *) tempdest)[7] = SWAP32 ((lk_video_font_draw_table32 [bits & 15][3] & eorx) ^ bgx);
			}
			pdest = pdest + VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
		}
		break;

	default:
	    break;
	}

}

/*****************************************************************************/

static inline void video_lk_drawstring (int this_xx, int this_yy, unsigned char *this_s)
{
    /* video_lk_drawstring */
	video_lk_drawchars (this_xx, this_yy, this_s, strlen ((char *)this_s));
}

/*****************************************************************************/

static void video_lk_putchar (int this_xx, int this_yy, unsigned char this_c)
{
    /* video_lk_putchar */
	video_lk_drawchars (this_xx, video_logo_height + this_yy, &this_c, 1);
}

/*****************************************************************************/
#if defined(CONFIG_CONSOLE_CURSOR) || defined(CONFIG_VIDEO_SW_CURSOR)
static void video_set_cursor (void)
{
	u32 backup_eorx = 0;
	u32 backup_fgx  = 0;
	u32 backup_bgx  = 0;

	/* back up eorx, fgx, bgx */
	backup_eorx = eorx;
	backup_fgx  = fgx;
	backup_bgx  = bgx;

	/* -swap-drawing-colors- */
	fgx  = backup_bgx;
	bgx  = backup_fgx;
	eorx = fgx ^ bgx;
	/* -draw-cursor- */
	video_lk_putchar (get_console_col() * VIDEO_FONT_WIDTH,
		              get_console_row() * VIDEO_FONT_HEIGHT,
		              ' ');

	/* -restore-drawing-colors- */
	fgx  = backup_fgx;
	bgx  = backup_bgx;
	eorx = backup_eorx;

}
#endif
/*****************************************************************************/
#ifdef CONFIG_CONSOLE_CURSOR
void console_cursor (int state)
{
    /* console_cursor */
	static int the_last_state = 0;
	int para_state = state;
	struct rtc_time this_tm;
	char this_info[16];

	/* -time-update-only-if-cursor-is-on-(faster-scroll)- */
	if (para_state) {
#ifdef CONFIG_CONSOLE_TIME
		rtc_get (&this_tm);

		sprintf (this_info, " %02d:%02d:%02d ", this_tm.tm_hour, this_tm.tm_min,
		         this_tm.tm_sec);
		video_lk_drawstring (VIDEO_VISIBLE_COLS - 10 * VIDEO_FONT_WIDTH,
				             VIDEO_INFO_Y,
				             (uchar *)this_info);

		sprintf (this_info, "%02d.%02d.%04d", this_tm.tm_mday, this_tm.tm_mon,
			     this_tm.tm_year);
		video_lk_drawstring (VIDEO_VISIBLE_COLS - 10 * VIDEO_FONT_WIDTH,
				             VIDEO_INFO_Y + 1 * VIDEO_FONT_HEIGHT,
				             (uchar *)this_info);
#endif
	}


	if (para_state && (the_last_state != para_state)) {
		video_set_cursor ();
	}

	if (!para_state && (the_last_state != para_state)) {
		/* -clear-cursor- */
		video_lk_putchar (console_col * VIDEO_FONT_WIDTH,
			       console_row * VIDEO_FONT_HEIGHT,
			       ' ');
	}

	the_last_state = para_state;
}
#endif

/*****************************************************************************/

#ifndef VIDEO_HW_RECTFILL
static void memsetl (int *lk_p, int lk_c, int lk_v)
{
	while (lk_c--)
	{
		*lk_p = lk_v;
		++lk_p;
    }
}
#endif

/*****************************************************************************/

#ifndef VIDEO_HW_BITBLT
static void memcpyl (int *lk_d, int *lk_s, int lk_c)
{
	while (lk_c--)
	{
	    *lk_d = *lk_s;
	    ++lk_d;
	    ++lk_s;
	}
}
#endif

/*****************************************************************************/

static void console_lk_scrollup (void)
{
    /* var for hw bitblt */
#ifdef VIDEO_HW_BITBLT
    unsigned int bpp     = VIDEO_PIXEL_SIZE;
    unsigned int src_x   = 0;
    unsigned int src_y   = video_logo_height + VIDEO_FONT_HEIGHT;
    unsigned int dst_x   = 0;
    unsigned int dst_y   = video_logo_height;
    unsigned int dim_x   = VIDEO_VISIBLE_COLS;
    unsigned int dim_y   = VIDEO_VISIBLE_ROWS - video_logo_height - VIDEO_FONT_HEIGHT;
#endif

    /* var for hw rectfill */
#ifdef VIDEO_HW_RECTFILL
    unsigned int r_bpp   = VIDEO_PIXEL_SIZE;
    unsigned int r_dst_x = 0;
    unsigned int r_dst_y = VIDEO_VISIBLE_ROWS - VIDEO_FONT_HEIGHT;
    unsigned int r_dim_x = VIDEO_VISIBLE_COLS;
    unsigned int r_dim_y = VIDEO_FONT_HEIGHT;
    unsigned int r_color = CONSOLE_BG_COL;
#endif

	/* *copy* up *rows* ignoring *the* first *one */

#ifdef VIDEO_HW_BITBLT
	video_hw_bitblt (bpp,	/* bytes per pixel */
			         src_x,	/* source pos x */
			         src_y, /* source pos y */
			         dst_x,	/* dest pos x */
			         dst_y,	/* dest pos y */
			         dim_x,	/* frame width */
			         dim_y	/* frame height */
		             );
#else
	memcpyl (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND,CONSOLE_SCROLL_SIZE >> 2);
#endif

	/* -clear-the-last-one- */
#ifdef VIDEO_HW_RECTFILL
	video_hw_rectfill (r_bpp,	/* bytes per pixel */
			           r_dst_x,	/* dest pos x */
			           r_dst_y,	/* dest pos y */
			           r_dim_x,	/* frame width */
			           r_dim_y,	/* frame height */
			           r_color	/* fill color */
		               );
#else
	memsetl (CONSOLE_ROW_LAST, CONSOLE_ROW_SIZE >> 2, CONSOLE_BG_COL);
#endif
}

/*****************************************************************************/

static void console_lk_back (void)
{
    /* console_lk_back */
	CURSOR_OFF

	increase_console_col(-1);

	if (get_console_col() < 0)
	{
		set_console_col(CONSOLE_COLS - 1);
		increase_console_row(-1);

		if (get_console_row() < 0)
		{
			set_console_row(0);
		}
	}


	video_lk_putchar (get_console_col() * VIDEO_FONT_WIDTH,
		              get_console_row() * VIDEO_FONT_HEIGHT,
		              ' ');
}

/*****************************************************************************/

static void console_newline (void)
{
     /* Check if last character in the line was just drawn. If so, cursor was
        overwriten and need not to be cleared. Cursor clearing without this
        check causes overwriting the 1st character of the line if line lenght
        is >= CONSOLE_COLS
      */

	if (get_console_col() < 0 || get_console_col() < (int)(CONSOLE_COLS))
	{
		CURSOR_OFF
	}

	increase_console_row(1);
	set_console_col(0);

	/* -Check-if-we-need-to-scroll-the-terminal- */
	if (get_console_row() >= (int)(CONSOLE_ROWS))
	{
		/* -Scroll-everything-up- */
		console_lk_scrollup ();

		/* -Decrement-row-number- */
		increase_console_row(-1);
	}
}

static void console_cr (void)
{
    /* console_cr */
	CURSOR_OFF
	set_console_col(0);
}

/*****************************************************************************/

void video_putc (const char c)
{
	static int nl = 1;

        // Jett: check newline here in order to
        //       scroll the screen immediately for the first time video_printf()
        //
        if (console_col >= (int)(CONSOLE_COLS))
          console_newline ();

	switch (c) {
	case 13:		/* back to first column */
		console_cr ();
		break;

	case '\n':		/* next line */
		if (console_col || (!console_col && nl))
			console_newline ();
		nl = 1;
		break;

	case 8:		/* -backspace- */
		console_lk_back ();
		break;

	case 9:		/* -tab-8- */
		CURSOR_OFF console_col |= 0x0008;
		console_col &= ~0x0007;

		if (get_console_col() >= (int)(CONSOLE_COLS))
		{
			console_newline ();
		}
		break;

	default:		/* -draw-the-char- */
		video_lk_putchar (get_console_col() * VIDEO_FONT_WIDTH,
			              get_console_row() * VIDEO_FONT_HEIGHT,
			              c);
		increase_console_col(1);

		/* -check-for-newline- */
		if (get_console_col() >= (int)(CONSOLE_COLS))
		{
			console_newline ();
			nl = 0;
		}
	}
CURSOR_SET}


/*****************************************************************************/

void video_puts (const char *s)
{
	int lk_count = strlen (s);

	while (lk_count--)
	{
		video_putc (*s);
		s++;
	}

	mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
}

/*****************************************************************************/

void video_printf (const char *fmt, ...)//copy from mtk u-boot-1.1.6
{
#ifdef CFB_CONSOLE_ON
	va_list args;
	//unsigned int i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	video_puts (printbuffer);
#else
  printf("%s\n", fmt);
#endif
}

/*****************************************************************************/

#if defined(CONFIG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)

#define FILL_32BIT_X888RGB(rr,gg,bb) {			\
	*(unsigned long *)fb = SWAP32((unsigned long)(((rr<<16) | (gg<<8) | bb))); \
	fb = fb + 4;					\
}

#define FILL_8BIT_332RGB(rr,gg,bb)	{			\
	*fb = ((rr>>5)<<5) | ((gg>>5)<<2) | (bb>>6);	\
	fb = fb + 1;						\
}

#define FILL_16BIT_565RGB(rr,gg,bb) {			\
	*(unsigned short *)fb = SWAP16((unsigned short)((((rr)>>3)<<11) | (((gg)>>2)<<5) | ((bb)>>3))); \
	fb = fb + 2;					\
}

#define FILL_15BIT_555RGB(rr,gg,bb) {			\
	*(unsigned short *)fb = SWAP16((unsigned short)(((rr>>3)<<10) | ((gg>>3)<<5) | (bb>>3))); \
	fb = fb + 2;					\
}



#ifdef VIDEO_FB_LITTLE_ENDIAN
#define FILL_24BIT_888RGB(rr,gg,bb) {			\
	fb[1] = gg;					\
	fb[0] = bb;					\
	fb[2] = rr;					\
	fb = fb + 3;					\
}
#else
#define FILL_24BIT_888RGB(rr,gg,bb) {			\
	fb[0] = rr;					\
	fb[2] = bb;					\
	fb[1] = gg;					\
	fb = fb + 3;					\
}
#endif
#endif


static int video_lk_init (void)
{
	unsigned char color8;
	unsigned int  data_format = 0;

	pGD = video_hw_init ();
	if (pGD == NULL)
	{
		return -1;
	}

	video_fb_address = (void *) VIDEO_FB_ADRS;
    data_format = VIDEO_DATA_FORMAT;
	/* -Init-drawing-pats */
	switch (data_format)
	{
	case GDF_15BIT_555RGB:
		fgx = ((FG_COL_5B << 26) | (FG_COL_5B << 21) | (FG_COL_5B << 16) | /* high 16 0555 */
		       (FG_COL_5B << 10) | (FG_COL_5B << 5)  |  FG_COL_5B); /* low 16 0555 */
		bgx = ((BG_COL_5B << 26) | (BG_COL_5B << 21) | (BG_COL_5B << 16) | /* high 16 0555 */
		       (BG_COL_5B << 10) | (BG_COL_5B << 5)  |  BG_COL_5B); /* low 16 0555 */
		break;
	case GDF_16BIT_565RGB:
		fgx = ((FG_COL_5B << 27) | (FG_COL_6B << 21) | (FG_COL_5B << 16) | /* high 16 565 */
		       (FG_COL_5B << 11) | (FG_COL_6B << 5)  |  FG_COL_5B); /* low 16 565 */
		bgx = ((BG_COL_5B << 27) | (BG_COL_6B << 21) | (BG_COL_5B << 16) | /* high 16 565 */
		       (BG_COL_5B << 11) | (BG_COL_6B << 5)  |  BG_COL_5B); /* low 16 565 */
		break;
	case GDF__8BIT_INDEX:
		video_set_lut (0x01, CONSOLE_FG_COL, CONSOLE_FG_COL, CONSOLE_FG_COL);
		video_set_lut (0x00, CONSOLE_BG_COL, CONSOLE_BG_COL, CONSOLE_BG_COL);
		fgx = 0x01010101;
		bgx = 0x00000000;
		break;
	case GDF__8BIT_332RGB:
		color8 = ((CONSOLE_FG_COL & 0xe0) |
			  ((CONSOLE_FG_COL >> 3) & 0x1c) | CONSOLE_FG_COL >> 6);
		fgx = (color8 << 24) | (color8 << 16) | (color8 << 8) | color8;
		color8 = ((CONSOLE_BG_COL & 0xe0) |
			  ((CONSOLE_BG_COL >> 3) & 0x1c) | CONSOLE_BG_COL >> 6);
		bgx = (color8 << 24) | (color8 << 16) | (color8 << 8) | color8;
		break;
	case GDF_24BIT_888RGB:
		fgx = (CONSOLE_FG_COL << 24) | (CONSOLE_FG_COL << 16) |
			(CONSOLE_FG_COL << 8) | CONSOLE_FG_COL;
		bgx = (CONSOLE_BG_COL << 24) | (CONSOLE_BG_COL << 16) |
			(CONSOLE_BG_COL << 8) | CONSOLE_BG_COL;
		break;
	case GDF_32BIT_X888RGB:
		fgx = (CONSOLE_FG_COL << 16) | (CONSOLE_FG_COL << 8) | CONSOLE_FG_COL;
		bgx = (CONSOLE_BG_COL << 16) | (CONSOLE_BG_COL << 8) | CONSOLE_BG_COL;
		break;
	default:
	    break;
	}
	eorx = fgx ^ bgx;

	video_console_address = video_fb_address;

	/* -Initialize-the-console- */
	set_console_col(0);
	set_console_row(0);

	return 0;
}


int __board_video_skip(void)
{
	/* As-default, don't-skip-test */
	return 0;
}
int board_video_skip(void) __attribute__((weak, alias("__board_video_skip")));

/*****************************************************************************/

/*
 * Implement-a-weak-default-function-for-boards-that-optionally-
 * need-to-skip-the-video-initialization.
 */
void video_set_cursor(int row, int col);
int drv_video_init (void)
{
	int flag = 0;
//	struct stdio_dev console_dev;

	/* Check-if-video-initialization-should-be-skipped */
	flag = board_video_skip();
	if (flag)
	{
		return 0;
	}

	/* Init-video-chip - returns-with-framebuffer-cleared */

    if (video_lk_init () == -1)
    {
        return 0;
    }

    // Jett: set cursor to the right-bottom corner
    //       scroll screen immediately for the first time video_printf()
    //
    video_set_cursor(CONSOLE_ROWS - 1, CONSOLE_COLS);
    return 0;

	//if (stdio_register (&console_dev) != 0)
	//	return 0;

	/* Return success */
	//return 1;
}

void video_clean_screen(void)
{
    memset((void*)VIDEO_FB_ADRS, 0, VIDEO_FB_SIZE);
}


void video_set_cursor(int row, int col)
{
    if (row >= 0 && row < (int)(CONSOLE_ROWS) &&
        col >= 0 && col <= (int)(CONSOLE_COLS))
    {
        console_row = row;
        console_col = col;
    }
}


int video_get_rows(void)
{
    return CONSOLE_ROWS;
}


int video_get_colums(void)
{
    return CONSOLE_COLS;
}
