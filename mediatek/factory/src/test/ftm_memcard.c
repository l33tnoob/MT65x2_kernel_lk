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
#include <errno.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "cust_mcard.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_MEMCARD

#define TAG                 "[MCARD] "

/* should be moved to customized part */
#define MAX_NUM_SDCARDS     (3)
#define MIN_SDCARD_IDX      (0)
#define MAX_SDCARD_IDX      (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)

enum {
#if defined(MTK_EMMC_SUPPORT) && !defined(MTK_SHARED_SDCARD) 
		ITEM_FORMAT_EMMC_FAT,
#endif
    ITEM_PASS,
    ITEM_FAIL,
};

char g_mountpoint1[PATH_MAX] = "/storage/sdcard0";
char g_mountpoint2[PATH_MAX] = "/storage/sdcard1";


static item_t mcard_items[] = {
#if defined(MTK_EMMC_SUPPORT) && !defined(MTK_SHARED_SDCARD) 
    //item(ITEM_FORMAT_EMMC_FAT,    uistr_info_emmc_format_item),
#endif
    //item(ITEM_PASS,   uistr_pass),
    //item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

int g_test_result = 0;

struct mcard {
    int          id;
    char         dev_path[512];
    char         sys_path[512];
    char         info[1024];
    char        *mntpnt;
    bool         mounted;
    bool         avail;
	int 		 blocknum;
    unsigned int checksum;
#ifdef MTK_EMMC_SUPPORT
	const char	*format_stat;
#endif
};
struct mcard_array{
	struct mcard* mcard[2];
	char   info[2048];
	char* ptr_step;
	int mcard_no;
	
	text_t title;
    text_t text;
    bool isFormatting;
    bool exit_thd;
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define mod_to_mcard_array(p)  (struct mcard_array*)((char*)(p) + sizeof(struct ftm_module))

#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)


static unsigned int mcard_mkcksum(const char *name, int maxlen)
{
    unsigned int cksum = 0;
    const char *p = name;

    while (*p != '\0' && maxlen) {
        cksum += *p++;
        maxlen--;
    }
    
    return cksum;
}

static unsigned int mcard_checksum(char *path)
{
    DIR *dp;
    struct dirent *dirp;
    unsigned int cksum = 0;

    if (NULL == (dp = opendir(path)))
        return -1;

    while (NULL != (dirp = readdir(dp))) {
        cksum += mcard_mkcksum(dirp->d_name, 256);
    }

    closedir(dp);

    return cksum;
}

static int mcard_statfs(char *mntpnt, struct statfs *stat)
{
    return statfs(mntpnt, stat);
}

static int mcard_mount(char *devpath, char *mntpnt, bool remount)
{

    int flags, rc;

    flags = MS_NODEV | MS_NOEXEC | MS_NOSUID | MS_DIRSYNC;

    LOGD(TAG "%s, mntpnt=%s, devpath=%s\n", __FUNCTION__, mntpnt, devpath);


    if (remount)
        flags |= MS_REMOUNT;

    /*
     * The mount masks restrict access so that:
     * 1. The 'system' user cannot access the SD card at all - 
     *    (protects system_server from grabbing file references)
     * 2. Group users can RWX
     * 3. Others can only RX
     */
    rc = mount(devpath, mntpnt, "vfat", flags,
               "utf8,uid=1000,gid=1015,fmask=702,dmask=702,shortname=mixed");

    if (rc && errno == EROFS) {
        flags |= MS_RDONLY;
        rc = mount(devpath, mntpnt, "vfat", flags,
            "utf8,uid=1000,gid=1015,fmask=702,dmask=702,shortname=mixed");
    } 

    if (rc) {
		LOGE(TAG "%s: mount fail, %d (%s)\n", __FUNCTION__, errno, strerror(errno));

    }

    return rc;
}

static int mcard_umount(char *mntpnt)
{
	  pid_t pid;
	  int child_stat = 0;

	  LOGD(TAG "%s: mntpnt=%s\n", __FUNCTION__, mntpnt);
	  if ((pid = fork()) < 0) 
	  {
		  LOGE(TAG "%s, fork fails: %d (%s)\n", __FUNCTION__, errno, strerror(errno));
		  return (-1);
	  } 
	  else if (pid == 0)  /*child process*/
	  {
		  int err;
		  err = execl("/system/bin/superumount", "superumount", mntpnt, NULL);
		  exit(-2) ;
	  } 
	  else	/*parent process*/
	  {		 
		  waitpid(pid, &child_stat, 0) ;
		  if (WIFEXITED(child_stat)) {
			  LOGE(TAG "%s: terminated by exit(%d)\n", __FUNCTION__, WEXITSTATUS(child_stat));
			  return WEXITSTATUS(child_stat);
		  }
		  else {
			  LOGE(TAG "%s: execl error, %d (%s)\n", __FUNCTION__, errno, strerror(errno));
			  return -1;
		  }
	  }
	  return -1;
}

static bool mcard_avail(struct mcard *mc)
{
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;

    if (mc->id < MIN_SDCARD_IDX || mc->id > MAX_SDCARD_IDX)
        return false;

    sprintf(name, "mmc%d", mc->id - MIN_SDCARD_IDX);

    ptr  = &mc->sys_path[0];
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
#if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
static void mcard_update_info_multi_storage(struct mcard_array *ma, char *info,int first)
{
    struct statfs stat;
    char *ptr;
    int rc;
	struct mcard *mc_fix = ma->mcard[0];
	struct mcard *mc_removable = ma->mcard[1];
	bool old_avail = mc_removable->avail;
	LOGD("old_avail(%d)\n",old_avail);
	#ifdef MTK_EMMC_SUPPORT
	int fix_host = CUST_EMMC_ID;
	#else
	int fix_host = CUST_MCARD_ID;
	#endif
	int hot_plug_host = CUST_MCARD_ID2;
    g_test_result = 0;
    mc_fix->avail = mcard_avail(mc_fix);
	mc_removable->avail = mcard_avail(mc_removable);
    /* recover previous broken mount */
    if (old_avail && !mc_removable->avail) {
        if (mc_removable->mounted) {
            mcard_umount(mc_removable->mntpnt);
            mc_removable->mounted  = false;
            mc_removable->checksum = -1;
        }
    }
    
    memset(&stat, 0, sizeof(struct statfs));
	LOGD("mc_fix->avail(%d)\n",mc_fix->avail);

    /* mount memory card if available */
    if (mc_fix->avail && !mc_fix->mounted) {
        int fd = -1;
		int len = 0;
		if(mc_fix->id > hot_plug_host && mc_removable->avail){
        	len = sprintf(mc_fix->dev_path, "/dev/block/mmcblk%d", 1);mc_fix->blocknum = 1;}
		else {
			len = sprintf(mc_fix->dev_path, "/dev/block/mmcblk%d", 0);mc_fix->blocknum = 0;}
		#ifdef MTK_EMMC_SUPPORT
			if(mc_fix->id == CUST_EMMC_ID) {				
				sprintf(mc_fix->dev_path, "/emmc@fat");
			}
			else
				strcpy(mc_fix->dev_path + len, "p1");
		#else
			strcpy(mc_fix->dev_path + len, "p1");
		#endif
        if ((fd = open(mc_fix->dev_path, O_RDONLY)) < 0) {
            mc_fix->dev_path[len] = '\0';
            fd = open(mc_fix->dev_path, O_RDONLY);
        }
        if (fd >= 0)
            close(fd);        
        #ifdef MTK_SHARED_SDCARD
		if(mc_fix->id == CUST_EMMC_ID){
				mc_fix->mounted  = true;
            	mc_fix->checksum = mcard_checksum(mc_fix->mntpnt);
			}
		else{
		#endif
        if (0 == (rc = mcard_mount(mc_fix->dev_path, mc_fix->mntpnt, false))) {
            mc_fix->mounted  = true;
            mc_fix->checksum = mcard_checksum(mc_fix->mntpnt);
        }
		#ifdef MTK_SHARED_SDCARD
		}
		#endif
    }

    /* query the information of memory card */
	LOGD("mc_fix->mounted(%d),index(%d)\n",mc_fix->mounted,mc_fix->id);
	LOGD("mc_fix->blocknum(%d)\n",mc_fix->blocknum);
	LOGD("mc_fix->dev_path(%s)\n",mc_fix->dev_path);
	LOGD("mc_fix->mntpnt(%s)\n",mc_fix->mntpnt);
    if (mc_fix->mounted) {
        mcard_statfs(mc_fix->mntpnt, &stat);
    }

    /* prepare info */
    ptr  = info;
	#ifdef MTK_EMMC_SUPPORT
		if(mc_fix->id == fix_host)
			ptr += sprintf(ptr, "%s: \n",uistr_info_emmc_fat);
            #ifndef MTK_SHARED_SDCARD
			ptr += sprintf(ptr, "%s: %s\n",uistr_info_emmc_format_stat, mc_fix->format_stat);
            #endif
	#else
		if(mc_fix->id == fix_host)
			ptr += sprintf(ptr, "%s: \n",uistr_info_sd1);
	#endif
    ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_sd_avail,mc_fix->avail ? uistr_info_emmc_sd_yes : uistr_info_emmc_sd_no);
    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_total_size, 
        (unsigned int)(stat.f_blocks * stat.f_bsize >> 20));
	#ifndef MTK_EMMC_SUPPORT
    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_free_size, 
        (unsigned int)(stat.f_bfree * stat.f_bsize >> 20));
  #endif
    ptr += sprintf(ptr, "%s: 0x%.x\n\n", uistr_info_emmc_sd_checksum,mc_fix->checksum);

	 memset(&stat, 0, sizeof(struct statfs));
	 
	 if(mc_fix->mounted && (access(mc_removable->mntpnt, F_OK) != 0))
				 if(-1 == mkdir(mc_removable->mntpnt,0000)){
					 LOGE("error:%s\n",strerror(errno));
					 goto done;}

    /* mount memory card if available */
	LOGD("mc_removable->avail(%d)\n",mc_removable->avail);
    if (mc_removable->avail && !mc_removable->mounted) {
        int fd = -1;
		int len = 0;
		if((mc_removable->id < mc_fix->id && first) || (mc_removable->blocknum == 0 )){
			len = sprintf(mc_removable->dev_path, "/dev/block/mmcblk%d", 0);mc_removable->blocknum = 0;}
		else{ 
			len = sprintf(mc_removable->dev_path, "/dev/block/mmcblk%d", 1);mc_removable->blocknum = 1;}
			LOGD("mc_removable\n");
		strcpy(mc_removable->dev_path + len, "p1");
        if ((fd = open(mc_removable->dev_path, O_RDONLY)) < 0) {
            mc_removable->dev_path[len] = '\0';
            fd = open(mc_removable->dev_path, O_RDONLY);
        }
        if (fd >= 0)
            close(fd);        
        
        if (0 == (rc = mcard_mount(mc_removable->dev_path, mc_removable->mntpnt, false))) {
            mc_removable->mounted  = true;
            mc_removable->checksum = mcard_checksum(mc_removable->mntpnt);
        }
    }

    /* query the information of memory card */
	LOGD("mc_removable->mounted(%d),index(%d)\n",mc_removable->mounted,mc_removable->id);
	LOGD("mc_removable->blocknum(%d)\n",mc_removable->blocknum);
	LOGD("mc_removable->dev_path(%s)\n",mc_removable->dev_path);
	LOGD("mc_removable->mntpnt(%s)\n",mc_removable->mntpnt);
    if (mc_removable->mounted) {
        mcard_statfs(mc_removable->mntpnt, &stat);
    }

    /* prepare info */
	ptr += sprintf(ptr, "%s: \n",uistr_info_sd2);
	
    ptr += sprintf(ptr, "%s: %s\n",uistr_info_emmc_sd_avail, mc_removable->avail ? uistr_info_emmc_sd_yes : uistr_info_emmc_sd_no);
    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_total_size, 
        (unsigned int)(stat.f_blocks * stat.f_bsize >> 20));
    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_free_size, 
        (unsigned int)(stat.f_bfree * stat.f_bsize >> 20));
    ptr += sprintf(ptr, "%s: 0x%.x\n",uistr_info_emmc_sd_checksum,mc_removable->checksum);
done:
	if(mc_fix->avail && mc_fix->mounted && mc_removable->avail && mc_removable->mounted)
	    g_test_result = 1;
	else
		g_test_result = 0;
    return;
}
#else
static void mcard_update_info(struct mcard *mc, char *info)
{
    struct statfs stat;
    char *ptr;
    int rc;
    bool old_avail = mc->avail;
    
    g_test_result = 0;

    mc->avail = mcard_avail(mc);

    /* recover previous broken mount */
    if (old_avail && !mc->avail) {
        if (mc->mounted) {
            mcard_umount(mc->mntpnt);
            mc->mounted  = false;
            mc->checksum = -1;
        }
    }
    
    memset(&stat, 0, sizeof(struct statfs));

    /* mount memory card if available */
    if (mc->avail && !mc->mounted) {
        int fd = -1;
        int len = sprintf(mc->dev_path, "/dev/block/mmcblk%d", 0);
		#ifdef MTK_EMMC_SUPPORT
			{
		 		sprintf(mc->dev_path, "/emmc@fat");
			}	     	
		#else
			   strcpy(mc->dev_path + len, "p1");
		#endif
        if ((fd = open(mc->dev_path, O_RDONLY)) < 0) {
            mc->dev_path[len] = '\0';
            fd = open(mc->dev_path, O_RDONLY);
        }
        if (fd >= 0)
            close(fd);        
        
        if (0 == (rc = mcard_mount(mc->dev_path, mc->mntpnt, false))) {
            mc->mounted  = true;
            mc->checksum = mcard_checksum(mc->mntpnt);
        }
    }

    /* query the information of memory card */
	LOGD("mc->mounted(%d),index(%d)\n",mc->mounted,index);
	LOGD("rc(%d)\n",rc);
    if (mc->mounted) {
        mcard_statfs(mc->mntpnt, &stat);
    }

    /* prepare info */
    ptr  = info;
	#ifdef MTK_EMMC_SUPPORT
		ptr += sprintf(ptr, "%s: \n",uistr_info_emmc);
	#else
		ptr += sprintf(ptr, "%s: \n",uistr_info_sd);
	#endif
    ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_sd_avail,mc->avail ? uistr_info_emmc_sd_yes : uistr_info_emmc_sd_no);
    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_total_size, 
        (unsigned int)(stat.f_blocks * stat.f_bsize >> 20));
    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_free_size, 
        (unsigned int)(stat.f_bfree * stat.f_bsize >> 20));
    ptr += sprintf(ptr, "%s: 0x%.x\n", uistr_info_emmc_sd_checksum,mc->checksum);
#ifdef MTK_EMMC_SUPPORT
	ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_fat,mc->format_stat);
#endif
	if(mc->avail && mc->mounted)
	    g_test_result = 1;
	else
		g_test_result = 0;	
    return;
}

#endif

static void *mcard_update_iv_thread(void *priv)
{
    struct mcard_array *ma = (struct mcard_array *)priv;
    struct itemview *iv = ma->iv;
    struct statfs stat;
    int count = 1, chkcnt = 10;
	int index = 0;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(100000);
        chkcnt--;

        if (ma->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        if (ma->isFormatting)
            continue;

        chkcnt = 10;
#if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
    	mcard_update_info_multi_storage(ma, ma->info,0);
#else
		mcard_update_info(ma->mcard[0],ma->info);
#endif
        iv->redraw(iv);
        ma->exit_thd = true;
    }
    pthread_exit(NULL);

    LOGD(TAG "%s: Exit\n", __FUNCTION__);

	return NULL;
}

int mcard_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    int index = 0;
    bool exit_val = false;
    struct mcard_array *ma = (struct mcard_array *)priv;
    struct itemview *iv;
    struct statfs stat;
#ifdef MTK_EMMC_SUPPORT
        pid_t pid;
#endif

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&ma->title, param->name, COLOR_YELLOW);
    init_text(&ma->text, &ma->info[0], COLOR_YELLOW);   

#if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
        mcard_update_info_multi_storage(ma, ma->info,0);
#else
        mcard_update_info(ma->mcard[0],ma->info);
#endif

    ma->isFormatting = false;
    ma->exit_thd = false;

    if (!ma->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        ma->iv = iv;
    }
    
    iv = ma->iv;
    iv->set_title(iv, &ma->title);
    iv->set_items(iv, mcard_items, 0);
    iv->set_text(iv, &ma->text);
    iv->redraw(iv);
    
    pthread_create(&ma->update_thd, NULL, mcard_update_iv_thread, priv);
#if 0
    do {
        chosen = iv->run(iv, &exit_val);
        switch (chosen) {
#if defined(MTK_EMMC_SUPPORT) && !defined(MTK_SHARED_SDCARD)
                case ITEM_FORMAT_EMMC_FAT:
                    #if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
                        mcard_update_info_multi_storage(ma,ma->info,0);
                    #else                   
                        mcard_update_info(ma->mcard[0], ma->info);
                    #endif

                    ma->isFormatting = true;
                    if (ma->mcard[0]->mounted) {
                        if (mcard_umount(ma->mcard[0]->mntpnt)) {
                            LOGE(TAG"eMMC: umount %s fails: %d (%s)\n", ma->mcard[0]->mntpnt, errno, strerror(errno));
                            ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_fail;
                            ma->isFormatting = false;
                            break;                 
                        }
                        else {
                           LOGD(TAG"eMMC: umount %s sucessfully \n", ma->mcard[0]->mntpnt);
                           ma->mcard[0]->mounted  = false;
                           ma->mcard[0]->checksum = -1;
                        }
                    }

                    if ((pid = fork()) < 0) 
                    {
                        LOGE(TAG"eMMC: fork fails: %d (%s)\n", errno, strerror(errno));                     
                        ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_fail;
                        ma->isFormatting = false;
                        break;
                    } 
                    else if (pid == 0)  /*child process*/
                    {
                        int err;
                        char fat_partition_path[64];
                        int partition_idx;

                        ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_start;

                
                        sprintf(fat_partition_path, "/emmc@fat");
                        err = execl("/system/bin/newfs_msdos", "newfs_msdos", fat_partition_path, NULL);
                        exit(-3) ;
                    } 
                    else  /*parent process*/
                    {
                        int child_stat ;
                        waitpid(pid, &child_stat, 0) ;
                        sync() ;
                        if (WIFEXITED(child_stat) && (WEXITSTATUS(child_stat) == 0)) {
                            LOGD(TAG"eMMC: pid = %d\n", pid);
                            ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_success;
                        }
                        else {
                            LOGD(TAG"eMMC: execl error: %s\n", strerror(errno));
                            ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_fail;
                        }

#if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
                        mcard_update_info_multi_storage(ma,ma->info,0);
#else                   
                        mcard_update_info(ma->mcard[0], ma->info);
#endif
                    }
                    ma->isFormatting = false;
                    break;
#endif
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                ma->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                ma->mod->test_result = FTM_TEST_FAIL;
            }         

#if defined(MTK_EMMC_SUPPORT)
            ma->mcard[0]->format_stat = NULL;
#endif  
            exit_val = true;
            break;
        }
        
        if (exit_val) {
            ma->exit_thd = true;
            break;
        }        
    } while (1);
#endif
    pthread_join(ma->update_thd, NULL);
	if (g_test_result > 0) {
			ma->mod->test_result = FTM_TEST_PASS;
		}
		else {
			ma->mod->test_result = FTM_TEST_FAIL;
		}

    return 0;
}

#if defined(MTK_EMMC_SUPPORT) && defined(MTK_MULTI_STORAGE_SUPPORT)
void mcard_auto_format_internal_sd(struct mcard_array *ma) {
    struct mcard *mc_fix = ma->mcard[0];
    char first_boot[10];
    int rc;
    pid_t pid;

    #ifdef MTK_SHARED_SDCARD 
      LOGD(TAG "%s: This is MTK_SHARED_SDCARD version, just skip.\n", __FUNCTION__);
      return;
    #endif

    property_get("persist.first_boot", first_boot, "1");
    if (!strcmp(first_boot, "1") && !mc_fix->mounted) {
        LOGD(TAG "%s: *** This is first boot!",__FUNCTION__);
        if (rc = mcard_mount(mc_fix->dev_path, mc_fix->mntpnt, false)) {
            LOGD(TAG "%s: mount fail. Try to format it.\n", __FUNCTION__);
           
            if ((pid = fork()) < 0) 
            {
                LOGE(TAG"eMMC: fork fails: %d (%s)\n", errno, strerror(errno));                     
                ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_fail;
                ma->isFormatting = false;
                return;
            } 
            else if (pid == 0)  /*child process*/
            {
                int err;
                char fat_partition_path[64];
                int partition_idx;

                ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_start;

        
                sprintf(fat_partition_path, "/emmc@fat");
                err = execl("/system/bin/newfs_msdos", "newfs_msdos", fat_partition_path, NULL);
                exit(-3) ;
            } 
            else  /*parent process*/
            {
                int child_stat ;
                waitpid(pid, &child_stat, 0) ;
                sync() ;
                if (WIFEXITED(child_stat) && (WEXITSTATUS(child_stat) == 0)) {
                    LOGD(TAG"Format ok. pid = %d\n", pid);
                    ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_success;
                }
                else {
                    LOGD(TAG"Format fail. execl error: %s\n", strerror(errno));
                    ma->mcard[0]->format_stat = uistr_info_emmc_format_stat_fail;
                }
            }
            ma->isFormatting = false;           
        }
        else {
            LOGD(TAG "%s: mount ok.\n", __FUNCTION__);

            mc_fix->mounted    = true;
            mc_fix->checksum = mcard_checksum(mc_fix->mntpnt);          
        }
        property_set("persist.first_boot", "0");
    }    
}
#endif

int mcard_init(void)
{
    int ret = 0;
    struct ftm_module *mod = NULL;
    struct mcard *mc = NULL;
    struct mcard_array *ma = NULL;
    char* env_var;


    LOGD(TAG "%s\n", __FUNCTION__);
    
    if ( NULL != (env_var = getenv("EXTERNAL_STORAGE"))) {
         strcpy(g_mountpoint1, env_var);
    }
    if ( NULL != (env_var = getenv("SECONDARY_STORAGE"))) {
         strcpy(g_mountpoint2, env_var);
    }
    LOGD(TAG "g_mountpoint1(%s), g_mountpoint2(%s)\n", g_mountpoint1, g_mountpoint2);    

    mod = ftm_alloc(ITEM_MEMCARD, sizeof(struct mcard_array));
	if (!mod)
        return -ENOMEM;
	mc = (struct mcard*)malloc(sizeof(struct mcard));
	if(!mc){
			ftm_free(mod);
			return -ENOMEM;
		}
	ma = (struct mcard_array*)malloc(sizeof(struct mcard_array));
	if(!ma){
			ftm_free(mod);
			free(mc);
			return -ENOMEM;
		}
		
	memset(ma,0,sizeof(struct mcard_array));
    ma  = mod_to_mcard_array(mod);

    ma->mod      = mod;
	#ifdef MTK_EMMC_SUPPORT
    mc->id       = CUST_EMMC_ID;
	#else
	mc->id		 = CUST_MCARD_ID;
	#endif 
    mc->mounted  = false;
    mc->avail    = false;
    mc->mntpnt   = g_mountpoint1;
    mc->checksum = -1;
	mc->blocknum = -1;
	ma->mcard[0] = mc;
	ma->mcard_no += 1;
#if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
	LOGD("MTK_MULTI_STORAGE_SUPPORT\n");
    struct mcard *mc2 = (struct mcard*)malloc(sizeof(struct mcard));;
    mc2->id       = CUST_MCARD_ID2;
    mc2->mounted  = false;
    mc2->avail    = false;
    mc2->mntpnt   = g_mountpoint2;
    mc2->checksum = -1;
	mc2->blocknum = -1;
	ma->mcard[1] = mc2;
	ma->mcard_no += 1; 
#endif
	LOGD(TAG "ma->mcard_no(%d)\n",ma->mcard_no);

#if defined(MTK_MULTI_STORAGE_SUPPORT) && !defined(MTK_FAT_ON_NAND)
			mcard_update_info_multi_storage(ma, ma->info,1);
#else
			mcard_update_info(ma->mcard[0],ma->info);
#endif

#if defined(MTK_EMMC_SUPPORT) && defined(MTK_MULTI_STORAGE_SUPPORT)
        mcard_auto_format_internal_sd(ma);
#endif

    ret = ftm_register(mod, mcard_entry, (void*)ma);
    return ret;
}

#endif
