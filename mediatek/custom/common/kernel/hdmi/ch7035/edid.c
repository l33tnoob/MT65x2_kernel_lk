// EDID utilities

// MTK-Add-Start
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/kernel.h>
// MTK-Add-End

#include "ch703xtype.h"
#include "ch703xerr.h"
#include "ch703xdriver.h"

#include "edid.h"

//#define  MSG_OUT		debug_print // MTK-Modify
//#define  DBG_OUT		debug_print // MTK-Modify
//#define  ERR_OUT		debug_print // MTK-Modify

/* These match the EDID encoding for Standard Timing block */
#define ASPECT_16_10 	0
#define ASPECT_4_3   	1
#define ASPECT_5_4   	2
#define ASPECT_16_9  	3
#define N_ASPECTS    	4

// static char *aspect_to_str[]={"16:10","4:3","5:4","16:9"};

#define	MAX_USER_MODES		32		// number of USER timing modes
#define MAX_TOTAL_MODES		(N_KNOWN_MODES + MAX_USER_MODES)
#define DEF_OUTPUT_MODE		HDMI_16_1920x1080P_60

static int			 gKnownModesNum = N_KNOWN_MODES;
static EDID_TIMING gKnownModes[MAX_TOTAL_MODES +1] = {
  { // DP_1900x1200P_60,
	154000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_HIGH, POL_LOW, POL_HIGH,
	60, 2080, 1920,	 48,  32, 1235, 1200,  3,  6,
  },
  { // DVI_1600x1200P_60
	162000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 2160, 1600,	 64, 192, 1250, 1200,  1,  3,
  },
  {	// HDMI_16_1920x1080P_60
  	148500, MK_PIXEL_HDMI(0, 16, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
  	60, 2200, 1920,  88,  44, 1125, 1080,  4,  5,
  },
  {	// HDMI_31_1920x1080P_50
    148500, MK_PIXEL_HDMI(0, 31, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
    50, 2640, 1920, 528,  44, 1125, 1080,  4,  5,
  },
  {	// HDMI_05_1920x1080I_60
  	74250,  MK_PIXEL_HDMI_I(0,  5, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
  	60, 2200, 1920,  88,  44, 1125, 1080,  4,  5,
  },
  {	// HDMI_20_1920x1080I_50
  	74250,  MK_PIXEL_HDMI_I(0, 20, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
  	50, 2640, 1920, 528,  44, 1125, 1080,  4,  5,
  },
  { // DVI_1680x1050P_60 (Reduced blanking)
	119000,	MK_PIXEL_HDMI_R(1,  0, AS_RATIO_16_10), POL_HIGH, POL_LOW, POL_HIGH,
	60, 1840, 1680,	 48,  32, 1080, 1050,  3,  6,
  },
  { // DVI_1400x1050P_75
	156000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_16_10), POL_LOW, POL_HIGH, POL_HIGH,
	75, 1896, 1400,	104, 144, 1099, 1050,  3,  4,
  },
  { // DVI_1400x1050P_60 (Reduced Blanking)
	101000,	MK_PIXEL_HDMI_R(1,  0, AS_RATIO_16_10), POL_HIGH, POL_LOW, POL_HIGH,
	60, 1560, 1400,	 48,  32, 1080, 1050,  3,  4,
  },
  { // DVI_1280x1024P_85
	157500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_5_4), POL_HIGH, POL_HIGH, POL_HIGH,
	85, 1728, 1280,  64, 160, 1072, 1024,  1,  3,
  },
  { // DVI_1280x1024P_75
	135000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_5_4), POL_HIGH, POL_HIGH, POL_HIGH,
	75, 1688, 1280,  16, 144, 1066, 1024,  1,  3,
  },
  { // DVI_1280x1024P_60
	108000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_5_4), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1688, 1280,  48, 112, 1066, 1024,  1,  3,
  },
  { // DVI_1280x1024P_50, 5:4
	75428,	MK_PIXEL_HDMI(1,  0, AS_RATIO_5_4), POL_HIGH, POL_HIGH, POL_HIGH,
	50, 1440, 1280,	 32,  48, 1049, 1024,  3,  7,
  },
  { // DVI_1280x960P_60, 4:3
	108000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1800, 1280,	 96, 112, 1000,  960,  1,  3,
  },
  { // DVI_1440x900P_75
	136750,	MK_PIXEL_HDMI(1,  0, AS_RATIO_16_10), POL_LOW, POL_HIGH, POL_HIGH,
	75, 1936, 1440,	 96, 152,  942,  900,  3,  6,
  },
  { // DVI_1440x900P_60 (Reduced blanking)
	88750,	MK_PIXEL_HDMI_R(1,  0, AS_RATIO_16_10), POL_HIGH, POL_LOW, POL_HIGH,
	60, 1600, 1440,	 48,  32,  934,  900,  3,  6,
  },
  { // DVI_1440x900P_60
	106500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_16_10), POL_LOW, POL_HIGH, POL_HIGH,
	60, 1904, 1440,	 80, 152,  926,  900,  3,  6,
  },
  { // DVI_1280x800P_75
	106500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_LOW, POL_HIGH, POL_HIGH,
	75, 1696, 1280,	 80, 128,  838,  800,  3,  6,
  },
  { // DVI_1280x800P_60 (Reduced blanking)
	71000,	MK_PIXEL_HDMI_R(1,  0, AS_RATIO_NA), POL_HIGH, POL_LOW, POL_HIGH,
	60, 1440, 1280,	 48, 32,  823,  800,  3,  6,
  },
  { // DVI_1366x768P_60
	76000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_16_9), POL_LOW,   POL_LOW,  POL_LOW,
	60, 1560, 1366,	 32,  64,  806,  768,  6, 12,
  },
  { // DVI_1366x768_CTM (Reduced blanking)
	72350,	MK_PIXEL_HDMI_R(1,  0, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1526, 1366,	 48,  32,  790,  768,  3,  5,
  },
  { // DVI_1360x768P_60
	85500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1792, 1360,	 64, 112,  795,  768,  3,  6,
  },
  { // DVI_1280x768P_75, 5:3
	102250,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_LOW, POL_HIGH, POL_HIGH,
	75, 1696, 1280,	 80, 128,  805,  768,  3,  7,
  },
  { // DVI_1280x768P_RDC (Reduced blanking)
	68250,	MK_PIXEL_HDMI_R(1,  0, AS_RATIO_NA), POL_HIGH, POL_LOW, POL_HIGH,
	60, 1440, 1280,	 48,  32,  790,  768,  3,  7,
  },
  { // DVI_1280x768P_60, from VESA standard
	79500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_LOW, POL_HIGH, POL_HIGH,
	60, 1664, 1280,	 64, 128,  798,  768,  3,  7,
  },
  { // DVI_1280x768P_TV, from a TV, non-standard
	80120,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1680, 1280,	 64, 134,  795,  768,  1,  3,
  },
  { // VGA_1024x768P_85
	94500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	85, 1376, 1024,	 48,  96,  808,  768,  1,  3,
  },
  { // VGA_1024x768P_75, 4:3
	78750,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	75, 1312, 1024,	 16,  96,  800,  768,  1,  3,
  },
  { // VGA_1024x768P_70
	75000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_LOW,  POL_LOW,  POL_LOW,
	70, 1328, 1024,  24, 136,  806,  768,  3,  6,
  },
  { // VGA_1024x768P_60
	65000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_LOW,  POL_LOW,  POL_LOW,
	60, 1344, 1024,  24, 136,  806,  768,  3,  6,
  },
  {	// HDMI_04_1280x720P_60
  	74250,  MK_PIXEL_HDMI(0,  4, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
  	60, 1650, 1280, 110,  40,  750,  720,  5,  5,
  },
  {	// HDMI_19_1280x720P_50
  	74250,  MK_PIXEL_HDMI(0, 19, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
  	50, 1980, 1280,	440,  40,  750,  720,  5,  5,
  },
  { // DVI_1024x600P_60
	47360,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1240, 1024,  40, 128,  638,  600, 10,  3,
  },
  { // DVI_1024x600_YING
	45000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA),  POL_LOW,  POL_LOW,  POL_LOW,
	60, 1200, 1024,	 40, 118,  625,  600,  1,  4,
  },
  { // VGA_800x600P_85
	56250,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	85, 1048,  800,	 32,  64,  631,  600,  1,  3,
  },
  { // VGA_800x600P_75
	49500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	75, 1056,  800,	 16,  80,  625,  600,  1,  3,
  },
  { // VGA_800x600P_60
	40000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
	60, 1056,  800,	 40, 128,  628,  600,  1,  4,
  },
  {	// HDMI_17_720x576P_50
  	27000,  MK_PIXEL_HDMI(0, 17, AS_RATIO_4_3), POL_HIGH, POL_HIGH, POL_HIGH,
  	50,  864,  720,  12,  64,  625,  576,  5,  5,
  },
  { // HDMI_02_720x480P_60
 	27000,  MK_PIXEL_HDMI(0,  2, AS_RATIO_16_9), POL_HIGH, POL_HIGH, POL_HIGH,
  	60,  858,  720,  16,  62,  525,  480,  9,  6,
  },
  { // VGA_640x480P_85, 5:3
	36000,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_LOW, POL_LOW, POL_LOW,
	85,  832,  640,	 56,  56,  509,  480,  1,  3,
  },
  { // VGA_640x480P_75, 5:3
	31500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_LOW, POL_LOW, POL_LOW,
	75,  840,  640,	 16,  64,  500,  480,  1,  3,
  },
  { // VGA_640x480P_72, 4:3, 72.8Hz, round down
	31500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_4_3), POL_LOW, POL_LOW, POL_LOW,
	72,  832,  640,	 24,  40,  520,  480,  9,  3,
  },
  { // HDMI_01_640x480P_60
	25200,	MK_PIXEL_HDMI(0,  1, AS_RATIO_4_3), POL_LOW, POL_LOW, POL_HIGH,
	60,  800,  640,  16,  96,  525,  480, 10,  2,
  },

  { // DVI_720x400P_85
	35500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_LOW, POL_HIGH, POL_HIGH,
	85,  936,  720,  36,  72,  446,  400,  1,  3,
  },
  { // DVI_640x400P_85
	31500,	MK_PIXEL_HDMI(1,  0, AS_RATIO_NA), POL_LOW, POL_HIGH, POL_HIGH,
	85,  832,  640,  32,  64,  445,  400,  1,  3,
  },

  // followed by user modes found in EDID ...
};

typedef struct _ESTABLISHTIMING {
	int			mode;	// index in known modes
	uint8		idx;	// byte in EDID
	uint8		mask;	// bit in byte above
} ESTABLISHTIMING;

static ESTABLISHTIMING	edid_estab[] = {
//	{  DVI_720x400P_70,		35,	0x80 },	 	// 720x400@70 not support
	{  DVI_720x400P_85,		35, 0x40 },		// 720x400@88
	{ HDMI_01_640x480P_60,	35,	0x20 },		// 640x480@60
//	{  VGA_640x480P_67,		35, 0x10 }, 	// 640x480@67 not support
	{  VGA_640x480P_72,		35, 0x08 },		// 640x480@72
	{  VGA_640x480P_75,		35, 0x04 },		// 640x480@75
//	{  VGA_800x600P_56,		35, 0x02 },		// 800x600@56 not support
	{  VGA_800x600P_60,		35,	0x01 },		// 800x600@60
//	{  VGA_800x600P_72,		36, 0x80 },		// 800x600@72 not support
	{  VGA_800x600P_75,		36, 0x40 },		// 800x600@75
//	{  DVI_832x624P_75,		36, 0x20 }, 	// 832x624@75 not support
//	{  VGA_1024x768I_87		36, 0x10 },		// 1024x768i@87 not support
	{  VGA_1024x768P_60,	36,	0x08 }, 	// 1024x768@60
	{  VGA_1024x768P_70,	36, 0x04 },		// 1024x768@70
	{  VGA_1024x768P_75,	36, 0x02 },		// 1024x768@75
	{  DVI_1280x1024P_75,	36, 0x01 },		// 1280x1024@75
//	{  DVI_1152x870P_75,	37, 0x80 },		// 1152x870@75 not support
};

// list modes from EDID, index in gKnownModes
// #define		MAX_MODES_IN_EDID		128
#define	MAX_MODES_IN_EDID		MAX_TOTAL_MODES
static unsigned char gEDIDModes[MAX_MODES_IN_EDID];
static int			 gEDIDModesNum = 0, gNativeModeIndex = 0, gFHDIndex = 0;

int edid_valid(uint8 *edid)
{
	return ((edid[0]==0x00) && (edid[1]==0xff) &&
			(edid[2]==0xff) && (edid[3]==0xff) &&
			(edid[4]==0xff) && (edid[5]==0xff) &&
			(edid[6]==0xff) && (edid[7]==0x00));
}


//static void show_ch_timing(const char * fmt, int fmtInt, TIMING_CH7036* timing)
//{ 
//	MSG_OUT(fmt, fmtInt);
//	MSG_OUT("  %dKHz, 0x%X, %d, %d, %d,\n", 
//		timing->clk_khz, timing->pixel_fmt,
//		timing->hs_pol, timing->vs_pol, timing->de_pol);
//	MSG_OUT("  %d, %d, %d, %d, %d, %d, %d, %d, %d 0x%04x\n",
//		timing->fps, timing->ht, timing->ha, timing->ho, timing->hw,
//		timing->vt, timing->va, timing->vo, timing->vw, timing->scale);
//}


// There are 3 cases to match timing modes in the known table:
// 1. Know the actual ht, ha, vt, va and pixel clock from DTD, but fps is
//    calculated and not actual(some modes round up, i.e. 59.94->60,
//    some modes round down, i.e. 72.8->72);
// uses MATCHMODE_TOTAL_PIXELS | MATCHMODE_PIXEL_CLOCK;
// 2. Know the actual ha, va and fps from the Standard timing section in EDID;
// uses MATCHMODE_INTERLACED | MATCHMODE_REFRESH_RATE;
// 3. know the HDMI format number from CEA;
// uses find_hdmi_mode(uint8 hdmi_fmt_num);
//
// Alway compare active pixels, Following define the additional parameters to
// compared
#define		MATCHMODE_REFRESH_RATE			0x0001
#define		MATCHMODE_TOTAL_PIXELS			0x0002
#define		MATCHMODE_PIXEL_CLOCK			0x0004
#define		MATCHMODE_INTERLACED			0x0008

static int	match_timing_mode(EDID_TIMING modes[], int num_modes,
						EDID_TIMING* timing, int match_mode)
{
	int		i;

	for(i = 0;  i < num_modes;  i++) {
		// Alway compare active pixels
		if( modes[i].ha != timing->ha || modes[i].va != timing->va )
			continue;

		// Compare refresh rate
		if( (match_mode & MATCHMODE_REFRESH_RATE) &&
			(modes[i].fps != timing->fps) )
			continue;

		// compare total pixels
		if( (match_mode & MATCHMODE_TOTAL_PIXELS) &&
			(modes[i].ht != timing->ht || modes[i].vt != timing->vt) )
			continue;

		// compare pixel clock
		if( (match_mode & MATCHMODE_PIXEL_CLOCK) &&
			(modes[i].clk_khz != timing->clk_khz) )
			continue;

		// compare interlaced or progressive
		if( (match_mode & MATCHMODE_INTERLACED) &&
			((modes[i].pixel_fmt ^ timing->pixel_fmt) & PIXEL_HDMI_ENCODE_INTERLACE) )
			continue;

//		DBG_OUT_TIMINGMODE("match known mode %d", i, &modes[i]);
		return	i;
	}

//	show_ch_timing("unmatch mode %d", i, timing);
	return -1;
}

static int  find_hdmi_mode(uint8 fmtID)
{
	int		i;

	for(i = 0; i < N_KNOWN_MODES; i++) {
		if( !(gKnownModes[i].pixel_fmt & PIXEL_HDMI_ENCODE_DVI) &&
			((uint8)PIXEL_HDMI_FOTMAT(gKnownModes[i].pixel_fmt) == fmtID) )
			return i;
	}

	return -1;
}

int get_DTD_timing(uint8 *dtd, EDID_TIMING *timing)
{
	uint32	tmp32;
	int		idx;
	uint8   mdflg;
//  int		hsiz, vsiz, hbdr, vbdr;

	tmp32 = dtd[0] + (dtd[1]<<8);
	if ( tmp32 == 0 )
		return ERR_INVALID_DTD;		// not a DTD

	timing->clk_khz = tmp32 * 10;
	timing->ha = dtd[2] + ((dtd[4] & 0xf0)<<4);	// hres
	timing->ht = timing->ha + (dtd[3] + ((dtd[4] & 0x0f)<<8));	// hbl
	timing->va = dtd[5] + ((dtd[7] & 0xf0)<<4);	// vres
	timing->vt = timing->va + (dtd[6] + ((dtd[7] & 0x0f)<<8));	// vbl
	timing->ho = dtd[8] + ((dtd[11] & 0xc0)<<2);	// hso
	timing->hw = dtd[9] + ((dtd[11] & 0x30)<<4);	// hsw
	timing->vo = (dtd[10] >> 4) + ((dtd[11] & 0x0c)<<2);	// vso;
	timing->vw = (dtd[10] & 0xf) + ((dtd[11] & 0x03)<<4);	// vsw
// 	hsiz = dtd[12] + ((dtd[14] & 0xf0)<<4);
//	vsiz = dtd[13] + ((dtd[14] & 0x0f)<<8);
//	hbdr = dtd[15];
//	vbdr = dtd[16];

	mdflg = dtd[17];
	timing->de_pol = // POL_HIGH; the same as hs_pol
	timing->hs_pol = (mdflg & 2) ? POL_HIGH : POL_LOW;
	timing->vs_pol = (mdflg & 4) ? POL_HIGH : POL_LOW;

	// Because of no clue to get the input pixel format, We fill pixel_fmt
	// with the output pixel format.
	// Override it out of this function if you want the input pixel format
	// Wang Youcheng 2010-9-25
	// How to identify if it is a reduced blanking mode ?
	if( mdflg & 0x80 ) {
		// adjust vertical paramters if interlaced mode
		timing->va <<= 1;
		timing->vt = (timing->vt << 1) + 1;
		timing->vo <<= 1;
		// not adjust vsw, timing->vw

		 // assume DVI mode. Find out if it is a HDMI mode
		timing->pixel_fmt = MK_PIXEL_HDMI_I(1, 0, AS_RATIO_NA);
		tmp32 = (timing->ht * timing->vt) >> 1;
	} else {
		 // assume DVI mode. Find out if it is a HDMI mode
		timing->pixel_fmt = MK_PIXEL_HDMI(1, 0, AS_RATIO_NA);
		tmp32 = timing->ht * timing->vt;
	}
	if(tmp32 > 0)
	{
	    timing->fps = (uint8)((timing->clk_khz * 1000 + (tmp32 >> 1)) / tmp32);
	}
	else
	{
	    timing->fps = 60;
	}
	// overwrite its format if it is a known mode
	idx = match_timing_mode(gKnownModes, gKnownModesNum, timing,
		MATCHMODE_PIXEL_CLOCK | MATCHMODE_TOTAL_PIXELS);
	if ( idx >= 0 )
		timing->pixel_fmt = gKnownModes[idx].pixel_fmt;

	return idx;
}

static void find_native_mode(void)
{
	int				i, n;
	EDID_TIMING 	*t, *cur;

	n = 0;	// assume the first is a native mode
	cur = &gKnownModes[ gEDIDModes[0] ];
	for(i = 1; i < gEDIDModesNum; i++) {
		t = &gKnownModes[ gEDIDModes[i] ];

        //printk("edid num=%d,mode=%d,fmt=0x%x\n", i, gEDIDModes[i], t->pixel_fmt); //MTK-Add
		// ignore non-native mode
		if ( !(t->pixel_fmt & PIXEL_HDMI_ENCODE_NATIVE) ) continue;

		// ignore interlaced mode
		if ( t->pixel_fmt & PIXEL_HDMI_ENCODE_INTERLACE ) continue;

		// both current mode and assumed mode are native
		if ( cur->pixel_fmt & PIXEL_HDMI_ENCODE_NATIVE ) {
			if( t->va < cur->va ) continue; 	// lower V resolution

			if( t->va == cur->va ) {
				if( t->ha < cur->ha ) continue;	// lower H resolution

				if( t->ha == cur->ha ) {
					if( t->clk_khz < cur->clk_khz ) continue; // lower refresh rate
				}
			}
		}

//		DBG_OUT_TIMINGMODE("index %d maybe native", i, t);
		n = i;	cur = t;
	}

	gNativeModeIndex = n;
}

static void find_fhd(void)
{
	int				i, n;
	EDID_TIMING 	*t, *cur;

	n = 0;	// assume the first is FHD
	cur = &gKnownModes[ gEDIDModes[0] ];
	for(i = 1; i < gEDIDModesNum; i++) {
		t = &gKnownModes[ gEDIDModes[i] ];

		// ignore interlaced mode
		if ( t->pixel_fmt & PIXEL_HDMI_ENCODE_INTERLACE ) continue;
		
		if( t->va < cur->va ) continue; 	// lower V resolution

		if( t->va == cur->va ) {
			if( t->ha < cur->ha ) continue;	// lower H resolution

			if( t->ha == cur->ha ) {
				if( t->clk_khz < cur->clk_khz ) continue; // lower refresh rate
			       if( t->clk_khz == cur->clk_khz ) {	
			        if( t->fps < cur->fps ) continue; // lower frame rate
			        }
			}
		}

		n = i;	cur = t;
	}

	gFHDIndex = n;
}

// sort the modes in EDID from more to less with the following priority
// vertical resolution -> horizontal resolution -> pixel clock
static void insert_edid_mode(int newIdx)
{
	int 			i, j;
	EDID_TIMING 	*t, *cur;

	cur = &gKnownModes[newIdx];
	for(i=0; i<gEDIDModesNum; i++) {
		t = &gKnownModes[ gEDIDModes[i] ];

		if ( t->va < cur->va ) break;

		if ( t->va == cur->va ) {
			if ( t->ha < cur->ha ) break;

			if ( t->ha == cur->ha ) {
				if( t->clk_khz < cur->clk_khz ) break;
			}
		}
	}

	for(j=gEDIDModesNum; j>i; j--)
		gEDIDModes[j] = gEDIDModes[j-1];

	gEDIDModes[i] = newIdx;
	gEDIDModesNum++;
}

static void add_edid_mode(int mode, int bNative)
{
	int		i;

	// some EDIDs repeat the same timing mode in different sections
	// check if we have listed the new mode
	for(i=0; i<gEDIDModesNum; i++) {
		if(mode == gEDIDModes[i]) {
//			DBG_OUT_TIMINGMODE("repeat mode %2d", mode, &gKnownModes[mode]);
			return;
		}
	}

//	DBG_OUT_TIMINGMODE("mode %2d", mode, &gKnownModes[mode]);
	if (gEDIDModesNum >= MAX_MODES_IN_EDID) {
		printk("too many modes %d in EDID\n", gEDIDModesNum); //MTK-Modify
		return;
	}
	insert_edid_mode( mode );

	if (bNative)
		gKnownModes[mode].pixel_fmt |= PIXEL_HDMI_ENCODE_NATIVE;
	else
		gKnownModes[mode].pixel_fmt &= ~PIXEL_HDMI_ENCODE_NATIVE;
}

static void add_edid_dtd(uint8 *dtd, int bNative)
{
	EDID_TIMING 	*timing;
	int				idx;

	timing = &gKnownModes[gKnownModesNum];
	idx = get_DTD_timing(dtd, timing);
	if ( idx < -1 )	// not an valid DTD
		return;

	if ( idx == -1 ) { 	// it is a new user mode
//		show_ch_timing("new user mode %2d", gKnownModesNum, timing);
		if (gKnownModesNum >= MAX_TOTAL_MODES) {  // copy to known modes table
			printk("too many user timing modes %d\n", gKnownModesNum); //MTK-Modify
			return;
		}
		idx = gKnownModesNum++;
	}

	// now, idx is the index in known modes table
	add_edid_mode(idx, bNative);
}

static void add_edid_standard(uint8 l, uint8 p, int bNative)
{
	EDID_TIMING 	*timing;
	int				idx;
	uint8   		as_ratio;

   	// padded data may be 0x01, 0x00, or 0x20
	if (((l == 0x01) && (p == 0x01)) ||
		((l == 0x00) && (p == 0x00)) ||
		((l == 0x20) && (p == 0x20)))
	return;

	timing = &gKnownModes[gKnownModesNum];
	timing->ha = (uint16)(l + 31) << 3;
	switch( p >> 6 ) {
		case ASPECT_16_10:
			as_ratio = AS_RATIO_16_10;
			timing->va = (timing->ha * 5) >> 3;
			break;
		case ASPECT_4_3:
			as_ratio = AS_RATIO_4_3;
			timing->va = (timing->ha * 3) >> 2;
			break;
		case ASPECT_5_4:
			as_ratio = AS_RATIO_5_4;
			timing->va = (timing->ha * 4) / 5;
			break;
		default:	//	case ASPECT_16_9:
			as_ratio = AS_RATIO_16_9;
			timing->va = (timing->ha * 9) >> 4;
	}
	timing->fps = 60 + (p & 0x3f);
	timing->pixel_fmt = MK_PIXEL_HDMI(0, 0, as_ratio); // assume progressive

	// To get other important paramters, such as pixel clock and total
	// resolution, only support those modes in the known modes table
	idx = match_timing_mode(gKnownModes, N_KNOWN_MODES, timing,
			MATCHMODE_INTERLACED | MATCHMODE_REFRESH_RATE);
	if ( idx >= 0  )
		add_edid_mode(idx, bNative);
}

// assume
//	 timing in established section as non-native
//	 timing in Standard section    as non-native
//	 timing in DTD                 as native
//	 timing in CEA                 as specified
int  parse_edid_timing(uint8 *edid)
{
	int		i, idx;
	uint8   l, p, n;
	uint8 	*ext, *dtd;

	// reset known modes table
	gEDIDModesNum = 0; gKnownModesNum = N_KNOWN_MODES;

	if ( !edid_valid(edid) ) {
		printk("invalid EDID\n"); //MTK-Modify
		return ERR_INVALID_EDID;
	}

//	DBG_OUT("Search established timing: %d\n", gEDIDModesNum);
	for(i = 0; i< (int)(sizeof(edid_estab)/sizeof(ESTABLISHTIMING)); i++) {
		if( edid[edid_estab[i].idx] & edid_estab[i].mask ) {
			add_edid_mode(edid_estab[i].mode, 0); // non-native
		}
	}

//	DBG_OUT("Search Standard timing: %d\n", gEDIDModesNum);
	for(i=0; i < 8; i++) {
		add_edid_standard(edid[38 + 2*i], edid[39 + 2*i], 0);	// non-native
	}

//	DBG_OUT("Search DTD: %d\n", gEDIDModesNum);
	for(dtd = edid + 54, i = 0; i < 4; i++, dtd += 18) {
		add_edid_dtd(dtd, 1);	// native
	}

	for( ext=edid+128, n=1; n <= edid[126]; n++, ext+=128 ) {
//		DBG_OUT("Search EDID extension %d\n", n);

		// not an EDID extension or no CEA
		if( (ext[0] != 0x02) || (ext[1] != 0x03) )
			continue;

//		DBG_OUT("Search in CEA: %d\n", gEDIDModesNum);
		p = 4;
		while( p < ext[2] ) {
			l = ext[p] & 0x1f;
			if( 0x40 != (ext[p] & 0xE0) ) { 	// ignore non-Vidio Data Block
				p += l + 1;
				continue;
			}

			for(p++; l>0; l--, p++) {
				// only support those modes in the known modes table
				idx = find_hdmi_mode(ext[p] & 0x7F);
				if( idx >= 0)
					add_edid_mode(idx, ext[p] & 0x80);
			}
		}

		// no DTD in EDID extension
		if( ext[2] < 4 )
			continue;

//		DBG_OUT("Search DTD in Extension: %d\n", gEDIDModesNum);
		for(dtd = ext+ext[2], i=(ext[3]&0x0F); i > 0; i--, dtd+=18) {
			add_edid_dtd(dtd, 1);	// native
		}
	}

	find_native_mode();
	find_fhd();

	printk("Total %d timing modes in EDID, %d is native mode\n",
		gEDIDModesNum, gNativeModeIndex); //MTK-Modify

	return gEDIDModesNum;
}

// get timing mode from known-modes table
//  return: >=0    index in the known modes table(same as index)
//          <0     error code
int  get_known_timing(EDID_TIMING* timing, int index)
{
	// get known mode, either default mode or specified mode
	if (index < 0 )		// default mode
		index = DEF_OUTPUT_MODE;
	
	if (index >= N_KNOWN_MODES)
		return  ERR_UNKOWN_MODE;

	memcpy(timing, &gKnownModes[index], sizeof(EDID_TIMING));

	return index;
}

// get mode from EDID after parsing
// timing modes are listed in the paramters order
// vertical resoluiton -> horizental resolution --> fps (half if interlaced)
int  get_edid_timing(EDID_TIMING* timing, int index)
{
	// get EDID mode, either native mode or specified mode
	if (index < 0)	// native mode
		index = gFHDIndex;//gNativeModeIndex;

	if (index >= gEDIDModesNum)
		return  ERR_UNKOWN_EDID_MODE;

	index = gEDIDModes[index]; // index in known modes table after parsed

	memcpy(timing, &gKnownModes[index], sizeof(EDID_TIMING));

	return index;
}
