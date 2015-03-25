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

#include "hi256yuv_Sensor.h"
#include "hi256yuv_Camera_Sensor_para.h"
#include "hi256yuv_CameraCustomized.h"

// #define OPEN_FLICKER_AUTO // Jiangde, atuo-flicker need to be debugged for flicker at 50Hz

#define HI256_DEBUG
#ifdef HI256_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

MSDK_SENSOR_CONFIG_STRUCT HI256SensorConfigData;
#define SENSOR_CORE_PCLK	83200000	//48M PCLK Output 78000000 

#define WINMO_USE 0
#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT
#define MIPI_INTERFACE

kal_bool HI256_VEDIO_MPEG4 = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4);

kal_uint8 HI256_Sleep_Mode;
kal_uint32 HI256_PV_dummy_pixels=616,HI256_PV_dummy_lines=20,HI256_isp_master_clock=260/*0*/;

static HI256_SENSOR_INFO_ST HI256_sensor;
static HI256_OPERATION_STATE_ST HI256_op_state;

static kal_uint32 HI256_zoom_factor = 0; 
static kal_bool HI256_gPVmode = KAL_TRUE; //PV size or Full size
static kal_bool HI256_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool HI256_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint8 HI256_Banding_setting = AE_FLICKER_MODE_50HZ;  //Wonder add
static kal_uint16  HI256_PV_Shutter = 0;
static kal_uint32  HI256_sensor_pclk=260;//520 //20110518

static kal_uint32 HI256_pv_HI256_exposure_lines=0x05f370, HI256_cp_HI256_exposure_lines=0;
static kal_uint16 HI256_Capture_Max_Gain16= 6*16;
static kal_uint16 HI256_Capture_Gain16=0 ;    
static kal_uint16 HI256_Capture_Shutter=0;
static kal_uint16 HI256_Capture_Extra_Lines=0;

static int HI256_CAPATURE_FLAG = 0;//Add By Paul
static int HI256_CAPATUREB_FLAG = 0;//Add By Paul

extern void SubCameraDigtalPDNCtrl(u32 onoff);// Add by paul

static kal_uint16  HI256_PV_Gain16 = 0;
static kal_uint16  HI256_PV_Extra_Lines = 0;
kal_uint32 HI256_capture_pclk_in_M=520,HI256_preview_pclk_in_M=390;

//extern static CAMERA_DUAL_CAMERA_SENSOR_ENUM g_currDualSensorIdx;
//extern static char g_currSensorName[32];
//extern int kdModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name);

//extern int iReadReg_Byte(u8 addr, u8 *buf, u8 i2cId);
//extern int iWriteReg_Byte(u8 addr, u8 buf, u32 size, u16 i2cId);
//SENSOR_REG_STRUCT HI256SensorCCT[FACTORY_END_ADDR]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
//SENSOR_REG_STRUCT HI256SensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
//	camera_para.SENSOR.cct	SensorCCT	=> SensorCCT
//	camera_para.SENSOR.reg	SensorReg

BOOL HI256_set_param_banding(UINT16 para);

//extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff, u16 i2cId);
//extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes, u16 i2cId);
//extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
/*ergate-017*/
//extern int iWriteRegI2C_ext(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId, u16 speed);
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
static void HI256_set_mirror_flip(kal_uint8 image_mirror);
kal_uint16 HI256_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    //char puSendCmd[2] = {(char)(addr & 0xFF) ,(char)(para & 0xFF)};

    //iWriteRegI2C_ext(puSendCmd , 2,HI256_WRITE_ID_0, 50);
    //iWriteReg_Byte(addr, para, 1, HI256_WRITE_ID_0);
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd , 2,HI256_WRITE_ID_0);
    return 0;
}

kal_uint8 HI256_read_cmos_sensor(kal_uint8 addr)
{
    //kal_uint8 get_byte=0;
    //iReadReg_Byte(addr, &get_byte, HI256_WRITE_ID_0);
    //return get_byte;
    
    kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
    iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte,1,HI256_WRITE_ID_0);
    return get_byte;
}

static kal_uint32 HI256_GetSensorID(kal_uint32 *sensorID)
{
    kal_uint16 sensor_id=0,hw_id=0;
    volatile signed char i;
    for(i=0;i<3;i++){
        sensor_id = HI256_read_cmos_sensor(0x04);
#if 0
        mt_set_gpio_mode(GPIO_SUB_CAM_ID,GPIO_SUB_CAM_ID_M_GPIO);
        mt_set_gpio_dir(GPIO_SUB_CAM_ID,GPIO_DIR_IN);
        hw_id=mt_get_gpio_in(GPIO_SUB_CAM_ID);
        mdelay(1);
        hw_id=mt_get_gpio_in(GPIO_SUB_CAM_ID);
#endif
        printk("HJDDBG, [HI256] sensor id = 0x%x,====hw_id=%d\n", sensor_id,hw_id);
        if (HI256_SENSOR_ID == sensor_id)
            break;
        
        if (2 != i)
        {
            mdelay(2); // Jiangde, retry after a while
            SENSORDB("HJDDBG, [HI256] retry after a while, retry = %d \n", i);
        }
    }

    *sensorID=sensor_id;
    // if (HI256_SENSOR_ID == sensor_id && 0x00 == hw_id) // Jiangde --
    if (HI256_SENSOR_ID == sensor_id) // && 0x00 == hw_id)
    {
        printk("HJDDBG, [HI256] cmk hi256. sensor id = 0x%x, hw_id=%d \n", sensor_id, hw_id);
	    SubCameraDigtalPDNCtrl(1);
    }
    else
    {
        *sensorID=0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;    
}


void HI256_Init_Cmds(void) 
{
    SENSORDB("[hi256] %s \n",__func__);
    //Initial Start
    /////// PAGE 0 START ///////
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x10, 0x10);  // preview1+hbin 0x91 , preview2 0x03
    HI256_write_cmos_sensor(0x11, 0x93);
    HI256_write_cmos_sensor(0x12, 0x00);

    HI256_write_cmos_sensor(0x0b, 0xaa);
    HI256_write_cmos_sensor(0x0c, 0xaa);
    HI256_write_cmos_sensor(0x0d, 0xaa);

    HI256_write_cmos_sensor(0x20, 0x00);
    HI256_write_cmos_sensor(0x21, 0x06);
    HI256_write_cmos_sensor(0x22, 0x00);
    HI256_write_cmos_sensor(0x23, 0x05);

    HI256_write_cmos_sensor(0x24, 0x04);
    HI256_write_cmos_sensor(0x25, 0xb0);
    HI256_write_cmos_sensor(0x26, 0x06);
    HI256_write_cmos_sensor(0x27, 0x40);

    HI256_write_cmos_sensor(0x40, 0x01); //Hblank 360
    HI256_write_cmos_sensor(0x41, 0x68); 
    HI256_write_cmos_sensor(0x42, 0x00); //Vblank 18
    HI256_write_cmos_sensor(0x43, 0x14); 

    HI256_write_cmos_sensor(0x45, 0x04);
    HI256_write_cmos_sensor(0x46, 0x18);
    HI256_write_cmos_sensor(0x47, 0xd8);

    //BLC
    HI256_write_cmos_sensor(0x80, 0x2e);
    HI256_write_cmos_sensor(0x81, 0x7e);
    HI256_write_cmos_sensor(0x82, 0x90);
    HI256_write_cmos_sensor(0x83, 0x00);
    HI256_write_cmos_sensor(0x84, 0x0c);
    HI256_write_cmos_sensor(0x85, 0x00);
    HI256_write_cmos_sensor(0x90, 0x0f); //BLC_TIME_TH_ON
    HI256_write_cmos_sensor(0x91, 0x0f); //BLC_TIME_TH_OFF 
    HI256_write_cmos_sensor(0x92, 0x78); //BLC_AG_TH_ON
    HI256_write_cmos_sensor(0x93, 0x70); //BLC_AG_TH_OFF
    HI256_write_cmos_sensor(0x94, 0xff);
    HI256_write_cmos_sensor(0x95, 0xff);
    HI256_write_cmos_sensor(0x96, 0xdc);
    HI256_write_cmos_sensor(0x97, 0xfe);
    HI256_write_cmos_sensor(0x98, 0x38);

    //Dark BLC
    HI256_write_cmos_sensor(0xa0, 0x00);
    HI256_write_cmos_sensor(0xa2, 0x00);
    HI256_write_cmos_sensor(0xa4, 0x00);
    HI256_write_cmos_sensor(0xa6, 0x00);

    //Normal BLC
    HI256_write_cmos_sensor(0xa8, 0x45);
    HI256_write_cmos_sensor(0xaa, 0x45);
    HI256_write_cmos_sensor(0xac, 0x45);
    HI256_write_cmos_sensor(0xae, 0x45);

    //OutDoor  BLC
    HI256_write_cmos_sensor(0x99, 0x43);
    HI256_write_cmos_sensor(0x9a, 0x43);
    HI256_write_cmos_sensor(0x9b, 0x43);
    HI256_write_cmos_sensor(0x9c, 0x43);
    /////// PAGE 0 END ///////

    /////// PAGE 2 START ///////
    HI256_write_cmos_sensor(0x03, 0x02);
    HI256_write_cmos_sensor(0x12, 0x03);
    HI256_write_cmos_sensor(0x13, 0x03);
    HI256_write_cmos_sensor(0x15, 0x00);
    HI256_write_cmos_sensor(0x16, 0x00);
    HI256_write_cmos_sensor(0x17, 0x8C);
    HI256_write_cmos_sensor(0x18, 0x4c); //Double_AG//
    HI256_write_cmos_sensor(0x19, 0x00);
    HI256_write_cmos_sensor(0x1a, 0x39); //Double_AG/ 38 ->39
    HI256_write_cmos_sensor(0x1c, 0x09);
    HI256_write_cmos_sensor(0x1d, 0x40);
    HI256_write_cmos_sensor(0x1e, 0x30);
    HI256_write_cmos_sensor(0x1f, 0x10);

    HI256_write_cmos_sensor(0x20, 0x77);
    HI256_write_cmos_sensor(0x21, 0xde);
    HI256_write_cmos_sensor(0x22, 0xa7);
    HI256_write_cmos_sensor(0x23, 0x30); //CLAMP
    HI256_write_cmos_sensor(0x24, 0x77); //ADD 2012.04.23 don`t touch
    HI256_write_cmos_sensor(0x27, 0x3c);
    HI256_write_cmos_sensor(0x2b, 0x80);
    HI256_write_cmos_sensor(0x2e, 0x00);//ADD 2012.04.23 don`t touch
    HI256_write_cmos_sensor(0x2f, 0x00);//ADD 2012.04.23 don`t touch
    HI256_write_cmos_sensor(0x30, 0x05); //For Hi-253 never no change 0x05

    HI256_write_cmos_sensor(0x50, 0x20);
    HI256_write_cmos_sensor(0x52, 0x01); //0x03 --> 0x01 (by À±Ã¥ÀÓ´Ô, 20100513)
    HI256_write_cmos_sensor(0x53, 0xc1);  //ADD 2012.04.23 don`t touch
    HI256_write_cmos_sensor(0x55, 0x1c);
    HI256_write_cmos_sensor(0x56, 0x11);

    HI256_write_cmos_sensor(0x58, 0x22);  //ADD 2012.04.23 don`t touch
    HI256_write_cmos_sensor(0x59, 0x20);  //ADD 2012.04.23 don`t touch

    HI256_write_cmos_sensor(0x5d, 0xa2);
    HI256_write_cmos_sensor(0x5e, 0x5a);

    HI256_write_cmos_sensor(0x60, 0x87);
    HI256_write_cmos_sensor(0x61, 0x99);
    HI256_write_cmos_sensor(0x62, 0x88);
    HI256_write_cmos_sensor(0x63, 0x97);
    HI256_write_cmos_sensor(0x64, 0x88);
    HI256_write_cmos_sensor(0x65, 0x97);

    HI256_write_cmos_sensor(0x67, 0x0c);
    HI256_write_cmos_sensor(0x68, 0x0c);
    HI256_write_cmos_sensor(0x69, 0x0c);

    HI256_write_cmos_sensor(0x72, 0x89);
    HI256_write_cmos_sensor(0x73, 0x96);
    HI256_write_cmos_sensor(0x74, 0x89);
    HI256_write_cmos_sensor(0x75, 0x96);
    HI256_write_cmos_sensor(0x76, 0x89);
    HI256_write_cmos_sensor(0x77, 0x96);

    HI256_write_cmos_sensor(0x7c, 0x85);
    HI256_write_cmos_sensor(0x7d, 0xaf);
    HI256_write_cmos_sensor(0x80, 0x01);
    HI256_write_cmos_sensor(0x81, 0x7f);
    HI256_write_cmos_sensor(0x82, 0x13);
    HI256_write_cmos_sensor(0x83, 0x24);
    HI256_write_cmos_sensor(0x84, 0x7d);
    HI256_write_cmos_sensor(0x85, 0x81);
    HI256_write_cmos_sensor(0x86, 0x7d);
    HI256_write_cmos_sensor(0x87, 0x81);

    HI256_write_cmos_sensor(0x92, 0x48);
    HI256_write_cmos_sensor(0x93, 0x54);
    HI256_write_cmos_sensor(0x94, 0x7d);
    HI256_write_cmos_sensor(0x95, 0x81);
    HI256_write_cmos_sensor(0x96, 0x7d);
    HI256_write_cmos_sensor(0x97, 0x81);

    HI256_write_cmos_sensor(0xa0, 0x02);
    HI256_write_cmos_sensor(0xa1, 0x7b);
    HI256_write_cmos_sensor(0xa2, 0x02);
    HI256_write_cmos_sensor(0xa3, 0x7b);
    HI256_write_cmos_sensor(0xa4, 0x7b);
    HI256_write_cmos_sensor(0xa5, 0x02);
    HI256_write_cmos_sensor(0xa6, 0x7b);
    HI256_write_cmos_sensor(0xa7, 0x02);

    HI256_write_cmos_sensor(0xa8, 0x85);
    HI256_write_cmos_sensor(0xa9, 0x8c);
    HI256_write_cmos_sensor(0xaa, 0x85);
    HI256_write_cmos_sensor(0xab, 0x8c);
    HI256_write_cmos_sensor(0xac, 0x10);
    HI256_write_cmos_sensor(0xad, 0x16);
    HI256_write_cmos_sensor(0xae, 0x10);
    HI256_write_cmos_sensor(0xaf, 0x16);

    HI256_write_cmos_sensor(0xb0, 0x99);
    HI256_write_cmos_sensor(0xb1, 0xa3);
    HI256_write_cmos_sensor(0xb2, 0xa4);
    HI256_write_cmos_sensor(0xb3, 0xae);
    HI256_write_cmos_sensor(0xb4, 0x9b);
    HI256_write_cmos_sensor(0xb5, 0xa2);
    HI256_write_cmos_sensor(0xb6, 0xa6);
    HI256_write_cmos_sensor(0xb7, 0xac);
    HI256_write_cmos_sensor(0xb8, 0x9b);
    HI256_write_cmos_sensor(0xb9, 0x9f);
    HI256_write_cmos_sensor(0xba, 0xa6);
    HI256_write_cmos_sensor(0xbb, 0xaa);
    HI256_write_cmos_sensor(0xbc, 0x9b);
    HI256_write_cmos_sensor(0xbd, 0x9f);
    HI256_write_cmos_sensor(0xbe, 0xa6);
    HI256_write_cmos_sensor(0xbf, 0xaa); 

    HI256_write_cmos_sensor(0xc4, 0x2c);
    HI256_write_cmos_sensor(0xc5, 0x43);
    HI256_write_cmos_sensor(0xc6, 0x63);
    HI256_write_cmos_sensor(0xc7, 0x79);

    HI256_write_cmos_sensor(0xc8, 0x2d);
    HI256_write_cmos_sensor(0xc9, 0x42);
    HI256_write_cmos_sensor(0xca, 0x2d);
    HI256_write_cmos_sensor(0xcb, 0x42);
    HI256_write_cmos_sensor(0xcc, 0x64);
    HI256_write_cmos_sensor(0xcd, 0x78);
    HI256_write_cmos_sensor(0xce, 0x64);
    HI256_write_cmos_sensor(0xcf, 0x78);
    HI256_write_cmos_sensor(0xd0, 0x0a);
    HI256_write_cmos_sensor(0xd1, 0x09);
    HI256_write_cmos_sensor(0xd4, 0x0f); //DCDC_TIME_TH_ON
    HI256_write_cmos_sensor(0xd5, 0x0f); //DCDC_TIME_TH_OFF 
    HI256_write_cmos_sensor(0xd6, 0x78); //DCDC_AG_TH_ON
    HI256_write_cmos_sensor(0xd7, 0x70); //DCDC_AG_TH_OFF
    HI256_write_cmos_sensor(0xe0, 0xc4);
    HI256_write_cmos_sensor(0xe1, 0xc4);
    HI256_write_cmos_sensor(0xe2, 0xc4);
    HI256_write_cmos_sensor(0xe3, 0xc4);
    HI256_write_cmos_sensor(0xe4, 0x00);
    HI256_write_cmos_sensor(0xe8, 0x80);
    HI256_write_cmos_sensor(0xe9, 0x40);
    HI256_write_cmos_sensor(0xea, 0x7f);

    HI256_write_cmos_sensor(0xf0, 0xc1);  //ADD 2012.04.23 don`t touch
    HI256_write_cmos_sensor(0xf1, 0xc1);
    HI256_write_cmos_sensor(0xf2, 0xc1);
    HI256_write_cmos_sensor(0xf3, 0xc1);
    HI256_write_cmos_sensor(0xf4, 0xc1);

    /////// PAGE 2 END ///////

    /////// PAGE 3 ///////
    HI256_write_cmos_sensor(0x03, 0x03);
    HI256_write_cmos_sensor(0x10, 0x10);
    /////// PAGE 3 END ///////

    /////// PAGE 10 START ///////
    HI256_write_cmos_sensor(0x03, 0x10);
    HI256_write_cmos_sensor(0x10, 0x03); // CrYCbY // For Demoset 0x03
    HI256_write_cmos_sensor(0x12, 0x30);
    HI256_write_cmos_sensor(0x13, 0x02);
    HI256_write_cmos_sensor(0x20, 0x00);
    HI256_write_cmos_sensor(0x30, 0x00);
    HI256_write_cmos_sensor(0x31, 0x00);
    HI256_write_cmos_sensor(0x32, 0x00);
    HI256_write_cmos_sensor(0x33, 0x00);

    HI256_write_cmos_sensor(0x34, 0x30);
    HI256_write_cmos_sensor(0x35, 0x00);
    HI256_write_cmos_sensor(0x36, 0x00);
    HI256_write_cmos_sensor(0x38, 0x00);
    HI256_write_cmos_sensor(0x3e, 0x58);
    HI256_write_cmos_sensor(0x3f, 0x00);
    HI256_write_cmos_sensor(0x40, 0x90);
    HI256_write_cmos_sensor(0x41, 0x08);
    HI256_write_cmos_sensor(0x48, 0x88);
    HI256_write_cmos_sensor(0x60, 0x67);
    HI256_write_cmos_sensor(0x61, 0x78); //77
    HI256_write_cmos_sensor(0x62, 0x78); //77
    HI256_write_cmos_sensor(0x63, 0x50); // Double_AG 50->30
    HI256_write_cmos_sensor(0x64, 0x27);

    HI256_write_cmos_sensor(0x66, 0x33);
    HI256_write_cmos_sensor(0x67, 0x00);

    HI256_write_cmos_sensor(0x6a, 0x80); //8a
    HI256_write_cmos_sensor(0x6b, 0x80); //74
    HI256_write_cmos_sensor(0x6c, 0x7a); //7e
    HI256_write_cmos_sensor(0x6d, 0x80); //8e
    HI256_write_cmos_sensor(0x76, 0x01); // white protection ON
    HI256_write_cmos_sensor(0x74, 0x66);
    HI256_write_cmos_sensor(0x79, 0x06);

    /////// PAGE 11 START ///////
    HI256_write_cmos_sensor(0x03, 0x11);
    HI256_write_cmos_sensor(0x10, 0x7f);
    HI256_write_cmos_sensor(0x11, 0x40);
    HI256_write_cmos_sensor(0x12, 0x0a); // Blue Max-Filter Delete
    HI256_write_cmos_sensor(0x13, 0xbb);

    HI256_write_cmos_sensor(0x26, 0x31); // Double_AG 31->20
    HI256_write_cmos_sensor(0x27, 0x34); // Double_AG 34->22
    HI256_write_cmos_sensor(0x28, 0x0f);
    HI256_write_cmos_sensor(0x29, 0x10);
    HI256_write_cmos_sensor(0x2b, 0x30);
    HI256_write_cmos_sensor(0x2c, 0x32);

    //Out2 D-LPF th
    HI256_write_cmos_sensor(0x30, 0x70);
    HI256_write_cmos_sensor(0x31, 0x10);
    HI256_write_cmos_sensor(0x32, 0x58);
    HI256_write_cmos_sensor(0x33, 0x09);
    HI256_write_cmos_sensor(0x34, 0x06);
    HI256_write_cmos_sensor(0x35, 0x03);

    //Out1 D-LPF th
    HI256_write_cmos_sensor(0x36, 0x70);
    HI256_write_cmos_sensor(0x37, 0x18);
    HI256_write_cmos_sensor(0x38, 0x58);
    HI256_write_cmos_sensor(0x39, 0x09);
    HI256_write_cmos_sensor(0x3a, 0x06);
    HI256_write_cmos_sensor(0x3b, 0x03);

    //Indoor D-LPF th
    HI256_write_cmos_sensor(0x3c, 0x80);
    HI256_write_cmos_sensor(0x3d, 0x18);
    HI256_write_cmos_sensor(0x3e, 0x80);
    HI256_write_cmos_sensor(0x3f, 0x0c);
    HI256_write_cmos_sensor(0x40, 0x05);
    HI256_write_cmos_sensor(0x41, 0x06);

    HI256_write_cmos_sensor(0x42, 0x80);
    HI256_write_cmos_sensor(0x43, 0x18);
    HI256_write_cmos_sensor(0x44, 0x80);
    HI256_write_cmos_sensor(0x45, 0x0c);
    HI256_write_cmos_sensor(0x46, 0x05);
    HI256_write_cmos_sensor(0x47, 0x06);

    HI256_write_cmos_sensor(0x48, 0x90);
    HI256_write_cmos_sensor(0x49, 0x40);
    HI256_write_cmos_sensor(0x4a, 0x80);
    HI256_write_cmos_sensor(0x4b, 0x13);
    HI256_write_cmos_sensor(0x4c, 0x10);
    HI256_write_cmos_sensor(0x4d, 0x11);

    HI256_write_cmos_sensor(0x4e, 0x80);
    HI256_write_cmos_sensor(0x4f, 0x30);
    HI256_write_cmos_sensor(0x50, 0x80);
    HI256_write_cmos_sensor(0x51, 0x13);
    HI256_write_cmos_sensor(0x52, 0x10);
    HI256_write_cmos_sensor(0x53, 0x13);

    HI256_write_cmos_sensor(0x54, 0x11);
    HI256_write_cmos_sensor(0x55, 0x17);
    HI256_write_cmos_sensor(0x56, 0x20);
    HI256_write_cmos_sensor(0x57, 0x01);
    HI256_write_cmos_sensor(0x58, 0x00);
    HI256_write_cmos_sensor(0x59, 0x00);

    HI256_write_cmos_sensor(0x5a, 0x18);
    HI256_write_cmos_sensor(0x5b, 0x00);
    HI256_write_cmos_sensor(0x5c, 0x00);

    HI256_write_cmos_sensor(0x60, 0x3f);
    HI256_write_cmos_sensor(0x62, 0x60);
    HI256_write_cmos_sensor(0x70, 0x06);
    /////// PAGE 11 END ///////

    /////// PAGE 12 START ///////
    HI256_write_cmos_sensor(0x03, 0x12);
    HI256_write_cmos_sensor(0x20, 0x0f);
    HI256_write_cmos_sensor(0x21, 0x0f);

    HI256_write_cmos_sensor(0x25, 0x00); //0x30

    HI256_write_cmos_sensor(0x28, 0x00);
    HI256_write_cmos_sensor(0x29, 0x00);
    HI256_write_cmos_sensor(0x2a, 0x00);

    HI256_write_cmos_sensor(0x30, 0x50);
    HI256_write_cmos_sensor(0x31, 0x18);
    HI256_write_cmos_sensor(0x32, 0x32);
    HI256_write_cmos_sensor(0x33, 0x40);
    HI256_write_cmos_sensor(0x34, 0x50);
    HI256_write_cmos_sensor(0x35, 0x70);
    HI256_write_cmos_sensor(0x36, 0xa0);

    //Out2 th
    HI256_write_cmos_sensor(0x40, 0xa0);
    HI256_write_cmos_sensor(0x41, 0x40);
    HI256_write_cmos_sensor(0x42, 0xa0);
    HI256_write_cmos_sensor(0x43, 0x90);
    HI256_write_cmos_sensor(0x44, 0x90);
    HI256_write_cmos_sensor(0x45, 0x80);

    //Out1 th
    HI256_write_cmos_sensor(0x46, 0xb0);
    HI256_write_cmos_sensor(0x47, 0x55);
    HI256_write_cmos_sensor(0x48, 0xa0);
    HI256_write_cmos_sensor(0x49, 0x90);
    HI256_write_cmos_sensor(0x4a, 0x90);
    HI256_write_cmos_sensor(0x4b, 0x80);

    //Indoor th
    HI256_write_cmos_sensor(0x4c, 0xb0);
    HI256_write_cmos_sensor(0x4d, 0x40);
    HI256_write_cmos_sensor(0x4e, 0x90);
    HI256_write_cmos_sensor(0x4f, 0x60);
    HI256_write_cmos_sensor(0x50, 0xa0);
    HI256_write_cmos_sensor(0x51, 0x80);

    //Dark1 th
    HI256_write_cmos_sensor(0x52, 0xb0);
    HI256_write_cmos_sensor(0x53, 0x40);
    HI256_write_cmos_sensor(0x54, 0x90);
    HI256_write_cmos_sensor(0x55, 0x60);
    HI256_write_cmos_sensor(0x56, 0xa0);
    HI256_write_cmos_sensor(0x57, 0x90);

    //Dark2 th
    HI256_write_cmos_sensor(0x58, 0x90);
    HI256_write_cmos_sensor(0x59, 0x40);
    HI256_write_cmos_sensor(0x5a, 0xd0);
    HI256_write_cmos_sensor(0x5b, 0xd0);
    HI256_write_cmos_sensor(0x5c, 0xe0);
    HI256_write_cmos_sensor(0x5d, 0x90);

    //Dark3 th
    HI256_write_cmos_sensor(0x5e, 0x88);
    HI256_write_cmos_sensor(0x5f, 0x40);
    HI256_write_cmos_sensor(0x60, 0xe0);
    HI256_write_cmos_sensor(0x61, 0xe0);
    HI256_write_cmos_sensor(0x62, 0xe0);
    HI256_write_cmos_sensor(0x63, 0xb0);

    HI256_write_cmos_sensor(0x70, 0x15);
    HI256_write_cmos_sensor(0x71, 0x01); //Don't Touch register

    HI256_write_cmos_sensor(0x72, 0x18);
    HI256_write_cmos_sensor(0x73, 0x01); //Don't Touch register

    HI256_write_cmos_sensor(0x74, 0x25);
    HI256_write_cmos_sensor(0x75, 0x15);

    HI256_write_cmos_sensor(0x80, 0x20);
    HI256_write_cmos_sensor(0x81, 0x40);
    HI256_write_cmos_sensor(0x82, 0x65);
    HI256_write_cmos_sensor(0x85, 0x1a);
    HI256_write_cmos_sensor(0x88, 0x00);
    HI256_write_cmos_sensor(0x89, 0x00);
    HI256_write_cmos_sensor(0x90, 0x5d); //For Preview

    //Dont Touch register
    HI256_write_cmos_sensor(0xD0, 0x0c);
    HI256_write_cmos_sensor(0xD1, 0x80);
    HI256_write_cmos_sensor(0xD2, 0x67);
    HI256_write_cmos_sensor(0xD3, 0x00);
    HI256_write_cmos_sensor(0xD4, 0x00);
    HI256_write_cmos_sensor(0xD5, 0x02);
    HI256_write_cmos_sensor(0xD6, 0xff);
    HI256_write_cmos_sensor(0xD7, 0x18);
    //End
    HI256_write_cmos_sensor(0x3b, 0x06);
    HI256_write_cmos_sensor(0x3c, 0x06);

    HI256_write_cmos_sensor(0xc5, 0x00);//55->48
    HI256_write_cmos_sensor(0xc6, 0x00);//48->40
    /////// PAGE 12 END ///////

    /////// PAGE 13 START ///////
    HI256_write_cmos_sensor(0x03, 0x13);
    //Edge
    HI256_write_cmos_sensor(0x10, 0xcb);
    HI256_write_cmos_sensor(0x11, 0x7b);
    HI256_write_cmos_sensor(0x12, 0x07);
    HI256_write_cmos_sensor(0x14, 0x00);

    HI256_write_cmos_sensor(0x20, 0x15);
    HI256_write_cmos_sensor(0x21, 0x13);
    HI256_write_cmos_sensor(0x22, 0x33);
    HI256_write_cmos_sensor(0x23, 0x05);
    HI256_write_cmos_sensor(0x24, 0x09);

    HI256_write_cmos_sensor(0x25, 0x0a);

    HI256_write_cmos_sensor(0x26, 0x18);
    HI256_write_cmos_sensor(0x27, 0x30);
    HI256_write_cmos_sensor(0x29, 0x12);
    HI256_write_cmos_sensor(0x2a, 0x50);

    //Low clip th
    HI256_write_cmos_sensor(0x2b, 0x02);
    HI256_write_cmos_sensor(0x2c, 0x02);
    HI256_write_cmos_sensor(0x25, 0x06);
    HI256_write_cmos_sensor(0x2d, 0x0c);
    HI256_write_cmos_sensor(0x2e, 0x12);
    HI256_write_cmos_sensor(0x2f, 0x12);

    //Out2 Edge
    HI256_write_cmos_sensor(0x50, 0x10);
    HI256_write_cmos_sensor(0x51, 0x14);
    HI256_write_cmos_sensor(0x52, 0x12);
    HI256_write_cmos_sensor(0x53, 0x0c);
    HI256_write_cmos_sensor(0x54, 0x0f);
    HI256_write_cmos_sensor(0x55, 0x0c);

    //Out1 Edge
    HI256_write_cmos_sensor(0x56, 0x10);
    HI256_write_cmos_sensor(0x57, 0x13);
    HI256_write_cmos_sensor(0x58, 0x12);
    HI256_write_cmos_sensor(0x59, 0x0c);
    HI256_write_cmos_sensor(0x5a, 0x0f);
    HI256_write_cmos_sensor(0x5b, 0x0c);

    //Indoor Edge
    HI256_write_cmos_sensor(0x5c, 0x0a);
    HI256_write_cmos_sensor(0x5d, 0x09);
    HI256_write_cmos_sensor(0x5e, 0x0d);
    HI256_write_cmos_sensor(0x5f, 0x0a);
    HI256_write_cmos_sensor(0x60, 0x0e);
    HI256_write_cmos_sensor(0x61, 0x0b);

    //Dark1 Edge
    HI256_write_cmos_sensor(0x62, 0x18);
    HI256_write_cmos_sensor(0x63, 0x18);
    HI256_write_cmos_sensor(0x64, 0x18);
    HI256_write_cmos_sensor(0x65, 0x16);
    HI256_write_cmos_sensor(0x66, 0x16);
    HI256_write_cmos_sensor(0x67, 0x16);

    //Dark2 Edge
    HI256_write_cmos_sensor(0x68, 0x07);
    HI256_write_cmos_sensor(0x69, 0x07);
    HI256_write_cmos_sensor(0x6a, 0x07);
    HI256_write_cmos_sensor(0x6b, 0x05);
    HI256_write_cmos_sensor(0x6c, 0x05);
    HI256_write_cmos_sensor(0x6d, 0x05);

    //Dark3 Edge
    HI256_write_cmos_sensor(0x6e, 0x07);
    HI256_write_cmos_sensor(0x6f, 0x07);
    HI256_write_cmos_sensor(0x70, 0x07);
    HI256_write_cmos_sensor(0x71, 0x05);
    HI256_write_cmos_sensor(0x72, 0x05);
    HI256_write_cmos_sensor(0x73, 0x05);

    //2DY
    HI256_write_cmos_sensor(0x80, 0xfd);
    HI256_write_cmos_sensor(0x81, 0x1f);
    HI256_write_cmos_sensor(0x82, 0x05);
    HI256_write_cmos_sensor(0x83, 0x31);

    HI256_write_cmos_sensor(0x90, 0x05);
    HI256_write_cmos_sensor(0x91, 0x05);
    HI256_write_cmos_sensor(0x92, 0x33);
    HI256_write_cmos_sensor(0x93, 0x30);
    HI256_write_cmos_sensor(0x94, 0x03);
    HI256_write_cmos_sensor(0x95, 0x14);
    HI256_write_cmos_sensor(0x97, 0x20);
    HI256_write_cmos_sensor(0x99, 0x20);

    HI256_write_cmos_sensor(0xa0, 0x01);
    HI256_write_cmos_sensor(0xa1, 0x02);
    HI256_write_cmos_sensor(0xa2, 0x01);
    HI256_write_cmos_sensor(0xa3, 0x02);
    HI256_write_cmos_sensor(0xa4, 0x05);
    HI256_write_cmos_sensor(0xa5, 0x05);
    HI256_write_cmos_sensor(0xa6, 0x07);
    HI256_write_cmos_sensor(0xa7, 0x08);
    HI256_write_cmos_sensor(0xa8, 0x07);
    HI256_write_cmos_sensor(0xa9, 0x08);
    HI256_write_cmos_sensor(0xaa, 0x07);
    HI256_write_cmos_sensor(0xab, 0x08);

    //Out2 
    HI256_write_cmos_sensor(0xb0, 0x22);
    HI256_write_cmos_sensor(0xb1, 0x2a);
    HI256_write_cmos_sensor(0xb2, 0x28);
    HI256_write_cmos_sensor(0xb3, 0x22);
    HI256_write_cmos_sensor(0xb4, 0x2a);
    HI256_write_cmos_sensor(0xb5, 0x28);

    //Out1 
    HI256_write_cmos_sensor(0xb6, 0x22);
    HI256_write_cmos_sensor(0xb7, 0x2a);
    HI256_write_cmos_sensor(0xb8, 0x28);
    HI256_write_cmos_sensor(0xb9, 0x22);
    HI256_write_cmos_sensor(0xba, 0x2a);
    HI256_write_cmos_sensor(0xbb, 0x28);

    //Indoor 
    HI256_write_cmos_sensor(0xbc, 0x25);
    HI256_write_cmos_sensor(0xbd, 0x2a);
    HI256_write_cmos_sensor(0xbe, 0x27);
    HI256_write_cmos_sensor(0xbf, 0x25);
    HI256_write_cmos_sensor(0xc0, 0x2a);
    HI256_write_cmos_sensor(0xc1, 0x27);

    //Dark1
    HI256_write_cmos_sensor(0xc2, 0x1e);
    HI256_write_cmos_sensor(0xc3, 0x24);
    HI256_write_cmos_sensor(0xc4, 0x20);
    HI256_write_cmos_sensor(0xc5, 0x1e);
    HI256_write_cmos_sensor(0xc6, 0x24);
    HI256_write_cmos_sensor(0xc7, 0x20);

    //Dark2
    HI256_write_cmos_sensor(0xc8, 0x18);
    HI256_write_cmos_sensor(0xc9, 0x20);
    HI256_write_cmos_sensor(0xca, 0x1e);
    HI256_write_cmos_sensor(0xcb, 0x18);
    HI256_write_cmos_sensor(0xcc, 0x20);
    HI256_write_cmos_sensor(0xcd, 0x1e);

    //Dark3 
    HI256_write_cmos_sensor(0xce, 0x18);
    HI256_write_cmos_sensor(0xcf, 0x20);
    HI256_write_cmos_sensor(0xd0, 0x1e);
    HI256_write_cmos_sensor(0xd1, 0x18);
    HI256_write_cmos_sensor(0xd2, 0x20);
    HI256_write_cmos_sensor(0xd3, 0x1e);
    /////// PAGE 13 END ///////

    /////// PAGE 14 START ///////
    HI256_write_cmos_sensor(0x03, 0x14);
    HI256_write_cmos_sensor(0x10, 0x11);

    HI256_write_cmos_sensor(0x14, 0x80); // GX
    HI256_write_cmos_sensor(0x15, 0x80); // GY
    HI256_write_cmos_sensor(0x16, 0x80); // RX
    HI256_write_cmos_sensor(0x17, 0x80); // RY
    HI256_write_cmos_sensor(0x18, 0x80); // BX
    HI256_write_cmos_sensor(0x19, 0x80); // BY

    HI256_write_cmos_sensor(0x20, 0x60); //X

    HI256_write_cmos_sensor(0x21, 0x80); //Y

    HI256_write_cmos_sensor(0x22, 0x80);
    HI256_write_cmos_sensor(0x23, 0x80);
    HI256_write_cmos_sensor(0x24, 0x80);

    HI256_write_cmos_sensor(0x30, 0xc8);
    HI256_write_cmos_sensor(0x31, 0x2b);
    HI256_write_cmos_sensor(0x32, 0x00);
    HI256_write_cmos_sensor(0x33, 0x00);
    HI256_write_cmos_sensor(0x34, 0x90);

    HI256_write_cmos_sensor(0x40, 0x42); //35
    HI256_write_cmos_sensor(0x50, 0x2d); //22
    HI256_write_cmos_sensor(0x60, 0x28); //1a
    HI256_write_cmos_sensor(0x70, 0x2d); //22
    /////// PAGE 14 END ///////

    /////// PAGE 15 START ///////
    HI256_write_cmos_sensor(0x03, 0x15);
    HI256_write_cmos_sensor(0x10, 0x0f);

    //Rstep H 16
    //Rstep L 14
    HI256_write_cmos_sensor(0x14, 0x52); //CMCOFSGH
    HI256_write_cmos_sensor(0x15, 0x42); //CMCOFSGM
    HI256_write_cmos_sensor(0x16, 0x32); //CMCOFSGL
    HI256_write_cmos_sensor(0x17, 0x2f); //CMC SIGN

    //CMC
    HI256_write_cmos_sensor(0x30, 0x8f);
    HI256_write_cmos_sensor(0x31, 0x59);
    HI256_write_cmos_sensor(0x32, 0x0a);
    HI256_write_cmos_sensor(0x33, 0x15);
    HI256_write_cmos_sensor(0x34, 0x5b);
    HI256_write_cmos_sensor(0x35, 0x06);
    HI256_write_cmos_sensor(0x36, 0x07);
    HI256_write_cmos_sensor(0x37, 0x40);
    HI256_write_cmos_sensor(0x38, 0x86); //86

    //CMC OFS
    HI256_write_cmos_sensor(0x40, 0x95);
    HI256_write_cmos_sensor(0x41, 0x1f);
    HI256_write_cmos_sensor(0x42, 0x8a);
    HI256_write_cmos_sensor(0x43, 0x86);
    HI256_write_cmos_sensor(0x44, 0x0a);
    HI256_write_cmos_sensor(0x45, 0x84);
    HI256_write_cmos_sensor(0x46, 0x87);
    HI256_write_cmos_sensor(0x47, 0x9b);
    HI256_write_cmos_sensor(0x48, 0x23);

    //CMC POFS
    HI256_write_cmos_sensor(0x50, 0x8c);
    HI256_write_cmos_sensor(0x51, 0x0c);
    HI256_write_cmos_sensor(0x52, 0x00);
    HI256_write_cmos_sensor(0x53, 0x07);
    HI256_write_cmos_sensor(0x54, 0x17);
    HI256_write_cmos_sensor(0x55, 0x9d);
    HI256_write_cmos_sensor(0x56, 0x00);
    HI256_write_cmos_sensor(0x57, 0x0b);
    HI256_write_cmos_sensor(0x58, 0x89);

    HI256_write_cmos_sensor(0x80, 0x03);
    HI256_write_cmos_sensor(0x85, 0x40);
    HI256_write_cmos_sensor(0x87, 0x02);
    HI256_write_cmos_sensor(0x88, 0x00);
    HI256_write_cmos_sensor(0x89, 0x00);
    HI256_write_cmos_sensor(0x8a, 0x00);
    /////// PAGE 15 END ///////

    /////// PAGE 16 START ///////
    HI256_write_cmos_sensor(0x03, 0x16);
    HI256_write_cmos_sensor(0x10, 0x31);
    HI256_write_cmos_sensor(0x18, 0x37);// Double_AG 5e->37
    HI256_write_cmos_sensor(0x19, 0x36);// Double_AG 5e->36
    HI256_write_cmos_sensor(0x1a, 0x0e);
    HI256_write_cmos_sensor(0x1b, 0x01);
    HI256_write_cmos_sensor(0x1c, 0xdc);
    HI256_write_cmos_sensor(0x1d, 0xfe);

    //GMA Default
    HI256_write_cmos_sensor(0x30, 0x00);
    HI256_write_cmos_sensor(0x31, 0x08);
    HI256_write_cmos_sensor(0x32, 0x19);
    HI256_write_cmos_sensor(0x33, 0x32);
    HI256_write_cmos_sensor(0x34, 0x55);
    HI256_write_cmos_sensor(0x35, 0x73);
    HI256_write_cmos_sensor(0x36, 0x86);
    HI256_write_cmos_sensor(0x37, 0x96);
    HI256_write_cmos_sensor(0x38, 0xa6);
    HI256_write_cmos_sensor(0x39, 0xb6);
    HI256_write_cmos_sensor(0x3a, 0xc5);
    HI256_write_cmos_sensor(0x3b, 0xd0);
    HI256_write_cmos_sensor(0x3c, 0xd8);
    HI256_write_cmos_sensor(0x3d, 0xe0);
    HI256_write_cmos_sensor(0x3e, 0xe8);
    HI256_write_cmos_sensor(0x3f, 0xf0);
    HI256_write_cmos_sensor(0x40, 0xf7);
    HI256_write_cmos_sensor(0x41, 0xfe);
    HI256_write_cmos_sensor(0x42, 0xff);

    HI256_write_cmos_sensor(0x50, 0x00);
    HI256_write_cmos_sensor(0x51, 0x05);
    HI256_write_cmos_sensor(0x52, 0x1b);
    HI256_write_cmos_sensor(0x53, 0x36);
    HI256_write_cmos_sensor(0x54, 0x5a);
    HI256_write_cmos_sensor(0x55, 0x75);
    HI256_write_cmos_sensor(0x56, 0x89);
    HI256_write_cmos_sensor(0x57, 0x9c);
    HI256_write_cmos_sensor(0x58, 0xac);
    HI256_write_cmos_sensor(0x59, 0xb8);
    HI256_write_cmos_sensor(0x5a, 0xc7);
    HI256_write_cmos_sensor(0x5b, 0xd2);
    HI256_write_cmos_sensor(0x5c, 0xdc);
    HI256_write_cmos_sensor(0x5d, 0xe5);
    HI256_write_cmos_sensor(0x5e, 0xed);
    HI256_write_cmos_sensor(0x5f, 0xf2);
    HI256_write_cmos_sensor(0x60, 0xf7);
    HI256_write_cmos_sensor(0x61, 0xf9);
    HI256_write_cmos_sensor(0x62, 0xfa);

    HI256_write_cmos_sensor(0x70, 0x0c);
    HI256_write_cmos_sensor(0x71, 0x19);
    HI256_write_cmos_sensor(0x72, 0x21);
    HI256_write_cmos_sensor(0x73, 0x31);
    HI256_write_cmos_sensor(0x74, 0x55);
    HI256_write_cmos_sensor(0x75, 0x6e);
    HI256_write_cmos_sensor(0x76, 0x83);
    HI256_write_cmos_sensor(0x77, 0x96);
    HI256_write_cmos_sensor(0x78, 0xa6);
    HI256_write_cmos_sensor(0x79, 0xb5);
    HI256_write_cmos_sensor(0x7a, 0xc2);
    HI256_write_cmos_sensor(0x7b, 0xcd);
    HI256_write_cmos_sensor(0x7c, 0xd7);
    HI256_write_cmos_sensor(0x7d, 0xe0);
    HI256_write_cmos_sensor(0x7e, 0xe8);
    HI256_write_cmos_sensor(0x7f, 0xf0);
    HI256_write_cmos_sensor(0x80, 0xf6);
    HI256_write_cmos_sensor(0x81, 0xfc);
    HI256_write_cmos_sensor(0x82, 0xff);
    /////// PAGE 16 END ///////

    /////// PAGE 17 START ///////
    HI256_write_cmos_sensor(0x03, 0x17);
    HI256_write_cmos_sensor(0x10, 0xf7);
    HI256_write_cmos_sensor(0xC4, 0x66); //FLK200 
    HI256_write_cmos_sensor(0xC5, 0x55); //FLK240 

    /////// PAGE 17 END ///////

    /////// PAGE 20 START ///////
    HI256_write_cmos_sensor(0x03, 0x20);
    HI256_write_cmos_sensor(0x11, 0x1c);
    HI256_write_cmos_sensor(0x18, 0x30);
    HI256_write_cmos_sensor(0x1a, 0x08);
    HI256_write_cmos_sensor(0x20, 0x01);
    HI256_write_cmos_sensor(0x21, 0x30);
    HI256_write_cmos_sensor(0x22, 0x10);
    HI256_write_cmos_sensor(0x23, 0x00);
    HI256_write_cmos_sensor(0x24, 0x00);

    HI256_write_cmos_sensor(0x28, 0xe7);
    HI256_write_cmos_sensor(0x29, 0x0d); //20100305 ad->0d
    HI256_write_cmos_sensor(0x2a, 0xff); 
    HI256_write_cmos_sensor(0x2b, 0x34); 
    //HI256_write_cmos_sensor(0x30, 0x78); 


    HI256_write_cmos_sensor(0x2c, 0xc3);
    HI256_write_cmos_sensor(0x2d, 0xcf);
    HI256_write_cmos_sensor(0x2e, 0x33);
    HI256_write_cmos_sensor(0x30, 0x78);
    HI256_write_cmos_sensor(0x32, 0x03);
    HI256_write_cmos_sensor(0x33, 0x2e);
    HI256_write_cmos_sensor(0x34, 0x30);
    HI256_write_cmos_sensor(0x35, 0xd4);
    HI256_write_cmos_sensor(0x36, 0xfe);
    HI256_write_cmos_sensor(0x37, 0x32);
    HI256_write_cmos_sensor(0x38, 0x04);	
    HI256_write_cmos_sensor(0x39, 0x22);
    HI256_write_cmos_sensor(0x3a, 0xde);
    HI256_write_cmos_sensor(0x3b, 0x22);
    HI256_write_cmos_sensor(0x3c, 0xde);

    HI256_write_cmos_sensor(0x50, 0x45);
    HI256_write_cmos_sensor(0x51, 0x88);

    HI256_write_cmos_sensor(0x56, 0x00);
    HI256_write_cmos_sensor(0x57, 0xff);
    HI256_write_cmos_sensor(0x58, 0x00);
    HI256_write_cmos_sensor(0x59, 0xff);
    HI256_write_cmos_sensor(0x5a, 0x04);

    //HI256_write_cmos_sensor(0x60, 0x55);
    //HI256_write_cmos_sensor(0x61, 0x55);
    //HI256_write_cmos_sensor(0x62, 0x6A);
    //HI256_write_cmos_sensor(0x63, 0xA9);
    //HI256_write_cmos_sensor(0x64, 0x6A);
    //HI256_write_cmos_sensor(0x65, 0xA9);
    //HI256_write_cmos_sensor(0x66, 0x6B);
    //HI256_write_cmos_sensor(0x67, 0xE9);
    //HI256_write_cmos_sensor(0x68, 0x6B);
    //HI256_write_cmos_sensor(0x69, 0xE9);
    //HI256_write_cmos_sensor(0x6a, 0x6A);
    //HI256_write_cmos_sensor(0x6b, 0xA9);
    //HI256_write_cmos_sensor(0x6c, 0x6A);
    //HI256_write_cmos_sensor(0x6d, 0xA9);
    //HI256_write_cmos_sensor(0x6e, 0x55);
    //HI256_write_cmos_sensor(0x6f, 0x55);


    HI256_write_cmos_sensor(0x60, 0x00);
    HI256_write_cmos_sensor(0x61, 0x00);
    HI256_write_cmos_sensor(0x62, 0x00);
    HI256_write_cmos_sensor(0x63, 0x00);
    HI256_write_cmos_sensor(0x64, 0x0a);
    HI256_write_cmos_sensor(0x65, 0xa0);
    HI256_write_cmos_sensor(0x66, 0x0b);
    HI256_write_cmos_sensor(0x67, 0xe0);
    HI256_write_cmos_sensor(0x68, 0x0b);
    HI256_write_cmos_sensor(0x69, 0xe0);
    HI256_write_cmos_sensor(0x6a, 0x0a);
    HI256_write_cmos_sensor(0x6b, 0xa0);
    HI256_write_cmos_sensor(0x6c, 0x00);
    HI256_write_cmos_sensor(0x6d, 0x00);
    HI256_write_cmos_sensor(0x6e, 0x00);
    HI256_write_cmos_sensor(0x6f, 0x00);

    HI256_write_cmos_sensor(0x70, 0x42); //6c
    //HI256_write_cmos_sensor(0x71, 0x80); //82(+8)
    HI256_write_cmos_sensor(0x76, 0x43);
    HI256_write_cmos_sensor(0x77, 0xe2);
    HI256_write_cmos_sensor(0x78, 0x23); //24
    HI256_write_cmos_sensor(0x79, 0x46); // Y Target 70 => 25, 72 => 26 //
    HI256_write_cmos_sensor(0x7a, 0x23); //23
    HI256_write_cmos_sensor(0x7b, 0x22); //22
    HI256_write_cmos_sensor(0x7d, 0x23);

    HI256_write_cmos_sensor(0x83, 0x06); //EXP Normal 33.33 fps 
    HI256_write_cmos_sensor(0x84, 0x2e); 
    HI256_write_cmos_sensor(0x85, 0x08); 
    HI256_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
    HI256_write_cmos_sensor(0x87, 0xf4); 
    HI256_write_cmos_sensor(0x88, 0x06); //EXP Max 8.33 fps 
    HI256_write_cmos_sensor(0x89, 0x2e); 
    HI256_write_cmos_sensor(0x8a, 0x08); 
    HI256_write_cmos_sensor(0x8B, 0x7e); //EXP100 
    HI256_write_cmos_sensor(0x8C, 0xf4); 
    HI256_write_cmos_sensor(0x8D, 0x69); //EXP120 
    HI256_write_cmos_sensor(0x8E, 0x78); 
    HI256_write_cmos_sensor(0x9c, 0x0b); //EXP Limit 541.67 fps 
    HI256_write_cmos_sensor(0x9d, 0xb8); 
    HI256_write_cmos_sensor(0x9e, 0x01); //EXP Unit 
    HI256_write_cmos_sensor(0x9f, 0xf4); 

    HI256_write_cmos_sensor(0xb0, 0x18);
    HI256_write_cmos_sensor(0xb1, 0x14);
    HI256_write_cmos_sensor(0xb2, 0x80);
    HI256_write_cmos_sensor(0xb3, 0x18);
    HI256_write_cmos_sensor(0xb4, 0x1a);
    HI256_write_cmos_sensor(0xb5, 0x44);
    HI256_write_cmos_sensor(0xb6, 0x2f);
    HI256_write_cmos_sensor(0xb7, 0x28);
    HI256_write_cmos_sensor(0xb8, 0x25);
    HI256_write_cmos_sensor(0xb9, 0x22);
    HI256_write_cmos_sensor(0xba, 0x21);
    HI256_write_cmos_sensor(0xbb, 0x20);
    HI256_write_cmos_sensor(0xbc, 0x1f);
    HI256_write_cmos_sensor(0xbd, 0x1f);

    HI256_write_cmos_sensor(0xc0, 0x14);
    HI256_write_cmos_sensor(0xc1, 0x1f);
    HI256_write_cmos_sensor(0xc2, 0x1f);
    HI256_write_cmos_sensor(0xc3, 0x18);
    HI256_write_cmos_sensor(0xc4, 0x10);

    HI256_write_cmos_sensor(0xc8, 0x80);
    HI256_write_cmos_sensor(0xc9, 0x40);
    /////// PAGE 20 END ///////

    /////// PAGE 22 START ///////
    HI256_write_cmos_sensor(0x03, 0x22);
    HI256_write_cmos_sensor(0x10, 0xff);
    HI256_write_cmos_sensor(0x11, 0x2e);
    HI256_write_cmos_sensor(0x19, 0x01); // Low On //
    HI256_write_cmos_sensor(0x20, 0x30);
    HI256_write_cmos_sensor(0x21, 0x80);
    HI256_write_cmos_sensor(0x24, 0x01);
    HI256_write_cmos_sensor(0x25, 0x7e); //7f New Lock Cond & New light stable

    HI256_write_cmos_sensor(0x30, 0x7f);
    HI256_write_cmos_sensor(0x31, 0x80);
    HI256_write_cmos_sensor(0x38, 0x11);
    HI256_write_cmos_sensor(0x39, 0x34);
    HI256_write_cmos_sensor(0x40, 0xf7);

    HI256_write_cmos_sensor(0x41, 0x33); //33//73
    HI256_write_cmos_sensor(0x42, 0x22); //22
    HI256_write_cmos_sensor(0x43, 0xf6);
    HI256_write_cmos_sensor(0x44, 0x55);
    HI256_write_cmos_sensor(0x45, 0x44);
    HI256_write_cmos_sensor(0x46, 0x00);
    HI256_write_cmos_sensor(0x50, 0xb2);
    HI256_write_cmos_sensor(0x51, 0x81);
    HI256_write_cmos_sensor(0x52, 0x98);

    HI256_write_cmos_sensor(0x80, 0x40);
    HI256_write_cmos_sensor(0x81, 0x20);
    HI256_write_cmos_sensor(0x82, 0x3e); //3a

    HI256_write_cmos_sensor(0x83, 0x4e);
    HI256_write_cmos_sensor(0x84, 0x16);
    HI256_write_cmos_sensor(0x85, 0x52);
    HI256_write_cmos_sensor(0x86, 0x30);

    HI256_write_cmos_sensor(0x87, 0x40);
    HI256_write_cmos_sensor(0x88, 0x35);
    HI256_write_cmos_sensor(0x89, 0x35); //38
    HI256_write_cmos_sensor(0x8a, 0x35); //2a

    HI256_write_cmos_sensor(0x8b, 0x38); //47
    HI256_write_cmos_sensor(0x8c, 0x33); 
    HI256_write_cmos_sensor(0x8d, 0x33); 
    HI256_write_cmos_sensor(0x8e, 0x25); //2c

    HI256_write_cmos_sensor(0x8f, 0x60);
    HI256_write_cmos_sensor(0x90, 0x5f);
    HI256_write_cmos_sensor(0x91, 0x5c);
    HI256_write_cmos_sensor(0x92, 0x4c);
    HI256_write_cmos_sensor(0x93, 0x41);
    HI256_write_cmos_sensor(0x94, 0x3b);
    HI256_write_cmos_sensor(0x95, 0x36);//2a
    HI256_write_cmos_sensor(0x96, 0x30);//24
    HI256_write_cmos_sensor(0x97, 0x27);//20
    HI256_write_cmos_sensor(0x98, 0x20);
    HI256_write_cmos_sensor(0x99, 0x1c);
    HI256_write_cmos_sensor(0x9a, 0x19);

    HI256_write_cmos_sensor(0x9b, 0x88);
    HI256_write_cmos_sensor(0x9c, 0x88);
    HI256_write_cmos_sensor(0x9d, 0x48);
    HI256_write_cmos_sensor(0x9e, 0x38);
    HI256_write_cmos_sensor(0x9f, 0x30);

    HI256_write_cmos_sensor(0xa0, 0x60);
    HI256_write_cmos_sensor(0xa1, 0x34);
    HI256_write_cmos_sensor(0xa2, 0x6f);
    HI256_write_cmos_sensor(0xa3, 0xff);

    HI256_write_cmos_sensor(0xa4, 0x14); //1500fps
    HI256_write_cmos_sensor(0xa5, 0x2c); // 700fps
    HI256_write_cmos_sensor(0xa6, 0xcf);

    HI256_write_cmos_sensor(0xad, 0x40);
    HI256_write_cmos_sensor(0xae, 0x4a);

    HI256_write_cmos_sensor(0xaf, 0x28);  // low temp Rgain
    HI256_write_cmos_sensor(0xb0, 0x26);  // low temp Rgain

    HI256_write_cmos_sensor(0xb1, 0x00); //0x20 -> 0x00 0405 modify
    HI256_write_cmos_sensor(0xb4, 0xea);
    HI256_write_cmos_sensor(0xb8, 0xa0); //a2: b-2, R+2 //b4 B-3, R+4 lowtemp
    HI256_write_cmos_sensor(0xb9, 0x00);
    /////// PAGE 22 END ///////

    /////// PAGE 48 (MiPi 1600x1200) ///////
    HI256_write_cmos_sensor(0x03, 0x48);

    // PLL Setting //
    HI256_write_cmos_sensor(0x70, 0x05);
    HI256_write_cmos_sensor(0x71, 0x30);		//Pllx1
    HI256_write_cmos_sensor(0x72, 0x85);		  // Full size & preview2 0x81, HBIN+preview1 0x85
    HI256_write_cmos_sensor(0x73, 0x10);
    HI256_write_cmos_sensor(0x70, 0x85);		// PLL Enable
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x70, 0x95);		// CLK_GEN_ENABLE

    // MIPI TX Setting //
    HI256_write_cmos_sensor(0x10, 0x1c);
    HI256_write_cmos_sensor(0x11, 0x10);
    HI256_write_cmos_sensor(0x12, 0x00);
    HI256_write_cmos_sensor(0x14, 0x00);		//HI256_write_cmos_sensor(0x14, 0x70);
    HI256_write_cmos_sensor(0x16, 0x04);
    HI256_write_cmos_sensor(0x18, 0x80);
    HI256_write_cmos_sensor(0x19, 0x00);
    HI256_write_cmos_sensor(0x1a, 0xa0);
    HI256_write_cmos_sensor(0x1b, 0x0d);
    HI256_write_cmos_sensor(0x1c, 0x01);
    HI256_write_cmos_sensor(0x1d, 0x0a);
    HI256_write_cmos_sensor(0x1e, 0x07);
    HI256_write_cmos_sensor(0x1f, 0x0b);
    //HI256_write_cmos_sensor(0x20, 0x00);

    HI256_write_cmos_sensor(0x23, 0x01);
    HI256_write_cmos_sensor(0x24, 0x1e);
    HI256_write_cmos_sensor(0x25, 0x00);
    HI256_write_cmos_sensor(0x26, 0x00);
    HI256_write_cmos_sensor(0x27, 0x08);
    HI256_write_cmos_sensor(0x28, 0x00);
    //HI256_write_cmos_sensor(0x2a, 0x06);
    //HI256_write_cmos_sensor(0x2b, 0x40);
    //HI256_write_cmos_sensor(0x2c, 0x04);
    //HI256_write_cmos_sensor(0x2d, 0xb0);

    HI256_write_cmos_sensor(0x30, 0x06);
    HI256_write_cmos_sensor(0x31, 0x40);

    HI256_write_cmos_sensor(0x32, 0x13);
    HI256_write_cmos_sensor(0x33, 0x0c);
    HI256_write_cmos_sensor(0x34, 0x04);
    HI256_write_cmos_sensor(0x35, 0x06);
    HI256_write_cmos_sensor(0x36, 0x01);
    HI256_write_cmos_sensor(0x37, 0x06);
    HI256_write_cmos_sensor(0x39, 0x4f);

    /////// PAGE 20 ///////
    HI256_write_cmos_sensor(0x03, 0x20);
    HI256_write_cmos_sensor(0x10, 0x9c);

    /////// PAGE 22 ///////
    HI256_write_cmos_sensor(0x03, 0x22);
    HI256_write_cmos_sensor(0x10, 0xe9);

    /////// PAGE 0 ///////
    HI256_write_cmos_sensor(0x03, 0x00);

    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x03, 0x00);

    HI256_write_cmos_sensor(0x0e, 0x03);
    HI256_write_cmos_sensor(0x0e, 0x73);

    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x01, 0x00);
    HI256_write_cmos_sensor(0x0a, 0x00);
    HI256_write_cmos_sensor(0x08, 0x0f);
    HI256_write_cmos_sensor(0x00, 0x00);

}


kal_uint32 HI256_read_shutter(void)
{
    kal_uint8 temp_reg0, temp_reg1, temp_reg2;
    kal_uint32 shutter;

    HI256_write_cmos_sensor(0x03, 0x20); 
    temp_reg0 = HI256_read_cmos_sensor(0x80); 
    temp_reg1 = HI256_read_cmos_sensor(0x81);
    temp_reg2 = HI256_read_cmos_sensor(0x82); 
    shutter = (temp_reg0 << 16) | (temp_reg1 << 8) | (temp_reg2 & 0xFF);

    return shutter;
}   

static void HI256_write_shutter(kal_uint16 shutter)
{
    SENSORDB("[hi256] %s \n",__func__);
    HI256_write_cmos_sensor(0x03, 0x20);
    HI256_write_cmos_sensor(0x83, shutter >> 16);			
    HI256_write_cmos_sensor(0x84, (shutter >> 8) & 0x00FF);	
    HI256_write_cmos_sensor(0x85, shutter & 0x0000FF);		
}    

void HI256_night_mode(kal_bool enable)	
{
    SENSORDB("[hi256] %s ==enable =%d \n",__func__,enable);
    if (HI256_sensor_cap_state == KAL_TRUE) 
    {
        return ;	
    }
    if (enable) 
    {

        if (HI256_Banding_setting == AE_FLICKER_MODE_50HZ) 
        {
            HI256_write_cmos_sensor(0x03,0x00); 	
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x01;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x1c);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x09); //EXP Normal 33.33 fps 
            HI256_write_cmos_sensor(0x84, 0xeb); 
            HI256_write_cmos_sensor(0x85, 0x10); 
            HI256_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI256_write_cmos_sensor(0x87, 0xf4); 
            HI256_write_cmos_sensor(0x88, 0x09); //EXP Max 8.33 fps 
            HI256_write_cmos_sensor(0x89, 0xeb); 
            HI256_write_cmos_sensor(0x8a, 0x10); 

            HI256_write_cmos_sensor(0x03, 0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x00;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x9c);

            HI256_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        } 
        else
        {
            HI256_write_cmos_sensor(0x03,0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x01;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x1c);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x09); //EXP Normal 33.33 fps 
            HI256_write_cmos_sensor(0x84, 0xe3); 
            HI256_write_cmos_sensor(0x85, 0x40); 
            HI256_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI256_write_cmos_sensor(0x87, 0xf4); 
            HI256_write_cmos_sensor(0x88, 0x09); //EXP Max 8.33 fps 
            HI256_write_cmos_sensor(0x89, 0xe3); 
            HI256_write_cmos_sensor(0x8a, 0x40); 

            HI256_write_cmos_sensor(0x03, 0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x00;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x8c);

            HI256_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        }
    } 
    else 
    {

        if (HI256_Banding_setting == AE_FLICKER_MODE_50HZ)
        {
            HI256_write_cmos_sensor(0x03,0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x01;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);
            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x1c);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x05); //EXP Normal 33.33 fps
            HI256_write_cmos_sensor(0x84, 0xf3);
            HI256_write_cmos_sensor(0x85, 0x70);
            HI256_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI256_write_cmos_sensor(0x87, 0xf4);
            HI256_write_cmos_sensor(0x88, 0x05); //EXP Max 8.33 fps
            HI256_write_cmos_sensor(0x89, 0xf3);
            HI256_write_cmos_sensor(0x8a, 0x70);

            HI256_write_cmos_sensor(0x03, 0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x00;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x9c);

            HI256_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        }
        else
        {
            HI256_write_cmos_sensor(0x03,0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x01;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);
            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x1c);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x06); //EXP Normal 33.33 fps
            HI256_write_cmos_sensor(0x84, 0x2e);
            HI256_write_cmos_sensor(0x85, 0x08);
            HI256_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
            HI256_write_cmos_sensor(0x87, 0xf4);
            HI256_write_cmos_sensor(0x88, 0x06); //EXP Max 8.33 fps
            HI256_write_cmos_sensor(0x89, 0x2e);
            HI256_write_cmos_sensor(0x8a, 0x08);

            HI256_write_cmos_sensor(0x03, 0x00);
            HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
            HI256_Sleep_Mode |= 0x00;
            HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x8c);

            HI256_write_cmos_sensor(0x18, 0x30);
            msleep(10);
        }
    }
}

void HI256_Initial_Cmds(void)
{
    kal_uint16 i,cnt;
    HI256_Init_Cmds();
}

UINT32 HI256Open(void)
{
    SENSORDB("[hi256] %s \n",__func__);
    volatile signed char i;
    kal_uint32 sensor_id=0;
    kal_uint8 temp_sccb_addr = 0;

    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x01, 0xf1);
    HI256_write_cmos_sensor(0x01, 0xf3);
    HI256_write_cmos_sensor(0x01, 0xf1);

    HI256_write_cmos_sensor(0x01, 0xf1);
    HI256_write_cmos_sensor(0x01, 0xf3);
    HI256_write_cmos_sensor(0x01, 0xf1);
    
    do{
        for (i=0; i < 3; i++)
        {
            sensor_id = HI256_read_cmos_sensor(0x04);
        
            if (sensor_id == HI256_SENSOR_ID)
            {
#ifdef HI256_DEBUG
                printk("[HI256YUV]:Read Sensor ID succ:0x%x\n", sensor_id);  
#endif
                break;
            }
        }

        mdelay(20);
    }while(0);

#ifdef HI256_DEBUG
    printk("[HI256YUV]:Read Sensor ID pass:0x%x\n", sensor_id);
#endif

    if (sensor_id != HI256_SENSOR_ID)
    {
	
#ifdef HI256_DEBUG
        printk("[HI256YUV]:Read Sensor ID fail:0x%x\n", sensor_id);  
#endif
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    else
    {
    	SubCameraDigtalPDNCtrl(1);
    }

    HI256_Initial_Cmds();
    //HI256_set_mirror_flip(3);

    return ERROR_NONE;
}	

UINT32 HI256Close(void)
{
    //CISModulePowerOn(FALSE);
    //kdModulePowerOn((CAMERA_DUAL_CAMERA_SENSOR_ENUM) g_currDualSensorIdx, g_currSensorName,false, CAMERA_HW_DRVNAME);
    return ERROR_NONE;
}	

static void HI256_set_mirror_flip(kal_uint8 image_mirror)
{
    kal_uint8 HI256_HV_Mirror;
    SENSORDB("[hi256] %s \n",__func__);
    HI256_write_cmos_sensor(0x03,0x00); 	
    HI256_HV_Mirror = (HI256_read_cmos_sensor(0x11) & 0xfc);

    switch (image_mirror) {
        case IMAGE_NORMAL:		
            HI256_HV_Mirror |= 0x03;
            break;
        case IMAGE_H_MIRROR:
            HI256_HV_Mirror |= 0x01;
            break;
        case IMAGE_V_MIRROR:
            HI256_HV_Mirror |= 0x02; 
            break;
        case IMAGE_HV_MIRROR:
            HI256_HV_Mirror |= 0x00; 
            break;
        default:
            break;
    }
    HI256_write_cmos_sensor(0x11, HI256_HV_Mirror);
}

UINT32 HI256Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{	
    SENSORDB("[hi256] %s \n",__func__);
    kal_uint16  iStartX = 0, iStartY = 0;

    HI256_sensor_cap_state = KAL_FALSE;
    HI256_sensor_pclk=390;

    HI256_gPVmode = KAL_TRUE;

    if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        HI256_VEDIO_encode_mode = KAL_TRUE;
    }
    else
    {
        HI256_VEDIO_encode_mode = KAL_FALSE;
    }

    HI256_write_cmos_sensor(0x03,0x00); 	
    HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
    HI256_Sleep_Mode |= 0x01;
    HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

    HI256_write_cmos_sensor(0x03, 0x20); 
    HI256_write_cmos_sensor(0x10, 0x1c);
    HI256_write_cmos_sensor(0x03, 0x22);
    HI256_write_cmos_sensor(0x10, 0x69);

    HI256_write_cmos_sensor(0x03, 0x00);
    HI256_write_cmos_sensor(0x10, 0x10);
    HI256_write_cmos_sensor(0x12, 0x04);

    HI256_write_cmos_sensor(0x20, 0x00); 
    HI256_write_cmos_sensor(0x21, 0x04);
    HI256_write_cmos_sensor(0x22, 0x00);
    HI256_write_cmos_sensor(0x23, 0x07);

    HI256_write_cmos_sensor(0x40, 0x01); //Hblank 360
    HI256_write_cmos_sensor(0x41, 0x68); 
    HI256_write_cmos_sensor(0x42, 0x00); //Vblank 18
    HI256_write_cmos_sensor(0x43, 0x14); 

    HI256_write_cmos_sensor(0x03, 0x20); 
    //HI256_write_cmos_sensor(0x18, 0x38);

    HI256_write_cmos_sensor(0x86, 0x01); //EXPMin 6500.00 fps
    HI256_write_cmos_sensor(0x87, 0xf4); 

    HI256_write_cmos_sensor(0x8B, 0x7e); //EXP100 
    HI256_write_cmos_sensor(0x8C, 0xf4); 
    HI256_write_cmos_sensor(0x8D, 0x69); //EXP120 
    HI256_write_cmos_sensor(0x8E, 0x78);

    HI256_write_cmos_sensor(0x9c, 0x0b); //EXP Limit 1083.33fps 
    HI256_write_cmos_sensor(0x9d, 0xb8); 
    HI256_write_cmos_sensor(0x9e, 0x01); //EXP Unit 
    HI256_write_cmos_sensor(0x9f, 0xf4); 

    /////// ****************PAGE 48 (MiPi 1600x1200) ***********///////
    HI256_write_cmos_sensor(0x03, 0x48);
    // PLL Setting //
    HI256_write_cmos_sensor(0x70, 0x05);
    HI256_write_cmos_sensor(0x71, 0x30);		//Pllx1
    HI256_write_cmos_sensor(0x72, 0x85);		  // Full size & preview2 0x81, HBIN+preview1 0x85
    HI256_write_cmos_sensor(0x73, 0x10);
    HI256_write_cmos_sensor(0x70, 0x85);		// PLL Enable
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x03, 0x48);
    HI256_write_cmos_sensor(0x70, 0x95);		// CLK_GEN_ENABLE

    // MIPI TX Setting //
    HI256_write_cmos_sensor(0x10, 0x1c);
    HI256_write_cmos_sensor(0x11, 0x10);
    HI256_write_cmos_sensor(0x12, 0x00);
    HI256_write_cmos_sensor(0x14, 0x00);		//HI256_write_cmos_sensor(0x14, 0x70);
    HI256_write_cmos_sensor(0x16, 0x04);
    HI256_write_cmos_sensor(0x18, 0x80);
    HI256_write_cmos_sensor(0x19, 0x00);
    HI256_write_cmos_sensor(0x1a, 0xa0);
    HI256_write_cmos_sensor(0x1b, 0x0d);
    HI256_write_cmos_sensor(0x1c, 0x01);
    HI256_write_cmos_sensor(0x1d, 0x0a);
    HI256_write_cmos_sensor(0x1e, 0x07);
    HI256_write_cmos_sensor(0x1f, 0x0b);
    //HI256_write_cmos_sensor(0x20, 0x00);

    HI256_write_cmos_sensor(0x23, 0x01);
    HI256_write_cmos_sensor(0x24, 0x1e);
    HI256_write_cmos_sensor(0x25, 0x00);
    HI256_write_cmos_sensor(0x26, 0x00);
    HI256_write_cmos_sensor(0x27, 0x08);
    HI256_write_cmos_sensor(0x28, 0x00);
    //HI256_write_cmos_sensor(0x2a, 0x06);
    //HI256_write_cmos_sensor(0x2b, 0x40);
    //HI256_write_cmos_sensor(0x2c, 0x04);
    //HI256_write_cmos_sensor(0x2d, 0xb0);

    HI256_write_cmos_sensor(0x30, 0x06);
    HI256_write_cmos_sensor(0x31, 0x40);

    HI256_write_cmos_sensor(0x32, 0x13);
    HI256_write_cmos_sensor(0x33, 0x0c);
    HI256_write_cmos_sensor(0x34, 0x04);
    HI256_write_cmos_sensor(0x35, 0x06);
    HI256_write_cmos_sensor(0x36, 0x01);
    HI256_write_cmos_sensor(0x37, 0x06);
    HI256_write_cmos_sensor(0x39, 0x4f);
    ///////*************** MiPi End******************* ///////

    HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
    HI256_Sleep_Mode |= 0x00;
    HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

    HI256_write_cmos_sensor(0x03, 0x20);//page 20
    HI256_write_cmos_sensor(0x10, 0x9c);//AE ON

    HI256_write_cmos_sensor(0x03, 0x22);
    HI256_write_cmos_sensor(0x10, 0xe9);//AWB ON

    //HI256_write_cmos_sensor(0x03, 0x20);
    //HI256_write_cmos_sensor(0x18, 0x30);

    image_window->GrabStartX = iStartX;
    image_window->GrabStartY = iStartY;
    image_window->ExposureWindowWidth = HI256_IMAGE_SENSOR_PV_WIDTH - 16;
    image_window->ExposureWindowHeight = HI256_IMAGE_SENSOR_PV_HEIGHT - 12;
    msleep(10);
    // copy sensor_config_data
    memcpy(&HI256SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}	

UINT32 HI256Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint8 temp_AE_reg;
    kal_uint8 CLK_DIV_REG = 0;
    SENSORDB("[hi256] %s \n",__func__);
    HI256_sensor_cap_state = KAL_TRUE;
#if 0
    if ((image_window->ImageTargetWidth<=HI256_IMAGE_SENSOR_PV_WIDTH)&&
        (image_window->ImageTargetHeight<=HI256_IMAGE_SENSOR_PV_HEIGHT))
    {
        /* Less than PV Mode */
        HI256_gPVmode=KAL_TRUE;
        HI256_capture_pclk_in_M = HI256_preview_pclk_in_M;   //Don't change the clk

        HI256_write_cmos_sensor(0x03, 0x00);

        HI256_write_cmos_sensor(0x20, 0x00); 
        HI256_write_cmos_sensor(0x21, 0x04); 
        HI256_write_cmos_sensor(0x22, 0x00); 
        HI256_write_cmos_sensor(0x23, 0x07); 

        HI256_write_cmos_sensor(0x40, 0x01); //Hblank 360
        HI256_write_cmos_sensor(0x41, 0x68); 
        HI256_write_cmos_sensor(0x42, 0x00); //Vblank 18
        HI256_write_cmos_sensor(0x43, 0x12); 

        HI256_write_cmos_sensor(0x03, 0x10);
        HI256_write_cmos_sensor(0x3f, 0x00);	

        //Page12
        HI256_write_cmos_sensor(0x03, 0x12); //Function
        HI256_write_cmos_sensor(0x20, 0x0f);
        HI256_write_cmos_sensor(0x21, 0x0f);
        HI256_write_cmos_sensor(0x90, 0x5d);  

        //Page13
        HI256_write_cmos_sensor(0x03, 0x13); //Function
        HI256_write_cmos_sensor(0x80, 0xfd); //Function

        // 800*600	
        HI256_write_cmos_sensor(0x03, 0x00);
        HI256_write_cmos_sensor(0x10, 0x11);

        HI256_write_cmos_sensor(0x03, 0x48);
        HI256_write_cmos_sensor(0x72, 0x81); 
        HI256_write_cmos_sensor(0x30, 0x06);
        HI256_write_cmos_sensor(0x31, 0x40);

        HI256_write_cmos_sensor(0x03, 0x20);
        HI256_pv_HI256_exposure_lines = (HI256_read_cmos_sensor(0x80) << 16)|(HI256_read_cmos_sensor(0x81) << 8)|HI256_read_cmos_sensor(0x82);

        HI256_cp_HI256_exposure_lines=HI256_pv_HI256_exposure_lines;	

        if(HI256_cp_HI256_exposure_lines<1)
            HI256_cp_HI256_exposure_lines=1;

        HI256_write_cmos_sensor(0x03, 0x20); 
        HI256_write_cmos_sensor(0x83, HI256_cp_HI256_exposure_lines >> 16);
        HI256_write_cmos_sensor(0x84, (HI256_cp_HI256_exposure_lines >> 8) & 0x000000FF);
        HI256_write_cmos_sensor(0x85, HI256_cp_HI256_exposure_lines & 0x000000FF);

        image_window->GrabStartX = 1;
        image_window->GrabStartY = 1;
        image_window->ExposureWindowWidth= HI256_IMAGE_SENSOR_PV_WIDTH - 16;
        image_window->ExposureWindowHeight = HI256_IMAGE_SENSOR_PV_HEIGHT - 12;
    }
    else 
#endif
    {    
        /* 2M FULL Mode */
        HI256_gPVmode = KAL_FALSE;

        HI256_write_cmos_sensor(0x03,0x00); 	
        HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
        HI256_Sleep_Mode |= 0x01;
        HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);

        CLK_DIV_REG=(HI256_read_cmos_sensor(0x12)&0xFc);    // don't divide,PCLK=48M
        CLK_DIV_REG |= 0x00;
        //read the shutter (manual exptime)
        HI256_write_cmos_sensor(0x03, 0x20);
        HI256_pv_HI256_exposure_lines = (HI256_read_cmos_sensor(0x80) << 16)|(HI256_read_cmos_sensor(0x81) << 8)|HI256_read_cmos_sensor(0x82);

        HI256_cp_HI256_exposure_lines = HI256_pv_HI256_exposure_lines;
		
        HI256_write_cmos_sensor(0x03, 0x00);

        HI256_write_cmos_sensor(0x20, 0x00); 
        HI256_write_cmos_sensor(0x21, 0x0a); 
        HI256_write_cmos_sensor(0x22, 0x00); 
        HI256_write_cmos_sensor(0x23, 0x0a); 

        HI256_write_cmos_sensor(0x40, 0x01); //Hblank 360
        HI256_write_cmos_sensor(0x41, 0x68); 
        HI256_write_cmos_sensor(0x42, 0x00); //Vblank 18
        HI256_write_cmos_sensor(0x43, 0x14); 

        HI256_write_cmos_sensor(0x03, 0x10);
        HI256_write_cmos_sensor(0x3f, 0x00);

        //Page12
        HI256_write_cmos_sensor(0x03, 0x12);
        HI256_write_cmos_sensor(0x20, 0x0f);
        HI256_write_cmos_sensor(0x21, 0x0f);
        HI256_write_cmos_sensor(0x90, 0x5d);

        //Page13
        HI256_write_cmos_sensor(0x03, 0x13);
        HI256_write_cmos_sensor(0x80, 0xfd);
	
        // 1600*1200	
        HI256_write_cmos_sensor(0x03, 0x00);
        HI256_write_cmos_sensor(0x10, 0x00);

        HI256_write_cmos_sensor(0x03, 0x48);
        HI256_write_cmos_sensor(0x72, 0x81); 
        HI256_write_cmos_sensor(0x30, 0x0c);
        HI256_write_cmos_sensor(0x31, 0x80);
#if 0
        if ((image_window->ImageTargetWidth<=HI256_IMAGE_SENSOR_FULL_WIDTH)&&
        (image_window->ImageTargetHeight<=HI256_IMAGE_SENSOR_FULL_HEIGHT))
        {
            HI256_capture_pclk_in_M = 520;
            HI256_sensor_pclk = 520;                 
        }
        else//Interpolate to 3M
        {
            HI256_capture_pclk_in_M = 520;
            HI256_sensor_pclk = 520;                
        }
#endif

        HI256_write_cmos_sensor(0x03, 0x00); 
        HI256_write_cmos_sensor(0x12,/*CLK_DIV_REG*/ 0x04);

        if(HI256_cp_HI256_exposure_lines<1)
            HI256_cp_HI256_exposure_lines=1;

        HI256_write_cmos_sensor(0x03, 0x20); 
        HI256_write_cmos_sensor(0x83, HI256_cp_HI256_exposure_lines >> 16);
        HI256_write_cmos_sensor(0x84, (HI256_cp_HI256_exposure_lines >> 8) & 0x000000FF);
        HI256_write_cmos_sensor(0x85, HI256_cp_HI256_exposure_lines & 0x000000FF);
	
        HI256_write_cmos_sensor(0x03,0x00); 	
        HI256_Sleep_Mode = (HI256_read_cmos_sensor(0x01) & 0xfe);
        HI256_Sleep_Mode |= 0x00;
        HI256_write_cmos_sensor(0x01, HI256_Sleep_Mode);       
#if 0
        image_window->GrabStartX=1;
        image_window->GrabStartY=1;
        image_window->ExposureWindowWidth=HI256_IMAGE_SENSOR_FULL_WIDTH - 16;
        image_window->ExposureWindowHeight=HI256_IMAGE_SENSOR_FULL_HEIGHT - 12;
#endif
    }

    msleep(10);
    // copy sensor_config_data
    memcpy(&HI256SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    HI256_CAPATURE_FLAG = 1;
    HI256_CAPATUREB_FLAG = 1;
    return ERROR_NONE;
}

UINT32 HI256GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("[hi256] %s \n",__func__);
    pSensorResolution->SensorFullWidth=HI256_FULL_GRAB_WIDTH;  
    pSensorResolution->SensorFullHeight=HI256_FULL_GRAB_HEIGHT;
    pSensorResolution->SensorPreviewWidth=HI256_PV_GRAB_WIDTH;
    pSensorResolution->SensorPreviewHeight=HI256_PV_GRAB_HEIGHT;
    pSensorResolution->SensorVideoWidth=HI256_PV_GRAB_WIDTH;
    pSensorResolution->SensorVideoHeight=HI256_PV_GRAB_HEIGHT;
    return ERROR_NONE;
}	

UINT32 HI256GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[hi256] %s \n",__func__);
    pSensorInfo->SensorPreviewResolutionX=HI256_PV_GRAB_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=HI256_PV_GRAB_HEIGHT;
    pSensorInfo->SensorFullResolutionX=HI256_FULL_GRAB_WIDTH;
    pSensorInfo->SensorFullResolutionY=HI256_FULL_GRAB_HEIGHT;

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

    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 3; 
    pSensorInfo->VideoDelayFrame = 20; 
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
            pSensorInfo->SensorGrabStartX = HI256_PV_GRAB_START_X;//0; 
            pSensorInfo->SensorGrabStartY = HI256_PV_GRAB_START_Y;//0;    		
#ifdef MIPI_INTERFACE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE; 		
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
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
            pSensorInfo->SensorGrabStartX = HI256_FULL_GRAB_START_X;//0; 
            pSensorInfo->SensorGrabStartY = HI256_FULL_GRAB_START_Y;//0;     		
#ifdef MIPI_INTERFACE
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
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
            pSensorInfo->SensorGrabStartX = HI256_PV_GRAB_START_X;//0; 
            pSensorInfo->SensorGrabStartY = HI256_PV_GRAB_START_Y;//0;     			
            break;
    }
    //HI256_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &HI256SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
}	

UINT32 HI256Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[hi256] %s ==ScenarioID=%d \n",__func__,ScenarioId);
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            HI256Preview(pImageWindow, pSensorConfigData);
            break;
	
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            HI256Capture(pImageWindow, pSensorConfigData);
            break;
	
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }

    return TRUE;
}	

BOOL HI256_set_param_wb(UINT16 para)
{
    SENSORDB("[hi256] %s ====para=%d \n",__func__,para);
    switch (para)
    {
        case AWB_MODE_AUTO:
            HI256_write_cmos_sensor(0x03, 0x22);			
            HI256_write_cmos_sensor(0x11, 0x2e);		
            HI256_write_cmos_sensor(0x83, 0x4E);
            HI256_write_cmos_sensor(0x84, 0x16);
            HI256_write_cmos_sensor(0x85, 0x50);
            HI256_write_cmos_sensor(0x86, 0x30);
            break;
        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
            HI256_write_cmos_sensor(0x03, 0x22);
            HI256_write_cmos_sensor(0x11, 0x28);
            HI256_write_cmos_sensor(0x80, 0x71);
            HI256_write_cmos_sensor(0x82, 0x2b);
            HI256_write_cmos_sensor(0x83, 0x72);
            HI256_write_cmos_sensor(0x84, 0x70);
            HI256_write_cmos_sensor(0x85, 0x2b);
            HI256_write_cmos_sensor(0x86, 0x28);
            break;
        case AWB_MODE_DAYLIGHT: //sunny
            HI256_write_cmos_sensor(0x03, 0x22);
            HI256_write_cmos_sensor(0x11, 0x28);		  
            HI256_write_cmos_sensor(0x80, 0x59);
            HI256_write_cmos_sensor(0x82, 0x29);
            HI256_write_cmos_sensor(0x83, 0x60);
            HI256_write_cmos_sensor(0x84, 0x50);
            HI256_write_cmos_sensor(0x85, 0x2f);
            HI256_write_cmos_sensor(0x86, 0x23);
            break;
        case AWB_MODE_INCANDESCENT: //office
            HI256_write_cmos_sensor(0x03, 0x22);
            HI256_write_cmos_sensor(0x11, 0x28);		  
            HI256_write_cmos_sensor(0x80, 0x29);
            HI256_write_cmos_sensor(0x82, 0x54);
            HI256_write_cmos_sensor(0x83, 0x2e);
            HI256_write_cmos_sensor(0x84, 0x23);
            HI256_write_cmos_sensor(0x85, 0x58);
            HI256_write_cmos_sensor(0x86, 0x4f);
            break;
        case AWB_MODE_TUNGSTEN: //home
            HI256_write_cmos_sensor(0x03, 0x22);
            HI256_write_cmos_sensor(0x80, 0x24);
            HI256_write_cmos_sensor(0x81, 0x20);
            HI256_write_cmos_sensor(0x82, 0x58);
            HI256_write_cmos_sensor(0x83, 0x27);
            HI256_write_cmos_sensor(0x84, 0x22);
            HI256_write_cmos_sensor(0x85, 0x58);
            HI256_write_cmos_sensor(0x86, 0x52);
            break;
        case AWB_MODE_FLUORESCENT:
            HI256_write_cmos_sensor(0x03, 0x22);
            HI256_write_cmos_sensor(0x11, 0x28);
            HI256_write_cmos_sensor(0x80, 0x41);
            HI256_write_cmos_sensor(0x82, 0x42);
            HI256_write_cmos_sensor(0x83, 0x44);
            HI256_write_cmos_sensor(0x84, 0x34);
            HI256_write_cmos_sensor(0x85, 0x46);
            HI256_write_cmos_sensor(0x86, 0x3a);
            break;	
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL HI256_set_param_effect(UINT16 para)
{
    SENSORDB("[hi256] %s ====para=%d \n",__func__,para);
    kal_uint32 ret = KAL_TRUE;
    switch (para)
    {
        case MEFFECT_OFF:
            HI256_write_cmos_sensor(0x03, 0x10);
            HI256_write_cmos_sensor(0x11, 0x03);
            HI256_write_cmos_sensor(0x12, 0x30);
            HI256_write_cmos_sensor(0x13, 0x0a);
            HI256_write_cmos_sensor(0x44, 0x80);
            HI256_write_cmos_sensor(0x45, 0x80);
            HI256_write_cmos_sensor(0x4a, 0x80);
            break;

        case MEFFECT_SEPIA:
            HI256_write_cmos_sensor(0x03, 0x10);
            HI256_write_cmos_sensor(0x11, 0x03);
            HI256_write_cmos_sensor(0x12, 0x33);
            HI256_write_cmos_sensor(0x13, 0x02);
            HI256_write_cmos_sensor(0x44, 0x70);
            HI256_write_cmos_sensor(0x45, 0x98);
            HI256_write_cmos_sensor(0x4a, 0x80);
            break;  
	
        case MEFFECT_NEGATIVE:		
            HI256_write_cmos_sensor(0x03, 0x10);
            HI256_write_cmos_sensor(0x11, 0x03);
            HI256_write_cmos_sensor(0x12, 0x38);
            HI256_write_cmos_sensor(0x13, 0x02);
            HI256_write_cmos_sensor(0x14, 0x00);
            HI256_write_cmos_sensor(0x4a, 0x80);
            break; 
	
        case MEFFECT_SEPIAGREEN:		
            HI256_write_cmos_sensor(0x03, 0x10);
            HI256_write_cmos_sensor(0x11, 0x03);
            HI256_write_cmos_sensor(0x12, 0x33);
            HI256_write_cmos_sensor(0x13, 0x02);
            HI256_write_cmos_sensor(0x44, 0x30);
            HI256_write_cmos_sensor(0x45, 0x50);
            HI256_write_cmos_sensor(0x4a, 0x80);
            break;
	
        case MEFFECT_SEPIABLUE:	
            HI256_write_cmos_sensor(0x03, 0x10);
            HI256_write_cmos_sensor(0x11, 0x03);
            HI256_write_cmos_sensor(0x12, 0x33);
            HI256_write_cmos_sensor(0x13, 0x02);
            HI256_write_cmos_sensor(0x44, 0xb0);
            HI256_write_cmos_sensor(0x45, 0x40);
            HI256_write_cmos_sensor(0x4a, 0x80);
            break;
	
        case MEFFECT_MONO:				
            HI256_write_cmos_sensor(0x03, 0x10);
            HI256_write_cmos_sensor(0x11, 0x03);
            HI256_write_cmos_sensor(0x12, 0x33);
            HI256_write_cmos_sensor(0x13, 0x02);
            HI256_write_cmos_sensor(0x44, 0x80);
            HI256_write_cmos_sensor(0x45, 0x80);
            HI256_write_cmos_sensor(0x4a, 0x80);
            break;

        default:
            ret = FALSE;
    }

    return ret;
}

BOOL HI256_set_param_banding(UINT16 para)
{
    kal_uint8 banding;
    
    SENSORDB("[hi256] %s \n",__func__);
    banding = HI256_read_cmos_sensor(0x3014);
    
    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
            SENSORDB("HJDDbg [hi256] %s, AE_FLICKER_MODE_50HZ\n", __func__);
            HI256_Banding_setting = AE_FLICKER_MODE_50HZ;
            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x1c);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x05);//50hz
            HI256_write_cmos_sensor(0x84, 0xf3);
            HI256_write_cmos_sensor(0x85, 0x70);

            HI256_write_cmos_sensor(0x88, 0x05);//50hz
            HI256_write_cmos_sensor(0x89, 0xf3);
            HI256_write_cmos_sensor(0x8a, 0x70);			

            HI256_write_cmos_sensor(0x18, 0x30);
            HI256_write_cmos_sensor(0x10, 0x9c);
            break;
            
        case AE_FLICKER_MODE_60HZ:
            SENSORDB("HJDDbg [hi256] %s, AE_FLICKER_MODE_60HZ\n", __func__);
            HI256_Banding_setting = AE_FLICKER_MODE_60HZ;
            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x0c);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x06); //60hz
            HI256_write_cmos_sensor(0x84, 0x2e);
            HI256_write_cmos_sensor(0x85, 0x08);

            HI256_write_cmos_sensor(0x88, 0x06); //60hz
            HI256_write_cmos_sensor(0x89, 0x2e);
            HI256_write_cmos_sensor(0x8a, 0x08);

            HI256_write_cmos_sensor(0x18, 0x30);
            HI256_write_cmos_sensor(0x10, 0x8c);
            break;
            
#ifdef OPEN_FLICKER_AUTO // Jiangde, auto-flicker need to be debugged for flicker at 50Hz
        case AE_FLICKER_MODE_AUTO:
            SENSORDB("HJDDbg [hi256] %s, Yes==OPEN_FLICKER_AUTO--AE_FLICKER_MODE_AUTO\n", __func__);
            HI256_Banding_setting = AE_FLICKER_MODE_AUTO;
            
        case AE_FLICKER_MODE_OFF:
            if (AE_FLICKER_MODE_OFF == para)
            {
                SENSORDB("HJDDbg [hi256] %s, Yes==OPEN_FLICKER_AUTO--AE_FLICKER_MODE_OFF\n", __func__);
                HI256_Banding_setting = AE_FLICKER_MODE_OFF;
            }
            
            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0x0c);
            HI256_write_cmos_sensor(0x18, 0x38);

            HI256_write_cmos_sensor(0x83, 0x05); //Auto flicker
            HI256_write_cmos_sensor(0x84, 0xf3);
            HI256_write_cmos_sensor(0x85, 0x70);

            HI256_write_cmos_sensor(0x88, 0x05); 
            HI256_write_cmos_sensor(0x89, 0xf3);
            HI256_write_cmos_sensor(0x8a, 0x70);
            HI256_write_cmos_sensor(0x18, 0x30);    

            HI256_write_cmos_sensor(0x03, 0x17);//Auto flicker
            HI256_write_cmos_sensor(0xc4, 0x41);
            HI256_write_cmos_sensor(0xc5, 0x36);

            HI256_write_cmos_sensor(0x03, 0x20);
            HI256_write_cmos_sensor(0x10, 0xdc);
            break;
#endif

        default:
            return FALSE;
    }

    return TRUE;
} 

BOOL HI256_set_param_exposure(UINT16 para)
{
    SENSORDB("[hi256] %s \n",__func__);
    HI256_write_cmos_sensor(0x03,0x10);
    HI256_write_cmos_sensor(0x12,(HI256_read_cmos_sensor(0x12)|0x10));//make sure the Yoffset control is opened.

    switch (para)
    {
        case AE_EV_COMP_n13:
            HI256_write_cmos_sensor(0x40,0xc0);
            break;
        case AE_EV_COMP_n10:
            HI256_write_cmos_sensor(0x40,0xb0);
            break;
        case AE_EV_COMP_n07:
            HI256_write_cmos_sensor(0x40,0xa0);
            break;
        case AE_EV_COMP_n03:
            HI256_write_cmos_sensor(0x40,0x98);
            break;
        case AE_EV_COMP_00:
            HI256_write_cmos_sensor(0x40,0x90);
            break;
        case AE_EV_COMP_03:
            HI256_write_cmos_sensor(0x40,0x10);
            break;
        case AE_EV_COMP_07:
            HI256_write_cmos_sensor(0x40,0x20);
            break;
        case AE_EV_COMP_10:
            HI256_write_cmos_sensor(0x40,0x30);
            break;
        case AE_EV_COMP_13:
            HI256_write_cmos_sensor(0x40,0x40);
            break;
        default:
            return FALSE;
    }

    return TRUE;
} 


UINT32 HI256YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
    SENSORDB("[hi256] %s \n",__func__);
    if (HI256_op_state.sensor_cap_state == KAL_TRUE)	/* Don't need it when capture mode. */
    {
    return KAL_TRUE;
    }

    switch (iCmd) 
    {
        case FID_SCENE_MODE:	    
            if( HI256_CAPATURE_FLAG == 0)
            {
                if (iPara == SCENE_MODE_OFF)
                {
                    HI256_night_mode(0);
                }
                else if (iPara == SCENE_MODE_NIGHTSCENE)
                {
                    HI256_night_mode(1);
                }
            }
            else
                HI256_CAPATURE_FLAG = 0;
            break; 

        case FID_AWB_MODE:
            HI256_set_param_wb(iPara);
            break;

        case FID_COLOR_EFFECT:	    
            HI256_set_param_effect(iPara);
            break;

        case FID_AE_EV:	       	      
            HI256_set_param_exposure(iPara);
            break;

        case FID_AE_FLICKER:    	    	    
            if( HI256_CAPATUREB_FLAG == 0)
                HI256_set_param_banding(iPara);
            else
                HI256_CAPATUREB_FLAG = 0;
            break;

        case FID_AE_SCENE_MODE: 
            break; 

        case FID_ZOOM_FACTOR:
            HI256_zoom_factor = iPara;
            break; 

        default:
            break;
    }
    return TRUE;
}   

UINT32 HI256YUVSetVideoMode(UINT16 u2FrameRate)
{
    SENSORDB("[hi256] %s \n",__func__);
    kal_uint16 temp_AE_reg = 0;
	
    HI256_write_cmos_sensor(0x03, 0x20); 
    temp_AE_reg = HI256_read_cmos_sensor(0x10);

    return TRUE;
}

UINT32 HI256FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
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
    SENSORDB("[hi256] %s =====FeatureID= %d \n",__func__,FeatureId);

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=HI256_FULL_GRAB_WIDTH;
            *pFeatureReturnPara16=HI256_FULL_GRAB_HEIGHT;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_GET_PERIOD:
            *pFeatureReturnPara16++=HI256_PV_PERIOD_PIXEL_NUMS+HI256_PV_dummy_pixels;
            *pFeatureReturnPara16=HI256_PV_PERIOD_LINE_NUMS+HI256_PV_dummy_lines;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *pFeatureReturnPara32 = HI256_sensor_pclk/10;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_SET_ESHUTTER:
            u2Temp = HI256_read_shutter(); 		
            break;
	
        case SENSOR_FEATURE_SET_NIGHTMODE:
            HI256_night_mode((BOOL) *pFeatureData16);
            break;
	
        case SENSOR_FEATURE_SET_GAIN:
            break; 
	
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
	
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            HI256_isp_master_clock=*pFeatureData32;
            break;
	
        case SENSOR_FEATURE_SET_REGISTER:
            HI256_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
	
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = HI256_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
	
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &HI256SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
            HI256_GetSensorID(pFeatureData32); 
            break;

        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;
	
        case SENSOR_FEATURE_SET_YUV_CMD:
            HI256YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
            break;	
	
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            HI256YUVSetVideoMode(*pFeatureData16);
            break; 

        default:
            break;			
    }
    return ERROR_NONE;
}	

SENSOR_FUNCTION_STRUCT	SensorFuncHI256=
{
    HI256Open,
    HI256GetInfo,
    HI256GetResolution,
    HI256FeatureControl,
    HI256Control,
    HI256Close
};

UINT32 HI256_MIPI_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncHI256;

    return ERROR_NONE;
}	

