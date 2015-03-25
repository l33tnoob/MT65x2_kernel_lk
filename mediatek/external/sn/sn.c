#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#include <cutils/xlog.h>

#define DEBUG 0

#define SN_LOG_TAG "sn"
#define SN_DBG_LOG(_fmt_, args...) \
    do { \
        if (DEBUG) { \
            sxlog_printf(ANDROID_LOG_INFO, SN_LOG_TAG, _fmt_, ##args); \
        } \
    } while(0)

#define SN_INFO_LOG(_fmt_, args...) \
    do { sxlog_printf(ANDROID_LOG_INFO, SN_LOG_TAG, _fmt_, ##args); } while(0)

#define FILENAME "opensesame"

#define PATH1 "/sdcard/" FILENAME
#define PATH2 "/storage/sdcard0/" FILENAME
#define PATH3 "/storage/sdcard1/" FILENAME
#define CMODE_SYS "/sys/devices/platform/mt_usb/cmode"

#define SN_LEN 20 /* The rule of SN pattern = "^([0-9A-Za-z]{0,20})$" */

FILE *chk_file(const char *path)
{
	int ret = 0;
	FILE *fp = NULL;

	/*Check file*/
	if (0 == access(path, R_OK)) {
		if ((fp = fopen(path, "r")) != NULL) {
			 SN_DBG_LOG("Open %s\n", path);
		} else {
			 SN_INFO_LOG("Fail to open err=%s\n", strerror(errno));
		}
	} else {
		SN_INFO_LOG("Fail to access err=%s", strerror(errno));
	}

	return fp;
}

int reconnect_usb()
{
	int sys_fp;
	int ret;
	
	sys_fp = open(CMODE_SYS, O_RDWR);
	SN_INFO_LOG("write 0\n");
	ret = write(sys_fp, "0", sizeof(char));
	if (ret <= 0)	{
		SN_INFO_LOG("Fail to write 0 ret=%x\n", ret);
		goto end;
	} 
	
	sleep(2);

	ret = write(sys_fp, "1", sizeof(char));
	SN_INFO_LOG("write 1\n");
	if (ret <= 0)	{
		SN_INFO_LOG("Fail to write 1 ret=%x\n", ret);
		goto end;
	} 
	
	SN_DBG_LOG("Success to write cmode\n");
	
end:
	if(sys_fp) close(sys_fp);
	return ret;
}

int main(int argc, char *argv[])
{
	int retry = 0;

	while(retry < 10) {
		/*READ Serial Number from file first*/
		char buf[SN_LEN+1] = {0};
		char sn[SN_LEN+1] = {0};
		int ret, len;
		FILE *fp = NULL;
		int sys_fp;

		SN_INFO_LOG("Retry %d\n", retry);

		if( (fp = chk_file(PATH1)) != NULL) {
			goto read;
		} else if( (fp = chk_file(PATH2)) != NULL) {
			goto read;
		} else if( (fp = chk_file(PATH3)) != NULL) {
			goto read;
		} else {
			SN_INFO_LOG("Check all possible paths\n");
			goto fail;
		}
read:
		/*Read file*/
		len = fread(buf, 1, sizeof(char)*SN_LEN, fp);
		if (ferror(fp) || len == 0 || len > SN_LEN) {
			SN_INFO_LOG("err=%s: fread fail ret=%x\n", strerror(errno), len);
			goto fail;
		}
		sn[len] = '\0';

		SN_DBG_LOG("Length=%x, Content=%s\n", len, buf);

		/*Write data to sysfs*/
		sys_fp = open("/sys/class/android_usb/android0/iSerial", O_RDWR);
		strncpy(sn, buf, sizeof(char)*(SN_LEN+1));

		ret = write(sys_fp, sn, sizeof(char) * strlen(sn));

		if (ret <= 0)	{
			SN_INFO_LOG("Fail to write %s ret=%x\n", sn, ret);
			goto fail;
		} else {
			SN_DBG_LOG("Success to write %s\n", sn);
			if(fp) fclose(fp);
			if(sys_fp) close(sys_fp);
			if(ret) reconnect_usb();
			break;
		}

fail:
		if(fp) fclose(fp);
		if(sys_fp) close(sys_fp);
		retry++;
		sleep(10);
	}
	return 0;
}
