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
#include <sys/time.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "mounts.h"
#include "mtdutils.h"

#define TAG                 "[NAND] "
#define DATA_PARTITION      "/data"

#define CMD_LINE_PATH       "/proc/driver/nand"
#define MAX_CMD_LINE_LEN    1024
#define NAND_MANF_ID        "nand_manf_id"
#define NAND_DEV_ID         "nand_dev_id"

unsigned int pattern[] = {
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA,
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F,
    0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA, 
    0x5A5A5A5A, 0xA5A5A5A5, 0xF0F0F0F0, 0x0F0F0F0F
};

#define KB                  1024
#define TEST_NUM            (KB >> 1)
#define TEST_SIZE           sizeof(pattern)/sizeof(unsigned int)

enum
{
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t flash_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

static item_t clear_flash_items[] = {
    item(-1, NULL),
};

int rc;

struct nand_info 
{
    int     id;
    char    info[1024];
    char    nand_maf_id[128];
    char    nand_dev_id[128];
    char    test_file_path[512];
    bool    base_rw_test_result;
    bool    erase_result;
    bool    exit_thd;
    bool    exit_clr_thd;

    unsigned long teststart;
    unsigned long testend;

    text_t    title;
    text_t    text;
    //text_t    left_btn;
    //text_t    center_btn;
    //text_t    right_btn;
    
    pthread_t update_thd;
    //pthread_t clr_update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};

#define mod_to_flash(p)     (struct nand_info*)((char*)(p) + sizeof(struct ftm_module))

#ifdef FEATURE_FTM_FLASH

static int check_root_path_mounted(const char* root_path)
{   
    const MountedVolume *volume;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    /* scan the volumes already mounted */
    int ret = scan_mounted_volumes();
    
    if (ret < 0) 
    {
        return ret;
    }

    /* Find if the path is already mounted */
    volume = find_mounted_volume_by_mount_point(root_path);
    
    if (volume == NULL) 
    {
        /* It's not mounted. */
        return -1;
    }    

    return 0;
}

static bool flash_basic_rw(void *priv, const char *root_path)
{
    struct nand_info *flash = (struct nand_info *)priv;
    bool result = false;
    unsigned int *ptr;
    unsigned int i, j;    

    unsigned int *buffer = NULL;
   
    buffer = malloc(sizeof(pattern)*TEST_NUM);

    LOGD(TAG "%s: Start\n", __FUNCTION__);    

    int fd,size;

    int len = sprintf(flash->test_file_path, "%s/%s", root_path, "test");

    LOGD(TAG "the test file path is %s\n", flash->test_file_path);
    
    fd = open(flash->test_file_path, O_WRONLY|O_CREAT|O_SYNC, NULL);

    if (-1 == fd)
    {
        LOGD(TAG "Open or create file named \"temp.log\" failed.\n");
        free(buffer);
        return false;
    }

    for(i=0; i< TEST_NUM; i++)
        write(fd, pattern, sizeof(pattern));

    close(fd);
    
    fd = open(flash->test_file_path, O_RDONLY, NULL);
    
    if (-1 == fd)
    {
        LOGD(TAG "Open file named \"temp.log\" failed.\n");
        free(buffer);
        return false;
    }
    
    size = read(fd, buffer, sizeof(pattern)*TEST_NUM);

    if(size!=sizeof(pattern)*TEST_NUM)
        LOGD(TAG "%d read counts is error %d\n", size, sizeof(pattern)*TEST_NUM);

    for(i=0, ptr=buffer; i< TEST_NUM; i++)
    {
        for(j=0; j< TEST_SIZE; j++)
        {
            if(*ptr!=pattern[j])
            {
                LOGD(TAG "0x%x != 0x%x\n", *ptr, pattern[j]);
                free(buffer);
                return false;
            }
            ptr++;    
        }        
    }

    close(fd); 
    free(buffer);
    result = true;
    
    return result;
}

static bool read_device_info(void *priv, const char *root_path)
{
    struct nand_info *flash = (struct nand_info *)priv;
    bool result = false;
    char *buffer = NULL;
    const char delimiters[] = " ,=";

    char *pstr = NULL;
    char *ptr = NULL;
    int index;
    /* The buffer for read the CMDLINE data */
    buffer = malloc(sizeof(char)*MAX_CMD_LINE_LEN);

    LOGD(TAG "%s: Start\n", __FUNCTION__);    

    int fd, size;    
    
    fd = open(root_path, O_RDONLY, NULL);

    if (-1 == fd)
    {
        LOGD(TAG "Open file named %s failed.\n", root_path);
        return false;
    }

    size = read(fd, buffer, sizeof(char)*MAX_CMD_LINE_LEN);

    LOGD(TAG "CMDLINE: %s\n", buffer);

    /* Parse the CMDLINE to get the manf and dev id */
    memcpy(flash->nand_maf_id,buffer+4,4);
		for(index=0;index<5*5;index++)
		{
			int comparison_value = 10;// 10 mean '\n'
			if(buffer[9+index]==comparison_value)	
			{
				break;
			}
			memcpy(flash->nand_dev_id+index,buffer+9+index,1);
		}
    
/*    ptr = buffer;
    pstr = strtok(ptr, delimiters);

    while(pstr != NULL)
    {
        //if(!strcmp(pstr, NAND_MANF_ID))
        {
            pstr = strtok(NULL, delimiters);
            strcpy(flash->nand_maf_id, pstr);
            LOGD(TAG "flash->nand_maf_id = %s\n", flash->nand_maf_id);
        }
        //if(!strcmp(pstr, NAND_DEV_ID))    
        {
            pstr = strtok(NULL, delimiters);
            strcpy(flash->nand_dev_id, pstr);
            LOGD(TAG "flash->nand_dev_id = %s\n", flash->nand_dev_id);
        }
        pstr = strtok(NULL, delimiters);
    }
*/    
    close(fd);

    result = true;
    
    return result;
}

static void flash_update_info(struct nand_info *flash, char *info)
{
    char *ptr;
    int rc;   

    /* preare text view info */
    ptr = info;
    ptr += sprintf(ptr, "%s\n",flash->title);        
	ptr += sprintf(ptr, uistr_info_nand_Manufacturer_ID);
    ptr += sprintf(ptr, " : %s\n", flash->nand_maf_id);
		ptr += sprintf(ptr, uistr_info_nand_Device_ID);
    ptr += sprintf(ptr, "		: %s\n", flash->nand_dev_id);
	ptr += sprintf(ptr, uistr_info_nand_R_W_tests_result);
    ptr += sprintf(ptr, ": %s\n", flash->base_rw_test_result ? uistr_pass : uistr_fail);
    return;
}

static int flash_key_handler(int key, void *priv) 
{
    int handled = 0, exit = 0;
    struct nand_info *flash = (struct nand_info *)priv;
    struct textview *tv = &flash->tv;
    struct ftm_module *fm = flash->mod;
    
    switch (key)
    {
    case UI_KEY_BACK:
        exit = 1;
        break;
    case UI_KEY_LEFT:
        fm->test_result = FTM_TEST_FAIL;
        exit = 1;
        break;
    case UI_KEY_CONFIRM:
        fm->test_result = FTM_TEST_PASS;
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }
    if (exit)
    {
        LOGD(TAG "%s: Exit thead\n", __FUNCTION__);
        flash->exit_thd = true;
        tv->exit(tv);        
    }
    return handled;
}

static void *flash_update_thread(void *priv)
{
    struct nand_info *flash = (struct nand_info*)priv;
    struct itemview *iv = flash->iv;
    struct statfs stat;
    int count = 1, chkcnt = 5;  

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1)
    {
        usleep(200000);
        chkcnt--;

        if (flash->exit_thd)
            break;

        if (chkcnt > 0)
            continue;        
        /* Prepare the info data to display texts on screen */
        flash_update_info(flash, flash->info);
        
        iv->set_text(iv, &flash->text);
        iv->redraw(iv);
        chkcnt = 5;
    }
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
	return NULL;
}

int flash_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct nand_info *flash = (struct nand_info *)priv;
    //struct textview *tv;
    struct itemview *iv;
    /* for time evaluations */
    struct timeval tv1, tv2;

    /* reset the read/write test result */
    flash->base_rw_test_result = false;
    
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&flash->title, param->name, COLOR_YELLOW);
    init_text(&flash->text, &flash->info[0], COLOR_YELLOW);

    /* Check if the data partition is mounted or not */
    if(!check_root_path_mounted(DATA_PARTITION))
    {
        /* Get the manf id and dev id */
        read_device_info(flash, CMD_LINE_PATH);
    
        gettimeofday(&tv1, NULL);
        flash->teststart = tv1.tv_sec * 1000000 + tv1.tv_usec;

        /* Start the basic read/write tests */
        flash->base_rw_test_result = flash_basic_rw(flash, DATA_PARTITION);
        
        gettimeofday(&tv2, NULL);
        flash->testend = tv2.tv_sec * 1000000 + tv2.tv_usec;

        /* Delete the test temp file */
        remove(flash->test_file_path);
    } else
    {
        LOGD(TAG "%s partition is not mounted !", DATA_PARTITION);
    }
    
    flash_update_info(flash, flash->info);
   
    flash->exit_thd = false;  

    /* Create a itemview */
    if (!flash->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }
        flash->iv = iv;
    }
    
    iv = flash->iv;
    iv->set_title(iv, &flash->title);
    iv->set_items(iv, flash_items, 0);
    iv->set_text(iv, &flash->text);
    iv->redraw(iv);
    usleep(1000000);      
    /* Create a thread to update screen */
    //pthread_create(&flash->update_thd, NULL, flash_update_thread, priv);
    //do
    //{
    //    chosen = iv->run(iv, &exit);
    //    switch (chosen)
    //    {
    //    case ITEM_PASS:
    //    case ITEM_FAIL:
    //    default:
    //        /* report test results */
    //        if (flash->base_rw_test_result == true)
    //        {
    //          flash->mod->test_result = FTM_TEST_PASS;
    //        }
    //        else
    //        {
    //          flash->mod->test_result = FTM_TEST_FAIL;
    //        }                       
    //        exit = true;
    //        break;
    //    }
    //    
    //    if (exit)
    //    {
    //        /* mark for exit update thread */
    //        flash->exit_thd = true;
    //        break;
    //    }        
    //}
    //while (1);
    //pthread_join(flash->update_thd, NULL); 
    if (flash->base_rw_test_result == true)            
    {                                                  
      flash->mod->test_result = FTM_TEST_PASS;         
    }                                                  
    else                                               
    {                                                  
      flash->mod->test_result = FTM_TEST_FAIL;         
    }                                                  
        
    return 0;
}

int flash_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct nand_info *flash;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    /* Alloc memory and register the test module */
    mod = ftm_alloc(ITEM_FLASH, sizeof(struct nand_info));

    if (!mod)
        return -ENOMEM;

    /* set the private data structure pointer to ftm_module's extra space */

    /* ---------------------------------------- *
     * |    ftm_module     |    nand_info     | *
     * ---------------------------------------- */
    flash = mod_to_flash(mod);

    flash->mod = mod;

    /* register the entry function to ftm_module */
    ret = ftm_register(mod, flash_entry, (void*)flash);

    return ret;
}

#endif

#ifdef FEATURE_FTM_CLEARFLASH
#define UBI_CTRL_DEV "/dev/ubi_ctrl"
#define UBI_CTRL_IOC_MAGIC 'o'

/* Attach an MTD device */
#define UBI_IOCATT _IOW(UBI_CTRL_IOC_MAGIC, 64, struct ubi_attach_req)
/* Detach an MTD device */
#define UBI_IOCDET _IOW(UBI_CTRL_IOC_MAGIC, 65, int32_t)

int ubi_detach_dev(int dev)
{
    int ret, ubi_ctrl;

    ubi_ctrl = open(UBI_CTRL_DEV, O_RDONLY);
    if (ubi_ctrl == -1)
    {
        LOGD(TAG "open file %s fail\n", UBI_CTRL_DEV);
        return -1;
    }

    ret = ioctl(ubi_ctrl, UBI_IOCDET, &dev);
    if (ret != 0)
    {
        LOGD(TAG "ioctl %s fail ret=%d \n", UBI_CTRL_DEV, ret);
    }
    close(ubi_ctrl);
    return ret;
}

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
    /* See if this root is already mounted. */
    const MountedVolume *volume;
    char *filesystem;
    char *device;
    int ret = scan_mounted_volumes();
    if (ret < 0)
    {
        LOGD(TAG "format_root_device: load mount info fail\n", root);
        return false;
    }
    
    volume = find_mounted_volume_by_mount_point(root);
    if (volume == NULL)
    {
        LOGD(TAG "The path %s is unmounted\n", root);
        return 0;
    }
    filesystem = strdup(volume->filesystem);
    device = strdup(volume->device);
    ret = unmount_mounted_volume(volume);
    if (ret < 0) 
    {
        LOGD(TAG "format_root_device: can't unmount \"%s\"\n", root);
        return false;
    }
    LOGD(TAG "format_root_device : unmount %s scucess, type is %s\n", root, filesystem);

    if (!strcmp(filesystem, "yaffs2"))
    {
        LOGD(TAG "format yaffs2 partitions\n", root);
    /* Format the device. */
    mtd_scan_partitions();
    const MtdPartition *partition = NULL;

    if(!strcmp(root, DATA_PARTITION))
    {
        LOGD(TAG "find the partition name is %s", root);
        partition = mtd_find_partition_by_name("userdata");
    }
    
    if (partition == NULL) 
    {
        LOGD(TAG "format_root_device: can't find mtd partition \"%s\"\n", root);
        return false;
    }
    
    MtdWriteContext *write = mtd_write_partition(partition);
    
    if (write == NULL) 
    {
        LOGD(TAG "format_root_device: can't open \"%s\"\n", root);
        return false;
        } else if (mtd_erase_blocks(write, -1) == (off_t) - 1)
    {
        LOGD(TAG "format_root_device: can't erase \"%s\"\n", root);
        mtd_write_close(write);
        return false;
        } else if (mtd_write_close(write))
        {
            LOGD(TAG "format_root_device: can't close \"%s\"\n", root);
            return false;
        } else
        {
            ret= true;
        }
    } else
    {
        if (!strcmp(filesystem, "ubifs"))
        {
            int ubi_num = -1, ubi_dev;
            int mtd_num = -1;
            char path[128];
            char mtd_dev_name[128];
            const char *binary_path = "/system/bin/ubiformat";
            const char* skip_questions = "-y";

            sscanf(device, "/dev/ubi%d_0", &ubi_num);
            if (ubi_num >= 0 && ubi_num <= 32)
            {
                LOGD(TAG "format_root_device: detach ubi device /dev/ubi%d_0\n", ubi_num);
            } else
            {
                LOGD(TAG "Can not find parse ubi num :%s\n", device);
                return false;
            }
            sprintf(path, "/sys/class/ubi/ubi%d/mtd_num", ubi_num);
            ubi_dev = open(path, O_RDONLY);
            if (ubi_dev != -1)
            {
                ret = read(ubi_dev, path, sizeof(path));
                close(ubi_dev);
                if (ret > 0)
                {
                    mtd_num = atoi(path);
                    LOGD(TAG "  %s mtd_num is %d \n", device, mtd_num);
                    sprintf(mtd_dev_name, "/dev/mtd/mtd%d", mtd_num);
                } else
                {
                    LOGD(TAG " read %s fail! \n", path);
                    return false;
                }
            } else
            {
                LOGD(TAG " open %s fail! \n", path);
                return false;
            }

            ret = ubi_detach_dev(ubi_num);
            if (ret != 0)
            {
                LOGD(TAG "format_root_device: detach ubi device /dev/ubi%d_0 fail!ret=%d\n", ubi_num, ret);
                return false;
            }
            //int check;
            //check = chmod(binary_path, 0777);
            //printf("chmod = %d\n", check);

            const char **args = (const char **)malloc(sizeof(char *) * 4);
            args[0] = binary_path;
            args[1] = mtd_dev_name;
            args[2] = skip_questions;
            args[3] = NULL;


            pid_t pid = fork();
            if (pid == 0)
            {
                execv(binary_path, (char *const *)args);
                fprintf(stdout, "E:Can't run %s (%s)\n", binary_path, strerror(errno));
                _exit(-1);
    }

            int status;
            waitpid(pid, &status, 0);

            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
                LOGE("Error in ubiformat\n(Status %d)\n", WEXITSTATUS(status));
        return false;
    }

           ret=true;

        } else
    {
            LOGD(TAG "format_root_device : unsupport filesystem %s\n", filesystem);
            return false;
    }    
    
    }
    free(filesystem);
    free(device);    
    return ret;
}

static void clear_flash_update_info(struct nand_info *flash, char *info)
{
    char *ptr;

    /* preare text view info */
#if 0    
    ptr = info;
    ptr += sprintf(ptr, "Format is %s ...\n", flash->erase_result ? "PASS" : "FAIL");
    ptr += sprintf(ptr, "Total test takes %lu secs.\n", (flash->testend - flash->teststart)/1000000);
    ptr += sprintf(ptr, "Rebooting ...\n");
#endif
    ptr = info;
    ptr += sprintf(ptr, uistr_info_nand_erase_info, rc++);
    return;
}

static void *clear_flash_update_thread(void *priv)
{
    struct nand_info *flash = (struct nand_info*)priv;
    struct itemview *iv = flash->iv;
    struct textview *tv = &flash->tv;
    struct statfs stat;
    int count = 1, chkcnt = 5;  

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    init_text(&flash->title, uistr_info_nand_clear_flash, COLOR_YELLOW);
    
    while (1)
    {
        usleep(200000);
        chkcnt--;

        if (flash->exit_clr_thd)
            break;

        if (chkcnt > 0)
            continue;        

        clear_flash_update_info(flash, flash->info);
#if 0        
        iv->set_title(iv, &flash->title);
        iv->set_text(iv, &flash->text);
        iv->redraw(iv);
#endif
        tv->set_title(tv, &flash->title);
        tv->set_text(tv, &flash->text);
        tv->redraw(tv);
        chkcnt = 5;
    }
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
	return NULL;
}

int clear_flash_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct nand_info *flash = (struct nand_info *)priv;
    struct textview *tv = &flash->tv;
    struct itemview *iv;
    struct timeval tv1, tv2;

    flash->erase_result = false;
    
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&flash->title, param->name, COLOR_YELLOW);
    init_text(&flash->text, &flash->info[0], COLOR_YELLOW);

    clear_flash_update_info(flash, flash->info);

    flash->exit_clr_thd = false;  

#if 0
    if (!flash->iv) 
    {
        iv = ui_new_itemview();
        if (!iv) 
        {
            LOGD(TAG "No memory");
            return -1;
        }
        flash->iv = iv;
    }
#endif
    ui_init_textview(tv, flash_key_handler, (void*)flash);
#if 0    
    iv = flash->iv;
    iv->set_title(iv, &flash->title);
    iv->set_items(iv, clear_flash_items, 0);
    iv->set_text(iv, &flash->text);
#endif
    tv->set_title(tv, &flash->title);
    tv->set_text(tv, &flash->text);
    
    pthread_create(&flash->update_thd, NULL, clear_flash_update_thread, priv);
#if 0    
    do
    {
        chosen = iv->run(iv, &exit);
        switch (chosen)
        {
        case ITEM_PASS:
        case ITEM_FAIL:
              if (chosen == ITEM_PASS)
              {
                flash->mod->test_result = FTM_TEST_PASS;
              } else if (chosen == ITEM_FAIL)
              {
                flash->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;
            break;
        }
        
        if (exit)
        {
            flash->exit_clr_thd = true;
            break;
        }        
    }
    while (1);
#endif
    LOGD(TAG "Start the NAND flash erase operations !\n");

    gettimeofday(&tv1, NULL);
    flash->teststart = tv1.tv_sec * 1000000 + tv1.tv_usec;
    
    flash->erase_result = format_root_device(DATA_PARTITION);
    
    gettimeofday(&tv2, NULL);
    flash->testend = tv2.tv_sec * 1000000 + tv2.tv_usec;
    
    LOGD(TAG "Finish the NAND flash erase operations !\n");
    
    flash->exit_clr_thd = true;
    
    pthread_join(flash->update_thd, NULL); 

    sync();
    reboot(RB_AUTOBOOT); 

    return 0;
}

int clear_flash_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct nand_info *flash;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_CLRFLASH, sizeof(struct nand_info));

    if (!mod)
        return -ENOMEM;

    flash = mod_to_flash(mod);

    flash->mod = mod;

    ret = ftm_register(mod, clear_flash_entry, (void*)flash);

    return ret;
}
#endif
