#include <jni.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#define DEV_IOC_MAGIC       'd'
#define READ_DEV_DATA       _IOR(DEV_IOC_MAGIC,  1, unsigned int)

#define LOG_TAG "ThermalStressJNI"
#include <cutils/xlog.h>
#define LOG(...) \
        do { \
            XLOGD(__VA_ARGS__); \
        } while (0)

JNIEXPORT jint JNICALL Java_com_mediatek_mtkthermalstress_MainActivity_getData
   (JNIEnv * env, jobject obj)
{
	jint data = -1;
	int fd = 0;
	int ret = 0;
	unsigned int devinfo_data = 17;

	fd = open("/dev/devmap", O_RDONLY, 0);
    if (fd >= 0) {
	    ret = ioctl(fd, READ_DEV_DATA, &devinfo_data);
	    if (ret != 0) {
			//Get data Fail
			data = -2;
			LOG("%s ioctl failed\n", __func__);
		}
		else {
			LOG("Raw 0x%x\n", devinfo_data);
			data = (devinfo_data & 0x1)<<5;
			data += (~devinfo_data & 0x2)<<5;
			data += (devinfo_data & 0x4)<<5;
			data += (~devinfo_data & 0x8)>>3;
			data += (devinfo_data & 0x10)>>3;
			data += (~devinfo_data & 0x20)>>3;
			data += (devinfo_data & 0x40)>>3;
			data += (~devinfo_data & 0x80)>>3;
			LOG("data %x\n", data);
		}
	}
	else{
		data = -3;
		LOG("%s Open fd failed\n", __func__);
	}
	return data;
}


JNIEXPORT jint JNICALL Java_com_mediatek_mtkthermalstress_MainActivity_getLevel
   (JNIEnv * env, jobject obj)
{
	jint data = 0;	//Return 0 as default
	int fd = 0;
	int ret = 0;
	unsigned int devinfo_data = 15;

	fd = open("/dev/devmap", O_RDONLY, 0);
    if (fd >= 0) {
	    ret = ioctl(fd, READ_DEV_DATA, &devinfo_data);
	    if (ret != 0) {
			LOG("%s ioctl failed\n", __func__);
		}
		else {
			LOG("Raw 0x%x\n", devinfo_data);
			data = ((devinfo_data)>>28)&0x7;
			LOG("level %x\n", data);
		}
	}
	else{
		LOG("%s Open fd failed\n", __func__);
	}
	return data;
}

