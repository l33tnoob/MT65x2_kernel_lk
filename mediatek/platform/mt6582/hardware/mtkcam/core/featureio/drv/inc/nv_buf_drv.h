#ifndef __NV_BUF_DRV_H__
#define __NV_BUF_DRV_H__

#ifdef WIN32
//#define	DEF_CriticalSection(cs) CRITICAL_SECTION cs;
#define	DEF_AutoLock(cs) AutoLock lock(cs)
#define	DEF_InitCs(cs) InitializeCriticalSection(&cs)
#else
//#define	DEF_CriticalSection(cs) mutable Mutex cs;
#define	DEF_AutoLock(cs) Mutex::Autolock lock(cs)
#define	DEF_InitCs(cs)
#endif

enum
{
	e_NV_SensorDevWrong=-1000,
	e_NV_NvramIdWrong,
};
class NvBufDrv
{
public:
	static NvBufDrv* getInstance();
	int getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p);
	int read(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);
	int write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);
	int readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);
	int forceRead(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);
private:
#ifndef WIN32
	mutable Mutex m_cs;
#endif
	NvBufDrv();
};


#endif