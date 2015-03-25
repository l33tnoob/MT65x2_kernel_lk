#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/mtgpio.h>
#include "meta_msensor.h"
#include <linux/sensors_io.h>

//#include "libhwm.h"
/*---------------------------------------------------------------------------*/
#define MS_PREFIX   "MS: "
#define MSENSOR_NAME	"/dev/msensor"

/*---------------------------------------------------------------------------*/
#define MMSLOGD(fmt, arg ...) META_LOG(MS_PREFIX fmt, ##arg)
//#define MMSLOGD(fmt, arg...) printf(MS_PREFIX fmt, ##arg)

/*---------------------------------------------------------------------------*/
static int fd = -1;
/*---------------------------------------------------------------------------*/
//static MS_CNF_CB meta_ms_cb = NULL;
/*---------------------------------------------------------------------------*/
/*
void Meta_MSensor_Register(MS_CNF_CB cb)
{
    meta_ms_cb = cb;
}
*/
/*---------------------------------------------------------------------------*/
bool Meta_MSensor_Open(void)
{
	if(fd < 0)
	{
		fd = open(MSENSOR_NAME, O_RDONLY);
		if(fd < 0)
		{
			MMSLOGD("Open msensor device error!\n");
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		MMSLOGD("msensor file handle is not right!\n");
		return false;
	}
}

int ms_exec_read_raw()
{
	int err = 0;
	int x, y, z;
	int i=0;
	static char buf[128];
	if(fd < 0)
	{
		MMSLOGD("null pointer: %d\n", fd);
		err= -1;
		return err;
	}
	
	err = ioctl(fd, MSENSOR_IOCTL_SET_MODE, 1);
	err = ioctl(fd, MSENSOR_IOCTL_INIT, 0);
	
	for(i=0; i< 20; i++ )
	{
	  err = ioctl(fd, MSENSOR_IOCTL_READ_SENSORDATA, buf);
	  if(err)
	  {
			MMSLOGD("read data fail\n");
	  }
	  else if(3 != sscanf(buf, "%x %x %x", &x, &y, &z))
	  {
		MMSLOGD("read format fail\n");
	  }
	  else
	  {
	    MMSLOGD("data x=%d , y=%d, z= %d\n",x,y,z);
	  }
	}
	
	return err;
}


int Meta_MSensor_OP()
{

	int err=0;
	err = ms_exec_read_raw();
	return err;
}
/*---------------------------------------------------------------------------*/
bool Meta_MSensor_Close(void)
{
    if(fd < 0)
    {
    	MMSLOGD("Msensor device handle is not valid\n");
		return false;
	}
	else
	{
		close(fd);
		return true;
	}   
}
/*---------------------------------------------------------------------------*/
