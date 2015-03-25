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
#include "LITEON3H7AF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 1 //LINE <> <DATE20140118> <S5500:M-Sensor can not work.> wupingzhou
//static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("LITEON3H7AF", 0x18)};
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("LITEON3H7AF", 0x17)}; // Jiangde, 0x19-->0x17


#define LITEON3H7AF_DRVNAME         "LITEON3H7AF"
#define LITEON3H7AF_VCM_WRITE_ID    0x18           // Jiangde

#define LITEON3H7AF_DEBUG
#ifdef LITEON3H7AF_DEBUG
#define LITEON3H7AFDB printk
#else
#define LITEON3H7AFDB(x,...)
#endif

#define AF_DLC_MODE
static spinlock_t g_LITEON3H7AF_SpinLock;

static struct i2c_client * g_pstLITEON3H7AF_I2Cclient = NULL;

static dev_t g_LITEON3H7AF_devno;
static struct cdev * g_pLITEON3H7AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4LITEON3H7AF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4LITEON3H7AF_INF = 0;
static unsigned long g_u4LITEON3H7AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static int g_sr = 5;
/*
extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);
*/

static int s4LITEON3H7AF_ReadReg(unsigned short * a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstLITEON3H7AF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0) 
    {
        LITEON3H7AFDB("[LITEON3H7AF] I2C read failed!! \n");
        return -1;
    }

    *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

    return 0;
}

static int s4LITEON3H7AF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+g_sr)};

    LITEON3H7AFDB("[LITEON3H7AF] g_sr %d, write %d \n", g_sr, a_u2Data);
    g_pstLITEON3H7AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSendCmd, 2);
	
    if (i4RetValue < 0) 
    {
        LITEON3H7AFDB("[LITEON3H7AF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}

inline static int getLITEON3H7AFInfo(__user stLITEON3H7AF_MotorInfo * pstMotorInfo)
{
    stLITEON3H7AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4LITEON3H7AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4LITEON3H7AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

	if (g_s4LITEON3H7AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stLITEON3H7AF_MotorInfo)))
    {
        LITEON3H7AFDB("[LITEON3H7AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveLITEON3H7AF(unsigned long a_u4Position)
{
    int ret = 0;
    
    if((a_u4Position > g_u4LITEON3H7AF_MACRO) || (a_u4Position < g_u4LITEON3H7AF_INF))
    {
        LITEON3H7AFDB("[LITEON3H7AF] out of range \n");
        return -EINVAL;
    }

    if (g_s4LITEON3H7AF_Opened == 1)
    {
        unsigned short InitPos;
        ret = s4LITEON3H7AF_ReadReg(&InitPos);
	    
        spin_lock(&g_LITEON3H7AF_SpinLock);
        if(ret == 0)
        {
            LITEON3H7AFDB("[LITEON3H7AF] Init Pos %6d \n", InitPos);
            g_u4CurrPosition = (unsigned long)InitPos;
        }
        else
        {		
            g_u4CurrPosition = 0;
        }
        g_s4LITEON3H7AF_Opened = 2;
        spin_unlock(&g_LITEON3H7AF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_LITEON3H7AF_SpinLock);	
        g_i4Dir = 1;
        spin_unlock(&g_LITEON3H7AF_SpinLock);	
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_LITEON3H7AF_SpinLock);	
        g_i4Dir = -1;
        spin_unlock(&g_LITEON3H7AF_SpinLock);			
    }
    else										{return 0;}

    spin_lock(&g_LITEON3H7AF_SpinLock);    
    g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_LITEON3H7AF_SpinLock);	

  LITEON3H7AFDB("[LITEON3H7AF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

            spin_lock(&g_LITEON3H7AF_SpinLock);
             g_sr = 3;
            g_i4MotorStatus = 0;
            spin_unlock(&g_LITEON3H7AF_SpinLock);	
		
            if(s4LITEON3H7AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
            {
                spin_lock(&g_LITEON3H7AF_SpinLock);		
                g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
                spin_unlock(&g_LITEON3H7AF_SpinLock);				
            }
            else
            {
                LITEON3H7AFDB("[LITEON3H7AF] set I2C failed when moving the motor \n");			
                spin_lock(&g_LITEON3H7AF_SpinLock);
                g_i4MotorStatus = -1;
                spin_unlock(&g_LITEON3H7AF_SpinLock);				
            }

    return 0;
}

inline static int setLITEON3H7AFInf(unsigned long a_u4Position)
{
    spin_lock(&g_LITEON3H7AF_SpinLock);
    g_u4LITEON3H7AF_INF = a_u4Position;
    spin_unlock(&g_LITEON3H7AF_SpinLock);	
    return 0;
}

inline static int setLITEON3H7AFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_LITEON3H7AF_SpinLock);
    g_u4LITEON3H7AF_MACRO = a_u4Position;
    spin_unlock(&g_LITEON3H7AF_SpinLock);	
    return 0;	
}

////////////////////////////////////////////////////////////////
static long LITEON3H7AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case LITEON3H7AFIOC_G_MOTORINFO :
            i4RetValue = getLITEON3H7AFInfo((__user stLITEON3H7AF_MotorInfo *)(a_u4Param));
        break;

        case LITEON3H7AFIOC_T_MOVETO :
            i4RetValue = moveLITEON3H7AF(a_u4Param);
        break;
 
        case LITEON3H7AFIOC_T_SETINFPOS :
            i4RetValue = setLITEON3H7AFInf(a_u4Param);
        break;

        case LITEON3H7AFIOC_T_SETMACROPOS :
            i4RetValue = setLITEON3H7AFMacro(a_u4Param);
        break;
		
        default :
      	    LITEON3H7AFDB("[LITEON3H7AF] No CMD \n");
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
static int LITEON3H7AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    LITEON3H7AFDB("[LITEON3H7AF] LITEON3H7AF_Open - Start\n");
    long i4RetValue = 0;
    spin_lock(&g_LITEON3H7AF_SpinLock);

    if(g_s4LITEON3H7AF_Opened)
    {
        spin_unlock(&g_LITEON3H7AF_SpinLock);
        LITEON3H7AFDB("[LITEON3H7AF] the device is opened \n");
        return -EBUSY;
    }
    g_s4LITEON3H7AF_Opened = 1;
		
    spin_unlock(&g_LITEON3H7AF_SpinLock);

    #ifdef AF_DLC_MODE
    /*char puSuspendCmd[2] = {(char)(0xEC), (char)(0xA3)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd, 2);

    char puSuspendCmd2[2] = {(char)(0xA1), (char)(0x05)};//0x0D
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd2, 2);

    char puSuspendCmd3[2] = {(char)(0xF2), (char)(0xF8)};//0xE8
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd3, 2);

    char puSuspendCmd4[2] = {(char)(0xDC), (char)(0x51)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd4, 2);*/

    char puSuspendCmd[2] = {(char)(0xEC), (char)(0xA3)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd, 2);

    char puSuspendCmd1[2] = {(char)(0xA1), (char)(0x0D)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd1, 2);

    char puSuspendCmd2[2] = {(char)(0xF2), (char)(0xE8)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd2, 2);

    char puSuspendCmd3[2] = {(char)(0xDC), (char)(0x51)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd3, 2);

    #else //AF_LSC_MODE

    char puSuspendCmd[2] = {(char)(0xEC), (char)(0xA3)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd, 2);

    char puSuspendCmd1[2] = {(char)(0xF2), (char)(0x80)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd1, 2);

    char puSuspendCmd2[2] = {(char)(0xDC), (char)(0x51)};
    i4RetValue = i2c_master_send(g_pstLITEON3H7AF_I2Cclient, puSuspendCmd2, 2);
 
    #endif


    LITEON3H7AFDB("[LITEON3H7AF] LITEON3H7AF_Open - End\n");

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int LITEON3H7AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    LITEON3H7AFDB("[LITEON3H7AF] LITEON3H7AF_Release - Start\n");

    if (g_s4LITEON3H7AF_Opened)
    {
        LITEON3H7AFDB("[LITEON3H7AF] feee \n");
        g_sr = 5;
	    s4LITEON3H7AF_WriteReg(200);
        msleep(10);
	    s4LITEON3H7AF_WriteReg(100);
        msleep(10);
            	            	    	    
        spin_lock(&g_LITEON3H7AF_SpinLock);
        g_s4LITEON3H7AF_Opened = 0;
        spin_unlock(&g_LITEON3H7AF_SpinLock);

    }

    LITEON3H7AFDB("[LITEON3H7AF] LITEON3H7AF_Release - End\n");

    return 0;
}

static const struct file_operations g_stLITEON3H7AF_fops = 
{
    .owner = THIS_MODULE,
    .open = LITEON3H7AF_Open,
    .release = LITEON3H7AF_Release,
    .unlocked_ioctl = LITEON3H7AF_Ioctl
};

inline static int Register_LITEON3H7AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    LITEON3H7AFDB("[LITEON3H7AF] Register_LITEON3H7AF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_LITEON3H7AF_devno, 0, 1,LITEON3H7AF_DRVNAME) )
    {
        LITEON3H7AFDB("[LITEON3H7AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pLITEON3H7AF_CharDrv = cdev_alloc();

    if(NULL == g_pLITEON3H7AF_CharDrv)
    {
        unregister_chrdev_region(g_LITEON3H7AF_devno, 1);

        LITEON3H7AFDB("[LITEON3H7AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pLITEON3H7AF_CharDrv, &g_stLITEON3H7AF_fops);

    g_pLITEON3H7AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pLITEON3H7AF_CharDrv, g_LITEON3H7AF_devno, 1))
    {
        LITEON3H7AFDB("[LITEON3H7AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_LITEON3H7AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv_liteon3h7"); // Jiangde
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        LITEON3H7AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_LITEON3H7AF_devno, NULL, LITEON3H7AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    LITEON3H7AFDB("[LITEON3H7AF] Register_LITEON3H7AF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_LITEON3H7AF_CharDrv(void)
{
    LITEON3H7AFDB("[LITEON3H7AF] Unregister_LITEON3H7AF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pLITEON3H7AF_CharDrv);

    unregister_chrdev_region(g_LITEON3H7AF_devno, 1);
    
    device_destroy(actuator_class, g_LITEON3H7AF_devno);

    class_destroy(actuator_class);

    LITEON3H7AFDB("[LITEON3H7AF] Unregister_LITEON3H7AF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////

static int LITEON3H7AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int LITEON3H7AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id LITEON3H7AF_i2c_id[] = {{LITEON3H7AF_DRVNAME,0},{}};   
struct i2c_driver LITEON3H7AF_i2c_driver = {                       
    .probe = LITEON3H7AF_i2c_probe,                                   
    .remove = LITEON3H7AF_i2c_remove,                           
    .driver.name = LITEON3H7AF_DRVNAME,                 
    .id_table = LITEON3H7AF_i2c_id,                             
};  

#if 0 
static int LITEON3H7AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, LITEON3H7AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int LITEON3H7AF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int LITEON3H7AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    LITEON3H7AFDB("[LITEON3H7AF] LITEON3H7AF_i2c_probe\n");

    /* Kirby: add new-style driver { */
    g_pstLITEON3H7AF_I2Cclient = client;
    
    //g_pstLITEON3H7AF_I2Cclient->addr = g_pstLITEON3H7AF_I2Cclient->addr >> 1;
    g_pstLITEON3H7AF_I2Cclient->addr = LITEON3H7AF_VCM_WRITE_ID >> 1; // Jiangde
    //Register char driver
    i4RetValue = Register_LITEON3H7AF_CharDrv();

    if(i4RetValue){

        LITEON3H7AFDB("[LITEON3H7AF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_LITEON3H7AF_SpinLock);

    LITEON3H7AFDB("[LITEON3H7AF] Attached!! \n");

    return 0;
}

static int LITEON3H7AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&LITEON3H7AF_i2c_driver);
}

static int LITEON3H7AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&LITEON3H7AF_i2c_driver);
    return 0;
}

static int LITEON3H7AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int LITEON3H7AF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stLITEON3H7AF_Driver = {
    .probe		= LITEON3H7AF_probe,
    .remove	= LITEON3H7AF_remove,
    .suspend	= LITEON3H7AF_suspend,
    .resume	= LITEON3H7AF_resume,
    .driver		= {
        .name	= "lens_actuator_liteon3h7", // Jiangde
        .owner	= THIS_MODULE,
    }
};

static struct platform_device actuator_dev3 = {
	.name		  = "lens_actuator_liteon3h7", // Jiangde
	.id		  = -1,
};

static int __init LITEON3H7AF_i2C_init(void)
{
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
    platform_device_register(&actuator_dev3); // Jiangde
	
    if(platform_driver_register(&g_stLITEON3H7AF_Driver)){
        LITEON3H7AFDB("failed to register LITEON3H7AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit LITEON3H7AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stLITEON3H7AF_Driver);
}

module_init(LITEON3H7AF_i2C_init);
module_exit(LITEON3H7AF_i2C_exit);

MODULE_DESCRIPTION("LITEON3H7AF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");


