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
#include <pthread.h>
#include <fcntl.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "uistrings.h"
#include "utils.h"
#include <DfoDefines.h>
#include <termios.h>
#include "hardware/ccci_intf.h"

#ifdef FEATURE_FTM_SIM

#define TAG    "[SIM] "

static void *sim_update_thread(void *priv);
static void *sim_update_thread_for_dualtalk(void *priv);
int sim_entry(struct ftm_param *param, void *priv);
int sim_init(void);

static int check3GSwitchStatus(const int fd);
static void sendEsuo(const int fd, int value);

static int sim_detect(const int fd, int id);

extern int send_at (const int fd, const char *pCMD);

#define ERROR_NONE           0
#define ERROR_INVALID_FD    -1
#define ERROR_AT_FAIL       -2

#define RET_ESIMS_NO        0
#define RET_ESIMS_YES       1

#define AT_CMD_BUFF_SIZE  128
#define HALT_INTERVAL     20000
#define HALT_TIME         100000
#define BUF_SIZE          256

#if 0
#define AT_RSP_ESIMS    "+ESIMS: "
#define AT_RSP_OK       "OK"

enum {
    ITEM_SIM1,
    ITEM_SIM2,
    ITEM_PASS,
    ITEM_FAIL,
};
#endif

static item_t sim_items[] = {
/*
#ifdef MTK_DT_SUPPORT
    item(ITEM_SIM1, "Modem 1(SIM1)"),
    item(ITEM_SIM2, "Modem 2(SIM2)"),
#else
    item(ITEM_SIM1, uistr_info_detect_sim1),
  #ifdef GEMINI
    item(ITEM_SIM2, uistr_info_detect_sim2),
  #endif
#endif
    item(ITEM_PASS, uistr_info_test_pass),
    item(ITEM_FAIL, uistr_info_test_fail),
*/
    item(-1, NULL),

};

struct sim_factory {
    char info[1024];
    text_t title;
    text_t text;
    struct ftm_module *mod;
    struct itemview *iv;
    pthread_t update_thread;
    bool exit_thread;
    bool test_done;
    int sim_id;
};

#define mod_to_sim(p)  (struct sim_factory*)((char*)(p) + sizeof(struct ftm_module))

#define SIM_ID_1   1 //slot id
#define SIM_ID_2   2 //slot id
#define SIM_ID_3   3 //slot id

typedef enum {
    FALSE = 0,
    TRUE,
} _BOOL;

char dev_node[32] = {0};
/// for dual talk
char dev_node_1[32] = {0};
char dev_node_2[32] = {0};

#define DEVICE_NAME_3   "/dev/ttyUSB1"
#define DEVICE_NAME_EXTRA   "/dev/ttyMT0"
int fd_at = -1;
int fd_atdt = -1;
#define SIM_SWITCH_MODE_CDMA    0x010001
#define SIM_SWITCH_MODE_GSM      0x010000
int fd_ioctlmd = -1;
extern int send_at (const int fd, const char *pCMD);
extern int wait4_ack (const int fd, char *pACK, int timeout);



static int checkESIMSStatus(const char* rsp_buf) {
    const char *tok_esims = "+ESIMS: ";
    const char *tok_eind = "+EIND";
    char *p = NULL;
    char *p_eind = NULL;
    int ret = -1;
      
    p = strstr(rsp_buf, tok_esims);
    p_eind = strstr(rsp_buf, tok_eind);
    if(p) {
        p += strlen(tok_esims);
        if ('1' == *p) {
           ret = RET_ESIMS_YES;
        } else {
           ret = RET_ESIMS_NO;
        }
    } else if(p_eind) {
      	LOGD(TAG "detect +EIND, redo\n");
    }
      
    return ret;
}

static void *sim_update_thread(void *priv)
{
    LOGD(TAG "%s: Start\n", __FUNCTION__);
    struct sim_factory *sim = (struct sim_factory*)priv;
    struct itemview *iv = sim->iv;
    int ret_ioctl_val = -1, fd_mdreset = -1, i = 0;
    int switchMode;
    char cmd_buf[BUF_SIZE] = {0};
    char rsp_buf[BUF_SIZE] = {0};
/// choose fd
    #ifdef EVDO_DT_VIA_SUPPORT
        unsigned char* str_sim1_mode = MTK_TELEPHONY_BOOTUP_MODE_SLOT1;
        unsigned char* str_sim2_mode = MTK_TELEPHONY_BOOTUP_MODE_SLOT2;
        int sim1_mode = atoi(str_sim1_mode);
        int sim2_mode = atoi(str_sim2_mode);        
        LOGD("bootup telephony mode [%d, %d].\n", sim1_mode, sim2_mode);
        char dev_node_ioctrl[32] = {0};
        if (sim1_mode == 0 || sim2_mode == 0) {
            /// use ioctrl to do sim switch
        if (MTK_ENABLE_MD1) {
                snprintf(dev_node_ioctrl, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM_IOCTL,MD_SYS1));
            } else if (MTK_ENABLE_MD2) {
                snprintf(dev_node_ioctrl, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM_IOCTL,MD_SYS2));

            } else {
                LOGD("not open md1 and md2's ioctrl");
            }
        }
        if (MTK_ENABLE_MD1) {
            fd_at= open(dev_node, O_RDWR | O_NONBLOCK);
            if (fd_at < 0) {
                LOGD(TAG "md1 open fd_at error");
                return 0;
            }
        else {
            LOGD(TAG "md1 open fd_at %d",fd_at);
        }

            if (sim1_mode == 0 || sim2_mode == 0) {
                fd_ioctlmd = open(dev_node_ioctrl, O_RDWR | O_NONBLOCK);
        if (fd_ioctlmd < 0) {
            LOGD(TAG "open fd_ioctlmd error");
            return 0;
        }
        else {
            LOGD(TAG "open fd_ioctlmd %d",fd_ioctlmd);
        }
            }
        //for (i = 0; i < 30; i++) usleep(50000); // sleep 1s wait for modem bootup
        } else {
            fd_at= open(dev_node, O_RDWR | O_NONBLOCK);
            if (fd_at < 0) {
                LOGD(TAG "md2 open fd_at error");
                return 0;
            } else {
            LOGD(TAG "md2 open fd_at %d",fd_at);
            }
 
            if (sim1_mode == 0 || sim2_mode == 0) {
                fd_ioctlmd = open(dev_node_ioctrl, O_RDWR | O_NONBLOCK);
            if (fd_ioctlmd < 0) {
                LOGD(TAG "open fd_ioctlmd error");
                return 0;
            } else {
                LOGD(TAG "open fd_ioctlmd %d",fd_ioctlmd);
            }
        }
        }
        /// if bootup mode is W+C or C+G, should do sim switch
        if (sim1_mode == 0 || sim2_mode == 0) {
        ///step1:off modem:AT+EPOF
        do
        {
            send_at (fd_at, "AT\r\n");
        } while (wait4_ack (fd_at, NULL, 300));

        LOGD(TAG "[AT]Enable Sleep Mode:\n");
        if (send_at (fd_at, "AT+ESLP=1\r\n")) return -1;
        if (wait4_ack (fd_at, NULL, 3000)) return -1;

        LOGD(TAG "[AT]Power OFF Modem:\n");
        if (send_at (fd_at, "AT+EFUN=0\r\n")) return -1;
        wait4_ack (fd_at, NULL, 15000);
        if (send_at (fd_at, "AT+EPOF\r\n")) return -1;
        wait4_ack (fd_at, NULL, 10000);
        ///step2:CCCI_IOC_ENTER_DEEP_FLIGHT
        LOGD(TAG "[AT]CCCI_IOC_ENTER_DEEP_FLIGHT \n");
        ret_ioctl_val = ioctl(fd_ioctlmd, CCCI_IOC_ENTER_DEEP_FLIGHT);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_ENTER_DEEP_FLIGHT);

        ///step3:modem switch
        switchMode = SIM_SWITCH_MODE_GSM;
        LOGD(TAG "Begin:switchMode to gsm with index %d", switchMode);
        ret_ioctl_val = ioctl(fd_ioctlmd, CCCI_IOC_SIM_SWITCH, &switchMode);
        if (ret_ioctl_val  == -1) {
            LOGD(TAG "strerror(errno)=%s", strerror(errno));
        }
        ///step4:CCCI_IOC_LEAVE_DEEP_FLIGHT
        LOGD(TAG "[AT]CCCI_IOC_LEAVE_DEEP_FLIGHT \n");
        ret_ioctl_val = ioctl(fd_ioctlmd, CCCI_IOC_LEAVE_DEEP_FLIGHT);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_LEAVE_DEEP_FLIGHT);
        ///wait 50ms close() for
        usleep(50000);

        ///close ttyC0 because of enter/leave flight modem, and  sim switch
        close(fd_at);
        LOGD(TAG "close fd_at %d",fd_at);
            char state_buf[6] = {0};
            /// check md open status
        int ret_mdreset = -1;
            int md_flag = 0;
        fd_mdreset = open("sys/class/BOOT/BOOT/boot/md", O_RDWR);
            LOGD(TAG "ret_mdreset = %d", fd_mdreset);
            LOGD(TAG "open sys/class/BOOT/BOOT/boot/md \n");
            if (fd_mdreset < 0) {
                fd_mdreset = open("sys/kernel/ccci/boot", O_RDWR);			
                LOGD(TAG "open sys/kernel/ccci/boot \n");
                md_flag = 1;
            }
            if (fd_mdreset >= 0 && md_flag == 0) {
        do{
            usleep(500000);
            ret_mdreset = read(fd_mdreset, state_buf, sizeof(state_buf));
                    LOGD(TAG "flag 0's state_buf = %s",state_buf);
        } while (state_buf[0]!= '2');
            } else if (fd_mdreset >= 0 && md_flag == 1) {		
                do{
                    usleep(500000);
                    ret_mdreset = read(fd_mdreset, state_buf, sizeof(state_buf));
                    LOGD(TAG "flag 1's state_buf = %s",state_buf);
                } while (state_buf[4]!= '2');
            } else {
                LOGE (TAG "open md open status file error");
            }
        ///wait a while for modem reset

        //for (i = 0; i < 10; i++) usleep(50000); // sleep 500ms wait for modem bootup

        ///step5: open ttyC0 again for AT cmd
            fd_at= open(dev_node, O_RDWR);
            if (fd_at < 0) {
                LOGD(TAG "open fd_at error");
                return 0;
            } else {
                LOGD(TAG "open fd_at %d",fd_at);
            }
        }
    #else //EVDO_DT_VIA_SUPPORT
        #if defined(PURE_AP_USE_EXTERNAL_MODEM)
            fd_at = open(DEVICE_NAME_3, O_RDWR);
        #else
            fd_at = open(dev_node, O_RDWR);
        #endif
        if(fd_at < 0) {
            LOGD(TAG "Fail to open %s: %s\n", dev_node,strerror(errno));
            return 0;
        }
    #endif  //EVDO_DT_VIA_SUPPORT
    LOGD(TAG "Device has been opened...\n");
    const int rdTimes = 3;
    int rdCount = 0;
    int tobreak = 0;
    int rsp_len = 0;

    struct termios options;
    cfmakeraw(&options);
    // no timeout but request at least one character per read
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN]  = 1;
    tcflush(fd_at, TCIOFLUSH);
    if (tcsetattr(fd_at, TCSANOW, &options) == -1) {
        LOGD(TAG "Fail to set %s attributes!! : %s\n",dev_node, strerror(errno));
    }    

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    usleep(3000000);  //ALPS01194291: sleep 3s to wait device ready
    memset(cmd_buf, 0, sizeof(cmd_buf));
    memset(rsp_buf, 0, sizeof(rsp_buf));
    strcpy(cmd_buf, "ATE0\r\n");
    tcflush(fd_at, TCIOFLUSH);   //ALPS01194291: clear buffer to avoid wrong data
    write(fd_at, cmd_buf, strlen(cmd_buf));
    LOGD(TAG "Send ATE0\n");
    usleep(HALT_TIME * 10);
#endif
    
    while(1) {
        usleep(500000);
        if (sim->exit_thread) {
            LOGD(TAG "Exit thread\n");
            break;
        }
        //memset(sim->info, 0, sizeof(sim->info) / sizeof(*(sim->info)));
        if (!sim->test_done) {
            int ret = -1;
            sim->test_done = true;
            memset(cmd_buf, 0, sizeof(cmd_buf));
            memset(rsp_buf, 0, sizeof(rsp_buf));

            // to detect 3G capability
            int sim_switch_flag = check3GSwitchStatus(fd_at);
            int swtich_to_SIM2 = (((sim->sim_id == SIM_ID_1) && (sim_switch_flag == 2)) ||
                                    ((sim->sim_id == SIM_ID_2) && (sim_switch_flag == 1)));

            int swtich_to_SIM3 = (((sim->sim_id == SIM_ID_1) && (sim_switch_flag == 3)) ||
                                    ((sim->sim_id == SIM_ID_3) && (sim_switch_flag == 1)));

            //SIM1=4, SIM2=5, SIM3=6 
            if(swtich_to_SIM2) {
                // switch UART to SIM2
                sendEsuo(fd_at, 5);
            } else if (swtich_to_SIM3) {
                // switch UART to SIM3
                sendEsuo(fd_at, 6);
            }

                int nread = 0;
                tcflush(fd_at, TCIOFLUSH);   //ALPS01194291: clear buffer to avoid wrong data
                memset(cmd_buf, 0, sizeof(cmd_buf));
                memset(rsp_buf, 0, sizeof(rsp_buf));

                strcpy(cmd_buf, "AT+ESIMS\r\n");
                write(fd_at, cmd_buf, strlen(cmd_buf));
                LOGD(TAG "Send AT+ESIMS\n");
                tobreak = 0;
                while (tobreak == 0) {
                    nread = read(fd_at, rsp_buf, BUF_SIZE);
                    rsp_len = strlen(rsp_buf);
                    LOGD(TAG "------AT+ESIMS(SIM%d) start------\n", sim->sim_id);
                    LOGD(TAG "nread= %d len=%d buf=%s \n", nread, rsp_len,rsp_buf);
                    LOGD(TAG "------AT+ESIMS(SIM%d) end------\n", sim->sim_id);
                    ret = checkESIMSStatus(rsp_buf);
                    if (ret != -1) {
                        tobreak = 1;
                        LOGD(TAG "Got response!------\n");
                        break;
                    }
                }
            
             // switch only if 3G on SIM 1
            if (swtich_to_SIM2 || swtich_to_SIM3) {
                sendEsuo(fd_at, 4);
            }

            if (ret != -1) {
                rdCount = 0;
            } else {
                if (rdCount < rdTimes) {
                    LOGD(TAG "detect unknown response, redo\n");
                    rdCount++;
                    sim->test_done = false;
                    continue;
                }
            }

            if(ret == RET_ESIMS_YES) {
                sprintf(sim->info + strlen(sim->info),
                    "%s%d: %s.\n", uistr_info_detect_sim, sim->sim_id, uistr_info_pass);
                LOGD(TAG "Detect SIM%d: Pass.\n",sim->sim_id);
            } else {
                sprintf(sim->info + strlen(sim->info),
                    "%s%d: %s!!\n", uistr_info_detect_sim, sim->sim_id, uistr_info_fail);
                LOGD(TAG "Detect SIM%d: Fail.\n",sim->sim_id);
            }
            //LOGD(TAG "redraw\n");
            //iv->redraw(iv);
        } // end if(!sim->test_done)
    } // end while(1)
    #ifdef EVDO_DT_VIA_SUPPORT
        ///maybe not need to do. This is Factory mode test, when normal power on, modem will reset again.
	if (sim1_mode == 0 || sim2_mode == 0) {
        close(fd_at);
        fd_at = -1;
        switchMode = SIM_SWITCH_MODE_CDMA;
        LOGD(TAG "End:switchMode to cdma with index %d", switchMode);
        ret_ioctl_val = ioctl(fd_ioctlmd, CCCI_IOC_SIM_SWITCH, &switchMode);
        if (ret_ioctl_val  == -1) {
            LOGD(TAG "strerror(errno)=%s", strerror(errno));
        }
        close(fd_ioctlmd);
        fd_ioctlmd = -1;
        close(fd_mdreset);
        fd_mdreset = -1;
	} else {
        close(fd_at);
        fd_at = -1;
	}
    #else
        close(fd_at);
        fd_at = -1;
    #endif
    LOGD(TAG "%s: Exit\n", __FUNCTION__);

    return NULL;
}
static void *sim_update_thread_for_dualtalk(void *priv) {
    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    struct sim_factory *sim = (struct sim_factory*)priv;
    struct itemview *iv = sim->iv;
    int ret = RET_ESIMS_NO;
    
    int fd1 = -1;
    fd1 = open(dev_node_1, O_RDWR);
    if(fd1 < 0) {
        LOGD(TAG "fail to open %d", fd1);
        return NULL;
    }
    
    int fd2 = -1;
    fd2 = open(dev_node_2, O_RDWR);
    if(fd2 < 0) {
        LOGD(TAG "fail to open %d", fd2);
        return NULL;
    }
    
    LOGD(TAG "dual device has been opened...\n");
    
    while(1) {
        usleep(200000);
        if(sim->exit_thread) {
            LOGD(TAG "exit thread");
            break;
        }

        LOGD(TAG "sim->test_done = %d", sim->test_done);
        if(!sim->test_done) {
            sim->test_done = true;
            if(sim->sim_id == SIM_ID_1) {
                ret = sim_detect(fd1, SIM_ID_1);
            } else if(sim->sim_id == SIM_ID_2) {
                ret = sim_detect(fd2, SIM_ID_2);
            } else {
                LOGD(TAG "invalid test item: %d\n", sim->sim_id);
            }
            
            char *s = NULL;
            if(RET_ESIMS_YES == ret) {
                s = uistr_info_yes;
            } else if(RET_ESIMS_NO == ret) {
                s = uistr_info_no;
            } else {
                s = uistr_info_fail;
            }
            if (RET_ESIMS_YES == ret) {
               sprintf(sim->info + strlen(sim->info),
                       "%s%d: %s.\n", uistr_info_detect_sim, sim->sim_id, uistr_info_pass);
                LOGD (TAG "sim_update_thread:sim->info:%s, lenth:%d",sim->info,strlen(sim->info));
            } else {
               sprintf(sim->info + strlen(sim->info),
                       "%s%d: %s!!\n", uistr_info_detect_sim, sim->sim_id, uistr_info_fail);
                LOGD (TAG "sim_update_thread:sim->info:%s, lenth:%d",sim->info,strlen(sim->info));
            }

            //iv->redraw(iv);
        }
    }
    
    close(fd1);
    close(fd2);
    
    LOGD(TAG "%s: End\n", __FUNCTION__);
    return NULL;
}

int sim_entry(struct ftm_param *param, void *priv)
{
    bool exit = false;
    bool evdoDtSupport = false;
    int  passCount = 0;
    struct sim_factory *sim = (struct sim_factory*)priv;
    struct itemview *iv = NULL;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    strcpy(sim->info, "");
    init_text(&sim->title, param->name, COLOR_YELLOW);
    init_text(&sim->text, &sim->info[0], COLOR_YELLOW);

    if(NULL == sim->iv) {
        iv = ui_new_itemview();
        if(!iv) {
            LOGD(TAG "No memory for item view");
            return -1;
        }
        sim->iv = iv;
    }
    iv = sim->iv;
    iv->set_title(iv, &sim->title);
    iv->set_items(iv, sim_items, 0);
    iv->set_text(iv, &sim->text);
    iv->start_menu(iv,0);

    iv->redraw(iv);

    sim->exit_thread = false;

#ifdef EVDO_DT_VIA_SUPPORT
    evdoDtSupport = true;
#endif

    if(MTK_DT_SUPPORT && !evdoDtSupport) {
        snprintf(dev_node_1, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM, MD_SYS1));    
        snprintf(dev_node_2, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM, MD_SYS2));
        pthread_create(&sim->update_thread, NULL, sim_update_thread_for_dualtalk, priv);
    } else {
        if (MTK_ENABLE_MD1) {
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM, MD_SYS1));
        } else if (MTK_ENABLE_MD2) {        
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM, MD_SYS2));
        } else if (MTK_ENABLE_MD5){
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_SIM, MD_SYS5));    
        } else {
            LOGD("not open md1,md2,md5");
        }
        pthread_create(&sim->update_thread, NULL, sim_update_thread, priv);
    }
#if 0
    while(!exit) {
    int chosen = iv->run(iv, &exit);
    switch(chosen) {
    case ITEM_SIM1:
    sim->sim_id = SIM_ID_1;
    sim->test_done = false;
    exit = false;
    break;

    case ITEM_SIM2:
    sim->sim_id = SIM_ID_2;
    sim->test_done = false;
    exit = false;
    break;

    case ITEM_PASS:
    case ITEM_FAIL:
    if(ITEM_PASS == chosen) {
      sim->mod->test_result = FTM_TEST_PASS;
    } else {
      sim->mod->test_result = FTM_TEST_FAIL;
    }

    sim->exit_thread = true;
    sim->test_done = true;
    exit = true;
    break;

    default:
    sim->exit_thread = true;
    sim->test_done = true;
    exit = true;
    LOGD(TAG "DEFAULT EXIT\n");
    break;
    } // end switch(chosen)
    if(exit) {
    sim->exit_thread = true;
    }
    } // end while(!exit)
#endif

    //Detect SIM 1
    //  strcpy(sim->info, "");
    memset(sim->info, 0, sizeof(sim->info) / sizeof(*(sim->info)));
    sim->sim_id = SIM_ID_1;
    sim->test_done = false;
    while (strlen(sim->info) == 0) {
        LOGD (TAG "detect slot 1:enter");
        LOGD (TAG "sim_entry:sim->info:%s, lenth:%d",sim->info,strlen(sim->info));
        usleep(200000);
        if (strstr(sim->info, uistr_info_pass)) {
            passCount++;
        }
    }
    LOGD(TAG "[SLOT 1]passCount = %d\n", passCount);
    LOGD (TAG "begin redraw");
    iv->redraw(iv);
    LOGD (TAG "end redraw");

    #if defined(GEMINI) || defined(MTK_GEMINI_3SIM_SUPPORT)|| defined(EVDO_DT_VIA_SUPPORT)
        //Detect SIM 2
        //  strcpy(sim->info, "");
        memset(sim->info, 0, sizeof(sim->info) / sizeof(*(sim->info)));
        sim->sim_id = SIM_ID_2;
        sim->test_done = false;
        while (strlen(sim->info) == 0) {
            LOGD (TAG "detect slot 2:enter");
            LOGD (TAG "sim_entry:sim->info:%s, lenth:%d",sim->info,strlen(sim->info));
            usleep(200000);
            if (strstr(sim->info, uistr_info_pass)) {
               passCount++;
            }
        }
        LOGD(TAG "[SLOT 2]passCount = %d\n", passCount);
        LOGD (TAG "begin redraw");
        iv->redraw(iv);
        LOGD (TAG "end redraw");
    #else
        passCount++;
        LOGD(TAG "GEMINI is not defined, do not need to check SIM2\n");
    #endif

    #if defined(MTK_GEMINI_3SIM_SUPPORT)
        //Detect SIM 3
        //  strcpy(sim->info, "");
        memset(sim->info, 0, sizeof(sim->info) / sizeof(*(sim->info)));
        sim->sim_id = SIM_ID_3;
        sim->test_done = false;
        while (strlen(sim->info) == 0) {
            LOGD (TAG "detect slot 3:enter");
            LOGD (TAG "sim_entry:sim->info:%s, lenth:%d",sim->info,strlen(sim->info));
            usleep(200000);
            if (strstr(sim->info, uistr_info_pass)) {
               passCount++;
            }
        }
        LOGD(TAG "[SLOT 3]passCount = %d\n", passCount);
        LOGD (TAG "begin redraw");
        iv->redraw(iv);
        LOGD (TAG "end redraw"); 
    #else
        passCount++;
        LOGD(TAG "MTK_GEMINI_3SIM_SUPPORT is not defined, do not need to check SIM3\n");
    #endif

    //Exit SIM detect thread
    sim->exit_thread = true;
    sim->test_done = true;

    pthread_join(sim->update_thread, NULL);

    //Check test result
    if (passCount == 3) {
        //SIM1, SIM2 and SIM3 are detected.
        sim->mod->test_result = FTM_TEST_PASS;
    } else {
        sim->mod->test_result = FTM_TEST_FAIL;
    }

    LOGD(TAG "%s: End\n", __FUNCTION__);

    return 0;
}

int sim_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct sim_factory *sim;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_SIM, sizeof(struct sim_factory));
    if(!mod) {
        return -ENOMEM;
    }
    sim = mod_to_sim(mod);
    sim->mod = mod;
    sim->test_done = true;

    ret = ftm_register(mod, sim_entry, (void*)sim);
    if(ret) {
        LOGD(TAG "register sim_entry failed (%d)\n", ret);
    }

    return ret;
}

static int check3GSwitchStatus(const int fd) {
    // to detect 3G capability
    char cmd_buf[BUF_SIZE] = {0};
    char rsp_buf[BUF_SIZE] = {0};
    int sim_switch_flag = 1; // 1 -> 3G on SIM1, 2 -> 3G on SIM2, 3 -> 3G on SIM3
    
    tcflush(fd, TCIOFLUSH);   //ALPS01194291: clear buffer to avoid wrong data
    strcpy(cmd_buf, "AT+ES3G?\r\n");
    write(fd, cmd_buf, strlen(cmd_buf));
    usleep(HALT_TIME);
    LOGD(TAG "Send AT+ES3G?\n");
    read(fd, cmd_buf, BUF_SIZE);
    LOGD(TAG "3G Capability: %s\n", cmd_buf);

    const char *TOK_ES3G = "+ES3G: ";
    char *p_es3g = NULL;
    p_es3g = strstr(cmd_buf, TOK_ES3G);
    if(p_es3g) {
        p_es3g += strlen(TOK_ES3G);
        if('2' == *p_es3g) {
            sim_switch_flag = 2;
        } else if ('4' == *p_es3g) {
            sim_switch_flag = 3;
        }
        LOGD(TAG "3G capability is on SIM %d\n", (sim_switch_flag));
    } else {
        LOGD(TAG "No response for AT+ES3G?");
    }

    return sim_switch_flag;
}

static void sendEsuo(const int fd, int value) {
    char cmd_buf[BUF_SIZE] = {0};
    char rsp_buf[BUF_SIZE] = {0};
    sprintf(cmd_buf, "AT+ESUO=%d\r\n", value);
    write(fd, cmd_buf, strlen(cmd_buf));
    LOGD(TAG "Send AT+ESUO=%d\n", value);
    usleep(HALT_TIME);
    read(fd, rsp_buf, BUF_SIZE);
    LOGD(TAG "%s\n", rsp_buf);
}

static int sim_detect(const int fd, int id) {
    LOGD(TAG "%s start\n", __FUNCTION__);

    if (fd < 0) {
        LOGD(TAG "invalid fd\n");
        return ERROR_INVALID_FD;
    }

    char cmd_buf[BUF_SIZE] = {0};
    char rsp_buf[BUF_SIZE] = {0};
    int ret = 0;
    int i, j;

    LOGD(TAG "***SIM id = %d\n", id);
    LOGD(TAG "[AT] detect sim status\n");

    int sim_switch_flag = check3GSwitchStatus(fd);
    int shouldSendEsuo = (((id == SIM_ID_1) && (sim_switch_flag == 1)) ||
                      ((id == SIM_ID_2) && (sim_switch_flag == 0)));

    for (j = 0; j < 2; j++) {
        //if (shouldSendEsuo) {
        if (j == 0) {
            sendEsuo(fd, 5);
        }

        strcpy(cmd_buf, "AT+ESIMS\r\n");
        for (i = 0; i < 5; i++) {
            write(fd, cmd_buf, strlen(cmd_buf));
            LOGD(TAG "Send AT+ESIMS\n");
            usleep(HALT_TIME);
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------AT+ESIMS result, start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+ESIMS result, end------\n");
            ret = checkESIMSStatus(rsp_buf);
            if (ret != 0) break;
        }

        //if (shouldSendEsuo) {
        if (j == 0) {
            sendEsuo(fd, 4);
        }

        if (ret != 0) break;
    }

    LOGD(TAG "%s end\n", __FUNCTION__);

    return ret;
}

#endif
