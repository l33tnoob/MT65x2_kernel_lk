#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/mtgpio.h>
#include <linux/sensors_io.h>

#include "meta_alsps.h"
#include "WM2Linux.h"

//#include "libhwm.h"
/*---------------------------------------------------------------------------*/
#define ALSPS_PREFIX   "ALSPS: "
#define ALSPS_NAME	"/dev/als_ps"

/*---------------------------------------------------------------------------*/
#define MALSPSLOGD(fmt, arg ...) META_LOG(ALSPS_PREFIX fmt, ##arg)
//#define MALSPSLOGD(fmt, arg...) printf(ALSPS_PREFIX fmt, ##arg)

/*---------------------------------------------------------------------------*/
static int fd = -1;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
BOOL Meta_ALSPS_Open(void)
{
    int retry =0;
	int max_retry = 3;
	int retry_period = 100;
	int err =0;
	unsigned int flags = 1;
	if(fd < 0)
	{
		fd = open(ALSPS_NAME, O_RDONLY);
		if(fd < 0)
		{
			MALSPSLOGD("Open alsps device error!\n");
			return false;
		}
		else
		{
		    MALSPSLOGD("Open alsps OK!\n");
		}
	}

	//enable als ps
	 retry = 0;
     while ((err = ioctl(fd, ALSPS_SET_ALS_MODE, &flags)) && (retry ++ < max_retry)) 
        usleep(retry_period*1000);
     if (err) 
	 {
        MALSPSLOGD("enable als fail: %s", strerror(errno));
        return false;            
     }
	 retry = 0;
     while ((err = ioctl(fd, ALSPS_SET_PS_MODE, &flags)) && (retry ++ < max_retry))
        usleep(retry_period*1000);
     if (err) 
	 {
         MALSPSLOGD("enable ps fail: %s", strerror(errno));
         return false;            
     } 
	 return true;
   
}

int alsps_exec_read_raw()
{
	int err = -1;
    int als_dat, ps_dat=0;
    if (fd == -1) 
	{
        MALSPSLOGD("invalid fd\n");
        return -1;
    } 
	
	if ((err = ioctl(fd, ALSPS_GET_ALS_RAW_DATA, &als_dat))) 
	{
        MALSPSLOGD("read als raw error \n");
        return err;
    }
	MALSPSLOGD("als=%x\n",als_dat);

	if ((err = ioctl(fd, ALSPS_GET_PS_RAW_DATA, &ps_dat))) 
	{
        MALSPSLOGD("read ps  raw error \n");
        return err;
    }
	MALSPSLOGD("ps=%x\n",ps_dat);
	
	return err;
}


int Meta_ALSPS_OP()
{

	int err=0;
	err = alsps_exec_read_raw();
	return err;
}
/*---------------------------------------------------------------------------*/
BOOL Meta_ALSPS_Close(void)
{
    if(fd < 0)
    {
    	MALSPSLOGD("alsps device handle is not valid\n");
		return false;
	}
	else
	{
		close(fd);
		return true;
	}   
}
/*---------------------------------------------------------------------------*/
