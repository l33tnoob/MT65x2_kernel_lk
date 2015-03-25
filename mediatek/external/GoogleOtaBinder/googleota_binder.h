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



#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/threads.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/xlog.h>

#include<sys/mount.h>
#include <mtd/mtd-abi.h>


#include <cutils/atomic.h>
#include <utils/Errors.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#undef LOG_TAG
#define LOG_TAG "GoogleOtaAgent"

using namespace android;
enum 
{
    TRANSACTION_SET_REBOOT_FLAG = IBinder::FIRST_CALL_TRANSACTION+100,
	TRANSACTION_CLEAR_UPGRADE_RESULT = IBinder::FIRST_CALL_TRANSACTION + 101,
    TRANSACTION_READ_UPGRADE_RESULT= IBinder::FIRST_CALL_TRANSACTION+111

};

class IGoogleOtaAgent:public IInterface 
{
public:
    DECLARE_META_INTERFACE(GoogleOtaAgent)
    virtual int setRebootFlag()=0;    
	virtual int clearUpgradeResult()=0;
    virtual int readUpgradeResult()=0;
};

class BpGoogleOtaAgent: public android::BpInterface<IGoogleOtaAgent>
{
public:
    BpGoogleOtaAgent(const android::sp<android::IBinder>& impl)
	: android::BpInterface<IGoogleOtaAgent>(impl)
        {
        }


    int setRebootFlag() {return 1;}
	int clearUpgradeResult() {return 1;}
    int readUpgradeResult() {return 0;}
};

class BnGoogleOtaAgent : public BnInterface<IGoogleOtaAgent>
{
public:
    status_t onTransact(uint32_t code,
			const Parcel &data,
			Parcel *reply,
			uint32_t flags);
    
};

class GoogleOtaAgent : public BnGoogleOtaAgent
{

public:
    static  void instantiate();
    GoogleOtaAgent();
    ~GoogleOtaAgent() {}
 
    virtual int setRebootFlag();
    int writeRebootFlash(char *rebootCmd);
	virtual int clearUpgradeResult();
    virtual int readUpgradeResult();
};


IMPLEMENT_META_INTERFACE(GoogleOtaAgent, "GoogleOtaBinder")

