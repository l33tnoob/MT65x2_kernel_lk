/*************************************************************************************************
3h7_otp.c
---------------------------------------------------------
OTP Application file From Truly for S5K3H7
2013.01.14
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

#include "s5k3h7ymipiraw_Sensor.h"
#include "s5k3h7ymipiraw_Camera_Sensor_para.h"
#include "s5k3h7ymipiraw_CameraCustomized.h"

#include <linux/xlog.h>


extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
//extern  BYTE S5K3H7Y_byteread_cmos_sensor(kal_uint32 addr);

#define SENSORDB_S5K3H7_OTP(fmt,arg...) xlog_printk(ANDROID_LOG_DEBUG , "[S5K3H7_OTP]", fmt, ##arg)  							//printk(LOG_TAG "%s: " fmt "\n", __FUNCTION__ ,##arg)

BYTE S5K3H7Y_byteread_cmos_sensor(kal_uint32 addr)
{
	BYTE get_byte=0;
	char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,S5K3H7YMIPI_WRITE_ID);
	return get_byte;
}

extern  void S5K3H7Y_wordwrite_cmos_sensor(u16 addr, u32 para);
extern  void S5K3H7Y_bytewrite_cmos_sensor(u16 addr, u32 para);
#define USHORT             unsigned short
#define BYTE               unsigned char
#define Sleep(ms) mdelay(ms)

#define DARLING_ID          0x05
#define VALID_OTP          0x00

#define GAIN_DEFAULT       0x0100
#define GAIN_GREENR_ADDR   0x020E
#define GAIN_BLUE_ADDR     0x0212
#define GAIN_RED_ADDR      0x0210
#define GAIN_GREENB_ADDR   0x0214

#define GOLDEN_RGr_RATIO  0x0319
#define GOLDEN_BGr_RATIO 0x025a
#define GOLDEN_GbGr_RATIO 0x0400


USHORT RGR_RATIO= 0;
USHORT BGR_RATIO= 0;
USHORT GBGR_RATIO= 0;

static u32  s_g_R_gain = 0;
static u32  s_g_Gr_gain = 0; 
static u32  s_g_Gb_gain = 0; 
static u32  s_g_B_gain = 0;

/*************************************************************************************************
* Function    :  start_read_otp
* Description :  before read otp , set the reading block setting  
* Parameters  :  [BYTE] zone : OTP PAGE index , 0x00~0x0f
* Return      :  0, reading block setting err
                 1, reading block setting ok 
**************************************************************************************************/
bool start_read_otp(BYTE zone)
{
    BYTE val = 0;
    int i;

    for(i=0;i<100;i++)
    {
        S5K3H7Y_wordwrite_cmos_sensor(0xFCFC, 0xD000);
        S5K3H7Y_bytewrite_cmos_sensor(0x0A02, zone);   //Select the page to write by writing to 0xD0000A02 0x01~0x0C
        S5K3H7Y_bytewrite_cmos_sensor(0x0A00, 0x0101);   //Enter read mode by writing 01h to 0xD0000A00, 0x01 --> 0x0101 by Qtech FAE 2013-07-24

        val = S5K3H7Y_byteread_cmos_sensor(0x0A01);
        if(val == 0x01)
            break;
        Sleep(10);
	}
    if(i == 100)
    {
        SENSORDB_S5K3H7_OTP("Read Page %d Err! \n", zone); // print log
        S5K3H7Y_bytewrite_cmos_sensor(0x0A00, 0x00);   //Reset the NVM interface by writing 00h to 0xD0000A00
        return 0;
    }
    return 1;
}

/*************************************************************************************************
* Function    :  stop_read_otp
* Description :  after read otp , stop and reset otp block setting  
**************************************************************************************************/
void stop_read_otp()
{
    S5K3H7Y_bytewrite_cmos_sensor(0x0A00, 0x00);   //Reset the NVM interface by writing 00h to 0xD0000A00
}


/*************************************************************************************************
* Function    :  get_otp_flag
* Description :  get otp WRITTEN_FLAG  
* Parameters  :  [BYTE] zone : OTP PAGE index , 0x00~0x0f
* Return      :  [BYTE], if 0x40 , this type has valid otp data, otherwise, invalid otp data
**************************************************************************************************/
BYTE get_otp_flag(BYTE zone)
{
    BYTE flag = 0xFF;
    if(!start_read_otp(zone))
    {
        SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", zone);
        return 0;
    }
    flag = S5K3H7Y_byteread_cmos_sensor(0x0A0B);
    stop_read_otp();

  //  flag = flag & 0xc0;

    SENSORDB_S5K3H7_OTP("get_otp_flag: 0x%02x \n", flag);

    return flag;
}


/*************************************************************************************************
* Function    :  get_otp_module_id
* Description :  get otp MID value 
* Parameters  :  [BYTE] zone : OTP PAGE index , 0x00~0x0f
* Return      :  [BYTE] 0 : OTP data fail 
                 other value : module ID data , TRULY ID is 0x0001            
**************************************************************************************************/
BYTE get_otp_module_id(BYTE zone)
{
    BYTE module_id = 0;

    if(!start_read_otp(zone))
    {
        SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", zone);
        return 0;
    }

    module_id = S5K3H7Y_byteread_cmos_sensor(0x0A0A);
	
    stop_read_otp();

    SENSORDB_S5K3H7_OTP("Module ID: 0x%02x. \n",module_id);

    return module_id;
}

/*************************************************************************************************
* Function    :  get_otp_lens_id
* Description :  get otp LENS_ID value 
* Parameters  :  [BYTE] zone : OTP PAGE index , 0x00~0x0f
* Return      :  [BYTE] 0 : OTP data fail 
                 other value : LENS ID data             
**************************************************************************************************/
BYTE get_otp_lens_id(BYTE zone)
{
    BYTE lens_id = 0;

    if(!start_read_otp(zone))
    {
        SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", zone);
        return 0;
    }
    lens_id = S5K3H7Y_byteread_cmos_sensor(0x0A0C);
    stop_read_otp();

    SENSORDB_S5K3H7_OTP("Lens ID: 0x%02x. \n",lens_id);

    return lens_id;
}

BYTE get_otp_flag_Darling(void)
{
    BYTE zone;
    BYTE i;
    BYTE FLG = 0x00;
    BYTE MID=0x00;

    zone = 1;
    for(i=0;i<3;i++)
    {
        SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, get_otp_flag_Darling");
        FLG = get_otp_flag(zone);
        SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, get_otp_flag_Darling(%d)=%d(0==OK)", zone, FLG);
        
        if(FLG == VALID_OTP)
            break;
        else
            zone++;
    }
    
    if(i==3)
    {
        SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, No OTP Data or OTP data is invalid \n");
        return 0;
    }
    
    MID = get_otp_module_id(zone);
    SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, get_otp_module_id=0x%x", MID);
    
    if(MID != DARLING_ID)
	{
        SENSORDB_S5K3H7_OTP("No Module} ! \n");
        return 0;
	}
	return zone;
}

/*************************************************************************************************
* Function    :  get_otp_LSC_data(BYTE zone)
* Description :  
* Return      :  
**************************************************************************************************/
bool get_otp_LSC_data(BYTE zone)
{
	BYTE page_idx = 4;
	BYTE cnt = 0;
	BYTE val = 0xFF;
	u16 reg_start = 0x0A04;
	
	if(zone == 1)
	{
		/////////////////////////////////
		//page 4 - 7
		/////////////////////////////////
		for(page_idx = 4; page_idx < 8; page_idx ++)
		{
			reg_start = 0x0A04;
			SENSORDB_S5K3H7_OTP("LSC data page = %d \n",  page_idx);
			if(!start_read_otp(page_idx))
			{
				SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", page_idx);
				return 0;
			}
			//Read full page size (64bytes)
			//Reg from 0x0A04 ~ 0x0A43
			for(cnt = 0; cnt < 64; cnt ++)
			{
				val = S5K3H7Y_byteread_cmos_sensor(reg_start);
				SENSORDB_S5K3H7_OTP("LSC data [0x%02x , 0x%02x] \n",  reg_start, val);
				reg_start ++;
			}
			stop_read_otp();
		}
		/////////////////////////////////
		//page 8
		/////////////////////////////////
		SENSORDB_S5K3H7_OTP("LSC data page = %d \n",  page_idx);
		if(!start_read_otp(page_idx))
		{
			SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", page_idx);
			return 0;
		}
		reg_start = 0x0A04;
		for(cnt = 0; cnt < 9; cnt ++)
		{
			val = S5K3H7Y_byteread_cmos_sensor(reg_start);
			SENSORDB_S5K3H7_OTP("LSC data [0x%02x , 0x%02x] \n",  reg_start, val);
			reg_start ++;
		}
		stop_read_otp();
	}
	else if(zone == 2)
	{
		/////////////////////////////////
		//page 9 - 12
		/////////////////////////////////
		SENSORDB_S5K3H7_OTP("LSC data page = %d \n",  page_idx);
		for(page_idx = 9; page_idx < 13; page_idx ++)
		{
			reg_start = 0x0A04;
			if(!start_read_otp(page_idx))
			{
				SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", page_idx);
				return 0;
			}
			//Read full page size (64bytes)
			//Reg from 0x0A04 ~ 0x0A43
			for(cnt = 0; cnt < 64; cnt ++)
			{
				val = S5K3H7Y_byteread_cmos_sensor(reg_start);
				SENSORDB_S5K3H7_OTP("LSC data [0x%02x , 0x%02x] \n",  reg_start, val);
				reg_start ++;
			}
			stop_read_otp();
		}
		/////////////////////////////////
		//page 13
		/////////////////////////////////
		SENSORDB_S5K3H7_OTP("LSC data page = %d\n",  page_idx);
		if(!start_read_otp(page_idx))
		{
			SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", page_idx);
			return 0;
		}
		reg_start = 0x0A04;
		for(cnt = 0; cnt < 9; cnt ++)
		{
			val = S5K3H7Y_byteread_cmos_sensor(reg_start);
			SENSORDB_S5K3H7_OTP("LSC data [0x%02x , 0x%02x] \n",  reg_start, val);
			reg_start ++;
		}
		stop_read_otp();
	}
	else
	{
		SENSORDB_S5K3H7_OTP("read LSC data zone err! \n");
		return 0;
	}
	return 1;
}


/*************************************************************************************************
* Function    :  get_otp_wb
* Description :  Get WB data    
* Parameters  :  [BYTE] zone : OTP PAGE index , 0x00~0x0f      
**************************************************************************************************/
bool get_otp_awb(BYTE zone)
{
    BYTE temph = 0;
    BYTE templ = 0;
   
//    RGR_RATIO = 0; BGR_RATIO = 0; GBGR_RATIO = 0;
    
    if(!start_read_otp(zone))
    {
    	SENSORDB_S5K3H7_OTP("Start read Page %d Fail! \n", zone);
    	return 0;
    }
    
    temph = S5K3H7Y_byteread_cmos_sensor(0x0A04);  
    templ = S5K3H7Y_byteread_cmos_sensor(0x0A05);   
    RGR_RATIO  = (USHORT)templ + ((USHORT)temph  << 8);

    temph = S5K3H7Y_byteread_cmos_sensor(0x0A06);   
    templ = S5K3H7Y_byteread_cmos_sensor(0x0A07);
    BGR_RATIO  = (USHORT)templ + ((USHORT)temph  << 8);
	
    temph  = S5K3H7Y_byteread_cmos_sensor(0x0A08);   
    templ = S5K3H7Y_byteread_cmos_sensor(0x0A09);
    GBGR_RATIO  = (USHORT)templ + ((USHORT)temph << 8);
    

    SENSORDB_S5K3H7_OTP("RGR_RATIO: %x,BGR_RATIO:%x,GBGR_RATIO:%x \n", RGR_RATIO,BGR_RATIO,GBGR_RATIO);

    stop_read_otp();
    
    return 1;
}

u16 awb_gain_set(u16 R_gain, u16 B_gain,u16 Gr_gain,u16 Gb_gain)
{
	SENSORDB_S5K3H7_OTP("HJDDbg3h7AWB, Darling AWB is set, R_gain:%x,B_gain:%x,Gr_gain:%x,Gb_gain:%x \n ", R_gain, B_gain, Gr_gain, Gb_gain);	
	S5K3H7Y_wordwrite_cmos_sensor(GAIN_GREENR_ADDR, Gr_gain); //Green 1 default gain 1x
	S5K3H7Y_wordwrite_cmos_sensor(GAIN_GREENB_ADDR, Gb_gain); //Green 2 default gain 1x
	S5K3H7Y_wordwrite_cmos_sensor(GAIN_RED_ADDR, R_gain);	
	S5K3H7Y_wordwrite_cmos_sensor(GAIN_BLUE_ADDR, B_gain);	
	return 1;
}
/*************************************************************************************************
* Function    :  otp_wb_update
* Description :  Update WB correction 
* Return      :  [bool] 0 : OTP data fail 
                        1 : otp_WB update success            
**************************************************************************************************/
bool otp_awb_update(BYTE zone)
{
      u32 R_gain, G_gain, B_gain, G_gain_R, G_gain_B,Gr_gain,Gb_gain;
      u32 rg = 0;
      u32 bg = 0;
      u32 GbGr = 0;
	if(!get_otp_awb(zone))  // get wb data from otp
		return 0;

    rg = RGR_RATIO;
    bg = BGR_RATIO;
    GbGr = GBGR_RATIO;
    if (0 == rg || 0 == bg || 0 == GbGr)
    {    
        SENSORDB_S5K3H7_OTP("HJDDbg3h7AWB, Fatal error! devide by 0, update_otp_AWB rg=0x%x, bg=0x%x, GbGr=%d \n", rg, bg, GbGr);
        return 0;
    }
    
    if (bg < GOLDEN_BGr_RATIO) {
        if (rg< GOLDEN_RGr_RATIO) {
            G_gain = GAIN_DEFAULT;
            B_gain = (GAIN_DEFAULT * GOLDEN_BGr_RATIO) / bg;
            R_gain = (GAIN_DEFAULT * GOLDEN_RGr_RATIO) / rg;
        }else {
            R_gain = GAIN_DEFAULT;
            G_gain = (GAIN_DEFAULT * rg) / GOLDEN_RGr_RATIO;
            B_gain = (G_gain * GOLDEN_BGr_RATIO) /bg;
    }
    }else {
        if (rg < GOLDEN_RGr_RATIO) {
            B_gain = GAIN_DEFAULT;
            G_gain = (GAIN_DEFAULT * bg)/ GOLDEN_BGr_RATIO;
            R_gain = (G_gain * GOLDEN_RGr_RATIO)/ rg;
        }else {
            G_gain_B = (GAIN_DEFAULT * bg) / GOLDEN_BGr_RATIO;
            G_gain_R = (GAIN_DEFAULT * rg) / GOLDEN_RGr_RATIO;

            if(G_gain_B > G_gain_R ) {
                B_gain = GAIN_DEFAULT;
                G_gain = G_gain_B;
                R_gain = (G_gain * GOLDEN_RGr_RATIO) /rg;
            }else {
                R_gain = GAIN_DEFAULT;
                G_gain = G_gain_R;
                B_gain = (G_gain * GOLDEN_BGr_RATIO) / bg;
            }
        	}
    	}

	if (GbGr <= GOLDEN_GbGr_RATIO)
		{Gr_gain = G_gain;
	         Gb_gain = G_gain * GOLDEN_GbGr_RATIO / GbGr;		 
              }
	else 
		{Gb_gain = G_gain;
	         Gr_gain = G_gain * GbGr /GOLDEN_GbGr_RATIO;
		  R_gain = R_gain * GbGr /GOLDEN_GbGr_RATIO;
		  B_gain = B_gain * GbGr /GOLDEN_GbGr_RATIO;
		  if(R_gain > B_gain)
		  	{GbGr = B_gain;}
		  else
		  	{GbGr = R_gain;}
		  if(GbGr > Gb_gain) GbGr = Gb_gain;

		  Gb_gain = Gb_gain * GAIN_DEFAULT / GbGr ;
	         Gr_gain = Gr_gain * GAIN_DEFAULT / GbGr;
		  R_gain = R_gain * GAIN_DEFAULT / GbGr ;
		  B_gain = B_gain * GAIN_DEFAULT / GbGr ;
		}

    SENSORDB_S5K3H7_OTP("HJDDbg3h7AWB, read OTP AWB, R_gain:%x,B_gain:%x,Gr_gain:%x,Gb_gain:%x \n ", R_gain, B_gain, Gr_gain, Gb_gain);	
    s_g_R_gain = R_gain;
    s_g_B_gain = B_gain;
    s_g_Gr_gain = Gr_gain; 
    s_g_Gb_gain = Gb_gain; 
	awb_gain_set(R_gain, B_gain,Gr_gain,Gb_gain);

	SENSORDB_S5K3H7_OTP("WB update finished! \n");
	return 1;
}

/*************************************************************************************************
* Function    :  otp_update()
* Description :  update otp data from otp , it otp data is valid, 
                 it include get ID and WB update function  
* Return      :  [bool] 0 : update fail
                        1 : update success

*update shunyu module and qiutai old module
**************************************************************************************************/
bool Darling_otp_update()
{
    BYTE zone = 0x00;
    zone = get_otp_flag_Darling();
    
    if (zone)
    {   
        get_otp_module_id(zone);	
        get_otp_lens_id(zone);	
        otp_awb_update(zone);
        return 1;
    }else
    {	
        SENSORDB_S5K3H7_OTP("otp_flag invalid or empty! \n");
        return 0;
    }
}


bool is_Darling3h7()
{
    BYTE module_id = 0;
    BYTE zone = 0;
    bool bRet = 0;

    SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, in is_Darling3h7");
    S5K3H7Y_bytewrite_cmos_sensor(0x0100, 0x01);    // smiaRegs_rw_general_setup_mode_select
    zone = get_otp_flag_Darling();

    if (zone)
    {   
        module_id = get_otp_module_id(zone);	
        SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, is_Darling3h7, module_id=0x%x \n", module_id);
        if (DARLING_ID == module_id)
        {
            SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, is_Darling3h7, it is Darling3h7! \n");
            bRet = 1;
        }
    }else
    {	
        SENSORDB_S5K3H7_OTP("otp_flag invalid or empty! \n");
    }

    S5K3H7Y_bytewrite_cmos_sensor(0x0100, 0x00);    // smiaRegs_rw_general_setup_mode_select    
    SENSORDB_S5K3H7_OTP("HJDDbg3h7ID, OTP, out is_Darling3h7, bRet=%d \n", bRet);
    return bRet;
}


/*************************************************************************************************
* Function    :  Darling_otp_set_AWB_gain()
* Description :  update AWB gain from static global variables, if not ready, read OTP again  
* Return      :  [bool] 0 : update fail
                        1 : update success
**************************************************************************************************/
bool Darling_otp_set_AWB_gain()
{
    if (0 != s_g_R_gain
        && 0 != s_g_Gr_gain
        && 0 != s_g_Gb_gain
        && 0 != s_g_B_gain)
    {
        printk("HJDDbg3h7AWB, Darling3h7otp, set_AWB_gain(OK), R=0x%x, Gr=0x%x, Gb=0x%x, B=0x%x \n", s_g_R_gain, s_g_Gr_gain, s_g_Gr_gain, s_g_B_gain);
        return awb_gain_set(s_g_R_gain, s_g_B_gain, s_g_Gr_gain, s_g_Gb_gain);
    }
    else
    {
        printk("HJDDbg3h7AWB, Darling3h7otp, set_AWB_gain(NOK), R=0x%x, Gr=0x%x, Gb=0x%x, B=0x%x \n", s_g_R_gain, s_g_Gr_gain, s_g_Gr_gain, s_g_B_gain);
        return Darling_otp_update();
    }
}

