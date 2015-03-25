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

#include <sys/stat.h>
#include <grp.h>
#include <stddef.h>
#include <cutils/sockets.h>
#include <android/log.h>
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
#include <linux/limits.h>
#include <cutils/sockets.h>
#include <cutils/memory.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <sys/endian.h>
#include <linux/serial.h> /* struct serial_struct  */
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <termios.h>
#include <sys/poll.h>
#include <linux/serial.h> /* struct serial_struct  */
#include <sched.h>
#include <netdb.h>
#include <pthread.h>
#include "os_linux.h"
#include "stp_dump.h"
#include "eloop.h"
#include <linux/netlink.h>
#include <sys/select.h>
#include <sys/types.h>


#define LOGE printf
#define LOGD printf

#define GENL_ID_CTRL    NLMSG_MIN_TYPE
#define GENL_HDRLEN     NLMSG_ALIGN(sizeof(struct genlmsghdr))

enum {
    CTRL_CMD_UNSPEC,
    CTRL_CMD_NEWFAMILY,
    CTRL_CMD_DELFAMILY,
    CTRL_CMD_GETFAMILY,
    CTRL_CMD_NEWOPS,
    CTRL_CMD_DELOPS,
    CTRL_CMD_GETOPS,
    CTRL_CMD_NEWMCAST_GRP,
    CTRL_CMD_DELMCAST_GRP,
    CTRL_CMD_GETMCAST_GRP, /* unused */
    __CTRL_CMD_MAX,
};
#define CTRL_CMD_MAX (__CTRL_CMD_MAX - 1)

enum {
    CTRL_ATTR_UNSPEC,
    CTRL_ATTR_FAMILY_ID,
    CTRL_ATTR_FAMILY_NAME,
    CTRL_ATTR_VERSION,
    CTRL_ATTR_HDRSIZE,
    CTRL_ATTR_MAXATTR,
    CTRL_ATTR_OPS,
    CTRL_ATTR_MCAST_GROUPS,
    __CTRL_ATTR_MAX,
};

#define CTRL_ATTR_MAX (__CTRL_ATTR_MAX - 1)

struct genlmsghdr {
    __u8    cmd;
    __u8    version;
    __u16   reserved;
};

#define GENLMSG_DATA(glh) ((void *)((int)NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))

typedef struct _tagGenericNetlinkPacket {
    struct nlmsghdr n;
    struct genlmsghdr g;
    char buf[512*16];
} GENERIC_NETLINK_PACKET, *P_GENERIC_NETLINK_PACKET;



////typedef int socklen_t;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
int stp_debug_level = MSG_MSGDUMP;

struct stp_ctrl_dst {
    struct stp_ctrl_dst *next;
    struct sockaddr_un addr;
    socklen_t addrlen;
    int debug_level;
    int errors;
};

struct ctrl_iface_priv {
    struct stp_dump *stp_d;
    int sock;
    struct stp_ctrl_dst *ctrl_dst;
};

struct trace_iface_priv {
    struct stp_dump *stp_d;
    int sock;
    struct {
        FILE *fp_pkt;
        FILE *fp_drv;
        FILE *fp_fw;
        FILE *fp_t32; 
    } log_f;
};

struct stp_dump {
    const char *ctrl_interface;
    const char *ifname;
    char *ctrl_interface_group;
    int wmt_fd;
    struct ctrl_iface_priv *ctrl_iface;
    struct trace_iface_priv *trace_iface;
};

void android_printf(int level, char *format, ...)
{
    if (level >= stp_debug_level) {
        va_list ap;
        if (level == MSG_ERROR) {
            level = ANDROID_LOG_ERROR;
        } else if (level == MSG_WARNING) {
            level = ANDROID_LOG_WARN;
        } else if (level == MSG_INFO) {
            level = ANDROID_LOG_INFO;
        } else {
            level = ANDROID_LOG_DEBUG;
        }
        va_start(ap, format);
        __android_log_vprint(level, "stp_dump", format, ap);
        va_end(ap);
    }
}



static void _stp_hexdump(int level, const char *title, const u8 *buf,
             size_t len, int show)
{
    size_t i;
    if (level < stp_debug_level)
        return;

    printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
    if (buf == NULL) {
        printf(" [NULL]");
    } else if (show) {
        for (i = 0; i < len; i++)
            printf(" %02x", buf[i]);
    } else {
        printf(" [REMOVED]");
    }
    printf("\n");
}

void stp_hexdump(int level, const char *title, const u8 *buf, size_t len)
{
    _stp_hexdump(level, title, buf, len, 1);
}

static void stp_dump_ctrl_iface_send(struct ctrl_iface_priv *priv,
                       int level, const char *buf,
                       size_t len);


static int stp_dump_ctrl_iface_attach(struct ctrl_iface_priv *priv,
                        struct sockaddr_un *from,
                        socklen_t fromlen)
{
    struct stp_ctrl_dst *dst;

    dst = os_zalloc(sizeof(*dst));
    if (dst == NULL)
        return -1;
    os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
    dst->addrlen = fromlen;
    dst->debug_level = MSG_INFO;
    dst->next = priv->ctrl_dst;
    priv->ctrl_dst = dst;
    stp_hexdump(MSG_DEBUG, "CTRL_IFACE monitor attached",
            (u8 *) from->sun_path,
            fromlen - offsetof(struct sockaddr_un, sun_path));
    return 0;
}

static int stp_dump_ctrl_iface_detach(struct ctrl_iface_priv *priv,
                        struct sockaddr_un *from,
                        socklen_t fromlen)
{
    struct stp_ctrl_dst *dst, *prev = NULL;

    dst = priv->ctrl_dst;
    while (dst) {
        if (fromlen == dst->addrlen &&
            os_memcmp(from->sun_path, dst->addr.sun_path,
                  fromlen - offsetof(struct sockaddr_un, sun_path))
            == 0) {
            if (prev == NULL)
                priv->ctrl_dst = dst->next;
            else
                prev->next = dst->next;
            os_free(dst);
            stp_hexdump(MSG_DEBUG, "CTRL_IFACE monitor detached",
                    (u8 *) from->sun_path,
                    fromlen -
                    offsetof(struct sockaddr_un, sun_path));
            return 0;
        }
        prev = dst;
        dst = dst->next;
    }
    return -1;
}

static int stp_dump_ctrl_iface_level(struct ctrl_iface_priv *priv,
                       struct sockaddr_un *from,
                       socklen_t fromlen,
                       char *level)
{
    struct stp_ctrl_dst *dst;

    stp_printf(MSG_DEBUG, "CTRL_IFACE LEVEL %s\n", level);

    dst = priv->ctrl_dst;
    while (dst) {
        if (fromlen == dst->addrlen &&
            os_memcmp(from->sun_path, dst->addr.sun_path,
                  fromlen - offsetof(struct sockaddr_un, sun_path))
            == 0) {
            stp_hexdump(MSG_DEBUG, "CTRL_IFACE changed monitor "
                    "level", (u8 *) from->sun_path,
                    fromlen -
                    offsetof(struct sockaddr_un, sun_path));
            dst->debug_level = atoi(level);
            return 0;
        }
        dst = dst->next;
    }

    return -1;
}

static void stp_dump_ctrl_iface_receive(int sock, void *eloop_ctx,
                          void *sock_ctx)
{
    struct stp_dump *stp_d = eloop_ctx;  
    struct ctrl_iface_priv *priv = sock_ctx;
    char buf[256];
    int res;
    struct sockaddr_un from;
    socklen_t fromlen = sizeof(from);
    char *reply = NULL;
    size_t reply_len = 0;
    int new_attached = 0;

    res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
               (struct sockaddr *) &from, &fromlen);
    if (res < 0) {
        perror("recvfrom(ctrl_iface)");
        return;
    }
    buf[res] = '\0';

    if (os_strcmp(buf, "ATTACH") == 0) {
        if (stp_dump_ctrl_iface_attach(priv, &from, fromlen))
            reply_len = 1;
        else {
            new_attached = 1;
            reply_len = 2;
        }
    } else if (os_strcmp(buf, "DETACH") == 0) {
        if (stp_dump_ctrl_iface_detach(priv, &from, fromlen))
            reply_len = 1;
        else
            reply_len = 2;
    } else if (os_strncmp(buf, "LEVEL ", 6) == 0) {
        if (stp_dump_ctrl_iface_level(priv, &from, fromlen,
                            buf + 6))
            reply_len = 1;
        else
            reply_len = 2;
    } else {
        //stp_hexdump(MSG_INFO, "stp_dump_ctrl_iface_process", buf, 4);
        //reply = stp_dump_ctrl_iface_process(stp_d, buf,
        //                      &reply_len);
    }

    if (reply) {
        sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
               fromlen);
        os_free(reply);
    } else if (reply_len == 1) {
        sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
               fromlen);
    } else if (reply_len == 2) {
        sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &from,
               fromlen);
    }
}


static char * stp_dump_ctrl_iface_path(struct stp_dump *stp_d)
{
    char *buf;
    size_t len;
    char *pbuf, *dir = NULL, *gid_str = NULL;
    int res;

    if (stp_d->ctrl_interface == NULL)
    {
        stp_printf(MSG_ERROR, "stp_d->ctrl_interface = NULL\n");
        return NULL;    
    }
    pbuf = os_strdup(stp_d->ctrl_interface);

    if (pbuf == NULL)
    {
        stp_printf(MSG_ERROR, "copy string failed\n");
        return NULL;
    }
    if (os_strncmp(pbuf, "DIR=", 4) == 0) {
        dir = pbuf + 4;
        gid_str = os_strstr(dir, " GROUP=");
        if (gid_str) {
            *gid_str = '\0';
            gid_str += 7;
        }
    } else
        dir = pbuf;
    
    len = os_strlen(dir) + os_strlen(stp_d->ifname) + 2;
    buf = os_malloc(len);
    if (buf == NULL) 
    {
        stp_printf(MSG_ERROR, "memory allocation failed\n");
        os_free(pbuf);
        return NULL;
    }

    res = os_snprintf(buf, len, "%s/%s", dir, stp_d->ifname);
    if (res < 0 || (size_t) res >= len) 
    {
        stp_printf(MSG_ERROR, "os_snprintf failed\n");
        os_free(pbuf);
        os_free(buf);
        return NULL;
    }
    os_free(pbuf);
    return buf;
}

static void stp_dump_ctrl_iface_msg_cb(void *ctx, int level,
                         const char *txt, size_t len)
{
    struct stp_dump *stp_d = ctx;
    if (stp_d == NULL || stp_d->ctrl_iface == NULL)
        return;
    stp_dump_ctrl_iface_send(stp_d->ctrl_iface, level, txt, len);
}

int stp_dump_ctrl_iface_trace_logger_init(struct trace_iface_priv *tr_priv){
    
    tr_priv->log_f.fp_t32 = fopen("/data/combo_t32.cmm", "a+");
    if(tr_priv->log_f.fp_t32 == NULL){
        stp_printf(MSG_ERROR, "create combo_fw.log fails, exit\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int stp_dump_ctrl_iface_trace_logger_decode(int sock, void *eloop_ctx,
                          void *sock_ctx)
{
    struct nlattr *na;
    char *stpmsghdr = NULL;
    char *packet_raw = NULL;      
    int size;
    struct trace_iface_priv *tr_priv = sock_ctx;
    GENERIC_NETLINK_PACKET mBuffer;
    GENERIC_NETLINK_PACKET *prBuffer;
    
    if ((size = recv(sock, &mBuffer, sizeof(mBuffer), 0)) < 0) 
    {
        LOGE("recv failed (%s)\n", strerror(errno));
        return -1;
    }
    
    prBuffer = &mBuffer;

    /* Validate response message */
    if (!NLMSG_OK(&(prBuffer->n), (unsigned int)size))
    {
        LOGE("invalid reply message\n");
        return -1;
    }
    else if (prBuffer->n.nlmsg_type == NLMSG_ERROR) 
    { /* error */
        LOGE("received error\n");
        return -1;
    }
    else if (!NLMSG_OK((&prBuffer->n), (unsigned int)size)) 
    {
        LOGE("invalid reply message received via Netlink\n");
        return -1;
    }

    size = GENLMSG_PAYLOAD(&prBuffer->n);
    na = (struct nlattr *) GENLMSG_DATA(prBuffer);
    
    stpmsghdr = (char *)NLA_DATA(na);
    
    if( (stpmsghdr[0] == '[') &&
         (stpmsghdr[1] == 'M' &&
          (stpmsghdr[2] == ']')
        )) { 
        printf("=>[M][%s]\n", &stpmsghdr[3]);
               

        {
            static char start_dump = 1;

            //TODO: parsing message to know the action to start dump
            if(start_dump)
            {
                fclose(tr_priv->log_f.fp_t32);
                tr_priv->log_f.fp_t32 = 0;
                tr_priv->log_f.fp_t32 = fopen("/data/combo_t32.cmm", "w");
                fprintf(tr_priv->log_f.fp_t32, "%s",  &stpmsghdr[3]);
                fflush(tr_priv->log_f.fp_t32); 
                ioctl(tr_priv->stp_d->wmt_fd, 10, &stpmsghdr[3]);
                start_dump = 0;
            }
            else
            {
                fprintf(tr_priv->log_f.fp_t32, "%s",  &stpmsghdr[3]);
                fflush(tr_priv->log_f.fp_t32); 
            }
            if(strstr(&stpmsghdr[3], "coredump end"))
            {
                //inform user to dump action is done
                ioctl(tr_priv->stp_d->wmt_fd, 11, 0);
                start_dump = 1;    
            }
        }
    }
    else
    {
        LOGE("invalid dump data\n");
    }
    
    return 0;
}

int stp_dump_ctrl_iface_trace_logger_sendto_fd(int s, const char *buf, int bufLen)
{
    struct sockaddr_nl nladdr;
    int r;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    while ((r = sendto(s, buf, bufLen, 0, (struct sockaddr *) &nladdr,
                    sizeof(nladdr))) < bufLen) 
    {
        if (r > 0) 
        {
            buf += r;
            bufLen -= r;
        } 
        else if (errno != EAGAIN)
        {
            stp_printf(MSG_ERROR, "%s failed\n", __func__); 
            return -1;
        }
    }
	stp_printf(MSG_INFO, "%s succeed\n", __func__); 
    return 0;
}

/*
 * Probe the controller in genetlink to find the family id
 */
int stp_dump_ctrl_iface_trace_logger_get_family_id(int sk, const char *family_name)
{
    struct nlattr *na;
    int rep_len;
    int id = -1;
    GENERIC_NETLINK_PACKET family_req, ans;

    /* Get family name */
    family_req.n.nlmsg_type = GENL_ID_CTRL;
    family_req.n.nlmsg_flags = NLM_F_REQUEST;
    family_req.n.nlmsg_seq = 0;
    family_req.n.nlmsg_pid = getpid();
    family_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    family_req.g.cmd = CTRL_CMD_GETFAMILY;
    family_req.g.version = 0x1;

    na = (struct nlattr *) GENLMSG_DATA(&family_req);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len = strlen(family_name) + 1 + NLA_HDRLEN;
    strcpy((char *)NLA_DATA(na), family_name);
 
    family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (stp_dump_ctrl_iface_trace_logger_sendto_fd(sk, (char *) &family_req, family_req.n.nlmsg_len) < 0) 
    {
        stp_printf(MSG_ERROR, "%s failed\n", __func__); 
        return -1;
    }

    rep_len = recv(sk, &ans, sizeof(ans), 0);
    if (rep_len < 0)
    {
        stp_printf(MSG_ERROR, "no response\n");
        return -1;
    }
    /* Validate response message */
    else if (!NLMSG_OK((&ans.n), (unsigned int)rep_len))
    {
        stp_printf(MSG_ERROR,"invalid reply message\n");
        return -1;
    }
    else if (ans.n.nlmsg_type == NLMSG_ERROR) 
    { /* error */
        stp_printf(MSG_ERROR, "received error\n");
        return -1;
    }

    na = (struct nlattr *) GENLMSG_DATA(&ans);
    na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
    if (na->nla_type == CTRL_ATTR_FAMILY_ID) 
    {
        id = *(__u16 *) NLA_DATA(na);
    }

    return id;
}

int stp_dump_ctrl_iface_trace_logger_start(struct trace_iface_priv *tr_priv){
    
    struct sockaddr_nl nladdr;
    int sz = 64 * 1024;
    GENERIC_NETLINK_PACKET ans, req;
    struct nlattr *na;
    int id;
    int mlength = 14;
    const char *message = "HELLO"; //message
    int rc;
    int count = 100;
      
    stp_dump_ctrl_iface_trace_logger_init(tr_priv);

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_pid = getpid();
    nladdr.nl_groups = 0xffffffff;

    if ((tr_priv->sock= socket(AF_NETLINK,
                        SOCK_RAW,NETLINK_GENERIC)) < 0) 
    {
        stp_printf(MSG_ERROR, "Unable to create uevent socket: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(tr_priv->sock, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) < 0) 
    {
        stp_printf(MSG_ERROR,"Unable to set uevent socket options: %s", strerror(errno));
        return -1;
    }

    if (bind(tr_priv->sock, (struct sockaddr *) &nladdr, sizeof(nladdr)) < 0) 
    {
        stp_printf(MSG_ERROR,"Unable to bind uevent socket: %s", strerror(errno));
        return -1;
    }

    while(count--) 
    {
        id = stp_dump_ctrl_iface_trace_logger_get_family_id(tr_priv->sock, "STP_DBG");
        if (-1 == id) 
        {
            stp_printf(MSG_ERROR,"Unable to get family id, Retry");
            sleep(3);
        } 
        else 
        {
            stp_printf(MSG_ERROR,"[STP_DBG] family id = %d\n", id);
            printf("[STP_DBG] family id = %d\n", id);
            break;
        }
    }
    
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = id;
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_seq = 60;
    req.n.nlmsg_pid = getpid();
    req.g.cmd = 1; 
    
    na = (struct nlattr *) GENLMSG_DATA(&req);
    na->nla_type = 1; //MTK_WIFI_ATTR_MSG
    na->nla_len = mlength + NLA_HDRLEN; //message length
    memcpy(NLA_DATA(na), message, strlen(message));
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    stp_printf(MSG_INFO, "sending dummy command\n");
    
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    rc = sendto(tr_priv->sock, (char *)&req, req.n.nlmsg_len, 0,
            (struct sockaddr *) &nladdr, sizeof(nladdr));
#if 0
    if (rc > 0) 
    {
        stp_printf(MSG_INFO, "sending dummy command okay\n");
    }
    else if (errno != EAGAIN)
    {
        stp_printf(MSG_ERROR, "%s failed\n", __func__); 
        return -1;
    }    
#endif
    eloop_register_read_sock(tr_priv->sock, stp_dump_ctrl_iface_trace_logger_decode,
                 tr_priv->stp_d, tr_priv);    
    return 0;
}

struct ctrl_iface_priv *
stp_dump_ctrl_iface_init(struct stp_dump *stp_d)
{
    struct ctrl_iface_priv *priv;
    struct trace_iface_priv *tr_priv;
    struct sockaddr_un addr;
    char *fname = NULL;
    gid_t gid = 0;
    int gid_set = 0;
    char *buf, *dir = NULL, *gid_str = NULL;
    struct group *grp;
    char *endp;

    priv = os_zalloc(sizeof(*priv));
    if (priv == NULL)
    {   
        stp_printf(MSG_ERROR, "memory allocation for priv failed\n");
        return NULL;
    }

    tr_priv = os_zalloc(sizeof(*tr_priv));
    if(tr_priv == NULL)
    {
        stp_printf(MSG_ERROR, "memory allocation for tr_priv failed\n");
        return NULL;
    }

    priv->stp_d = stp_d;
    priv->sock = -1;

    tr_priv->stp_d = stp_d;
    tr_priv->sock = -1;

    if (stp_d->ctrl_interface == NULL)
    {   
        stp_printf(MSG_ERROR, "stp_d->ctrl_interface = NULL\n");
        return priv;
    }

    buf = os_strdup(stp_d->ctrl_interface);
    if (buf == NULL)
    {
        stp_printf(MSG_ERROR, "os_strdup\n");
        goto fail;
    }

    stp_dump_ctrl_iface_trace_logger_start(tr_priv);

    os_snprintf(addr.sun_path, sizeof(addr.sun_path), "stp_%s",
            stp_d->ctrl_interface);
    stp_printf(MSG_INFO, "addr.sun_path:%s\n", addr.sun_path);
    priv->sock = android_get_control_socket(addr.sun_path);
    if (priv->sock >= 0)
    {
        stp_printf(MSG_INFO, "priv->sock already exist\n");
        goto havesock;
    }
    if (os_strncmp(buf, "DIR=", 4) == 0) {
        dir = buf + 4;
        gid_str = os_strstr(dir, " GROUP=");
        if (gid_str) {
            *gid_str = '\0';
            gid_str += 7;
        }
    } else {
        dir = buf;
        gid_str = stp_d->ctrl_interface_group;
    }

    if (mkdir(dir, S_IRWXU | S_IRWXG) < 0) 
    {
        if (errno == EEXIST) 
        {
            stp_printf(MSG_INFO, "Using existing control "
                   "interface directory.\n");
        } 
        else 
        {
            stp_printf(MSG_ERROR, "mkdir (%s) failed\n", dir);
            perror("mkdir[ctrl_interface]");
            goto fail;
        }
    }
    stp_printf(MSG_ERROR, "mkdir (%s) succeed\n", dir);
    if (gid_str) 
    {
        grp = getgrnam("system");
        if (grp) 
        {
            gid = grp->gr_gid;
            gid_set = 1;
            stp_printf(MSG_INFO, "ctrl_interface_group=%d"
                   " (from group name '%s')\n",
                   (int) gid, gid_str);
        } 
        else 
        {
            /* Group name not found - try to parse this as gid */
            gid = strtol(gid_str, &endp, 10);
            if (*gid_str == '\0' || *endp != '\0') {
                stp_printf(MSG_ERROR, "CTRL: Invalid group "
                       "'%s'\n", gid_str);
                goto fail;
            }
            gid_set = 1;
            stp_printf(MSG_INFO, "ctrl_interface_group=%d\n",
                   (int) gid);
        }
    }

    if (gid_set && chown(dir, -1, gid) < 0) 
    {
        perror("chown[ctrl_interface]");
		stp_printf(MSG_ERROR, "chown (%s) failed\n", dir);
        goto fail;
    }
    stp_printf(MSG_INFO, "chown (%s) succeed\n", dir);
	
    /* Make sure the group can enter and read the directory */
    if (gid_set &&
        chmod(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP) < 0) 
    {
        stp_printf(MSG_ERROR, "CTRL: chmod[%s]: %s\n", dir, strerror(errno));
        goto fail;
    }
    stp_printf(MSG_INFO, "CTRL: chmod[%s] succeed\n", dir);
    if (os_strlen(dir) + 1 + os_strlen(stp_d->ifname) >=
        sizeof(addr.sun_path)) 
    {
        stp_printf(MSG_ERROR, "ctrl_iface path limit exceeded\n");
        goto fail;
    }


    priv->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (priv->sock < 0) 
    {
        perror("socket(PF_UNIX)");
		stp_printf(MSG_ERROR, "create socket failed\n");
        goto fail;
    }

    os_memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    fname = stp_dump_ctrl_iface_path(stp_d);

    if (fname == NULL)
    {
        stp_printf(MSG_ERROR, "stp_dump_ctrl_iface_path failed\n");
        goto fail;
    }
    os_strlcpy(addr.sun_path, fname, sizeof(addr.sun_path));
    if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        stp_printf(MSG_ERROR, "ctrl_iface bind(PF_UNIX) failed: %s\n",
               strerror(errno));
        if (connect(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            stp_printf(MSG_ERROR, "ctrl_iface exists, but does not"
                   " allow connections - assuming it was left"
                   "over from forced program termination\n");
            if (unlink(fname) < 0) {
                perror("unlink[ctrl_iface]");
                stp_printf(MSG_ERROR, "Could not unlink "
                       "existing ctrl_iface socket '%s'\n",
                       fname);
                goto fail;
            }
            if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                perror("bind(PF_UNIX)");
                goto fail;
            }
            stp_printf(MSG_DEBUG, "Successfully replaced leftover " "ctrl_iface socket '%s'\n", fname);
        } else {
            stp_printf(MSG_INFO, "ctrl_iface exists and seems to " "be in use - cannot override it\n");
            stp_printf(MSG_INFO, "Delete '%s' manually if it is "
                   "not used anymore\n", fname);
            os_free(fname);
            fname = NULL;
            goto fail;
        }
    }
    if (gid_set && chown(fname, -1, gid) < 0) {
        perror("chown[ctrl_interface/ifname]");
		stp_printf(MSG_ERROR, "chown(%s) failed, ", fname);
        goto fail;
    }

    if (chmod(fname, S_IRWXU | S_IRWXG) < 0) {
        perror("chmod[ctrl_interface/ifname]");
		stp_printf(MSG_ERROR, "chmod(%s) failed, ", fname);
        goto fail;
    }
    os_free(fname);

    /* open wmt dev */
    stp_d->wmt_fd = open("/dev/stpwmt", O_RDWR | O_NOCTTY);
    if (stp_d->wmt_fd < 0) {
        stp_printf(MSG_ERROR, "[%s] Can't open stpwmt \n", __FUNCTION__);
        goto fail;
    }

havesock:
    eloop_register_read_sock(priv->sock, stp_dump_ctrl_iface_receive,
                 stp_d, priv);

    os_free(buf);
    return priv;

fail:
    if (priv->sock >= 0)
        close(priv->sock);
    os_free(priv);
    if (fname) {
        unlink(fname);
        os_free(fname);
    }
    os_free(buf);
    return NULL;
}


void stp_dump_ctrl_iface_deinit(struct ctrl_iface_priv *priv)
{
#if 0
    struct wpa_ctrl_dst *dst, *prev;
#endif
    struct stp_ctrl_dst *dst, *prev;
    stp_printf(MSG_INFO, "%s\n", __func__);
    os_sleep(1, 0);
    /* close wmt dev */
    if (priv->stp_d->wmt_fd >= 0) {
        close(priv->stp_d->wmt_fd);
    }

    if (priv->sock > -1) {
        char *fname;
        char *buf, *dir = NULL, *gid_str = NULL;
        eloop_unregister_read_sock(priv->sock);
        if (priv->ctrl_dst) {
            /*
             * Wait a second before closing the control socket if
             * there are any attached monitors in order to allow
             * them to receive any pending messages.
             */
            stp_printf(MSG_DEBUG, "CTRL_IFACE wait for attached "
                   "monitors to receive messages\n");
            os_sleep(1, 0);
        }
        close(priv->sock);
        priv->sock = -1;
#if 0        
        fname = wpa_supplicant_ctrl_iface_path(priv->wpa_s);
#endif
        fname = stp_dump_ctrl_iface_path(priv->stp_d);
        if (fname) {
            unlink(fname);
            os_free(fname);
        }
#if 0
        buf = os_strdup(priv->wpa_s->conf->ctrl_interface);
#endif  
        buf = os_strdup(priv->stp_d->ctrl_interface);
        if (buf == NULL)
            goto free_dst;
        if (os_strncmp(buf, "DIR=", 4) == 0) {
            dir = buf + 4;
            gid_str = os_strstr(dir, " GROUP=");
            if (gid_str) {
                *gid_str = '\0';
                gid_str += 7;
            }
        } else
            dir = buf;

        if (rmdir(dir) < 0) {
            if (errno == ENOTEMPTY) {
                stp_printf(MSG_DEBUG, "Control interface "
                       "directory not empty - leaving it "
                       "behind\n");
            } else {
                perror("rmdir[ctrl_interface]");
            }
        }
        os_free(buf);
    }

free_dst:
    dst = priv->ctrl_dst;
    while (dst) {
        prev = dst;
        dst = dst->next;
        os_free(prev);
    }
    os_free(priv);
}


/**
 * wpa_supplicant_ctrl_iface_send - Send a control interface packet to monitors
 * @priv: Pointer to private data from wpa_supplicant_ctrl_iface_init()
 * @level: Priority level of the message
 * @buf: Message data
 * @len: Message length
 *
 * Send a packet to all monitor programs attached to the control interface.
 */
static void stp_dump_ctrl_iface_send(struct ctrl_iface_priv *priv,
                       int level, const char *buf,
                       size_t len)
{
    struct stp_ctrl_dst *dst, *next;
    char levelstr[10];
    int idx, res;
    struct msghdr msg;
    struct iovec io[2];

    dst = priv->ctrl_dst;
    if (priv->sock < 0 || dst == NULL)
        return;

    res = os_snprintf(levelstr, sizeof(levelstr), "<%d>", level);
    if (res < 0 || (size_t) res >= sizeof(levelstr))
        return;
    io[0].iov_base = levelstr;
    io[0].iov_len = os_strlen(levelstr);
    io[1].iov_base = (char *) buf;
    io[1].iov_len = len;
    os_memset(&msg, 0, sizeof(msg));
    msg.msg_iov = io;
    msg.msg_iovlen = 2;

    idx = 0;
    while (dst) {
        next = dst->next;
        if (level >= dst->debug_level) {
            stp_hexdump(MSG_DEBUG, "CTRL_IFACE monitor send",
                    (u8 *) dst->addr.sun_path, dst->addrlen -
                    offsetof(struct sockaddr_un, sun_path));
            msg.msg_name = (void *) &dst->addr;
            msg.msg_namelen = dst->addrlen;
            if (sendmsg(priv->sock, &msg, 0) < 0) {
                int _errno = errno;
                stp_printf(MSG_INFO, "CTRL_IFACE monitor[%d]: "
                       "%d - %s\n",
                       idx, errno, strerror(errno));
                dst->errors++;
                if (dst->errors > 10 || _errno == ENOENT) {
                    stp_dump_ctrl_iface_detach(
                        priv, &dst->addr,
                        dst->addrlen);
                }
            } else
                dst->errors = 0;
        }
        idx++;
        dst = next;
    }
}


void stp_dump_ctrl_iface_wait(struct ctrl_iface_priv *priv)
{
    char buf[256];
    int res;
    struct sockaddr_un from;
    socklen_t fromlen = sizeof(from);

    for (;;) {
        stp_printf(MSG_DEBUG, "CTRL_IFACE - %s - wait for monitor to "
               "attach\n", priv->stp_d->ifname);
        eloop_wait_for_read_sock(priv->sock);

        res = recvfrom(priv->sock, buf, sizeof(buf) - 1, 0,
                   (struct sockaddr *) &from, &fromlen);
        if (res < 0) {
            perror("recvfrom(ctrl_iface)");
            continue;
        }
        buf[res] = '\0';

        if (os_strcmp(buf, "ATTACH") == 0) {
            /* handle ATTACH signal of first monitor interface */
            if (!stp_dump_ctrl_iface_attach(priv, &from,
                                  fromlen)) {
                sendto(priv->sock, "OK\n", 3, 0,
                       (struct sockaddr *) &from, fromlen);
                /* OK to continue */
                return;
            } else {
                sendto(priv->sock, "FAIL\n", 5, 0,
                       (struct sockaddr *) &from, fromlen);
            }
        } else {
            /* return FAIL for all other signals */
            sendto(priv->sock, "FAIL\n", 5, 0,
                   (struct sockaddr *) &from, fromlen);
        }
    }
}

static void stp_dump_terminate(int sig, void *eloop_ctx,
                     void *signal_ctx)
{
    eloop_terminate();
}

int stp_dump_run(void)
{
    eloop_register_signal_terminate(stp_dump_terminate, NULL);
    eloop_run();
    return 0;
}

void set_sched_prio(void)
{
    struct sched_param sched, test_sched;
    int policy = 0xff;
    int err=0xff;

    policy = sched_getscheduler(0);
    sched_getparam(0, &test_sched);
    printf("Before %s policy = %d, priority = %d\n", "main" , policy, test_sched.sched_priority);

    sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
    err = sched_setscheduler(0, SCHED_FIFO, &sched);
    if(err == 0){
         printf("pthread_setschedparam SUCCESS \n");
         policy = sched_getscheduler(0);
         sched_getparam(0, &test_sched);
         printf("After %s policy = %d, priority = %d\n", "main" ,policy , test_sched.sched_priority);
    }
    else{
         if(err == EINVAL) printf("policy is not one of SCHED_OTHER, SCHED_RR, SCHED_FIFO\n");
         if(err == EINVAL) printf("the  priority  value  specified by param is not valid for the specified policy\n");
         if(err == EPERM) printf("the calling process does not have superuser permissions\n");
         if(err == ESRCH) printf("the target_thread is invalid or has already terminated\n");
         if(err == EFAULT)  printf("param points outside the process memory space\n");
         printf("pthread_setschedparam FAIL \n");
    }
}

int main(int argc, char *argv[]){

    struct stp_dump *stp_d = NULL;
    const char *conf ="DIR=/data/misc/stp_dump GROUP=stp";
    const char *ifname = "stpd";

    stp_printf(MSG_INFO, "==>%s \n", __func__ );
    stp_d = os_zalloc(sizeof(struct stp_dump));
    if (stp_d == NULL)
    {   
        stp_printf(MSG_ERROR, "memory allocation for stp_dump failed\n");
        return -1;
    }
    stp_d->ctrl_interface = conf;
    stp_d->ifname = ifname;

    stp_d->ctrl_iface = stp_dump_ctrl_iface_init(stp_d);
    if (stp_d->ctrl_iface == NULL) 
    {
        stp_printf(MSG_ERROR,
               "Failed to initialize control interface '%s'.\n"
               "You may have another stp_dump process "
               "already running or the file was\n"
               "left by an unclean termination of stp_dump "
               "in which case you will need\n"
               "to manually remove this file before starting "
               "wpa_supplicant again.\n",
               "used by stp_dump\n");
        return -1;
    } 
    else 
    {
        stp_printf(MSG_INFO, "stp_dump_ctrl_iface_init succeed.\n");
    }
    stp_printf(MSG_INFO, "==>%s222 \n", __func__ );

    //set_sched_prio();
   
    stp_dump_run();

    stp_dump_ctrl_iface_deinit(stp_d->ctrl_iface);
    
    os_free(stp_d);

    return 0;
}


