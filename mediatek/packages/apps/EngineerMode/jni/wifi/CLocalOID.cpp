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

/*
** $Id: CLocalOID.cpp,v 1.2 2008/06/04 09:04:33 MTK01385 Exp $
*/

/*******************************************************************************
** Copyright (c) 2005 - 2007 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

/*
** $Log: CLocalOID.cpp,v $
 *
 * 04 29 2011 xiao.liu
 * [ALPS00044734] [Need Patch] [Volunteer Patch][EM] resolve all build warning. alps.GB
 * warning. alps
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.2  2008/06/04 09:04:33  MTK01385
** 1. move setNdisOID(), queryNdisOID() and lookupNdiAdapter() to CLocalOID class.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.1  2007/11/12 03:19:00  MTK01267
** Initial version
**
**
*/


#include "COid.h"
#include <sys/socket.h>		    /* For AF_INET & struct sockaddr */
#include <netinet/in.h>         /* For struct sockaddr_in */
#include "param.h"
#include <sys/ioctl.h>

#ifdef ANDROID
//#include <linux/capability.h>
#include <sys/capability.h>
#include <linux/prctl.h>
#include <private/android_filesystem_config.h>
#endif

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

namespace android{

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
static int wifi_rfkill_id = -1;
static char *wifi_rfkill_state_path = NULL;

int os_program_init(void);
/*******************************************************************************
**  setNdisOID
**
**  descriptions: Set NDIS OID
**  parameters:
**  return:
**		TRUE: Successful, FALSE: otherwise
**  note:
*******************************************************************************/
BOOL 
setNdisOID ( 
    HANDLE  m_handle,
    const TCHAR * ptcDeviceName, 
    UINT_32 oid, 
    CHAR * setBuffer, 
    UINT_32 bufLen)
{
#ifdef HAVE_ANDROID_OS
	NDIS_TRANSPORT_STRUCT	ndis_ts;
	iwreq	iwr;
	BOOL ret = FALSE;
	int s = -1;
	int length;
	CHAR* str;
	assert(ptcDeviceName);
	//em_printf(MSG_DEBUG, "%s", __FUNCTION__);
	//assert(setBuffer);
	if(setBuffer == NULL && bufLen != 0)
	{
		em_error((char*)"%s:assert failed, setBuffer = %d, bufLen = %d", __FUNCTION__, setBuffer, bufLen); 
		return false;
	}
	memset(&iwr, 0, sizeof(iwr));
	strcpy(iwr.ifr_ifrn.ifrn_name, ptcDeviceName);

	memset(&ndis_ts, sizeof(ndis_ts), 0);
	ndis_ts.ndisOidCmd = oid;
	
	if(bufLen <= 64){			
		ndis_ts.inNdisOidlength = bufLen;
		
		if(bufLen != 0)
		{
			memcpy(ndis_ts.ndisOidContent, setBuffer, bufLen);
			em_printf(MSG_DEBUG, (char*)"%s:begin dump setBuffer, setBuffer = 0x%x, length = %d",__FUNCTION__, setBuffer, bufLen);
			em_dump(setBuffer, bufLen);// for test use
			android_printf(MSG_DUMP, "end dump setBuffer");
		}
	}else{
		em_error((char*)"%s bufLen is out of range", __FUNCTION__);
		return FALSE;
	}
	
	length = sizeof(ndis_ts);
    str = (CHAR *)&ndis_ts;
#ifdef DEBUG_DRIVER	    
	em_printf(MSG_DEBUG, (char*)"begin dump ndis_ts, &ndis_ts = 0x%x, length = %d" , str, length);									
	em_dump(str, length);
	android_printf(MSG_DUMP, "end dump ndis_ts");
#endif	
	iwr.u.data.flags = PRIV_CMD_OID;
	iwr.u.data.pointer = &ndis_ts;
	iwr.u.data.length = sizeof(ndis_ts);
	
	if(m_handle < 0)
	{
		os_program_init();
		m_handle = socket(AF_INET, SOCK_DGRAM, 0);
		//m_handle = socket(PF_INET, SOCK_DGRAM, 0);
	}
	//edit by chaozhong@11.10
	if (m_handle < 0) {
        	em_error((char*)"%s:m_handle invalid", __FUNCTION__);
	        return -1;
    	}
    
    length = sizeof(iwr);
    str = (CHAR *)&iwr;
#ifdef DEBUG_DRIVER	    
    em_printf(MSG_DEBUG, (char*)"begin dump iwr, &iwr = 0x%x, length = %d", str, length);									
	em_dump(str, length);
	android_printf(MSG_DUMP, "end dump iwr");
#endif	
	if(ioctl(m_handle, IOCTL_SET_STRUCT_FOR_EM, &iwr) < 0){
		em_error((char*)"%s ioctl failed, %s", __FUNCTION__, strerror(errno));
		ret = FALSE;
	}else{
		em_printf(MSG_DEBUG, (char*)"%s:operation succeed", __FUNCTION__);
		ret = TRUE;
	}	
	return ret;	
#else
    PNDISUIO_SET_OID setOid;
    UINT_32 bytesconsumed;
    BOOL bValue = FALSE;
    INT_32 errorCode;
    UCHAR * buffer = NULL;

    DEBUGFUNC("CLocalOID::setNdisOID");
	INITLOG((_T("\n")));

    buffer = (UCHAR *)malloc(bufLen + FIELD_OFFSET(NDISUIO_SET_OID, Data));

    if( buffer == NULL) {
        return FALSE;
	}

    setOid = (PNDISUIO_SET_OID) buffer;

    setOid->Oid = oid;		// IPC Custom OID;
    setOid->ptcDeviceName = ptcDeviceName;         // You dont need this once you        have done a OPEN Device.

    if (bufLen) {
        memcpy(setOid->Data, setBuffer, bufLen);
    }

    bValue = DeviceIoControl (m_handle,
            IOCTL_NDISUIO_SET_OID_VALUE,
            (LPVOID) setOid,
            bufLen + FIELD_OFFSET(NDISUIO_SET_OID, Data),
            NULL,
            0,
            &bytesconsumed,
            NULL);

    if (bValue == 0) {
		errorCode = GetLastError();

		TCHAR output[256];

		_stprintf(output, _T("NDISUIO_SET_OID Dev name:%s Error Code:%d return value:%d return bytes:%d\n"),
		        ptcDeviceName,
		        errorCode,
		        bValue,
		        bytesconsumed);
		OutputDebugString(output);
    }

    if (buffer) {
		free(buffer);
	}
	if (bValue == 0) {
		return FALSE;
	}
    return TRUE;
#endif
}

/*******************************************************************************
**  queryNdisOID
**
**  descriptions: Query NDIS OID
**  parameters:
**  return:
**		TRUE: Successful, FALSE: otherwise
**  note:
*******************************************************************************/
BOOL 
queryNdisOID (
    HANDLE  m_handle,
    const TCHAR * ptcDeviceName, 
    UINT_32 oid, 
    CHAR * queryBuffer, 
    UINT_32 bufLen, 
    UINT_32 * outputLen
    )
{
#ifdef HAVE_ANDROID_OS
	
//	return getIWreq(m_handle, ptcDeviceName, oid, queryBuffer, bufLen, outputLen);
	NDIS_TRANSPORT_STRUCT	ndis_ts;
	iwreq	iwr;
	BOOL ret = FALSE;
	int length;
	CHAR* str;													

	//em_printf(MSG_DEBUG, "%s", __FUNCTION__);
	assert(ptcDeviceName);
	//assert(queryBuffer);

	if(queryBuffer == NULL && bufLen != 0)
	{
		em_error((char*)"%s:assert failed, queryBuffer = %d, bufLen = %d", __FUNCTION__, queryBuffer, bufLen); 
		return false;
	}
	memset(&iwr, 0, sizeof(iwr));
	if(strlen(ptcDeviceName) < IFNAMSIZ)
		strcpy(iwr.ifr_ifrn.ifrn_name, ptcDeviceName);
	else
		goto out;
	
	memset(&ndis_ts, sizeof(ndis_ts), 0);
	ndis_ts.ndisOidCmd = oid;
	if(bufLen <= 64){			
		ndis_ts.inNdisOidlength = bufLen;
		if(bufLen != 0)
		{
			memcpy(ndis_ts.ndisOidContent, queryBuffer, bufLen);
			em_printf(MSG_DEBUG, (char*)"%s:begin dump queryBuffer, queryBuffer = 0x%x, length = %d", __FUNCTION__, queryBuffer, bufLen);	
			em_dump(queryBuffer, bufLen);
			android_printf(MSG_DUMP, "end dump queryBuffer");
		}
	}else{
		em_error((char*)"%s: bufLen is out of range", __FUNCTION__);
		ret = FALSE;
		goto out;
	}
	
	length = sizeof(ndis_ts);
    str = (CHAR *)&ndis_ts;
#ifdef DEBUG_DRIVER	    
	em_printf(MSG_DEBUG, (char*)"begin dump ndis_ts, &ndis_ts = 0x%x, length = %d" , str, length);									
	em_dump(str, length);
	android_printf(MSG_DUMP, "end dump ndis_ts");
#endif	
	iwr.u.data.flags = PRIV_CMD_OID;
	iwr.u.data.pointer = &ndis_ts;
	iwr.u.data.length = sizeof(ndis_ts);
	
	length = sizeof(iwr);
	str = (CHAR *)&iwr;													
#ifdef DEBUG_DRIVER
	em_printf(MSG_DEBUG, (char*)"begin dump iwr, &iwr = 0x%x, length = %d" , str, length);
	em_dump(str, length);
	android_printf(MSG_DUMP, "end dump iwr");
#endif	
	if(ioctl(m_handle, IOCTL_GET_STRUCT, &iwr) < 0){
		em_error((char*)"%s ioctl failed, %s", __FUNCTION__, strerror(errno));
		ret = FALSE;
	}else{
		em_printf(MSG_DEBUG, (char*)"%s:operation succeed", __FUNCTION__);
		if(outputLen != NULL)
		{
			*outputLen = ndis_ts.outNdisOidLength;
		}
		memcpy(queryBuffer, ndis_ts.ndisOidContent, ndis_ts.outNdisOidLength);
		ret = TRUE;
	}
out:	
	return ret;
#else
    PNDISUIO_QUERY_OID queryOid;
    UINT_32 bytesconsumed;
    BOOL bValue = FALSE;
    INT_32 errorCode;
    UCHAR * buffer = NULL;

    DEBUGFUNC("CLocalOID::queryNdisOID");
	INITLOG((_T("\n")));

    buffer = (UCHAR*)malloc(bufLen + FIELD_OFFSET(NDISUIO_SET_OID, Data));

    if (buffer == NULL)
        return FALSE;

    queryOid = (PNDISUIO_QUERY_OID) buffer;

    queryOid->Oid = oid; // IPC Custom OID;
    queryOid->ptcDeviceName = ptcDeviceName; // You dont need this once you have done a OPEN Device.
    memcpy(queryOid->Data, queryBuffer, bufLen);

    bValue = DeviceIoControl (m_handle,
		        IOCTL_NDISUIO_QUERY_OID_VALUE,
		        (LPVOID) queryOid,
		        bufLen + FIELD_OFFSET(NDISUIO_SET_OID, Data),
		        (LPVOID) queryOid,
		        bufLen + FIELD_OFFSET(NDISUIO_SET_OID, Data),
		        &bytesconsumed,
		        NULL);

    if (bValue == 0) {
		errorCode = GetLastError();

		TCHAR output[256];
		_stprintf(output, _T("queryNdisOID Dev name:%s Error Code:%d Byte %d\n"),
		        ptcDeviceName,
		        errorCode,
		        bytesconsumed);
		OutputDebugString(output);

		/* Because NDIS UIO's bug, there is no way to know how many bytes are required */
		*outputLen = 0;
    }
    else {
        memcpy(queryBuffer, queryOid->Data, bytesconsumed - FIELD_OFFSET(NDISUIO_SET_OID, Data));
        *outputLen = bytesconsumed - FIELD_OFFSET(NDISUIO_SET_OID, Data);
    }

    if (buffer) {
		free(buffer);
	}
	if (bValue == 0) {
		return FALSE;
	}

    return TRUE;
#endif
}




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
				em_error((char*)"open(%s) failed: %s (%d)\n", path, strerror(errno), errno);
				return -1;
			}
			sz = read(fd, &buf, sizeof(buf));
			close(fd);
			if (sz >= 4 && memcmp(buf, "wlan", 4) == 0) {
				wifi_rfkill_id = id;
				break;
			}
		}
	
		asprintf(&wifi_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", wifi_rfkill_id);
		return 0;
}

static int wifi_set_power(int on) 
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) goto out;
    }

    fd = open(wifi_rfkill_state_path, O_WRONLY);
    if (fd < 0) {
        em_error((char*)"open(%s) for write failed: %s (%d)", wifi_rfkill_state_path,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        em_error((char*)"write(%s) failed: %s (%d)", wifi_rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}

#define BTWLAN_DEVNAME         "/dev/btwlan_em"
#define BTWLAN_EM_IOC_MAGIC     0xf6
#define BTWLAN_EM_IOCTL_SET_BTPWR    _IOWR(BTWLAN_EM_IOC_MAGIC, 0, uint32_t)
#define BTWLAN_EM_IOCTL_SET_WIFIPWR  _IOWR(BTWLAN_EM_IOC_MAGIC, 1, uint32_t)

static int wifi_set_power_ext(int on)
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');
    const char *btwlan = BTWLAN_DEVNAME;

    fd = open(btwlan, O_WRONLY);
    if (fd < 0) {
        em_error((char*)"open(%s) for write failed: %s (%d)", btwlan,
             strerror(errno), errno);
        goto out;
    }

    sz = ioctl(fd, BTWLAN_EM_IOCTL_SET_WIFIPWR, &on);;
    if (sz < 0) {
        em_error((char*)"write(%s) failed: %s (%d)", btwlan,
        		strerror(errno), errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;

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
        em_error((char*)"open(%s) failed: %s (%d)", wifi_rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    sz = read(fd, &buffer, 1);
    if (sz != 1) {
        em_error((char*)"read(%s) failed: %s (%d)", wifi_rfkill_state_path, strerror(errno),
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


int wifi_init_iface(const char *ifname)
{
    int s, ret = 0;
    struct iwreq wrq;
    char buf[33];

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        em_error((char*)"socket(AF_INET,SOCK_DGRAM)");
        return -1;
    }

    em_error((char*)"[WIFI] wifi_init_iface: set mode\n");

    memset(&wrq, 0, sizeof(struct iwreq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    wrq.u.mode = IW_MODE_INFRA;

    if (ioctl(s, SIOCSIWMODE, &wrq) < 0) {
        em_error((char*)"ioctl(SIOCSIWMODE) %s\n", strerror(errno));
        ret = -1;
        goto exit;
    }

    memset(&wrq, 0, sizeof(struct iwreq));    
    memset(buf, '\0', sizeof(buf));

    em_error((char*)"[WIFI] wifi_init_iface: set essid\n");

    strcpy(buf, "aaa");
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
    wrq.u.essid.flags = 1; /* flags: 1 = ESSID is active, 0 = not (promiscuous) */
    wrq.u.essid.pointer = (caddr_t) buf;
    wrq.u.essid.length = strlen(buf);    
    if (WIRELESS_EXT < 21)
        wrq.u.essid.length++;

    if (ioctl(s, SIOCSIWESSID, &wrq) < 0) {
        em_error((char*)"ioctl(SIOCSIWESSID) \n",strerror(errno));
        ret = -1;
        goto exit;
    }

exit:
    close(s);

    return ret;    
}

int os_program_init(void)
{
#ifdef ANDROID
	/* We ignore errors here since errors are normal if we
	 * are already running as non-root.
	 */
	gid_t groups[] = { AID_INET, AID_WIFI, AID_KEYSTORE };
	setgroups(sizeof(groups)/sizeof(groups[0]), groups);

	prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);

	setgid(AID_WIFI);
	setuid(AID_WIFI);

	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap;
	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = 0;
	cap.effective = cap.permitted =
		(1 << CAP_NET_ADMIN) | (1 << CAP_NET_RAW);
	cap.inheritable = 0;
	capset(&header, &cap);
	em_error((char*)"os_program_init\n");
#endif


	return 0;
}
//added by mtk80758 for MT5921 and MT6620
CLocalOID::CLocalOID(const char* ifname, const char *chipName)
{
	if(strlen(ifname) < IFNAMSIZ)	{
		strcpy(m_name, ifname);
		em_printf(MSG_DEBUG, (char*)"ifname = %s", ifname);
	}
	else{
		em_error((char*)"ifname is out of range");
		goto out;
	}

	//for permission
	os_program_init();
	em_printf(MSG_DEBUG, (char*)"chipName = %s", chipName);
	strcpy(this->chipName, chipName);
	//open rfkill

	//wifi_set_power(1);

	/*the next line is deleted by chaozhong .liang for test use
	if(strcmp(this.chipName, "CMT5921") == 0)
	{
		wifi_set_power_ext(1);
	}
	*/
	//init_iface
	
		wifi_init_iface(ifname);
	
	//m_handle = socket(PF_INET, SOCK_DGRAM, 0);
	m_handle = socket(AF_INET, SOCK_DGRAM, 0);//meta_tool socket type
	if(m_handle < 0){
		em_error((char*)"Allocating m_handle socket failed");
	}
	else
	{
		em_printf(MSG_DEBUG, (char*)"Allocating m_handle socket succeed");
	}
out:
	return;		
}
CLocalOID::CLocalOID(const char* ifname)
{

#ifdef HAVE_ANDROID_OS
	if(strlen(ifname) < IFNAMSIZ)	{
		strcpy(m_name, ifname);
		em_printf(MSG_DEBUG, (char*)"ifname = %s", ifname);
	}
	else{
		em_error((char*)"ifname is out of range");
		goto out;
	}

	//for permission
	os_program_init();

	//open rfkill
	//wifi_set_power(1);
	/*the next line is deleted by chaozhong .liang for test use*/
	//wifi_set_power_ext(1);

	//init_iface
	wifi_init_iface(ifname);

	//m_handle = socket(PF_INET, SOCK_DGRAM, 0);
	m_handle = socket(AF_INET, SOCK_DGRAM, 0);//meta_tool socket type
	if(m_handle < 0){
		em_error((char*)"Allocating m_handle socket failed");
	}
	else
	{
		em_printf(MSG_DEBUG, (char*)"Allocating m_handle socket succeed");
	}
out:
	return;	
	
#else
	DEBUGFUNC(_Func)("CLocalOID::CLocalOID");
	INITLOG((_T("\n")));

    //get handler of protocol driver
    m_handle = CreateFile (	NDISUIO_DEVICE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL);

	if (m_handle == INVALID_HANDLE_VALUE) {
		//TCHAR buf[128];
		//_stprintf(buf, TEXT("Creating file failed, error code: %d\n"), GetLastError());
        //ERRORLOG((buf));
        return;
    }
#endif

}

CLocalOID::~CLocalOID(void)
{
#ifdef HAVE_ANDROID_OS
	if(m_handle > 0)
		close(m_handle);

	//close rfkill
	//wifi_set_power(0);

	//wifi_set_power_ext(0);
#else	
	DEBUGFUNC("CLocalOID::~CLocalOID");
    INITLOG((_T("\n")));

    //CloseHandle
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
    }
#endif
}

#if 0
INT_32
CLocalOID::EnumAdapter(deque<AdapterName> & adaptersNameList)
{
#ifdef 	HAVE_ANDROID_OS
	return 0;
#else
	DEBUGFUNC("CLocalOID::EnumAdapter");
	INITLOG((_T("\n")));

	//get adapter name list
    //CHAR Buf[1024];
	//PQUERY_BINDING_STRUC qryBinding_p = (PQUERY_BINDING_STRUC)Buf;
	AdapterName aName;
	
	TCHAR nameList[1024];
    int numNdisAdapter;
    TCHAR * devName;
    
    numNdisAdapter = lookupNdisAdapter(m_handle, nameList, sizeof(nameList));
    devName = nameList;
    
    int nameLen;
    /* Enumerate though all NDIS devices. */
    for (int i=0; i < numNdisAdapter; i++) {
        INT_32 nNetworkType;  // Status Of I/O Operation.
        UINT_32 nLength = sizeof( nNetworkType );
        BOOL result;

        nameLen = _tcsclen(devName);

        result = queryNdisOID( 
                m_handle,
        		devName, 
            	OID_802_11_NETWORK_TYPE_IN_USE,
                (UCHAR*)&nNetworkType,
                nLength,
                &nLength);

        if (result == TRUE) {
			aName.Name = (WCHAR_T*)devName;
			aName.Desc = (WCHAR_T*)devName;
			adaptersNameList.push_back(aName);
        }
		/*
        else {
            UINT_32 rc = GetLastError();
            if (rc != ERROR_NO_MORE_ITEMS) {
                ERRORLOG((TEXT("EnumAdapter: terminated abnormally\n")));
            }
            break;
        }
		*/
        devName += nameLen + 1;
    }

	return (INT_32) adaptersNameList.size();
#endif
}

INT_32
lookupNdisAdapter (
        HANDLE uioHandle, 
        TCHAR * outputBuf, 
        INT_32 maxBufLen
        )
{
#ifdef HAVE_ANDROID_OS
	return 0;
#else
	TCHAR chData[1024];
	PNDISUIO_QUERY_BINDING pBindInfo;
	INT_32 length, index = 0, totalLen = 0;
	BOOL bValue = TRUE;
	UINT_32 bytesconsumed;
	UINT_32 errorCode;

	DEBUGFUNC("COID_Local::lookupNdisAdapter");
	INITLOG((_T("\n")));

    while (bValue) {
        pBindInfo = (PNDISUIO_QUERY_BINDING) chData;
        pBindInfo->BindingIndex = index;
        length = sizeof (NDISUIO_QUERY_BINDING);

        bValue = DeviceIoControl (uioHandle,
            IOCTL_NDISUIO_QUERY_BINDING,
            pBindInfo,
            length,
            NULL,
            1024,
            &bytesconsumed,
            NULL);

        DEBUGMSG(TRUE, (_T("DeviceIoControl returns %d\r\n"), bValue));

        if (!bValue) {
            errorCode = GetLastError();
            DEBUGMSG(TRUE, 
            		(_T("Error Code returns %d no more? %d\r\n"),
                    errorCode, 
                    errorCode == ERROR_NO_MORE_ITEMS));
            break;
        }
#if 0
        TCHAR adpater_name[256];

        _stprintf(adpater_name, _T("Dev name:%d Len:%d %s\n"),
            pBindInfo->DeviceNameOffset,
            pBindInfo->DeviceNameLength,
            (TCHAR*)((UCHAR*)pBindInfo + pBindInfo->DeviceNameOffset));
        OutputDebugString(adpater_name);
#endif

        _stprintf(outputBuf + totalLen, _T("%s"),
                (TCHAR*)((UCHAR*)pBindInfo + pBindInfo->DeviceNameOffset));
        totalLen += pBindInfo->DeviceNameLength/sizeof(TCHAR);
        index ++;
    }

    return index;
#endif
}
#endif

BOOL
CLocalOID::GetChipID(UINT_32 & ChipID, const char *Name)
{
#ifdef HAVE_ANDROID_OS
	BOOL ret = FALSE;
	UINT_32 data;
	
	ret = queryNdisOID(m_handle, 
			m_name,
			OID_IPC_OID_INTERFACE_VERSION,
			(CHAR *)&data, 
			sizeof(data), 
			NULL);
	if(ret){
		ChipID = data;
	}

	return ret;
#else
	UINT_32 data;
	BOOL bResult;
	UINT_32 bytesReturned;

     	bResult = queryNdisOID( 
                m_handle,
          		(WCHAR_T*)Name.c_str(), 
            	OID_IPC_OID_INTERFACE_VERSION,
                (UCHAR*)&data,
                sizeof(UINT_32),
                &bytesReturned);

	if (bResult) {
		ChipID = data;
		return bResult;
	}
	ChipID = 0;
	return bResult;
#endif
}


INT_32
CLocalOID::setOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen)
{
#ifdef HAVE_ANDROID_OS
	BOOL  ret = FALSE;

	ret = setNdisOID(m_handle, 
			m_name,
			oid, 
			buf,
			bufLen);

	return ret;
#else
	BOOL bResult;
	char *devName = aAdapter->GetWstrAdapterID();
  
    bResult = ::setNdisOID(
            m_handle,
    		devName, 
        	oid, 
        	(UCHAR*)buf, 
        	bufLen);
        
  	return bResult;
#endif
}


INT_32
CLocalOID::queryOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen, UINT_32 *bytesWrite)
{

#ifdef HAVE_ANDROID_OS
	BOOL  ret = FALSE;

	ret = queryNdisOID(m_handle, 
			m_name,
			oid, 
			buf,
			bufLen,
			bytesWrite);

	return ret;
	
#else
	BOOL bResult;
	UINT_32 bytesReturned;
	const char* devName = aAdapter->GetWstrAdapterID();
	
    	bResult = queryNdisOID(
                	m_handle,
        			devName, 
            		oid,
                	buf,
                	bufLen,
                	&bytesReturned);
		
	if(bytesWrite != NULL)
		*bytesWrite = bytesReturned;

    return bResult;
#endif
}


/*
//do mux work
BOOL
CLocalOID::checkDevice(CAdapter *aAdapter)
{
    return TRUE;
}

BOOL 
CLocalOID::closeDevice(const wstring &AdapterName) 
{
	return TRUE;
}
BOOL 
CLocalOID::openDevice(const wstring &AdapterName) 
{
	return TRUE;
}
*/

}
