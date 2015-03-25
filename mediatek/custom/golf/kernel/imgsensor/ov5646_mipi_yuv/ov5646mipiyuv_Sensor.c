/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 * Author:
 * -------
 *   PC Huang (MTK02204)
 *
 *============================================================================
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
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
#include <asm/io.h>
#include <asm/system.h>	 
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"
#include "ov5646mipiyuv_Sensor.h"
#include "ov5646mipiyuv_Camera_Sensor_para.h"
#include "ov5646mipiyuv_CameraCustomized.h" 
#define OV5646MIPIYUV_DEBUG
#ifdef OV5646MIPIYUV_DEBUG
#define OV5646MIPISENSORDB printk
#else
#define OV5646MIPISENSORDB(x,...)
#endif

#define OV5646_OTP_DEBUG

static DEFINE_SPINLOCK(ov5646mipi_drv_lock);
static bool OV5646MIPI_AF_Power = false;
static MSDK_SCENARIO_ID_ENUM OV5646CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define OV5646MIPI_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,OV5646MIPI_WRITE_ID)
#define mDELAY(ms)  mdelay(ms)
kal_uint8 OV5646MIPI_sensor_socket = DUAL_CAMERA_NONE_SENSOR;
int OV5646MIPI_flash_mode = 2;
typedef enum
{
	OV5646MIPI_PRV_W=1280,
	OV5646MIPI_PRV_H=960
}OV5646MIPI_PREVIEW_VIEW_SIZE;

kal_uint16 OV5646MIPIYUV_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
	iReadReg((u16) addr ,(u8*)&get_byte,OV5646MIPI_WRITE_ID);
	return get_byte;
}


bool OV5646MIPIYUV_read_otp(void)
{
	int i;
	kal_uint16 mid, lens_id ;
	kal_uint32 address = 0x3d05;
	OV5646MIPI_write_cmos_sensor(0x3000, 0x00);
	OV5646MIPI_write_cmos_sensor(0x3004, 0xff);
	mDELAY(10);
	OV5646MIPI_write_cmos_sensor(0x3D21, 0x01);
#ifdef OV5646_OTP_DEBUG
	for(i=0; i<32; i++)
	{
	    printk("[OV5646]mingji test otp data.res[0x%x]==0x%x\n",0x3D00+i,OV5646MIPIYUV_read_cmos_sensor(0x3D00+i));
	}
#endif
	for(i=0; i<3; i++)
	{
	    address = 0X3d05 + i*9;
	    printk("[OV5646]mingji test address == 0x%x\n",address);
	    mid = OV5646MIPIYUV_read_cmos_sensor(address);
	    lens_id = OV5646MIPIYUV_read_cmos_sensor(address+1);
	    if ((mid == 0x05)&&((lens_id == 0x00)||(lens_id == 0x89)))  break;//darling af modules.
	}
	mDELAY(40);
	OV5646MIPI_write_cmos_sensor(0x3D21, 0x00);

	if (i<3)
	{
	    printk("[OV5646]mingji test. otp read success.");
	    return true;//darling af modules.
	}
	else
	{
	    printk("[OV5646]mingji test. otp read fail.");
	    return false;
	}

}


#define OV5646MIPI_MAX_AXD_GAIN (32) //max gain = 32
#define OV5646MIPI_MAX_EXPOSURE_TIME (1968) // preview:984,capture 984*2
static struct
{
	//kal_uint8   Banding;
	kal_bool	  NightMode;
	kal_bool	  VideoMode;
	kal_uint16  Fps;
	kal_uint16  ShutterStep;
	kal_uint8   IsPVmode;
	kal_uint32  PreviewDummyPixels;
	kal_uint32  PreviewDummyLines;
	kal_uint32  CaptureDummyPixels;
	kal_uint32  CaptureDummyLines;
	kal_uint32  PreviewPclk;
	kal_uint32  CapturePclk;
	kal_uint32  ZsdturePclk;
	kal_uint32  PreviewShutter;
	kal_uint32  PreviewExtraShutter;
	kal_uint32  SensorGain;
	kal_bool    	manualAEStart;
	kal_bool    	userAskAeLock;
	kal_bool    	userAskAwbLock;
	kal_uint32      currentExposureTime;
	kal_uint32      currentShutter;
	kal_uint32      currentextshutter;
	kal_uint32      currentAxDGain;
	kal_uint32  	sceneMode;
	unsigned char isoSpeed;
	unsigned char zsd_flag;
	kal_uint32      AF_window_x;
	kal_uint32      AF_window_y;
	unsigned char   awbMode;
	UINT16 iWB;
	OV5646MIPI_SENSOR_MODE SensorMode;
} OV5646MIPISensor;

/* Global Valuable */
static kal_uint32 OV5646MIPI_zoom_factor = 0; 
static kal_int8 OV5646MIPI_DELAY_AFTER_PREVIEW = -1;
static kal_uint8 OV5646MIPI_Banding_setting = AE_FLICKER_MODE_50HZ; 
static kal_bool OV5646MIPI_AWB_ENABLE = KAL_TRUE; 
static kal_bool OV5646MIPI_AE_ENABLE = KAL_TRUE; 
MSDK_SENSOR_CONFIG_STRUCT OV5646MIPISensorConfigData;
#define OV5646_TEST_PATTERN_CHECKSUM (0x7ba87eae)
kal_bool OV5646MIPI_run_test_potten=0;
#define OV5646_TAF_TOLERANCE (100)
void OV5646MIPI_set_scene_mode(UINT16 para);
BOOL OV5646MIPI_set_param_wb(UINT16 para);


typedef enum
{
	OV5646MIPI_AE_SECTION_INDEX_BEGIN=0,
	OV5646MIPI_AE_SECTION_INDEX_1=OV5646MIPI_AE_SECTION_INDEX_BEGIN,
	OV5646MIPI_AE_SECTION_INDEX_2,
	OV5646MIPI_AE_SECTION_INDEX_3,
	OV5646MIPI_AE_SECTION_INDEX_4,
	OV5646MIPI_AE_SECTION_INDEX_5,
	OV5646MIPI_AE_SECTION_INDEX_6,
	OV5646MIPI_AE_SECTION_INDEX_7,
	OV5646MIPI_AE_SECTION_INDEX_8,
	OV5646MIPI_AE_SECTION_INDEX_9,
	OV5646MIPI_AE_SECTION_INDEX_10,
	OV5646MIPI_AE_SECTION_INDEX_11,
	OV5646MIPI_AE_SECTION_INDEX_12,
	OV5646MIPI_AE_SECTION_INDEX_13,
	OV5646MIPI_AE_SECTION_INDEX_14,
	OV5646MIPI_AE_SECTION_INDEX_15,
	OV5646MIPI_AE_SECTION_INDEX_16,
	OV5646MIPI_AE_SECTION_INDEX_MAX
}OV5646MIPI_AE_SECTION_INDEX;
typedef enum
{
	OV5646MIPI_AE_VERTICAL_BLOCKS=4,
	OV5646MIPI_AE_VERTICAL_BLOCKS_MAX,
	OV5646MIPI_AE_HORIZONTAL_BLOCKS=4,
	OV5646MIPI_AE_HORIZONTAL_BLOCKS_MAX
}OV5646MIPI_AE_VERTICAL_HORIZONTAL_BLOCKS;
static UINT32 OV5646MIPI_line_coordinate[OV5646MIPI_AE_VERTICAL_BLOCKS_MAX] = {0};//line[0]=0      line[1]=160     line[2]=320     line[3]=480     line[4]=640
static UINT32 OV5646MIPI_row_coordinate[OV5646MIPI_AE_HORIZONTAL_BLOCKS_MAX] = {0};//line[0]=0       line[1]=120     line[2]=240     line[3]=360     line[4]=480
static BOOL OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_MAX] = {FALSE};
static BOOL OV5646MIPI_AE_2_ARRAY[OV5646MIPI_AE_HORIZONTAL_BLOCKS][OV5646MIPI_AE_VERTICAL_BLOCKS] = {{FALSE},{FALSE},{FALSE},{FALSE}};//how to ....
//=====================touch AE begin==========================//
void OV5646MIPI_writeAEReg(void)
{	
	UINT8 temp;
	//write 1280X960
	OV5646MIPI_write_cmos_sensor(0x501d,0x10);
	OV5646MIPI_write_cmos_sensor(0x5680,0x00); 
	OV5646MIPI_write_cmos_sensor(0x5681,0x00);
	OV5646MIPI_write_cmos_sensor(0x5682,0x00);  
	OV5646MIPI_write_cmos_sensor(0x5683,0x00);
	OV5646MIPI_write_cmos_sensor(0x5684,0x05); //width=256  
	OV5646MIPI_write_cmos_sensor(0x5685,0x00);
	OV5646MIPI_write_cmos_sensor(0x5686,0x03); //heght=256
	OV5646MIPI_write_cmos_sensor(0x5687,0xc0);
	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_1]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_2]==TRUE)    { temp=temp|0xF0;}
	//write 0x5688
	OV5646MIPI_write_cmos_sensor(0x5688,temp);
    
	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_3]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_4]==TRUE)    { temp=temp|0xF0;}
	//write 0x5689
	OV5646MIPI_write_cmos_sensor(0x5689,temp);

	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_5]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_6]==TRUE)    { temp=temp|0xF0;}
	//write 0x568A
	OV5646MIPI_write_cmos_sensor(0x568A,temp);
    
	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_7]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_8]==TRUE)    { temp=temp|0xF0;}
	//write 0x568B
	OV5646MIPI_write_cmos_sensor(0x568B,temp);

	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_9]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_10]==TRUE)  { temp=temp|0xF0;}
	//write 0x568C
	OV5646MIPI_write_cmos_sensor(0x568C,temp);

	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_11]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_12]==TRUE)    { temp=temp|0xF0;}
	//write 0x568D
	OV5646MIPI_write_cmos_sensor(0x568D,temp);    
    
	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_13]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_14]==TRUE)    { temp=temp|0xF0;}
	//write 0x568E
	OV5646MIPI_write_cmos_sensor(0x568E,temp);

	temp=0x11;
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_15]==TRUE)    { temp=temp|0x0F;}
	if(OV5646MIPI_AE_1_ARRAY[OV5646MIPI_AE_SECTION_INDEX_16]==TRUE)    { temp=temp|0xF0;}
	//write 0x568F
	OV5646MIPI_write_cmos_sensor(0x568F,temp);
}


void OV5646MIPI_printAE_1_ARRAY(void)
{
	UINT32 i;
	for(i=0; i<OV5646MIPI_AE_SECTION_INDEX_MAX; i++)
	{
		OV5646MIPISENSORDB("OV5646MIPI_AE_1_ARRAY[%2d]=%d\n", i, OV5646MIPI_AE_1_ARRAY[i]);
	}
}

void OV5646MIPI_printAE_2_ARRAY(void)
{
	UINT32 i, j;
	OV5646MIPISENSORDB("\t\t");
	for(i=0; i<OV5646MIPI_AE_VERTICAL_BLOCKS; i++)
	{
		OV5646MIPISENSORDB("      line[%2d]", i);
	}
	printk("\n");
	for(j=0; j<OV5646MIPI_AE_HORIZONTAL_BLOCKS; j++)
	{
		OV5646MIPISENSORDB("\trow[%2d]", j);
		for(i=0; i<OV5646MIPI_AE_VERTICAL_BLOCKS; i++)
		{
			//SENSORDB("OV5646MIPI_AE_2_ARRAY[%2d][%2d]=%d\n", j,i,OV5646MIPI_AE_2_ARRAY[j][i]);
			OV5646MIPISENSORDB("  %7d", OV5646MIPI_AE_2_ARRAY[j][i]);
		}
		OV5646MIPISENSORDB("\n");
	}
}

void OV5646MIPI_clearAE_2_ARRAY(void)
{
	UINT32 i, j;
	for(j=0; j<OV5646MIPI_AE_HORIZONTAL_BLOCKS; j++)
	{
		for(i=0; i<OV5646MIPI_AE_VERTICAL_BLOCKS; i++)
		{OV5646MIPI_AE_2_ARRAY[j][i]=FALSE;}
	}
}

void OV5646MIPI_mapAE_2_ARRAY_To_AE_1_ARRAY(void)
{
	UINT32 i, j;
	for(j=0; j<OV5646MIPI_AE_HORIZONTAL_BLOCKS; j++)
	{
		for(i=0; i<OV5646MIPI_AE_VERTICAL_BLOCKS; i++)
		{ OV5646MIPI_AE_1_ARRAY[j*OV5646MIPI_AE_VERTICAL_BLOCKS+i] = OV5646MIPI_AE_2_ARRAY[j][i];}
	}
}

void OV5646MIPI_mapMiddlewaresizePointToPreviewsizePoint(
    UINT32 mx,
    UINT32 my,
    UINT32 mw,
    UINT32 mh,
    UINT32 * pvx,
    UINT32 * pvy,
    UINT32 pvw,
    UINT32 pvh
)
{
	*pvx = pvw * mx / mw;
	*pvy = pvh * my / mh;
	OV5646MIPISENSORDB("mapping middlware x[%d],y[%d], [%d X %d]\n\t\tto x[%d],y[%d],[%d X %d]\n ",mx, my, mw, mh, *pvx, *pvy, pvw, pvh);
}


void OV5646MIPI_calcLine(void)
{//line[5]
	UINT32 i;
	UINT32 step = OV5646MIPI_PRV_W / OV5646MIPI_AE_VERTICAL_BLOCKS;
	for(i=0; i<=OV5646MIPI_AE_VERTICAL_BLOCKS; i++)
	{
		*(&OV5646MIPI_line_coordinate[0]+i) = step*i;
		OV5646MIPISENSORDB("line[%d]=%d\t",i, *(&OV5646MIPI_line_coordinate[0]+i));
	}
	OV5646MIPISENSORDB("\n");
}

void OV5646MIPI_calcRow(void)
{//row[5]
	UINT32 i;
	UINT32 step = OV5646MIPI_PRV_H / OV5646MIPI_AE_HORIZONTAL_BLOCKS;
	for(i=0; i<=OV5646MIPI_AE_HORIZONTAL_BLOCKS; i++)
	{
		*(&OV5646MIPI_row_coordinate[0]+i) = step*i;
		OV5646MIPISENSORDB("row[%d]=%d\t",i,*(&OV5646MIPI_row_coordinate[0]+i));
	}
	OV5646MIPISENSORDB("\n");
}

void OV5646MIPI_calcPointsAELineRowCoordinate(UINT32 x, UINT32 y, UINT32 * linenum, UINT32 * rownum)
{
	UINT32 i;
	i = 1;
	while(i<=OV5646MIPI_AE_VERTICAL_BLOCKS)
	{
		if(x<OV5646MIPI_line_coordinate[i])
		{
			*linenum = i;
			break;
		}
		*linenum = i++;
	}
    
	i = 1;
	while(i<=OV5646MIPI_AE_HORIZONTAL_BLOCKS)
	{
		if(y<OV5646MIPI_row_coordinate[i])
		{
			*rownum = i;
			break;
		}
		*rownum = i++;
	}
	OV5646MIPISENSORDB("PV point [%d, %d] to section line coordinate[%d] row[%d]\n",x,y,*linenum,*rownum);
}



MINT32 OV5646MIPI_clampSection(UINT32 x, UINT32 min, UINT32 max)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

void OV5646MIPI_mapCoordinate(UINT32 linenum, UINT32 rownum, UINT32 * sectionlinenum, UINT32 * sectionrownum)
{
	*sectionlinenum = OV5646MIPI_clampSection(linenum-1,0,OV5646MIPI_AE_VERTICAL_BLOCKS-1);
	*sectionrownum = OV5646MIPI_clampSection(rownum-1,0,OV5646MIPI_AE_HORIZONTAL_BLOCKS-1);	
	OV5646MIPISENSORDB("OV5646MIPI_mapCoordinate from[%d][%d] to[%d][%d]\n",linenum, rownum,*sectionlinenum,*sectionrownum);
}

void OV5646MIPI_mapRectToAE_2_ARRAY(UINT32 x0, UINT32 y0, UINT32 x1, UINT32 y1)
{
	UINT32 i, j;
	OV5646MIPISENSORDB("([%d][%d]),([%d][%d])\n", x0,y0,x1,y1);
	OV5646MIPI_clearAE_2_ARRAY();
	x0=OV5646MIPI_clampSection(x0,0,OV5646MIPI_AE_VERTICAL_BLOCKS-1);
	y0=OV5646MIPI_clampSection(y0,0,OV5646MIPI_AE_HORIZONTAL_BLOCKS-1);
	x1=OV5646MIPI_clampSection(x1,0,OV5646MIPI_AE_VERTICAL_BLOCKS-1);
	y1=OV5646MIPI_clampSection(y1,0,OV5646MIPI_AE_HORIZONTAL_BLOCKS-1);

	for(j=y0; j<=y1; j++)
	{
		for(i=x0; i<=x1; i++)
		{
			OV5646MIPI_AE_2_ARRAY[j][i]=TRUE;
		}
	}
}

void OV5646MIPI_resetPVAE_2_ARRAY(void)
{
	OV5646MIPI_mapRectToAE_2_ARRAY(1,1,2,2);
}

//update ae window
//@input zone[] addr
void OV5646_FOCUS_Set_AE_Window(UINT32 zone_addr)
{
	//update global zone
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_Set_AE_Window function\n");
	//input:
	UINT32 FD_XS;
	UINT32 FD_YS;	
	UINT32 x0, y0, x1, y1;
	UINT32 pvx0, pvy0, pvx1, pvy1;
	UINT32 linenum, rownum;
	UINT32 rightbottomlinenum,rightbottomrownum;
	UINT32 leftuplinenum,leftuprownum;
	UINT32* zone = (UINT32*)zone_addr;
	x0 = *zone;
	y0 = *(zone + 1);
	x1 = *(zone + 2);
	y1 = *(zone + 3);	
	FD_XS = *(zone + 4);
	FD_YS = *(zone + 5);

	OV5646MIPISENSORDB("AE x0=%d,y0=%d,x1=%d,y1=%d,FD_XS=%d,FD_YS=%d\n",x0, y0, x1, y1, FD_XS, FD_YS);	
    
	//print_sensor_ae_section();
	//print_AE_section();	

	//1.transfer points to preview size
	//UINT32 pvx0, pvy0, pvx1, pvy1;
	OV5646MIPI_mapMiddlewaresizePointToPreviewsizePoint(x0,y0,FD_XS,FD_YS,&pvx0, &pvy0, OV5646MIPI_PRV_W, OV5646MIPI_PRV_H);
	OV5646MIPI_mapMiddlewaresizePointToPreviewsizePoint(x1,y1,FD_XS,FD_YS,&pvx1, &pvy1, OV5646MIPI_PRV_W, OV5646MIPI_PRV_H);
    
	//2.sensor AE line and row coordinate
	OV5646MIPI_calcLine();
	OV5646MIPI_calcRow();

	//3.calc left up point to section
	//UINT32 linenum, rownum;
	OV5646MIPI_calcPointsAELineRowCoordinate(pvx0,pvy0,&linenum,&rownum);    
	//UINT32 leftuplinenum,leftuprownum;
	OV5646MIPI_mapCoordinate(linenum, rownum, &leftuplinenum, &leftuprownum);
	//SENSORDB("leftuplinenum=%d,leftuprownum=%d\n",leftuplinenum,leftuprownum);

	//4.calc right bottom point to section
	OV5646MIPI_calcPointsAELineRowCoordinate(pvx1,pvy1,&linenum,&rownum);    
	//UINT32 rightbottomlinenum,rightbottomrownum;
	OV5646MIPI_mapCoordinate(linenum, rownum, &rightbottomlinenum, &rightbottomrownum);
	//SENSORDB("rightbottomlinenum=%d,rightbottomrownum=%d\n",rightbottomlinenum,rightbottomrownum);

	//5.update global section array
	OV5646MIPI_mapRectToAE_2_ARRAY(leftuplinenum, leftuprownum, rightbottomlinenum, rightbottomrownum);
	//print_AE_section();

	//6.write to reg
	OV5646MIPI_mapAE_2_ARRAY_To_AE_1_ARRAY();
	//OV5646MIPI_printAE_1_ARRAY();
	OV5646MIPI_printAE_2_ARRAY();
	OV5646MIPI_writeAEReg();
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_Set_AE_Window function\n");
}
//=====================touch AE end==========================//


static void OV5646MIPIinitalvariable()
{
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.VideoMode = KAL_FALSE;
	OV5646MIPISensor.NightMode = KAL_FALSE;
	OV5646MIPISensor.Fps = 100;
	OV5646MIPISensor.ShutterStep= 0xde;
	OV5646MIPISensor.CaptureDummyPixels = 0;
	OV5646MIPISensor.CaptureDummyLines = 0;
	OV5646MIPISensor.PreviewDummyPixels = 0;
	OV5646MIPISensor.PreviewDummyLines = 0;
	OV5646MIPISensor.SensorMode= SENSOR_MODE_INIT;
	OV5646MIPISensor.IsPVmode= KAL_TRUE;	
	OV5646MIPISensor.PreviewPclk= 560;
	OV5646MIPISensor.CapturePclk= 900;
	OV5646MIPISensor.ZsdturePclk= 900;
	OV5646MIPISensor.PreviewShutter=0x5c4; 
	OV5646MIPISensor.PreviewExtraShutter=0x00; 
	OV5646MIPISensor.SensorGain=0x38;
	OV5646MIPISensor.manualAEStart=0;
	OV5646MIPISensor.isoSpeed=AE_ISO_100;
	OV5646MIPISensor.userAskAeLock=KAL_FALSE;
	OV5646MIPISensor.userAskAwbLock=KAL_FALSE;
	OV5646MIPISensor.currentExposureTime=0;
	OV5646MIPISensor.currentShutter=0;
	OV5646MIPISensor.zsd_flag=0;
	OV5646MIPISensor.currentextshutter=0;
	OV5646MIPISensor.AF_window_x=0;
	OV5646MIPISensor.AF_window_y=0;
	OV5646MIPISensor.awbMode = AWB_MODE_AUTO;
	OV5646MIPISensor.iWB=AWB_MODE_AUTO;
	spin_unlock(&ov5646mipi_drv_lock);
}
void OV5646MIPIGetExifInfo(UINT32 exifAddr)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIGetExifInfo function\n");
	SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
	pExifInfo->FNumber = 20;
	pExifInfo->AEISOSpeed = OV5646MIPISensor.isoSpeed;
	pExifInfo->AWBMode = OV5646MIPISensor.awbMode;
	pExifInfo->FlashLightTimeus = 0;
	pExifInfo->RealISOValue = OV5646MIPISensor.isoSpeed;
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIGetExifInfo function\n");
}
static void OV5646MIPISetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines)
{
        OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPISetDummy function:dummy_pixels=%d,dummy_lines=%d\n",dummy_pixels,dummy_lines);
        if (OV5646MIPISensor.IsPVmode)  
        {
            dummy_pixels = dummy_pixels+OV5646MIPI_PV_PERIOD_PIXEL_NUMS; 
            OV5646MIPI_write_cmos_sensor(0x380D,( dummy_pixels&0xFF));         
            OV5646MIPI_write_cmos_sensor(0x380C,(( dummy_pixels&0xFF00)>>8)); 
      
            dummy_lines= dummy_lines+OV5646MIPI_PV_PERIOD_LINE_NUMS; 
            OV5646MIPI_write_cmos_sensor(0x380F,(dummy_lines&0xFF));       
            OV5646MIPI_write_cmos_sensor(0x380E,((dummy_lines&0xFF00)>>8));  
        } 
        else
        {
            dummy_pixels = dummy_pixels+OV5646MIPI_FULL_PERIOD_PIXEL_NUMS; 
            OV5646MIPI_write_cmos_sensor(0x380D,( dummy_pixels&0xFF));         
            OV5646MIPI_write_cmos_sensor(0x380C,(( dummy_pixels&0xFF00)>>8)); 
      
            dummy_lines= dummy_lines+OV5646MIPI_FULL_PERIOD_LINE_NUMS; 
            OV5646MIPI_write_cmos_sensor(0x380F,(dummy_lines&0xFF));       
            OV5646MIPI_write_cmos_sensor(0x380E,((dummy_lines&0xFF00)>>8));  
        }
        OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPISetDummy function:\n ");
}    /* OV5646MIPI_set_dummy */

/*************************************************************************
* FUNCTION
*	OV5646MIPIWriteShutter
*
* DESCRIPTION
*	This function used to write the shutter.
*
* PARAMETERS
*	1. kal_uint32 : The shutter want to apply to sensor.
*
* RETURNS
*	None
*
*************************************************************************/
static void OV5646MIPIWriteShutter(kal_uint32 shutter)
{
	kal_uint32 extra_exposure_vts = 0;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIWriteShutter function:shutter=%d\n ",shutter);
	if (shutter < 1)
	{
		shutter = 1;
	}
	if (shutter > OV5646MIPI_FULL_EXPOSURE_LIMITATION) 
	{
		extra_exposure_vts =shutter+4;
		OV5646MIPI_write_cmos_sensor(0x380f, extra_exposure_vts & 0xFF);          // EXVTS[b7~b0]
		OV5646MIPI_write_cmos_sensor(0x380e, (extra_exposure_vts & 0xFF00) >> 8); // EXVTS[b15~b8]
		OV5646MIPI_write_cmos_sensor(0x350D,0x00); 
		OV5646MIPI_write_cmos_sensor(0x350C,0x00);
	}
	shutter*=16;
	OV5646MIPI_write_cmos_sensor(0x3502, (shutter & 0x00FF));           //AEC[7:0]
	OV5646MIPI_write_cmos_sensor(0x3501, ((shutter & 0x0FF00) >>8));  //AEC[15:8]
	OV5646MIPI_write_cmos_sensor(0x3500, ((shutter & 0xFF0000) >> 16));	
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIWriteShutter function:\n ");
}    /* OV5646MIPI_write_shutter */
/*************************************************************************
* FUNCTION
*	OV5646MIPIExpWriteShutter
*
* DESCRIPTION
*	This function used to write the shutter.
*
* PARAMETERS
*	1. kal_uint32 : The shutter want to apply to sensor.
*
* RETURNS
*	None
*
*************************************************************************/
static void OV5646MIPIWriteExpShutter(kal_uint32 shutter)
{
	shutter*=16;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIWriteExpShutter function:shutter=%d\n ",shutter);
	OV5646MIPI_write_cmos_sensor(0x3502, (shutter & 0x00FF));           //AEC[7:0]
	OV5646MIPI_write_cmos_sensor(0x3501, ((shutter & 0x0FF00) >>8));  //AEC[15:8]
	OV5646MIPI_write_cmos_sensor(0x3500, ((shutter & 0xFF0000) >> 16));	
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIWriteExpShutter function:\n ");
}    /* OV5646MIPI_write_shutter */

/*************************************************************************
* FUNCTION
*	OV5646MIPIExtraWriteShutter
*
* DESCRIPTION
*	This function used to write the shutter.
*
* PARAMETERS
*	1. kal_uint32 : The shutter want to apply to sensor.
*
* RETURNS
*	None
*
*************************************************************************/
static void OV5646MIPIWriteExtraShutter(kal_uint32 shutter)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIWriteExtraShutter function:shutter=%d\n ",shutter);
	OV5646MIPI_write_cmos_sensor(0x350D, shutter & 0xFF);          // EXVTS[b7~b0]
	OV5646MIPI_write_cmos_sensor(0x350C, (shutter & 0xFF00) >> 8); // EXVTS[b15~b8]
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIWriteExtraShutter function:\n ");
}    /* OV5646MIPI_write_shutter */

/*************************************************************************
* FUNCTION
*	OV5646MIPIWriteSensorGain
*
* DESCRIPTION
*	This function used to write the sensor gain.
*
* PARAMETERS
*	1. kal_uint32 : The sensor gain want to apply to sensor.
*
* RETURNS
*	None
*
*************************************************************************/
static void OV5646MIPIWriteSensorGain(kal_uint32 gain)
{
	kal_uint16 temp_reg ;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIWriteSensorGain function:gain=%d\n",gain);
	if(gain > 1024)  ASSERT(0);
	temp_reg = 0;
	temp_reg=gain&0x0FF;	
	OV5646MIPI_write_cmos_sensor(0x350B,temp_reg);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIWriteSensorGain function:\n ");
}  /* OV5646MIPI_write_sensor_gain */

/*************************************************************************
* FUNCTION
*	OV5646MIPIReadShutter
*
* DESCRIPTION
*	This function read current shutter for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current shutter value.
*
*************************************************************************/
static kal_uint32 OV5646MIPIReadShutter(void)
{
	kal_uint16 temp_reg1, temp_reg2 ,temp_reg3;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIReadShutter function:\n ");
	temp_reg1 = OV5646MIPIYUV_read_cmos_sensor(0x3500);    // AEC[b19~b16]
	temp_reg2 = OV5646MIPIYUV_read_cmos_sensor(0x3501);    // AEC[b15~b8]
	temp_reg3 = OV5646MIPIYUV_read_cmos_sensor(0x3502);    // AEC[b7~b0]

	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.PreviewShutter  = (temp_reg1 <<12)| (temp_reg2<<4)|(temp_reg3>>4);
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIReadShutter function:\n ");	
	return OV5646MIPISensor.PreviewShutter;
} /* OV5646MIPI_read_shutter */

/*************************************************************************
* FUNCTION
*	OV5646MIPIReadExtraShutter
*
* DESCRIPTION
*	This function read current shutter for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current shutter value.
*
*************************************************************************/
static kal_uint32 OV5646MIPIReadExtraShutter(void)
{
	kal_uint16 temp_reg1, temp_reg2 ;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIReadExtraShutter function:\n ");
	temp_reg1 = OV5646MIPIYUV_read_cmos_sensor(0x350c);    // AEC[b15~b8]
	temp_reg2 = OV5646MIPIYUV_read_cmos_sensor(0x350d);    // AEC[b7~b0]
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.PreviewExtraShutter  = ((temp_reg1<<8)| temp_reg2);
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIReadExtraShutter function:\n ");		
	return OV5646MIPISensor.PreviewExtraShutter;
} /* OV5646MIPI_read_shutter */
/*************************************************************************
* FUNCTION
*	OV5646MIPIReadSensorGain
*
* DESCRIPTION
*	This function read current sensor gain for calculate the exposure.
*
* PARAMETERS
*	None
*
* RETURNS
*	kal_uint16 : The current sensor gain value.
*
*************************************************************************/
static kal_uint32 OV5646MIPIReadSensorGain(void)
{
	kal_uint32 sensor_gain = 0;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIReadSensorGain function:\n ");
	sensor_gain=(OV5646MIPIYUV_read_cmos_sensor(0x350B)&0xFF); 
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIReadSensorGain function:\n ");
	return sensor_gain;
}  /* OV5646MIPIReadSensorGain */

void OV5646MIPI_SetShutter(kal_uint32 iShutter)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_SetShutter function:iShutter == 0x%x\n",iShutter);
	kal_uint32 extra_exposure_vts = 0;

	if (iShutter < 1)
	{
		iShutter = 1;
	}
	if (iShutter > OV5646MIPI_FULL_EXPOSURE_LIMITATION)
	{
		extra_exposure_vts =iShutter+4;
		OV5646MIPI_write_cmos_sensor(0x380f, extra_exposure_vts & 0xFF);          // EXVTS[b7~b0]
		OV5646MIPI_write_cmos_sensor(0x380e, (extra_exposure_vts & 0xFF00) >> 8); // EXVTS[b15~b8]
		OV5646MIPI_write_cmos_sensor(0x350D,0x00);
		OV5646MIPI_write_cmos_sensor(0x350C,0x00);
	}
	iShutter*=16;
	OV5646MIPI_write_cmos_sensor(0x3502, (iShutter & 0x00FF));           //AEC[7:0]
	OV5646MIPI_write_cmos_sensor(0x3501, ((iShutter & 0x0FF00) >>8));  //AEC[15:8]
	OV5646MIPI_write_cmos_sensor(0x3500, ((iShutter & 0xFF0000) >> 16));
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_SetShutter function:\n ");
}

static void OV5646MIPI_SetGain(kal_uint32 iGain)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_SetGain function:iGain == 0x%x\n",iGain);
	kal_uint16 temp_reg ;
	if(iGain > 1024)  ASSERT(0);
	temp_reg = 0;
	temp_reg=iGain&0x0FF;
	OV5646MIPI_write_cmos_sensor(0x350B,temp_reg);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_SetGain function:\n ");
}

void OV5646MIPI_GetAEFlashlightInfo(UINT32 infoAddr)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_GetAEFlashlightInfo function:\n ");
	SENSOR_FLASHLIGHT_AE_INFO_STRUCT* pAeInfo = (SENSOR_FLASHLIGHT_AE_INFO_STRUCT*) infoAddr;

	pAeInfo->Exposuretime = OV5646MIPIReadShutter();
	pAeInfo->Gain = OV5646MIPIReadSensorGain();
	pAeInfo->u4Fno = 28;
	pAeInfo->GAIN_BASE = 50;
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_GetAEFlashlightInfo function:\n ");
}

/*************************************************************************
* FUNCTION
*	OV5646MIPI_set_AE_mode
*
* DESCRIPTION
*	This function OV5646MIPI_set_AE_mode.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void OV5646MIPI_set_AE_mode(kal_bool AE_enable)
{
	kal_uint8 AeTemp;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_AE_mode function:\n ");
	AeTemp = OV5646MIPIYUV_read_cmos_sensor(0x3503);
	if (AE_enable == KAL_TRUE)
	{
		// turn on AEC/AGC
		OV5646MIPI_write_cmos_sensor(0x3503,(AeTemp&(~0x07)));
	}
	else
	{
		// turn off AEC/AGC
		OV5646MIPI_write_cmos_sensor(0x3503,(AeTemp|0x07));
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_AE_mode function:\n ");
}

/*************************************************************************
* FUNCTION
*	OV5646MIPI_set_AWB_mode
*
* DESCRIPTION
*	This function OV5646MIPI_set_AWB_mode.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void OV5646MIPI_set_AWB_mode(kal_bool AWB_enable)
{
	kal_uint8 AwbTemp;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_AWB_mode function:\n ");
	AwbTemp = OV5646MIPIYUV_read_cmos_sensor(0x3406);   

	if (AWB_enable == KAL_TRUE)
	{
		OV5646MIPI_write_cmos_sensor(0x3406,AwbTemp&0xFE);
	}
	else
	{             
		OV5646MIPI_write_cmos_sensor(0x3406,AwbTemp|0x01);
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_AWB_mode function:\n ");
}

static void OV5646MIPI_set_AWB_mode_UNLOCK()
{
	OV5646MIPI_set_AWB_mode(KAL_TRUE);
	if (!((SCENE_MODE_OFF == OV5646MIPISensor.sceneMode) || (SCENE_MODE_NORMAL == OV5646MIPISensor.sceneMode) || (SCENE_MODE_HDR == OV5646MIPISensor.sceneMode)))
	{
		OV5646MIPI_set_scene_mode(OV5646MIPISensor.sceneMode);        
	}
	if (!((AWB_MODE_OFF == OV5646MIPISensor.iWB) || (AWB_MODE_AUTO == OV5646MIPISensor.iWB)))
	{
		OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_AWB_mode_UNLOCK function:iWB=%d\n ",OV5646MIPISensor.iWB);
		OV5646MIPI_set_param_wb(OV5646MIPISensor.iWB);
	}
	return;
}

/*************************************************************************
* FUNCTION
*	OV5646MIPI_GetSensorID
*
* DESCRIPTION
*	This function get the sensor ID
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
//static 
kal_uint32 OV5646MIPI_GetSensorID(kal_uint32 *sensorID)
{
    volatile signed char i;
	kal_uint32 sensor_id=0;
	kal_uint8 temp_sccb_addr = 0;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_GetSensorID function:\n ");
	OV5646MIPI_write_cmos_sensor(0x3008,0x82);// Reset sensor
	mDELAY(5);
	for(i=0;i<3;i++)
	{
		sensor_id = (OV5646MIPIYUV_read_cmos_sensor(0x300A) << 8) | OV5646MIPIYUV_read_cmos_sensor(0x300B);
		OV5646MIPISENSORDB("OV5646MIPI READ ID: %x",sensor_id);
		if(sensor_id != OV5645MIPI_SENSOR_ID)
		{
			*sensorID =0xffffffff;
			return ERROR_SENSOR_CONNECT_FAIL;
		}
		else if (OV5646MIPIYUV_read_otp()) //read mid, darling ff modules mid is 0x05 and lens_id is 0x00.
		{
		        OV5646MIPISENSORDB("[OV5646MIPI]This is darling modules.");
			*sensorID=OV5646MIPI_SENSOR_ID;
		        break;
		}
		else
		{
		        OV5646MIPISENSORDB("[OV5646MIPI]This is other modules.");
			*sensorID =0xffffffff;
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_GetSensorID function:\n ");
	return ERROR_NONE;    
}

UINT32 OV5646SetTestPatternMode(kal_bool bEnable)
{
	OV5646MIPISENSORDB("[OV5646MIPI_OV5646SetTestPatternMode]test pattern bEnable:=%d\n",bEnable);
	if(bEnable)
	{
		OV5646MIPI_write_cmos_sensor(0x503d,0x80);
		OV5646MIPI_run_test_potten=1;
	}
	else
	{
		OV5646MIPI_write_cmos_sensor(0x503d,0x00);
		OV5646MIPI_run_test_potten=0;
	}
	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*    OV5646MIPIInitialSetting
*
* DESCRIPTION
*    This function initialize the registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
//static 
void OV5646MIPIInitialSetting(void)
{
	//;OV5646MIPI 1280x960,30fps
	//56Mhz, 224Mbps/Lane, 2 Lane
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIInitialSetting function:\n ");
	OV5646MIPI_write_cmos_sensor(0x3103, 0x11);//	; PLL clock selection
	OV5646MIPI_write_cmos_sensor(0x3008, 0x82);//	; software reset	
	mDELAY(5);            										//; delay 2ms                           					
	OV5646MIPI_write_cmos_sensor(0x3008, 0x42);//	; software power down
	OV5646MIPI_write_cmos_sensor(0x3103, 0x03);//	; clock from PLL
	OV5646MIPI_write_cmos_sensor(0x3503, 0x07);//	; AGC manual, AEC manual
	OV5646MIPI_write_cmos_sensor(0x3406, 0x01);//	; awb manual, 	
	OV5646MIPI_write_cmos_sensor(0x3000, 0x30);
	OV5646MIPI_write_cmos_sensor(0x3004, 0xef);
	OV5646MIPI_write_cmos_sensor(0x3002, 0x1c);//	; system reset
	OV5646MIPI_write_cmos_sensor(0x3006, 0xc3);//	; clock enable
	OV5646MIPI_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5646MIPI_write_cmos_sensor(0x3017, 0x40);//	; Frex, CSK input, Vsync output
	OV5646MIPI_write_cmos_sensor(0x3018, 0x00);//	; GPIO input
	OV5646MIPI_write_cmos_sensor(0x302c, 0x02);//	; GPIO input
	OV5646MIPI_write_cmos_sensor(0x302e, 0x0b);//
	OV5646MIPI_write_cmos_sensor(0x3031, 0x00);//	zhouliao:from 0x08	//
	OV5646MIPI_write_cmos_sensor(0x3611, 0x06);//   laimingji:from 0x06, low light noise 0xc6
	OV5646MIPI_write_cmos_sensor(0x3612, 0xab);//   laimingji:from 0xab, low light noise 0x2b
	OV5646MIPI_write_cmos_sensor(0x3614, 0x50);//
	OV5646MIPI_write_cmos_sensor(0x3618, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x4800, 0x04);//chage mipi data free/gate
	OV5646MIPI_write_cmos_sensor(0x3034, 0x18);//	; PLL, MIPI 8-bit mode
	OV5646MIPI_write_cmos_sensor(0x3035, 0x21);//	; PLL
	OV5646MIPI_write_cmos_sensor(0x3036, 0x70);//	; PLL
	OV5646MIPI_write_cmos_sensor(0x3037, 0x13); // ; PLL
	OV5646MIPI_write_cmos_sensor(0x3108, 0x01); // ; PLL
	OV5646MIPI_write_cmos_sensor(0x3824, 0x01); // ; PLL
	OV5646MIPI_write_cmos_sensor(0x460c, 0x20); // ; PLL
	OV5646MIPI_write_cmos_sensor(0x3400, 0x05);// r
	OV5646MIPI_write_cmos_sensor(0x3401, 0x42); 
	OV5646MIPI_write_cmos_sensor(0x3402, 0x04);// g
	OV5646MIPI_write_cmos_sensor(0x3403, 0x00); 
	OV5646MIPI_write_cmos_sensor(0x3404, 0x07);// b
	OV5646MIPI_write_cmos_sensor(0x3405, 0x60); 
	OV5646MIPI_write_cmos_sensor(0x3600, 0x09);//
	OV5646MIPI_write_cmos_sensor(0x3601, 0x43);//
	OV5646MIPI_write_cmos_sensor(0x3620, 0x33);//
	OV5646MIPI_write_cmos_sensor(0x3621, 0xe0);//
	OV5646MIPI_write_cmos_sensor(0x3622, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x3630, 0x2d);//
	OV5646MIPI_write_cmos_sensor(0x3631, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x3632, 0x32);//
	OV5646MIPI_write_cmos_sensor(0x3633, 0x52);//
	OV5646MIPI_write_cmos_sensor(0x3634, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x3635, 0x13);//
	OV5646MIPI_write_cmos_sensor(0x3636, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x3702, 0x6e);//
	OV5646MIPI_write_cmos_sensor(0x3703, 0x52);//
	OV5646MIPI_write_cmos_sensor(0x3704, 0xa0);//
	OV5646MIPI_write_cmos_sensor(0x3705, 0x33);//
	OV5646MIPI_write_cmos_sensor(0x3708, 0x66);//
	OV5646MIPI_write_cmos_sensor(0x3709, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x370b, 0x61);//
	OV5646MIPI_write_cmos_sensor(0x370c, 0xc3);//
	OV5646MIPI_write_cmos_sensor(0x370f, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x3715, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x3717, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x371b, 0x20);//
	OV5646MIPI_write_cmos_sensor(0x3731, 0x22);//
	OV5646MIPI_write_cmos_sensor(0x3739, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x3901, 0x0a);//
	OV5646MIPI_write_cmos_sensor(0x3905, 0x02);//
	OV5646MIPI_write_cmos_sensor(0x3906, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x3719, 0x86);//
	OV5646MIPI_write_cmos_sensor(0x3800, 0x00);//	; HS = 0
	OV5646MIPI_write_cmos_sensor(0x3801, 0x00);//	; HS
	OV5646MIPI_write_cmos_sensor(0x3802, 0x00);//	; VS = 250
	OV5646MIPI_write_cmos_sensor(0x3803, 0x06);//	; VS
	OV5646MIPI_write_cmos_sensor(0x3804, 0x0a);//	; HW = 2623
	OV5646MIPI_write_cmos_sensor(0x3805, 0x3f);//	; HW
	OV5646MIPI_write_cmos_sensor(0x3806, 0x07);//	; VH = 1705
	OV5646MIPI_write_cmos_sensor(0x3807, 0x9d);//	; VH
	OV5646MIPI_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
	OV5646MIPI_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
	OV5646MIPI_write_cmos_sensor(0x380a, 0x03);//	; DVPHO
	OV5646MIPI_write_cmos_sensor(0x380b, 0xc0);//	; DVPVO
	OV5646MIPI_write_cmos_sensor(0x380c, 0x07);//	; HTS = 2160
	OV5646MIPI_write_cmos_sensor(0x380d, 0x68);//	; HTS
	OV5646MIPI_write_cmos_sensor(0x380e, 0x03);//	; VTS = 740
	OV5646MIPI_write_cmos_sensor(0x380f, 0xd8);//	; VTS
	OV5646MIPI_write_cmos_sensor(0x3810, 0x00);//	; H OFF = 16
	OV5646MIPI_write_cmos_sensor(0x3811, 0x10);//	; H OFF
	OV5646MIPI_write_cmos_sensor(0x3812, 0x00);//	; V OFF = 4
	OV5646MIPI_write_cmos_sensor(0x3813, 0x06);//	; V OFF
	OV5646MIPI_write_cmos_sensor(0x3814, 0x31);//	; X INC
	OV5646MIPI_write_cmos_sensor(0x3815, 0x31);//	; Y INC
	OV5646MIPI_write_cmos_sensor(0x3820, 0x47);//	; flip off, V bin on
	OV5646MIPI_write_cmos_sensor(0x3821, 0x01);//	; mirror on, H bin on
	OV5646MIPI_write_cmos_sensor(0x3826, 0x03); // 
	OV5646MIPI_write_cmos_sensor(0x3828, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x3a02, 0x03);//	; max exp 60 = 740
	OV5646MIPI_write_cmos_sensor(0x3a03, 0xd8);//	; max exp 60
	OV5646MIPI_write_cmos_sensor(0x3a08, 0x01);//	; B50 = 222
	OV5646MIPI_write_cmos_sensor(0x3a09, 0x27); // ; B50
	OV5646MIPI_write_cmos_sensor(0x3a0a, 0x00); // ; B60 = 185
	OV5646MIPI_write_cmos_sensor(0x3a0b, 0xf6); // ; B60
	OV5646MIPI_write_cmos_sensor(0x3a0e, 0x03); // ; max 50
	OV5646MIPI_write_cmos_sensor(0x3a0d, 0x04); // ; max 60
	OV5646MIPI_write_cmos_sensor(0x3a14, 0x03); // ; max exp 50 = 740
	OV5646MIPI_write_cmos_sensor(0x3a15, 0xd8);//	; max exp 50
	OV5646MIPI_write_cmos_sensor(0x3a18, 0x00);//	; gain ceiling = 15.5x
	OV5646MIPI_write_cmos_sensor(0x3a19, 0x60);//	; gain ceiling
	OV5646MIPI_write_cmos_sensor(0x3a05, 0x30);//	; enable band insert, ken,  
	OV5646MIPI_write_cmos_sensor(0x3c01, 0xb4); // ;manual banding mode
	OV5646MIPI_write_cmos_sensor(0x3c00, 0x04); // ;50 Banding mode 
	OV5646MIPI_write_cmos_sensor(0x3c04, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x3c05, 0x98);//
	OV5646MIPI_write_cmos_sensor(0x3c07, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x3c08, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x3c09, 0xc2);//
	OV5646MIPI_write_cmos_sensor(0x3c0a, 0x9c);//
	OV5646MIPI_write_cmos_sensor(0x3c0b, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x4001, 0x02);//	; BLC start line
	OV5646MIPI_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
	OV5646MIPI_write_cmos_sensor(0x4005, 0x18);//	; BLC update triggered by gain change
	OV5646MIPI_write_cmos_sensor(0x4050, 0x6e);//	; BLC line number
	OV5646MIPI_write_cmos_sensor(0x4051, 0x8f);//	; BLC update triggered by gain change
	OV5646MIPI_write_cmos_sensor(0x4300, 0x30);//	; YUV 422, YUYV
	OV5646MIPI_write_cmos_sensor(0x4514, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x4520, 0xb0);//
	OV5646MIPI_write_cmos_sensor(0x460b, 0x37);//
	OV5646MIPI_write_cmos_sensor(0x4818, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x481d, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x481f, 0x50);//
	OV5646MIPI_write_cmos_sensor(0x4823, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x4831, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x4837, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5000, 0xa7);//	; Lenc on, raw gamma on, BPC on, WPC on, color interpolation on
	OV5646MIPI_write_cmos_sensor(0x5001, 0xa3);//	; SDE on, scale off, UV adjust off, color matrix on, AWB on
	OV5646MIPI_write_cmos_sensor(0x5002, 0x80);//   
	OV5646MIPI_write_cmos_sensor(0x501f, 0x00);//	; select ISP YUV 422
	OV5646MIPI_write_cmos_sensor(0x503d, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x505c, 0x30);//
	OV5646MIPI_write_cmos_sensor(0x5181, 0x59);//
	OV5646MIPI_write_cmos_sensor(0x5183, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5191, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5192, 0x03);//    
	OV5646MIPI_write_cmos_sensor(0x5684, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x5685, 0xa0);//
	OV5646MIPI_write_cmos_sensor(0x5686, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x5687, 0x78);//
	OV5646MIPI_write_cmos_sensor(0x5a00, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x5a21, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5a24, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x3008, 0x02);//	; wake up from software standby

#if 0
	//AWB For darling modules
	OV5646MIPI_write_cmos_sensor(0x5180, 0xff);//awb
	OV5646MIPI_write_cmos_sensor(0x5181, 0xf2);//
	OV5646MIPI_write_cmos_sensor(0x5182, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5183, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5184, 0x25);//
	OV5646MIPI_write_cmos_sensor(0x5185, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5186, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x5187, 0x15);//
	OV5646MIPI_write_cmos_sensor(0x5188, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x5189, 0x7b);//0x7b
	OV5646MIPI_write_cmos_sensor(0x518a, 0x5c);//0x5c
	OV5646MIPI_write_cmos_sensor(0x518b, 0xc3);//
	OV5646MIPI_write_cmos_sensor(0x518c, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x518d, 0x3f);//
	OV5646MIPI_write_cmos_sensor(0x518e, 0x30);//
	OV5646MIPI_write_cmos_sensor(0x518f, 0x57);//
	OV5646MIPI_write_cmos_sensor(0x5190, 0x3d);//
	OV5646MIPI_write_cmos_sensor(0x5191, 0xf8);//
	OV5646MIPI_write_cmos_sensor(0x5192, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5193, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x5194, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5195, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5196, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5197, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5198, 0x05);//
	OV5646MIPI_write_cmos_sensor(0x5199, 0x1a);//
	OV5646MIPI_write_cmos_sensor(0x519a, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519b, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x519c, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519d, 0xb2);//
	OV5646MIPI_write_cmos_sensor(0x519e, 0x38);//
#else
	//AWB For darling modules
/*
	OV5646MIPI_write_cmos_sensor(0x5180, 0xff);//awb
	OV5646MIPI_write_cmos_sensor(0x5181, 0xf6);//
	OV5646MIPI_write_cmos_sensor(0x5182, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5183, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5184, 0x25);//
	OV5646MIPI_write_cmos_sensor(0x5185, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5186, 0x0d);//
	OV5646MIPI_write_cmos_sensor(0x5187, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5188, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x5189, 0x77);//0x7b
	OV5646MIPI_write_cmos_sensor(0x518a, 0x5a);//0x5c
	OV5646MIPI_write_cmos_sensor(0x518b, 0x9f);//
	OV5646MIPI_write_cmos_sensor(0x518c, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x518d, 0x3d);//
	OV5646MIPI_write_cmos_sensor(0x518e, 0x30);//
	OV5646MIPI_write_cmos_sensor(0x518f, 0x50);//
	OV5646MIPI_write_cmos_sensor(0x5190, 0x3d);//
	OV5646MIPI_write_cmos_sensor(0x5191, 0xf8);//
	OV5646MIPI_write_cmos_sensor(0x5192, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5193, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x5194, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5195, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5196, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5197, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5198, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5199, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x519a, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519b, 0x79);//
	OV5646MIPI_write_cmos_sensor(0x519c, 0x09);//
	OV5646MIPI_write_cmos_sensor(0x519d, 0x2a);//
	OV5646MIPI_write_cmos_sensor(0x519e, 0x38);//

	OV5646MIPI_write_cmos_sensor(0x5180, 0xff);//awb
	OV5646MIPI_write_cmos_sensor(0x5181, 0xf2);//
	OV5646MIPI_write_cmos_sensor(0x5182, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5183, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5184, 0x25);//
	OV5646MIPI_write_cmos_sensor(0x5185, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5186, 0x0d);//
	OV5646MIPI_write_cmos_sensor(0x5187, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5188, 0x0d);//
	OV5646MIPI_write_cmos_sensor(0x5189, 0x75);//0x75
	OV5646MIPI_write_cmos_sensor(0x518a, 0x57);//
	OV5646MIPI_write_cmos_sensor(0x518b, 0xcc);//0xcc
	OV5646MIPI_write_cmos_sensor(0x518c, 0x81);//
	OV5646MIPI_write_cmos_sensor(0x518d, 0x3b);//
	OV5646MIPI_write_cmos_sensor(0x518e, 0x31);//
	OV5646MIPI_write_cmos_sensor(0x518f, 0x52);//
	OV5646MIPI_write_cmos_sensor(0x5190, 0x3d);//
	OV5646MIPI_write_cmos_sensor(0x5191, 0xf8);//
	OV5646MIPI_write_cmos_sensor(0x5192, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5193, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x5194, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5195, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5196, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5197, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5198, 0x05);//
	OV5646MIPI_write_cmos_sensor(0x5199, 0x52);//
	OV5646MIPI_write_cmos_sensor(0x519a, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519b, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x519c, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519d, 0xd9);//
	OV5646MIPI_write_cmos_sensor(0x519e, 0x38);//
*/
	OV5646MIPI_write_cmos_sensor(0x5180, 0xff);//awb
	OV5646MIPI_write_cmos_sensor(0x5181, 0xf2);//
	OV5646MIPI_write_cmos_sensor(0x5182, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5183, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5184, 0x25);//
	OV5646MIPI_write_cmos_sensor(0x5185, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5186, 0x0e);//
	OV5646MIPI_write_cmos_sensor(0x5187, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5188, 0x0d);//
	OV5646MIPI_write_cmos_sensor(0x5189, 0x74);//0x75
	OV5646MIPI_write_cmos_sensor(0x518a, 0x54);//
	OV5646MIPI_write_cmos_sensor(0x518b, 0xd8);//0xcc
	OV5646MIPI_write_cmos_sensor(0x518c, 0x84);//
	OV5646MIPI_write_cmos_sensor(0x518d, 0x3b);//
	OV5646MIPI_write_cmos_sensor(0x518e, 0x31);//
	OV5646MIPI_write_cmos_sensor(0x518f, 0x52);//
	OV5646MIPI_write_cmos_sensor(0x5190, 0x3d);//
	OV5646MIPI_write_cmos_sensor(0x5191, 0xf8);//
	OV5646MIPI_write_cmos_sensor(0x5192, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5193, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x5194, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5195, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5196, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5197, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5198, 0x05);//
	OV5646MIPI_write_cmos_sensor(0x5199, 0x18);//
	OV5646MIPI_write_cmos_sensor(0x519a, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519b, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x519c, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x519d, 0x33);//
	OV5646MIPI_write_cmos_sensor(0x519e, 0x38);//
/*
	OV5646MIPI_write_cmos_sensor(0x5180, 0xff);//awb
	OV5646MIPI_write_cmos_sensor(0x5181, 0x50);//
	OV5646MIPI_write_cmos_sensor(0x5182, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5183, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5184, 0x25);//
	OV5646MIPI_write_cmos_sensor(0x5185, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5186, 0x0e);// 
	OV5646MIPI_write_cmos_sensor(0x5187, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5188, 0x0d);// 
	OV5646MIPI_write_cmos_sensor(0x5189, 0x75);//0x75
	OV5646MIPI_write_cmos_sensor(0x518a, 0x5e);//
	OV5646MIPI_write_cmos_sensor(0x518b, 0xb0);//0xcc
	OV5646MIPI_write_cmos_sensor(0x518c, 0x1b);//
	OV5646MIPI_write_cmos_sensor(0x518d, 0x3d);//
	OV5646MIPI_write_cmos_sensor(0x518e, 0x33);//
	OV5646MIPI_write_cmos_sensor(0x518f, 0x51);//
	OV5646MIPI_write_cmos_sensor(0x5190, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x5191, 0xf8);//
	OV5646MIPI_write_cmos_sensor(0x5192, 0x04);// 
	OV5646MIPI_write_cmos_sensor(0x5193, 0x70);//
	OV5646MIPI_write_cmos_sensor(0x5194, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5195, 0xf0);//
	OV5646MIPI_write_cmos_sensor(0x5196, 0x03);// 
	OV5646MIPI_write_cmos_sensor(0x5197, 0x01);// 
	OV5646MIPI_write_cmos_sensor(0x5198, 0x05);// 
	OV5646MIPI_write_cmos_sensor(0x5199, 0x3e);//
	OV5646MIPI_write_cmos_sensor(0x519a, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x519b, 0x00);// 
	OV5646MIPI_write_cmos_sensor(0x519c, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x519d, 0x4b);//
	OV5646MIPI_write_cmos_sensor(0x519e, 0x38);//
*/
#endif
        //CCM
	OV5646MIPI_write_cmos_sensor(0x5381, 0x21);//ccm
	OV5646MIPI_write_cmos_sensor(0x5382, 0x54);
	OV5646MIPI_write_cmos_sensor(0x5383, 0x15);
	OV5646MIPI_write_cmos_sensor(0x5384, 0x08);
	OV5646MIPI_write_cmos_sensor(0x5385, 0x75);
	OV5646MIPI_write_cmos_sensor(0x5386, 0x7D);
	OV5646MIPI_write_cmos_sensor(0x5387, 0x81);
	OV5646MIPI_write_cmos_sensor(0x5388, 0x74);
	OV5646MIPI_write_cmos_sensor(0x5389, 0x0D);
	OV5646MIPI_write_cmos_sensor(0x538a, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x538b, 0x98);//

	//From OV Kobe sharpness
	OV5646MIPI_write_cmos_sensor(0x5308, 0x35);
	OV5646MIPI_write_cmos_sensor(0x5300, 0x08);//	; sharpen MT th1
	OV5646MIPI_write_cmos_sensor(0x5301, 0x48);//	; sharpen MT th2
	OV5646MIPI_write_cmos_sensor(0x5302, 0x20);//	; sharpen MT off1
	OV5646MIPI_write_cmos_sensor(0x5303, 0x00);//	; sharpen MT off2
	OV5646MIPI_write_cmos_sensor(0x5304, 0x08);//	; DNS th1
	OV5646MIPI_write_cmos_sensor(0x5305, 0x30);//	; DNS th2
	OV5646MIPI_write_cmos_sensor(0x5306, 0x10);//	; DNS off1 //0x08 0x10
	OV5646MIPI_write_cmos_sensor(0x5307, 0x16);//	; DNS off2 //0x16 0x20
	OV5646MIPI_write_cmos_sensor(0x5309, 0x08);//	; sharpen TH th1
	OV5646MIPI_write_cmos_sensor(0x530a, 0x30);//	; sharpen TH th2
	OV5646MIPI_write_cmos_sensor(0x530b, 0x04);//	; sharpen TH th2
	OV5646MIPI_write_cmos_sensor(0x530c, 0x06);//	; sharpen TH off2

#if 0
        //gamma setting
	OV5646MIPI_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5646MIPI_write_cmos_sensor(0x5481, 0x0a);//	; Y yst 00
	OV5646MIPI_write_cmos_sensor(0x5482, 0x18);//
	OV5646MIPI_write_cmos_sensor(0x5483, 0x30);//
	OV5646MIPI_write_cmos_sensor(0x5484, 0x58);//
	OV5646MIPI_write_cmos_sensor(0x5485, 0x66);//
	OV5646MIPI_write_cmos_sensor(0x5486, 0x76);//
	OV5646MIPI_write_cmos_sensor(0x5487, 0x80);//
	OV5646MIPI_write_cmos_sensor(0x5488, 0x8b);//
	OV5646MIPI_write_cmos_sensor(0x5489, 0x95);//
	OV5646MIPI_write_cmos_sensor(0x548a, 0xa0);//
	OV5646MIPI_write_cmos_sensor(0x548b, 0xb0);//
	OV5646MIPI_write_cmos_sensor(0x548c, 0xbe);//
	OV5646MIPI_write_cmos_sensor(0x548d, 0xd0);//
	OV5646MIPI_write_cmos_sensor(0x548e, 0xe0);//
	OV5646MIPI_write_cmos_sensor(0x548f, 0xec);//
	OV5646MIPI_write_cmos_sensor(0x5490, 0x20);//
#else
	OV5646MIPI_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5646MIPI_write_cmos_sensor(0x5481, 0x08);//	; Y yst 00
	OV5646MIPI_write_cmos_sensor(0x5482, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5483, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x5484, 0x51);//
	OV5646MIPI_write_cmos_sensor(0x5485, 0x65);//
	OV5646MIPI_write_cmos_sensor(0x5486, 0x71);//
	OV5646MIPI_write_cmos_sensor(0x5487, 0x7d);//
	OV5646MIPI_write_cmos_sensor(0x5488, 0x87);//
	OV5646MIPI_write_cmos_sensor(0x5489, 0x91);//
	OV5646MIPI_write_cmos_sensor(0x548a, 0x9a);//
	OV5646MIPI_write_cmos_sensor(0x548b, 0xaa);//
	OV5646MIPI_write_cmos_sensor(0x548c, 0xb8);//
	OV5646MIPI_write_cmos_sensor(0x548d, 0xcd);//
	OV5646MIPI_write_cmos_sensor(0x548e, 0xdd);//
	OV5646MIPI_write_cmos_sensor(0x548f, 0xea);//
	OV5646MIPI_write_cmos_sensor(0x5490, 0x1d);//
/*
	OV5646MIPI_write_cmos_sensor(0x5480, 0x01);//	; bias on
	OV5646MIPI_write_cmos_sensor(0x5481, 0x0a);//	; Y yst 00
	OV5646MIPI_write_cmos_sensor(0x5482, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5483, 0x2e);//
	OV5646MIPI_write_cmos_sensor(0x5484, 0x57);//
	OV5646MIPI_write_cmos_sensor(0x5485, 0x69);//
	OV5646MIPI_write_cmos_sensor(0x5486, 0x7a);//
	OV5646MIPI_write_cmos_sensor(0x5487, 0x88);//
	OV5646MIPI_write_cmos_sensor(0x5488, 0x93);//
	OV5646MIPI_write_cmos_sensor(0x5489, 0x9f);//
	OV5646MIPI_write_cmos_sensor(0x548a, 0xa9);//
	OV5646MIPI_write_cmos_sensor(0x548b, 0xb7);//
	OV5646MIPI_write_cmos_sensor(0x548c, 0xc0);//
	OV5646MIPI_write_cmos_sensor(0x548d, 0xd1);//
	OV5646MIPI_write_cmos_sensor(0x548e, 0xdf);//
	OV5646MIPI_write_cmos_sensor(0x548f, 0xea);//
	OV5646MIPI_write_cmos_sensor(0x5490, 0x1d);//
*/
#endif
        //DPC laimingji 20130815
	OV5646MIPI_write_cmos_sensor(0x5780, 0xfc);//
	OV5646MIPI_write_cmos_sensor(0x5781, 0x13);//
	OV5646MIPI_write_cmos_sensor(0x5782, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5786, 0x20);//
	OV5646MIPI_write_cmos_sensor(0x5787, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x5788, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x5789, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x578a, 0x02);//
	OV5646MIPI_write_cmos_sensor(0x578b, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x578c, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x578d, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x578e, 0x02);//
	OV5646MIPI_write_cmos_sensor(0x578f, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5790, 0x01);//
#if 0
        //UV
	OV5646MIPI_write_cmos_sensor(0x5580, 0x06);//uv
	OV5646MIPI_write_cmos_sensor(0x5588, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5583, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x5584, 0x30);//
	OV5646MIPI_write_cmos_sensor(0x5589, 0x18);//
	OV5646MIPI_write_cmos_sensor(0x558a, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x558b, 0x88);//
#else
	OV5646MIPI_write_cmos_sensor(0x5580, 0x06);//uv
	OV5646MIPI_write_cmos_sensor(0x5588, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5583, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x5584, 0x20);//
	OV5646MIPI_write_cmos_sensor(0x5589, 0x18);//
	OV5646MIPI_write_cmos_sensor(0x558a, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x558b, 0x3c);//
#endif

#if 1
	//From OV Kobe DNP.
	OV5646MIPI_write_cmos_sensor(0x5800, 0x1C);//lsc
	OV5646MIPI_write_cmos_sensor(0x5801, 0x17);//
	OV5646MIPI_write_cmos_sensor(0x5802, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5803, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5804, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5805, 0x18);//
	OV5646MIPI_write_cmos_sensor(0x5806, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5807, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5808, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5809, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x580a, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x580b, 0x0F);//
	OV5646MIPI_write_cmos_sensor(0x580c, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x580d, 0x05);//
	OV5646MIPI_write_cmos_sensor(0x580e, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x580f, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5810, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5811, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5812, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x5813, 0x05);//
	OV5646MIPI_write_cmos_sensor(0x5814, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5815, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5816, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5817, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5818, 0x0C);//
	OV5646MIPI_write_cmos_sensor(0x5819, 0x0D);//
	OV5646MIPI_write_cmos_sensor(0x581a, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x581b, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x581c, 0x0B);//
	OV5646MIPI_write_cmos_sensor(0x581d, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x581e, 0x3E);//
	OV5646MIPI_write_cmos_sensor(0x581f, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5820, 0x15);//
	OV5646MIPI_write_cmos_sensor(0x5821, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5822, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5823, 0x19);//
	OV5646MIPI_write_cmos_sensor(0x5824, 0x46);//
	OV5646MIPI_write_cmos_sensor(0x5825, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x5826, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5827, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5828, 0x2A);//
	OV5646MIPI_write_cmos_sensor(0x5829, 0x2A);//
	OV5646MIPI_write_cmos_sensor(0x582a, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x582b, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x582c, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x582d, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x582e, 0x0C);//
	OV5646MIPI_write_cmos_sensor(0x582f, 0x22);//
	OV5646MIPI_write_cmos_sensor(0x5830, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x5831, 0x22);//
	OV5646MIPI_write_cmos_sensor(0x5832, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x5833, 0x2A);//
	OV5646MIPI_write_cmos_sensor(0x5834, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x5835, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5836, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5837, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5838, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5839, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x583a, 0x2A);//
	OV5646MIPI_write_cmos_sensor(0x583b, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x583c, 0x44);//
	OV5646MIPI_write_cmos_sensor(0x583d, 0xCE);//
/*
	//From OV Kobe office.
	OV5646MIPI_write_cmos_sensor(0x5800, 0x1C);//lsc
	OV5646MIPI_write_cmos_sensor(0x5801, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5802, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5803, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x5804, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5805, 0x1A);//
	OV5646MIPI_write_cmos_sensor(0x5806, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x5807, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5808, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5809, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x580a, 0x09);//
	OV5646MIPI_write_cmos_sensor(0x580b, 0x0F);//
	OV5646MIPI_write_cmos_sensor(0x580c, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x580d, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x580e, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x580f, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5810, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5811, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5812, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5813, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5814, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5815, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5816, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5817, 0x0B);//
	OV5646MIPI_write_cmos_sensor(0x5818, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x5819, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x581a, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x581b, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x581c, 0x0B);//
	OV5646MIPI_write_cmos_sensor(0x581d, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x581e, 0x1C);//
	OV5646MIPI_write_cmos_sensor(0x581f, 0x15);//
	OV5646MIPI_write_cmos_sensor(0x5820, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x5821, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5822, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5823, 0x20);//
	OV5646MIPI_write_cmos_sensor(0x5824, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x5825, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x5826, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5827, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x5828, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x5829, 0x43);//
	OV5646MIPI_write_cmos_sensor(0x582a, 0x21);//
	OV5646MIPI_write_cmos_sensor(0x582b, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x582c, 0x21);//
	OV5646MIPI_write_cmos_sensor(0x582d, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x582e, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x582f, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x5830, 0x81);//
	OV5646MIPI_write_cmos_sensor(0x5831, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x5832, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5833, 0x43);//
	OV5646MIPI_write_cmos_sensor(0x5834, 0x21);//
	OV5646MIPI_write_cmos_sensor(0x5835, 0x21);//
	OV5646MIPI_write_cmos_sensor(0x5836, 0x21);//
	OV5646MIPI_write_cmos_sensor(0x5837, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x5838, 0x41);//
	OV5646MIPI_write_cmos_sensor(0x5839, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x583a, 0x45);//
	OV5646MIPI_write_cmos_sensor(0x583b, 0x23);//
	OV5646MIPI_write_cmos_sensor(0x583c, 0x45);//
	OV5646MIPI_write_cmos_sensor(0x583d, 0xAF);//
*/
#else
/*
	//From OV Kobe D65.
	OV5646MIPI_write_cmos_sensor(0x5800, 0x1e);//lsc
	OV5646MIPI_write_cmos_sensor(0x5801, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5802, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5803, 0x15);//
	OV5646MIPI_write_cmos_sensor(0x5804, 0x1e);//
	OV5646MIPI_write_cmos_sensor(0x5805, 0x27);//
	OV5646MIPI_write_cmos_sensor(0x5806, 0x0f);//
	OV5646MIPI_write_cmos_sensor(0x5807, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x5808, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5809, 0x09);//
	OV5646MIPI_write_cmos_sensor(0x580a, 0x0f);//
	OV5646MIPI_write_cmos_sensor(0x580b, 0x13);//
	OV5646MIPI_write_cmos_sensor(0x580c, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x580d, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x580e, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x580f, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5810, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5811, 0x0e);//
	OV5646MIPI_write_cmos_sensor(0x5812, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x5813, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5814, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5815, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5816, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5817, 0x0e);//
	OV5646MIPI_write_cmos_sensor(0x5818, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x5819, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x581a, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x581b, 0x09);//
	OV5646MIPI_write_cmos_sensor(0x581c, 0x0d);//
	OV5646MIPI_write_cmos_sensor(0x581d, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x581e, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x581f, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5820, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5821, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x5822, 0x17);//
	OV5646MIPI_write_cmos_sensor(0x5823, 0x21);//
	OV5646MIPI_write_cmos_sensor(0x5824, 0x46);//
	OV5646MIPI_write_cmos_sensor(0x5825, 0x0c);//
	OV5646MIPI_write_cmos_sensor(0x5826, 0x0e);//
	OV5646MIPI_write_cmos_sensor(0x5827, 0x2c);//
	OV5646MIPI_write_cmos_sensor(0x5828, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5829, 0x2c);//
	OV5646MIPI_write_cmos_sensor(0x582a, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x582b, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x582c, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x582d, 0x0a);//
	OV5646MIPI_write_cmos_sensor(0x582e, 0x2c);//
	OV5646MIPI_write_cmos_sensor(0x582f, 0x44);//
	OV5646MIPI_write_cmos_sensor(0x5830, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x5831, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5832, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x5833, 0x2A);//
	OV5646MIPI_write_cmos_sensor(0x5834, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x5835, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5836, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x5837, 0x2a);//
	OV5646MIPI_write_cmos_sensor(0x5838, 0x46);//
	OV5646MIPI_write_cmos_sensor(0x5839, 0x2c);//
	OV5646MIPI_write_cmos_sensor(0x583a, 0x2c);//
	OV5646MIPI_write_cmos_sensor(0x583b, 0x2a);//
	OV5646MIPI_write_cmos_sensor(0x583c, 0x28);//
	OV5646MIPI_write_cmos_sensor(0x583d, 0xcc);//
*/
	//From OV Kobe CWF.
	OV5646MIPI_write_cmos_sensor(0x5800, 0x19);//lsc
	OV5646MIPI_write_cmos_sensor(0x5801, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x5802, 0x0F);//
	OV5646MIPI_write_cmos_sensor(0x5803, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x5804, 0x1A);//
	OV5646MIPI_write_cmos_sensor(0x5805, 0x2C);//
	OV5646MIPI_write_cmos_sensor(0x5806, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x5807, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x5808, 0x05);//
	OV5646MIPI_write_cmos_sensor(0x5809, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x580a, 0x0B);//
	OV5646MIPI_write_cmos_sensor(0x580b, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x580c, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x580d, 0x02);//
	OV5646MIPI_write_cmos_sensor(0x580e, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x580f, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5810, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5811, 0x0F);//
	OV5646MIPI_write_cmos_sensor(0x5812, 0x0A);//
	OV5646MIPI_write_cmos_sensor(0x5813, 0x03);//
	OV5646MIPI_write_cmos_sensor(0x5814, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5815, 0x01);//
	OV5646MIPI_write_cmos_sensor(0x5816, 0x06);//
	OV5646MIPI_write_cmos_sensor(0x5817, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x5818, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x5819, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x581a, 0x07);//
	OV5646MIPI_write_cmos_sensor(0x581b, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x581c, 0x0E);//
	OV5646MIPI_write_cmos_sensor(0x581d, 0x14);//
	OV5646MIPI_write_cmos_sensor(0x581e, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x581f, 0x11);//
	OV5646MIPI_write_cmos_sensor(0x5820, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x5821, 0x13);//
	OV5646MIPI_write_cmos_sensor(0x5822, 0x16);//
	OV5646MIPI_write_cmos_sensor(0x5823, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x5824, 0x64);//
	OV5646MIPI_write_cmos_sensor(0x5825, 0x46);//
	OV5646MIPI_write_cmos_sensor(0x5826, 0x26);//
	OV5646MIPI_write_cmos_sensor(0x5827, 0x46);//
	OV5646MIPI_write_cmos_sensor(0x5828, 0x66);//
	OV5646MIPI_write_cmos_sensor(0x5829, 0x44);//
	OV5646MIPI_write_cmos_sensor(0x582a, 0x64);//
	OV5646MIPI_write_cmos_sensor(0x582b, 0x62);//
	OV5646MIPI_write_cmos_sensor(0x582c, 0x42);//
	OV5646MIPI_write_cmos_sensor(0x582d, 0x24);//
	OV5646MIPI_write_cmos_sensor(0x582e, 0x44);//
	OV5646MIPI_write_cmos_sensor(0x582f, 0x82);//
	OV5646MIPI_write_cmos_sensor(0x5830, 0xA0);//
	OV5646MIPI_write_cmos_sensor(0x5831, 0x40);//
	OV5646MIPI_write_cmos_sensor(0x5832, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x5833, 0x64);//
	OV5646MIPI_write_cmos_sensor(0x5834, 0x42);//
	OV5646MIPI_write_cmos_sensor(0x5835, 0x22);//
	OV5646MIPI_write_cmos_sensor(0x5836, 0x42);//
	OV5646MIPI_write_cmos_sensor(0x5837, 0x46);//
	OV5646MIPI_write_cmos_sensor(0x5838, 0x86);//
	OV5646MIPI_write_cmos_sensor(0x5839, 0x66);//
	OV5646MIPI_write_cmos_sensor(0x583a, 0x68);//
	OV5646MIPI_write_cmos_sensor(0x583b, 0x66);//
	OV5646MIPI_write_cmos_sensor(0x583c, 0x88);//
	OV5646MIPI_write_cmos_sensor(0x583d, 0x8E);//
#endif
#if 0
	/*AE Weight - CenterAverage*/ 
	OV5646MIPI_write_cmos_sensor(0x5688, 0x11);
	OV5646MIPI_write_cmos_sensor(0x5689, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568a, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568b, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568c, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568d, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568e, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568f, 0x11);
//#else
	OV5646MIPI_write_cmos_sensor(0x5688, 0x11);
	OV5646MIPI_write_cmos_sensor(0x5689, 0x11);
	OV5646MIPI_write_cmos_sensor(0x568a, 0x41);
	OV5646MIPI_write_cmos_sensor(0x568b, 0x14);
	OV5646MIPI_write_cmos_sensor(0x568c, 0x42);
	OV5646MIPI_write_cmos_sensor(0x568d, 0x24);
	OV5646MIPI_write_cmos_sensor(0x568e, 0x21);
	OV5646MIPI_write_cmos_sensor(0x568f, 0x12);
#endif

	OV5646MIPI_write_cmos_sensor(0x583e, 0x10);//
	OV5646MIPI_write_cmos_sensor(0x583f, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x5840, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5025, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
	OV5646MIPI_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
	OV5646MIPI_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
	OV5646MIPI_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out H
	OV5646MIPI_write_cmos_sensor(0x3a1e, 0x23);//	; AEC out L
	OV5646MIPI_write_cmos_sensor(0x3a11, 0x53);//	; control zone H
	OV5646MIPI_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L	
	OV5646MIPI_write_cmos_sensor(0x501d, 0x00);//	; control zone L	
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIInitialSetting function:\n ");
}

/*****************************************************************
* FUNCTION
*    OV5646MIPIPreviewSetting
*
* DESCRIPTION
*    This function config Preview setting related registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV5646MIPIPreviewSetting_SVGA(void)
{
	//;OV5646MIPI 1280x960,30fps
	//56Mhz, 224Mbps/Lane, 2Lane.
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIPreviewSetting_SVGA function:\n ");
	OV5646MIPI_write_cmos_sensor(0x4202, 0x0f);//	; stop mipi stream
	OV5646MIPI_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5646MIPI_write_cmos_sensor(0x3034, 0x18);// PLL, MIPI 8-bit mode
	OV5646MIPI_write_cmos_sensor(0x3035, 0x21);// PLL
	OV5646MIPI_write_cmos_sensor(0x3036, 0x70);// PLL
	OV5646MIPI_write_cmos_sensor(0x3037, 0x13);// PLL
	OV5646MIPI_write_cmos_sensor(0x3108, 0x01);// PLL
	OV5646MIPI_write_cmos_sensor(0x3824, 0x01);// PLL
	OV5646MIPI_write_cmos_sensor(0x460c, 0x20);// PLL
	OV5646MIPI_write_cmos_sensor(0x3618, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x3600, 0x09);//
	OV5646MIPI_write_cmos_sensor(0x3601, 0x43);//
	OV5646MIPI_write_cmos_sensor(0x3708, 0x66);//
	OV5646MIPI_write_cmos_sensor(0x3709, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x370c, 0xc3);//
	OV5646MIPI_write_cmos_sensor(0x3800, 0x00); // HS = 0
	OV5646MIPI_write_cmos_sensor(0x3801, 0x00); // HS
	OV5646MIPI_write_cmos_sensor(0x3802, 0x00); // VS = 250
	OV5646MIPI_write_cmos_sensor(0x3803, 0x06); // VS
	OV5646MIPI_write_cmos_sensor(0x3804, 0x0a); // HW = 2623
	OV5646MIPI_write_cmos_sensor(0x3805, 0x3f);//	; HW
	OV5646MIPI_write_cmos_sensor(0x3806, 0x07);//	; VH = 
	OV5646MIPI_write_cmos_sensor(0x3807, 0x9d);//	; VH
	OV5646MIPI_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
	OV5646MIPI_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
	OV5646MIPI_write_cmos_sensor(0x380a, 0x03);//	; DVPVO = 960
	OV5646MIPI_write_cmos_sensor(0x380b, 0xc0);//	; DVPVO
	OV5646MIPI_write_cmos_sensor(0x380c, 0x07);//	; HTS = 2160
	OV5646MIPI_write_cmos_sensor(0x380d, 0x68);//	; HTS
	OV5646MIPI_write_cmos_sensor(0x380e, 0x03);//	; VTS = 740
	OV5646MIPI_write_cmos_sensor(0x380f, 0xd8);//	; VTS
	OV5646MIPI_write_cmos_sensor(0x3810, 0x00); // H OFF = 16
	OV5646MIPI_write_cmos_sensor(0x3811, 0x10); // H OFF
	OV5646MIPI_write_cmos_sensor(0x3812, 0x00); // V OFF = 4
	OV5646MIPI_write_cmos_sensor(0x3813, 0x06);//	; V OFF
	OV5646MIPI_write_cmos_sensor(0x3814, 0x31);//	; X INC
	OV5646MIPI_write_cmos_sensor(0x3815, 0x31);//	; Y INC
	OV5646MIPI_write_cmos_sensor(0x3820, 0x47);//	; flip off, V bin on
	OV5646MIPI_write_cmos_sensor(0x3821, 0x01);//	; mirror on, H bin on
	OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
        if(OV5646MIPISensor.NightMode)
        {
	    OV5646MIPI_write_cmos_sensor(0x3a02, 0x17);//	; max exp 60 = 740
	    OV5646MIPI_write_cmos_sensor(0x3a03, 0x10);//	; max exp 60
	    OV5646MIPI_write_cmos_sensor(0x3a14, 0x17);//	; max exp 50 = 740
	    OV5646MIPI_write_cmos_sensor(0x3a15, 0x10);//	; max exp 50
        }else
        {
	    OV5646MIPI_write_cmos_sensor(0x3a02, 0x0b);//	; max exp 60 = 740
	    OV5646MIPI_write_cmos_sensor(0x3a03, 0x88);//	; max exp 60
	    OV5646MIPI_write_cmos_sensor(0x3a14, 0x0b);//	; max exp 50 = 740
	    OV5646MIPI_write_cmos_sensor(0x3a15, 0x88);//	; max exp 50
        }
	OV5646MIPI_write_cmos_sensor(0x3a08, 0x01);//	; B50 = 222
	OV5646MIPI_write_cmos_sensor(0x3a09, 0x27);//	; B50
	OV5646MIPI_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
	OV5646MIPI_write_cmos_sensor(0x3a0b, 0xf6);//	; B60
	OV5646MIPI_write_cmos_sensor(0x3a0e, 0x03);//	; max 50
	OV5646MIPI_write_cmos_sensor(0x3a0d, 0x04);//	; max 60
	OV5646MIPI_write_cmos_sensor(0x3c07, 0x07);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3c08, 0x01);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3c09, 0xc2);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
	OV5646MIPI_write_cmos_sensor(0x4005, 0x18);//	; BLC triggered by gain change
	OV5646MIPI_write_cmos_sensor(0x4837, 0x11); // MIPI global timing 16           
	OV5646MIPI_write_cmos_sensor(0x503d, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5000, 0xa7);//
	OV5646MIPI_write_cmos_sensor(0x5001, 0xa3);//
	OV5646MIPI_write_cmos_sensor(0x5002, 0x80);//
	OV5646MIPI_write_cmos_sensor(0x5003, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x3032, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x4000, 0x89);//
	OV5646MIPI_write_cmos_sensor(0x3a00, 0x3c);//	; ae mode	
	//OV5646MIPI_write_cmos_sensor(0x5302, 0x20);//	; sharpen MT
	//OV5646MIPI_write_cmos_sensor(0x5303, 0x18);//	; sharpen MT
	//OV5646MIPI_write_cmos_sensor(0x5306, 0x08);//	; DNS off1
	//OV5646MIPI_write_cmos_sensor(0x5307, 0x10);//	; DNS off2
	if(OV5646MIPI_run_test_potten)
	{
		OV5646MIPI_run_test_potten=0;
		OV5646SetTestPatternMode(1);
	}
	OV5646MIPIWriteExtraShutter(OV5646MIPISensor.PreviewExtraShutter);
	OV5646MIPIWriteExpShutter(OV5646MIPISensor.PreviewShutter);   
	OV5646MIPIWriteSensorGain(OV5646MIPISensor.SensorGain);
	OV5646MIPI_write_cmos_sensor(0x4202, 0x00);//	; open mipi stream
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIPreviewSetting_SVGA function:\n ");
}

/*************************************************************************
* FUNCTION
*     OV5646MIPIFullSizeCaptureSetting
*
* DESCRIPTION
*    This function config full size capture setting related registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV5646MIPIFullSizeCaptureSetting(void)
{
	//OV5646MIPI 2592x1944,10fps
	//90Mhz, 360Mbps/Lane, 2Lane.15
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIFullSizeCaptureSetting function:\n ");
	OV5646MIPI_write_cmos_sensor(0x4202, 0x0f);//	; stop mipi stream
	OV5646MIPI_write_cmos_sensor(0x3a02, 0x03);//	; max exp 60 = 740
	OV5646MIPI_write_cmos_sensor(0x3a03, 0xd8);//	; max exp 60
	OV5646MIPI_write_cmos_sensor(0x3a14, 0x03);//	; max exp 50 = 740
	OV5646MIPI_write_cmos_sensor(0x3a15, 0xd8);//	; max exp 50
	OV5646MIPI_write_cmos_sensor(0x3c07, 0x07);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3c08, 0x01);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3c09, 0xc2);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
	OV5646MIPI_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5646MIPI_write_cmos_sensor(0x3034, 0x18); //PLL, MIPI 8-bit mode
	OV5646MIPI_write_cmos_sensor(0x3035, 0x11); //PLL
	OV5646MIPI_write_cmos_sensor(0x3036, 0x5a); //PLL
	OV5646MIPI_write_cmos_sensor(0x3037, 0x13); //PLL
	OV5646MIPI_write_cmos_sensor(0x3108, 0x01); //PLL
	OV5646MIPI_write_cmos_sensor(0x3824, 0x01); //PLL
	OV5646MIPI_write_cmos_sensor(0x460c, 0x20); //PLL
	OV5646MIPI_write_cmos_sensor(0x3618, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x3600, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x3601, 0x33);//
	OV5646MIPI_write_cmos_sensor(0x3708, 0x63);//
	OV5646MIPI_write_cmos_sensor(0x3709, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x370c, 0xc0);//
	OV5646MIPI_write_cmos_sensor(0x3800, 0x00); //HS = 0
	OV5646MIPI_write_cmos_sensor(0x3801, 0x00); //HS
	OV5646MIPI_write_cmos_sensor(0x3802, 0x00); //VS = 0
	OV5646MIPI_write_cmos_sensor(0x3803, 0x00); //VS
	OV5646MIPI_write_cmos_sensor(0x3804, 0x0a); //HW = 2623
	OV5646MIPI_write_cmos_sensor(0x3805, 0x3f);//	; HW
	OV5646MIPI_write_cmos_sensor(0x3806, 0x07);//	; VH = 1705
	OV5646MIPI_write_cmos_sensor(0x3807, 0x9f);//	; VH
	OV5646MIPI_write_cmos_sensor(0x3808, 0x0a);//	; DVPHO = 2560
	OV5646MIPI_write_cmos_sensor(0x3809, 0x20);//	; DVPHO
	OV5646MIPI_write_cmos_sensor(0x380a, 0x07);//	; DVPVO = 1440
	OV5646MIPI_write_cmos_sensor(0x380b, 0x98);//	; DVPVO
	OV5646MIPI_write_cmos_sensor(0x380c, 0x0b);//	; HTS = 2984
	OV5646MIPI_write_cmos_sensor(0x380d, 0xec);//	; HTS
	OV5646MIPI_write_cmos_sensor(0x380e, 0x07);//	; VTS = 1464
	OV5646MIPI_write_cmos_sensor(0x380f, 0xb0);//	; VTS
	OV5646MIPI_write_cmos_sensor(0x3810, 0x00); //H OFF = 16
	OV5646MIPI_write_cmos_sensor(0x3811, 0x10); //H OFF
	OV5646MIPI_write_cmos_sensor(0x3812, 0x00); //V OFF = 4
	OV5646MIPI_write_cmos_sensor(0x3813, 0x06);//	; V OFF
	OV5646MIPI_write_cmos_sensor(0x3814, 0x11);//	; X INC
	OV5646MIPI_write_cmos_sensor(0x3815, 0x11);//	; Y INC
	OV5646MIPI_write_cmos_sensor(0x3820, 0x46);//	; flip off, V bin off
	OV5646MIPI_write_cmos_sensor(0x3821, 0x00);//	; mirror on, H bin off
	OV5646MIPI_write_cmos_sensor(0x4514, 0xa8);//   ; 0x00
	OV5646MIPI_write_cmos_sensor(0x3a0e, 0x06);//	; flip off, V bin off
	OV5646MIPI_write_cmos_sensor(0x3a0d, 0x08);//	; mirror on, H bin off
	OV5646MIPI_write_cmos_sensor(0x4004, 0x06);//	; BLC line number
	OV5646MIPI_write_cmos_sensor(0x4837, 0x16);//; MIPI global timing  
	OV5646MIPI_write_cmos_sensor(0x503d, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5000, 0xa7);//
	OV5646MIPI_write_cmos_sensor(0x5001, 0x83);//
	OV5646MIPI_write_cmos_sensor(0x5002, 0x80);
	OV5646MIPI_write_cmos_sensor(0x5003, 0x08);
	OV5646MIPI_write_cmos_sensor(0x3032, 0x00);
	OV5646MIPI_write_cmos_sensor(0x4000, 0x89);
	OV5646MIPI_write_cmos_sensor(0x350c, 0x00);
	OV5646MIPI_write_cmos_sensor(0x350d, 0x00);
	//OV5646MIPI_write_cmos_sensor(0x5302, 0x30);//	; sharpen MT
	//OV5646MIPI_write_cmos_sensor(0x5303, 0x20);//	; sharpen MT
	//OV5646MIPI_write_cmos_sensor(0x5306, 0x08);//	; DNS off1
	//OV5646MIPI_write_cmos_sensor(0x5307, 0x10);//	; DNS off2
	if(OV5646MIPI_run_test_potten)
	{
		OV5646MIPI_run_test_potten=0;
		OV5646SetTestPatternMode(1);
	}
	OV5646MIPI_write_cmos_sensor(0x4202, 0x00);//	; open mipi stream
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.IsPVmode = KAL_FALSE;
	OV5646MIPISensor.CapturePclk= 900;	
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIFullSizeCaptureSetting function:\n ");
}

/*************************************************************************
* FUNCTION
*     OV5646MIPIFullSizeZSDSetting
*
* DESCRIPTION
*    This function config full size capture setting related registers of CMOS sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV5646MIPIFullSizeZSDSetting(void)
{
	//OV5646MIPI 2592x1944,7-15fps
	//90Mhz, 360Mbps/Lane, 2Lane.15
	OV5646MIPI_write_cmos_sensor(0x4202, 0x0f);//	; stop mipi stream
	OV5646MIPI_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
	OV5646MIPI_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
	OV5646MIPI_write_cmos_sensor(0x3034, 0x18); //PLL, MIPI 8-bit mode
	OV5646MIPI_write_cmos_sensor(0x3035, 0x11); //PLL
	OV5646MIPI_write_cmos_sensor(0x3036, 0x5a); //PLL
	OV5646MIPI_write_cmos_sensor(0x3037, 0x13); //PLL
	OV5646MIPI_write_cmos_sensor(0x3108, 0x01); //PLL
	OV5646MIPI_write_cmos_sensor(0x3824, 0x01); //PLL
	OV5646MIPI_write_cmos_sensor(0x460c, 0x20); //PLL
	OV5646MIPI_write_cmos_sensor(0x3618, 0x04);//
	OV5646MIPI_write_cmos_sensor(0x3600, 0x08);//
	OV5646MIPI_write_cmos_sensor(0x3601, 0x33);//
	OV5646MIPI_write_cmos_sensor(0x3708, 0x63);//
	OV5646MIPI_write_cmos_sensor(0x3709, 0x12);//
	OV5646MIPI_write_cmos_sensor(0x370c, 0xc0);//
	OV5646MIPI_write_cmos_sensor(0x3800, 0x00); //HS = 0
	OV5646MIPI_write_cmos_sensor(0x3801, 0x00); //HS
	OV5646MIPI_write_cmos_sensor(0x3802, 0x00); //VS = 0
	OV5646MIPI_write_cmos_sensor(0x3803, 0x00); //VS
	OV5646MIPI_write_cmos_sensor(0x3804, 0x0a); //HW = 2623
	OV5646MIPI_write_cmos_sensor(0x3805, 0x3f);//	; HW
	OV5646MIPI_write_cmos_sensor(0x3806, 0x07);//	; VH = 1705
	OV5646MIPI_write_cmos_sensor(0x3807, 0x9f);//	; VH
	OV5646MIPI_write_cmos_sensor(0x3808, 0x0a);//	; DVPHO = 2560
	OV5646MIPI_write_cmos_sensor(0x3809, 0x20);//	; DVPHO
	OV5646MIPI_write_cmos_sensor(0x380a, 0x07);//	; DVPVO = 1440
	OV5646MIPI_write_cmos_sensor(0x380b, 0x98);//	; DVPVO
	OV5646MIPI_write_cmos_sensor(0x380c, 0x0b);//	; HTS = 2984
	OV5646MIPI_write_cmos_sensor(0x380d, 0xec);//	; HTS
	OV5646MIPI_write_cmos_sensor(0x380e, 0x07);//	; VTS = 1464
	OV5646MIPI_write_cmos_sensor(0x380f, 0xb0);//	; VTS
	OV5646MIPI_write_cmos_sensor(0x3810, 0x00); //H OFF = 16
	OV5646MIPI_write_cmos_sensor(0x3811, 0x10); //H OFF
	OV5646MIPI_write_cmos_sensor(0x3812, 0x00); //V OFF = 4
	OV5646MIPI_write_cmos_sensor(0x3813, 0x06);//	; V OFF	
	OV5646MIPI_write_cmos_sensor(0x3814, 0x11);//	; X INC
	OV5646MIPI_write_cmos_sensor(0x3815, 0x11);//	; Y INC
	OV5646MIPI_write_cmos_sensor(0x3820, 0x46);//	; flip off, V bin off
	OV5646MIPI_write_cmos_sensor(0x3821, 0x00);//	; mirror on, H bin off
	OV5646MIPI_write_cmos_sensor(0x4514, 0xa8);
	OV5646MIPI_write_cmos_sensor(0x3a02, 0x10);//	; max exp 60 = 740
	OV5646MIPI_write_cmos_sensor(0x3a03, 0x24);//	; max exp 60
	OV5646MIPI_write_cmos_sensor(0x3a14, 0x10);//	; max exp 50 = 740
	OV5646MIPI_write_cmos_sensor(0x3a15, 0x24);//	; max exp 50
	OV5646MIPI_write_cmos_sensor(0x3c07, 0x07);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3c08, 0x01);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3c09, 0xc2);//	; 50/60 auto detect
	OV5646MIPI_write_cmos_sensor(0x3a0e, 0x06);//	; flip off, V bin off
	OV5646MIPI_write_cmos_sensor(0x3a0d, 0x08);//	; mirror on, H bin off
	OV5646MIPI_write_cmos_sensor(0x4004, 0x06);//	; BLC line number
	OV5646MIPI_write_cmos_sensor(0x4837, 0x16);//; MIPI global timing  
	OV5646MIPI_write_cmos_sensor(0x503d, 0x00);//
	OV5646MIPI_write_cmos_sensor(0x5000, 0xa7);//
	OV5646MIPI_write_cmos_sensor(0x5001, 0x83);//
	OV5646MIPI_write_cmos_sensor(0x5002, 0x80);
	OV5646MIPI_write_cmos_sensor(0x5003, 0x08);
	OV5646MIPI_write_cmos_sensor(0x3032, 0x00);
	OV5646MIPI_write_cmos_sensor(0x4000, 0x89);
	OV5646MIPI_write_cmos_sensor(0x350c, 0x00);
	OV5646MIPI_write_cmos_sensor(0x350d, 0x00);
	OV5646MIPI_write_cmos_sensor(0x3a00, 0x3c);//	; ae mode
	OV5646MIPI_write_cmos_sensor(0x4202, 0x00);//	; open mipi stream
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.IsPVmode = KAL_FALSE;
	OV5646MIPISensor.CapturePclk= 900;	
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIFullSizeCaptureSetting function:\n ");
}

/*************************************************************************
* FUNCTION
*    OV5646MIPISetHVMirror
*
* DESCRIPTION
*    This function set sensor Mirror
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
static void OV5646MIPISetHVMirror(kal_uint8 Mirror, kal_uint8 Mode)
{
	kal_uint8 mirror= 0, flip=0;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPISetHVMirror function:\n ");
	flip = OV5646MIPIYUV_read_cmos_sensor(0x3820);
	mirror=OV5646MIPIYUV_read_cmos_sensor(0x3821);
	
	if (Mode==SENSOR_MODE_PREVIEW)
	{
		switch (Mirror)
		{
		case IMAGE_NORMAL:
			OV5646MIPI_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_H_MIRROR:
			OV5646MIPI_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror|0x06);
			OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_V_MIRROR: 
			OV5646MIPI_write_cmos_sensor(0x3820, flip|0x06);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
			break;		
		case IMAGE_HV_MIRROR:
			OV5646MIPI_write_cmos_sensor(0x3820, flip|0x06);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror|0x06);
			OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
			break; 		
		default:
			ASSERT(0);
		}
	}
	else if (Mode== SENSOR_MODE_CAPTURE)
	{
		switch (Mirror)
		{
		case IMAGE_NORMAL:
			OV5646MIPI_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_H_MIRROR:
			OV5646MIPI_write_cmos_sensor(0x3820, flip&0xf9);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror|0x06);
			OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
			break;
		case IMAGE_V_MIRROR: 
			OV5646MIPI_write_cmos_sensor(0x3820, flip|0x06);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror&0xf9);
			OV5646MIPI_write_cmos_sensor(0x4514, 0xa8);//0xaa
			break;		
		case IMAGE_HV_MIRROR:
			OV5646MIPI_write_cmos_sensor(0x3820, flip|0x06);     
			OV5646MIPI_write_cmos_sensor(0x3821, mirror|0x06);
			OV5646MIPI_write_cmos_sensor(0x4514, 0xbb);
			break; 		
		default:
			ASSERT(0);
		}
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPISetHVMirror function:\n ");
}

void OV5646MIPI_Standby(void)
{
	OV5646MIPI_write_cmos_sensor(0x3008,0x42);
}

void OV5646MIPI_Wakeup(void)
{
	OV5646MIPI_write_cmos_sensor(0x3008,0x02);
}

static UINT32 OV5646read_vcm_adc()  {
	kal_uint16 high, low, adcvalue;
	high= OV5646MIPIYUV_read_cmos_sensor(0x3603);
	low= OV5646MIPIYUV_read_cmos_sensor(0x3602);
	adcvalue= ((high&0x3f)<<4) + ((low&0xf0)>>4); 
	return adcvalue;
}

static UINT32 OV5646set_vcm_adc(adcvalue) {
	kal_uint16 high, low;
	high = adcvalue>>4;
	low = (adcvalue&0x0f)<<4;
	OV5646MIPI_write_cmos_sensor(0x3603,(OV5646MIPIYUV_read_cmos_sensor(0x3603)&0xC0)|high);
	OV5646MIPI_write_cmos_sensor(0x3602,(OV5646MIPIYUV_read_cmos_sensor(0x3602)&0x0F)|low);
	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   OV5646_FOCUS_OVT_AFC_Init
* DESCRIPTION
*   This function is to load micro code for AF function
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/
static void OV5646_FOCUS_OVT_AFC_Init(void)
{
	int i, length, address, AF_status;
	OV5646MIPI_write_cmos_sensor(0x3000,0x20);

	length = sizeof(OV5646MIPI_firmware)/sizeof(int);
	address = 0x8000;
	for(i=0;i<length;i++) {
		OV5646MIPI_write_cmos_sensor(address, OV5646MIPI_firmware[i]);
		address++;
	}

	OV5646MIPI_write_cmos_sensor(0x3022,0x00);
	OV5646MIPI_write_cmos_sensor(0x3023,0x00);
	OV5646MIPI_write_cmos_sensor(0x3024,0x00);
	OV5646MIPI_write_cmos_sensor(0x3025,0x00);
	OV5646MIPI_write_cmos_sensor(0x3026,0x00);
	OV5646MIPI_write_cmos_sensor(0x3027,0x00);
	OV5646MIPI_write_cmos_sensor(0x3028,0x00);
	OV5646MIPI_write_cmos_sensor(0x3029,0x7f);
	OV5646MIPI_write_cmos_sensor(0x3000, 0x00);        // Enable MCU
	if(false == OV5646MIPI_AF_Power)
	{
		OV5646MIPI_write_cmos_sensor(0x3602,0x00);
		OV5646MIPI_write_cmos_sensor(0x3603,0x00);
	}
}

/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Constant_Focus
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
static void OV5646_FOCUS_OVT_AFC_Constant_Focus(void)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_OVT_AFC_Constant_Focus function:\n ");
	OV5646MIPI_write_cmos_sensor(0x3023,0x01);
	OV5646MIPI_write_cmos_sensor(0x3022,0x04);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_OVT_AFC_Constant_Focus function:\n ");
}   
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Single_Focus
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
static void OV5646_FOCUS_OVT_AFC_Single_Focus()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_OVT_AFC_Single_Focus function:\n ");
	OV5646MIPI_write_cmos_sensor(0x3023,0x01);
	OV5646MIPI_write_cmos_sensor(0x3022,0x81);
	mDELAY(20);
	OV5646MIPI_write_cmos_sensor(0x3022,0x03);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_OVT_AFC_Single_Focus function:\n ");
}
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Pause_Focus
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
static void OV5646_FOCUS_OVT_AFC_Pause_Focus()
{
	OV5646MIPI_write_cmos_sensor(0x3023,0x01);
	OV5646MIPI_write_cmos_sensor(0x3022,0x06);
}
static void OV5646_FOCUS_Get_AF_Max_Num_Focus_Areas(UINT32 *pFeatureReturnPara32)
{ 	  
	*pFeatureReturnPara32 = 0;    
	OV5646MIPISENSORDB(" *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);	
}

static void OV5646_FOCUS_Get_AE_Max_Num_Metering_Areas(UINT32 *pFeatureReturnPara32)
{ 	
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_Get_AE_Max_Num_Metering_Areas function:\n ");
	*pFeatureReturnPara32 = 0;    
	OV5646MIPISENSORDB(" *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_Get_AE_Max_Num_Metering_Areas function:\n ");
}
static void OV5646_FOCUS_OVT_AFC_Touch_AF(UINT32 x,UINT32 y)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_OVT_AFC_Touch_AF function:\n ");
	int x_view,y_view;
	int x_tmp,y_tmp;
	if(x<1)
	{
		x_view=1;
	}
	else if(x>79)
	{
		x_view=79;
	}
	else
	{
		x_view= x;
	}
     
	if(y<1)
	{
		y_view=1;
	}
	else if(y>59)
	{
		y_view=59;
	}
	else
	{
		y_view= y;
	}
	OV5646MIPISENSORDB("[OV5646MIPI]AF x_view=%d,y_view=%d\n",x_view, y_view);
	OV5646MIPI_write_cmos_sensor(0x3024,x_view);
	OV5646MIPI_write_cmos_sensor(0x3025,y_view);   
	x_tmp = OV5646MIPIYUV_read_cmos_sensor(0x3024);
	y_tmp = OV5646MIPIYUV_read_cmos_sensor(0x3025);
	OV5646MIPISENSORDB("[OV5646MIPI]AF x_tmp1=%d,y_tmp1=%d\n",x_tmp, y_tmp);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_OVT_AFC_Touch_AF function:\n ");
}

static void OV5646_FOCUS_Set_AF_Window(UINT32 zone_addr)
{//update global zone
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_Set_AF_Window function:\n ");
	UINT32 FD_XS;
	UINT32 FD_YS;   
	UINT32 x0, y0, x1, y1;
	UINT32 pvx0, pvy0, pvx1, pvy1;
	UINT32 linenum, rownum;
	UINT32 AF_pvx, AF_pvy;
	UINT32* zone = (UINT32*)zone_addr;
	x0 = *zone;
	y0 = *(zone + 1);
	x1 = *(zone + 2);
	y1 = *(zone + 3);   
	FD_XS = *(zone + 4);
	FD_YS = *(zone + 5);
	  
	OV5646MIPISENSORDB("AE x0=%d,y0=%d,x1=%d,y1=%d,FD_XS=%d,FD_YS=%d\n",x0, y0, x1, y1, FD_XS, FD_YS);  
	OV5646MIPI_mapMiddlewaresizePointToPreviewsizePoint(x0,y0,FD_XS,FD_YS,&pvx0, &pvy0, OV5646MIPI_PRV_W, OV5646MIPI_PRV_H);
	OV5646MIPI_mapMiddlewaresizePointToPreviewsizePoint(x1,y1,FD_XS,FD_YS,&pvx1, &pvy1, OV5646MIPI_PRV_W, OV5646MIPI_PRV_H);  
	OV5646MIPISENSORDB("[OV5646MIPI]AF pvx0=%d,pvy0=%d\n",pvx0, pvy0);
	OV5646MIPISENSORDB("[OV5646MIPI]AF pvx0=%d,pvy0=%d\n",pvx1, pvy1);
	AF_pvx =(pvx0+pvx1)/32;
	AF_pvy =(pvy0+pvy1)/32;
	OV5646MIPISENSORDB("[OV5646MIPI]AF AF_pvx=%d,AF_pvy=%d\n",AF_pvx, AF_pvy);
	OV5646_FOCUS_OVT_AFC_Touch_AF(AF_pvx ,AF_pvy);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_Set_AF_Window function:\n ");
}
static void OV5646_FOCUS_Get_AF_Macro(UINT32 *pFeatureReturnPara32)
{
	*pFeatureReturnPara32 = 0;
}
static void OV5646_FOCUS_Get_AF_Inf(UINT32 * pFeatureReturnPara32)
{
	*pFeatureReturnPara32 = 0;
}
/*************************************************************************
//ŽËº¯Êý±äÁ¿ÎŽ¶šÒå,ÇëžùŸÝ±àÒë¶šÒåÊýŸÝ±äÁ¿.
//prview 1280*960 
//16µÄÕûÊý±¶ ; n*16*80/1280
//16µÄÕûÊý±¶ ; n*16*60/960
//touch_x  Îª¶ÔÓŠpreviewµÄºá×ø±ê[0-1280]
//touch_y  Îª¶ÔÓŠpreviewµÄ×Ý×ø±ê[0-960]

*************************************************************************/ 
static UINT32 OV5646_FOCUS_Move_to(UINT32 a_u2MovePosition)//??how many bits for ov3640??
{
}
/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Get_AF_Status
* DESCRIPTION
*   GET af stauts
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/                        
static void OV5646_FOCUS_OVT_AFC_Get_AF_Status(UINT32 *pFeatureReturnPara32)
{
	UINT32 state_3028=0;
	UINT32 state_3029=0;
	*pFeatureReturnPara32 = SENSOR_AF_IDLE;
	state_3028 = OV5646MIPIYUV_read_cmos_sensor(0x3028);
	state_3029 = OV5646MIPIYUV_read_cmos_sensor(0x3029);
	mDELAY(1);
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646_FOCUS_OVT_AFC_Get_AF_Status function:state_3028=%d,state_3029=%d\n",state_3028,state_3029);
	if (state_3028==0)
	{
		*pFeatureReturnPara32 = SENSOR_AF_ERROR;    
	}
	else if (state_3028==1)
	{
		switch (state_3029)
		{
			case 0x70:
				*pFeatureReturnPara32 = SENSOR_AF_IDLE;
				break;
			case 0x00:
				*pFeatureReturnPara32 = SENSOR_AF_FOCUSING;
				break;
			case 0x10:
				*pFeatureReturnPara32 = SENSOR_AF_FOCUSED;
				break;
			case 0x20:
				*pFeatureReturnPara32 = SENSOR_AF_FOCUSED;
				break;
			default:
				*pFeatureReturnPara32 = SENSOR_AF_SCENE_DETECTING; 
				break;
		}                                  
	}    
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646_FOCUS_OVT_AFC_Get_AF_Status function:state_3028=%d,state_3029=%d\n",state_3028,state_3029);
}

/*************************************************************************
* FUNCTION
*   OV5640_FOCUS_OVT_AFC_Cancel_Focus
* DESCRIPTION
*   cancel af 
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/     
static void OV5646_FOCUS_OVT_AFC_Cancel_Focus()
{
    //OV5646MIPI_write_cmos_sensor(0x3023,0x01);
    //OV5646MIPI_write_cmos_sensor(0x3022,0x08);    
}

/*************************************************************************
* FUNCTION
*   OV5646WBcalibattion
* DESCRIPTION
*   color calibration
* PARAMETERS
*   None
* RETURNS
*   None
* GLOBALS AFFECTED
*************************************************************************/	
static void OV5646WBcalibattion(kal_uint32 color_r_gain,kal_uint32 color_b_gain)
{
	kal_uint32 color_r_gain_w = 0;
	kal_uint32 color_b_gain_w = 0;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646WBcalibattion function:\n ");
	OV5646MIPISENSORDB("[OV5646MIPI]OV5646WBcalibattion function: color_r_gain == 0x%x\n",color_r_gain);
	OV5646MIPISENSORDB("[OV5646MIPI]OV5646WBcalibattion function: color_b_gain == 0x%x\n",color_b_gain);
	kal_uint8 temp = OV5646MIPIYUV_read_cmos_sensor(0x350b);
#if 1
	if(temp>=0xb0)
	{
	    color_r_gain_w=color_r_gain*97/100;
	    color_b_gain_w=color_b_gain*99/100;
	}
	else if (temp>=0x70)
	{
	    color_r_gain_w=color_r_gain*97/100;
	    color_b_gain_w=color_b_gain*99/100;
	}
	else if (temp>=0x30)
	{
	    color_r_gain_w=color_r_gain*97/100;
	    color_b_gain_w=color_b_gain*99/100;
	}
	else
	{
	    color_r_gain_w=color_r_gain*97/100;
	    color_b_gain_w=color_b_gain*99/100;
	}
#else
        if(color_r_gain>0x600)//d65   66b 5a7 4a7 449
        {
            if (temp>=0x70)
            {
                color_r_gain_w=color_r_gain*95/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x58)
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*98/100;
            }
            else if (temp>=0x48)
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*98/100;
            }
            else if (temp>=0x30)
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*98/100;
            }
            else
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*98/100;
            }
        }
        else if(color_r_gain>0x540)//cwf
        {
            if (temp>=0x70)
            {
                color_r_gain_w=color_r_gain *97/100;
                color_b_gain_w=color_b_gain*98/100;
            }
            else if (temp>=0x58)
            {
                color_r_gain_w=color_r_gain*98/100;
                color_b_gain_w=color_b_gain*99/100;
            }
            else if (temp>=0x48)
            {
                color_r_gain_w=color_r_gain*98/100;
                color_b_gain_w=color_b_gain*99/100;
            }
            else if (temp>=0x30)
            {
                color_r_gain_w=color_r_gain*99/100;
                color_b_gain_w=color_b_gain*99/100;
            }
            else
            {
                color_r_gain_w=color_r_gain*99/100;
                color_b_gain_w=color_b_gain*99/100;
            }
        }
        else if(color_r_gain>0x480)//tl84
        {
            if (temp>=0x70)
            {
                color_r_gain_w=color_r_gain *97/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x58)
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x48)
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x30)
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else
            {
                color_r_gain_w=color_r_gain*97/100;
                color_b_gain_w=color_b_gain*97/100;
            }
        }
        else//h/a
        {
            if (temp>=0x70)
            {
                color_r_gain_w=color_r_gain *98/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x58)
            {
                color_r_gain_w=color_r_gain*98/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x48)
            {
                color_r_gain_w=color_r_gain*98/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else if (temp>=0x30)
            {
                color_r_gain_w=color_r_gain*98/100;
                color_b_gain_w=color_b_gain*97/100;
            }
            else
            {
                color_r_gain_w=color_r_gain*98/100;
                color_b_gain_w=color_b_gain*97/100;
            }
        }
#endif
	OV5646MIPISENSORDB("[OV5646MIPI]OV5646WBcalibattion function: color_r_gain_w == 0x%x\n",color_r_gain_w);
	OV5646MIPISENSORDB("[OV5646MIPI]OV5646WBcalibattion function: color_b_gain_w == 0x%x\n",color_b_gain_w);
	OV5646MIPI_write_cmos_sensor(0x3400,(color_r_gain_w & 0xff00)>>8);
	OV5646MIPI_write_cmos_sensor(0x3401,color_r_gain_w & 0xff);
	OV5646MIPI_write_cmos_sensor(0x3404,(color_b_gain_w & 0xff00)>>8);
	OV5646MIPI_write_cmos_sensor(0x3405,color_b_gain_w & 0xff);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646WBcalibattion function:\n ");
}	
/*************************************************************************
* FUNCTION
*	OV5646MIPIOpen
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
UINT32 OV5646MIPIOpen(void)
{
	volatile signed int i;
	kal_uint16 sensor_id = 0;
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIOpen function:\n ");
	OV5646MIPI_write_cmos_sensor(0x3103,0x11);
	OV5646MIPI_write_cmos_sensor(0x3008,0x82);
	mDELAY(5);
	for(i=0;i<3;i++)
	{
		sensor_id = (OV5646MIPIYUV_read_cmos_sensor(0x300A) << 8) | OV5646MIPIYUV_read_cmos_sensor(0x300B);
		OV5646MIPISENSORDB("OV5646MIPI READ ID :%x",sensor_id);
		if(sensor_id != OV5645MIPI_SENSOR_ID)
		{
			return ERROR_SENSOR_CONNECT_FAIL;
		}
		else
		{
			sensor_id = OV5646MIPI_SENSOR_ID;
		}
	}
	OV5646MIPIinitalvariable();
	OV5646MIPIInitialSetting();
	//OV5646_FOCUS_OVT_AFC_Init();
	OV5646MIPI_set_AWB_mode(KAL_TRUE);
#if 0
	if(false == OV5646MIPI_AF_Power)
	{
		OV5646MIPISENSORDB("[OV5646Sensor] AF Power on.\n");
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,"OV5646_AF"))
		{
			printk("[CAMERA SENSOR AF] Fail to enable analog power\n");
			return -EIO;
		}  
		OV5646MIPI_AF_Power = true;
	}
	else
	{
		OV5646MIPISENSORDB("[OV5646Sensor] AF Power has already on.\n");
	}
#endif
	mdelay(8);

	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIOpen function:\n ");
	return ERROR_NONE;
}	/* OV5646MIPIOpen() */

/*************************************************************************
* FUNCTION
*	OV5646MIPIClose
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
UINT32 OV5646MIPIClose(void)
{
#if 0
	OV5646MIPI_flash_mode = 2;
	OV5646MIPI_write_cmos_sensor(0x3023,0x01);
	OV5646MIPI_write_cmos_sensor(0x3022,0x06); 
 	kal_uint16 lastadc, num, i, now, step = 100;
    lastadc = OV5646read_vcm_adc();
 	num = lastadc/step;
	//now = OV5646read_vcm_adc();
	//printk("[OV5646Sensor] out adc value = %d, num = %d\n", now, num);
 	for(i = 1; i <= num; i++)
  	{
  		OV5646set_vcm_adc(lastadc-i*step);
		//now = OV5646read_vcm_adc();
		//printk("[OV5646Sensor] now%d adc value = %d\n",i, now);
		mdelay(30);
	}

	OV5646set_vcm_adc(0);
	OV5646MIPI_write_cmos_sensor(0x3023,0x01);
	OV5646MIPI_write_cmos_sensor(0x3022,0x08); 

	if(true == OV5646MIPI_AF_Power)
	{
		OV5646MIPISENSORDB("[OV5646Sensor] AF Power down.\n");
		//now = OV5646read_vcm_adc();
		//printk("[OV5646Sensor] last adc value = %d\n", now);
	    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,"OV5646_AF"))
	    {
		 	printk("[CAMERA SENSOR AF] Fail to enable analog power\n");
			return -EIO;
		}
		OV5646MIPI_AF_Power = false;
	}
	else
	{
		OV5646MIPISENSORDB("[OV5646Sensor] AF Power is already off.\n");
	}
#endif
	return ERROR_NONE;
}	/* OV5646MIPIClose() */
/*************************************************************************
* FUNCTION
*	OV5646MIPIPreview
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
UINT32 OV5646MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	OV5646MIPIPreviewSetting_SVGA();
	mDELAY(100);	
	OV5646MIPI_set_AE_mode(KAL_TRUE);
	OV5646MIPI_set_AWB_mode(KAL_TRUE);
	mDELAY(20);
	//OV5646_FOCUS_OVT_AFC_Constant_Focus();
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
	OV5646MIPISensor.IsPVmode = KAL_TRUE;
	OV5646MIPISensor.PreviewPclk= 560;	
	OV5646MIPISensor.zsd_flag=0;
	spin_unlock(&ov5646mipi_drv_lock);
	return ERROR_NONE ;
}	/* OV5646MIPIPreview() */
/*************************************************************************
* FUNCTION
*	OV5646MIPIZSDPreview
*
* DESCRIPTION
*	This function start the sensor ZSD preview.
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
UINT32 OV5646MIPIZSDPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    OV5646MIPIFullSizeZSDSetting();
    mDELAY(100);
    OV5646MIPI_set_AE_mode(KAL_TRUE);
    OV5646MIPI_set_AWB_mode(KAL_TRUE);
    mDELAY(20);
    //OV5646_FOCUS_OVT_AFC_Constant_Focus();		
    spin_lock(&ov5646mipi_drv_lock);
    OV5646MIPISensor.zsd_flag=1;
    OV5646MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
    spin_unlock(&ov5646mipi_drv_lock);
    return ERROR_NONE ;
}	/* OV5646MIPIPreview() */
/*************************************************************************
* FUNCTION
*	OV5646MIPICapture
*
* DESCRIPTION
*	This function start the sensor OV5646MIPICapture
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
UINT32 OV5646MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint32 shutter = 0;	
    kal_uint32 extshutter = 0;
    kal_uint32 color_r_gain = 0;
    kal_uint32 color_b_gain = 0;
    kal_uint32 readgain=0;
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPICapture function:\n ");
    //kal_uint32 gain10=0;//////
    //kal_uint32 shutter10 = 0;//////
    //kal_uint32 fixframe10line=0xb88;//////
    //OV5646MIPI_flash_mode = 1;//////
    if(SENSOR_MODE_PREVIEW == OV5646MIPISensor.SensorMode )
    {
	shutter=OV5646MIPIReadShutter();
	extshutter=OV5646MIPIReadExtraShutter();
	readgain=OV5646MIPIReadSensorGain();
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.PreviewShutter=shutter;
	OV5646MIPISensor.PreviewExtraShutter=extshutter;	
	OV5646MIPISensor.SensorGain=readgain;
	spin_unlock(&ov5646mipi_drv_lock);	
	OV5646MIPI_set_AE_mode(KAL_FALSE);
	OV5646MIPI_set_AWB_mode(KAL_FALSE);
	color_r_gain=((OV5646MIPIYUV_read_cmos_sensor(0x3401)&0xFF)+((OV5646MIPIYUV_read_cmos_sensor(0x3400)&0xFF)*256));  
	color_b_gain=((OV5646MIPIYUV_read_cmos_sensor(0x3405)&0xFF)+((OV5646MIPIYUV_read_cmos_sensor(0x3404)&0xFF)*256)); 
	OV5646MIPIFullSizeCaptureSetting();
	spin_lock(&ov5646mipi_drv_lock);	
	OV5646MIPISensor.SensorMode= SENSOR_MODE_CAPTURE;
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646WBcalibattion(color_r_gain,color_b_gain);
	OV5646MIPISENSORDB("[OV5646MIPI]Before shutter=%d:\n",shutter);
	if(OV5646MIPISensor.zsd_flag==0)
	{
            shutter = shutter*2;
            /*if( shutter>fixframe10line)//////
            {//////
                shutter10=fixframe10line;//////
                gain10= readgain*shutter/fixframe10line;//////
            }else//////
            {//////
                shutter10=shutter;//////
                gain10=readgain;//////
            }//////
	}else//////
	{//////
            shutter10=shutter;//////
            gain10=readgain;//////*/
	}
	if (SCENE_MODE_HDR == OV5646MIPISensor.sceneMode)
	{
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPISensor.currentExposureTime=shutter;
            OV5646MIPISensor.currentextshutter=extshutter;
            OV5646MIPISensor.currentAxDGain=readgain;
            spin_unlock(&ov5646mipi_drv_lock);
	}
	else
	{
            //OV5646MIPIWriteSensorGain(readgain);//OV5646MIPISensor.SensorGain
            OV5646MIPIWriteShutter(shutter);
            //OV5646MIPIWriteSensorGain(gain10);//////
            //OV5646MIPIWriteShutter(shutter10);//////
	}
	OV5646MIPISENSORDB("[OV5646MIPI]after shutter, shutter=%d:\n",shutter);
	OV5646MIPISENSORDB("[OV5646MIPI]after shutter, OV5646MIPISensor.SensorGain=%d:\n",OV5646MIPISensor.SensorGain);
	//OV5646MIPISENSORDB("[OV5646MIPI]after shutter, shutter10=%d:\n",shutter10);//////
	//OV5646MIPISENSORDB("[OV5646MIPI]after shutter, gain10=%d:\n",gain10);//////
	mDELAY(200);
    }

    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPICapture function:\n ");
    return ERROR_NONE; 
}/* OV5646MIPICapture() */

BOOL OV5646MIPI_set_param_exposure_for_HDR(UINT16 para)
{
    kal_uint32 totalGain = 0, exposureTime = 0;
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_param_exposure_for_HDR function:\n ");
    OV5646MIPISENSORDB("[OV5646MIPI]enter para=%d,manualAEStart%d\n",para,OV5646MIPISensor.manualAEStart);
    if (0 == OV5646MIPISensor.manualAEStart)
    {       
        OV5646MIPI_set_AE_mode(KAL_FALSE);//Manual AE enable
        spin_lock(&ov5646mipi_drv_lock);	
        OV5646MIPISensor.manualAEStart = 1;
        spin_unlock(&ov5646mipi_drv_lock);
    }
    totalGain = OV5646MIPISensor.currentAxDGain;
    exposureTime = OV5646MIPISensor.currentExposureTime;
    switch (para)
    {
        case AE_EV_COMP_20:	//+2 EV
        case AE_EV_COMP_10:	// +1 EV
            totalGain = totalGain<<1;
            exposureTime = exposureTime<<1;
            OV5646MIPISENSORDB("[4EC] HDR AE_EV_COMP_20\n");
            break;
        case AE_EV_COMP_00:	// +0 EV
            OV5646MIPISENSORDB("[4EC] HDR AE_EV_COMP_00\n");
            break;
        case AE_EV_COMP_n10:  // -1 EV
        case AE_EV_COMP_n20:  // -2 EV
            totalGain = totalGain >> 1;
            exposureTime = exposureTime >> 1;
            OV5646MIPISENSORDB("[4EC] HDR AE_EV_COMP_n20\n");
            break;
        default:
            break;//return FALSE;
    }
    totalGain = (totalGain > OV5646MIPI_MAX_AXD_GAIN) ? OV5646MIPI_MAX_AXD_GAIN : totalGain;
    //exposureTime = (exposureTime > OV5646MIPI_MAX_EXPOSURE_TIME) ? OV5646MIPI_MAX_EXPOSURE_TIME : exposureTime;
    OV5646MIPIWriteSensorGain(totalGain);
    OV5646MIPIWriteShutter(exposureTime);
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_param_exposure_for_HDR function:\n ");
    return TRUE;
}
UINT32 OV5646MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIGetResolution function:\n ");
	pSensorResolution->SensorPreviewWidth=  OV5646MIPI_IMAGE_SENSOR_SVGA_WIDTH-2*OV5646MIPI_PV_GRAB_START_X;
	pSensorResolution->SensorPreviewHeight= OV5646MIPI_IMAGE_SENSOR_SVGA_HEIGHT-2*OV5646MIPI_PV_GRAB_START_Y;
	pSensorResolution->SensorFullWidth= OV5646MIPI_IMAGE_SENSOR_QSXGA_WITDH-2*OV5646MIPI_FULL_GRAB_START_X; 
	pSensorResolution->SensorFullHeight= OV5646MIPI_IMAGE_SENSOR_QSXGA_HEIGHT-2*OV5646MIPI_FULL_GRAB_START_Y;
	pSensorResolution->SensorVideoWidth= OV5646MIPI_IMAGE_SENSOR_SVGA_WIDTH-2*OV5646MIPI_PV_GRAB_START_X; 
	pSensorResolution->SensorVideoHeight= OV5646MIPI_IMAGE_SENSOR_SVGA_HEIGHT-2*OV5646MIPI_PV_GRAB_START_Y;;
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIGetResolution function:\n ");
	return ERROR_NONE;
}	/* OV5646MIPIGetResolution() */

UINT32 OV5646MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,MSDK_SENSOR_INFO_STRUCT *pSensorInfo,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIGetInfo function:\n ");
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorPreviewResolutionX=OV5646MIPI_IMAGE_SENSOR_SVGA_WIDTH-2*OV5646MIPI_FULL_GRAB_START_X;//OV5646MIPI_IMAGE_SENSOR_QSXGA_WITDH ;
			pSensorInfo->SensorPreviewResolutionY=OV5646MIPI_IMAGE_SENSOR_SVGA_HEIGHT-2*OV5646MIPI_FULL_GRAB_START_Y;//OV5646MIPI_IMAGE_SENSOR_QSXGA_HEIGHT ;
			pSensorInfo->SensorCameraPreviewFrameRate=15;
			break;
		default:
			pSensorInfo->SensorPreviewResolutionX=OV5646MIPI_IMAGE_SENSOR_SVGA_WIDTH-2*OV5646MIPI_PV_GRAB_START_X;
			pSensorInfo->SensorPreviewResolutionY=OV5646MIPI_IMAGE_SENSOR_SVGA_HEIGHT-2*OV5646MIPI_PV_GRAB_START_Y;
			pSensorInfo->SensorCameraPreviewFrameRate=30;
			break;
	}		 		
	pSensorInfo->SensorFullResolutionX= OV5646MIPI_IMAGE_SENSOR_QSXGA_WITDH-2*OV5646MIPI_FULL_GRAB_START_X;
	pSensorInfo->SensorFullResolutionY= OV5646MIPI_IMAGE_SENSOR_QSXGA_HEIGHT-2*OV5646MIPI_FULL_GRAB_START_Y;
	//pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=5;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=4;
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;  
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 2;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;
	pSensorInfo->CaptureDelayFrame = 2;
	pSensorInfo->PreviewDelayFrame = 3; 
	pSensorInfo->VideoDelayFrame = 3; 		
	pSensorInfo->SensorMasterClockSwitch = 0; 
	pSensorInfo->YUVAwbDelayFrame = 3;
	pSensorInfo->YUVEffectDelayFrame= 3; 
	pSensorInfo->AEShutDelayFrame= 0;
 	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   		
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=	5;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = OV5646MIPI_PV_GRAB_START_X; 
			pSensorInfo->SensorGrabStartY = OV5646MIPI_PV_GRAB_START_Y;   
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
			pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			pSensorInfo->SensorWidthSampling = 0; 
			pSensorInfo->SensorHightSampling = 0;  	
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
			pSensorInfo->SensorGrabStartX = OV5646MIPI_FULL_GRAB_START_X; 
			pSensorInfo->SensorGrabStartY = OV5646MIPI_FULL_GRAB_START_Y;             
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
			pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount =14; 
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->SensorWidthSampling = 0; 
			pSensorInfo->SensorHightSampling = 0;
			pSensorInfo->SensorPacketECCOrder = 1;
			break;
		default:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=5;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV5646MIPI_PV_GRAB_START_X; 
			pSensorInfo->SensorGrabStartY = OV5646MIPI_PV_GRAB_START_Y; 			
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
			pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			pSensorInfo->SensorWidthSampling = 0;
			pSensorInfo->SensorHightSampling = 0;	
			pSensorInfo->SensorPacketECCOrder = 1;
			break;
	}
	memcpy(pSensorConfigData, &OV5646MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));	
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIGetInfo function:\n ");	
	return ERROR_NONE;
}	/* OV5646MIPIGetInfo() */

UINT32 OV5646MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	  OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIControl function:\n ");
	  spin_lock(&ov5646mipi_drv_lock);
	  OV5646CurrentScenarioId = ScenarioId;
	  spin_unlock(&ov5646mipi_drv_lock);
	  switch (ScenarioId)
	  {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		    OV5646MIPIPreview(pImageWindow, pSensorConfigData);
		    break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
		    OV5646MIPIZSDPreview(pImageWindow, pSensorConfigData);
		    break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		    OV5646MIPICapture(pImageWindow, pSensorConfigData);
		    break;
		default:
		    return ERROR_INVALID_SCENARIO_ID;
	  }
	  OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIControl function:\n ");
	  return ERROR_NONE;
}	/* OV5646MIPIControl() */
BOOL OV5646MIPI_set_param_wb(UINT16 para)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_param_wb function:para == %d\n",para);
    spin_lock(&ov5646mipi_drv_lock);
    OV5646MIPISensor.awbMode = para;
    spin_unlock(&ov5646mipi_drv_lock);
    switch (para)
    {
        case AWB_MODE_OFF:
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPI_AWB_ENABLE = KAL_FALSE; 
            spin_unlock(&ov5646mipi_drv_lock);
            OV5646MIPI_set_AWB_mode(OV5646MIPI_AWB_ENABLE);
            break;                    
        case AWB_MODE_AUTO:
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPI_AWB_ENABLE = KAL_TRUE; 
            spin_unlock(&ov5646mipi_drv_lock);
            OV5646MIPI_set_AWB_mode(OV5646MIPI_AWB_ENABLE);
            break;
        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
            OV5646MIPI_write_cmos_sensor(0x3212,0x03); 
            OV5646MIPI_set_AWB_mode(KAL_FALSE);         	                
            OV5646MIPI_write_cmos_sensor(0x3400,0x06); 
            OV5646MIPI_write_cmos_sensor(0x3401,0x30); 
            OV5646MIPI_write_cmos_sensor(0x3402,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3403,0x00); 
            OV5646MIPI_write_cmos_sensor(0x3404,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3405,0x30);
            OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
            OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
            break;
        case AWB_MODE_DAYLIGHT: //sunny
            OV5646MIPI_write_cmos_sensor(0x3212,0x03);
            OV5646MIPI_set_AWB_mode(KAL_FALSE);                           
            OV5646MIPI_write_cmos_sensor(0x3400,0x06); 
            OV5646MIPI_write_cmos_sensor(0x3401,0x10); 
            OV5646MIPI_write_cmos_sensor(0x3402,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3403,0x00); 
            OV5646MIPI_write_cmos_sensor(0x3404,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3405,0x48);
            OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
            OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
            break;
        case AWB_MODE_INCANDESCENT: //office
            OV5646MIPI_write_cmos_sensor(0x3212,0x03);
            OV5646MIPI_set_AWB_mode(KAL_FALSE);                           
            OV5646MIPI_write_cmos_sensor(0x3400,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3401,0xe0); 
            OV5646MIPI_write_cmos_sensor(0x3402,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3403,0x00); 
            OV5646MIPI_write_cmos_sensor(0x3404,0x05); 
            OV5646MIPI_write_cmos_sensor(0x3405,0xa0);
            OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
            OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
            break; 
        case AWB_MODE_TUNGSTEN:
            OV5646MIPI_write_cmos_sensor(0x3212,0x03);
            OV5646MIPI_set_AWB_mode(KAL_FALSE);                         
            OV5646MIPI_write_cmos_sensor(0x3400,0x05); 
            OV5646MIPI_write_cmos_sensor(0x3401,0x48); 
            OV5646MIPI_write_cmos_sensor(0x3402,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3403,0x00); 
            OV5646MIPI_write_cmos_sensor(0x3404,0x05); 
            OV5646MIPI_write_cmos_sensor(0x3405,0xe0); 
            OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
            OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
            break;
        case AWB_MODE_FLUORESCENT:
            OV5646MIPI_write_cmos_sensor(0x3212,0x03);
            OV5646MIPI_set_AWB_mode(KAL_FALSE);                           
            OV5646MIPI_write_cmos_sensor(0x3400,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3401,0x00); 
            OV5646MIPI_write_cmos_sensor(0x3402,0x04); 
            OV5646MIPI_write_cmos_sensor(0x3403,0x00); 
            OV5646MIPI_write_cmos_sensor(0x3404,0x06); 
            OV5646MIPI_write_cmos_sensor(0x3405,0x50); 
            OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
            OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
            break;

        default:
            break;
    }
    spin_lock(&ov5646mipi_drv_lock);
    OV5646MIPISensor.iWB = para;
    spin_unlock(&ov5646mipi_drv_lock);
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_param_wb function:\n ");
    return TRUE;
} /* OV5646MIPI_set_param_wb */

void OV5646MIPI_set_contrast(UINT16 para)
{   
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_contrast function:para == %d\n",para);
    switch (para)
    {
        case ISP_CONTRAST_LOW:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5586,0x14);
             OV5646MIPI_write_cmos_sensor(0x5585,0x14);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_CONTRAST_HIGH:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5586,0x2c);
             OV5646MIPI_write_cmos_sensor(0x5585,0x1c);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_CONTRAST_MIDDLE:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5586,0x20);
             OV5646MIPI_write_cmos_sensor(0x5585,0x00);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        default:
             break;
    }
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_contrast function:\n ");
    return;
}

void OV5646MIPI_set_brightness(UINT16 para)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_brightness function:para == %d\n",para);
    switch (para)
    {
        case ISP_BRIGHT_LOW:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5587,0x40);
             OV5646MIPI_write_cmos_sensor(0x5588,0x09);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_BRIGHT_HIGH:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5587,0x40);
             OV5646MIPI_write_cmos_sensor(0x5588,0x01);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_BRIGHT_MIDDLE:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5587,0x00);
             OV5646MIPI_write_cmos_sensor(0x5588,0x01);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        default:
             return KAL_FALSE;
             break;
    }
    mDELAY(20);
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_brightness function:\n ");
    return;
}
void OV5646MIPI_set_saturation(UINT16 para)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_saturation function:para == %d\n",para);
    switch (para)
    {
        case ISP_SAT_HIGH:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5381,0x22);//ccm
             OV5646MIPI_write_cmos_sensor(0x5382,0x55);//
             OV5646MIPI_write_cmos_sensor(0x5383,0x12);//
             OV5646MIPI_write_cmos_sensor(0x5384,0x04 );//
             OV5646MIPI_write_cmos_sensor(0x5385,0x94);//
             OV5646MIPI_write_cmos_sensor(0x5386,0x98);//
             OV5646MIPI_write_cmos_sensor(0x5387,0xa9);//
             OV5646MIPI_write_cmos_sensor(0x5388,0x9a);//
             OV5646MIPI_write_cmos_sensor(0x5389,0x0e);//    
             OV5646MIPI_write_cmos_sensor(0x538a,0x01);//      
             OV5646MIPI_write_cmos_sensor(0x538b,0x98);//      			                                           
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_SAT_LOW:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5381,0x22);//ccm
             OV5646MIPI_write_cmos_sensor(0x5382,0x55);//
             OV5646MIPI_write_cmos_sensor(0x5383,0x12);//
             OV5646MIPI_write_cmos_sensor(0x5384,0x02);//
             OV5646MIPI_write_cmos_sensor(0x5385,0x62);//
             OV5646MIPI_write_cmos_sensor(0x5386,0x66);//
             OV5646MIPI_write_cmos_sensor(0x5387,0x71);//
             OV5646MIPI_write_cmos_sensor(0x5388,0x66);//
             OV5646MIPI_write_cmos_sensor(0x5389,0x0a);//    
             OV5646MIPI_write_cmos_sensor(0x538a,0x01);//      
             OV5646MIPI_write_cmos_sensor(0x538b,0x98);//      			                                           
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case ISP_SAT_MIDDLE:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5381,0x24);//ccm //0x22
             OV5646MIPI_write_cmos_sensor(0x5382,0x5b);// //0x55
             OV5646MIPI_write_cmos_sensor(0x5383,0x13);// //0x12
             OV5646MIPI_write_cmos_sensor(0x5384,0x3 );//
#if 0
             OV5646MIPI_write_cmos_sensor(0x5385,0x74);//
             OV5646MIPI_write_cmos_sensor(0x5386,0x77);//
             OV5646MIPI_write_cmos_sensor(0x5387,0x84);//
             OV5646MIPI_write_cmos_sensor(0x5388,0x78);//
             OV5646MIPI_write_cmos_sensor(0x5389,0x0C);//
#else
             OV5646MIPI_write_cmos_sensor(0x5385,0x7f);//
             OV5646MIPI_write_cmos_sensor(0x5386,0x83);//
             OV5646MIPI_write_cmos_sensor(0x5387,0x91);//
             OV5646MIPI_write_cmos_sensor(0x5388,0x84);//
             OV5646MIPI_write_cmos_sensor(0x5389,0x0d);//
#endif
             OV5646MIPI_write_cmos_sensor(0x538a,0x01);//      
             OV5646MIPI_write_cmos_sensor(0x538b,0x98);//      			                                           
             OV5646MIPI_write_cmos_sensor(0x3212,0x13);
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        default:
             return KAL_FALSE;
             break;
    }
    mDELAY(50);
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_saturation function:\n ");
    return;
}
void OV5646MIPI_scene_mode_PORTRAIT()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_scene_mode_PORTRAIT function:\n ");
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.NightMode = KAL_FALSE;
	spin_unlock(&ov5646mipi_drv_lock);

	OV5646MIPI_write_cmos_sensor(0x3212,0x03);  
	/*FRAME rate*/
	OV5646MIPI_write_cmos_sensor(0x3A00,0x3c); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a03,0x88);
	OV5646MIPI_write_cmos_sensor(0x3a14,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a15,0x88);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
	/*AE Weight - CenterAverage*/
	OV5646MIPI_write_cmos_sensor(0x501d,0x00);
	OV5646MIPI_write_cmos_sensor(0x5688,0x11);
	OV5646MIPI_write_cmos_sensor(0x5689,0x11);
	OV5646MIPI_write_cmos_sensor(0x568a,0x21);
	OV5646MIPI_write_cmos_sensor(0x568b,0x12);
	OV5646MIPI_write_cmos_sensor(0x568c,0x12);
	OV5646MIPI_write_cmos_sensor(0x568d,0x21);
	OV5646MIPI_write_cmos_sensor(0x568e,0x11);
	OV5646MIPI_write_cmos_sensor(0x568f,0x11);
	OV5646MIPI_write_cmos_sensor(0x3212,0x13);
	OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_scene_mode_PORTRAIT function:\n ");
}
void OV5646MIPI_scene_mode_LANDSCAPE()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_scene_mode_LANDSCAPE function:\n ");
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.NightMode = KAL_FALSE;
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPI_write_cmos_sensor(0x3212,0x03); 
	/*FRAME rate*/
	OV5646MIPI_write_cmos_sensor(0x3A00,0x3c); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a03,0x88);
	OV5646MIPI_write_cmos_sensor(0x3a14,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a15,0x88);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
	/*AE Weight - CenterAverage*/
	OV5646MIPI_write_cmos_sensor(0x501d,0x00);
	OV5646MIPI_write_cmos_sensor(0x5688,0x11);
	OV5646MIPI_write_cmos_sensor(0x5689,0x11);
	OV5646MIPI_write_cmos_sensor(0x568a,0x11);
	OV5646MIPI_write_cmos_sensor(0x568b,0x11);
	OV5646MIPI_write_cmos_sensor(0x568c,0x11);
	OV5646MIPI_write_cmos_sensor(0x568d,0x11);
	OV5646MIPI_write_cmos_sensor(0x568e,0x11);
	OV5646MIPI_write_cmos_sensor(0x568f,0x11);
	OV5646MIPI_write_cmos_sensor(0x3212,0x13);
	OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_scene_mode_LANDSCAPE function:\n ");
}
void OV5646MIPI_scene_mode_SUNSET()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_scene_mode_SUNSET function:\n ");
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.NightMode = KAL_FALSE;
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPI_write_cmos_sensor(0x3212,0x03);
	/*FRAME rate*/
	OV5646MIPI_write_cmos_sensor(0x3A00,0x3c); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x0b); 
	OV5646MIPI_write_cmos_sensor(0x3a03,0x88);                         
	OV5646MIPI_write_cmos_sensor(0x3a14,0x0b); 
	OV5646MIPI_write_cmos_sensor(0x3a15,0x88);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
	/*AE Weight - CenterAverage*/
	OV5646MIPI_write_cmos_sensor(0x501d,0x00);
	OV5646MIPI_write_cmos_sensor(0x5688,0x11);
	OV5646MIPI_write_cmos_sensor(0x5689,0x11);
	OV5646MIPI_write_cmos_sensor(0x568a,0x11);
	OV5646MIPI_write_cmos_sensor(0x568b,0x11);
	OV5646MIPI_write_cmos_sensor(0x568c,0x11);
	OV5646MIPI_write_cmos_sensor(0x568d,0x11);
	OV5646MIPI_write_cmos_sensor(0x568e,0x11);
	OV5646MIPI_write_cmos_sensor(0x568f,0x11);
	OV5646MIPI_write_cmos_sensor(0x3212,0x13);
	OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_scene_mode_SUNSET function:\n ");
}
void OV5646MIPI_scene_mode_SPORTS()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_scene_mode_SPORTS function:\n ");
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.NightMode = KAL_FALSE;
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPI_write_cmos_sensor(0x3212,0x03);
	/*FRAME rate*/
#if 0   //Fix SBELI-302
	OV5646MIPI_write_cmos_sensor(0x3A00,0x38); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x03);
	OV5646MIPI_write_cmos_sensor(0x3a03,0xd8);
	OV5646MIPI_write_cmos_sensor(0x3a14,0x03);
	OV5646MIPI_write_cmos_sensor(0x3a15,0xd8);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
#else
	OV5646MIPI_write_cmos_sensor(0x3A00,0x3c); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a03,0x88);
	OV5646MIPI_write_cmos_sensor(0x3a14,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a15,0x88);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
#endif
	/*AE Weight - CenterAverage*/
	OV5646MIPI_write_cmos_sensor(0x501d,0x00);
	OV5646MIPI_write_cmos_sensor(0x5688,0x11);
	OV5646MIPI_write_cmos_sensor(0x5689,0x11);
	OV5646MIPI_write_cmos_sensor(0x568a,0x11);
	OV5646MIPI_write_cmos_sensor(0x568b,0x11);
	OV5646MIPI_write_cmos_sensor(0x568c,0x11);
	OV5646MIPI_write_cmos_sensor(0x568d,0x11);
	OV5646MIPI_write_cmos_sensor(0x568e,0x11);
	OV5646MIPI_write_cmos_sensor(0x568f,0x11);
	OV5646MIPI_write_cmos_sensor(0x3212,0x13);
	OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_scene_mode_SPORTS function:\n ");
}
void OV5646MIPI_scene_mode_OFF()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_scene_mode_OFF function:\n ");
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.NightMode = KAL_FALSE;
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPI_write_cmos_sensor(0x3212,0x03);
	/*FRAME rate*/
	OV5646MIPI_write_cmos_sensor(0x3A00,0x3c); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a03,0x88);
	OV5646MIPI_write_cmos_sensor(0x3a14,0x0b);
	OV5646MIPI_write_cmos_sensor(0x3a15,0x88);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
	/*AE Weight - CenterAverage*/
	OV5646MIPI_write_cmos_sensor(0x501d,0x00);
	OV5646MIPI_write_cmos_sensor(0x5688,0x11);
	OV5646MIPI_write_cmos_sensor(0x5689,0x11);
	OV5646MIPI_write_cmos_sensor(0x568a,0x11);
	OV5646MIPI_write_cmos_sensor(0x568b,0x11);
	OV5646MIPI_write_cmos_sensor(0x568c,0x11);
	OV5646MIPI_write_cmos_sensor(0x568d,0x11);
	OV5646MIPI_write_cmos_sensor(0x568e,0x11);
	OV5646MIPI_write_cmos_sensor(0x568f,0x11);
	OV5646MIPI_write_cmos_sensor(0x3212,0x13);
	OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
	mDELAY(100);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_scene_mode_OFF function:\n ");
}
void OV5646MIPI_scene_mode_NIGHT()
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_scene_mode_NIGHT function:\n ");
	spin_lock(&ov5646mipi_drv_lock);
	OV5646MIPISensor.NightMode = KAL_TRUE;
	spin_unlock(&ov5646mipi_drv_lock);
	OV5646MIPI_write_cmos_sensor(0x3212,0x03);
	/*FRAME rate*/
	OV5646MIPI_write_cmos_sensor(0x3A00,0x3c); //10-30
	OV5646MIPI_write_cmos_sensor(0x3a02,0x17);
	OV5646MIPI_write_cmos_sensor(0x3a03,0x10);
	OV5646MIPI_write_cmos_sensor(0x3a14,0x17);
	OV5646MIPI_write_cmos_sensor(0x3a15,0x10);
	OV5646MIPI_write_cmos_sensor(0x350c,0x00);
	OV5646MIPI_write_cmos_sensor(0x350d,0x00);
	/*AE Weight - CenterAverage*/
	OV5646MIPI_write_cmos_sensor(0x501d,0x00);
	OV5646MIPI_write_cmos_sensor(0x5688,0x11);
	OV5646MIPI_write_cmos_sensor(0x5689,0x11);
	OV5646MIPI_write_cmos_sensor(0x568a,0x11);
	OV5646MIPI_write_cmos_sensor(0x568b,0x11);
	OV5646MIPI_write_cmos_sensor(0x568c,0x11);
	OV5646MIPI_write_cmos_sensor(0x568d,0x11);
	OV5646MIPI_write_cmos_sensor(0x568e,0x11);
	OV5646MIPI_write_cmos_sensor(0x568f,0x11);
	OV5646MIPI_write_cmos_sensor(0x3212,0x13);
	OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_scene_mode_NIGHT function:\n ");
}

void OV5646MIPI_set_scene_mode(UINT16 para)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_scene_mode function:\n ");	
    OV5646MIPISENSORDB("[OV5646MIPI] OV5646MIPI_set_scene_mode=%d\n",para);
    spin_lock(&ov5646mipi_drv_lock);
    OV5646MIPISensor.sceneMode=para;
    spin_unlock(&ov5646mipi_drv_lock);
    switch (para)
    {
        case SCENE_MODE_NIGHTSCENE:
            OV5646MIPI_scene_mode_NIGHT(); 
            break;
        case SCENE_MODE_PORTRAIT:
            OV5646MIPI_scene_mode_PORTRAIT();		 
            break;
        case SCENE_MODE_LANDSCAPE:
            OV5646MIPI_scene_mode_LANDSCAPE();		 
            break;
        case SCENE_MODE_SUNSET:
            OV5646MIPI_scene_mode_SUNSET();		 
            break;
        case SCENE_MODE_SPORTS:
            OV5646MIPI_scene_mode_SPORTS();
            break;
        case SCENE_MODE_HDR:
            if (1 == OV5646MIPISensor.manualAEStart)
            {
                OV5646MIPI_set_AE_mode(KAL_TRUE);//Manual AE disable
                spin_lock(&ov5646mipi_drv_lock);
            	OV5646MIPISensor.manualAEStart = 0;
                OV5646MIPISensor.currentExposureTime = 0;
                OV5646MIPISensor.currentAxDGain = 0;
                spin_unlock(&ov5646mipi_drv_lock);
            }
            break;
        case SCENE_MODE_OFF:
            OV5646MIPISENSORDB("[OV5646MIPI]set SCENE_MODE_OFF :\n ");
            OV5646MIPI_scene_mode_OFF();
            break;
        default:
            OV5646MIPISENSORDB("[OV5646MIPI]set default mode :\n ");
            OV5646MIPI_scene_mode_OFF();
            break;
    }
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_scene_mode function:\n ");
    return;
}
void OV5646MIPI_set_iso(UINT16 para)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_iso function:para == %d\n",para);
    spin_lock(&ov5646mipi_drv_lock);
    OV5646MIPISensor.isoSpeed = para;
    spin_unlock(&ov5646mipi_drv_lock);   
    switch (para)
    {
        case AE_ISO_100:
             OV5646MIPI_write_cmos_sensor(0x3a18, 0x00);
             OV5646MIPI_write_cmos_sensor(0x3a19, 0x60);
             break;
        case AE_ISO_200:
             //ISO 200
             OV5646MIPI_write_cmos_sensor(0x3a18, 0x00);
             OV5646MIPI_write_cmos_sensor(0x3a19, 0x90);
             break;
        case AE_ISO_400:
             //ISO 400
             OV5646MIPI_write_cmos_sensor(0x3a18, 0x00);
             OV5646MIPI_write_cmos_sensor(0x3a19, 0xc0);
             break;
        default:
             break;
    }
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_iso function:\n ");
    return;
}

BOOL OV5646MIPI_set_param_effect(UINT16 para)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_param_effect function:para == %d\n",para);
    switch (para)
    {
        case MEFFECT_OFF:  
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5580,0x06); 
             OV5646MIPI_write_cmos_sensor(0x5583,0x40); 
             OV5646MIPI_write_cmos_sensor(0x5584,0x20);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case MEFFECT_SEPIA: 
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5580,0x1e);
             OV5646MIPI_write_cmos_sensor(0x5583,0x40); 
             OV5646MIPI_write_cmos_sensor(0x5584,0xa0);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case MEFFECT_NEGATIVE:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5580,0x46);
             OV5646MIPI_write_cmos_sensor(0x5583,0x40); 
             OV5646MIPI_write_cmos_sensor(0x5584,0x20);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case MEFFECT_SEPIAGREEN:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5580,0x1e);			 
             OV5646MIPI_write_cmos_sensor(0x5583,0x60); 
             OV5646MIPI_write_cmos_sensor(0x5584,0x60);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case MEFFECT_SEPIABLUE:
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5580,0x1e);             
             OV5646MIPI_write_cmos_sensor(0x5583,0xa0); 
             OV5646MIPI_write_cmos_sensor(0x5584,0x40);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        case MEFFECT_MONO: //B&W
             OV5646MIPI_write_cmos_sensor(0x3212,0x03);
             OV5646MIPI_write_cmos_sensor(0x5580,0x1e);      		
             OV5646MIPI_write_cmos_sensor(0x5583,0x80); 
             OV5646MIPI_write_cmos_sensor(0x5584,0x80);
             OV5646MIPI_write_cmos_sensor(0x3212,0x13); 
             OV5646MIPI_write_cmos_sensor(0x3212,0xa3);
             break;
        default:
             //return KAL_FALSE;
             break;
    }
    mDELAY(50);
    OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_param_effect function:\n ");
    return KAL_FALSE;
} /* OV5646MIPI_set_param_effect */

BOOL OV5646MIPI_set_param_banding(UINT16 para)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_param_banding function:para == %d\n",para);
	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			spin_lock(&ov5646mipi_drv_lock);
			OV5646MIPI_Banding_setting = AE_FLICKER_MODE_50HZ;
			spin_unlock(&ov5646mipi_drv_lock);
			OV5646MIPI_write_cmos_sensor(0x3c00,0x04);
			OV5646MIPI_write_cmos_sensor(0x3c01,0x80);
			break;
	case AE_FLICKER_MODE_60HZ:			
			spin_lock(&ov5646mipi_drv_lock);
			OV5646MIPI_Banding_setting = AE_FLICKER_MODE_60HZ;
			spin_unlock(&ov5646mipi_drv_lock);
			OV5646MIPI_write_cmos_sensor(0x3c00,0x00);
			OV5646MIPI_write_cmos_sensor(0x3c01,0x80);
			break;
	default:
			//return FALSE;
			break;
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_param_banding function:\n ");
	return TRUE;
} /* OV5646MIPI_set_param_banding */

BOOL OV5646MIPI_set_param_exposure(UINT16 para)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_set_param_exposure function:para == %d\n",para);
	OV5646MIPISENSORDB("[OV5646MIPI]para=%d:\n",para);
	//spin_lock(&ov5646mipi_drv_lock);
	if (SCENE_MODE_HDR == OV5646MIPISensor.sceneMode && SENSOR_MODE_CAPTURE == OV5646MIPISensor.SensorMode)
	{
		//spin_unlock(&ov5646mipi_drv_lock);
		OV5646MIPI_set_param_exposure_for_HDR(para);
		return TRUE;
	}
	//spin_unlock(&ov5646mipi_drv_lock);
	switch (para)
	{	
		case AE_EV_COMP_20:	                   
			OV5646MIPI_write_cmos_sensor(0x3a0f, 0x50);//	; AEC in H
			OV5646MIPI_write_cmos_sensor(0x3a10, 0x48);//	; AEC in L
			OV5646MIPI_write_cmos_sensor(0x3a11, 0x90);//	; AEC out H
			OV5646MIPI_write_cmos_sensor(0x3a1b, 0x50);//	; AEC out L
			OV5646MIPI_write_cmos_sensor(0x3a1e, 0x48);//	; control zone H
			OV5646MIPI_write_cmos_sensor(0x3a1f, 0x24);//	; control zone L   
			break;
		case AE_EV_COMP_10:	                   
			OV5646MIPI_write_cmos_sensor(0x3a0f, 0x40);//	; AEC in H
			OV5646MIPI_write_cmos_sensor(0x3a10, 0x38);//	; AEC in L
			OV5646MIPI_write_cmos_sensor(0x3a11, 0x80);//	; AEC out H
			OV5646MIPI_write_cmos_sensor(0x3a1b, 0x40);//	; AEC out L
			OV5646MIPI_write_cmos_sensor(0x3a1e, 0x38);//	; control zone H
			OV5646MIPI_write_cmos_sensor(0x3a1f, 0x1c);//	; control zone L   
			break;
		case AE_EV_COMP_00:
			OV5646MIPI_write_cmos_sensor(0x3a0f, 0x29);//	; AEC in H
			OV5646MIPI_write_cmos_sensor(0x3a10, 0x23);//	; AEC in L
			OV5646MIPI_write_cmos_sensor(0x3a11, 0x53);//	; AEC out H
			OV5646MIPI_write_cmos_sensor(0x3a1b, 0x29);//	; AEC out L
			OV5646MIPI_write_cmos_sensor(0x3a1e, 0x23);//	; control zone H
			OV5646MIPI_write_cmos_sensor(0x3a1f, 0x12);//	; control zone L  
			break;
		case AE_EV_COMP_n10:
			OV5646MIPI_write_cmos_sensor(0x3a0f, 0x22);//	; AEC in H
			OV5646MIPI_write_cmos_sensor(0x3a10, 0x1e);//	; AEC in L
			OV5646MIPI_write_cmos_sensor(0x3a11, 0x45);//	; AEC out H
			OV5646MIPI_write_cmos_sensor(0x3a1b, 0x22);//	; AEC out L
			OV5646MIPI_write_cmos_sensor(0x3a1e, 0x1e);//	; control zone H
			OV5646MIPI_write_cmos_sensor(0x3a1f, 0x10);//	; control zone L   
			break;
		case AE_EV_COMP_n20:  // -2 EV
			OV5646MIPI_write_cmos_sensor(0x3a0f, 0x1e);//	; AEC in H
			OV5646MIPI_write_cmos_sensor(0x3a10, 0x18);//	; AEC in L
			OV5646MIPI_write_cmos_sensor(0x3a11, 0x3c);//	; AEC out H
			OV5646MIPI_write_cmos_sensor(0x3a1b, 0x1e);//	; AEC out L
			OV5646MIPI_write_cmos_sensor(0x3a1e, 0x18);//	; control zone H
			OV5646MIPI_write_cmos_sensor(0x3a1f, 0x0c);//	; control zone L
			break;
		default:
			//return FALSE;
			break;
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_set_param_exposure function:\n ");
	return TRUE;
} /* OV5646MIPI_set_param_exposure */

UINT32 OV5646MIPIYUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
	OV5646MIPISENSORDB("OV5646MIPIYUVSensorSetting:iCmd=%d,iPara=%d, %d \n",iCmd, iPara);
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIYUVSensorSetting function:\n ");
	switch (iCmd) {
		case FID_SCENE_MODE:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_SCENE_MODE\n");
			OV5646MIPI_set_scene_mode(iPara);
			break; 	    
		case FID_AWB_MODE:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_AWB_MODE para=%d\n", iPara);
			OV5646MIPI_set_param_wb(iPara);
			break;
		case FID_COLOR_EFFECT:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_COLOR_EFFECT para=%d\n", iPara);
			OV5646MIPI_set_param_effect(iPara);
			break;
		case FID_AE_EV:   
			OV5646MIPI_set_param_exposure(iPara);
			OV5646MIPISENSORDB("[OV5646MIPI]FID_AE_EV para=%d\n", iPara);
			break;
		case FID_AE_FLICKER:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_AE_FLICKER para=%d\n", iPara);
			OV5646MIPI_set_param_banding(iPara);
			break;
		case FID_AE_SCENE_MODE: 
			OV5646MIPISENSORDB("[OV5646MIPI]FID_AE_SCENE_MODE para=%d\n", iPara);
			break; 
		case FID_ISP_CONTRAST:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_ISP_CONTRAST para=%d\n", iPara);
			OV5646MIPI_set_contrast(iPara);
			break;
		case FID_ISP_BRIGHT:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_ISP_BRIGHT para=%d\n", iPara);
			OV5646MIPI_set_brightness(iPara);
			break;
		case FID_ISP_SAT:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_ISP_SAT para=%d\n", iPara);
			OV5646MIPI_set_saturation(iPara);
			break;
		case FID_ZOOM_FACTOR:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_ZOOM_FACTOR:%d\n", iPara); 	    
			spin_lock(&ov5646mipi_drv_lock);
			OV5646MIPI_zoom_factor = iPara; 
			spin_unlock(&ov5646mipi_drv_lock);
			break;
		case FID_AE_ISO:
			OV5646MIPISENSORDB("[OV5646MIPI]FID_AE_ISO:%d\n", iPara);
			OV5646MIPI_set_iso(iPara);
			break;           
		default:
			break;
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIYUVSensorSetting function:\n ");
	return TRUE;
}   /* OV5646MIPIYUVSensorSetting */

UINT32 OV5646MIPIYUVSetVideoMode(UINT16 u2FrameRate)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIYUVSetVideoMode function:\n ");
	OV5646MIPI_flash_mode = 2;
	if (u2FrameRate == 30)
	{
		//;OV5646MIPI 1280x960,30fps
		//56Mhz, 224Mbps/Lane, 2Lane.
		OV5646MIPISENSORDB("[OV5646MIPI]OV5646MIPIYUVSetVideoMode enter u2FrameRate == 30 setting  :\n ");	
		OV5646MIPI_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
		OV5646MIPI_write_cmos_sensor(0x3034, 0x18); // PLL, MIPI 8-bit mode
		OV5646MIPI_write_cmos_sensor(0x3035, 0x21); // PLL
		OV5646MIPI_write_cmos_sensor(0x3036, 0x70); // PLL
		OV5646MIPI_write_cmos_sensor(0x3037, 0x13); // PLL
		OV5646MIPI_write_cmos_sensor(0x3108, 0x01); // PLL
		OV5646MIPI_write_cmos_sensor(0x3824, 0x01); // PLL
		OV5646MIPI_write_cmos_sensor(0x460c, 0x20); // PLL
		OV5646MIPI_write_cmos_sensor(0x3618, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x3600, 0x09);//
		OV5646MIPI_write_cmos_sensor(0x3601, 0x43);//
		OV5646MIPI_write_cmos_sensor(0x3708, 0x66);//
		OV5646MIPI_write_cmos_sensor(0x3709, 0x12);//
		OV5646MIPI_write_cmos_sensor(0x370c, 0xc3);//
		OV5646MIPI_write_cmos_sensor(0x3800, 0x00); // HS = 0
		OV5646MIPI_write_cmos_sensor(0x3801, 0x00); // HS
		OV5646MIPI_write_cmos_sensor(0x3802, 0x00); // VS = 250
		OV5646MIPI_write_cmos_sensor(0x3803, 0x06); // VS
		OV5646MIPI_write_cmos_sensor(0x3804, 0x0a); // HW = 2623
		OV5646MIPI_write_cmos_sensor(0x3805, 0x3f);//	; HW
		OV5646MIPI_write_cmos_sensor(0x3806, 0x07);//	; VH = 
		OV5646MIPI_write_cmos_sensor(0x3807, 0x9d);//	; VH
		OV5646MIPI_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
		OV5646MIPI_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
		OV5646MIPI_write_cmos_sensor(0x380a, 0x03);//	; DVPVO = 960
		OV5646MIPI_write_cmos_sensor(0x380b, 0xc0);//	; DVPVO
		OV5646MIPI_write_cmos_sensor(0x380c, 0x07);//	; HTS = 2160
		OV5646MIPI_write_cmos_sensor(0x380d, 0x68);//	; HTS
		OV5646MIPI_write_cmos_sensor(0x380e, 0x03);//	; VTS = 740
		OV5646MIPI_write_cmos_sensor(0x380f, 0xd8);//	; VTS
		OV5646MIPI_write_cmos_sensor(0x3810, 0x00); // H OFF = 16
		OV5646MIPI_write_cmos_sensor(0x3811, 0x10); // H OFF
		OV5646MIPI_write_cmos_sensor(0x3812, 0x00); // V OFF = 4
		OV5646MIPI_write_cmos_sensor(0x3813, 0x06);//	; V OFF
		OV5646MIPI_write_cmos_sensor(0x3814, 0x31);//	; X INC
		OV5646MIPI_write_cmos_sensor(0x3815, 0x31);//	; Y INC
		OV5646MIPI_write_cmos_sensor(0x3820, 0x47);//	; flip off, V bin on
		OV5646MIPI_write_cmos_sensor(0x3821, 0x01);//	; mirror on, H bin on
		OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
		OV5646MIPI_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
		OV5646MIPI_write_cmos_sensor(0x3a02, 0x03);//	; max exp 60 = 740
		OV5646MIPI_write_cmos_sensor(0x3a03, 0xd8);//	; max exp 60
		OV5646MIPI_write_cmos_sensor(0x3a08, 0x01);//	; B50 = 222
		OV5646MIPI_write_cmos_sensor(0x3a09, 0x27);//	; B50
		OV5646MIPI_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
		OV5646MIPI_write_cmos_sensor(0x3a0b, 0xf6);//	; B60
		OV5646MIPI_write_cmos_sensor(0x3a0e, 0x03);//	; max 50
		OV5646MIPI_write_cmos_sensor(0x3a0d, 0x04);//	; max 60
		OV5646MIPI_write_cmos_sensor(0x3a14, 0x03);//	; max exp 50 = 740
		OV5646MIPI_write_cmos_sensor(0x3a15, 0xd8);//	; max exp 50
		OV5646MIPI_write_cmos_sensor(0x3c07, 0x07);//	; 50/60 auto detect
		OV5646MIPI_write_cmos_sensor(0x3c08, 0x01);//	; 50/60 auto detect
		OV5646MIPI_write_cmos_sensor(0x3c09, 0xc2);//	; 50/60 auto detect
		OV5646MIPI_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
		OV5646MIPI_write_cmos_sensor(0x4005, 0x18);//	; BLC triggered by gain change
		OV5646MIPI_write_cmos_sensor(0x4837, 0x11); // MIPI global timing 16           
		OV5646MIPI_write_cmos_sensor(0x503d, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x5000, 0xa7);//
		OV5646MIPI_write_cmos_sensor(0x5001, 0xa3);//
		OV5646MIPI_write_cmos_sensor(0x5002, 0x80);//
		OV5646MIPI_write_cmos_sensor(0x5003, 0x08);//
		OV5646MIPI_write_cmos_sensor(0x3032, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x4000, 0x89);//
		OV5646MIPI_write_cmos_sensor(0x350c, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x350d, 0x00);//
		OV5646MIPISENSORDB("[OV5646MIPI]OV5646MIPIYUVSetVideoMode exit u2FrameRate == 30 setting  :\n ");
	}
	else if (u2FrameRate == 15)   
	{
		//;OV5646MIPI 1280x960,15fps
		//28Mhz, 112Mbps/Lane, 2Lane.
		OV5646MIPISENSORDB("[OV5646MIPI]OV5646MIPIYUVSetVideoMode enter u2FrameRate == 15 setting  :\n ");
		OV5646MIPI_write_cmos_sensor(0x300e, 0x45);//	; MIPI 2 lane
		OV5646MIPI_write_cmos_sensor(0x3034, 0x18); // PLL, MIPI 8-bit mode
		OV5646MIPI_write_cmos_sensor(0x3035, 0x21); // PLL
		OV5646MIPI_write_cmos_sensor(0x3036, 0x38); // PLL
		OV5646MIPI_write_cmos_sensor(0x3037, 0x13); // PLL
		OV5646MIPI_write_cmos_sensor(0x3108, 0x01); // PLL
		OV5646MIPI_write_cmos_sensor(0x3824, 0x01); // PLL
		OV5646MIPI_write_cmos_sensor(0x460c, 0x20); // PLL
		OV5646MIPI_write_cmos_sensor(0x3618, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x3600, 0x09);//
		OV5646MIPI_write_cmos_sensor(0x3601, 0x43);//
		OV5646MIPI_write_cmos_sensor(0x3708, 0x66);//
		OV5646MIPI_write_cmos_sensor(0x3709, 0x12);//
		OV5646MIPI_write_cmos_sensor(0x370c, 0xc3);//
		OV5646MIPI_write_cmos_sensor(0x3800, 0x00); // HS = 0
		OV5646MIPI_write_cmos_sensor(0x3801, 0x00); // HS
		OV5646MIPI_write_cmos_sensor(0x3802, 0x00); // VS = 250
		OV5646MIPI_write_cmos_sensor(0x3803, 0x06); // VS
		OV5646MIPI_write_cmos_sensor(0x3804, 0x0a); // HW = 2623
		OV5646MIPI_write_cmos_sensor(0x3805, 0x3f);//	; HW
		OV5646MIPI_write_cmos_sensor(0x3806, 0x07);//	; VH = 
		OV5646MIPI_write_cmos_sensor(0x3807, 0x9d);//	; VH
		OV5646MIPI_write_cmos_sensor(0x3808, 0x05);//	; DVPHO = 1280
		OV5646MIPI_write_cmos_sensor(0x3809, 0x00);//	; DVPHO
		OV5646MIPI_write_cmos_sensor(0x380a, 0x03);//	; DVPVO = 960
		OV5646MIPI_write_cmos_sensor(0x380b, 0xc0);//	; DVPVO
		OV5646MIPI_write_cmos_sensor(0x380c, 0x07);//	; HTS = 2160
		OV5646MIPI_write_cmos_sensor(0x380d, 0x68);//	; HTS
		OV5646MIPI_write_cmos_sensor(0x380e, 0x03);//	; VTS = 740
		OV5646MIPI_write_cmos_sensor(0x380f, 0xd8);//	; VTS
		OV5646MIPI_write_cmos_sensor(0x3810, 0x00); // H OFF = 16
		OV5646MIPI_write_cmos_sensor(0x3811, 0x10); // H OFF
		OV5646MIPI_write_cmos_sensor(0x3812, 0x00); // V OFF = 4
		OV5646MIPI_write_cmos_sensor(0x3813, 0x06);//	; V OFF
		OV5646MIPI_write_cmos_sensor(0x3814, 0x31);//	; X INC
		OV5646MIPI_write_cmos_sensor(0x3815, 0x31);//	; Y INC
		OV5646MIPI_write_cmos_sensor(0x3820, 0x47);//	; flip off, V bin on
		OV5646MIPI_write_cmos_sensor(0x3821, 0x01);//	; mirror on, H bin on
		OV5646MIPI_write_cmos_sensor(0x4514, 0x00);
		OV5646MIPI_write_cmos_sensor(0x3a00, 0x38);//	; ae mode	
		OV5646MIPI_write_cmos_sensor(0x3a02, 0x03);//	; max exp 60 = 740
		OV5646MIPI_write_cmos_sensor(0x3a03, 0xd8);//	; max exp 60
		OV5646MIPI_write_cmos_sensor(0x3a08, 0x00);//	; B50 = 222
		OV5646MIPI_write_cmos_sensor(0x3a09, 0x94);//	; B50
		OV5646MIPI_write_cmos_sensor(0x3a0a, 0x00);//	; B60 = 185
		OV5646MIPI_write_cmos_sensor(0x3a0b, 0x7b);//	; B60
		OV5646MIPI_write_cmos_sensor(0x3a0e, 0x06);//	; max 50
		OV5646MIPI_write_cmos_sensor(0x3a0d, 0x07);//	; max 60
		OV5646MIPI_write_cmos_sensor(0x3a14, 0x03);//	; max exp 50 = 740
		OV5646MIPI_write_cmos_sensor(0x3a15, 0xd8);//	; max exp 50
		OV5646MIPI_write_cmos_sensor(0x3c07, 0x08);//	; 50/60 auto detect
		OV5646MIPI_write_cmos_sensor(0x3c08, 0x00);//	; 50/60 auto detect
		OV5646MIPI_write_cmos_sensor(0x3c09, 0x1c);//	; 50/60 auto detect
		OV5646MIPI_write_cmos_sensor(0x4004, 0x02);//	; BLC line number
		OV5646MIPI_write_cmos_sensor(0x4005, 0x18);//	; BLC triggered by gain change
		OV5646MIPI_write_cmos_sensor(0x4837, 0x11); // MIPI global timing 16           
		OV5646MIPI_write_cmos_sensor(0x503d, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x5000, 0xa7);//
		OV5646MIPI_write_cmos_sensor(0x5001, 0xa3);//
		OV5646MIPI_write_cmos_sensor(0x5002, 0x80);//
		OV5646MIPI_write_cmos_sensor(0x5003, 0x08);//
		OV5646MIPI_write_cmos_sensor(0x3032, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x4000, 0x89);//
		OV5646MIPI_write_cmos_sensor(0x350c, 0x00);//
		OV5646MIPI_write_cmos_sensor(0x350d, 0x00);//
		OV5646MIPISENSORDB("[OV5646MIPI]OV5646MIPIYUVSetVideoMode exit u2FrameRate == 15 setting  :\n ");
	}   
	else 
	{
		printk("Wrong frame rate setting \n");
	} 
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIYUVSetVideoMode function:\n ");
	return TRUE; 
}

/**************************/
static void OV5646MIPIGetEvAwbRef(UINT32 pSensorAEAWBRefStruct)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIGetEvAwbRef function:\n ");
	PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
	Ref->SensorAERef.AeRefLV05Shutter=0x170c;
	Ref->SensorAERef.AeRefLV05Gain=0x30;
	Ref->SensorAERef.AeRefLV13Shutter=0x24e;
	Ref->SensorAERef.AeRefLV13Gain=0x10;
	Ref->SensorAwbGainRef.AwbRefD65Rgain=0x610;
	Ref->SensorAwbGainRef.AwbRefD65Bgain=0x448;
	Ref->SensorAwbGainRef.AwbRefCWFRgain=0x4e0;
	Ref->SensorAwbGainRef.AwbRefCWFBgain=0x5a0;
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIGetEvAwbRef function:\n ");
}

static void OV5646MIPIGetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIGetCurAeAwbInfo function:\n ");
	PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
	Info->SensorAECur.AeCurShutter=OV5646MIPIReadShutter();
	Info->SensorAECur.AeCurGain=OV5646MIPIReadSensorGain() ;
	Info->SensorAwbGainCur.AwbCurRgain=((OV5646MIPIYUV_read_cmos_sensor(0x3401)&&0xff)+((OV5646MIPIYUV_read_cmos_sensor(0x3400)&&0xff)*256));
	Info->SensorAwbGainCur.AwbCurBgain=((OV5646MIPIYUV_read_cmos_sensor(0x3405)&&0xff)+((OV5646MIPIYUV_read_cmos_sensor(0x3404)&&0xff)*256));
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIGetCurAeAwbInfo function:\n ");
}
UINT32 OV5646MIPIMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) 
{
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
	OV5646MIPISENSORDB("OV5646MIPIMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIMaxFramerateByScenario function:\n ");
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = 56000000;
			lineLength = OV5646MIPI_IMAGE_SENSOR_SVGA_WIDTH;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV5646MIPI_IMAGE_SENSOR_SVGA_HEIGHT;
			if(dummyLine<0)
				dummyLine = 0;
			spin_lock(&ov5646mipi_drv_lock);
			OV5646MIPISensor.SensorMode= SENSOR_MODE_PREVIEW;
			OV5646MIPISensor.PreviewDummyLines = dummyLine;
			spin_unlock(&ov5646mipi_drv_lock);
			//OV5646MIPISetDummy(OV5646MIPISensor.PreviewDummyPixels, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = 56000000;
			lineLength = OV5646MIPI_IMAGE_SENSOR_VIDEO_WITDH;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV5646MIPI_IMAGE_SENSOR_VIDEO_HEIGHT;
			if(dummyLine<0)
				dummyLine = 0;
			//spin_lock(&ov5646mipi_drv_lock);
			//ov8825.sensorMode = SENSOR_MODE_VIDEO;
			//spin_unlock(&ov5646mipi_drv_lock);
			//OV5646MIPISetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = 90000000;
			lineLength = OV5646MIPI_IMAGE_SENSOR_QSXGA_WITDH;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - OV5646MIPI_IMAGE_SENSOR_QSXGA_HEIGHT;
			if(dummyLine<0)
				dummyLine = 0;
			spin_lock(&ov5646mipi_drv_lock);
			OV5646MIPISensor.CaptureDummyLines = dummyLine;
			OV5646MIPISensor.SensorMode= SENSOR_MODE_CAPTURE;
			spin_unlock(&ov5646mipi_drv_lock);
			//OV5646MIPISetDummy(OV5646MIPISensor.CaptureDummyPixels, dummyLine);			
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
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIMaxFramerateByScenario function:\n ");
	return ERROR_NONE;
}


UINT32 OV5646MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPIGetDefaultFramerateByScenario function:\n ");
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 150;
			 break;		
		case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
		case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
		case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			 *pframeRate = 300;
			 break;		
		default:
			 break;
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIGetDefaultFramerateByScenario function:\n ");
	return ERROR_NONE;
}
void OV5646MIPI_get_AEAWB_lock(UINT32 *pAElockRet32, UINT32 *pAWBlockRet32)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_get_AEAWB_lock function:\n ");
	*pAElockRet32 =1;
	*pAWBlockRet32=1;
	OV5646MIPISENSORDB("[OV5646MIPI]OV5646MIPI_get_AEAWB_lock,AE=%d,AWB=%d\n",*pAElockRet32,*pAWBlockRet32);
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_get_AEAWB_lock function:\n ");
}
void OV5646MIPI_GetDelayInfo(UINT32 delayAddr)
{
	OV5646MIPISENSORDB("[OV5646MIPI]enter OV5646MIPI_GetDelayInfo function:\n ");
	SENSOR_DELAY_INFO_STRUCT *pDelayInfo=(SENSOR_DELAY_INFO_STRUCT*)delayAddr;
	pDelayInfo->InitDelay=0;
	pDelayInfo->EffectDelay=0;
	pDelayInfo->AwbDelay=0;
	pDelayInfo->AFSwitchDelayFrame=50;
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPI_GetDelayInfo function:\n ");
}
void OV5646MIPI_3ACtrl(ACDK_SENSOR_3A_LOCK_ENUM action)
{
    OV5646MIPISENSORDB("[OV5646MIPI]enter ACDK_SENSOR_3A_LOCK_ENUM function:action=%d\n",action);
    switch (action)
    {
        case SENSOR_3A_AE_LOCK:
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPISensor.userAskAeLock = TRUE;
            spin_unlock(&ov5646mipi_drv_lock);
            OV5646MIPI_set_AE_mode(KAL_FALSE);
            break;
        case SENSOR_3A_AE_UNLOCK:
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPISensor.userAskAeLock = FALSE;
            spin_unlock(&ov5646mipi_drv_lock);
            OV5646MIPI_set_AE_mode(KAL_TRUE);
            break;

        case SENSOR_3A_AWB_LOCK:
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPISensor.userAskAwbLock = TRUE;
            spin_unlock(&ov5646mipi_drv_lock);
            OV5646MIPI_set_AWB_mode(KAL_FALSE);
            break;

        case SENSOR_3A_AWB_UNLOCK:
            spin_lock(&ov5646mipi_drv_lock);
            OV5646MIPISensor.userAskAwbLock = FALSE;
            spin_unlock(&ov5646mipi_drv_lock);
            OV5646MIPI_set_AWB_mode_UNLOCK();
            break;
        default:
            break;
    }
    OV5646MIPISENSORDB("[OV5646MIPI]exit ACDK_SENSOR_3A_LOCK_ENUM function:action=%d\n",action);
    return;
}

#define OV5646MIPI_FLASH_BV_THRESHOLD 0x25
static void OV5646MIPI_FlashTriggerCheck(unsigned int *pFeatureReturnPara32)
{
	unsigned int NormBr;	   
	NormBr = OV5646MIPIYUV_read_cmos_sensor(0x56A1); 
	OV5646MIPISENSORDB("[OV5646MIPI] OV5646MIPI_FlashTriggerCheck: 0x%x\n", NormBr);
	if (NormBr > OV5646MIPI_FLASH_BV_THRESHOLD)
	{
	   *pFeatureReturnPara32 = FALSE;
	   return;
	}
	*pFeatureReturnPara32 = TRUE;
	return;
}

UINT32 OV5646MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	UINT32 Tony_Temp1 = 0;
	UINT32 Tony_Temp2 = 0;
	Tony_Temp1 = pFeaturePara[0];
	Tony_Temp2 = pFeaturePara[1];
	OV5646MIPISENSORDB("[OV5646MIPI]enter[OV5646MIPIFeatureControl]feature id=%d \n",FeatureId);
	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=OV5646MIPI_IMAGE_SENSOR_QSXGA_WITDH;
			*pFeatureReturnPara16=OV5646MIPI_IMAGE_SENSOR_QSXGA_HEIGHT;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_GET_PERIOD:
			switch(OV5646CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara16++=OV5646MIPI_FULL_PERIOD_PIXEL_NUMS + OV5646MIPISensor.CaptureDummyPixels;
					*pFeatureReturnPara16=OV5646MIPI_FULL_PERIOD_LINE_NUMS + OV5646MIPISensor.CaptureDummyLines;
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara16++=OV5646MIPI_PV_PERIOD_PIXEL_NUMS + OV5646MIPISensor.PreviewDummyPixels;
					*pFeatureReturnPara16=OV5646MIPI_PV_PERIOD_LINE_NUMS + OV5646MIPISensor.PreviewDummyLines;
					*pFeatureParaLen=4;
					break;
			}
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			switch(OV5646CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara32 = OV5646MIPISensor.ZsdturePclk * 1000 *100;	 //unit: Hz				
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara32 = OV5646MIPISensor.PreviewPclk * 1000 *100;	 //unit: Hz
					*pFeatureParaLen=4;
					break;
			}
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			break;
		/**********************Strobe Ctrl Start *******************************/
		case SENSOR_FEATURE_SET_ESHUTTER:
			OV5646MIPISENSORDB("[OV5646MIPI] F_SET_ESHUTTER: %d\n", *pFeatureData16);
			OV5646MIPI_SetShutter(*pFeatureData16);
			break;
		case SENSOR_FEATURE_SET_GAIN:
			OV5646MIPISENSORDB("[OV5646MIPI] F_SET_GAIN: %d\n", *pFeatureData16);
			OV5646MIPI_SetGain(*pFeatureData16);
			break;
		case SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO:
			OV5646MIPISENSORDB("[OV5646MIPI] F_GET_AE_FLASHLIGHT_INFO: %d\n", *pFeatureData32);
			OV5646MIPI_GetAEFlashlightInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
			OV5646MIPI_FlashTriggerCheck(pFeatureData32);
			OV5646MIPISENSORDB("[OV5646MIPI] F_GET_TRIGGER_FLASHLIGHT_INFO: %d\n", pFeatureData32);
			break;		
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			OV5646MIPISENSORDB("OV5646MIPI SENSOR_FEATURE_SET_FLASHLIGHT\n");
			break;
		/**********************Strobe Ctrl End *******************************/
		
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			OV5646MIPISENSORDB("[OV5646MIPI] F_SET_ISP_MASTER_CLOCK_FREQ\n");
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			OV5646MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = OV5646MIPIYUV_read_cmos_sensor(pSensorRegData->RegAddr);
			break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &OV5646MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_SET_YUV_CMD:
			OV5646MIPIYUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
			break;	
		case SENSOR_FEATURE_SET_YUV_3A_CMD:
			OV5646MIPI_3ACtrl((ACDK_SENSOR_3A_LOCK_ENUM)*pFeatureData32);
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_SET_VIDEO_MODE\n");
			OV5646MIPIYUVSetVideoMode(*pFeatureData16);
			break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_CHECK_SENSOR_ID\n");
			OV5646MIPI_GetSensorID(pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_EV_AWB_REF:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_EV_AWB_REF\n");
			OV5646MIPIGetEvAwbRef(*pFeatureData32);
			break;		
		case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN\n");
			OV5646MIPIGetCurAeAwbInfo(*pFeatureData32);			
			break;
		case SENSOR_FEATURE_GET_EXIF_INFO:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_EXIF_INFO\n");
			OV5646MIPIGetExifInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_DELAY_INFO:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_DELAY_INFO\n");
			OV5646MIPI_GetDelayInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_SET_SLAVE_I2C_ID:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_SET_SLAVE_I2C_ID\n");
			OV5646MIPI_sensor_socket = *pFeatureData32;
			break;
		case SENSOR_FEATURE_SET_TEST_PATTERN: 
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_SET_TEST_PATTERN\n");
			OV5646SetTestPatternMode((BOOL)*pFeatureData16);            
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
			OV5646MIPISENSORDB("[OV5646MIPI]OV5646_TEST_PATTERN_CHECKSUM\n");
			*pFeatureReturnPara32=OV5646_TEST_PATTERN_CHECKSUM;
			*pFeatureParaLen=4;
			break;				
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO\n");
			OV5646MIPIMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32,*(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:\
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO\n");
			OV5646MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32,(MUINT32 *)*(pFeatureData32+1));
			break;
	    /**********************below is AF control**********************/	
		case SENSOR_FEATURE_INITIALIZE_AF:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_INITIALIZE_AF\n");
			//OV5646_FOCUS_OVT_AFC_Init();
			break;
		case SENSOR_FEATURE_MOVE_FOCUS_LENS:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_MOVE_FOCUS_LENS\n");
			OV5646_FOCUS_Move_to(*pFeatureData16);
			break;
		case SENSOR_FEATURE_GET_AF_STATUS:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_AF_STATUS\n");
			OV5646_FOCUS_OVT_AFC_Get_AF_Status(pFeatureReturnPara32);            
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_SINGLE_FOCUS_MODE\n");
			OV5646_FOCUS_OVT_AFC_Single_Focus();
			break;
		case SENSOR_FEATURE_CONSTANT_AF:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_CONSTANT_AF\n");
			OV5646_FOCUS_OVT_AFC_Constant_Focus();
			break;
		case SENSOR_FEATURE_CANCEL_AF:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_CANCEL_AF\n");
			OV5646_FOCUS_OVT_AFC_Cancel_Focus();
			break;
		case SENSOR_FEATURE_GET_AF_INF:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_AF_INF\n");
			OV5646_FOCUS_Get_AF_Inf(pFeatureReturnPara32);
			*pFeatureParaLen=4;            
			break;
		case SENSOR_FEATURE_GET_AF_MACRO:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_AF_MACRO\n");
			OV5646_FOCUS_Get_AF_Macro(pFeatureReturnPara32);
			*pFeatureParaLen=4;            
			break;
		case SENSOR_FEATURE_SET_AF_WINDOW: 
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_SET_AF_WINDOW\n");
			OV5646_FOCUS_Set_AF_Window(*pFeatureData32);
			break;       					
		case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS\n");
			OV5646_FOCUS_Get_AF_Max_Num_Focus_Areas(pFeatureReturnPara32);            
			*pFeatureParaLen=4;
			break; 			
		case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
			OV5646MIPISENSORDB("[OV5646MIPI]SENSOR_FEATURE_GET_AF_STATUS\n");
			OV5646MIPI_get_AEAWB_lock(*pFeatureData32, *(pFeatureData32+1));
			break;					                              	               
		case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
			OV5646MIPISENSORDB("[OV5646MIPI]AE zone addr = 0x%x\n",*pFeatureData32);
			OV5646_FOCUS_Get_AE_Max_Num_Metering_Areas(pFeatureReturnPara32);            
			*pFeatureParaLen=4;
			break;        
		case SENSOR_FEATURE_SET_AE_WINDOW:
			OV5646MIPISENSORDB("[OV5646MIPI]AE zone addr = 0x%x\n",*pFeatureData32);			
			OV5646_FOCUS_Set_AE_Window(*pFeatureData32);
			break; 
		default:
			break;			
	}
	OV5646MIPISENSORDB("[OV5646MIPI]exit OV5646MIPIFeatureControl function:\n ");
	return ERROR_NONE;
}	/* OV5646MIPIFeatureControl() */

SENSOR_FUNCTION_STRUCT	SensorFuncOV5646MIPI=
{
	OV5646MIPIOpen,
	OV5646MIPIGetInfo,
	OV5646MIPIGetResolution,
	OV5646MIPIFeatureControl,
	OV5646MIPIControl,
	OV5646MIPIClose
};

UINT32 OV5646_MIPI_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV5646MIPI;
	return ERROR_NONE;
}	/* SensorInit() */



