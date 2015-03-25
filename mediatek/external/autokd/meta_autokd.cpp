#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/rtc.h>
#include <sys/mman.h>
#include <utils/Log.h>
#include <string.h>

#include "meta_autok_para.h"
#include "meta_common.h"
#include "FT_Public.h"

#define TAG "[AUTOK] "

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGI ALOGI
#define LOGW ALOGW

#define LTE_PROC_NODE	        "/proc/lte"
#define AUTOK_PROC_NODE	        "/proc/autok"
#define AUTOK_DATA	            "/data/autok"
#define LTE_AUTOK_PROC_NODE     "/proc/lte_autok"
#define ACTION_SDIO_AUTOK       "sdio_autok"
#define ACTION_LTE_DRV          "lte_drv"
#define ACTION_AUTOK_DONE       "autok_done"
#define UEVENT_TEST_PATTERN     "UEVENT_TEST"
#define KEY_LTE                 "SDIOFUNC="
#define KEY_SDIO                "FROM="
#define ENUM_KEY_SDIO           1
#define ENUM_KEY_LTE            2

#define READ_SIZE		4

const char SYS_SERVER[] = "system_server";

#if 0
static int dbg = 1;
#define dprintf(x...)	do { if (dbg) printf(x); } while (0)
#else
#define dprintf(x...)	do {} while (0)
#endif

struct uevent {
    int key;
    const char *action;
};

#define UEVENT_MSG_LEN  1024
static int device_fd = -1;
#define BUF_LEN     16

static void parse_event(const char *msg, struct uevent *uevent);
static void handle_autok_event(struct uevent *uevent);



static void handle_autok_event(struct uevent *uevent)
{
    char WBuf[BUF_LEN] = {'\0'};
    char rBuf[BUF_LEN] = {'\0'};
    static char sdio_func[UEVENT_MSG_LEN];
    static char autokParams[256];
    static char procParams[512];
    int paramsOffset = 0;
    int autokLen = 0;
    int fd_wr, fd_rd;
    LOGI("Enter %s\n", __FUNCTION__);
    if (!strcmp(uevent->action, ACTION_SDIO_AUTOK)){
        // Set AutoK first event
        fd_rd = open(AUTOK_PROC_NODE, O_RDONLY, 0000);
        if (read(fd_rd, rBuf, BUF_LEN) == -1) {
            LOGE("Can't read %s\n", AUTOK_PROC_NODE);
            close(fd_rd);
            return;
        }
        close(fd_rd);
        fd_wr = open(AUTOK_DATA, O_WRONLY, 0000);
        if (write(fd_wr, rBuf, strlen(SYS_SERVER)) == -1){
            LOGE("Can't write %s\n", AUTOK_DATA);
            fclose(fd_wr);
            return;
        }
        close(fd_wr);
        
    } else if (!strcmp(uevent->action, ACTION_LTE_DRV)){
        char stage[1];
        stage[0] = 0;
        // Set LTE event
        memcpy(procParams+paramsOffset, sdio_func, strlen(sdio_func));
        paramsOffset += strlen(sdio_func);
        if(is_file_exist(AUTOK_DATA)){  // /data/autok is exist
            // 1. Fill the stage with 1
            stage[0] = 2;
            memcpy(procParams+paramsOffset, stage, 1);
            paramsOffset += 1;
            // 2. Get the data from /data/autoK
            memset(autokParams, 0, sizeof(autokParams)/sizeof(autokParams[0]));
            fd_rd = open(AUTOK_DATA, O_RDONLY, 0000);
            if (read(fd_rd, autokParams, sizeof(autokParams)/sizeof(autokParams[0])) == -1) 
            {
                memcpy(procParams+paramsOffset, autokParams, strlen(autokParams));
                paramsOffset += strlen(autokParams);
                LOGE("Can't read %s\n", AUTOK_PROC_NODE);
                fclose(fd_rd);
                return;
            }
        } else {    // /data/autok is not exist
            // Fill the stage with 1
            stage[0] = 1;
            memcpy(procParams+paramsOffset, stage, 1);
            paramsOffset += 1;
        }
        // Write result to /proc/autoK
        fd_wr = open(AUTOK_PROC_NODE, O_WRONLY, 0000);
        if (write(fd_wr, procParams, strlen(procParams)) == -1){
            LOGE("Can't write %s\n", AUTOK_PROC_NODE);
            fclose(fd_wr);
            return;
        }
        close(fd_wr);
        
    } else if (!strcmp(uevent->action, ACTION_AUTOK_DONE)){
        // Set AutoK Done Event
        char lteprocParams[] = "autok_done";
        fd_wr = open(LTE_AUTOK_PROC_NODE, O_WRONLY, 0000);
        if (write(fd_wr, lteprocParams, strlen(lteprocParams)) == -1){
            LOGE("Can't write %s\n", AUTOK_PROC_NODE);
            fclose(fd_wr);
            return;
        }
        close(fd_wr);
        
    } else if ((!strcmp(uevent->key, KEY_LTE)) && strcmp(uevent->action, UEVENT_TEST_PATTERN)){
        memset(sdio_func, 0, UEVENT_MSG_LEN);
        strcpy (sdio_func, uevent->action );
    }
}

void handle_uevent_fd()
{
    char msg[UEVENT_MSG_LEN+2];
    int n;
    
    while ((n = uevent_kernel_multicast_recv(device_fd, msg, UEVENT_MSG_LEN)) > 0) {
        if(n >= UEVENT_MSG_LEN)   /* overflow -- discard */
            continue;

        msg[n] = '\0';
        msg[n+1] = '\0';

        struct uevent uevent;
        parse_event(msg, &uevent);
        
        //handle_autok_event(&uevent);
    }
}

static void parse_event(const char *msg, struct uevent *uevent)
{
    uevent->action = "";
    
    /* currently ignoring SEQNUM */
    while(*msg) {
        if(!strncmp(msg, KEY_SDIO, 5)) {
            msg += 5;
            uevent->key = ENUM_KEY_SDIO;
            uevent->action = msg;
            LOGI("event { '%s':'%s'}\n", uevent->key, uevent->action);
            break;
        } else if(!strncmp(msg, KEY_LTE, 9)) {
            msg += 9;
            uevent->key = ENUM_KEY_LTE;
            uevent->action = msg;
            LOGI("event { '%s':'%s'}\n", uevent->key, uevent->action);
            break;
        }

        /* advance to after the next \0 */
        //while(*msg++)
        //    ;
    }

    
}


int autok_init(void)
{
    int fd;
    /* is 64K enough? udev uses 16MB! */
    device_fd = uevent_open_socket(64*1024, true);
    if(device_fd < 0)
        return -1;
    LOGI("Enter AutoK Init");
    fcntl(device_fd, F_SETFD, FD_CLOEXEC);
    fcntl(device_fd, F_SETFL, O_NONBLOCK);

    fd = open(LTE_PROC_NODE, O_WRONLY, 0000);
    if (write(fd, SYS_SERVER, strlen(SYS_SERVER)) == -1){
        LOGE("Can't write %s\n", LTE_PROC_NODE);
        goto EXIT;
    }
EXIT:
    close(fd);
    return 0;
}

bool Meta_AUTOK_Open()
{
	int r, fd;
	bool result = false;
	autok_init();
    handle_uevent_fd();
	return result;
}

bool META_AUTOK_Close ()
{
	int fd;
	bool result = false;
    return result;
}


