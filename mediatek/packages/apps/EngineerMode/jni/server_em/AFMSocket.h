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

// AFMSocket.h: interface for the AFMSocket class.
#ifndef __AFMSocket_H__
#define __AFMSocket_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_un SOCKADDR_UN;
typedef struct sockaddr SOCKADDR;
typedef unsigned short WORD;
typedef int SOCKET;
typedef struct hostent HOSTENT;
typedef struct hostent * LPHOSTENT;
typedef struct in_addr * LPIN_ADDR;

typedef const char*	LPCTSTR;
typedef char*		LPCSTR;
//define this to use local socket.
#define USE_LOCAL_SOCKET


#include "common.h"
#include "AFMSync.h"
#include "AFMThread.h"

class AFMSocket;

void* do_SocketReadRun(AFMSocket* socket);
void* do_SocketAcceptRun(AFMSocket* socket);

class AFMSocket  
{
public:
	AFMSocket();
	virtual ~AFMSocket();	

  //lpszSocketAddress = NULL is client socket, or server socket.
	bool Create(unsigned int nSocketPort = 0, int nSocketType=SOCK_STREAM, LPCTSTR lpszSocketAddress = NULL);


	bool SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel = SOL_SOCKET);
	bool GetSockOpt(int nOptionName, void* lpOptionValue, int* lpOptionLen, int nLevel = SOL_SOCKET);


	static int GetLastError();
	static void SetLastError(int err);

	virtual bool Accept(AFMSocket& rConnectedSocket, SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);
	virtual void Close();

	bool Connect(LPCTSTR lpszHostAddress, unsigned int nHostPort, unsigned int time_msec=30000);
	bool Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen, unsigned int time_msec=30000);
	
	bool IOCtl(long lCommand, int* lpArgument);

	bool Listen(int nConnectionBacklog=5);


	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);


	bool ShutDown(int nHow = SHUT_WR);

	virtual int Send(const void* lpBuf, int nBufLen, int nFlags = 0);

	void StopAndDelete();

	SOCKET m_hSocket;
	
	AFMLock m_Lock;
	AFMLock m_WriteLock;
	
protected:

	AFMThread* m_thr;        
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnAccept(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);

private:	

	int DoAcceptRun();
	int DoReadRun();

	bool	m_SocketCanSend ;
	friend void* do_SocketAcceptRun(AFMSocket* socket);
	friend void* do_SocketReadRun(AFMSocket* socket);

};

#endif 
