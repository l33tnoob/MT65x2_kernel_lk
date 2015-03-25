#ifdef WIN32
  #include "win32_test.h"
  CRITICAL_SECTION m_cs;
#else
  #include <utils/Errors.h>
  #include <utils/Log.h>
  #include <fcntl.h>
  #include "../inc/nvram_drv.h"
  #include "nvram_drv_imp.h"
  #include "libnvram.h"
  #include "CFG_file_lid.h"
  #include "camera_custom_AEPlinetable.h"
  #include <aaa_types.h>
  #include "flash_param.h"
  #include "flash_tuning_custom.h"
  //#include "camera_custom_flicker_table.h"
  #include "camera_custom_msdk.h"
  #include <mtkcam/hal/sensor_hal.h>
#endif
#include "nv_buf_drv.h"




static int g_nvramArrInd[6]=
{
	(int)CAMERA_NVRAM_DATA_ISP,
	(int)CAMERA_NVRAM_DATA_3A,
	(int)CAMERA_NVRAM_DATA_SHADING,
	(int)CAMERA_NVRAM_DATA_LENS,
	(int)CAMERA_DATA_AE_PLINETABLE,
	(int)CAMERA_NVRAM_DATA_STROBE,
};
static int g_nvramSize[6]=
{
	(int)sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
	(int)sizeof(NVRAM_CAMERA_3A_STRUCT),
	(int)sizeof(NVRAM_CAMERA_SHADING_STRUCT),
	(int)sizeof(NVRAM_LENS_PARA_STRUCT),
	(int)sizeof(AE_PLINETABLE_T),
	(int)sizeof(NVRAM_CAMERA_STROBE_STRUCT),
};

static int g_sensorArrInd[3]=
{
	(int)DUAL_CAMERA_MAIN_SENSOR,
	(int)DUAL_CAMERA_SUB_SENSOR,
	(int)DUAL_CAMERA_MAIN_2_SENSOR
};

static int g_sensorHalInd[3]=
{
	(int)SENSOR_DEV_MAIN,
	(int)SENSOR_DEV_SUB,
	(int)SENSOR_DEV_MAIN_2
};

static int g_isNvBufRead[6][3];

////////////////////////////////////////////////////
template <class T>
static T* getMemMain()
{
	static T st;
	return &st;
}
template <class T>
static T* getMemSub()
{
	static T st;
	return &st;
}
template <class T>
static T* getMemMain2()
{
	static T st;
	return &st;
}

template <class T>
static int getMemDataType(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& m)
{
	if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
	{
		m = getMemMain<T>();
		return 0;
	}
	else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
	{
		m= getMemSub<T>();
		return 0;
	}
	else if(sensorDev==DUAL_CAMERA_MAIN_2_SENSOR)
	{
		m= getMemMain2<T>();
		return 0;
	}
	else
	{
		return e_NV_SensorDevWrong;
	}
}

static int getMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& buf)
{
	if(nvRamId==CAMERA_NVRAM_DATA_ISP)		return getMemDataType<NVRAM_CAMERA_ISP_PARAM_STRUCT>(sensorDev, buf);
	else if(nvRamId==CAMERA_NVRAM_DATA_3A)		return getMemDataType<NVRAM_CAMERA_3A_STRUCT>(sensorDev, buf);
	else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)		return getMemDataType<NVRAM_CAMERA_SHADING_STRUCT>(sensorDev, buf);
	else if(nvRamId==CAMERA_NVRAM_DATA_LENS)		return getMemDataType<NVRAM_LENS_PARA_STRUCT>(sensorDev, buf);
	else if(nvRamId==CAMERA_DATA_AE_PLINETABLE)		return getMemDataType<AE_PLINETABLE_T>(sensorDev, buf);
	else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)		return getMemDataType<NVRAM_CAMERA_STROBE_STRUCT>(sensorDev, buf);
	return e_NV_NvramIdWrong;
}
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
int getSenorArrInd(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
	int sensorArrSz;
	sensorArrSz = sizeof(g_sensorArrInd)/sizeof(int);
	int arrInd=-1;
	int i;
	for(i=0;i<sensorArrSz;i++)
	{
		if(sensorDev==g_sensorArrInd[i])
			arrInd=i;
	}
	return arrInd;
}
int getNvramArrInd(CAMERA_DATA_TYPE_ENUM nvEnum)
{
	int nvArrSz;
	nvArrSz = sizeof(g_nvramArrInd)/sizeof(int);
	int arrInd=-1;
	int i;
	for(i=0;i<nvArrSz;i++)
	{
		if(nvEnum==g_nvramArrInd[i])
			arrInd=i;
	}
	return arrInd;
}
NvBufDrv::NvBufDrv()
{
}
NvBufDrv* NvBufDrv::getInstance()
{
	static int bInit=0;
	if(bInit==0)
	{
		DEF_InitCs(m_cs);
		bInit=1;
		int nvRamArrSz;
		nvRamArrSz = sizeof(g_nvramArrInd)/sizeof(int);
		int sensorArrSz;
		sensorArrSz = sizeof(g_sensorArrInd)/sizeof(int);
		int i;
		int j;
		for(i=0;i<nvRamArrSz;i++)
		for(j=0;j<sensorArrSz;j++)
			g_isNvBufRead[i][j]=0;

	}
	static NvBufDrv obj;
	return &obj;
}

static int getSensorID(CAMERA_DUAL_CAMERA_SENSOR_ENUM i4SensorDev, int& seonsorId)
{
	int arrInd;
	arrInd = getSenorArrInd(i4SensorDev);
	if(arrInd<0)
		return e_NV_SensorDevWrong;

	int seonsorHalInd;
	seonsorHalInd = g_sensorHalInd[arrInd];

    SensorHal*const pSensorHal = SensorHal::createInstance();
    int id;
	pSensorHal->sendCommand((halSensorDev_e)seonsorHalInd, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&id), 0, 0);
	seonsorId=id;
    if  ( pSensorHal )
        pSensorHal->destroyInstance();

    return 0;
}

static int getNvSize(CAMERA_DATA_TYPE_ENUM camDataType, int& sz)
{
	int arrInd;
	arrInd = getNvramArrInd(camDataType);
	if(arrInd<0)
		return e_NV_NvramIdWrong;
	sz = g_nvramSize[arrInd];
	return 0;
}

static int isBufRead(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& isRead)
{
	int nvArrInd;
	int sensorArrInd;
	nvArrInd = getNvramArrInd(nvRamId);
	if(nvArrInd<0)
		return e_NV_NvramIdWrong;

	sensorArrInd = getSenorArrInd(sensorDev);
	if(sensorArrInd<0)
		return e_NV_SensorDevWrong;

	isRead = g_isNvBufRead[nvArrInd][sensorArrInd];
	g_isNvBufRead[nvArrInd][sensorArrInd]=1;
	return 0;
}

int forceReadNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
	int err;
	int u4SensorID;
	err = getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID);
	if(err!=0)
		return err;
	void* buf;
	err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf);
	if(err!=0)
		return err;
	int nvSize;
	err = getNvSize(nvRamId, nvSize);
	if(err!=0)
		return err;
	NvramDrvBase* nvDrv;
	nvDrv = NvramDrvBase::createInstance();
	err  = nvDrv->readNvram (
			(CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
			buf, nvSize	);
	nvDrv->destroyInstance();
	return err;
}

int readNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
	int err;
	int bRead;
	err = isBufRead(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, bRead);
	if(err!=0)
		return err;
	if(bRead==0)
		err = forceReadNoLock(nvRamId, sensorDev);
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int NvBufDrv::getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p)
{
	DEF_AutoLock(m_cs);
	int err;
	err = readNoLock(nvRamId, sensorDev);
	if(err != 0)
		return err;
	return getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
}
int NvBufDrv::read(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
	DEF_AutoLock(m_cs);
	return readNoLock(nvRamId, sensorDev);
}
int NvBufDrv::write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
	DEF_AutoLock(m_cs);
	int err;
	int u4SensorID;
	err = getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID);
	if(err!=0)
		return err;
	void* buf;
	err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf);
	if(err!=0)
		return err;
	int nvSize;
	err = getNvSize(nvRamId, nvSize);
	if(err!=0)
		return err;
	NvramDrvBase* nvDrv;
	nvDrv = NvramDrvBase::createInstance();
	err  = nvDrv->writeNvram(
			(CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
			buf, nvSize	);
	nvDrv->destroyInstance();
	return err;
}
int NvBufDrv::readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
	DEF_AutoLock(m_cs);
	return 0;
}

int NvBufDrv::forceRead(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
	DEF_AutoLock(m_cs);
	return forceReadNoLock(nvRamId, sensorDev);
}