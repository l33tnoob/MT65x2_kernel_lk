/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov8835mipiraw_Sensor.h"
#include "ov8835mipiraw_Camera_Sensor_para.h"
#include "ov8835mipiraw_CameraCustomized.h"

// #define OV8835_TEMPERATURE_TEST // Jiangde

#ifdef OV8835MIPI_DEBUG
//#define SENSORDB printk
#define SENSORDB(fmt, arg...) printk( "[OV8835MIPIRaw] "  fmt, ##arg)
#else
#define SENSORDB(x,...)
#endif
//#define ACDK
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

//#define OV8835MIPI_PREVIEW_CLK 138666667  //65000000
//#define OV8835MIPI_CAPTURE_CLK 134333333  //117000000  //69333333
//#define OV8835MIPI_DEBUG_SETTING
#ifdef OV8835MIPI_DEBUG_SETTING
#define OV8835MIPI_PREVIEW_CLK 138666667  //65000000
#define OV8835MIPI_CAPTURE_CLK 134333333  //117000000  //69333333
#define OV8835MIPI_ZSD_PRE_CLK 134333333 //117000000  
#define OV8835MIPI_VIDEO_CLK 134333333 //117000000  
#else
#ifndef OV8835MIPI_4LANE
#define OV8835MIPI_PREVIEW_CLK 143000000  //65000000
#define OV8835MIPI_CAPTURE_CLK 143000000  //117000000  //69333333
#define OV8835MIPI_ZSD_PRE_CLK 143000000 //117000000
#define OV8835MIPI_VIDEO_CLK 143000000 //117000000
#else
#ifdef OV8835MIPI_4LANE_CAP_30FPS
#define OV8835MIPI_PREVIEW_CLK 208000000  //65000000
#define OV8835MIPI_CAPTURE_CLK 273000000  //117000000  //69333333
#define OV8835MIPI_ZSD_PRE_CLK 273000000 //117000000
#define OV8835MIPI_VIDEO_CLK 273000000 //117000000
#else

    #define OV8835MIPI_PREVIEW_CLK 208000000  //65000000
#ifdef USE_24FPS_SETTING // LINE <><2013.10.31><Use 24FPS to test blc> Jiangde
    #define OV8835MIPI_CAPTURE_CLK 216666667  
    #define OV8835MIPI_ZSD_PRE_CLK 216666667 // test for 24fps
#else
    #define OV8835MIPI_CAPTURE_CLK 260000000  //117000000  //69333333 
    #define OV8835MIPI_ZSD_PRE_CLK 260000000 //117000000
#endif
    #define OV8835MIPI_VIDEO_CLK 260000000 //117000000 Jiangde video 1/3
    
#endif
#endif
#endif

//#define OV8835MIPI_ZSD_PRE_CLK 134333333 //117000000
#define OV8835MIPI_TEST_PATTERN_CHECKSUM (0x8974faf0)

MSDK_SCENARIO_ID_ENUM OV8835MIPIMIPIRAWCurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;//ACDK_SCENARIO_ID_CAMERA_PREVIEW;

kal_uint32 OV8835MIPI_FeatureControl_PERIOD_PixelNum=OV8835MIPI_PV_PERIOD_PIXEL_NUMS;
kal_uint32 OV8835MIPI_FeatureControl_PERIOD_LineNum=OV8835MIPI_PV_PERIOD_LINE_NUMS;

static OV8835MIPI_sensor_struct OV8835MIPI_sensor =
{
  .eng =
  {
    .reg = CAMERA_SENSOR_REG_DEFAULT_VALUE,
    .cct = CAMERA_SENSOR_CCT_DEFAULT_VALUE,
  },
  .eng_info =
  {
    .SensorId = 128,
    .SensorType = CMOS_SENSOR,
    .SensorOutputDataFormat = OV8835MIPI_COLOR_FORMAT,
  },
  .shutter = 0x20,  
  .gain = 0x20,
  .pv_pclk = OV8835MIPI_PREVIEW_CLK,
  .cap_pclk = OV8835MIPI_CAPTURE_CLK,
  .video_pclk = OV8835MIPI_VIDEO_CLK,
  .frame_length = OV8835MIPI_PV_PERIOD_LINE_NUMS,
  .line_length = OV8835MIPI_PV_PERIOD_PIXEL_NUMS,
  .is_zsd = KAL_FALSE, //for zsd
  .dummy_pixels = 0,
  .dummy_lines = 0,  //for zsd
  .is_autofliker = KAL_FALSE,
  .sensorMode = OV8835MIPI_SENSOR_MODE_INIT,
};

static DEFINE_SPINLOCK(ov8835mipi_drv_lock);

kal_uint16 OV8835MIPI_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,OV8835MIPI_sensor.write_id);
#ifdef OV8835MIPI_DRIVER_TRACE
	//SENSORDB("OV8835MIPI_read_cmos_sensor, addr:%x;get_byte:%x \n",addr,get_byte);
#endif		
    return get_byte;
}


kal_uint16 OV8835MIPI_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    //kal_uint16 reg_tmp;
	
    char puSendCmd[3] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 3,OV8835MIPI_sensor.write_id);

	//SENSORDB("OV8835MIPI_write_cmos_sensor, addr:%x;get_byte:%x \n",addr,para);

	//for(i=0;i<0x100;i++)
	//	{
			
	//	}
	
	//reg_tmp = OV8835MIPI_read_cmos_sensor(addr);

	//SENSORDB("OV8835MIPI_read_cmos_sensor, addr:%x;get_byte:%x \n",addr,reg_tmp);
	return 0;
}


#define OV8835MIPI_USE_OTP //LINE <> <DATE20130625> <truly ov8835 OTP> wupingzhou

#ifdef OV8835MIPI_USE_OTP //LINE <> <DATE20130625> <truly ov8835 OTP> wupingzhou

#define OTP_DATA_ADDR         0x3D00
#define OTP_LOAD_ADDR         0x3D81
#define OTP_BANK_ADDR         0x3D84

#define LENC_START_ADDR       0x5800
#define LENC_REG_SIZE         62

#define OTP_LENC_GROUP_ADDR   0x3D00

#define OTP_WB_GROUP_ADDR     0x3D00
#define OTP_WB_GROUP_SIZE     16

#define GAIN_RH_ADDR          0x3400
#define GAIN_RL_ADDR          0x3401
#define GAIN_GH_ADDR          0x3402
#define GAIN_GL_ADDR          0x3403
#define GAIN_BH_ADDR          0x3404
#define GAIN_BL_ADDR          0x3405

#define GAIN_DEFAULT_VALUE    0x0400 // 1x gain

#define OTP_MID               0x02


// R/G and B/G of current camera module
static unsigned short rg_ratio = 0;
static unsigned short bg_ratio = 0;

static unsigned char otp_lenc_data[62];


// Enable OTP read function
static void otp_read_enable(void)
{
	OV8835MIPI_write_cmos_sensor(OTP_LOAD_ADDR, 0x01);
	mdelay(15); // sleep > 10ms
}

// Disable OTP read function
static void otp_read_disable(void)
{
	OV8835MIPI_write_cmos_sensor(OTP_LOAD_ADDR, 0x00);
	mdelay(15); // sleep > 10ms
}

static void otp_read(unsigned short otp_addr, unsigned char* otp_data)
{
	otp_read_enable();
	*otp_data = OV8835MIPI_read_cmos_sensor(otp_addr);
	otp_read_disable();
}

/*******************************************************************************
* Function    :  otp_clear
* Description :  Clear OTP buffer 
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
static void otp_clear(void)
{
	// After read/write operation, the OTP buffer should be cleared to avoid accident write
	unsigned char i;
	for (i=0; i<16; i++) 
	{
		OV8835MIPI_write_cmos_sensor(OTP_DATA_ADDR+i, 0x00);
	}
}

/*******************************************************************************
* Function    :  otp_check_wb_group
* Description :  Check OTP Space Availability
* Parameters  :  [in] index : index of otp group (0, 1, 2)
* Return      :  0, group index is empty
                 1, group index has invalid data
                 2, group index has valid data
                -1, group index error
*******************************************************************************/	
static signed char otp_check_wb_group(unsigned char index)
{   
	unsigned char  flag;

    if (index > 2)
	{
		SENSORDB("OTP input wb group index %d error\n", index);
		return -1;
	}
		
	// select bank 1-3
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (index+1));

	otp_read(OTP_WB_GROUP_ADDR, &flag);
	otp_clear();

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	
	flag &= 0xc0;
	if (!flag)
	{
		SENSORDB("wb group %d is empty\n", index);
		return 0;
	}
	else if (flag == 0x40)
	{
		SENSORDB("wb group %d has valid data\n", index);
		return 2;
	}
	else
	{
		SENSORDB("wb group %d has invalid data\n", index);
		return 1;
	}
}

/*******************************************************************************
* Function    :  otp_read_wb_group
* Description :  Read group value and store it in OTP Struct 
* Parameters  :  [in] index : index of otp group (0, 1, 2)
* Return      :  group index (0, 1, 2)
                 -1, error
*******************************************************************************/	
static signed char otp_read_wb_group(signed char index)
{
	unsigned char  mid, AWB_light_LSB, rg_ratio_MSB, bg_ratio_MSB;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_wb_group(index) == 2)
			{
				SENSORDB("read wb from group %d\n", index);
				break;
			}
		}

		if (index > 2)
		{
			SENSORDB("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_wb_group(index) != 2)
		{
			SENSORDB("read wb from group %d failed\n", index);
			return -1;
		}
	}


	// select bank 1-3
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (index+1));

	otp_read(OTP_WB_GROUP_ADDR+1, &mid);
	if (mid != OTP_MID)
	{
		return -1;
	}

	otp_read(OTP_WB_GROUP_ADDR+6, &rg_ratio_MSB);
	otp_read(OTP_WB_GROUP_ADDR+7, &bg_ratio_MSB);
	otp_read(OTP_WB_GROUP_ADDR+10, &AWB_light_LSB);	
	otp_clear();
	rg_ratio = (rg_ratio_MSB<<2) | ((AWB_light_LSB & 0xC0)>>6);
	bg_ratio = (bg_ratio_MSB<<2) | ((AWB_light_LSB & 0x30)>>4);
	SENSORDB("rg_ratio =0x%x\n",rg_ratio);
	SENSORDB("bg_ratio =0x%x\n",bg_ratio);
	SENSORDB("read wb finished\n");
	return index;
}

#ifdef SUPPORT_FLOATING //Use this if support floating point values
/*******************************************************************************
* Function    :  otp_apply_wb
* Description :  Calcualte and apply R, G, B gain to module
* Parameters  :  [in] golden_rg : R/G of golden camera module
                 [in] golden_bg : B/G of golden camera module
* Return      :  1, success; 0, fail
*******************************************************************************/	
static bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;

	double ratio_r, ratio_g, ratio_b;
	double cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		SENSORDB("golden_rg / golden_bg can not be zero\n");
		return 0;
	}

	// Calcualte R, G, B gain of current module from R/G, B/G of golden module
        // and R/G, B/G of current module
	cmp_rg = 1.0 * rg_ratio / golden_rg;
	cmp_bg = 1.0 * bg_ratio / golden_bg;

	if ((cmp_rg<1) && (cmp_bg<1))
	{
		// R/G < R/G golden, B/G < B/G golden
		ratio_g = 1;
		ratio_r = 1 / cmp_rg;
		ratio_b = 1 / cmp_bg;
	}
	else if (cmp_rg > cmp_bg)
	{
		// R/G >= R/G golden, B/G < B/G golden
		// R/G >= R/G golden, B/G >= B/G golden
		ratio_r = 1;
		ratio_g = cmp_rg;
		ratio_b = cmp_rg / cmp_bg;
	}
	else
	{
		// B/G >= B/G golden, R/G < R/G golden
		// B/G >= B/G golden, R/G >= R/G golden
		ratio_b = 1;
		ratio_g = cmp_bg;
		ratio_r = cmp_bg / cmp_rg;
	}

	// write sensor wb gain to registers
	// 0x0400 = 1x gain
	if (ratio_r != 1)
	{
		gain_r = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_r);
		OV8835MIPI_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV8835MIPI_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1)
	{
		gain_g = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_g);
		OV8835MIPI_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV8835MIPI_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1)
	{
		gain_b = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_b);
		OV8835MIPI_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV8835MIPI_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	SENSORDB("cmp_rg=%f, cmp_bg=%f\n", cmp_rg, cmp_bg);
	SENSORDB("ratio_r=%f, ratio_g=%f, ratio_b=%f\n", ratio_r, ratio_g, ratio_b);
	SENSORDB("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}

#else //Use this if not support floating point values

#define OTP_MULTIPLE_FAC	10000
static bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;

	unsigned short ratio_r, ratio_g, ratio_b;
	unsigned short cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		SENSORDB("golden_rg / golden_bg can not be zero\n");
		return 0;
	}

	// Calcualte R, G, B gain of current module from R/G, B/G of golden module
    // and R/G, B/G of current module
	cmp_rg = OTP_MULTIPLE_FAC * rg_ratio / golden_rg;
	cmp_bg = OTP_MULTIPLE_FAC * bg_ratio / golden_bg;

	if ((cmp_rg < 1 * OTP_MULTIPLE_FAC) && (cmp_bg < 1 * OTP_MULTIPLE_FAC))
	{
		// R/G < R/G golden, B/G < B/G golden
		ratio_g = 1 * OTP_MULTIPLE_FAC;
		ratio_r = 1 * OTP_MULTIPLE_FAC * OTP_MULTIPLE_FAC / cmp_rg;
		ratio_b = 1 * OTP_MULTIPLE_FAC * OTP_MULTIPLE_FAC / cmp_bg;
	}
	else if (cmp_rg > cmp_bg)
	{
		// R/G >= R/G golden, B/G < B/G golden
		// R/G >= R/G golden, B/G >= B/G golden
		ratio_r = 1 * OTP_MULTIPLE_FAC;
		ratio_g = cmp_rg;
		ratio_b = OTP_MULTIPLE_FAC * cmp_rg / cmp_bg;
	}
	else
	{
		// B/G >= B/G golden, R/G < R/G golden
		// B/G >= B/G golden, R/G >= R/G golden
		ratio_b = 1 * OTP_MULTIPLE_FAC;
		ratio_g = cmp_bg;
		ratio_r = OTP_MULTIPLE_FAC * cmp_bg / cmp_rg;
	}

	// write sensor wb gain to registers
	// 0x0400 = 1x gain
	if (ratio_r != 1 * OTP_MULTIPLE_FAC)
	{
		gain_r = GAIN_DEFAULT_VALUE * ratio_r / OTP_MULTIPLE_FAC;
		OV8835MIPI_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV8835MIPI_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1 * OTP_MULTIPLE_FAC)
	{
		gain_g = GAIN_DEFAULT_VALUE * ratio_g / OTP_MULTIPLE_FAC;
		OV8835MIPI_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV8835MIPI_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1 * OTP_MULTIPLE_FAC)
	{
		gain_b = GAIN_DEFAULT_VALUE * ratio_b / OTP_MULTIPLE_FAC;
		OV8835MIPI_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV8835MIPI_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	SENSORDB("cmp_rg=%d, cmp_bg=%d\n", cmp_rg, cmp_bg);
	SENSORDB("ratio_r=%d, ratio_g=%d, ratio_b=%d\n", ratio_r, ratio_g, ratio_b);
	SENSORDB("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}
#endif /* SUPPORT_FLOATING */

/*******************************************************************************
* Function    :  otp_update_wb
* Description :  Update white balance settings from OTP
* Parameters  :  [in] golden_rg : R/G of golden camera module
                 [in] golden_bg : B/G of golden camera module
* Return      :  1, success; 0, fail
*******************************************************************************/	
static bool otp_update_wb(unsigned short golden_rg, unsigned short golden_bg) 
{
	SENSORDB("start wb update\n");

	if (otp_read_wb_group(-1) != -1)
	{
		if (otp_apply_wb(golden_rg, golden_bg) == 1)
		{
			SENSORDB("wb update finished\n");
			return 1;
		}
	}

	SENSORDB("wb update failed\n");
	return 0;
}

/*******************************************************************************
* Function    :  otp_check_lenc_group
* Description :  Check OTP Space Availability
* Parameters  :  [in] BYTE index : index of otp group (0, 1, 2)
* Return      :  0, group index is empty
                 1, group index has invalid data
                 2, group index has valid data
                -1, group index error
*******************************************************************************/	
static signed char otp_check_lenc_group(BYTE index)
{   
	unsigned char  flag;
	unsigned char  bank;

    if (index > 2)
	{
	//	SENSORDB("OTP input lenc group index %d error\n", index);
		return -1;
	}
		
	// select bank: index*4 + 4
	bank = 0xc0 | (index*4 + 4);
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_read(OTP_LENC_GROUP_ADDR, &flag);
	otp_clear();

	flag &= 0xc0;

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	if (!flag)
	{
	//	SENSORDB("lenc group %d is empty\n", index);
		return 0;
	}
	else if (flag == 0x40)
	{
	//	SENSORDB("lenc group %d has valid data\n", index);
		return 2;
	}
	else
	{
	//	SENSORDB("lenc group %d has invalid data\n", index);
		return 1;
	}
}

/*******************************************************************************
* Function    :  otp_read_lenc_group
* Description :  Read group value and store it in OTP Struct 
* Parameters  :  [in] int index : index of otp group (0, 1, 2)
* Return      :  group index (0, 1, 2)
                 -1, error
*******************************************************************************/	
static signed char otp_read_lenc_group(int index)
{
	unsigned short otp_addr;
	unsigned char  bank;
	unsigned char  i;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_lenc_group(index) == 2)
			{
				SENSORDB("read lenc from group %d\n", index);
				break;
			}
		}

		if (index > 2)
		{
			SENSORDB("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_lenc_group(index) != 2) 
		{
			SENSORDB("read lenc from group %d failed\n", index);
			return -1;
		}
	}

	// select bank: index*4 + 4
	bank = 0xc0 | (index*4 + 4);
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR+1;

	otp_read_enable();
	for (i=0; i<15; i++) 
	{
		otp_lenc_data[i] = OV8835MIPI_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();

	// select next bank
	bank++;
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=15; i<31; i++) 
	{
		otp_lenc_data[i] = OV8835MIPI_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	// select next bank
	bank++;
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=31; i<47; i++) 
	{
		otp_lenc_data[i] = OV8835MIPI_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	// select next bank
	bank++;
	OV8835MIPI_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=47; i<62; i++) 
	{
		otp_lenc_data[i] = OV8835MIPI_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	SENSORDB("read lenc finished\n");
	return index;
}

/*******************************************************************************
* Function    :  otp_apply_lenc
* Description :  Apply lens correction setting to module
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
static void otp_apply_lenc(void)
{
	// write lens correction setting to registers
	SENSORDB("apply lenc setting\n");

	unsigned char i;


	for (i=0; i<LENC_REG_SIZE; i++)
	{
		OV8835MIPI_write_cmos_sensor(LENC_START_ADDR+i, otp_lenc_data[i]);
		SENSORDB("0x%x, 0x%x\n", LENC_START_ADDR+i, otp_lenc_data[i]);
	}
	OV8835MIPI_write_cmos_sensor(0x5000, 0x86); //LENC on //LINE <> <DATE20130822> <ov8835 lenc on> wupingzhou
}

/*******************************************************************************
* Function    :  otp_update_lenc
* Description :  Get lens correction setting from otp, then apply to module
* Parameters  :  none
* Return      :  1, success; 0, fail
*******************************************************************************/	
static bool otp_update_lenc(void) 
{
	SENSORDB("start lenc update\n");

	if (otp_read_lenc_group(-1) != -1)
	{
		otp_apply_lenc();
		SENSORDB("lenc update finished\n");
		return 1;
	}

	SENSORDB("lenc update failed\n");
	return 0;
}

/*******************************************************************************
* Function    :  otp_update_BLC
* Description :  Get BLC value from otp, then apply to module
* Parameters  :  none
* Return      :  0, set to 0x20; 1, use data from 0x3D0A; 2,use data from 0x3D0B
*******************************************************************************/
static int otp_update_BLC(void) 
{
	unsigned char  k;
	unsigned char  temp;
	//select bank 31
	OV8835MIPI_write_cmos_sensor(0x3d84, 0xdf);
	otp_read(0x3d0b, &k);
	if(k!=0)
	{
		if((k>=0x15) && (k<=0x40))
		{
			//auto load mode
			temp = OV8835MIPI_read_cmos_sensor(0x4008);
			temp &=0xfb;
   			OV8835MIPI_write_cmos_sensor(0x4008, temp);
   			mdelay(20); 
    			temp = OV8835MIPI_read_cmos_sensor(0x4000);
			temp &=0xf7;
   			OV8835MIPI_write_cmos_sensor(0x4000, temp);
   			mdelay(20); 
   			SENSORDB("BLC value from 0x3d0b: %x\n",k);
   			return 2;
   		}
	}
	
	otp_read(0x3d0a, &k);
	
	if((k>=0x10) && (k<=0x40))
	{
		//manual load mode
		OV8835MIPI_write_cmos_sensor(0x4006, k);
		temp = OV8835MIPI_read_cmos_sensor(0x4008);
		temp &=0xfb;
   		OV8835MIPI_write_cmos_sensor(0x4008, temp);
   		mdelay(20); 
    		temp = OV8835MIPI_read_cmos_sensor(0x4000);
		temp |=0x08;
   		OV8835MIPI_write_cmos_sensor(0x4000, temp);
   		mdelay(20); 
   		SENSORDB("BLC value from 0x3d0a:%x\n",k);
   		return 1;	
	}
	else
	{
		//set to default
		OV8835MIPI_write_cmos_sensor(0x4006, 0x20);
		temp = OV8835MIPI_read_cmos_sensor(0x4008);
		temp &=0xfb;
   		OV8835MIPI_write_cmos_sensor(0x4008, temp);
   		mdelay(20); 
    		temp = OV8835MIPI_read_cmos_sensor(0x4000);
		temp |=0x08;
   		OV8835MIPI_write_cmos_sensor(0x4000, temp);
   		mdelay(20); 
   		SENSORDB("BLC value is set to 0x20\n");
	}
	
	return 0; 	
}




#endif


static void OV8835MIPI_Set_Dummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
	kal_uint16 line_length, frame_length;
	unsigned long flags;
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Set_Dummy:iPixels:%x; iLines:%x \n",iPixels,iLines);
#endif
//return;
	if ( OV8835MIPI_SENSOR_MODE_PREVIEW == OV8835MIPI_sensor.sensorMode )	//SXGA size output
	{
		line_length = OV8835MIPI_PV_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV8835MIPI_PV_PERIOD_LINE_NUMS + iLines;
	}
	else if( OV8835MIPI_SENSOR_MODE_VIDEO == OV8835MIPI_sensor.sensorMode )
	{
		line_length = OV8835MIPI_VIDEO_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV8835MIPI_VIDEO_PERIOD_LINE_NUMS + iLines;
	}
	else if( OV8835MIPI_SENSOR_MODE_CAPTURE == OV8835MIPI_sensor.sensorMode )
	{
		line_length = OV8835MIPI_FULL_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV8835MIPI_FULL_PERIOD_LINE_NUMS + iLines;
	}
	else
	{
		line_length = OV8835MIPI_PV_PERIOD_PIXEL_NUMS + iPixels;
		frame_length = OV8835MIPI_PV_PERIOD_LINE_NUMS + iLines;
	}

	if ((line_length >= 0x1FFF)||(frame_length >= 0xFFF)) {
		SENSORDB("[soso][OV8835MIPI_Set_Dummy] Error: line_length=%d, frame_length = %d \n",line_length, frame_length);
	}

	if(frame_length < (OV8835MIPI_sensor.shutter+14))
	{
		frame_length = OV8835MIPI_sensor.shutter +14;
	}
	
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("line_length:%x; frame_length:%x \n",line_length,frame_length);
#endif

//	if ((line_length >= 0x1FFF)||(frame_length >= 0xFFF))
//	{
//#ifdef OV8835MIPI_DRIVER_TRACE
//		SENSORDB("Warnning: line length or frame height is overflow!!!!!!!!  \n");
//#endif
//		return ;
//	}


//	if((line_length == OV8835MIPI_sensor.line_length)&&(frame_length == OV8835MIPI_sensor.frame_length))
//		return ;
	spin_lock_irqsave(&ov8835mipi_drv_lock,flags);

	OV8835MIPI_sensor.line_length = line_length;
	OV8835MIPI_sensor.frame_length = frame_length;
	//OV8835MIPI_sensor.dummy_pixels = iPixels;
	//OV8835MIPI_sensor.dummy_lines = iLines;
	spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);

	SENSORDB("line_length:%x; frame_length:%x \n",line_length,frame_length);
	
    /*  Add dummy pixels: */
    /* 0x380c [0:4], 0x380d defines the PCLKs in one line of OV8835MIPI  */  
    /* Add dummy lines:*/
    /* 0x380e [0:1], 0x380f defines total lines in one frame of OV8835MIPI */
    OV8835MIPI_write_cmos_sensor(0x380c, line_length >> 8);
    OV8835MIPI_write_cmos_sensor(0x380d, line_length & 0xFF);
    OV8835MIPI_write_cmos_sensor(0x380e, frame_length >> 8);
    OV8835MIPI_write_cmos_sensor(0x380f, frame_length & 0xFF);
	
}   /*  OV8835MIPI_Set_Dummy    */

UINT32 OV8835MIPISetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("bEnable=%d\n", bEnable);
	if(bEnable) 
	{
        OV8835MIPI_write_cmos_sensor(0x5E00,0x80);
	}
	else        
	{
		OV8835MIPI_write_cmos_sensor(0x5E00,0x00);	
	}
    return ERROR_NONE;
}

static UINT32 OV8835MIPISetMaxFrameRate(UINT16 u2FrameRate)
{
	kal_int16 dummy_line=0;
	kal_uint16 frame_length = OV8835MIPI_sensor.frame_length;
	unsigned long flags;
		
	SENSORDB("[soso][OV8835MIPISetMaxFrameRate]u2FrameRate=%d \n",u2FrameRate);

	frame_length= (10*OV8835MIPI_sensor.pclk) / u2FrameRate / OV8835MIPI_sensor.line_length;

    /*
	if(KAL_FALSE == OV8835MIPI_sensor.pv_mode){
		if(frame_length < OV8835MIPI_FULL_PERIOD_LINE_NUMS)
			frame_length = OV8835MIPI_FULL_PERIOD_LINE_NUMS;		
	}
    */

		spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
		OV8835MIPI_sensor.frame_length = frame_length;
		spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);
	
		if((OV8835MIPIMIPIRAWCurrentScenarioId == MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG)||(OV8835MIPIMIPIRAWCurrentScenarioId == MSDK_SCENARIO_ID_CAMERA_ZSD)){
			dummy_line = frame_length - OV8835MIPI_FULL_PERIOD_LINE_NUMS;
		}
		else if(OV8835MIPIMIPIRAWCurrentScenarioId==MSDK_SCENARIO_ID_CAMERA_PREVIEW){
			dummy_line = frame_length - OV8835MIPI_PV_PERIOD_LINE_NUMS;
		}
		else if(OV8835MIPIMIPIRAWCurrentScenarioId==MSDK_SCENARIO_ID_VIDEO_PREVIEW) {
			dummy_line = frame_length - OV8835MIPI_VIDEO_PERIOD_LINE_NUMS;
		}
		else
			dummy_line = frame_length - OV8835MIPI_PV_PERIOD_LINE_NUMS;
		SENSORDB("[soso][OV8835MIPISetMaxFrameRate]frame_length = %d, dummy_line=%d \n",OV8835MIPI_sensor.frame_length,dummy_line);
		if(dummy_line<0) {
			dummy_line = 0;
		}
	    /* to fix VSYNC, to fix frame rate */
		OV8835MIPI_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
	//}
	return (UINT32)u2FrameRate;
}


void OV8835MIPI_Write_Shutter(kal_uint16 ishutter)
{

kal_uint16 extra_shutter = 0;
kal_uint16 frame_length = 0;
kal_uint16 realtime_fp = 0;
kal_uint32 pclk = 0;
unsigned long flags;

//return;
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPI_write_shutter:%x \n",ishutter);
#endif
   


			SENSORDB("OV8835MIPI_sensor.is_autofliker:%x \n",OV8835MIPI_sensor.is_autofliker);
#if 0
	if(OV8835MIPI_sensor.is_autofliker == KAL_TRUE)
		{
		  		if(OV8835MIPI_sensor.is_zsd == KAL_TRUE)
		  			{
		  				pclk = OV8835MIPI_ZSD_PRE_CLK;
		  			}
				 else
				 	{
				 		pclk = OV8835MIPI_PREVIEW_CLK;
				 	}
					
				realtime_fp = pclk *10 / (OV8835MIPI_sensor.line_length * OV8835MIPI_sensor.frame_length);
			    SENSORDB("[OV8835MIPI_Write_Shutter]pv_clk:%d\n",pclk);
				SENSORDB("[OV8835MIPI_Write_Shutter]line_length:%d\n",OV8835MIPI_sensor.line_length);
				SENSORDB("[OV8835MIPI_Write_Shutter]frame_length:%d\n",OV8835MIPI_sensor.frame_length);
			    SENSORDB("[OV8835MIPI_Write_Shutter]framerate(10base):%d\n",realtime_fp);

				if((realtime_fp >= 297)&&(realtime_fp <= 303))
					{
						realtime_fp = 296;
						spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
						OV8835MIPI_sensor.frame_length = pclk *10 / (OV8835MIPI_sensor.line_length * realtime_fp);
						spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);

						SENSORDB("[autofliker realtime_fp=30,extern heights slowdown to 29.6fps][height:%d]",OV8835MIPI_sensor.frame_length);
					}
				else if((realtime_fp >= 147)&&(realtime_fp <= 153))
					{
						realtime_fp = 146;
						spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
						OV8835MIPI_sensor.frame_length = pclk *10 / (OV8835MIPI_sensor.line_length * realtime_fp);
						spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);
						
						SENSORDB("[autofliker realtime_fp=15,extern heights slowdown to 14.6fps][height:%d]",OV8835MIPI_sensor.frame_length);
					}
				//OV8835MIPI_sensor.frame_length = OV8835MIPI_sensor.frame_length +(OV8835MIPI_sensor.frame_length>>7);

		}
#endif
#if 1
	if(OV8835MIPI_sensor.is_autofliker == KAL_TRUE)
	{
		if(OV8835MIPI_sensor.video_mode == KAL_FALSE)
			{
				realtime_fp = OV8835MIPI_sensor.pclk *10 / (OV8835MIPI_sensor.line_length * OV8835MIPI_sensor.frame_length);
				SENSORDB("[OV8835MIPI_Write_Shutter]pv_clk:%d\n",pclk);
				SENSORDB("[OV8835MIPI_Write_Shutter]line_length:%d\n",OV8835MIPI_sensor.line_length);
				SENSORDB("[OV8835MIPI_Write_Shutter]frame_length:%d\n",OV8835MIPI_sensor.frame_length);
				SENSORDB("[OV8835MIPI_Write_Shutter]framerate(10base):%d\n",realtime_fp);
				
				if((realtime_fp >= 297)&&(realtime_fp <= 303))
				{
					realtime_fp = 296;
					OV8835MIPISetMaxFrameRate((UINT16)realtime_fp);
				}
				else if((realtime_fp >= 147)&&(realtime_fp <= 153))
				{
					realtime_fp = 146;
					OV8835MIPISetMaxFrameRate((UINT16)realtime_fp);
				}		
			}
	}
#endif
   	if (!ishutter) ishutter = 1; /* avoid 0 */

	spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
	frame_length = OV8835MIPI_sensor.max_exposure_lines;
	spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);

	if(ishutter > (frame_length -14))
	{
		extra_shutter = ishutter - frame_length + 14;
		SENSORDB("[shutter > frame_length] extra_shutter:%x \n", extra_shutter);
	}
	else
	{
		extra_shutter = 0;
	}
	spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
	OV8835MIPI_sensor.frame_length = frame_length+extra_shutter;
	spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);
	
	SENSORDB("OV8835MIPI_sensor.frame_length:%x\n",OV8835MIPI_sensor.frame_length);

    OV8835MIPI_write_cmos_sensor(0x380e, (OV8835MIPI_sensor.frame_length>>8)&0xFF);
	OV8835MIPI_write_cmos_sensor(0x380f, (OV8835MIPI_sensor.frame_length)&0xFF);

	
    OV8835MIPI_write_cmos_sensor(0x3500, (ishutter >> 12) & 0xF);
	OV8835MIPI_write_cmos_sensor(0x3501, (ishutter >> 4) & 0xFF);	
	OV8835MIPI_write_cmos_sensor(0x3502, (ishutter << 4) & 0xFF);

}




/*************************************************************************
* FUNCTION
*	OV8835MIPI_SetShutter
*
* DESCRIPTION
*	This function set e-shutter of OV8835MIPI to change exposure time.
*
* PARAMETERS
*   iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/


void set_OV8835MIPI_shutter(kal_uint16 iShutter)
{

	unsigned long flags;
	
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("set_OV8835MIPI_shutter:%x \n",iShutter);
#endif

    if((OV8835MIPI_sensor.pv_mode == KAL_FALSE)&&(OV8835MIPI_sensor.is_zsd == KAL_FALSE))
    	{
    	   SENSORDB("[set_OV8835MIPI_shutter]now is in 1/4size cap mode\n");
    	   //return;
    	}
	else if((OV8835MIPI_sensor.is_zsd == KAL_TRUE)&&(OV8835MIPI_sensor.is_zsd_cap == KAL_TRUE))
		{
			SENSORDB("[set_OV8835MIPI_shutter]now is in zsd cap mode\n");

			//SENSORDB("[set_OV8835MIPI_shutter]0x3500:%x\n",OV8835MIPI_read_cmos_sensor(0x3500));
			//SENSORDB("[set_OV8835MIPI_shutter]0x3500:%x\n",OV8835MIPI_read_cmos_sensor(0x3501));
			//SENSORDB("[set_OV8835MIPI_shutter]0x3500:%x\n",OV8835MIPI_read_cmos_sensor(0x3502));
			//return;
		}

/*	if(OV8835MIPI_sensor.shutter == iShutter)
		{
			SENSORDB("[set_OV8835MIPI_shutter]shutter is the same with previous, skip\n");
			return;
		}*/

	spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
	OV8835MIPI_sensor.shutter = iShutter;
	spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);
	
    OV8835MIPI_Write_Shutter(iShutter);

}   /*  Set_OV8835MIPI_Shutter */

 kal_uint16 OV8835MIPIGain2Reg(const kal_uint16 iGain)
{
    kal_uint16 iReg = 0x00;

	//iReg = ((iGain / BASEGAIN) << 4) + ((iGain % BASEGAIN) * 16 / BASEGAIN);
	iReg = iGain *16 / BASEGAIN;

	iReg = iReg & 0xFF;
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPIGain2Reg:iGain:%x; iReg:%x \n",iGain,iReg);
#endif
    return iReg;
}


kal_uint16 OV8835MIPI_SetGain(kal_uint16 iGain)
{
   kal_uint16 iReg;
   unsigned long flags;
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPI_SetGain:%x;\n",iGain);
#endif
/*   if(OV8835MIPI_sensor.gain == iGain)
   	{
   		SENSORDB("[OV8835MIPI_SetGain]:gain is the same with previous,skip\n");
	 	return 0;
   	}*/

   spin_lock_irqsave(&ov8835mipi_drv_lock,flags);
   OV8835MIPI_sensor.gain = iGain;
   spin_unlock_irqrestore(&ov8835mipi_drv_lock,flags);

  iReg = OV8835MIPIGain2Reg(iGain);
   
	  if (iReg < 0x10) //MINI gain is 0x10	 16 = 1x
	   {
		   iReg = 0x10;
	   }
   
	  else if(iReg > 0xFF) //max gain is 0xFF
		   {
			   iReg = 0xFF;
		   }
		  
	   //OV8835MIPI_write_cmos_sensor(0x350a, (iReg>>8)&0xFF);
	   OV8835MIPI_write_cmos_sensor(0x350b, iReg&0xFF);//only use 0x350b for gain control
	  return iGain;
}




/*************************************************************************
* FUNCTION
*	OV8835MIPI_SetGain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*   iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/

#if 0
void OV8835MIPI_set_isp_driving_current(kal_uint16 current)
{
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPI_set_isp_driving_current:current:%x;\n",current);
#endif
  //iowrite32((0x2 << 12)|(0<<28)|(0x8880888), 0xF0001500);
}
#endif

/*************************************************************************
* FUNCTION
*	OV8835MIPI_NightMode
*
* DESCRIPTION
*	This function night mode of OV8835MIPI.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV8835MIPI_night_mode(kal_bool enable)
{
}   /*  OV8835MIPI_NightMode    */


/* write camera_para to sensor register */
static void OV8835MIPI_camera_para_to_sensor(void)
{
  kal_uint32 i;
#ifdef OV8835MIPI_DRIVER_TRACE
	 SENSORDB("OV8835MIPI_camera_para_to_sensor\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV8835MIPI_sensor.eng.reg[i].Addr; i++)
  {
    OV8835MIPI_write_cmos_sensor(OV8835MIPI_sensor.eng.reg[i].Addr, OV8835MIPI_sensor.eng.reg[i].Para);
  }
  for (i = OV8835MIPI_FACTORY_START_ADDR; 0xFFFFFFFF != OV8835MIPI_sensor.eng.reg[i].Addr; i++)
  {
    OV8835MIPI_write_cmos_sensor(OV8835MIPI_sensor.eng.reg[i].Addr, OV8835MIPI_sensor.eng.reg[i].Para);
  }
  OV8835MIPI_SetGain(OV8835MIPI_sensor.gain); /* update gain */
}

/* update camera_para from sensor register */
static void OV8835MIPI_sensor_to_camera_para(void)
{
  kal_uint32 i;
  kal_uint32 temp_data;
  
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPI_sensor_to_camera_para\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV8835MIPI_sensor.eng.reg[i].Addr; i++)
  {
  	temp_data = OV8835MIPI_read_cmos_sensor(OV8835MIPI_sensor.eng.reg[i].Addr);

	spin_lock(&ov8835mipi_drv_lock);
    OV8835MIPI_sensor.eng.reg[i].Para = temp_data;
	spin_unlock(&ov8835mipi_drv_lock);
	
  }
  for (i = OV8835MIPI_FACTORY_START_ADDR; 0xFFFFFFFF != OV8835MIPI_sensor.eng.reg[i].Addr; i++)
  {
  	temp_data = OV8835MIPI_read_cmos_sensor(OV8835MIPI_sensor.eng.reg[i].Addr);
	
	spin_lock(&ov8835mipi_drv_lock);
    OV8835MIPI_sensor.eng.reg[i].Para = temp_data;
	spin_unlock(&ov8835mipi_drv_lock);
  }
}

/* ------------------------ Engineer mode ------------------------ */
inline static void OV8835MIPI_get_sensor_group_count(kal_int32 *sensor_count_ptr)
{
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPI_get_sensor_group_count\n");
#endif
  *sensor_count_ptr = OV8835MIPI_GROUP_TOTAL_NUMS;
}

inline static void OV8835MIPI_get_sensor_group_info(MSDK_SENSOR_GROUP_INFO_STRUCT *para)
{
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPI_get_sensor_group_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8835MIPI_PRE_GAIN:
    sprintf(para->GroupNamePtr, "CCT");
    para->ItemCount = 5;
    break;
  case OV8835MIPI_CMMCLK_CURRENT:
    sprintf(para->GroupNamePtr, "CMMCLK Current");
    para->ItemCount = 1;
    break;
  case OV8835MIPI_FRAME_RATE_LIMITATION:
    sprintf(para->GroupNamePtr, "Frame Rate Limitation");
    para->ItemCount = 2;
    break;
  case OV8835MIPI_REGISTER_EDITOR:
    sprintf(para->GroupNamePtr, "Register Editor");
    para->ItemCount = 2;
    break;
  default:
    ASSERT(0);
  }
}

inline static void OV8835MIPI_get_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{

  const static kal_char *cct_item_name[] = {"SENSOR_BASEGAIN", "Pregain-R", "Pregain-Gr", "Pregain-Gb", "Pregain-B"};
  const static kal_char *editer_item_name[] = {"REG addr", "REG value"};
  
#ifdef OV8835MIPI_DRIVER_TRACE
	 SENSORDB("OV8835MIPI_get_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8835MIPI_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV8835MIPI_SENSOR_BASEGAIN:
    case OV8835MIPI_PRE_GAIN_R_INDEX:
    case OV8835MIPI_PRE_GAIN_Gr_INDEX:
    case OV8835MIPI_PRE_GAIN_Gb_INDEX:
    case OV8835MIPI_PRE_GAIN_B_INDEX:
      break;
    default:
      ASSERT(0);
    }
    sprintf(para->ItemNamePtr, cct_item_name[para->ItemIdx - OV8835MIPI_SENSOR_BASEGAIN]);
    para->ItemValue = OV8835MIPI_sensor.eng.cct[para->ItemIdx].Para * 1000 / BASEGAIN;
    para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
    para->Min = OV8835MIPI_MIN_ANALOG_GAIN * 1000;
    para->Max = OV8835MIPI_MAX_ANALOG_GAIN * 1000;
    break;
  case OV8835MIPI_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Drv Cur[2,4,6,8]mA");
      switch (OV8835MIPI_sensor.eng.reg[OV8835MIPI_CMMCLK_CURRENT_INDEX].Para)
      {
      case ISP_DRIVING_2MA:
        para->ItemValue = 2;
        break;
      case ISP_DRIVING_4MA:
        para->ItemValue = 4;
        break;
      case ISP_DRIVING_6MA:
        para->ItemValue = 6;
        break;
      case ISP_DRIVING_8MA:
        para->ItemValue = 8;
        break;
      default:
        ASSERT(0);
      }
      para->IsTrueFalse = para->IsReadOnly = KAL_FALSE;
      para->IsNeedRestart = KAL_TRUE;
      para->Min = 2;
      para->Max = 8;
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8835MIPI_FRAME_RATE_LIMITATION:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Max Exposure Lines");
      para->ItemValue = 5998;
      break;
    case 1:
      sprintf(para->ItemNamePtr, "Min Frame Rate");
      para->ItemValue = 5;
      break;
    default:
      ASSERT(0);
    }
    para->IsTrueFalse = para->IsNeedRestart = KAL_FALSE;
    para->IsReadOnly = KAL_TRUE;
    para->Min = para->Max = 0;
    break;
  case OV8835MIPI_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
    case 0:
    case 1:
      sprintf(para->ItemNamePtr, editer_item_name[para->ItemIdx]);
      para->ItemValue = 0;
      para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
      para->Min = 0;
      para->Max = (para->ItemIdx == 0 ? 0xFFFF : 0xFF);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
}

inline static kal_bool OV8835MIPI_set_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{
  kal_uint16 temp_para;
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPI_set_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8835MIPI_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV8835MIPI_SENSOR_BASEGAIN:
    case OV8835MIPI_PRE_GAIN_R_INDEX:
    case OV8835MIPI_PRE_GAIN_Gr_INDEX:
    case OV8835MIPI_PRE_GAIN_Gb_INDEX:
    case OV8835MIPI_PRE_GAIN_B_INDEX:
	  spin_lock(&ov8835mipi_drv_lock);
      OV8835MIPI_sensor.eng.cct[para->ItemIdx].Para = para->ItemValue * BASEGAIN / 1000;
	  spin_unlock(&ov8835mipi_drv_lock);
	  
      OV8835MIPI_SetGain(OV8835MIPI_sensor.gain); /* update gain */
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8835MIPI_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      switch (para->ItemValue)
      {
      case 2:
        temp_para = ISP_DRIVING_2MA;
        break;
      case 3:
      case 4:
        temp_para = ISP_DRIVING_4MA;
        break;
      case 5:
      case 6:
        temp_para = ISP_DRIVING_6MA;
        break;
      default:
        temp_para = ISP_DRIVING_8MA;
        break;
      }
      //OV8835MIPI_set_isp_driving_current((kal_uint16)temp_para);
	  spin_lock(&ov8835mipi_drv_lock);
      OV8835MIPI_sensor.eng.reg[OV8835MIPI_CMMCLK_CURRENT_INDEX].Para = temp_para;
	  spin_unlock(&ov8835mipi_drv_lock);
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8835MIPI_FRAME_RATE_LIMITATION:
    ASSERT(0);
    break;
  case OV8835MIPI_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
      static kal_uint32 fac_sensor_reg;
    case 0:
      if (para->ItemValue < 0 || para->ItemValue > 0xFFFF) return KAL_FALSE;
      fac_sensor_reg = para->ItemValue;
      break;
    case 1:
      if (para->ItemValue < 0 || para->ItemValue > 0xFF) return KAL_FALSE;
      OV8835MIPI_write_cmos_sensor(fac_sensor_reg, para->ItemValue);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
  return KAL_TRUE;
}




void OV8835MIPI_globle_setting(void)
{
	//OV8835MIPI_Global_setting
	//Base_on_OV8835MIPI_APP_R1.11
	//2012_2_1
	// 
	//;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;

//#ifdef OV8835MIPI_DEBUG_SETTING
	//@@OV8835_init
	
	SENSORDB("OV8835MIPI_globle_setting  start \n");
	//@@OV8835_init_15fps

	OV8835MIPI_write_cmos_sensor(0x0103, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x0102, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3001, 0x2a);//
	OV8835MIPI_write_cmos_sensor(0x3002, 0x88);//
	OV8835MIPI_write_cmos_sensor(0x3005, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3011, 0x41);//
	OV8835MIPI_write_cmos_sensor(0x3015, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x301b, 0xb4);//
	OV8835MIPI_write_cmos_sensor(0x301d, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3021, 0x20);// LINE <> <DATE20130909> <truly ov8835 DVDD> Jiangde, 0x00-->0x20(0x00 = internal DVDD; 0x20 = external DVDD)
	OV8835MIPI_write_cmos_sensor(0x3022, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3081, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3083, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3090, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x3091, 0x23);//
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3092, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3098, 0x02);//0x04);//
	OV8835MIPI_write_cmos_sensor(0x3099, 0x13);//0x10);//
	OV8835MIPI_write_cmos_sensor(0x309a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x309b, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x30a2, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x30b0, 0x05);//
	OV8835MIPI_write_cmos_sensor(0x30b2, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x64);//
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3104, 0xa1);//
	OV8835MIPI_write_cmos_sensor(0x3106, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3400, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3401, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3402, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3403, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3404, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3405, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3406, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3500, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3503, 0x07);//
	OV8835MIPI_write_cmos_sensor(0x3504, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3505, 0x30);//
	OV8835MIPI_write_cmos_sensor(0x3506, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3508, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3509, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x350a, 0x00);//
	//OV8835MIPI_write_cmos_sensor(0x350b, 0x10);//;38
	OV8835MIPI_write_cmos_sensor(0x3600, 0x98);//
	OV8835MIPI_write_cmos_sensor(0x3601, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3602, 0x7c);//
	OV8835MIPI_write_cmos_sensor(0x3604, 0x38);//
	OV8835MIPI_write_cmos_sensor(0x3612, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3620, 0x41);//
	OV8835MIPI_write_cmos_sensor(0x3621, 0xa4);//
	//OV8835MIPI_write_cmos_sensor(0x3622, 0x0b);//
	OV8835MIPI_write_cmos_sensor(0x3622, 0x0f);//for low light noise
	OV8835MIPI_write_cmos_sensor(0x3625, 0x44);//
	OV8835MIPI_write_cmos_sensor(0x3630, 0x55);//
	OV8835MIPI_write_cmos_sensor(0x3631, 0xf2);//
	OV8835MIPI_write_cmos_sensor(0x3632, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3633, 0x34);//
	OV8835MIPI_write_cmos_sensor(0x3634, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x364d, 0x0d);//
	OV8835MIPI_write_cmos_sensor(0x364f, 0xCF);//;60
	OV8835MIPI_write_cmos_sensor(0x3662, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3665, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3666, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3667, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366a, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x366c, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366d, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366e, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366f, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x3680, 0xE5);//;b5
	OV8835MIPI_write_cmos_sensor(0x3681, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3701, 0x14);//
	OV8835MIPI_write_cmos_sensor(0x3702, 0xBF);//;50
	OV8835MIPI_write_cmos_sensor(0x3703, 0x8c);//
	OV8835MIPI_write_cmos_sensor(0x3704, 0x78);//;68
	OV8835MIPI_write_cmos_sensor(0x3705, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x370a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x370b, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x370c, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x370d, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x370e, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x370f, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3710, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x371c, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x371f, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3721, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3724, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3726, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x372a, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3730, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x3738, 0x22);//
	OV8835MIPI_write_cmos_sensor(0x3739, 0x08);//;d0
	OV8835MIPI_write_cmos_sensor(0x373a, 0x51);//;50
	OV8835MIPI_write_cmos_sensor(0x373b, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x373c, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x373f, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3740, 0x42);//
	OV8835MIPI_write_cmos_sensor(0x3741, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3742, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x3743, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3744, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3747, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x374c, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3751, 0xf0);//
	OV8835MIPI_write_cmos_sensor(0x3752, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3753, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3754, 0xc0);//
	OV8835MIPI_write_cmos_sensor(0x3755, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3756, 0x1a);//
	OV8835MIPI_write_cmos_sensor(0x3758, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3759, 0x0f);//
	OV8835MIPI_write_cmos_sensor(0x375c, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3767, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x376b, 0x44);//
	OV8835MIPI_write_cmos_sensor(0x3774, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3776, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x377f, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3780, 0x22);//
	OV8835MIPI_write_cmos_sensor(0x3781, 0x0c);//;cc
	OV8835MIPI_write_cmos_sensor(0x3784, 0x2c);//
	OV8835MIPI_write_cmos_sensor(0x3785, 0x1E);//;08
	OV8835MIPI_write_cmos_sensor(0x3786, 0x16);//
	OV8835MIPI_write_cmos_sensor(0x378f, 0xf5);//
	OV8835MIPI_write_cmos_sensor(0x3791, 0xb0);//
	OV8835MIPI_write_cmos_sensor(0x3795, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3796, 0x64);//;94
	OV8835MIPI_write_cmos_sensor(0x3797, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3798, 0x30);//
	OV8835MIPI_write_cmos_sensor(0x3799, 0x41);//
	OV8835MIPI_write_cmos_sensor(0x379a, 0x07);//
	OV8835MIPI_write_cmos_sensor(0x379b, 0xb0);//
	OV8835MIPI_write_cmos_sensor(0x379c, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x37c5, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37c6, 0xa0);//
	OV8835MIPI_write_cmos_sensor(0x37c7, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37c9, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37ca, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37cb, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37cc, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37cd, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37ce, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x37cf, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37d1, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x37de, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37df, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3810, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3811, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3812, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3813, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3823, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3824, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3825, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3826, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3827, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x382a, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3a06, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3a07, 0xf8);//
	OV8835MIPI_write_cmos_sensor(0x3b00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b02, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b03, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b04, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b05, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d01, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d02, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d03, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d04, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d05, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d06, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d07, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d08, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d09, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0b, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0c, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0d, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0e, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0f, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d80, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d81, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d84, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x4000, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x4001, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x4002, 0xc5);//0x45);//
	OV8835MIPI_write_cmos_sensor(0x4005, 0x18);//
	//OV8835MIPI_write_cmos_sensor(0x4006, 0x22);//;20
	OV8835MIPI_write_cmos_sensor(0x4006, 0x1f);//;20
	OV8835MIPI_write_cmos_sensor(0x4008, 0x20);//;24
	OV8835MIPI_write_cmos_sensor(0x4009, 0x10);//;10
	OV8835MIPI_write_cmos_sensor(0x4100, 0x10);//;17
	OV8835MIPI_write_cmos_sensor(0x4101, 0x12);//;03
	OV8835MIPI_write_cmos_sensor(0x4102, 0x24);//;04
	OV8835MIPI_write_cmos_sensor(0x4103, 0x00);//;03
	OV8835MIPI_write_cmos_sensor(0x4104, 0x5B);//;5a
	OV8835MIPI_write_cmos_sensor(0x4307, 0x30);//
	OV8835MIPI_write_cmos_sensor(0x4315, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x4511, 0x05);//

	//
	OV8835MIPI_write_cmos_sensor(0x4800, 0x14); // send line short packet for each line//for 89
	//
	OV8835MIPI_write_cmos_sensor(0x4805, 0x21);//
	OV8835MIPI_write_cmos_sensor(0x4806, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x481f, 0x36);//
	OV8835MIPI_write_cmos_sensor(0x4831, 0x6c);//
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0a);//
	OV8835MIPI_write_cmos_sensor(0x4a00, 0xaa);//
	OV8835MIPI_write_cmos_sensor(0x4a03, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x4a05, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x4a0a, 0x88);//
	OV8835MIPI_write_cmos_sensor(0x4d03, 0xbb);//
	OV8835MIPI_write_cmos_sensor(0x5000, 0x06);//
	OV8835MIPI_write_cmos_sensor(0x5001, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x5002, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x5003, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x5013, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x5046, 0x4a);//
	OV8835MIPI_write_cmos_sensor(0x5780, 0x1c);//
	OV8835MIPI_write_cmos_sensor(0x5786, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x5787, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x5788, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x578a, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x578b, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x578c, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x578e, 0x06);//
	OV8835MIPI_write_cmos_sensor(0x578f, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5790, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5791, 0xff);//
	OV8835MIPI_write_cmos_sensor(0x5a08, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5e00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x5e10, 0x0c);//

#ifdef OV8835_BINNING_SUM
//BEGIN <> <DATE20130826> <ov8835 truly update> wupingzhou
	OV8835MIPI_write_cmos_sensor(0x5780, 0xfc);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5781, 0x13);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5782, 0x03);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5786, 0x20);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5787, 0x40);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5788, 0x08);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5789, 0x08);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578a, 0x02);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578b, 0x01);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578c, 0x01);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578d, 0x0c);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578e, 0x02);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578f, 0x01);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5790, 0x01);  
//END <> <DATE20130826> <ov8835 truly update> wupingzhou
#endif

	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);//


	SENSORDB("OV8835MIPI_globle_setting  end \n");
                                                                   		
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
OV8835MIPI_1632_1224_2Lane_30fps_Mclk26M_setting(void)
{

	//#ifdef OV8835MIPI_DEBUG_SETTING		

	//@@Ov8835_1632x1224_2lane_30fps_143MSclk_693Mbps
	
	SENSORDB("OV8835MIPI_1632_1224_2Lane_30fps_Mclk26M_setting start \n");
	
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);
	//OV8835MIPI_write_cmos_sensor(0x3501, 0x50);
	//OV8835MIPI_write_cmos_sensor(0x3502, 0x00);
	OV8835MIPI_write_cmos_sensor(0x3507, 0x08);
	OV8835MIPI_write_cmos_sensor(0x3660, 0x80);
	OV8835MIPI_write_cmos_sensor(0x3708, 0xe6);
	OV8835MIPI_write_cmos_sensor(0x3709, 0x43);
	OV8835MIPI_write_cmos_sensor(0x3800, 0x00);
	OV8835MIPI_write_cmos_sensor(0x3801, 0x08);
	OV8835MIPI_write_cmos_sensor(0x3802, 0x00);
	OV8835MIPI_write_cmos_sensor(0x3803, 0x08);
	OV8835MIPI_write_cmos_sensor(0x3804, 0x0c);
	OV8835MIPI_write_cmos_sensor(0x3805, 0xd7);
	OV8835MIPI_write_cmos_sensor(0x3806, 0x09);
	OV8835MIPI_write_cmos_sensor(0x3807, 0xa7);
	OV8835MIPI_write_cmos_sensor(0x3808, 0x06);
	OV8835MIPI_write_cmos_sensor(0x3809, 0x60);
	OV8835MIPI_write_cmos_sensor(0x380a, 0x04);
	OV8835MIPI_write_cmos_sensor(0x380b, 0xc8);
	OV8835MIPI_write_cmos_sensor(0x380c, 0x0e);
	OV8835MIPI_write_cmos_sensor(0x380d, 0x18);
	OV8835MIPI_write_cmos_sensor(0x380e, 0x05);
	OV8835MIPI_write_cmos_sensor(0x380f, 0x28);
	OV8835MIPI_write_cmos_sensor(0x3814, 0x31);
	OV8835MIPI_write_cmos_sensor(0x3815, 0x31);
	OV8835MIPI_write_cmos_sensor(0x3820, 0x11);
	OV8835MIPI_write_cmos_sensor(0x3821, 0x0f);
	OV8835MIPI_write_cmos_sensor(0x3a04, 0x04);
	OV8835MIPI_write_cmos_sensor(0x3a05, 0xc9);
	OV8835MIPI_write_cmos_sensor(0x4004, 0x02);
	OV8835MIPI_write_cmos_sensor(0x404f, 0xa0);
    
#ifdef OV8835_BINNING_SUM
	OV8835MIPI_write_cmos_sensor(0x4512, 0x00);//nick binning sum//LINE <> <DATE20130826> <ov8835 truly update> wupingzhou
#else
	OV8835MIPI_write_cmos_sensor(0x4512, 0x01);
#endif
	
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  );
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  ); 										;PLL
	OV8835MIPI_write_cmos_sensor(0x3011, 0x21); //; MIPI 2 lane, MIPI enable
	OV8835MIPI_write_cmos_sensor(0x3015, 0xc8); //; MIPI 2 lane on, select MIPI
	OV8835MIPI_write_cmos_sensor(0x3090, 0x03); //; PLL
	OV8835MIPI_write_cmos_sensor(0x3091, 0x21); //;23 ; PLL
	//OV8835MIPI_write_cmos_sensor(0x3091, 0x18); //;23 ; PLL//0x18 104M
	OV8835MIPI_write_cmos_sensor(0x3092, 0x01); //;00 ; PLL _SCLK_143M
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00); //; PLL
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00); //;
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  );
//	OV8835MIPI_write_cmos_sensor(0x30b3, 0x50); //; 64 ; PLL _PLL_MIPICLK_693MBps
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x3c); //; 64 ; PLL _PLL_MIPICLK_693MBps
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03); //; PLL
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04); //; PLL
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01); //; PLL
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0f);//0x0c); //; MIPI pclk period
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  );
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);

	SENSORDB("OV8835MIPI_1632_1224_2Lane_30fps_Mclk26M_setting end \n");

}

OV8835MIPI_3264_2448_2Lane_15fps_Mclk26M_setting(void)
{
	
//	;//OV8835MIPI_3264*2448_setting_2lanes_690Mbps/lane_15fps									   
//	;//Base_on_OV8835MIPI_APP_R1.11 															   
//	;//2012_2_1 																			   
//	;//Tony Li																				   
//	;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;	


	//@@Ov8835_3264x2448_2lane_15fps_143MSclk_693Mbps

	SENSORDB("OV8835MIPI_3264_2448_2Lane_15fps_Mclk26M_setting start \n");

	//@@Ov8835_3264x2448_2lane_15fps_143MSclk_693Mbps
	//OV8835MIPI_write_cmos_sensor(0x
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);
	//OV8835MIPI_write_cmos_sensor(0x3501, 0xA0);
	//OV8835MIPI_write_cmos_sensor(0x3502, 0xC0);
	OV8835MIPI_write_cmos_sensor(0x3507, 0x10);
	OV8835MIPI_write_cmos_sensor(0x3660, 0x80);
	OV8835MIPI_write_cmos_sensor(0x3708, 0xe3);
	OV8835MIPI_write_cmos_sensor(0x3709, 0x43);
	OV8835MIPI_write_cmos_sensor(0x3800, 0x00);
	OV8835MIPI_write_cmos_sensor(0x3801, 0x0c);
	OV8835MIPI_write_cmos_sensor(0x3802, 0x00);
	OV8835MIPI_write_cmos_sensor(0x3803, 0x0c);
	OV8835MIPI_write_cmos_sensor(0x3804, 0x0c);
	OV8835MIPI_write_cmos_sensor(0x3805, 0xd3);
	OV8835MIPI_write_cmos_sensor(0x3806, 0x09);
	OV8835MIPI_write_cmos_sensor(0x3807, 0xa3);
	OV8835MIPI_write_cmos_sensor(0x3808, 0x0c);
	OV8835MIPI_write_cmos_sensor(0x3809, 0xc0);
	OV8835MIPI_write_cmos_sensor(0x380a, 0x09);
	OV8835MIPI_write_cmos_sensor(0x380b, 0x90);
	OV8835MIPI_write_cmos_sensor(0x380c, 0x0e);
	OV8835MIPI_write_cmos_sensor(0x380d, 0x18);
	OV8835MIPI_write_cmos_sensor(0x380e, 0x0A);
	OV8835MIPI_write_cmos_sensor(0x380f, 0x52);//0x1A);
	OV8835MIPI_write_cmos_sensor(0x3814, 0x11);
	OV8835MIPI_write_cmos_sensor(0x3815, 0x11);
	OV8835MIPI_write_cmos_sensor(0x3820, 0x10);
	OV8835MIPI_write_cmos_sensor(0x3821, 0x0e);
	OV8835MIPI_write_cmos_sensor(0x3a04, 0x09);
	OV8835MIPI_write_cmos_sensor(0x3a05, 0xa9);
	OV8835MIPI_write_cmos_sensor(0x4004, 0x08);
	OV8835MIPI_write_cmos_sensor(0x404f, 0xA0);
	OV8835MIPI_write_cmos_sensor(0x4512, 0x01);
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  );
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  );              ;PLL
	OV8835MIPI_write_cmos_sensor(0x3011, 0x21); //; MIPI 2 lane, MIPI enable
	OV8835MIPI_write_cmos_sensor(0x3015, 0xc8); //; MIPI 2 lane on, select MIPI
	OV8835MIPI_write_cmos_sensor(0x3090, 0x03); //; PLL
	OV8835MIPI_write_cmos_sensor(0x3091, 0x21); //;23 ; PLL
	//OV8835MIPI_write_cmos_sensor(0x3091, 0x18); //;23 ; PLL //capture clk
	//OV8835MIPI_write_cmos_sensor(0x3091, 0x18); //;23 ; PLL
	OV8835MIPI_write_cmos_sensor(0x3092, 0x01); //; PLL _SCLK_143M
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00); //; PLL
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00); //;
	//OV8835MIPI_write_cmos_sensor(0x    , 0x  );
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x50);//0x4d); //; 64 ; PLL _PLL_MIPICLK_800MBps//50:693  //4d:667
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03); //; PLL
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04); //; PLL
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01); //; PLL
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0c); //; MIPI pclk period
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);
	//                                   , 0x

	SENSORDB("OV8835MIPI_3264_2448_2Lane_15fps_Mclk26M_setting end \n");

	//SENSORDB("OV8835MIPI_3264_2448_2Lane_15fps_Mclk26M_setting end \n");
	
}


#ifdef OV8835MIPI_4LANE
void OV8835MIPI_4LANE_globle_setting(void)
{
	//@@OV8835_init_Lisa_15fps
	
	OV8835MIPI_write_cmos_sensor(0x0103, 0x01);//
	mdelay(5); // Jiangde ++
	
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x0102, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3001, 0x2a);//
	OV8835MIPI_write_cmos_sensor(0x3002, 0x88);//
	OV8835MIPI_write_cmos_sensor(0x3005, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3011, 0x41);//
	OV8835MIPI_write_cmos_sensor(0x3015, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x301b, 0xb4);//
	OV8835MIPI_write_cmos_sensor(0x301d, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3021, 0x20);// LINE <> <DATE20130909> <truly ov8835 DVDD> Jiangde, 0x00-->0x20(0x00 = internal DVDD; 0x20 = external DVDD)
	OV8835MIPI_write_cmos_sensor(0x3022, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3081, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3083, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3090, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x3091, 0x23);//
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3092, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3098, 0x02);//;04
	OV8835MIPI_write_cmos_sensor(0x3099, 0x13);//;10
	OV8835MIPI_write_cmos_sensor(0x309a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x309b, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x30a2, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x30b0, 0x05);//
	OV8835MIPI_write_cmos_sensor(0x30b2, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x64);//
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3104, 0xa1);//
	OV8835MIPI_write_cmos_sensor(0x3106, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3400, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3401, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3402, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3403, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3404, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3405, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3406, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3500, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3503, 0x07);//
	OV8835MIPI_write_cmos_sensor(0x3504, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3505, 0x30);//
	OV8835MIPI_write_cmos_sensor(0x3506, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3508, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3509, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x350a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x350b, 0x10);//;38
	OV8835MIPI_write_cmos_sensor(0x3600, 0x98);//
	OV8835MIPI_write_cmos_sensor(0x3601, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3602, 0x7c);//
	OV8835MIPI_write_cmos_sensor(0x3604, 0x38);//
	OV8835MIPI_write_cmos_sensor(0x3612, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3620, 0x41);//
	OV8835MIPI_write_cmos_sensor(0x3621, 0xa4);//
	OV8835MIPI_write_cmos_sensor(0x3622, 0x0b);//
	OV8835MIPI_write_cmos_sensor(0x3625, 0x44);//
	OV8835MIPI_write_cmos_sensor(0x3630, 0x55);//
	OV8835MIPI_write_cmos_sensor(0x3631, 0xf2);//
	OV8835MIPI_write_cmos_sensor(0x3632, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3633, 0x34);//
	OV8835MIPI_write_cmos_sensor(0x3634, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x364d, 0x0d);//
	OV8835MIPI_write_cmos_sensor(0x364f, 0xCF);//;60
	OV8835MIPI_write_cmos_sensor(0x3662, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3665, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3666, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3667, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366a, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x366c, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366d, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366e, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x366f, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x3680, 0xE5);//;b5
	OV8835MIPI_write_cmos_sensor(0x3681, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3701, 0x14);//
	OV8835MIPI_write_cmos_sensor(0x3702, 0xBF);//;50
	OV8835MIPI_write_cmos_sensor(0x3703, 0x8c);//
	OV8835MIPI_write_cmos_sensor(0x3704, 0x78);//;68
	OV8835MIPI_write_cmos_sensor(0x3705, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x370a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x370b, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x370c, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x370d, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x370e, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x370f, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3710, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x371c, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x371f, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3721, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3724, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3726, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x372a, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3730, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x3738, 0x22);//
	OV8835MIPI_write_cmos_sensor(0x3739, 0x08);//;d0
	OV8835MIPI_write_cmos_sensor(0x373a, 0x51);//;50
	OV8835MIPI_write_cmos_sensor(0x373b, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x373c, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x373f, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3740, 0x42);//
	OV8835MIPI_write_cmos_sensor(0x3741, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3742, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x3743, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3744, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x3747, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x374c, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3751, 0xf0);//
	OV8835MIPI_write_cmos_sensor(0x3752, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3753, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3754, 0xc0);//
	OV8835MIPI_write_cmos_sensor(0x3755, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3756, 0x1a);//
	OV8835MIPI_write_cmos_sensor(0x3758, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3759, 0x0f);//
	OV8835MIPI_write_cmos_sensor(0x375c, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3767, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x376b, 0x44);//
	OV8835MIPI_write_cmos_sensor(0x3774, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3776, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x377f, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3780, 0x22);//
	OV8835MIPI_write_cmos_sensor(0x3781, 0x0c);//;cc
	OV8835MIPI_write_cmos_sensor(0x3784, 0x2c);//
	OV8835MIPI_write_cmos_sensor(0x3785, 0x1E);//;08
	OV8835MIPI_write_cmos_sensor(0x3786, 0x16);//
	OV8835MIPI_write_cmos_sensor(0x378f, 0xf5);//
	OV8835MIPI_write_cmos_sensor(0x3791, 0xb0);//
	OV8835MIPI_write_cmos_sensor(0x3795, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3796, 0x64);//;94
	OV8835MIPI_write_cmos_sensor(0x3797, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3798, 0x30);//
	OV8835MIPI_write_cmos_sensor(0x3799, 0x41);//
	OV8835MIPI_write_cmos_sensor(0x379a, 0x07);//
	OV8835MIPI_write_cmos_sensor(0x379b, 0xb0);//
	OV8835MIPI_write_cmos_sensor(0x379c, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x37c5, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37c6, 0xa0);//
	OV8835MIPI_write_cmos_sensor(0x37c7, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37c9, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37ca, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37cb, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37cc, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37cd, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37ce, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x37cf, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37d1, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x37de, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x37df, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3810, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3811, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3812, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3813, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3823, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3824, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3825, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3826, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3827, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x382a, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3a06, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3a07, 0xf8);//
	OV8835MIPI_write_cmos_sensor(0x3b00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b02, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b03, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b04, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3b05, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d01, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d02, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d03, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d04, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d05, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d06, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d07, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d08, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d09, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0a, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0b, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0c, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0d, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0e, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d0f, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d80, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d81, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3d84, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x4000, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x4001, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x4002, 0xc5);//;45
	OV8835MIPI_write_cmos_sensor(0x4005, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x4006, 0x1f);//;20
	OV8835MIPI_write_cmos_sensor(0x4008, 0x20);//;24
	OV8835MIPI_write_cmos_sensor(0x4009, 0x10);//;10
	OV8835MIPI_write_cmos_sensor(0x4100, 0x10);//;17
	OV8835MIPI_write_cmos_sensor(0x4101, 0x12);//;03
	OV8835MIPI_write_cmos_sensor(0x4102, 0x24);//;04
	OV8835MIPI_write_cmos_sensor(0x4103, 0x00);//;03
	OV8835MIPI_write_cmos_sensor(0x4104, 0x5B);//;5a
	OV8835MIPI_write_cmos_sensor(0x4307, 0x30);//
	OV8835MIPI_write_cmos_sensor(0x4315, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x4511, 0x05);//
	OV8835MIPI_write_cmos_sensor(0x4800, 0x14);//;04 enable short packet
	OV8835MIPI_write_cmos_sensor(0x4805, 0x21);//
	OV8835MIPI_write_cmos_sensor(0x4806, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x481f, 0x36);//
	OV8835MIPI_write_cmos_sensor(0x4831, 0x6c);//
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0a);//
	OV8835MIPI_write_cmos_sensor(0x4a00, 0xaa);//
	OV8835MIPI_write_cmos_sensor(0x4a03, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x4a05, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x4a0a, 0x88);//
	OV8835MIPI_write_cmos_sensor(0x4d03, 0xbb);//
	OV8835MIPI_write_cmos_sensor(0x5000, 0x06);//
	OV8835MIPI_write_cmos_sensor(0x5001, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x5002, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x5003, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x5013, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x5046, 0x4a);//
	OV8835MIPI_write_cmos_sensor(0x5780, 0x1c);//
	OV8835MIPI_write_cmos_sensor(0x5786, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x5787, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x5788, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x578a, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x578b, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x578c, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x578e, 0x06);//
	OV8835MIPI_write_cmos_sensor(0x578f, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5790, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5791, 0xff);//
	OV8835MIPI_write_cmos_sensor(0x5a08, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5e00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x5e10, 0x0c);//

	OV8835MIPI_write_cmos_sensor(0x5780, 0xfc);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5781, 0x13);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5782, 0x03);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5786, 0x20);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5787, 0x40);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5788, 0x08);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5789, 0x08);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578a, 0x02);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578b, 0x01);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578c, 0x01);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578d, 0x0c);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578e, 0x02);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x578f, 0x01);                                                                                                          
	OV8835MIPI_write_cmos_sensor(0x5790, 0x01);  
	OV8835MIPI_write_cmos_sensor(0x5791, 0xff);//
	OV8835MIPI_write_cmos_sensor(0x5a08, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x5e00, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x5e10, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);//


#ifdef OV8835MIPI_USE_OTP //LINE <> <DATE20130625> <truly ov8835 OTP> wupingzhou
    otp_update_wb(0x120,0x105);
    otp_update_lenc();
    otp_update_BLC();
#endif
}

void OV8835MIPI_1632_1224_4LANE_30fps_Mclk26M_setting(void) // Jiangde preview
{
	//@@Ov8835_1632x1224_4lane_30fps_208MSclk_780Mbps
	
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);//
	//OV8835MIPI_write_cmos_sensor(0x3501, 0x31);//
	//OV8835MIPI_write_cmos_sensor(0x3502, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3507, 0x08);//
	//OV8835MIPI_write_cmos_sensor(0x350b, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3660, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3708, 0xe6);//
	OV8835MIPI_write_cmos_sensor(0x3709, 0x43);//
	OV8835MIPI_write_cmos_sensor(0x3800, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3801, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3802, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3803, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3804, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3805, 0xd7);//
	OV8835MIPI_write_cmos_sensor(0x3806, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x3807, 0xa7);//
	OV8835MIPI_write_cmos_sensor(0x3808, 0x06);//
	OV8835MIPI_write_cmos_sensor(0x3809, 0x60);//
	OV8835MIPI_write_cmos_sensor(0x380a, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x380b, 0xc8);//
	OV8835MIPI_write_cmos_sensor(0x380c, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x380d, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x380e, 0x07);//	;insert VTS for 30fps 
	OV8835MIPI_write_cmos_sensor(0x380f, 0x82);// 
	OV8835MIPI_write_cmos_sensor(0x3814, 0x31);//
	OV8835MIPI_write_cmos_sensor(0x3815, 0x31);//
	OV8835MIPI_write_cmos_sensor(0x3820, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3821, 0x0f);//
	OV8835MIPI_write_cmos_sensor(0x3a04, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x3a05, 0xc9);//
	OV8835MIPI_write_cmos_sensor(0x4004, 0x02);//
	OV8835MIPI_write_cmos_sensor(0x4005, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x404F, 0xA0);//
	
#ifdef OV8835_BINNING_SUM
	OV8835MIPI_write_cmos_sensor(0x4512, 0x00);// Jiangde preview; 00(sum),0x01(average) //nick binning sum//LINE <> <DATE20130826> <ov8835 truly update> wupingzhou
#else
    OV8835MIPI_write_cmos_sensor(0x4512, 0x01);// Jiangde preview; 00(sum),0x01(average) //nick binning sum//LINE <> <DATE20130826> <ov8835 truly update> wupingzhou
#endif

	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	//OV8835MIPI_write_cmos_sensor(0x////, 0x//);/////;PLL
	OV8835MIPI_write_cmos_sensor(0x3011, 0x41);// ; 4 Lane, MIPI enable
	OV8835MIPI_write_cmos_sensor(0x3015, 0x08);//; MIPI mode,
	OV8835MIPI_write_cmos_sensor(0x3090, 0x02);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3091, 0x10);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3092, 0x00);// ; PLL _SCLK_208M
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00);// ;
	OV8835MIPI_write_cmos_sensor(0x3098, 0x02);// ; 
	OV8835MIPI_write_cmos_sensor(0x3099, 0x13);// ;
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x5a);// ; PLL _PLL_MIPICLK_780MBps
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0a);// ; MIPI pclk period
	//OV8835MIPI_write_cmos_sensor(0x////, 0x//);//////////;DCBLC
	OV8835MIPI_write_cmos_sensor(0x364f, 0xCF);//
	OV8835MIPI_write_cmos_sensor(0x3680, 0xE5);//
	OV8835MIPI_write_cmos_sensor(0x3702, 0xBF);//
	OV8835MIPI_write_cmos_sensor(0x3704, 0x78);//
	OV8835MIPI_write_cmos_sensor(0x3739, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x373a, 0x51);//
	OV8835MIPI_write_cmos_sensor(0x3781, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3785, 0x1E);//
	OV8835MIPI_write_cmos_sensor(0x3796, 0x64);//
	OV8835MIPI_write_cmos_sensor(0x4008, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x4100, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x4101, 0x12);//
	OV8835MIPI_write_cmos_sensor(0x4102, 0x24);//
	OV8835MIPI_write_cmos_sensor(0x4103, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x4104, 0x5B);//
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);//
	
	/////////////100 99 1632 1224
	/////////////100 98 1 0
	/////////////102 84 01

}
void OV8835MIPI_3264_2448_4LANE_30fps_Mclk26M_setting(void) // Jiangde capture
{
    
#ifdef OV8835_TEMPERATURE_TEST
	kal_uint16 get_byte1 = 0;
	kal_uint16 get_byte2 = 0;

    OV8835MIPI_write_cmos_sensor(0x4d0b, 0x00);
    mdelay(5);
    get_byte1 = OV8835MIPI_read_cmos_sensor(0x4d0b);

    OV8835MIPI_write_cmos_sensor(0x4d0b, 0x80);
    mdelay(5);   
    get_byte2 = OV8835MIPI_read_cmos_sensor(0x4d0b);
    SENSORDB("HJDDbg, temperature reg, before=%x, 0x4d0b=%x \n", get_byte1, get_byte2);
#endif

	//@@Ov8835_3264x2448_4lane_30fps_273MSclk_780Mbps

	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);//
	//OV8835MIPI_write_cmos_sensor(0x3501, 0xA0);//
	//OV8835MIPI_write_cmos_sensor(0x3502, 0xC0);//
	OV8835MIPI_write_cmos_sensor(0x3507, 0x10);//
	//OV8835MIPI_write_cmos_sensor(0x350b, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3660, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3708, 0xe3);//
	OV8835MIPI_write_cmos_sensor(0x3709, 0x43);//
	OV8835MIPI_write_cmos_sensor(0x3800, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3801, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3802, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3803, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3804, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3805, 0xd3);//
	OV8835MIPI_write_cmos_sensor(0x3806, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x3807, 0xa3);//
	OV8835MIPI_write_cmos_sensor(0x3808, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3809, 0xc0);//
	OV8835MIPI_write_cmos_sensor(0x380a, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x380b, 0x90);//
	OV8835MIPI_write_cmos_sensor(0x380c, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x380d, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x380e, 0x09);// 
	OV8835MIPI_write_cmos_sensor(0x380f, 0xDA);//	;for 30fps 
	OV8835MIPI_write_cmos_sensor(0x3814, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3815, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3820, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3821, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x3a04, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x3a05, 0xa9);//
	OV8835MIPI_write_cmos_sensor(0x4004, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x4005, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x404F, 0xA0);//
	OV8835MIPI_write_cmos_sensor(0x4512, 0x01);//
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	//OV8835MIPI_write_cmos_sensor(0x////, 0x//);///////;PLL
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	OV8835MIPI_write_cmos_sensor(0x3011, 0x41);// ; 4 Lane, MIPI enable
	OV8835MIPI_write_cmos_sensor(0x3015, 0x08);//; MIPI mode,
	OV8835MIPI_write_cmos_sensor(0x3090, 0x02);// ; PLL
	#ifdef OV8835MIPI_4LANE_CAP_30FPS
	OV8835MIPI_write_cmos_sensor(0x3091, 0x15);// ; PLL//273M
	#else
	OV8835MIPI_write_cmos_sensor(0x3091, 0x14);// ; PLL//260M
	#endif
	OV8835MIPI_write_cmos_sensor(0x3092, 0x00);// ; PLL _SCLK_273M
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00);// ;
	OV8835MIPI_write_cmos_sensor(0x3098, 0x02);// ; 
	OV8835MIPI_write_cmos_sensor(0x3099, 0x14);// ;
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x5a);// ; PLL _PLL_MIPICLK_780MBps
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0a);// ; MIPI pclk period
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	//OV8835MIPI_write_cmos_sensor(0x////, 0x//);////;CVDNBLC
	OV8835MIPI_write_cmos_sensor(0x364f, 0x60);//
	OV8835MIPI_write_cmos_sensor(0x3680, 0xb5);//
	OV8835MIPI_write_cmos_sensor(0x3702, 0x50);//
	OV8835MIPI_write_cmos_sensor(0x3704, 0x68);//
	OV8835MIPI_write_cmos_sensor(0x3739, 0xd0);//
	OV8835MIPI_write_cmos_sensor(0x373a, 0x50);//
	OV8835MIPI_write_cmos_sensor(0x3781, 0xcc);//
	OV8835MIPI_write_cmos_sensor(0x3785, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3796, 0x94);//
	OV8835MIPI_write_cmos_sensor(0x4008, 0x24);//
	OV8835MIPI_write_cmos_sensor(0x4100, 0x17);//
	OV8835MIPI_write_cmos_sensor(0x4101, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x4102, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x4103, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x4104, 0x5a);//
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);//
	
	////////////100 99 3264 2448
	////////////100 98 1 0
	////////////102 84 01

}

void Ov8835_3264x2448_4lane_24fps_216MSclk_780Mbps(void) // Jiangde capture 24fps
{
    
#ifdef OV8835_TEMPERATURE_TEST
	kal_uint16 get_byte1 = 0;
	kal_uint16 get_byte2 = 0;

    OV8835MIPI_write_cmos_sensor(0x4d0b, 0x00);
    mdelay(5);
    get_byte1 = OV8835MIPI_read_cmos_sensor(0x4d0b);

    OV8835MIPI_write_cmos_sensor(0x4d0b, 0x80);
    mdelay(5);   
    get_byte2 = OV8835MIPI_read_cmos_sensor(0x4d0b);
    SENSORDB("HJDDbg, temperature reg, before=%x, 0x4d0b=%x \n", get_byte1, get_byte2);
#endif

    SENSORDB("HJDDbg, Capture, Ov8835_3264x2448_4lane_24fps_216MSclk_780Mbps \n");
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3501, 0xA0);//
	OV8835MIPI_write_cmos_sensor(0x3502, 0xC0);//
	OV8835MIPI_write_cmos_sensor(0x3507, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x350b, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3660, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3708, 0xe3);//
	OV8835MIPI_write_cmos_sensor(0x3709, 0x43);//
	OV8835MIPI_write_cmos_sensor(0x3800, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3801, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3802, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3803, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3804, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3805, 0xd3);//
	OV8835MIPI_write_cmos_sensor(0x3806, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x3807, 0xa3);//
	OV8835MIPI_write_cmos_sensor(0x3808, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3809, 0xc0);//
	OV8835MIPI_write_cmos_sensor(0x380a, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x380b, 0x90);//
	OV8835MIPI_write_cmos_sensor(0x380c, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x380d, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x380e, 0x09);// 
	OV8835MIPI_write_cmos_sensor(0x380f, 0xc8);//	;for 24fps 
	OV8835MIPI_write_cmos_sensor(0x3814, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3815, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3820, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3821, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x3a04, 0x09);//
	OV8835MIPI_write_cmos_sensor(0x3a05, 0xa9);//
	OV8835MIPI_write_cmos_sensor(0x4004, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x4005, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x404F, 0xA0);//
	OV8835MIPI_write_cmos_sensor(0x4512, 0x01);//

	OV8835MIPI_write_cmos_sensor(0x3011, 0x41);// ; 4 Lane, MIPI enable
	OV8835MIPI_write_cmos_sensor(0x3015, 0x08);//; MIPI mode,
	OV8835MIPI_write_cmos_sensor(0x3090, 0x03);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3091, 0x19);// ; PLL//273M
	OV8835MIPI_write_cmos_sensor(0x3092, 0x00);// ; PLL _SCLK_273M
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00);// ;
	OV8835MIPI_write_cmos_sensor(0x3098, 0x02);// ; 
	OV8835MIPI_write_cmos_sensor(0x3099, 0x13);// ;

	OV8835MIPI_write_cmos_sensor(0x30b3, 0x5a);// ; PLL _PLL_MIPICLK_780MBps
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0a);// ; MIPI pclk period

	OV8835MIPI_write_cmos_sensor(0x364f, 0xcf);//
	OV8835MIPI_write_cmos_sensor(0x3680, 0xe5);//
	OV8835MIPI_write_cmos_sensor(0x3702, 0xbf);//
	OV8835MIPI_write_cmos_sensor(0x3704, 0x78);//
	OV8835MIPI_write_cmos_sensor(0x3739, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x373a, 0x51);//
	OV8835MIPI_write_cmos_sensor(0x3781, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3785, 0x1e);//
	OV8835MIPI_write_cmos_sensor(0x3796, 0x64);//
	OV8835MIPI_write_cmos_sensor(0x4008, 0x20);//
	OV8835MIPI_write_cmos_sensor(0x4100, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x4101, 0x12);//
	OV8835MIPI_write_cmos_sensor(0x4102, 0x24);//
	OV8835MIPI_write_cmos_sensor(0x4103, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x4104, 0x5b);//
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);//

    OV8835MIPI_write_cmos_sensor(0x3500, 0x01);//
    OV8835MIPI_write_cmos_sensor(0x3501, 0x1b);//
    OV8835MIPI_write_cmos_sensor(0x3502, 0xe0);//
    
    OV8835MIPI_write_cmos_sensor(0x350b, 0xc0);//
    OV8835MIPI_write_cmos_sensor(0x380e, 0x11);//
    OV8835MIPI_write_cmos_sensor(0x380f, 0xcc);//
    
	////////////100 99 3264 2448
	////////////100 98 1 0
	////////////102 84 01
}

void OV8835MIPI_3264_1836_4LANE_30fps_Mclk26M_setting(void) // Jiangde Video, change to 1080p
{
	//@@Ov8835_3264x1836_4lane_30fps_273MSclk_780Mbps
	
	OV8835MIPI_write_cmos_sensor(0x0100, 0x00);//
	//OV8835MIPI_write_cmos_sensor(0x3501, 0xA0);//
	//OV8835MIPI_write_cmos_sensor(0x3502, 0xC0);//
	OV8835MIPI_write_cmos_sensor(0x3507, 0x10);//
	//OV8835MIPI_write_cmos_sensor(0x350b, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3660, 0x80);//
	OV8835MIPI_write_cmos_sensor(0x3708, 0xe3);//
	OV8835MIPI_write_cmos_sensor(0x3709, 0x43);//
	OV8835MIPI_write_cmos_sensor(0x3800, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x3801, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3802, 0x01);//
	OV8835MIPI_write_cmos_sensor(0x3803, 0x40);//
	OV8835MIPI_write_cmos_sensor(0x3804, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3805, 0xd3);//
	OV8835MIPI_write_cmos_sensor(0x3806, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3807, 0x73);//

    // BEGIN <><20130911><3264x1836-->1080P> Jiangde
#ifdef USE_1080P_VIDEO	
	//OV8835MIPI_write_cmos_sensor(0x3808, 0x07);// 0x780=1920
	//OV8835MIPI_write_cmos_sensor(0x3809, 0x80);//
	//OV8835MIPI_write_cmos_sensor(0x380a, 0x04);// 0x438=1080
	//OV8835MIPI_write_cmos_sensor(0x380b, 0x38);//

	OV8835MIPI_write_cmos_sensor(0x3808, 0x05);// 0x500=1280
	OV8835MIPI_write_cmos_sensor(0x3809, 0x00);//
	OV8835MIPI_write_cmos_sensor(0x380a, 0x02);// 0x2d0=720
	OV8835MIPI_write_cmos_sensor(0x380b, 0xd0);//
#else
	OV8835MIPI_write_cmos_sensor(0x3808, 0x0c);//
	OV8835MIPI_write_cmos_sensor(0x3809, 0xc0);//
	OV8835MIPI_write_cmos_sensor(0x380a, 0x07);//
	OV8835MIPI_write_cmos_sensor(0x380b, 0x2c);//
#endif
    // END <><20130911><3264x1836-->1080P> Jiangde
    
	OV8835MIPI_write_cmos_sensor(0x380c, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x380d, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x380e, 0x09);// 
	#ifdef OV8835MIPI_4LANE_CAP_30FPS
	OV8835MIPI_write_cmos_sensor(0x380f, 0xDA);//	;for 30fps 
	#else
	OV8835MIPI_write_cmos_sensor(0x380f, 0x60);//2400	;for 30fps 
	#endif
	OV8835MIPI_write_cmos_sensor(0x3814, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3815, 0x11);//
	OV8835MIPI_write_cmos_sensor(0x3820, 0x10);//
	OV8835MIPI_write_cmos_sensor(0x3821, 0x0e);//
	OV8835MIPI_write_cmos_sensor(0x3a04, 0x07);//
	OV8835MIPI_write_cmos_sensor(0x3a05, 0x49);//
	OV8835MIPI_write_cmos_sensor(0x4004, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x4005, 0x18);//
	OV8835MIPI_write_cmos_sensor(0x404f, 0xA0);//
	OV8835MIPI_write_cmos_sensor(0x4512, 0x01);//
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	//OV8835MIPI_write_cmos_sensor(0x////, 0x//);/////;PLL
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	OV8835MIPI_write_cmos_sensor(0x3011, 0x41);// ; 4 Lane, MIPI enable
	OV8835MIPI_write_cmos_sensor(0x3015, 0x08);//; MIPI mode,
	OV8835MIPI_write_cmos_sensor(0x3090, 0x02);// ; PLL
	#ifdef OV8835MIPI_4LANE_CAP_30FPS
	OV8835MIPI_write_cmos_sensor(0x3091, 0x15);// ; PLL//273M
	#else
	OV8835MIPI_write_cmos_sensor(0x3091, 0x14);// ; PLL//260M
	#endif
	OV8835MIPI_write_cmos_sensor(0x3092, 0x00);// ; PLL _SCLK_273M
	OV8835MIPI_write_cmos_sensor(0x3093, 0x00);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x3094, 0x00);// ;
	OV8835MIPI_write_cmos_sensor(0x3098, 0x02);// ; 
	OV8835MIPI_write_cmos_sensor(0x3099, 0x14);// ;
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	OV8835MIPI_write_cmos_sensor(0x30b3, 0x5a);// ; PLL _PLL_MIPICLK_780MBps
	OV8835MIPI_write_cmos_sensor(0x30b4, 0x03);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b5, 0x04);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x30b6, 0x01);// ; PLL
	OV8835MIPI_write_cmos_sensor(0x4837, 0x0a);// ; MIPI pclk period
	//OV8835MIPI_write_cmos_sensor(0x	 , 0x  );//
	//OV8835MIPI_write_cmos_sensor(0x////, 0x//);/////;CVDNBLC
	OV8835MIPI_write_cmos_sensor(0x364f, 0x60);//
	OV8835MIPI_write_cmos_sensor(0x3680, 0xb5);//
	OV8835MIPI_write_cmos_sensor(0x3702, 0x50);//
	OV8835MIPI_write_cmos_sensor(0x3704, 0x68);//
	OV8835MIPI_write_cmos_sensor(0x3739, 0xd0);//
	OV8835MIPI_write_cmos_sensor(0x373a, 0x50);//
	OV8835MIPI_write_cmos_sensor(0x3781, 0xcc);//
	OV8835MIPI_write_cmos_sensor(0x3785, 0x08);//
	OV8835MIPI_write_cmos_sensor(0x3796, 0x94);//
	OV8835MIPI_write_cmos_sensor(0x4008, 0x24);//
	OV8835MIPI_write_cmos_sensor(0x4100, 0x17);//
	OV8835MIPI_write_cmos_sensor(0x4101, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x4102, 0x04);//
	OV8835MIPI_write_cmos_sensor(0x4103, 0x03);//
	OV8835MIPI_write_cmos_sensor(0x4104, 0x5a);//
	OV8835MIPI_write_cmos_sensor(0x0100, 0x01);//
	
	/////////////100 99 3264 1836
	/////////////100 98 1 0
	/////////////102 84 01

}
#endif


UINT32 OV8835MIPIOpen(void)
{
	kal_uint16 sensor_id=0; 

	//added by mandrave
	int i;
	const kal_uint16 sccb_writeid[] = {OV8835MIPI_SLAVE_WRITE_ID_1,OV8835MIPI_SLAVE_WRITE_ID_2};

	spin_lock(&ov8835mipi_drv_lock);
	OV8835MIPI_sensor.is_zsd = KAL_FALSE;  //for zsd full size preview
	OV8835MIPI_sensor.is_zsd_cap = KAL_FALSE;
	OV8835MIPI_sensor.is_autofliker = KAL_FALSE; //for autofliker.
	OV8835MIPI_sensor.pv_mode = KAL_TRUE;
	OV8835MIPI_sensor.sensorMode = OV8835MIPI_SENSOR_MODE_INIT;
	OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
	spin_unlock(&ov8835mipi_drv_lock);
   
  	for(i = 0; i <(sizeof(sccb_writeid)/sizeof(sccb_writeid[0])); i++)
  	{
  		spin_lock(&ov8835mipi_drv_lock);
  	   OV8835MIPI_sensor.write_id = sccb_writeid[i];
	   OV8835MIPI_sensor.read_id = (sccb_writeid[i]|0x01);
	   spin_unlock(&ov8835mipi_drv_lock);

	   sensor_id=((OV8835MIPI_read_cmos_sensor(0x300A) << 8) | OV8835MIPI_read_cmos_sensor(0x300B));	
	   
#ifdef OV8835MIPI_DRIVER_TRACE
		SENSORDB("OV8835MIPIOpen, sensor_id:%x \n",sensor_id);
#endif
		if(OV8835MIPI_SENSOR_ID == sensor_id)
			{
				SENSORDB("OV8835MIPI slave write id:%x \n",OV8835MIPI_sensor.write_id);
				break;
			}
  	}
  
	// check if sensor ID correct		
	if (sensor_id != OV8835MIPI_SENSOR_ID) 
	{	
		//sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
#ifndef OV8835MIPI_4LANE
	OV8835MIPI_globle_setting();
#else
	OV8835MIPI_4LANE_globle_setting();
	mdelay(50); // Jiangde ++
#endif
	
	spin_lock(&ov8835mipi_drv_lock);
	OV8835MIPI_sensor.shutter = 0x500;//init shutter
	OV8835MIPI_sensor.gain = 0x20;//init gain
	spin_unlock(&ov8835mipi_drv_lock);

	SENSORDB("test for bootimage \n");
 	
	return ERROR_NONE;
}   /* OV8835MIPIOpen  */

/*************************************************************************
* FUNCTION
*   OV5642GetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPIGetSensorID(UINT32 *sensorID) 
{
  //added by mandrave
    int i;
    const kal_uint16 sccb_writeid[] = { OV8835MIPI_SLAVE_WRITE_ID_1, OV8835MIPI_SLAVE_WRITE_ID_2,
                                        OV8835MIPI_SLAVE_WRITE_ID_1, OV8835MIPI_SLAVE_WRITE_ID_2,
                                        OV8835MIPI_SLAVE_WRITE_ID_1, OV8835MIPI_SLAVE_WRITE_ID_2}; // Jiangde, try more
 

    for(i = 0; i <(sizeof(sccb_writeid)/sizeof(sccb_writeid[0])); i++)
  	{
  		spin_lock(&ov8835mipi_drv_lock);
  	    OV8835MIPI_sensor.write_id = sccb_writeid[i];
	    OV8835MIPI_sensor.read_id = (sccb_writeid[i]|0x01);
	    spin_unlock(&ov8835mipi_drv_lock);

	    *sensorID=((OV8835MIPI_read_cmos_sensor(0x300A) << 8) | OV8835MIPI_read_cmos_sensor(0x300B));	
	   
#ifdef OV8835MIPI_DRIVER_TRACE
		SENSORDB("HJDDBG, OV8835MIPIOpen, sensor_id:%x \n",*sensorID);
#endif
		if(OV8835MIPI_SENSOR_ID == *sensorID)
		{
			SENSORDB("HJDDBG, it's OV8835, slave write id:%x \n",OV8835MIPI_sensor.write_id);
			break;
		}

        if (1 == i % 2 && (sizeof(sccb_writeid) / sizeof(sccb_writeid[0]) - 1) != i)
        {
            SENSORDB("HJDDBG, Retry, i = %x \n", i);
            mdelay(2); // Jiangde, retry after a while
        }
  	}
  
	// check if sensor ID correct		
	if (*sensorID != OV8835MIPI_SENSOR_ID) 
	{	
		*sensorID = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	
   return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	OV8835MIPIClose
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPIClose(void)
{
#ifdef OV8835MIPI_DRIVER_TRACE
   SENSORDB("OV8835MIPIClose\n");
#endif
  //CISModulePowerOn(FALSE);
//	DRV_I2CClose(OV8835MIPIhDrvI2C);
	return ERROR_NONE;
}   /* OV8835MIPIClose */



//begin add by lishengli 20130516
void OV8835MIPISetFlipMirror(kal_int32 imgMirror)
{
	kal_int16 mirror=0,flip=0;
	flip = OV8835MIPI_read_cmos_sensor(0x3820);
	mirror  = OV8835MIPI_read_cmos_sensor(0x3821);
    switch (imgMirror)
    {
        case IMAGE_H_MIRROR://IMAGE_NORMAL:
            OV8835MIPI_write_cmos_sensor(0x3820, (flip & (0xBD)));//Set normal
            OV8835MIPI_write_cmos_sensor(0x3821, (mirror & (0xF9)));	//Set normal
            break;
        case IMAGE_NORMAL://IMAGE_V_MIRROR:
            OV8835MIPI_write_cmos_sensor(0x3820, (flip & (0xBD)));//Set flip
            OV8835MIPI_write_cmos_sensor(0x3821, (mirror | (0x06)));	//Set flip
            break;
        case IMAGE_HV_MIRROR://IMAGE_H_MIRROR:
            OV8835MIPI_write_cmos_sensor(0x3820, (flip |(0x42)));	//Set mirror
            OV8835MIPI_write_cmos_sensor(0x3821, (mirror & (0xF9)));	//Set mirror
            break;
        case IMAGE_V_MIRROR://IMAGE_HV_MIRROR:
            OV8835MIPI_write_cmos_sensor(0x3820, (flip |(0x42)));	//Set mirror & flip
            OV8835MIPI_write_cmos_sensor(0x3821, (mirror |(0x06)));	//Set mirror & flip
            break;
    }
}
//end add by lishengli 20130516



/*************************************************************************
* FUNCTION
* OV8835MIPIPreview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//kal_uint16 dummy_line;
	//kal_uint16 ret;
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPIPreview setting\n");
#endif
	//OV8835MIPI_Sensor_1M();
	//OV8835MIPI_globle_setting();

    spin_lock(&ov8835mipi_drv_lock);
	OV8835MIPI_sensor.pv_mode = KAL_TRUE;
	OV8835MIPI_sensor.sensorMode = OV8835MIPI_SENSOR_MODE_PREVIEW;
	OV8835MIPI_sensor.pclk = OV8835MIPI_PREVIEW_CLK;
	spin_unlock(&ov8835mipi_drv_lock);
	#ifndef OV8835MIPI_4LANE
	OV8835MIPI_1632_1224_2Lane_30fps_Mclk26M_setting();
	#else
	// OV8835MIPI_1632_1224_4LANE_30fps_Mclk26M_setting();
	OV8835MIPI_1632_1224_4LANE_30fps_Mclk26M_setting(); // Jiangde preview
	#endif
	
    //msleep(30);
	
	//OV8835MIPI_set_mirror(sensor_config_data->SensorImageMirror);
	switch (sensor_config_data->SensorOperationMode)
	{
	  case MSDK_SENSOR_OPERATION_MODE_VIDEO: 
	  	spin_lock(&ov8835mipi_drv_lock);
		OV8835MIPI_sensor.video_mode = KAL_TRUE;		
		OV8835MIPI_sensor.normal_fps = OV8835MIPI_FPS(30);
		OV8835MIPI_sensor.night_fps = OV8835MIPI_FPS(15);
		spin_unlock(&ov8835mipi_drv_lock);
		//dummy_line = 0;
#ifdef OV8835MIPI_DRIVER_TRACE
		SENSORDB("Video mode \n");
#endif
	   break;
	  default: /* ISP_PREVIEW_MODE */
	  	spin_lock(&ov8835mipi_drv_lock);
		OV8835MIPI_sensor.video_mode = KAL_FALSE;
		spin_unlock(&ov8835mipi_drv_lock);
		//dummy_line = 0;
#ifdef OV8835MIPI_DRIVER_TRACE
		SENSORDB("Camera preview mode \n");
#endif
	  break;
	}

	spin_lock(&ov8835mipi_drv_lock);
	OV8835MIPI_sensor.dummy_pixels = 0;
	OV8835MIPI_sensor.dummy_lines = 0;
	OV8835MIPI_sensor.line_length = OV8835MIPI_PV_PERIOD_PIXEL_NUMS+OV8835MIPI_sensor.dummy_pixels;
	OV8835MIPI_sensor.frame_length = OV8835MIPI_PV_PERIOD_LINE_NUMS+OV8835MIPI_sensor.dummy_lines;
	OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
	spin_unlock(&ov8835mipi_drv_lock);
	
    OV8835MIPI_Write_Shutter(OV8835MIPI_sensor.shutter);
	OV8835MIPI_Set_Dummy(OV8835MIPI_sensor.dummy_pixels, OV8835MIPI_sensor.dummy_lines); /* modify dummy_pixel must gen AE table again */

	OV8835MIPISetFlipMirror(IMAGE_HV_MIRROR);	//add by lishengli 20130516
	
	mdelay(50); // Jiangde, 40-->50
	return ERROR_NONE;
	
}   /*  OV8835MIPIPreview   */

/*************************************************************************
* FUNCTION
*	OV8835MIPICapture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	const kal_uint16 pv_line_length = OV8835MIPI_sensor.line_length;
	const kal_uint32 pv_pclk = OV8835MIPI_sensor.pv_pclk;
	const kal_uint32 cap_pclk = OV8835MIPI_sensor.cap_pclk;
	kal_uint32 shutter = OV8835MIPI_sensor.shutter;
	kal_uint16 dummy_pixel;
	//kal_uint32 temp;
	//kal_uint16 ret;
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPICapture setting start \n");
#endif
	//if((OV8835MIPI_sensor.pv_mode == KAL_TRUE)||(OV8835MIPI_sensor.is_zsd == KAL_TRUE))
	//if(OV8835MIPI_sensor.pv_mode == KAL_TRUE)
		if(OV8835MIPI_sensor.sensorMode == OV8835MIPI_SENSOR_MODE_CAPTURE)
			return ERROR_NONE;
	{
		// 
		spin_lock(&ov8835mipi_drv_lock);
		OV8835MIPI_sensor.video_mode = KAL_FALSE;
		OV8835MIPI_sensor.is_autofliker = KAL_FALSE;		
		OV8835MIPI_sensor.sensorMode = OV8835MIPI_SENSOR_MODE_CAPTURE;
		OV8835MIPI_sensor.pclk = OV8835MIPI_CAPTURE_CLK;
		spin_unlock(&ov8835mipi_drv_lock);
		
		if(OV8835MIPI_sensor.is_zsd == KAL_TRUE)
			{
				spin_lock(&ov8835mipi_drv_lock);
				OV8835MIPI_sensor.pv_mode = KAL_FALSE;
				spin_unlock(&ov8835mipi_drv_lock);

				//OV8835MIPI_3264_2448_2Lane_13fps_Mclk26M_setting();
				#ifndef OV8835MIPI_4LANE
				OV8835MIPI_3264_2448_2Lane_15fps_Mclk26M_setting();
				SENSORDB("OV8835MIPI_FPS 15 \n");
				#else

                #ifdef USE_24FPS_SETTING
    				SENSORDB("HJDDbg, Capture1, OV8835MIPI_FPS 24fps \n");
                    Ov8835_3264x2448_4lane_24fps_216MSclk_780Mbps(); // Jiangde 24fps
                #else
    				OV8835MIPI_3264_2448_4LANE_30fps_Mclk26M_setting(); // Jiangde Capture
    				SENSORDB("OV8835MIPI_FPS 30 \n");
                #endif
				
				#endif

				spin_lock(&ov8835mipi_drv_lock);
				OV8835MIPI_sensor.dummy_pixels = 0;
				OV8835MIPI_sensor.dummy_lines = 0;
				OV8835MIPI_sensor.line_length = OV8835MIPI_FULL_PERIOD_PIXEL_NUMS +OV8835MIPI_sensor.dummy_pixels;
				OV8835MIPI_sensor.frame_length = OV8835MIPI_FULL_PERIOD_LINE_NUMS+OV8835MIPI_sensor.dummy_lines;
				OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
				spin_unlock(&ov8835mipi_drv_lock);

				//#ifdef OV8835MIPI_USE_OTP			   
				#if 0
					ret = ov8835mipi_update_wb_register_from_otp();
				   if(1 == ret)
					   {
						   SENSORDB("ov8835mipi_update_wb_register_from_otp invalid\n");
					   }
				   else if(0 == ret)
					   {
						   SENSORDB("ov8835mipi_update_wb_register_from_otp success\n");
					   }
			   
				   ret = ov8835mipi_update_lenc_register_from_otp();
				   if(1 == ret)
					   {
						   SENSORDB("ov8835mipi_update_lenc_register_from_otp invalid\n");
					   }
				   else if(0 == ret)
					   {
						   SENSORDB("ov8835mipi_update_lenc_register_from_otp success\n");
					   }
				#endif
				
				OV8835MIPI_Set_Dummy(OV8835MIPI_sensor.dummy_pixels, OV8835MIPI_sensor.dummy_lines);
			   
			}
		else
			{
				spin_lock(&ov8835mipi_drv_lock);
				OV8835MIPI_sensor.pv_mode = KAL_FALSE;
				spin_unlock(&ov8835mipi_drv_lock);
				
				//if(OV8835MIPI_sensor.pv_mode == KAL_TRUE)
				#ifndef OV8835MIPI_4LANE
				OV8835MIPI_3264_2448_2Lane_15fps_Mclk26M_setting();
				SENSORDB("OV8835MIPI_FPS 15 \n");
				#else
                
                #ifdef USE_24FPS_SETTING
    				SENSORDB("HJDDbg, Capture2, OV8835MIPI_FPS 24fps \n");
                    Ov8835_3264x2448_4lane_24fps_216MSclk_780Mbps(); // Jiangde 24fps
                #else
    				OV8835MIPI_3264_2448_4LANE_30fps_Mclk26M_setting(); // Jiangde capture
    				SENSORDB("OV8835MIPI_FPS 30 \n");
                #endif
                
				#endif
				
				spin_lock(&ov8835mipi_drv_lock);
				OV8835MIPI_sensor.dummy_pixels = 0;
				OV8835MIPI_sensor.dummy_lines = 0;
				OV8835MIPI_sensor.line_length = OV8835MIPI_FULL_PERIOD_PIXEL_NUMS +OV8835MIPI_sensor.dummy_pixels;
				OV8835MIPI_sensor.frame_length = OV8835MIPI_FULL_PERIOD_LINE_NUMS+OV8835MIPI_sensor.dummy_lines;
				OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
				spin_unlock(&ov8835mipi_drv_lock);

				//cap_fps = OV8835MIPI_FPS(8);

			    //dummy_pixel=0;
				OV8835MIPI_Set_Dummy(OV8835MIPI_sensor.dummy_pixels, OV8835MIPI_sensor.dummy_lines);

				
				#if 0
						//dummy_pixel = OV8835MIPI_sensor.cap_pclk * OV8835MIPI_FPS(1) / (OV8835MIPI_FULL_PERIOD_LINE_NUMS * cap_fps);
						//dummy_pixel = dummy_pixel < OV8835MIPI_FULL_PERIOD_PIXEL_NUMS ? 0 : dummy_pixel - OV8835MIPI_FULL_PERIOD_PIXEL_NUMS;

						//OV8835MIPI_Set_Dummy(dummy_pixel, 0);
						
				/* shutter translation */
				//shutter = shutter * pv_line_length / OV8835MIPI_sensor.line_length;
				
				SENSORDB("pv shutter %d\n",shutter);
				SENSORDB("cap pclk %d\n",cap_pclk);
				SENSORDB("pv pclk %d\n",pv_pclk);
				SENSORDB("pv line length %d\n",pv_line_length);
				SENSORDB("cap line length %d\n",OV8835MIPI_sensor.line_length);

				//shutter = shutter * (cap_pclk / pv_pclk);
				//SENSORDB("pv shutter %d\n",shutter);
				//shutter = shutter * pv_line_length / OV8835MIPI_sensor.line_length;
				//SENSORDB("pv shutter %d\n",shutter);
				shutter = ((cap_pclk / 1000) * shutter) / (pv_pclk / 1000);
				SENSORDB("pv shutter %d\n",shutter);
				#ifdef OV8835_BINNING_SUM
				shutter = 2*(shutter * pv_line_length) /OV8835MIPI_sensor.line_length*94/100;//*101/107;
				#else
				shutter = (shutter * pv_line_length) /OV8835MIPI_sensor.line_length;
				#endif
				SENSORDB("cp shutter %d\n",shutter);
				
				//shutter *= 2;
				//shutter = ( shutter * cap_pclk * pv_line_length) / (pv_pclk * OV8835MIPI_sensor.line_length);
				OV8835MIPI_Write_Shutter(shutter);
				//set_OV8835MIPI_shutter(shutter);
				#endif
				
			}
		
		//OV8835MIPI_Set_Dummy(OV8835MIPI_sensor.dummy_pixels, OV8835MIPI_sensor.dummy_lines);
			
		//mdelay(80);
	}

	OV8835MIPISetFlipMirror(IMAGE_HV_MIRROR);	//add by lishengli 20130516
	mdelay(50);

#ifdef OV8835MIPI_DRIVER_TRACE
		SENSORDB("OV8835MIPICapture end\n");
#endif

	return ERROR_NONE;
}   /* OV8835MIPI_Capture() */
/*************************************************************************
* FUNCTION
*	OV8835MIPIVideo
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPIVideo(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	const kal_uint16 pv_line_length = OV8835MIPI_sensor.line_length;
	const kal_uint32 pv_pclk = OV8835MIPI_sensor.pv_pclk;
	const kal_uint32 cap_pclk = OV8835MIPI_sensor.cap_pclk;
	kal_uint32 shutter = OV8835MIPI_sensor.shutter;
	kal_uint16 dummy_pixel;
	//kal_uint32 temp;
	//kal_uint16 ret;
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPIVideo setting start \n");
#endif
	//if((OV8835MIPI_sensor.pv_mode == KAL_TRUE)||(OV8835MIPI_sensor.is_zsd == KAL_TRUE))
	//if(OV8835MIPI_sensor.pv_mode == KAL_TRUE)
	{
		// 
		spin_lock(&ov8835mipi_drv_lock);
		OV8835MIPI_sensor.video_mode = KAL_FALSE;
		OV8835MIPI_sensor.is_autofliker = KAL_FALSE;
		OV8835MIPI_sensor.sensorMode = OV8835MIPI_SENSOR_MODE_VIDEO;
		OV8835MIPI_sensor.pclk = OV8835MIPI_VIDEO_CLK;
		spin_unlock(&ov8835mipi_drv_lock);
			{
				spin_lock(&ov8835mipi_drv_lock);
				OV8835MIPI_sensor.pv_mode = KAL_FALSE;
				spin_unlock(&ov8835mipi_drv_lock);
				
				#ifndef OV8835MIPI_4LANE
				OV8835MIPI_1632_1224_2Lane_30fps_Mclk26M_setting();
				SENSORDB("OV8835MIPI_FPS 30 \n");
				#else
				OV8835MIPI_3264_1836_4LANE_30fps_Mclk26M_setting(); // Jiangde Video, change to 1080p
				SENSORDB("OV8835MIPI_FPS 30 \n");
				#endif
				
				spin_lock(&ov8835mipi_drv_lock);
				OV8835MIPI_sensor.dummy_pixels = 0;
				OV8835MIPI_sensor.dummy_lines = 0;
				OV8835MIPI_sensor.line_length = OV8835MIPI_VIDEO_PERIOD_PIXEL_NUMS + OV8835MIPI_sensor.dummy_pixels;
				OV8835MIPI_sensor.frame_length = OV8835MIPI_VIDEO_PERIOD_LINE_NUMS + OV8835MIPI_sensor.dummy_lines;
				OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
				spin_unlock(&ov8835mipi_drv_lock);

				//cap_fps = OV8835MIPI_FPS(8);
				//SENSORDB("OV8835MIPI_FPS 15 \n");

			    //dummy_pixel=0;
				OV8835MIPI_Set_Dummy(OV8835MIPI_sensor.dummy_pixels, OV8835MIPI_sensor.dummy_lines);
			}
	}
	OV8835MIPISetFlipMirror(IMAGE_HV_MIRROR);	//add by lishengli 20130516
	mdelay(50);

#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPIVideo end\n");
#endif

	return ERROR_NONE;
}   /* OV8835MIPI_Capture() */

UINT32 OV8835MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
#ifdef OV8835MIPI_DRIVER_TRACE
		SENSORDB("OV8835MIPIGetResolution \n");
#endif		
//#ifdef ACDK
//	pSensorResolution->SensorFullWidth=OV8835MIPI_IMAGE_SENSOR_PV_WIDTH;
//	pSensorResolution->SensorFullHeight=OV8835MIPI_IMAGE_SENSOR_PV_HEIGHT;
//#else
	pSensorResolution->SensorFullWidth=OV8835MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight=OV8835MIPI_IMAGE_SENSOR_FULL_HEIGHT;
//#endif

	pSensorResolution->SensorPreviewWidth=OV8835MIPI_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight=OV8835MIPI_IMAGE_SENSOR_PV_HEIGHT;
	
    pSensorResolution->SensorVideoWidth		= OV8835MIPI_IMAGE_SENSOR_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight    = OV8835MIPI_IMAGE_SENSOR_VIDEO_HEIGHT;
	return ERROR_NONE;
}	/* OV8835MIPIGetResolution() */

UINT32 OV8835MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPIGetInfo£¬FeatureId:%d\n",ScenarioId);
#endif
#if 1
	switch(ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorPreviewResolutionX=OV8835MIPI_IMAGE_SENSOR_FULL_WIDTH;
			pSensorInfo->SensorPreviewResolutionY=OV8835MIPI_IMAGE_SENSOR_FULL_HEIGHT;
			pSensorInfo->SensorCameraPreviewFrameRate = 15;
		break;
		default:
  	        pSensorInfo->SensorPreviewResolutionX=OV8835MIPI_IMAGE_SENSOR_PV_WIDTH;
			pSensorInfo->SensorPreviewResolutionY=OV8835MIPI_IMAGE_SENSOR_PV_HEIGHT;
			pSensorInfo->SensorCameraPreviewFrameRate = 30;
		break;
	}

	//pSensorInfo->SensorPreviewResolutionX=OV8835MIPI_IMAGE_SENSOR_PV_WIDTH;
	//pSensorInfo->SensorPreviewResolutionY=OV8835MIPI_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX=OV8835MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=OV8835MIPI_IMAGE_SENSOR_FULL_HEIGHT;

	//pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	#ifndef OV8835MIPI_4LANE
	pSensorInfo->SensorStillCaptureFrameRate=10;
	#else
	#ifdef OV8835MIPI_4LANE_CAP_30FPS
	pSensorInfo->SensorStillCaptureFrameRate=30;			
	#else
	pSensorInfo->SensorStillCaptureFrameRate=28;//260M=3608*2522*28.6
	#endif
	#endif

	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE; //low active
	pSensorInfo->SensorResetDelayCount=5; 
#endif
	pSensorInfo->SensorOutputDataFormat=OV8835MIPI_COLOR_FORMAT;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
#if 1
	pSensorInfo->SensorInterruptDelayLines = 4;
	
	//#ifdef MIPI_INTERFACE
   		pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_MIPI;
   	//#else
   	//	pSensorInfo->SensroInterfaceType		= SENSOR_INTERFACE_TYPE_PARALLEL;
   	//#endif

/*    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=TRUE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=TRUE;*/
#endif
	pSensorInfo->CaptureDelayFrame = 2; 
	pSensorInfo->PreviewDelayFrame = 3; 
	pSensorInfo->VideoDelayFrame = 1; 	

	pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;
    pSensorInfo->AEShutDelayFrame = 0;		   /* The frame of setting shutter default 0 for TG int */
	pSensorInfo->AESensorGainDelayFrame = 0;	   /* The frame of setting sensor gain */
	pSensorInfo->AEISPGainDelayFrame = 2;    
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_PV_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_PV_Y_START; 
			
			#ifndef OV8835MIPI_4LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		
			#else
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE; 	
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
		break;
		
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_PV_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_PV_Y_START; 
			
			#ifndef OV8835MIPI_4LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		
			#else
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE; 	
			#endif
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
		break;
		
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount= 3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_FULL_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_FULL_Y_START; 

			#ifndef OV8835MIPI_4LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		
			#else
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE; 	
			#endif		
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
		break;
		default:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;		
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_PV_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_PV_Y_START; 
		
			#ifndef OV8835MIPI_4LANE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		
			#else
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE; 	
			#endif		
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
		break;
	}
	pSensorInfo->SensorGrabStartX = pSensorInfo->SensorGrabStartX-1; 
	pSensorInfo->SensorGrabStartY = pSensorInfo->SensorGrabStartY-1; 
#if 0
	//OV8835MIPIPixelClockDivider=pSensorInfo->SensorPixelClockCount;
	memcpy(pSensorConfigData, &OV8835MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
#endif		
  return ERROR_NONE;
}	/* OV8835MIPIGetInfo() */


UINT32 OV8835MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPIControl£¬FeatureId:%d\n",ScenarioId);
#endif	

	spin_lock(&ov8835mipi_drv_lock);
	OV8835MIPIMIPIRAWCurrentScenarioId = ScenarioId;
	spin_unlock(&ov8835mipi_drv_lock);

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			OV8835MIPIPreview(pImageWindow, pSensorConfigData);
			break;
			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			OV8835MIPIVideo(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			if(OV8835MIPI_sensor.is_zsd == KAL_TRUE)
				{
					spin_lock(&ov8835mipi_drv_lock);
					OV8835MIPI_sensor.is_zsd_cap = KAL_TRUE;
					spin_unlock(&ov8835mipi_drv_lock);
					SENSORDB("OV8835MIPIControl£¬is_zsd_cap is true!\n");
				}
			else
				{
					spin_lock(&ov8835mipi_drv_lock);
					OV8835MIPI_sensor.is_zsd_cap = KAL_FALSE;
					spin_unlock(&ov8835mipi_drv_lock);
					SENSORDB("OV8835MIPIControl£¬is_zsd_cap is false!\n");
				}
			
			OV8835MIPICapture(pImageWindow, pSensorConfigData);
			break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.is_zsd = KAL_TRUE;  //for zsd full size preview
			OV8835MIPI_sensor.is_zsd_cap = KAL_FALSE;
			spin_unlock(&ov8835mipi_drv_lock);
			OV8835MIPICapture(pImageWindow, pSensorConfigData);
		break;		
        default:
            return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}	/* OV8835MIPIControl() */

UINT32 OV8835MIPISetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	
	//kal_uint32 pv_max_frame_rate_lines = OV8835MIPI_sensor.dummy_lines;
	
	SENSORDB("[OV8835MIPISetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);

	if(bEnable)
		{
		    
			spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.is_autofliker = KAL_TRUE;
			spin_unlock(&ov8835mipi_drv_lock);

			//if(OV8835MIPI_sensor.video_mode = KAL_TRUE)
			//	{
			//		pv_max_frame_rate_lines = OV8835MIPI_sensor.frame_length + (OV8835MIPI_sensor.frame_length>>7);
			//		OV8835MIPI_write_cmos_sensor(0x380e, (pv_max_frame_rate_lines>>8)&0xFF);
			//		OV8835MIPI_write_cmos_sensor(0x380f, (pv_max_frame_rate_lines)&0xFF);
			//	}	
		}
	else
		{
			spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.is_autofliker = KAL_FALSE;
			spin_unlock(&ov8835mipi_drv_lock);
			
			//if(OV8835MIPI_sensor.video_mode = KAL_TRUE)
			//	{
			//		pv_max_frame_rate_lines = OV8835MIPI_sensor.frame_length;
			//		OV8835MIPI_write_cmos_sensor(0x380e, (pv_max_frame_rate_lines>>8)&0xFF);
			//		OV8835MIPI_write_cmos_sensor(0x380f, (pv_max_frame_rate_lines)&0xFF);
			//	}	
		}
	SENSORDB("[OV8835MIPISetAutoFlickerMode]bEnable:%x \n",bEnable);
	return 0;
}


UINT32 OV8835MIPISetVideoMode(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
	UINT16 frameRate;
    /* to fix VSYNC, to fix frame rate */
#ifdef OV8835MIPI_DRIVER_TRACE
	SENSORDB("OV8835MIPISetVideoMode, u2FrameRate:%d\n",u2FrameRate);
#endif	
	if(u2FrameRate==0)
	{
		SENSORDB("Disable Video Mode or dynimac fps\n");
		spin_lock(&ov8835mipi_drv_lock);
		OV8835MIPI_sensor.video_mode = KAL_FALSE;
		spin_unlock(&ov8835mipi_drv_lock);
		return KAL_TRUE;
	}

	if(OV8835MIPI_sensor.is_autofliker == KAL_TRUE)
	{
		if (u2FrameRate==30)
			frameRate= 306;
		else if(u2FrameRate==15)
			frameRate= 148;//148;
		else
			frameRate=u2FrameRate*10;
	}
	else
		frameRate=u2FrameRate*10;

	OV8835MIPISetMaxFrameRate(frameRate);
	spin_lock(&ov8835mipi_drv_lock);
	OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
	OV8835MIPI_sensor.video_mode = KAL_TRUE;
	spin_unlock(&ov8835mipi_drv_lock);


#if 0
    if((30 == u2FrameRate)||(15 == u2FrameRate))
    	{
			if( OV8835MIPIMIPIRAWCurrentScenarioId == MSDK_SCENARIO_ID_CAMERA_ZSD)
				dummy_line = OV8835MIPI_sensor.cap_pclk / u2FrameRate / OV8835MIPI_FULL_PERIOD_PIXEL_NUMS - OV8835MIPI_FULL_PERIOD_LINE_NUMS;
			else
				dummy_line = OV8835MIPI_sensor.pv_pclk / u2FrameRate / OV8835MIPI_PV_PERIOD_PIXEL_NUMS - OV8835MIPI_PV_PERIOD_LINE_NUMS;
				
			if (dummy_line < 0) dummy_line = 0;
         #ifdef OV8835MIPI_DRIVER_TRACE
			SENSORDB("dummy_line %d\n", dummy_line);
         #endif
		 
			OV8835MIPI_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
		 
		 	spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.video_mode = KAL_TRUE;
			spin_unlock(&ov8835mipi_drv_lock);
			
    	}
#endif
    return KAL_TRUE;
}
UINT32 OV8835MIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint16 pclk, lineLength;
	kal_int16 dummyLine;
	kal_uint16 frame_length;
	OV8835MIPIMIPIRAWCurrentScenarioId = scenarioId;

	SENSORDB("OV8835MIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = OV8835MIPI_PREVIEW_CLK;
			lineLength = OV8835MIPI_PV_PERIOD_PIXEL_NUMS;
			frame_length = (10 * pclk)/frameRate/lineLength;
			dummyLine = frame_length - OV8835MIPI_PV_PERIOD_LINE_NUMS;
			//OV8835MIPI_sensor.sensorMode = SENSOR_MODE_PREVIEW;
			OV8835MIPI_Set_Dummy(0, dummyLine);		
			spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
			spin_unlock(&ov8835mipi_drv_lock);	
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = OV8835MIPI_VIDEO_CLK;
			lineLength = OV8835MIPI_VIDEO_PERIOD_PIXEL_NUMS;
			frame_length = (10 * pclk)/frameRate/lineLength;
			dummyLine = frame_length - OV8835MIPI_VIDEO_PERIOD_LINE_NUMS;
			//OV8835MIPI_sensor.sensorMode = SENSOR_MODE_VIDEO;
			OV8835MIPI_Set_Dummy(0, dummyLine);		
			spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
			spin_unlock(&ov8835mipi_drv_lock);	
			break;			
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = OV8835MIPI_CAPTURE_CLK;
			lineLength = OV8835MIPI_FULL_PERIOD_PIXEL_NUMS;
			frame_length = (10 * pclk)/frameRate/lineLength;
			dummyLine = frame_length - OV8835MIPI_FULL_PERIOD_LINE_NUMS;
			//OV8835MIPI_sensor.sensorMode = SENSOR_MODE_CAPTURE;
			OV8835MIPI_Set_Dummy(0, dummyLine);	
			spin_lock(&ov8835mipi_drv_lock);
			OV8835MIPI_sensor.max_exposure_lines = OV8835MIPI_sensor.frame_length;
			spin_unlock(&ov8835mipi_drv_lock);		
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
			break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			break;		
		default:
			break;
	}	
	return ERROR_NONE;
}


UINT32 OV8835MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			#ifndef OV8835MIPI_4LANE
			*pframeRate = 150;
			#else	
			#ifdef OV8835MIPI_4LANE_CAP_30FPS
			*pframeRate = 300;
			#else
			*pframeRate = 280;		
			#endif
			#endif
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			 *pframeRate = 300;
			break;		
		default:
			break;
	}

	return ERROR_NONE;
}


UINT32 OV8835MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	UINT32 OV8835MIPISensorRegNumber;
	UINT32 i;
	//PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
	//MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

#ifdef OV8835MIPI_DRIVER_TRACE
	//SENSORDB("OV8835MIPIFeatureControl£¬FeatureId:%d\n",FeatureId); 
#endif		
	switch (FeatureId)
	{
	
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=OV8835MIPI_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=OV8835MIPI_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:	/* 3 */
			//switch(OV8835MIPIMIPIRAWCurrentScenarioId)
				{
					/*case MSDK_SCENARIO_ID_CAMERA_ZSD:
						*pFeatureReturnPara16++= OV8835MIPI_FeatureControl_PERIOD_PixelNum;
						*pFeatureReturnPara16= OV8835MIPI_FeatureControl_PERIOD_LineNum;
						*pFeatureParaLen=4;
			            #ifdef OV8835MIPI_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PERIOD£¬OV8835MIPI cap line length:%d\n",OV8835MIPI_FULL_PERIOD_PIXEL_NUMS + OV8835MIPI_sensor.dummy_pixels); 
			            #endif	
						break;

						
					default:*/
						*pFeatureReturnPara16++= OV8835MIPI_sensor.line_length;
						*pFeatureReturnPara16= OV8835MIPI_sensor.frame_length;
						*pFeatureParaLen=4;
			            #ifdef OV8835MIPI_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PERIOD£¬OV8835MIPI pv line length:%d\n",OV8835MIPI_FeatureControl_PERIOD_PixelNum); 
			            #endif	
						break;
				}		
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:  /* 3 */
			switch(OV8835MIPIMIPIRAWCurrentScenarioId)
				{
				  /* case MSDK_SCENARIO_ID_CAMERA_ZSD:
						*pFeatureReturnPara32 = OV8835MIPI_ZSD_PRE_CLK; //OV8835MIPI_sensor.cap_pclk;
						*pFeatureParaLen=4;
						#ifdef OV8835MIPI_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬OV8835MIPI_ZSD_PRE_CLK:%d\n",OV8835MIPI_ZSD_PRE_CLK); 
			            #endif
						break;*/

						
						case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
							*pFeatureReturnPara32 = OV8835MIPI_PREVIEW_CLK;
							*pFeatureParaLen=4;
							#ifdef OV8835MIPI_DRIVER_TRACE
				          	SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬OV8835MIPI_PREVIEW_CLK:%d\n",OV8835MIPI_PREVIEW_CLK); 
			            	#endif
							break;
						case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
							*pFeatureReturnPara32 = OV8835MIPI_VIDEO_CLK;
							*pFeatureParaLen=4;
							#ifdef OV8835MIPI_DRIVER_TRACE
				          	SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬MSDK_SCENARIO_ID_VIDEO_PREVIEW:%d\n",OV8835MIPI_VIDEO_CLK); 
			            	#endif
							break;
						case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
						case MSDK_SCENARIO_ID_CAMERA_ZSD:
							*pFeatureReturnPara32 = OV8835MIPI_CAPTURE_CLK;
							*pFeatureParaLen=4;
							#ifdef OV8835MIPI_DRIVER_TRACE
				          	SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬OV8835MIPI_CAPTURE_CLK/ZSD:%d\n",OV8835MIPI_CAPTURE_CLK); 
			            	#endif
							break;
							
						default:
							*pFeatureReturnPara32 = OV8835MIPI_sensor.pv_pclk;
							*pFeatureParaLen=4;
							#ifdef OV8835MIPI_DRIVER_TRACE
							SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬OV8835MIPI_sensor.pv_pclk:%d\n",OV8835MIPI_sensor.pv_pclk); 
							#endif
							break;
				}
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:	/* 4 */
			set_OV8835MIPI_shutter(*pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			//OV8835MIPI_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:	/* 6 */
			OV8835MIPI_SetGain((UINT16) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
		case SENSOR_FEATURE_SET_REGISTER:
		OV8835MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = OV8835MIPI_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
			//memcpy(&OV8835MIPI_sensor.eng.cct, pFeaturePara, sizeof(OV8835MIPI_sensor.eng.cct));
			OV8835MIPISensorRegNumber = OV8835MIPI_FACTORY_END_ADDR;
			for (i=0;i<OV8835MIPISensorRegNumber;i++)
            {
                spin_lock(&ov8835mipi_drv_lock);
                OV8835MIPI_sensor.eng.cct[i].Addr=*pFeatureData32++;
                OV8835MIPI_sensor.eng.cct[i].Para=*pFeatureData32++;
			    spin_unlock(&ov8835mipi_drv_lock);
            }
			
		break;
		case SENSOR_FEATURE_GET_CCT_REGISTER:	/* 12 */
			if (*pFeatureParaLen >= sizeof(OV8835MIPI_sensor.eng.cct) + sizeof(kal_uint32))
			{
			  *((kal_uint32 *)pFeaturePara++) = sizeof(OV8835MIPI_sensor.eng.cct);
			  memcpy(pFeaturePara, &OV8835MIPI_sensor.eng.cct, sizeof(OV8835MIPI_sensor.eng.cct));
			}
			break;
		case SENSOR_FEATURE_SET_ENG_REGISTER:
			//memcpy(&OV8835MIPI_sensor.eng.reg, pFeaturePara, sizeof(OV8835MIPI_sensor.eng.reg));
			OV8835MIPISensorRegNumber = OV8835MIPI_ENGINEER_END;
			for (i=0;i<OV8835MIPISensorRegNumber;i++)
            {
                spin_lock(&ov8835mipi_drv_lock);
                OV8835MIPI_sensor.eng.reg[i].Addr=*pFeatureData32++;
                OV8835MIPI_sensor.eng.reg[i].Para=*pFeatureData32++;
			    spin_unlock(&ov8835mipi_drv_lock);
            }
			break;
		case SENSOR_FEATURE_GET_ENG_REGISTER:	/* 14 */
			if (*pFeatureParaLen >= sizeof(OV8835MIPI_sensor.eng.reg) + sizeof(kal_uint32))
			{
			  *((kal_uint32 *)pFeaturePara++) = sizeof(OV8835MIPI_sensor.eng.reg);
			  memcpy(pFeaturePara, &OV8835MIPI_sensor.eng.reg, sizeof(OV8835MIPI_sensor.eng.reg));
			}
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->Version = NVRAM_CAMERA_SENSOR_FILE_VERSION;
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorId = OV8835MIPI_SENSOR_ID;
			memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorEngReg, &OV8835MIPI_sensor.eng.reg, sizeof(OV8835MIPI_sensor.eng.reg));
			memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorCCTReg, &OV8835MIPI_sensor.eng.cct, sizeof(OV8835MIPI_sensor.eng.cct));
			*pFeatureParaLen = sizeof(NVRAM_SENSOR_DATA_STRUCT);
			break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pFeaturePara, &OV8835MIPI_sensor.cfg_data, sizeof(OV8835MIPI_sensor.cfg_data));
			*pFeatureParaLen = sizeof(OV8835MIPI_sensor.cfg_data);
			break;
		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		     OV8835MIPI_camera_para_to_sensor();
		break;
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
			OV8835MIPI_sensor_to_camera_para();
		break;							
		case SENSOR_FEATURE_GET_GROUP_COUNT:
			OV8835MIPI_get_sensor_group_count((kal_uint32 *)pFeaturePara);
			*pFeatureParaLen = 4;
		break;										
		  OV8835MIPI_get_sensor_group_info((MSDK_SENSOR_GROUP_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_GET_ITEM_INFO:
		  OV8835MIPI_get_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_SET_ITEM_INFO:
		  OV8835MIPI_set_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_GET_ENG_INFO:
     		memcpy(pFeaturePara, &OV8835MIPI_sensor.eng_info, sizeof(OV8835MIPI_sensor.eng_info));
     		*pFeatureParaLen = sizeof(OV8835MIPI_sensor.eng_info);
     		break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		       OV8835MIPISetVideoMode(*pFeatureData16);
        break; 
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV8835MIPIGetSensorID(pFeatureReturnPara32); 
            break; 
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			OV8835MIPISetAutoFlickerMode((BOOL)*pFeatureData16,*(pFeatureData16+1));
			break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            OV8835MIPISetTestPatternMode((BOOL)*pFeatureData16);
            break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			OV8835MIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			OV8835MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE://for factory mode auto testing 			
            *pFeatureReturnPara32= OV8835MIPI_TEST_PATTERN_CHECKSUM;
			*pFeatureParaLen=4; 							
			break;
		default:
			break;
	}
	return ERROR_NONE;
}	/* OV8835MIPIFeatureControl() */
SENSOR_FUNCTION_STRUCT	SensorFuncOV8835MIPI=
{
	OV8835MIPIOpen,
	OV8835MIPIGetInfo,
	OV8835MIPIGetResolution,
	OV8835MIPIFeatureControl,
	OV8835MIPIControl,
	OV8835MIPIClose
};

UINT32 OV8835_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV8835MIPI;

	return ERROR_NONE;
}	/* SensorInit() */



