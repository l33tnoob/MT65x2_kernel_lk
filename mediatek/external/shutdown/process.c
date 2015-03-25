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
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include <signal.h>

#define LOG_TAG "shutdownKiller"
#include <cutils/log.h>

int read_symlink(const char *path, char *link, size_t max) {
    struct stat s;
    int length;

    if (lstat(path, &s) < 0)
        return 0;
    if ((s.st_mode & S_IFMT) != S_IFLNK)
        return 0;
   
    length = readlink(path, link, max- 1);
    if (length <= 0) 
        return 0;
    link[length] = 0;
    return 1;
}

int path_is_mountpoint(const char* path, const char* mountPoint) {
    int length = strlen(mountPoint);
    if (length > 1 && strncmp(path, mountPoint, length) == 0) {
        if (mountPoint[length - 1] == '/')
            return 1;
        return (path[length] == 0 || path[length] == '/');
    }
    
    return 0;
}

void get_procname(int pid, char *buffer, size_t max) {
    int fd;
    snprintf(buffer, max, "/proc/%d/cmdline", pid);
    fd = open(buffer, O_RDONLY);
    if (fd < 0) {
        strcpy(buffer, "???");
    } else {
        int length = read(fd, buffer, max - 1);
        buffer[length] = 0;
        close(fd);
    }
}

int is_dir_symlink(int pid, const char *mountPoint, 
    char *openFilename, size_t max) {

    char    path[PATH_MAX];
    DIR *dir;

    int parent_length;
    struct dirent* de;
    char link[PATH_MAX];

    sprintf(path, "/proc/%d/fd", pid);
    dir = opendir(path);
    if (!dir)
        return 0;

    parent_length = strlen(path);
    path[parent_length++] = '/';

    while ((de = readdir(dir))) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")
                || strlen(de->d_name) + parent_length + 1 >= PATH_MAX)
            continue;
        
        path[parent_length] = 0;
        strcat(path, de->d_name);

        if (read_symlink(path, link, sizeof(link)) && path_is_mountpoint(link, mountPoint)) {
            if (openFilename) {
                memset(openFilename, 0, max);
                strncpy(openFilename, link, max-1);
            }
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

int is_maps(int pid, const char *mountPoint, char *openFilename, size_t max) {
    FILE *file;
    char buffer[PATH_MAX + 100];

    sprintf(buffer, "/proc/%d/maps", pid);
    file = fopen(buffer, "r");
    if (!file)
        return 0;
    
    while (fgets(buffer, sizeof(buffer), file)) {
        const char* path = strchr(buffer, '/');
        if (path && path_is_mountpoint(path, mountPoint)) {
            if (openFilename) {
                memset(openFilename, 0, max);
                strncpy(openFilename, path, max-1);
            }
            fclose(file);
            return 1;
        }
    }
    
    fclose(file);
    return 0;
}

int is_symlink(int pid, const char *mountPoint, const char *name) {
    char    path[PATH_MAX];
    char    link[PATH_MAX];

    sprintf(path, "/proc/%d/%s", pid, name);
    if (read_symlink(path, link, sizeof(link)) && path_is_mountpoint(link, mountPoint)) 
        return 1;
    return 0;
}

int get_pid(const char *s) {
    int result = 0;
    while (*s) {
        if (!isdigit(*s)) return -1;
        result = 10 * result + (*s++ - '0');
    }
    return result;
}

void kill_process(const char *path) {
    DIR*    dir;
    struct dirent* dent;
    char name[PATH_MAX];
    char openfile[PATH_MAX];

    int killed;
    int pid;

    if (!(dir = opendir("/proc"))) {
        SLOGE("opendir fails, errno=(%s)", strerror(errno));
        return;
    }

    while ((dent = readdir(dir))) {
        killed = 0;
        pid = get_pid(dent->d_name);

        if (pid == -1)
            continue;
        get_procname(pid, name, sizeof(name));
        if (is_dir_symlink(pid, path, openfile, sizeof(openfile))) {
            SLOGE("Process %s (%d) has openfile %s", name, pid, openfile);
        } else if (is_maps(pid, path, openfile, sizeof(openfile))) {
            SLOGE("Process %s (%d) has open filemap for %s", name, pid, openfile);
        } else if (is_symlink(pid, path, "cwd")) {
            SLOGE("Process %s (%d) has cwd within %s", name, pid, path);
        } else if (is_symlink(pid, path, "root")) {
            SLOGE("Process %s (%d) has chroot within %s", name, pid, path);
        } else if (is_symlink(pid, path, "exe")) {
            SLOGE("Process %s (%d) has executable path within %s", name, pid, path);
        } else {
            continue;
        }
        SLOGE("kill process %d", pid);
        kill(pid, SIGKILL);
    }
    closedir(dir);
}
