/*************************************************************************************************
a5142_otp.c
---------------------------------------------------------
OTP Application file From Truly for AR0543
2012.10.26
---------------------------------------------------------
NOTE:
The modification is appended to initialization of image sensor. 
After sensor initialization, use the function , and get the id value.
bool otp_wb_update(BYTE zone)
and
bool otp_lenc_update(BYTE zone), 
then the calibration of AWB and LSC will be applied. 
After finishing the OTP written, we will provide you the golden_rg and golden_bg settings.
**************************************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "a5142mipi_Sensor.h"
#include "a5142mipi_Camera_Sensor_para.h"
#include "a5142mipi_CameraCustomized.h"

#include "a5142_otp.h"

#define A5142MIPI_DEBUG
#ifdef A5142MIPI_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define TRULY_ID           0x0001
#define LSC_REGSIZE        106

#define GAIN_DEFAULT       0x112A
#define GAIN_GREEN1_ADDR   0x3056
#define GAIN_BLUE_ADDR     0x3058
#define GAIN_RED_ADDR      0x305A
#define GAIN_GREEN2_ADDR   0x305C

USHORT golden_r;
USHORT golden_gr;
USHORT golden_gb;
USHORT golden_b;

USHORT current_r;
USHORT current_gr;
USHORT current_gb;
USHORT current_b;

extern kal_uint16 A5142MIPI_write_cmos_sensor(kal_uint32 addr, kal_uint32 para);
extern kal_uint16 A5142MIPI_read_cmos_sensor(kal_uint32 addr);

USHORT otp_reglist[] = 
{
	0x381e, //LSC Start
	0x3820,
	0x3822,
	0x3824,
	0x3826,
	0x3828,
	0x382a,
	0x382c,
	0x382e,
	0x3830,      
	0x3832,
	0x3834,
	0x3836,
	0x3838,
	0x383a,
	0x383c,
	0x383e,
	0x3840,      
	0x3842,
	0x3844,
	0x3846,
	0x3848,
	0x384a,
	0x384c,
	0x384e,
	0x3850,      
	0x3852,
	0x3854,
	0x3856,
	0x3858,
	0x385a,
	0x385c,
	0x385e,
	0x3860,      
	0x3862,
	0x3864,
	0x3866,
	0x3868,
	0x386a,
	0x386c,
	0x386e,
	0x3870,      
	0x3872,
	0x3874,
	0x3876,
	0x3878,
	0x387a,
	0x387c,
	0x387e,
	0x3880,      
	0x3882,
	0x3884,
	0x3886,
	0x3888,
	0x388a,
	0x388c,
	0x388e,
	0x3890,      
	0x3892,
	0x3894,
	0x3896,
	0x3898,
	0x389a,
	0x389c,
	0x389e,
	0x38a0,      
	0x38a2,
	0x38a4,
	0x38a6,
	0x38a8,
	0x38aa,
	0x38ac,
	0x38ae,
	0x38b0,      
	0x38b2,
	0x38b4,
	0x38b6,
	0x38b8,
	0x38ba,
	0x38bc,
	0x38be,
	0x38c0,      
	0x38c2,
	0x38c4,
	0x38c6,
	0x38c8,
	0x38ca,
	0x38cc,
	0x38ce,
	0x38d0,      
	0x38d2,
	0x38d4,
	0x38d6,
	0x38d8,
	0x38da,
	0x38dc,
	0x38de,
	0x38e0,      
	0x38e2,
	0x38e4,
	0x38e6,
	0x38e8,
	0x38ea,
	0x38ec,
	0x38ee,
	0x38f0,
};

USHORT lsc_reglist[] = 
{
   //A5141 A8141
	0x3600, 
	0x3602, 
	0x3604, 
	0x3606, 
	0x3608, 
	0x360A, 
	0x360C, 
	0x360E, 
	0x3610, 
	0x3612, 
	0x3614, 
	0x3616, 
	0x3618, 
	0x361A, 
	0x361C, 
	0x361E, 
	0x3620, 
	0x3622, 
	0x3624, 
	0x3626, 

	0x3640, 
	0x3642, 
	0x3644, 
	0x3646, 
	0x3648, 
	0x364A, 
	0x364C, 
	0x364E, 
	0x3650, 
	0x3652, 
	0x3654, 
	0x3656, 
	0x3658, 
	0x365A, 
	0x365C, 
	0x365E, 
	0x3660, 
	0x3662, 
	0x3664, 
	0x3666, 

	0x3680, 
	0x3682, 
	0x3684, 
	0x3686, 
	0x3688, 
	0x368A, 
	0x368C, 
	0x368E, 
	0x3690, 
	0x3692, 
	0x3694, 
	0x3696, 
	0x3698, 
	0x369A, 
	0x369C, 
	0x369E, 
	0x36A0, 
	0x36A2, 
	0x36A4, 
	0x36A6, 

	0x36C0, 
	0x36C2, 
	0x36C4, 
	0x36C6, 
	0x36C8, 
	0x36CA, 
	0x36CC, 
	0x36CE, 
	0x36D0, 
	0x36D2, 
	0x36D4, 
	0x36D6, 
	0x36D8, 
	0x36DA, 
	0x36DC, 
	0x36DE, 
	0x36E0, 
	0x36E2, 
	0x36E4, 
	0x36E6, 

	0x3700, 
	0x3702, 
	0x3704, 
	0x3706, 
	0x3708, 
	0x370A, 
	0x370C, 
	0x370E, 
	0x3710, 
	0x3712, 
	0x3714, 
	0x3716, 
	0x3718, 
	0x371A, 
	0x371C, 
	0x371E, 
	0x3720, 
	0x3722, 
	0x3724, 
	0x3726,
	
	0x3782, 
	0x3784,
	
	0x37C0, 
	0x37C2, 
	0x37C4, 
	0x37C6, 
};


/*************************************************************************************************
* Function    :  start_read_otp
* Description :  before read otp , set the reading block setting  
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  0, reading block setting err
                 1, reading block setting ok 
**************************************************************************************************/
bool start_read_otp(USHORT zone)
{
	USHORT record_type;
	
	USHORT temp;
	USHORT cnt;
	cnt = 0;

	if(zone > 0x0f)
		return 0;

	record_type = 0x3000 | (zone * 256) ;

//	TRACE("Read:record_type = 0x%04x\n",record_type);

//	A5142MIPI_write_cmos_sensor(0x301A, 0x0630); // RESET_REGISTER_REG_RD_EN
	A5142MIPI_write_cmos_sensor(0x301A, 0x0610); // disable streaming
	
	A5142MIPI_write_cmos_sensor(0x3134, 0xCD95); // timing parameters for OTPM write

	A5142MIPI_write_cmos_sensor(0x304C, record_type); // choose read record type
//	A5142MIPI_write_cmos_sensor(0x304A, 0x0200); // auto read start  0x0010

	A5142MIPI_write_cmos_sensor(0x304A, 0x0210); // auto read start  0x0010

	while (cnt < 50)
	{
		temp = A5142MIPI_read_cmos_sensor(0x304A);
		temp = temp & 0x0060;
		if (temp == 0x0060)
		{
			break;
		}
		cnt++;
		mDELAY(10); 		
	}

	if (cnt == 50)
	{
	//	TRACE("POLL R0x304A[6:5] = 11 failed");	

	//	TRACE("otp read type 0x%02x error", zone|0x30);

		return 0;
	}
	return 1;
}


/*************************************************************************************************
* Function    :  get_otp_date
* Description :  get otp date value 
* Parameters  :  [BYTE] zone : OTP type index , ARO543 zone == 0
* Return      :  [CString] "" : OTP data fail 
                  other value : otp_written_date. example : 2012.10.26               
**************************************************************************************************/
/*
CString get_otp_date(BYTE zone) 
{
	CString info;
	USHORT year, date;
	if(!start_read_otp(zone))
	{
		TRACE("Otp read start Fail!");
		return "";
	}

	year = A5142MIPI_read_cmos_sensor(0x3800);
	date = A5142MIPI_read_cmos_sensor(0x3802);

	BYTE month, day;
	day = (date) & 0x00ff;
	month = ((date)>>8) & 0xff;

    info.Format("%04d.%02d.%02d", year,month,day);
	TRACE("Module date:%04d.%02d.%02d", year,month,day);

	return info;
}
*/


/*************************************************************************************************
* Function    :  get_otp_flag
* Description :  get otp WRITTEN_FLAG  
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [USHORT], if 0x0001 , this type has valid otp data, otherwise, invalid otp data
**************************************************************************************************/
USHORT get_otp_flag(USHORT zone)
{
	USHORT flag;
	if(!start_read_otp(zone))
	{
	//	TRACE("Otp read start Fail!");
		return 0;
	}

	flag = A5142MIPI_read_cmos_sensor(0x3826);

//	TRACE("Flag:0x%04x",flag );

	return flag;
}


/*************************************************************************************************
* Function    :  get_otp_module_id
* Description :  get otp MID value 
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [USHORT] 0 : OTP data fail 
                 other value : module ID data , TRULY ID is 0x0001            
**************************************************************************************************/
USHORT get_otp_module_id(USHORT zone)
{

	USHORT module_id;
	if(!start_read_otp(zone))
	{
		SENSORDB("DBG_DH:Otp read start Fail!\n");
		return 0;
	}

	module_id = A5142MIPI_read_cmos_sensor(0x3806);

	SENSORDB("DBG_DH:Module ID: 0x%04x\n", module_id);

	return module_id;
}


/*************************************************************************************************
* Function    :  get_otp_lens_id
* Description :  get otp LENS_ID value 
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [USHORT] 0 : OTP data fail 
                 other value : LENS ID data             
**************************************************************************************************/
USHORT get_otp_lens_id(USHORT zone)
{
	USHORT lens_id;
	if(!start_read_otp(zone))
	{
//		TRACE("Otp read start Fail!");
		return 0;
	}

	lens_id = A5142MIPI_read_cmos_sensor(0x3808);

//	TRACE("Lens ID: 0x%04x.",lens_id);

	return lens_id;
}


/*************************************************************************************************
* Function    :  get_otp_vcm_id
* Description :  get otp VCM_ID value 
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [USHORT] 0 : OTP data fail 
                 other value : VCM ID data             
**************************************************************************************************/
USHORT get_otp_vcm_id(USHORT zone)
{
	USHORT vcm_id;
	if(!start_read_otp(zone))
	{
	//	TRACE("Otp read start Fail!");
		return 0;
	}

	vcm_id = A5142MIPI_read_cmos_sensor(0x380A);

//	TRACE("VCM ID: 0x%04x.",vcm_id);

	return vcm_id;	
}


/*************************************************************************************************
* Function    :  get_otp_driver_id
* Description :  get otp driver id value 
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [USHORT] 0 : OTP data fail 
                 other value : driver ID data             
**************************************************************************************************/
USHORT get_otp_driver_id(USHORT zone)
{
	USHORT driver_id;
	if(!start_read_otp(zone))
	{
	//	TRACE("Otp read start Fail!"));
		return 0;
	}

	driver_id = A5142MIPI_read_cmos_sensor(0x380C);

//	TRACE("Driver ID: 0x%04x.",driver_id);

	return driver_id;
}

/*************************************************************************************************
* Function    :  get_light_id
* Description :  get otp environment light temperature value  , ARO543 zone == 0
* Parameters  :  [USHORT] zone : OTP type index
* Return      :  [CString] "" : OTP data fail 
                 other value : Light temperature data             
**************************************************************************************************/
/*
CString get_light_id(USHORT zone)
{
	CString info;
	USHORT light_id;
	if(!start_read_otp(zone))
	{
		TRACE("Otp read start Fail!");
		return "";
	}

	light_id = A5142MIPI_read_cmos_sensor(0x380E);

	TRACE("Light ID: 0x%04x.",light_id);

	if(light_id == 0x0000 )
		info.Format("D50(5000K");
	else if(light_id == 0x0001 )
		info.Format("CWF(4000K)");
	else if(light_id == 0x0002 )
		info.Format("A(2800K)");
	return info;
}
*/


/*************************************************************************************************
* Function    :  otp_lenc_update
* Description :  Update lens correction 
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [bool] 0 : OTP data fail 
                        1 : otp_lenc update success            
**************************************************************************************************/
bool otp_lenc_update(USHORT zone)
{
	USHORT IsShading;
	int i;

//	USHORT *otp_lsc;
	USHORT *otp_srt;
	USHORT *otp_dst;
	USHORT otp_lsc[LSC_REGSIZE] = {0};
	otp_srt = otp_reglist;
	otp_dst = lsc_reglist;

	if(!start_read_otp(zone))
	{
		SENSORDB("DBG_DH:LSC update err\n");
		return 0;
	}

	//otp_lsc = new USHORT[LSC_REGSIZE];
	if(otp_lsc == NULL)
	{
	//	TRACE("Create otp_lsc buff err!");
		return 0;
	}

	for(i=0;i<LSC_REGSIZE;i++)
	{
		*(otp_lsc + i) = A5142MIPI_read_cmos_sensor(*(otp_srt + i));

  //   	TRACE("REG = 0x%04x , 0x%04x\n",*(otp_srt + i),*(otp_lsc + i));		
	}

	for(i=0;i<LSC_REGSIZE;i++)
	{
		A5142MIPI_write_cmos_sensor(*(otp_dst + i), *(otp_lsc + i));
	}

	//delete otp_lsc;

	IsShading = A5142MIPI_read_cmos_sensor(0x3780);
	IsShading = IsShading | 0x8000;

	A5142MIPI_write_cmos_sensor(0x3780, IsShading); //enable lsc

	SENSORDB("DBG_DH:LSC update from OTP Okay!\n");

	return 1;
}


/*************************************************************************************************
* Function    :  wb_multi_cal
* Description :  Calculate register gain to gain muliple,100x, the value is 100, actual is 1.0
* Parameters  :  [USHORT]value : Register value
* Return      :  [int]0 : wb_multi_cal fail 
                  other : wb multiple value           
**************************************************************************************************/
int wb_multi_cal(USHORT value)
{
	USHORT temp;

	int multiple;

	int Digital_Gain;
	int Column_Gain;
	int Asc1_Gain;
	int Initial_Gain;

	Digital_Gain = (value >> 12) & 0x000f;
	if(Digital_Gain < 1 || Digital_Gain > 7)
	{
//		TRACE("Digital_Gain Err");
		return 0;
	}

	temp = (value >> 10) & 0x0003;
	if(temp == 0)
    	Column_Gain  = 1;
	else if(temp == 1)
    	Column_Gain  = 3;
	else if(temp == 2)
    	Column_Gain  = 2;
	else if(temp == 3)
    	Column_Gain  = 4;

	temp = ( value >> 8) & 0x0003;
	if(temp == 3)
		Asc1_Gain = 40; // 4.0 multiple 
	else if(temp == 2)
		Asc1_Gain = 20; // 2.0 multiple 
	else if(temp == 1)
		Asc1_Gain = 13; // 1.3 multiple 
	else 
		Asc1_Gain = 10; // 1.0 multiple 

	Initial_Gain = value & 0x007f;

	multiple = (10 * Digital_Gain * Column_Gain * Asc1_Gain * Initial_Gain) >> 5 ;
	
	return multiple;
}


/*************************************************************************************************
* Function    :  wb_gain_cal
* Description :  Calculate gain muliple to register gain
* Parameters  :  [float]multiple : multiple value
* Return      :  [USHORT]other : wb multiple value           
**************************************************************************************************/
USHORT wb_gain_cal(int multiple)
{
	USHORT value;

	if( multiple <= 130)     // Multiple 1.3
	{
		value = (USHORT)(32 * multiple / 100);
		value = value | 0x1000;
	}	    
	else if(multiple <= 200) // Multiple 2.0
	{
		value = (USHORT)(32 * multiple / 130);
		value = 0x1100 | value;
	}
	else if(multiple <= 400) //Multiple 4.0
	{
		value = (USHORT)(32 * multiple / 200);
		value = 0x1200 | value;
	}
	else if(multiple <= 800) //Multiple 8.0
	{
		value = (USHORT)(32 * multiple / 400);
		value = 0x1300 | value;
	}
	else if(multiple <= 1200) //Multiple 12.0
	{
		value = (USHORT)(32 * multiple / 800);
		value = 0x1B00 | value;
	}
	else if(multiple <= 1600) //Multiple 16.0
	{
		value = (USHORT)(32 * multiple / 1200);
		value = 0x1700 | value;
	}
	else if(multiple <= 6400)
	{
		value = (USHORT)(32 * multiple / 1600);
		value = 0x1F00 | value;
	}
	else
	{
		return 0;
	}
	
	return value;
}


/*************************************************************************************************
* Function    :  wb_gain_set
* Description :  Set WB ratio to register gain setting  100x
* Parameters  :  [int] r_ratio : R ratio data compared with golden module R
                       b_ratio : B ratio data compared with golden module B
* Return      :  [bool] 0 : set wb fail 
                        1 : WB set success            
**************************************************************************************************/
bool wb_gain_set(int r_ratio, int b_ratio)
{
	int gain_multiple;

	int R_gain_multiple;
	int B_gain_multiple;

	USHORT R_GAIN;
	USHORT B_GAIN;

	if(!r_ratio || !b_ratio)
	{
		SENSORDB("DBG_DH:OTP WB ratio Data Err\n");
		return 0;
	}

	A5142MIPI_write_cmos_sensor(GAIN_GREEN1_ADDR, GAIN_DEFAULT); //Green 1 default gain 1.7x
	A5142MIPI_write_cmos_sensor(GAIN_GREEN2_ADDR, GAIN_DEFAULT); //Green 2 default gain 1.7x

	gain_multiple = wb_multi_cal(GAIN_DEFAULT);

	if(r_ratio == 100)
		A5142MIPI_write_cmos_sensor(GAIN_RED_ADDR, GAIN_DEFAULT); //RED default gain 1.7x
	else
	{
		R_gain_multiple = gain_multiple * r_ratio / 100;
		R_GAIN = wb_gain_cal(R_gain_multiple);
		A5142MIPI_write_cmos_sensor(GAIN_RED_ADDR, R_GAIN);
	}

	if(b_ratio == 100)
		A5142MIPI_write_cmos_sensor(GAIN_BLUE_ADDR, GAIN_DEFAULT); //BLUE default gain 1.7x
	else
	{
		B_gain_multiple = gain_multiple * b_ratio / 100;
		B_GAIN = wb_gain_cal(B_gain_multiple);
		A5142MIPI_write_cmos_sensor(GAIN_BLUE_ADDR, B_GAIN);
	}
	return 1;
}


/*************************************************************************************************
* Function    :  otp_wb_update
* Description :  Update WB correction 
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [bool] 0 : OTP data fail 
                        1 : otp_WB update success            
**************************************************************************************************/
bool otp_wb_update(USHORT zone)
{
/*
  USHORT golden_r;
	USHORT golden_b;
	USHORT golden_gr;
	USHORT golden_gb;
	USHORT current_r;
	USHORT current_b;
	USHORT current_gr;
	USHORT current_gb;
*/
	USHORT golden_g, current_g;

	int r_ratio;
	int b_ratio;

	if(!start_read_otp(zone))
	{
//		TRACE("Otp read start Fail!");
		return 0;
	}

	golden_r  = A5142MIPI_read_cmos_sensor(0x3816);     //MTK request
	golden_b  = A5142MIPI_read_cmos_sensor(0x3818);     //MTK request
	golden_gr = A5142MIPI_read_cmos_sensor(0x381A);     //MTK request
	golden_gb = A5142MIPI_read_cmos_sensor(0x381C);     //MTK request

	current_r  = A5142MIPI_read_cmos_sensor(0x381E); //MTK request
	current_b  = A5142MIPI_read_cmos_sensor(0x3820); //MTK request
	current_gr = A5142MIPI_read_cmos_sensor(0x3822); //MTK request 
	current_gb = A5142MIPI_read_cmos_sensor(0x3824); //MTK request

	golden_g = (golden_gr + golden_gb) / 2;
	current_g = (current_gr + current_gb) / 2;

	if(!golden_g || !current_g || !golden_r || !golden_b || !current_r || !current_b)
	{
		SENSORDB("DBG_DH:wb update err\n");
		return 0;
	}

	r_ratio = 100 * golden_r * current_g /( golden_g * current_r );
	b_ratio = 100 * golden_b * current_g /( golden_g * current_b );

	wb_gain_set(r_ratio, b_ratio);

	SENSORDB("DBG_DH:wb update Okay\n");	

	return 1;
}


/*************************************************************************************************
* Function    :  get_otp_wb
* Description :  Get WB data  
* Parameters  :  [USHORT] zone : OTP type index , ARO543 zone == 0
* Return      :  [bool] 0 : OTP data fail 
                        1 : otp_WB update success            
**************************************************************************************************/
bool get_otp_wb(USHORT zone)
{
	if(!start_read_otp(zone))
	{
//		TRACE("Otp read start Fail!");
		return 0;
	}

	golden_r  = A5142MIPI_read_cmos_sensor(0x3816);     //MTK request
	golden_b  = A5142MIPI_read_cmos_sensor(0x3818);     //MTK request
	golden_gr = A5142MIPI_read_cmos_sensor(0x381A);     //MTK request
	golden_gb = A5142MIPI_read_cmos_sensor(0x381C);     //MTK request

	current_r  = A5142MIPI_read_cmos_sensor(0x381E); //MTK request
	current_b  = A5142MIPI_read_cmos_sensor(0x3820); //MTK request
	current_gr = A5142MIPI_read_cmos_sensor(0x3822); //MTK request 
	current_gb = A5142MIPI_read_cmos_sensor(0x3824); //MTK request

//	TRACE("get wb data ok");

	return 1;
}
