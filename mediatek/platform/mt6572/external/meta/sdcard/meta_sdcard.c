#include "meta_sdcard.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utils/Log.h>


#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)
#define CHKERR(x)   do { if((x) < 0) goto error; } while(0)

static char *path = NULL;
static SDCARD_CNF_CB cnf_cb = NULL;

static int meta_sdcard_read_info(const char *filename, void *buf, ssize_t bufsz)
{
    int fd, rsize;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		META_SDCARD_LOG("Open %s failed errno(%s)",filename,(char*)strerror(errno));
        return -1;
    }

    rsize = read(fd, buf, bufsz);
	if(rsize < 0)
		META_SDCARD_LOG("read %s failed errno(%s)",filename,(char*)strerror(errno));
    close(fd);

    return rsize;
}

static void meta_sdcard_send_resp(SDCARD_CNF *cnf)
{
    if (cnf_cb)
        cnf_cb(cnf);
    else
        WriteDataToPC(cnf, sizeof(SDCARD_CNF), NULL, 0);
}

void Meta_SDcard_Register(SDCARD_CNF_CB callback)
{
    cnf_cb = callback;
}

bool Meta_SDcard_Init(SDCARD_REQ *req)
{
    int id = (int)req->dwSDHCIndex;
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;
	META_SDCARD_LOG("ID(%d)\n",id);
    if (id < MIN_SDCARD_IDX || id > MAX_SDCARD_IDX){
		META_SDCARD_LOG("ID error(%d)\n",id);
        return false;}

    if (!path && NULL == (path = malloc(512))) {
        META_SDCARD_LOG("No memory\n");
        return false;
    }

    sprintf(name, "mmc%d", id - MIN_SDCARD_IDX);

    ptr = path;
    ptr += sprintf(ptr, "/sys/class/mmc_host/%s", name);

    if (NULL == (dp = opendir(path)))
        goto error;

    while (NULL != (dirp = readdir(dp))) {
        if (strstr(dirp->d_name, name)) {
            ptr += sprintf(ptr, "/%s", dirp->d_name);
            break;
        }
    }

    closedir(dp);

    if (!dirp)
        goto error;
	META_SDCARD_LOG("[META_SD] path found: %s/\n", path); 
    return true;

error:
    META_SDCARD_LOG("[META_SD] path not found: %s/\n", path); 

    FREEIF(path);
    return false;
}

bool Meta_SDcard_Deinit(void)
{
    FREEIF(path);    
    return true;
}


void Meta_SDcard_OP(SDCARD_REQ *req, char *peer_buf, unsigned short peer_len)
{
    SDCARD_CNF cnf;
    int bufsz = 512;
    char fname[512];
    char buf[512];
	unsigned char cid[16];
	unsigned char csd[16];

    memset(&cnf, 0, sizeof(SDCARD_CNF));

    cnf.header.id = FT_SDCARD_CNF_ID;
    cnf.header.token = req->header.token;
    cnf.status = META_SUCCESS;

    sprintf(fname, "%s/cid", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
   	memcpy(cid,buf,16*sizeof(unsigned char));

    sprintf(fname, "%s/csd", path);
    CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
    memcpy(csd,buf,16*sizeof(unsigned char));
	
    meta_sdcard_send_resp(&cnf);
    return;

error:
    cnf.status = META_FAILED;
    meta_sdcard_send_resp(&cnf);
    return;
}

