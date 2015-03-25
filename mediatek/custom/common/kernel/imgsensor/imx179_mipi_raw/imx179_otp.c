/*****************************************************************************
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
 *
*****************************************************************************/

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

#include "imx179mipiraw_Sensor.h"
#include "imx179mipiraw_Camera_Sensor_para.h"
#include "imx179mipiraw_CameraCustomized.h"

#define IMX179_OTP_DEBUG

#define IMX179_SUNNY_MID 0x01
#define IMX179_TRULY_MID 0x02

extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
#define IMX179MIPI_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, IMX179MIPI_WRITE_ID)
extern kal_uint16 IMX179MIPI_read_cmos_sensor(kal_uint32 addr);

struct imx179_otp_struct {
    uint year;
    uint month;
    uint day;
    uint vendor_id;
    uint lens_id;
    uint vcm_id;
    uint32_t rg_ratio;
    uint32_t bg_ratio;
    uint32_t gg_ratio;
};

uint32_t imx179_R_gain1=0x100,imx179_B_gain1=0x100,imx179_G_gain1=0x100;

// R/G and B/G of typical camera module is defined here
//add for distinguish diffrent moudle Truly or Liteon
uint32_t IMX179_RG_Ratio_typical=0x00;
uint32_t IMX179_BG_Ratio_typical=0x00;
uint32_t IMX179_GG_Ratio_typical=0x00;

uint32_t IMX179_RG_Ratio_typical_Truly=0x00;
uint32_t IMX179_BG_Ratio_typical_Truly=0x00;
uint32_t IMX179_GG_Ratio_typical_Truly=0x00;

uint32_t IMX179_RG_Ratio_typical_Sunny=0x025F;//607
uint32_t IMX179_BG_Ratio_typical_Sunny=0x0230;//560
uint32_t IMX179_GG_Ratio_typical_Sunny=0x03FF;//1023


// index: index of otp group. (0, 1)
// return: 0, group index is empty
// return: 1, group index has valid data
int imx179_check_otp_awb(uint index)
{
    printk("[Sunny] imx179_check_otp_awb!\n");
    uint flag;
    uint page = 0x00 + index;//page 3/2/1

    //1.Read out ECC ON, set 0x00 on,set 0x08 off.
    IMX179MIPI_write_cmos_sensor(0x3380, 0x00);

    //2.Status check : 0x3400 = 0x01 (bit0 1:read ready).
    IMX179MIPI_write_cmos_sensor(0x3400, 0x01);

    //3.Select page.
    IMX179MIPI_write_cmos_sensor(0x3402, page);

    //4.Read flag.
    flag = IMX179MIPI_read_cmos_sensor(0x3404);
    if (0x01 == flag) {
        printk("[Sunny] imx179_check_otp_awb success,page = %d have valid data!\n",page);
        return 1;
    }
    else {
        printk("[Sunny] imx179_check_otp_awb error,no page have valid data!\n");
        return 0;
    }
}

int imx179_read_otp_info(uint page, uint32_t address, uint* array, uint32_t size)
{
    uint32_t i;

    //1.Read out ECC ON, set 0x00 on,set 0x08 off.
    IMX179MIPI_write_cmos_sensor(0x3380, 0x00);
    //2.Status check : 0x3400 = 0x01 (bit0 1:read ready).
    IMX179MIPI_write_cmos_sensor(0x3400, 0x01);
    //3.Select page.
    IMX179MIPI_write_cmos_sensor(0x3402, page);
    for (i=0; i<size; i++) {
       *(array+i) = IMX179MIPI_read_cmos_sensor(address+i);
        #ifdef IMX179_OTP_DEBUG
        printk("[Sunny] read_otp [%d] == 0x%x\n",i,array[i]);
        #endif
    }
    return 0;
}

// index: index of otp group. (0, 1, 2)
// return:0,
int imx179_read_otp_wb(uint index, struct imx179_otp_struct* otp)
{
    printk("[Sunny] imx179_read_otp_wb++!\n");
    uint32_t arr[42]={0};
    uint32_t address = 0x3404;
    uint page = 0x00 + index;//page 3/2/1
    uint sum=0, i;

    //1.read out otp info
    imx179_read_otp_info(page, address, arr, 42);
    //2.check sum
    for(i=2;i<42;i++)
    {
        sum = sum + arr[i];
    }
    #ifdef IMX179_OTP_DEBUG
    printk("[Sunny] imx179_read_otp_wb:arr[1] == 0x%x\n",arr[1]);
    printk("[Sunny] imx179_read_otp_wb:sum == 0x%x\n",sum%256);
    #endif
    if(arr[1] != (sum%256))
    {
	printk("[Sunny] imx179_read_otp_wb:read otp data error!!!\n");
	return 1;
    }

    #ifdef IMX179_OTP_DEBUG
    printk("[Sunny] imx179_read_otp_wb:arr[3] == %d\n",arr[3]);
    printk("[Sunny] imx179_read_otp_wb:arr[4] == %d\n",arr[4]);
    printk("[Sunny] imx179_read_otp_wb:arr[5] == %d\n",arr[5]);
    printk("[Sunny] imx179_read_otp_wb:arr[6] == %d\n",arr[6]);
    printk("[Sunny] imx179_read_otp_wb:arr[7] == %d\n",arr[7]);
    printk("[Sunny] imx179_read_otp_wb:arr[8] == %d\n",arr[8]);
    printk("[Sunny] imx179_read_otp_wb:arr[15] == %d\n",arr[15]);
    printk("[Sunny] imx179_read_otp_wb:arr[16] == %d\n",arr[16]);
    printk("[Sunny] imx179_read_otp_wb:arr[17] == %d\n",arr[17]);
    printk("[Sunny] imx179_read_otp_wb:arr[18] == %d\n",arr[18]);
    printk("[Sunny] imx179_read_otp_wb:arr[19] == %d\n",arr[19]);
    printk("[Sunny] imx179_read_otp_wb:arr[20] == %d\n",arr[20]);
    #endif

    otp->year = arr[3];
    otp->month = arr[4];
    otp->day = arr[5];
    otp->vendor_id = arr[6];
    otp->lens_id = arr[7];
    otp->vcm_id =  arr[8];
    otp->rg_ratio = (arr[15]<<8)|arr[16];
    otp->bg_ratio = (arr[17]<<8)|arr[18];
    otp->gg_ratio = (arr[19]<<8)|arr[20];

    printk("[Sunny] imx179_read_otp_wb--!\n");
    return 0;
}


// call this function after imx179 initialization
// return value: 0, update success
//               1, no OTP
int imx179_update_otp_wb(void)
{
    printk("[Sunny] imx179_update_otp_wb++\n");
    uint i, temp, otp_index;
    struct imx179_otp_struct current_otp;
    uint32_t R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
	
    // R/G and B/G of current camera module is read out from sensor OTP
    // check first wb OTP with valid data
    for(i=3;i>0;i--) {
        temp = imx179_check_otp_awb(i);
        if (temp == 1) {
            otp_index = i;
            break;
        }
    }
    if(i<0) {
        // no valid wb OTP data
        printk("[Sunny] imx179_update_otp_wb! no valid wb OTP data\n");
        return 1;
    }

    memset(&current_otp, 0, sizeof(current_otp));
    if(imx179_read_otp_wb(otp_index, &current_otp)) return 1;

    //add for distinguish diffrent moudle Truly or Sunny
    printk("[Sunny] #########current_otp->vendor_id = 0x%x\n",current_otp.vendor_id);

    if(current_otp.vendor_id == IMX179_SUNNY_MID)
    {
        IMX179_RG_Ratio_typical = IMX179_RG_Ratio_typical_Sunny;
        IMX179_BG_Ratio_typical = IMX179_BG_Ratio_typical_Sunny;
        IMX179_GG_Ratio_typical = IMX179_GG_Ratio_typical_Sunny;
    }
    else if(current_otp.vendor_id == IMX179_TRULY_MID)
    {
        IMX179_RG_Ratio_typical = IMX179_RG_Ratio_typical_Truly;
        IMX179_BG_Ratio_typical = IMX179_BG_Ratio_typical_Truly;
        IMX179_GG_Ratio_typical = IMX179_GG_Ratio_typical_Truly;
    }
    else
    {
        printk("[Sunny] imx179_update_otp_wb,read module_id err.\n");
        return 1;
    }

    //calculate gain
    //0x400 = 1x gain
    printk("[Sunny] #########current_otp.rg_ratio = 0x%x\n",current_otp.rg_ratio);
    printk("[Sunny] #########current_otp.bg_ratio = 0x%x\n",current_otp.bg_ratio);
    printk("[Sunny] #########current_otp.gg_ratio = 0x%x\n",current_otp.gg_ratio);
    printk("[Sunny] #########IMX179_RG_Ratio_typical = 0x%x\n",IMX179_RG_Ratio_typical);
    printk("[Sunny] #########IMX179_BG_Ratio_typical = 0x%x\n",IMX179_BG_Ratio_typical);
    printk("[Sunny] #########IMX179_GG_Ratio_typical = 0x%x\n",IMX179_GG_Ratio_typical);

    if(current_otp.bg_ratio < IMX179_BG_Ratio_typical)
    {
        if (current_otp.rg_ratio < IMX179_RG_Ratio_typical)
        {
            G_gain = 0x100;
            B_gain = 0x100 * IMX179_BG_Ratio_typical / current_otp.bg_ratio;
            R_gain = 0x100 * IMX179_RG_Ratio_typical / current_otp.rg_ratio;
        }
        else
        {
            R_gain = 0x100;
            G_gain = 0x100 * current_otp.rg_ratio / IMX179_RG_Ratio_typical;
            B_gain = G_gain * IMX179_BG_Ratio_typical / current_otp.bg_ratio;
        }
    }
    else
    {
        if (current_otp.rg_ratio < IMX179_RG_Ratio_typical)
        {
            B_gain = 0x100;
            G_gain = 0x100 * current_otp.bg_ratio / IMX179_BG_Ratio_typical;
            R_gain = G_gain * IMX179_RG_Ratio_typical / current_otp.rg_ratio;
        }
        else
        {
            G_gain_B = 0x100 * current_otp.bg_ratio / IMX179_BG_Ratio_typical;
            G_gain_R = 0x100 * current_otp.rg_ratio / IMX179_RG_Ratio_typical;
            if(G_gain_B > G_gain_R )
            {
                B_gain = 0x100;
                G_gain = G_gain_B;
                R_gain = G_gain * IMX179_RG_Ratio_typical / current_otp.rg_ratio;
            }
            else
            {
                R_gain = 0x100;
                G_gain = G_gain_R;
                B_gain = G_gain * IMX179_BG_Ratio_typical / current_otp.bg_ratio;
            }
        }
    }

    if (R_gain < 0x100)
    {
        R_gain = 0x100;
    }
    if (G_gain < 0x100)
    {
        G_gain = 0x100;
    }
    if (B_gain < 0x100)
    {
        B_gain = 0x100;
    }

    // write sensor wb gain to registers
    imx179_R_gain1=R_gain&0xFF;
    imx179_G_gain1=G_gain&0xFF;
    imx179_B_gain1=B_gain&0xFF;

    #ifdef IMX179_OTP_DEBUG
    printk("[Sunny] R_gain == 0x%x\n",R_gain);
    printk("[Sunny] G_gain == 0x%x\n",G_gain);
    printk("[Sunny] B_gain == 0x%x\n",B_gain);

    printk("[Sunny] imx179_R_gain1 == 0x%x\n",imx179_R_gain1);
    printk("[Sunny] imx179_G_gain1 == 0x%x\n",imx179_G_gain1);
    printk("[Sunny] imx179_B_gain1 == 0x%x\n",imx179_B_gain1);
    #endif

    printk("[Sunny] imx179_update_otp_wb--\n");
    return 0;
}


// R_gain: red gain of sensor AWB, 0x400 = 1
// G_gain: green gain of sensor AWB, 0x400 = 1
// B_gain: blue gain of sensor AWB, 0x400 = 1
// return 0
int imx179_update_awb_gain()//uint32_t R_gain, uint32_t G_gain, uint32_t B_gain)
{
    printk("[Sunny] imx179_update_awb_gain++!\n");
    #ifdef IMX179_OTP_DEBUG
    printk("[Sunny] imx179_R_gain1 == 0x%x\n",imx179_R_gain1);
    printk("[Sunny] imx179_G_gain1 == 0x%x\n",imx179_G_gain1);
    printk("[Sunny] imx179_B_gain1 == 0x%x\n",imx179_B_gain1);
    #endif

    //reset the digital gain
    IMX179MIPI_write_cmos_sensor(0x020F, imx179_G_gain1);
    IMX179MIPI_write_cmos_sensor(0x0211, imx179_R_gain1);
    IMX179MIPI_write_cmos_sensor(0x0213, imx179_B_gain1);
    IMX179MIPI_write_cmos_sensor(0x0215, imx179_G_gain1);
    printk("[Sunny] imx179_update_awb_gain--!\n");
    return 0;
}


int imx179_check_mid(uint mid)
{
    printk("[Sunny] imx179_check_mid++\n");
    uint i,temp, otp_index;
    struct imx179_otp_struct current_otp;
	
    // R/G and B/G of current camera module is read out from sensor OTP
    // check first wb OTP with valid data
    for(i=3;i>0;i--) {
        temp = imx179_check_otp_awb(i);
        if (temp == 1) {
            otp_index = i;
            break;
        }
    }
    if(i<0) {
        // no valid wb OTP data
        printk("[Sunny] imx179_check_mid! no valid wb OTP data\n");
        return 1;
    }

    memset(&current_otp, 0, sizeof(current_otp));
    if(imx179_read_otp_wb(otp_index, &current_otp)) return 1;

    //add for distinguish diffrent moudle Truly or Sunny
    printk("[Sunny] #########current_otp->vendor_id = 0x%x\n",current_otp.vendor_id);

    if(current_otp.vendor_id != mid)
    {
        return 1;
    }

    return 0;
}

