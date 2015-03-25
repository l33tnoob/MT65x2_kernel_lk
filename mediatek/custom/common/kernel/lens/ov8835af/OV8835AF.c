/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "OV8835AF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("OV8835AF", 0x1c)}; // LINE <><20130905><AfMotorCompatible> Jiangde 0x18-->0x1c


#define OV8835AF_DRVNAME "OV8835AF"
#define OV8835AF_VCM_WRITE_ID           0x18

#define OV8835AF_DEBUG
#ifdef OV8835AF_DEBUG
#define OV8835AFDB printk
#else
#define OV8835AFDB(x,...)
#endif

static spinlock_t g_OV8835AF_SpinLock;

static struct i2c_client * g_pstOV8835AF_I2Cclient = NULL;

static dev_t g_OV8835AF_devno;
static struct cdev * g_pOV8835AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4OV8835AF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4OV8835AF_INF = 0;
static unsigned long g_u4OV8835AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static int g_sr = 3;

#if 0
extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);


static int s4OV8835AF_ReadReg(unsigned short * a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstOV8835AF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C read failed!! \n");
        return -1;
    }

    *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

    return 0;
}

static int s4OV8835AF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    // char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+g_sr)};     // Jiangde, IMX135 SUNNY
    // char puSendCmd[2] = {(char)(((a_u2Data >> 8) & 0x03) | 0xc4), (char)(a_u2Data & 0xff)}; // Jiangde, IMX135 TRULY
    char puSendCmd[2] = {(char)(((a_u2Data >> 8) & 0x03) | 0xc0), (char)(a_u2Data & 0xff)}; // Jiangde OV8835AF

    //OV8835AFDB("[OV8835AF] g_sr %d, write %d \n", g_sr, a_u2Data);
    g_pstOV8835AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstOV8835AF_I2Cclient, puSendCmd, 2);
	
    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}
#else
static int s4OV8835AF_ReadReg(unsigned short * a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstOV8835AF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C read failed!! \n");
        return -1;
    }

		*a_pu2Result = (((u16)(pBuff[0] & 0x03)) << 8) + pBuff[1];

    return 0;
}

static int s4OV8835AF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(((a_u2Data >> 8)&0x03)|0xc4) , (char)(a_u2Data & 0x00FF)};

   OV8835AFDB("[OV8835AF] g_sr %d, write %d ,p1=0x%x,p2=0x%x\n", g_sr, a_u2Data,((a_u2Data >> 8)&0x03)|0xc4,(a_u2Data & 0x00FF));
    g_pstOV8835AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstOV8835AF_I2Cclient, puSendCmd, 2);
	
    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}
#endif


inline static int getOV8835AFInfo(__user stOV8835AF_MotorInfo * pstMotorInfo)
{
    stOV8835AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4OV8835AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4OV8835AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

	if (g_s4OV8835AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stOV8835AF_MotorInfo)))
    {
        OV8835AFDB("[OV8835AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveOV8835AF(unsigned long a_u4Position)
{
    int ret = 0;
    OV8835AFDB("%s,%d,a_u4Position=%d\n",__func__,__LINE__,a_u4Position);
    if((a_u4Position > g_u4OV8835AF_MACRO) || (a_u4Position < g_u4OV8835AF_INF))
    {
        OV8835AFDB("[OV8835AF] out of range \n");
        return -EINVAL;
    }

    if (g_s4OV8835AF_Opened == 1)
    {
        unsigned short InitPos;
        ret = s4OV8835AF_ReadReg(&InitPos);
	    
        spin_lock(&g_OV8835AF_SpinLock);
        if(ret == 0)
        {
            OV8835AFDB("[OV8835AF] Init Pos %6d \n", InitPos);
            g_u4CurrPosition = (unsigned long)InitPos;
        }
        else
        {		
            g_u4CurrPosition = 0;
        }
        g_s4OV8835AF_Opened = 2;
        spin_unlock(&g_OV8835AF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_OV8835AF_SpinLock);	
        g_i4Dir = 1;
        spin_unlock(&g_OV8835AF_SpinLock);	
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_OV8835AF_SpinLock);	
        g_i4Dir = -1;
        spin_unlock(&g_OV8835AF_SpinLock);			
    }
    else										{return 0;}

    spin_lock(&g_OV8835AF_SpinLock);    
    g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_OV8835AF_SpinLock);	

    //OV8835AFDB("[OV8835AF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

            spin_lock(&g_OV8835AF_SpinLock);
            g_sr = 3;
            g_i4MotorStatus = 0;
            spin_unlock(&g_OV8835AF_SpinLock);	
		
            if(s4OV8835AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
            {
                spin_lock(&g_OV8835AF_SpinLock);		
                g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
                spin_unlock(&g_OV8835AF_SpinLock);				
            }
            else
            {
                OV8835AFDB("[OV8835AF] set I2C failed when moving the motor \n");			
                spin_lock(&g_OV8835AF_SpinLock);
                g_i4MotorStatus = -1;
                spin_unlock(&g_OV8835AF_SpinLock);				
            }

    return 0;
}

inline static int setOV8835AFInf(unsigned long a_u4Position)
{
    spin_lock(&g_OV8835AF_SpinLock);
    g_u4OV8835AF_INF = a_u4Position;
    spin_unlock(&g_OV8835AF_SpinLock);	
    return 0;
}

inline static int setOV8835AFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_OV8835AF_SpinLock);
    g_u4OV8835AF_MACRO = a_u4Position;
    spin_unlock(&g_OV8835AF_SpinLock);	
    return 0;	
}

////////////////////////////////////////////////////////////////
static long OV8835AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case OV8835AFIOC_G_MOTORINFO :
            i4RetValue = getOV8835AFInfo((__user stOV8835AF_MotorInfo *)(a_u4Param));
        break;

        case OV8835AFIOC_T_MOVETO :
            i4RetValue = moveOV8835AF(a_u4Param);
        break;
 
        case OV8835AFIOC_T_SETINFPOS :
            i4RetValue = setOV8835AFInf(a_u4Param);
        break;

        case OV8835AFIOC_T_SETMACROPOS :
            i4RetValue = setOV8835AFMacro(a_u4Param);
        break;
		
        default :
      	    OV8835AFDB("[OV8835AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int OV8835AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    OV8835AFDB("[OV8835AF] OV8835AF_Open - Start\n");

    char puSendCmd[2];
    int  i4RetValue = 0;
    spin_lock(&g_OV8835AF_SpinLock);

    if(g_s4OV8835AF_Opened)
    {
        spin_unlock(&g_OV8835AF_SpinLock);
        OV8835AFDB("[OV8835AF] the device is opened \n");
        return -EBUSY;
    }

    // FM50 SRC Initial Setting -- Add by truly 2013.07.05
    OV8835AFDB("[OV8835AF] FM50 SRC Init Start!! \n");
    
    // Tvib=13.5ms
    // rf[4:0]=00111[80Hz], slew_rate[1:0]=11
    puSendCmd[0] = 0xCC;
    puSendCmd[1] = 0x3B;
    i4RetValue = i2c_master_send(g_pstOV8835AF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C send failed!! \n");
    }
    
    // uncont1[9:0]=80
    puSendCmd[0] = 0xD4;
    puSendCmd[1] = 0x50;
    i4RetValue = i2c_master_send(g_pstOV8835AF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C send failed!! \n");
    }
    
    // uncont2[9:0]=180
    puSendCmd[0] = 0xDC;
    puSendCmd[1] = 0xB4;
    i4RetValue = i2c_master_send(g_pstOV8835AF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C send failed!! \n");
    }
    
    // stt[4:0]=00001[50us], str[2:0]=010[2LSB]
    puSendCmd[0] = 0xE4;
    puSendCmd[1] = 0x41;
    i4RetValue = i2c_master_send(g_pstOV8835AF_I2Cclient, puSendCmd, 2);
    if (i4RetValue < 0) 
    {
        OV8835AFDB("[OV8835AF] I2C send failed!! \n");
    }
    
    OV8835AFDB("[OV8835AF] FM50 SRC Init End!! \n");
    g_s4OV8835AF_Opened = 1;
		
    spin_unlock(&g_OV8835AF_SpinLock);

    OV8835AFDB("[OV8835AF] OV8835AF_Open - End\n");

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int OV8835AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    OV8835AFDB("[OV8835AF] OV8835AF_Release - Start\n");

    if (g_s4OV8835AF_Opened)
    {
        OV8835AFDB("[OV8835AF] feee \n");
        g_sr = 5;
	    s4OV8835AF_WriteReg(200);
        msleep(10);
	    s4OV8835AF_WriteReg(100);
        msleep(10);
            	            	    	    
        spin_lock(&g_OV8835AF_SpinLock);
        g_s4OV8835AF_Opened = 0;
        spin_unlock(&g_OV8835AF_SpinLock);

    }

    OV8835AFDB("[OV8835AF] OV8835AF_Release - End\n");

    return 0;
}

static const struct file_operations g_stOV8835AF_fops = 
{
    .owner = THIS_MODULE,
    .open = OV8835AF_Open,
    .release = OV8835AF_Release,
    .unlocked_ioctl = OV8835AF_Ioctl
};

inline static int Register_OV8835AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    OV8835AFDB("[OV8835AF] Register_OV8835AF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_OV8835AF_devno, 0, 1,OV8835AF_DRVNAME) )
    {
        OV8835AFDB("[OV8835AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pOV8835AF_CharDrv = cdev_alloc();

    if(NULL == g_pOV8835AF_CharDrv)
    {
        unregister_chrdev_region(g_OV8835AF_devno, 1);

        OV8835AFDB("[OV8835AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pOV8835AF_CharDrv, &g_stOV8835AF_fops);

    g_pOV8835AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pOV8835AF_CharDrv, g_OV8835AF_devno, 1))
    {
        OV8835AFDB("[OV8835AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_OV8835AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv_ov8835af"); // LINE <><20130905><AfMotorCompatible> Jiangde
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        OV8835AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_OV8835AF_devno, NULL, OV8835AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    OV8835AFDB("[OV8835AF] Register_OV8835AF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_OV8835AF_CharDrv(void)
{
    OV8835AFDB("[OV8835AF] Unregister_OV8835AF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pOV8835AF_CharDrv);

    unregister_chrdev_region(g_OV8835AF_devno, 1);
    
    device_destroy(actuator_class, g_OV8835AF_devno);

    class_destroy(actuator_class);

    OV8835AFDB("[OV8835AF] Unregister_OV8835AF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////

static int OV8835AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int OV8835AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id OV8835AF_i2c_id[] = {{OV8835AF_DRVNAME,0},{}};   
struct i2c_driver OV8835AF_i2c_driver = {                       
    .probe = OV8835AF_i2c_probe,                                   
    .remove = OV8835AF_i2c_remove,                           
    .driver.name = OV8835AF_DRVNAME,                 
    .id_table = OV8835AF_i2c_id,                             
};  

#if 0 
static int OV8835AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, OV8835AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int OV8835AF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int OV8835AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    printk("[OV8835AF] %s,%d\n",__func__,__LINE__);

    /* Kirby: add new-style driver { */
    g_pstOV8835AF_I2Cclient = client;

    g_pstOV8835AF_I2Cclient->addr = OV8835AF_VCM_WRITE_ID; // LINE <><20130905><AfMotorCompatible> Jiangde, change to real one    
    g_pstOV8835AF_I2Cclient->addr = g_pstOV8835AF_I2Cclient->addr >> 1;
    
    //Register char driver
    i4RetValue = Register_OV8835AF_CharDrv();

    if(i4RetValue){

        OV8835AFDB("[OV8835AF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_OV8835AF_SpinLock);

    OV8835AFDB("[OV8835AF] Attached!! \n");

    return 0;
}

static int OV8835AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&OV8835AF_i2c_driver);
}

static int OV8835AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&OV8835AF_i2c_driver);
    return 0;
}

static int OV8835AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int OV8835AF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stOV8835AF_Driver = {
    .probe		= OV8835AF_probe,
    .remove	= OV8835AF_remove,
    .suspend	= OV8835AF_suspend,
    .resume	= OV8835AF_resume,
    .driver		= {
        .name	= "lens_actuator_ov8835af", // LINE <><20130905><AfMotorCompatible> Jiangde
        .owner	= THIS_MODULE,
    }
};


// BEGIN <><20130905><AfMotorCompatible> Jiangde
static struct platform_device actuator_truly_treater = {
	.name   = "lens_actuator_ov8835af",
	.id     = -1,
};
// END <><20130905><AfMotorCompatible> Jiangde


static int __init OV8835AF_i2C_init(void)
{
    printk("########%s,%d\n",__func__,__LINE__);
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
    platform_device_register(&actuator_truly_treater);// LINE <><20130905><AfMotorCompatible> Jiangde
	
    if(platform_driver_register(&g_stOV8835AF_Driver)){
        OV8835AFDB("failed to register OV8835AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit OV8835AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stOV8835AF_Driver);
}

module_init(OV8835AF_i2C_init);
module_exit(OV8835AF_i2C_exit);

MODULE_DESCRIPTION("OV8835AF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");


