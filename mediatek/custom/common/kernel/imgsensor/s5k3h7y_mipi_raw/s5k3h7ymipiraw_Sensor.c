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

#define S5K3H7_TEST_PATTERN_CHECKSUM (0xdc0c1113)

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
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,2,s5k3h7y_slave_addr);
	return ((get_byte<<8)&0xff00)|((get_byte>>8)&0x00ff);
}

inline void S5K3H7Y_wordwrite_cmos_sensor(u16 addr, u32 para)
{
	char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,  (char)(para >> 8),	(char)(para & 0xFF) };
	iWriteRegI2C(puSendCmd , 4,s5k3h7y_slave_addr);
}

inline void S5K3H7Y_bytewrite_cmos_sensor(u16 addr, u32 para)
{
	char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF)  ,	(char)(para & 0xFF) };
	iWriteRegI2C(puSendCmd , 3,s5k3h7y_slave_addr);
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

static inline void SetFramelength(kal_uint16 u2Framelength)
{
	SENSORDB("u2Framelength=%d\n",u2Framelength);
	
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	s5k3h7y.maxExposureLines = u2Framelength;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x01);	 //Grouped parameter hold	 
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,u2Framelength);		
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00);	 //Grouped parameter release	
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	S5K3H7Y_FRAME_LENGTH_COUNT = u2Framelength;
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
	
	//u2Gain=S5K3H7Y_read_cmos_sensor(0x0204);
	//SENSORDB("read gain=%d\n",u2Gain);
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

/*
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
}  
*/

static void S5K3H7YInitSetting(void)
{
	SENSORDB("enter\n");
  //1600x1200	
  	#if 0
	S5K3H7Y_wordwrite_cmos_sensor(0x6010,0x0001);	// Reset		
	Sleep(10);//; delay(10ms)
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,0xF1D0);	// open all clocks
	S5K3H7Y_wordwrite_cmos_sensor(0x6214,0xF9F0);	// open all clocks
	S5K3H7Y_wordwrite_cmos_sensor(0xF400,0x0BBC); // workaround for the SW standby current
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,0x0001);	// open APB clock for I2C transaction
	S5K3H7Y_wordwrite_cmos_sensor(0xB0C0,0x000C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,0x0000);	// close APB clock for I2C transaction
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,0xF9F0);	// close all clocks
	S5K3H7Y_wordwrite_cmos_sensor(0x38FA,0x0030);  // gisp_offs_gains_bls_offs_0_
	S5K3H7Y_wordwrite_cmos_sensor(0x38FC,0x0030);  // gisp_offs_gains_bls_offs_1_
	S5K3H7Y_wordwrite_cmos_sensor(0x32CE,0x0060);	// senHal_usWidthStOfsInit		   
	S5K3H7Y_wordwrite_cmos_sensor(0x32D0,0x0024);	// senHal_usHeightStOfsInit 
	#ifdef USE_MIPI_2_LANES
	S5K3H7Y_wordwrite_cmos_sensor(0x0114,0x01);	// #smiaRegs_rw_output_lane_mode
	#endif
	S5K3H7Y_wordwrite_cmos_sensor(0x0086,0x01FF);	// analogue_gain_code_max
	S5K3H7Y_wordwrite_cmos_sensor(0x0136,0x1800);	// #smiaRegs_rw_op_cond_extclk_frequency_mhz
	S5K3H7Y_wordwrite_cmos_sensor(0x0300,0x0002);	// smiaRegs_rw_clocks_vt_pix_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x0302,0x0001);	// smiaRegs_rw_clocks_vt_sys_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x0304,0x0006);	// smiaRegs_rw_clocks_pre_pll_clk_div
    S5K3H7Y_wordwrite_cmos_sensor(0x0306,0x008C);    // smiaRegs_rw_clocks_pll_multiplier  
	S5K3H7Y_wordwrite_cmos_sensor(0x0308,0x0008);	// smiaRegs_rw_clocks_op_pix_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x030A,0x0001);	// smiaRegs_rw_clocks_op_sys_clk_div
	S5K3H7Y_wordwrite_cmos_sensor(0x030C,0x0006);	// smiaRegs_rw_clocks_secnd_pre_pll_clk_div
   	S5K3H7Y_wordwrite_cmos_sensor(0x030E,0x00A2);    // smiaRegs_rw_clocks_secnd_pll_multiplier
    S5K3H7Y_wordwrite_cmos_sensor(0x311C,0x0BB8);   //Increase Blank time on account of process time
    S5K3H7Y_wordwrite_cmos_sensor(0x311E,0x0BB8);   //Increase Blank time on account of process time
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,0x0660);	// smiaRegs_rw_frame_timing_x_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,0x04C8);	// smiaRegs_rw_frame_timing_y_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);	// #smiaRegs_rw_sub_sample_x_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0003);	// #smiaRegs_rw_sub_sample_x_odd_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);	// #smiaRegs_rw_sub_sample_y_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0003);	// #smiaRegs_rw_sub_sample_y_odd_inc

	S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);	  //raw 10 foramt
	//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);             //line start/end short packet
	S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);	      //pixel order B Gb Gr R

	S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0001);	// #smiaRegs_rw_binning_mode
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0022);	// #smiaRegs_rw_binning_type
	S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0001);	// #smiaRegs_rw_binning_weighting
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_PV_PERIOD_PIXEL_NUMS);	// smiaRegs_rw_frame_timing_line_length_pck
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_PV_PERIOD_LINE_NUMS);	// smiaRegs_rw_frame_timing_frame_length_lines
	S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0BEF);	  // smiaRegs_rw_integration_time_fine_integration_time
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09D9);	  // smiaRegs_rw_integration_time_coarse_integration_time

	S5K3H7Y_bytewrite_cmos_sensor(0x37F8,0x0001);	  // Analog Gain Precision, 0/1/2/3 = 32/64/128/256 base 1X, set 1=> 64 =1X
	s5k3h7y.sensorBaseGain=64;

	S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);	// X1
	S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x0001);	  // #smiaRegs_rw_isp_mapped_couplet_correct_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x0000);	  // #smiaRegs_rw_isp_shading_correction_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x0001);	  // smiaRegs_rw_general_setup_mode_select
	#if 0  // test pattern = Color Checker
    S5K3H7Y_wordwrite_cmos_sensor(0x0600,0x0100);	
	#endif
	#else if
	S5K3H7Y_wordwrite_cmos_sensor(0x6010,0x0001);	// Reset
	Sleep(1);//; delay(10ms)
	S5K3H7Y_wordwrite_cmos_sensor(0x38FA,	0x0030);	
	S5K3H7Y_wordwrite_cmos_sensor(0x38FC,	0x0030);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0086,	0x01FF);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,	0xF1D0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6214,	0xF9F0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,	0x0001);	
	S5K3H7Y_wordwrite_cmos_sensor(0xB0C0,	0x000C);
	S5K3H7Y_wordwrite_cmos_sensor(0xF400,	0x0BBC);	
	S5K3H7Y_wordwrite_cmos_sensor(0xF616,	0x0004);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,	0x0000);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,	0xF9F0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,	0xF1D0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6214,	0xF9F0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,	0x0001);	
	S5K3H7Y_wordwrite_cmos_sensor(0xB0C0,	0x000C);
	S5K3H7Y_wordwrite_cmos_sensor(0x6226,	0x0000);	
	S5K3H7Y_wordwrite_cmos_sensor(0x6218,	0xF9F0);	
	S5K3H7Y_wordwrite_cmos_sensor(0x38FA,	0x0030);	
	S5K3H7Y_wordwrite_cmos_sensor(0x38FC,	0x0030);	
	S5K3H7Y_wordwrite_cmos_sensor(0x32CE,	0x0060);	
	S5K3H7Y_wordwrite_cmos_sensor(0x32D0,	0x0024);	
	S5K3H7Y_bytewrite_cmos_sensor(0x0114,	0x03	);
	S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,	0x0E68);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,	0x09E2);
	S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,	0x09C2);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020);	
	S5K3H7Y_bytewrite_cmos_sensor(0x3011,	0x02	);
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,	0x01  );
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,	0x12	);
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,	0x0662);
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,	0x04C8);
	S5K3H7Y_wordwrite_cmos_sensor(0x6028,	0xD000);
	S5K3H7Y_wordwrite_cmos_sensor(0x602A,	0x012A);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x0040);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7077);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7777);	
	S5K3H7Y_wordwrite_cmos_sensor(0x0100,	0x0100);	
	#endif
	mdelay(2);
}

void S5K3H7YPreviewSetting(void)
{
	//1600x1200
	#if 0
		SENSORDB("preview stting enter\n");
        S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x00  ); // smiaRegs_rw_general_setup_mode_select
        S5K3H7Y_wordwrite_cmos_sensor(0x034C,0x0660);   // smiaRegs_rw_frame_timing_x_output_size 
        S5K3H7Y_wordwrite_cmos_sensor(0x034E,0x04C8);   // smiaRegs_rw_frame_timing_y_output_size 
        S5K3H7Y_wordwrite_cmos_sensor(0x0344,0x0004);   // smiaRegs_rw_frame_timing_x_addr_start
        S5K3H7Y_wordwrite_cmos_sensor(0x0346,0x0004);   // smiaRegs_rw_frame_timing_y_addr_start
        S5K3H7Y_wordwrite_cmos_sensor(0x0348,0x0CC3);   // smiaRegs_rw_frame_timing_x_addr_end
        S5K3H7Y_wordwrite_cmos_sensor(0x034A,0x0993);   // smiaRegs_rw_frame_timing_y_addr_end

        S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_PV_PERIOD_PIXEL_NUMS);     // smiaRegs_rw_frame_timing_line_length_pck
        S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_PV_PERIOD_LINE_NUMS);      // smiaRegs_rw_frame_timing_frame_length_lines
        SENSORDB("[0x0342,%d],[0x0340,%d]\n",S5K3H7Y_read_cmos_sensor(0x0342),S5K3H7Y_read_cmos_sensor(0x0340));
        S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);   // #smiaRegs_rw_sub_sample_x_even_inc
        S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0003);   // #smiaRegs_rw_sub_sample_x_odd_inc
        S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);    // #smiaRegs_rw_sub_sample_y_even_inc
        S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0003);    // #smiaRegs_rw_sub_sample_y_odd_inc
        S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0001);    // #smiaRegs_rw_binning_mode
        S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0022);    // #smiaRegs_rw_binning_type
        S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0001);    // #smiaRegs_rw_binning_weighting

        S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0BEF);     // smiaRegs_rw_integration_time_fine_integration_time
        S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09D9);     // smiaRegs_rw_integration_time_coarse_integration_time

        S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);   // X1
        S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x01  ); // #smiaRegs_rw_isp_mapped_couplet_correct_enable
        S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x00  ); // #smiaRegs_rw_isp_shading_correction_enable
        S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);     //raw 10 foramt
		//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);           //line start/end short packet
        S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);        //0x03   //pixel order B Gb Gr R
        S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x01  ); // smiaRegs_rw_general_setup_mode_select
	#else if
			SENSORDB("preview stting enter\n");
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
	S5K3H7Y_bytewrite_cmos_sensor(0x0105,	0x04 		);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104,	0x01    );
	S5K3H7Y_bytewrite_cmos_sensor(0x0114,	0x03	  );
	S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5	);
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,	0x0E68	);
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,	0x09E2	);
	S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618	);
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,	0x09C2	);
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020	);
	S5K3H7Y_bytewrite_cmos_sensor(0x3011,	0x02	  );
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,	0x01    );
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,	0x12    );
	S5K3H7Y_wordwrite_cmos_sensor(0x0346,	0x0004	);
	S5K3H7Y_wordwrite_cmos_sensor(0x034A,	0x0993	);
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,	0x0662	);
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,	0x04C8	);
	S5K3H7Y_wordwrite_cmos_sensor(0x6004,	0x0000  );
	S5K3H7Y_wordwrite_cmos_sensor(0x6028,	0xD000  );
	S5K3H7Y_wordwrite_cmos_sensor(0x602A,	0x012A  );
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x0040	);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7077	);
	S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7777	);
	S5K3H7Y_bytewrite_cmos_sensor(0x0104,	0x00    );
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	S5K3H7Y_FRAME_LENGTH_COUNT = S5K3H7Y_PV_PERIOD_LINE_NUMS;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);
	#endif
        mdelay(2);
}

void S5K3H7YCaptureSetting(void)
{
	SENSORDB("enter\n");
	//Full 8M
	#if 0
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x00  ); // smiaRegs_rw_general_setup_mode_select
	S5K3H7Y_wordwrite_cmos_sensor(0x034C,0x0CC0);	// smiaRegs_rw_frame_timing_x_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x034E,0x0990);	// smiaRegs_rw_frame_timing_y_output_size
	S5K3H7Y_wordwrite_cmos_sensor(0x0344,0x0004);	// smiaRegs_rw_frame_timing_x_addr_start
	S5K3H7Y_wordwrite_cmos_sensor(0x0346,0x0004);	// smiaRegs_rw_frame_timing_y_addr_start
	S5K3H7Y_wordwrite_cmos_sensor(0x0348,0x0CC3);	// smiaRegs_rw_frame_timing_x_addr_end
	S5K3H7Y_wordwrite_cmos_sensor(0x034A,0x0993);	// smiaRegs_rw_frame_timing_y_addr_end
	S5K3H7Y_wordwrite_cmos_sensor(0x0342,S5K3H7Y_FULL_PERIOD_PIXEL_NUMS);	// smiaRegs_rw_frame_timing_line_length_pck 
	S5K3H7Y_wordwrite_cmos_sensor(0x0340,S5K3H7Y_FULL_PERIOD_LINE_NUMS);	// smiaRegs_rw_frame_timing_frame_length_lines 
	SENSORDB("[0x0342,%d],[0x0340,%d]\n",S5K3H7Y_read_cmos_sensor(0x0342),S5K3H7Y_read_cmos_sensor(0x0340));
	S5K3H7Y_wordwrite_cmos_sensor(0x0380,0x0001);	// #smiaRegs_rw_sub_sample_x_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0382,0x0001);	// #smiaRegs_rw_sub_sample_x_odd_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0384,0x0001);	// #smiaRegs_rw_sub_sample_y_even_inc
	S5K3H7Y_wordwrite_cmos_sensor(0x0386,0x0001);	// #smiaRegs_rw_sub_sample_y_odd_inc
	S5K3H7Y_bytewrite_cmos_sensor(0x0900,0x0000);	// #smiaRegs_rw_binning_mode
	S5K3H7Y_bytewrite_cmos_sensor(0x0901,0x0000);	// #smiaRegs_rw_binning_type
	S5K3H7Y_bytewrite_cmos_sensor(0x0902,0x0000);	// #smiaRegs_rw_binning_weighting

	S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0BEF);	// smiaRegs_rw_integration_time_fine_integration_time (fixed value)
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09D9);	// smiaRegs_rw_integration_time_coarse_integration_time (40ms)
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);	// X1
	S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x01  ); // #smiaRegs_rw_isp_mapped_couplet_correct_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x00  ); // #smiaRegs_rw_isp_shading_correction_enable
	S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);	  //raw 10 foramt
	//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);	      //line start/end short packet
	S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);	   //0x03   //pixel order B Gb Gr R
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x01  ); // smiaRegs_rw_general_setup_mode_select
	#else
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
S5K3H7Y_bytewrite_cmos_sensor(0x0105,	0x04   );
S5K3H7Y_bytewrite_cmos_sensor(0x0104,	0x01   );
S5K3H7Y_bytewrite_cmos_sensor(0x0114,	0x03	 );
S5K3H7Y_wordwrite_cmos_sensor(0x030E,	0x00A5 );
S5K3H7Y_wordwrite_cmos_sensor(0x0342,	0x0E68 );
S5K3H7Y_wordwrite_cmos_sensor(0x0340,	0x09E2 );
S5K3H7Y_wordwrite_cmos_sensor(0x0200,	0x0618 );
S5K3H7Y_wordwrite_cmos_sensor(0x0202,	0x09C2 );
S5K3H7Y_wordwrite_cmos_sensor(0x0204,	0x0020 );
S5K3H7Y_wordwrite_cmos_sensor(0x0346,	0x0004 );
S5K3H7Y_wordwrite_cmos_sensor(0x034A,	0x0993 );
S5K3H7Y_wordwrite_cmos_sensor(0x034C,	0x0CC0 );
S5K3H7Y_wordwrite_cmos_sensor(0x034E,	0x0990 );
S5K3H7Y_wordwrite_cmos_sensor(0x0900,	0x0011 );
S5K3H7Y_wordwrite_cmos_sensor(0x0901,	0x0011 );
S5K3H7Y_wordwrite_cmos_sensor(0x3011,	0x0001 );
S5K3H7Y_wordwrite_cmos_sensor(0x6004,	0x0000 );
S5K3H7Y_wordwrite_cmos_sensor(0x6028,	0xD000 );
S5K3H7Y_wordwrite_cmos_sensor(0x602A,	0x012A );
S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x0040 );
S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7077 );
S5K3H7Y_wordwrite_cmos_sensor(0x6F12,	0x7777 );
S5K3H7Y_bytewrite_cmos_sensor(0x0104,	0x00   );
spin_lock(&s5k3h7ymipiraw_drv_lock);
S5K3H7Y_FRAME_LENGTH_COUNT = S5K3H7Y_PV_PERIOD_LINE_NUMS;
spin_unlock(&s5k3h7ymipiraw_drv_lock);
	#endif
	mdelay(2);
}

void S5K3H7YVideoSetting(void)
{
	SENSORDB("enter\n");
	#if 0
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

	S5K3H7Y_wordwrite_cmos_sensor(0x0200,0x0BEF);	// smiaRegs_rw_integration_time_fine_integration_time (fixed value)
	S5K3H7Y_wordwrite_cmos_sensor(0x0202,0x09D9);	// smiaRegs_rw_integration_time_coarse_integration_time (40ms)
	S5K3H7Y_wordwrite_cmos_sensor(0x0204,0x0020);	// X1
	S5K3H7Y_bytewrite_cmos_sensor(0x0B05,0x01  ); // #smiaRegs_rw_isp_mapped_couplet_correct_enable
	S5K3H7Y_bytewrite_cmos_sensor(0x0B00,0x00  ); // #smiaRegs_rw_isp_shading_correction_enable
	S5K3H7Y_wordwrite_cmos_sensor(0x0112,0x0A0A);	  //raw 10 foramt
	//S5K3H7Y_bytewrite_cmos_sensor(0x3053,0x01);	      //line start/end short packet
	S5K3H7Y_bytewrite_cmos_sensor(0x300D,0x02);	   //0x03   //pixel order B Gb Gr R
	S5K3H7Y_bytewrite_cmos_sensor(0x0100,0x01  ); // smiaRegs_rw_general_setup_mode_select
	#else
	
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
	
	S5K3H7Y_bytewrite_cmos_sensor(0x0105,	0x04);	 
	S5K3H7Y_bytewrite_cmos_sensor(0x0104,	0x01);	 
	S5K3H7Y_bytewrite_cmos_sensor(0x0114,	  0x03);	 
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
	S5K3H7Y_bytewrite_cmos_sensor(0x0104, 0x00	);	 
	spin_lock(&s5k3h7ymipiraw_drv_lock);
	S5K3H7Y_FRAME_LENGTH_COUNT = S5K3H7Y_PV_PERIOD_LINE_NUMS;
	spin_unlock(&s5k3h7ymipiraw_drv_lock);
	#endif
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
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID2;
				break;
			case 1:
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID;
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
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID2;
				break;
			case 1:
				s5k3h7y_slave_addr = S5K3H7YMIPI_WRITE_ID;
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
			SENSORDB("Read sensor ID=0x%x\n",*sensorID);
			if(S5K3H7Y_SENSOR_ID == *sensorID)
			{
				break;
			}		
		}
		
		
	}


	if (*sensorID != S5K3H7Y_SENSOR_ID)
	{
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
	S5K3H7YSetFlipMirror(sensor_config_data->SensorImageMirror);
	
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
	S5K3H7YSetFlipMirror(sensor_config_data->SensorImageMirror);
	
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
	
	S5K3H7YSetFlipMirror(sensor_config_data->SensorImageMirror);
	
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
	
	S5K3H7YSetFlipMirror(sensor_config_data->SensorImageMirror);
	
    return ERROR_NONE;
}	/* S5K3H7YCapture() */

UINT32 S5K3H7YGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("enter\n");
	pSensorResolution->SensorPreviewWidth	= 	S5K3H7Y_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight	= 	S5K3H7Y_IMAGE_SENSOR_PV_HEIGHT;
	pSensorResolution->SensorVideoWidth		=	S5K3H7Y_IMAGE_SENSOR_VIDEO_WIDTH;
	pSensorResolution->SensorVideoHeight 	=	S5K3H7Y_IMAGE_SENSOR_VIDEO_HEIGHT;
	pSensorResolution->SensorFullWidth		= 	S5K3H7Y_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight		= 	S5K3H7Y_IMAGE_SENSOR_FULL_HEIGHT;
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
	s5k3h7y.sensorMode=MSDK_SCENARIO_ID_VIDEO_PREVIEW;
    SENSORDB("u2FrameRate=%d,sensorMode=%d\n", u2FrameRate,s5k3h7y.sensorMode);
	
	//if(u2FrameRate >30 || u2FrameRate <5)
	//{
	//    return ERROR_NONE;
	//}
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
	kal_uint16 frameLength = 0;
		
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


