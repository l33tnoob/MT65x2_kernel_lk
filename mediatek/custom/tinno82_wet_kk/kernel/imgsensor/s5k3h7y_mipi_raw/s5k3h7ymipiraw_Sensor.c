/*******************************************************************************************/


/*******************************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/xlog.h>
#include <asm/atomic.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "s5k3h7ymipiraw_Sensor.h"
#include "s5k3h7ymipiraw_Camera_Sensor_para.h"
#include "s5k3h7ymipiraw_CameraCustomized.h"
static DEFINE_SPINLOCK(s5k3h7ymipiraw_drv_lock);


//#define S5K3H7Y_DEBUG_SOFIA

#define mDELAY(ms)  mdelay(ms)
#define Sleep(ms) mdelay(ms)

#define S5K3H7Y_DEBUG
#ifdef S5K3H7Y_DEBUG
#define LOG_TAG (__FUNCTION__)
#define SENSORDB(fmt,arg...) xlog_printk(ANDROID_LOG_DEBUG , LOG_TAG, fmt, ##arg)  							//printk(LOG_TAG "%s: " fmt "\n", __FUNCTION__ ,##arg)
#else
#define SENSORDB(fmt,arg...)
#endif

#define SENSOR_PCLK_PREVIEW  	28000*10000 //26000*10000  //27600*10000
#define SENSOR_PCLK_VIDEO  		SENSOR_PCLK_PREVIEW //26000*10000
#define SENSOR_PCLK_CAPTURE  	SENSOR_PCLK_PREVIEW //26000*10000
#define SENSOR_PCLK_ZSD  		SENSOR_PCLK_CAPTURE

#define S5K3H7_TEST_PATTERN_CHECKSUM (0xadc56499)

#if 0
#define S5K3H7Y_DEBUG
#ifdef S5K3H7Y_DEBUG
	//#define S5K3H7YDB(fmt, arg...) printk( "[S5K3H7YRaw] "  fmt, ##arg)
	#define S5K3H7YDB(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[S5K3H7YRaw]" fmt, #arg)
#else
	#define S5K3H7YDB(x,...)
#endif

#ifdef S5K3H7Y_DEBUG_SOFIA
	#define S5K3H7YDBSOFIA(fmt, arg...) printk( "[S5K3H7YRaw] "  fmt, ##arg)
#else
	#define S5K3H7YDBSOFIA(x,...)
#endif
#endif
#define S5K3H7Y_OTP_USE
#ifdef S5K3H7Y_OTP_USE
extern bool Darling_otp_set_AWB_gain();
extern bool is_Darling3h7();
#endif

//kal_uint32 S5K3H7Y_FeatureControl_PERIOD_PixelNum=S5K3H7Y_PV_PERIOD_PIXEL_NUMS;
//kal_uint32 S5K3H7Y_FeatureControl_PERIOD_LineNum=S5K3H7Y_PV_PERIOD_LINE_NUMS;
MSDK_SENSOR_CONFIG_STRUCT S5K3H7YSensorConfigData;

kal_uint32 S5K3H7Y_FAC_SENSOR_REG;
static MSDK_SCENARIO_ID_ENUM s_S5K3H7YCurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT S5K3H7YSensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT S5K3H7YSensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/

static S5K3H7Y_PARA_STRUCT s5k3h7y;
static kal_uint16 s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID;

extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
UINT32 S5K3H7YMIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate);

kal_uint32 S5K3H7Y_FRAME_LENGTH_COUNT = S5K3H7Y_PV_PERIOD_LINE_NUMS;
inline kal_uint16 S5K3H7Y_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
	char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte, 2, s5k3h7y_slave_addr);
	return ((get_byte<<8)&0xff00)|((get_byte>>8)&0x00ff);
}

inline void S5K3H7Y_wordwrite_cmos_sensor(u16 addr, u32 para)
{
	char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,  (char)(para >> 8),	(char)(para & 0xFF) };
	iWriteRegI2C(puSendCmd , 4, s5k3h7y_slave_addr);
}

inline void S5K3H7Y_bytewrite_cmos_sensor(u16 addr, u32 para)
{
	char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF)  ,	(char)(para & 0xFF) };
	iWriteRegI2C(puSendCmd, 3, s5k3h7y_slave_addr);
}

static inline kal_uint32 GetScenarioLinelength(void)
{
	kal_uint32 u4Linelength=S5K3H7Y_PV_PERIOD_PIXEL_NUMS; //+s5k3h7y.DummyPixels;
	switch(s_S5K3H7YCurrentScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			u4Linelength=S5K3H7Y_PV_PERIOD_PIXEL_NUMS; //+s5k3h7y.DummyPixels;
		break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			u4Linelength=S5K3H7Y_VIDEO_PERIOD_PIXEL_NUMS; //+s5k3h7y.DummyPixels;
		break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			u4Linelength=S5K3H7Y_ZSD_PERIOD_PIXEL_NUMS; //+s5k3h7y.DummyPixels;
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			u4Linelength=S5K3H7Y_FULL_PERIOD_PIXEL_NUMS; //+s5k3h7y.DummyPixels;
		break;
		default:
		break;
	}
	//SENSORDB("u4Linelength=%d\n",u4Linelength);
	return u4Linelength;
}

static inline kal_uint32 GetScenarioPixelClock(void)
{
	kal_uint32 Pixelcloclk = s5k3h7y.pvPclk;
	switch(s_S5K3H7YCurrentScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			Pixelcloclk = s5k3h7y.pvPclk;
		break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			Pixelcloclk = s5k3h7y.m_vidPclk;
		break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			Pixelcloclk = s5k3h7y.capPclk;
		break;
		default:
		break;
	}
	//SENSORDB("u4Linelength=%d\n",u4Linelength);
	return Pixelcloclk;		
}


static inline kal_uint32 GetScenarioFramelength(void)
{
	kal_uint32 u4Framelength=S5K3H7Y_PV_PERIOD_LINE_NUMS; //+s5k3h7y.DummyLines ;
	switch(s_S5K3H7YCurrentScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			u4Framelength=S5K3H7Y_PV_PERIOD_LINE_NUMS; //+s5k3h7y.DummyLines ;
		break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			u4Framelength=S5K3H7Y_VIDEO_PERIOD_LINE_NUMS; //+s5k3h7y.DummyLines ;
		break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			u4Framelength=S5K3H7Y_ZSD_PERIOD_LINE_NUMS; //+s5k3h7y.DummyLines ;
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			u4Framelength=S5K3H7Y_FULL_PERIOD_LINE_NUMS; //+s5k3h7y.DummyLines ;
		break;
		default:
		break;
	}
	//SENSORDB("u4Framelength=%d\n",u4Framelength);
	return u4Framelength;
}


static inline void SetLinelength(kal_uint16 u2Linelength)
{
	SENSORDB("u4Linelength=%d\n",u2Linelength);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x01);	 //Grouped parameter hold
	S5K3H7Y_wordwrite_cmos_sensor(0x342,u2Linelength);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00);	 //Grouped parameter release
}


static inline void SetFramelength(kal_uint16 a_u2FrameLen)
{
  SENSORDB("a_u2FrameLen=%d\n",a_u2FrameLen);
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.maxExposureLines = a_u2FrameLen;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x01);	 //Grouped parameter hold
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,a_u2FrameLen);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00);	 //Grouped parameter release
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	S5K3H7Y_FRAME_LENGTH_COUNT = a_u2FrameLen;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);
}



void S5K3H7Y_write_shutter(kal_uint32 shutter)
{
	kal_uint16 frame_length = 0, line_length = 0, framerate = 0 , frame_length_min = 0;	
	kal_uint32 pixelclock = 0;
	unsigned long flags;

	#define SHUTTER_FRAMELENGTH_MARGIN 16
	
	//frame_length = GetScenarioFramelength();

	//frame_length = (s5k3h7y.FixedFrameLength>frame_length)?s5k3h7y.FixedFrameLength:frame_length;
	
	if (shutter < 3)
		shutter = 3;

	//if (shutter+SHUTTER_FRAMELENGTH_MARGIN > frame_length)
		frame_length = shutter + SHUTTER_FRAMELENGTH_MARGIN; //extend framelength

	//frame_length_min = GetScenarioFramelength();

	frame_length_min = S5K3H7Y_FRAME_LENGTH_COUNT;
	SENSORDB("0x0340=%d\n",frame_length_min);
	if(frame_length < frame_length_min)
		frame_length = frame_length_min;
	

	if(s5k3h7y.S5K3H7YAutoFlickerMode == KAL_TRUE)
	{
		line_length = GetScenarioLinelength();
		pixelclock = GetScenarioPixelClock();
		framerate = (10 * pixelclock) / (frame_length * line_length);
		  
		if(framerate > 290)
		{
		  	framerate = 290;
		  	frame_length = (10 * pixelclock) / (framerate * line_length);
		}
		else if(framerate > 147 && framerate < 152)
		{
		  	framerate = 147;
				frame_length = (10 * pixelclock) / (framerate * line_length);
		}
	}

	spin_lock_irqsave(&s5k3h7ymipiraw_drv_lock,flags);
	s5k3h7y.maxExposureLines = frame_length;
	s5k3h7y.shutter = shutter;
	spin_unlock_irqrestore(&s5k3h7ymipiraw_drv_lock,flags);

	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x01);    //Grouped parameter hold
	S5K3H7Y_wordwrite_cmos_sensor(0x0340, frame_length);
 	S5K3H7Y_wordwrite_cmos_sensor(0x0202, shutter);
 	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00);    //Grouped parameter release

	SENSORDB("shutter=%d,frame_length=%d,framerate=%d\n",shutter,frame_length, framerate);
}   /* write_S5K3H7Y_shutter */


void write_S5K3H7Y_gain(kal_uint16 gain)
{
	unsigned long flags;
	SENSORDB("gain=%d\n",gain);
	spin_lock_irqsave(&s5k3h7ymipiraw_drv_lock,flags);
	s5k3h7y.sensorGain = gain/2;
	spin_unlock_irqrestore(&s5k3h7ymipiraw_drv_lock,flags);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x01);    //Grouped parameter hold
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,s5k3h7y.sensorGain);   //sensor base gain is 32
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00);    //Grouped parameter release
}

/*************************************************************************
* FUNCTION
*    S5K3H7Y_SetGain
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
void S5K3H7Y_SetGain(UINT16 iGain)
{

	write_S5K3H7Y_gain(iGain);

}


/*************************************************************************
* FUNCTION
*    read_S5K3H7Y_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 read_S5K3H7Y_gain(void)
{
	kal_uint16 read_gain=S5K3H7Y_read_cmos_sensor(0x0204);
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.sensorGain = read_gain;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);
	return s5k3h7y.sensorGain;
}


void S5K3H7Y_camera_para_to_sensor(void)
{
  /*  kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=S5K3H7YSensorReg[i].Addr; i++)
    {
        S5K3H7Y_wordwrite_cmos_sensor(S5K3H7YSensorReg[i].Addr, S5K3H7YSensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=S5K3H7YSensorReg[i].Addr; i++)
    {
        S5K3H7Y_wordwrite_cmos_sensor(S5K3H7YSensorReg[i].Addr, S5K3H7YSensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        S5K3H7Y_wordwrite_cmos_sensor(S5K3H7YSensorCCT[i].Addr, S5K3H7YSensorCCT[i].Para);
    }*/
}


/*************************************************************************
* FUNCTION
*    S5K3H7Y_sensor_to_camera_para
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
void S5K3H7Y_sensor_to_camera_para(void)
{
/*    kal_uint32    i, temp_data;
    for(i=0; 0xFFFFFFFF!=S5K3H7YSensorReg[i].Addr; i++)
    {
         temp_data = S5K3H7Y_read_cmos_sensor(S5K3H7YSensorReg[i].Addr);
		 spin_lock(&s5k3h7ymipiraw_drv_lock);
		 S5K3H7YSensorReg[i].Para =temp_data;
		 spin_unlock(&s5k3h7ymipiraw_drv_lock);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=S5K3H7YSensorReg[i].Addr; i++)
    {
        temp_data = S5K3H7Y_read_cmos_sensor(S5K3H7YSensorReg[i].Addr);
		spin_lock(&s5k3h7ymipiraw_drv_lock);
		S5K3H7YSensorReg[i].Para = temp_data;
		spin_unlock(&s5k3h7ymipiraw_drv_lock);
    }*/
}

/*************************************************************************
* FUNCTION
*    S5K3H7Y_get_sensor_group_count
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
kal_int32  S5K3H7Y_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void S5K3H7Y_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
 /*  switch (group_idx)
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
	}*/
}

void S5K3H7Y_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{
/*    kal_int16 temp_reg=0;
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

            temp_para= S5K3H7YSensorCCT[temp_addr].Para;
			temp_gain= (temp_para*1000/s5k3h7y.sensorBaseGain) ;

            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min= S5K3H7Y_MIN_ANALOG_GAIN * 1000;
            info_ptr->Max= S5K3H7Y_MAX_ANALOG_GAIN * 1000;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");

                    //temp_reg=MT9P017SensorReg[CMMCLK_CURRENT_INDEX].Para;
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
                    info_ptr->ItemValue=    111;  //MT9P017_MAX_EXPOSURE_LINES;
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
    }*/
}



kal_bool S5K3H7Y_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
/*
   kal_uint16  temp_gain=0,temp_addr=0, temp_para=0;
   switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
				case 0:	temp_addr = PRE_GAIN_R_INDEX;		break;
				case 1:	temp_addr = PRE_GAIN_Gr_INDEX;		break;
				case 2: temp_addr = PRE_GAIN_Gb_INDEX;		break;
				case 3: temp_addr = PRE_GAIN_B_INDEX;		break;
				case 4:	temp_addr = SENSOR_BASEGAIN;		break;
				default: ASSERT(0);
          }

			temp_gain=((ItemValue*s5k3h7y.sensorBaseGain+500)/1000);			//+500:get closed integer value

		  spin_lock(&s5k3h7ymipiraw_drv_lock);
          S5K3H7YSensorCCT[temp_addr].Para = temp_para;
		  spin_unlock(&s5k3h7ymipiraw_drv_lock);
          S5K3H7Y_wordwrite_cmos_sensor(S5K3H7YSensorCCT[temp_addr].Addr,temp_para);
          break;

        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    //no need to apply this item for driving current
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
					spin_lock(&s5k3h7ymipiraw_drv_lock);
                    S5K3H7Y_FAC_SENSOR_REG=ItemValue;
					spin_unlock(&s5k3h7ymipiraw_drv_lock);
                    break;
                case 1:
                    S5K3H7Y_wordwrite_cmos_sensor(S5K3H7Y_FAC_SENSOR_REG,ItemValue);
                    break;
                default:
                    ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }*/
    return KAL_TRUE;
}
#if 0
static void S5K3H7Y_SetDummy( const kal_uint32 iPixels, const kal_uint32 iLines )
{
	kal_uint16 u2Linelength = 0,u2Framelength = 0;
	SENSORDB("iPixels=%d,iLines=%d\n",iPixels,iLines);

	switch (s_S5K3H7YCurrentScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			u2Linelength = S5K3H7Y_PV_PERIOD_PIXEL_NUMS+iPixels;
			u2Framelength = S5K3H7Y_PV_PERIOD_LINE_NUMS+iLines;
		break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			u2Linelength = S5K3H7Y_VIDEO_PERIOD_PIXEL_NUMS+iPixels;
			u2Framelength = S5K3H7Y_VIDEO_PERIOD_LINE_NUMS+iLines;
		break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			u2Linelength = S5K3H7Y_ZSD_PERIOD_PIXEL_NUMS+iPixels;
			u2Framelength = S5K3H7Y_ZSD_PERIOD_LINE_NUMS+iLines;
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			u2Linelength = S5K3H7Y_FULL_PERIOD_PIXEL_NUMS+iPixels;
			u2Framelength = S5K3H7Y_FULL_PERIOD_LINE_NUMS+iLines;
		break;
		default:
			u2Linelength = S5K3H7Y_PV_PERIOD_PIXEL_NUMS+iPixels;
			u2Framelength = S5K3H7Y_PV_PERIOD_LINE_NUMS+iLines;
		break;
	}

	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.maxExposureLines = u2Framelength;
	//S5K3H7Y_FeatureControl_PERIOD_PixelNum = u2Linelength;
	//S5K3H7Y_FeatureControl_PERIOD_LineNum = u2Framelength;
	s5k3h7y.DummyPixels=iPixels;
	s5k3h7y.DummyLines=iLines;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x01);    //Grouped parameter hold
	S5K3H7Y_wordwrite_cmos_sensor(0x340,u2Framelength);
	S5K3H7Y_wordwrite_cmos_sensor(0x342,u2Linelength);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00);    //Grouped parameter hold
}   /*  S5K3H7Y_SetDummy */
#endif
static void S5K3H7YInitSetting(void)
{
	SENSORDB("enter\n");
  //1600x1200
	S5K3H7Y_wordwrite_cmos_sensor(0x6010,0x0001);	// Reset		
	Sleep(10);//; delay(10ms)

	//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);             //line start/end short packet




		// Start T&P part
	// DO NOT DELETE T&P SECTION COMMENTS! They are required to debug T&P related issues.
	// https://svn/svn/SVNRoot/System/Software/tcevb/SDK+FW/ISP_3H5_7/Firmware
	// SVN Rev: 42829-42829
	// ROM Rev: A2
	// Signature:
	// md5 6635cfefc46e5d2dd5b22f432aec0332 .btp
	// md5 4580d7ed6db736afc59a5e3ea0e17055 .htp
	// md5 0356eb91915c3ca8721b185cd3fae77e .RegsMap.h
	// md5 e0442036cb967231ecfd2342ec017ef2 .RegsMap.bin
	// md5 08aee70892241325891780836db778d2 .base.RegsMap.h
	// md5 8b85eff39783953fbe358970e8f6a9fa .base.RegsMap.bin
	//
	S5K3H7Y_wordwrite_cmos_sensor(0x6028, 0x7000);
	S5K3H7Y_wordwrite_cmos_sensor(0x602A, 0x1750);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x10B5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x6DFB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x6FFB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x10BC);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x08BC);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1847);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2DE9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3840);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x10E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0050);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9F15);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x6006);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9015);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5013);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x000A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0A00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5446);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD700);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x001A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0600);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0120);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2F00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA901);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDDE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA001);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC4E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD700);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0500);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBDE8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3840);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA501);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2016);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x012C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8030);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x83E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x013C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC3E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBE28);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFBA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF9FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2FE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1EFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2DE9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF4C5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDCE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1021);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x52E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x001A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0A00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE8E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8CE0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0231);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8EE0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8250);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD5E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB050);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x93E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD840);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0120);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9504);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2444);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x52E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x83E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD840);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFBA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF5FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBDE8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8B01);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2DE9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8B01);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB005);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7310);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBDE8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA805);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFEA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE6FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2DE9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFF4F);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8C45);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA4D0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB20D);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9CA0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0150);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB40D);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5AE3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA023);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x10A0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDB00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD710);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2020);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1500);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0310);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x01E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFF70);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xCDE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBC07);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xCDE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBC05);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C10);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF600);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD4E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD910);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1500);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA00F);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1815);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x6201);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1015);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1820);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5E01);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0425);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x92E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0020);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDC04);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD2E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5921);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x90E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xAC30);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8221);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8310);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFA30);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBE04);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x60E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x012C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x02E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9302);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x20E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9120);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4008);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9884);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5410);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x88E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFC0B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0150);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0064);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4668);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA053);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE043);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3F01);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0098);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4998);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0140);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x05B0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8450);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x55E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF200);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9600);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x89E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3301);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0140);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC5E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x54E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0A00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFDA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF4FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0090);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5810);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8900);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFE09);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0064);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4668);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA053);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE043);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2301);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0088);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4888);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0140);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8450);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x55E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF200);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9600);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x88E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1801);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0140);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC5E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x54E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0A00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFDA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF4FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0080);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0860);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0850);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2300);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1E00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x45E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7C10);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5C20);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8410);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0BE0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5BE3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0A3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE0B3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0101);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0B00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFE00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA410);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0120);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8610);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1117);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1227);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x62B2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0020);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0200);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF200);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x88E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0080);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x86E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0160);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0140);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x54E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0500);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFDA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDEFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x85E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0150);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x55E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0A00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFDA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD9FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x021B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4014);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x51E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x020B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C10);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xAC20);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0A1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80A2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x020B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0111);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA820);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0B3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x020B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0008);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4008);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0410);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x89E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0190);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0D1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x59E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0F00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFBA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA1FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0B00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFBA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7EFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xAC20);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x020A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0C1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0C1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC01F);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80C0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA109);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8412);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0D3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x010C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0C1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC006);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9C10);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9C10);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0050);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0088);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4888);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB480);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0060);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x40E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x029B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8600);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF210);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x40E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x07E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF8C1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0130);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x40E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x02E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDCE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0720);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1310);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA11F);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDCE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDA20);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1302);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4030);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x44C0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x23E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9931);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x533C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x40C0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA00F);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x21E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x98C1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x44C0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x511C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x01E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9301);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x81E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50B2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA820);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9DE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xAC00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0501);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80A0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDAE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8300);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x40E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0B00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0140);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x54E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0F00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x85E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0150);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xCAE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFBA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD3FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x86E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0160);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x56E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0B00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFBA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC8FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8DE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB4D0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBDE8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF04F);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2FE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1EFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2DE9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF041);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7700);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBD08);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF041);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA003);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA003);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x000A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4811);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBA01);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBC21);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD1E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBE11);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0208);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3051);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3001);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD5E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF030);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBAEA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBCCA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD5E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF220);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x930C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x42E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0360);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x02E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9E02);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4CE0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0E40);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0410);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x40E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0200);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0080);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0001);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x10E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x020C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA011);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0700);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x001B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5E00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x56E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE003);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x000A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0300);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x47E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0610);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5500);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC5E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBDE8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF041);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2FE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1EFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC810);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2DE9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC420);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x42E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0110);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB415);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB800);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4FE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD410);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7020);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x011C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x82E0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8030);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0100);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x50E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC3E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFF3A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFAFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9010);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x3A00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4FE2);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x650E);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x84E5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD0E1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB012);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x51E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x009A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0200);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xA0E3);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x090C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00EB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2C00);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFEA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFEFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBDE8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2FE1);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1EFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD005);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x5014);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xD41E);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1013);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB412);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x9C1E);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBC1E);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0400);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00D0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0093);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8012);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC00B);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE012);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDC1E);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7005);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x902D);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x90A6);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4018);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xF804);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xDC17);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x2418);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0070);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xB417);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC06A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7847);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC046);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xFFEA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xBFFF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7847);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xC046);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x6CCE);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x54C0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8448);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x146C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4C7E);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8CDC);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x48DD);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x7C55);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x744C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE8DE);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x4045);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x1FE5);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x04F0);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0xE8CD);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x80F9);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FA);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FC);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FD);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FE);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0001);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0002);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0003);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0005);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0006);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x8006);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FB);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FC);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FD);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FE);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x00FF);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0001);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0002);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0003);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0005);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12, 0x0000);
	//
	// Parameters Defined in T&P:
	//                                                              1610633348 70000000 STRUCT
	// smiaRegs_rw                                                    576 70000000 STRUCT
	// smiaRegs_ro                                                    432 70000000 STRUCT
	// smiaRegs_rd                                                    112 70000000 STRUCT
	// smiaRegs                                                       688 70000010 STRUCT
	// ContextB                                                       820 70000E30 STRUCT
	// smiaRegsB                                                      688 70000E30 STRUCT
	// smiaRegsB_rd                                                   112 70000E30 STRUCT
	// smiaRegsB_rd_general                                            32 70000E30 STRUCT
	// smiaRegsB_rd_general_model_id                                    2 70000E30 SHORT
	// smiaRegsB_rd_general_revision_number_major                       1 70000E32 CHAR
	// smiaRegsB_rd_general_manufacturer_id                             1 70000E33 CHAR
	// smiaRegsB_rd_general_smia_version                                1 70000E34 CHAR
	// smiaRegsB_rd_general_frame_count                                 1 70000E35 CHAR
	// smiaRegsB_rd_general_pixel_order                                 1 70000E36 CHAR
	// smiaRegsB_rd_general_reserved0                                   1 70000E37 CHAR
	// smiaRegsB_rd_general_data_pedestal                               2 70000E38 SHORT
	// smiaRegsB_rd_general_temperature                                 2 70000E3A SHORT
	// smiaRegsB_rd_general_pixel_depth                                 1 70000E3C CHAR
	// smiaRegsB_rd_general_reserved2                                   3 70000E3D ARRAY
	// smiaRegsB_rd_general_reserved2[0]                                1 70000E3D CHAR
	// smiaRegsB_rd_general_reserved2[1]                                1 70000E3E CHAR
	// smiaRegsB_rd_general_reserved2[2]                                1 70000E3F CHAR
	// smiaRegsB_rd_general_revision_number_minor                       1 70000E40 CHAR
	// smiaRegsB_rd_general_additional_specification_version            1 70000E41 CHAR
	// smiaRegsB_rd_general_module_date_year                            1 70000E42 CHAR
	// smiaRegsB_rd_general_module_date_month                           1 70000E43 CHAR
	// smiaRegsB_rd_general_module_date_day                             1 70000E44 CHAR
	// smiaRegsB_rd_general_module_date_phase                           1 70000E45 CHAR
	// smiaRegsB_rd_general_sensor_model_id                             2 70000E46 SHORT
	// smiaRegsB_rd_general_sensor_revision_number                      1 70000E48 CHAR
	// smiaRegsB_rd_general_sensor_manufacturer_id                      1 70000E49 CHAR
	// smiaRegsB_rd_general_sensor_firmware_version                     1 70000E4A CHAR
	// smiaRegsB_rd_general_reserved3                                   1 70000E4B CHAR
	// smiaRegsB_rd_general_serial_number_hword                         2 70000E4C SHORT
	// smiaRegsB_rd_general_serial_number_lword                         2 70000E4E SHORT
	// smiaRegsB_rd_frame_format                                       32 70000E50 STRUCT
	// smiaRegsB_rd_frame_format_frame_format_model_type                1 70000E50 CHAR
	// smiaRegsB_rd_frame_format_frame_format_model_subtype_col_row     1 70000E51 CHAR
	// smiaRegsB_rd_frame_format_frame_format_descriptor_0              2 70000E52 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_1              2 70000E54 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_2              2 70000E56 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_3              2 70000E58 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_4              2 70000E5A SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_5              2 70000E5C SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_6              2 70000E5E SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_7              2 70000E60 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_8              2 70000E62 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_9              2 70000E64 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_10             2 70000E66 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_11             2 70000E68 SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_12             2 70000E6A SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_13             2 70000E6C SHORT
	// smiaRegsB_rd_frame_format_frame_format_descriptor_14             2 70000E6E SHORT
	// smiaRegsB_rd_analog_gain                                        32 70000E70 STRUCT
	// smiaRegsB_rd_analog_gain_analogue_gain_capabiltiy                2 70000E70 SHORT
	// smiaRegsB_rd_analog_gain_reserved                                2 70000E72 SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_code_min                  2 70000E74 SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_code_max                  2 70000E76 SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_code_step                 2 70000E78 SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_type                      2 70000E7A SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_m0                        2 70000E7C SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_c0                        2 70000E7E SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_m1                        2 70000E80 SHORT
	// smiaRegsB_rd_analog_gain_analogue_gain_c1                        2 70000E82 SHORT
	// smiaRegsB_rd_analog_gain_dummy_align                            12 70000E84 ARRAY
	// smiaRegsB_rd_analog_gain_dummy_align[0]                          2 70000E84 SHORT
	// smiaRegsB_rd_analog_gain_dummy_align[1]                          2 70000E86 SHORT
	// smiaRegsB_rd_analog_gain_dummy_align[2]                          2 70000E88 SHORT
	// smiaRegsB_rd_analog_gain_dummy_align[3]                          2 70000E8A SHORT
	// smiaRegsB_rd_analog_gain_dummy_align[4]                          2 70000E8C SHORT
	// smiaRegsB_rd_analog_gain_dummy_align[5]                          2 70000E8E SHORT
	// smiaRegsB_rd_data_format                                        16 70000E90 STRUCT
	// smiaRegsB_rd_data_format_data_format_model_type                  1 70000E90 CHAR
	// smiaRegsB_rd_data_format_data_format_model_subtype               1 70000E91 CHAR
	// smiaRegsB_rd_data_format_data_format_descriptor_0                2 70000E92 SHORT
	// smiaRegsB_rd_data_format_data_format_descriptor_1                2 70000E94 SHORT
	// smiaRegsB_rd_data_format_data_format_descriptor_2                2 70000E96 SHORT
	// smiaRegsB_rd_data_format_data_format_descriptor_3                2 70000E98 SHORT
	// smiaRegsB_rd_data_format_data_format_descriptor_4                2 70000E9A SHORT
	// smiaRegsB_rd_data_format_data_format_descriptor_5                2 70000E9C SHORT
	// smiaRegsB_rd_data_format_data_format_descriptor_6                2 70000E9E SHORT
	// smiaRegsB_rw                                                   576 70000EA0 STRUCT
	// smiaRegs_ro_edof_cap_uAlphaTempInd                               1 D0001989 CHAR
	// smiaRegs_ro_edof_cap_dummy_align                                 6 D000198A ARRAY
	// smiaRegs_ro_edof_cap_dummy_align[0]                              1 D000198A CHAR
	// smiaRegs_ro_edof_cap_dummy_align[1]                              1 D000198B CHAR
	// smiaRegs_ro_edof_cap_dummy_align[2]                              1 D000198C CHAR
	// smiaRegs_ro_edof_cap_dummy_align[3]                              1 D000198D CHAR
	// smiaRegs_ro_edof_cap_dummy_align[4]                              1 D000198E CHAR
	// smiaRegs_ro_edof_cap_dummy_align[5]                              1 D000198F CHAR
	//
	// End T&P part


	////////////////////////////////////////////////
	//                                            //
	//     End of Parsing Excel File //
	//                                            //
	////////////////////////////////////////////////
	
	S5K3H7Y_wordwrite_cmos_sensor(0x38FA,	0x0030);	
	S5K3H7Y_wordwrite_cmos_sensor(0x38FC,	0x0030);	
	S5K3H7Y_wordwrite_cmos_sensor(0x32CE,0x0060);   // senHal_usWidthStOfsInit		 
	S5K3H7Y_wordwrite_cmos_sensor(0x32D0,0x0024);   // senHal_usHeightStOfsInit 
	S5K3H7Y_wordwrite_cmos_sensor(0x0086,	0x01FF);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,	0xF1D0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6214,	0xF9F0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,	0x0001);	
	S5K3H7Y_wordwrite_cmos_sensor(0xB0C0,	0x000C);
	S5K3H7Y_wordwrite_cmos_sensor(0xF400,	0x0BBC);	
	S5K3H7Y_wordwrite_cmos_sensor(0xF616,	0x0004);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,	0x0000);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,	0xF9F0);	

	#ifdef USE_MIPI_2_LANES
	S5K3H7Y_wordwrite_cmos_sensor(0x0114,0x01);	// #smiaRegs_rw_output_lane_mode
	#endif
//	S5K3H7Y_wordwrite_cmos_sensor(0x3338,0x0264);  //senHal_MaxCdsTime 								0264
	S5K3H7Y_wordwrite_cmos_sensor(0x0136,0x1800);	// #smiaRegs_rw_op_cond_extclk_frequency_mhz
	S5K3H7Y_wordwrite_cmos_sensor(0x0300,0x0002);	// smiaRegs_rw_clocks_vt_pix_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x0302,0x0001);	// smiaRegs_rw_clocks_vt_sys_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x0304,0x0006);	// smiaRegs_rw_clocks_pre_pll_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x0306,0x008C);	// smiaRegs_rw_clocks_pll_multiplier  
	S5K3H7Y_wordwrite_cmos_sensor(0x0308,0x0008);	// smiaRegs_rw_clocks_op_pix_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x030A,0x0001);	// smiaRegs_rw_clocks_op_sys_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x030C,0x0006);	// smiaRegs_rw_clocks_secnd_pre_pll_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5);	
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,0x0660);	// smiaRegs_rw_frame_timing_x_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,0x04C8);	// smiaRegs_rw_frame_timing_y_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);	// #smiaRegs_rw_sub_sample_x_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0003);	// #smiaRegs_rw_sub_sample_x_odd_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);	// #smiaRegs_rw_sub_sample_y_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0003);	// #smiaRegs_rw_sub_sample_y_odd_inc
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0001);	// #smiaRegs_rw_binning_mode
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0022);	// #smiaRegs_rw_binning_type
	S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0001);	// #smiaRegs_rw_binning_weighting
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_PV_PERIOD_PIXEL_NUMS);	// smiaRegs_rw_frame_timing_line_length_pck
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_PV_PERIOD_LINE_NUMS);	// smiaRegs_rw_frame_timing_frame_length_lines
	S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09A5);	  // smiaRegs_rw_integration_time_coarse_integration_time
	S5K3H7Y_bytewrite_cmos_sensor(0x37F8,0x0001);	  // Analog Gain Precision, 0/1/2/3 = 32/64/128/256 base 1X, set 1=> 64 =1X
	s5k3h7y.sensorBaseGain=64;
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020);	
	S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x0001);	  // #smiaRegs_rw_isp_mapped_couplet_correct_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x0000);	  // #smiaRegs_rw_isp_shading_correction_enable

//CONFIGURATION REGISTERS 

	//M2M
	S5K3H7Y_wordwrite_cmos_sensor(0x31FE, 0xC004); // ash_uDecompressXgrid[0]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3200, 0xC4F0); // ash_uDecompressXgrid[1]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3202, 0xCEC8); // ash_uDecompressXgrid[2]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3204, 0xD8A0); // ash_uDecompressXgrid[3]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3206, 0xE278); // ash_uDecompressXgrid[4]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3208, 0xEC50); // ash_uDecompressXgrid[5]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x320A, 0xF628); // ash_uDecompressXgrid[6]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x320C, 0x0000); // ash_uDecompressXgrid[7]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x320E, 0x09D8); // ash_uDecompressXgrid[8]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3210, 0x13B0); // ash_uDecompressXgrid[9]                        
	S5K3H7Y_wordwrite_cmos_sensor(0x3212, 0x1D88); // ash_uDecompressXgrid[10]                       
	S5K3H7Y_wordwrite_cmos_sensor(0x3214, 0x2760); // ash_uDecompressXgrid[11]                       
	S5K3H7Y_wordwrite_cmos_sensor(0x3216, 0x3138); // ash_uDecompressXgrid[12]                       
	S5K3H7Y_wordwrite_cmos_sensor(0x3218, 0x3B10); // ash_uDecompressXgrid[13]                       
	S5K3H7Y_wordwrite_cmos_sensor(0x321A, 0x3FFC); // ash_uDecompressXgrid[14]                       
	                           
	S5K3H7Y_wordwrite_cmos_sensor(0x321C, 0xC004); // ash_uDecompressYgrid[0]     
	S5K3H7Y_wordwrite_cmos_sensor(0x321E, 0xCCD0); // ash_uDecompressYgrid[1]     
	S5K3H7Y_wordwrite_cmos_sensor(0x3220, 0xD99C); // ash_uDecompressYgrid[2]     
	S5K3H7Y_wordwrite_cmos_sensor(0x3222, 0xE668); // ash_uDecompressYgrid[3]     
	S5K3H7Y_wordwrite_cmos_sensor(0x3224, 0xF334); // ash_uDecompressYgrid[4]     
	S5K3H7Y_wordwrite_cmos_sensor(0x3226, 0x0000); // ash_uDecompressYgrid[5]     
	S5K3H7Y_wordwrite_cmos_sensor(0x3228, 0x0CCC); // ash_uDecompressYgrid[6]     
	S5K3H7Y_wordwrite_cmos_sensor(0x322A, 0x1998); // ash_uDecompressYgrid[7]     
	S5K3H7Y_wordwrite_cmos_sensor(0x322C, 0x2664); // ash_uDecompressYgrid[8]     
	S5K3H7Y_wordwrite_cmos_sensor(0x322E, 0x3330); // ash_uDecompressYgrid[9]     
	S5K3H7Y_wordwrite_cmos_sensor(0x3230, 0x3FFC); // ash_uDecompressYgrid[10]    

	S5K3H7Y_wordwrite_cmos_sensor(0x3232, 0x0100); // ash_uDecompressWidth  
	S5K3H7Y_wordwrite_cmos_sensor(0x3234, 0x0100); // ash_uDecompressHeight 

	S5K3H7Y_bytewrite_cmos_sensor(0x3237, 0x00); // ash_uDecompressScale          // 00 - the value for this register is read from NVM page #0 byte #47 bits [3]-[7] i.e. 5 MSB bits  // other value - e.g. 0E, will be read from this register settings in the set file and ignore the value set in NVM as described above
	S5K3H7Y_bytewrite_cmos_sensor(0x3238, 0x09); // ash_uDecompressRadiusShifter 
	S5K3H7Y_bytewrite_cmos_sensor(0x3239, 0x09); // ash_uDecompressParabolaScale 
	S5K3H7Y_bytewrite_cmos_sensor(0x323A, 0x0B); // ash_uDecompressFinalScale    
	S5K3H7Y_bytewrite_cmos_sensor(0x3160, 0x06); // ash_GrasCfg  06  // 36  // [5:5] fegras_gain_clamp   0 _ clamp gain to 0..1023 // _V_// 1 _ clamp_gain to 256..1023// [4:4] fegras_plus_zero   Adjust final gain by the one or the zero // 0 _ [Output = Input x Gain x Alfa]  // _V_// 1 _ [Output = Input x (1 + Gain x Alfa)]
	//BASE Profile parabola start
	//BASE Profile parabola end
	S5K3H7Y_bytewrite_cmos_sensor(0x0B01, 0x32); // smiaRegs_rw_isp_luminance_correction_level 4F :85%  32:70%

	S5K3H7Y_bytewrite_cmos_sensor(0x3161, 0x00); // ash_GrasShifter 00
	S5K3H7Y_wordwrite_cmos_sensor(0x3164, 0x09C4); // ash_luma_params[0]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x3166, 0x0100); // ash_luma_params[0]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x3168, 0x0100); // ash_luma_params[0]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x316A, 0x0100); // ash_luma_params[0]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x316C, 0x0100); // ash_luma_params[0]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x316E, 0x0011); // ash_luma_params[0]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3170, 0x002F); // ash_luma_params[0]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3172, 0x0000); // ash_luma_params[0]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3174, 0x0011); // ash_luma_params[0]_beta[3]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3176, 0x0A8C); // ash_luma_params[1]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x3178, 0x0100); // ash_luma_params[1]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x317A, 0x0100); // ash_luma_params[1]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x317C, 0x0100); // ash_luma_params[1]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x317E, 0x0100); // ash_luma_params[1]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x3180, 0x0011); // ash_luma_params[1]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3182, 0x002F); // ash_luma_params[1]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3184, 0x0000); // ash_luma_params[1]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3186, 0x0011); // ash_luma_params[1]_beta[3]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3188, 0x0CE4); // ash_luma_params[2]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x318A, 0x0100); // ash_luma_params[2]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x318C, 0x0100); // ash_luma_params[2]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x318E, 0x0100); // ash_luma_params[2]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x3190, 0x0100); // ash_luma_params[2]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x3192, 0x0011); // ash_luma_params[2]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3194, 0x002F); // ash_luma_params[2]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3196, 0x0000); // ash_luma_params[2]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x3198, 0x0011); // ash_luma_params[2]_beta[3]  
	S5K3H7Y_wordwrite_cmos_sensor(0x319A, 0x1004); // ash_luma_params[3]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x319C, 0x0100); // ash_luma_params[3]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x319E, 0x0100); // ash_luma_params[3]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31A0, 0x0100); // ash_luma_params[3]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31A2, 0x0100); // ash_luma_params[3]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31A4, 0x0011); // ash_luma_params[3]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31A6, 0x002F); // ash_luma_params[3]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31A8, 0x0000); // ash_luma_params[3]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31AA, 0x0011); // ash_luma_params[3]_beta[3]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31AC, 0x1388); // ash_luma_params[4]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x31AE, 0x0100); // ash_luma_params[4]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31B0, 0x0100); // ash_luma_params[4]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31B2, 0x0100); // ash_luma_params[4]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31B4, 0x0100); // ash_luma_params[4]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31B6, 0x0011); // ash_luma_params[4]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31B8, 0x002F); // ash_luma_params[4]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31BA, 0x0000); // ash_luma_params[4]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31BC, 0x0011); // ash_luma_params[4]_beta[3]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31BE, 0x1964); // ash_luma_params[5]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x31C0, 0x0100); // ash_luma_params[5]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31C2, 0x0100); // ash_luma_params[5]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31C4, 0x0100); // ash_luma_params[5]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31C6, 0x0100); // ash_luma_params[5]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31C8, 0x0011); // ash_luma_params[5]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31CA, 0x002F); // ash_luma_params[5]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31CC, 0x0000); // ash_luma_params[5]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31CE, 0x0011); // ash_luma_params[5]_beta[3]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31D0, 0x1D4C); // ash_luma_params[6]_tmpr     
	S5K3H7Y_wordwrite_cmos_sensor(0x31D2, 0x0100); // ash_luma_params[6]_alpha[0] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31D4, 0x0100); // ash_luma_params[6]_alpha[1] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31D6, 0x0100); // ash_luma_params[6]_alpha[2] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31D8, 0x0100); // ash_luma_params[6]_alpha[3] 
	S5K3H7Y_wordwrite_cmos_sensor(0x31DA, 0x0011); // ash_luma_params[6]_beta[0]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31DC, 0x002F); // ash_luma_params[6]_beta[1]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31DE, 0x0000); // ash_luma_params[6]_beta[2]  
	S5K3H7Y_wordwrite_cmos_sensor(0x31E0, 0x0011); // ash_luma_params[6]_beta[3]  
	S5K3H7Y_bytewrite_cmos_sensor(0x3162, 0x00);  // ash_bLumaMode 01
	S5K3H7Y_wordwrite_cmos_sensor(0x301C, 0x0100); // smiaRegs_vendor_gras_nvm_address
	S5K3H7Y_bytewrite_cmos_sensor(0x301E, 0x03);  // smiaRegs_vendor_gras_load_from 03
	S5K3H7Y_bytewrite_cmos_sensor(0x323C, 0x00);  // ash_bSkipNvmGrasOfs 01 // skipping the value set in nvm page 0 address 47
	S5K3H7Y_bytewrite_cmos_sensor(0x323D, 0x01);  // ash_uNvmGrasTblOfs 01 // load shading table 1 from nvm
	S5K3H7Y_bytewrite_cmos_sensor(0x1989, 0x04);  //smiaRegs_ro_edof_cap_uAlphaTempInd 04
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00, 0x01);  // smiaRegs_rw_isp_shading_correction_enable 01
	
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x0001);	  // smiaRegs_rw_general_setup_mode_select

	mdelay(2);
    
    // Darling_otp_update();    // Jiangde--
    SENSORDB("HJDDbg3h7AWB, Init,  Darling_otp_set_AWB_gain() \n");
    Darling_otp_set_AWB_gain(); // Jiangde++
}

void S5K3H7YPreviewSetting(void)
{
	//1600x1200
        S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x00  ); // smiaRegs_rw_general_setup_mode_select
        S5K3H7Y_wordwrite_cmos_sensor(0x034C,0x0660);   // smiaRegs_rw_frame_timing_x_output_size 
        S5K3H7Y_wordwrite_cmos_sensor(0x034E,0x04C8);   // smiaRegs_rw_frame_timing_y_output_size 
        S5K3H7Y_wordwrite_cmos_sensor(0x0344,0x0004);   // smiaRegs_rw_frame_timing_x_addr_start
        S5K3H7Y_wordwrite_cmos_sensor(0x0346,0x0004);   // smiaRegs_rw_frame_timing_y_addr_start
        S5K3H7Y_wordwrite_cmos_sensor(0x0348,0x0CC3);   // smiaRegs_rw_frame_timing_x_addr_end
        S5K3H7Y_wordwrite_cmos_sensor(0x034A,0x0993);   // smiaRegs_rw_frame_timing_y_addr_end

        S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_PV_PERIOD_PIXEL_NUMS);     // smiaRegs_rw_frame_timing_line_length_pck
        S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_PV_PERIOD_LINE_NUMS);      // smiaRegs_rw_frame_timing_frame_length_lines
        S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);   // #smiaRegs_rw_sub_sample_x_even_inc
        S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0003);   // #smiaRegs_rw_sub_sample_x_odd_inc
        S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);    // #smiaRegs_rw_sub_sample_y_even_inc
        S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0003);    // #smiaRegs_rw_sub_sample_y_odd_inc
        S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0001);    // #smiaRegs_rw_binning_mode
        S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0022);    // #smiaRegs_rw_binning_type
        S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0001);    // #smiaRegs_rw_binning_weighting
		S5K3H7Y_wordwrite_cmos_sensor(0x0404,0x0010); // smiaRegs_rw_scaling_scale_m

        S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0618);     // smiaRegs_rw_integration_time_fine_integration_time
        S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09A5);     // smiaRegs_rw_integration_time_coarse_integration_time

        S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);   // X1
        S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x01  ); // #smiaRegs_rw_isp_mapped_couplet_correct_enable
        S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x01  ); // #smiaRegs_rw_isp_shading_correction_enable
 //       S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);     //raw 10 foramt
		//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);           //line start/end short packet
//        S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);        //0x03   //pixel order B Gb Gr R
        S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x01  ); // smiaRegs_rw_general_setup_mode_select
	/*
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,	0x00  );
	S5K3H7Y_bytewrite_cmos_sensor(0x0114,	0x03	);
	S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5);
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,	0x0E68);
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,	0x09E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618);
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,	0x09C2);
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020);
	S5K3H7Y_bytewrite_cmos_sensor(0x3011,	0x02	);
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,	0x01	);
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,	0x12	);
	S5K3H7Y_wordwrite_cmos_sensor(0x0346,	0x0004);
	S5K3H7Y_wordwrite_cmos_sensor(0x034A,	0x0993);
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,	0x0662);
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,	0x04C8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6004,	0x0000);
	S5K3H7Y_wordwrite_cmos_sensor(0x6028,	0xD000);
	S5K3H7Y_wordwrite_cmos_sensor(0x602A,	0x012A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x0040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7077);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7777);
	S5K3H7Y_wordwrite_cmos_sensor(0x0100,	0x0100);
	*/
        mdelay(2);
}

void S5K3H7YCaptureSetting(void)
{
	SENSORDB("enter\n");
	//Full 8M
	//#ifndef FullSize_preview
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x00  ); // smiaRegs_rw_general_setup_mode_select
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,0x0CC0);	// smiaRegs_rw_frame_timing_x_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,0x0990);	// smiaRegs_rw_frame_timing_y_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x0344,0x0004);	// smiaRegs_rw_frame_timing_x_addr_start
	S5K3H7Y_wordwrite_cmos_sensor(0x0346,0x0004);	// smiaRegs_rw_frame_timing_y_addr_start
	S5K3H7Y_wordwrite_cmos_sensor(0x0348,0x0CC3);	// smiaRegs_rw_frame_timing_x_addr_end
	S5K3H7Y_wordwrite_cmos_sensor(0x034A,0x0993);	// smiaRegs_rw_frame_timing_y_addr_end
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_FULL_PERIOD_PIXEL_NUMS);	// smiaRegs_rw_frame_timing_line_length_pck 
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_FULL_PERIOD_LINE_NUMS);	// smiaRegs_rw_frame_timing_frame_length_lines 
	S5K3H7Y_wordwrite_cmos_sensor(0x0404,0x0010); // smiaRegs_rw_scaling_scale_m
	S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);	// #smiaRegs_rw_sub_sample_x_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0001);	// #smiaRegs_rw_sub_sample_x_odd_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);	// #smiaRegs_rw_sub_sample_y_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0001);	// #smiaRegs_rw_sub_sample_y_odd_inc
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0000);	// #smiaRegs_rw_binning_mode
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0000);	// #smiaRegs_rw_binning_type
	S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0000);	// #smiaRegs_rw_binning_weighting

	S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0618);	// smiaRegs_rw_integration_time_fine_integration_time (fixed value)
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09A5);	// smiaRegs_rw_integration_time_coarse_integration_time (40ms)
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);	// X1
	S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x01  ); // #smiaRegs_rw_isp_mapped_couplet_correct_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x01 ); // #smiaRegs_rw_isp_shading_correction_enable
//	S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);	  //raw 10 foramt
	//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);	      //line start/end short packet
//	S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);	   //0x03   //pixel order B Gb Gr R
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x01  ); // smiaRegs_rw_general_setup_mode_select
/*	
S5K3H7Y_bytewrite_cmos_sensor(0x0100,	0x00  );
S5K3H7Y_bytewrite_cmos_sensor(0x0114,	0x03	);
S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5);
S5K3H7Y_wordwrite_cmos_sensor(0x0342,	0x0E68);
S5K3H7Y_wordwrite_cmos_sensor(0x0340,	0x09E2);
S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618);
S5K3H7Y_wordwrite_cmos_sensor(0x0202,	0x09C2);
S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020);
S5K3H7Y_wordwrite_cmos_sensor(0x0346,	0x0004);
S5K3H7Y_wordwrite_cmos_sensor(0x034A,	0x0993);
S5K3H7Y_wordwrite_cmos_sensor(0x034C,	0x0CC0);
S5K3H7Y_wordwrite_cmos_sensor(0x034E,	0x0990);
S5K3H7Y_wordwrite_cmos_sensor(0x0900,	0x0011);
S5K3H7Y_wordwrite_cmos_sensor(0x0901,	0x0011);
S5K3H7Y_wordwrite_cmos_sensor(0x3011,	0x0001);
S5K3H7Y_wordwrite_cmos_sensor(0x6004,	0x0000);
S5K3H7Y_wordwrite_cmos_sensor(0x6028,	0xD000);
S5K3H7Y_wordwrite_cmos_sensor(0x602A,	0x012A);
S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x0040);
S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7077);
S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7777);
S5K3H7Y_wordwrite_cmos_sensor(0x0100,	0x0100);
*/
	//#endif
	mdelay(2);
}

void S5K3H7YVideoSetting(void)
{
	SENSORDB("S5K3H7YVideoSetting enter");

	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x00  ); // smiaRegs_rw_general_setup_mode_select
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,S5K3H7Y_IMAGE_SENSOR_VIDEO_WIDTH_SETTING);	// smiaRegs_rw_frame_timing_x_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,S5K3H7Y_IMAGE_SENSOR_VIDEO_HEIGHT_SETTING);	// smiaRegs_rw_frame_timing_y_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x0344,0x0004);	// smiaRegs_rw_frame_timing_x_addr_start
	S5K3H7Y_wordwrite_cmos_sensor(0x0346,0x0136);	// smiaRegs_rw_frame_timing_y_addr_start
	S5K3H7Y_wordwrite_cmos_sensor(0x0348,S5K3H7Y_IMAGE_SENSOR_VIDEO_WIDTH_SETTING+3);	// smiaRegs_rw_frame_timing_x_addr_end
	S5K3H7Y_wordwrite_cmos_sensor(0x034A,S5K3H7Y_IMAGE_SENSOR_VIDEO_HEIGHT_SETTING+0x135);	// smiaRegs_rw_frame_timing_y_addr_end
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_VIDEO_PERIOD_PIXEL_NUMS);	// smiaRegs_rw_frame_timing_line_length_pck 
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_VIDEO_PERIOD_LINE_NUMS);	// smiaRegs_rw_frame_timing_frame_length_lines 
	SENSORDB("[0x0342,%d],[0x0340,%d]\n",S5K3H7Y_read_cmos_sensor(0x0342),S5K3H7Y_read_cmos_sensor(0x0340));
	S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);	// #smiaRegs_rw_sub_sample_x_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0001);	// #smiaRegs_rw_sub_sample_x_odd_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);	// #smiaRegs_rw_sub_sample_y_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0001);	// #smiaRegs_rw_sub_sample_y_odd_inc
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0000);	// #smiaRegs_rw_binning_mode
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0000);	// #smiaRegs_rw_binning_type
	S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0000);	// #smiaRegs_rw_binning_weighting

	S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0618);	// smiaRegs_rw_integration_time_fine_integration_time (fixed value)
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09A5);	// smiaRegs_rw_integration_time_coarse_integration_time (40ms)
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);	// X1
	S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x01  ); // #smiaRegs_rw_isp_mapped_couplet_correct_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x01  ); // #smiaRegs_rw_isp_shading_correction_enable
//	S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);	  //raw 10 foramt
	//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);	      //line start/end short packet
//	S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);	   //0x03   //pixel order B Gb Gr R
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x01  ); // smiaRegs_rw_general_setup_mode_select
    SENSORDB("S5K3H7YInitvideoSetting end");
	
	/*												 
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,	0x00 ) ;    
	S5K3H7Y_bytewrite_cmos_sensor(0x0114,	0x03	);	 
	S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,	0x0E68);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,	0x09E2);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,	0x09C2);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0346,	0x0136);    
	S5K3H7Y_wordwrite_cmos_sensor(0x034A,	0x0861);    
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,	0x0CC0);    
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,	0x072C);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0900,	0x0011);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0901,	0x0011);    
	S5K3H7Y_wordwrite_cmos_sensor(0x3011,	0x0001);    
	S5K3H7Y_wordwrite_cmos_sensor(0x6004,	0x0000);    
	S5K3H7Y_wordwrite_cmos_sensor(0x6028,	0xD000);    
	S5K3H7Y_wordwrite_cmos_sensor(0x602A,	0x012A);    
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x0040);    
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7077);    
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7777);    
	S5K3H7Y_wordwrite_cmos_sensor(0x0100,	0x0100);    
	*/
	
	mdelay(2);
}

   /*  S5K3H7YInitSetting  */

/*************************************************************************
* FUNCTION
*   S5K3H7YOpen
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

UINT32 S5K3H7YOpen(void)
{

	volatile signed int i,j;
	kal_uint16 sensor_id = 0;

	SENSORDB("enter\n");

	//  Read sensor ID to adjust I2C is OK?
	for(j=0;j<2;j++)
	{
		SENSORDB("Read sensor ID=0x%x\n",sensor_id);
		if(S5K3H7Y_SENSOR_ID == sensor_id)
		{
			break;
		}

		switch(j) {
			case 0:
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID;
				break;
			case 1:
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID2;
				break;
			default:
				break;
		}
        
        SENSORDB("s5k3h7y_slave_addr =0x%x\n",s5k3h7y_slave_addr);
		for(i=3;i>0;i--)
		{
			sensor_id = S5K3H7Y_read_cmos_sensor(0x0000);
			SENSORDB("Read sensor ID=0x%x\n",sensor_id);
			if(S5K3H7Y_SENSOR_ID == sensor_id)
			{
				break;
			}
		}
	}
    
	if(S5K3H7Y_SENSOR_ID != sensor_id)
	{
		return ERROR_SENSOR_CONNECT_FAIL;
	}

    SENSORDB("HJDDbg3h7ID, Open, is_Darling3h7() \n");
    if (!is_Darling3h7())
    {        
        SENSORDB("HJDDbg3h7ID, Open, It is darling 3h7! ");
        return ERROR_SENSOR_CONNECT_FAIL;
    }   

	S5K3H7YInitSetting();
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   S5K3H7YGetSensorID
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
UINT32 S5K3H7YGetSensorID(UINT32 *sensorID)
{
    //int  retry = 2;
	int i=0,j =0;

	SENSORDB("enter\n");
	for(j=0;j<2;j++)
	{
		SENSORDB("Read sensor ID=0x%x\n",*sensorID);
		if(S5K3H7Y_SENSOR_ID == *sensorID)
		{
			break;
		}

		switch(j) {
			case 0:
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID;
				break;
			case 1:
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID2;
				break;
			default:
				break;
		}

        SENSORDB("s5k3h7y_slave_addr =0x%x\n",s5k3h7y_slave_addr);
		for(i=3;i>0;i--)
		{
			S5K3H7Y_wordwrite_cmos_sensor(0x6010,0x0001);	// Reset
	    	mDELAY(1);
			*sensorID = S5K3H7Y_read_cmos_sensor(0x0000);
			SENSORDB("HJDDbg3h7ID, Darling Read sensor ID=0x%x\n", *sensorID);
			if(S5K3H7Y_SENSOR_ID == *sensorID)
			{
				break;
			}
		}
	}

	if (*sensorID != S5K3H7Y_SENSOR_ID)
    {
        SENSORDB("HJDDbg3h7ID, No 3h7, Read sensor ID=0x%x\n", *sensorID);
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    SENSORDB("HJDDbg3h7ID, getSensoriD, is_Darling3h7() \n");
    if (!is_Darling3h7())
    {        
        SENSORDB("HJDDbg3h7ID, getSensoriD, No Darling 3h7! ");
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }    

	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.sensorMode = SENSOR_MODE_INIT;
	s5k3h7y.S5K3H7YAutoFlickerMode = KAL_FALSE;
	s5k3h7y.S5K3H7YVideoMode = KAL_FALSE;
	s5k3h7y.DummyLines= 0;
	s5k3h7y.DummyPixels= 0;
	s5k3h7y.pvPclk = SENSOR_PCLK_PREVIEW; //260MHz
	s5k3h7y.m_vidPclk= SENSOR_PCLK_VIDEO;
	s5k3h7y.capPclk= SENSOR_PCLK_CAPTURE;
	s5k3h7y.shutter = 0x4EA;
	s5k3h7y.pvShutter = 0x4EA;
	s5k3h7y.maxExposureLines = S5K3H7Y_PV_PERIOD_LINE_NUMS;
	s5k3h7y.FixedFrameLength = S5K3H7Y_PV_PERIOD_LINE_NUMS;
	s5k3h7y.sensorGain = 0x1f;//sensor gain read from 0x350a 0x350b; 0x1f as 3.875x
	s5k3h7y.pvGain = 0x1f*3; //SL for brighter to SMT load
	s5k3h7y.imgMirror = IMAGE_NORMAL;
	s_S5K3H7YCurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*   S5K3H7Y_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of S5K3H7Y to change exposure time.
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
void S5K3H7Y_SetShutter(kal_uint32 iShutter)
{
   S5K3H7Y_write_shutter(iShutter);

}   /*  S5K3H7Y_SetShutter   */



/*************************************************************************
* FUNCTION
*   S5K3H7Y_read_shutter
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
UINT32 S5K3H7Y_read_shutter(void)
{
	return S5K3H7Y_read_cmos_sensor(0x0202);   // smiaRegs_rw_integration_time_coarse_integration_time

}

/*************************************************************************
* FUNCTION
*   S5K3H7Y_night_mode
*
* DESCRIPTION
*   This function night mode of S5K3H7Y.
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
void S5K3H7Y_NightMode(kal_bool bEnable)
{
}/*	S5K3H7Y_NightMode */



/*************************************************************************
* FUNCTION
*   S5K3H7YClose
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
UINT32 S5K3H7YClose(void)
{
    //  CISModulePowerOn(FALSE);
    //s_porting
    //  DRV_I2CClose(S5K3H7YhDrvI2C);
    //e_porting
    return ERROR_NONE;
}	/* S5K3H7YClose() */

void S5K3H7YSetFlipMirror(kal_int32 imgMirror)
{
	SENSORDB("imgMirror=%d\n",imgMirror);
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.imgMirror = imgMirror; //(imgMirror+IMAGE_HV_MIRROR)%(IMAGE_HV_MIRROR+1);
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

    switch (imgMirror)
    {
        case IMAGE_H_MIRROR://IMAGE_NORMAL:  bit0 mirror,   bit1 flip.
			S5K3H7Y_bytewrite_cmos_sensor(0x0101,0x01  ); //morror
            break;
        case IMAGE_NORMAL://IMAGE_V_MIRROR:
			S5K3H7Y_bytewrite_cmos_sensor(0x0101,0x00  );
            break;
        case IMAGE_HV_MIRROR://IMAGE_H_MIRROR:
			S5K3H7Y_bytewrite_cmos_sensor(0x0101,0x03  );   //morror +flip
            break;
        case IMAGE_V_MIRROR://IMAGE_HV_MIRROR:
			S5K3H7Y_bytewrite_cmos_sensor(0x0101,0x02  ); //flip
            break;
    }
}


/*************************************************************************
* FUNCTION
*   S5K3H7YPreview
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
UINT32 S5K3H7YPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	SENSORDB("enter\n");

	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.sensorMode = SENSOR_MODE_PREVIEW; // Need set preview setting after capture mode
	//S5K3H7Y_FeatureControl_PERIOD_PixelNum=S5K3H7Y_PV_PERIOD_PIXEL_NUMS+ s5k3h7y.DummyPixels;
	//S5K3H7Y_FeatureControl_PERIOD_LineNum=S5K3H7Y_PV_PERIOD_LINE_NUMS+s5k3h7y.DummyLines;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	S5K3H7YPreviewSetting();

	//S5K3H7Y_write_shutter(s5k3h7y.shutter);
	//write_S5K3H7Y_gain(s5k3h7y.pvGain);

	//set mirror & flip
	S5K3H7YSetFlipMirror(IMAGE_NORMAL);

    return ERROR_NONE;
}	/* S5K3H7YPreview() */

UINT32 S5K3H7YVideo(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	SENSORDB("enter\n");

	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.sensorMode = SENSOR_MODE_VIDEO; // Need set preview setting after capture mode
	//S5K3H7Y_FeatureControl_PERIOD_PixelNum=S5K3H7Y_PV_PERIOD_PIXEL_NUMS+ s5k3h7y.DummyPixels;
	//S5K3H7Y_FeatureControl_PERIOD_LineNum=S5K3H7Y_PV_PERIOD_LINE_NUMS+s5k3h7y.DummyLines;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	S5K3H7YVideoSetting();

	//S5K3H7Y_write_shutter(s5k3h7y.shutter);
	//write_S5K3H7Y_gain(s5k3h7y.pvGain);

	//set mirror & flip
	S5K3H7YSetFlipMirror(IMAGE_NORMAL);

    return ERROR_NONE;
}	/* S5K3H7YPreview() */


UINT32 S5K3H7YZSDPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	SENSORDB("enter\n");
	// Full size setting
	S5K3H7YCaptureSetting();

	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.sensorMode = SENSOR_MODE_ZSD_PREVIEW;
	//S5K3H7Y_FeatureControl_PERIOD_PixelNum = S5K3H7Y_FULL_PERIOD_PIXEL_NUMS + s5k3h7y.DummyPixels;
	//S5K3H7Y_FeatureControl_PERIOD_LineNum = S5K3H7Y_FULL_PERIOD_LINE_NUMS + s5k3h7y.DummyLines;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	S5K3H7YSetFlipMirror(IMAGE_NORMAL);

    return ERROR_NONE;
}

UINT32 S5K3H7YCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	SENSORDB("sensorMode=%d\n",s5k3h7y.sensorMode);

	// Full size setting
	#ifdef CAPTURE_USE_VIDEO_SETTING
	S5K3H7YVideoSetting();
	#else
	S5K3H7YCaptureSetting();
	#endif

	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.sensorMode = SENSOR_MODE_CAPTURE;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	S5K3H7YSetFlipMirror(IMAGE_NORMAL);

    return ERROR_NONE;
}	/* S5K3H7YCapture() */

UINT32 S5K3H7YGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("enter\n");
	pSensorResolution->SensorPreviewWidth	= 	S5K3H7Y_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight	= 	S5K3H7Y_IMAGE_SENSOR_PV_HEIGHT;
	pSensorResolution->SensorVideoWidth	=	S5K3H7Y_IMAGE_SENSOR_VIDEO_WIDTH;
	pSensorResolution->SensorVideoHeight 	=	S5K3H7Y_IMAGE_SENSOR_VIDEO_HEIGHT;
	pSensorResolution->SensorFullWidth		= 	S5K3H7Y_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight	= 	S5K3H7Y_IMAGE_SENSOR_FULL_HEIGHT;
	//SENSORDB("Video width/height: %d/%d",pSensorResolution->SensorVideoWidth,pSensorResolution->SensorVideoHeight);
    return ERROR_NONE;
}   /* S5K3H7YGetResolution() */

UINT32 S5K3H7YGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch(s_S5K3H7YCurrentScenarioId)
	{
    	case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorPreviewResolutionX= S5K3H7Y_IMAGE_SENSOR_FULL_WIDTH;
			pSensorInfo->SensorPreviewResolutionY= S5K3H7Y_IMAGE_SENSOR_FULL_HEIGHT;
			break;
		default:
			pSensorInfo->SensorPreviewResolutionX= S5K3H7Y_IMAGE_SENSOR_PV_WIDTH;
			pSensorInfo->SensorPreviewResolutionY= S5K3H7Y_IMAGE_SENSOR_PV_HEIGHT;
			break;
	}

	pSensorInfo->SensorFullResolutionX= S5K3H7Y_IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY= S5K3H7Y_IMAGE_SENSOR_FULL_HEIGHT;

	SENSORDB("SensorImageMirror=%d\n", pSensorConfigData->SensorImageMirror);

	switch(s5k3h7y.imgMirror)
	{
		case IMAGE_NORMAL:
   			pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_B;
		break;
		case IMAGE_H_MIRROR:
   			pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_Gr;
		break;
		case IMAGE_V_MIRROR:
   			pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_Gb;   			
		break;
		case IMAGE_HV_MIRROR:
   			pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_R;
		break;
		default:
			pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_B;
	}
    pSensorInfo->SensorClockPolarity =SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;

    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;

    pSensorInfo->CaptureDelayFrame = 3;
    pSensorInfo->PreviewDelayFrame = 3;
    pSensorInfo->VideoDelayFrame = 2;

    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
    pSensorInfo->AEShutDelayFrame = 0;//0;		    /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0 ;//0;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;

	pSensorInfo->SensorClockFreq=24;  //26
	pSensorInfo->SensorClockRisingCount= 0;
	#ifdef USE_MIPI_2_LANES
	pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
	#else
	pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_4_LANE;
	#endif
	pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 0x20;
	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->SensorPacketECCOrder = 1;

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pSensorInfo->SensorGrabStartX = S5K3H7Y_PV_X_START;
            pSensorInfo->SensorGrabStartY = S5K3H7Y_PV_Y_START;
		break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pSensorInfo->SensorGrabStartX = S5K3H7Y_VIDEO_X_START;
			pSensorInfo->SensorGrabStartY = S5K3H7Y_VIDEO_Y_START;
        break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorGrabStartX = S5K3H7Y_FULL_X_START;	//2*S5K3H7Y_IMAGE_SENSOR_PV_STARTX;
            pSensorInfo->SensorGrabStartY = S5K3H7Y_FULL_Y_START;	//2*S5K3H7Y_IMAGE_SENSOR_PV_STARTY;
        break;
        default:
            pSensorInfo->SensorGrabStartX = S5K3H7Y_PV_X_START;
            pSensorInfo->SensorGrabStartY = S5K3H7Y_PV_Y_START;
            break;
    }

    memcpy(pSensorConfigData, &S5K3H7YSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* S5K3H7YGetInfo() */


UINT32 S5K3H7YControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s_S5K3H7YCurrentScenarioId = ScenarioId;
	s5k3h7y.FixedFrameLength = GetScenarioFramelength();
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	SENSORDB("s_S5K3H7YCurrentScenarioId=%d\n",s_S5K3H7YCurrentScenarioId);

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            S5K3H7YPreview(pImageWindow, pSensorConfigData);
        break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			S5K3H7YVideo(pImageWindow, pSensorConfigData);
		break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			S5K3H7YCapture(pImageWindow, pSensorConfigData);
        break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			S5K3H7YZSDPreview(pImageWindow, pSensorConfigData);
		break;
        default:
            return ERROR_INVALID_SCENARIO_ID;

    }
    return ERROR_NONE;
} /* S5K3H7YControl() */


UINT32 S5K3H7YSetVideoMode(UINT16 u2FrameRate)
{

    //kal_uint32 MIN_Frame_length =0,frameRate=0,extralines=0;

	s5k3h7y.sensorMode=MSDK_SCENARIO_ID_VIDEO_PREVIEW;
    SENSORDB("u2FrameRate=%d,sensorMode=%d\n", u2FrameRate,s5k3h7y.sensorMode);

	//if(u2FrameRate >30 || u2FrameRate <5)
	if(0==u2FrameRate) //do not fix frame rate 
	{
		spin_lock(&s5k3h7ymipiraw_drv_lock);
		s5k3h7y.FixedFrameLength = GetScenarioFramelength();
		spin_unlock(&s5k3h7ymipiraw_drv_lock);
		SENSORDB("s5k3h7y.FixedFrameLength=%d\n",s5k3h7y.FixedFrameLength);
	    return ERROR_NONE;
	}
	S5K3H7YMIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_VIDEO_PREVIEW,u2FrameRate*10);
	return ERROR_NONE;
}

static void S5K3H7YSetMaxFrameRate(UINT16 u2FrameRate)
{
	kal_uint16 FrameHeight;
		
	SENSORDB("[S5K4H5YX] [S5K4H5YXMIPISetMaxFrameRate] u2FrameRate=%d\n",u2FrameRate);

	if(SENSOR_MODE_PREVIEW == s5k3h7y.sensorMode)
	{
		FrameHeight= (10 * s5k3h7y.pvPclk) / u2FrameRate / S5K3H7Y_PV_PERIOD_PIXEL_NUMS;
		FrameHeight = (FrameHeight > S5K3H7Y_PV_PERIOD_LINE_NUMS) ? FrameHeight : S5K3H7Y_PV_PERIOD_LINE_NUMS;
	}
	else if(SENSOR_MODE_CAPTURE== s5k3h7y.sensorMode || SENSOR_MODE_ZSD_PREVIEW == s5k3h7y.sensorMode)
	{
		FrameHeight= (10 * s5k3h7y.capPclk) / u2FrameRate / S5K3H7Y_FULL_PERIOD_PIXEL_NUMS;
		FrameHeight = (FrameHeight > S5K3H7Y_FULL_PERIOD_LINE_NUMS) ? FrameHeight : S5K3H7Y_FULL_PERIOD_LINE_NUMS;
	}
	else
	{
		FrameHeight = (10 * s5k3h7y.m_vidPclk) / u2FrameRate / S5K3H7Y_VIDEO_PERIOD_PIXEL_NUMS;
		FrameHeight = (FrameHeight > S5K3H7Y_VIDEO_PERIOD_LINE_NUMS) ? FrameHeight : S5K3H7Y_VIDEO_PERIOD_LINE_NUMS;
	}
	SENSORDB("[S5K4H5YX] [S5K4H5YXMIPISetMaxFrameRate] FrameHeight=%d",FrameHeight);
	SetFramelength(FrameHeight); /* modify dummy_pixel must gen AE table again */	
}


UINT32 S5K3H7YSetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	if(bEnable) 
	{
		SENSORDB("[S5K4H5YX] [S5K4H5YXSetAutoFlickerMode] enable\n");
		spin_lock(&s5k3h7ymipiraw_drv_lock);
		s5k3h7y.S5K3H7YAutoFlickerMode = KAL_TRUE;
		spin_unlock(&s5k3h7ymipiraw_drv_lock);

		if(u2FrameRate == 300)
			S5K3H7YSetMaxFrameRate(296);
		else if(u2FrameRate == 150)
			S5K3H7YSetMaxFrameRate(148);
    } 
	else 
	{
    	SENSORDB("[S5K4H5YX] [S5K4H5YXSetAutoFlickerMode] disable\n");
    	spin_lock(&s5k3h7ymipiraw_drv_lock);
        s5k3h7y.S5K3H7YAutoFlickerMode = KAL_FALSE;
		spin_unlock(&s5k3h7ymipiraw_drv_lock);
    }
    return ERROR_NONE;
}


UINT32 S5K3H7YSetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("bEnable=%d\n", bEnable);
    if(bEnable)
    {
        S5K3H7Y_wordwrite_cmos_sensor(0x0600,0x0002);

    }
    else
    {
        S5K3H7Y_wordwrite_cmos_sensor(0x0600,0x0000);
    }
    return ERROR_NONE;
}

UINT32 S5K3H7YMIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate)
{
	//kal_uint32 pclk;
	//kal_uint16 u2dummyLine;
	kal_uint16 frameLength=0;//lineLength,frameLength;

	SENSORDB("scenarioId=%d,frameRate=%d\n",scenarioId,frameRate);
	switch (scenarioId)
	{
		//SetDummy() has to switch scenarioId again, so we do not use it here
		//when SetDummy() is ok, we'll switch to using SetDummy()
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frameLength = (s5k3h7y.pvPclk)/frameRate*10/S5K3H7Y_PV_PERIOD_PIXEL_NUMS;
			frameLength = (frameLength>S5K3H7Y_PV_PERIOD_LINE_NUMS)?(frameLength):(S5K3H7Y_PV_PERIOD_LINE_NUMS);
		break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			frameLength = (s5k3h7y.m_vidPclk)/frameRate*10/S5K3H7Y_VIDEO_PERIOD_PIXEL_NUMS;
			frameLength = (frameLength>S5K3H7Y_VIDEO_PERIOD_LINE_NUMS)?(frameLength):(S5K3H7Y_VIDEO_PERIOD_LINE_NUMS);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			frameLength = (s5k3h7y.m_vidPclk)/frameRate*10/S5K3H7Y_FULL_PERIOD_PIXEL_NUMS;
			frameLength = (frameLength>S5K3H7Y_FULL_PERIOD_LINE_NUMS)?(frameLength):(S5K3H7Y_FULL_PERIOD_LINE_NUMS);
		break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			frameLength = (s5k3h7y.m_vidPclk)/frameRate*10/S5K3H7Y_ZSD_PERIOD_PIXEL_NUMS;
			frameLength = (frameLength>S5K3H7Y_ZSD_PERIOD_LINE_NUMS)?(frameLength):(S5K3H7Y_ZSD_PERIOD_LINE_NUMS);
		break;
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
		break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added
		break;
		default:
			frameLength = S5K3H7Y_PV_PERIOD_LINE_NUMS;
		break;
	}
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.FixedFrameLength = frameLength;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);

	SetFramelength(frameLength); //direct set frameLength
	return ERROR_NONE;
}


UINT32 S5K3H7YMIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate)
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
		#ifdef FULL_SIZE_30_FPS
			 *pframeRate = 300;
		#else
			*pframeRate = 240;
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

UINT32 S5K3H7YMIPIGetTemperature(UINT32 *temperature)
{

	*temperature = 0;//read register
    return ERROR_NONE;
}



UINT32 S5K3H7YFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
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

	SENSORDB("FeatureId=%d\n",FeatureId);
    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++= S5K3H7Y_IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16= S5K3H7Y_IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
				*pFeatureReturnPara16++= GetScenarioLinelength();
				*pFeatureReturnPara16= GetScenarioFramelength();
				*pFeatureParaLen=4;
				break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			//same pclk for preview/capture
    	 	*pFeatureReturnPara32 = s5k3h7y.pvPclk;
			SENSORDB("sensor clock=%d\n",*pFeatureReturnPara32);
    	 	*pFeatureParaLen=4;
 			 break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            S5K3H7Y_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            S5K3H7Y_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            S5K3H7Y_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            //S5K3H7Y_isp_master_clock=*pFeatureData32;
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            S5K3H7Y_wordwrite_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = S5K3H7Y_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&s5k3h7ymipiraw_drv_lock);
                S5K3H7YSensorCCT[i].Addr=*pFeatureData32++;
                S5K3H7YSensorCCT[i].Para=*pFeatureData32++;
				spin_unlock(&s5k3h7ymipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return ERROR_INVALID_PARA;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=S5K3H7YSensorCCT[i].Addr;
                *pFeatureData32++=S5K3H7YSensorCCT[i].Para;
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&s5k3h7ymipiraw_drv_lock);
                S5K3H7YSensorReg[i].Addr=*pFeatureData32++;
                S5K3H7YSensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&s5k3h7ymipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return ERROR_INVALID_PARA;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=S5K3H7YSensorReg[i].Addr;
                *pFeatureData32++=S5K3H7YSensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=S5K3H7Y_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, S5K3H7YSensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, S5K3H7YSensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return ERROR_INVALID_PARA;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &S5K3H7YSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            S5K3H7Y_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            S5K3H7Y_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=S5K3H7Y_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            S5K3H7Y_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            S5K3H7Y_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            S5K3H7Y_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
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
            S5K3H7YSetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            S5K3H7YGetSensorID(pFeatureReturnPara32);
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            S5K3H7YSetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));
	        break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            S5K3H7YSetTestPatternMode((BOOL)*pFeatureData16);
            break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			S5K3H7YMIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			S5K3H7YMIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE://for factory mode auto testing
            *pFeatureReturnPara32= S5K3H7_TEST_PATTERN_CHECKSUM;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_GET_SENSOR_CURRENT_TEMPERATURE:
			S5K3H7YMIPIGetTemperature(pFeatureReturnPara32);
			*pFeatureParaLen=4;
			break;
        default:
            break;
    }
    return ERROR_NONE;
}	/* S5K3H7YFeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncS5K3H7Y=
{
    S5K3H7YOpen,
    S5K3H7YGetInfo,
    S5K3H7YGetResolution,
    S5K3H7YFeatureControl,
    S5K3H7YControl,
    S5K3H7YClose
};

UINT32 S5K3H7Y_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncS5K3H7Y;

    return ERROR_NONE;
}   /* SensorInit() */


