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

#include "hi258yuv_Sensor.h"
#include "hi258yuv_Camera_Sensor_para.h"
#include "hi258yuv_CameraCustomized.h"

#define HI258_DEBUG
#ifdef HI258_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

MSDK_SENSOR_CONFIG_STRUCT HI258SensorConfigData;
#define SENSOR_CORE_PCLK	83200000	//48M PCLK Output 78000000 

#define WINMO_USE 0
#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT
#define MIPI_INTERFACE

kal_bool HI258_VEDIO_MPEG4 = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4);

kal_uint8 HI258_Sleep_Mode;
kal_uint32 HI258_PV_dummy_pixels=616,HI258_PV_dummy_lines=20,HI258_isp_master_clock=260/*0*/;

static HI258_SENSOR_INFO_ST HI258_sensor;
static HI258_OPERATION_STATE_ST HI258_op_state;

static kal_uint32 HI258_zoom_factor = 0; 
static kal_bool HI258_gPVmode = KAL_TRUE; //PV size or Full size
static kal_bool HI258_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool HI258_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint8 HI258_Banding_setting = AE_FLICKER_MODE_50HZ;  //Wonder add
static kal_uint16  HI258_PV_Shutter = 0;
static kal_uint32  HI258_sensor_pclk=260;//520 //20110518

static kal_uint32 HI258_pv_HI258_exposure_lines=0x05f370, HI258_cp_HI258_exposure_lines=0;
static kal_uint16 HI258_Capture_Max_Gain16= 6*16;
static kal_uint16 HI258_Capture_Gain16=0 ;    
static kal_uint16 HI258_Capture_Shutter=0;
static kal_uint16 HI258_Capture_Extra_Lines=0;

static int HI258_CAPATURE_FLAG = 0;//Add By Paul
static int HI258_CAPATUREB_FLAG = 0;//Add By Paul


static kal_uint16  HI258_PV_Gain16 = 0;
static kal_uint16  HI258_PV_Extra_Lines = 0;
kal_uint32 HI258_capture_pclk_in_M=520,HI258_preview_pclk_in_M=390;

//extern static CAMERA_DUAL_CAMERA_SENSOR_ENUM g_currDualSensorIdx;
//extern static char g_currSensorName[32];
//extern int kdModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name);

//extern int iReadReg_Byte(u8 addr, u8 *buf, u8 i2cId);
//extern int iWriteReg_Byte(u8 addr, u8 buf, u32 size, u16 i2cId);
//SENSOR_REG_STRUCT HI258SensorCCT[FACTORY_END_ADDR]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
//SENSOR_REG_STRUCT HI258SensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
//	camera_para.SENSOR.cct	SensorCCT	=> SensorCCT
//	camera_para.SENSOR.reg	SensorReg

BOOL HI258_set_param_banding(UINT16 para);

//extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff, u16 i2cId);
//extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes, u16 i2cId);
//extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
/*ergate-017*/
//extern int iWriteRegI2C_ext(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId, u16 speed);
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
static void HI258_set_mirror_flip(kal_uint8 image_mirror);
kal_uint16 HI258_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    //char puSendCmd[2] = {(char)(addr & 0xFF) ,(char)(para & 0xFF)};

    //iWriteRegI2C_ext(puSendCmd , 2,HI258_WRITE_ID_0, 50);
    //iWriteReg_Byte(addr, para, 1, HI258_WRITE_ID_0);
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd , 2,HI258_WRITE_ID_0);
    return 0;
}

kal_uint8 HI258_read_cmos_sensor(kal_uint8 addr)
{
    //kal_uint8 get_byte=0;
    //iReadReg_Byte(addr, &get_byte, HI258_WRITE_ID_0);
    //return get_byte;
    
    kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
    iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte,1,HI258_WRITE_ID_0);
    return get_byte;
}
#define GPIO_SUB_CAM_ID   GPIO118
#define GPIO_SUB_CAM_ID_M_GPIO   GPIO_MODE_00 
static kal_uint32 HI258_GetSensorID(kal_uint32 *sensorID)
{
    kal_uint16 sensor_id=0,hw_id=0;
    volatile signed char i;
    for(i=0;i<3;i++){
        sensor_id = HI258_read_cmos_sensor(0x04);
        
        mt_set_gpio_mode(GPIO_SUB_CAM_ID,GPIO_SUB_CAM_ID_M_GPIO);
        mt_set_gpio_dir(GPIO_SUB_CAM_ID,GPIO_DIR_IN);
	 mt_set_gpio_pull_enable(GPIO_SUB_CAM_ID, GPIO_PULL_ENABLE);
        hw_id=mt_get_gpio_in(GPIO_SUB_CAM_ID);
        mdelay(1);
        hw_id=mt_get_gpio_in(GPIO_SUB_CAM_ID);

	//mdelay(3000);	
        printk("[HI258] sensor id = 0x%x,====hw_id=%d\n", sensor_id,hw_id);
        
        if (HI258_SENSOR_ID == sensor_id)
            break;
    }
printk("[HI255]  sensor_id = %d  hw_id = %d",sensor_id , hw_id);
    *sensorID=sensor_id;
    if (HI258_SENSOR_ID == sensor_id && 0x00 == hw_id)
    {
        printk("[HI255] this is cmk HI258 camera module.");
    }
    else
    {
        *sensorID=0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;    
}


void HI258_Init_Cmds(void) 
{
HI258_write_cmos_sensor(0x01, 0x01); //sleep on
HI258_write_cmos_sensor(0x01, 0x03); //sleep off
HI258_write_cmos_sensor(0x01, 0x01); //sleep on
// PAGE 20
HI258_write_cmos_sensor(0x03, 0x20); // page 20
HI258_write_cmos_sensor(0x10, 0x1c); // AE off 60hz

// PAGE 22
HI258_write_cmos_sensor(0x03, 0x22); // page 22
HI258_write_cmos_sensor(0x10, 0x69); // AWB off

HI258_write_cmos_sensor(0x03, 0x00); //Dummy 750us
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);

HI258_write_cmos_sensor(0x08, 0x00);
HI258_write_cmos_sensor(0x09, 0x77);		// pad strength = max
HI258_write_cmos_sensor(0x0a, 0x07);		// pad strength = max

//PLL Setting
HI258_write_cmos_sensor(0x03, 0x00); 
HI258_write_cmos_sensor(0xd0, 0x05); //PLL pre_div 1/6 = 4 Mhz
HI258_write_cmos_sensor(0xd1, 0x30); //PLL maim_div 
HI258_write_cmos_sensor(0xd2, 0x05); //isp_div[1:0] mipi_4x_div[3:2]  mipi_1x_div[4] pll_bias_opt[7:5]    
HI258_write_cmos_sensor(0xd3, 0x20); //isp_clk_inv[0]  mipi_4x_inv[1]  mipi_1x_inv[2]
HI258_write_cmos_sensor(0xd0, 0x85);
HI258_write_cmos_sensor(0xd0, 0x85);
HI258_write_cmos_sensor(0xd0, 0x85);
HI258_write_cmos_sensor(0xd0, 0x95);

HI258_write_cmos_sensor(0x03, 0x00); //Dummy 750us
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x03, 0x00);


///// PAGE 20 /////
HI258_write_cmos_sensor(0x03, 0x20); //page 20
HI258_write_cmos_sensor(0x10, 0x1c); //AE off 50hz

///// PAGE 22 /////
HI258_write_cmos_sensor(0x03, 0x22); //page 22
HI258_write_cmos_sensor(0x10, 0x69); //AWB off

///// Initial Start /////
///// PAGE 0 Start /////
HI258_write_cmos_sensor(0x03, 0x00); //page 0
HI258_write_cmos_sensor(0x10, 0x10);
HI258_write_cmos_sensor(0x11, 0x93); //Windowing On + 1Frame Skip
HI258_write_cmos_sensor(0x12, 0x04); //Rinsing edge 0x04 // Falling edge 0x00
HI258_write_cmos_sensor(0x14, 0x05);

HI258_write_cmos_sensor(0x20, 0x00); //Row H
HI258_write_cmos_sensor(0x21, 0x04); //Row L
HI258_write_cmos_sensor(0x22, 0x00); //Col H
HI258_write_cmos_sensor(0x23, 0x04); //Col L

HI258_write_cmos_sensor(0x24, 0x04); //Window height_H //= 1200
HI258_write_cmos_sensor(0x25, 0xb0); //Window height_L //
HI258_write_cmos_sensor(0x26, 0x06); //Window width_H  //= 1600
HI258_write_cmos_sensor(0x27, 0x40); //Window wight_L

HI258_write_cmos_sensor(0x28, 0x04); //Full row start y-flip 
HI258_write_cmos_sensor(0x29, 0x01); //Pre1 row start no-flip
HI258_write_cmos_sensor(0x2a, 0x02); //Pre1 row start y-flip 

HI258_write_cmos_sensor(0x2b, 0x04); //Full wid start x-flip 
HI258_write_cmos_sensor(0x2c, 0x04); //Pre2 wid start no-flip
HI258_write_cmos_sensor(0x2d, 0x02); //Pre2 wid start x-flip 

HI258_write_cmos_sensor(0x40, 0x01); //Hblank_456
HI258_write_cmos_sensor(0x41, 0x78);
HI258_write_cmos_sensor(0x42, 0x00); //Vblank
HI258_write_cmos_sensor(0x43, 0x14); //Flick Stop

HI258_write_cmos_sensor(0x50, 0x00); //Test Pattern

///// BLC /////
HI258_write_cmos_sensor(0x80, 0x2e);
HI258_write_cmos_sensor(0x81, 0x7e);
HI258_write_cmos_sensor(0x82, 0x90);
HI258_write_cmos_sensor(0x83, 0x00);
HI258_write_cmos_sensor(0x84, 0xcc); //20130604 0x0c->0xcc
HI258_write_cmos_sensor(0x85, 0x00);
HI258_write_cmos_sensor(0x86, 0x00);
HI258_write_cmos_sensor(0x87, 0x0f);
HI258_write_cmos_sensor(0x88, 0x34);
HI258_write_cmos_sensor(0x8a, 0x0b);
HI258_write_cmos_sensor(0x8e, 0x80); //Pga Blc Hold

HI258_write_cmos_sensor(0x90, 0x11); //BLC_TIME_TH_ON
HI258_write_cmos_sensor(0x91, 0x11); //BLC_TIME_TH_OFF
HI258_write_cmos_sensor(0x92, 0x98); //BLC_AG_TH_ON
HI258_write_cmos_sensor(0x93, 0x90); //BLC_AG_TH_OFF
HI258_write_cmos_sensor(0x96, 0xdc); //BLC Outdoor Th On
HI258_write_cmos_sensor(0x97, 0xfe); //BLC Outdoor Th Off
HI258_write_cmos_sensor(0x98, 0x38);

//OutDoor  BLC
HI258_write_cmos_sensor(0x99, 0x43); //R,Gr,B,Gb Offset

//Dark BLC
HI258_write_cmos_sensor(0xa0, 0x01); //R,Gr,B,Gb Offset

//Normal BLC
HI258_write_cmos_sensor(0xa8, 0x43); //R,Gr,B,Gb Offset
///// PAGE 0 END /////

///// PAGE 2 START /////
HI258_write_cmos_sensor(0x03, 0x02);
HI258_write_cmos_sensor(0x10, 0x00);
HI258_write_cmos_sensor(0x13, 0x00);
HI258_write_cmos_sensor(0x14, 0x00);
HI258_write_cmos_sensor(0x18, 0xcc);
HI258_write_cmos_sensor(0x19, 0x01); // pmos switch on (for cfpn)
HI258_write_cmos_sensor(0x1A, 0x39); //20130604 0x09->0xcc
HI258_write_cmos_sensor(0x1B, 0x00);
HI258_write_cmos_sensor(0x1C, 0x1a); // for ncp
HI258_write_cmos_sensor(0x1D, 0x14); // for ncp
HI258_write_cmos_sensor(0x1E, 0x30); // for ncp
HI258_write_cmos_sensor(0x1F, 0x10);

HI258_write_cmos_sensor(0x20, 0x77);
HI258_write_cmos_sensor(0x21, 0xde);
HI258_write_cmos_sensor(0x22, 0xa7);
HI258_write_cmos_sensor(0x23, 0x30);
HI258_write_cmos_sensor(0x24, 0x77);
HI258_write_cmos_sensor(0x25, 0x10);
HI258_write_cmos_sensor(0x26, 0x10);
HI258_write_cmos_sensor(0x27, 0x3c);
HI258_write_cmos_sensor(0x2b, 0x80);
HI258_write_cmos_sensor(0x2c, 0x02);
HI258_write_cmos_sensor(0x2d, 0x58);
HI258_write_cmos_sensor(0x2e, 0x11);//20130604 0xde->0x11
HI258_write_cmos_sensor(0x2f, 0x11);//20130604 0xa7->0x11

HI258_write_cmos_sensor(0x30, 0x00);
HI258_write_cmos_sensor(0x31, 0x99);
HI258_write_cmos_sensor(0x32, 0x00);
HI258_write_cmos_sensor(0x33, 0x00);
HI258_write_cmos_sensor(0x34, 0x22);
HI258_write_cmos_sensor(0x36, 0x75);
HI258_write_cmos_sensor(0x38, 0x88);
HI258_write_cmos_sensor(0x39, 0x88);
HI258_write_cmos_sensor(0x3d, 0x03);
HI258_write_cmos_sensor(0x3f, 0x02);

HI258_write_cmos_sensor(0x49, 0xc1);//20130604 0x87->0xd1 --> mode Change Issue modify -> 0xc1 
HI258_write_cmos_sensor(0x4a, 0x10);

HI258_write_cmos_sensor(0x50, 0x21);
HI258_write_cmos_sensor(0x53, 0xb1);
HI258_write_cmos_sensor(0x54, 0x10);
HI258_write_cmos_sensor(0x55, 0x1c); // for ncp
HI258_write_cmos_sensor(0x56, 0x11);
HI258_write_cmos_sensor(0x58, 0x3a);//20130604 add
HI258_write_cmos_sensor(0x59, 0x38);//20130604 add
HI258_write_cmos_sensor(0x5d, 0xa2);
HI258_write_cmos_sensor(0x5e, 0x5a);

HI258_write_cmos_sensor(0x60, 0x87);
HI258_write_cmos_sensor(0x61, 0x98);
HI258_write_cmos_sensor(0x62, 0x88);
HI258_write_cmos_sensor(0x63, 0x96);
HI258_write_cmos_sensor(0x64, 0x88);
HI258_write_cmos_sensor(0x65, 0x96);
HI258_write_cmos_sensor(0x67, 0x3f);
HI258_write_cmos_sensor(0x68, 0x3f);
HI258_write_cmos_sensor(0x69, 0x3f);

HI258_write_cmos_sensor(0x72, 0x89);
HI258_write_cmos_sensor(0x73, 0x95);
HI258_write_cmos_sensor(0x74, 0x89);
HI258_write_cmos_sensor(0x75, 0x95);
HI258_write_cmos_sensor(0x7C, 0x84);
HI258_write_cmos_sensor(0x7D, 0xaf);

HI258_write_cmos_sensor(0x80, 0x01);
HI258_write_cmos_sensor(0x81, 0x7a);
HI258_write_cmos_sensor(0x82, 0x13);
HI258_write_cmos_sensor(0x83, 0x24);
HI258_write_cmos_sensor(0x84, 0x78);
HI258_write_cmos_sensor(0x85, 0x7c);

HI258_write_cmos_sensor(0x92, 0x44);
HI258_write_cmos_sensor(0x93, 0x59);
HI258_write_cmos_sensor(0x94, 0x78);
HI258_write_cmos_sensor(0x95, 0x7c);

HI258_write_cmos_sensor(0xA0, 0x02);
HI258_write_cmos_sensor(0xA1, 0x74);
HI258_write_cmos_sensor(0xA4, 0x74);
HI258_write_cmos_sensor(0xA5, 0x02);
HI258_write_cmos_sensor(0xA8, 0x85);
HI258_write_cmos_sensor(0xA9, 0x8c);
HI258_write_cmos_sensor(0xAC, 0x10);
HI258_write_cmos_sensor(0xAD, 0x16);

HI258_write_cmos_sensor(0xB0, 0x99);
HI258_write_cmos_sensor(0xB1, 0xa3);
HI258_write_cmos_sensor(0xB4, 0x9b);
HI258_write_cmos_sensor(0xB5, 0xa2);
HI258_write_cmos_sensor(0xB8, 0x9b);
HI258_write_cmos_sensor(0xB9, 0x9f);
HI258_write_cmos_sensor(0xBC, 0x9b);
HI258_write_cmos_sensor(0xBD, 0x9f);

HI258_write_cmos_sensor(0xc4, 0x29);
HI258_write_cmos_sensor(0xc5, 0x40);
HI258_write_cmos_sensor(0xc6, 0x5c);
HI258_write_cmos_sensor(0xc7, 0x72);
HI258_write_cmos_sensor(0xc8, 0x2a);
HI258_write_cmos_sensor(0xc9, 0x3f);
HI258_write_cmos_sensor(0xcc, 0x5d);
HI258_write_cmos_sensor(0xcd, 0x71);

HI258_write_cmos_sensor(0xd0, 0x10);
HI258_write_cmos_sensor(0xd1, 0x14);
HI258_write_cmos_sensor(0xd2, 0x20);
HI258_write_cmos_sensor(0xd3, 0x00);
HI258_write_cmos_sensor(0xd4, 0x11); //DCDC_TIME_TH_ON
HI258_write_cmos_sensor(0xd5, 0x11); //DCDC_TIME_TH_OFF 
HI258_write_cmos_sensor(0xd6, 0x98); //DCDC_AG_TH_ON
HI258_write_cmos_sensor(0xd7, 0x90); //DCDC_AG_TH_OFF
HI258_write_cmos_sensor(0xdc, 0x00);
HI258_write_cmos_sensor(0xdd, 0xa3);
HI258_write_cmos_sensor(0xde, 0x00);
HI258_write_cmos_sensor(0xdf, 0x84);

HI258_write_cmos_sensor(0xe0, 0xa4);
HI258_write_cmos_sensor(0xe1, 0xa4);
HI258_write_cmos_sensor(0xe2, 0xa4);
HI258_write_cmos_sensor(0xe3, 0xa4);
HI258_write_cmos_sensor(0xe4, 0xa4);
HI258_write_cmos_sensor(0xe5, 0x01);
HI258_write_cmos_sensor(0xe8, 0x00);
HI258_write_cmos_sensor(0xe9, 0x00);
HI258_write_cmos_sensor(0xea, 0x77);

HI258_write_cmos_sensor(0xF0, 0x00);
HI258_write_cmos_sensor(0xF1, 0x00);
HI258_write_cmos_sensor(0xF2, 0x00);

///// PAGE 2 END /////


///// PAGE 10 START /////
HI258_write_cmos_sensor(0x03, 0x10); //page 10
HI258_write_cmos_sensor(0x10, 0x03); //S2D enable _ YUYV Order º¯°æ
HI258_write_cmos_sensor(0x11, 0x03);
HI258_write_cmos_sensor(0x12, 0x30);
HI258_write_cmos_sensor(0x13, 0x03);

HI258_write_cmos_sensor(0x20, 0x00);
HI258_write_cmos_sensor(0x21, 0x40);
HI258_write_cmos_sensor(0x22, 0x0f);
HI258_write_cmos_sensor(0x24, 0x20);
HI258_write_cmos_sensor(0x25, 0x10);
HI258_write_cmos_sensor(0x26, 0x01);
HI258_write_cmos_sensor(0x27, 0x02);
HI258_write_cmos_sensor(0x28, 0x11);

HI258_write_cmos_sensor(0x40, 0x00);
HI258_write_cmos_sensor(0x41, 0x00); //D-YOffset Th
HI258_write_cmos_sensor(0x42, 0x00); //Cb Offset
HI258_write_cmos_sensor(0x43, 0x00); //Cr Offset
HI258_write_cmos_sensor(0x44, 0x80);
HI258_write_cmos_sensor(0x45, 0x80);
HI258_write_cmos_sensor(0x46, 0xf0);
HI258_write_cmos_sensor(0x48, 0x80);
HI258_write_cmos_sensor(0x4a, 0x80);

HI258_write_cmos_sensor(0x50, 0xa0); //D-YOffset AG

HI258_write_cmos_sensor(0x60, 0x4f);
HI258_write_cmos_sensor(0x61, 0x90); //Sat B
HI258_write_cmos_sensor(0x62, 0x88); //Sat R
HI258_write_cmos_sensor(0x63, 0x80); //Auto-De Color

HI258_write_cmos_sensor(0x66, 0x42);
HI258_write_cmos_sensor(0x67, 0x22);

HI258_write_cmos_sensor(0x6a, 0x5a); //White Protection Offset Dark/Indoor
HI258_write_cmos_sensor(0x74, 0x0c); //White Protection Offset Outdoor
HI258_write_cmos_sensor(0x76, 0x01); //White Protection Enable
///// PAGE 10 END /////

///// PAGE 11 START /////
HI258_write_cmos_sensor(0x03, 0x11); //page 11

//LPF Auto Control
HI258_write_cmos_sensor(0x20, 0x00);
HI258_write_cmos_sensor(0x21, 0x00);
HI258_write_cmos_sensor(0x26, 0x5a); // Double_AG
HI258_write_cmos_sensor(0x27, 0x58); // Double_AG
HI258_write_cmos_sensor(0x28, 0x0f);
HI258_write_cmos_sensor(0x29, 0x10);
HI258_write_cmos_sensor(0x2b, 0x30);
HI258_write_cmos_sensor(0x2c, 0x32);

//GBGR 
HI258_write_cmos_sensor(0x70, 0x2b);
HI258_write_cmos_sensor(0x74, 0x30);
HI258_write_cmos_sensor(0x75, 0x18);
HI258_write_cmos_sensor(0x76, 0x30);
HI258_write_cmos_sensor(0x77, 0xff);
HI258_write_cmos_sensor(0x78, 0xa0);
HI258_write_cmos_sensor(0x79, 0xff); //Dark GbGr Th
HI258_write_cmos_sensor(0x7a, 0x30);
HI258_write_cmos_sensor(0x7b, 0x20);
HI258_write_cmos_sensor(0x7c, 0xf4); //Dark Dy Th B[7:4]
HI258_write_cmos_sensor(0x7d, 0x02);
HI258_write_cmos_sensor(0x7e, 0xb0);
HI258_write_cmos_sensor(0x7f, 0x10);
///// PAGE 11 END /////

///// PAGE 12 START /////
HI258_write_cmos_sensor(0x03, 0x12); //page 11

//YC2D
HI258_write_cmos_sensor(0x10, 0x03); //Y DPC Enable
HI258_write_cmos_sensor(0x11, 0x08); //
HI258_write_cmos_sensor(0x12, 0x10); //0x30 -> 0x10
HI258_write_cmos_sensor(0x20, 0x53); //Y_lpf_enable
HI258_write_cmos_sensor(0x21, 0x03); //C_lpf_enable_on
HI258_write_cmos_sensor(0x22, 0xe6); //YC2D_CrCbY_Dy

HI258_write_cmos_sensor(0x23, 0x14); //Outdoor Dy Th
HI258_write_cmos_sensor(0x24, 0x20); //Indoor Dy Th // For reso Limit 0x20
HI258_write_cmos_sensor(0x25, 0x30); //Dark Dy Th

//Outdoor LPF Flat
HI258_write_cmos_sensor(0x30, 0xff); //Y Hi Th
HI258_write_cmos_sensor(0x31, 0x00); //Y Lo Th
HI258_write_cmos_sensor(0x32, 0xf0); //Std Hi Th //Reso Improve Th Low //50
HI258_write_cmos_sensor(0x33, 0x00); //Std Lo Th
HI258_write_cmos_sensor(0x34, 0x00); //Median ratio

//Indoor LPF Flat
HI258_write_cmos_sensor(0x35, 0xff); //Y Hi Th
HI258_write_cmos_sensor(0x36, 0x00); //Y Lo Th
HI258_write_cmos_sensor(0x37, 0xff); //Std Hi Th //Reso Improve Th Low //50
HI258_write_cmos_sensor(0x38, 0x00); //Std Lo Th
HI258_write_cmos_sensor(0x39, 0x00); //Median ratio

//Dark LPF Flat
HI258_write_cmos_sensor(0x3a, 0xff); //Y Hi Th
HI258_write_cmos_sensor(0x3b, 0x00); //Y Lo Th
HI258_write_cmos_sensor(0x3c, 0x93); //Std Hi Th //Reso Improve Th Low //50
HI258_write_cmos_sensor(0x3d, 0x00); //Std Lo Th
HI258_write_cmos_sensor(0x3e, 0x00); //Median ratio

//Outdoor Cindition
HI258_write_cmos_sensor(0x46, 0xa0); //Out Lum Hi
HI258_write_cmos_sensor(0x47, 0x40); //Out Lum Lo

//Indoor Cindition
HI258_write_cmos_sensor(0x4c, 0xb0); //Indoor Lum Hi
HI258_write_cmos_sensor(0x4d, 0x40); //Indoor Lum Lo

//Dark Cindition
HI258_write_cmos_sensor(0x52, 0xb0); //Dark Lum Hi
HI258_write_cmos_sensor(0x53, 0x50); //Dark Lum Lo

//C-Filter
HI258_write_cmos_sensor(0x70, 0x10); //Outdoor(2:1) AWM Th Horizontal
HI258_write_cmos_sensor(0x71, 0x0a); //Outdoor(2:1) Diff Th Vertical
HI258_write_cmos_sensor(0x72, 0x10); //Indoor,Dark1 AWM Th Horizontal
HI258_write_cmos_sensor(0x73, 0x0a); //Indoor,Dark1 Diff Th Vertical
HI258_write_cmos_sensor(0x74, 0x18); //Dark(2:3) AWM Th Horizontal
HI258_write_cmos_sensor(0x75, 0x0f); //Dark(2:3) Diff Th Vertical

//DPC
HI258_write_cmos_sensor(0x90, 0x7d);
HI258_write_cmos_sensor(0x91, 0x34);
HI258_write_cmos_sensor(0x99, 0x28);
HI258_write_cmos_sensor(0x9c, 0x0a);
HI258_write_cmos_sensor(0x9d, 0x15);
HI258_write_cmos_sensor(0x9e, 0x28);
HI258_write_cmos_sensor(0x9f, 0x28);
HI258_write_cmos_sensor(0xb0, 0x0e); //Zipper noise Detault change (0x75->0x0e)
HI258_write_cmos_sensor(0xb8, 0x44);
HI258_write_cmos_sensor(0xb9, 0x15);
///// PAGE 12 END /////

///// PAGE 13 START /////
HI258_write_cmos_sensor(0x03, 0x13); //page 13

HI258_write_cmos_sensor(0x80, 0xfd); //Sharp2D enable _ YUYV Order
HI258_write_cmos_sensor(0x81, 0x07); //Sharp2D Clip/Limit
HI258_write_cmos_sensor(0x82, 0x73); //Sharp2D Filter
HI258_write_cmos_sensor(0x83, 0x00); //Sharp2D Low Clip
HI258_write_cmos_sensor(0x85, 0x00);

HI258_write_cmos_sensor(0x92, 0x33); //Sharp2D Slop n/p
HI258_write_cmos_sensor(0x93, 0x30); //Sharp2D LClip
HI258_write_cmos_sensor(0x94, 0x02); //Sharp2D HiClip1 Th
HI258_write_cmos_sensor(0x95, 0xf0); //Sharp2D HiClip2 Th
HI258_write_cmos_sensor(0x96, 0x1e); //Sharp2D HiClip2 Resolution
HI258_write_cmos_sensor(0x97, 0x40); 
HI258_write_cmos_sensor(0x98, 0x80);
HI258_write_cmos_sensor(0x99, 0x40);

//Sharp Lclp
HI258_write_cmos_sensor(0xa2, 0x03); //Outdoor Lclip_N
HI258_write_cmos_sensor(0xa3, 0x04); //Outdoor Lclip_P
HI258_write_cmos_sensor(0xa4, 0x02); //Indoor Lclip_N 0x03 For reso Limit 0x0e
HI258_write_cmos_sensor(0xa5, 0x03); //Indoor Lclip_P 0x0f For reso Limit 0x0f
HI258_write_cmos_sensor(0xa6, 0x80); //Dark Lclip_N
HI258_write_cmos_sensor(0xa7, 0x80); //Dark Lclip_P

//Outdoor Slope
HI258_write_cmos_sensor(0xb6, 0x28); //Lum negative Hi
HI258_write_cmos_sensor(0xb7, 0x20); //Lum negative middle
HI258_write_cmos_sensor(0xb8, 0x24); //Lum negative Low
HI258_write_cmos_sensor(0xb9, 0x28); //Lum postive Hi
HI258_write_cmos_sensor(0xba, 0x20); //Lum postive middle
HI258_write_cmos_sensor(0xbb, 0x24); //Lum postive Low

//Indoor Slope
HI258_write_cmos_sensor(0xbc, 0x28); //Lum negative Hi
HI258_write_cmos_sensor(0xbd, 0x20); //Lum negative middle
HI258_write_cmos_sensor(0xbe, 0x24); //Lum negative Low
HI258_write_cmos_sensor(0xbf, 0x28); //Lum postive Hi
HI258_write_cmos_sensor(0xc0, 0x20); //Lum postive middle
HI258_write_cmos_sensor(0xc1, 0x24); //Lum postive Low

//Dark Slope
HI258_write_cmos_sensor(0xc2, 0x14); //Lum negative Hi
HI258_write_cmos_sensor(0xc3, 0x24); //Lum negative middle
HI258_write_cmos_sensor(0xc4, 0x1d); //Lum negative Low
HI258_write_cmos_sensor(0xc5, 0x14); //Lum postive Hi
HI258_write_cmos_sensor(0xc6, 0x24); //Lum postive middle
HI258_write_cmos_sensor(0xc7, 0x1d); //Lum postive Low
///// PAGE 13 END /////

///// PAGE 14 START /////
HI258_write_cmos_sensor(0x03, 0x14); //page 14
HI258_write_cmos_sensor(0x10, 0x09);

HI258_write_cmos_sensor(0x20, 0x80); //X-Center
HI258_write_cmos_sensor(0x21, 0x80); //Y-Center

HI258_write_cmos_sensor(0x22, 0x40); //LSC R 1b->15 20130125
HI258_write_cmos_sensor(0x23, 0x34); //LSC G
HI258_write_cmos_sensor(0x24, 0x34); //LSC B

HI258_write_cmos_sensor(0x25, 0xf0); //LSC Off
HI258_write_cmos_sensor(0x26, 0xf0); //LSC On
///// PAGE 14 END /////

/////// PAGE 15 START ///////
HI258_write_cmos_sensor(0x03, 0x15); //page 15
HI258_write_cmos_sensor(0x10, 0x0f);
HI258_write_cmos_sensor(0x14, 0x52);
HI258_write_cmos_sensor(0x15, 0x42);
HI258_write_cmos_sensor(0x16, 0x32);
HI258_write_cmos_sensor(0x17, 0x2f);

//CMC
HI258_write_cmos_sensor(0x30, 0x8e);
HI258_write_cmos_sensor(0x31, 0x75);
HI258_write_cmos_sensor(0x32, 0x25);
HI258_write_cmos_sensor(0x33, 0x15);
HI258_write_cmos_sensor(0x34, 0x5a);
HI258_write_cmos_sensor(0x35, 0x05);
HI258_write_cmos_sensor(0x36, 0x07);
HI258_write_cmos_sensor(0x37, 0x40);
HI258_write_cmos_sensor(0x38, 0x85);

//CMC OFS
HI258_write_cmos_sensor(0x40, 0x95);
HI258_write_cmos_sensor(0x41, 0x1f);
HI258_write_cmos_sensor(0x42, 0x8a);
HI258_write_cmos_sensor(0x43, 0x86);
HI258_write_cmos_sensor(0x44, 0x0a);
HI258_write_cmos_sensor(0x45, 0x84);
HI258_write_cmos_sensor(0x46, 0x87);
HI258_write_cmos_sensor(0x47, 0x9b);
HI258_write_cmos_sensor(0x48, 0x23);

//CMC POFS
HI258_write_cmos_sensor(0x50, 0x8c);
HI258_write_cmos_sensor(0x51, 0x0c);
HI258_write_cmos_sensor(0x52, 0x00);
HI258_write_cmos_sensor(0x53, 0x07);
HI258_write_cmos_sensor(0x54, 0x17);
HI258_write_cmos_sensor(0x55, 0x9d);
HI258_write_cmos_sensor(0x56, 0x00);
HI258_write_cmos_sensor(0x57, 0x0b);
HI258_write_cmos_sensor(0x58, 0x89);
///// PAGE 15 END /////

///// PAGE 16 START /////
HI258_write_cmos_sensor(0x03, 0x16); //page 16
HI258_write_cmos_sensor(0x10, 0x31);
HI258_write_cmos_sensor(0x18, 0x5a); //Double_AG 5e->37
HI258_write_cmos_sensor(0x19, 0x58); //Double_AG 5e->36
HI258_write_cmos_sensor(0x1a, 0x0e);
HI258_write_cmos_sensor(0x1b, 0x01);
HI258_write_cmos_sensor(0x1c, 0xdc);
HI258_write_cmos_sensor(0x1d, 0xfe);

//Indoor
HI258_write_cmos_sensor(0x30, 0x00);
HI258_write_cmos_sensor(0x31, 0x03);
HI258_write_cmos_sensor(0x32, 0x0e);
HI258_write_cmos_sensor(0x33, 0x19);
HI258_write_cmos_sensor(0x34, 0x3d);
HI258_write_cmos_sensor(0x35, 0x57);
HI258_write_cmos_sensor(0x36, 0x6b);
HI258_write_cmos_sensor(0x37, 0x7f);
HI258_write_cmos_sensor(0x38, 0x90);
HI258_write_cmos_sensor(0x39, 0xa0);
HI258_write_cmos_sensor(0x3a, 0xae);
HI258_write_cmos_sensor(0x3b, 0xbc);
HI258_write_cmos_sensor(0x3c, 0xc8);
HI258_write_cmos_sensor(0x3d, 0xd4);
HI258_write_cmos_sensor(0x3e, 0xde);
HI258_write_cmos_sensor(0x3f, 0xe8);
HI258_write_cmos_sensor(0x40, 0xf1);
HI258_write_cmos_sensor(0x41, 0xf8);
HI258_write_cmos_sensor(0x42, 0xff);


//Outdoor
HI258_write_cmos_sensor(0x50, 0x00);
HI258_write_cmos_sensor(0x51, 0x03);
HI258_write_cmos_sensor(0x52, 0x10);
HI258_write_cmos_sensor(0x53, 0x26);
HI258_write_cmos_sensor(0x54, 0x43);
HI258_write_cmos_sensor(0x55, 0x5d);
HI258_write_cmos_sensor(0x56, 0x79);
HI258_write_cmos_sensor(0x57, 0x8c);
HI258_write_cmos_sensor(0x58, 0x9f);
HI258_write_cmos_sensor(0x59, 0xaa);
HI258_write_cmos_sensor(0x5a, 0xb6);
HI258_write_cmos_sensor(0x5b, 0xc3);
HI258_write_cmos_sensor(0x5c, 0xce);
HI258_write_cmos_sensor(0x5d, 0xd9);
HI258_write_cmos_sensor(0x5e, 0xe1);
HI258_write_cmos_sensor(0x5f, 0xe9);
HI258_write_cmos_sensor(0x60, 0xf0);
HI258_write_cmos_sensor(0x61, 0xf4);
HI258_write_cmos_sensor(0x62, 0xf5);

//Dark
HI258_write_cmos_sensor(0x70, 0x00);
HI258_write_cmos_sensor(0x71, 0x10);
HI258_write_cmos_sensor(0x72, 0x1c);
HI258_write_cmos_sensor(0x73, 0x2e);
HI258_write_cmos_sensor(0x74, 0x4e);
HI258_write_cmos_sensor(0x75, 0x6c);
HI258_write_cmos_sensor(0x76, 0x82);
HI258_write_cmos_sensor(0x77, 0x96);
HI258_write_cmos_sensor(0x78, 0xa7);
HI258_write_cmos_sensor(0x79, 0xb6);
HI258_write_cmos_sensor(0x7a, 0xc4);
HI258_write_cmos_sensor(0x7b, 0xd0);
HI258_write_cmos_sensor(0x7c, 0xda);
HI258_write_cmos_sensor(0x7d, 0xe2);
HI258_write_cmos_sensor(0x7e, 0xea);
HI258_write_cmos_sensor(0x7f, 0xf0);
HI258_write_cmos_sensor(0x80, 0xf6);
HI258_write_cmos_sensor(0x81, 0xfa);
HI258_write_cmos_sensor(0x82, 0xff);
///// PAGE 16 END /////

///// PAGE 17 START /////
HI258_write_cmos_sensor(0x03, 0x17); //page 17
HI258_write_cmos_sensor(0xc1, 0x00);
HI258_write_cmos_sensor(0xc4, 0x4b);
HI258_write_cmos_sensor(0xc5, 0x3f);
HI258_write_cmos_sensor(0xc6, 0x02);
HI258_write_cmos_sensor(0xc7, 0x20);
///// PAGE 17 END /////

///// PAGE 18 START /////
HI258_write_cmos_sensor(0x03, 0x18); //page 18
HI258_write_cmos_sensor(0x10, 0x00);	//Scale Off
HI258_write_cmos_sensor(0x11, 0x00);
HI258_write_cmos_sensor(0x12, 0x58);
HI258_write_cmos_sensor(0x13, 0x01);
HI258_write_cmos_sensor(0x14, 0x00); //Sawtooth
HI258_write_cmos_sensor(0x15, 0x00);
HI258_write_cmos_sensor(0x16, 0x00);
HI258_write_cmos_sensor(0x17, 0x00);
HI258_write_cmos_sensor(0x18, 0x00);
HI258_write_cmos_sensor(0x19, 0x00);
HI258_write_cmos_sensor(0x1a, 0x00);
HI258_write_cmos_sensor(0x1b, 0x00);
HI258_write_cmos_sensor(0x1c, 0x00);
HI258_write_cmos_sensor(0x1d, 0x00);
HI258_write_cmos_sensor(0x1e, 0x00);
HI258_write_cmos_sensor(0x1f, 0x00);
HI258_write_cmos_sensor(0x20, 0x05);	//zoom wid
HI258_write_cmos_sensor(0x21, 0x00);
HI258_write_cmos_sensor(0x22, 0x01);	//zoom hgt
HI258_write_cmos_sensor(0x23, 0xe0);
HI258_write_cmos_sensor(0x24, 0x00);	//zoom start x
HI258_write_cmos_sensor(0x25, 0x00);
HI258_write_cmos_sensor(0x26, 0x00);	//zoom start y
HI258_write_cmos_sensor(0x27, 0x00);
HI258_write_cmos_sensor(0x28, 0x05);	//zoom end x
HI258_write_cmos_sensor(0x29, 0x00);
HI258_write_cmos_sensor(0x2a, 0x01);	//zoom end y
HI258_write_cmos_sensor(0x2b, 0xe0);
HI258_write_cmos_sensor(0x2c, 0x0a);	//zoom step vert
HI258_write_cmos_sensor(0x2d, 0x00);
HI258_write_cmos_sensor(0x2e, 0x0a);	//zoom step horz
HI258_write_cmos_sensor(0x2f, 0x00);
HI258_write_cmos_sensor(0x30, 0x44);	//zoom fifo

///// PAGE 18 END /////

HI258_write_cmos_sensor(0x03, 0x19); //Page 0x19
HI258_write_cmos_sensor(0x10, 0x7f); //mcmc_ctl1
HI258_write_cmos_sensor(0x11, 0x7f); //mcmc_ctl2
HI258_write_cmos_sensor(0x12, 0x1e); //mcmc_delta1
HI258_write_cmos_sensor(0x13, 0x32); //mcmc_center1
HI258_write_cmos_sensor(0x14, 0x1e); //mcmc_delta2
HI258_write_cmos_sensor(0x15, 0x6e); //mcmc_center2
HI258_write_cmos_sensor(0x16, 0x0a); //mcmc_delta3
HI258_write_cmos_sensor(0x17, 0xb8); //mcmc_center3
HI258_write_cmos_sensor(0x18, 0x1e); //mcmc_delta4
HI258_write_cmos_sensor(0x19, 0xe6); //mcmc_center4
HI258_write_cmos_sensor(0x1a, 0x9e); //mcmc_delta5
HI258_write_cmos_sensor(0x1b, 0x22); //mcmc_center5
HI258_write_cmos_sensor(0x1c, 0x9e); //mcmc_delta6
HI258_write_cmos_sensor(0x1d, 0x5e); //mcmc_center6
HI258_write_cmos_sensor(0x1e, 0x3b); //mcmc_sat_gain1
HI258_write_cmos_sensor(0x1f, 0x3e); //mcmc_sat_gain2
HI258_write_cmos_sensor(0x20, 0x4b); //mcmc_sat_gain3
HI258_write_cmos_sensor(0x21, 0x52); //mcmc_sat_gain4
HI258_write_cmos_sensor(0x22, 0x59); //mcmc_sat_gain5
HI258_write_cmos_sensor(0x23, 0x46); //mcmc_sat_gain6
HI258_write_cmos_sensor(0x24, 0x00); //mcmc_hue_angle1
HI258_write_cmos_sensor(0x25, 0x01); //mcmc_hue_angle2
HI258_write_cmos_sensor(0x26, 0x0e); //mcmc_hue_angle3
HI258_write_cmos_sensor(0x27, 0x04); //mcmc_hue_angle4
HI258_write_cmos_sensor(0x28, 0x00); //mcmc_hue_angle5
HI258_write_cmos_sensor(0x29, 0x8c); //mcmc_hue_angle6
HI258_write_cmos_sensor(0x53, 0x10); //mcmc_ctl3
HI258_write_cmos_sensor(0x6c, 0xff); //mcmc_lum_ctl1
HI258_write_cmos_sensor(0x6d, 0x3f); //mcmc_lum_ctl2
HI258_write_cmos_sensor(0x6e, 0x00); //mcmc_lum_ctl3
HI258_write_cmos_sensor(0x6f, 0x00); //mcmc_lum_ctl4
HI258_write_cmos_sensor(0x70, 0x00); //mcmc_lum_ctl5
HI258_write_cmos_sensor(0x71, 0x3f); //rg1_lum_gain_wgt_th1
HI258_write_cmos_sensor(0x72, 0x3f); //rg1_lum_gain_wgt_th2
HI258_write_cmos_sensor(0x73, 0x3f); //rg1_lum_gain_wgt_th3
HI258_write_cmos_sensor(0x74, 0x3f); //rg1_lum_gain_wgt_th4
HI258_write_cmos_sensor(0x75, 0x30); //rg1_lum_sp1
HI258_write_cmos_sensor(0x76, 0x50); //rg1_lum_sp2
HI258_write_cmos_sensor(0x77, 0x80); //rg1_lum_sp3
HI258_write_cmos_sensor(0x78, 0xb0); //rg1_lum_sp4
HI258_write_cmos_sensor(0x79, 0x3f); //rg2_gain_wgt_th1
HI258_write_cmos_sensor(0x7a, 0x3f); //rg2_gain_wgt_th2
HI258_write_cmos_sensor(0x7b, 0x3f); //rg2_gain_wgt_th3
HI258_write_cmos_sensor(0x7c, 0x3f); //rg2_gain_wgt_th4
HI258_write_cmos_sensor(0x7d, 0x28); //rg2_lum_sp1
HI258_write_cmos_sensor(0x7e, 0x50); //rg2_lum_sp2
HI258_write_cmos_sensor(0x7f, 0x80); //rg2_lum_sp3
HI258_write_cmos_sensor(0x80, 0xb0); //rg2_lum_sp4
HI258_write_cmos_sensor(0x81, 0x28); //rg3_gain_wgt_th1
HI258_write_cmos_sensor(0x82, 0x3f); //rg3_gain_wgt_th2
HI258_write_cmos_sensor(0x83, 0x3f); //rg3_gain_wgt_th3
HI258_write_cmos_sensor(0x84, 0x3f); //rg3_gain_wgt_th4
HI258_write_cmos_sensor(0x85, 0x28); //rg3_lum_sp1
HI258_write_cmos_sensor(0x86, 0x50); //rg3_lum_sp2
HI258_write_cmos_sensor(0x87, 0x80); //rg3_lum_sp3
HI258_write_cmos_sensor(0x88, 0xb0); //rg3_lum_sp4
HI258_write_cmos_sensor(0x89, 0x1a); //rg4_gain_wgt_th1
HI258_write_cmos_sensor(0x8a, 0x28); //rg4_gain_wgt_th2
HI258_write_cmos_sensor(0x8b, 0x3f); //rg4_gain_wgt_th3
HI258_write_cmos_sensor(0x8c, 0x3f); //rg4_gain_wgt_th4
HI258_write_cmos_sensor(0x8d, 0x10); //rg4_lum_sp1
HI258_write_cmos_sensor(0x8e, 0x30); //rg4_lum_sp2
HI258_write_cmos_sensor(0x8f, 0x60); //rg4_lum_sp3
HI258_write_cmos_sensor(0x90, 0x90); //rg4_lum_sp4
HI258_write_cmos_sensor(0x91, 0x1a); //rg5_gain_wgt_th1
HI258_write_cmos_sensor(0x92, 0x28); //rg5_gain_wgt_th2
HI258_write_cmos_sensor(0x93, 0x3f); //rg5_gain_wgt_th3
HI258_write_cmos_sensor(0x94, 0x3f); //rg5_gain_wgt_th4
HI258_write_cmos_sensor(0x95, 0x28); //rg5_lum_sp1
HI258_write_cmos_sensor(0x96, 0x50); //rg5_lum_sp2
HI258_write_cmos_sensor(0x97, 0x80); //rg5_lum_sp3
HI258_write_cmos_sensor(0x98, 0xb0); //rg5_lum_sp4
HI258_write_cmos_sensor(0x99, 0x1a); //rg6_gain_wgt_th1
HI258_write_cmos_sensor(0x9a, 0x28); //rg6_gain_wgt_th2
HI258_write_cmos_sensor(0x9b, 0x3f); //rg6_gain_wgt_th3
HI258_write_cmos_sensor(0x9c, 0x3f); //rg6_gain_wgt_th4
HI258_write_cmos_sensor(0x9d, 0x28); //rg6_lum_sp1
HI258_write_cmos_sensor(0x9e, 0x50); //rg6_lum_sp2
HI258_write_cmos_sensor(0x9f, 0x80); //rg6_lum_sp3
HI258_write_cmos_sensor(0xa0, 0xb0); //rg6_lum_sp4
HI258_write_cmos_sensor(0xa1, 0x00); //mcmc_allgain_ctl

HI258_write_cmos_sensor(0xa2, 0x00);
HI258_write_cmos_sensor(0xe5, 0x80); //add 20120709 Bit[7] On MCMC --> YC2D_LPF

/////// PAGE 20 START ///////
HI258_write_cmos_sensor(0x03, 0x20);
HI258_write_cmos_sensor(0x10, 0x1c);
HI258_write_cmos_sensor(0x11, 0x0c);//14
HI258_write_cmos_sensor(0x18, 0x30);
HI258_write_cmos_sensor(0x20, 0x25); //8x8 Ae weight 0~7 Outdoor / Weight Outdoor On B[5]
HI258_write_cmos_sensor(0x21, 0x30);
HI258_write_cmos_sensor(0x22, 0x10);
HI258_write_cmos_sensor(0x23, 0x00);

HI258_write_cmos_sensor(0x28, 0xf7);
HI258_write_cmos_sensor(0x29, 0x0d);
HI258_write_cmos_sensor(0x2a, 0xff);
HI258_write_cmos_sensor(0x2b, 0x04); //Adaptive Off,1/100 Flicker

HI258_write_cmos_sensor(0x2c, 0x83); //AE After CI
HI258_write_cmos_sensor(0x2d, 0xe3); 
HI258_write_cmos_sensor(0x2e, 0x13);
HI258_write_cmos_sensor(0x2f, 0x0b);

HI258_write_cmos_sensor(0x30, 0x78);
HI258_write_cmos_sensor(0x31, 0xd7);
HI258_write_cmos_sensor(0x32, 0x10);
HI258_write_cmos_sensor(0x33, 0x2e);
HI258_write_cmos_sensor(0x34, 0x20);
HI258_write_cmos_sensor(0x35, 0xd4);
HI258_write_cmos_sensor(0x36, 0xfe);
HI258_write_cmos_sensor(0x37, 0x32);
HI258_write_cmos_sensor(0x38, 0x04);
HI258_write_cmos_sensor(0x39, 0x22);
HI258_write_cmos_sensor(0x3a, 0xde);
HI258_write_cmos_sensor(0x3b, 0x22);
HI258_write_cmos_sensor(0x3c, 0xde);
HI258_write_cmos_sensor(0x3d, 0xe1);

HI258_write_cmos_sensor(0x3e, 0xc9); //Option of changing Exp max
HI258_write_cmos_sensor(0x41, 0x23); //Option of changing Exp max

HI258_write_cmos_sensor(0x50, 0x45);
HI258_write_cmos_sensor(0x51, 0x88);

HI258_write_cmos_sensor(0x56, 0x1f); // for tracking
HI258_write_cmos_sensor(0x57, 0xa6); // for tracking
HI258_write_cmos_sensor(0x58, 0x1a); // for tracking
HI258_write_cmos_sensor(0x59, 0x7a); // for tracking

HI258_write_cmos_sensor(0x5a, 0x04);
HI258_write_cmos_sensor(0x5b, 0x04);

HI258_write_cmos_sensor(0x5e, 0xc7);
HI258_write_cmos_sensor(0x5f, 0x95);

HI258_write_cmos_sensor(0x62, 0x10);
HI258_write_cmos_sensor(0x63, 0xc0);
HI258_write_cmos_sensor(0x64, 0x10);
HI258_write_cmos_sensor(0x65, 0x8a);
HI258_write_cmos_sensor(0x66, 0x58);
HI258_write_cmos_sensor(0x67, 0x58);

HI258_write_cmos_sensor(0x70, 0x58); //6c
HI258_write_cmos_sensor(0x71, 0x89); //81(+4),89(-4)

HI258_write_cmos_sensor(0x76, 0x32);
HI258_write_cmos_sensor(0x77, 0xa1);
HI258_write_cmos_sensor(0x78, 0x22); //24
HI258_write_cmos_sensor(0x79, 0x30); // Y Target 70 => 25, 72 => 26 //
HI258_write_cmos_sensor(0x7a, 0x23); //23
HI258_write_cmos_sensor(0x7b, 0x22); //22
HI258_write_cmos_sensor(0x7d, 0x23);

HI258_write_cmos_sensor(0x83, 0x02); //EXP Normal 33.33 fps 
HI258_write_cmos_sensor(0x84, 0xbd); 
HI258_write_cmos_sensor(0x85, 0x5e); 
HI258_write_cmos_sensor(0x86, 0x02); //EXPMin 11428.57 fps
HI258_write_cmos_sensor(0x87, 0x0d); 
HI258_write_cmos_sensor(0x88, 0x0c); //EXP Max 60hz 7.50 fps 
HI258_write_cmos_sensor(0x89, 0x2d); 
HI258_write_cmos_sensor(0x8a, 0x30); 
HI258_write_cmos_sensor(0xa5, 0x0b); //EXP Max 50hz 7.69 fps 
HI258_write_cmos_sensor(0xa6, 0xdf); 
HI258_write_cmos_sensor(0xa7, 0x42); 
HI258_write_cmos_sensor(0x8B, 0xe9); //EXP100 
HI258_write_cmos_sensor(0x8C, 0xca); 
HI258_write_cmos_sensor(0x8D, 0xc2); //EXP120 
HI258_write_cmos_sensor(0x8E, 0xd3); 
HI258_write_cmos_sensor(0x9c, 0x1c); //EXP Limit 816.33 fps 
HI258_write_cmos_sensor(0x9d, 0xb6); 
HI258_write_cmos_sensor(0x9e, 0x02); //EXP Unit 
HI258_write_cmos_sensor(0x9f, 0x0d); 
HI258_write_cmos_sensor(0xa3, 0x00); //Outdoor Int 
HI258_write_cmos_sensor(0xa4, 0xd2); 

HI258_write_cmos_sensor(0xb0, 0x15);
HI258_write_cmos_sensor(0xb1, 0x14);
HI258_write_cmos_sensor(0xb2, 0x80);
HI258_write_cmos_sensor(0xb3, 0x15);
HI258_write_cmos_sensor(0xb4, 0x16);
HI258_write_cmos_sensor(0xb5, 0x3c);
HI258_write_cmos_sensor(0xb6, 0x29);
HI258_write_cmos_sensor(0xb7, 0x23);
HI258_write_cmos_sensor(0xb8, 0x20);
HI258_write_cmos_sensor(0xb9, 0x1e);
HI258_write_cmos_sensor(0xba, 0x1c);
HI258_write_cmos_sensor(0xbb, 0x1b);
HI258_write_cmos_sensor(0xbc, 0x1b);
HI258_write_cmos_sensor(0xbd, 0x1a);

HI258_write_cmos_sensor(0xc0, 0x10);
HI258_write_cmos_sensor(0xc1, 0x40);
HI258_write_cmos_sensor(0xc2, 0x40);
HI258_write_cmos_sensor(0xc3, 0x40);
HI258_write_cmos_sensor(0xc4, 0x06);

HI258_write_cmos_sensor(0xc6, 0x80); //Exp max 1frame target AG

HI258_write_cmos_sensor(0xc8, 0x80);
HI258_write_cmos_sensor(0xc9, 0x80);
///// PAGE 20 END /////

///// PAGE 21 START /////
HI258_write_cmos_sensor(0x03, 0x21); //page 21

//Indoor Weight
HI258_write_cmos_sensor(0x20, 0x11);
HI258_write_cmos_sensor(0x21, 0x11);
HI258_write_cmos_sensor(0x22, 0x11);
HI258_write_cmos_sensor(0x23, 0x11);
HI258_write_cmos_sensor(0x24, 0x14);
HI258_write_cmos_sensor(0x25, 0x44);
HI258_write_cmos_sensor(0x26, 0x44);
HI258_write_cmos_sensor(0x27, 0x41);
HI258_write_cmos_sensor(0x28, 0x14);
HI258_write_cmos_sensor(0x29, 0x44);
HI258_write_cmos_sensor(0x2a, 0x44);
HI258_write_cmos_sensor(0x2b, 0x41);
HI258_write_cmos_sensor(0x2c, 0x14);
HI258_write_cmos_sensor(0x2d, 0x47);
HI258_write_cmos_sensor(0x2e, 0x74);
HI258_write_cmos_sensor(0x2f, 0x41);
HI258_write_cmos_sensor(0x30, 0x14);
HI258_write_cmos_sensor(0x31, 0x47);
HI258_write_cmos_sensor(0x32, 0x74);
HI258_write_cmos_sensor(0x33, 0x41);
HI258_write_cmos_sensor(0x34, 0x14);
HI258_write_cmos_sensor(0x35, 0x44);
HI258_write_cmos_sensor(0x36, 0x44);
HI258_write_cmos_sensor(0x37, 0x41);
HI258_write_cmos_sensor(0x38, 0x14);
HI258_write_cmos_sensor(0x39, 0x44);
HI258_write_cmos_sensor(0x3a, 0x44);
HI258_write_cmos_sensor(0x3b, 0x41);
HI258_write_cmos_sensor(0x3c, 0x11);
HI258_write_cmos_sensor(0x3d, 0x11);
HI258_write_cmos_sensor(0x3e, 0x11);
HI258_write_cmos_sensor(0x3f, 0x11);

//Outdoor Weight
HI258_write_cmos_sensor(0x40, 0x11);
HI258_write_cmos_sensor(0x41, 0x11);
HI258_write_cmos_sensor(0x42, 0x11);
HI258_write_cmos_sensor(0x43, 0x11);
HI258_write_cmos_sensor(0x44, 0x14);
HI258_write_cmos_sensor(0x45, 0x44);
HI258_write_cmos_sensor(0x46, 0x44);
HI258_write_cmos_sensor(0x47, 0x41);
HI258_write_cmos_sensor(0x48, 0x14);
HI258_write_cmos_sensor(0x49, 0x44);
HI258_write_cmos_sensor(0x4a, 0x44);
HI258_write_cmos_sensor(0x4b, 0x41);
HI258_write_cmos_sensor(0x4c, 0x14);
HI258_write_cmos_sensor(0x4d, 0x47);
HI258_write_cmos_sensor(0x4e, 0x74);
HI258_write_cmos_sensor(0x4f, 0x41);
HI258_write_cmos_sensor(0x50, 0x14);
HI258_write_cmos_sensor(0x51, 0x47);
HI258_write_cmos_sensor(0x52, 0x74);
HI258_write_cmos_sensor(0x53, 0x41);
HI258_write_cmos_sensor(0x54, 0x14);
HI258_write_cmos_sensor(0x55, 0x44);
HI258_write_cmos_sensor(0x56, 0x44);
HI258_write_cmos_sensor(0x57, 0x41);
HI258_write_cmos_sensor(0x58, 0x14);
HI258_write_cmos_sensor(0x59, 0x44);
HI258_write_cmos_sensor(0x5a, 0x44);
HI258_write_cmos_sensor(0x5b, 0x41);
HI258_write_cmos_sensor(0x5c, 0x11);
HI258_write_cmos_sensor(0x5d, 0x11);
HI258_write_cmos_sensor(0x5e, 0x11);
HI258_write_cmos_sensor(0x5f, 0x11);


///// PAGE 22 START /////
HI258_write_cmos_sensor(0x03, 0x22); //page 22
HI258_write_cmos_sensor(0x10, 0xfd);
HI258_write_cmos_sensor(0x11, 0x2e);
HI258_write_cmos_sensor(0x19, 0x02);
HI258_write_cmos_sensor(0x20, 0x30); //For AWB Speed
HI258_write_cmos_sensor(0x21, 0x80);
HI258_write_cmos_sensor(0x22, 0x00);
HI258_write_cmos_sensor(0x23, 0x00);
HI258_write_cmos_sensor(0x24, 0x01);
HI258_write_cmos_sensor(0x25, 0x4f); //2013-09-13 AWB Hunting

HI258_write_cmos_sensor(0x30, 0x80);
HI258_write_cmos_sensor(0x31, 0x80);
HI258_write_cmos_sensor(0x38, 0x11);
HI258_write_cmos_sensor(0x39, 0x34);
HI258_write_cmos_sensor(0x40, 0xe4); //Stb Yth
HI258_write_cmos_sensor(0x41, 0x33); //Stb cdiff
HI258_write_cmos_sensor(0x42, 0x22); //Stb csum
HI258_write_cmos_sensor(0x43, 0xf3); //Unstb Yth
HI258_write_cmos_sensor(0x44, 0x55); //Unstb cdiff55
HI258_write_cmos_sensor(0x45, 0x33); //Unstb csum
HI258_write_cmos_sensor(0x46, 0x00);
HI258_write_cmos_sensor(0x47, 0x09); //2013-09-13 AWB Hunting
HI258_write_cmos_sensor(0x48, 0x00); //2013-09-13 AWB Hunting
HI258_write_cmos_sensor(0x49, 0x0a);

HI258_write_cmos_sensor(0x60, 0x04);
HI258_write_cmos_sensor(0x61, 0xc4);
HI258_write_cmos_sensor(0x62, 0x04);
HI258_write_cmos_sensor(0x63, 0x92);
HI258_write_cmos_sensor(0x66, 0x04);
HI258_write_cmos_sensor(0x67, 0xc4);
HI258_write_cmos_sensor(0x68, 0x04);
HI258_write_cmos_sensor(0x69, 0x92);

HI258_write_cmos_sensor(0x80, 0x36);
HI258_write_cmos_sensor(0x81, 0x20);
HI258_write_cmos_sensor(0x82, 0x2a);

HI258_write_cmos_sensor(0x83, 0x58);
HI258_write_cmos_sensor(0x84, 0x16);
HI258_write_cmos_sensor(0x85, 0x4f);
HI258_write_cmos_sensor(0x86, 0x15);

HI258_write_cmos_sensor(0x87, 0x3b);
HI258_write_cmos_sensor(0x88, 0x30);
HI258_write_cmos_sensor(0x89, 0x29);
HI258_write_cmos_sensor(0x8a, 0x18);

HI258_write_cmos_sensor(0x8b, 0x3c);
HI258_write_cmos_sensor(0x8c, 0x32);
HI258_write_cmos_sensor(0x8d, 0x2a);
HI258_write_cmos_sensor(0x8e, 0x1b);

HI258_write_cmos_sensor(0x8f, 0x4d);
HI258_write_cmos_sensor(0x90, 0x46);
HI258_write_cmos_sensor(0x91, 0x40);
HI258_write_cmos_sensor(0x92, 0x3a);
HI258_write_cmos_sensor(0x93, 0x2f);
HI258_write_cmos_sensor(0x94, 0x21);
HI258_write_cmos_sensor(0x95, 0x19);
HI258_write_cmos_sensor(0x96, 0x16);
HI258_write_cmos_sensor(0x97, 0x13);
HI258_write_cmos_sensor(0x98, 0x12);
HI258_write_cmos_sensor(0x99, 0x11);
HI258_write_cmos_sensor(0x9a, 0x10);

HI258_write_cmos_sensor(0x9b, 0x88);
HI258_write_cmos_sensor(0x9c, 0x77);
HI258_write_cmos_sensor(0x9d, 0x48);
HI258_write_cmos_sensor(0x9e, 0x38);
HI258_write_cmos_sensor(0x9f, 0x30);

HI258_write_cmos_sensor(0xa0, 0x70);
HI258_write_cmos_sensor(0xa1, 0x54);
HI258_write_cmos_sensor(0xa2, 0x6f);
HI258_write_cmos_sensor(0xa3, 0xff);

HI258_write_cmos_sensor(0xa4, 0x14); //1536fps
HI258_write_cmos_sensor(0xa5, 0x2c); //698fps
HI258_write_cmos_sensor(0xa6, 0xcf); //148fps

HI258_write_cmos_sensor(0xad, 0x2e);
HI258_write_cmos_sensor(0xae, 0x2a);

HI258_write_cmos_sensor(0xaf, 0x28); //Low temp Rgain
HI258_write_cmos_sensor(0xb0, 0x26); //Low temp Rgain

HI258_write_cmos_sensor(0xb1, 0x08);
HI258_write_cmos_sensor(0xb4, 0xbf); //For Tracking AWB Weight
HI258_write_cmos_sensor(0xb8, 0x91); //(0+,1-)High Cb , (0+,1-)Low Cr
HI258_write_cmos_sensor(0xb9, 0xb0);
/////// PAGE 22 END ///////

//// MIPI Setting /////
HI258_write_cmos_sensor(0x03, 0x48);
HI258_write_cmos_sensor(0x39, 0x4f); //lvds_bias_ctl    [2:0]mipi_tx_bias   [4:3]mipi_vlp_sel   [6:5]mipi_vcm_sel
HI258_write_cmos_sensor(0x10, 0x1c); //lvds_ctl_1       [5]mipi_pad_disable [4]lvds_en [0]serial_data_len 
HI258_write_cmos_sensor(0x11, 0x10); //lvds_ctl_2       [4]mipi continous mode setting
//HI258_write_cmos_sensor(0x14, 0x00} //ser_out_ctl_1  [2:0]serial_sout_a_phase   [6:4]serial_cout_a_phase

HI258_write_cmos_sensor(0x16, 0x00); //lvds_inout_ctl1  [0]vs_packet_pos_sel [1]data_neg_sel [4]first_vsync_end_opt
HI258_write_cmos_sensor(0x18, 0x80); //lvds_inout_ctl3
HI258_write_cmos_sensor(0x19, 0x00); //lvds_inout_ctl4
HI258_write_cmos_sensor(0x1a, 0xf0); //lvds_time_ctl
HI258_write_cmos_sensor(0x24, 0x1e); //long_packet_id

//====== MIPI Timing Setting =========
HI258_write_cmos_sensor(0x36, 0x01); //clk_tlpx_time_dp
HI258_write_cmos_sensor(0x37, 0x05); //clk_tlpx_time_dn
HI258_write_cmos_sensor(0x34, 0x04); //clk_prepare_time
HI258_write_cmos_sensor(0x32, 0x15); //clk_zero_time
HI258_write_cmos_sensor(0x35, 0x04); //clk_trail_time
HI258_write_cmos_sensor(0x33, 0x0d); //clk_post_time

HI258_write_cmos_sensor(0x1c, 0x01); //tlps_time_l_dp
HI258_write_cmos_sensor(0x1d, 0x0b); //tlps_time_l_dn
HI258_write_cmos_sensor(0x1e, 0x06); //hs_zero_time
HI258_write_cmos_sensor(0x1f, 0x09); //hs_trail_time

//long_packet word count 
HI258_write_cmos_sensor(0x30, 0x06);
HI258_write_cmos_sensor(0x31, 0x40); //long_packet word count

/////// PAGE 20 ///////
HI258_write_cmos_sensor(0x03, 0x20);
HI258_write_cmos_sensor(0x10, 0x9c); //AE On 50hz

/////// PAGE 22 ///////
HI258_write_cmos_sensor(0x03, 0x22);
HI258_write_cmos_sensor(0x10, 0xe9); //AWB On

HI258_write_cmos_sensor(0x03, 0x00);
HI258_write_cmos_sensor(0x01, 0x00);
}

kal_uint32 HI258_read_shutter(void)
{
    kal_uint8 temp_reg0, temp_reg1, temp_reg2;
    kal_uint32 shutter;

    HI258_write_cmos_sensor(0x03, 0x20); 
    temp_reg0 = HI258_read_cmos_sensor(0x80); 
    temp_reg1 = HI258_read_cmos_sensor(0x81);
    temp_reg2 = HI258_read_cmos_sensor(0x82); 
    shutter = (temp_reg0 << 16) | (temp_reg1 << 8) | (temp_reg2 & 0xFF);

    return shutter;
}   

static void HI258_write_shutter(kal_uint16 shutter)
{
    SENSORDB("[HI258] %s \n",__func__);
    HI258_write_cmos_sensor(0x03, 0x20);
    HI258_write_cmos_sensor(0x83, shutter >> 16);			
    HI258_write_cmos_sensor(0x84, (shutter >> 8) & 0x00FF);	
    HI258_write_cmos_sensor(0x85, shutter & 0x0000FF);		
}    

void HI258_night_mode(kal_bool enable)	
{
    SENSORDB("[HI258] %s ==enable =%d \n",__func__,enable);
    if (HI258_sensor_cap_state == KAL_TRUE) 
    {
        return ;	
    }
    if (enable) 
    {

        if (HI258_Banding_setting == AE_FLICKER_MODE_50HZ) 
        {
            HI258_write_cmos_sensor(0x03,0x00); 	
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x01;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x1c);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x18, 0x38);

            HI258_write_cmos_sensor(0x83, 0x13); //EXP Normal 33.33 fps 
            HI258_write_cmos_sensor(0x84, 0xd6); 
            HI258_write_cmos_sensor(0x85, 0x20); 
            HI258_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI258_write_cmos_sensor(0x87, 0xf4); 
		          //EXP Max 60hz
            HI258_write_cmos_sensor(0x88, 0x13); //EXP Max 5 fps
            HI258_write_cmos_sensor(0x89, 0xc6);
            HI258_write_cmos_sensor(0x8a, 0x80);

			        //EXP Max 50hz
            HI258_write_cmos_sensor(0xa5, 0x13); //EXP Max 5 fps
            HI258_write_cmos_sensor(0xa6, 0xd6);
            HI258_write_cmos_sensor(0xa7, 0x20);

            HI258_write_cmos_sensor(0x03, 0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x00;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x9c);

            HI258_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        } 
        else
        {
            HI258_write_cmos_sensor(0x03,0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x01;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x1c);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x18, 0x38);

            HI258_write_cmos_sensor(0x83, 0x13); //EXP Normal 33.33 fps 
            HI258_write_cmos_sensor(0x84, 0xc6); 
            HI258_write_cmos_sensor(0x85, 0x80); 
            HI258_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI258_write_cmos_sensor(0x87, 0xf4);
			
		          //EXP Max 60hz
            HI258_write_cmos_sensor(0x88, 0x13); //EXP Max 5 fps
            HI258_write_cmos_sensor(0x89, 0xc6);
            HI258_write_cmos_sensor(0x8a, 0x80);

			        //EXP Max 50hz
            HI258_write_cmos_sensor(0xa5, 0x13); //EXP Max 5 fps
            HI258_write_cmos_sensor(0xa6, 0xd6);
            HI258_write_cmos_sensor(0xa7, 0x20);

            HI258_write_cmos_sensor(0x03, 0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x00;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x8c);

            HI258_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        }
    } 
    else 
    {

        if (HI258_Banding_setting == AE_FLICKER_MODE_50HZ)
        {
            HI258_write_cmos_sensor(0x03,0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x01;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);
            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x1c);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x18, 0x38);

            HI258_write_cmos_sensor(0x83, 0x0b); //EXP Normal 33.33 fps
            HI258_write_cmos_sensor(0x84, 0xe6);
            HI258_write_cmos_sensor(0x85, 0xe0);
            HI258_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI258_write_cmos_sensor(0x87, 0xf4);
		          //EXP Max 60hz
            HI258_write_cmos_sensor(0x88, 0x0c); //EXP Max 8.33 fps
            HI258_write_cmos_sensor(0x89, 0x5c);
            HI258_write_cmos_sensor(0x8a, 0x10);

			        //EXP Max 50hz
            HI258_write_cmos_sensor(0xa5, 0x0b); //EXP Max 8.33 fps
            HI258_write_cmos_sensor(0xa6, 0xe6);
            HI258_write_cmos_sensor(0xa7, 0xe0);
			

            HI258_write_cmos_sensor(0x03, 0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x00;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x9c);

            HI258_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        }
        else
        {
            HI258_write_cmos_sensor(0x03,0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x01;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);
            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x1c);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x18, 0x38);

            HI258_write_cmos_sensor(0x83, 0x0c); //EXP Normal 33.33 fps
            HI258_write_cmos_sensor(0x84, 0x5c);
            HI258_write_cmos_sensor(0x85, 0x10);
            HI258_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI258_write_cmos_sensor(0x87, 0xf4);
			           //EXP Max 60hz
            HI258_write_cmos_sensor(0x88, 0x0c); //EXP Max 8.33 fps
            HI258_write_cmos_sensor(0x89, 0x5c);
            HI258_write_cmos_sensor(0x8a, 0x10);
			           //EXP Max 50hz
            HI258_write_cmos_sensor(0xa5, 0x0b); //EXP Max 8.33 fps
            HI258_write_cmos_sensor(0xa6, 0xe6);
            HI258_write_cmos_sensor(0xa7, 0xe0);

            HI258_write_cmos_sensor(0x03, 0x00);
            HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
            HI258_Sleep_Mode |= 0x00;
            HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

            HI258_write_cmos_sensor(0x03, 0x20);
            HI258_write_cmos_sensor(0x10, 0x8c);

            HI258_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        }
    }
}

void HI258_Initial_Cmds(void)
{
    kal_uint16 i,cnt;
    HI258_Init_Cmds();
}

UINT32 HI258Open(void)
{
    SENSORDB("[HI258] %s \n",__func__);
    volatile signed char i;
    kal_uint32 sensor_id=0;
    kal_uint8 temp_sccb_addr = 0;

    HI258_write_cmos_sensor(0x03, 0x00);
    HI258_write_cmos_sensor(0x01, 0xf1);
    HI258_write_cmos_sensor(0x01, 0xf3);
    HI258_write_cmos_sensor(0x01, 0xf1);

    HI258_write_cmos_sensor(0x01, 0xf1);
    HI258_write_cmos_sensor(0x01, 0xf3);
    HI258_write_cmos_sensor(0x01, 0xf1);
    
    do{
        for (i=0; i < 3; i++)
        {
            sensor_id = HI258_read_cmos_sensor(0x04);
printk("[HI258YUV]:Read Sensor ID succ:0x%x\n", sensor_id);  
            if (sensor_id == HI258_SENSOR_ID)
            {
#ifdef HI258_DEBUG
                printk("[HI258YUV]:Read Sensor ID succ:0x%x\n", sensor_id);  
#endif
                break;
            }
        }

        mdelay(20);
    }while(0);

    if (sensor_id != HI258_SENSOR_ID)
    {
	
#ifdef HI258_DEBUG
        printk("[HI258YUV]:Read Sensor ID fail:0x%x\n", sensor_id);  
#endif
        return ERROR_SENSOR_CONNECT_FAIL;
    }
#ifdef HI258_DEBUG
    printk("[HI258YUV]:Read Sensor ID pass:0x%x\n", sensor_id);
#endif

    HI258_Initial_Cmds();
    //HI258_set_mirror_flip(3);

    return ERROR_NONE;
}	

UINT32 HI258Close(void)
{
    //CISModulePowerOn(FALSE);
    //kdModulePowerOn((CAMERA_DUAL_CAMERA_SENSOR_ENUM) g_currDualSensorIdx, g_currSensorName,false, CAMERA_HW_DRVNAME);
    return ERROR_NONE;
}	

static void HI258_set_mirror_flip(kal_uint8 image_mirror)
{
    kal_uint8 HI258_HV_Mirror;
    SENSORDB("[HI258] %s \n",__func__);
    HI258_write_cmos_sensor(0x03,0x00); 	
    HI258_HV_Mirror = (HI258_read_cmos_sensor(0x11) & 0xfc);

    switch (image_mirror) {
        case IMAGE_NORMAL:		
            HI258_HV_Mirror |= 0x03;
            break;
        case IMAGE_H_MIRROR:
            HI258_HV_Mirror |= 0x01;
            break;
        case IMAGE_V_MIRROR:
            HI258_HV_Mirror |= 0x02; 
            break;
        case IMAGE_HV_MIRROR:
            HI258_HV_Mirror |= 0x00; 
            break;
        default:
            break;
    }
    HI258_write_cmos_sensor(0x11, HI258_HV_Mirror);
}

UINT32 HI258Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{	
    SENSORDB("[HI258] %s \n",__func__);
    kal_uint16  iStartX = 0, iStartY = 0;

    HI258_sensor_cap_state = KAL_FALSE;
    HI258_sensor_pclk=390;

    HI258_gPVmode = KAL_TRUE;

    if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        HI258_VEDIO_encode_mode = KAL_TRUE;
    }
    else
    {
        HI258_VEDIO_encode_mode = KAL_FALSE;
    }

    HI258_write_cmos_sensor(0x03,0x00); 	
    HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
    HI258_Sleep_Mode |= 0x01;
    HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

    HI258_write_cmos_sensor(0x03, 0x20); 
    HI258_write_cmos_sensor(0x10, 0x1c);
    HI258_write_cmos_sensor(0x03, 0x22);
    HI258_write_cmos_sensor(0x10, 0x69);

    HI258_write_cmos_sensor(0x03, 0x00);
    HI258_write_cmos_sensor(0x10, 0x10); // preview mode off
    HI258_write_cmos_sensor(0x12, 0x04);

    HI258_write_cmos_sensor(0x20, 0x00); 
    HI258_write_cmos_sensor(0x21, 0x04);
    HI258_write_cmos_sensor(0x22, 0x00);
    HI258_write_cmos_sensor(0x23, 0x07);

    HI258_write_cmos_sensor(0x40, 0x01); //Hblank 378
    HI258_write_cmos_sensor(0x41, 0x78); 
    HI258_write_cmos_sensor(0x42, 0x00); //Vblank 20
    HI258_write_cmos_sensor(0x43, 0x14); 

    HI258_write_cmos_sensor(0x03, 0x20); 
    //HI258_write_cmos_sensor(0x18, 0x38);

    HI258_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
    HI258_write_cmos_sensor(0x87, 0xf4); 

    HI258_write_cmos_sensor(0x8B, 0xfd); //EXP100 
    HI258_write_cmos_sensor(0x8C, 0xe8); 
    HI258_write_cmos_sensor(0x8D, 0xd2); //EXP120 
    HI258_write_cmos_sensor(0x8E, 0xf0);

    HI258_write_cmos_sensor(0x9c, 0x21); //EXP Limit 1083.33fps 
    HI258_write_cmos_sensor(0x9d, 0x34); 
    HI258_write_cmos_sensor(0x9e, 0x01); //EXP Unit 
    HI258_write_cmos_sensor(0x9f, 0xf4); 

	//PLL Setting
	HI258_write_cmos_sensor(0x03, 0x00); 
	HI258_write_cmos_sensor(0xd0, 0x05); //PLL pre_div 1/6 = 4 Mhz
	HI258_write_cmos_sensor(0xd1, 0x30); //PLL maim_div 
	HI258_write_cmos_sensor(0xd2, 0x05); //isp_div[1:0] mipi_4x_div[3:2]  mipi_1x_div[4] pll_bias_opt[7:5]	  
	HI258_write_cmos_sensor(0xd3, 0x20); //isp_clk_inv[0]  mipi_4x_inv[1]  mipi_1x_inv[2]
	HI258_write_cmos_sensor(0xd0, 0x85);
	HI258_write_cmos_sensor(0xd0, 0x85);
	HI258_write_cmos_sensor(0xd0, 0x85);
	HI258_write_cmos_sensor(0xd0, 0x95);
	
	HI258_write_cmos_sensor(0x03, 0x00); //Dummy 750us
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);
	HI258_write_cmos_sensor(0x03, 0x00);


    // MIPI TX Setting //
    HI258_write_cmos_sensor(0x03, 0x48);
    HI258_write_cmos_sensor(0x30, 0x06);
    HI258_write_cmos_sensor(0x31, 0x40);



	
    HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
    HI258_Sleep_Mode |= 0x00;
    HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

    HI258_write_cmos_sensor(0x03, 0x20);//page 20
    HI258_write_cmos_sensor(0x10, 0x9c);//AE ON

    HI258_write_cmos_sensor(0x03, 0x22);
    HI258_write_cmos_sensor(0x10, 0xe9);//AWB ON

    //HI258_write_cmos_sensor(0x03, 0x20);
    //HI258_write_cmos_sensor(0x18, 0x30);

    image_window->GrabStartX = iStartX;
    image_window->GrabStartY = iStartY;
    image_window->ExposureWindowWidth = HI258_IMAGE_SENSOR_PV_WIDTH - 16;
    image_window->ExposureWindowHeight = HI258_IMAGE_SENSOR_PV_HEIGHT - 12;
    msleep(10);
    // copy sensor_config_data
    memcpy(&HI258SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}	

UINT32 HI258Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint8 temp_AE_reg;
    kal_uint8 CLK_DIV_REG = 0;
    SENSORDB("[HI258] %s \n",__func__);
    HI258_sensor_cap_state = KAL_TRUE;
#if 0
    if ((image_window->ImageTargetWidth<=HI258_IMAGE_SENSOR_PV_WIDTH)&&
        (image_window->ImageTargetHeight<=HI258_IMAGE_SENSOR_PV_HEIGHT))
    {
        /* Less than PV Mode */
        HI258_gPVmode=KAL_TRUE;
        HI258_capture_pclk_in_M = HI258_preview_pclk_in_M;   //Don't change the clk

        HI258_write_cmos_sensor(0x03, 0x00);

        HI258_write_cmos_sensor(0x20, 0x00); 
        HI258_write_cmos_sensor(0x21, 0x04); 
        HI258_write_cmos_sensor(0x22, 0x00); 
        HI258_write_cmos_sensor(0x23, 0x07); 

        HI258_write_cmos_sensor(0x40, 0x01); //Hblank 360
        HI258_write_cmos_sensor(0x41, 0x68); 
        HI258_write_cmos_sensor(0x42, 0x00); //Vblank 18
        HI258_write_cmos_sensor(0x43, 0x12); 

        HI258_write_cmos_sensor(0x03, 0x10);
        HI258_write_cmos_sensor(0x3f, 0x00);	

        //Page12
        HI258_write_cmos_sensor(0x03, 0x12); //Function
        HI258_write_cmos_sensor(0x20, 0x0f);
        HI258_write_cmos_sensor(0x21, 0x0f);
        HI258_write_cmos_sensor(0x90, 0x5d);  

        //Page13
        HI258_write_cmos_sensor(0x03, 0x13); //Function
        HI258_write_cmos_sensor(0x80, 0xfd); //Function

        // 800*600	
        HI258_write_cmos_sensor(0x03, 0x00);
        HI258_write_cmos_sensor(0x10, 0x11);

        HI258_write_cmos_sensor(0x03, 0x48);
        HI258_write_cmos_sensor(0x72, 0x81); 
        HI258_write_cmos_sensor(0x30, 0x06);
        HI258_write_cmos_sensor(0x31, 0x40);

        HI258_write_cmos_sensor(0x03, 0x20);
        HI258_pv_HI258_exposure_lines = (HI258_read_cmos_sensor(0x80) << 16)|(HI258_read_cmos_sensor(0x81) << 8)|HI258_read_cmos_sensor(0x82);

        HI258_cp_HI258_exposure_lines=HI258_pv_HI258_exposure_lines;	

        if(HI258_cp_HI258_exposure_lines<1)
            HI258_cp_HI258_exposure_lines=1;

        HI258_write_cmos_sensor(0x03, 0x20); 
        HI258_write_cmos_sensor(0x83, HI258_cp_HI258_exposure_lines >> 16);
        HI258_write_cmos_sensor(0x84, (HI258_cp_HI258_exposure_lines >> 8) & 0x000000FF);
        HI258_write_cmos_sensor(0x85, HI258_cp_HI258_exposure_lines & 0x000000FF);

        image_window->GrabStartX = 1;
        image_window->GrabStartY = 1;
        image_window->ExposureWindowWidth= HI258_IMAGE_SENSOR_PV_WIDTH - 16;
        image_window->ExposureWindowHeight = HI258_IMAGE_SENSOR_PV_HEIGHT - 12;
    }
    else 
#endif
    {    
        /* 2M FULL Mode */
        HI258_gPVmode = KAL_FALSE;

        HI258_write_cmos_sensor(0x03,0x00); 	
        HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
        HI258_Sleep_Mode |= 0x01;
        HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);

        CLK_DIV_REG=(HI258_read_cmos_sensor(0x12)&0xFc);    // don't divide,PCLK=48M
        CLK_DIV_REG |= 0x00;
        //read the shutter (manual exptime)
        HI258_write_cmos_sensor(0x03, 0x20);
        HI258_pv_HI258_exposure_lines = (HI258_read_cmos_sensor(0x80) << 16)|(HI258_read_cmos_sensor(0x81) << 8)|HI258_read_cmos_sensor(0x82);

        HI258_cp_HI258_exposure_lines = HI258_pv_HI258_exposure_lines;
		
        HI258_write_cmos_sensor(0x03, 0x00);

        HI258_write_cmos_sensor(0x20, 0x00); 
        HI258_write_cmos_sensor(0x21, 0x0a); 
        HI258_write_cmos_sensor(0x22, 0x00); 
        HI258_write_cmos_sensor(0x23, 0x0a); 

        HI258_write_cmos_sensor(0x40, 0x01); //Hblank 360
        HI258_write_cmos_sensor(0x41, 0x78); 
        HI258_write_cmos_sensor(0x42, 0x00); //Vblank 18
        HI258_write_cmos_sensor(0x43, 0x14); 

	
        // 1600*1200	
        HI258_write_cmos_sensor(0x03, 0x00);
        HI258_write_cmos_sensor(0x10, 0x00);
        HI258_write_cmos_sensor(0xd2, 0x01); //isp_div[1:0] mipi_4x_div[3:2]  mipi_1x_div[4] pll_bias_opt[7:5]    

        HI258_write_cmos_sensor(0x03, 0x48);
        HI258_write_cmos_sensor(0x72, 0x81); 
        HI258_write_cmos_sensor(0x30, 0x0c);
        HI258_write_cmos_sensor(0x31, 0x80);
#if 0
        if ((image_window->ImageTargetWidth<=HI258_IMAGE_SENSOR_FULL_WIDTH)&&
        (image_window->ImageTargetHeight<=HI258_IMAGE_SENSOR_FULL_HEIGHT))
        {
            HI258_capture_pclk_in_M = 520;
            HI258_sensor_pclk = 520;                 
        }
        else//Interpolate to 3M
        {
            HI258_capture_pclk_in_M = 520;
            HI258_sensor_pclk = 520;                
        }
#endif

        HI258_write_cmos_sensor(0x03, 0x00); 
        HI258_write_cmos_sensor(0x12,/*CLK_DIV_REG*/ 0x04);

        if(HI258_cp_HI258_exposure_lines<1)
            HI258_cp_HI258_exposure_lines=1;

        HI258_write_cmos_sensor(0x03, 0x20); 
        HI258_write_cmos_sensor(0x83, HI258_cp_HI258_exposure_lines >> 16);
        HI258_write_cmos_sensor(0x84, (HI258_cp_HI258_exposure_lines >> 8) & 0x000000FF);
        HI258_write_cmos_sensor(0x85, HI258_cp_HI258_exposure_lines & 0x000000FF);
	
        HI258_write_cmos_sensor(0x03,0x00); 	
        HI258_Sleep_Mode = (HI258_read_cmos_sensor(0x01) & 0xfe);
        HI258_Sleep_Mode |= 0x00;
        HI258_write_cmos_sensor(0x01, HI258_Sleep_Mode);       
#if 0
        image_window->GrabStartX=1;
        image_window->GrabStartY=1;
        image_window->ExposureWindowWidth=HI258_IMAGE_SENSOR_FULL_WIDTH - 16;
        image_window->ExposureWindowHeight=HI258_IMAGE_SENSOR_FULL_HEIGHT - 12;
#endif
    }

    msleep(10);
    // copy sensor_config_data
    memcpy(&HI258SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    HI258_CAPATURE_FLAG = 1;
    HI258_CAPATUREB_FLAG = 1;
    return ERROR_NONE;
}

UINT32 HI258GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("[HI258] %s \n",__func__);
    pSensorResolution->SensorFullWidth=HI258_FULL_GRAB_WIDTH;  
    pSensorResolution->SensorFullHeight=HI258_FULL_GRAB_HEIGHT;
    pSensorResolution->SensorPreviewWidth=HI258_PV_GRAB_WIDTH;
    pSensorResolution->SensorPreviewHeight=HI258_PV_GRAB_HEIGHT;
    pSensorResolution->SensorVideoWidth=HI258_PV_GRAB_WIDTH;
    pSensorResolution->SensorVideoHeight=HI258_PV_GRAB_HEIGHT;
    return ERROR_NONE;
}	

UINT32 HI258GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[HI258] %s \n",__func__);
    pSensorInfo->SensorPreviewResolutionX=HI258_PV_GRAB_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=HI258_PV_GRAB_HEIGHT;
    pSensorInfo->SensorFullResolutionX=HI258_FULL_GRAB_WIDTH;
    pSensorInfo->SensorFullResolutionY=HI258_FULL_GRAB_HEIGHT;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;

    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;

    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;

#ifdef MIPI_INTERFACE
    pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_MIPI;
#else
    pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_PARALLEL;
#endif

    pSensorInfo->CaptureDelayFrame = 1; 
    pSensorInfo->PreviewDelayFrame = 1; 
    pSensorInfo->VideoDelayFrame = 1; 
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   	
	
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
            /*ergate-004*/
            pSensorInfo->SensorGrabStartX = HI258_PV_GRAB_START_X;//0; 
            pSensorInfo->SensorGrabStartY = HI258_PV_GRAB_START_Y;//0;    		
#ifdef MIPI_INTERFACE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE; 		
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;// 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;	// 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
#endif
            break;

        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=26;
            pSensorInfo->SensorClockDividCount=	3;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            /*ergate-004*/
            pSensorInfo->SensorGrabStartX = HI258_FULL_GRAB_START_X;//0; 
            pSensorInfo->SensorGrabStartY = HI258_FULL_GRAB_START_Y;//0;     		
#ifdef MIPI_INTERFACE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;//14 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
#endif
            break;

        default:
            pSensorInfo->SensorClockFreq=26;
            pSensorInfo->SensorClockDividCount=3;
            pSensorInfo->SensorClockRisingCount=0;
            pSensorInfo->SensorClockFallingCount=2;
            pSensorInfo->SensorPixelClockCount=3;
            pSensorInfo->SensorDataLatchCount=2;
            /*ergate-004*/
            pSensorInfo->SensorGrabStartX = HI258_PV_GRAB_START_X;//0; 
            pSensorInfo->SensorGrabStartY = HI258_PV_GRAB_START_Y;//0;     			
            break;
    }
    //HI258_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &HI258SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
}	

UINT32 HI258Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[HI258] %s ==ScenarioID=%d \n",__func__,ScenarioId);
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            HI258Preview(pImageWindow, pSensorConfigData);
            break;
	
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            HI258Capture(pImageWindow, pSensorConfigData);
            break;
	
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }

    return TRUE;
}	

BOOL HI258_set_param_wb(UINT16 para)
{
    SENSORDB("[HI258] %s ====para=%d \n",__func__,para);
    switch (para)
    {
        case AWB_MODE_AUTO:
            HI258_write_cmos_sensor(0x03, 0x22);			
            HI258_write_cmos_sensor(0x11, 0x2e);		
            HI258_write_cmos_sensor(0x83, 0x58);
            HI258_write_cmos_sensor(0x84, 0x16);
            HI258_write_cmos_sensor(0x85, 0x4f);
            HI258_write_cmos_sensor(0x86, 0x20);
            break;
        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
            HI258_write_cmos_sensor(0x03, 0x22);
            HI258_write_cmos_sensor(0x11, 0x28);
            HI258_write_cmos_sensor(0x80, 0x71);
            HI258_write_cmos_sensor(0x82, 0x2b);
            HI258_write_cmos_sensor(0x83, 0x72);
            HI258_write_cmos_sensor(0x84, 0x70);
            HI258_write_cmos_sensor(0x85, 0x2b);
            HI258_write_cmos_sensor(0x86, 0x28);
            break;
        case AWB_MODE_DAYLIGHT: //sunny
            HI258_write_cmos_sensor(0x03, 0x22);
            HI258_write_cmos_sensor(0x11, 0x28);		  
            HI258_write_cmos_sensor(0x80, 0x59);
            HI258_write_cmos_sensor(0x82, 0x29);
            HI258_write_cmos_sensor(0x83, 0x60);
            HI258_write_cmos_sensor(0x84, 0x50);
            HI258_write_cmos_sensor(0x85, 0x2f);
            HI258_write_cmos_sensor(0x86, 0x23);
            break;
        case AWB_MODE_INCANDESCENT: //office
            HI258_write_cmos_sensor(0x03, 0x22);
            HI258_write_cmos_sensor(0x11, 0x28);		  
            HI258_write_cmos_sensor(0x80, 0x29);
            HI258_write_cmos_sensor(0x82, 0x54);
            HI258_write_cmos_sensor(0x83, 0x2e);
            HI258_write_cmos_sensor(0x84, 0x23);
            HI258_write_cmos_sensor(0x85, 0x58);
            HI258_write_cmos_sensor(0x86, 0x4f);
            break;
        case AWB_MODE_TUNGSTEN: //home
            HI258_write_cmos_sensor(0x03, 0x22);
            HI258_write_cmos_sensor(0x80, 0x24);
            HI258_write_cmos_sensor(0x81, 0x20);
            HI258_write_cmos_sensor(0x82, 0x58);
            HI258_write_cmos_sensor(0x83, 0x27);
            HI258_write_cmos_sensor(0x84, 0x22);
            HI258_write_cmos_sensor(0x85, 0x58);
            HI258_write_cmos_sensor(0x86, 0x52);
            break;
        case AWB_MODE_FLUORESCENT:
            HI258_write_cmos_sensor(0x03, 0x22);
            HI258_write_cmos_sensor(0x11, 0x28);
            HI258_write_cmos_sensor(0x80, 0x41);
            HI258_write_cmos_sensor(0x82, 0x42);
            HI258_write_cmos_sensor(0x83, 0x44);
            HI258_write_cmos_sensor(0x84, 0x34);
            HI258_write_cmos_sensor(0x85, 0x46);
            HI258_write_cmos_sensor(0x86, 0x3a);
            break;	
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL HI258_set_param_effect(UINT16 para)
{
    SENSORDB("[HI258] %s ====para=%d \n",__func__,para);
    kal_uint32 ret = KAL_TRUE;
    switch (para)
    {
        case MEFFECT_OFF:
            HI258_write_cmos_sensor(0x03, 0x10);
            HI258_write_cmos_sensor(0x11, 0x03);
            HI258_write_cmos_sensor(0x12, 0x30);
            HI258_write_cmos_sensor(0x13, 0x03);
            HI258_write_cmos_sensor(0x44, 0x80);
            HI258_write_cmos_sensor(0x45, 0x80);
            HI258_write_cmos_sensor(0x4a, 0x80);
            break;

        case MEFFECT_SEPIA:
            HI258_write_cmos_sensor(0x03, 0x10);
            HI258_write_cmos_sensor(0x11, 0x03);
            HI258_write_cmos_sensor(0x12, 0x33);
            HI258_write_cmos_sensor(0x13, 0x02);
            HI258_write_cmos_sensor(0x44, 0x70);
            HI258_write_cmos_sensor(0x45, 0x98);
            HI258_write_cmos_sensor(0x4a, 0x80);
            break;  
	
        case MEFFECT_NEGATIVE:		
            HI258_write_cmos_sensor(0x03, 0x10);
            HI258_write_cmos_sensor(0x11, 0x03);
            HI258_write_cmos_sensor(0x12, 0x38);
            HI258_write_cmos_sensor(0x13, 0x02);
            HI258_write_cmos_sensor(0x14, 0x00);
            HI258_write_cmos_sensor(0x4a, 0x80);
            break; 
	
        case MEFFECT_SEPIAGREEN:		
            HI258_write_cmos_sensor(0x03, 0x10);
            HI258_write_cmos_sensor(0x11, 0x03);
            HI258_write_cmos_sensor(0x12, 0x33);
            HI258_write_cmos_sensor(0x13, 0x02);
            HI258_write_cmos_sensor(0x44, 0x30);
            HI258_write_cmos_sensor(0x45, 0x50);
            HI258_write_cmos_sensor(0x4a, 0x80);
            break;
	
        case MEFFECT_SEPIABLUE:	
            HI258_write_cmos_sensor(0x03, 0x10);
            HI258_write_cmos_sensor(0x11, 0x03);
            HI258_write_cmos_sensor(0x12, 0x33);
            HI258_write_cmos_sensor(0x13, 0x02);
            HI258_write_cmos_sensor(0x44, 0xb0);
            HI258_write_cmos_sensor(0x45, 0x40);
            HI258_write_cmos_sensor(0x4a, 0x80);
            break;
	
        case MEFFECT_MONO:				
            HI258_write_cmos_sensor(0x03, 0x10);
            HI258_write_cmos_sensor(0x11, 0x03);
            HI258_write_cmos_sensor(0x12, 0x33);
            HI258_write_cmos_sensor(0x13, 0x02);
            HI258_write_cmos_sensor(0x44, 0x80);
            HI258_write_cmos_sensor(0x45, 0x80);
            HI258_write_cmos_sensor(0x4a, 0x80);
            break;

        default:
            ret = FALSE;
    }

    return ret;
}

BOOL HI258_set_param_banding(UINT16 para)
{
    kal_uint8 banding;
    SENSORDB("[HI258] %s \n",__func__);
    banding = HI258_read_cmos_sensor(0x3014);
    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
            HI258_Banding_setting = AE_FLICKER_MODE_50HZ;
            HI258_write_cmos_sensor(0x03,0x20);
    HI258_write_cmos_sensor(0x10,0x1c);
    HI258_write_cmos_sensor(0x18,0x38);

    HI258_write_cmos_sensor(0x83, 0x0b);//EXP Normal 8.00 fps
    HI258_write_cmos_sensor(0x84, 0xe6);
    HI258_write_cmos_sensor(0x85, 0xe0);

    HI258_write_cmos_sensor(0x18, 0x30);
    HI258_write_cmos_sensor(0x10, 0x9c);
    break;
    case AE_FLICKER_MODE_60HZ:
    HI258_Banding_setting = AE_FLICKER_MODE_60HZ;
    HI258_write_cmos_sensor(0x03, 0x20);
    HI258_write_cmos_sensor(0x10, 0x0c);
    HI258_write_cmos_sensor(0x18, 0x38);

    HI258_write_cmos_sensor(0x83, 0x0c); //EXP Normal 8.00 fps
    HI258_write_cmos_sensor(0x84, 0x5c);
    HI258_write_cmos_sensor(0x85, 0x10);

            HI258_write_cmos_sensor(0x18,0x30);
            HI258_write_cmos_sensor(0x10,0x8c);
            break;
        default:
            return FALSE;
    }

    return TRUE;
} 

BOOL HI258_set_param_exposure(UINT16 para)
{
    SENSORDB("[HI258] %s \n",__func__);
    HI258_write_cmos_sensor(0x03,0x10);
    HI258_write_cmos_sensor(0x12,(HI258_read_cmos_sensor(0x12)|0x10));//make sure the Yoffset control is opened.

    switch (para)
    {
        case AE_EV_COMP_n13:
            HI258_write_cmos_sensor(0x40,0xc0);
            break;
        case AE_EV_COMP_n10:
            HI258_write_cmos_sensor(0x40,0xb0);
            break;
        case AE_EV_COMP_n07:
            HI258_write_cmos_sensor(0x40,0xa0);
            break;
        case AE_EV_COMP_n03:
            HI258_write_cmos_sensor(0x40,0x98);
            break;
        case AE_EV_COMP_00:
            HI258_write_cmos_sensor(0x40,0x80);
            break;
        case AE_EV_COMP_03:
            HI258_write_cmos_sensor(0x40,0x10);
            break;
        case AE_EV_COMP_07:
            HI258_write_cmos_sensor(0x40,0x20);
            break;
        case AE_EV_COMP_10:
            HI258_write_cmos_sensor(0x40,0x30);
            break;
        case AE_EV_COMP_13:
            HI258_write_cmos_sensor(0x40,0x40);
            break;
        default:
            return FALSE;
    }

    return TRUE;
} 


UINT32 HI258YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
    SENSORDB("[HI258] %s \n",__func__);
    if (HI258_op_state.sensor_cap_state == KAL_TRUE)	/* Don't need it when capture mode. */
    {
    return KAL_TRUE;
    }

    switch (iCmd) 
    {
        case FID_SCENE_MODE:	    
            if( HI258_CAPATURE_FLAG == 0)
            {
                if (iPara == SCENE_MODE_OFF)
                {
                    HI258_night_mode(0);
                }
                else if (iPara == SCENE_MODE_NIGHTSCENE)
                {
                    HI258_night_mode(1);
                }
            }
            else
                HI258_CAPATURE_FLAG = 0;
            break; 

        case FID_AWB_MODE:
            HI258_set_param_wb(iPara);
            break;

        case FID_COLOR_EFFECT:	    
            HI258_set_param_effect(iPara);
            break;

        case FID_AE_EV:	       	      
            HI258_set_param_exposure(iPara);
            break;

        case FID_AE_FLICKER:    	    	    
            if( HI258_CAPATUREB_FLAG == 0)
                HI258_set_param_banding(iPara);
            else
                HI258_CAPATUREB_FLAG = 0;
            break;

        case FID_AE_SCENE_MODE: 
            break; 

        case FID_ZOOM_FACTOR:
            HI258_zoom_factor = iPara;
            break; 

        default:
            break;
    }
    return TRUE;
}   

UINT32 HI258YUVSetVideoMode(UINT16 u2FrameRate)
{
    SENSORDB("[HI258] %s \n",__func__);
    kal_uint16 temp_AE_reg = 0;
	
    HI258_write_cmos_sensor(0x03, 0x20); 
    temp_AE_reg = HI258_read_cmos_sensor(0x10);

    return TRUE;
}

UINT32 HI258FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 u2Temp = 0; 

    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;

    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;
    SENSORDB("[HI258] %s =====FeatureID= %d \n",__func__,FeatureId);

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=HI258_FULL_GRAB_WIDTH;
            *pFeatureReturnPara16=HI258_FULL_GRAB_HEIGHT;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_GET_PERIOD:
            *pFeatureReturnPara16++=HI258_PV_PERIOD_PIXEL_NUMS+HI258_PV_dummy_pixels;
            *pFeatureReturnPara16=HI258_PV_PERIOD_LINE_NUMS+HI258_PV_dummy_lines;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *pFeatureReturnPara32 = HI258_sensor_pclk/10;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_SET_ESHUTTER:
            u2Temp = HI258_read_shutter(); 		
            break;
	
        case SENSOR_FEATURE_SET_NIGHTMODE:
            HI258_night_mode((BOOL) *pFeatureData16);
            break;
	
        case SENSOR_FEATURE_SET_GAIN:
            break; 
	
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
	
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            HI258_isp_master_clock=*pFeatureData32;
            break;
	
        case SENSOR_FEATURE_SET_REGISTER:
            HI258_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
	
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = HI258_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
	
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &HI258SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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

        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            HI258_GetSensorID(pFeatureData32); 
            break;

        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_SET_YUV_CMD:
            HI258YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
            break;	
	
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            HI258YUVSetVideoMode(*pFeatureData16);
            break; 

        default:
            break;			
    }
    return ERROR_NONE;
}	

SENSOR_FUNCTION_STRUCT	SensorFuncHI258=
{
    HI258Open,
    HI258GetInfo,
    HI258GetResolution,
    HI258FeatureControl,
    HI258Control,
    HI258Close
};

UINT32 HI258_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncHI258;

    return ERROR_NONE;
}	

