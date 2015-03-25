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

/* //device/libs/telephony/ril_event.cpp
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#define LOG_TAG "RILC"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ril_event.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <telephony/ril.h>

#include <pthread.h>
static pthread_mutex_t listMutex;
#define MUTEX_ACQUIRE() pthread_mutex_lock(&listMutex)
#define MUTEX_RELEASE() pthread_mutex_unlock(&listMutex)
#define MUTEX_INIT() pthread_mutex_init(&listMutex, NULL)
#define MUTEX_DESTROY() pthread_mutex_destroy(&listMutex)

#ifndef timeradd
#define timeradd(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;       \
		if ((vvp)->tv_usec >= 1000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_usec -= 1000000;			\
		}							\
	} while (0)
#endif

#ifndef timercmp
#define timercmp(a, b, op)               \
        ((a)->tv_sec == (b)->tv_sec      \
        ? (a)->tv_usec op (b)->tv_usec   \
        : (a)->tv_sec op (b)->tv_sec)
#endif

#ifndef timersub
#define timersub(a, b, res)                           \
    do {                                              \
        (res)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (res)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((res)->tv_usec < 0) {                     \
            (res)->tv_usec += 1000000;                \
            (res)->tv_sec -= 1;                       \
        }                                             \
    } while(0);
#endif

static fd_set readFds;
static int nfds = 0;

static struct ril_event * watch_table[MAX_FD_EVENTS];
static struct ril_event timer_list;
static struct ril_event pending_list;

#define DEBUG 0

#if DEBUG
#define dlog(x...) LOGD( x )
static void dump_event(struct ril_event * ev)
{
    dlog("~~~~ Event %x ~~~~", (unsigned int)ev);
    dlog("     next    = %x", (unsigned int)ev->next);
    dlog("     prev    = %x", (unsigned int)ev->prev);
    dlog("     fd      = %d", ev->fd);
    dlog("     pers    = %d", ev->persist);
    dlog("     timeout = %ds + %dus", (int)ev->timeout.tv_sec, (int)ev->timeout.tv_usec);
    dlog("     func    = %x", (unsigned int)ev->func);
    dlog("     param   = %x", (unsigned int)ev->param);
    dlog("~~~~~~~~~~~~~~~~~~");
}
#else
#define dlog(x...)
#define dump_event(x)
#endif

static void getNow(struct timeval * tv)
{
#ifdef HAVE_POSIX_CLOCKS
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec/1000;
#else
    gettimeofday(tv, NULL);
#endif
}

static void init_list(struct ril_event * list)
{
    memset(list, 0, sizeof(struct ril_event));
    list->next = list;
    list->prev = list;
    list->fd = -1;
}

static void addToList(struct ril_event * ev, struct ril_event * list)
{
    ev->next = list;
    ev->prev = list->prev;
    ev->prev->next = ev;
    list->prev = ev;
    dump_event(ev);
}

static void removeFromList(struct ril_event * ev)
{
    dlog("~~~~ Removing event ~~~~");
    dump_event(ev);

    ev->next->prev = ev->prev;
    ev->prev->next = ev->next;
    ev->next = NULL;
    ev->prev = NULL;
}


static void removeWatch(struct ril_event * ev, int index)
{
    watch_table[index] = NULL;
    ev->index = -1;

    FD_CLR(ev->fd, &readFds);

    if (ev->fd+1 == nfds) {
        int n = 0;

        for (int i = 0; i < MAX_FD_EVENTS; i++) {
            struct ril_event * rev = watch_table[i];

            if ((rev != NULL) && (rev->fd > n)) {
                n = rev->fd;
            }
        }
        nfds = n + 1; 
        dlog("~~~~ nfds = %d ~~~~", nfds);
    }
}

static void processTimeouts()
{
    dlog("~~~~ +processTimeouts ~~~~");
    MUTEX_ACQUIRE();
    struct timeval now;
    struct ril_event * tev = timer_list.next;
    struct ril_event * next;

    getNow(&now);
    // walk list, see if now >= ev->timeout for any events

    dlog("~~~~ Looking for timers <= %ds + %dus ~~~~", (int)now.tv_sec, (int)now.tv_usec);
    while ((tev != &timer_list) && (timercmp(&now, &tev->timeout, >))) {
        // Timer expired
        dlog("~~~~ firing timer ~~~~");
        next = tev->next;
        removeFromList(tev);
        addToList(tev, &pending_list);
        tev = next;
    }
    MUTEX_RELEASE();
    dlog("~~~~ -processTimeouts ~~~~");
}

static void processReadReadies(fd_set * rfds, int n)
{
    dlog("~~~~ +processReadReadies (%d) ~~~~", n);
    MUTEX_ACQUIRE();

    for (int i = 0; (i < MAX_FD_EVENTS) && (n > 0); i++) {
        struct ril_event * rev = watch_table[i];
        if (rev != NULL && FD_ISSET(rev->fd, rfds)) {
            addToList(rev, &pending_list);
            if (rev->persist == false) {
                removeWatch(rev, i);
            }
            n--;
        }
    }

    MUTEX_RELEASE();
    dlog("~~~~ -processReadReadies (%d) ~~~~", n);
}

static void firePending()
{
    dlog("~~~~ +firePending ~~~~");
    struct ril_event * ev = pending_list.next;
    while (ev != &pending_list) {
        struct ril_event * next = ev->next;
        removeFromList(ev);
        ev->func(ev->fd, 0, ev->param); 
        ev = next;
    }
    dlog("~~~~ -firePending ~~~~");
}

static int calcNextTimeout(struct timeval * tv)
{
    struct ril_event * tev = timer_list.next;
    struct timeval now;

    getNow(&now);

    // Sorted list, so calc based on first node
    if (tev == &timer_list) {
        // no pending timers
        return -1;
    }

    dlog("~~~~ now = %ds + %dus ~~~~", (int)now.tv_sec, (int)now.tv_usec);
    dlog("~~~~ next = %ds + %dus ~~~~",
            (int)tev->timeout.tv_sec, (int)tev->timeout.tv_usec);
    if (timercmp(&tev->timeout, &now, >)) {
        timersub(&tev->timeout, &now, tv);
    } else {
        // timer already expired.
        tv->tv_sec = tv->tv_usec = 0;
    }
    return 0;
}

// Initialize internal data structs
void ril_event_init()
{
    MUTEX_INIT();

    FD_ZERO(&readFds);
    init_list(&timer_list);
    init_list(&pending_list);
    memset(watch_table, 0, sizeof(watch_table));
}

// Initialize an event
void ril_event_set(struct ril_event * ev, int fd, bool persist, ril_event_cb func, void * param)
{
    dlog("~~~~ ril_event_set %x ~~~~", (unsigned int)ev);
    memset(ev, 0, sizeof(struct ril_event));
    ev->fd = fd;
    ev->index = -1;
    ev->persist = persist;
    ev->func = func;
    ev->param = param;
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

// Add event to watch list
void ril_event_add(struct ril_event * ev)
{
    dlog("~~~~ +ril_event_add ~~~~");
    MUTEX_ACQUIRE();
    for (int i = 0; i < MAX_FD_EVENTS; i++) {
        if (watch_table[i] == NULL) {
            watch_table[i] = ev;
            ev->index = i;
            dlog("~~~~ added at %d ~~~~", i);
            dump_event(ev);
            FD_SET(ev->fd, &readFds);
            if (ev->fd >= nfds) nfds = ev->fd+1;
            dlog("~~~~ nfds = %d ~~~~", nfds);
            break;
        } 
    }
    MUTEX_RELEASE();
    dlog("~~~~ -ril_event_add ~~~~");
}

// Add timer event
void ril_timer_add(struct ril_event * ev, struct timeval * tv)
{
    dlog("~~~~ +ril_timer_add ~~~~");
    MUTEX_ACQUIRE();

    struct ril_event * list;
    if (tv != NULL) {
        // add to timer list
        list = timer_list.next;
        ev->fd = -1; // make sure fd is invalid

        struct timeval now;
        getNow(&now);
        timeradd(&now, tv, &ev->timeout);

        // keep list sorted
        while (timercmp(&list->timeout, &ev->timeout, < )
                && (list != &timer_list)) {
            list = list->next;
        }
        // list now points to the first event older than ev
        addToList(ev, list);
    } 

    MUTEX_RELEASE();
    dlog("~~~~ -ril_timer_add ~~~~");
}

// Remove event from watch or timer list
void ril_event_del(struct ril_event * ev)
{
    dlog("~~~~ +ril_event_del ~~~~");
    MUTEX_ACQUIRE();

    if (ev->index < 0 || ev->index >= MAX_FD_EVENTS) {
        return;
    }

    removeWatch(ev, ev->index);

    MUTEX_RELEASE();
    dlog("~~~~ -ril_event_del ~~~~");
}

void ril_event_loop()
{
    int n;
    fd_set rfds;
    struct timeval tv;
    struct timeval * ptv;


    for (;;) {

        // make local copy of read fd_set
        memcpy(&rfds, &readFds, sizeof(fd_set));
        if (-1 == calcNextTimeout(&tv)) {
            // no pending timers; block indefinitely
            dlog("~~~~ no timers; blocking indefinitely ~~~~");
            ptv = NULL;
        } else {
            dlog("~~~~ blocking for %ds + %dus ~~~~", (int)tv.tv_sec, (int)tv.tv_usec);
            ptv = &tv;
        }
        n = select(nfds, &rfds, NULL, NULL, ptv);
        dlog("~~~~ %d events fired ~~~~", n);
        if (n < 0) {
            if (errno == EINTR) continue;

            LOGE("ril_event: select error (%d)", errno);
            // bail?

            int selectFd = -1;
            for (int i=0; (i < MAX_FD_EVENTS) && (n>0); i++) {
                struct ril_event * rev = watch_table[i];
                if(rev != NULL && FD_ISSET(rev->fd, &rfds)) {
                    FD_CLR(rev->fd, &rfds);
                    selectFd = rev->fd;
                    n = select(nfds, &rfds, NULL, NULL, ptv);
                    if(n >=0) {
                        LOGE("ril_event: after remove fd: %d, select success", selectFd);
                        break;
                    }
                }
            }
            LOGE("ril_event: after remove fd:%d, select success2", selectFd);

            int errcode_tmp = errno;
            if (errcode_tmp == 9)
            {
                struct timeval    tv;
                int    rc;
                int fd_max = 0;  
				int i = 0;
				fd_set fds_tmp;
				int error_fd = 0;
				int errcode_test = 0;

				fd_max = nfds;

				for(i = 0; i < fd_max; i++)
				{
				    if (FD_ISSET(i, &readFds))
				    {
				        FD_ZERO(&fds_tmp);
						FD_SET(i,  &fds_tmp);
						
                        tv.tv_sec = tv.tv_usec = 0;

                        rc = select(i + 1, &fds_tmp, NULL, NULL, &tv);	
						
						if(rc < 0)
						{
						    errcode_test = errno;
							if(errcode_test == 9)
							{
							    LOGE("ril_event: test error fd= (%d)", i);
							}
							else
							{
							    LOGE("ril_event: test error (%d)", errcode_test);
							}
						}
					
				    }
				}
            }
            return;
        }

        // Check for timeouts
        processTimeouts();
        // Check for read-ready
        processReadReadies(&rfds, n);
        // Fire away
        firePending();
    }
}
