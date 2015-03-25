#include "stdlib.h"
#include "fcntl.h"
#include "errno.h"

#define LOG_TAG "wlanLoader"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

//#ifdef WIFI_DRIVER_MODULE_PATH
//static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
//#else
static char DRIVER_MODULE_PATH[50]  = "/system/lib/modules/wlan_";
static char DRIVER_MODULE_ARG[50] = "";
//#endif
//
#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG          ""
#endif


static const char CHIP_PROP[] = "persist.mtk.wcn.combo.chipid";
static char chip_version[PROPERTY_VALUE_MAX];


#if 0
#if (SUPPORT_MT6620 == 0)&& (SUPPORT_MT6628 == 0)
	#warning "Should Support One Combo Chip"
#endif
#endif


/*
The flow  
get the chip version ->  check the support list -> load the driver -> exit
*/

//check the support list
int checkSupport(char *chipName)
{
  int ret = -1;

  ALOGD("checkSupport %s\n", chipName);	
  if(NULL != chipName){
#ifdef SUPPORT_MT6620
	if(0 == strncmp(chipName, "0x6620", strlen("0x6620"))){
	        ALOGD("Match 0x6620\n");
        	ret = 0;
	}else{
		ALOGD("Unmatch 0x6620\n");
	}
#endif

#ifdef SUPPORT_MT6628
	if(0 == strncmp(chipName, "0x6628", strlen("0x6620"))){
	        ALOGD("Match 0x6628\n");
		ret = 0;
       }else{
                ALOGD("Unmatch 0x6628\n");
 	}
#endif
  
  }  	

	return ret;
}

char* getChipVersion()
{
  int retry = 10;
  char *ret = NULL;
  
  while(retry-- > 0){
  	
  	if(property_get(CHIP_PROP, chip_version, NULL)){
				chip_version[PROPERTY_VALUE_MAX - 1] = '\0';
				
  			if(0 == checkSupport(chip_version)){
  				chip_version[0] = 'm';
  				chip_version[1] = 't';
  				ret = chip_version;
  				ALOGD("Get the correct chip version %s\n", chip_version);
  				break;
  			}
  					
  	}
   	usleep(500000);		
  }
  
  if(NULL == chip_version)
  	ALOGD("Can't get the correct chip version %s.\n", chip_version);
  else
	ALOGD("Get chip Version %s.\n", chip_version);	
	
  return ret;
}


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



int main(int argc, char *argv[])
{
	char* chipVersion = NULL;
	int ret = -1;
	
  //Get the chipVersion
  chipVersion = getChipVersion();
  
#if 1
  if(NULL != chipVersion){
		strcat(DRIVER_MODULE_PATH, chipVersion);
		strcat(DRIVER_MODULE_PATH, ".ko");
	
	        ALOGD("DRIVER_MODULE_PATH is %s\n", DRIVER_MODULE_PATH);   
		if(0 == insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG)){  
  			ret = 0;
  			ALOGD("Success to insmod the %s\n", DRIVER_MODULE_PATH);
  		}else
  			ALOGD("Fail to insmod the %s\n", DRIVER_MODULE_PATH);	  
  		
  }else{
  	ALOGD("Fail to get the corroct combo chip version");
  }
#else

		if(0 == insmod("/system/lib/modules/wlan_mt.ko", DRIVER_MODULE_ARG)){  
  			ret = 0;
  			ALOGD("Success to insmod the %s\n", DRIVER_MODULE_PATH);
  		}else
  			ALOGD("Fail to insmod the %s\n", DRIVER_MODULE_PATH);	  
#endif

	return ret;
}



