/*******************************************************************************************/
      
  
/*******************************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>    
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <linux/xlog.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "s5k4h5yx2lanemipiraw_Sensor.h"
#include "s5k4h5yx2lanemipiraw_Camera_Sensor_para.h"
#include "s5k4h5yx2lanemipiraw_CameraCustomized.h"

static DEFINE_SPINLOCK(s5k4h5yx2lanemipiraw_drv_lock);

#define S5K4H5YX_2LANE_DEBUG


#ifdef S5K4H5YX_2LANE_DEBUG
	#define S5K4H5YX_2LANEDB(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[S5K4H5YX_2LANEMIPI]" , fmt, ##arg)
#else
	#define S5K4H5YX_2LANEDB(x,...)
#endif

#define mDELAY(ms)  mdelay(ms)

kal_uint32 S5K4H5YX_2LANE_FeatureControl_PERIOD_PixelNum;
kal_uint32 S5K4H5YX_2LANE_FeatureControl_PERIOD_LineNum;

kal_uint32 cont_preview_line_length = 3688;
kal_uint32 cont_preview_frame_length = 1266;
kal_uint32 cont_capture_line_length = 3688;
kal_uint32 cont_capture_frame_length = 2530;
kal_uint32 cont_video_frame_length = 1264;

MSDK_SENSOR_CONFIG_STRUCT s5k4h5yx2laneSensorConfigData;

kal_uint32 S5K4H5YX_2LANE_FAC_SENSOR_REG;

MSDK_SCENARIO_ID_ENUM S5K4H5YX_2LANECurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

SENSOR_REG_STRUCT S5K4H5YX_2LANESensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT S5K4H5YX_2LANESensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;

static S5K4H5YX_2LANE_PARA_STRUCT S5K4H5YX_2LANE;

extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);

#define S5K4H5YX_2LANE_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, S5K4H5YX_2LANEMIPI_WRITE_ID)

kal_uint16 S5K4H5YX_2LANE_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,S5K4H5YX_2LANEMIPI_WRITE_ID);
    return get_byte;
}

#define Sleep(ms) mdelay(ms)


//#define S5K4H5YX_2LANE_USE_AWB_OTP

#if defined(S5K4H5YX_2LANE_USE_AWB_OTP)

#define RG_TYPICAL 0x2a1
#define BG_TYPICAL 0x23f


kal_uint32 tRG_Ratio_typical = RG_TYPICAL;
kal_uint32 tBG_Ratio_typical = BG_TYPICAL;


void S5K4H5YX_2LANE_MIPI_read_otp_wb(struct S5K4H5YX_2LANE_MIPI_otp_struct *otp)
{
	kal_uint32 R_to_G, B_to_G, G_to_G;
	kal_uint16 PageCount;
	for(PageCount = 4; PageCount>=1; PageCount--)
	{
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] PageCount=%d\n", PageCount);	
		S5K4H5YX_2LANE_write_cmos_sensor(0x3a02, PageCount); //page set
		S5K4H5YX_2LANE_write_cmos_sensor(0x3a00, 0x01); //otp enable read
		R_to_G = (S5K4H5YX_2LANE_read_cmos_sensor(0x3a09)<<8)+S5K4H5YX_2LANE_read_cmos_sensor(0x3a0a);
		B_to_G = (S5K4H5YX_2LANE_read_cmos_sensor(0x3a0b)<<8)+S5K4H5YX_2LANE_read_cmos_sensor(0x3a0c);
		G_to_G = (S5K4H5YX_2LANE_read_cmos_sensor(0x3a0d)<<8)+S5K4H5YX_2LANE_read_cmos_sensor(0x3a0e);
		S5K4H5YX_2LANE_write_cmos_sensor(0x3a00, 0x00); //otp disable read

		if((R_to_G != 0) && (B_to_G != 0) && (G_to_G != 0))
			break;	

		if(PageCount == 1)
			S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] otp all value is zero");
	}

	otp->R_to_G = R_to_G;
	otp->B_to_G = B_to_G;
	otp->G_to_G = 0x400;
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] otp->R_to_G=0x%x\n", otp->R_to_G);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] otp->B_to_G=0x%x\n", otp->B_to_G);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] otp->G_to_G=0x%x\n", otp->G_to_G);	
}

void S5K4H5YX_2LANE_MIPI_algorithm_otp_wb1(struct S5K4H5YX_2LANE_MIPI_otp_struct *otp)
{
	kal_uint32 R_to_G, B_to_G, G_to_G;
	kal_uint32 R_Gain, B_Gain, G_Gain;
	kal_uint32 G_gain_R, G_gain_B;
	
	R_to_G = otp->R_to_G;
	B_to_G = otp->B_to_G;
	G_to_G = otp->G_to_G;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] R_to_G=%d\n", R_to_G);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] B_to_G=%d\n", B_to_G);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] G_to_G=%d\n", G_to_G);

	if(B_to_G < tBG_Ratio_typical)
		{
			if(R_to_G < tRG_Ratio_typical)
				{
					G_Gain = 0x100;
					B_Gain = 0x100 * tBG_Ratio_typical / B_to_G;
					R_Gain = 0x100 * tRG_Ratio_typical / R_to_G;
				}
			else
				{
			        R_Gain = 0x100;
					G_Gain = 0x100 * R_to_G / tRG_Ratio_typical;
					B_Gain = G_Gain * tBG_Ratio_typical / B_to_G;	        
				}
		}
	else
		{
			if(R_to_G < tRG_Ratio_typical)
				{
			        B_Gain = 0x100;
					G_Gain = 0x100 * B_to_G / tBG_Ratio_typical;
					R_Gain = G_Gain * tRG_Ratio_typical / R_to_G;
				}
			else
				{
			        G_gain_B = 0x100*B_to_G / tBG_Ratio_typical;
				    G_gain_R = 0x100*R_to_G / tRG_Ratio_typical;
					
					if(G_gain_B > G_gain_R)
						{
							B_Gain = 0x100;
							G_Gain = G_gain_B;
							R_Gain = G_Gain * tRG_Ratio_typical / R_to_G;
						}
					else
						{
							R_Gain = 0x100;
							G_Gain = G_gain_R;
							B_Gain = G_Gain * tBG_Ratio_typical / B_to_G;
						}        
				}	
		}

	otp->R_Gain = R_Gain;
	otp->B_Gain = B_Gain;
	otp->G_Gain = G_Gain;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] R_gain=0x%x\n", otp->R_Gain);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] B_gain=0x%x\n", otp->B_Gain);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] G_gain=0x%x\n", otp->G_Gain);
}



void S5K4H5YX_2LANE_MIPI_write_otp_wb(struct S5K4H5YX_2LANE_MIPI_otp_struct *otp)
{
	kal_uint16 R_GainH, B_GainH, G_GainH;
	kal_uint16 R_GainL, B_GainL, G_GainL;
	kal_uint32 temp;

	temp = otp->R_Gain;
	R_GainH = (temp & 0xff00)>>8;
	temp = otp->R_Gain;
	R_GainL = (temp & 0x00ff);

	temp = otp->B_Gain;
	B_GainH = (temp & 0xff00)>>8;
	temp = otp->B_Gain;
	B_GainL = (temp & 0x00ff);

	temp = otp->G_Gain;
	G_GainH = (temp & 0xff00)>>8;
	temp = otp->G_Gain;
	G_GainL = (temp & 0x00ff);

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] R_GainH=0x%x\n", R_GainH);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] R_GainL=0x%x\n", R_GainL);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] B_GainH=0x%x\n", B_GainH);
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] B_GainL=0x%x\n", B_GainL);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] G_GainH=0x%x\n", G_GainH);	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] G_GainL=0x%x\n", G_GainL);

	S5K4H5YX_2LANE_write_cmos_sensor(0x020e, G_GainH);
	S5K4H5YX_2LANE_write_cmos_sensor(0x020f, G_GainL);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0210, R_GainH);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0211, R_GainL);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0212, B_GainH);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0213, B_GainL);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0214, G_GainH);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0215, G_GainL);

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x020e,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x020e));	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x020f,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x020f));	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x0210,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x0210));
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x0211,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x0211));	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x0212,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x0212));	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x0213,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x0213));
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x0214,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x0214));	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_MIPI_read_otp_wb] [0x0215,0x%x]\n", S5K4H5YX_2LANE_read_cmos_sensor(0x0215));
}



void S5K4H5YX_2LANE_MIPI_update_wb_register_from_otp(void)
{
	struct S5K4H5YX_2LANE_MIPI_otp_struct current_otp;
	S5K4H5YX_2LANE_MIPI_read_otp_wb(&current_otp);
	S5K4H5YX_2LANE_MIPI_algorithm_otp_wb1(&current_otp);
	S5K4H5YX_2LANE_MIPI_write_otp_wb(&current_otp);
}
#endif



void S5K4H5YX_2LANE_write_shutter(kal_uint32 shutter)
{
	kal_uint32 frame_length = 0;
	kal_uint32 line_length_read;
	kal_uint32 framerate = 0;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_write_shutter] shutter=%d\n", shutter);

  if (shutter < 3)
	  shutter = 3;

  if (S5K4H5YX_2LANE.sensorMode == SENSOR_MODE_PREVIEW) 
  {
	  if(shutter > (cont_preview_frame_length - 16))
		  frame_length = shutter + 16;
	  else 
		  frame_length = cont_preview_frame_length;

	  if(S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode == KAL_TRUE)
	  {
	  	  framerate = (10 * S5K4H5YX_2LANE.pvPclk) / (frame_length * S5K4H5YX_2LANE_preview_line_length);
		  
		  if(framerate == 300)
		  	framerate = 296;
		  else if(framerate == 150)
		  	framerate = 148;

		  frame_length = (10 * S5K4H5YX_2LANE.pvPclk) / (framerate * S5K4H5YX_2LANE_preview_line_length);
	  }
	  S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_write_shutter] SENSOR_MODE_PREVIEW , shutter = %d, frame_length = %d, framerate = %d\n",shutter, frame_length, framerate);
  }
  else if(S5K4H5YX_2LANE.sensorMode==SENSOR_MODE_VIDEO)
  {
	   	if(cont_video_frame_length <= (shutter + 16))
			frame_length = shutter + 16;
		else
			frame_length = cont_video_frame_length;

	    if(S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode == KAL_TRUE)
	  {
	  	  framerate = (10 * S5K4H5YX_2LANE.videoPclk) / (frame_length * S5K4H5YX_2LANE_video_line_length);
		  
		  if(framerate == 300)
		  	framerate = 296;
		  else if(framerate == 150)
		  	framerate = 148;

		  frame_length = (10 * S5K4H5YX_2LANE.videoPclk) / (framerate * S5K4H5YX_2LANE_video_line_length);
	  }
	   S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_write_shutter] SENSOR_MODE_VIDEO , shutter = %d, frame_length = %d, framerate = %d\n",shutter, frame_length, framerate);
  }
  else
  {
	  if(shutter > (cont_capture_frame_length - 16))
		  frame_length = shutter + 16;
	  else 
		  frame_length = cont_capture_frame_length;

	  if(S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode == KAL_TRUE)
	  {
	  	  framerate = (10 * S5K4H5YX_2LANE.capPclk) / (frame_length * S5K4H5YX_2LANE_capture_line_length);
		  
		  if(framerate == 300)
		  	framerate = 296;
		  else if(framerate == 150)
		  	framerate = 148;

		  frame_length = (10 * S5K4H5YX_2LANE.capPclk) / (framerate * S5K4H5YX_2LANE_capture_line_length);
	  }
	  S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_write_shutter] SENSOR_MODE_CAPTURE , shutter = %d, frame_length = %d\n",shutter, frame_length);
  }
  
 	S5K4H5YX_2LANE_write_cmos_sensor(0x0104, 0x01);   

	S5K4H5YX_2LANE_write_cmos_sensor(0x0340, (frame_length >>8) & 0xFF);
 	S5K4H5YX_2LANE_write_cmos_sensor(0x0341, frame_length & 0xFF);	 

 	S5K4H5YX_2LANE_write_cmos_sensor(0x0202, (shutter >> 8) & 0xFF);
 	S5K4H5YX_2LANE_write_cmos_sensor(0x0203, shutter  & 0xFF);
 
 	S5K4H5YX_2LANE_write_cmos_sensor(0x0104, 0x00);   

	line_length_read = ((S5K4H5YX_2LANE_read_cmos_sensor(0x0342)<<8)+S5K4H5YX_2LANE_read_cmos_sensor(0x0343));

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_write_shutter][read_check] shutter=%d,  line_length_read=%d, frame_length=%d\n", shutter, line_length_read, frame_length);
}   


void write_S5K4H5YX_2LANE_gain(kal_uint16 gain)
{
	gain = gain / 2;
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [write_S5K4H5YX_2LANE_gain] gain=%d\n", gain);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0104, 0x01);	
	S5K4H5YX_2LANE_write_cmos_sensor(0x0204,(gain>>8));
	S5K4H5YX_2LANE_write_cmos_sensor(0x0205,(gain&0xff));
	S5K4H5YX_2LANE_write_cmos_sensor(0x0104, 0x00);
	return;
}



void S5K4H5YX_2LANE_SetGain(UINT16 iGain)
{
	unsigned long flags;
	spin_lock_irqsave(&s5k4h5yx2lanemipiraw_drv_lock,flags);
	S5K4H5YX_2LANE.realGain = iGain;
	spin_unlock_irqrestore(&s5k4h5yx2lanemipiraw_drv_lock,flags);
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetGain] gain=%d\n", iGain);
	write_S5K4H5YX_2LANE_gain(iGain);
} 



kal_uint16 read_S5K4H5YX_2LANE_gain(void)
{
	kal_uint16 read_gain=0;
	read_gain=((S5K4H5YX_2LANE_read_cmos_sensor(0x0204) << 8) | S5K4H5YX_2LANE_read_cmos_sensor(0x0205));
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [read_S5K4H5YX_2LANE_gain] gain=%d\n", read_gain);
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.sensorGlobalGain = read_gain;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	return S5K4H5YX_2LANE.sensorGlobalGain;
} 



void S5K4H5YX_2LANE_camera_para_to_sensor(void)
{
    kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=S5K4H5YX_2LANESensorReg[i].Addr; i++)
    {
        S5K4H5YX_2LANE_write_cmos_sensor(S5K4H5YX_2LANESensorReg[i].Addr, S5K4H5YX_2LANESensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=S5K4H5YX_2LANESensorReg[i].Addr; i++)
    {
        S5K4H5YX_2LANE_write_cmos_sensor(S5K4H5YX_2LANESensorReg[i].Addr, S5K4H5YX_2LANESensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        S5K4H5YX_2LANE_write_cmos_sensor(S5K4H5YX_2LANESensorCCT[i].Addr, S5K4H5YX_2LANESensorCCT[i].Para);
    }
}



void S5K4H5YX_2LANE_sensor_to_camera_para(void)
{
    kal_uint32    i, temp_data;
    for(i=0; 0xFFFFFFFF!=S5K4H5YX_2LANESensorReg[i].Addr; i++)
    {
         temp_data = S5K4H5YX_2LANE_read_cmos_sensor(S5K4H5YX_2LANESensorReg[i].Addr);
		 spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
		 S5K4H5YX_2LANESensorReg[i].Para =temp_data;
		 spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=S5K4H5YX_2LANESensorReg[i].Addr; i++)
    {
        temp_data = S5K4H5YX_2LANE_read_cmos_sensor(S5K4H5YX_2LANESensorReg[i].Addr);
		spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
		S5K4H5YX_2LANESensorReg[i].Para = temp_data;
		spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
    }
}



kal_int32  S5K4H5YX_2LANE_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}



void S5K4H5YX_2LANE_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
   switch (group_idx)
   {
        case PRE_GAIN:
            sprintf((char *)group_name_ptr, "CCT");
            *item_count_ptr = 2;
            break;
        case CMMCLK_CURRENT:
            sprintf((char *)group_name_ptr, "CMMCLK Current");
            *item_count_ptr = 1;
            break;
        case FRAME_RATE_LIMITATION:
            sprintf((char *)group_name_ptr, "Frame Rate Limitation");
            *item_count_ptr = 2;
            break;
        case REGISTER_EDITOR:
            sprintf((char *)group_name_ptr, "Register Editor");
            *item_count_ptr = 2;
            break;
        default:
            ASSERT(0);
	}
}



void S5K4H5YX_2LANE_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{
    kal_int16 temp_reg=0;
    kal_uint16 temp_gain=0, temp_addr=0, temp_para=0;

    switch (group_idx)
    {
        case PRE_GAIN:
           switch (item_idx)
          {
              case 0:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-R");
                  temp_addr = PRE_GAIN_R_INDEX;
              break;
              case 1:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Gr");
                  temp_addr = PRE_GAIN_Gr_INDEX;
              break;
              case 2:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Gb");
                  temp_addr = PRE_GAIN_Gb_INDEX;
              break;
              case 3:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-B");
                  temp_addr = PRE_GAIN_B_INDEX;
              break;
              case 4:
                 sprintf((char *)info_ptr->ItemNamePtr,"SENSOR_BASEGAIN");
                 temp_addr = SENSOR_BASEGAIN;
              break;
              default:
                 ASSERT(0);
          }

            temp_para= S5K4H5YX_2LANESensorCCT[temp_addr].Para;

            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min= S5K4H5YX_2LANE_MIN_ANALOG_GAIN * 1000;
            info_ptr->Max= S5K4H5YX_2LANE_MAX_ANALOG_GAIN * 1000;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                    temp_reg = ISP_DRIVING_2MA;
                    if(temp_reg==ISP_DRIVING_2MA)
                    {
                        info_ptr->ItemValue=2;
                    }
                    else if(temp_reg==ISP_DRIVING_4MA)
                    {
                        info_ptr->ItemValue=4;
                    }
                    else if(temp_reg==ISP_DRIVING_6MA)
                    {
                        info_ptr->ItemValue=6;
                    }
                    else if(temp_reg==ISP_DRIVING_8MA)
                    {
                        info_ptr->ItemValue=8;
                    }

                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_TRUE;
                    info_ptr->Min=2;
                    info_ptr->Max=8;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Max Exposure Lines");
                    info_ptr->ItemValue=    111;  
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"Min Frame Rate");
                    info_ptr->ItemValue=12;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Addr.");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Value");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                default:
                ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
}



kal_bool S5K4H5YX_2LANE_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
   kal_uint16  temp_gain=0,temp_addr=0, temp_para=0;

   switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
              case 0:
                temp_addr = PRE_GAIN_R_INDEX;
              break;
              case 1:
                temp_addr = PRE_GAIN_Gr_INDEX;
              break;
              case 2:
                temp_addr = PRE_GAIN_Gb_INDEX;
              break;
              case 3:
                temp_addr = PRE_GAIN_B_INDEX;
              break;
              case 4:
                temp_addr = SENSOR_BASEGAIN;
              break;
              default:
                 ASSERT(0);
          }

		 temp_gain=((ItemValue*BASEGAIN+500)/1000);	
		 
		  spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
          S5K4H5YX_2LANESensorCCT[temp_addr].Para = temp_para;
		  spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
          S5K4H5YX_2LANE_write_cmos_sensor(S5K4H5YX_2LANESensorCCT[temp_addr].Addr,temp_para);
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            ASSERT(0);
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
					spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
                    S5K4H5YX_2LANE_FAC_SENSOR_REG=ItemValue;
					spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
                    break;
                case 1:
                    S5K4H5YX_2LANE_write_cmos_sensor(S5K4H5YX_2LANE_FAC_SENSOR_REG,ItemValue);
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



static void S5K4H5YX_2LANE_SetDummy( const kal_uint32 iPixels, const kal_uint32 iLines )
{
	kal_uint16 line_length = 0;
	kal_uint16 frame_length = 0;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetDummy] iPixels=%d, iLines=%d\n", iPixels, iLines);

	if ( SENSOR_MODE_PREVIEW == S5K4H5YX_2LANE.sensorMode )	
	{
		line_length = S5K4H5YX_2LANE_preview_line_length ;
		frame_length = S5K4H5YX_2LANE_preview_frame_length + iLines;
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetDummy] SENSOR_MODE_PREVIEW, line_length = %d, frame_length = %d\n", line_length, frame_length);
	}
	else if( SENSOR_MODE_VIDEO == S5K4H5YX_2LANE.sensorMode )		
	{
		line_length = S5K4H5YX_2LANE_video_line_length;
		frame_length = S5K4H5YX_2LANE_video_frame_length + iLines; 
		cont_video_frame_length = frame_length;
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetDummy] SENSOR_MODE_VIDEO, line_length = %d, frame_length = %d\n", line_length, frame_length);
	}
	else
	{
		line_length = S5K4H5YX_2LANE_capture_line_length ;
		frame_length = S5K4H5YX_2LANE_capture_frame_length + iLines;
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetDummy] SENSOR_MODE_CAPTURE, line_length = %d, frame_length = %d\n", line_length, frame_length);
	}

	if(S5K4H5YX_2LANE.maxExposureLines > frame_length - 16)
	{
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetDummy] maxExposureLines > frame_length - 16\n");
		return;
	}

	S5K4H5YX_2LANE_write_cmos_sensor(0x0104, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0340, (frame_length >> 8) & 0xFF);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0341, frame_length & 0xFF);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0342, (line_length >> 8) & 0xFF);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0343, line_length & 0xFF);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0104, 0x00);	
}   



void S5K4H5YX_2LANEPreviewSetting(void)
{
	S5K4H5YX_2LANE_write_cmos_sensor(0x0100, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0200, 0x0C);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0201, 0x78);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0342, 0x0E);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0343, 0x68);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0344, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0345, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0346, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0347, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0348, 0x0C);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0349, 0xCF);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034A, 0x09);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034B, 0x9F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034C, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034D, 0x68);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034E, 0x04);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034F, 0xD0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0390, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0391, 0x22);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0381, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0383, 0x03);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0385, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0387, 0x03);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0114, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0301, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0303, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0305, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0306, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0307, 0x46);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0309, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030B, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C59, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030D, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030E, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030F, 0xA2);//20130426 a8->a2
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C5A, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0310, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C50, 0x53);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C62, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C63, 0xA0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C64, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C65, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C1E, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x302A, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x303D, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x304B, 0x2A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3205, 0x84);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3207, 0x85);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3214, 0x94);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3216, 0x95);
	S5K4H5YX_2LANE_write_cmos_sensor(0x303a, 0x9f);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3201, 0x07);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3051, 0xff);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3052, 0xff);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3054, 0xF0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x305C, 0x8F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x302D, 0x7F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3B29, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3903, 0x1F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3002, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300A, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3037, 0x12);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3045, 0x04);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300c, 0x78);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300d, 0x80);
	S5K4H5YX_2LANE_write_cmos_sensor(0x305c, 0x82);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3010, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0100, 0x01);
}

	
void S5K4H5YX_2LANEVideoSetting(void)
{
	S5K4H5YX_2LANE_write_cmos_sensor(0x0100, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0200, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0201, 0xE8);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0342, 0x0E);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0343, 0x68);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0344, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0345, 0x18);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0346, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0347, 0x62);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0348, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0349, 0xB7);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034A, 0x07);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034B, 0x3B);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034C, 0x08);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034D, 0xA0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034E, 0x04);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034F, 0xDA);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0390, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0391, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0381, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0383, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0385, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0387, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0114, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0301, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0303, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0305, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0306, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0307, 0x46);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0309, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030B, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C59, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030D, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030E, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030F, 0xA2);//20130426 a8->a2
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C5A, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0310, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C50, 0x53);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C62, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C63, 0xA0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C64, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C65, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C1E, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x302A, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x303D, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x304B, 0x2A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3205, 0x84);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3207, 0x85);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3214, 0x94);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3216, 0x95);
	S5K4H5YX_2LANE_write_cmos_sensor(0x303a, 0x9f);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3201, 0x07);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3051, 0xff);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3052, 0xff);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3054, 0xF0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x305C, 0x8F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x302D, 0x7F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3B29, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3002, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300A, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3037, 0x12);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3045, 0x04);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300c, 0x78);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300d, 0x80);
	S5K4H5YX_2LANE_write_cmos_sensor(0x305c, 0x82);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3010, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0100, 0x01);
}



void S5K4H5YX_2LANECaptureSetting(void)
{
	S5K4H5YX_2LANE_write_cmos_sensor(0x0100, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0200, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0201, 0xE8);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0342, 0x0E);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0343, 0x68);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0344, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0345, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0346, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0347, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0348, 0x0C);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0349, 0xCF);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034A, 0x09);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034B, 0x9F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034C, 0x0C);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034D, 0xD0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034E, 0x09);
	S5K4H5YX_2LANE_write_cmos_sensor(0x034F, 0xA0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0390, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0391, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0381, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0383, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0385, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0387, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0114, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0301, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0303, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0305, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0306, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0307, 0x41);//mipi clock 
	S5K4H5YX_2LANE_write_cmos_sensor(0x0309, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030B, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C59, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030D, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030E, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x030F, 0xA2);//modify data rate 648Mhz
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C5A, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0310, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C50, 0x53);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C62, 0x02);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C63, 0xA0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C64, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C65, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3C1E, 0x00);
	S5K4H5YX_2LANE_write_cmos_sensor(0x302A, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x303D, 0x06);
	S5K4H5YX_2LANE_write_cmos_sensor(0x304B, 0x2A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3205, 0x84);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3207, 0x85);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3214, 0x94);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3216, 0x95);
	S5K4H5YX_2LANE_write_cmos_sensor(0x303a, 0x9f);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3201, 0x07);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3051, 0xff);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3052, 0xff);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3054, 0xF0);
	S5K4H5YX_2LANE_write_cmos_sensor(0x305C, 0x8F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x302D, 0x7F);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3B29, 0x01);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3002, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300A, 0x0D);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3037, 0x12);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3045, 0x04);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300c, 0x78);
	S5K4H5YX_2LANE_write_cmos_sensor(0x300d, 0x80);
	S5K4H5YX_2LANE_write_cmos_sensor(0x305c, 0x82);
	S5K4H5YX_2LANE_write_cmos_sensor(0x3010, 0x0A);
	S5K4H5YX_2LANE_write_cmos_sensor(0x0100, 0x01);
}



UINT32 S5K4H5YX_2LANEOpen(void)
{
	volatile signed int i;
	kal_uint16 sensor_id = 0;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEOpen]\n");

	for(i=0;i<3;i++)
	{
		sensor_id = (S5K4H5YX_2LANE_read_cmos_sensor(0x0000)<<8)|S5K4H5YX_2LANE_read_cmos_sensor(0x0001);
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEOpen] sensor_id=%x\n",sensor_id);
		if(sensor_id != S5K4H5YX_2LANE_SENSOR_ID)
		{
			return ERROR_SENSOR_CONNECT_FAIL;
		}else
			break;
	}
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_INIT;
	S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode = KAL_FALSE;
	S5K4H5YX_2LANE.S5K4H5YX_2LANEVideoMode = KAL_FALSE;
	S5K4H5YX_2LANE.DummyLines= 0;
	S5K4H5YX_2LANE.DummyPixels= 0;

	S5K4H5YX_2LANE.pvPclk =  S5K4H5YX_2LANE_preview_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;  
	S5K4H5YX_2LANE.videoPclk = S5K4H5YX_2LANE_video_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz; 
	
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);

    	switch(S5K4H5YX_2LANECurrentScenarioId)
		{
			case MSDK_SCENARIO_ID_CAMERA_ZSD:
				spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
				S5K4H5YX_2LANE.capPclk = S5K4H5YX_2LANE_capture_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
			
				spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
				break;
        	default:
				spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
				S5K4H5YX_2LANE.capPclk = S5K4H5YX_2LANE_capture_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
				spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
				break;
          }
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.shutter = 0x4EA;
	S5K4H5YX_2LANE.pvShutter = 0x4EA;
	S5K4H5YX_2LANE.maxExposureLines =S5K4H5YX_2LANE_preview_frame_length - 16;
	S5K4H5YX_2LANE.ispBaseGain = BASEGAIN;
	S5K4H5YX_2LANE.sensorGlobalGain = 0x1f;
	S5K4H5YX_2LANE.pvGain = 0x1f;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	
    return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANEGetSensorID(UINT32 *sensorID)
{
    int  retry = 1;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEGetSensorID]\n");

    do {
        *sensorID = (S5K4H5YX_2LANE_read_cmos_sensor(0x0000)<<8)|S5K4H5YX_2LANE_read_cmos_sensor(0x0001);
        if (*sensorID == S5K4H5YX_2LANE_SENSOR_ID)
        	{
        		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEGetSensorID] Sensor ID = 0x%04x\n", *sensorID);
            	break;
        	}
        S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEGetSensorID] Read Sensor ID Fail = 0x%04x\n", *sensorID);
        retry--;
    } while (retry > 0);

    if (*sensorID != S5K4H5YX_2LANE_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}



void S5K4H5YX_2LANE_SetShutter(kal_uint32 iShutter)
{
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_SetShutter] shutter = %d\n", iShutter);
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.shutter= iShutter;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE_write_shutter(iShutter);
}  



UINT32 S5K4H5YX_2LANE_read_shutter(void)
{
	UINT32 shutter =0;
	shutter = (S5K4H5YX_2LANE_read_cmos_sensor(0x0202) << 8) | S5K4H5YX_2LANE_read_cmos_sensor(0x0203);
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANE_read_shutter] shutter = %d\n", shutter);
	return shutter;
}



UINT32 S5K4H5YX_2LANEClose(void)
{
    return ERROR_NONE;
}	



void S5K4H5YX_2LANESetFlipMirror(kal_int32 imgMirror)
{
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetFlipMirror] imgMirror = %d\n", imgMirror);
	    switch (imgMirror)
    {
        case IMAGE_NORMAL: 
            S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x03);	
            break;
        case IMAGE_V_MIRROR: 
            S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x01);	
            break;
        case IMAGE_H_MIRROR: 
            S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x02);	
            break;
        case IMAGE_HV_MIRROR: 
            S5K4H5YX_2LANE_write_cmos_sensor(0x0101, 0x00);	
            break;
    }
}



UINT32 S5K4H5YX_2LANEPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEPreview]\n");

	if(S5K4H5YX_2LANE.sensorMode == SENSOR_MODE_PREVIEW)
	{
	}
	else
	{
		S5K4H5YX_2LANEPreviewSetting();
	}
	
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_PREVIEW;
	S5K4H5YX_2LANE.DummyPixels = 0;
	S5K4H5YX_2LANE.DummyLines = 0 ;
	cont_preview_line_length=S5K4H5YX_2LANE_preview_line_length;
	cont_preview_frame_length=S5K4H5YX_2LANE_preview_frame_length+S5K4H5YX_2LANE.DummyLines;
	S5K4H5YX_2LANE_FeatureControl_PERIOD_PixelNum = S5K4H5YX_2LANE_preview_line_length;
	S5K4H5YX_2LANE_FeatureControl_PERIOD_LineNum = cont_preview_frame_length;
	S5K4H5YX_2LANE.imgMirror = sensor_config_data->SensorImageMirror;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	
	S5K4H5YX_2LANESetFlipMirror(IMAGE_HV_MIRROR);
    return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANEVideo(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEVideo]\n");

	if(S5K4H5YX_2LANE.sensorMode == SENSOR_MODE_VIDEO)
	{
		// do nothing
	}
	else
	{
		S5K4H5YX_2LANEVideoSetting();
	}
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_VIDEO;
	S5K4H5YX_2LANE_FeatureControl_PERIOD_PixelNum = S5K4H5YX_2LANE_video_line_length;
	S5K4H5YX_2LANE_FeatureControl_PERIOD_LineNum = S5K4H5YX_2LANE_video_frame_length;
	cont_video_frame_length = S5K4H5YX_2LANE_video_frame_length;
	S5K4H5YX_2LANE.imgMirror = sensor_config_data->SensorImageMirror;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	
	S5K4H5YX_2LANESetFlipMirror(IMAGE_HV_MIRROR);
    return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANECapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
 	kal_uint32 shutter = S5K4H5YX_2LANE.shutter;
	kal_uint32 temp_data;

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANECapture]\n");

	if( SENSOR_MODE_CAPTURE== S5K4H5YX_2LANE.sensorMode)
	{
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANECapture] BusrtShot!!\n");
	}
	else{

	shutter=S5K4H5YX_2LANE_read_shutter();
	temp_data =  read_S5K4H5YX_2LANE_gain();
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.pvShutter =shutter;
	S5K4H5YX_2LANE.sensorGlobalGain = temp_data;
	S5K4H5YX_2LANE.pvGain =S5K4H5YX_2LANE.sensorGlobalGain;
	S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_CAPTURE;	
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);

	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANECapture] S5K4H5YX_2LANE.shutter=%d, read_pv_shutter=%d, read_pv_gain = 0x%x\n",S5K4H5YX_2LANE.shutter, shutter,S5K4H5YX_2LANE.sensorGlobalGain);
	S5K4H5YX_2LANECaptureSetting();
	S5K4H5YX_2LANE_SetDummy(S5K4H5YX_2LANE.DummyPixels,S5K4H5YX_2LANE.DummyLines);

	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.imgMirror = sensor_config_data->SensorImageMirror;
	S5K4H5YX_2LANE.DummyPixels = 0;
	S5K4H5YX_2LANE.DummyLines = 0 ;
	cont_capture_line_length = S5K4H5YX_2LANE_capture_line_length;
	cont_capture_frame_length = S5K4H5YX_2LANE_capture_frame_length + S5K4H5YX_2LANE.DummyLines;
	S5K4H5YX_2LANE_FeatureControl_PERIOD_PixelNum = S5K4H5YX_2LANE_capture_line_length;
	S5K4H5YX_2LANE_FeatureControl_PERIOD_LineNum = cont_capture_frame_length;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	
	S5K4H5YX_2LANESetFlipMirror(IMAGE_HV_MIRROR);

    if(S5K4H5YX_2LANECurrentScenarioId==MSDK_SCENARIO_ID_CAMERA_ZSD)
    {
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANECapture] S5K4H5YX_2LANECapture exit ZSD!\n");
		return ERROR_NONE;
    }
	}

    return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANEGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	//1640*1232
	//1632*1226
	pSensorResolution->SensorPreviewWidth	= S5K4H5YX_2LANE_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight	= S5K4H5YX_2LANE_IMAGE_SENSOR_PV_HEIGHT;

	//3280*2464
	//3264*2448
    pSensorResolution->SensorFullWidth		= S5K4H5YX_2LANE_IMAGE_SENSOR_FULL_WIDTH ;
    pSensorResolution->SensorFullHeight		= S5K4H5YX_2LANE_IMAGE_SENSOR_FULL_HEIGHT ;
		
	//2208*1242
	//1920*1080
    pSensorResolution->SensorVideoWidth		= S5K4H5YX_2LANE_IMAGE_SENSOR_VIDEO_WIDTH ;
    pSensorResolution->SensorVideoHeight    = S5K4H5YX_2LANE_IMAGE_SENSOR_VIDEO_HEIGHT ;

    return ERROR_NONE;
}   /* S5K4H5YX_2LANEGetResolution() */



UINT32 S5K4H5YX_2LANEGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	 S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEGetInfo]\n");
	pSensorInfo->SensorPreviewResolutionX= S5K4H5YX_2LANE_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY= S5K4H5YX_2LANE_IMAGE_SENSOR_PV_HEIGHT;
	
	pSensorInfo->SensorFullResolutionX= S5K4H5YX_2LANE_IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY= S5K4H5YX_2LANE_IMAGE_SENSOR_FULL_HEIGHT;
	
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANE.imgMirror = pSensorConfigData->SensorImageMirror ;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);

   	pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_Gr; 

    pSensorInfo->SensorClockPolarity =SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;

    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;

    pSensorInfo->CaptureDelayFrame = 1;
    pSensorInfo->PreviewDelayFrame = 1;
    pSensorInfo->VideoDelayFrame = 2;

    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
    pSensorInfo->AEShutDelayFrame = 0;
    pSensorInfo->AESensorGainDelayFrame = 0 ;
    pSensorInfo->AEISPGainDelayFrame = 2;

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;
			
			pSensorInfo->SensorGrabStartX = S5K4H5YX_2LANE_PV_X_START;
            pSensorInfo->SensorGrabStartY = S5K4H5YX_2LANE_PV_Y_START;
			
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 6;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = S5K4H5YX_2LANE_VIDEO_X_START;
            pSensorInfo->SensorGrabStartY = S5K4H5YX_2LANE_VIDEO_Y_START;
			
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 6;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

			pSensorInfo->SensorGrabStartX = S5K4H5YX_2LANE_FULL_X_START;	
            pSensorInfo->SensorGrabStartY = S5K4H5YX_2LANE_FULL_Y_START;	

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 6;
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        default:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = S5K4H5YX_2LANE_PV_X_START;
            pSensorInfo->SensorGrabStartY = S5K4H5YX_2LANE_PV_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 6;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
    }

    memcpy(pSensorConfigData, &s5k4h5yx2laneSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
} 


UINT32 S5K4H5YX_2LANEControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANECurrentScenarioId = ScenarioId;
	spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEControl]\n");
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            S5K4H5YX_2LANEPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			S5K4H5YX_2LANEVideo(pImageWindow, pSensorConfigData);
			break;   
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            S5K4H5YX_2LANECapture(pImageWindow, pSensorConfigData);
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANESetVideoMode(UINT16 u2FrameRate)
{

    kal_uint32 MIN_Frame_length =0,frameRate=0,extralines=0;
    S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] frame rate = %d\n", u2FrameRate);
	
	if(u2FrameRate==0)
	{
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] Disable Video Mode or dynimac fps\n");
		return KAL_TRUE;
	}
	if(u2FrameRate >30 || u2FrameRate <5)
	    S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] error frame rate seting\n");

    if(S5K4H5YX_2LANE.sensorMode == SENSOR_MODE_VIDEO)
    {
    	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] SENSOR_MODE_VIDEO\n");
    	if(S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode == KAL_TRUE)
    	{
    		if (u2FrameRate==30)
				frameRate= 306;
			else if(u2FrameRate==15)
				frameRate= 148;
			else
				frameRate=u2FrameRate*10;

			MIN_Frame_length = (S5K4H5YX_2LANE.videoPclk)/(S5K4H5YX_2LANE_video_line_length+ S5K4H5YX_2LANE.DummyPixels)/frameRate*10;
    	}
		else
			MIN_Frame_length = (S5K4H5YX_2LANE.videoPclk) /(S5K4H5YX_2LANE_video_line_length + S5K4H5YX_2LANE.DummyPixels)/u2FrameRate;

		if((MIN_Frame_length <=S5K4H5YX_2LANE_video_frame_length))
		{
			MIN_Frame_length = S5K4H5YX_2LANE_video_frame_length;
		}
		extralines = MIN_Frame_length - S5K4H5YX_2LANE_video_frame_length;

		spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
		S5K4H5YX_2LANE.DummyPixels = 0;
		S5K4H5YX_2LANE.DummyLines = extralines ;
		spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);

		S5K4H5YX_2LANE_SetDummy(S5K4H5YX_2LANE.DummyPixels,extralines);
    }
	else if(S5K4H5YX_2LANE.sensorMode == SENSOR_MODE_CAPTURE)
	{
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] SENSOR_MODE_CAPTURE\n");
		if(S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode == KAL_TRUE)
    	{
    		if (u2FrameRate==15)
			    frameRate= 148;
			else
				frameRate=u2FrameRate*10;
			
			MIN_Frame_length = S5K4H5YX_2LANE.capPclk /(S5K4H5YX_2LANE_capture_line_length + S5K4H5YX_2LANE.DummyPixels)/frameRate*10;
    	}
		else
			MIN_Frame_length = S5K4H5YX_2LANE.capPclk /(S5K4H5YX_2LANE_capture_line_length + S5K4H5YX_2LANE.DummyPixels)/u2FrameRate;

		if((MIN_Frame_length <=S5K4H5YX_2LANE_capture_frame_length))
		{
			MIN_Frame_length = S5K4H5YX_2LANE_capture_frame_length;
			S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] current fps = %d\n", S5K4H5YX_2LANE.capPclk /(S5K4H5YX_2LANE_capture_line_length)/S5K4H5YX_2LANE_capture_frame_length);
		}
		
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] current fps (10 base)= %d\n", S5K4H5YX_2LANE.capPclk*10/(S5K4H5YX_2LANE_capture_line_length + S5K4H5YX_2LANE.DummyPixels)/MIN_Frame_length);

		extralines = MIN_Frame_length - S5K4H5YX_2LANE_capture_frame_length;

		spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
		S5K4H5YX_2LANE.DummyPixels = 0;
		S5K4H5YX_2LANE.DummyLines = extralines ;
		spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);

		S5K4H5YX_2LANE_SetDummy(S5K4H5YX_2LANE.DummyPixels,extralines);
	}
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetVideoMode] MIN_Frame_length=%d,S5K4H5YX_2LANE.DummyLines=%d\n",MIN_Frame_length,S5K4H5YX_2LANE.DummyLines);

    return KAL_TRUE;
}



static void S5K4H5YX_2LANEMIPISetMaxFrameRate(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
	kal_uint16 FrameHeight;
		
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEMIPISetMaxFrameRate] u2FrameRate=%d\n",u2FrameRate);

	if(SENSOR_MODE_PREVIEW == S5K4H5YX_2LANE.sensorMode)
	{
		FrameHeight= (10 * S5K4H5YX_2LANE.pvPclk) / u2FrameRate / S5K4H5YX_2LANE_preview_line_length;
		dummy_line = FrameHeight - S5K4H5YX_2LANE_preview_frame_length;

	}
	else if(SENSOR_MODE_CAPTURE== S5K4H5YX_2LANE.sensorMode)
	{
		FrameHeight= (10 * S5K4H5YX_2LANE.capPclk) / u2FrameRate / S5K4H5YX_2LANE_capture_line_length;
		dummy_line = FrameHeight - S5K4H5YX_2LANE_capture_frame_length;
	}
	else
	{
		FrameHeight = (10 * S5K4H5YX_2LANE.videoPclk) / u2FrameRate / S5K4H5YX_2LANE_video_line_length;
		dummy_line = FrameHeight - S5K4H5YX_2LANE_video_frame_length;
	}
	
	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEMIPISetMaxFrameRate] dummy_line=%d",dummy_line);
	dummy_line = ((dummy_line>0) ? dummy_line : 0);
	S5K4H5YX_2LANE_SetDummy(0, dummy_line); 
}



UINT32 S5K4H5YX_2LANESetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	if(bEnable) 
	{
		S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetAutoFlickerMode] enable\n");
		spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
		S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode = KAL_TRUE;
		spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
		if(u2FrameRate == 300)
			S5K4H5YX_2LANEMIPISetMaxFrameRate(296);
		else if(u2FrameRate == 150)
			S5K4H5YX_2LANEMIPISetMaxFrameRate(148);
    } 
	else 
	{
    	S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANESetAutoFlickerMode] disable\n");
    	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
        S5K4H5YX_2LANE.S5K4H5YX_2LANEAutoFlickerMode = KAL_FALSE;
		spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
    }
    return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANESetTestPatternMode(kal_bool bEnable)
{
    return TRUE;
}



UINT32 S5K4H5YX_2LANEMIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) 
{
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEMIPISetMaxFramerateByScenario] MSDK_SCENARIO_ID_CAMERA_PREVIEW\n");
			pclk = S5K4H5YX_2LANE_preview_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
			lineLength = S5K4H5YX_2LANE_preview_line_length;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - S5K4H5YX_2LANE_preview_frame_length;
			S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_PREVIEW;
			S5K4H5YX_2LANE_SetDummy(0, dummyLine);	
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEMIPISetMaxFramerateByScenario] MSDK_SCENARIO_ID_VIDEO_PREVIEW\n");
			pclk = S5K4H5YX_2LANE_video_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
			lineLength = S5K4H5YX_2LANE_video_line_length;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - S5K4H5YX_2LANE_video_frame_length;
			S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_VIDEO;
			S5K4H5YX_2LANE_SetDummy(0, dummyLine);			
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:	
			S5K4H5YX_2LANEDB("[S5K4H5YX_2LANE] [S5K4H5YX_2LANEMIPISetMaxFramerateByScenario] MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG or MSDK_SCENARIO_ID_CAMERA_ZSD\n");
			pclk = S5K4H5YX_2LANE_capture_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
			lineLength = S5K4H5YX_2LANE_capture_line_length;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - S5K4H5YX_2LANE_capture_frame_length;
			S5K4H5YX_2LANE.sensorMode = SENSOR_MODE_CAPTURE;
			S5K4H5YX_2LANE_SetDummy(0, dummyLine);
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: 
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE:  
			break;		
		default:
			break;
	}	
	return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANEMIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 250;
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: 
			 *pframeRate = 300;
			break;		
		default:
			break;
	}

	return ERROR_NONE;
}



UINT32 S5K4H5YX_2LANEFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++= 3264;
            *pFeatureReturnPara16= 2448;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
				*pFeatureReturnPara16++= S5K4H5YX_2LANE_FeatureControl_PERIOD_PixelNum;
				*pFeatureReturnPara16= S5K4H5YX_2LANE_FeatureControl_PERIOD_LineNum;
				*pFeatureParaLen=4;
				break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			switch(S5K4H5YX_2LANECurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
					*pFeatureReturnPara32 = S5K4H5YX_2LANE_preview_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
					*pFeatureParaLen=4;
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*pFeatureReturnPara32 = S5K4H5YX_2LANE_video_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
					*pFeatureParaLen=4;
					break;	 
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara32 = S5K4H5YX_2LANE_capture_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara32 = S5K4H5YX_2LANE_preview_pclk_frequency * S5K4H5YX_2LANE_mhz_to_hz;
					*pFeatureParaLen=4;
					break;
			}
		    break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            S5K4H5YX_2LANE_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            break;
        case SENSOR_FEATURE_SET_GAIN:
            S5K4H5YX_2LANE_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            S5K4H5YX_2LANE_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = S5K4H5YX_2LANE_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
                S5K4H5YX_2LANESensorCCT[i].Addr=*pFeatureData32++;
                S5K4H5YX_2LANESensorCCT[i].Para=*pFeatureData32++;
				spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=S5K4H5YX_2LANESensorCCT[i].Addr;
                *pFeatureData32++=S5K4H5YX_2LANESensorCCT[i].Para;
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&s5k4h5yx2lanemipiraw_drv_lock);
                S5K4H5YX_2LANESensorReg[i].Addr=*pFeatureData32++;
                S5K4H5YX_2LANESensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&s5k4h5yx2lanemipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=S5K4H5YX_2LANESensorReg[i].Addr;
                *pFeatureData32++=S5K4H5YX_2LANESensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=S5K4H5YX_2LANE_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, S5K4H5YX_2LANESensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, S5K4H5YX_2LANESensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &s5k4h5yx2laneSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            S5K4H5YX_2LANE_camera_para_to_sensor();
            break;
        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            S5K4H5YX_2LANE_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=S5K4H5YX_2LANE_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            S5K4H5YX_2LANE_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            S5K4H5YX_2LANE_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_SET_ITEM_INFO:
            S5K4H5YX_2LANE_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;   
   			pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_Gr; 
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_INITIALIZE_AF:
        case SENSOR_FEATURE_CONSTANT_AF:
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            S5K4H5YX_2LANESetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            S5K4H5YX_2LANEGetSensorID(pFeatureReturnPara32);
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            S5K4H5YX_2LANESetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));
	        break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            S5K4H5YX_2LANESetTestPatternMode((BOOL)*pFeatureData16);
            break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			S5K4H5YX_2LANEMIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			S5K4H5YX_2LANEMIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
        default:
            break;
    }
	
    return ERROR_NONE;
}



SENSOR_FUNCTION_STRUCT	SensorFuncS5K4H5YX_2LANE=
{
    S5K4H5YX_2LANEOpen,
    S5K4H5YX_2LANEGetInfo,
    S5K4H5YX_2LANEGetResolution,
    S5K4H5YX_2LANEFeatureControl,
    S5K4H5YX_2LANEControl,
    S5K4H5YX_2LANEClose
};



UINT32 S5K4H5YX_2LANE_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncS5K4H5YX_2LANE;

    return ERROR_NONE;
}

