/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef OS_LINUX_H
#define OS_LINUX_H

typedef long os_time_t;

void os_sleep(os_time_t sec, os_time_t usec);

struct os_time {
    os_time_t sec;
    os_time_t usec;
};


#define os_time_before(a, b) \
    ((a)->sec < (b)->sec || \
     ((a)->sec == (b)->sec && (a)->usec < (b)->usec))

#define os_time_sub(a, b, res) do { \
    (res)->sec = (a)->sec - (b)->sec; \
    (res)->usec = (a)->usec - (b)->usec; \
    if ((res)->usec < 0) { \
        (res)->sec--; \
        (res)->usec += 1000000; \
    } \
} while (0)

int os_get_time(struct os_time *t);
int os_mktime(int year, int month, int day, int hour, int min, int sec,
          os_time_t *t);
int os_daemonize(const char *pid_file);
void os_daemonize_terminate(const char *pid_file);
int os_get_random(unsigned char *buf, size_t len);
unsigned long os_random(void);
char * os_rel2abs_path(const char *rel_path);
int os_program_init(void);
void os_program_deinit(void);
int os_setenv(const char *name, const char *value, int overwrite);
int os_unsetenv(const char *name);
char * os_readfile(const char *name, size_t *len);
void * os_zalloc(size_t size);
void * os_malloc(size_t size);
void * os_realloc(void *ptr, size_t size);
void os_free(void *ptr);
void * os_memcpy(void *dest, const void *src, size_t n);
void * os_memmove(void *dest, const void *src, size_t n);
void * os_memset(void *s, int c, size_t n);
int os_memcmp(const void *s1, const void *s2, size_t n);
char * os_strdup(const char *s);
size_t os_strlen(const char *s);
int os_strcasecmp(const char *s1, const char *s2);
int os_strncasecmp(const char *s1, const char *s2, size_t n);
char * os_strchr(const char *s, int c);
char * os_strrchr(const char *s, int c);
int os_strcmp(const char *s1, const char *s2);
int os_strncmp(const char *s1, const char *s2, size_t n);
char * os_strncpy(char *dest, const char *src, size_t n);
size_t os_strlcpy(char *dest, const char *src, size_t siz);

char * os_strstr(const char *haystack, const char *needle);
int os_snprintf(char *str, size_t size, const char *format, ...);
void os_sleep(os_time_t sec, os_time_t usec);
int os_get_time(struct os_time *t);
int os_mktime(int year, int month, int day, int hour, int min, int sec,
          os_time_t *t);
#define os_daemon daemon
int os_daemonize(const char *pid_file);
void os_daemonize_terminate(const char *pid_file);
int os_get_random(unsigned char *buf, size_t len);
unsigned long os_random(void);
char * os_rel2abs_path(const char *rel_path);
int os_program_init(void);
void os_program_deinit(void);
int os_setenv(const char *name, const char *value, int overwrite);
int os_unsetenv(const char *name);
char * os_readfile(const char *name, size_t *len);
void * os_zalloc(size_t size);
size_t os_strlcpy(char *dest, const char *src, size_t siz);

#endif

