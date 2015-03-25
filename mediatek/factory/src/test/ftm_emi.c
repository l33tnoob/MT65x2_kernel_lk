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
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
//#include <cutils/pmem.h>
#include <common.h>
#include <miniui.h>
#include <ftm.h>
#include <dlfcn.h>
#include "multimediaFactoryTest.h"
#include "MediaTypes.h"
#if defined(FEATURE_FTM_EMI)

#define TAG "[FTM_EMI] "
#define PAGE_SIZE 4096
enum { ITEM_PASS, ITEM_FAIL };

static item_t emi_items[] = 
{
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

/******** EMI CLK **********/
#define MHz_208    0
#define MHz_266    1
#define MHz_293    2

#define EMI_CLK   MHz_293
/**************************/

/******** CPU CLK **********/
#define MHz_676    12
#define MHz_689    13
#define MHz_702    14
#define MHz_715    15
#define MHz_728    16
#define MHz_741    17
#define MHz_754    18
#define MHz_767    19
#define MHz_780    20
#define MHz_793    21
#define MHz_806    22

#define CPU_CLK   MHz_702
/**************************/

static unsigned int clk_range[] = {208, 266, 293};

struct emi_info 
{
    char    info[1024];
    bool    stress_result;
    bool    exit_thd;
    text_t    title;
    text_t    text;
    pthread_t update_thd;
    pthread_t march_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define STR_BUF_LEN 20

#define mod_to_emi(p)     (struct emi_info*)((char*)(p) + sizeof(struct ftm_module))
#if 0
static void emi_update_info(struct emi_info *emi, char *info)
{
    char *ptr;
    int rc;   

    /* preare text view info */
    ptr = info;
    ptr += sprintf(ptr, "%s: %s\n", uistr_info_stress_test_result, emi->stress_result ? uistr_pass : uistr_fail);
    return;
}
#endif
/*
 * emi_update_thread: status-update thread function.
 * @priv:
 */
static void *emi_update_thread(void *priv)
{
    struct emi_info *emi = (struct emi_info*)priv;
    struct itemview *iv = emi->iv;
    int count = 1, chkcnt = 5;  

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(200000);
        chkcnt--;

        if (emi->exit_thd)
            break;

        if (chkcnt > 0)
            continue;        

        /* Prepare the info data to display texts on screen */
        //emi_update_info(emi, emi->info);
        
        iv->set_text(iv, &emi->text);
        iv->redraw(iv);
        chkcnt = 5;
    }
    pthread_exit(NULL);
    
    return NULL;
}

/*
 * update_screen_thread: screen-update thread function.
 * @priv:
 */
static bool update_screen_exit = false;
static void *update_screen_thread(void *priv)
{
    LOGD(TAG "enter update_screen_thread\n");
    while (!update_screen_exit){
        ui_flip();
    }
    LOGD(TAG "exit update_screen_thread\n");
    pthread_exit(NULL);
    return NULL;
}

/*
 * MBIST_March_MTK_Test: MTK March test.
 * @start: start of the test region.
 * @len: length of the test region.
 * Return error code.
 */
#define TEST_PASS 0
#define TEST_FAIL 1
static bool march_exit = false;
int MBIST_March_MTK_Test(unsigned int start, unsigned int len)
{
    unsigned char pattern8;
    volatile unsigned char *MEM8_BASE = (volatile unsigned char *)start;
    int i;

    //Write background to 0
    for (i = 0; i < (int)len; i++) //W0
    {
        MEM8_BASE[i] = 0;
    }
    //Marh Algorithm
    for (i = 0; i <= (int)(len - 1); i++) //R0, W1, R1
    {
        if (MEM8_BASE[i] == 0) MEM8_BASE[i]=0xFF;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0xFF) return TEST_FAIL;       
    }
    for (i = 0; i <= (int)(len - 1); i++) //R1, W0, R0
    {
        if (MEM8_BASE[i] == 0xFF) MEM8_BASE[i]=0x0;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0) return TEST_FAIL;       
    }
    for (i = (len-1); i >= 0; i--) //R0, W1, R1
    {
        if (MEM8_BASE[i] == 0) MEM8_BASE[i]=0xFF;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0xFF) return TEST_FAIL;      
    }
    for (i = (len-1); i >= 0; i--) //R1, W0, R0
    {
        if (MEM8_BASE[i] == 0xFF) MEM8_BASE[i]=0x0;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0) return TEST_FAIL;     
    }
    for (i = (len - 1); i >= 0; i--) //R0
    {
        if (MEM8_BASE[i] != 0) return TEST_FAIL;
    }

    //Write background to 1
    for (i = 0; i < (int)len; i++) //W1
    {
        MEM8_BASE[i] = 0xFF;
    }
    //Marh Algorithm
    for (i = 0; i <= (int)(len - 1); i++) //R1, W0, R0
    {
        if (MEM8_BASE[i] == 0xFF) MEM8_BASE[i]=0x0;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0) return TEST_FAIL;     
    }
    for (i = 0; i <= (int)(len - 1); i++) //R0, W1, R1
    {
        if (MEM8_BASE[i] == 0) MEM8_BASE[i]=0xFF;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0xFF) return TEST_FAIL;     
    }
    for (i = (len - 1); i >= 0; i--) //R1, W0, R0
    {
        if (MEM8_BASE[i] == 0xFF) MEM8_BASE[i]=0;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0) return TEST_FAIL;      
    }
    for (i = (len - 1); i >= 0; i--) //R0, W1, R1
    {
        if (MEM8_BASE[i] == 0) MEM8_BASE[i]=0xFF;
        else return TEST_FAIL;
        if (MEM8_BASE[i]!=0xFF) return TEST_FAIL;     
    }
    for(i = (len-1); i >= 0; i--) //R1
    {
        if (MEM8_BASE[i] != 0xFF) return TEST_FAIL;
    }

    //dbg_print("MTK MBIST March Test - PASS!\n\r");
    return TEST_PASS;
}

/*
 * mtk_march_thread: MTK March test thread function.
 * @priv:
 */
static void *mtk_march_thread(void *priv)
{
    struct emi_info *emi = (struct emi_info*)priv;
    unsigned int *pmem_va;
    int fd, ret;
    
    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
//        pmem_va = pmem_alloc_sync(0x200000, &fd);
        pmem_va = malloc (0x200000);
        LOGD("EMI PMEM addr = %x\n", (unsigned int)pmem_va);
        if (pmem_va == NULL) {
            LOGE(TAG "allocate PMEM failed\n");
        } else {
            ret = MBIST_March_MTK_Test((unsigned int)pmem_va, 0x200000);
            if(ret != TEST_PASS)
                LOGD("MTK MBIST March Test - FAIL!\n\r");
//            pmem_free(pmem_va, 0x200000, fd);
            free (pmem_va);
        }
        if (march_exit) {
            break;
        }
    }

    pthread_exit(NULL);

    return NULL;
}
#if 0
static int get_3d_test_base(void)
{
    int ret = 0;
    int fd = 0;
    unsigned int base;
    const char *test_3d = "/sys/bus/platform/drivers/emi_clk_test/emi_clk_3d_test";
    char str[STR_BUF_LEN], result[STR_BUF_LEN];
    ssize_t s;
    bzero(str, STR_BUF_LEN);
    bzero(result, STR_BUF_LEN);

    fd = open(test_3d, O_RDWR);
    if  (fd < 0) {
        LOGD("Fail to open: %s. Terminate.\n", test_3d);
    } else { 
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            LOGD("Fail to read %s. Terminate\n", test_3d);
        } else {
            LOGD("%s, %s\n", test_3d, result); //output to screen here...
            base = strtoul(result, (char **) NULL, 10);
            LOGD("In %s: base:%08x\n", __func__, base);
        }
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    return 0;
}
static int set_3d_test(int en)
{
    int ret = 0;
    int fd = 0;
    const char *test_3d = "/sys/bus/platform/drivers/emi_clk_test/emi_clk_mtcmos_test";
    char str[STR_BUF_LEN];
    bzero(str, STR_BUF_LEN);

    fd = open(test_3d, O_RDWR);
    if  (fd < 0) {
        LOGD("Fail to open: %s. Terminate.\n", test_3d);
    } else { 
        LOGD("Able to write %s.\n", test_3d);
        sprintf(str, "%d", en);
        write(fd, str, strlen(str));
        fsync(fd);
        //lseek(fd, 0, SEEK_SET);
        LOGD("In %s: en:%d\n", __func__, en);
        //bzero(str, STR_BUF_LEN);
        close(fd); 
    }
    return 0;
}
#endif
static int dram_overclock(int clk, char *ptr)
{
    const char *dram_overclock = "/sys/bus/platform/drivers/emi_clk_test/emi_clk_test";  
    int i = 0, ret = 1;
    int fd = 0;
    ssize_t s;
    char freq[STR_BUF_LEN], result[STR_BUF_LEN];
    /* do DRAM overclocking */
//for(i = 2; i >= 0; --i) {
//for(i = 0; i <= 2; ++i) {
    fd = open(dram_overclock, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", dram_overclock);
        ret = -1;
    } else { 
        s = read(fd, (void*)result, STR_BUF_LEN);
        if (s <= 0)
        {
            printf("Fail to read %s. Terminate\n", dram_overclock);
            ret = -1;
            goto dram_overclock_end;
        }
        lseek(fd, 0 , SEEK_SET);
        sprintf(freq, "%d", clk);
        //sprintf(freq, "%d", clk_range[EMI_CLK]);
        //sprintf(freq, "%d", clk_range[i]);
        write(fd, freq, strlen(freq)); // Change clk
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", dram_overclock);
            ret = -1;
        } else {
            printf("%s\n", result); //output to screen here...
            ptr += sprintf(ptr, "%s\n", result);
            usleep(100000);
        }
dram_overclock_end:
        close(fd);
    }
//}
    return ret;
}
static int unplug_cpus(void)
{
    int ret = 0;
    int fd = 0;
    const char *perf = "performance";
    const char *scaling = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    const char *g_enable = "/sys/module/mt_hotplug_mechanism/parameters/g_enable";
    const char *cpu1 = "/sys/devices/system/cpu/cpu1/online";
    const char *cpu2 = "/sys/devices/system/cpu/cpu2/online";
    const char *cpu3 = "/sys/devices/system/cpu/cpu3/online";
    char str[STR_BUF_LEN], result[STR_BUF_LEN];
    ssize_t s;
    bzero(str, STR_BUF_LEN);
    bzero(result, STR_BUF_LEN);

    fd = open(scaling, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", scaling);
    } else { 
        sprintf(str, "%s", perf);
        write(fd, str, strlen(str)); // Change scaling_governor
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", scaling);
        } else 
            printf("%s: %s\n", scaling, result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    fd = open(g_enable, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", g_enable);
    } else { 
        sprintf(str, "%d", 0);
        write(fd, str, strlen(str)); // Change g_enable
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", g_enable);
        } else 
            printf("%s: %s\n", g_enable, result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    fd = open(cpu1, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", cpu1);
    } else { 
        sprintf(str, "%d", 0);
        write(fd, str, strlen(str)); // Change cpu1
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", cpu1);
        } else 
            printf("%s, %s\n", cpu1, result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    fd = open(cpu2, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", cpu2);
    } else { 
        sprintf(str, "%d", 0);
        write(fd, str, strlen(str)); // Change cpu2
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", cpu2);
        } else 
            printf("%s, %s\n", cpu2, result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    fd = open(cpu3, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", cpu3);
    } else { 
        sprintf(str, "%d", 0);
        write(fd, str, strlen(str)); // Change cpu3
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", cpu3);
        } else 
            printf("%s, %s\n", cpu3, result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    return 0;
}
static int recover_cpus(void)
{
    int ret = 0;
    int fd = 0;
    const char *perf = "hotplug";
    const char *scaling = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    const char *g_enable = "/sys/module/mt_hotplug_mechanism/parameters/g_enable";
    char str[STR_BUF_LEN], result[STR_BUF_LEN];
    ssize_t s;
    fd = open(scaling, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", scaling);
    } else { 
        sprintf(str, "%s", perf);
        write(fd, str, strlen(str)); // Change scaling_governor
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", scaling);
        } else 
            printf("%s\n", result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    fd = open(g_enable, O_RDWR);
    if  (fd < 0) {
        printf("Fail to open: %s. Terminate.\n", g_enable);
    } else { 
        sprintf(str, "%d", 1);
        write(fd, str, strlen(str)); // Change g_enable
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Terminate\n", g_enable);
        } else 
            printf("%s\n", result); //output to screen here...
        bzero(str, STR_BUF_LEN);
        bzero(result, STR_BUF_LEN);
        close(fd); 
    }
    return 0;
}
/*
 * emi_entry: factory mode entry function.
 * @param:
 * @priv:
 * Return error code.
 */
static int emi_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct emi_info *emi = (struct emi_info *)priv;
    struct itemview *iv;
    pthread_t update_screen;
    update_screen_exit = false;
    const char *cpu_overclock = "/sys/bus/platform/drivers/arm_pwr_test/arm_pwr_test_gui"; 
    int ret;
    int fd = 0;
    ssize_t s;
    char freq[STR_BUF_LEN], result[STR_BUF_LEN];
    
    emi->stress_result = false;
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&emi->title, param->name, COLOR_YELLOW);
    init_text(&emi->text, &emi->info[0], COLOR_YELLOW);

    //emi_update_info(emi, emi->info);
  
    emi->exit_thd = false;  

    /* Create a itemview */
    if (!emi->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        emi->iv = iv;
    }
    
    iv = emi->iv;
    iv->set_title(iv, &emi->title);
    iv->set_items(iv, emi_items, 0);
    iv->set_text(iv, &emi->text);
    
    /* create a thread for the test pattern: frame buffer update update */
    pthread_create(&emi->update_thd, NULL, emi_update_thread, priv); 
    
    /* create a thread for the test pattern: MTK March test */
    pthread_create(&emi->march_thd, NULL, mtk_march_thread, priv);

    /* create a thread for screen update */
    if (pthread_create(&update_screen, NULL, update_screen_thread, NULL)) {
        LOGD(TAG "create update_screen_thread failed\n");
    }

    ptr = emi->info;

    /* do CPU overclocking */
#if 0
    fd = open(cpu_overclock, O_RDWR);
    if (fd < 0) {
        printf("Fail to open: %s. Ignore.\n", cpu_overclock);
    } else { 
        sprintf(freq, "%d", CPU_CLK);
        write(fd, freq, strlen(freq)); // start test
        fsync(fd);

        lseek(fd, 0, SEEK_SET);

        s = read(fd, (void *)result, STR_BUF_LEN); // read back result
        if (s <= 0) {
            printf("Fail to read %s. Ignore.\n", cpu_overclock);
        } else {
            printf("%s\n", result); //output to screen here...
            ptr += sprintf(ptr, "%s\n", result);
            usleep(100000);
        }

        close(fd);
    }
#endif
    
    ret = unplug_cpus();
    if (ret < 0) {
        printf("Fail to unplug_cpus: %d\n", ret);
    }
    ret = dram_overclock(clk_range[EMI_CLK], ptr);
    if (ret < 0) {
        printf("Fail to dram_overclock: %d, in %d\n", ret, __LINE__);
        goto emi_entry_exit;
    }
    /* run multimedia test patterns */
    LOGD(TAG "start to run multimedia test patterns\n");
    ret = mHalFactory(NULL);
    if(ret == 0)
        emi->stress_result = true;
    else
        emi->stress_result = false;
    LOGD(TAG "complete to run multimedia test patterns\n");
    usleep(200000);
    printf("DRAM: clk range = %d\n", clk_range[EMI_CLK]);
    printf("DRAM: stress result = %d\n", emi->stress_result);
    printf("DRAM test all done\n"); //output to screen here...
    ptr += sprintf(ptr, "--> %s: %s\n", uistr_info_stress_test_result, emi->stress_result ? uistr_pass : uistr_fail);      

emi_entry_exit:
    ret = dram_overclock(clk_range[1], ptr);
    if (ret < 0) {
        printf("Fail to dram_overclock: %d, in %d\n", ret, __LINE__);
    }
    recover_cpus();

    /* stop the test pattern: frame buffer update */
    update_screen_exit = true;
    /* stop the test pattern: MTK March test */
    march_exit = true;

    if (fd >= 0) {
        close(fd);
    }

    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            /* report test results */
            if (chosen == ITEM_PASS) {
                emi->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                emi->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;
            break;
        default:
            break;
        }
        
        if (exit) {
            /* stop the screen-update thread */
            emi->exit_thd = true;
            break;
        }
    } while (1);

    pthread_join(emi->update_thd, NULL); 
    pthread_join(emi->march_thd, NULL);
    //update_screen_exit = true;
    pthread_join(update_screen, NULL);

    return 0;
}

/*
 * emi_init: factory mode initialization function.
 * Return error code.
 */
int emi_init(void)
{
    int index;
    int ret = 0;
    struct ftm_module *mod;
    struct emi_info *emi;
    //pid_t p_id;

    LOGD(TAG "%s\n", __FUNCTION__);

    /* Alloc memory and register the test module */
    mod = ftm_alloc(ITEM_EMI, sizeof(struct emi_info));
    if (!mod)
        return -ENOMEM;

    emi = mod_to_emi(mod);
    emi->mod = mod;
  
    /* register the entry function to ftm_module */
    ret = ftm_register(mod, emi_entry, (void*)emi);

    return ret;
}

#endif  /* FEATURE_FTM_EMI */
