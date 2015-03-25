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

#include "RPCClient.h"
#include "stdio.h"
#include "stdlib.h"
#include "Modules.h"

RPCClient::RPCClient()
{
}
RPCClient::~RPCClient()
{
}

void RPCClient::OnReceive(int nErrorCode)
{
	printf("Client onReceive ErrCode %d\n", nErrorCode);
	if(nErrorCode == 0)
	{		
		int len = ReadInt();		
		if(-1 == len)
		{			
			return;
		}
		
		char* buf = new char[len+1];
		buf[len] = 0;
		len = Receive(buf, len);
		printf("receive package len %d\n", len);
		if(len == -1)
			return ;

		int feature_id = atoi(buf);
		
		delete [] buf;

		printf("feature_id :%d\n", feature_id);
		
		AFMModules::Execute(feature_id, this);

		StopAndDelete();
	}
}

void RPCClient::OnClose(int nErrorCode)
{
	printf("Client onClose err %d\n", nErrorCode);	
	StopAndDelete();
}

int RPCClient::PostMsg(char* msg)
{
	AFMSingleLock lock(&m_WriteMsgLock) ;
	lock.Lock() ;
	if(msg == 0)
		return -1;
	WriteInt(strlen(msg));
	int len = Send(msg, strlen(msg));
	if(len == -1){
		return -1;
	}else{
		return 0;
	}
}

int RPCClient::ReadInt()
{
	char buf[4];
	int val=0;	
	int len = Receive(buf, 4);
	
	if(len == -1)
		return -1;

	if(1 == IsBigEndian())
	{		
		val = *((int*)buf);		
	}
	else
	{	
		char* p = (char*)&val;
		p[0]=buf[3];
		p[1]=buf[2];
		p[2]=buf[1];
		p[3]=buf[0];
	}
	return val;
}

int RPCClient::WriteInt(int val)
{	
	char buf[4];
	char* p = (char*)&val;

	if(1 == IsBigEndian())
	{
		buf[0]=p[0];
		buf[1]=p[1];
		buf[2]=p[2];
		buf[3]=p[3];		
	}
	else
	{		
		buf[0]=p[3];
		buf[1]=p[2];
		buf[2]=p[1];
		buf[3]=p[0];
	}
	
	int len = Send(buf, 4);
	if(len == -1)
	{
		printf("RPCClient WriteInt-1!!!!\n");
		return -1;
	}
	printf("RPCClient WriteInt retlen%d, %x\n",len, *((int*)buf));
	return val;
	
	
}
int RPCClient::IsBigEndian()
{
	unsigned short s = 0xFF00;
	if( (unsigned char)s == 0xFF)
		return 1;    //bigEndian
	else
		return 0;
}
