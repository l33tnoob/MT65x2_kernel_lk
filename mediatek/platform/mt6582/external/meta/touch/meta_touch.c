#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "meta_touch.h"
#include "WM2Linux.h"

#define TOUCH_LOG META_LOG
const char *FILENAME = "/sys/module/tpd_setting/parameters/tpd_load_status";

void Meta_Touch_OP(Touch_REQ *req, char *peer_buff, unsigned short peer_len)
{
	Touch_CNF TouchMetaReturn;
	int ret, fd = -1;
	int status = 0;

	memset(&TouchMetaReturn, 0, sizeof(TouchMetaReturn));

	TouchMetaReturn.header.id=req->header.id+1;
	TouchMetaReturn.header.token = req->header.token;
	TouchMetaReturn.status=META_FAILED;
	
	/* open file */
	fd = open(FILENAME,O_RDWR, 0);
	if (fd < 0) {
		TOUCH_LOG("Open %s : ERROR \n", FILENAME);
		TOUCH_LOG("Open %s : ERROR \n", FILENAME);
		goto Touch_Finish;
	}
	
	ret = read(fd, &status, sizeof(int));
	if((status & 0X0ff) == '1')
		TouchMetaReturn.status=META_SUCCESS; 
		
	TOUCH_LOG("Cap touch status:%d\n", status);
	close(fd);

Touch_Finish:
	if (false == WriteDataToPC(&TouchMetaReturn,sizeof(Touch_CNF),NULL,0)) {
		TOUCH_LOG("%s : WriteDataToPC() fail 2\n", __FUNCTION__);
    }
	TOUCH_LOG("%s : Finish !\n", __FUNCTION__);
	
}

BOOL Meta_Touch_Init(void)
{
	return true;
}

BOOL Meta_Touch_Deinit(void)
{
	return true;
}
