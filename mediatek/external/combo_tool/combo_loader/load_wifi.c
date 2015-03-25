/*
*WIFI driver kernel module insmod file for wmt dynamic loader
*/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

//For directory operation
#include <dirent.h>

extern int load_wifi_module(int chip_id);
extern int init_module(void *, unsigned long, const char *);

static char DRIVER_MODULE_PATH[50]  = "/system/lib/modules/wlan_";
static char DRIVER_MODULE_ARG[50] = "";
#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG          ""
#endif

//insmod
static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        ALOGD("Unable to unload driver module \"%s\": %s\n",
             modname, strerror(errno));
    return ret;
}

int load_wifi_module(int chip_id)
{
    int ret=-1;
    
    if(chip_id == 0x6630){
        //insert 6630 driver
		if(0 == insmod("/system/lib/modules/mtk_wmt_wifi.ko", DRIVER_MODULE_ARG)){  
            ret = 0;
            printf("Success to insmod wmt wifi module\n");
        }else
            printf("Fail to insmod wmt wifi module\n");	

        if(0 == insmod("/system/lib/modules/wlan_mt6630.ko", DRIVER_MODULE_ARG)){  
            ret = 0;
            printf("Success to insmod wlan module\n");
        }else
            printf("Fail to insmod wlan module\n");	 
    }
    else if(chip_id == 0x6628){
        //insert 6628 driver
        if(0 == insmod("/system/lib/modules/mtk_wmt_wifi.ko", DRIVER_MODULE_ARG)){  
            ret = 0;
            printf("Success to insmod wmt wifi module\n");
        }else
            printf("Fail to insmod wmt wifi module\n");	

        if(0 == insmod("/system/lib/modules/wlan_mt6628.ko", DRIVER_MODULE_ARG)){  
            ret = 0;
            printf("Success to insmod wlan module\n");
        }else
            printf("Fail to insmod wlan module\n");	 
    }
    else if(chip_id == 0x6620){
        //insert 6620 driver
    }else { //for soc chip, same naming
        //insert wmt_wifi => for temp naming
        if(0 == insmod("/system/lib/modules/mtk_wmt_wifi_soc.ko", DRIVER_MODULE_ARG)){  
            ret = 0;
            printf("Success to insmod wmt wifi module\n");
        }else
            printf("Fail to insmod wmt wifi module\n");	    

        //insert wifi => for temp naming
        if(0 == insmod("/system/lib/modules/wlan_mt.ko", DRIVER_MODULE_ARG)){  
            ret = 0;
            printf("Success to insmod the %s\n", DRIVER_MODULE_PATH);
        }else
            printf("Fail to insmod the %s\n", DRIVER_MODULE_PATH);	    
    }
    return ret;
}
