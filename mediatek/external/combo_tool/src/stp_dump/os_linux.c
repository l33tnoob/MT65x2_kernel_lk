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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <dirent.h>
#include <cutils/properties.h>
#include <sys/un.h>
#include <dirent.h>
#include "os_linux.h"

void os_sleep(os_time_t sec, os_time_t usec)
{
    if (sec)
        sleep(sec);
    if (usec)
        usleep(usec);
}

int os_get_time(struct os_time *t)
{
    int res;
    struct timeval tv;
    res = gettimeofday(&tv, NULL);
    t->sec = tv.tv_sec;
    t->usec = tv.tv_usec;
    return res;
}


int os_mktime(int year, int month, int day, int hour, int min, int sec,
          os_time_t *t)
{
    struct tm tm;

    if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 ||
        sec > 60)
        return -1;

    os_memset(&tm, 0, sizeof(tm));
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;

    *t = (os_time_t) mktime(&tm);
    return 0;
}


int os_daemonize(const char *pid_file)
{
    if (daemon(0, 0)) {
        perror("daemon");
        return -1;
    }

    if (pid_file) {
        FILE *f = fopen(pid_file, "w");
        if (f) {
            fprintf(f, "%u\n", getpid());
            fclose(f);
        }
    }

    return -0;
}


void os_daemonize_terminate(const char *pid_file)
{
    if (pid_file)
        unlink(pid_file);
}


int os_get_random(unsigned char *buf, size_t len)
{
    FILE *f;
    size_t rc;

    f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        printf("Could not open /dev/urandom.\n");
        return -1;
    }

    rc = fread(buf, 1, len, f);
    fclose(f);

    return rc != len ? -1 : 0;
}


unsigned long os_random(void)
{
    return random();
}


char * os_rel2abs_path(const char *rel_path)
{
    char *buf = NULL, *cwd, *ret;
    size_t len = 128, cwd_len, rel_len, ret_len;

    if (rel_path[0] == '/')
        return os_strdup(rel_path);

    for (;;) {
        buf = os_malloc(len);
        if (buf == NULL)
            return NULL;
        cwd = getcwd(buf, len);
        if (cwd == NULL) {
            os_free(buf);
            if (errno != ERANGE) {
                return NULL;
            }
            len *= 2;
        } else {
            break;
        }
    }

    cwd_len = strlen(cwd);
    rel_len = strlen(rel_path);
    ret_len = cwd_len + 1 + rel_len + 1;
    ret = os_malloc(ret_len);
    if (ret) {
        os_memcpy(ret, cwd, cwd_len);
        ret[cwd_len] = '/';
        os_memcpy(ret + cwd_len + 1, rel_path, rel_len);
        ret[ret_len - 1] = '\0';
    }
    os_free(buf);
    return ret;
}


int os_program_init(void)
{
    return 0;
}


void os_program_deinit(void)
{
}


int os_setenv(const char *name, const char *value, int overwrite)
{
    return setenv(name, value, overwrite);
}


int os_unsetenv(const char *name)
{
    return unsetenv(name);
}


char * os_readfile(const char *name, size_t *len)
{
    FILE *f;
    char *buf;

    f = fopen(name, "rb");
    if (f == NULL)
        return NULL;

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = os_malloc(*len);
    if (buf == NULL) {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, *len, f);
    fclose(f);

    return buf;
}


void * os_zalloc(size_t size)
{
    void *n = os_malloc(size);
    if (n)
        os_memset(n, 0, size);
    return n;
}


void * os_malloc(size_t size)
{
    return malloc(size);
}


void * os_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}


void os_free(void *ptr)
{
    free(ptr);
}


void * os_memcpy(void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;
    while (n--)
        *d++ = *s++;
    return dest;
}


void * os_memmove(void *dest, const void *src, size_t n)
{
    if (dest < src)
        os_memcpy(dest, src, n);
    else {
        /* overlapping areas */
        char *d = (char *) dest + n;
        const char *s = (const char *) src + n;
        while (n--)
            *--d = *--s;
    }
    return dest;
}


void * os_memset(void *s, int c, size_t n)
{
    char *p = s;
    while (n--)
        *p++ = c;
    return s;
}


int os_memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = s1, *p2 = s2;

    if (n == 0)
        return 0;

    while (*p1 == *p2) {
        p1++;
        p2++;
        n--;
        if (n == 0)
            return 0;
    }

    return *p1 - *p2;
}


char * os_strdup(const char *s)
{
    char *res;
    size_t len;
    if (s == NULL)
        return NULL;
    len = os_strlen(s);
    res = os_malloc(len + 1);
    if (res)
        os_memcpy(res, s, len + 1);
    return res;
}


size_t os_strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}


int os_strcasecmp(const char *s1, const char *s2)
{
    /*
     * Ignoring case is not required for main functionality, so just use
     * the case sensitive version of the function.
     */
    return os_strcmp(s1, s2);
}


int os_strncasecmp(const char *s1, const char *s2, size_t n)
{
    /*
     * Ignoring case is not required for main functionality, so just use
     * the case sensitive version of the function.
     */
    return os_strncmp(s1, s2, n);
}


char * os_strchr(const char *s, int c)
{
    while (*s) {
        if (*s == c)
            return (char *) s;
        s++;
        //printf("==>s = %p\n", s);
    }
    return NULL;
}


char * os_strrchr(const char *s, int c)
{
    const char *p = s;
    while (*p)
        p++;
    p--;
    while (p >= s) {
        if (*p == c)
            return (char *) p;
        p--;
    }
    return NULL;
}


int os_strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
            break;
        s1++;
        s2++;
    }

    return *s1 - *s2;
}


int os_strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    while (*s1 == *s2) {
        if (*s1 == '\0')
            break;
        s1++;
        s2++;
        n--;
        if (n == 0)
            return 0;
    }

    return *s1 - *s2;
}


char * os_strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;

    while (n--) {
        *d = *src;
        if (*src == '\0')
            break;
        d++;
        src++;
    }

    return dest;
}


size_t os_strlcpy(char *dest, const char *src, size_t siz)
{
    const char *s = src;
    size_t left = siz;

    if (left) {
        /* Copy string up to the maximum size of the dest buffer */
        while (--left != 0) {
            if ((*dest++ = *s++) == '\0')
                break;
        }
    }

    if (left == 0) {
        /* Not enough room for the string; force NUL-termination */
        if (siz != 0)
            *dest = '\0';
        while (*s++)
            ; /* determine total src string length */
    }

    return s - src - 1;
}


char * os_strstr(const char *haystack, const char *needle)
{
    size_t len = os_strlen(needle);
    while (*haystack) {
        if (os_strncmp(haystack, needle, len) == 0)
            return (char *) haystack;
        haystack++;
    }

    return NULL;
}


int os_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    int ret;

    /* See http://www.ijs.si/software/snprintf/ for portable
     * implementation of snprintf.
     */

    va_start(ap, format);
    ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    if (size > 0)
        str[size - 1] = '\0';
    return ret;
}
