/*===========================================================================

                        EDIT HISTORY FOR V11

when              comment tag        who                  what, where, why                           
----------    ------------     -----------      --------------------------      

===========================================================================*/
/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
*/

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
	
#include "ov8825mipiraw_Sensor.h"
#include "ov8825mipiraw_Camera_Sensor_para.h"
#include "ov8825mipiraw_CameraCustomized.h"

#undef CDBG
#define CDBG(fmt, args...) printk(KERN_INFO "ov8825_OTP.c: " fmt, ## args)

extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define OV8825_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, OV8825MIPI_WRITE_ID)
int moudle_type = 0;
struct otp_struct {
    uint customer_id;
    uint module_integrator_id;
    uint lens_id;
    uint rg_ratio;
    uint bg_ratio;
    uint user_data[5];
    uint lenc[62];
};

struct otp_struct current_otp2;
uint32_t R_gain1=0x400,B_gain1=0x400,G_gain1=0x400;

extern kal_uint16 OV8825_read_cmos_sensor(kal_uint32 addr);
//extern OV8825_write_cmos_sensor(addr, para);
/*static int32_t OV8825_write_cmos_sensor(unsigned short waddr, uint8_t bdata);
static uint16_t OV8825_read_cmos_sensor(unsigned short raddr);

static int32_t OV8825_write_cmos_sensor(unsigned short waddr, uint8_t bdata)
{
    return OV8825_write_cmos_sensor(0x6C >>2,waddr,bdata);
}

static uint16_t OV8825_read_cmos_sensor(unsigned short raddr)
{
    uint16_t  rdata;
    if(OV8825_read_cmos_sensor(raddr, &rdata,1) < 0)
        return -1;
    return rdata;
}
*/
// index: index of otp group. (0, 1, 2)
// return: 0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int check_otp_awb(uint index)
{
    uint flag, i;
    uint32_t address;
    // select bank 0
    OV8825_write_cmos_sensor(0x3d84, 0x08);
    // load otp to buffer
    OV8825_write_cmos_sensor(0x3d81, 0x01);
    // read flag
    address = 0x3d05 + index*9;
    flag = OV8825_read_cmos_sensor(address);
    // disable otp read
    OV8825_write_cmos_sensor(0x3d81, 0x00);
    // clear otp buffer
    address = 0x3d00;
    for(i=0; i<32;i++) {
        OV8825_write_cmos_sensor(address + i, 0x00);
    }
    if (!flag) {
        return 0;
    }
    else if(!(flag & 0x80) && (flag&0x7f)) {
        return 2;
    }
    else {
        return 1;
    }
}

// index: index of otp group. (0, 1, 2)
// return:0, group index is empty
//      1, group index has invalid data
//      2, group index has valid data
int check_otp_lenc(uint index)
{
    uint flag, i;
    uint32_t address;
    // select bank: index*2 + 1
    OV8825_write_cmos_sensor(0x3d84, 0x09 + index*2);
    // read otp
    OV8825_write_cmos_sensor(0x3d81, 0x01);
    address = 0x3d00;
    flag = OV8825_read_cmos_sensor(address);
    flag = flag & 0xc0;
    // disable otp read
    OV8825_write_cmos_sensor(0x3d81, 0x00);
    // clear otp buffer
    address = 0x3d00;
    for (i=0; i<32;i++) {
    OV8825_write_cmos_sensor(address + i, 0x00);
    }
    if (!flag) {
    return 0;
    }
    else if (flag == 0x40) {
    return 2;
    }
    else {
    return 1;
    }
}

// index: index of otp group. (0, 1, 2)
// return:0,
int read_otp_wb(uint index, struct otp_struct* otp)
{
    uint32_t address, i;
    // select bank 0
    OV8825_write_cmos_sensor(0x3d84, 0x08);
    // read otp
    OV8825_write_cmos_sensor(0x3d81, 0x01);
    address = 0x3d05 + index*9;
    otp->customer_id = (OV8825_read_cmos_sensor(address) & 0x7f);
    otp->module_integrator_id = OV8825_read_cmos_sensor(address);
    otp->lens_id = OV8825_read_cmos_sensor(address + 1);
    otp->rg_ratio = OV8825_read_cmos_sensor(address + 2);
    otp->bg_ratio = OV8825_read_cmos_sensor(address + 3);
    otp->user_data[0] = OV8825_read_cmos_sensor(address + 4);
    otp->user_data[1] = OV8825_read_cmos_sensor(address + 5);
    otp->user_data[2] = OV8825_read_cmos_sensor(address + 6);
    otp->user_data[3] = OV8825_read_cmos_sensor(address + 7);
    otp->user_data[4] = OV8825_read_cmos_sensor(address + 8);
    // disable otp read
    OV8825_write_cmos_sensor(0x3d81, 0x00);
    // clear otp buffer
    address = 0x3d00;
    for (i=0; i<32;i++) {
    OV8825_write_cmos_sensor(address + i, 0x00);
    }
    return 0;
}

// index: index of otp group. (0, 1, 2)
// return:0
int read_otp_lenc(uint index, struct otp_struct* otp)
{
    uint bank, temp, i;
    uint32_t address;
    // select bank: index*2 + 1
    bank = index*2 + 1;
    temp = 0x08 | bank;
    OV8825_write_cmos_sensor(0x3d84, temp);
    // read otp
    OV8825_write_cmos_sensor(0x3d81, 0x01);
    address = 0x3d01;
    for (i=0; i<31; i++) {
    otp->lenc[i] = OV8825_read_cmos_sensor(address);
    address++;
    }
    // disable otp read
    OV8825_write_cmos_sensor(0x3d81, 0x00);
    // clear otp buffer
    address = 0x3d00;
    for (i=0; i<32;i++) {
    OV8825_write_cmos_sensor(address + i, 0x00);
    }
    // select next bank
    bank++;
    temp = 0x08 | bank;
    OV8825_write_cmos_sensor(0x3d84, temp);
    // read otp
    OV8825_write_cmos_sensor(0x3d81, 0x01);
    address = 0x3d00;
    for (i=31; i<62; i++) {
    otp->lenc[i] = OV8825_read_cmos_sensor(address);
    address++;
    }
    // disable otp read
    OV8825_write_cmos_sensor(0x3d81, 0x00);
    // clear otp buffer
    address = 0x3d00;
    for (i=0; i<32;i++) {
    OV8825_write_cmos_sensor(address + i, 0x00);
    }
    return 0;
}

// R_gain: red gain of sensor AWB, 0x400 = 1
// G_gain: green gain of sensor AWB, 0x400 = 1
// B_gain: blue gain of sensor AWB, 0x400 = 1
// return 0
/*
int update_awb_gain(uint32_t R_gain, uint32_t G_gain, uint32_t B_gain)
{
    if (R_gain>0x400) {
	if (moudle_type == 1)//xuyongfu@20120524@for reduce truly red percent
	{
		R_gain = R_gain - 50;	
	}
        OV8825_write_cmos_sensor(0x3400, R_gain>>8);
        OV8825_write_cmos_sensor(0x3401, R_gain & 0x00ff);
    }
    if (G_gain>0x400) {
        OV8825_write_cmos_sensor(0x3402, G_gain>>8);
        OV8825_write_cmos_sensor(0x3403, G_gain & 0x00ff);
    }
    if (B_gain>0x400) {
        OV8825_write_cmos_sensor(0x3404, B_gain>>8);
        OV8825_write_cmos_sensor(0x3405, B_gain & 0x00ff);
    }
    return 0;
}
*/

int update_awb_gain()//uint32_t R_gain, uint32_t G_gain, uint32_t B_gain)
{
    if (R_gain1>0x400) {
	if (moudle_type == 1)//xuyongfu@20120524@for reduce truly red percent
	{
		R_gain1 = R_gain1 - 50;	
	}
        OV8825_write_cmos_sensor(0x3400, R_gain1>>8);
        OV8825_write_cmos_sensor(0x3401, R_gain1 & 0x00ff);
    }
    if (G_gain1>0x400) {
        OV8825_write_cmos_sensor(0x3402, G_gain1>>8);
        OV8825_write_cmos_sensor(0x3403, G_gain1& 0x00ff);
    }
    if (B_gain1>0x400) {
        OV8825_write_cmos_sensor(0x3404, B_gain1>>8);
        OV8825_write_cmos_sensor(0x3405, B_gain1 & 0x00ff);
    }
    return 0;
}


// return 0
/*
int update_lens(struct otp_struct otp)
{
    uint i, temp;
    temp = 0x80 | otp.lenc[0];
    OV8825_write_cmos_sensor(0x5800, temp);
    for(i=1;i<62;i++) {
        OV8825_write_cmos_sensor(0x5800+i, otp.lenc[i]);
    }
    return 0;   
}
*/
int update_lens()//(struct otp_struct otp)
{
    uint i, temp;
    temp = 0x80 | current_otp2.lenc[0];
    OV8825_write_cmos_sensor(0x5800, temp);
    for(i=1;i<62;i++) {
        OV8825_write_cmos_sensor(0x5800+i, current_otp2.lenc[i]);
    }
    OV8825_write_cmos_sensor(0x5000, 0x86); //LENC on
    return 0;
}


// R/G and B/G of typical camera module is defined here
#if 0 //hucz must change
RG_Ratio_typical = RG_TYPICAL;
BG_Ratio_typical = BG_TYPICAL;
#else
//add by buxiangyu for distinguish diffrent moudle Truly or Liteon
uint32_t RG_Ratio_typical=0x00;//xuyongfu@modify
uint32_t BG_Ratio_typical=0x00;//xuyongfu@modify
uint32_t RG_Ratio_typical_Liteon=0x47;
uint32_t BG_Ratio_typical_Liteon=0x44;

uint32_t RG_Ratio_typical_Truly=0x4b;
uint32_t BG_Ratio_typical_Truly=0x4e;

uint32_t RG_Ratio_typical_Sunny=0x56;//0x57
uint32_t BG_Ratio_typical_Sunny=0x58;//0x56

static uint32_t RG_Ratio_typical_Sunny_MTM=0x56;
static uint32_t BG_Ratio_typical_Sunny_MTM=0x59;
#endif
// call this function after ov8825 initialization
// return value: 0 update success
//              1, no OTP
int update_otp_wb(void)
{
    uint i, temp, otp_index;
    struct otp_struct current_otp;
    uint32_t R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
	
    // R/G and B/G of current camera module is read out from sensor OTP
    // check first wb OTP with valid data
    printk("[Sunny] update_otp_wb++\n");
    for(i=0;i<3;i++) {
        temp = check_otp_awb(i);
        if (temp == 2) {
        otp_index = i;
        break;
        }
    }
    if(i==3) {
        // no valid wb OTP data
        printk("[Sunny] no valid wb OTP data\n");
        return 1;
    }

    memset(&current_otp, 0, sizeof(current_otp));
    read_otp_wb(otp_index, &current_otp);
	//add by buxiangyu for distinguish diffrent moudle Truly or Liteon
	printk("[Sunny] #########current_otp->customer_id = 0x%x\n",current_otp.customer_id);
	if(current_otp.customer_id == 0x02)
		{
			moudle_type = 1;  // 1  mean Truly 
			RG_Ratio_typical = RG_Ratio_typical_Truly;
			BG_Ratio_typical = BG_Ratio_typical_Truly;
		}
	else if(current_otp.customer_id == 0x15)
		{
			RG_Ratio_typical = RG_Ratio_typical_Liteon;
			BG_Ratio_typical = BG_Ratio_typical_Liteon;
		}
	else if(current_otp.customer_id == 0x01)
		{
            if(current_otp.lens_id== 0x03)
            {
			    RG_Ratio_typical = RG_Ratio_typical_Sunny_MTM;
			    BG_Ratio_typical = BG_Ratio_typical_Sunny_MTM;
            }
            else
            {
			RG_Ratio_typical = RG_Ratio_typical_Sunny;
			BG_Ratio_typical = BG_Ratio_typical_Sunny;
            }
		}
    //calculate gain
    //0x400 = 1x gain
    printk("[Sunny] #########current_otp.bg_ratio = 0x%x\n",current_otp.bg_ratio);
    printk("[Sunny] #########current_otp.rg_ratio = 0x%x\n",current_otp.rg_ratio);
    printk("[Sunny] #########RG_Ratio_typical = 0x%x\n",RG_Ratio_typical);
    printk("[Sunny] #########BG_Ratio_typical = 0x%x\n",BG_Ratio_typical);
    if(current_otp.bg_ratio < BG_Ratio_typical)
    {
        if (current_otp.rg_ratio < RG_Ratio_typical)
        {
            // current_otp.bg_ratio < BG_Ratio_typical &&
            // current_otp.rg_ratio < RG_Ratio_typical
            G_gain = 0x400;
            B_gain = 0x400 * BG_Ratio_typical / current_otp.bg_ratio;
            R_gain = 0x400 * RG_Ratio_typical / current_otp.rg_ratio;
        }
        else
        {
            // current_otp.bg_ratio < BG_Ratio_typical &&
            // current_otp.rg_ratio >= RG_Ratio_typical
            R_gain = 0x400;
            G_gain = 0x400 * current_otp.rg_ratio / RG_Ratio_typical;
            B_gain = G_gain * BG_Ratio_typical / current_otp.bg_ratio;
        }
    }
    else
    {
        if (current_otp.rg_ratio < RG_Ratio_typical)
        {
            // current_otp.bg_ratio >= BG_Ratio_typical &&
            // current_otp.rg_ratio < RG_Ratio_typical
            B_gain = 0x400;
            G_gain = 0x400 * current_otp.bg_ratio / BG_Ratio_typical;
            R_gain = G_gain * RG_Ratio_typical / current_otp.rg_ratio;
        }
        else
        {
            // current_otp.bg_ratio >= BG_Ratio_typical &&
            // current_otp.rg_ratio >= RG_Ratio_typical
            G_gain_B = 0x400 * current_otp.bg_ratio / BG_Ratio_typical;
            G_gain_R = 0x400 * current_otp.rg_ratio / RG_Ratio_typical;
            if(G_gain_B > G_gain_R )
            {
                B_gain = 0x400;
                G_gain = G_gain_B;
                R_gain = G_gain * RG_Ratio_typical / current_otp.rg_ratio;
            }
            else
            {
                R_gain = 0x400;
                G_gain = G_gain_R;
                B_gain = G_gain * BG_Ratio_typical / current_otp.bg_ratio;
            }
        }
    }
    // write sensor wb gain to registers
    R_gain1=R_gain;
    G_gain1=G_gain;
    B_gain1=B_gain;
//    update_awb_gain(R_gain, G_gain, B_gain);
    printk("[Sunny] update_otp_wb--\n");
    return 0;
}

// call this function after ov8825 initialization
// return value: 0 update success
//              1, no OTP
int update_otp_lenc(void)
{
    uint i, temp, otp_index;
    struct otp_struct current_otp;
    printk("[Sunny] update_otp_lenc++\n");
    // check first lens correction OTP with valid data
    for(i=0;i<3;i++) {
        temp = check_otp_lenc(i);
        if (temp == 2) {
            otp_index = i;
            break;
        }
    }
    if (i==3) {
        // no lens correction data
        printk("[Sunny] no lens correction data\n");
        return 1;
    }
    read_otp_lenc(otp_index, &current_otp);
	current_otp2=current_otp;
	//update_lens(current_otp);
    //success
    printk("[Sunny] update_otp_lenc--\n");
    return 0;
}

