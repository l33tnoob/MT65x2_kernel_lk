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
 *   sp0a19yuv_sub_Sensor.c
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:


 * ------------
 *   Image sensor driver function
 *   V1.2.3
 *
 * Author:
 * -------
 *   Leo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2012.02.29  kill bugs
 *   
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *=============================================================
 ******************************************************************************/
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
#include "kd_camera_feature.h"

#include "sp0a19yuv_Sensor.h"
#include "sp0a19yuv_Camera_Sensor_para.h"
#include "sp0a19yuv_CameraCustomized.h"


#define SP0A19YUV_DEBUG
#ifdef SP0A19YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

kal_bool   SP0A19_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 SP0A19_dummy_pixels = 0, SP0A19_dummy_lines = 0;

kal_uint32 SP0A19_isp_master_clock= 0;
static kal_uint32 SP0A19_PCLK = 26000000;

kal_uint8 sp0a19_isBanding = 0; // 0: 50hz  1:60hz
static kal_uint32 zoom_factor = 0; 

typedef enum
{
    IMAGE_SENSOR_MIRROR_NORMAL=0,
    IMAGE_SENSOR_MIRROR_H,
    IMAGE_SENSOR_MIRROR_V,
    IMAGE_SENSOR_MIRROR_HV,
} IMAGE_SENSOR_MIRROR_ENUM;

//extern kal_bool searchMainSensor;

MSDK_SENSOR_CONFIG_STRUCT SP0A19SensorConfigData;
MSDK_SCENARIO_ID_ENUM SP0A19CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

#define SP0A19_SET_PAGE0 	SP0A19_write_cmos_sensor(0xfd, 0x00)
#define SP0A19_SET_PAGE1 	SP0A19_write_cmos_sensor(0xfd, 0x01)


//#define SP0A19_LOAD_FROM_T_FLASH
#ifdef SP0A19_LOAD_FROM_T_FLASH

kal_uint8 fromsd = 0;
kal_uint16 SP0A19_write_cmos_sensor(kal_uint8 addr, kal_uint8 para);

#define SP2528_OP_CODE_INI		0x00		/* Initial value. */
#define SP2528_OP_CODE_REG		0x01		/* Register */
#define SP2528_OP_CODE_DLY		0x02		/* Delay */
#define SP2528_OP_CODE_END		0x03		/* End of initial setting. */

	typedef struct
	{
		u16 init_reg;
		u16 init_val;	/* Save the register value and delay tick */
		u8 op_code;		/* 0 - Initial value, 1 - Register, 2 - Delay, 3 - End of setting. */
	} SP2528_initial_set_struct;

	SP2528_initial_set_struct SP2528_Init_Reg[1000];
	
	u32 strtol(const char *nptr, u8 base)
	{
		u8 ret;
		if(!nptr || (base!=16 && base!=10 && base!=8))
		{
			printk("%s(): NULL pointer input\n", __FUNCTION__);
			return -1;
		}
		for(ret=0; *nptr; nptr++)
		{
			if((base==16 && *nptr>='A' && *nptr<='F') || 
			(base==16 && *nptr>='a' && *nptr<='f') || 
			(base>=10 && *nptr>='0' && *nptr<='9') ||
			(base>=8 && *nptr>='0' && *nptr<='7') )
			{
				ret *= base;
				if(base==16 && *nptr>='A' && *nptr<='F')
				ret += *nptr-'A'+10;
				else if(base==16 && *nptr>='a' && *nptr<='f')
				ret += *nptr-'a'+10;
				else if(base>=10 && *nptr>='0' && *nptr<='9')
				ret += *nptr-'0';
				else if(base>=8 && *nptr>='0' && *nptr<='7')
				ret += *nptr-'0';
			}
			else
				return ret;
		}
		return ret;
	}

	u8 SP2528_Initialize_from_T_Flash()
	{
		//FS_HANDLE fp = -1;				/* Default, no file opened. */
		//u8 *data_buff = NULL;
		u8 *curr_ptr = NULL;
		u32 file_size = 0;
		//u32 bytes_read = 0;
		u32 i = 0, j = 0;
		u8 func_ind[4] = {0};	/* REG or DLY */


		struct file *fp; 
		mm_segment_t fs; 
		loff_t pos = 0; 
		static u8 data_buff[10*1024] ;

		fp = filp_open("/storage/sdcard0/sp0a19_sd.dat", O_RDONLY , 0); 
		if (IS_ERR(fp)) { 
			printk("create file error\n"); 
			return 2;//-1; 
		} 
		else
		printk("SP2528_Initialize_from_T_Flash Open File Success\n");

		fs = get_fs(); 
		set_fs(KERNEL_DS); 

		file_size = vfs_llseek(fp, 0, SEEK_END);
		vfs_read(fp, data_buff, file_size, &pos); 
		filp_close(fp, NULL); 
		set_fs(fs);

		printk("1\n");

		/* Start parse the setting witch read from t-flash. */
		curr_ptr = data_buff;
		while (curr_ptr < (data_buff + file_size))
		{
			while ((*curr_ptr == ' ') || (*curr_ptr == '\t'))/* Skip the Space & TAB */
			curr_ptr++;				

			if (((*curr_ptr) == '/') && ((*(curr_ptr + 1)) == '*'))
			{
				while (!(((*curr_ptr) == '*') && ((*(curr_ptr + 1)) == '/')))
				{
					curr_ptr++;		/* Skip block comment code. */
				}

				while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
				{
					curr_ptr++;
				}

				curr_ptr += 2;						/* Skip the enter line */

				continue ;
			}

			if (((*curr_ptr) == '/') || ((*curr_ptr) == '{') || ((*curr_ptr) == '}'))		/* Comment line, skip it. */
			{
				while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
				{
					curr_ptr++;
				}

				curr_ptr += 2;						/* Skip the enter line */

				continue ;
			}
			/* This just content one enter line. */
			if (((*curr_ptr) == 0x0D) && ((*(curr_ptr + 1)) == 0x0A))
			{
				curr_ptr += 2;
				continue ;
			}
			//printk(" curr_ptr1 = %s\n",curr_ptr);
			memcpy(func_ind, curr_ptr, 3);

			if (strcmp((const char *)func_ind, "REG") == 0)		/* REG */
			{
				curr_ptr += 6;				/* Skip "REG(0x" or "DLY(" */
				SP2528_Init_Reg[i].op_code = SP2528_OP_CODE_REG;

				SP2528_Init_Reg[i].init_reg = strtol((const char *)curr_ptr, 16);
				curr_ptr += 5;	/* Skip "00, 0x" */

				SP2528_Init_Reg[i].init_val = strtol((const char *)curr_ptr, 16);
				curr_ptr += 4;	/* Skip "00);" */
			}
			else									/* DLY */
			{
				/* Need add delay for this setting. */
				curr_ptr += 4;	
				SP2528_Init_Reg[i].op_code = SP2528_OP_CODE_DLY;

				SP2528_Init_Reg[i].init_reg = 0xFF;
				SP2528_Init_Reg[i].init_val = strtol((const char *)curr_ptr,  10);	/* Get the delay ticks, the delay should less then 50 */
			}
			i++;

			/* Skip to next line directly. */
			while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
			{
				curr_ptr++;
			}
			curr_ptr += 2;
		}
		printk("2\n");
		/* (0xFFFF, 0xFFFF) means the end of initial setting. */
		SP2528_Init_Reg[i].op_code = SP2528_OP_CODE_END;
		SP2528_Init_Reg[i].init_reg = 0xFF;
		SP2528_Init_Reg[i].init_val = 0xFF;
		i++;
		//for (j=0; j<i; j++)
		//printk(" %x  ==  %x\n",SP2528_Init_Reg[j].init_reg, SP2528_Init_Reg[j].init_val);

		/* Start apply the initial setting to sensor. */
#if 1
		for (j=0; j<i; j++)
		{
			if (SP2528_Init_Reg[j].op_code == SP2528_OP_CODE_END)	/* End of the setting. */
			{
				break ;
			}
			else if (SP2528_Init_Reg[j].op_code == SP2528_OP_CODE_DLY)
			{
				msleep(SP2528_Init_Reg[j].init_val);		/* Delay */
			}
			else if (SP2528_Init_Reg[j].op_code == SP2528_OP_CODE_REG)
			{
				SP0A19_write_cmos_sensor((kal_uint8)SP2528_Init_Reg[j].init_reg, (kal_uint8)SP2528_Init_Reg[j].init_val);
			}
			else
			{
				printk("REG ERROR!\n");
			}
		}
#endif
		printk("3\n");
		return 1;	
	}

#endif


extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 SP0A19_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 2, SP0A19_WRITE_ID);
	

}
kal_uint16 SP0A19_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte, 1, SP0A19_WRITE_ID);
	
    return get_byte;
}


static void SP0A19_InitialPara(void)
{
  SP0A19_CurrentStatus.iNightMode = KAL_FALSE;
  SP0A19_CurrentStatus.iWB = AWB_MODE_AUTO;
  SP0A19_CurrentStatus.iEffect = MEFFECT_OFF;
  SP0A19_CurrentStatus.iBanding = AE_FLICKER_MODE_50HZ;
  SP0A19_CurrentStatus.iEV = AE_EV_COMP_00;
  SP0A19_CurrentStatus.iMirror = IMAGE_NORMAL;
  SP0A19_CurrentStatus.iFrameRate = 0;
  SP0A19_CurrentStatus.iBrightness =ISP_BRIGHT_MIDDLE;
  SP0A19_CurrentStatus.iIso =AE_ISO_AUTO;

  sp0a19_isBanding = 0;
  
}



/*************************************************************************
 * FUNCTION
 *	SP0A19_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of SP0A19 to change exposure time.
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
void SP0A19_Set_Shutter(kal_uint16 iShutter)
{
    
} 


/*************************************************************************
 * FUNCTION
 *	SP0A19_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of SP0A19 .
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 SP0A19_Read_Shutter(void)
{
    kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	SP0A19_write_cmos_sensor(0xfd, 0x00);
	temp_reg1 = SP0A19_read_cmos_sensor(0x04);
	temp_reg2 = SP0A19_read_cmos_sensor(0x03);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	//SENSORDB("SP0A19_Read_Shutter shutter=%d \n",shutter);

	return shutter;
} /* SP0A19_read_shutter */


/*************************************************************************
 * FUNCTION
 *	SP0A19_write_reg
 *
 * DESCRIPTION
 *	This function set the register of SP0A19.
 *
 * PARAMETERS
 *	addr : the register index of SP0A19
 *  para : setting parameter of the specified register of SP0A19
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void SP0A19_write_reg(kal_uint32 addr, kal_uint32 para)
{
	SP0A19_write_cmos_sensor(addr, para);
}


/*************************************************************************
 * FUNCTION
 *	SP0A19_read_reg
 *
 * DESCRIPTION
 *	This function read parameter of specified register from SP0A19.
 *
 * PARAMETERS
 *	addr : the register index of SP0A19
 *
 * RETURNS
 *	the data that read from SP0A19
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 SP0A19_read_reg(kal_uint32 addr)
{
	return SP0A19_read_cmos_sensor(addr);
}



/*************************************************************************
* FUNCTION
*	SP0A19_awb_enable
*
* DESCRIPTION
*	This function enable or disable the awb (Auto White Balance).
*
* PARAMETERS
*	1. kal_bool : KAL_TRUE - enable awb, KAL_FALSE - disable awb.
*
* RETURNS
*	kal_bool : It means set awb right or not.
*
*************************************************************************/
#if 0
static void SP0A19_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;

	SP0A19_write_cmos_sensor(0xfd, 0x00);
	temp_AWB_reg = SP0A19_read_cmos_sensor(0x32);
	
	if (enalbe)
	{
		SP0A19_write_cmos_sensor(0x32, (temp_AWB_reg |0x08));
	}
	else
	{
		SP0A19_write_cmos_sensor(0x32, (temp_AWB_reg & (~0x08)));
	}

}
#endif



/*************************************************************************
 * FUNCTION
 *	SP0A19_NightMode
 *
 * DESCRIPTION
 *	This function night mode of SP0A19.
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
BOOL SP0A19_set_param_wb(UINT16 para);
void SP0A19NightMode(kal_bool bEnable)
{
	SP0A19_CurrentStatus.iNightMode = bEnable;

	#ifdef SP0A19_LOAD_FROM_T_FLASH
	return;
	#endif

	if(bEnable)//night mode
	{ 
	   SENSORDB("[Enter] SP0A19NightMode!\n");
	   SP0A19_set_param_wb(AWB_MODE_AUTO);//Lock auto AWB in night mode.
	   SP0A19_write_cmos_sensor(0xfd,0x00);
	   SP0A19_write_cmos_sensor(0xb2,0x25);
	   SP0A19_write_cmos_sensor(0xb3,0x1f);
					
	   if(SP0A19_MPEG4_encode_mode == KAL_TRUE)
		{
			if(sp0a19_isBanding== 0)
			{
				//SI13_SP0A19 26M 1分频 50Hz 8.0392-8fps
				//ae setting
				SENSORDB(" video 50Hz night\r\n");
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0x7b);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x09);
				SP0A19_write_cmos_sensor(0x0a,0x11);
				SP0A19_write_cmos_sensor(0xf0,0x29);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x0c);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x29);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status   
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0xec);
				SP0A19_write_cmos_sensor(0xcf,0x01);
				SP0A19_write_cmos_sensor(0xd0,0xec);
				SP0A19_write_cmos_sensor(0xd1,0x01);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}
			else if(sp0a19_isBanding == 1)
			{
				//SI13_SP0A19 26M 1分频 60Hz 8-8fps
				//ae setting
				SENSORDB(" video 60Hz night\r\n");
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0x66);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x09);
				SP0A19_write_cmos_sensor(0x0a,0x20);
				SP0A19_write_cmos_sensor(0xf0,0x22);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x0f);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x22);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status 
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0xfe);
				SP0A19_write_cmos_sensor(0xcf,0x01);
				SP0A19_write_cmos_sensor(0xd0,0xfe);
				SP0A19_write_cmos_sensor(0xd1,0x01);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}
		}	
		else 
		{  
			if(sp0a19_isBanding== 0)
			{
				//SI13_SP0A19 26M 1分频 50Hz 15.098-5fps
				//ae setting
				SENSORDB(" priview 50Hz night\r\n");    
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0xe7);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x03);
				SP0A19_write_cmos_sensor(0x0a,0x46);
				SP0A19_write_cmos_sensor(0xf0,0x4d);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x14);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x4d);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status   
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0x04);
				SP0A19_write_cmos_sensor(0xcf,0x06);
				SP0A19_write_cmos_sensor(0xd0,0x04);
				SP0A19_write_cmos_sensor(0xd1,0x06);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}  
			else if(sp0a19_isBanding== 1)
			{
				//SI13_SP0A19 26M 1分频 60Hz 15.0588-5fps
				//ae setting
				SENSORDB(" priview 60Hz night\r\n");	
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0xc0);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x03);
				SP0A19_write_cmos_sensor(0x0a,0x4b);
				SP0A19_write_cmos_sensor(0xf0,0x40);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x18);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x40);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status   
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0x00);
				SP0A19_write_cmos_sensor(0xcf,0x06);
				SP0A19_write_cmos_sensor(0xd0,0x00);
				SP0A19_write_cmos_sensor(0xd1,0x06);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}
		} 		
	}
	else    // daylight mode
	{

		SP0A19_write_cmos_sensor(0xfd,0x00);
		SP0A19_write_cmos_sensor(0xb2,0x20);
		SP0A19_write_cmos_sensor(0xb3,0x1f);

		if(SP0A19_MPEG4_encode_mode == KAL_TRUE)
		{
			if(sp0a19_isBanding== 0)
			{
				//SI13_SP0A19 26M 1分频 50Hz 15.098-15fps
				//ae setting
				SENSORDB(" video 50Hz normal\r\n");				
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0xe7);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x03);
				SP0A19_write_cmos_sensor(0x0a,0x46);
				SP0A19_write_cmos_sensor(0xf0,0x4d);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x06);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x4d);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status   
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0xce);
				SP0A19_write_cmos_sensor(0xcf,0x01);
				SP0A19_write_cmos_sensor(0xd0,0xce);
				SP0A19_write_cmos_sensor(0xd1,0x01);
				SP0A19_write_cmos_sensor(0xfd,0x00);

			}
			else if(sp0a19_isBanding == 1)
			{
				//SI13_SP0A19 26M 1分频 60Hz 15.0588-15fps
				//ae setting
				SENSORDB(" video 60Hz normal \n");	
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0xc0);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x03);
				SP0A19_write_cmos_sensor(0x0a,0x4b);
				SP0A19_write_cmos_sensor(0xf0,0x40);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x08);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x40);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status  
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0x00);
				SP0A19_write_cmos_sensor(0xcf,0x02);
				SP0A19_write_cmos_sensor(0xd0,0x00);
				SP0A19_write_cmos_sensor(0xd1,0x02);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}
		}
		else 
		{
			if(sp0a19_isBanding== 0)
			{
				//SI13_SP0A19 26M 1分频 50Hz 15.098-7fps
				//ae setting
				SENSORDB(" priview 50Hz normal\r\n");
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0xe7);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x03);
				SP0A19_write_cmos_sensor(0x0a,0x46);
				SP0A19_write_cmos_sensor(0xf0,0x4d);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x0e);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x4d);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status 
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0x36);
				SP0A19_write_cmos_sensor(0xcf,0x04);
				SP0A19_write_cmos_sensor(0xd0,0x36);
				SP0A19_write_cmos_sensor(0xd1,0x04);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}
			else if(sp0a19_isBanding== 1)
			{
				//SI13_SP0A19 26M 1分频 60Hz 15.0588-7fps
				//ae setting
				SENSORDB(" priview 60Hz normal\r\n");
				SP0A19_write_cmos_sensor(0xfd,0x00);
				SP0A19_write_cmos_sensor(0x03,0x00);
				SP0A19_write_cmos_sensor(0x04,0xc0);
				SP0A19_write_cmos_sensor(0x06,0x00);
				SP0A19_write_cmos_sensor(0x09,0x03);
				SP0A19_write_cmos_sensor(0x0a,0x4b);
				SP0A19_write_cmos_sensor(0xf0,0x40);
				SP0A19_write_cmos_sensor(0xf1,0x00);
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0x90,0x11);
				SP0A19_write_cmos_sensor(0x92,0x01);
				SP0A19_write_cmos_sensor(0x98,0x40);
				SP0A19_write_cmos_sensor(0x99,0x00);
				SP0A19_write_cmos_sensor(0x9a,0x01);
				SP0A19_write_cmos_sensor(0x9b,0x00);
				//Status   
				SP0A19_write_cmos_sensor(0xfd,0x01);
				SP0A19_write_cmos_sensor(0xce,0x40);
				SP0A19_write_cmos_sensor(0xcf,0x04);
				SP0A19_write_cmos_sensor(0xd0,0x40);
				SP0A19_write_cmos_sensor(0xd1,0x04);
				SP0A19_write_cmos_sensor(0xfd,0x00);
			}
		}
	}  
	SP0A19_CurrentStatus.iNightMode = bEnable;
}

/*************************************************************************
* FUNCTION
*	SP0A19_Sensor_Init
*
* DESCRIPTION
*	This function apply all of the initial setting to sensor.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
*************************************************************************/
void SP0A19_Sensor_Init(void)
{  
	SENSORDB("SP0A19_Sensor_Init enter \n");
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	SP0A19_write_cmos_sensor(0x1C,0x28);
	SP0A19_write_cmos_sensor(0x32,0x00);
	SP0A19_write_cmos_sensor(0x0f,0x2f);
	SP0A19_write_cmos_sensor(0x10,0x2e);
	SP0A19_write_cmos_sensor(0x11,0x00);
	SP0A19_write_cmos_sensor(0x12,0x18);
	SP0A19_write_cmos_sensor(0x13,0x2f);
	SP0A19_write_cmos_sensor(0x14,0x00);
	SP0A19_write_cmos_sensor(0x15,0x3f);
	SP0A19_write_cmos_sensor(0x16,0x00);
	SP0A19_write_cmos_sensor(0x17,0x18);
	SP0A19_write_cmos_sensor(0x25,0x40);
	SP0A19_write_cmos_sensor(0x1a,0x0b);
	SP0A19_write_cmos_sensor(0x1b,0x0c);
	SP0A19_write_cmos_sensor(0x1e,0x0b);
	SP0A19_write_cmos_sensor(0x20,0x3f);
	SP0A19_write_cmos_sensor(0x21,0x13);
	SP0A19_write_cmos_sensor(0x22,0x19);
	SP0A19_write_cmos_sensor(0x26,0x1a);
	SP0A19_write_cmos_sensor(0x27,0xab);
	SP0A19_write_cmos_sensor(0x28,0xfd);
	SP0A19_write_cmos_sensor(0x30,0x00);
	SP0A19_write_cmos_sensor(0x31,0x00);
	SP0A19_write_cmos_sensor(0xfb,0x31);
	SP0A19_write_cmos_sensor(0x1f,0x08);
	
	//Blacklevel
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0x65,0x00);
	SP0A19_write_cmos_sensor(0x66,0x00);
	SP0A19_write_cmos_sensor(0x67,0x00);
	SP0A19_write_cmos_sensor(0x68,0x00);
	SP0A19_write_cmos_sensor(0x45,0x00);
	SP0A19_write_cmos_sensor(0x46,0x0f);
	
	//SI13_SP0A19 26M 1分频 50Hz 15.098-7fps
	//ae setting
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0x03,0x00);
	SP0A19_write_cmos_sensor(0x04,0xe7);
	SP0A19_write_cmos_sensor(0x06,0x00);
	SP0A19_write_cmos_sensor(0x09,0x03);
	SP0A19_write_cmos_sensor(0x0a,0x46);
	SP0A19_write_cmos_sensor(0xf0,0x4d);
	SP0A19_write_cmos_sensor(0xf1,0x00);
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x90,0x0e);
	SP0A19_write_cmos_sensor(0x92,0x01);
	SP0A19_write_cmos_sensor(0x98,0x4d);
	SP0A19_write_cmos_sensor(0x99,0x00);
	SP0A19_write_cmos_sensor(0x9a,0x01);
	SP0A19_write_cmos_sensor(0x9b,0x00);
	//Status 
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0xce,0x36);
	SP0A19_write_cmos_sensor(0xcf,0x04);
	SP0A19_write_cmos_sensor(0xd0,0x36);
	SP0A19_write_cmos_sensor(0xd1,0x04);
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0xc4,0x56);
	SP0A19_write_cmos_sensor(0xc5,0x7a);
	SP0A19_write_cmos_sensor(0xca,0x30);
	SP0A19_write_cmos_sensor(0xcb,0x45);
	SP0A19_write_cmos_sensor(0xcc,0xb0);
	SP0A19_write_cmos_sensor(0xcd,0x48);
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	//lsc  for st 
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x35,0x15);
	SP0A19_write_cmos_sensor(0x36,0x15);
	SP0A19_write_cmos_sensor(0x37,0x15);
	SP0A19_write_cmos_sensor(0x38,0x15);
	SP0A19_write_cmos_sensor(0x39,0x15);
	SP0A19_write_cmos_sensor(0x3a,0x15);
	SP0A19_write_cmos_sensor(0x3b,0x13);
	SP0A19_write_cmos_sensor(0x3c,0x15);
	SP0A19_write_cmos_sensor(0x3d,0x15);
	SP0A19_write_cmos_sensor(0x3e,0x15);
	SP0A19_write_cmos_sensor(0x3f,0x15);
	SP0A19_write_cmos_sensor(0x40,0x18);
	SP0A19_write_cmos_sensor(0x41,0x00);
	SP0A19_write_cmos_sensor(0x42,0x04);
	SP0A19_write_cmos_sensor(0x43,0x04);
	SP0A19_write_cmos_sensor(0x44,0x00);
	SP0A19_write_cmos_sensor(0x45,0x00);
	SP0A19_write_cmos_sensor(0x46,0x00);
	SP0A19_write_cmos_sensor(0x47,0x00);
	SP0A19_write_cmos_sensor(0x48,0x00);
	SP0A19_write_cmos_sensor(0x49,0xfd);
	SP0A19_write_cmos_sensor(0x4a,0x00);
	SP0A19_write_cmos_sensor(0x4b,0x00);
	SP0A19_write_cmos_sensor(0x4c,0xfd);
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	//awb 1
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x28,0xc9);
	SP0A19_write_cmos_sensor(0x29,0x98);
	
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0xe7,0x03);
	SP0A19_write_cmos_sensor(0xe7,0x00);	
	
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x2e,0x01);
	SP0A19_write_cmos_sensor(0x2f,0x08);
	SP0A19_write_cmos_sensor(0x17,0x17);
	SP0A19_write_cmos_sensor(0x18,0x19);
	SP0A19_write_cmos_sensor(0x19,0x45);
	
	
	SP0A19_write_cmos_sensor(0x2a,0xef);
	SP0A19_write_cmos_sensor(0x2b,0x15);
	
	//awb2
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x73,0x80);
	SP0A19_write_cmos_sensor(0x1a,0x80);
	SP0A19_write_cmos_sensor(0x1b,0x8c);
	//d65
	SP0A19_write_cmos_sensor(0x65,0xd1);
	SP0A19_write_cmos_sensor(0x66,0xe7);
	SP0A19_write_cmos_sensor(0x67,0x7f);
	SP0A19_write_cmos_sensor(0x68,0x94);
	//indoor 
	SP0A19_write_cmos_sensor(0x69,0xbb);
	SP0A19_write_cmos_sensor(0x6a,0xdd);
	SP0A19_write_cmos_sensor(0x6b,0x96);
	SP0A19_write_cmos_sensor(0x6c,0xbb);
	//f 
	SP0A19_write_cmos_sensor(0x61,0x7b);
	SP0A19_write_cmos_sensor(0x62,0xa1);
	SP0A19_write_cmos_sensor(0x63,0xbb);
	SP0A19_write_cmos_sensor(0x64,0xe3);
	//cwf
	SP0A19_write_cmos_sensor(0x6d,0xad);
	SP0A19_write_cmos_sensor(0x6e,0xd1);
	SP0A19_write_cmos_sensor(0x6f,0xb2);
	SP0A19_write_cmos_sensor(0x70,0xd5);
	
	//skin detect
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x08,0x15);
	SP0A19_write_cmos_sensor(0x09,0x01);
	SP0A19_write_cmos_sensor(0x0a,0x20);
	SP0A19_write_cmos_sensor(0x0b,0x12);
	SP0A19_write_cmos_sensor(0x0c,0x27);
	SP0A19_write_cmos_sensor(0x0d,0x06);
	SP0A19_write_cmos_sensor(0x0f,0x63);
	
	//BPC_grad
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0x79,0xf0);
	SP0A19_write_cmos_sensor(0x7a,0x80);
	SP0A19_write_cmos_sensor(0x7b,0x80);
	SP0A19_write_cmos_sensor(0x7c,0x20);
	
	//smooth
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	//
	SP0A19_write_cmos_sensor(0x57,0x08);
	SP0A19_write_cmos_sensor(0x58,0x0c);
	SP0A19_write_cmos_sensor(0x56,0x0e);
	SP0A19_write_cmos_sensor(0x59,0x12);
	//GrGb
	SP0A19_write_cmos_sensor(0x89,0x08);
	SP0A19_write_cmos_sensor(0x8a,0x0c);
	SP0A19_write_cmos_sensor(0x9c,0x0e);
	SP0A19_write_cmos_sensor(0x9d,0x12);
	//Gr\Gb
	SP0A19_write_cmos_sensor(0x81,0xe0);
	SP0A19_write_cmos_sensor(0x82,0x80);
	SP0A19_write_cmos_sensor(0x83,0x40);
	SP0A19_write_cmos_sensor(0x84,0x20);
	//GrGb
	SP0A19_write_cmos_sensor(0x85,0xe0);
	SP0A19_write_cmos_sensor(0x86,0x80);
	SP0A19_write_cmos_sensor(0x87,0x40);
	SP0A19_write_cmos_sensor(0x88,0x20);
	//
	SP0A19_write_cmos_sensor(0x5a,0xff);
	SP0A19_write_cmos_sensor(0x5b,0xe0);
	SP0A19_write_cmos_sensor(0x5c,0x80);
	SP0A19_write_cmos_sensor(0x5d,0x20);
	
	//sharpen 
	SP0A19_write_cmos_sensor(0xfd,0x01);
	
	SP0A19_write_cmos_sensor(0xe2,0x50);
	SP0A19_write_cmos_sensor(0xe4,0xa0);
	
	SP0A19_write_cmos_sensor(0xe5,0x08);
	SP0A19_write_cmos_sensor(0xd3,0x08);
	SP0A19_write_cmos_sensor(0xd7,0x08);
	
	SP0A19_write_cmos_sensor(0xe6,0x0a);
	SP0A19_write_cmos_sensor(0xd4,0x0a);
	SP0A19_write_cmos_sensor(0xd8,0x0a);
	
	SP0A19_write_cmos_sensor(0xe7,0x12);
	SP0A19_write_cmos_sensor(0xd5,0x12);
	SP0A19_write_cmos_sensor(0xd9,0x12);
	
	SP0A19_write_cmos_sensor(0xd2,0x15);
	SP0A19_write_cmos_sensor(0xd6,0x15);
	SP0A19_write_cmos_sensor(0xda,0x15);
	
	SP0A19_write_cmos_sensor(0xe8,0x20);
	SP0A19_write_cmos_sensor(0xec,0x2c);
	
	SP0A19_write_cmos_sensor(0xe9,0x20);
	SP0A19_write_cmos_sensor(0xed,0x2c);
	
	SP0A19_write_cmos_sensor(0xea,0x18);
	SP0A19_write_cmos_sensor(0xef,0x1c);
	
	SP0A19_write_cmos_sensor(0xeb,0x15);
	SP0A19_write_cmos_sensor(0xf0,0x18);
	
	//CCM
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0xa0,0x80);
	SP0A19_write_cmos_sensor(0xa1,0x0 );
	SP0A19_write_cmos_sensor(0xa2,0x0 );
	SP0A19_write_cmos_sensor(0xa3,0xf3);
	SP0A19_write_cmos_sensor(0xa4,0x8e);
	SP0A19_write_cmos_sensor(0xa5,0x0 );
	SP0A19_write_cmos_sensor(0xa6,0x0 );
	SP0A19_write_cmos_sensor(0xa7,0xe6);
	SP0A19_write_cmos_sensor(0xa8,0x9a);
	SP0A19_write_cmos_sensor(0xa9,0x0 );
	SP0A19_write_cmos_sensor(0xaa,0x3 );
	SP0A19_write_cmos_sensor(0xab,0xc );
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	//gamma  
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0x8b,0x0 );
	SP0A19_write_cmos_sensor(0x8c,0xC );
	SP0A19_write_cmos_sensor(0x8d,0x19);
	SP0A19_write_cmos_sensor(0x8e,0x2C);
	SP0A19_write_cmos_sensor(0x8f,0x49);
	SP0A19_write_cmos_sensor(0x90,0x61);
	SP0A19_write_cmos_sensor(0x91,0x77);
	SP0A19_write_cmos_sensor(0x92,0x8A);
	SP0A19_write_cmos_sensor(0x93,0x9B);
	SP0A19_write_cmos_sensor(0x94,0xA9);
	SP0A19_write_cmos_sensor(0x95,0xB5);
	SP0A19_write_cmos_sensor(0x96,0xC0);
	SP0A19_write_cmos_sensor(0x97,0xCA);
	SP0A19_write_cmos_sensor(0x98,0xD4);
	SP0A19_write_cmos_sensor(0x99,0xDD);
	SP0A19_write_cmos_sensor(0x9a,0xE6);
	SP0A19_write_cmos_sensor(0x9b,0xEF);
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x8d,0xF7);
	SP0A19_write_cmos_sensor(0x8e,0xFF);
	SP0A19_write_cmos_sensor(0xfd,0x00);
	
	//rpc
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0xe0,0x4c);
	SP0A19_write_cmos_sensor(0xe1,0x3c);
	SP0A19_write_cmos_sensor(0xe2,0x34);
	SP0A19_write_cmos_sensor(0xe3,0x2e);


	SP0A19_write_cmos_sensor(0xe4,0x2e);
	SP0A19_write_cmos_sensor(0xe5,0x2c);
	SP0A19_write_cmos_sensor(0xe6,0x2c);
	SP0A19_write_cmos_sensor(0xe8,0x2a);
	SP0A19_write_cmos_sensor(0xe9,0x2a);
	SP0A19_write_cmos_sensor(0xea,0x2a);
	SP0A19_write_cmos_sensor(0xeb,0x28);
	SP0A19_write_cmos_sensor(0xf5,0x28);
	SP0A19_write_cmos_sensor(0xf6,0x28);
	
	//ae min gain  
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x94,0xa0);
	SP0A19_write_cmos_sensor(0x95,0x28);
	SP0A19_write_cmos_sensor(0x9c,0xa0);
	SP0A19_write_cmos_sensor(0x9d,0x28);
	//ae target
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0xed,0x90);
	SP0A19_write_cmos_sensor(0xf7,0x8c);
	SP0A19_write_cmos_sensor(0xf8,0x84);
	SP0A19_write_cmos_sensor(0xec,0x80);
	
	SP0A19_write_cmos_sensor(0xef,0x7c);
	SP0A19_write_cmos_sensor(0xf9,0x78);
	SP0A19_write_cmos_sensor(0xfa,0x70);
	SP0A19_write_cmos_sensor(0xee,0x6c);
	
	
	//gray detect
	SP0A19_write_cmos_sensor(0xfd,0x01);
	SP0A19_write_cmos_sensor(0x30,0x40);
	//add 0813 
	SP0A19_write_cmos_sensor(0x31,0x70);
	
	SP0A19_write_cmos_sensor(0x32,0x40);
	SP0A19_write_cmos_sensor(0x33,0xef);
	SP0A19_write_cmos_sensor(0x34,0x05);
	SP0A19_write_cmos_sensor(0x4d,0x2f);
	SP0A19_write_cmos_sensor(0x4e,0x20);
	SP0A19_write_cmos_sensor(0x4f,0x16);
	
	//lowlight lum
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0xb2,0x20);
	SP0A19_write_cmos_sensor(0xb3,0x1f);
	SP0A19_write_cmos_sensor(0xb4,0x70);
	SP0A19_write_cmos_sensor(0xb5,0x8c);
	
	//saturation
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0xbe,0xff);
	SP0A19_write_cmos_sensor(0xbf,0x01);
	SP0A19_write_cmos_sensor(0xc0,0xff);
	SP0A19_write_cmos_sensor(0xc1,0xd8);
	SP0A19_write_cmos_sensor(0xd3,0x80);
	SP0A19_write_cmos_sensor(0xd4,0x80);
	SP0A19_write_cmos_sensor(0xd6,0x78);
	SP0A19_write_cmos_sensor(0xd7,0x6c);
	
	//HEQ
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0xdc,0x00);
	SP0A19_write_cmos_sensor(0xdd,0x7c);
	SP0A19_write_cmos_sensor(0xde,0xaa);
	SP0A19_write_cmos_sensor(0xdf,0x80);
	
	//func enable
	SP0A19_write_cmos_sensor(0xfd,0x00);
	SP0A19_write_cmos_sensor(0x32,0x15);
	SP0A19_write_cmos_sensor(0x34,0x76);
	SP0A19_write_cmos_sensor(0x35,0x00);
	SP0A19_write_cmos_sensor(0x33,0xef);
	SP0A19_write_cmos_sensor(0x5f,0x51);
	SP0A19_write_cmos_sensor(0xf4,0x19);
}



/*************************************************************************
* FUNCTION
*	SP0A19_GAMMA_Select
*
* DESCRIPTION
*	This function is served for FAE to select the appropriate GAMMA curve.
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
UINT32 SP0A19GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;

	SENSORDB("SP0A19GetSensorID \n");	

#if 0 //specical config for SP0A19,other sensor don't follow this 
	if(searchMainSensor == KAL_TRUE)
	{
		*sensorID=0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
#endif


	SP0A19_write_cmos_sensor(0xfd,0x00);
	sensor_id=SP0A19_read_cmos_sensor(0x02);
	SENSORDB("Sensor Read SP0A19GetSensorID: %x \r\n",sensor_id);

	*sensorID=sensor_id;
	if (sensor_id != SP0A19_SENSOR_ID)
	{
		*sensorID=0xFFFFFFFF;
		SENSORDB("Sensor Read ByeBye \r\n");
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;	  
}


/*************************************************************************
 * FUNCTION
 *	SP0A19Open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
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
UINT32 SP0A19Open(void)
{
    kal_uint16 sensor_id=0;

    SENSORDB("SP0A19Open \n");

		
    SP0A19_write_cmos_sensor(0xfd,0x00);
    sensor_id=SP0A19_read_cmos_sensor(0x02);
    SENSORDB("SP0A19Open Sensor id = %x\n", sensor_id);
	
    if (sensor_id != SP0A19_SENSOR_ID) 
    {
        SENSORDB("Sensor Read ByeBye \r\n");
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    mdelay(10);

#ifdef SP0A19_LOAD_FROM_T_FLASH  //gepeiwei   120903
    //判断手机对应目录下是否有名为sp2528_sd 的文件,没有默认参数

    //介于各种原因，本版本初始化参数在_s_fmt中。
    struct file *fp; 
    mm_segment_t fs; 
    loff_t pos = 0; 
    //static char buf[10*1024] ;

    printk("SP0A19 Open File Start\n");
    fp = filp_open("/storage/sdcard0/sp2528_sd.dat", O_RDONLY , 0); 
    if (IS_ERR(fp)) 
    { 
        fromsd = 0;   
        printk("open file error\n");
        ////////return 0;
    } 
    else 
    {
        printk("open file success\n");
        fromsd = 1;
        //SP2528_Initialize_from_T_Flash();
        filp_close(fp, NULL); 
    }
    //set_fs(fs);
    if(fromsd == 1)//是否从SD读取//gepeiwei   120903
        SP2528_Initialize_from_T_Flash();//从SD卡读取的主要函数
    else
    {
        SP0A19_InitialPara();
        SP0A19_Sensor_Init();
    }
#else
    SP0A19_InitialPara();
    SP0A19_Sensor_Init();		
#endif
    mdelay(500);	// 300 add by sp_yjp,20130314
    SENSORDB("SP0A19Open end \n");
    return ERROR_NONE;
}


/*************************************************************************
 * FUNCTION
 *	SP0A19Close
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
UINT32 SP0A19Close(void)
{
   SENSORDB("SP0A19Close\n");
    return ERROR_NONE;
} 


#if 0
//Sensor vendor suggest not try to revise  dummy pixel,it is relatiton with ae table ,it may effect to flicker
static void SP0A19_set_dummy( const kal_uint32 iPixels, const kal_uint32 iLines )
{

}
#endif



/*************************************************************************
* FUNCTION
*    SP0A19SetMirror
*
* DESCRIPTION
*    This function set the Mirror to the CMOS sensor
*
* PARAMETERS
*    Mirror
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void SP0A19SetMirror(kal_uint8 Mirror)
{
	SP0A19_SET_PAGE0;
	switch (Mirror)
	{
		case IMAGE_SENSOR_MIRROR_H:
		SP0A19_write_cmos_sensor(0x31, 0x20);
		break;
		case IMAGE_SENSOR_MIRROR_V:
		SP0A19_write_cmos_sensor(0x31, 0x40);
		break;
		case IMAGE_SENSOR_MIRROR_HV:
		SP0A19_write_cmos_sensor(0x31, 0x60);
		break;
		default:
		SP0A19_write_cmos_sensor(0x31, 0x00);
	}
}

static void SP0A19_set_dummy_line(const kal_uint32 iLines )
{

   kal_uint32 dummyLine= 0x00;
   
    if(dummyLine >256)
        dummyLine=256;
	
	SP0A19_write_cmos_sensor(0xfd, 0x00);
	SP0A19_write_cmos_sensor(0x06, dummyLine);
}


/*************************************************************************
 * FUNCTION
 * SP0A19Preview
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
UINT32 SP0A19Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
	SENSORDB("SP0A19Preview enter \n");

    image_window->GrabStartX= SP0A19_IMAGE_SENSOR_START_PIXELS;
    image_window->GrabStartY= SP0A19_IMAGE_SENSOR_START_LINES;
    image_window->ExposureWindowWidth = SP0A19_IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight = SP0A19_IMAGE_SENSOR_PV_HEIGHT;

	SP0A19NightMode(SP0A19_CurrentStatus.iNightMode);

    // copy sensor_config_data
    memcpy(&SP0A19SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	SENSORDB("SP0A19Preview out \n");
    return ERROR_NONE;
} 


UINT32 SP0A19Video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)


{
    SENSORDB("SP0A19Video enter \n");

    image_window->GrabStartX= SP0A19_IMAGE_SENSOR_START_PIXELS;
    image_window->GrabStartY= SP0A19_IMAGE_SENSOR_START_LINES;
    image_window->ExposureWindowWidth = SP0A19_IMAGE_SENSOR_VIDEO_WIDTH;
    image_window->ExposureWindowHeight = SP0A19_IMAGE_SENSOR_VIDEO_HEIGHT;

    SENSORDB("SP0A19Video SP0A19_CurrentStatus.iNightMode == %d!\n",SP0A19_CurrentStatus.iNightMode );
    SP0A19_MPEG4_encode_mode = KAL_FALSE;
    SP0A19NightMode(SP0A19_CurrentStatus.iNightMode);
    SP0A19_MPEG4_encode_mode = KAL_TRUE;

    // copy sensor_config_data
    memcpy(&SP0A19SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    SENSORDB("SP0A19Preview out \n");
    return ERROR_NONE;
} 

/*************************************************************************
 * FUNCTION
 *	SP0A19Capture
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
UINT32 SP0A19Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
	SENSORDB("SP0A19Capture enter \n");

    image_window->GrabStartX = SP0A19_IMAGE_SENSOR_START_PIXELS;
    image_window->GrabStartY = SP0A19_IMAGE_SENSOR_START_LINES;
    image_window->ExposureWindowWidth=  SP0A19_IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = SP0A19_IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&SP0A19SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	SENSORDB("SP0A19Capture out \n");
	
    return ERROR_NONE;
} /* SP0A19_Capture() */



UINT32 SP0A19GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=		SP0A19_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=	SP0A19_IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=	SP0A19_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=	SP0A19_IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorVideoWidth=	SP0A19_IMAGE_SENSOR_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight=	SP0A19_IMAGE_SENSOR_VIDEO_HEIGHT;
	
    return ERROR_NONE;
}


UINT32 SP0A19GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
	
    //pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
    //pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_UYVY;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_VYUY;//MTK default
	
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
    pSensorInfo->SensorInterruptDelayLines = 1;
	
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;

    pSensorInfo->CaptureDelayFrame = 1;
    pSensorInfo->PreviewDelayFrame = 1;
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;

	pSensorInfo->YUVAwbDelayFrame = 2;
	pSensorInfo->YUVEffectDelayFrame= 2;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = SP0A19_IMAGE_SENSOR_START_PIXELS;
        pSensorInfo->SensorGrabStartY = SP0A19_IMAGE_SENSOR_START_LINES;

        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = SP0A19_IMAGE_SENSOR_START_PIXELS;
        pSensorInfo->SensorGrabStartY = SP0A19_IMAGE_SENSOR_START_LINES;
        break;
    default:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = SP0A19_IMAGE_SENSOR_START_PIXELS;
        pSensorInfo->SensorGrabStartY = SP0A19_IMAGE_SENSOR_START_LINES;
        break;
    }
    memcpy(pSensorConfigData, &SP0A19SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
}


UINT32 SP0A19Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

    SP0A19CurrentScenarioId = ScenarioId;

	SENSORDB("SP0A19Control:ScenarioId =%d \n",SP0A19CurrentScenarioId);
   
    switch (ScenarioId)
    {
	    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
	        SP0A19_MPEG4_encode_mode = KAL_FALSE;
	        SP0A19Preview(pImageWindow, pSensorConfigData);
	        break;
			
	    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
	        SP0A19_MPEG4_encode_mode = KAL_TRUE;
	        SP0A19Video(pImageWindow, pSensorConfigData);
	        break;
	    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	    case MSDK_SCENARIO_ID_CAMERA_ZSD:
	        SP0A19_MPEG4_encode_mode = KAL_FALSE;
	        SP0A19Capture(pImageWindow, pSensorConfigData);
	        break;
	    default:
	        return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
}	/* SP0A19Control() */


BOOL SP0A19_set_param_wb(UINT16 para)
{
	SENSORDB("[Enter] SP0A19_set_param_wb!SP0A19_CurrentStatus.iWB = %d\n",SP0A19_CurrentStatus.iWB);
	SENSORDB("[Enter] SP0A19_set_param_wb!para = %d\n",para);
	if(SP0A19_CurrentStatus.iWB == para)
		return TRUE;

	#ifdef SP0A19_LOAD_FROM_T_FLASH
	return TRUE;
	#endif

	switch (para)
	{            
		case AWB_MODE_AUTO:
			SP0A19_write_cmos_sensor(0xfd,0x00);                                                          
			SP0A19_write_cmos_sensor(0x32,0x05);                                                          
			SP0A19_write_cmos_sensor(0xe7,0x03);                                                          
			SP0A19_write_cmos_sensor(0xfd,0x01);  
  			SP0A19_write_cmos_sensor(0x28,0xc9);                                                          
			SP0A19_write_cmos_sensor(0x29,0x98);
			
			SP0A19_write_cmos_sensor(0xfd,0x00);   // AUTO 3000K~7000K                                                          
			SP0A19_write_cmos_sensor(0xe7,0x00);                                                          
			SP0A19_write_cmos_sensor(0x32,0x15);                                                          
            break;
		case AWB_MODE_OFF:
	         {
			 SP0A19_write_cmos_sensor(0xfd,0x01);														   
			 SP0A19_write_cmos_sensor(0x28,0xc4);																
			 SP0A19_write_cmos_sensor(0x29,0x9e);
			 SP0A19_write_cmos_sensor(0xfd,0x00);  // AUTO 3000K~7000K		   
			 SP0A19_write_cmos_sensor(0x32,0x05); //awb2				 
		     }
			 break;

		case AWB_MODE_CLOUDY_DAYLIGHT:
		    // SP0A19_reg_WB_auto   阴天
			SP0A19_write_cmos_sensor(0xfd,0x00);   //7000K                                     
			SP0A19_write_cmos_sensor(0x32,0x05);                                                          
			SP0A19_write_cmos_sensor(0xfd,0x01);                                                          
			SP0A19_write_cmos_sensor(0x28,0xbf);		                                                       
			SP0A19_write_cmos_sensor(0x29,0x89);		                                                       
			SP0A19_write_cmos_sensor(0xfd,0x00);   
		    break;
		case AWB_MODE_DAYLIGHT:
	        // SP0A19_reg_WB_auto  白天 
			SP0A19_write_cmos_sensor(0xfd,0x00);  //6500K                                     
			SP0A19_write_cmos_sensor(0x32,0x05);                                                          
			SP0A19_write_cmos_sensor(0xfd,0x01);                                                          
			SP0A19_write_cmos_sensor(0x28,0xbc);		                                                       
			SP0A19_write_cmos_sensor(0x29,0x5d);		                                                       
			SP0A19_write_cmos_sensor(0xfd,0x00); 
		    break;
		case AWB_MODE_INCANDESCENT:	
		    // SP0A19_reg_WB_auto 白炽灯 
			SP0A19_write_cmos_sensor(0xfd,0x00);  //2800K~3000K                                     
			SP0A19_write_cmos_sensor(0x32,0x05);                                                          
			SP0A19_write_cmos_sensor(0xfd,0x01);                                                          
			SP0A19_write_cmos_sensor(0x28,0x89);		                                                       
			SP0A19_write_cmos_sensor(0x29,0xb8);		                                                       
			SP0A19_write_cmos_sensor(0xfd,0x00); 
		    break;  
		case AWB_MODE_FLUORESCENT:
	        //SP0A19_reg_WB_auto  荧光灯 
			SP0A19_write_cmos_sensor(0xfd,0x00);  //4200K~5000K                                     
			SP0A19_write_cmos_sensor(0x32,0x05);                                                          
			SP0A19_write_cmos_sensor(0xfd,0x01);                                                          
			SP0A19_write_cmos_sensor(0x28,0xb5);		                                                       
			SP0A19_write_cmos_sensor(0x29,0xa5);		                                                       
			SP0A19_write_cmos_sensor(0xfd,0x00); 
		    break;  
		case AWB_MODE_TUNGSTEN:
	        // SP0A19_reg_WB_auto 白热光
			SP0A19_write_cmos_sensor(0xfd,0x00);  //4000K                                   
			SP0A19_write_cmos_sensor(0x32,0x05);                                                          
			SP0A19_write_cmos_sensor(0xfd,0x01);                                                          
			SP0A19_write_cmos_sensor(0x28,0xaf);		                                                       
			SP0A19_write_cmos_sensor(0x29,0x99);		                                                       
			SP0A19_write_cmos_sensor(0xfd,0x00);  
		    break;
		default:
			return KAL_FALSE;
	}

	SP0A19_CurrentStatus.iWB = para;
	
	return KAL_TRUE;
}


BOOL SP0A19_set_param_effect(UINT16 para)
{

	
	if(SP0A19_CurrentStatus.iEffect == para)
		return TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x36, 0x20);
			SP0A19_write_cmos_sensor(0x06, 0xff);
			mdelay(180);
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x62, 0x00);
			SP0A19_write_cmos_sensor(0x63, 0x80);
			SP0A19_write_cmos_sensor(0x64, 0x80);
			mdelay(10);
			SP0A19_write_cmos_sensor(0x36, 0x00);
			SP0A19_write_cmos_sensor(0x06, 0x00);
			mdelay(10);
	        break;
		case MEFFECT_SEPIA:
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x36, 0x20);
			SP0A19_write_cmos_sensor(0x06, 0xff);
			mdelay(180);
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x62, 0x10);
			SP0A19_write_cmos_sensor(0x63, 0xc0);
			SP0A19_write_cmos_sensor(0x64, 0x20);
			mdelay(10);
			SP0A19_write_cmos_sensor(0x36, 0x00);
			SP0A19_write_cmos_sensor(0x06, 0x00);
			mdelay(10);
			break;  
		case MEFFECT_NEGATIVE:
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x36, 0x20);
			SP0A19_write_cmos_sensor(0x06, 0xff);
			mdelay(180);
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x62, 0x04);
			SP0A19_write_cmos_sensor(0x63, 0x80);
			SP0A19_write_cmos_sensor(0x64, 0x80);
			mdelay(10);
			SP0A19_write_cmos_sensor(0x36, 0x00);
			SP0A19_write_cmos_sensor(0x06, 0x00);
			mdelay(10);
			break; 
		case MEFFECT_SEPIAGREEN:	
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x36, 0x20);
			SP0A19_write_cmos_sensor(0x06, 0xff);
			mdelay(180);
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x62, 0x10);
			SP0A19_write_cmos_sensor(0x63, 0x20);
			SP0A19_write_cmos_sensor(0x64, 0x20);
			mdelay(10);
			SP0A19_write_cmos_sensor(0x36, 0x00);
			SP0A19_write_cmos_sensor(0x06, 0x00);
			mdelay(10);
			break;
		case MEFFECT_SEPIABLUE:
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x36, 0x20);
			SP0A19_write_cmos_sensor(0x06, 0xff);
			mdelay(180);
		  	SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x62, 0x10);
			SP0A19_write_cmos_sensor(0x63, 0x20);
			SP0A19_write_cmos_sensor(0x64, 0xf0);
			mdelay(10);
			SP0A19_write_cmos_sensor(0x36, 0x00);
			SP0A19_write_cmos_sensor(0x06, 0x00);
			mdelay(10);
			break;        
		case MEFFECT_MONO:
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x36, 0x20);
			SP0A19_write_cmos_sensor(0x06, 0xff);
			mdelay(180);
			SP0A19_write_cmos_sensor(0xfd, 0x00);
			SP0A19_write_cmos_sensor(0x62, 0x20);
			SP0A19_write_cmos_sensor(0x63, 0x80);
			SP0A19_write_cmos_sensor(0x64, 0x80);
			mdelay(10);
			SP0A19_write_cmos_sensor(0x36, 0x00);
			SP0A19_write_cmos_sensor(0x06, 0x00);
			mdelay(10);
			break;
		default:
			return KAL_FALSE;
	}
	
	SP0A19_CurrentStatus.iEffect = para;

	return KAL_TRUE;
}


BOOL SP0A19_set_param_banding(UINT16 para)
{
	if(SP0A19_CurrentStatus.iBanding == para)
      return TRUE;

	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			sp0a19_isBanding = 0;
			break;

		case AE_FLICKER_MODE_60HZ:
			sp0a19_isBanding = 1;
		break;
		default:
			return FALSE;
	}
	 SP0A19NightMode(SP0A19_CurrentStatus.iNightMode);

	SP0A19_CurrentStatus.iBanding =para;

	return TRUE;
} /* SP0A19_set_param_banding */


BOOL SP0A19_set_param_exposure(UINT16 para)
{
	if(SP0A19_CurrentStatus.iEV == para)
      return TRUE;

	switch (para)
	{
		case AE_EV_COMP_13:  
		case AE_EV_COMP_10:  
		case AE_EV_COMP_07:  
		case AE_EV_COMP_03: 
			#if 1
			  SP0A19_write_cmos_sensor(0xfd,0x00);
			  SP0A19_write_cmos_sensor(0xdc,0x20);
			#else
			//ae target
			  SP0A19_write_cmos_sensor(0xfd,0x00); 
			  SP0A19_write_cmos_sensor(0xed,0x8c+0x18); //indoor ae upper bound
			  SP0A19_write_cmos_sensor(0xf7,0x88+0x18); //indoor ae target
			  SP0A19_write_cmos_sensor(0xf8,0x80+0x18);
			  SP0A19_write_cmos_sensor(0xec,0x7c+0x18); //indoor ae low bound
			  
			  SP0A19_write_cmos_sensor(0xef,0x74+0x18); //out door
			  SP0A19_write_cmos_sensor(0xf9,0x70+0x18); 
			  SP0A19_write_cmos_sensor(0xfa,0x68+0x18); 
			  SP0A19_write_cmos_sensor(0xee,0x64+0x18);
			  #endif
			break;
		case AE_EV_COMP_00:  // +0 EV
			  #if 1
			  SP0A19_write_cmos_sensor(0xfd,0x00);
			  SP0A19_write_cmos_sensor(0xdc,0x00);
			 #else
			//ae target
			  SP0A19_write_cmos_sensor(0xfd,0x00); 
			  SP0A19_write_cmos_sensor(0xed,0x8c); 
			  SP0A19_write_cmos_sensor(0xf7,0x88); 
			  SP0A19_write_cmos_sensor(0xf8,0x80); 
			  SP0A19_write_cmos_sensor(0xec,0x7c); 
			  
			  SP0A19_write_cmos_sensor(0xef,0x74); 
			  SP0A19_write_cmos_sensor(0xf9,0x70); 
			  SP0A19_write_cmos_sensor(0xfa,0x68); 
			  SP0A19_write_cmos_sensor(0xee,0x64); 
			  #endif
			break;
		case AE_EV_COMP_n03:  
		case AE_EV_COMP_n07:	
		case AE_EV_COMP_n10:  
		case AE_EV_COMP_n13:
			   #if 1
			  SP0A19_write_cmos_sensor(0xfd,0x00);
			  SP0A19_write_cmos_sensor(0xdc,0xe0);
			  #else
			//ae target
			  SP0A19_write_cmos_sensor(0xfd,0x00); 
			  SP0A19_write_cmos_sensor(0xed,0x8c-0x18); 
			  SP0A19_write_cmos_sensor(0xf7,0x88-0x18); 
			  SP0A19_write_cmos_sensor(0xf8,0x80-0x18); 
			  SP0A19_write_cmos_sensor(0xec,0x7c-0x18); 
			  
			  SP0A19_write_cmos_sensor(0xef,0x74-0x18); 
			  SP0A19_write_cmos_sensor(0xf9,0x70-0x18); 
			  SP0A19_write_cmos_sensor(0xfa,0x68-0x18); 
			  SP0A19_write_cmos_sensor(0xee,0x64-0x18); 
			  #endif
			break;
		default:
			return KAL_FALSE;
	}
	SP0A19_CurrentStatus.iEV = para;
	return KAL_TRUE;
} 




void SP0A19_set_saturation(UINT16 para)
{
    switch (para)
    {

		SP0A19_write_cmos_sensor(0xfd,0x00); 
		
        case ISP_SAT_HIGH:
			SP0A19_write_cmos_sensor(0xd3,0x88+0x20); //outdoor 
			SP0A19_write_cmos_sensor(0xd4,0x88+0x20); //normal 
			SP0A19_write_cmos_sensor(0xd6,0x70+0x20); //dummy
			SP0A19_write_cmos_sensor(0xd7,0x60+0x20); //low light
             break;
			 
        case ISP_SAT_LOW:
			SP0A19_write_cmos_sensor(0xd3,0x88-0x20); 
			SP0A19_write_cmos_sensor(0xd4,0x88-0x20); 
			SP0A19_write_cmos_sensor(0xd6,0x70-0x20); 
			SP0A19_write_cmos_sensor(0xd7,0x60-0x20);
             break;
			 
        case ISP_SAT_MIDDLE:
        default:
			SP0A19_write_cmos_sensor(0xd3,0x88);
			SP0A19_write_cmos_sensor(0xd4,0x88); 
			SP0A19_write_cmos_sensor(0xd6,0x70); 
			SP0A19_write_cmos_sensor(0xd7,0x60);
             break;
    }
     return;
}

void SP0A19_set_contrast(UINT16 para)
{
	//gamma
	switch (para)
	{
		SP0A19_write_cmos_sensor(0xfd,0x00);

		case ISP_CONTRAST_LOW:
			SP0A19_write_cmos_sensor(0xdd,0x80-0x10); 
			SP0A19_write_cmos_sensor(0xde,0xa8-0x20); 
			 break;
		case ISP_CONTRAST_HIGH:
			SP0A19_write_cmos_sensor(0xd4,0x80+0x10); 
			SP0A19_write_cmos_sensor(0xd6,0xa8+0x20); 
			 break;
		case ISP_CONTRAST_MIDDLE:
		default:
			SP0A19_write_cmos_sensor(0xdd,0x80); 
			SP0A19_write_cmos_sensor(0xde,0xa8); 
			 break;
	}
	
	return;
}

#if 0
void SP0A19_set_Brightness_ISO(void)
{
	if(SP0A19_CurrentStatus.iIso== ISP_BRIGHT_LOW)
	{





	}




}
#endif

void SP0A19_set_brightness(UINT16 para)
{
    //ae offset
    switch (para)
    {
	    SP0A19_write_cmos_sensor(0xfd,0x00); 
		
        case ISP_BRIGHT_LOW:
			SP0A19_CurrentStatus.iBrightness = ISP_BRIGHT_LOW;
			 SP0A19_write_cmos_sensor(0xdc,0xe0); 
             break;
        case ISP_BRIGHT_HIGH:
			SP0A19_CurrentStatus.iBrightness = ISP_BRIGHT_HIGH;
			 SP0A19_write_cmos_sensor(0xfd,0x00+0x10); 
             break;
        case ISP_BRIGHT_MIDDLE:
        default:
			SP0A19_CurrentStatus.iBrightness = ISP_BRIGHT_MIDDLE;
			SP0A19_write_cmos_sensor(0xfd,0x00); 
             break;
    }
	//SP0A19_set_Brightness_ISO();
    return;
}


void SP0A19_set_iso(UINT16 para)
{
    //ae offset
    switch (para)
    {
        case AE_ISO_100:
			SP0A19_CurrentStatus.iIso =AE_ISO_100;
             break;
        case AE_ISO_200:
			SP0A19_CurrentStatus.iIso =AE_ISO_200;
             break;
        case AE_ISO_400:
			SP0A19_CurrentStatus.iIso =AE_ISO_400;
             break;
        default:
        case AE_ISO_AUTO:
			SP0A19_CurrentStatus.iIso =AE_ISO_AUTO;
             break;
    }
	//SP0A19_set_Brightness_ISO();
    return;
}


void SP0A19_AE_Lock_Enable(UINT16 iPara)
{
	kal_uint16 temp_AE_reg = 0;

	SP0A19_write_cmos_sensor(0xfd, 0x00);
	temp_AE_reg = SP0A19_read_cmos_sensor(0x32);

	switch(iPara)
	{

		case AE_MODE_OFF:
			SP0A19_write_cmos_sensor(0x32, (temp_AE_reg & (~0x01)));
			break;

		//case AE_MODE_ON:
		default:
			SP0A19_write_cmos_sensor(0x32, (temp_AE_reg |0x01));
			break;

	}		
}


UINT32 SP0A19YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {
		
	case FID_COLOR_EFFECT:
		SP0A19_set_param_effect(iPara);
		break;

    case FID_AE_EV:
        SP0A19_set_param_exposure(iPara);
        break;

	case FID_SCENE_MODE:
		SP0A19NightMode(iPara);
        break;
		
    case FID_AWB_MODE:
        SP0A19_set_param_wb(iPara);
        break;

    case FID_AE_FLICKER:
        SP0A19_set_param_banding(iPara);
		break;

	case FID_ISP_SAT:
		SP0A19_set_saturation(iPara);
		break;
		
	case FID_ISP_CONTRAST:
		SP0A19_set_contrast(iPara);
		break;

	case FID_ISP_BRIGHT:
		SP0A19_set_brightness(iPara);
		break;
		
	case FID_AE_ISO:
		SP0A19_set_iso(iPara);
		break;
	
	case FID_AE_SCENE_MODE:
		SP0A19_AE_Lock_Enable(iPara);
		break;

	case FID_ZOOM_FACTOR:
		zoom_factor = iPara; 
		break; 
    default:
        break;
    }
    return TRUE;
} /* SP0A19YUVSensorSetting */


UINT32 SP0A19YUVSetVideoMode(UINT16 u2FrameRate)
{
    SENSORDB("SP0A19YUVSetVideoMode enter!\n" );
    SP0A19_MPEG4_encode_mode = KAL_FALSE;
    SENSORDB("SP0A19YUVSetVideoMode SP0A19_CurrentStatus.iNightMode == %d!\n",SP0A19_CurrentStatus.iNightMode );
    //set fps 
    SP0A19NightMode(SP0A19_CurrentStatus.iNightMode);
    SP0A19_MPEG4_encode_mode = KAL_TRUE;

    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*    AP0A19GetEvAwbRef
*
* DESCRIPTION
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void SP0A19GetEvAwbRef(UINT32 pSensorAEAWBRefStruct)//checked in lab
{
    PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
    SENSORDB("AP0A19GetEvAwbRef  \n" );
    	
	Ref->SensorAERef.AeRefLV05Shutter = 1530;
    Ref->SensorAERef.AeRefLV05Gain = 1280; /* 128 base */ 
    Ref->SensorAERef.AeRefLV13Shutter = 61;
    Ref->SensorAERef.AeRefLV13Gain = 320; /*  128 base */
    Ref->SensorAwbGainRef.AwbRefD65Rgain = 197; /* 128 base */
    Ref->SensorAwbGainRef.AwbRefD65Bgain = 134; /* 128 base */
    Ref->SensorAwbGainRef.AwbRefCWFRgain = 177; /* 128 base */
    Ref->SensorAwbGainRef.AwbRefCWFBgain = 183; /* 128 base */
}

kal_uint16 SP0A19ReadGain(void)
{
    kal_uint32 temp=0,Sensor_GainBase=16,Feature_GainBase=128;

	SP0A19_write_cmos_sensor(0xfd, 0x00);
	temp = SP0A19_read_cmos_sensor(0x23);//AGain
	//SENSORDB("SP0A19ReadGain -org temp=%d \n",temp);
	
	temp = temp*Feature_GainBase/Sensor_GainBase;
	//SENSORDB("SP0A19ReadGain temp=%d \n",temp);

	return temp;
}


kal_uint16 SP0A19ReadAwbRGain(void)
{
    kal_uint32 temp=0,Sensor_GainBase=128,Feature_GainBase=128;
	
	SP0A19_write_cmos_sensor(0xfd, 0x01);
	temp = SP0A19_read_cmos_sensor(0x28);//RGain
	
	temp = temp*Feature_GainBase/Sensor_GainBase;
	//SENSORDB("SP0A19ReadAwbRGain_R temp=%d \n",temp);

	return temp;
}

kal_uint16 SP0A19ReadAwbBGain(void)
{
    kal_uint16 temp=0x0000,Sensor_GainBase=128,Feature_GainBase=128;
	
	SP0A19_write_cmos_sensor(0xfd, 0x01);
	temp = SP0A19_read_cmos_sensor(0x29);//BGain
	
	temp = temp*Feature_GainBase/Sensor_GainBase;
	//SENSORDB("SP0A19ReadAwbBGain_B temp=%d \n",temp);

	return temp;
}


/*************************************************************************
* FUNCTION
*    AP0A19GetCurAeAwbInfo
*
* DESCRIPTION
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void SP0A19GetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct)
{
    PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
    SENSORDB("AP0A19GetCurAeAwbInfo  \n" );

    Info->SensorAECur.AeCurShutter = SP0A19_Read_Shutter();
    Info->SensorAECur.AeCurGain = SP0A19ReadGain(); /* 128 base */
    
    Info->SensorAwbGainCur.AwbCurRgain = SP0A19ReadAwbRGain(); /* 128 base */
    
    Info->SensorAwbGainCur.AwbCurBgain = SP0A19ReadAwbBGain(); /* 128 base */
}


void SP0A19GetAFMaxNumFocusAreas(UINT32 *pFeatureReturnPara32)
{	
    *pFeatureReturnPara32 = 0;    
    SENSORDB("SP0A19GetAFMaxNumFocusAreas, *pFeatureReturnPara32 = %d\n",*pFeatureReturnPara32);

}


void SP0A19GetAFMaxNumMeteringAreas(UINT32 *pFeatureReturnPara32)
{	
    *pFeatureReturnPara32 = 0;    
    SENSORDB("SP0A19GetAFMaxNumMeteringAreas,*pFeatureReturnPara32 = %d\n",*pFeatureReturnPara32);

}


void SP0A19GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = AE_ISO_100;
    pExifInfo->AWBMode = SP0A19_CurrentStatus.iWB;
    pExifInfo->CapExposureTime = 0;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = AE_ISO_100;
}


void SP0A19GetDelayInfo(UINT32 delayAddr)
{
    SENSOR_DELAY_INFO_STRUCT* pDelayInfo = (SENSOR_DELAY_INFO_STRUCT*)delayAddr;
    pDelayInfo->InitDelay = 3;
    pDelayInfo->EffectDelay = 2;
    pDelayInfo->AwbDelay = 2;
   // pDelayInfo->AFSwitchDelayFrame = 50;
}


void SP0A19GetAEAWBLock(UINT32 *pAElockRet32,UINT32 *pAWBlockRet32)
{
    *pAElockRet32 = 1;
	*pAWBlockRet32 = 1;
    SENSORDB("SP0A19GetAEAWBLock,AE=%d ,AWB=%d\n,",*pAElockRet32,*pAWBlockRet32);
}


UINT32 SP0A19_SetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("[SP0A19_SetTestPatternMode] Test pattern enable:%d\n", bEnable);

	SP0A19_write_cmos_sensor(0xfd, 0x00);

	if(bEnable) //enable test pattern output
	{
		SP0A19_write_cmos_sensor(0x0d, 0x10);// 16 nums bar
		//SP0A19_write_cmos_sensor(0x0d, 0x14);// 8 nums bar

	}
	else //disable test pattern output    
	{
		SP0A19_write_cmos_sensor(0x0d, 0x00);// 16 nums bar
	}
    return ERROR_NONE;
}



kal_uint32 SP0A19GetLineLength(void)
{

	kal_uint32 Current_LineLength = 0;

	SP0A19_write_cmos_sensor(0xfd, 0x00);

	//can only get dummy pixel num,can't get total pixel num (no related registers)
    Current_LineLength = ((SP0A19_read_cmos_sensor(0x09)&0x0f)<<8)|(SP0A19_read_cmos_sensor(0x08));
	Current_LineLength =Current_LineLength + SP0A19_VGA_PERIOD_PIXEL_NUMS;

	SENSORDB("SP0A19GetLineLength: Current_LineLength = %d\n",Current_LineLength);
    return Current_LineLength;
}


UINT32 SP0A19SetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	SENSORDB("SP0A19SetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = SP0A19_PCLK;
			
			//lineLength = SP0A19_VGA_PERIOD_PIXEL_NUMS;
			lineLength = SP0A19GetLineLength();
			
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - SP0A19_VGA_PERIOD_LINE_NUMS;
			if(dummyLine<0)
				dummyLine = 0;
			SENSORDB("SP0A19SetMaxFramerateByScenario MSDK_SCENARIO_ID_CAMERA_PREVIEW: lineLength = %d, dummy=%d\n",lineLength,dummyLine);

			//SP0A19_set_dummy(0, dummyLine);	
			SP0A19_set_dummy_line(dummyLine);
			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = SP0A19_PCLK;
			
			//lineLength = SP0A19_VGA_PERIOD_PIXEL_NUMS;
			lineLength = SP0A19GetLineLength();
			
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - SP0A19_VGA_PERIOD_LINE_NUMS;
			if(dummyLine<0)
				dummyLine = 0;
			SENSORDB("SP0A19SetMaxFramerateByScenario MSDK_SCENARIO_ID_VIDEO_PREVIEW: lineLength = %d, dummy=%d\n",lineLength,dummyLine);			
			//SP0A19_set_dummy(0, dummyLine);	
			SP0A19_set_dummy_line(dummyLine);
			break;			
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = SP0A19_PCLK;
			
			//lineLength = SP0A19_VGA_PERIOD_PIXEL_NUMS;
			lineLength = SP0A19GetLineLength();
			
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - SP0A19_VGA_PERIOD_LINE_NUMS;
			if(dummyLine<0)
				dummyLine = 0;
			SENSORDB("SP0A19SetMaxFramerateByScenario MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG: lineLength = %d, dummy=%d\n",lineLength,dummyLine);			
			//SP0A19_set_dummy(0, dummyLine);
			SP0A19_set_dummy_line(dummyLine);
			
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



UINT32 SP0A19GetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 300;
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


UINT32 SP0A19FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=SP0A19_IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=SP0A19_IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        //*pFeatureReturnPara16++=SP0A19_VGA_PERIOD_PIXEL_NUMS+SP0A19_dummy_pixels;
        //*pFeatureReturnPara16=SP0A19_VGA_PERIOD_LINE_NUMS+SP0A19_dummy_lines;
        //*pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = SP0A19_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        SP0A19NightMode((BOOL) *pFeatureData16);
        break;

	case SENSOR_FEATURE_SET_VIDEO_MODE:
		SP0A19YUVSetVideoMode(*pFeatureData16);
		break; 
		
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        SP0A19_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        SP0A19_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = SP0A19_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &SP0A19SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
        *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
        break;
    case SENSOR_FEATURE_SET_CCT_REGISTER:
    case SENSOR_FEATURE_GET_CCT_REGISTER:
    case SENSOR_FEATURE_SET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
    case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
    case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
    case SENSOR_FEATURE_GET_GROUP_INFO:
    case SENSOR_FEATURE_GET_ITEM_INFO:
    case SENSOR_FEATURE_SET_ITEM_INFO:
    case SENSOR_FEATURE_GET_ENG_INFO:
        break;
	case SENSOR_FEATURE_GET_GROUP_COUNT:
        *pFeatureReturnPara32++=0;
        *pFeatureParaLen=4;	   
		break;
		
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
        // if EEPROM does not exist in camera module.
        *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_YUV_CMD:
        SP0A19YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
		SP0A19GetSensorID(pFeatureData32);
		break;

	case SENSOR_FEATURE_GET_EV_AWB_REF:
		 SP0A19GetEvAwbRef(*pFeatureData32);
		break;

	case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
		 SP0A19GetCurAeAwbInfo(*pFeatureData32);	
		 break;

	case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
		 SP0A19GetAFMaxNumFocusAreas(pFeatureReturnPara32);		   
		 *pFeatureParaLen=4;
		 break;	   

	case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
		 SP0A19GetAFMaxNumMeteringAreas(pFeatureReturnPara32);			  
		 *pFeatureParaLen=4;
		 break;

	case SENSOR_FEATURE_GET_EXIF_INFO:
		SENSORDB("SENSOR_FEATURE_GET_EXIF_INFO\n");
		SENSORDB("EXIF addr = 0x%x\n",*pFeatureData32); 		 
		SP0A19GetExifInfo(*pFeatureData32);
		break;

	case SENSOR_FEATURE_GET_DELAY_INFO:
		SENSORDB("SENSOR_FEATURE_GET_DELAY_INFO\n");
		SP0A19GetDelayInfo(*pFeatureData32);
		break;

	case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
		SP0A19GetAEAWBLock((UINT32*)(*pFeatureData32),(UINT32*)*(pFeatureData32+1));
		break;

	case SENSOR_FEATURE_SET_TEST_PATTERN:
		 //SP0A19_SetTestPatternMode((BOOL)*pFeatureData16);
		 break;

	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:	   
		 *pFeatureReturnPara32= SP0A19_TEST_PATTERN_CHECKSUM; 		  
		 *pFeatureParaLen=4;							 
		 break;
	
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		SP0A19SetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		SP0A19GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
		break;
    default:
        break;
	}
return ERROR_NONE;
}	/* SP0A19FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncSP0A19YUV=
{
	SP0A19Open,
	SP0A19GetInfo,
	SP0A19GetResolution,
	SP0A19FeatureControl,
	SP0A19Control,
	SP0A19Close
};


UINT32 SP0A19_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncSP0A19YUV;
	return ERROR_NONE;
} /* SensorInit() */
