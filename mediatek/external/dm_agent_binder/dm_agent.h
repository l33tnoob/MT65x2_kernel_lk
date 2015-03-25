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


#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/String16.h>
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
#include <utils/Log.h>
#include <cutils/xlog.h>

#include<sys/mount.h>
#include <mtd/mtd-abi.h>
#include "../../external/nvram/libfile_op/libfile_op.h"
#include "custom_prop.h"

#include <cutils/atomic.h>
#include <utils/Errors.h>
#include <binder/IServiceManager.h>
#include <utils/String16.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <utils/Vector.h>
#undef LOG_TAG
#define LOG_TAG "DmAgent"

using namespace android;
enum {
	TRANSACTION_readDmTree = IBinder::FIRST_CALL_TRANSACTION,
	TRANSACTION_writeDmTree,
	TRANSACTION_isLockFlagSet,
	TRANSACTION_setLockFlag,
	TRANSACTION_clearLockFlag,
	TRANSACTION_readImsi,
	TRANSACTION_writeImsi,
	TRANSACTION_readOperatorName,
	TRANSACTION_getRegisterSwitch,
	TRANSACTION_setRegisterSwitch,
	TRANSACTION_setRebootFlag,
	TRANSACTION_getLockType,
	TRANSACTION_getOperatorId,
	TRANSACTION_getOperatorName,
	TRANSACTION_isHangMoCallLocking,
	TRANSACTION_isHangMtCallLocking,
	TRANSACTION_clearRebootFlag,
	TRANSACTION_isBootRecoveryFlag,
	TRANSACTION_getUpgradeStatus,
	TRANSACTION_restartAndroid,
	TRANSACTION_isWipeSet,
	TRANSACTION_setWipeFlag,
	TRANSACTION_clearWipeFlag,
	TRANSACTION_readOtaResult,
	TRANSACTION_clearOtaResult,
	TRANSACTION_getSwitchValue,
	TRANSACTION_setSwitchValue,
	TRANSACTION_getDmSwitchValue,
	TRANSACTION_setDmSwitchValue,
	TRANSACTION_getSmsRegSwitchValue,
	TRANSACTION_setSmsRegSwitchValue,
    TRANSACTION_setRegisterFlag,
    TRANSACTION_readRegisterFlag,
    TRANSACTION_writeImsi1,
    TRANSACTION_writeImsi2,
    TRANSACTION_readImsi1,
    TRANSACTION_readImsi2,
};

class IDmAgent: public IInterface {
public:
	DECLARE_META_INTERFACE( DmAgent)
	virtual char * readDmTree(int & size)=0;
	virtual int writeDmTree(char* tree, int size)=0;
	virtual int isLockFlagSet()=0;
	virtual int setLockFlag(char *lockType, int len)=0;
	virtual int clearLockFlag()=0;
	virtual char * readImsi(int & size)=0;
	virtual int writeImsi(char * imsi, int size)=0;
	virtual char * getRegisterSwitch(int & size)=0;
	virtual int setRegisterSwitch(char * registerSwitch, int size)=0;
	virtual char * readOperatorName(int & size)=0;
	virtual int setRebootFlag()=0;
	virtual int getLockType()=0;
	virtual char * getOperatorName()=0;
	virtual int getOperatorId()=0;
	virtual int isHangMtCallLocking()=0;
	virtual int isHangMoCallLocking()=0;
	virtual int isBootRecoveryFlag()=0;
	virtual int clearRebootFlag()=0;
	virtual int getUpgradeStatus()=0;
	virtual int restartAndroid()=0;
	virtual int isWipeSet()=0;
	virtual int setWipeFlag(char *wipeType, int len)=0;
	virtual int clearWipeFlag()=0;

	virtual int readOtaResult()=0;
	virtual int clearOtaResult()=0;

	virtual char * getSwitchValue(int & size)=0;
	virtual int setSwitchValue(char * registerSwitch, int size)=0;
	virtual char * getDmSwitchValue(int & size)=0;
	virtual int setDmSwitchValue(char * registerSwitch, int size)=0;
	virtual char * getSmsRegSwitchValue(int & size)=0;
	virtual int setSmsRegSwitchValue(char * registerSwitch, int size)=0;

    virtual bool setRegisterFlag(char * flag, int size)=0;
    virtual char * readRegisterFlag(int & size)=0;
    virtual bool writeImsi1(char * imsi, int size)=0;
    virtual bool writeImsi2(char * imsi, int size)=0;
    virtual char * readImsi1(int & size)=0;
    virtual char * readImsi2(int & size)=0;
};

class BpDmAgent: public android::BpInterface<IDmAgent>
{
public:
	BpDmAgent(const android::sp<android::IBinder>& impl)
	: android::BpInterface<IDmAgent>(impl) {}
    char* readDmTree(int & size) {return 0;}
    int writeDmTree(char* tree,int size) {return 1;}
    int isLockFlagSet() {return 1;}
    int setLockFlag(char *lockType, int len) {return 1;}
    int clearLockFlag() {return 1;}
    char * readImsi (int & size) {return 0;}
    int writeImsi (char * imsi,int size) {return 0;}
    char * getRegisterSwitch(int & size) {return 0;}
    int setRegisterSwitch(char * registerSwitch, int size) {return 0;}
    char * readOperatorName (int & size) {return 0;}


    int setRebootFlag() {return 1;}
    int getLockType() {return 0;};    
    char * getOperatorName() {return 0;};    
    int getOperatorId() {return 0;};  
    int isHangMtCallLocking() {return 0;};
    int isHangMoCallLocking() {return 0;};
    int isBootRecoveryFlag() {return 1;}
    int clearRebootFlag() {return 1;}
    int getUpgradeStatus() {return 1;}
    int restartAndroid(){return 1;}
    int isWipeSet() {return 1;}
    int setWipeFlag(char *wipeType, int len) {return 1;}
    int clearWipeFlag() {return 1;}

    int readOtaResult() { return 0; }
    int clearOtaResult() { return 0; }

    char * getSwitchValue(int & size) {return 0;}
    int setSwitchValue(char * registerSwitch, int size) {return 0;}
    char * getDmSwitchValue(int & size) {return 0;}
    int setDmSwitchValue(char * registerSwitch, int size) {return 0;}
    char * getSmsRegSwitchValue(int & size) {return 0;}
    int setSmsRegSwitchValue(char * registerSwitch, int size) {return 0;}

    bool setRegisterFlag(char * flag,int size) {return 1;}
    char * readRegisterFlag(int & size) {return 0;}
    bool writeImsi1(char * imsi,int size) {return 0;}
    bool writeImsi2(char * imsi,int size) {return 0;}
    char * readImsi1(int & size) {return 0;}
    char * readImsi2(int & size) {return 0;}
};

class BnDmAgent: public BnInterface<IDmAgent>
{
public:
    status_t onTransact(uint32_t code,
			const Parcel &data,
			Parcel *reply,
			uint32_t flags);
    
};

class DmAgent: public BnDmAgent
{

public:
    static  void instantiate();
	DmAgent();
    ~DmAgent() {}
	virtual char* readDmTree(int & size);
	virtual int writeDmTree(char* tree, int size);
	virtual int isLockFlagSet();
	virtual int setLockFlag(char *lockType, int len);
	virtual int clearLockFlag();
	virtual char * readImsi(int & size);
	virtual int writeImsi(char * imsi, int size);
	virtual char * getRegisterSwitch(int & size);
	virtual int setRegisterSwitch(char * registerSwitch, int size);
	virtual char * readOperatorName(int & size);

	virtual int setRebootFlag();
	int writeRebootFlash();
	virtual int getLockType();
	virtual char * getOperatorName();
	virtual int getOperatorId();
	virtual int isHangMtCallLocking();
	virtual int isHangMoCallLocking();
	virtual int clearRebootFlag();
	virtual int isBootRecoveryFlag();
	virtual int getUpgradeStatus();
	int writeRebootFlash(char *rebootCmd);
	int setRecoveryCommand();
	char * readMiscPartition(int readSize);
	virtual int restartAndroid();

	virtual int isWipeSet();
	virtual int setWipeFlag(char *wipeType, int len);
	virtual int clearWipeFlag();

	virtual int readOtaResult();
	virtual int clearOtaResult();

	virtual char * getSwitchValue(int & size);
	virtual int setSwitchValue(char * registerSwitch, int size);
	virtual char * getDmSwitchValue(int & size);
	virtual int setDmSwitchValue(char * registerSwitch, int size);
	virtual char * getSmsRegSwitchValue(int & size);
	virtual int setSmsRegSwitchValue(char * registerSwitch, int size);

    virtual bool setRegisterFlag(char * flag, int size);
    virtual char * readRegisterFlag(int & size);
    virtual bool writeImsi1(char * imsi, int size);
    virtual bool writeImsi2(char * imsi, int size);
    virtual char * readImsi1(int & size);
    virtual char * readImsi2(int & size);
};

IMPLEMENT_META_INTERFACE(DmAgent, "DmAgent")

