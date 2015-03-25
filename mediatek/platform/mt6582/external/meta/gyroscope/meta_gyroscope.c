#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <linux/mtgpio.h>
#include "meta_gyroscope.h"
#include "libhwm.h"
/*---------------------------------------------------------------------------*/
#define GYRO_PREFIX   "GYRO: "
/*---------------------------------------------------------------------------*/
#define MGYROLOGD(fmt, arg...) META_LOG(GYRO_PREFIX fmt, ##arg)
/*---------------------------------------------------------------------------*/
static int fd = -1;
/*---------------------------------------------------------------------------*/
static GYRO_CNF_CB meta_gyro_cb = NULL;
/*---------------------------------------------------------------------------*/
void Meta_Gyroscope_Register(GYRO_CNF_CB cb)
{
    meta_gyro_cb = cb;
}
/*---------------------------------------------------------------------------*/
bool Meta_Gyroscope_Open(void)
{
	if(fd < 0)
	{
		gyroscope_open(&fd);
		if(fd < 0)
		{
			MGYROLOGD("Open gyroscope device error!\n");
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		MGYROLOGD("gyroscope file handle is not right!\n");
		return false;
	}
}
/*---------------------------------------------------------------------------*/
int gyro_exec_cali(GYRO_CMD_CALI* arg, HwmData *cali)
{  
	/*because the resolution in driver and nvram are different, the calibration will be set 
	to driver and re-get the data*/
	int err;
	HwmData cur;
	if((fd < 0) || !arg || !cali)
	{
		MGYROLOGD("null pointer: %p %p %p\n", fd, arg, cali);               
	}
	else if(err = gyroscope_calibration(fd, arg->delay, arg->num, arg->tolerance, 0, &cur))
	{
		MGYROLOGD("calibrate acc: %d\n", err);
		return err;
	}
	else if(err = gyroscope_set_cali(fd, &cur))
	{    
		MGYROLOGD("set calibration fail: (%s) %d\n", strerror(errno), err);
		return err;
	}
	else if(err = gyroscope_get_cali(fd, &cur))
	{    
		MGYROLOGD("get calibration fail: (%s) %d\n", strerror(errno), err);
		return err;
	}
	else
	{
		cali->rx = cur.x;
		cali->ry = cur.y;
		cali->rz = cur.z;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int gyro_exec_read_raw(HwmData *dat)
{
	int err = 0;
	HwmData cur;
	if((fd < 0) || !dat)
	{
		MGYROLOGD("null pointer: %p %p\n", fd, dat);               
	}
	else if(err = gyroscope_read(fd, &cur))
	{
		MGYROLOGD("read fail: %d(%s)", errno, strerror(errno));    
	}
	else
	{
		dat->rx = cur.x;
		dat->ry = cur.y;
		dat->rz = cur.z;
	}
	return err;
}
/*---------------------------------------------------------------------------*/
int gyro_exec_write_nvram(GYRO_CMD_WRITE_NVRAM* arg)
{
	HwmData dat;

	if((fd < 0) || !arg)
	{
		MGYROLOGD("null pointer: %p %p\n", fd, arg);               
		return -EINVAL;
	}
	
	dat.x = arg->x;
	dat.y = arg->y;
	dat.z = arg->z;
	return gyroscope_write_nvram(&dat);
}
/*---------------------------------------------------------------------------*/
int gyro_exec_read_nvram(HwmData *dat)
{
	int err;
	HwmData cur;
	if((fd < 0) || !dat)
	{
		MGYROLOGD("null pointer: %p %p\n", fd, dat);               
		return -EINVAL;
	}
	else if(err = gyroscope_read_nvram(&cur))
	{
		MGYROLOGD("read nvram fail: %d (%s)\n",errno, strerror(errno));
	}
	else
	{    
		dat->rx = GRA_TO_LSB(cur.x);
		dat->ry = GRA_TO_LSB(cur.y);
		dat->rz = GRA_TO_LSB(cur.z);
		printf("%f %f %f\n", cur.x, cur.y, cur.z);
	}
	return err;
}
/*---------------------------------------------------------------------------*/
void Meta_Gyroscope_OP(GYRO_REQ *req, char *peer_buff, unsigned short peer_len)
{
	HwmData dat;
	GYRO_CNF cnf;
	int err;

	memset(&cnf, 0, sizeof(GYRO_CNF));	
	cnf.header.id = FT_GYROSCOPE_CNF_ID;
	cnf.header.token = req->header.token;
	cnf.op = req->op;    
	switch (req->op)
	{
		case GYRO_OP_CALI:
		cnf.gyro_err = gyro_exec_cali(&req->cmd.cali, &dat);
		if(cnf.gyro_err)
		{
			cnf.status = META_FAILED;
		}
		else
		{
			cnf.status = META_SUCCESS;
			cnf.ack.cali.x = dat.rx;
			cnf.ack.cali.y = dat.ry;
			cnf.ack.cali.z = dat.rz;
		}
		break;
		
		case GYRO_OP_READ_RAW:
		cnf.gyro_err = gyro_exec_read_raw(&dat);
		if(cnf.gyro_err)
		{
			cnf.status = META_FAILED;                
		}
		else
		{
			cnf.status = META_SUCCESS;
			cnf.ack.readraw.x = dat.rx;
			cnf.ack.readraw.y = dat.ry;
			cnf.ack.readraw.z = dat.rz;
		}
		break;
		
		case GYRO_OP_WRITE_NVRAM:
		cnf.gyro_err = gyro_exec_write_nvram(&req->cmd.writenv);
		cnf.ack.writenv.dummy = 0;
		if(cnf.gyro_err)
		{
			cnf.status = META_FAILED;
		}
		else
		{
			cnf.status = META_SUCCESS;                
		}
		break;
		
		case GYRO_OP_READ_NVRAM:
		cnf.gyro_err = gyro_exec_read_nvram(&dat);
		if(cnf.gyro_err)
		{
			cnf.status = META_FAILED;
		}
		else
		{
			cnf.status = META_SUCCESS;
			cnf.ack.readnv.x = dat.rx;
			cnf.ack.readnv.y = dat.ry;
			cnf.ack.readnv.z = dat.rz;
		}
		break;
		
		default:
		cnf.gyro_err = -EINVAL;
		cnf.status = META_FAILED;
		MGYROLOGD("unknown OP: %d\n", req->op);
		break;
	}
	
	if(meta_gyro_cb)
	{
		meta_gyro_cb(&cnf);
	}
	else
	{
		WriteDataToPC(&cnf, sizeof(GYRO_CNF), NULL, 0);
	}
}
/*---------------------------------------------------------------------------*/
bool Meta_Gyroscope_Close(void)
{
    if(fd < 0)
    {
    	MGYROLOGD("Gyroscope device handle is not valid\n");
		return false;
	}
	else
	{
		close(fd);
		return true;
	}   
}
/*---------------------------------------------------------------------------*/
