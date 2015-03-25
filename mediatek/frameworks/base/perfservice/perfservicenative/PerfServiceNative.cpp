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


#define LOG_TAG "PerfService"
#include "utils/Log.h"
#include "PerfServiceNative.h"

#include <stdio.h>

#include <unistd.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
//#include <binder/IInterface.h>

namespace android
{

/* It should be sync with IPerfService.aidl */
enum {
	TRANSACTION_boostEnable = IBinder::FIRST_CALL_TRANSACTION,
	TRANSACTION_boostDisable,
	TRANSACTION_boostEnableTimeout,
	TRANSACTION_notifyAppState,
	TRANSACTION_userReg,
	TRANSACTION_userUnreg,
	TRANSACTION_userEnable,
	TRANSACTION_userEnableTimeout,
	TRANSACTION_userDisable,
	TRANSACTION_userResetAll,
	TRANSACTION_userDisableAll,
};

static int inited = 0;
static sp<IServiceManager> sm ;
static sp<IBinder> binder;

static void init(void)
{
    if(!inited) {
        sm = defaultServiceManager();
        //binder = sm->getService(String16("mtk-perfservice"));
        binder = sm->checkService(String16("mtk-perfservice")); // use check to avoid null binder
		if(binder!=NULL) {
        	inited = 1;
        }
    }
}

extern "C"
void PerfServiceNative_boostEnable(int scenario)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_boostEnable:%d", scenario);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(scenario);
        binder->transact(TRANSACTION_boostEnable ,data,&reply); // should sync with IPerfService
    }
}

extern "C"
void PerfServiceNative_boostDisable(int scenario)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_boostDisable:%d", scenario);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(scenario);
        binder->transact(TRANSACTION_boostDisable ,data,&reply);
    }
}

extern "C"
void PerfServiceNative_boostEnableTimeout(int scenario, int timeout)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_boostEnableTimeout:%d, %d", scenario, timeout);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(scenario);
        data.writeInt32(timeout);
        binder->transact(TRANSACTION_boostEnableTimeout ,data,&reply);
	}
}

extern "C"
int PerfServiceNative_userReg(int scn_core, int scn_freq)
{
	Parcel data, reply;
	int    handle = -1;
    init();

    ALOGI("PerfServiceNative_userReg: %d, %d", scn_core, scn_freq);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(scn_core);
	    data.writeInt32(scn_freq);
        binder->transact(TRANSACTION_userReg ,data,&reply); // should sync with IPerfService
        reply.readExceptionCode();
        handle = reply.readInt32();
    }
	return handle;
}

extern "C"
void PerfServiceNative_userUnreg(int handle)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_userUnreg:%d", handle);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(handle);
        binder->transact(TRANSACTION_userUnreg ,data,&reply); // should sync with IPerfService
    }
}


extern "C"
void PerfServiceNative_userEnable(int handle)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_userEnable:%d", handle);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(handle);
        binder->transact(TRANSACTION_userEnable ,data,&reply); // should sync with IPerfService
    }
}

extern "C"
void PerfServiceNative_userDisable(int handle)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_userDisable:%d", handle);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(handle);
        binder->transact(TRANSACTION_userDisable ,data,&reply);
    }
}

extern "C"
void PerfServiceNative_userEnableTimeout(int handle, int timeout)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_userEnableTimeout:%d, %d", handle, timeout);

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        data.writeInt32(handle);
        data.writeInt32(timeout);
        binder->transact(TRANSACTION_userEnableTimeout ,data,&reply);
    }
}

extern "C"
void PerfServiceNative_userResetAll(void)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_userResetAll");

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
   	    binder->transact(TRANSACTION_userResetAll ,data,&reply);
    }
}

extern "C"
void PerfServiceNative_userDisableAll(void)
{
	Parcel data, reply;
    init();

    ALOGI("PerfServiceNative_userDisableAll");

	if(binder!=NULL) {
        data.writeInterfaceToken(String16("com.mediatek.common.perfservice.IPerfService"));
        binder->transact(TRANSACTION_userDisableAll ,data,&reply);
	}
}

}

