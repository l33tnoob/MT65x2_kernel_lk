/*
*FM radio driver kernel module insmod file for wmt dynamic loader
*/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

//For directory operation
#include <dirent.h>

#define FMR_MODULES_PATH "/system/lib/modules/mtk_fm_drv.ko"
extern int load_fm_module(int chip_id);
extern int init_module(void *, unsigned long, const char *);

//insmod
static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret = -1;
	int retry = 10;

	printf("filename(%s)\n",filename);
	
    module = load_file(filename, &size);
    if (!module)
    {
    	printf("load file fail\n");
        return -1;
    }
	
	while(retry-- > 0){
	    ret = init_module(module, size, args);

		if(ret < 0)
		{
			printf("insmod module fail(%d)\n",ret);
			usleep(10000);
		}
		else
			break;
	
	}
	
    free(module);

    return ret;
}

int load_fm_module(int chip_id)
{
    int ret=-1;
    ret = insmod(FMR_MODULES_PATH, "");
    if(ret)
    {
        printf("insert mtk_fm_drv.ko fail(%d)\n",ret);
    }
    else
    {
        printf("insert mtk_fm_drv.ko ok\n");
    }
    return ret;
}

