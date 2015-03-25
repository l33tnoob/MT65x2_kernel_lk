// Description: ch7033-35 interface


#ifndef _CH703x_LIB_H_
#define _CH703x_LIB_H_

//--------------- TIMING INFORMATION -----------------------
typedef struct{
	uint32		ht;		// horizontal tatal pixels
	uint32		ha;		// horizontal active pixels
	uint32		ho;		// horizontal offset (back porch)
	uint32		hw;		// horizontal sync pulse width
	uint32		vt;		// vertical total lines
	uint32		va;		// vertical active lines
	uint32		vo;		// vertical offset (back porch)
	uint32		vw;		// vertical sync pulse width
}CH703x_TIMING, *PCH703x_TIMING;

//--------------- INPUT INFORMATION -----------------------
// pixel format for input
#define PIXEL_FMT_RGB888	0	// RGB 888 format (24-bit)
#define PIXEL_FMT_RGB666	1	// RGB 666 format (18-bit)
#define PIXEL_FMT_RGB565	2	// RGB 565 format (16-bit)
#define PIXEL_FMT_RGB555	3	// RGB 555 format (16-bit)
#define PIXEL_FMT_RGBDVO	4	// RGB DVO
#define PIXEL_FMT_YUV422_8	5	// YCbCr 4:2:2 8-bit
#define PIXEL_FMT_YUV422_10	6	// YCbCr 4:2:2 10-bit
#define PIXEL_FMT_YUV444	7	// YCbCr 4:4:4 8-bit
#define PIXEL_FMT_RGB666_C	9	// consecutive RGB666
#define PIXEL_FMT_RGB565_C	10	// consecutive RGB565
#define PIXEL_FMT_RGB555_C	11	// consecutive RGB555

// polarity and invert
#define POL_HIGH			1
#define POL_LOW				0
#define POL_INVERT			1
#define POL_NO_INV			0

// audio input type
#define AUDIO_I2S			1
#define AUDIO_SPDIF			0

#define SPDIF_MOD_DIFF		0
#define SPDIF_MOD_CMOS		1

// I2S audio format
#define I2S_FMT_STD			0
#define I2S_FMT_RTJ			1		// right justified
#define I2S_FMT_LTJ			2		// left justified

#define I2S_LEN_16B			0		// 16 - bit
#define I2S_LEN_20B			1		// 20 - bit
#define I2S_LEN_24B			2		// 24 - bit

typedef enum{
	IF_PRO = 0,		// progressive video interface
	IF_INT,			// interlace video interface
	IF_CPU,			// video interface like CPU data interface
}IF_TYPE;

// Multi 2x format:
#define M2X_SEQ_NO_INV		0
#define M2X_SEQ_INVERT		1
#define M2X_ALIGH_LBIT		0
#define M2X_ALIGH_HBIT		1
typedef struct{
	// for 2x multi:
	uint8		halfword_swap;	// halfword swap: refer to "SWP_PAPB" in datasheet
	uint8		halfword_align;	// halfword align select: refer to "AH_LB" in datasheet
	uint8		yc_swap;		// indicate Y and C swap (only for BT656 input)
	uint8		reserved;
}M2X_FMT, *PM2X_FMT;

// Multi 3x format;
#define M3X_ALIGH_LBIT		0
#define M3X_ALIGH_MBIT		1
#define M3X_ALIGH_HBIT		2
typedef struct{
	// for 3x multi:
	uint8		byte_align;		// byte align: refer to "POS3X" in datasheet
	uint8		reserved[3];
}M3X_FMT, *PM3X_FMT;

#define MULTI_1X		0
#define MULTI_2X		1
#define MULTI_3X		2

// input pixel format byte swap, byte_swap
#define BYTE_SWAP_RGB	0
#define BYTE_SWAP_RBG	1
#define BYTE_SWAP_GRB	2
#define BYTE_SWAP_GBR	3
#define BYTE_SWAP_BRG	4
#define BYTE_SWAP_BGR	5

typedef struct{
	uint8		format;			// input data format, refer to IDF in datasheet
	uint8		bit_swap;		// select bit swap for each byte, refer to REVERSE in datasheet
	uint8		byte_swap;		// R-G-B or Y-Pb-Pr swap, refer to SWAP in datasheet
	uint8		byte_align;		// MSB align or LSB align, refer to HIGH in datasheet
	uint8		multiplexed;	// indicate if data is multiplexed: refer to MULTI in datasheet
	uint8		embedded_sync;	// indicate if embedded sync is used
	uint8		reserved[2];
	M2X_FMT		m2x_fmt;		// multiplexed 2x data properties
	M3X_FMT		m3x_fmt;		// multiplexed 3x data properties
}PIXEL_FMT, *PPIXEL_FMT;

typedef struct{
	IF_TYPE			if_type;	// interface type
	CH703x_TIMING	timing;
	uint32			pclk_khz;	// pixel clock frequecy
	PIXEL_FMT		pixel_fmt;
	uint8			hs_pol;		// horizontal sync polarity
	uint8			vs_pol;		// vertical sync polarity
	uint8			de_pol;		// data enable  polarity
	// audio related:
	uint8			audio_type;	// 0: spdif - 1: i2s
	uint8			spdif_mode;	// 0: differential mode - 1: cmos mode
	uint8			i2s_pol;
	uint8			i2s_len;
	uint8			i2s_fmt;
	// if using crystal:
	uint8			crystal_used;	// 0: no crystal - 1: using crystal
	uint8			reserved[3];
	uint32			crystal_khz;
}CH703x_INPUT_INFO, *PCH703x_INPUT_INFO;

//--------------- OUTPUT INFORMATION -----------------------
//channel bit define:
#define LVDS_DATA0_SEL			0
#define LVDS_DATA1_SEL			1
#define LVDS_DATA2_SEL			2
#define LVDS_DATA3_SEL			3
#define LVDS_CLOCK_SEL			4

#define LVDS_CHANNEL_SWAP_DEF	0
#define LVDS_CHANNEL_SWAP_OP1	1

#define LVDS_SPWG_T1_MIN		1
#define LVDS_SPWG_T1_MAX		50
#define LVDS_SPWG_T2_MIN		200
#define LVDS_SPWG_T2_MAX		1023
#define LVDS_SPWG_T3_MIN		200
#define LVDS_SPWG_T3_MAX		1023
#define LVDS_SPWG_T4_MIN		1
#define LVDS_SPWG_T4_MAX		50
#define LVDS_SPWG_T5_MIN		200
#define LVDS_SPWG_T5_MAX		1023

// value of pwm_freq, Hz
#define IDX_PWM_FREQ_100Hz		100
#define IDX_PWM_FREQ_200Hz		200
#define IDX_PWM_FREQ_2KHz		2000
#define IDX_PWM_FREQ_4KHz		4000
#define IDX_PWM_FREQ_16KHz		16000
#define IDX_PWM_FREQ_32KHz		32000
#define IDX_PWM_FREQ_64KHz		64000
#define IDX_PWM_FREQ_128KHz		128000

typedef struct{
	uint8		bypass;			// indicate if bypass function enabled
	uint8		channel_swap;	// refer to spec: HD_LV_SEQ
	uint8		channel_pol;	// channel polarity
	uint8		hs_pol;			// hsync polarity
	uint8		vs_pol;			// vsync polarity
	uint8		de_pol;			// de polarity
	//externed at 2011.05.25
	uint8		pwm_duty;		// pwm duty-cycle	0~255
	uint8		pwm_inv;		// pwm inverter  POL_INVERT or POL_NO_INV
	uint32		pwm_freq;		// pwm frequency, which should be 100Hz, 200Hz, 2kHz, 4KHz, 16Khz, 32KHZ, 64KHz, 128KHz
	uint32		spwg_t1;		// power sequency of T1
	uint32		spwg_t2;		// power sequency of T2
	uint32		spwg_t3;		// power sequency of T3
	uint32		spwg_t4;		// power sequency of T4
	uint32		spwg_t5;		// power sequency of T5
}LVDS_FMT, *PLVDS_FMT;

// aspect ratio, These do not match the EDID encoding
#define AS_RATIO_NA			0	// support
#define AS_RATIO_4_3		1	// support
#define AS_RATIO_16_9		2	// support
#define AS_RATIO_16_10		3	// not support
#define AS_RATIO_5_4		4	// not support

//hdmi channel swap selection, description "channel_swap" in HDMI_FMT below:
//here only define default macro, for other condition, use number directly...
#define HDMI_CHANNEL_SWAP_DEF	0

typedef struct{
	uint8		bypass;			// indicate if bypass mode enabled
	uint8		is_dvi_mode;	// indicate if DVI mode required
	uint8		format_index;	// VIC index
	uint8		aspect_ratio;	// 1: 4:3 - 2: 16:9
	uint8		channel_swap;	// refer to "HD_LV_SEQ" in datasheet.
	uint8		data_pol_invert;// if invert the polarity of input of HDMI module, refer to HD_LV_POL in spec.
	uint8		hs_pol;			// HDMI output horizontal sync polarity
	uint8		vs_pol;			// HDMI output vertical sync polarity
}HDMI_FMT, *PHDMI_FMT;

//vga channel swap selection, descript "channel_swap" in VGA_FMT below:
//here only define default macro, for other condition, use number directly...
#define VGA_CHANNEL_SWAP_DEF	0

//vga sync swap selection, descript "sync_swap" in VGA_FMT below:
//here only define default macro, for other condition, use number directly...
#define VGA_SYNC_SWAP_DEF		0

#define VGA_CSYNC_XOR			0
#define VGA_CSYNC_OR			1
#define VGA_CSYNC_AND			2

typedef struct{
	uint8		bypass;			// indicate if bypass function enabled
	uint8		channel_swap;	// R G B swap control, refer to "DACSP" in datasheet
	uint8		sync_swap;		// sync swap control, refer to "SYNCS" in datasheet
	uint8		csync_type;		// CSync type
	uint8		hs_pol;			// horizontal sync polarity
	uint8		vs_pol;			// vetical sync polairy
	uint8		de_pol;			// Data enable signal polarity
	uint8		reserved;
}VGA_FMT, *PVGA_FMT;

//hdtv channel swap selection, descript "channel_swap" in HDTV_FMT below:
//here only define default macro, for other condition, use number directly...
#define HDTV_CHANNEL_SWAP_DEF	0

typedef struct{
	uint8		bypass;			// indicate if bypass function enabled
	uint8		format_index;	// refer to datasheet
	uint8		channel_swap;	// Y Pb Pr channel swap, refer to "DACSP" in datasheet
	uint8		reserved;
}HDTV_FMT, *PHDTV_FMT;

// channel bits, output channel
#define CHANNEL_LVDS		0x01
#define CHANNEL_HDMI		0x02
#define CHANNEL_VGA			0x04
#define CHANNEL_HDTV		0x08
#define CHANNEL_VGA1		0x10
// channel bits, input channel
#define CHANNEL_INPUTLOST	0x20

#define ROTATE_NO			0
#define ROTATE_90			1
#define ROTATE_180			2
#define ROTATE_270			3

#define DOWN_SCALING_MAX	80		// -+wyc, [0..80]. 0=100%, no down scaling; 80=down scale to 20% in size

typedef struct{
	CH703x_TIMING	timing;			// Timing
	uint32			uclk_khz;		// output clock frequency
	uint8			channel;		// combined channel bits
	uint8			reserved1[3];
	LVDS_FMT		lvds_fmt;		// LVDS special format if output to LVDS
	HDMI_FMT		hdmi_fmt;		// HDMI special format if output to LVDS
	VGA_FMT			vga_fmt;		// VGA special format if output to LVDS
	HDTV_FMT		hdtv_fmt;		// HDTV special format  if output to LVDS

	//	features
	uint8			ds_percent_h;	// down scaling percent for horizontal direction
	uint8			ds_percent_v;	// down scaling percent for vertical direction
	uint8			rotate;			// Rotation control
	uint8			h_flip;			// Horizontal flip control
	uint8			v_flip;			// Vertical flip control
	uint8			reserved2[3];
}CH703x_OUTPUT_INFO, *PCH703x_OUTPUT_INFO;

// get which panel's EDID, currently only support reading EDID from HDMI or VGA
#define  GET_VGA_EDID		0x80
#define  GET_HDMI_EDID		0x00
#define  GET_EDID_MASK		0x80

//--------------- DEVICE INFORMATION -----------------------
typedef enum{
	DEV_CH7033E = 0,
	DEV_CH7034E,
	DEV_CH7035E,

	DEV_CH7033F,
	DEV_CH7034F,
	DEV_CH7035F,

	DEV_UNKNOW,		// always be the last
}CH703x_DEV_TYPE;

typedef struct _FW703X_CFG {
  unsigned char size; 		// total size <=16 bytes, currently 7 bytes
  unsigned char ver_major;	// MCU firmware version
  unsigned char ver_minor;
  unsigned char did;		// device id
  unsigned char rid;		// revision id
  unsigned char capbility; 	// Firmware capability;
  unsigned char reserved;
  unsigned char reserve2;
} FW703X_CFG;

//------------------ Interface --------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// open I2C device before calling any of the following functions

// Enter:
//	0..DEV_UNKNOW-1:	check whether the specified device present
//		 DEV_UNKNOW:	check which device present
// Return:
//	0..DEV_UNKNOW-1:	the device present
//		 DEV_UNKNOW:	not found device present
CH703x_DEV_TYPE  ch_check_chip(CH703x_DEV_TYPE chip);

// start firmware before calling any other functions
int32   ch_start_firmware(CH703x_DEV_TYPE chip);

// get MCU information
int32   ch_getMcuVersion(CH703x_DEV_TYPE chip, FW703X_CFG *cfg);

// load the firmware of ch703x
int32 	ch_load_firmware(CH703x_DEV_TYPE chip, uint8 *fw, uint16 size);

// set ch703x working mode
// this function also sets these features: scaling down, rotation or flip
int32   ch_set_mode(CH703x_DEV_TYPE chip, CH703x_INPUT_INFO *in,
			CH703x_OUTPUT_INFO *out);

// force ch703x to output color bar(generated by itself) instead of the input.
// ch_set_mode() should be successfully called before calling this function.
// test only.
int32   ch_set_test_mode(CH703x_DEV_TYPE chip);

// get edid from VGA pannel or HDMI pannel
// Enter:
// 		 buf:  buffer to store EDID, >= 256 bytes, 512 bytes recommended.
// 		 len:  buffer length
// whichEDID: GET_VGA_EDID or GET_HDMI_EDID
int32   ch_get_edid(CH703x_DEV_TYPE chip, uint8 *buf, uint16 len, uint8 whichEDID);

#define VGA_HOTPLUG_DELAY		50	// unit ms
// For output channel, currently only support HDMI and VGA hotplg detecting,
// could not detect HDTV, VGA1, LVDS hotplug.
// For input channel, currently support input channel lost detecting.
// There is a delay when hardware detecting VGA connection, VGA_HOTPLUG_DELAY
// is workable. If you periodically detect hotplug, you can specify the delay
// to 0. It is not effective for firmware detecting.
// Return:
//		channel bits to indicate which outputs are connected and
//      whether input lost.
uint8   ch_check_connection(CH703x_DEV_TYPE chip, int delay);
int32   ch_set_plug_out_power(CH703x_DEV_TYPE chip, uint8 enable);
// Enter
// channel_bits: combined output channel bits
// 		1: power on  this output channel
// 		0: power off this output channel
int32   ch_set_channel_power(CH703x_DEV_TYPE chip, uint8 channel_bits);

// Enter:
// channel_bits: combined output channel bits
// 		0x00:  force chip to enter standby state to save power.
// 		other: wake up chip and power on the specified output channel,
//		  	   then you must call ch_set_mode to setup chip again
int32   ch_set_chip_power(CH703x_DEV_TYPE chip, uint8 channel_bits);

// dither: 0 - disable; 1 - enable
int32   ch_set_dither(CH703x_DEV_TYPE chip, uint8 dither);

// Enter:
//		pwm_duty:	same as pwm_duty in LVDS_FMT, 0~255
// Return:  old setting
uint8	ch_set_lvds_brightness(CH703x_DEV_TYPE chip, uint8 pwm_duty);

#define MCUAUTO_SETMODE		0x40
// currently you can only enable/disable MCU auto set display mode function.
// auto ouput channel hotplug and input channel lost detection are always
// enabled. If you enable MCU auto function, when output channel hotplug,
// MCU will automatically close/display to this output channel.
// Enter:
// 		mcuAutoBits: 	combined bits to enable MCU auto functions
// Return:	old setting
uint8	ch_enable_MCU_AutoFunc(CH703x_DEV_TYPE chip, uint8 mcuAutoBits);

// -------- lvds_bypass_mode, add on 2011-11-11 ----------

// Enter:
//		current:  bit 7,    0 - enable, 1 - disable
//				  bi7 2..0, 0 ~ 7 set current to
//			0 - 2.45mA,  1 - 2.8mA,  2 - 3.15mA,  3 - 3.5mA(default)
//			4 - 3.85mA,  5 - 4.2mA,  6 - 4.55mA,  7 - 4.9mA
// Note:
//		only set this when the output is lvds bypass mode
//		some settings may cause panel fail to power on 
// Return:  old setting
//uint8	ch_set_lvds_current(CH703x_DEV_TYPE chip, uint8 current);

// Enter:
//		bOn:	0 - Off; 1 - On
// Return:  old setting
uint8	ch_set_lvds_common_voltage(CH703x_DEV_TYPE chip, uint8 bOn);

// MTK-Add
void ch_dump_reg(void);
void ch_read_reg(unsigned char u8Reg);
void ch_write_reg(unsigned char u8Reg, unsigned char u8Data);
void hw_set_HPD_MAX(uint8 addr);
void hw_set_inv_clk(uint8 inv);
void hw_set_suspend(void);
void hw_set_resume(void);

#ifdef __cplusplus
}
#endif

#endif
