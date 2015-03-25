/*
NOTE:
The modification is appended to initialization of image sensor. 
After sensor initialization, use the function
bool otp_update_wb(unsigned short golden_rg, unsigned short golden_bg),
then the calibration of AWB will be applied. 
After finishing the OTP written, we will provide you the golden_rg and golden_bg settings.
*/
/*
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
	
#include "ov5648_Sensor.h"
#include "ov5648_Camera_Sensor_para.h"
#include "ov5648_CameraCustomized.h"
*/


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

#include "ov5648mipi_Sensor.h"
#include "ov5648mipi_Camera_Sensor_para.h"
#include "ov5648mipi_CameraCustomized.h"


//extern kal_uint16 OV5648_write_cmos_sensor(kal_uint32 addr, kal_uint32 para);
//extern kal_uint16 OV5648_read_cmos_sensor(kal_uint32 addr);


#undef TRACE
//#define TRACE(fmt, args...) printk(KERN_INFO "ov5648_OTP.c: " fmt, ## args)
#define TRACE(fmt, arg...) printk("[OV5648MIPIRaw_OTP]%s: " fmt "\n", __FUNCTION__ ,##arg)//LINE <> <DATE20130923> <ov5648 OTP log> wupingzhou


extern kal_uint16 OV5648MIPI_read_cmos_sensor(kal_uint32 addr);
extern kal_uint16 OV5648MIPI_write_cmos_sensor(kal_uint32 addr, kal_uint32 para);

//#define OV5648_write_cmos_sensor(kal_uint32 addr, kal_uint32 para) OV5648MIPI_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
//#define OV5648_read_cmos_sensor(kal_uint32 addr) OV5648MIPI_read_cmos_sensor(kal_uint32 addr)

#define OV5648_write_cmos_sensor(addr,para) OV5648MIPI_write_cmos_sensor(addr,para)
#define OV5648_read_cmos_sensor(addr) OV5648MIPI_read_cmos_sensor(addr)

//#define SUPPORT_FLOATING

#define OTP_DATA_ADDR         0x3D00
#define OTP_LOAD_ADDR         0x3D81

#define OTP_WB_GROUP_ADDR     0x3D05
#define OTP_WB_GROUP_SIZE     9
#define OTP_BANK_ADDR         0x3D84
#define OTP_BANK              0x3D85
#define OTP_END_ADDR          0x3D86

#define GAIN_RH_ADDR          0x5186
#define GAIN_RL_ADDR          0x5187
#define GAIN_GH_ADDR          0x5188
#define GAIN_GL_ADDR          0x5189
#define GAIN_BH_ADDR          0x518A
#define GAIN_BL_ADDR          0x518B

#define GAIN_DEFAULT_VALUE    0x0400 // 1x gain

#define OTP_MID               0x02


// R/G and B/G of current camera module
unsigned char RG_MSB = 0;
unsigned char BG_MSB = 0;
unsigned char AWB_LSB = 0;


// Enable OTP read function
void otp_read_enable(void)
{
	OV5648_write_cmos_sensor(OTP_LOAD_ADDR, 0x01);
	mdelay(15); // sleep > 10ms
}

// Disable OTP read function
void otp_read_disable(void)
{
	OV5648_write_cmos_sensor(OTP_LOAD_ADDR, 0x00);
}

void otp_read(unsigned short otp_addr, unsigned char* otp_data)
{
	otp_read_enable();
	*otp_data = OV5648_read_cmos_sensor(otp_addr);
	otp_read_disable();
}

/*******************************************************************************
* Function    :  otp_clear
* Description :  Clear OTP buffer 
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
void otp_clear(void)
{
	// After read/write operation, the OTP buffer should be cleared to avoid accident write
	unsigned char i;
	for (i=0; i<16; i++) 
	{
		OV5648_write_cmos_sensor(OTP_DATA_ADDR+i, 0x00);
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
signed char otp_check_wb_group(unsigned char index)
{   
	unsigned short otp_addr = OTP_WB_GROUP_ADDR + index * OTP_WB_GROUP_SIZE;
	unsigned char  flag;

    	if (index > 2)
	{
		TRACE("OTP input wb group index %d error\n", index);
		return -1;
	}
   	else if (index < 2)
   	{
   		// select bank 0
   		OV5648_write_cmos_sensor(OTP_BANK_ADDR, 0xc0);
   		OV5648_write_cmos_sensor(OTP_BANK, 0x00);
   		OV5648_write_cmos_sensor(OTP_END_ADDR, 0x0f);	
   	}
   	else /*index==2*/
   	{
   		// select bank 1
   		OV5648_write_cmos_sensor(OTP_BANK_ADDR, 0xc0);
   		OV5648_write_cmos_sensor(OTP_BANK, 0x10);
   		OV5648_write_cmos_sensor(OTP_END_ADDR, 0x1f);	
   		otp_addr = 0x3D07;
   	}
    
	otp_read(otp_addr, &flag);
	otp_clear();

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	if (!flag)
	{
		TRACE("wb group %d is empty\n", index);
		return 0;
	}
	else if ((!(flag&0x80)) && ((flag&0x7f) == OTP_MID))
	{
		TRACE("wb group %d has valid data\n", index);
		return 2;
	}
	else
	{
		TRACE("wb group %d has invalid data\n", index);
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
signed char otp_read_wb_group(signed char index)
{
	unsigned short awb_addr;
	unsigned char  mid;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_wb_group(index) == 2)
			{
				TRACE("read wb from group %d", index);
				break;
			}
		}

		if (index > 2)
		{
			TRACE("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_wb_group(index) != 2)
		{
			TRACE("read wb from group %d failed\n", index);
			return -1;
		}
	}


	if (index == 0)
	{
		// select bank 0
   		OV5648_write_cmos_sensor(OTP_BANK_ADDR, 0xc0);
   		OV5648_write_cmos_sensor(OTP_BANK, 0x00);
   		OV5648_write_cmos_sensor(OTP_END_ADDR, 0x0f);	
   		otp_read(OTP_WB_GROUP_ADDR+2, &RG_MSB);
		otp_read(OTP_WB_GROUP_ADDR+3, &BG_MSB);
		otp_read(OTP_WB_GROUP_ADDR+6, &AWB_LSB);
	}
	else /* (index == 1 || index ==2)*/
	{
		awb_addr = OTP_DATA_ADDR + (index - 1) * OTP_WB_GROUP_SIZE;
		// select bank 1
   		OV5648_write_cmos_sensor(OTP_BANK_ADDR, 0xc0);
   		OV5648_write_cmos_sensor(OTP_BANK, 0x10);
   		OV5648_write_cmos_sensor(OTP_END_ADDR, 0x1f);	
   		otp_read(awb_addr, &RG_MSB);
		otp_read(awb_addr+1, &BG_MSB);
		otp_read(awb_addr+4, &AWB_LSB);	
	}


	otp_clear();

	TRACE("read wb finished\n");
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
bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;
    	unsigned short rg_ratio = (RG_MSB<<2) | ((AWB_LSB & 0xC0)>>6);
    	unsigned short bg_ratio = (BG_MSB<<2) | ((AWB_LSB & 0x30)>>4);
	double ratio_r, ratio_g, ratio_b;
	double cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		TRACE("golden_rg / golden_bg can not be zero\n");
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
		OV5648_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV5648_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1)
	{
		gain_g = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_g);
		OV5648_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV5648_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1)
	{
		gain_b = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_b);
		OV5648_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV5648_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	TRACE("cmp_rg=%f, cmp_bg=%f\n", cmp_rg, cmp_bg);
	TRACE("ratio_r=%f, ratio_g=%f, ratio_b=%f\n", ratio_r, ratio_g, ratio_b);
	TRACE("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}

#else //Use this if not support floating point values

#define OTP_MULTIPLE_FAC	10000
bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;
    unsigned short rg_ratio = (RG_MSB<<2) | ((AWB_LSB & 0xC0)>>6);
    unsigned short bg_ratio = (BG_MSB<<2) | ((AWB_LSB & 0x30)>>4);
	unsigned short ratio_r, ratio_g, ratio_b;
	unsigned short cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		TRACE("golden_rg / golden_bg can not be zero\n");
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
		OV5648_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV5648_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1 * OTP_MULTIPLE_FAC)
	{
		gain_g = GAIN_DEFAULT_VALUE * ratio_g / OTP_MULTIPLE_FAC;
		OV5648_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV5648_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1 * OTP_MULTIPLE_FAC)
	{
		gain_b = GAIN_DEFAULT_VALUE * ratio_b / OTP_MULTIPLE_FAC;
		OV5648_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV5648_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	TRACE("------golden_rg=0x%x, golden_bg=0x%x\n", golden_rg, golden_bg);//LINE <> <DATE20130923> <ov5648 OTP log> wupingzhou
	TRACE("------current rg_ratio=0x%x, bg_ratio=0x%x\n", rg_ratio, bg_ratio);//LINE <> <DATE20130923> <ov5648 OTP log> wupingzhou
	TRACE("cmp_rg=%d, cmp_bg=%d\n", cmp_rg, cmp_bg);
	TRACE("ratio_r=%d, ratio_g=%d, ratio_b=%d\n", ratio_r, ratio_g, ratio_b);
	TRACE("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
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
bool otp_update_wb(unsigned short golden_rg, unsigned short golden_bg) 
{
	TRACE("start wb update\n");

	if (otp_read_wb_group(-1) != -1)
	{
		if (otp_apply_wb(golden_rg, golden_bg) == 1)
		{
		       mdelay(1);
			TRACE("wb update finished\n");
			// mdelay(1);
			//TRACE("the current truly module's R/G value is  0x%x", RG_MSB );
			 //mdelay(1);
			//TRACE("the current truly module's B/G value is  0x%x", BG_MSB );
			return 1;
		}
	}

	TRACE("wb update failed\n");
	return 0;
}


signed char check_trulyMID(signed char index)
	{


/*
		if(otp_read_wb_group(index)!=-1)		//it is truly module
		
			{

				TRACE("it is truly module\n");
				
				return 1;		
			}
		
		else
			
			{	

				TRACE("it is not truly module\n");
				
				return 0;
			}

*/
 		//TRACE("the value of 0x0103 register is  0x%x",OV5648_read_cmos_sensor(0x0103));		
		//mdelay(1);
		//TRACE("the value of 0x0100 register is  0x%x",OV5648_read_cmos_sensor(0x0100));		
		//mdelay(1);

		//enable to read the otp infomation
		OV5648_write_cmos_sensor(0x0103, 0x01);
		OV5648_write_cmos_sensor(0x0100, 0x01);
		mdelay(10);
		
		for (index=0; index<3; index++)
		{
			if (otp_check_wb_group(index) == 2)
			{
			
				TRACE("check_trulyMID sucessed %d", index);		

				//disable to read the otp infomation
				OV5648_write_cmos_sensor(0x0103, 0x00);
				OV5648_write_cmos_sensor(0x0100, 0x00);
				mdelay(10);
				return 1;
			}

			
		}

		TRACE("check_trulyMID failed");
		
		//disable to read the otp infomation
		OV5648_write_cmos_sensor(0x0103, 0x00);
		OV5648_write_cmos_sensor(0x0100, 0x00);
		mdelay(10);
		return 0;
		
	}


