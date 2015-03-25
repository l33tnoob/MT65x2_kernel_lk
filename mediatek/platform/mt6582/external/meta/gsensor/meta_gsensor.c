#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <linux/mtgpio.h>
#include "meta_gsensor.h"
#include "libhwm.h"
/*---------------------------------------------------------------------------*/
#define GS_PREFIX   "GS: "
/*---------------------------------------------------------------------------*/
#define MGSLOGD(fmt, arg...) META_LOG(GS_PREFIX fmt, ##arg)
/*---------------------------------------------------------------------------*/
static int fd = -1;
/*---------------------------------------------------------------------------*/
static GS_CNF_CB meta_gs_cb = NULL;
/*---------------------------------------------------------------------------*/
void Meta_GSensor_Register(GS_CNF_CB cb)
{
    meta_gs_cb = cb;
}
/*---------------------------------------------------------------------------*/
bool Meta_GSensor_Open(void)
{
	if(fd < 0)
	{
		gsensor_open(&fd);
		if(fd < 0)
		{
			MGSLOGD("Open gsensor device error!\n");
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		MGSLOGD("gsensor file handle is not right!\n");
		return false;
	}
}
/*---------------------------------------------------------------------------*/
int gs_exec_cali(GS_CMD_CALI* arg, HwmData *cali)
{  
	/*because the resolution in driver and nvram are different, the calibration will be set 
	to driver and re-get the data*/
	int err;
	HwmData cur;
	if((fd < 0) || !arg || !cali)
	{
		MGSLOGD("null pointer: %p %p %p\n", fd, arg, cali);               
	}
	else if(err = gsensor_calibration(fd, arg->delay, arg->num, arg->tolerance, 0, &cur))
	{
		MGSLOGD("calibrate acc: %d\n", err);
		return err;
	}
	else if(err = gsensor_set_cali(fd, &cur))
	{    
		MGSLOGD("set calibration fail: (%s) %d\n", strerror(errno), err);
		return err;
	}
	else if(err = gsensor_get_cali(fd, &cur))
	{    
		MGSLOGD("get calibration fail: (%s) %d\n", strerror(errno), err);
		return err;
	}
	else
	{
		cali->rx = GRA_TO_LSB(cur.x);
		cali->ry = GRA_TO_LSB(cur.y);
		cali->rz = GRA_TO_LSB(cur.z);
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int gs_exec_read_raw(HwmData *dat)
{
	int err = 0;
	HwmData cur;
	if((fd < 0) || !dat)
	{
		MGSLOGD("null pointer: %p %p\n", fd, dat);               
	}
	else if(err = gsensor_read(fd, &cur))
	{
		MGSLOGD("read fail: %d(%s)", errno, strerror(errno));    
	}
	else
	{
		dat->rx = GRA_TO_LSB(cur.x);
		dat->ry = GRA_TO_LSB(cur.y);
		dat->rz = GRA_TO_LSB(cur.z);
	}
	return err;
}
/*---------------------------------------------------------------------------*/
int gs_exec_write_nvram(GS_CMD_WRITE_NVRAM* arg)
{
	HwmData dat;

	if((fd < 0) || !arg)
	{
		MGSLOGD("null pointer: %p %p\n", fd, arg);               
		return -EINVAL;
	}
	
	dat.x = LSB_TO_GRA(arg->x);
	dat.y = LSB_TO_GRA(arg->y);
	dat.z = LSB_TO_GRA(arg->z);
	return gsensor_write_nvram(&dat);
}
/*---------------------------------------------------------------------------*/
int gs_exec_read_nvram(HwmData *dat)
{
	int err;
	HwmData cur;
	if((fd < 0) || !dat)
	{
		MGSLOGD("null pointer: %p %p\n", fd, dat);               
		return -EINVAL;
	}
	else if(err = gsensor_read_nvram(&cur))
	{
		MGSLOGD("read nvram fail: %d (%s)\n",errno, strerror(errno));
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
void Meta_GSensor_OP(GS_REQ *req, char *peer_buff, unsigned short peer_len)
{
	HwmData dat;
	GS_CNF cnf;
	int err;

	memset(&cnf, 0, sizeof(GS_CNF));	
	cnf.header.id = FT_GSENSOR_CNF_ID;
	cnf.header.token = req->header.token;
	cnf.op = req->op;    
	switch (req->op)
	{
		case GS_OP_CALI:
		cnf.gs_err = gs_exec_cali(&req->cmd.cali, &dat);
		if(cnf.gs_err)
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
		
		case GS_OP_READ_RAW:
		cnf.gs_err = gs_exec_read_raw(&dat);
		if(cnf.gs_err)
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
		
		case GS_OP_WRITE_NVRAM:
		cnf.gs_err = gs_exec_write_nvram(&req->cmd.writenv);
		cnf.ack.writenv.dummy = 0;
		if(cnf.gs_err)
		{
			cnf.status = META_FAILED;
		}
		else
		{
			cnf.status = META_SUCCESS;                
		}
		break;
		
		case GS_OP_READ_NVRAM:
		cnf.gs_err = gs_exec_read_nvram(&dat);
		if(cnf.gs_err)
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
		cnf.gs_err = -EINVAL;
		cnf.status = META_FAILED;
		MGSLOGD("unknown OP: %d\n", req->op);
		break;
	}
	
	if(meta_gs_cb)
	{
		meta_gs_cb(&cnf);
	}
	else
	{
		WriteDataToPC(&cnf, sizeof(GS_CNF), NULL, 0);
	}
}
/*---------------------------------------------------------------------------*/
bool Meta_GSensor_Close(void)
{
    if(fd < 0)
    {
    	MGSLOGD("Gsensor device handle is not valid\n");
		return false;
	}
	else
	{
		close(fd);
		return true;
	}   
}
/*---------------------------------------------------------------------------*/
