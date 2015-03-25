#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <string.h>
#endif 

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>

#elif (defined BUILD_UBOOT)
#include <asm/arch/mt6577_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
//#include "ti_sn65dsi8x_driver.h"
#include "sn65dsi83_i2c.h"
#endif
#include "lcm_drv.h"


#define LVDS_PANEL_8BITS_SUPPORT
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (800)
#define FRAME_HEIGHT (1280) 

#define LVDS_LCM_STBY   	GPIO_LCM_MIPI2LVDS_EN //GPIO112 //GPIO179
//#define LVDS_LCM_RESET 	GPIO132 //GPIO178   
#define LVDS_LCM_BIASON 	GPIO_LCM_LVDS_EN //GPIO118 //GPIO131
#define LVDS_LCM_VCC        GPIO_LCM_LVDS_PWR_EN //GPIO119
#define LVDS_MIPI2_VCC_EN         GPIO_LCM_MIPI2LVDS_PWR_EN //GPIO113
//#define SN65DSI_DEBUG  //for check system(bb dsi and ti chip) statu
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

//static LCM_UTIL_FUNCS lcm_util = {0}; //fixed for build warning
static LCM_UTIL_FUNCS lcm_util = 
{
	.set_reset_pin = NULL,
	.udelay = NULL,
	.mdelay = NULL,
};


#define SET_RESET_PIN(v)    (mt_set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY 0xAB

typedef unsigned char    kal_uint8;


typedef struct {
    unsigned char cmd;
    unsigned char data;
} sn65dsi8x_setting_table;

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
//extern void DSI_Continuous_HS(void); //DSI_continuous_clock(void);
//extern void DSI_no_continuous_clock(void);
extern void DSI_clk_HS_mode(bool enter);

 //i2c hardware type -start
#ifdef BUILD_LK

#define I2C_CH                0
#define sn65dsi83_SLAVE_ADDR_WRITE/*sn65dsi83_I2C_ADDR*/       0x58  //0x2d //write ;0x5b read

#if 0
 U32 sn65dsi8x_reg_i2c_read (U8 addr, U8 *dataBuffer)
 {
	 U32 ret_code = I2C_OK;
	 U8 write_data = addr;
 
	 /* set register command */
	 ret_code = mt_i2c_write(I2C_CH, sn65dsi8x_I2C_ADDR, &write_data, 1, 0); // 0:I2C_PATH_NORMAL
 
	 if (ret_code != I2C_OK)
		 return ret_code;
 
	 ret_code = mt_i2c_read(I2C_CH, (sn65dsi8x_I2C_ADDR|0x1), dataBuffer, 1,0); // 0:I2C_PATH_NORMAL
 
	 return ret_code;
 }
 
 U32 sn65dsi8x_reg_i2c_write(U8 addr, U8 value)
 {
	 U32 ret_code = I2C_OK;
	 U8 write_data[2];
 
	 write_data[0]= addr;
	 write_data[1] = value;
 	
	 ret_code = mt_i2c_write(I2C_CH, sn65dsi8x_I2C_ADDR, write_data, 2,0); // 0:I2C_PATH_NORMAL
	 printf("sn65dsi8x_reg_i2c_write cmd=0x%x  data=0x%x ret_code=0x%x\n",addr,value,ret_code);
	 
	 return ret_code;
 }
#else
#define SN65DSI83_I2C_ID	I2C0
static struct mt_i2c_t sn65dsi83_i2c;

/**********************************************************
  *
  *   [I2C Function For Read/Write fan5405] 
  *
  *********************************************************/
kal_uint32 sn65dsi83_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    sn65dsi83_i2c.id = SN65DSI83_I2C_ID;
    /* Since i2c will left shift 1 bit, we need to set SN65DSI83 I2C address to >>1 */
    sn65dsi83_i2c.addr = (sn65dsi83_SLAVE_ADDR_WRITE >> 1);
    sn65dsi83_i2c.mode = ST_MODE;
    sn65dsi83_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&sn65dsi83_i2c, write_data, len);
    //dprintf(INFO, "%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

kal_uint32 sn65dsi83_read_byte (kal_uint8 addr, kal_uint8 *dataBuffer) 
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint16 len;
    *dataBuffer = addr;

    sn65dsi83_i2c.id = SN65DSI83_I2C_ID;
    /* Since i2c will left shift 1 bit, we need to set SN65DSI83 I2C address to >>1 */
    sn65dsi83_i2c.addr = (sn65dsi83_SLAVE_ADDR_WRITE >> 1);
    sn65dsi83_i2c.mode = ST_MODE;
    sn65dsi83_i2c.speed = 100;
    len = 1;

    ret_code = i2c_write_read(&sn65dsi83_i2c, dataBuffer, len, len);
    //dprintf(INFO, "%s: i2c_read: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

#endif
 //end
 
 /******************************************************************************
 *IIC drvier,:protocol type 2 add by chenguangjian end
 ******************************************************************************/
#else
extern int sn65dsi83_read_byte(kal_uint8 cmd, kal_uint8 *returnData);
extern int sn65dsi83_write_byte(kal_uint8 cmd, kal_uint8 Data);
#endif
//sn65dis83 chip init table
static/* struct*/ sn65dsi8x_setting_table sn65dis83_init_table[]=
{
//#if defined(LVDS_PANEL_8BITS_SUPPORT)  //200mhz 4lan 1024-768 normal mode
#if defined(LVDS_PANEL_8BITS_SUPPORT)
#if 1
{0x09,              0x00},
{0x0A ,             0x05},
{0x0B ,             0x10}, //0x18
{0x0D ,             0x00},
{0x10  ,            0x26},
{0x11  ,            0x00},
{0x12 ,             0x2c},//0x2b//0x2a//0x2c //0x35
{0x13  ,            0x00},
{0x18  ,            0x78},
{0x19  ,            0x00},
{0x1A ,             0x03},
{0x1B  ,            0x00},
{0x20  ,            0x20},
{0x21 ,             0x03},
{0x22 ,             0x00},
{0x23 ,             0x00},
{0x24,              0x00},
{0x25,              0x00},
{0x26 ,             0x00},
{0x27 ,             0x00},
{0x28,              0x21},//21
{0x29,              0x00},
{0x2A ,             0x00},
{0x2B,              0x00},
{0x2C ,             0x0a},
{0x2D ,             0x00},
{0x2E ,             0x00},
{0x2F  ,            0x00},
{0x30  ,            0x02},
{0x31  ,            0x00},
{0x32  ,            0x00},
{0x33  ,            0x00},
{0x34   ,           0x18},
{0x35  ,            0x00},
{0x36  ,            0x00},
{0x37  ,            0x00},
{0x38  ,            0x00},
{0x39  ,            0x00},
{0x3A  ,            0x00},
{0x3B  ,            0x00},
{0x3C  ,            0x00},
{0x3D   ,           0x00},
{0x3E   ,           0x00},
{0x0D , 		  0x01},
{REGFLAG_DELAY,10},//delay 5ms
{0x09 , 		  0x01},//soft reset
{0xFF,0x00},//ended flag

#else
{0x09,              0x00},
{0x0A ,             0x05},
{0x0B ,             0x10},
{0x0D ,             0x00},
{0x10  ,            0x26},
{0x11  ,            0x00},
{0x12 ,             0x28},
{0x13  ,            0x00},
{0x18  ,            0x78},//0x70
{0x19  ,            0x00},
{0x1A ,             0x03},
{0x1B  ,            0x00},
{0x20  ,            0x20},
{0x21 ,             0x03},
{0x22 ,             0x00},
{0x23 ,             0x00},
{0x24,              0x00},
{0x25,              0x00},
{0x26 ,             0x00},
{0x27 ,             0x00},
{0x28,              0x21},//22
{0x29,              0x00},
{0x2A ,             0x00},
{0x2B,              0x00},
{0x2C ,             0x0a},
{0x2D ,             0x00},
{0x2E ,             0x00},
{0x2F  ,            0x00},
{0x30  ,            0x02},
{0x31  ,            0x00},
{0x32  ,            0x00},
{0x33  ,            0x00},
{0x34   ,           0x18},
{0x35  ,            0x00},
{0x36  ,            0x00},
{0x37  ,            0x00},
{0x38  ,            0x00},
{0x39  ,            0x00},
{0x3A  ,            0x00},
{0x3B  ,            0x00},
{0x3C  ,            0x00},
{0x3D   ,           0x00},
{0x3E   ,           0x00},
{0x0D , 		  0x01},
{REGFLAG_DELAY,10},//delay 5ms
{0x09 , 		  0x01},//soft reset
{0xFF,0x00},//ended flag
#endif
#else
{0x09,              0x00},
{0x0A ,             0x05},
{0x0B ,             0x10},
{0x0D ,             0x00},
{0x10  ,            0x26},
{0x11  ,            0x00},
{0x12 ,             0x28},
{0x13  ,            0x00},
{0x18  ,            0x70},//0x72
{0x19  ,            0x00},
{0x1A ,             0x03},
{0x1B  ,            0x00},
{0x20  ,            0x20},
{0x21 ,             0x03},
{0x22 ,             0x00},
{0x23 ,             0x00},
{0x24,              0x00},
{0x25,              0x00},
{0x26 ,             0x00},
{0x27 ,             0x00},
{0x28,              0x22},//21
{0x29,              0x00},
{0x2A ,             0x00},
{0x2B,              0x00},
{0x2C ,             0x0a},
{0x2D ,             0x00},
{0x2E ,             0x00},
{0x2F  ,            0x00},
{0x30  ,            0x02},
{0x31  ,            0x00},
{0x32  ,            0x00},
{0x33  ,            0x00},
{0x34   ,           0x18},
{0x35  ,            0x00},
{0x36  ,            0x00},
{0x37  ,            0x00},
{0x38  ,            0x00},
{0x39  ,            0x00},
{0x3A  ,            0x00},
{0x3B  ,            0x00},
{0x3C  ,            0x00},
{0x3D   ,           0x00},
{0x3E   ,           0x00},
{0x0D , 		  0x01},
{REGFLAG_DELAY,10},//delay 5ms
{0x09 , 		  0x01},//soft reset
{0xFF,0x00},//ended flag
#endif 

};
static void push_table(/*struct fixed for build warning*/sn65dsi8x_setting_table *table, unsigned int count)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
        switch (cmd) {	
            case REGFLAG_DELAY :
            MDELAY(table[i].data);
                break;		
            case 0xFF:
                break;
				
            default:
		#ifdef BUILD_LK
		sn65dsi83_write_byte/*sn65dsi8x_reg_i2c_write*/(cmd, table[i].data);//TI_Sensor_Write(cmd, table[i].data);
		#else
		sn65dsi83_write_byte(cmd, table[i].data);
		#endif
       	}
    }
	
}
#ifdef SN65DSI_DEBUG   //fixed for build warning
static void dump_reg_table(/*struct*/ sn65dsi8x_setting_table *table, unsigned int count)
{
	unsigned int i;
	unsigned char data;
	
    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
        switch (cmd) {	
            case REGFLAG_DELAY :
            MDELAY(table[i].data);
                break;		
            case 0xFF:
                break;
				
            default:
		#ifdef BUILD_LK
		sn65dsi83_read_byte/*sn65dsi8x_reg_i2c_read*/(cmd,&data);	
		printf("dump cmd=0x%x  data=0x%x \n",cmd,data);
		#else
		sn65dsi83_read_byte(cmd,&data);
		printk("dump cmd=0x%x  data=0x%x \n",cmd,data);
		#endif
       	}
    }
	
}
#endif

void init_sn65dsi8x(void)
{
	#ifdef SN65DSI_DEBUG//add for debug
	unsigned char data;
	#endif
    	push_table(sn65dis83_init_table, sizeof(sn65dis83_init_table)/sizeof(/*struct*/ sn65dsi8x_setting_table));
	//MDELAY(5);
	//sn65dsi8x_reg_i2c_write(0x09,1);//soft reset
	//MDELAY(5);
	
	#ifdef SN65DSI_DEBUG//add for debug
	sn65dsi83_write_byte/*sn65dsi8x_reg_i2c_write*/(0xe0,1);//
	sn65dsi83_write_byte/*sn65dsi8x_reg_i2c_write*/(0xe1,0xff);//
	MDELAY(5);
	sn65dsi83_read_byte/*sn65dsi8x_reg_i2c_read*/(0xe5, &data);  
	printf("dump cmd=0xe5  data=0x%x \n",data);
	
	dump_reg_table(sn65dis83_init_table, sizeof(sn65dis83_init_table)/sizeof(/*struct*/ sn65dsi8x_setting_table)); //for debug
	#endif//debug end
}

/************************************************************************
*power fuction
*************************************************************************/
#ifdef BUILD_LK
extern	void upmu_set_rg_vgp2_vosel(U32 val);
extern  void upmu_set_rg_vgp6_vosel(U32 val);
extern	void upmu_set_rg_vgp2_en(U32 val);
extern	void upmu_set_rg_vgp6_en(U32 val);
void lvds_power_init(void)
{
   upmu_set_rg_vgp1_vosel(7);//7=3.3v
   upmu_set_rg_vgp1_en(1);//

   //upmu_set_rg_vgp6_vosel(7);//7=3.3v
   //upmu_set_rg_vgp6_en(1);//
}
#else //for kernel
extern bool hwPowerOn(MT65XX_POWER powerId, MT65XX_POWER_VOLTAGE powerVolt, char *mode_name);
void lvds_kernel_power_init(void)
{
	hwPowerOn(MT65XX_POWER_LDO_VGP1,VOL_3300,"LVDS");
	//hwPowerOn(MT65XX_POWER_LDO_VGP6,VOL_3300,"LVDS");
}
void lvds_kernel_power_deinit(void)
{
	hwPowerDown(MT65XX_POWER_LDO_VGP1,"LVDS");
	//hwPowerDown(MT65XX_POWER_LDO_VGP6,"LVDS");
}

#endif
//
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{	
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		params->dsi.mode   = SYNC_EVENT_VDO_MODE; //BURST_VDO_MODE;// BURST_VDO_MODE;
	
		// DSI
		/* Command mode setting */  
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		#if defined(LVDS_PANEL_8BITS_SUPPORT)
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888; //LCM_DSI_FORMAT_RGB888;
		#else
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666; //LCM_DSI_FORMAT_RGB888;
		#endif

	      	params->dsi.word_count=800*3;	

		params->dsi.vertical_sync_active= 2; //5;//10;
		params->dsi.vertical_backporch= 2; //17;//12;
		params->dsi.vertical_frontporch= 4; //16;//9;
		params->dsi.vertical_active_line= FRAME_HEIGHT;//hight

		params->dsi.horizontal_sync_active				=10; //110;//100;  //
		params->dsi.horizontal_backporch				= 24; //100;//80; //
		params->dsi.horizontal_frontporch				= 30; //110;//100; //
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;//=wight

		#if defined(LVDS_PANEL_8BITS_SUPPORT)
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888; //LCM_PACKED_PS_24BIT_RGB888;
		#else
		params->dsi.PS=LCM_PACKED_PS_18BIT_RGB666; //LCM_PACKED_PS_24BIT_RGB888;
		#endif
		
		params->dsi.pll_select=0;	//0: MIPI_PLL; 1: LVDS_PLL
		params->dsi.PLL_CLOCK = 224; //218; //216; //214; //220; //267; //201; //LCM_DSI_6589_PLL_CLOCK_201_5;//this value must be in MTK suggested table
		params->dsi.cont_clock = 1;
											//if not config this para, must config other 7 or 3 paras to gen. PLL
}


static void lcm_init(void)
{
#ifdef BUILD_LK

   printf("tM070ddh06--BUILD_LK--lcm_init \n");	
	//step1: sn65dsi8x enbable and init
	mt_set_gpio_mode(LVDS_MIPI2_VCC_EN, GPIO_MODE_00);
    mt_set_gpio_dir(LVDS_MIPI2_VCC_EN, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_MIPI2_VCC_EN, GPIO_OUT_ONE);
	MDELAY(20);
    mt_set_gpio_mode(LVDS_LCM_STBY, GPIO_MODE_00);
    mt_set_gpio_dir(LVDS_LCM_STBY, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ONE);
    MDELAY(5);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ZERO);
    MDELAY(20);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ONE);
    MDELAY(10);
	
	
    //mt_i2c_channel_init(I2C_CH);
    MDELAY(50);
	//step2 :set  dsi :continuous and HS mode  
	DSI_clk_HS_mode(1);
	//DSI_continuous_clock();   
	MDELAY(5);
   	init_sn65dsi8x();
	MDELAY(10);
//step 3 :lvds lcd init
    lvds_power_init();//set io =3.3v and VDD enable

    //mt_set_gpio_mode(LVDS_LCM_STBY, GPIO_MODE_00);    
    //mt_set_gpio_dir(LVDS_LCM_STBY, GPIO_DIR_OUT);
   // mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ONE); // LCM_STBY
    MDELAY(20);
	mt_set_gpio_mode(LVDS_LCM_VCC, GPIO_MODE_00);    
    mt_set_gpio_dir(LVDS_LCM_VCC, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_VCC, GPIO_OUT_ONE); // / LCM VCC :enable LCD VCC
	
    //mt_set_gpio_mode(LVDS_LCM_RESET, GPIO_MODE_00);    
    //mt_set_gpio_dir(LVDS_LCM_RESET, GPIO_DIR_OUT);
   // mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ONE); // LCM_RST -h
    MDELAY(50);
   // mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ZERO); // LCM_RST -L
    //MDELAY(3);
    //mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ONE); // LCM_RST -H
    
    mt_set_gpio_mode(LVDS_LCM_BIASON, GPIO_MODE_00);    
    mt_set_gpio_dir(LVDS_LCM_BIASON, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_BIASON, GPIO_OUT_ONE); // / LCMBIASON :enable VGH +VHL+ AVDD+ VCOM
    MDELAY(50);
	//mt_set_gpio_mode(LVDS_LCM_VCC, GPIO_MODE_00);    
    //mt_set_gpio_dir(LVDS_LCM_VCC, GPIO_DIR_OUT);
    //mt_set_gpio_out(LVDS_LCM_VCC, GPIO_OUT_ONE); // / LCM VCC :enable LCD VCC
    MDELAY(10);
	
#elif (defined BUILD_UBOOT)
	
#else
	   printk("tM070ddh06--kernel--lcm_init \n");	
	   DSI_clk_HS_mode(1);
	  // DSI_continuous_clock();	 

#endif    
}


static void lcm_suspend(void)
{
#ifndef BUILD_LK
	unsigned char temp;
 
	///step 1 power down lvds lcd
	mt_set_gpio_mode(LVDS_MIPI2_VCC_EN, GPIO_MODE_00);
    mt_set_gpio_dir(LVDS_MIPI2_VCC_EN, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_MIPI2_VCC_EN, GPIO_OUT_ZERO);
	MDELAY(10);
    mt_set_gpio_mode(LVDS_LCM_STBY, GPIO_MODE_00);    
    mt_set_gpio_dir(LVDS_LCM_STBY, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ZERO); // LCM_STBY     
    MDELAY(50);
    //mt_set_gpio_mode(LVDS_LCM_RESET, GPIO_MODE_00);    
   // mt_set_gpio_dir(LVDS_LCM_RESET, GPIO_DIR_IN);
   // mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ZERO); // LCM_RST
    
	 MDELAY(20);
	mt_set_gpio_mode(LVDS_LCM_BIASON, GPIO_MODE_00);	
	 mt_set_gpio_dir(LVDS_LCM_BIASON, GPIO_DIR_OUT);
	 mt_set_gpio_out(LVDS_LCM_BIASON, GPIO_OUT_ZERO); // LCMBIASON :VGH VHL

    MDELAY(30); // avoid LCD resume transint
    mt_set_gpio_mode(LVDS_LCM_VCC, GPIO_MODE_00);    
    mt_set_gpio_dir(LVDS_LCM_VCC, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_VCC, GPIO_OUT_ZERO); // / LCM VCC :enable LCD VCC
   //step 2 suspend sn65dsi8x
	sn65dsi83_read_byte(0x0a,&temp);//for test wether ti lock the pll clok
	printk("lcm_suspend  0x0a  value=0x%x \n",temp);

	sn65dsi83_read_byte(0x0d,&temp);
    	printk("lcm_suspend  0x0d  value=0x%x \n",temp);
       sn65dsi83_write_byte(0x0d, (temp&0xfe));//set bit0: 0
       //mt_set_gpio_out(GPIO133, GPIO_OUT_ZERO);
	   
      //step 3 set dsi LP mode
	DSI_clk_HS_mode(0);
	//DSI_no_continuous_clock();  
	//step 4:ldo power off
	lvds_kernel_power_deinit();
#else
printf("tM070ddh06--suspend \n");	

#endif	
    
}


static void lcm_resume(void)
{    
#ifndef BUILD_LK
      unsigned char temp;

	DSI_clk_HS_mode(1);
	//DSI_continuous_clock();   
	MDELAY(50);

	//step 1 resume sn65dsi8x
	mt_set_gpio_mode(LVDS_MIPI2_VCC_EN, GPIO_MODE_00);
    mt_set_gpio_dir(LVDS_MIPI2_VCC_EN, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_MIPI2_VCC_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_mode(LVDS_LCM_STBY, GPIO_MODE_00);
    mt_set_gpio_dir(LVDS_LCM_STBY, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ONE);
    MDELAY(5);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ZERO);
    MDELAY(20);
    mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ONE);
    MDELAY(10);

	init_sn65dsi8x();
	MDELAY(10);
	
	#if 1  //def SN65DSI_DEBUG
	sn65dsi83_read_byte(0x0a,&temp);
	printk("lcm_resume cmd-- 0x0a=0x%x \n",temp);
	sn65dsi83_read_byte(0x0d,&temp);
	printk("lcm_resume cmd-- 0x0d=0x%x \n",temp);
	sn65dsi83_read_byte(0x09,&temp);
	printk("lcm_resume cmd-- 0x09=0x%x \n",temp);
	#endif
	//step 2 resume lvds
   lvds_kernel_power_init();//set io=3.3v
	
    //mt_set_gpio_mode(LVDS_LCM_STBY, GPIO_MODE_00);    
    //mt_set_gpio_dir(LVDS_LCM_STBY, GPIO_DIR_OUT);
    //mt_set_gpio_out(LVDS_LCM_STBY, GPIO_OUT_ONE); // LCM_STBY
    MDELAY(5);
    //mt_set_gpio_mode(LVDS_LCM_RESET, GPIO_MODE_00);    
    //mt_set_gpio_dir(LVDS_LCM_RESET, GPIO_DIR_OUT);
    //mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ONE); // LCM_RST -h
    //MDELAY(40);
   // mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ZERO); // LCM_RST -L
   // MDELAY(3);
   // mt_set_gpio_out(LVDS_LCM_RESET, GPIO_OUT_ONE); // LCM_RST -H
	mt_set_gpio_mode(LVDS_LCM_VCC, GPIO_MODE_00);    
    mt_set_gpio_dir(LVDS_LCM_VCC, GPIO_DIR_OUT);
    mt_set_gpio_out(LVDS_LCM_VCC, GPIO_OUT_ONE); // / LCM VCC :enable LCD VCC
	 MDELAY(50); 
	
 	mt_set_gpio_mode(LVDS_LCM_BIASON, GPIO_MODE_00);	
 	mt_set_gpio_dir(LVDS_LCM_BIASON, GPIO_DIR_OUT);
 	mt_set_gpio_out(LVDS_LCM_BIASON, GPIO_OUT_ONE); // LCMBIASON :VGH VHL
 
    MDELAY(50); 
	
    MDELAY(20);
	printk("tM070ddh06--lcm_resume end \n");
#else
printf("tM070ddh06--suspend \n");
#endif
}
static unsigned int lcm_compare_id(void)
{
#if defined(BUILD_LK)
		printf("TM070DDH06_MIPI2LVDS  lcm_compare_id \n");
#endif

    return 1;
}

LCM_DRIVER cpt_clap070wp03xg_sn65dsi83_lcm_drv = 
{
    	.name		    = "cpt_clap070wp03xg_sn65dsi83",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

