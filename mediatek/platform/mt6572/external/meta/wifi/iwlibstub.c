/*******************************************************************************
** Copyright (c) 2006 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
//#include <net/ethernet.h>
#include <sys/time.h>
#include <unistd.h>
#include <linux/socket.h>
#include <linux/if.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/types.h>
#ifndef __user
#define __user
#endif
#include "wireless.h"
#include "iwlibstub.h"

int openNetHandle(void){
   int sock;		/* generic raw socket desc.	*/


    /* Try to open the socket, if success returns it */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock >= 0)
        return sock;
    else
        return -1;
}

void closeNetHandle(int skfd){
    /* Close the socket. */
    close(skfd);
}

int enumNetIf(int skfd, _enum_handler fn, void* argc){
    struct  ifconf ifc;
    char    buff[1024];
    struct  ifreq *ifr;
    int     i, num = 0;

    ifc.ifc_len = sizeof(buff);
    ifc.ifc_buf = buff;
    if(ioctl(skfd, SIOCGIFCONF, &ifc) < 0)
    {
        fprintf(stderr, "SIOCGIFCONF: %s\n", strerror(errno));
        return num;
    }
    ifr = ifc.ifc_req;

    for(i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; ifr++){
        (*fn)(skfd,ifr->ifr_name, argc);
        num ++;
    }
    return num;
}


int setIWreq(int skfd, char* if_name,
    unsigned long ndisOid,
    unsigned char* ndisData,
    unsigned long bufLen,
    unsigned long* outputBufLen)
{
    struct iwreq		wrq;
    unsigned char *	buffer = NULL;
    unsigned int    reqBufLen = 0;
    NDIS_TRANSPORT_STRUCT*   reqStruct_p = NULL;
    int     result;

    reqBufLen = bufLen + sizeof(*reqStruct_p) -
        sizeof(reqStruct_p->ndisOidContent);

    buffer = (unsigned char *)malloc(reqBufLen);

    if (buffer == NULL) {
        printf("malloc(%d) fail\n", reqBufLen);
        return -1;
    }

    reqStruct_p = (NDIS_TRANSPORT_STRUCT*)buffer;

    reqStruct_p->ndisOidCmd = ndisOid;
    reqStruct_p->inNdisOidlength = bufLen;

    if (bufLen != 0) {
        memcpy(reqStruct_p->ndisOidContent, ndisData, bufLen);
    }

    /* Try to read the results */
    wrq.u.data.pointer = buffer;
    wrq.u.data.flags = PRIV_CMD_OID;
    wrq.u.data.length = reqBufLen;

#if 0
    printf("%s: buffer(0x%p), flags(%d), length(%d)\n",
        __FUNCTION__,
        buffer, PRIV_CMD_OID, reqBufLen
        );
#endif

    /* Set device name */
    strncpy(wrq.ifr_name, if_name, IFNAMSIZ);
    /* Do the request */
    result = ioctl(skfd, IOCTL_SET_STRUCT, &wrq);

    if (result < 0) {
        fprintf(stderr, "result %d %s\n", result, strerror(errno));
    }
    *outputBufLen = reqStruct_p->outNdisOidLength;

    free(buffer);

    if(result == 0)
        return 0;
    return -1;
}

int getIWreq(int skfd, char* if_name,
    unsigned long ndisOid,
    unsigned char* ndisData,
    unsigned long bufLen,
    unsigned long* outputBufLen)
{
    struct iwreq		wrq;
    unsigned char *	buffer = NULL;
    unsigned long   reqBufLen = 0;
    NDIS_TRANSPORT_STRUCT*   reqStruct_p = NULL;
    int     result;

    //printf("%s\n", __FUNCTION__);

    reqBufLen = bufLen + sizeof(*reqStruct_p) -
        sizeof(reqStruct_p->ndisOidContent);

    buffer = (unsigned char *)malloc(reqBufLen);

    if(buffer == NULL)
        return -1;

    reqStruct_p = (NDIS_TRANSPORT_STRUCT*)buffer;

    reqStruct_p->ndisOidCmd = ndisOid;
    reqStruct_p->inNdisOidlength = bufLen;

    /* IMPORTANT!! copy input data to buffer (ex. mcr index) */
    memcpy(reqStruct_p->ndisOidContent, ndisData, bufLen);

    /* Try to read the results */
    wrq.u.data.pointer = buffer;
    wrq.u.data.flags = PRIV_CMD_OID;
    wrq.u.data.length = reqBufLen;

    /* Set device name */
    strncpy(wrq.ifr_name, if_name, IFNAMSIZ);
    /* Do the request */
    result = ioctl(skfd, IOCTL_GET_STRUCT, &wrq);

    if(result < 0)
        fprintf(stderr, "getIWreq result %s %d %s\n", if_name, result, strerror(errno));

    if(result == 0){
        memcpy(ndisData, reqStruct_p->ndisOidContent, reqStruct_p->outNdisOidLength);
    }

    *outputBufLen = reqStruct_p->outNdisOidLength;

    free(buffer);

    if(result == 0)
        return 0;
    return -1;
}

