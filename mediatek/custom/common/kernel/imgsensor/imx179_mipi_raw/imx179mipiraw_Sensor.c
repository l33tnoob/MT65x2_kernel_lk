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
/*****************************************************************************

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

#include "imx179mipiraw_Sensor.h"
#include "imx179mipiraw_Camera_Sensor_para.h"
#include "imx179mipiraw_CameraCustomized.h"

kal_bool  IMX179MIPI_MPEG4_encode_mode = KAL_FALSE;
kal_bool IMX179MIPI_Auto_Flicker_mode = KAL_FALSE;


kal_uint8 IMX179MIPI_sensor_write_I2C_address = IMX179MIPI_WRITE_ID;
kal_uint8 IMX179MIPI_sensor_read_I2C_address = IMX179MIPI_READ_ID;

#define IMX179_USE_OTP

#ifdef IMX179_USE_OTP
static uint16_t used_otp = 0;
static uint16_t ret = -1;
extern int imx179_update_otp_wb(void);
extern int imx179_update_awb_gain(void);
extern int imx179_check_mid(uint mid);
#endif
	
static struct IMX179MIPI_sensor_STRUCT IMX179MIPI_sensor={IMX179MIPI_WRITE_ID,IMX179MIPI_READ_ID,KAL_TRUE,KAL_FALSE,KAL_TRUE,KAL_FALSE,
KAL_FALSE,KAL_FALSE,KAL_FALSE,259200000,259200000,259200000,0,0,0,64,64,64,IMX179MIPI_PV_LINE_LENGTH_PIXELS,IMX179MIPI_PV_FRAME_LENGTH_LINES,
IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS,IMX179MIPI_VIDEO_FRAME_LENGTH_LINES,IMX179MIPI_FULL_LINE_LENGTH_PIXELS,IMX179MIPI_FULL_FRAME_LENGTH_LINES,0,0,0,0,0,0,30};
MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;	
kal_uint16	IMX179MIPI_sensor_gain_base=0x0;
/* MAX/MIN Explosure Lines Used By AE Algorithm */
kal_uint16 IMX179MIPI_MAX_EXPOSURE_LINES = IMX179MIPI_PV_FRAME_LENGTH_LINES-5;//650;
kal_uint8  IMX179MIPI_MIN_EXPOSURE_LINES = 2;
kal_uint32 IMX179MIPI_isp_master_clock;
static DEFINE_SPINLOCK(imx179_drv_lock);

#define SENSORDB(fmt, arg...) printk( "[IMX179MIPIRaw] "  fmt, ##arg)
#define RETAILMSG(x,...)
#define TEXT
UINT8 IMX179MIPIPixelClockDivider=0;
kal_uint16 IMX179MIPI_sensor_id=0;
MSDK_SENSOR_CONFIG_STRUCT IMX179MIPISensorConfigData;
kal_uint32 IMX179MIPI_FAC_SENSOR_REG;
kal_uint16 IMX179MIPI_sensor_flip_value; 
#define IMX179MIPI_MaxGainIndex (97)
kal_uint16 IMX179MIPI_sensorGainMapping[IMX179MIPI_MaxGainIndex][2] ={
{ 64 ,0  },   
{ 68 ,12 },   
{ 71 ,23 },   
{ 74 ,33 },   
{ 77 ,42 },   
{ 81 ,52 },   
{ 84 ,59 },   
{ 87 ,66 },   
{ 90 ,73 },   
{ 93 ,79 },   
{ 96 ,85 },   
{ 100,91 },   
{ 103,96 },   
{ 106,101},   
{ 109,105},   
{ 113,110},   
{ 116,114},   
{ 120,118},   
{ 122,121},   
{ 125,125},   
{ 128,128},   
{ 132,131},   
{ 135,134},   
{ 138,137},
{ 141,139},
{ 144,142},   
{ 148,145},   
{ 151,147},   
{ 153,149}, 
{ 157,151},
{ 160,153},      
{ 164,156},   
{ 168,158},   
{ 169,159},   
{ 173,161},   
{ 176,163},   
{ 180,165}, 
{ 182,166},   
{ 187,168},
{ 189,169},
{ 193,171},
{ 196,172},
{ 200,174},
{ 203,175}, 
{ 205,176},
{ 208,177}, 
{ 213,179}, 
{ 216,180},  
{ 219,181},   
{ 222,182},
{ 225,183},  
{ 228,184},   
{ 232,185},
{ 235,186},
{ 238,187},
{ 241,188},
{ 245,189},
{ 249,190},
{ 253,191},
{ 256,192}, 
{ 260,193},
{ 265,194},
{ 269,195},
{ 274,196},   
{ 278,197},
{ 283,198},
{ 288,199},
{ 293,200},
{ 298,201},   
{ 304,202},   
{ 310,203},
{ 315,204},
{ 322,205},   
{ 328,206},   
{ 335,207},   
{ 342,208},   
{ 349,209},   
{ 357,210},   
{ 365,211},   
{ 373,212}, 
{ 381,213},
{ 400,215},      
{ 420,217},   
{ 432,218},   
{ 443,219},      
{ 468,221},   
{ 482,222},   
{ 497,223},   
{ 512,224},
{ 529,225}, 	 
{ 546,226},   
{ 566,227},   
{ 585,228}, 	 
{ 607,229},   
{ 631,230},   
{ 656,231},   
{ 683,232}
};
/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT IMX179MIPISensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT IMX179MIPISensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define IMX179MIPI_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, IMX179MIPI_WRITE_ID)

kal_uint16 IMX179MIPI_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,IMX179MIPI_WRITE_ID);
    return get_byte;
}

void IMX179MIPI_write_shutter(kal_uint16 shutter)
{
	kal_uint32 frame_length = 0,line_length=0,shutter1=0;
    kal_uint32 extra_lines = 0;
	kal_uint32 max_exp_shutter = 0;
	unsigned long flags;	
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_write_shutter function\n"); 
    if (IMX179MIPI_sensor.pv_mode == KAL_TRUE) 
	 {
	   max_exp_shutter = IMX179MIPI_PV_FRAME_LENGTH_LINES + IMX179MIPI_sensor.pv_dummy_lines-5;
     }
     else if (IMX179MIPI_sensor.video_mode== KAL_TRUE) 
     {
       max_exp_shutter = IMX179MIPI_VIDEO_FRAME_LENGTH_LINES + IMX179MIPI_sensor.video_dummy_lines-5;
	 }	 
     else if (IMX179MIPI_sensor.capture_mode== KAL_TRUE) 
     {
       max_exp_shutter = IMX179MIPI_FULL_FRAME_LENGTH_LINES + IMX179MIPI_sensor.cp_dummy_lines-5;
	 }	 
	 else
	 	{
	 	
		SENSORDB("sensor mode error\n");
	 	}
	 
	 if(shutter > max_exp_shutter)
	   extra_lines = shutter - max_exp_shutter;
	 else 
	   extra_lines = 0;
	 if (IMX179MIPI_sensor.pv_mode == KAL_TRUE) 
	 {
       frame_length =IMX179MIPI_PV_FRAME_LENGTH_LINES+ IMX179MIPI_sensor.pv_dummy_lines + extra_lines;
	   line_length = IMX179MIPI_PV_LINE_LENGTH_PIXELS+ IMX179MIPI_sensor.pv_dummy_pixels;
	   spin_lock_irqsave(&imx179_drv_lock,flags);
	   IMX179MIPI_sensor.pv_line_length = line_length;
	   IMX179MIPI_sensor.pv_frame_length = frame_length;
	   spin_unlock_irqrestore(&imx179_drv_lock,flags);
	 }
	 else if (IMX179MIPI_sensor.video_mode== KAL_TRUE) 
     {
	    frame_length = IMX179MIPI_VIDEO_FRAME_LENGTH_LINES+ IMX179MIPI_sensor.video_dummy_lines + extra_lines;
		line_length =IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS + IMX179MIPI_sensor.video_dummy_pixels;
		spin_lock_irqsave(&imx179_drv_lock,flags);
		IMX179MIPI_sensor.video_line_length = line_length;
	    IMX179MIPI_sensor.video_frame_length = frame_length;
		spin_unlock_irqrestore(&imx179_drv_lock,flags);
	 } 
	 else if(IMX179MIPI_sensor.capture_mode== KAL_TRUE)
	 	{
	    frame_length = IMX179MIPI_FULL_FRAME_LENGTH_LINES+ IMX179MIPI_sensor.cp_dummy_lines + extra_lines;
		line_length =IMX179MIPI_FULL_LINE_LENGTH_PIXELS + IMX179MIPI_sensor.cp_dummy_pixels;
		spin_lock_irqsave(&imx179_drv_lock,flags);
		IMX179MIPI_sensor.cp_line_length = line_length;
	    IMX179MIPI_sensor.cp_frame_length = frame_length;
		spin_unlock_irqrestore(&imx179_drv_lock,flags);
	 }
	 else
	 	{
	 	
		SENSORDB("sensor mode error\n");
	 	}
	//IMX179MIPI_write_cmos_sensor(0x0100,0x00);// STREAM STop
	IMX179MIPI_write_cmos_sensor(0x0104, 1);        
	IMX179MIPI_write_cmos_sensor(0x0340, (frame_length >>8) & 0xFF);
    IMX179MIPI_write_cmos_sensor(0x0341, frame_length & 0xFF);	  
    IMX179MIPI_write_cmos_sensor(0x0202, (shutter >> 8) & 0xFF);
    IMX179MIPI_write_cmos_sensor(0x0203, shutter  & 0xFF);
    IMX179MIPI_write_cmos_sensor(0x0104, 0);    
    SENSORDB("[IMX179MIPI]exit IMX179MIPI_write_shutter function\n");
}   /* write_IMX179MIPI_shutter */

static kal_uint16 IMX179MIPIReg2Gain(const kal_uint8 iReg)
{
	SENSORDB("[IMX179MIPI]enter IMX179MIPIReg2Gain function\n");
    kal_uint8 iI;
    // Range: 1x to 8x
    for (iI = 0; iI < IMX179MIPI_MaxGainIndex; iI++) 
	{
        if(iReg < IMX179MIPI_sensorGainMapping[iI][1])
		{
            break;
        }
		if(iReg == IMX179MIPI_sensorGainMapping[iI][1])			
		{			
			return IMX179MIPI_sensorGainMapping[iI][0];
		}    
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPIReg2Gain function\n");
    return IMX179MIPI_sensorGainMapping[iI-1][0];
}
static kal_uint8 IMX179MIPIGain2Reg(const kal_uint16 iGain)
{
	kal_uint8 iI;
    SENSORDB("[IMX179MIPI]enter IMX179MIPIGain2Reg function\n");
    for (iI = 0; iI < (IMX179MIPI_MaxGainIndex-1); iI++) 
	{
        if(iGain <IMX179MIPI_sensorGainMapping[iI][0])
		{    
            break;
        }
		if(iGain < IMX179MIPI_sensorGainMapping[iI][0])
		{                
			return IMX179MIPI_sensorGainMapping[iI][1];       
		}
			
    }
    if(iGain != IMX179MIPI_sensorGainMapping[iI][0])
    {
         printk("[IMX179MIPIGain2Reg] Gain mapping don't correctly:%d %d \n", iGain, IMX179MIPI_sensorGainMapping[iI][0]);
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPIGain2Reg function\n");
    return IMX179MIPI_sensorGainMapping[iI-1][1];
	//return NONE;
}

/*************************************************************************
* FUNCTION
*    IMX179MIPI_SetGain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    gain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX179MIPI_SetGain(UINT16 iGain)
{   
    kal_uint8 iReg;
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_SetGain function\n");
    iReg = IMX179MIPIGain2Reg(iGain);
	IMX179MIPI_write_cmos_sensor(0x0104, 1);
    IMX179MIPI_write_cmos_sensor(0x0205, (kal_uint8)iReg);
    IMX179MIPI_write_cmos_sensor(0x0104, 0);
    SENSORDB("[IMX179MIPI]exit IMX179MIPI_SetGain function\n");
}   /*  IMX179MIPI_SetGain_SetGain  */


/*************************************************************************
* FUNCTION
*    read_IMX179MIPI_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 read_IMX179MIPI_gain(void)
{  
	SENSORDB("[IMX179MIPI]enter read_IMX179MIPI_gain function\n");
    return (kal_uint16)((IMX179MIPI_read_cmos_sensor(0x0204)<<8) | IMX179MIPI_read_cmos_sensor(0x0205)) ;
}  /* read_IMX179MIPI_gain */

void write_IMX179MIPI_gain(kal_uint16 gain)
{
    IMX179MIPI_SetGain(gain);
}
void IMX179MIPI_camera_para_to_sensor(void)
{

	kal_uint32    i;
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_camera_para_to_sensor function\n");
    for(i=0; 0xFFFFFFFF!=IMX179MIPISensorReg[i].Addr; i++)
    {
        IMX179MIPI_write_cmos_sensor(IMX179MIPISensorReg[i].Addr, IMX179MIPISensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=IMX179MIPISensorReg[i].Addr; i++)
    {
        IMX179MIPI_write_cmos_sensor(IMX179MIPISensorReg[i].Addr, IMX179MIPISensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        IMX179MIPI_write_cmos_sensor(IMX179MIPISensorCCT[i].Addr, IMX179MIPISensorCCT[i].Para);
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPI_camera_para_to_sensor function\n");

}


/*************************************************************************
* FUNCTION
*    IMX179MIPI_sensor_to_camera_para
*
* DESCRIPTION
*    // update camera_para from sensor register
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX179MIPI_sensor_to_camera_para(void)
{

	kal_uint32    i,temp_data;
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_sensor_to_camera_para function\n");
    for(i=0; 0xFFFFFFFF!=IMX179MIPISensorReg[i].Addr; i++)
    {
		temp_data=IMX179MIPI_read_cmos_sensor(IMX179MIPISensorReg[i].Addr);
		spin_lock(&imx179_drv_lock);
		IMX179MIPISensorReg[i].Para = temp_data;
		spin_unlock(&imx179_drv_lock);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=IMX179MIPISensorReg[i].Addr; i++)
    {
    	temp_data=IMX179MIPI_read_cmos_sensor(IMX179MIPISensorReg[i].Addr);
         spin_lock(&imx179_drv_lock);
        IMX179MIPISensorReg[i].Para = temp_data;
		spin_unlock(&imx179_drv_lock);
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPI_sensor_to_camera_para function\n");

}

/*************************************************************************
* FUNCTION
*    IMX179MIPI_get_sensor_group_count
*
* DESCRIPTION
*    //
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_int32  IMX179MIPI_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void IMX179MIPI_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
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

void IMX179MIPI_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
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
                 SENSORDB("[IMX105MIPI][Error]get_sensor_item_info error!!!\n");
          }
           	spin_lock(&imx179_drv_lock);    
            temp_para=IMX179MIPISensorCCT[temp_addr].Para;	
			spin_unlock(&imx179_drv_lock);
            temp_gain = IMX179MIPIReg2Gain(temp_para);
            temp_gain=(temp_gain*1000)/BASEGAIN;
            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min=1000;
            info_ptr->Max=15875;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                
                    //temp_reg=IMX179MIPISensorReg[CMMCLK_CURRENT_INDEX].Para;
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
                    info_ptr->ItemValue=IMX179MIPI_MAX_EXPOSURE_LINES;
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
kal_bool IMX179MIPI_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
   kal_uint16 temp_addr=0, temp_para=0;

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
                 SENSORDB("[IMX105MIPI][Error]set_sensor_item_info error!!!\n");
          }
            temp_para = IMX179MIPIGain2Reg(ItemValue);
            spin_lock(&imx179_drv_lock);    
            IMX179MIPISensorCCT[temp_addr].Para = temp_para;
			spin_unlock(&imx179_drv_lock);
            IMX179MIPI_write_cmos_sensor(IMX179MIPISensorCCT[temp_addr].Addr,temp_para);
			temp_para=read_IMX179MIPI_gain();	
            spin_lock(&imx179_drv_lock);    
            IMX179MIPI_sensor_gain_base=temp_para;
			spin_unlock(&imx179_drv_lock);

            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    if(ItemValue==2)
                    {			
                    spin_lock(&imx179_drv_lock);    
                        IMX179MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_2MA;
					spin_unlock(&imx179_drv_lock);
                        //IMX179MIPI_set_isp_driving_current(ISP_DRIVING_2MA);
                    }
                    else if(ItemValue==3 || ItemValue==4)
                    {
                    	spin_lock(&imx179_drv_lock);    
                        IMX179MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_4MA;
						spin_unlock(&imx179_drv_lock);
                        //IMX179MIPI_set_isp_driving_current(ISP_DRIVING_4MA);
                    }
                    else if(ItemValue==5 || ItemValue==6)
                    {
                    	spin_lock(&imx179_drv_lock);    
                        IMX179MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_6MA;
						spin_unlock(&imx179_drv_lock);
                        //IMX179MIPI_set_isp_driving_current(ISP_DRIVING_6MA);
                    }
                    else
                    {
                    	spin_lock(&imx179_drv_lock);    
                        IMX179MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_8MA;
						spin_unlock(&imx179_drv_lock);
                        //IMX179MIPI_set_isp_driving_current(ISP_DRIVING_8MA);
                    }
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
					spin_lock(&imx179_drv_lock);    
                    IMX179MIPI_FAC_SENSOR_REG=ItemValue;
					spin_unlock(&imx179_drv_lock);
                    break;
                case 1:
                    IMX179MIPI_write_cmos_sensor(IMX179MIPI_FAC_SENSOR_REG,ItemValue);
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
static void IMX179MIPI_SetDummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
	kal_uint32 frame_length = 0, line_length = 0;
    if(IMX179MIPI_sensor.pv_mode == KAL_TRUE)
   	{
   	 spin_lock(&imx179_drv_lock);    
   	 IMX179MIPI_sensor.pv_dummy_pixels = iPixels;
	 IMX179MIPI_sensor.pv_dummy_lines = iLines;
   	 IMX179MIPI_sensor.pv_line_length = IMX179MIPI_PV_LINE_LENGTH_PIXELS + iPixels;
	 IMX179MIPI_sensor.pv_frame_length = IMX179MIPI_PV_FRAME_LENGTH_LINES + iLines;
	 spin_unlock(&imx179_drv_lock);
	 line_length = IMX179MIPI_sensor.pv_line_length;
	 frame_length = IMX179MIPI_sensor.pv_frame_length;	 	
   	}
   else if(IMX179MIPI_sensor.video_mode== KAL_TRUE)
   	{
   	 spin_lock(&imx179_drv_lock);    
   	 IMX179MIPI_sensor.video_dummy_pixels = iPixels;
	 IMX179MIPI_sensor.video_dummy_lines = iLines;
   	 IMX179MIPI_sensor.video_line_length = IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS + iPixels;
	 IMX179MIPI_sensor.video_frame_length = IMX179MIPI_VIDEO_FRAME_LENGTH_LINES + iLines;
	 spin_unlock(&imx179_drv_lock);
	 line_length = IMX179MIPI_sensor.video_line_length;
	 frame_length = IMX179MIPI_sensor.video_frame_length;
   	}
	else if(IMX179MIPI_sensor.capture_mode== KAL_TRUE) 
		{
	  spin_lock(&imx179_drv_lock);	
   	  IMX179MIPI_sensor.cp_dummy_pixels = iPixels;
	  IMX179MIPI_sensor.cp_dummy_lines = iLines;
	  IMX179MIPI_sensor.cp_line_length = IMX179MIPI_FULL_LINE_LENGTH_PIXELS + iPixels;
	  IMX179MIPI_sensor.cp_frame_length = IMX179MIPI_FULL_FRAME_LENGTH_LINES + iLines;
	   spin_unlock(&imx179_drv_lock);
	  line_length = IMX179MIPI_sensor.cp_line_length;
	  frame_length = IMX179MIPI_sensor.cp_frame_length;
    }
	else
	{
	 SENSORDB("[IMX179MIPI]%s(),sensor mode error",__FUNCTION__);
	}
      IMX179MIPI_write_cmos_sensor(0x0104, 1);        	  
      IMX179MIPI_write_cmos_sensor(0x0340, (frame_length >>8) & 0xFF);
      IMX179MIPI_write_cmos_sensor(0x0341, frame_length & 0xFF);	
      IMX179MIPI_write_cmos_sensor(0x0342, (line_length >>8) & 0xFF);
      IMX179MIPI_write_cmos_sensor(0x0343, line_length & 0xFF);
      IMX179MIPI_write_cmos_sensor(0x0104, 0);
}   /*  IMX179MIPI_SetDummy */
static void IMX179MIPI_Sensor_Init(void)
{
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_Sensor_Init function\n");
	IMX179MIPI_write_cmos_sensor(0x41C0, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0104, 0x01);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x00);//STREAM OFF 	
	IMX179MIPI_write_cmos_sensor(0x0103, 0x01);//SW reset
	IMX179MIPI_write_cmos_sensor(0x0101, 0x03);//0x00-HV,0x03-NORMAL
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);		   
	IMX179MIPI_write_cmos_sensor(0x0202, 0x09); //0x0202, 0x09		 
	IMX179MIPI_write_cmos_sensor(0x0203, 0xcc);//(0x0203, 0xcc)
	//PLL setting
	IMX179MIPI_write_cmos_sensor(0x0301, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x0303, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0305, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x0309, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x030B, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x030C, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x030D, 0xA2); //(0x030D, 0xA2)
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	
	IMX179MIPI_write_cmos_sensor(0x0340, 0x09);//(0x0340, 0x09)
	IMX179MIPI_write_cmos_sensor(0x0341, 0xD0); 		 
	IMX179MIPI_write_cmos_sensor(0x0342, 0x0D); 		
	IMX179MIPI_write_cmos_sensor(0x0343, 0x70);//(0x0343, 0x0D) 		 
	IMX179MIPI_write_cmos_sensor(0x0344, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0345, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0346, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0347, 0x00);// (0x0347, 0x00)		 
	IMX179MIPI_write_cmos_sensor(0x0348, 0x0C); 		
	IMX179MIPI_write_cmos_sensor(0x0349, 0xCF); 		 
	IMX179MIPI_write_cmos_sensor(0x034A, 0x09);// (0x034A, 0x09)		 
	IMX179MIPI_write_cmos_sensor(0x034B, 0x9F);//(0x034B, 0x9F) 		 
	IMX179MIPI_write_cmos_sensor(0x034C, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x034D, 0x68); 		 
	IMX179MIPI_write_cmos_sensor(0x034E, 0x04); 		 
	IMX179MIPI_write_cmos_sensor(0x034F, 0xD0);//(0x034F, 0xD0)		 
	IMX179MIPI_write_cmos_sensor(0x0383, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0387, 0x01); 		
	IMX179MIPI_write_cmos_sensor(0x0390, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0401, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0405, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3020, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3041, 0x15); 		 
	IMX179MIPI_write_cmos_sensor(0x3042, 0x87); 		 
	IMX179MIPI_write_cmos_sensor(0x3089, 0x4F); 		 
	IMX179MIPI_write_cmos_sensor(0x3309, 0x9A); 		 
	IMX179MIPI_write_cmos_sensor(0x3344, 0x57);//(0x3344, 0x57)		 
	IMX179MIPI_write_cmos_sensor(0x3345, 0x1F); 		 
	IMX179MIPI_write_cmos_sensor(0x3362, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3363, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3364, 0x00); 		
	IMX179MIPI_write_cmos_sensor(0x3368, 0x18); 		 
	IMX179MIPI_write_cmos_sensor(0x3369, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x3370, 0x77);// (0x3370, 0x77)		 	 
	IMX179MIPI_write_cmos_sensor(0x3371, 0x2F);//(0x3371, 0x2F)             
	IMX179MIPI_write_cmos_sensor(0x3372, 0x4F);             
	IMX179MIPI_write_cmos_sensor(0x3373, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3374, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3375, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x3376, 0x9F);             
	IMX179MIPI_write_cmos_sensor(0x3377, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x33C8, 0x00);             
	IMX179MIPI_write_cmos_sensor(0x33D4, 0x06);             
	IMX179MIPI_write_cmos_sensor(0x33D5, 0x68);             
	IMX179MIPI_write_cmos_sensor(0x33D6, 0x04);             
	IMX179MIPI_write_cmos_sensor(0x33D7, 0xD0);             
	IMX179MIPI_write_cmos_sensor(0x4100, 0x0E);             
	IMX179MIPI_write_cmos_sensor(0x4108, 0x01);             
	IMX179MIPI_write_cmos_sensor(0x4109, 0x7C);
#ifdef IMX179_USE_OTP
        if(ret == 0)
        {
	    SENSORDB("[IMX179_USE_OTP]IMX179MIPI_Sensor_Init function,imx179_update_awb_gain\n");
	    imx179_update_awb_gain();
        }
#endif

	IMX179MIPI_write_cmos_sensor(0x0104, 0x00);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x01);//STREAM ON
	// The register only need to enable 1 time.    
	spin_lock(&imx179_drv_lock);  
	IMX179MIPI_Auto_Flicker_mode = KAL_FALSE;	  // reset the flicker status	 
	spin_unlock(&imx179_drv_lock);
	SENSORDB("[IMX179MIPI]exit IMX179MIPI_Sensor_Init function\n");
}   /*  IMX179MIPI_Sensor_Init  */
void VideoFullSizeSetting(void)//16:9   6M
{
	SENSORDB("[IMX179MIPI]enter VideoFullSizeSetting function\n");
	IMX179MIPI_write_cmos_sensor(0x41C0, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0104, 0x01);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x00);//STREAM OFF 	
	IMX179MIPI_write_cmos_sensor(0x0103, 0x01);//SW reset
	IMX179MIPI_write_cmos_sensor(0x0101, 0x03);//0x00-HV,0x03-NORMAL
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);		   
	IMX179MIPI_write_cmos_sensor(0x0202, 0x09); //0x0202, 0x09		 
	IMX179MIPI_write_cmos_sensor(0x0203, 0xcc);//(0x0203, 0xcc)
	//PLL setting
	IMX179MIPI_write_cmos_sensor(0x0301, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x0303, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0305, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x0309, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x030B, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x030C, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x030D, 0xA4); //(0x030D, 0xA2)
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	
	IMX179MIPI_write_cmos_sensor(0x0340, 0x09);//(0x0340, 0x09)
	IMX179MIPI_write_cmos_sensor(0x0341, 0xD0); 		 
	IMX179MIPI_write_cmos_sensor(0x0342, 0x0D); 		
	IMX179MIPI_write_cmos_sensor(0x0343, 0x70);//(0x0343, 0x0D) 		 
	IMX179MIPI_write_cmos_sensor(0x0344, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0345, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0346, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0347, 0x32);// (0x0347, 0x00)		 
	IMX179MIPI_write_cmos_sensor(0x0348, 0x0C); 		
	IMX179MIPI_write_cmos_sensor(0x0349, 0xCF); 		 
	IMX179MIPI_write_cmos_sensor(0x034A, 0x08);// (0x034A, 0x09)		 
	IMX179MIPI_write_cmos_sensor(0x034B, 0x6D);//(0x034B, 0x9F) 		 
	IMX179MIPI_write_cmos_sensor(0x034C, 0x0C); 		 
	IMX179MIPI_write_cmos_sensor(0x034D, 0xD0); 		 
	IMX179MIPI_write_cmos_sensor(0x034E, 0x07); 		 
	IMX179MIPI_write_cmos_sensor(0x034F, 0x3C);//(0x034F, 0xD0)		 
	IMX179MIPI_write_cmos_sensor(0x0383, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0387, 0x01); 		
	IMX179MIPI_write_cmos_sensor(0x0390, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0401, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0405, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3020, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3041, 0x15); 		 
	IMX179MIPI_write_cmos_sensor(0x3042, 0x87); 		 
	IMX179MIPI_write_cmos_sensor(0x3089, 0x4F); 		 
	IMX179MIPI_write_cmos_sensor(0x3309, 0x9A); 		 
	IMX179MIPI_write_cmos_sensor(0x3344, 0x57);//(0x3344, 0x57)		 
	IMX179MIPI_write_cmos_sensor(0x3345, 0x1F); 		 
	IMX179MIPI_write_cmos_sensor(0x3362, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3363, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3364, 0x00); 		
	IMX179MIPI_write_cmos_sensor(0x3368, 0x18); 		 
	IMX179MIPI_write_cmos_sensor(0x3369, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x3370, 0x77);// (0x3370, 0x77)		 	 
	IMX179MIPI_write_cmos_sensor(0x3371, 0x2F);//(0x3371, 0x2F)             
	IMX179MIPI_write_cmos_sensor(0x3372, 0x4F);             
	IMX179MIPI_write_cmos_sensor(0x3373, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3374, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3375, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x3376, 0x9F);             
	IMX179MIPI_write_cmos_sensor(0x3377, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x33C8, 0x00);             
	IMX179MIPI_write_cmos_sensor(0x33D4, 0x0C);             
	IMX179MIPI_write_cmos_sensor(0x33D5, 0xD0);             
	IMX179MIPI_write_cmos_sensor(0x33D6, 0x07);             
	IMX179MIPI_write_cmos_sensor(0x33D7, 0x3C);             
	IMX179MIPI_write_cmos_sensor(0x4100, 0x0E);             
	IMX179MIPI_write_cmos_sensor(0x4108, 0x01);             
	IMX179MIPI_write_cmos_sensor(0x4109, 0x7C);
#ifdef IMX179_USE_OTP
        if(ret == 0)
        {
	    SENSORDB("[IMX179_USE_OTP]VideoFullSizeSetting function,imx179_update_awb_gain\n");
	    imx179_update_awb_gain();
        }
#endif
	IMX179MIPI_write_cmos_sensor(0x0104, 0x00);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x01);//STREAM ON
	SENSORDB("[IMX179MIPI]exit VideoFullSizeSetting function\n");
}


void VideoUsePreviewSetting(void)
{
	SENSORDB("[IMX179MIPI]enter VideoUsePreviewSetting function\n");
	IMX179MIPI_write_cmos_sensor(0x41C0, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0104, 0x01);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x00);//STREAM OFF 	
	IMX179MIPI_write_cmos_sensor(0x0103, 0x01);//SW reset
	IMX179MIPI_write_cmos_sensor(0x0101, 0x03);//0x00-HV,0x03-NORMAL
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);		   
	IMX179MIPI_write_cmos_sensor(0x0202, 0x09); //0x0202, 0x09		 
	IMX179MIPI_write_cmos_sensor(0x0203, 0xcc);//(0x0203, 0xcc)
	//PLL setting
	IMX179MIPI_write_cmos_sensor(0x0301, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x0303, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0305, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x0309, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x030B, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x030C, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x030D, 0xA2); //(0x030D, 0xA2)
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	
	IMX179MIPI_write_cmos_sensor(0x0340, 0x09);//(0x0340, 0x09)
	IMX179MIPI_write_cmos_sensor(0x0341, 0xD0); 		 
	IMX179MIPI_write_cmos_sensor(0x0342, 0x0D); 		
	IMX179MIPI_write_cmos_sensor(0x0343, 0x70);//(0x0343, 0x0D) 		 
	IMX179MIPI_write_cmos_sensor(0x0344, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0345, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0346, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0347, 0x00);// (0x0347, 0x00)		 
	IMX179MIPI_write_cmos_sensor(0x0348, 0x0C); 		
	IMX179MIPI_write_cmos_sensor(0x0349, 0xCF); 		 
	IMX179MIPI_write_cmos_sensor(0x034A, 0x09);// (0x034A, 0x09)		 
	IMX179MIPI_write_cmos_sensor(0x034B, 0x9F);//(0x034B, 0x9F) 		 
	IMX179MIPI_write_cmos_sensor(0x034C, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x034D, 0x68); 		 
	IMX179MIPI_write_cmos_sensor(0x034E, 0x04); 		 
	IMX179MIPI_write_cmos_sensor(0x034F, 0xD0);//(0x034F, 0xD0)		 
	IMX179MIPI_write_cmos_sensor(0x0383, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0387, 0x01); 		
	IMX179MIPI_write_cmos_sensor(0x0390, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0401, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0405, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3020, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3041, 0x15); 		 
	IMX179MIPI_write_cmos_sensor(0x3042, 0x87); 		 
	IMX179MIPI_write_cmos_sensor(0x3089, 0x4F); 		 
	IMX179MIPI_write_cmos_sensor(0x3309, 0x9A); 		 
	IMX179MIPI_write_cmos_sensor(0x3344, 0x57);//(0x3344, 0x57)		 
	IMX179MIPI_write_cmos_sensor(0x3345, 0x1F); 		 
	IMX179MIPI_write_cmos_sensor(0x3362, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3363, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3364, 0x00); 		
	IMX179MIPI_write_cmos_sensor(0x3368, 0x18); 		 
	IMX179MIPI_write_cmos_sensor(0x3369, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x3370, 0x77);// (0x3370, 0x77)		 	 
	IMX179MIPI_write_cmos_sensor(0x3371, 0x2F);//(0x3371, 0x2F)             
	IMX179MIPI_write_cmos_sensor(0x3372, 0x4F);             
	IMX179MIPI_write_cmos_sensor(0x3373, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3374, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3375, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x3376, 0x9F);             
	IMX179MIPI_write_cmos_sensor(0x3377, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x33C8, 0x00);             
	IMX179MIPI_write_cmos_sensor(0x33D4, 0x06);             
	IMX179MIPI_write_cmos_sensor(0x33D5, 0x68);             
	IMX179MIPI_write_cmos_sensor(0x33D6, 0x04);             
	IMX179MIPI_write_cmos_sensor(0x33D7, 0xD0);             
	IMX179MIPI_write_cmos_sensor(0x4100, 0x0E);             
	IMX179MIPI_write_cmos_sensor(0x4108, 0x01);             
	IMX179MIPI_write_cmos_sensor(0x4109, 0x7C);
#ifdef IMX179_USE_OTP
        if(ret == 0)
        {
	    SENSORDB("[IMX179_USE_OTP]PreviewSetting function,imx179_update_awb_gain\n");
	    imx179_update_awb_gain();
        }
#endif
	IMX179MIPI_write_cmos_sensor(0x0104, 0x00);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x01);//STREAM ON
	// The register only need to enable 1 time.    
	spin_lock(&imx179_drv_lock);  
	IMX179MIPI_Auto_Flicker_mode = KAL_FALSE;	  // reset the flicker status	 
	spin_unlock(&imx179_drv_lock);
	SENSORDB("[IMX179MIPI]exit VideoUsePreviewSetting function\n");
}


void Video720pSetting(void) // 720P  // Brown
{
	SENSORDB("[IMX179MIPI]enter Video 720P function. -----Warning!!!  No AE in video call \n");
#if 0    
	IMX179MIPI_write_cmos_sensor(0x41C0, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0104, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0100, 0x00);
	IMX179MIPI_write_cmos_sensor(0x0103, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0101, 0x00);
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0202, 0x09);
	IMX179MIPI_write_cmos_sensor(0x0203, 0xcA);
	//PLL setting                              
	IMX179MIPI_write_cmos_sensor(0x0301, 0x05);
	IMX179MIPI_write_cmos_sensor(0x0303, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0305, 0x06);
	IMX179MIPI_write_cmos_sensor(0x0309, 0x05);
	IMX179MIPI_write_cmos_sensor(0x030B, 0x01);
	IMX179MIPI_write_cmos_sensor(0x030C, 0x00);
	IMX179MIPI_write_cmos_sensor(0x030D, 0xA0);
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	                                           
	IMX179MIPI_write_cmos_sensor(0x0340, 0x09);
	IMX179MIPI_write_cmos_sensor(0x0341, 0xCE);
	IMX179MIPI_write_cmos_sensor(0x0342, 0x0D);
	IMX179MIPI_write_cmos_sensor(0x0343, 0x48);
	IMX179MIPI_write_cmos_sensor(0x0344, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0345, 0x60);
	IMX179MIPI_write_cmos_sensor(0x0346, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0347, 0x08);
	IMX179MIPI_write_cmos_sensor(0x0348, 0x0B);
	IMX179MIPI_write_cmos_sensor(0x0349, 0x6F);
	IMX179MIPI_write_cmos_sensor(0x034A, 0x08);
	IMX179MIPI_write_cmos_sensor(0x034B, 0x97);
                                             
	IMX179MIPI_write_cmos_sensor(0x034C, 0x05);
	IMX179MIPI_write_cmos_sensor(0x034D, 0x08);
	IMX179MIPI_write_cmos_sensor(0x034E, 0x03);
	IMX179MIPI_write_cmos_sensor(0x034F, 0xC8);
                                             
                                             
	IMX179MIPI_write_cmos_sensor(0x0383, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0387, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0390, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0401, 0x00);
	IMX179MIPI_write_cmos_sensor(0x0405, 0x10);
	IMX179MIPI_write_cmos_sensor(0x3020, 0x10);
	IMX179MIPI_write_cmos_sensor(0x3041, 0x15);
	IMX179MIPI_write_cmos_sensor(0x3042, 0x87);
	IMX179MIPI_write_cmos_sensor(0x3089, 0x4F);
	IMX179MIPI_write_cmos_sensor(0x3309, 0x9A);
	IMX179MIPI_write_cmos_sensor(0x3344, 0x57);
	IMX179MIPI_write_cmos_sensor(0x3345, 0x1F);
	IMX179MIPI_write_cmos_sensor(0x3362, 0x0A);
	IMX179MIPI_write_cmos_sensor(0x3363, 0x0A);
	IMX179MIPI_write_cmos_sensor(0x3364, 0x00);
	IMX179MIPI_write_cmos_sensor(0x3368, 0x18);
	IMX179MIPI_write_cmos_sensor(0x3369, 0x00);
	IMX179MIPI_write_cmos_sensor(0x3370, 0x77);
	IMX179MIPI_write_cmos_sensor(0x3371, 0x2F);
	IMX179MIPI_write_cmos_sensor(0x3372, 0x4F);
	IMX179MIPI_write_cmos_sensor(0x3373, 0x2F);
	IMX179MIPI_write_cmos_sensor(0x3374, 0x2F);
	IMX179MIPI_write_cmos_sensor(0x3375, 0x37);
	IMX179MIPI_write_cmos_sensor(0x3376, 0x9F);
	IMX179MIPI_write_cmos_sensor(0x3377, 0x37);
	IMX179MIPI_write_cmos_sensor(0x33C8, 0x00);
	IMX179MIPI_write_cmos_sensor(0x33D4, 0x05);
	IMX179MIPI_write_cmos_sensor(0x33D5, 0x08);
	IMX179MIPI_write_cmos_sensor(0x33D6, 0x03); // Jiangde 02--> 03
	IMX179MIPI_write_cmos_sensor(0x33D7, 0xc8); // Jiangde d6--> c8
	IMX179MIPI_write_cmos_sensor(0x4100, 0x0E);
	IMX179MIPI_write_cmos_sensor(0x4108, 0x01);
	IMX179MIPI_write_cmos_sensor(0x4109, 0x7C);
    
#ifdef IMX179_USE_OTP
        if(ret == 0)
        {
	    SENSORDB("[IMX179_USE_OTP]VideoFullSizeSetting function,imx179_update_awb_gain\n");
	    imx179_update_awb_gain();
        }
#endif

	IMX179MIPI_write_cmos_sensor(0x0104, 0x00);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x01);//STREAM ON
#endif
	SENSORDB("[IMX179MIPI]exit Video720pSetting function\n");
}

void PreviewSetting(void)
{
	SENSORDB("[IMX179MIPI]enter PreviewSetting function\n");
	IMX179MIPI_write_cmos_sensor(0x41C0, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0104, 0x01);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x00);//STREAM OFF 	
	IMX179MIPI_write_cmos_sensor(0x0103, 0x01);//SW reset
	IMX179MIPI_write_cmos_sensor(0x0101, 0x03);//0x00-HV,0x03-NORMAL
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);		   
	IMX179MIPI_write_cmos_sensor(0x0202, 0x09); //0x0202, 0x09		 
	IMX179MIPI_write_cmos_sensor(0x0203, 0xcc);//(0x0203, 0xcc)
	//PLL setting
	IMX179MIPI_write_cmos_sensor(0x0301, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x0303, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0305, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x0309, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x030B, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x030C, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x030D, 0xA2); //(0x030D, 0xA2)
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	
	IMX179MIPI_write_cmos_sensor(0x0340, 0x09);//(0x0340, 0x09)
	IMX179MIPI_write_cmos_sensor(0x0341, 0xD0); 		 
	IMX179MIPI_write_cmos_sensor(0x0342, 0x0D); 		
	IMX179MIPI_write_cmos_sensor(0x0343, 0x70);//(0x0343, 0x0D) 		 
	IMX179MIPI_write_cmos_sensor(0x0344, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0345, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0346, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0347, 0x00);// (0x0347, 0x00)		 
	IMX179MIPI_write_cmos_sensor(0x0348, 0x0C); 		
	IMX179MIPI_write_cmos_sensor(0x0349, 0xCF); 		 
	IMX179MIPI_write_cmos_sensor(0x034A, 0x09);// (0x034A, 0x09)		 
	IMX179MIPI_write_cmos_sensor(0x034B, 0x9F);//(0x034B, 0x9F) 		 
	IMX179MIPI_write_cmos_sensor(0x034C, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x034D, 0x68); 		 
	IMX179MIPI_write_cmos_sensor(0x034E, 0x04); 		 
	IMX179MIPI_write_cmos_sensor(0x034F, 0xD0);//(0x034F, 0xD0)		 
	IMX179MIPI_write_cmos_sensor(0x0383, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0387, 0x01); 		
	IMX179MIPI_write_cmos_sensor(0x0390, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0401, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0405, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3020, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3041, 0x15); 		 
	IMX179MIPI_write_cmos_sensor(0x3042, 0x87); 		 
	IMX179MIPI_write_cmos_sensor(0x3089, 0x4F); 		 
	IMX179MIPI_write_cmos_sensor(0x3309, 0x9A); 		 
	IMX179MIPI_write_cmos_sensor(0x3344, 0x57);//(0x3344, 0x57)		 
	IMX179MIPI_write_cmos_sensor(0x3345, 0x1F); 		 
	IMX179MIPI_write_cmos_sensor(0x3362, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3363, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3364, 0x00); 		
	IMX179MIPI_write_cmos_sensor(0x3368, 0x18); 		 
	IMX179MIPI_write_cmos_sensor(0x3369, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x3370, 0x77);// (0x3370, 0x77)		 	 
	IMX179MIPI_write_cmos_sensor(0x3371, 0x2F);//(0x3371, 0x2F)             
	IMX179MIPI_write_cmos_sensor(0x3372, 0x4F);             
	IMX179MIPI_write_cmos_sensor(0x3373, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3374, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3375, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x3376, 0x9F);             
	IMX179MIPI_write_cmos_sensor(0x3377, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x33C8, 0x00);             
	IMX179MIPI_write_cmos_sensor(0x33D4, 0x06);             
	IMX179MIPI_write_cmos_sensor(0x33D5, 0x68);             
	IMX179MIPI_write_cmos_sensor(0x33D6, 0x04);             
	IMX179MIPI_write_cmos_sensor(0x33D7, 0xD0);             
	IMX179MIPI_write_cmos_sensor(0x4100, 0x0E);             
	IMX179MIPI_write_cmos_sensor(0x4108, 0x01);             
	IMX179MIPI_write_cmos_sensor(0x4109, 0x7C);
#ifdef IMX179_USE_OTP
        if(ret == 0)
        {
	    SENSORDB("[IMX179_USE_OTP]PreviewSetting function,imx179_update_awb_gain\n");
	    imx179_update_awb_gain();
        }
#endif
	IMX179MIPI_write_cmos_sensor(0x0104, 0x00);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x01);//STREAM ON
	// The register only need to enable 1 time.    
	spin_lock(&imx179_drv_lock);  
	IMX179MIPI_Auto_Flicker_mode = KAL_FALSE;	  // reset the flicker status	 
	spin_unlock(&imx179_drv_lock);
	SENSORDB("[IMX179MIPI]exit PreviewSetting function\n");
}

void IMX179MIPI_set_8M(void)
{	//77 capture setting
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_set_8M function\n");
	IMX179MIPI_write_cmos_sensor(0x41C0, 0x01);
	IMX179MIPI_write_cmos_sensor(0x0104, 0x01);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x00);//STREAM OFF 	
	IMX179MIPI_write_cmos_sensor(0x0103, 0x01);//SW reset
	IMX179MIPI_write_cmos_sensor(0x0101, 0x03);//0x00-HV,0x03-NORMAL
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);		   
	IMX179MIPI_write_cmos_sensor(0x0202, 0x0C); //0x0202, 0x09		 
	IMX179MIPI_write_cmos_sensor(0x0203, 0x40);//(0x0203, 0xcc)
	//PLL setting
	IMX179MIPI_write_cmos_sensor(0x0301, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x0303, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0305, 0x06); 		 
	IMX179MIPI_write_cmos_sensor(0x0309, 0x05); 		 
	IMX179MIPI_write_cmos_sensor(0x030B, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x030C, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x030D, 0xA2); //(0x030D, 0xA2)
	IMX179MIPI_write_cmos_sensor(0x030E, 0x01);
	
	IMX179MIPI_write_cmos_sensor(0x0340, 0x0C);//(0x0340, 0x09)
	IMX179MIPI_write_cmos_sensor(0x0341, 0x44); 		 
	IMX179MIPI_write_cmos_sensor(0x0342, 0x0D); 		
	IMX179MIPI_write_cmos_sensor(0x0343, 0x70);//(0x0343, 0x0D) 		 
	IMX179MIPI_write_cmos_sensor(0x0344, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0345, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0346, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0347, 0x00);// (0x0347, 0x00)		 
	IMX179MIPI_write_cmos_sensor(0x0348, 0x0C); 		
	IMX179MIPI_write_cmos_sensor(0x0349, 0xCF); 		 
	IMX179MIPI_write_cmos_sensor(0x034A, 0x09);// (0x034A, 0x09)		 
	IMX179MIPI_write_cmos_sensor(0x034B, 0x9F);//(0x034B, 0x9F) 		 
	IMX179MIPI_write_cmos_sensor(0x034C, 0x0C); 		 
	IMX179MIPI_write_cmos_sensor(0x034D, 0xD0); 		 
	IMX179MIPI_write_cmos_sensor(0x034E, 0x09); 		 
	IMX179MIPI_write_cmos_sensor(0x034F, 0xA0);//(0x034F, 0xD0)		 
	IMX179MIPI_write_cmos_sensor(0x0383, 0x01); 		 
	IMX179MIPI_write_cmos_sensor(0x0387, 0x01); 		
	IMX179MIPI_write_cmos_sensor(0x0390, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0401, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x0405, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3020, 0x10); 		 
	IMX179MIPI_write_cmos_sensor(0x3041, 0x15); 		 
	IMX179MIPI_write_cmos_sensor(0x3042, 0x87); 		 
	IMX179MIPI_write_cmos_sensor(0x3089, 0x4F); 		 
	IMX179MIPI_write_cmos_sensor(0x3309, 0x9A); 		 
	IMX179MIPI_write_cmos_sensor(0x3344, 0x57);//(0x3344, 0x57)		 
	IMX179MIPI_write_cmos_sensor(0x3345, 0x1F); 		 
	IMX179MIPI_write_cmos_sensor(0x3362, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3363, 0x0A); 		 
	IMX179MIPI_write_cmos_sensor(0x3364, 0x00); 		
	IMX179MIPI_write_cmos_sensor(0x3368, 0x18); 		 
	IMX179MIPI_write_cmos_sensor(0x3369, 0x00); 		 
	IMX179MIPI_write_cmos_sensor(0x3370, 0x77);// (0x3370, 0x77)		 	 
	IMX179MIPI_write_cmos_sensor(0x3371, 0x2F);//(0x3371, 0x2F)             
	IMX179MIPI_write_cmos_sensor(0x3372, 0x4F);             
	IMX179MIPI_write_cmos_sensor(0x3373, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3374, 0x2F);             
	IMX179MIPI_write_cmos_sensor(0x3375, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x3376, 0x9F);             
	IMX179MIPI_write_cmos_sensor(0x3377, 0x37);             
	IMX179MIPI_write_cmos_sensor(0x33C8, 0x00);             
	IMX179MIPI_write_cmos_sensor(0x33D4, 0x0C);             
	IMX179MIPI_write_cmos_sensor(0x33D5, 0xD0);             
	IMX179MIPI_write_cmos_sensor(0x33D6, 0x09);             
	IMX179MIPI_write_cmos_sensor(0x33D7, 0xA0);             
	IMX179MIPI_write_cmos_sensor(0x4100, 0x0E);             
	IMX179MIPI_write_cmos_sensor(0x4108, 0x01);             
	IMX179MIPI_write_cmos_sensor(0x4109, 0x7C);
#ifdef IMX179_USE_OTP
        if(ret == 0)
        {
	    SENSORDB("[IMX179_USE_OTP]IMX179MIPI_set_8M function,imx179_update_awb_gain\n");
	    imx179_update_awb_gain();
        }
#endif
	IMX179MIPI_write_cmos_sensor(0x0104, 0x00);//group
	IMX179MIPI_write_cmos_sensor(0x0100, 0x01);//STREAM ON
	SENSORDB("[IMX179MIPI]exit IMX179MIPI_set_8M function\n"); 
}
/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*   IMX179MIPIOpen
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/

UINT32 IMX179MIPIOpen(void)
{
#ifdef IMX179_USE_OTP
    if(0 == used_otp){
	printk("[IMX179_USE_OTP] before update otp wb...........................................\n");
	printk("[IMX179_USE_OTP] before update otp wb...........................................\n");
	printk("[IMX179_USE_OTP] before update otp wb...........................................\n");
	ret = imx179_update_otp_wb();

	used_otp =1;
	printk("[IMX179_USE_OTP] after update otp wb............................................\n");
	printk("[IMX179_USE_OTP] after update otp wb............................................\n");
	printk("[IMX179_USE_OTP] after update otp wb............................................\n");
    }
#endif
    int  retry = 0; 
	kal_uint16 sensorid;
    // check if sensor ID correct
    retry = 3; 
	SENSORDB("[IMX179MIPI]enter IMX179MIPIOpen function\n");
    do {
	   sensorid=(kal_uint16)(((IMX179MIPI_read_cmos_sensor(0x0002)&&0x0f)<<8) | IMX179MIPI_read_cmos_sensor(0x0003));  
	   spin_lock(&imx179_drv_lock);    
	   IMX179MIPI_sensor_id =sensorid;
	   spin_unlock(&imx179_drv_lock);
		if (IMX179MIPI_sensor_id == IMX179MIPI_SENSOR_ID)
			break; 
		retry--; 
	    }
	while (retry > 0);
    SENSORDB("Read Sensor ID = 0x%04x\n", IMX179MIPI_sensor_id);
    if (IMX179MIPI_sensor_id != IMX179MIPI_SENSOR_ID)
        return ERROR_SENSOR_CONNECT_FAIL;
    IMX179MIPI_Sensor_Init();
	sensorid=read_IMX179MIPI_gain();
	spin_lock(&imx179_drv_lock);	
    IMX179MIPI_sensor_gain_base = sensorid;
	spin_unlock(&imx179_drv_lock);
	mdelay(50); // Jiangde ++    
	SENSORDB("[IMX179MIPI]exit IMX179MIPIOpen function\n");
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   IMX179MIPIGetSensorID
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
UINT32 IMX179MIPIGetSensorID(UINT32 *sensorID) 
{
    int  retry = 3; 
	SENSORDB("[IMX179MIPI]enter IMX179MIPIGetSensorID function\n");
    // check if sensor ID correct
    do {		
	   *sensorID =(kal_uint16)(((IMX179MIPI_read_cmos_sensor(0x0002)&&0x0f)<<8) | IMX179MIPI_read_cmos_sensor(0x0003)); 

        SENSORDB("HJDDBG, [IMX179MIPI] sensorID = 0x%x. \n");
        if (*sensorID == IMX179MIPI_SENSOR_ID)
            break;
        retry--;

        if (retry > 0)
        {
            mdelay(2); // Jiangde, retry after a while
            SENSORDB("HJDDBG, [IMX179MIPI] retry after a while, retry=%d. \n", retry);
        }
    } while (retry > 0);

    if (*sensorID != IMX179MIPI_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPIGetSensorID function\n");
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*   IMX179MIPI_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of IMX179MIPI to change exposure time.
*
* PARAMETERS
*   shutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX179MIPI_SetShutter(kal_uint16 iShutter)
{

	SENSORDB("[IMX179MIPI]%s():shutter=%d\n",__FUNCTION__,iShutter);
    if (iShutter < 1)
        iShutter = 1; 
	else if(iShutter > 0xffff)
		iShutter = 0xffff;
	unsigned long flags;
	spin_lock_irqsave(&imx179_drv_lock,flags);
    IMX179MIPI_sensor.pv_shutter = iShutter;	
	spin_unlock_irqrestore(&imx179_drv_lock,flags);
    IMX179MIPI_write_shutter(iShutter);
	SENSORDB("[IMX179MIPI]exit IMX179MIPIGetSensorID function\n");
}   /*  IMX179MIPI_SetShutter   */



/*************************************************************************
* FUNCTION
*   IMX179MIPI_read_shutter
*
* DESCRIPTION
*   This function to  Get exposure time.
*
* PARAMETERS
*   None
*
* RETURNS
*   shutter : exposured lines
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT16 IMX179MIPI_read_shutter(void)
{
    return (UINT16)( (IMX179MIPI_read_cmos_sensor(0x0202)<<8) | IMX179MIPI_read_cmos_sensor(0x0203) );
}

/*************************************************************************
* FUNCTION
*   IMX179MIPI_night_mode
*
* DESCRIPTION
*   This function night mode of IMX179MIPI.
*
* PARAMETERS
*   none
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void IMX179MIPI_NightMode(kal_bool bEnable)
{
	SENSORDB("[IMX179MIPI]enter IMX179MIPI_NightMode function\n");
#if 0
    /************************************************************************/
    /*                      Auto Mode: 30fps                                                                                          */
    /*                      Night Mode:15fps                                                                                          */
    /************************************************************************/
    if(bEnable)
    {
        if(OV5642_MPEG4_encode_mode==KAL_TRUE)
        {
            OV5642_MAX_EXPOSURE_LINES = (kal_uint16)((OV5642_sensor_pclk/15)/(OV5642_PV_PERIOD_PIXEL_NUMS+OV5642_PV_dummy_pixels));
            OV5642_write_cmos_sensor(0x350C, (OV5642_MAX_EXPOSURE_LINES >> 8) & 0xFF);
            OV5642_write_cmos_sensor(0x350D, OV5642_MAX_EXPOSURE_LINES & 0xFF);
            OV5642_CURRENT_FRAME_LINES = OV5642_MAX_EXPOSURE_LINES;
            OV5642_MAX_EXPOSURE_LINES = OV5642_CURRENT_FRAME_LINES - OV5642_SHUTTER_LINES_GAP;
        }
    }
    else// Fix video framerate 30 fps
    {
        if(OV5642_MPEG4_encode_mode==KAL_TRUE)
        {
            OV5642_MAX_EXPOSURE_LINES = (kal_uint16)((OV5642_sensor_pclk/30)/(OV5642_PV_PERIOD_PIXEL_NUMS+OV5642_PV_dummy_pixels));
            if(OV5642_pv_exposure_lines < (OV5642_MAX_EXPOSURE_LINES - OV5642_SHUTTER_LINES_GAP)) // for avoid the shutter > frame_lines,move the frame lines setting to shutter function
            {
                OV5642_write_cmos_sensor(0x350C, (OV5642_MAX_EXPOSURE_LINES >> 8) & 0xFF);
                OV5642_write_cmos_sensor(0x350D, OV5642_MAX_EXPOSURE_LINES & 0xFF);
                OV5642_CURRENT_FRAME_LINES = OV5642_MAX_EXPOSURE_LINES;
            }
            OV5642_MAX_EXPOSURE_LINES = OV5642_MAX_EXPOSURE_LINES - OV5642_SHUTTER_LINES_GAP;
        }
    }
	
#endif	
	SENSORDB("[IMX179MIPI]exit IMX179MIPI_NightMode function\n");
}/*	IMX179MIPI_NightMode */



/*************************************************************************
* FUNCTION
*   IMX179MIPIClose
*
* DESCRIPTION
*   This function is to turn off sensor module power.
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 IMX179MIPIClose(void)
{
    IMX179MIPI_write_cmos_sensor(0x0100,0x00);
    return ERROR_NONE;
}	/* IMX179MIPIClose() */

void IMX179MIPISetFlipMirror(kal_int32 imgMirror)
{
    kal_uint8  iTemp; 
    SENSORDB("[IMX179MIPI]enter IMX179MIPISetFlipMirror function,imgMirror == %d\n",imgMirror);
    iTemp = IMX179MIPI_read_cmos_sensor(0x0101) & 0x03;	//Clear the mirror and flip bits.
    switch (imgMirror)
    {
        case IMAGE_NORMAL:
            IMX179MIPI_write_cmos_sensor(0x0101, 0x03);	//Set normal
            break;
        case IMAGE_V_MIRROR:
            IMX179MIPI_write_cmos_sensor(0x0101, iTemp | 0x01);	//Set flip
            break;
        case IMAGE_H_MIRROR:
            IMX179MIPI_write_cmos_sensor(0x0101, iTemp | 0x02);	//Set mirror
            break;
        case IMAGE_HV_MIRROR:
            IMX179MIPI_write_cmos_sensor(0x0101, 0x00);	//Set mirror and flip
            break;
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPISetFlipMirror function\n");
}


/*************************************************************************
* FUNCTION
*   IMX179MIPIPreview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 IMX179MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint16 iStartX = 0, iStartY = 0;	
	SENSORDB("[IMX179MIPI]enter IMX179MIPIPreview function\n");
	spin_lock(&imx179_drv_lock);    
	IMX179MIPI_MPEG4_encode_mode = KAL_FALSE;
	IMX179MIPI_sensor.video_mode=KAL_FALSE;
	IMX179MIPI_sensor.pv_mode=KAL_TRUE;
	IMX179MIPI_sensor.capture_mode=KAL_FALSE;
	spin_unlock(&imx179_drv_lock);
	PreviewSetting();
	IMX179MIPISetFlipMirror(IMAGE_NORMAL);
	iStartX += IMX179MIPI_IMAGE_SENSOR_PV_STARTX;
	iStartY += IMX179MIPI_IMAGE_SENSOR_PV_STARTY;
	spin_lock(&imx179_drv_lock);	
	IMX179MIPI_sensor.cp_dummy_pixels = 0;
	IMX179MIPI_sensor.cp_dummy_lines = 0;
	IMX179MIPI_sensor.pv_dummy_pixels = 0;
	IMX179MIPI_sensor.pv_dummy_lines = 0;
	IMX179MIPI_sensor.video_dummy_pixels = 0;
	IMX179MIPI_sensor.video_dummy_lines = 0;
	IMX179MIPI_sensor.pv_line_length = IMX179MIPI_PV_LINE_LENGTH_PIXELS+IMX179MIPI_sensor.pv_dummy_pixels; 
	IMX179MIPI_sensor.pv_frame_length = IMX179MIPI_PV_FRAME_LENGTH_LINES+IMX179MIPI_sensor.pv_dummy_lines;
	spin_unlock(&imx179_drv_lock);

	IMX179MIPI_SetDummy(IMX179MIPI_sensor.pv_dummy_pixels,IMX179MIPI_sensor.pv_dummy_lines);
	IMX179MIPI_SetShutter(IMX179MIPI_sensor.pv_shutter);
	spin_lock(&imx179_drv_lock);	
	memcpy(&IMX179MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&imx179_drv_lock);
	image_window->GrabStartX= iStartX;
	image_window->GrabStartY= iStartY;
	image_window->ExposureWindowWidth= IMX179MIPI_REAL_PV_WIDTH ;
	image_window->ExposureWindowHeight= IMX179MIPI_REAL_PV_HEIGHT ; 
	mdelay(50); // Jiangde ++    
	SENSORDB("[IMX179MIPI]eXIT IMX179MIPIPreview function\n"); 
	return ERROR_NONE;
	}	/* IMX179MIPIPreview() */

/*************************************************************************
* FUNCTION
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 IMX179MIPIVideo(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint16 iStartX = 0, iStartY = 0;
	SENSORDB("[IMX179MIPI]enter IMX179MIPIVideo function\n"); 
	spin_lock(&imx179_drv_lock);    
    IMX179MIPI_MPEG4_encode_mode = KAL_TRUE;  
	IMX179MIPI_sensor.video_mode=KAL_TRUE;
	IMX179MIPI_sensor.pv_mode=KAL_FALSE;
	IMX179MIPI_sensor.capture_mode=KAL_FALSE;
	spin_unlock(&imx179_drv_lock);
    
#ifndef USE_PREVIW_FOR_VIDEO
    VideoFullSizeSetting();
#else
    VideoUsePreviewSetting(); // Jiangde
	// Video720pSetting(); // Jiangde
  	// PreviewSetting(); // Jiangde 
#endif
  	
	IMX179MIPISetFlipMirror(IMAGE_NORMAL);
	iStartX += IMX179MIPI_IMAGE_SENSOR_VIDEO_STARTX;
	iStartY += IMX179MIPI_IMAGE_SENSOR_VIDEO_STARTY;
	spin_lock(&imx179_drv_lock);	
	IMX179MIPI_sensor.cp_dummy_pixels = 0;
	IMX179MIPI_sensor.cp_dummy_lines = 0;
	IMX179MIPI_sensor.pv_dummy_pixels = 0;
	IMX179MIPI_sensor.pv_dummy_lines = 0;
	IMX179MIPI_sensor.video_dummy_pixels = 0;
	IMX179MIPI_sensor.video_dummy_lines = 0;
	IMX179MIPI_sensor.video_line_length = IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS+IMX179MIPI_sensor.video_dummy_pixels; 
	IMX179MIPI_sensor.video_frame_length = IMX179MIPI_VIDEO_FRAME_LENGTH_LINES+IMX179MIPI_sensor.video_dummy_lines;
	spin_unlock(&imx179_drv_lock);
	
	IMX179MIPI_SetDummy(IMX179MIPI_sensor.video_dummy_pixels,IMX179MIPI_sensor.video_dummy_lines);
	IMX179MIPI_SetShutter(IMX179MIPI_sensor.video_shutter);
	spin_lock(&imx179_drv_lock);	
	memcpy(&IMX179MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&imx179_drv_lock);
	image_window->GrabStartX= iStartX;
	image_window->GrabStartY= iStartY;    
	mdelay(50); // Jiangde ++    
    SENSORDB("[IMX179MIPI]eXIT IMX179MIPIVideo function\n"); 
	return ERROR_NONE;
}	/* IMX179MIPIPreview() */

UINT32 IMX179MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint16 iStartX = 0, iStartY = 0;
	SENSORDB("[IMX179MIPI]enter IMX179MIPICapture function\n");
	spin_lock(&imx179_drv_lock);	
	IMX179MIPI_sensor.video_mode=KAL_FALSE;
	IMX179MIPI_sensor.pv_mode=KAL_FALSE;
	IMX179MIPI_sensor.capture_mode=KAL_TRUE;
	IMX179MIPI_MPEG4_encode_mode = KAL_FALSE; 
	IMX179MIPI_Auto_Flicker_mode = KAL_FALSE;       
	IMX179MIPI_sensor.cp_dummy_pixels = 0;
	IMX179MIPI_sensor.cp_dummy_lines = 0;
	spin_unlock(&imx179_drv_lock);
	IMX179MIPI_set_8M();
	IMX179MIPISetFlipMirror(IMAGE_NORMAL); 
	spin_lock(&imx179_drv_lock);    
	IMX179MIPI_sensor.cp_line_length=IMX179MIPI_FULL_LINE_LENGTH_PIXELS+IMX179MIPI_sensor.cp_dummy_pixels;
	IMX179MIPI_sensor.cp_frame_length=IMX179MIPI_FULL_FRAME_LENGTH_LINES+IMX179MIPI_sensor.cp_dummy_lines;
	spin_unlock(&imx179_drv_lock);
	iStartX = IMX179MIPI_IMAGE_SENSOR_CAP_STARTX;
	iStartY = IMX179MIPI_IMAGE_SENSOR_CAP_STARTY;
	image_window->GrabStartX=iStartX;
	image_window->GrabStartY=iStartY;
	image_window->ExposureWindowWidth=IMX179MIPI_REAL_CAP_WIDTH ;
	image_window->ExposureWindowHeight=IMX179MIPI_REAL_CAP_HEIGHT;
	IMX179MIPI_SetDummy(IMX179MIPI_sensor.cp_dummy_pixels, IMX179MIPI_sensor.cp_dummy_lines);   
	spin_lock(&imx179_drv_lock);	
	memcpy(&IMX179MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&imx179_drv_lock);
	mdelay(50); // Jiangde ++    
	SENSORDB("[IMX179MIPI]exit IMX179MIPICapture function\n");
	return ERROR_NONE;
}	/* IMX179MIPICapture() */

UINT32 IMX179MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("[IMX179MIPI]eXIT IMX179MIPIGetResolution function\n");
    pSensorResolution->SensorPreviewWidth	= IMX179MIPI_REAL_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight	= IMX179MIPI_REAL_PV_HEIGHT;
    pSensorResolution->SensorFullWidth		= IMX179MIPI_REAL_CAP_WIDTH;
    pSensorResolution->SensorFullHeight		= IMX179MIPI_REAL_CAP_HEIGHT;
    pSensorResolution->SensorVideoWidth		= IMX179MIPI_REAL_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight    = IMX179MIPI_REAL_VIDEO_HEIGHT;
    SENSORDB("IMX179MIPIGetResolution :8-14");    

    return ERROR_NONE;
}   /* IMX179MIPIGetResolution() */

UINT32 IMX179MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{ 
	SENSORDB("[IMX179MIPI]enter IMX179MIPIGetInfo function\n");
	switch(ScenarioId){
			case MSDK_SCENARIO_ID_CAMERA_ZSD:
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG://hhl 2-28
				pSensorInfo->SensorFullResolutionX=IMX179MIPI_REAL_CAP_WIDTH;
				pSensorInfo->SensorFullResolutionY=IMX179MIPI_REAL_CAP_HEIGHT;
				pSensorInfo->SensorStillCaptureFrameRate=22;

			break;//hhl 2-28
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				pSensorInfo->SensorPreviewResolutionX=IMX179MIPI_REAL_VIDEO_WIDTH;
				pSensorInfo->SensorPreviewResolutionY=IMX179MIPI_REAL_VIDEO_HEIGHT;
				pSensorInfo->SensorCameraPreviewFrameRate=30;
			break;
		default:
        pSensorInfo->SensorPreviewResolutionX=IMX179MIPI_REAL_PV_WIDTH;
        pSensorInfo->SensorPreviewResolutionY=IMX179MIPI_REAL_PV_HEIGHT;
				pSensorInfo->SensorCameraPreviewFrameRate=30;
			break;
	}
    pSensorInfo->SensorVideoFrameRate=30;	
    pSensorInfo->SensorStillCaptureFrameRate=24;
    pSensorInfo->SensorWebCamCaptureFrameRate=24;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=5;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;//SENSOR_OUTPUT_FORMAT_RAW_R
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW; /*??? */
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;

    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 2; 
    pSensorInfo->VideoDelayFrame = 2; 
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;      
    pSensorInfo->AEShutDelayFrame = 0;		    /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;	
	   
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = IMX179MIPI_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = IMX179MIPI_IMAGE_SENSOR_PV_STARTY;           		
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
            break;	
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			   pSensorInfo->SensorClockFreq=24;
			   pSensorInfo->SensorClockDividCount= 5;
			   pSensorInfo->SensorClockRisingCount= 0;
			   pSensorInfo->SensorClockFallingCount= 2;
			   pSensorInfo->SensorPixelClockCount= 3;
			   pSensorInfo->SensorDataLatchCount= 2;
			   pSensorInfo->SensorGrabStartX = IMX179MIPI_IMAGE_SENSOR_VIDEO_STARTX; 
			   pSensorInfo->SensorGrabStartY = IMX179MIPI_IMAGE_SENSOR_VIDEO_STARTY;				   
			   pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;		   
			   pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			   pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			   pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			   pSensorInfo->SensorPacketECCOrder = 1;

			break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = IMX179MIPI_IMAGE_SENSOR_CAP_STARTX;	//2*IMX179MIPI_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = IMX179MIPI_IMAGE_SENSOR_CAP_STARTY;	//2*IMX179MIPI_IMAGE_SENSOR_PV_STARTY;          			
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        default:
			 pSensorInfo->SensorClockFreq=24;
			 pSensorInfo->SensorClockDividCount= 5;
			 pSensorInfo->SensorClockRisingCount= 0;
			 pSensorInfo->SensorClockFallingCount= 2;
			 pSensorInfo->SensorPixelClockCount= 3;
			 pSensorInfo->SensorDataLatchCount= 2;
			 pSensorInfo->SensorGrabStartX = IMX179MIPI_IMAGE_SENSOR_PV_STARTX; 
			 pSensorInfo->SensorGrabStartY = IMX179MIPI_IMAGE_SENSOR_PV_STARTY; 				 
			 pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;		 
			 pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		     pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		  	 pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			 pSensorInfo->SensorWidthSampling = 0;	// 0 is default 1x
			 pSensorInfo->SensorHightSampling = 0;	 // 0 is default 1x 
			 pSensorInfo->SensorPacketECCOrder = 1;

            break;
    }
	spin_lock(&imx179_drv_lock);	
    IMX179MIPIPixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &IMX179MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&imx179_drv_lock);
    SENSORDB("[IMX179MIPI]exit IMX179MIPIGetInfo function\n");
    return ERROR_NONE;
}   /* IMX179MIPIGetInfo() */


UINT32 IMX179MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{    
		spin_lock(&imx179_drv_lock);	
		CurrentScenarioId = ScenarioId;
		spin_unlock(&imx179_drv_lock);
		SENSORDB("[IMX179MIPI]enter IMX179MIPIControl function\n");
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            IMX179MIPIPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			IMX179MIPIVideo(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	    case MSDK_SCENARIO_ID_CAMERA_ZSD:
            IMX179MIPICapture(pImageWindow, pSensorConfigData);//hhl 2-28
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
	SENSORDB("[IMX179MIPI]exit IMX179MIPIControl function\n");
    return ERROR_NONE;
} /* IMX179MIPIControl() */

UINT32 IMX179MIPISetVideoMode(UINT16 u2FrameRate)
{
    SENSORDB("[IMX179MIPISetVideoMode] frame rate = %d\n", u2FrameRate);
	kal_uint16 IMX179MIPI_Video_Max_Expourse_Time = 0;
	SENSORDB("[IMX179MIPI]%s():fix_frame_rate=%d\n",__FUNCTION__,u2FrameRate);
	spin_lock(&imx179_drv_lock);
	IMX179MIPI_sensor.fix_video_fps = KAL_TRUE;
	spin_unlock(&imx179_drv_lock);
	u2FrameRate=u2FrameRate*10;//10*FPS
	SENSORDB("[IMX179MIPI][Enter Fix_fps func] IMX179MIPI_Fix_Video_Frame_Rate = %d\n", u2FrameRate/10);
	IMX179MIPI_Video_Max_Expourse_Time = (kal_uint16)((IMX179MIPI_sensor.video_pclk*10/u2FrameRate)/IMX179MIPI_sensor.video_line_length);
	
	if (IMX179MIPI_Video_Max_Expourse_Time > IMX179MIPI_VIDEO_FRAME_LENGTH_LINES/*IMX179MIPI_sensor.pv_frame_length*/) 
	{
		spin_lock(&imx179_drv_lock);    
		IMX179MIPI_sensor.video_frame_length = IMX179MIPI_Video_Max_Expourse_Time;
		IMX179MIPI_sensor.video_dummy_lines = IMX179MIPI_sensor.video_frame_length-IMX179MIPI_VIDEO_FRAME_LENGTH_LINES;
		spin_unlock(&imx179_drv_lock);
		SENSORDB("[IMX179MIPI]%s():frame_length=%d,dummy_lines=%d\n",__FUNCTION__,IMX179MIPI_sensor.video_frame_length,IMX179MIPI_sensor.video_dummy_lines);
		IMX179MIPI_SetDummy(IMX179MIPI_sensor.video_dummy_pixels,IMX179MIPI_sensor.video_dummy_lines);
	}
	spin_lock(&imx179_drv_lock);    
	IMX179MIPI_MPEG4_encode_mode = KAL_TRUE; 
	spin_unlock(&imx179_drv_lock);
	SENSORDB("[IMX179MIPI]exit IMX179MIPISetVideoMode function\n");
	return ERROR_NONE;
}

UINT32 IMX179MIPISetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	kal_uint32 pv_max_frame_rate_lines=0;

	if(IMX179MIPI_sensor.pv_mode==TRUE)
	pv_max_frame_rate_lines=IMX179MIPI_PV_FRAME_LENGTH_LINES;
	else
    pv_max_frame_rate_lines=IMX179MIPI_VIDEO_FRAME_LENGTH_LINES	;
    SENSORDB("[IMX179MIPISetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);
    if(bEnable) 
	{   // enable auto flicker   
    	spin_lock(&imx179_drv_lock);    
        IMX179MIPI_Auto_Flicker_mode = KAL_TRUE; 
		spin_unlock(&imx179_drv_lock);
        if(IMX179MIPI_MPEG4_encode_mode == KAL_TRUE) 
		{ // in the video mode, reset the frame rate
            pv_max_frame_rate_lines = IMX179MIPI_MAX_EXPOSURE_LINES + (IMX179MIPI_MAX_EXPOSURE_LINES>>7);            
            IMX179MIPI_write_cmos_sensor(0x0104, 1);        
            IMX179MIPI_write_cmos_sensor(0x0340, (pv_max_frame_rate_lines >>8) & 0xFF);
            IMX179MIPI_write_cmos_sensor(0x0341, pv_max_frame_rate_lines & 0xFF);	
            IMX179MIPI_write_cmos_sensor(0x0104, 0);        	
        }
    } 
	else 
	{
    	spin_lock(&imx179_drv_lock);    
        IMX179MIPI_Auto_Flicker_mode = KAL_FALSE; 
		spin_unlock(&imx179_drv_lock);
        if(IMX179MIPI_MPEG4_encode_mode == KAL_TRUE) 
		{    // in the video mode, restore the frame rate
            IMX179MIPI_write_cmos_sensor(0x0104, 1);        
            IMX179MIPI_write_cmos_sensor(0x0340, (IMX179MIPI_MAX_EXPOSURE_LINES >>8) & 0xFF);
            IMX179MIPI_write_cmos_sensor(0x0341, IMX179MIPI_MAX_EXPOSURE_LINES & 0xFF);	
            IMX179MIPI_write_cmos_sensor(0x0104, 0);        	
        }
        printk("Disable Auto flicker\n");    
    }
    return ERROR_NONE;
}
UINT32 IMX179MIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;	
	SENSORDB("IMX179MIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = 259200000;
			lineLength = IMX179MIPI_PV_LINE_LENGTH_PIXELS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - IMX179MIPI_PV_FRAME_LENGTH_LINES;

			
			if(dummyLine<0)
				dummyLine = 0;
			spin_lock(&imx179_drv_lock);	
			IMX179MIPI_sensor.pv_mode=TRUE;
			spin_unlock(&imx179_drv_lock);
			IMX179MIPI_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = 259200000;
			lineLength = IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - IMX179MIPI_VIDEO_FRAME_LENGTH_LINES;
			if(dummyLine<0)
				dummyLine = 0;
			spin_lock(&imx179_drv_lock);	
			IMX179MIPI_sensor.pv_mode=TRUE;
			spin_unlock(&imx179_drv_lock);
			IMX179MIPI_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = 259200000;
			lineLength = IMX179MIPI_FULL_LINE_LENGTH_PIXELS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - IMX179MIPI_FULL_FRAME_LENGTH_LINES;
			if(dummyLine<0)
				dummyLine = 0;
			
			spin_lock(&imx179_drv_lock);	
			IMX179MIPI_sensor.pv_mode=FALSE;
			spin_unlock(&imx179_drv_lock);
			IMX179MIPI_SetDummy(0, dummyLine);			
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
	SENSORDB("[IMX179MIPI]exit IMX179MIPISetMaxFramerateByScenario function\n");
	return ERROR_NONE;
}
UINT32 IMX179MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 220;
			break;		//hhl 2-28
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
UINT32 IMX179MIPISetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("[IMX179MIPISetTestPatternMode] Test pattern enable:%d\n", bEnable);
    
    if(bEnable) {   // enable color bar   
        IMX179MIPI_write_cmos_sensor(0x30D8, 0x10);  // color bar test pattern
        IMX179MIPI_write_cmos_sensor(0x0600, 0x00);  // color bar test pattern
        IMX179MIPI_write_cmos_sensor(0x0601, 0x02);  // color bar test pattern 
    } else {
        IMX179MIPI_write_cmos_sensor(0x30D8, 0x00);  // disable color bar test pattern
    }
    return ERROR_NONE;
}

UINT32 IMX179MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
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
            *pFeatureReturnPara16++=IMX179MIPI_REAL_CAP_WIDTH;
            *pFeatureReturnPara16=IMX179MIPI_REAL_CAP_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
        		switch(CurrentScenarioId)
        		{
        			case MSDK_SCENARIO_ID_CAMERA_ZSD:
        		    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
 		            *pFeatureReturnPara16++=IMX179MIPI_sensor.cp_line_length;  
 		            *pFeatureReturnPara16=IMX179MIPI_sensor.cp_frame_length;
		            SENSORDB("Sensor period:%d %d\n",IMX179MIPI_sensor.cp_line_length, IMX179MIPI_sensor.cp_frame_length); 
		            *pFeatureParaLen=4;        				
        				break;
        			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*pFeatureReturnPara16++=IMX179MIPI_sensor.video_line_length;  
					*pFeatureReturnPara16=IMX179MIPI_sensor.video_frame_length;
					 SENSORDB("Sensor period:%d %d\n", IMX179MIPI_sensor.video_line_length, IMX179MIPI_sensor.video_frame_length); 
					 *pFeatureParaLen=4;
						break;
        			default:	
					*pFeatureReturnPara16++=IMX179MIPI_sensor.pv_line_length;  
					*pFeatureReturnPara16=IMX179MIPI_sensor.pv_frame_length;
		            SENSORDB("Sensor period:%d %d\n", IMX179MIPI_sensor.pv_line_length, IMX179MIPI_sensor.pv_frame_length); 
		            *pFeatureParaLen=4;
	            break;
          	}
          	break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        		switch(CurrentScenarioId)
        		{
        			case MSDK_SCENARIO_ID_CAMERA_ZSD:
        			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		            *pFeatureReturnPara32 = IMX179MIPI_sensor.cp_pclk; 
		            *pFeatureParaLen=4;		         	
					
		            SENSORDB("Sensor CPCLK:%dn",IMX179MIPI_sensor.cp_pclk); 
		         		break; //hhl 2-28
					case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
						*pFeatureReturnPara32 = IMX179MIPI_sensor.video_pclk;
						*pFeatureParaLen=4;
						SENSORDB("Sensor videoCLK:%d\n",IMX179MIPI_sensor.video_pclk); 
						break;
		         		default:
		            *pFeatureReturnPara32 = IMX179MIPI_sensor.pv_pclk;
		            *pFeatureParaLen=4;
					SENSORDB("Sensor pvclk:%d\n",IMX179MIPI_sensor.pv_pclk); 
		            break;
		         }
		         break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            IMX179MIPI_SetShutter(*pFeatureData16); 
            break;
		case SENSOR_FEATURE_SET_SENSOR_SYNC: 
			break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            IMX179MIPI_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
           IMX179MIPI_SetGain((UINT16) *pFeatureData16); 
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			spin_lock(&imx179_drv_lock);    
            IMX179MIPI_isp_master_clock=*pFeatureData32;
			spin_unlock(&imx179_drv_lock);
            break;
        case SENSOR_FEATURE_SET_REGISTER:
			IMX179MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = IMX179MIPI_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&imx179_drv_lock);    
                IMX179MIPISensorCCT[i].Addr=*pFeatureData32++;
                IMX179MIPISensorCCT[i].Para=*pFeatureData32++; 
				spin_unlock(&imx179_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=IMX179MIPISensorCCT[i].Addr;
                *pFeatureData32++=IMX179MIPISensorCCT[i].Para; 
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {	spin_lock(&imx179_drv_lock);    
                IMX179MIPISensorReg[i].Addr=*pFeatureData32++;
                IMX179MIPISensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&imx179_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=IMX179MIPISensorReg[i].Addr;
                *pFeatureData32++=IMX179MIPISensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=IMX179MIPI_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, IMX179MIPISensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, IMX179MIPISensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &IMX179MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            IMX179MIPI_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            IMX179MIPI_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=IMX179MIPI_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            IMX179MIPI_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            IMX179MIPI_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            IMX179MIPI_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            IMX179MIPISetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            IMX179MIPIGetSensorID(pFeatureReturnPara32); 
            break;             
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            IMX179MIPISetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));            
	        break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            IMX179MIPISetTestPatternMode((BOOL)*pFeatureData16);        	
            break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			IMX179MIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			IMX179MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
        default:
            break;
    }
    return ERROR_NONE;
}	/* IMX179MIPIFeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncIMX179MIPI=
{
    IMX179MIPIOpen,
    IMX179MIPIGetInfo,
    IMX179MIPIGetResolution,
    IMX179MIPIFeatureControl,
    IMX179MIPIControl,
    IMX179MIPIClose
};

UINT32 IMX179_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncIMX179MIPI;
    return ERROR_NONE;
}   /* SensorInit() */

