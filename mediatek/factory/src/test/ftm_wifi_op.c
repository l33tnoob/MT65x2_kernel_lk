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

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <net/if_arp.h>		    /* For ARPHRD_ETHER */
#include <sys/socket.h>		    /* For AF_INET & struct sockaddr */
#include <netinet/in.h>         /* For struct sockaddr_in */
#include <netinet/if_ether.h>
#include <linux/wireless.h>

#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>

#include <net/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "common.h"
#include "iwlib.h"

#include "ftm.h"


#define LOG_TAG "WIFI-FM"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "private/android_filesystem_config.h"
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

#define LOAD_WIFI_MODULE_ONCE

#define TAG                 "[FT_WIFI] "

extern int ifc_init();
extern void ifc_close();
extern int ifc_up(const char *name);
extern int ifc_down(const char *name);
extern int init_module(void *, unsigned long, const char *);
extern int delete_module(const char *, unsigned int);

extern sp_ata_data return_data;

static char iface[PROPERTY_VALUE_MAX];
// TODO: use new ANDROID_SOCKET mechanism, once support for multiple
// sockets is in

#ifndef WIFI_DRIVER_MODULE_PATH
#define WIFI_DRIVER_MODULE_PATH     "/system/lib/modules/wlan.ko"
#endif
#ifndef WIFI_DRIVER_MODULE_NAME
#define WIFI_DRIVER_MODULE_NAME     "wlan"
#endif
#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG      ""
#endif
#ifndef WIFI_FIRMWARE_LOADER
#define WIFI_FIRMWARE_LOADER        ""
#endif
#define WIFI_TEST_INTERFACE         "sta"

#define WIFI_POWER_PATH     "/dev/wmtWifi"

static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";
static const char DRIVER_MODULE_NAME[]  = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = WIFI_DRIVER_MODULE_ARG;
static const char FIRMWARE_LOADER[]     = WIFI_FIRMWARE_LOADER;
static const char MODULE_FILE[]         = "/proc/modules";
static const char WIFI_PROP_NAME[]      = "WIFI.SSID";
//static int fpreferssid = -1;
//static const char PREFER_SSID[40];

/* MTK, Infinity, 20090814, Add for WiFi power management { */ 
//static int wifi_rfkill_id = -1;
//static char *wifi_rfkill_state_path = NULL;

static int skfd = -1;
/* PF Link message */
static int  sPflink = -1;

pthread_t* wifi_thread;

//wireless_scan_head scanlist;

//wireless_scan ap;

//wireless_info	info;

static char* g_output_buf = NULL;
static int   g_output_buf_len;

int	iw_ignore_version = 0;

typedef struct ap_info{
    char ssid[33];
    unsigned char mac[6];
    int mode;    
    int channel;    
    unsigned int rssi;
    int rate;
    int media_status;	
}ap_info;

enum{
    media__disconnect=0,
    media_connecting,
    media_connected
}media_status;

/* function declaim  */

extern char *ftm_get_prop(const char *name);
//static int wifi_init_rfkill(void);
//static int wifi_check_power(void);
static int wifi_set_power(int on);
static int insmod(const char *filename, const char *args);
static int rmmod(const char *modname);
static int check_driver_loaded();
int wifi_init_iface(char *ifname);
int wifi_load_driver();
int wifi_unload_driver();

int
iw_get_range_info(int		skfd,
		  const char *	ifname,
		  iwrange *	range);

int
iw_get_stats(int		skfd,
	     const char *	ifname,
	     iwstats *		stats,
	     const iwrange *	range,
	     int		has_range);

double
iw_freq2float(const iwfreq *	in);

int
iw_get_basic_config(int			skfd,
		    const char *	ifname,
		    wireless_config *	info);

int
iw_extract_event_stream(struct stream_descr *	stream,	/* Stream of events */
			struct iw_event *	iwe,	/* Extracted event */
			int			we_version);

static inline struct wireless_scan *
iw_process_scanning_token(struct iw_event *		event,
			  struct wireless_scan *	wscan); 

int
iw_process_scan(int			skfd,
		char *			ifname,
		int			we_version,
		wireless_scan_head *	context); 

void
iw_init_event_stream(struct stream_descr *	stream,	/* Stream of events */
		     char *			data,
		     int			len);

int
iw_scan(int			skfd,
    char *			ifname,
    int			we_version,
    wireless_scan_head *	context);
		  
int
iw_freq_to_channel(double			freq,
		   const struct iw_range *	range);
		   

int wifi_select_ap();
void update_Text_Info(ap_info * pApInfo, char* output_buf, int buf_len);
int wifi_connect();
int wifi_disconnect(); 
int FM_WIFI_init(char* output_buf, int buf_len, int* p_result);
int FM_WIFI_deinit(void);
int read_preferred_ssid(char * ssid, int len);
int wifi_update_status();

static inline char *
iw_get_ifname(char *	name,
	      int	nsize,	
	      char *	buf);

int find_wifi_device();

/*------------------------------------------------------------------*/
/*
 * Extract the interface name out of /proc/net/wireless or /proc/net/dev.
 */
static inline char *
iw_get_ifname(char *	name,	/* Where to store the name */
	      int	nsize,	/* Size of name buffer */
	      char *	buf)	/* Current position in buffer */
{
  char *	end;

  /* Skip leading spaces */
  while(isspace(*buf))
    buf++;

  end = strrchr(buf, ':');

  /* Not found ??? To big ??? */
  if((end == NULL) || (((end - buf) + 1) > nsize))
    return(NULL);

  /* Copy */
  memcpy(name, buf, (end - buf));
  name[end - buf] = '\0';

  /* Return value currently unused, just make sure it's non-NULL */
  return(end);
}

int find_wifi_device()
{
    FILE *	fh;   
    char		buff[1024];	
	int ret = -1;
	
    fh = fopen(PROC_NET_DEV, "r");

    if(fh != NULL)
    {
      /* Success : use data from /proc/net/wireless */

      /* Eat 2 lines of header */
      fgets(buff, sizeof(buff), fh);
      fgets(buff, sizeof(buff), fh);

      /* Read each device line */
      while(fgets(buff, sizeof(buff), fh))
	{
	  char	name[IFNAMSIZ + 1];
	  char *s;

	  /* Skip empty or almost empty lines. It seems that in some
	   * cases fgets return a line with only a newline. */
	  if((buff[0] == '\0') || (buff[1] == '\0'))
	    continue;
	  /* Extract interface name */
	  s = iw_get_ifname(name, sizeof(name), buff);

	  if(s)
	  	{
	  	    LOGD(TAG "[find_wifi_device]%s",name);
            if( strcmp(name, "wlan0") == 0 ){
				ret = 0;
                break;
		    }
	    }
	}

      fclose(fh);
    }

	return ret;
}

/*
* Control Wi-Fi power by RFKILL interface is deprecated.
* Use character device to control instead.
*/
#if 0
static int wifi_init_rfkill(void) 
{
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;
    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            LOGW("open(%s) failed: %s (%d)\n", path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 4 && memcmp(buf, "wlan", 4) == 0) {
            wifi_rfkill_id = id;
            break;
        }
    }

    asprintf(&wifi_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", 
        wifi_rfkill_id);
    return 0;
}

static int wifi_check_power(void) 
{
    int sz;
    int fd = -1;
    int ret = -1;
    char buffer;

    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) goto out;
    }

    fd = open(wifi_rfkill_state_path, O_RDONLY);
    if (fd < 0) {
        LOGE("open(%s) failed: %s (%d)", wifi_rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    sz = read(fd, &buffer, 1);
    if (sz != 1) {
        LOGE("read(%s) failed: %s (%d)", wifi_rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }

    switch (buffer) {
    case '1':
        ret = 1;
        break;
    case '0':
        ret = 0;
        break;
    }

out:
    if (fd >= 0) close(fd);
    return ret;
}

static int wifi_set_power(int on) 
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');
    	
    LOGD("wifi_set_power, %d",on);
    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) goto out;
    }

    fd = open(wifi_rfkill_state_path, O_WRONLY);
    LOGD("wifi_set_power,%s", wifi_rfkill_state_path);    
    if (fd < 0) {
        LOGE("open(%s) for write failed: %s (%d)", wifi_rfkill_state_path,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        LOGE("write(%s) failed: %s (%d)", wifi_rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}
#else
static int wifi_set_power(int on) 
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');
    	
    LOGD("wifi_set_power, %d",on);

    fd = open(WIFI_POWER_PATH, O_WRONLY);
    LOGD("wifi_set_power,%s", WIFI_POWER_PATH);    
    if (fd < 0) {
        LOGE("open(%s) for write failed: %s (%d)", WIFI_POWER_PATH,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        LOGE("write(%s) failed: %s (%d)", WIFI_POWER_PATH, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}
#endif
/* MTK, Infinity, 20090814, Add for WiFi power management } */

static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        LOGD("Unable to unload driver module \"%s\": %s\n",
             modname, strerror(errno));
    return ret;
}


static int check_driver_loaded() {
/*ALPS01328414: No need to check for build-in driver or kernel module*/
#if 1
    return 1;
#else
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];

    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        LOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        return 0;
    }
    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }
    fclose(proc);
    return 0;
#endif
}


int wifi_init_iface(char *ifname)
{
    int s, ret = 0;
    struct iwreq wrq;
    char buf[33];

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        LOGE("socket(AF_INET,SOCK_DGRAM)");
        return -1;
    }

    LOGD("[WIFI] wifi_init_iface: set mode\n");

    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    wrq.u.mode = IW_MODE_INFRA;

    if (ioctl(s, SIOCSIWMODE, &wrq) < 0) {
        LOGE("ioctl(SIOCSIWMODE)");
        ret = -1;
        goto exit;
    }

    memset(&wrq, 0, sizeof(struct iwreq));    
    memset(buf, '\0', sizeof(buf));

    LOGD("[WIFI] wifi_init_iface: set essid\n");

    strcpy(buf, "aaa");
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    wrq.u.essid.flags = 1; /* flags: 1 = ESSID is active, 0 = not (promiscuous) */
    wrq.u.essid.pointer = (caddr_t) buf;
    wrq.u.essid.length = strlen(buf);    
    if (WIRELESS_EXT < 21)
        wrq.u.essid.length++;

    if (ioctl(s, SIOCSIWESSID, &wrq) < 0) {
        LOGD("ioctl(SIOCSIWESSID)");
        ret = -1;
        goto exit;
    }

exit:
    close(s);

    return ret;    
}

int wifi_load_driver()
{
    char driver_status[PROPERTY_VALUE_MAX];
    int count = 60;

    LOGD("[WIFI] wifi_load_driver\n");

    wifi_set_power(1);

    if (!check_driver_loaded()) {
    	  LOGD(TAG "[wifi_load_driver] loading wifi driver ... ...\n");    	  
        if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0) {
        	  LOGD(TAG "[wifi_load_driver] failed to load wifi driver!!\n");    	
            goto error;
        }
    }

    sched_yield();
	
	while(count -- > 0){
		if(find_wifi_device()==0){
    	    LOGD(TAG "[wifi_load_driver] find wifi device\n");
			break;
		}
		usleep(50000);
	}
    usleep(50000);		
    return wifi_init_iface("wlan0");
error:
    LOGD("[WIFI] wifi_load_driver error\n");
    wifi_set_power(0);
    return -1;
}

int wifi_unload_driver()
{
    int count = 20; /* wait at most 10 seconds for completion */

    LOGD("[WIFI] wifi_unload_driver\n");

#ifdef LOAD_WIFI_MODULE_ONCE
    wifi_set_power(0);
    return 0;
#else
    if (rmmod(DRIVER_MODULE_NAME) == 0) {
        while (count-- > 0) {
            if (!check_driver_loaded())
                break;
            usleep(500000);
        }
        sched_yield();
        wifi_set_power(0);
        if (count)
            return 0;
        return -1;
    } else {
        return -1;
    }
#endif
}
 
/*------------------------------------------------------------------*/
/*
 * Read /proc/net/wireless to get the latest statistics
 * Note : strtok not thread safe, not used in WE-12 and later.
 */
int
iw_get_stats(int		skfd,
	     const char *	ifname,
	     iwstats *		stats,
	     const iwrange *	range,
	     int		has_range)
{
  /* Fortunately, we can always detect this condition properly */
  if((has_range) && (range->we_version_compiled > 11))
    {
      struct iwreq		wrq;
      wrq.u.data.pointer = (caddr_t) stats;
      wrq.u.data.length = sizeof(struct iw_statistics);
      wrq.u.data.flags = 1;		/* Clear updated flag */
      strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
      if(iw_get_ext(skfd, ifname, SIOCGIWSTATS, &wrq) < 0)
	return(-1);

      /* Format has not changed since WE-12, no conversion */
      return(0);
    }
  else
    {
      FILE *	f = fopen(PROC_NET_WIRELESS, "r");
      char	buf[256];
      char *	bp;
      int	t;

      if(f==NULL)
	return -1;
      /* Loop on all devices */
      while(fgets(buf,255,f))
	{
	  bp=buf;
	  while(*bp&&isspace(*bp))
	    bp++;
	  /* Is it the good device ? */
	  if(strncmp(bp,ifname,strlen(ifname))==0 && bp[strlen(ifname)]==':')
	    {
	      /* Skip ethX: */
	      bp=strchr(bp,':');
	      bp++;
	      /* -- status -- */
	      bp = strtok(bp, " ");
	      sscanf(bp, "%X", &t);
	      stats->status = (unsigned short) t;
	      /* -- link quality -- */
	      bp = strtok(NULL, " ");
	      if(strchr(bp,'.') != NULL)
		stats->qual.updated |= 1;
	      sscanf(bp, "%d", &t);
	      stats->qual.qual = (unsigned char) t;
	      /* -- signal level -- */
	      bp = strtok(NULL, " ");
	      if(strchr(bp,'.') != NULL)
		stats->qual.updated |= 2;
	      sscanf(bp, "%d", &t);
	      stats->qual.level = (unsigned char) t;
	      /* -- noise level -- */
	      bp = strtok(NULL, " ");
	      if(strchr(bp,'.') != NULL)
		stats->qual.updated += 4;
	      sscanf(bp, "%d", &t);
	      stats->qual.noise = (unsigned char) t;
	      /* -- discarded packets -- */
	      bp = strtok(NULL, " ");
	      sscanf(bp, "%d", &stats->discard.nwid);
	      bp = strtok(NULL, " ");
	      sscanf(bp, "%d", &stats->discard.code);
	      bp = strtok(NULL, " ");
	      sscanf(bp, "%d", &stats->discard.misc);
	      fclose(f);
	      /* No conversion needed */
	      return 0;
	    }
	}
      fclose(f);
      return -1;
    }
}

/*------------------------------------------------------------------*/
/*
 * Get the range information out of the driver
 */
int
iw_get_range_info(int		skfd,
		  const char *	ifname,
		  iwrange *	range)
{
  struct iwreq		wrq;
  char			buffer[sizeof(iwrange) * 2];	/* Large enough */
  union iw_range_raw *	range_raw;

  /* Cleanup */
  bzero(buffer, sizeof(buffer));

  wrq.u.data.pointer = (caddr_t) buffer;
  wrq.u.data.length = sizeof(buffer);
  wrq.u.data.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWRANGE, &wrq) < 0)
    return(-1);

  /* Point to the buffer */
  range_raw = (union iw_range_raw *) buffer;

  /* For new versions, we can check the version directly, for old versions
   * we use magic. 300 bytes is a also magic number, don't touch... */
  if(wrq.u.data.length < 300)
    {
      /* That's v10 or earlier. Ouch ! Let's make a guess...*/
      range_raw->range.we_version_compiled = 9;
    }

  /* Check how it needs to be processed */
  if(range_raw->range.we_version_compiled > 15)
    {
      /* This is our native format, that's easy... */
      /* Copy stuff at the right place, ignore extra */
      memcpy((char *) range, buffer, sizeof(iwrange));
    }
  else
    {
      /* Zero unknown fields */
      bzero((char *) range, sizeof(struct iw_range));

      /* Initial part unmoved */
      memcpy((char *) range,
	     buffer,
	     iwr15_off(num_channels));
      /* Frequencies pushed futher down towards the end */
      memcpy((char *) range + iwr_off(num_channels),
	     buffer + iwr15_off(num_channels),
	     iwr15_off(sensitivity) - iwr15_off(num_channels));
      /* This one moved up */
      memcpy((char *) range + iwr_off(sensitivity),
	     buffer + iwr15_off(sensitivity),
	     iwr15_off(num_bitrates) - iwr15_off(sensitivity));
      /* This one goes after avg_qual */
      memcpy((char *) range + iwr_off(num_bitrates),
	     buffer + iwr15_off(num_bitrates),
	     iwr15_off(min_rts) - iwr15_off(num_bitrates));
      /* Number of bitrates has changed, put it after */
      memcpy((char *) range + iwr_off(min_rts),
	     buffer + iwr15_off(min_rts),
	     iwr15_off(txpower_capa) - iwr15_off(min_rts));
      /* Added encoding_login_index, put it after */
      memcpy((char *) range + iwr_off(txpower_capa),
	     buffer + iwr15_off(txpower_capa),
	     iwr15_off(txpower) - iwr15_off(txpower_capa));
      /* Hum... That's an unexpected glitch. Bummer. */
      memcpy((char *) range + iwr_off(txpower),
	     buffer + iwr15_off(txpower),
	     iwr15_off(avg_qual) - iwr15_off(txpower));
      /* Avg qual moved up next to max_qual */
      memcpy((char *) range + iwr_off(avg_qual),
	     buffer + iwr15_off(avg_qual),
	     sizeof(struct iw_quality));
    }

  /* We are now checking much less than we used to do, because we can
   * accomodate more WE version. But, there are still cases where things
   * will break... */
  if(!iw_ignore_version)
    {
      /* We don't like very old version (unfortunately kernel 2.2.X) */
      if(range->we_version_compiled <= 10)
	{
	  fprintf(stderr, "Warning: Driver for device %s has been compiled with an ancient version\n", ifname);
	  fprintf(stderr, "of Wireless Extension, while this program support version 11 and later.\n");
	  fprintf(stderr, "Some things may be broken...\n\n");
	}

      /* We don't like future versions of WE, because we can't cope with
       * the unknown */
      if(range->we_version_compiled > WE_MAX_VERSION)
	{
	  fprintf(stderr, "Warning: Driver for device %s has been compiled with version %d\n", ifname, range->we_version_compiled);
	  fprintf(stderr, "of Wireless Extension, while this program supports up to version %d.\n", WE_MAX_VERSION);
	  fprintf(stderr, "Some things may be broken...\n\n");
	}

      /* Driver version verification */
      if((range->we_version_compiled > 10) &&
	 (range->we_version_compiled < range->we_version_source))
	{
	  fprintf(stderr, "Warning: Driver for device %s recommend version %d of Wireless Extension,\n", ifname, range->we_version_source);
	  fprintf(stderr, "but has been compiled with version %d, therefore some driver features\n", range->we_version_compiled);
	  fprintf(stderr, "may not be available...\n\n");
	}
      /* Note : we are only trying to catch compile difference, not source.
       * If the driver source has not been updated to the latest, it doesn't
       * matter because the new fields are set to zero */
    }

  /* Don't complain twice.
   * In theory, the test apply to each individual driver, but usually
   * all drivers are compiled from the same kernel. */
  iw_ignore_version = 1;

  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Get essential wireless config from the device driver
 * We will call all the classical wireless ioctl on the driver through
 * the socket to know what is supported and to get the settings...
 * Note : compare to the version in iwconfig, we extract only
 * what's *really* needed to configure a device...
 */
int
iw_get_basic_config(int			skfd,
		    const char *	ifname,
		    wireless_config *	info)
{
  struct iwreq		wrq;

  memset((char *) info, 0, sizeof(struct wireless_config));

  /* Get wireless name */
  if(iw_get_ext(skfd, ifname, SIOCGIWNAME, &wrq) < 0)
    /* If no wireless name : no wireless extensions */
    return(-1);
  else
    {
      strncpy(info->name, wrq.u.name, IFNAMSIZ);
      info->name[IFNAMSIZ] = '\0';
    }

  /* Get network ID */
  if(iw_get_ext(skfd, ifname, SIOCGIWNWID, &wrq) >= 0)
    {
      info->has_nwid = 1;
      memcpy(&(info->nwid), &(wrq.u.nwid), sizeof(iwparam));
    }

  /* Get frequency / channel */
  if(iw_get_ext(skfd, ifname, SIOCGIWFREQ, &wrq) >= 0)
    {
      info->has_freq = 1;
      info->freq = iw_freq2float(&(wrq.u.freq));
      info->freq_flags = wrq.u.freq.flags;
    }

  /* Get encryption information */
  wrq.u.data.pointer = (caddr_t) info->key;
  wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
  wrq.u.data.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWENCODE, &wrq) >= 0)
    {
      info->has_key = 1;
      info->key_size = wrq.u.data.length;
      info->key_flags = wrq.u.data.flags;
    }

  /* Get ESSID */
  wrq.u.essid.pointer = (caddr_t) info->essid;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWESSID, &wrq) >= 0)
    {
      info->has_essid = 1;
      info->essid_on = wrq.u.data.flags;
    }

  /* Get operation mode */
  if(iw_get_ext(skfd, ifname, SIOCGIWMODE, &wrq) >= 0)
    {
      info->has_mode = 1;
      /* Note : event->u.mode is unsigned, no need to check <= 0 */
      if(wrq.u.mode < IW_NUM_OPER_MODE)
	info->mode = wrq.u.mode;
      else
	info->mode = IW_NUM_OPER_MODE;	/* Unknown/bug */
    }

  return(0);
}
/*------------------------------------------------------------------*/
/*
 * Convert our internal representation of frequencies to a floating point.
 */
double
iw_freq2float(const iwfreq *	in)
{
#ifdef WE_NOLIBM
  /* Version without libm : slower */
  int		i;
  double	res = (double) in->m;
  for(i = 0; i < in->e; i++)
    res *= 10;
  return(res);
#else	/* WE_NOLIBM */
  /* Version with libm : faster */
  return ((double) in->m) * pow(10,in->e);
#endif	/* WE_NOLIBM */
}

/*------------------------------------------------------------------*/
/*
 * Extract the next event from the event stream.
 */
int
iw_extract_event_stream(struct stream_descr *	stream,	/* Stream of events */
			struct iw_event *	iwe,	/* Extracted event */
			int			we_version)
{
    const struct iw_ioctl_description *	descr = NULL;
    int		event_type = 0;
    unsigned int	event_len = 1;		/* Invalid */
    char *	pointer;
    /* Don't "optimise" the following variable, it will crash */
    unsigned	cmd_index;		/* *MUST* be unsigned */
    
    /* Check for end of stream */
    if((stream->current + IW_EV_LCP_PK_LEN) > stream->end)
        return(0);

#ifdef DEBUG
    printf("DBG - stream->current = %p, stream->value = %p, stream->end = %p\n",
	      stream->current, stream->value, stream->end);
#endif

    /* Extract the event header (to get the event id).
     * Note : the event may be unaligned, therefore copy... */
    memcpy((char *) iwe, stream->current, IW_EV_LCP_PK_LEN);

#ifdef DEBUG
    printf("DBG - iwe->cmd = 0x%X, iwe->len = %d\n",
        iwe->cmd, iwe->len);
#endif

    /* Check invalid events */
    if(iwe->len <= IW_EV_LCP_PK_LEN)
        return(-1);

    /* Get the type and length of that event */
    if(iwe->cmd <= SIOCIWLAST)
    {
        cmd_index = iwe->cmd - SIOCIWFIRST;
        if(cmd_index < standard_ioctl_num)
	  descr = &(standard_ioctl_descr[cmd_index]);
    }
    else
    {
        cmd_index = iwe->cmd - IWEVFIRST;
        if(cmd_index < standard_event_num)
	          descr = &(standard_event_descr[cmd_index]);
    }
    if(descr != NULL)
      event_type = descr->header_type;
    /* Unknown events -> event_type=0 => IW_EV_LCP_PK_LEN */
    event_len = event_type_size[event_type];
    /* Fixup for earlier version of WE */
    if((we_version <= 18) && (event_type == IW_HEADER_TYPE_POINT))
      event_len += IW_EV_POINT_OFF;
    
    /* Check if we know about this event */
    if(event_len <= IW_EV_LCP_PK_LEN)
      {
        /* Skip to next event */
        stream->current += iwe->len;
        return(2);
      }
    event_len -= IW_EV_LCP_PK_LEN;
    
    /* Set pointer on data */
    if(stream->value != NULL)
      pointer = stream->value;			/* Next value in event */
    else
      pointer = stream->current + IW_EV_LCP_PK_LEN;	/* First value in event */
    
#ifdef DEBUG
  printf("DBG - event_type = %d, event_len = %d, pointer = %p\n",
	 event_type, event_len, pointer);
#endif

  /* Copy the rest of the event (at least, fixed part) */
  if((pointer + event_len) > stream->end)
    {
      /* Go to next event */
      stream->current += iwe->len;
      return(-2);
    }
  /* Fixup for WE-19 and later : pointer no longer in the stream */
  /* Beware of alignement. Dest has local alignement, not packed */
  if((we_version > 18) && (event_type == IW_HEADER_TYPE_POINT))
    memcpy((char *) iwe + IW_EV_LCP_LEN + IW_EV_POINT_OFF,
	   pointer, event_len);
  else
    memcpy((char *) iwe + IW_EV_LCP_LEN, pointer, event_len);

  /* Skip event in the stream */
  pointer += event_len;

  /* Special processing for iw_point events */
  if(event_type == IW_HEADER_TYPE_POINT)
    {
      /* Check the length of the payload */
      unsigned int	extra_len = iwe->len - (event_len + IW_EV_LCP_PK_LEN);
      if(extra_len > 0)
	{
	  /* Set pointer on variable part (warning : non aligned) */
	  iwe->u.data.pointer = pointer;

	  /* Check that we have a descriptor for the command */
	  if(descr == NULL)
	    /* Can't check payload -> unsafe... */
	    iwe->u.data.pointer = NULL;	/* Discard paylod */
	  else
	    {
	      /* Those checks are actually pretty hard to trigger,
	       * because of the checks done in the kernel... */

	      unsigned int	token_len = iwe->u.data.length * descr->token_size;

	      /* Ugly fixup for alignement issues.
	       * If the kernel is 64 bits and userspace 32 bits,
	       * we have an extra 4+4 bytes.
	       * Fixing that in the kernel would break 64 bits userspace. */
	      if((token_len != extra_len) && (extra_len >= 4))
		{
		  __u16		alt_dlen = *((__u16 *) pointer);
		  unsigned int	alt_token_len = alt_dlen * descr->token_size;
		  if((alt_token_len + 8) == extra_len)
		    {
#ifdef DEBUG
		      printf("DBG - alt_token_len = %d\n", alt_token_len);
#endif
		      /* Ok, let's redo everything */
		      pointer -= event_len;
		      pointer += 4;
		      /* Dest has local alignement, not packed */
		      memcpy((char *) iwe + IW_EV_LCP_LEN + IW_EV_POINT_OFF,
			     pointer, event_len);
		      pointer += event_len + 4;
		      iwe->u.data.pointer = pointer;
		      token_len = alt_token_len;
		    }
		}

	      /* Discard bogus events which advertise more tokens than
	       * what they carry... */
	      if(token_len > extra_len)
		iwe->u.data.pointer = NULL;	/* Discard paylod */
	      /* Check that the advertised token size is not going to
	       * produce buffer overflow to our caller... */
	      if((iwe->u.data.length > descr->max_tokens)
		 && !(descr->flags & IW_DESCR_FLAG_NOMAX))
		iwe->u.data.pointer = NULL;	/* Discard paylod */
	      /* Same for underflows... */
	      if(iwe->u.data.length < descr->min_tokens)
		iwe->u.data.pointer = NULL;	/* Discard paylod */
#ifdef DEBUG
	      printf("DBG - extra_len = %d, token_len = %d, token = %d, max = %d, min = %d\n",
		     extra_len, token_len, iwe->u.data.length, descr->max_tokens, descr->min_tokens);
#endif
	    }
	}
      else
	/* No data */
	iwe->u.data.pointer = NULL;

      /* Go to next event */
      stream->current += iwe->len;
    }
  else
    {
      /* Ugly fixup for alignement issues.
       * If the kernel is 64 bits and userspace 32 bits,
       * we have an extra 4 bytes.
       * Fixing that in the kernel would break 64 bits userspace. */
      if((stream->value == NULL)
	 && ((((iwe->len - IW_EV_LCP_PK_LEN) % event_len) == 4)
	     || ((iwe->len == 12) && ((event_type == IW_HEADER_TYPE_UINT) ||
				      (event_type == IW_HEADER_TYPE_QUAL))) ))
	{
#ifdef DEBUG
	  printf("DBG - alt iwe->len = %d\n", iwe->len - 4);
#endif
	  pointer -= event_len;
	  pointer += 4;
	  /* Beware of alignement. Dest has local alignement, not packed */
	  memcpy((char *) iwe + IW_EV_LCP_LEN, pointer, event_len);
	  pointer += event_len;
	}

      /* Is there more value in the event ? */
      if((pointer + event_len) <= (stream->current + iwe->len))
	/* Go to next value */
	stream->value = pointer;
      else
	{
	  /* Go to next event */
	  stream->value = NULL;
	  stream->current += iwe->len;
	}
    }
  return(1);
}

/*********************** SCANNING SUBROUTINES ***********************/
/*
 * The Wireless Extension API 14 and greater define Wireless Scanning.
 * The normal API is complex, this is an easy API that return
 * a subset of the scanning results. This should be enough for most
 * applications that want to use Scanning.
 * If you want to have use the full/normal API, check iwlist.c...
 *
 * Precaution when using scanning :
 * The scanning operation disable normal network traffic, and therefore
 * you should not abuse of scan.
 * The scan need to check the presence of network on other frequencies.
 * While you are checking those other frequencies, you can *NOT* be on
 * your normal frequency to listen to normal traffic in the cell.
 * You need typically in the order of one second to actively probe all
 * 802.11b channels (do the maths). Some cards may do that in background,
 * to reply to scan commands faster, but they still have to do it.
 * Leaving the cell for such an extended period of time is pretty bad.
 * Any kind of streaming/low latency traffic will be impacted, and the
 * user will perceive it (easily checked with telnet). People trying to
 * send traffic to you will retry packets and waste bandwidth. Some
 * applications may be sensitive to those packet losses in weird ways,
 * and tracing those weird behavior back to scanning may take time.
 * If you are in ad-hoc mode, if two nodes scan approx at the same
 * time, they won't see each other, which may create associations issues.
 * For those reasons, the scanning activity should be limited to
 * what's really needed, and continuous scanning is a bad idea.
 * Jean II
 */

/*------------------------------------------------------------------*/
/*
 * Process/store one element from the scanning results in wireless_scan
 */
static inline struct wireless_scan *
iw_process_scanning_token(struct iw_event *		event,
			  struct wireless_scan *	wscan)
{
  struct wireless_scan *	oldwscan;

  /* Now, let's decode the event */
  switch(event->cmd)
    {
    case SIOCGIWAP:
      /* New cell description. Allocate new cell descriptor, zero it. */
      oldwscan = wscan;
      wscan = (struct wireless_scan *) malloc(sizeof(struct wireless_scan));
      if(wscan == NULL)
	return(wscan);
      /* Link at the end of the list */
      if(oldwscan != NULL)
	oldwscan->next = wscan;

      /* Reset it */
      bzero(wscan, sizeof(struct wireless_scan));

      /* Save cell identifier */
      wscan->has_ap_addr = 1;
      memcpy(&(wscan->ap_addr), &(event->u.ap_addr), sizeof (sockaddr));
      break;
    case SIOCGIWNWID:
      wscan->b.has_nwid = 1;
      memcpy(&(wscan->b.nwid), &(event->u.nwid), sizeof(iwparam));
      break;
    case SIOCGIWFREQ:
      wscan->b.has_freq = 1;
      wscan->b.freq = iw_freq2float(&(event->u.freq));
      wscan->b.freq_flags = event->u.freq.flags;
      break;
    case SIOCGIWMODE:
      wscan->b.mode = event->u.mode;
      if((wscan->b.mode < IW_NUM_OPER_MODE) && (wscan->b.mode >= 0))
	wscan->b.has_mode = 1;
      break;
    case SIOCGIWESSID:
      wscan->b.has_essid = 1;
      wscan->b.essid_on = event->u.data.flags;
      memset(wscan->b.essid, '\0', IW_ESSID_MAX_SIZE+1);
      if((event->u.essid.pointer) && (event->u.essid.length))
	memcpy(wscan->b.essid, event->u.essid.pointer, event->u.essid.length);
      break;
    case SIOCGIWENCODE:
      wscan->b.has_key = 1;
      wscan->b.key_size = event->u.data.length;
      wscan->b.key_flags = event->u.data.flags;
      if(event->u.data.pointer)
	memcpy(wscan->b.key, event->u.essid.pointer, event->u.data.length);
      else
	wscan->b.key_flags |= IW_ENCODE_NOKEY;
      break;
    case IWEVQUAL:
      /* We don't get complete stats, only qual */
      wscan->has_stats = 1;
      memcpy(&wscan->stats.qual, &event->u.qual, sizeof(struct iw_quality));
      break;
    case SIOCGIWRATE:
      /* Scan may return a list of bitrates. As we have space for only
       * a single bitrate, we only keep the largest one. */
      if((!wscan->has_maxbitrate) ||
	 (event->u.bitrate.value > wscan->maxbitrate.value))
	{
	  wscan->has_maxbitrate = 1;
	  memcpy(&(wscan->maxbitrate), &(event->u.bitrate), sizeof(iwparam));
	}
    case IWEVCUSTOM:
      /* How can we deal with those sanely ? Jean II */
    default:
      break;
   }	/* switch(event->cmd) */

  return(wscan);
}

/*------------------------------------------------------------------*/
/*
 * Initiate the scan procedure, and process results.
 * This is a non-blocking procedure and it will return each time
 * it would block, returning the amount of time the caller should wait
 * before calling again.
 * Return -1 for error, delay to wait for (in ms), or 0 for success.
 * Error code is in errno
 */
int
iw_process_scan(int			skfd,
		char *			ifname,
		int			we_version,
		wireless_scan_head *	context)
{
  struct iwreq		wrq;
  unsigned char *	buffer = NULL;		/* Results */
  int			buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
  unsigned char *	newbuf;

  /* Don't waste too much time on interfaces (150 * 100 = 15s) */
  context->retry++;
  if(context->retry > 150)
    {
      errno = ETIME;
      return(-1);
    }

  /* If we have not yet initiated scanning on the interface */
  if(context->retry == 1)
    {
      /* Initiate Scan */
      wrq.u.data.pointer = NULL;		/* Later */
      wrq.u.data.flags = 0;
      wrq.u.data.length = 0;
      /* Remember that as non-root, we will get an EPERM here */
      if((iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0)
	 && (errno != EPERM))
	return(-1);
      /* Success : now, just wait for event or results */
      return(1500);	/* Wait 250 ms */
    }

 realloc:
  /* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
  newbuf = realloc(buffer, buflen);
  if(newbuf == NULL)
    {
      /* man says : If realloc() fails the original block is left untouched */
      if(buffer)
	free(buffer);
      errno = ENOMEM;
      return(-1);
    }
  buffer = newbuf;

  /* Try to read the results */
  wrq.u.data.pointer = buffer;
  wrq.u.data.flags = 0;
  wrq.u.data.length = buflen;
  if(iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0)
    {
      /* Check if buffer was too small (WE-17 only) */
      if((errno == E2BIG) && (we_version > 16))
	{
	  /* Some driver may return very large scan results, either
	   * because there are many cells, or because they have many
	   * large elements in cells (like IWEVCUSTOM). Most will
	   * only need the regular sized buffer. We now use a dynamic
	   * allocation of the buffer to satisfy everybody. Of course,
	   * as we don't know in advance the size of the array, we try
	   * various increasing sizes. Jean II */

	  /* Check if the driver gave us any hints. */
	  if(wrq.u.data.length > buflen)
	    buflen = wrq.u.data.length;
	  else
	    buflen *= 2;

	  /* Try again */
	  goto realloc;
	}

      /* Check if results not available yet */
      if(errno == EAGAIN)
	{
	  free(buffer);
	  /* Wait for only 100ms from now on */
	  return(100);	/* Wait 100 ms */
	}

      free(buffer);
      /* Bad error, please don't come back... */
      return(-1);
    }

  /* We have the results, process them */
  if(wrq.u.data.length)
    {
      struct iw_event		iwe;
      struct stream_descr	stream;
      struct wireless_scan *	wscan = NULL;
      int			ret;
#if 1
      /* Debugging code. In theory useless, because it's debugged ;-) */
      int	i;
      printf("Scan result [%02X", buffer[0]);
      for(i = 1; i < wrq.u.data.length; i++)
	printf(":%02X", buffer[i]);
      printf("]\n");
#endif

      /* Init */
      iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
      /* This is dangerous, we may leak user data... */
      context->result = NULL;

      /* Look every token */
      do
	{
	  /* Extract an event and print it */
	  ret = iw_extract_event_stream(&stream, &iwe, we_version);
	  if(ret > 0)
	    {
	      /* Convert to wireless_scan struct */
	      wscan = iw_process_scanning_token(&iwe, wscan);
	      /* Check problems */
	      if(wscan == NULL)
		{
		  free(buffer);
		  errno = ENOMEM;
		  return(-1);
		}
	      /* Save head of list */
	      if(context->result == NULL)
		context->result = wscan;
	    }
	}
      while(ret > 0);
    }

  /* Done with this interface - return success */
  free(buffer);
  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Initialise the struct stream_descr so that we can extract
 * individual events from the event stream.
 */
void
iw_init_event_stream(struct stream_descr *	stream,	/* Stream of events */
		     char *			data,
		     int			len)
{
  /* Cleanup */
  memset((char *) stream, '\0', sizeof(struct stream_descr));

  /* Set things up */
  stream->current = data;
  stream->end = data + len;
}

/*------------------------------------------------------------------*/
/*
 * Perform a wireless scan on the specified interface.
 * This is a blocking procedure and it will when the scan is completed
 * or when an error occur.
 *
 * The scan results are given in a linked list of wireless_scan objects.
 * The caller *must* free the result himself (by walking the list).
 * If there is an error, -1 is returned and the error code is available
 * in errno.
 *
 * The parameter we_version can be extracted from the range structure
 * (range.we_version_compiled - see iw_get_range_info()), or using
 * iw_get_kernel_we_version(). For performance reason, you should
 * cache this parameter when possible rather than querying it every time.
 *
 * Return -1 for error and 0 for success.
 */
int
iw_scan(int			skfd,
    char *			ifname,
    int			we_version,
    wireless_scan_head *	context)
{
    int		delay;		/* in ms */
    
    /* Clean up context. Potential memory leak if(context.result != NULL) */
    context->result = NULL;
    context->retry = 0;
    
    /* Wait until we get results or error */
    while(1){
        /* Try to get scan results */
        delay = iw_process_scan(skfd, ifname, we_version, context);
    
        /* Check termination */
        if(delay <= 0)
            break;
    
        /* Wait a bit */
        usleep(delay * 1000);
    }
    
    /* End - return -1 or 0 */
    return(delay);
}

/************************* DISPLAY ROUTINES **************************/

/*------------------------------------------------------------------*/
/*
 * Get wireless informations & config from the device driver
 * We will call all the classical wireless ioctl on the driver through
 * the socket to know what is supported and to get the settings...
 */
static int
get_info(int			skfd,
	 char *			ifname,
	 struct wireless_info *	info)
{
  struct iwreq		wrq;

  memset((char *) info, 0, sizeof(struct wireless_info));

  /* Get basic information */
  if(iw_get_basic_config(skfd, ifname, &(info->b)) < 0)
    {
      /* If no wireless name : no wireless extensions */
      /* But let's check if the interface exists at all */
      struct ifreq ifr;

      strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
      if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	return(-ENODEV);
      else
	return(-ENOTSUP);
    }

  /* Get ranges */
  if(iw_get_range_info(skfd, ifname, &(info->range)) >= 0)
    info->has_range = 1;

  /* Get AP address */
  if(iw_get_ext(skfd, ifname, SIOCGIWAP, &wrq) >= 0)
    {
      info->has_ap_addr = 1;
      memcpy(&(info->ap_addr), &(wrq.u.ap_addr), sizeof (sockaddr));
    }

  /* Get bit rate */
  if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
    {
      info->has_bitrate = 1;
      memcpy(&(info->bitrate), &(wrq.u.bitrate), sizeof(iwparam));
    }

  /* Get Power Management settings */
  wrq.u.power.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWPOWER, &wrq) >= 0)
    {
      info->has_power = 1;
      memcpy(&(info->power), &(wrq.u.power), sizeof(iwparam));
    }

  /* Get stats */
  if(iw_get_stats(skfd, ifname, &(info->stats),
		  &info->range, info->has_range) >= 0)
    {
      info->has_stats = 1;
    }

#ifndef WE_ESSENTIAL
  /* Get NickName */
  wrq.u.essid.pointer = (caddr_t) info->nickname;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWNICKN, &wrq) >= 0)
    if(wrq.u.data.length > 1)
      info->has_nickname = 1;

  if((info->has_range) && (info->range.we_version_compiled > 9))
    {
      /* Get Transmit Power */
      if(iw_get_ext(skfd, ifname, SIOCGIWTXPOW, &wrq) >= 0)
	{
	  info->has_txpower = 1;
	  memcpy(&(info->txpower), &(wrq.u.txpower), sizeof(iwparam));
	}
    }

  /* Get sensitivity */
  if(iw_get_ext(skfd, ifname, SIOCGIWSENS, &wrq) >= 0)
    {
      info->has_sens = 1;
      memcpy(&(info->sens), &(wrq.u.sens), sizeof(iwparam));
    }

  if((info->has_range) && (info->range.we_version_compiled > 10))
    {
      /* Get retry limit/lifetime */
      if(iw_get_ext(skfd, ifname, SIOCGIWRETRY, &wrq) >= 0)
	{
	  info->has_retry = 1;
	  memcpy(&(info->retry), &(wrq.u.retry), sizeof(iwparam));
	}
    }

  /* Get RTS threshold */
  if(iw_get_ext(skfd, ifname, SIOCGIWRTS, &wrq) >= 0)
    {
      info->has_rts = 1;
      memcpy(&(info->rts), &(wrq.u.rts), sizeof(iwparam));
    }

  /* Get fragmentation threshold */
  if(iw_get_ext(skfd, ifname, SIOCGIWFRAG, &wrq) >= 0)
    {
      info->has_frag = 1;
      memcpy(&(info->frag), &(wrq.u.frag), sizeof(iwparam));
    }
#endif	/* WE_ESSENTIAL */

  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Convert a frequency to a channel (negative -> error)
 */
 #define KILO	1e3
 
int
iw_freq_to_channel(double			freq,
		   const struct iw_range *	range)
{
  double	ref_freq;
  int		k;
  /* Check if it's a frequency or not already a channel */
  if(freq < KILO)
    return(-1);

  /* We compare the frequencies as double to ignore differences
   * in encoding. Slower, but safer... */
  for(k = 0; k < range->num_frequency; k++)
    {
    	
      ref_freq = iw_freq2float(&(range->freq[k]));
      if(freq == ref_freq)
	return(range->freq[k].i);
    }
  /* Not found */
  return(-2);
}


int wifi_update_status()
{
    struct iwreq		wrq;    
    ap_info apinfo;
    wireless_info	wlan_info;
            
    if(iw_get_ext(skfd, "wlan0", SIOCGIWNAME, &wrq) < 0){
        LOGE(TAG" [wifi_update_status] SIOCGIWNAME failed\n");
        return(-1);
    }
    
    if(strcmp(wrq.u.name, "Disconnected")==0){
        LOGD(TAG" [wifi_update_status] status = Disconnected\n");    	
        return(-1);    	
    }
    
    if( get_info( skfd, "wlan0", &wlan_info) < 0 ) {
        LOGE(TAG" [wifi_update_status] failed to get wlan0 info!\n");
        return -1;	        	        
    }

    memcpy( apinfo.ssid, wlan_info.b.essid,sizeof(wlan_info.b.essid));
    memcpy(apinfo.mac,wlan_info.ap_addr.sa_data,sizeof(apinfo.mac));    	  
    apinfo.mode = wlan_info.b.mode;
    apinfo.channel = iw_freq_to_channel( wlan_info.b.freq/1000,&(wlan_info.range));
    apinfo.rssi = (unsigned int)(wlan_info.stats.qual.level) - 0x100;
    apinfo.rate = wlan_info.bitrate.value/1000000;    	  
    apinfo.media_status = media_connected;    	      	  

    LOGD(TAG" [wifi_update_status] connected %s\n", apinfo.ssid);
    update_Text_Info(&apinfo, g_output_buf, g_output_buf_len);  


	return_data.wifi.channel = apinfo.channel;
	return_data.wifi.wifi_rssi = apinfo.rssi;
	return_data.wifi.rate = apinfo.rate;
	sprintf(return_data.wifi.wifi_mac, "%02x-%02x-%02x-%02x-%02x-%02x", 
        apinfo.mac[0], apinfo.mac[1], apinfo.mac[2], 
        apinfo.mac[3], apinfo.mac[4], apinfo.mac[5]);
	strcpy(return_data.wifi.wifi_name, apinfo.ssid);
    
    return 0;
}
 
int wifi_connect(char * ssid)
{
    int ret = 0;
    struct iwreq wrq;
    char buf[33];
    LOGD("[WIFI] wifi_connect: set mode\n");
    
    if(!ssid || strlen(ssid)>32){
        LOGE("[WIFI] wifi_connect: invalid param\n");    	
        return -1;
    }
    
    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, "wlan0", IFNAMSIZ);
    wrq.u.mode = IW_MODE_INFRA;

    if (ioctl(skfd, SIOCSIWMODE, &wrq) < 0) {
        perror("ioctl(SIOCSIWMODE)");
        ret = -1;
        goto exit;
    }

    memset(&wrq, 0, sizeof(struct iwreq));    
    memset(buf, '\0', sizeof(buf));

    LOGD("[WIFI] wifi_init_iface: set essid\n");

    strcpy(buf, ssid);
    //buf[strlen(buf)]="\0";
    LOGD("[WIFI] wifi_init_iface: set essid %s\n",ssid);
    strncpy(wrq.ifr_name, "wlan0", IFNAMSIZ);
    //wrq.u.essid.flags = 1; /* flags: 1 = ESSID is active, 0 = not (promiscuous) */
    wrq.u.essid.pointer = (caddr_t) buf;
    wrq.u.essid.length = strlen(buf);    
    //if (WIRELESS_EXT < 21)
    //    wrq.u.essid.length++;

    if (ioctl(skfd, SIOCSIWESSID, &wrq) < 0) {
        perror("ioctl(SIOCSIWESSID)");
        ret = -1;
        goto exit;
    }

exit:
    
    return  ret;
}

int wifi_disconnect()
{
    int i = 0;
    char ssid[8];

    LOGE(TAG "wifi_disconnect\n");
    LOGD(TAG "Let's start.\n");	
    for(i = 0; i < 7; i++)
    	ssid[i] = rand() & 0xff;
    ssid[7] = '\0';
    wifi_connect(ssid);	

    return 0;
} 
 
int wifi_fm_test()
{
    wireless_scan *	item;		
    char ctemp[40];
    char ssid[33];
    unsigned char mac[6];
    wireless_scan_head scanlist;
    wireless_scan * ap = NULL;
    wireless_info	wlan_info;        
    int fixed_ssid = -1;
    int ret = 0;
    
    if( get_info( skfd, "wlan0", &wlan_info) < 0 ) {
        LOGE("[wifi_select_ap] failed to get wlan0 info!\n");	     
        if( g_output_buf ) {
        	  memset(g_output_buf,0,g_output_buf_len);
            sprintf(g_output_buf, "[ERROR] can't get wlan info\n");
        }
        return -1;	        	        
    }
    
    if( iw_scan(skfd, "wlan0", 21, &scanlist) <0 ) {
        LOGE("[wifi_select_ap] failed to scan!\n");   
        if( g_output_buf ) {
        	  memset(g_output_buf,0,g_output_buf_len);
            sprintf(g_output_buf, "[ERROR] scan failed\n");
        }
        return -1;	
    }
   
    if( scanlist.result == NULL ){
        LOGE("[wifi_select_ap] no scan result!\n");   
        if( g_output_buf ) {
        	  memset(g_output_buf,0,g_output_buf_len);
            sprintf(g_output_buf, "[WARN]no network avail.\n");
        }
        return 0;	
    }
    
    if( read_preferred_ssid(ssid,sizeof(ssid)) == 0 ) {
        fixed_ssid = 1;
        LOGD("[wifi_select_ap] use specified ssid!\n");   
    }
    
    
    for(item = scanlist.result; item!= NULL ;item=item->next){
#if 1    	
    	 LOGD("[wifi_select_ap] new STA +++++++++!\n");	   
     	  if(item->b.has_essid && strlen(item->b.essid)>0) {
            LOGD("[wifi_select_ap] SSID : %s!\n",item->b.essid);	       
    	  }   	 
    	 
    	  if(item->has_ap_addr) {
    	  	  char mac[6];
    	      memcpy(mac,item->ap_addr.sa_data,sizeof(mac));
            sprintf(ctemp, "mac : %02x-%02x-%02x-%02x-%02x-%02x", 
                mac[0], mac[1], mac[2], 
                mac[3], mac[4], mac[5]);
                
            LOGD("[wifi_select_ap] %s\n",ctemp);	       
    	  }
    	  
    	  if(item->b.has_mode) {
            LOGD("[wifi_select_ap] mode :  %s!\n",item->b.mode==2?"Infrastructure":
            	(item->b.mode==1?"Ad-hoc":"unknown"));	       
    	  }     
    	    	  
    	  if(item->has_stats) {
            LOGD("[wifi_select_ap] rssi : %d dBm\n", item->stats.qual.level - 0x100);
    	  }    	  
    	  if(item->b.has_freq) {          	  	  
    	  	  int channel = iw_freq_to_channel( item->b.freq/1000,&(wlan_info.range));
            if( channel < 0 )
                LOGD("[wifi_select_ap] invalid channel num\n");	       
            else
            	  LOGD("[wifi_select_ap] channel : %d!\n",channel);	 
    	  } 
   	    if(item->has_maxbitrate) {
            LOGD("[wifi_select_ap] rate : %dM!\n",item->maxbitrate.value/1000000);	       
    	  }    
#endif  
        if( fixed_ssid == 1 ) {
     	      if(item->b.has_essid && (strcmp( ssid, item->b.essid) == 0) ) {
                LOGD("[wifi_select_ap] find specified AP %s\n",ssid);	                
                ap = item;
                break;
    	      }   	     	
        }

		/* skip wrong SSID */
		if (item->b.has_essid && (strncmp(item->b.essid, "NVRAM WARNING: Err =", strlen("NVRAM WARNING: Err =")) != 0))
		{
			if( (item->b.key_flags & IW_ENCODE_DISABLED ) ) {
				if(!ap) {
					ap = item;
				} else if( ap->stats.qual.level < item->stats.qual.level){
					  ap = item; 	        	
				}
			}
		}
    }

    if(ap){
    	  ap_info apinfo;
    	  
    	  memcpy( apinfo.ssid, ap->b.essid,sizeof(ap->b.essid));
    	  memcpy(apinfo.mac,ap->ap_addr.sa_data,sizeof(apinfo.mac));    	  
    	  apinfo.mode = ap->b.mode;
    	  apinfo.channel = iw_freq_to_channel( ap->b.freq/1000,&(wlan_info.range));
    	  apinfo.rssi = (unsigned int)(ap->stats.qual.level) - 0x100;
    	  apinfo.rate = ap->maxbitrate.value/1000000;    	  
    	  apinfo.media_status = media_connecting;    	      	  
    	  
    	  update_Text_Info(&apinfo, g_output_buf, g_output_buf_len);
    	  
		usleep(2000000); /* avoid scan again before scan done */
        if( wifi_connect(ap->b.essid) < 0) {
            LOGE("[wifi_select_ap] wifi_connect failed\n");
            if( g_output_buf ) {
            	  memset(g_output_buf,0,g_output_buf_len);
                sprintf(g_output_buf, "[ERROR] connect failed\n");
            }    
            ret = -1;               	
        } 
    } else {
        LOGE("[wifi_select_ap] no suitable AP\n");
        if( g_output_buf ) {
        	  memset(g_output_buf,0,g_output_buf_len);
            sprintf(g_output_buf, "[WARN] no siutable AP\n");
        }   	
    }
    
    if( scanlist.result )
        free(scanlist.result);
    return 0;
}

int read_preferred_ssid(char * ssid, int len)
{
    char * temp;
    
	  temp = ftm_get_prop(WIFI_PROP_NAME);
      if(temp!=NULL)
	  if(temp != NULL && strlen(temp) < 33){
	      LOGD(TAG "[read_preferred_ssid] Find perferred ssid %s\n", temp);
	      memcpy(ssid , temp, strlen(temp)+sizeof(char));
	      return 0;
	  }
	  
    return -1;
}

void update_Text_Info(ap_info * pApInfo, char* output_buf, int buf_len)
{
    int i = 0;
    char *ptr;
        
    if(!pApInfo){
        LOGE("[update_Text_Info]invalid param\n");    
        return;
    }
 
  
    ptr = output_buf;

    ptr += sprintf(ptr, "%s : %s\n", 
        uistr_info_wifi_status,
        pApInfo->media_status==media__disconnect?uistr_info_wifi_disconnect: 
        (pApInfo->media_status==media_connecting?uistr_info_wifi_connecting:
        (pApInfo->media_status==media_connected?uistr_info_wifi_connected:uistr_info_wifi_unknown)));
    ptr += sprintf(ptr, "SSID : %s \n", pApInfo->ssid);
    ptr += sprintf(ptr, "MAC : %02x-%02x-%02x-%02x-%02x-%02x \n", 
        pApInfo->mac[0], pApInfo->mac[1], pApInfo->mac[2], 
        pApInfo->mac[3], pApInfo->mac[4], pApInfo->mac[5]);  
    ptr += sprintf(ptr, "%s : %s \n", uistr_info_wifi_mode, pApInfo->mode==2?uistr_info_wifi_infra:
            	(pApInfo->mode==1?uistr_info_wifi_adhoc:uistr_info_wifi_unknown));
    ptr += sprintf(ptr, "%s : %d \n", uistr_info_wifi_channel, pApInfo->channel);
    ptr += sprintf(ptr, "%s : %d dBm \n", uistr_info_wifi_rssi, pApInfo->rssi);
    ptr += sprintf(ptr, "%s: %d M \n", uistr_info_wifi_rate, pApInfo->rate);
    
    return;
}

 
int FM_WIFI_init(char* output_buf, int buf_len, int* p_result)
{
    struct sockaddr_nl local;	
    LOGD("[FM_WIFI_init]++\n");   

    if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        LOGE("[FM_WIFI_init] failed to open net socket\n");
        return -1;
    }  
    
    sPflink = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sPflink < 0) {
        LOGE("[FM_WIFI_init] failed socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
        close(skfd);
        skfd = -1;
        return -1;
    }
    
    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = RTMGRP_LINK;
    if (bind(sPflink, (struct sockaddr *) &local, sizeof(local)) < 0) {
        LOGE("[FM_WIFI_init] failed bind(netlink)");
        close(skfd);
        skfd = -1;
        close(sPflink);
        sPflink = -1;
        return -1;
    }

  
    if( wifi_load_driver() < 0 ) {
        LOGD("[FM_WIFI_init] wifi_load_driver failed!\n");
        close(skfd);
        skfd = -1;
        close(sPflink);
        sPflink = -1;        
        return -1;
    }   
    g_output_buf_len = buf_len;
    g_output_buf = output_buf;
    
    return 0;
}

 
int FM_WIFI_deinit(void)
{
    close(skfd);
    skfd = -1;
    close(sPflink);
    sPflink = -1;
    return wifi_unload_driver();
}
