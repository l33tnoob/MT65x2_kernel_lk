// EDID utility

#ifndef _EDID_UTILS_H_
#define _EDID_UTILS_H_

// pixel format output
//  bit[19]	  label whether it is a reduced blanking mode
//  bit[18]	  label whether it is a native mode
//  bit[17]   indicates force interlace
//  bit[16]   DVI(1) or HDMI(0) mode
//  bit[15:8] aspect ratio
//  bit[7:0]  HDMI format number,
#define PIXEL_HDMI_ENCODE_DVI		0x10000		// DVI or HDMI
#define PIXEL_HDMI_ENCODE_INTERLACE	0x20000		// interlace
#define PIXEL_HDMI_ENCODE_NATIVE	0x40000		// native mode
#define PIXEL_REDUCED_BLANKING		0x80000		// reduced blanking mode

#define PIXEL_HDMI_FOTMAT(x)		(0xFF & (x))
#define PIXEL_HDMI_ASPECT(x)	 	((0xFF00 & (x))>>8)

// output pixel format
#define MK_PIXEL_HDMI(dvi,fmt,aspect)  	((((dvi) & 0x01)<<16)+(((aspect) & 0xFF)<<8)+ ((fmt) & 0xFF))
#define MK_PIXEL_HDMI_I(dvi,fmt,aspect) (PIXEL_HDMI_ENCODE_INTERLACE +(((dvi) & 0x01L)<<16)+(((aspect) & 0xFF)<<8)+ ((fmt) & 0xFF))
#define MK_PIXEL_HDMI_R(dvi,fmt,aspect) (PIXEL_REDUCED_BLANKING +(((dvi) & 0x01L)<<16)+(((aspect) & 0xFF)<<8)+ ((fmt) & 0xFF))

// scale down
#define SCALE_HDOWN(x)  		(0xFF & (x))
#define SCALE_VDOWN(x)  		((0xFF00 & (x))>>8)
#define MK_SCALE(hdown, vdown)  ((0xFF & (hdown))+ ((0xFF & (vdown))<<8))

// get native mode from EDID, >=0 means the specified index of mode from EDID
#define	GETTIMING_NATIVEMODE		-1

// MTK-Modify
//------------------ known modes ----------------------------------
// timing modes are listed in the paramters order
// vertical resoluiton -> horizental resolution --> fps (half if interlaced)
// update edid_estab[] if you update known modes
typedef  enum _KNOWNMODEID {
	DP_1900x1200P_60,
	DVI_1600x1200P_60,
	HDMI_16_1920x1080P_60,
	HDMI_31_1920x1080P_50,
	HDMI_05_1920x1080I_60,
	HDMI_20_1920x1080I_50,
	DVI_1680x1050P_60,
	DVI_1400x1050P_75,
	DVI_1400x1050P_60,
	DVI_1280x1024P_85,
	DVI_1280x1024P_75,
	DVI_1280x1024P_60,
	DVI_1280x1024P_50,
	DVI_1280x960P_60,
	DVI_1440x900P_75,
	DVI_1440x900P_RDC,
	DVI_1440x900P_60,
	DVI_1280x800P_75,
	DVI_1280x800P_60,
	DVI_1366x768P_60,
    DVI_1366x768_CTM,
	DVI_1360x768P_60,
	DVI_1280x768P_75,
	DVI_1280x768P_RDC,	// reduced blanking
	DVI_1280x768P_60,	// VESA standard
	DVI_1280x768P_TV,	// non-standard
	VGA_1024x768P_85,
	VGA_1024x768P_75,
	VGA_1024x768P_70,
	VGA_1024x768P_60,
	HDMI_04_1280x720P_60,
	HDMI_19_1280x720P_50,
	DVI_1024x600P_60,
	DVI_1024x600_YING,
	VGA_800x600P_85,
	VGA_800x600P_75,
	VGA_800x600P_60,
	HDMI_17_720x576P_50,
	HDMI_02_720x480P_60,
	VGA_640x480P_85,
	VGA_640x480P_75,
	VGA_640x480P_72,
	HDMI_01_640x480P_60,
	DVI_720x400P_85,
 	DVI_640x400P_85,

	N_KNOWN_MODES,		// always be the last
} KNOWNMODEID;

typedef struct _edid_timing {
	uint32		clk_khz;
	uint32  	pixel_fmt;
	uint8  		hs_pol;
	uint8  		vs_pol;
	uint8  		de_pol;
	uint8		fps;
	uint16 		ht;
	uint16 		ha;
	uint16 		ho;
	uint16 		hw;
	uint16 		vt;
	uint16 		va;
	uint16 		vo;
	uint16 		vw;
} EDID_TIMING;

#ifdef __cplusplus
extern "C" {
#endif

//  return: >=0    index in the known modes table
//           -1    new mode, not in the known modes table
//         other   error code
int  get_DTD_timing(uint8 *dtd, EDID_TIMING *timing);

//  return: >=0    index in the known modes table(same as index)
//          <0     error code
int  get_known_timing(EDID_TIMING* timing, int index);

//  return: >0     number of timing modes parsed
//          <0     error code
int  parse_edid_timing(uint8 *edid);

//  enter:
//	   index: >=0  index in the parsed EDID timing modes
//			  -1   native mode of the EDID
//  return:   >=0  index in the known modes table
//            <0   error code
//  call this function after parse_edid_timing
int  get_edid_timing(EDID_TIMING* timing, int index);

#ifdef __cplusplus
}
#endif

#endif
