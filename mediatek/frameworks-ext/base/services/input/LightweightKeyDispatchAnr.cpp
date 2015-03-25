/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "LightweightKeyDispatchAnr"

#include <utils/Log.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <cutils/sched_policy.h>
#include <pthread.h>

#include <androidfw/LightweightKeyDispatchAnr.h>

#define MONITOR_TIMER_KEY_DISPATCH_ANR (5)

namespace android {

LightweightKeyDispatchAnr::LightweightKeyDispatchAnr() {

}

LightweightKeyDispatchAnr::~LightweightKeyDispatchAnr() {

}

void LightweightKeyDispatchAnr::createTracesfiles(int pid) {
#if 0	
    DIR* test = NULL;
    int dirExist = 0;
    FILE* f = NULL;

    dirExist = 0;
    test = opendir("/data/anr");
    if(test!=NULL) {
        closedir(test);
        dirExist = 1;
    } else {
        if(errno == ENOENT) {
            LOGW("/data/anr not existed,will create it now\n");
            if(mkdir("/data/anr",0775)==-1) dirExist = 0;
            else dirExist = 1;
        }
    }
    if(dirExist) {
        if((f = fopen ("/data/anr/traces.txt", "w+"))!=NULL) {
            fclose(f);
            if(pid!=-1) {
                LOGW("dump pid:%d backtrace to traces.txt\n",pid);
                kill(pid,16);

                // get all java process pid list and send sig 3
                //dump_java_backtrace();
            }
        } else {
            LOGE("can not create /data/anr/traces.txt\n");
        }
    } else {
        LOGE("/data/anr not existed and can not create this folder\n");
    }
#endif

    if (pid != -1) {
        ALOGW("dump pid:%d backtrace to traces.txt\n", pid);
        kill(pid, 16);

        // get all java process pid list and send sig 3
        //dump_java_backtrace();
    }

    return;
}

bool LightweightKeyDispatchAnr::createMonitorThread() {
    pthread_t hTh = 0;
    pthread_attr_t threadAttr;
    struct sched_param param;
    DIR* test = NULL;

    createTracesfiles(-1);
    test = opendir("/data/anr/jbt");
    if (test != NULL) {
        closedir(test);
    } else {
        if (errno == ENOENT) {
            ALOGW("/data/jbt not existed,will create it now\n");
            if (mkdir("/data/anr/jbt", 0777) == -1)
                ALOGE("create /data/anr/jbt fail\n");
        }
    }

    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    if (!pthread_create(&hTh, &threadAttr, monitorThread, NULL)) {
        ALOGI("LightweightKeyDispatchAnr:create monitor thread succed\n");
        return true;
    } else {
        ALOGI("LightweightKeyDispatchAnr:create monitor thread failed\n");
        return false;
    }
}

void* LightweightKeyDispatchAnr::monitorThread(void* useless) {
    time_t now;
    struct timespec timeout;
    int retCode;
    struct sched_param param;
    DIR* test = NULL;
    int dirExist = 0;
    FILE* f = NULL;

    ALOGI("LightweightKeyDispatchAnr:monitor thread start\n");

    pthread_t thread_id = pthread_self();
    ALOGI("LightweightKeyDispatchAnr:monitorThread tid:%d", gettid());

    param.sched_priority = 99;

    if (pthread_setschedparam(thread_id, SCHED_RR, &param) != 0) {
        ALOGE("LightweightKeyDispatchAnr:failed pthread_setschedparam");
        return NULL;
    }

    if (pthread_mutex_init(&mut_keydispatch_anr_start, NULL) != 0) {
        ALOGE(
                "LightweightKeyDispatchAnr:failed init mut_keydispatch_anr_start\n");
        return NULL;
    }
    if (pthread_cond_init(&cond_keydispatch_anr_start, NULL) != 0) {
        ALOGE(
                "LightweightKeyDispatchAnr:failed init cond_keydispatch_anr_start\n");
        return NULL;
    }

    if (pthread_mutex_init(&mut_keydispatch_anr_timeout, NULL) != 0) {
        ALOGE(
                "LightweightKeyDispatchAnr:failed init mut_keydispatch_anr_timeout\n");
        return NULL;
    }
    if (pthread_cond_init(&cond_keydispatch_anr_timeout, NULL) != 0) {
        ALOGE(
                "LightweightKeyDispatchAnr:failed init cond_keydispatch_anr_timeout\n");
        return NULL;
    }

    while (true) {

        pthread_mutex_lock(&mut_keydispatch_anr_start);
        pthread_cond_wait(&cond_keydispatch_anr_start,
                &mut_keydispatch_anr_start);
        pthread_mutex_unlock(&mut_keydispatch_anr_start);

        time(&now);
        timeout.tv_sec = now + MONITOR_TIMER_KEY_DISPATCH_ANR;
        timeout.tv_nsec = 0;
        retCode = 0;

        pthread_mutex_lock(&mut_keydispatch_anr_timeout);

        retCode = pthread_cond_timedwait(&cond_keydispatch_anr_timeout,
                &mut_keydispatch_anr_timeout, &timeout);

        if (retCode == ETIMEDOUT) {
            ALOGD("LightweightKeyDispatchAnr:timout happened\n");
            createTracesfiles( curPid);
            /// M: 20120724 ALPS00317478 KeyDispatchingTimeout predump Mechanism @{
            if(curPid != getpid())
            {
                createTracesfiles( getpid());
            }
            /// @}
        }
        pthread_mutex_unlock(&mut_keydispatch_anr_timeout);
    }

    return NULL;
}

char * LightweightKeyDispatchAnr::nexttoksep(char **strp, char *sep) {
    char *p = strsep(strp, sep);
    return (p == 0) ? (char *) "" : p;
}

char * LightweightKeyDispatchAnr::nexttok(char **strp) {
    return nexttoksep(strp, " ");
}

int LightweightKeyDispatchAnr::parse_pid(int *dump_pid_array) {
    DIR *d;
    struct dirent *de;
    int index = 0;
    char proc_zygote[64];
    int zygote_pid = 0;
    int pid = 0;
    int ppid = 0;

    d = opendir("/proc");

    if (d == 0)
        return -1;

    while ((de = readdir(d)) != 0) {
        if (isdigit(de->d_name[0])) {
            int pid = atoi(de->d_name);
            ps_parser(pid, proc_zygote, 0);

            ALOGD("%s\r\n", proc_zygote);

            if (!strcmp(proc_zygote, "zygote")) {
                zygote_pid = pid;
                break;
            }

        }
    }

    ALOGD("zygote pid: %d\r\n", zygote_pid);

    while ((de = readdir(d)) != 0) {
        if (isdigit(de->d_name[0])) {
            int pid = atoi(de->d_name);
            ppid = ps_parser(pid, proc_zygote, 1);

            ALOGD("pid: %d, ppid: %d\r\n", pid, ppid);

            if (zygote_pid == ppid) {
                if (index < 64) {
                    dump_pid_array[index] = pid;
                    index++;
                } else
                    break;

            }
        }
    }

    closedir(d);

    return index;

}

int LightweightKeyDispatchAnr::ps_parser(int pid, char *proc_name,
        int ppid_flag) {
    char statline[1024];
    char cmdline[1024];
    int ppid;
    int fd;
    int r = 0;
    char *ptr;
    char *name;

    sprintf(statline, "/proc/%d", pid);

    sprintf(statline, "/proc/%d/stat", pid);

    if (ppid_flag == 0) {
        sprintf(cmdline, "/proc/%d/cmdline", pid);
        fd = open(cmdline, O_RDONLY);
        if (fd == 0) {
            r = 0;
        } else {
            r = read(fd, cmdline, 1023);
            close(fd);
            if (r < 0)
                r = 0;
        }
        cmdline[r] = 0;

        strcpy(proc_name, cmdline);
        goto __exit;

    }

    fd = open(statline, O_RDONLY);

    if (fd == 0)
        return -1;
    r = read(fd, statline, 1023);
    close(fd);
    if (r < 0)
        return -1;
    statline[r] = 0;

    ptr = statline;
    nexttok(&ptr); // skip pid
    ptr++; // skip "("

    name = ptr;
    ptr = strrchr(ptr, ')'); // Skip to *last* occurence of ')',
    *ptr++ = '\0'; // and null-terminate name.

    ptr++; // skip " "
    nexttok(&ptr);
    nexttok(&ptr);
    ppid = atoi(ptr);

    __exit: return ppid;

}

int LightweightKeyDispatchAnr::dump_java_backtrace() {

    int dump_pid_array[64];
    int pid_num = 0;
    int index = 0;
    char new_file_name[64];
    char file_index[8];
    int ret = 0;
#if 0
    int timeout = 0;
#endif

    memset(dump_pid_array, 0, sizeof(dump_pid_array));

    pid_num = parse_pid(dump_pid_array);

    for (index = 0; index < pid_num; index++) {

        memset(new_file_name, 0, sizeof(new_file_name));

        strcpy(new_file_name, "/data/anr/traces_");

        sprintf(file_index, "%d", dump_pid_array[index]);

        strcat(new_file_name, file_index);

        strcat(new_file_name, ".txt");

        kill(dump_pid_array[index], 16);

        //usleep(200*1000);

        ALOGD("[%d] save pid[%d] jbt to data/anr/traces.txt\r\n", index,
                dump_pid_array[index]);

        while (0) {
            // sleep 100*1000us
            usleep(100 * 1000);

            //ret  = rename("/data/anr/traces.txt", new_file_name);

            //system("cat /data/anr/traces.txt >> /data/anr/full_traces.txt");

            if (ret < 0) {
                //LOGD("rename fail %s\r\n", strerror(errno));
            } else {
                ALOGD("[%d] save pid[%d] jbt to data/anr/traces.txt\r\n",
                        index, dump_pid_array[index]);
            }

            break;

        }

    }

    return 0;

}

pthread_mutex_t LightweightKeyDispatchAnr::mut_keydispatch_anr_start =
        PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t LightweightKeyDispatchAnr::cond_keydispatch_anr_start =
        PTHREAD_COND_INITIALIZER;
pthread_mutex_t LightweightKeyDispatchAnr::mut_keydispatch_anr_timeout =
        PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t LightweightKeyDispatchAnr::cond_keydispatch_anr_timeout =
        PTHREAD_COND_INITIALIZER;
int LightweightKeyDispatchAnr::curPid = -1;
bool LightweightKeyDispatchAnr::IS_ENG_BUILD = false;

}
