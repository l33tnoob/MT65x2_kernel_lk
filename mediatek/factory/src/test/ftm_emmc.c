/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
//#include "include/linux/mmc/sd_misc.h"
#include "cust_mcard.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "mounts.h"
#include "make_ext4fs.h"

#if 1 
#include <sys/ioctl.h>
#include <fcntl.h>
#endif
#if 1

#define MSDC_ERASE_PARTITION 7
#endif

#ifdef FEATURE_FTM_EMMC

#define TAG                 "[EMMC] "
#define DATA_PARTITION      "/data"
/* should be moved to customized part */
#define MAX_NUM_SDCARDS     (3)
#define MIN_SDCARD_IDX      (0)
#define MAX_SDCARD_IDX      (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)

#define EXT_CSD_REV                     192 /* R */
#define EXT_CSD_SEC_CNT                 212 /* RO, 4 bytes */
#define EXT_CSD_BOOT_SIZE_MULTI	226
#define EXT_CSD_STRUCTURE	194	/* RO */

#define MSDC_DRIVING_SETTING              (0)
#define MSDC_CLOCK_FREQUENCY              (1)
#define MSDC_SINGLE_READ_WRITE            (2)
#define MSDC_MULTIPLE_READ_WRITE          (3)
#define MSDC_GET_CID                      (4)
#define MSDC_GET_CSD                      (5)
#define MSDC_GET_EXCSD                    (6)
#define MSDC_ERASE_PARTITION              (7)
#define MSDC_HOPPING_SETTING              (8)

struct msdc_ioctl{
	int  opcode;
	int  host_num;
	int  iswrite;
	int  trans_type;
	unsigned int  total_size;
	unsigned int  address;
	unsigned int* buffer;
	int  cmd_pu_driving;
	int  cmd_pd_driving;
	int  dat_pu_driving;
	int  dat_pd_driving;
	int  clk_pu_driving;
	int  clk_pd_driving;
	int  clock_freq;
  int  partition;
  int  hopping_bit;
  int  hopping_time;
	int  result;
	int sd30_mode;
	int sd30_max_current;
	int sd30_drive;
	int sd30_power_control;
};


typedef unsigned int u32;
enum {
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t emmc_items[] = {
    //    item(ITEM_PASS,   uistr_pass),
    //    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};
int g_test_result_emmc = 0;

struct emmc {
    int          id;
	char         sys_path[512];
    char         info[1024];
	u32		 	csd[4];
	unsigned char 		ext_csd[512];
	unsigned int capacity;
	unsigned int boot_size;
    bool         avail;

	text_t title;
    text_t text;
    bool exit_thd;
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define DEVICE_PATH "/dev/misc-sd"
//#define DEVICE_PATH "/dev/simple-mt6573-sd0"


#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		u32 __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})

#define mod_to_emmc(p)  (struct emmc*)((char*)(p) + sizeof(struct ftm_module))

#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)


static bool emmc_avail(struct emmc *mc)
{
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;

    if (mc->id < MIN_SDCARD_IDX || mc->id > MAX_SDCARD_IDX)
        return false;

    sprintf(name, "mmc%d", mc->id - MIN_SDCARD_IDX);

    ptr  = &(mc->sys_path[0]);
    ptr += sprintf(ptr, "/sys/class/mmc_host/%s", name);

    if (NULL == (dp = opendir(mc->sys_path)))
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

    return true;

error:
    return false;
}

static void emmc_update_info(struct emmc *mc, char *info)
{
    struct statfs stat;
	unsigned int e, m;
    char *ptr;
    int rc;
    bool old_avail = mc->avail;
	int inode = 0;
	int result = 0;
	int rev = 0;
	unsigned int sectors = 0;
	unsigned char structure = 0;
	struct msdc_ioctl l_ioctl_arg;
	memset(&l_ioctl_arg,0,sizeof(struct msdc_ioctl));
	l_ioctl_arg.host_num = mc->id;
	l_ioctl_arg.opcode = MSDC_GET_CSD;
	l_ioctl_arg.total_size = 16;
	l_ioctl_arg.buffer = mc->csd;
    g_test_result_emmc = 0;  /* 0: test no pass, 1: test pass */

    mc->avail = emmc_avail(mc);
	inode = open(DEVICE_PATH, O_RDONLY);
		if (inode < 0) {
			printf("open device error!\n");
			return;
		}
	result = ioctl(inode, MSDC_GET_CSD, &l_ioctl_arg);
	if(result < 0){
		printf("ioct error!\n");
		close(inode);
		return;
		}
	close(inode);
	e = UNSTUFF_BITS(l_ioctl_arg.buffer, 47, 3);
	m = UNSTUFF_BITS(l_ioctl_arg.buffer, 62, 12);
	mc->capacity	  = (1 + m) << (e + 2);
	
	structure = UNSTUFF_BITS(l_ioctl_arg.buffer, 126, 2);

	memset(&l_ioctl_arg,0,sizeof(struct msdc_ioctl));
	l_ioctl_arg.host_num = mc->id;
	l_ioctl_arg.opcode = MSDC_GET_EXCSD;
	l_ioctl_arg.total_size = 512;
	l_ioctl_arg.buffer = mc->ext_csd;
	
	inode = open(DEVICE_PATH, O_RDONLY);
			if (inode < 0) {
				printf("open device error!\n");
				return;
			}
	result = ioctl(inode, MSDC_GET_EXCSD, &l_ioctl_arg);
	if(result < 0){
		LOGE(TAG "ioct error!\n", __FUNCTION__);
		close(inode);
		return;
		}
	close(inode);
	
	
	
	if (structure == 3) {
		int ext_csd_struct = mc->ext_csd[EXT_CSD_STRUCTURE];
		if (ext_csd_struct > 2) {
			LOGE(TAG " unrecognised EXT_CSD structure version %d\n",ext_csd_struct);
			return;
		}
	}
	rev =mc->ext_csd[EXT_CSD_REV];
	if (rev > 7) {
		LOGE(TAG " unrecognised EXT_CSD revision %d\n",rev);
		return;
	}
	if (rev >= 2) {
		sectors =
			mc->ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
			mc->ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
			mc->ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
			mc->ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
		if (sectors) {
			unsigned boot_sectors;
			/* size is in 256K chunks, i.e. 512 sectors each */
			boot_sectors = mc->ext_csd[EXT_CSD_BOOT_SIZE_MULTI] * 512;
		}
	}
	//printf("sectors (%d),sectors * 512 (%d),sectors * 512/1024(%d)\n",sectors,sectors * 512,sectors * 512/1024);
	//printf("mc->capacity (%d)\n",mc->capacity);
	mc->capacity = (float)(sectors /1024 * 512)> (mc->capacity)?(float)(sectors /1024 * 512) :(mc->capacity);
    /* prepare info */
    ptr  = info;
	
	
	ptr += sprintf(ptr, "%s\n",uistr_info_emmc);

    ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_sd_avail,mc->avail ? uistr_info_emmc_sd_yes : uistr_info_emmc_sd_no);
    ptr += sprintf(ptr, "%s: %.2f GB\n",uistr_info_emmc_sd_total_size, 
        (float)(mc->capacity)/(1024*1024)); 
	
    g_test_result_emmc = 1;  /* all things done, mark successful */
    return;
}

static void *emmc_update_iv_thread(void *priv)
{
    struct emmc *mc = (struct emmc *)priv;
    struct itemview *iv = mc->iv;
    struct statfs stat;
    int count = 1, chkcnt = 10;
	int index = 0;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
 /*   while (1) {
        usleep(100000);
        chkcnt--;

        if (mc->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        chkcnt = 10;

        emmc_update_info(mc, mc->info);
        iv->redraw(iv);
        mc->exit_thd = true;
    }
    */
    //pthread_exit(NULL);
    	emmc_update_info(mc, mc->info);
        iv->redraw(iv);

    LOGD(TAG "%s: Exit\n", __FUNCTION__);

	return NULL;
}

int emmc_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
	int index = 0;
    bool exit = false;
    struct emmc *mc = (struct emmc *)priv;
    struct itemview *iv;
    struct statfs stat;
	
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&mc->title, param->name, COLOR_YELLOW);
    init_text(&mc->text, &mc->info[0], COLOR_YELLOW);   

    emmc_update_info(mc, mc->info);

    mc->exit_thd = false;

    if (!mc->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        mc->iv = iv;
    }
    
    iv = mc->iv;
    iv->set_title(iv, &mc->title);
    iv->set_items(iv, emmc_items, 0);
    iv->set_text(iv, &mc->text);
    iv->redraw(iv);
    
	emmc_update_iv_thread(priv);
    //pthread_create(&mc->update_thd, NULL, emmc_update_iv_thread, priv);
    /*do {
      chosen = iv->run(iv, &exit);
      switch (chosen) {
      case ITEM_PASS:
      case ITEM_FAIL:
      if (chosen == ITEM_PASS) {
      mc->mod->test_result = FTM_TEST_PASS;
      } else if (chosen == ITEM_FAIL) {
      mc->mod->test_result = FTM_TEST_FAIL;
      }           
      exit = true;
      break;
      }

      if (exit) {
      mc->exit_thd = true;
      break;
      }        
      } while (1);*/
    //pthread_join(mc->update_thd, NULL);

    if (g_test_result_emmc > 0) {
        mc->mod->test_result = FTM_TEST_PASS;
    }
    else {
        mc->mod->test_result = FTM_TEST_FAIL;
    }


    return 0;
}

int emmc_init(void)
{
    int ret = 0;
	int index = 0;
    struct ftm_module *mod;
    struct emmc *mmc = NULL;
	 
    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_EMMC, sizeof(struct emmc));
	if (!mod)
        return -ENOMEM;
    mmc  = mod_to_emmc(mod);

    mmc->mod      = mod;
    mmc->id       = CUST_EMMC_ID;
    mmc->avail    = false;

	emmc_update_info(mmc, mmc->info); 

    ret = ftm_register(mod, (ftm_entry_fn)emmc_entry, (void*)mmc);

    return ret;
}

#endif

/*nfy add for clear emmc*/
#ifdef FEATURE_FTM_CLEAREMMC
int ensure_root_path_unmounted(const char *root_path)
{
    /* See if this root is already mounted. */
    int ret = scan_mounted_volumes();
    
    if (ret < 0) 
    {
        return ret;
    }

    const MountedVolume *volume;
    volume = find_mounted_volume_by_mount_point(root_path);

    if (volume == NULL) 
    {
        /* It's not mounted. */
        LOGD(TAG "The path %s is unmounted\n", root_path);
        return 0;
    }

    return unmount_mounted_volume(volume);
}

int format_root_device(const char *root)
{
    /* Don't try to format a mounted device. */
    int ret = ensure_root_path_unmounted(root);  
    if (ret < 0) 
    {
        LOGD(TAG "format_root_device: can't unmount \"%s\"\n", root);
        return false;
    }
#if 1
        int fd;
        struct msdc_ioctl msdc_io;

        fd = open("/dev/misc-sd", O_RDONLY);
        if (fd < 0) {
            LOGE("open: /dev/misc-sd failed\n");
            return -1;
        }
        
        msdc_io.opcode = MSDC_ERASE_PARTITION;
        if (!strcmp(root, "/cache")) {
            msdc_io.buffer = (unsigned int*) "cache";
            msdc_io.total_size = 6;
        } else if (!strcmp(root, "/data")) {
            msdc_io.buffer = (unsigned int*) "usrdata";
            msdc_io.total_size = 8;
        }

        ioctl(fd, 0, &msdc_io);
        close(fd);
#endif
    if (!strcmp(root, "/data")) {
	    int result = make_ext4fs("/emmc@usrdata", 0, 0, 0);
	    if (result != 0) {	
		LOGE("format_volume: make_extf4fs failed on /emmc@usrdata\n");
	    	return -1;
    	    }
    }	

    return 0;
}

int clear_emmc_entry(struct ftm_param *param, void *priv)
{
    int result = 0;

    ui_printf(uistr_info_emmc_format_data_start);
    result = format_root_device(DATA_PARTITION);
    sync();
    ui_printf(uistr_info_reboot);
    reboot(RB_AUTOBOOT); 

    return result;
}

int clear_emmc_init(void)
{
    int ret = 0;
    int index = 0;
    struct ftm_module *mod;
    struct emmc *mmc = NULL;
	    
    mod = ftm_alloc(ITEM_CLREMMC, sizeof(struct emmc));
    if (!mod)
        return -ENOMEM;
    mmc = mod_to_emmc(mod);
 
    ret = ftm_register(mod, (ftm_entry_fn)clear_emmc_entry, (void*)mmc);
    return ret;
}
#endif
