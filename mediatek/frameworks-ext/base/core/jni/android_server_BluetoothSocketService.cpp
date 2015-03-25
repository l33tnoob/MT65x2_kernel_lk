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


#define LOG_TAG "BluetoothSocketService.cpp"

#include "android_runtime/AndroidRuntime.h"
#include "JNIHelp.h"
#include "utils/Log.h"
#include "cutils/sockets.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <cutils/xlog.h>

#ifdef __BTMTK__
extern "C"
{
#include "bt_jsr82_api.h"
#include "bt_jsr82_hdl.h"
#include "bt_event.h"
#include "bt_jsr82_mmi_event.h"
}
#endif


namespace android {

typedef enum
{
	SERVER_STATE_NONE,
//	SERVER_STATE_ENABLING,
//	SERVER_STATE_ENABLED,
	SERVER_STATE_TURNING_ON,
	SERVER_STATE_TURN_ON,
	SERVER_STATE_TURNING_OFF,
//	SERVER_STATE_DISABLING
} SERVER_STATE;

typedef struct {
    JavaVM *vm;
    int envVer;
    jobject me;  // for callbacks to java

    pthread_mutex_t thread_mutex;
    pthread_t thread;

    int jsr82srvcsock;    // for receiving IND
    int jsr82sock;           // for send REQ and receive RSP
} native_data_t;

static native_data_t *nat = NULL;  // global native data

#ifdef __BTMTK__
/* Local Context for each channel */
typedef struct
{
	jboolean inUse;
	jbyte index;	// index from JBT
	short l2cap_id;	// l2cap_id from JBT
	jbyte ps_type;	// L2CAP or RFCOMM
	short channelNumber;	// 1. RFCOMM channel	2. L2CAP PSM
	jboolean bServer;	// Server port or not
	jboolean bConnected;	// Connection exists or not
	jboolean bWriting;	// This port is writing
	jboolean bRegistering; // This Server port is registering
	jboolean bAborting;	// This port is aborting
	U8 bdAddr[6];		// BD_ADDR of the connected remote device

	int fd;	// Local handle used to distinquish each APP; Assigned by JNI
	pthread_cond_t jsr82ConnectCond;	// Condition variable for waiting
	pthread_cond_t jsr82ReadCond; 	// Condition variable for READ
	pthread_cond_t jsr82WriteCond; 	// Condition variable for READ
	pthread_cond_t jsr82RegisterCond; 	// Condition variable for Service Registration
	jboolean bAuthentication;	// Authentication
	jboolean bEncryption;	// Encryption
	jint port;		// port: the channel number that APP registered.

	SERVER_STATE serverState;
}btmtk_jsr82_mmi_context_struct;

#define ANDROID_JSR82_FD_BASE 0x8000
#define BDADDR_SIZE 18
#define SERVICE_REGISTRATION_TIMEOUT 10000	// milliseconds
#define WRITE_DATA_TIMEOUT            17000


btmtk_jsr82_mmi_context_struct  g_jsr82MMIContext[JSR82_PORT_NUM];


/* Keep types in sync with BluetoothSocket.java */
static const int TYPE_RFCOMM = 1;
static const int TYPE_SCO = 2;
static const int TYPE_L2CAP = 3;


jboolean btmtk_jsr82_allocate_cntx(int *iIndex);
void btmtk_jsr82_clear_cntx(int iIndex);
int btmtk_jsr82_context_to_fd(int iCntx);
int btmtk_jsr82_fd_to_context(int fd);
jboolean btmtk_jsr82_search_cntx_by_fd(int fdHandle, int *iIndex);
jboolean btmtk_jsr82_search_cntx_by_conn_index(jbyte conn_index, int *iIndex);
jbyte btmtk_jsr82_security_mask(jboolean bAuth, jboolean bEncrypt);
void btmtk_jsr82_convert_string2bdaddr(const char *source, U8 *dest);
void btmtk_jsr82_android_cb_enable_service_cnf(void *parms);
void btmtk_jsr82_android_cb_disable_service_cnf(void *parms);
void btmtk_jsr82_android_cb_turn_on_service_cnf(void *parms);
void btmtk_jsr82_android_cb_turn_off_service_cnf(void *parms);
void btmtk_jsr82_android_cb_connect_ind(void *parms);
void btmtk_jsr82_android_cb_connect_cnf(void *parms);
void btmtk_jsr82_android_cb_disconnect_ind(void *parms);
void btmtk_jsr82_android_cb_rx_ready_ind(void *parms);
void btmtk_jsr82_cb_event_handler(void *context, BT_CALLBACK_EVENT event, void *parms, U16 datasize);
static jboolean jsr82_startEventLoop(JNIEnv* env, jobject object);
static void *jsr82_eventLoopMain(void *ptr);
static jboolean jsr82_registerSocket(JNIEnv *env, jobject object);


jboolean btmtk_jsr82_allocate_cntx(int *iIndex)
{
	int iCount = 0;

	for (iCount = 0; iCount < JSR82_PORT_NUM; iCount++)
	{
		if (JNI_FALSE == g_jsr82MMIContext[iCount].inUse)
		{
        		XLOGE("[JSR82] alloc context : index=%d", iCount);
			memset(&g_jsr82MMIContext[iCount], 0x0, sizeof(btmtk_jsr82_mmi_context_struct));
			btmtk_jsr82_clear_cntx(iCount);
			g_jsr82MMIContext[iCount].inUse = JNI_TRUE;
			g_jsr82MMIContext[iCount].bServer = JNI_FALSE;
			g_jsr82MMIContext[iCount].bConnected = JNI_FALSE;
			g_jsr82MMIContext[iCount].bWriting = JNI_FALSE;
			g_jsr82MMIContext[iCount].bRegistering = JNI_FALSE;
			g_jsr82MMIContext[iCount].bAborting = JNI_FALSE;
			g_jsr82MMIContext[iCount].bAuthentication = JNI_FALSE;
			g_jsr82MMIContext[iCount].bEncryption = JNI_FALSE;
			*iIndex = iCount;
			return JNI_TRUE;
		}
	}
	return JNI_FALSE;
}


void btmtk_jsr82_clear_cntx(int iIndex)
{
        XLOGE("[JSR82] Clear context : index=%d, ctx.index=%d, ctx.fd=%d", iIndex, g_jsr82MMIContext[iIndex].index,g_jsr82MMIContext[iIndex].fd);
//	memset(&g_jsr82MMIContext[iIndex], 0x0, sizeof(btmtk_jsr82_mmi_context_struct));
	g_jsr82MMIContext[iIndex].inUse = JNI_FALSE;
	g_jsr82MMIContext[iIndex].index = 0xFF;
}


jint btmtk_jsr82_context_to_fd(int iCntx)
{
	return (jint)(ANDROID_JSR82_FD_BASE + iCntx);
}


int btmtk_jsr82_fd_to_context(int fd)
{
	return (fd - ANDROID_JSR82_FD_BASE);
}


jboolean btmtk_jsr82_search_cntx_by_fd(int fdHandle, int *iIndex)
{
	int iCount = 0;

	if (NULL == iIndex)
	{
		XLOGE("[JSR82][JNI] btmtk_jsr82_search_cntx_by_fd: Invalid parameter iIndex.");
		return JNI_FALSE;
	}

	for (iCount = 0; iCount < JSR82_PORT_NUM; iCount++)
	{
		if ((JNI_TRUE == g_jsr82MMIContext[iCount].inUse) && 
			(fdHandle == g_jsr82MMIContext[iCount].fd))
		{
			*iIndex = iCount;
			return JNI_TRUE;
		}
	}
	return JNI_FALSE;
}


jboolean btmtk_jsr82_search_cntx_by_conn_index(jbyte conn_index, int *iIndex)
{
	int iCount = 0;

	if (NULL == iIndex)
	{
		XLOGE("[JSR82][JNI] btmtk_jsr82_search_cntx_by_conn_index: Invalid parameter iIndex.");
		return JNI_FALSE;
	}

	for (iCount = 0; iCount < JSR82_PORT_NUM; iCount++)
	{
		if ((JNI_TRUE == g_jsr82MMIContext[iCount].inUse) && 
			(conn_index == g_jsr82MMIContext[iCount].index))
		{
			*iIndex = iCount;
			return JNI_TRUE;
		}
	}
	return JNI_FALSE;
}


jbyte btmtk_jsr82_security_mask(jboolean bAuth, jboolean bEncrypt)
{
	if ((JNI_TRUE == bAuth) || (JNI_TRUE  == bEncrypt))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


void btmtk_jsr82_convert_string2bdaddr(const char *source, U8 *dest)
{
	U8 addr[6];
	int count;
	char *ptr = (char*)source;

	XLOGI("[JSR82][JNI] btmtk_jsr82_convert_string2bdaddr: <%s> ", source);
	for (count = 0; count < 6; count++)
	{
		addr[count] = strtoul(ptr, &ptr, 16);
		ptr++;
	}
	memcpy(dest, addr, 6);
	XLOGI("[JSR82][JNI] btmtk_jsr82_convert_string2bdaddr: <%s> ==> <0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X>", 
		source, dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
}
#endif

static void initNative(JNIEnv *env, jobject obj)
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	XLOGI("[JSR82][JNI] initNative +++");
	nat = (native_data_t *)calloc(1, sizeof(native_data_t));
	if (NULL == nat) 
	{
		XLOGE("[JSR82][JNI] %s: out of memory!", __FUNCTION__);
		return ;
	}

	memset(nat,0,sizeof(native_data_t));
	//Init socket handler value
	nat->jsr82srvcsock = -1;
	nat->jsr82sock = -1;
	env->GetJavaVM( &(nat->vm) );
	nat->envVer = env->GetVersion();
	nat->me = env->NewGlobalRef(obj);
	pthread_mutex_init(&(nat->thread_mutex), NULL);
	
        for(int i = 0;i < JSR82_PORT_NUM;i++) {
            btmtk_jsr82_clear_cntx(i);
        }
	
	XLOGI("[JSR82][JNI] initNative ---");
	return ;
#endif
	//jniThrowIOException(env, ENOSYS);
}


static void cleanupNative(JNIEnv* env, jobject object)
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	if (nat) 
	{
		pthread_mutex_destroy(&(nat->thread_mutex));
		free(nat);
	}
	return ;
#endif
	//jniThrowIOException(env, ENOSYS);
}


static int initSocketNative(JNIEnv *env, jobject obj, jint type, jboolean auth, jboolean encrypt, jint port) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	int iCntxNew = 0xFF;
	jboolean bAllocateResult = FALSE;
	jint typeLocal;
	jboolean authLocal;
	jboolean encryptLocal;
	jint portLocal;

	XLOGI("[JSR82][JNI] initSocketNative +++.");

	typeLocal = type;
	authLocal = auth;
	encryptLocal = encrypt;
	portLocal = port;

	XLOGI("[JSR82][JNI] initSocketNative: start to initialize socket.");
	XLOGI("[JSR82][JNI] type=%d, auth=%d, encrypt=%d, port=%d", typeLocal, authLocal, encryptLocal, portLocal);
	// Initialize socket
	if(!nat)
	{
		XLOGW("[JSR82][JNI] nat is NULL");
		//jniThrowIOException(env, EHOSTDOWN);
		return -1;
	}
	if(-1 == nat->jsr82srvcsock)
	{
		jboolean ret;

		XLOGI("[JSR82][JNI] initSocketNative: nat->jsr82srvcsock=-1");
		ret = jsr82_registerSocket(env, obj);
		if(!ret)
		{
			XLOGE("[JSR82][JNI] jsr82_registerSocket fail");
			//jniThrowIOException(env, ENETUNREACH);
			return -1;
		}
	}
	XLOGI("[JSR82][JNI] initSocketNative: Initialize socket done.");

	// Assign FD to this port
	// Update local context: type, security

	pthread_mutex_lock(&(nat->thread_mutex));
	bAllocateResult = btmtk_jsr82_allocate_cntx(&iCntxNew);
	if (JNI_TRUE == bAllocateResult)
	{
		// Context initialization
		g_jsr82MMIContext[iCntxNew].fd = btmtk_jsr82_context_to_fd(iCntxNew);
		g_jsr82MMIContext[iCntxNew].bAuthentication = authLocal;
		g_jsr82MMIContext[iCntxNew].bEncryption = encryptLocal;
		g_jsr82MMIContext[iCntxNew].port = portLocal;
		pthread_cond_init(&g_jsr82MMIContext[iCntxNew].jsr82ConnectCond, NULL);
		pthread_cond_init(&g_jsr82MMIContext[iCntxNew].jsr82ReadCond, NULL);
		pthread_cond_init(&g_jsr82MMIContext[iCntxNew].jsr82WriteCond, NULL);
		pthread_cond_init(&g_jsr82MMIContext[iCntxNew].jsr82RegisterCond, NULL);
	//	g_jsr82MMIContext[iCntxNew].jsr82ConnectCond = PTHREAD_COND_INITIALIZER;
	//	g_jsr82MMIContext[iCntxNew].jsr82ReadCond = PTHREAD_COND_INITIALIZER;
	//	g_jsr82MMIContext[iCntxNew].jsr82RegisterCond = PTHREAD_COND_INITIALIZER;
		switch (typeLocal) 
		{
			case TYPE_RFCOMM:
				g_jsr82MMIContext[iCntxNew].ps_type = JSR82_SESSION_PS_RFCOMM;
				break;
			case TYPE_L2CAP:
				g_jsr82MMIContext[iCntxNew].ps_type = JSR82_SESSION_PS_L2CAP;
				break;
			case TYPE_SCO:
				// This tye is not supported currently
			default:
				pthread_mutex_unlock(&(nat->thread_mutex));
				XLOGE("[JSR82][JNI] initSocketNative: Invalid type (%d) !!", typeLocal);
				//jniThrowIOException(env, EINVAL);
				return -1;
		}
		pthread_mutex_unlock(&(nat->thread_mutex));

		// Initialize Cond variable
		if (0 == g_jsr82MMIContext[iCntxNew].jsr82ConnectCond.value)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			if( 0 != pthread_cond_init(&(g_jsr82MMIContext[iCntxNew].jsr82ConnectCond), NULL))
			{
				pthread_mutex_unlock(&(nat->thread_mutex));
				XLOGE("[JSR82][JNI] pthread_cond_init failed <Connect>");
				return -1;
			}
			else
			{
				pthread_mutex_unlock(&(nat->thread_mutex));
				XLOGI("[JSR82][JNI] jsr82ConnectCond (%d) initialization success ", iCntxNew);
			}
		}
		if (0 == g_jsr82MMIContext[iCntxNew].jsr82ReadCond.value)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			if( 0 != pthread_cond_init(&(g_jsr82MMIContext[iCntxNew].jsr82ReadCond), NULL))
			{
				XLOGE("[JSR82][JNI] pthread_cond_init failed <READ>");
				pthread_mutex_unlock(&(nat->thread_mutex));
				return -1;
			}
			else
			{
				pthread_mutex_unlock(&(nat->thread_mutex));
				XLOGI("[JSR82][JNI] jsr82ReadCond (%d) initialization success <READ> ", iCntxNew);
			}
		}		
		if (0 == g_jsr82MMIContext[iCntxNew].jsr82WriteCond.value)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			if( 0 != pthread_cond_init(&(g_jsr82MMIContext[iCntxNew].jsr82WriteCond), NULL))
			{
				XLOGE("[JSR82][JNI] pthread_cond_init failed <WRITE>");
				pthread_mutex_unlock(&(nat->thread_mutex));
				return -1;
			}
			else
			{
				pthread_mutex_unlock(&(nat->thread_mutex));
				XLOGI("[JSR82][JNI] jsr82WriteCond (%d) initialization success <WRITE> ", iCntxNew);
			}
		}
		if (0 == g_jsr82MMIContext[iCntxNew].jsr82RegisterCond.value)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			if( 0 != pthread_cond_init(&(g_jsr82MMIContext[iCntxNew].jsr82RegisterCond), NULL))
			{
				XLOGE("[JSR82][JNI] pthread_cond_init failed <REGISTER>");
				pthread_mutex_unlock(&(nat->thread_mutex));
				return -1;
			}
			else
			{
				pthread_mutex_unlock(&(nat->thread_mutex));
				XLOGI("[JSR82][JNI] jsr82RegisterCond (%d) initialization success <REGISTER> ", iCntxNew);
			}
		}		
	}
	else
	{
		pthread_mutex_unlock(&(nat->thread_mutex));
		XLOGE("[JSR82][JNI] No free context space; the APP has registered %d services.", JSR82_PORT_NUM);
		return -1;
	}	
	XLOGI("[JSR82][JNI] initSocketNative ---. fdHandle=%d", g_jsr82MMIContext[iCntxNew].fd);
	return g_jsr82MMIContext[iCntxNew].fd;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static jint connectNative(JNIEnv *env, jobject obj, jint fdHandle, jstring sAddr, jint channelNumber) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	int iWaitResult = 0;
	jbyte securityMask = 0;
	U8 statusResult = 0;
	jshort mtu = 0;
	BT_BOOL bConnectResult = FALSE;
	const char *sAddrPtr;
	U8 aAddr[6];

	XLOGI("[JSR82][JNI] connectNative +++. fd=%d.", fdHandle);

	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		XLOGI("[JSR82][JNI] Connect to remote device.....");
		pthread_mutex_lock(&(nat->thread_mutex));
		g_jsr82MMIContext[iCntxIndex].channelNumber = (short) channelNumber;
		pthread_mutex_unlock(&(nat->thread_mutex));
		securityMask = btmtk_jsr82_security_mask(g_jsr82MMIContext[iCntxIndex].bAuthentication, g_jsr82MMIContext[iCntxIndex].bEncryption);
		if (JSR82_SESSION_PS_RFCOMM == g_jsr82MMIContext[iCntxIndex].ps_type)
		{
			mtu = JSR82_SESSION_PS_RFCOMM_MTU;
		}
		else
		{
			mtu = JSR82_SESSION_PS_L2CAP_MTU;
		}
		sAddrPtr = env->GetStringUTFChars(sAddr, NULL);
		XLOGI("[JSR82][JNI] sAddr=%s", sAddrPtr);
		if (sAddrPtr == NULL)
		{
			XLOGE("[JSR82][JNI] connectNative: NULL BD_ADDR string!");
			//jniThrowIOException(env, EINVAL);
			return -1;
		}
		// sAddrPtr conversion to aAddr
		btmtk_jsr82_convert_string2bdaddr(sAddrPtr, aAddr);

		bConnectResult = btmtk_jsr82_session_connect_req(g_jsr82MMIContext[iCntxIndex].fd, 
			aAddr, JSR82_SESSION_PS_RFCOMM, g_jsr82MMIContext[iCntxIndex].channelNumber, 
			mtu, securityMask, &statusResult);
		if (TRUE != bConnectResult)
		{
	    		XLOGE("[JSR82][JNI] btmtk_jsr82_session_connect_req failed..");
			//jniThrowIOException(env, ECONNABORTED);
			return -1;
	    	}
		env->ReleaseStringUTFChars(sAddr, sAddrPtr);

		// Block this thread and wait for the CONNECT_CNF
		iWaitResult = pthread_cond_wait(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond), NULL);
		if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting)
		{
			XLOGI("[JSR82][JNI] connectNative: This port is aborting.");
			//jniThrowIOException(env, ECANCELED);
			return -1;
		}			
		if (0 != iWaitResult)
		{
			XLOGE("[JSR82][JNI] connectNative -- Wait result is not correct.");
			//jniThrowIOException(env, ECONNABORTED);
			return -1;
		}
		// Check if this CONNECT_REQ succeeded or failed.
		if (JNI_TRUE != g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			//jniThrowIOException(env, ECONNREFUSED);
			return -1;
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] connectNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}
	
	XLOGI("[JSR82][JNI] connectNative ---.");
	return 0;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static int bindListenNative(JNIEnv *env, jobject obj, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	jbyte securityMask = 0;
	jshort mtu = 0;
	BT_BOOL bRegisterResult = FALSE;
	U8 statusResult = 0;
	int iWaitResult = 0;
	int channelNumber = 0;
	struct timespec nextTs;
	struct timeval currentTime;

	// Bind & Listen the socket
	XLOGI("[JSR82][JNI] bindListenNative +++. fd=%d.", fdHandle);

	// Register a Server port
	// 1. Search context if the Handle already exist
	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		pthread_mutex_lock(&(nat->thread_mutex));
		g_jsr82MMIContext[iCntxIndex].bServer = JNI_TRUE;
		g_jsr82MMIContext[iCntxIndex].bRegistering = JNI_TRUE;
		pthread_mutex_unlock(&(nat->thread_mutex));
		securityMask = btmtk_jsr82_security_mask(g_jsr82MMIContext[iCntxIndex].bAuthentication, g_jsr82MMIContext[iCntxIndex].bEncryption);
		if (JSR82_SESSION_PS_RFCOMM == g_jsr82MMIContext[iCntxIndex].ps_type)
		{
			mtu = JSR82_SESSION_PS_RFCOMM_MTU;
		}
		else
		{
			mtu = JSR82_SESSION_PS_L2CAP_MTU;
		}

		// If this is the port that generated by previous connected Server port, use the same channel number to register this port
		if (0 != g_jsr82MMIContext[iCntxIndex].channelNumber)
		{
			bRegisterResult = btmtk_jsr82_session_service_registration_use_existing_chnl_num(
							g_jsr82MMIContext[iCntxIndex].ps_type, 
							mtu, securityMask, g_jsr82MMIContext[iCntxIndex].fd, 
							g_jsr82MMIContext[iCntxIndex].channelNumber);
		}
		else
		{
			bRegisterResult = btmtk_jsr82_session_service_registration(g_jsr82MMIContext[iCntxIndex].ps_type, 
							mtu, securityMask, g_jsr82MMIContext[iCntxIndex].fd, &statusResult);
		}
		if (TRUE != bRegisterResult)
		{
			XLOGE("[JSR82][JNI] bindListenNative: btmtk_jsr82_session_service_registration failed.");
			//jniThrowIOException(env, ECONNABORTED);
			return -1;
		}

		// Registration result (ENABLE_CNF) has already come back owing to context switch
		pthread_mutex_lock(&(nat->thread_mutex));
		if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bRegistering)
		{
			// ENABLE_CNF thread & this thread interleaves
			pthread_mutex_unlock(&(nat->thread_mutex));
			// Registration finished.
			XLOGI("[JSR82][JNI] Registration has been finished.");
		}
		else
		{
			// Wait for the registration result and return channel nubmer back to upper layer
			XLOGI("[JSR82][JNI] Registering a new Server port. Wait for the response...");
			gettimeofday(&currentTime, NULL);
			nextTs.tv_sec = currentTime.tv_sec;
			nextTs.tv_nsec = (currentTime.tv_usec * 1000);
			nextTs.tv_sec += (SERVICE_REGISTRATION_TIMEOUT/1000);
			pthread_mutex_unlock(&(nat->thread_mutex));
			iWaitResult = pthread_cond_timedwait(&(g_jsr82MMIContext[iCntxIndex].jsr82RegisterCond), NULL, &nextTs);
			
			if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting)
			{
				XLOGI("[JSR82][JNI] bindListenNative: This port is aborting. Return -1.");
				//jniThrowIOException(env, ECANCELED);
				return -1;
			}			
			if (0 != iWaitResult)
			{
				XLOGE("[JSR82][JNI] bindListenNative - Wait result is not correct. %d, %s", errno, strerror(errno));
				//jniThrowIOException(env, ECONNABORTED);
				return -1;
			}
		}

		// Registration finished
		if (0 < g_jsr82MMIContext[iCntxIndex].channelNumber)
		{
			XLOGI("[JSR82][JNI] bindListenNative: channelNumber %d is registered.", g_jsr82MMIContext[iCntxIndex].channelNumber);
			channelNumber = g_jsr82MMIContext[iCntxIndex].channelNumber;
		}
		else
		{
			XLOGI("[JSR82][JNI] bindListenNative: channelNumber registration failed.");
			return -1;
		}

	}
	else
	{
		XLOGE("[JSR82][JNI] bindListenNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}

	XLOGI("[JSR82][JNI] bindListenNative ---.");
	return channelNumber;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static int acceptNative(JNIEnv *env, jobject obj, int timeout, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	// 1. Open one server and wait for the connection from other devices
	// 2. Return connected object if connection is created
	int iWaitResult = 0;
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	BT_BOOL bTurnOnResult = FALSE;
	U8 statusResult = 0;
	struct timespec nextTs;
	struct timeval currentTime;
	int fdHandleNew = -1;
	int iCntxIndexNew = 0xFF;
	BT_BOOL bIntSocket = FALSE;
	
	XLOGI("[JSR82][JNI] acceptNative +++. timeout=%d, fd=%d", timeout, fdHandle);

	if (timeout < 0 && timeout != -1)
	{
		goto exit;
	}

	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		pthread_mutex_lock(&(nat->thread_mutex));

		//check the state to turn on
		if (SERVER_STATE_NONE == g_jsr82MMIContext[iCntxIndex].serverState)
		{
			bTurnOnResult = btmtk_jsr82_session_service_turn_on(g_jsr82MMIContext[iCntxIndex].ps_type, 
							g_jsr82MMIContext[iCntxIndex].index, g_jsr82MMIContext[iCntxIndex].fd, &statusResult);
			if (TRUE != bTurnOnResult)
			{
				XLOGE("[JSR82][JNI] acceptNative: btmtk_jsr82_session_service_turn_on failed.");
				pthread_mutex_unlock(&(nat->thread_mutex));
				goto exit;
			}	
			g_jsr82MMIContext[iCntxIndex].serverState = SERVER_STATE_TURNING_ON;
		}	

		if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			// Block this thread and wait for the CONNECT_IND
			struct timespec *Ts = NULL;
			if (0 < timeout)
			{
				gettimeofday(&currentTime, NULL);
				nextTs.tv_sec = currentTime.tv_sec;
				nextTs.tv_nsec = (currentTime.tv_usec * 1000);
				nextTs.tv_sec += (timeout/1000);
				Ts = &nextTs;
			}
			XLOGI("[JSR82][JNI] acceptNative -- Waiting for connection........");
			iWaitResult = pthread_cond_timedwait(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond), &(nat->thread_mutex), Ts);

			if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting)
			{
				XLOGI("[JSR82][JNI] acceptNative: This port is aborting!");
			}
			else if (0 != iWaitResult)
			{
				XLOGI("[JSR82][JNI] acceptNative -- Wait result(%d) is not correct.", iWaitResult);
			}
			else
			{
				bIntSocket = TRUE;
			}
			
			pthread_mutex_unlock(&(nat->thread_mutex));

			if (FALSE == bIntSocket)
			{
				goto exit;
			}	
			XLOGI("[JSR82][JNI] acceptNative: Connection is created!");			
		}
		else
		{
			pthread_mutex_unlock(&(nat->thread_mutex));
			XLOGI("[JSR82][JNI] acceptNative: Connection has already been created!");
		}
		
		
		// Allocate a new context to keep listening for service.
		// New context is for original Server object
		fdHandleNew = initSocketNative(env, obj, g_jsr82MMIContext[iCntxIndex].ps_type, g_jsr82MMIContext[iCntxIndex].bAuthentication, 
			g_jsr82MMIContext[iCntxIndex].bEncryption, g_jsr82MMIContext[iCntxIndex].port);
		// New port uses the same channel number as the connected one
		bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandleNew, &iCntxIndexNew);
		if (JNI_TRUE == bSearchResult)
		{
			g_jsr82MMIContext[iCntxIndexNew].channelNumber = g_jsr82MMIContext[iCntxIndex].channelNumber;
		}
		else
		{
			XLOGE("[JSR82][JNI] acceptNative: The context of  new FD (%d) is gone.", fdHandleNew);
			//jniThrowIOException(env, EINVAL);
			return -1;
		}

		bindListenNative(env, obj, fdHandleNew);

		// Connected - return new FdHandle

	}
	else
	{
		XLOGE("[JSR82][JNI] acceptNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}
exit:
	
	XLOGI("[JSR82][JNI] acceptNative ---.");
	return fdHandleNew;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static jint availableNative(JNIEnv *env, jobject obj, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	I16 availLength = 0;
	int iRet;
	int iWaitResult = 0;

	XLOGI("[JSR82][JNI] availableNative +++. fd=%d", fdHandle);

	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		// Return available bytes for READ
		if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			availLength = btmtk_jsr82_session_GetAvailableDataLength(g_jsr82MMIContext[iCntxIndex].ps_type, 
							g_jsr82MMIContext[iCntxIndex].index, g_jsr82MMIContext[iCntxIndex].l2cap_id, 
							JBT_SESSION_RX_BUF_TYPE);
			if (0 < availLength)
			{
				XLOGI("[JSR82][JNI] %d bytes available", availLength);
			}
			else if (0 == availLength)
			{
				XLOGI("[JSR82][JNI] availableNative: No data available currently. Wait for data incoming...");
				iWaitResult = pthread_cond_wait(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond), &(nat->thread_mutex));
				if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting)
				{
					XLOGI("[JSR82][JNI] availableNative: This port is aborting. Return -1.");
					availLength = -1;
				}			
				if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected)
				{
					XLOGI("[JSR82][JNI] availableNative: This port is disconnected. Return -1.");
					availLength = -1;
				}	
				
				if (0 != iWaitResult)
				{
					XLOGE("[JSR82][JNI] availableNative - Wait result is not correct.");
					availLength = -1;
				}
				else
				{
					availLength = btmtk_jsr82_session_GetAvailableDataLength(g_jsr82MMIContext[iCntxIndex].ps_type, 
							g_jsr82MMIContext[iCntxIndex].index, g_jsr82MMIContext[iCntxIndex].l2cap_id, 
							JBT_SESSION_RX_BUF_TYPE);
					if (0 > availLength)
					{
						XLOGE("[JSR82][JNI] btmtk_jsr82_session_GetAvailableDataLength() failed <2>.");
						availLength = -1;
					}
				}
			}
			else
			{
				XLOGI("[JSR82][JNI] No available data");
				availLength = 0;
			}
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
		else
		{
			XLOGE("[JSR82][JNI] availableNative: This port is not yet connected.");
			//jniThrowIOException(env, EINVAL);
			return -1;
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] availableNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}
	XLOGI("[JSR82][JNI] availableNative ---.");
	return availLength;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static jint readNative(JNIEnv *env, jobject obj, jbyteArray jb, jint offset, jint length, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	jbyte *bufPtr = NULL;
	int arraySize = 0;
	I16 iReadResult = 0;
	int iWaitResult = 0;

	XLOGI("[JSR82][JNI] readNative +++. fd=%d.", fdHandle);
        if (0 == length )
	{
		return 0;
	}
	arraySize = env->GetArrayLength(jb);
	if (offset < 0 || length < 0 || offset + length > arraySize)
	{
		XLOGE("[JSR82][JNI] ByteArray size (%d) invalid. offset=%d, length=%d", arraySize, offset, length);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}
	bufPtr = env->GetByteArrayElements(jb, NULL);
	if (bufPtr == NULL)
	{
		XLOGE("[JSR82][JNI] ByteArray pointer bufPtr is NULL.");
		env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}
	
	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		pthread_mutex_lock(&(nat->thread_mutex));
		//Allow reading data even if disconnected
	/*	if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			XLOGE("[JSR82][JNI] This port is not yet connected.");
			pthread_mutex_unlock(&(nat->thread_mutex));
			return -1;
		}
		*/
		iReadResult = btmtk_jsr82_session_GetBytes(g_jsr82MMIContext[iCntxIndex].ps_type, 
					g_jsr82MMIContext[iCntxIndex].index, g_jsr82MMIContext[iCntxIndex].l2cap_id, 
					(U8*)&bufPtr[offset], (U16)((length > 0x7FFF) ? 0x7FFF : length) );
		if (0 > iReadResult)
		{
			XLOGE("[JSR82][JNI] btmtk_jsr82_session_GetBytes() failed.");
			env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
			//jniThrowIOException(env, ECONNABORTED);
			pthread_mutex_unlock(&(nat->thread_mutex));
			return -1;
		}
		else if (0 == iReadResult)
		{
			if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected) 
			{
				pthread_mutex_unlock(&(nat->thread_mutex));
				env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
				return -1;
			}
			// Block this thread and wait for the data
			XLOGI("[JSR82][JNI] No data available currently. Wait for data incoming...");
		//	pthread_mutex_unlock(&(nat->thread_mutex));
			iWaitResult = pthread_cond_wait(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond), &(nat->thread_mutex));
			pthread_mutex_unlock(&(nat->thread_mutex));
			if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting)
			{
				XLOGI("[JSR82][JNI] readNative: This port is aborting. Return -1.");
				env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
				//jniThrowIOException(env, ECANCELED);
				return -1;
			}			
			if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected)
			{
				XLOGI("[JSR82][JNI] readNative: This port is disconnected. Return -1.");
				env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
				//jniThrowIOException(env, ECANCELED);
				return -1;
			}	
			if (0 != iWaitResult)
			{
				XLOGE("[JSR82][JNI] readNative - Wait result is not correct.");
				env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
				//jniThrowIOException(env, ECONNABORTED);
				return -1;
			}
			else
			{
				// There is data available in this port.
				iReadResult = 0;
				iReadResult = btmtk_jsr82_session_GetBytes(g_jsr82MMIContext[iCntxIndex].ps_type, 
							g_jsr82MMIContext[iCntxIndex].index, g_jsr82MMIContext[iCntxIndex].l2cap_id, 
							(U8*)&bufPtr[offset], (U16)((length > 0x7FFF) ? 0x7FFF : length));
				if (0 > iReadResult)
				{
					XLOGE("[JSR82][JNI] btmtk_jsr82_session_GetBytes() failed <2>.");
					env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
					//jniThrowIOException(env, ECONNABORTED);
					return -1;
				}
				else
				{
					env->ReleaseByteArrayElements(jb, bufPtr, 0);
					XLOGI("[JSR82][JNI] readNative ---.");
					return iReadResult;
				}
			}
		}
		else
		{
			env->ReleaseByteArrayElements(jb, bufPtr, 0);
			pthread_mutex_unlock(&(nat->thread_mutex));
			XLOGI("[JSR82][JNI] readNative ---.");
			return iReadResult;
		}

	}
	else
	{
		XLOGE("[JSR82][JNI] readNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}
	return -1;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;

}


static jint writeNative(JNIEnv *env, jobject obj, jbyteArray jb, jint offset, jint length, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	jbyte *bufPtr = NULL;
	int arraySize = 0;
	int totalWriteResult = 0;
	I16 iWriteResult = 0;
	int iWaitResult = 0;
	int curOffset = offset;
	int curLen = length;
	struct timespec nextTs;
	struct timeval currentTime;

	XLOGI("[JSR82][JNI] writeNative +++. fd=%d.", fdHandle);
	arraySize = env->GetArrayLength(jb);
	if (offset < 0 || length < 0 || offset + length > arraySize)
	{
		XLOGE("[JSR82][JNI] ByteArray size (%d) invalid. offset=%d, length=%d", arraySize, offset, length);
		totalWriteResult = -1;
		goto exit;
	}
	bufPtr = env->GetByteArrayElements(jb, NULL);;
	if (bufPtr == NULL)
	{
		XLOGE("[JSR82][JNI] ByteArray pointer bufPtr is NULL.");
		totalWriteResult = -1;
		goto exit;
	}

	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			XLOGE("[JSR82][JNI] This port is not yet connected.");
			totalWriteResult = -1;
			goto exit;
		}
		pthread_mutex_lock(&(nat->thread_mutex));
	        do 
		{
			if (totalWriteResult < length) 
			{
				iWriteResult = btmtk_jsr82_session_PutBytes(g_jsr82MMIContext[iCntxIndex].ps_type, 
					g_jsr82MMIContext[iCntxIndex].index, g_jsr82MMIContext[iCntxIndex].l2cap_id, 
					(U8*)&bufPtr[curOffset], (U16)((curLen > 0x7FFF) ? 0x7FFF : curLen));
				if (0 > iWriteResult)
				{
					XLOGI("[JSR82][JNI] fail to write");						
					g_jsr82MMIContext[iCntxIndex].bWriting = JNI_FALSE;
					totalWriteResult = -1;
					goto exit;
				} 
				else 
				{
					XLOGI("[JSR82][JNI] buffer is full, Wait for buffer available again...");	
					g_jsr82MMIContext[iCntxIndex].bWriting = JNI_TRUE;
					totalWriteResult += iWriteResult;
					curOffset += iWriteResult;
					curLen -= iWriteResult;	
				}	
			}

			if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bWriting)
			{
				gettimeofday(&currentTime, NULL);
				nextTs.tv_sec = currentTime.tv_sec;
				nextTs.tv_nsec = (currentTime.tv_usec * 1000);
				nextTs.tv_sec += (WRITE_DATA_TIMEOUT/1000);
		
				iWaitResult = pthread_cond_timedwait(&(g_jsr82MMIContext[iCntxIndex].jsr82WriteCond), &(nat->thread_mutex), &nextTs);
				if ((0 != iWaitResult) || 
					(ETIMEDOUT == iWaitResult) ||
					(JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting) ||
					(JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected))
				{
					XLOGE("[JSR82][JNI] writeNative error - Wait result=%d, bAbort=%d, bConnected=%d", 
						iWaitResult,g_jsr82MMIContext[iCntxIndex].bAborting, g_jsr82MMIContext[iCntxIndex].bConnected);
					totalWriteResult = -1;
					g_jsr82MMIContext[iCntxIndex].bWriting = JNI_FALSE;
					break;
				}
				else 
				{
					XLOGV("[JSR82][JNI] writeNative, signal is received");
				}
			}
			
		} while ((totalWriteResult < length) || (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bWriting));

		pthread_mutex_unlock(&(nat->thread_mutex));
		
	}
	else
	{
		XLOGE("[JSR82][JNI] writeNative: The FD (%d) doesn't exist.", fdHandle);
		totalWriteResult = -1;
	}
exit:
	XLOGI("[JSR82][JNI] writeNative ---.");
	env->ReleaseByteArrayElements(jb, bufPtr, JNI_ABORT);
	return totalWriteResult;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}



static int abortNative(JNIEnv *env, jobject obj, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	U8 statusResult = 0;

	// Abort all blocking API and return immediately.
	XLOGI("[JSR82][JNI] abortNative +++. fd=%d.", fdHandle);

	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		pthread_mutex_lock(&(nat->thread_mutex));
		g_jsr82MMIContext[iCntxIndex].bAborting = JNI_TRUE;
		pthread_mutex_unlock(&(nat->thread_mutex));

		if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			BT_BOOL bDiscResult = FALSE;

			
			// Disconnect this port first
			bDiscResult = btmtk_jsr82_session_disconnect_req(g_jsr82MMIContext[iCntxIndex].fd, 
						g_jsr82MMIContext[iCntxIndex].ps_type, g_jsr82MMIContext[iCntxIndex].index, 
						g_jsr82MMIContext[iCntxIndex].l2cap_id, &statusResult);
			if (TRUE != bDiscResult)
			{
				XLOGE("[JSR82][JNI] abortNative: btmtk_jsr82_session_disconnect_req failed.");
				//jniThrowIOException(env, ECONNABORTED);
				return -1;
			}
		}
		// Avoid the case that Server connection is just created & a new port (this port) is just allocated
		else if ((JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bServer) && (JNI_TRUE != g_jsr82MMIContext[iCntxIndex].bRegistering))
		{
			BT_BOOL bDeRegResult = FALSE;

			bDeRegResult = btmtk_jsr82_session_service_deregistration(g_jsr82MMIContext[iCntxIndex].ps_type, g_jsr82MMIContext[iCntxIndex].fd, 
							g_jsr82MMIContext[iCntxIndex].index, &statusResult);
			if (TRUE != bDeRegResult)
			{
				XLOGE("[JSR82][JNI] abortNative: btmtk_jsr82_session_service_deregistration failed.");
				//jniThrowIOException(env, ECONNABORTED);
				return -1;
			}
		}

		pthread_mutex_lock(&(nat->thread_mutex));
		// Stop waiting procedure
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82WriteCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82RegisterCond));
		pthread_mutex_unlock(&(nat->thread_mutex));
	}
	else
	{
		XLOGE("[JSR82][JNI] abortNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}

	XLOGI("[JSR82][JNI] abortNative ---.");
	return 0;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static int destroyNative(JNIEnv *env, jobject obj, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
//	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = -1;

	// De-initialization this socket (port)
	XLOGI("[JSR82][JNI] destroyNative: fd=%d.", fdHandle);

//	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	for (int iCount = 0; iCount < JSR82_PORT_NUM; iCount++)
	{
		if (fdHandle == g_jsr82MMIContext[iCount].fd)
		{
			iCntxIndex = iCount;
			break;
		}
	}
	if (0 <= iCntxIndex)
	{
		pthread_mutex_lock(&(nat->thread_mutex));
		pthread_cond_destroy(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond));
		pthread_cond_destroy(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond));
		pthread_cond_destroy(&(g_jsr82MMIContext[iCntxIndex].jsr82WriteCond));
		pthread_cond_destroy(&(g_jsr82MMIContext[iCntxIndex].jsr82RegisterCond));
		pthread_mutex_unlock(&(nat->thread_mutex));
		//For client, reset contect if 1. Not connected   2. Client port
		/*For Server, abort operation takes chage of deregistration*/
		if ((JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bConnected) && (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bServer))
		//if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bServer)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			btmtk_jsr82_clear_cntx(iCntxIndex);
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] destroyNative: The FD (%d) doesn't exist.", fdHandle);
		//jniThrowIOException(env, EINVAL);
		return -1;
	}

	return 0;
#endif
	jniThrowIOException(env, ENOSYS);
	return -1;
}


static void throwErrnoNative(JNIEnv *env, jobject obj, jint err, jint fdHandle) 
{
	XLOGI("[JSR82][JNI] throwErrnoNative: fd=%d.", fdHandle);
	jniThrowIOException(env, err);
}


static jstring getAddrNative(JNIEnv *env, jobject obj, jint fdHandle) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	jboolean bSearchResult = JNI_FALSE;
	int iCntxIndex = 0xFF;
	char addr_cstr[BDADDR_SIZE];
	jstring addr_jstr;

	// This API is used to get the BD_ADDR string after connection is created (Server)
	XLOGI("[JSR82][JNI] getAddrNative: fd=%d.", fdHandle);

	bSearchResult = btmtk_jsr82_search_cntx_by_fd(fdHandle, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bConnected)
		{
			sprintf(addr_cstr, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			        g_jsr82MMIContext[iCntxIndex].bdAddr[0], g_jsr82MMIContext[iCntxIndex].bdAddr[1], g_jsr82MMIContext[iCntxIndex].bdAddr[2], 
			        g_jsr82MMIContext[iCntxIndex].bdAddr[3], g_jsr82MMIContext[iCntxIndex].bdAddr[4], g_jsr82MMIContext[iCntxIndex].bdAddr[5]);

			XLOGI("[JSR82][JNI] getAddrNative: getAddrNative addr_cstr=<%s>", addr_cstr);

			addr_jstr = env->NewStringUTF(addr_cstr);
			return addr_jstr;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] getAddrNative: The FD (%d) doesn't exist.", fdHandle);
		return NULL;
	}

#endif
    return NULL;
}


static int getRealServerChannelNative(JNIEnv *env, jobject obj, jint channelOriginal) 
{
    XLOGV(__FUNCTION__);
#ifdef __BTMTK__		///////////////////////////////////////////////////////////
	// This API is used to get the channel number with the channel that application would like to register originally (Server)
	XLOGI("[JSR82][JNI] getRealServerChannelNative: channelOriginal=%d.", channelOriginal);

	int iCount = 0;

	for (iCount = 0; iCount < JSR82_PORT_NUM; iCount++)
	{
		if ((JNI_TRUE == g_jsr82MMIContext[iCount].inUse) && 
			(channelOriginal == g_jsr82MMIContext[iCount].port))
		{
			return g_jsr82MMIContext[iCount].channelNumber;
		}
	}
	
	return -1;
#endif
    return -1;
}


static JNINativeMethod sMethods[] = 
{
    {"initNative", "()V",  (void*) initNative},
    {"cleanupNative", "()V",  (void*) cleanupNative},
    {"initSocketNative", "(IZZI)I",  (void*) initSocketNative},
    {"connectNative", "(ILjava/lang/String;I)I", (void *) connectNative},
    {"bindListenNative", "(I)I", (void *) bindListenNative},
    {"acceptNative", "(II)I", (void *) acceptNative},
    {"availableNative", "(I)I",    (void *) availableNative},
    {"readNative", "([BIII)I",    (void *) readNative},
    {"writeNative", "([BIII)I",    (void *) writeNative},
    {"abortNative", "(I)I",    (void *) abortNative},
    {"destroyNative", "(I)I",    (void *) destroyNative},
    {"throwErrnoNative", "(II)V",    (void *) throwErrnoNative},
    {"getAddrNative", "(I)Ljava/lang/String;", (void *) getAddrNative},
    {"getRealServerChannelNative", "(I)I",    (void *) getRealServerChannelNative},
};


int register_android_bluetooth_BluetoothSocketService(JNIEnv *env) 
{
    jclass clazz = env->FindClass("android/server/BluetoothSocketService");
    if (clazz == NULL)
        return -1;
    return AndroidRuntime::registerNativeMethods(env,
        "android/server/BluetoothSocketService", sMethods, NELEM(sMethods));
}

#ifdef __BTMTK__
void btmtk_jsr82_android_cb_enable_service_cnf(void *parms)
{
	bt_jsr82_enable_service_cnf_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;
	
	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_ENABLE_SERVICE_CNF");

	msg = (bt_jsr82_enable_service_cnf_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, channel=%d, identify=%d, result=%d", msg->index, msg->ps_type, msg->channel, (int)msg->identify, msg->result);
	bSearchResult = btmtk_jsr82_search_cntx_by_fd(msg->identify, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if (JNI_TRUE != g_jsr82MMIContext[iCntxIndex].bRegistering)
		{
			XLOGE("[JSR82][JNI] ERROR! This port is not requested to enable.");
			return ;
		}
		
		if (JSR82_SESSION_REGISTRARION_SUCCESS == msg->result)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			g_jsr82MMIContext[iCntxIndex].index = msg->index;
			g_jsr82MMIContext[iCntxIndex].channelNumber = msg->channel;
			g_jsr82MMIContext[iCntxIndex].bRegistering = JNI_FALSE;
			pthread_mutex_unlock(&(nat->thread_mutex));

			// Check if this port is aborting
			if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting)
			{
				// De-register this port
				BT_BOOL bDeRegResult = FALSE;
				U8 statusResult = 0;

				bDeRegResult = btmtk_jsr82_session_service_deregistration(g_jsr82MMIContext[iCntxIndex].ps_type, g_jsr82MMIContext[iCntxIndex].fd, 
								g_jsr82MMIContext[iCntxIndex].index, &statusResult);
				if (TRUE != bDeRegResult)
				{
					XLOGE("[JSR82][JNI] abortNative: btmtk_jsr82_session_service_deregistration failed.");
					return ;
				}
			}

			pthread_mutex_lock(&(nat->thread_mutex));
			// Signal the waiting REGISTER request
			// Do this signaling after checking for aborting status to avoid context switch
			pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82RegisterCond));
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
		else
		{
			XLOGE("[JSR82][JNI] Service registration failed for index (%d), channel=%d.", msg->index, msg->channel);
			// Signal the waiting REGISTER request
			pthread_mutex_lock(&(nat->thread_mutex));
			pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82RegisterCond));
			pthread_mutex_unlock(&(nat->thread_mutex));
			return ;
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}


void btmtk_jsr82_android_cb_disable_service_cnf(void *parms)
{
	bt_jsr82_disable_service_cnf_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_DISABLE_SERVICE_CNF");

	msg = (bt_jsr82_disable_service_cnf_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, identify=%d, result=%d", msg->index, msg->ps_type, (int)msg->identify, msg->result);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if (BT_JSR82_SESSION_DISABLED_SUCCESS == msg->result)
		{
			g_jsr82MMIContext[iCntxIndex].bConnected = FALSE;;
		}
		else
		{
			XLOGE("[JSR82][JNI] Service de-registration failed for index (%d), identify=%d.", msg->index, (int)msg->identify);
		}

		// Clear context
		pthread_mutex_lock(&(nat->thread_mutex));

		/*Stop waiting procedure*/
		g_jsr82MMIContext[iCntxIndex].bAborting = TRUE;
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82WriteCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82RegisterCond));
		
		btmtk_jsr82_clear_cntx(iCntxIndex);
		pthread_mutex_unlock(&(nat->thread_mutex));
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}


void btmtk_jsr82_android_cb_turn_on_service_cnf(void *parms)
{
	bt_jsr82_turnon_service_cnf_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_TURNON_SERVICE_CNF");

	msg = (bt_jsr82_turnon_service_cnf_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, identify=%d, result=%d", msg->index, msg->ps_type, (int)msg->identify, msg->result);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if ((int)msg->identify != g_jsr82MMIContext[iCntxIndex].fd)
		{
			XLOGE("[JSR82][JNI] transaction_id (%d) doesn't match with fd (%d)", (int)msg->identify, g_jsr82MMIContext[iCntxIndex].fd);
			return ;
		}
		pthread_mutex_lock(&(nat->thread_mutex));
		if (JSR82_SESSION_TURNON_SUCCESS != msg->result)
		{
			XLOGE("[JSR82][JNI] Service Turn-ON failed.");
			g_jsr82MMIContext[iCntxIndex].serverState = SERVER_STATE_NONE;
		}
		else
		{
			g_jsr82MMIContext[iCntxIndex].serverState = SERVER_STATE_TURN_ON;			
		}
		pthread_mutex_unlock(&(nat->thread_mutex));
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}


void btmtk_jsr82_android_cb_turn_off_service_cnf(void *parms)
{
	bt_jsr82_turnoff_service_cnf_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_TURNOFF_SERVICE_CNF");

	msg = (bt_jsr82_turnoff_service_cnf_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, identify=%d, result=%d", msg->index, msg->ps_type, (int)msg->identify, msg->result);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if ((int)msg->identify != g_jsr82MMIContext[iCntxIndex].fd)
		{
			XLOGE("[JSR82][JNI] transaction_id (%d) doesn't match with fd (%d)", (int)msg->identify, g_jsr82MMIContext[iCntxIndex].fd);
			return ;
		}
		if (JSR82_SESSION_TURNOFF_SUCCESS != msg->result)
		{
			XLOGE("[JSR82][JNI] Service Turn-OFF failed.");
			return ;
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}


void btmtk_jsr82_android_cb_connect_ind(void *parms)
{
	bt_jsr82_connect_ind_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_CONNECT_IND");

	msg = (bt_jsr82_connect_ind_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, l2cap_id=%d, channel=%d, identify=%d, result=%d", 
			msg->index, msg->ps_type, msg->l2cap_id, (int)msg->channel, (int)msg->identify, msg->rsp_result);
	XLOGI("[JSR82][JNI] BD_ADDR <%x, %x, %x, %x, %x, %x>", msg->bd_addr[0], msg->bd_addr[1], msg->bd_addr[2], 
			msg->bd_addr[3], msg->bd_addr[4], msg->bd_addr[5]);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if (JSR82_SESSION_CONNECT_IND_CONNECTED == msg->rsp_result)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			g_jsr82MMIContext[iCntxIndex].bConnected = JNI_TRUE;
			g_jsr82MMIContext[iCntxIndex].l2cap_id = msg->l2cap_id;
			pthread_mutex_unlock(&(nat->thread_mutex));
			memcpy(g_jsr82MMIContext[iCntxIndex].bdAddr, msg->bd_addr, 6);
			XLOGI("[JSR82][JNI] BD_ADDR in context <%x, %x, %x, %x, %x, %x>", g_jsr82MMIContext[iCntxIndex].bdAddr[0], g_jsr82MMIContext[iCntxIndex].bdAddr[1], g_jsr82MMIContext[iCntxIndex].bdAddr[2], 
					g_jsr82MMIContext[iCntxIndex].bdAddr[3],g_jsr82MMIContext[iCntxIndex].bdAddr[4], g_jsr82MMIContext[iCntxIndex].bdAddr[5]);
			// Signal the waiting thread to return a Bluetooth Socket object
			pthread_mutex_lock(&(nat->thread_mutex));
			pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond));
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
		else
		{
			XLOGE("[JSR82][JNI] Un-handled connection result (%d) for index (%d).", msg->rsp_result, msg->index);
			return ;
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}


void btmtk_jsr82_android_cb_connect_cnf(void *parms)
{
	bt_jsr82_connect_cnf_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_CONNECT_CNF");

	msg = (bt_jsr82_connect_cnf_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, l2cap_id=%d, channel=%d, identify=%d, result=%d", 
			msg->index, msg->ps_type, msg->l2cap_id, (int)msg->channel, (int)msg->identify, msg->result);
	XLOGI("[JSR82][JNI] BD_ADDR <%x, %x, %x, %x, %x, %x>]", msg->bd_addr[0], msg->bd_addr[1], msg->bd_addr[2], 
			msg->bd_addr[3], msg->bd_addr[4], msg->bd_addr[5]);
	bSearchResult = btmtk_jsr82_search_cntx_by_fd(msg->identify, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		if (JSR82_SESSION_CONNECT_CLIENT_SUCCESS == msg->result)
		{
			pthread_mutex_lock(&(nat->thread_mutex));
			g_jsr82MMIContext[iCntxIndex].index = msg->index;
			g_jsr82MMIContext[iCntxIndex].l2cap_id = msg->l2cap_id;
			g_jsr82MMIContext[iCntxIndex].bConnected = JNI_TRUE;
			// Signal the waiting thread to return
			pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond));
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
		else
		{
			XLOGE("[JSR82][JNI] Connection_CNF failed for identify (%d).", (int)msg->identify);
			pthread_mutex_lock(&(nat->thread_mutex));
			g_jsr82MMIContext[iCntxIndex].bConnected = JNI_FALSE;
			// Signal the waiting thread to return ERROR
			pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ConnectCond));
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		if (JSR82_SESSION_CONNECT_CLIENT_SUCCESS == msg->result)
		{
			U8 status_result;
			btmtk_jsr82_session_disconnect_req(msg->identify, msg->ps_type, msg->index, msg->l2cap_id, &status_result);
		}
		return ;
	}
}


void btmtk_jsr82_android_cb_disconnect_ind(void *parms)
{
	bt_jsr82_disconnect_ind_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_DISCONNECT_IND");

	msg = (bt_jsr82_disconnect_ind_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, ps_type=%d, identify=%d, l2cap_id=%d", msg->index, msg->ps_type, (int)msg->identify, msg->l2cap_id);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		pthread_mutex_lock(&(nat->thread_mutex));
		g_jsr82MMIContext[iCntxIndex].bConnected = JNI_FALSE;
		pthread_mutex_unlock(&(nat->thread_mutex));

		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82WriteCond));

		if (JNI_FALSE == g_jsr82MMIContext[iCntxIndex].bServer)
		{			
			pthread_mutex_lock(&(nat->thread_mutex));
			if (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting) 
			{
				btmtk_jsr82_clear_cntx(iCntxIndex);
			}
			pthread_mutex_unlock(&(nat->thread_mutex));
		}
		// Check if it is necessary to DISABLE this port: Server case
		else if ((JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bServer) && (JNI_TRUE == g_jsr82MMIContext[iCntxIndex].bAborting))
		{
			// De-register this Server port
			BT_BOOL bDeRegResult = FALSE;
			U8 statusResult = 0;

			XLOGE("[JSR82][JNI] btmtk_jsr82_session_service_deregistration =========== CCC");			
			bDeRegResult = btmtk_jsr82_session_service_deregistration(g_jsr82MMIContext[iCntxIndex].ps_type, g_jsr82MMIContext[iCntxIndex].fd, 
							g_jsr82MMIContext[iCntxIndex].index, &statusResult);
			if (TRUE != bDeRegResult)
			{
				XLOGE("[JSR82][JNI] abortNative: btmtk_jsr82_session_service_deregistration failed.");
				return ;
			}
		}
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}


void btmtk_jsr82_android_cb_rx_ready_ind(void *parms)
{
	bt_jsr82_rx_ready_ind_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;

	XLOGI("[JSR82][JNI] Event: EVENT_JSR82_MMI_RX_READY_IND");

	msg = (bt_jsr82_rx_ready_ind_struct*) parms;
	XLOGI("[JSR82][JNI] index=%d, l2cap_id=%d, ps_type=%d, length=%d", msg->index, msg->l2cap_id, msg->ps_type, msg->length);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		// Signal the waiting READ request
		pthread_mutex_lock(&(nat->thread_mutex));
		pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82ReadCond));
		pthread_mutex_unlock(&(nat->thread_mutex));
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}
void btmtk_jsr82_android_cb_tx_ready_ind(void *parms)
{
	bt_jsr82_tx_ready_ind_struct *msg;
	int iCntxIndex = 0xFF;
	jboolean bSearchResult = JNI_FALSE;
	msg = (bt_jsr82_tx_ready_ind_struct*) parms;
	XLOGI("[JSR82][JNI] EVENT_JSR82_MMI_TX_READY_IND: index=%d, l2cap_id=%d, ps_type=%d, isTxEmpty=%d", 
		msg->index, msg->l2cap_id, msg->ps_type, msg->isTxEmpty);
	bSearchResult = btmtk_jsr82_search_cntx_by_conn_index(msg->index, &iCntxIndex);
	if (JNI_TRUE == bSearchResult)
	{
		// Signal the waiting READ request
		pthread_mutex_lock(&(nat->thread_mutex));
		g_jsr82MMIContext[iCntxIndex].bWriting = (msg->isTxEmpty) ? JNI_FALSE : JNI_TRUE;
			pthread_cond_signal(&(g_jsr82MMIContext[iCntxIndex].jsr82WriteCond));
		pthread_mutex_unlock(&(nat->thread_mutex));
	}
	else
	{
		XLOGE("[JSR82][JNI] There is no corresponding context for index (%d).", msg->index);
		return ;
	}
}



void btmtk_jsr82_cb_event_handler(void *context, BT_CALLBACK_EVENT event, void *parms, U16 datasize)
{
	switch(event)
	{
		/* To JSR82 VM */
		case EVENT_JSR82_MMI_ENABLE_SERVICE_CNF:
			btmtk_jsr82_android_cb_enable_service_cnf(parms);
			break;
		case EVENT_JSR82_MMI_DISABLE_SERVICE_CNF:
			btmtk_jsr82_android_cb_disable_service_cnf(parms);
			break;
		case EVENT_JSR82_MMI_TURNON_SERVICE_CNF:
			btmtk_jsr82_android_cb_turn_on_service_cnf(parms);
			break;
		case EVENT_JSR82_MMI_TURNOFF_SERVICE_CNF:
			btmtk_jsr82_android_cb_turn_off_service_cnf(parms);
			break;
		case EVENT_JSR82_MMI_CONNECT_IND:
			btmtk_jsr82_android_cb_connect_ind(parms);
			break;
		case EVENT_JSR82_MMI_CONNECT_CNF:
			btmtk_jsr82_android_cb_connect_cnf(parms);
			break;
		case EVENT_JSR82_MMI_DISCONNECT_IND:
			btmtk_jsr82_android_cb_disconnect_ind(parms);
			break;
		case EVENT_JSR82_MMI_RX_READY_IND:
			btmtk_jsr82_android_cb_rx_ready_ind(parms);
			break;
		case EVENT_JSR82_MMI_TX_READY_IND:
			btmtk_jsr82_android_cb_tx_ready_ind(parms);
			break;

		default:
			// Report error
			XLOGE("[JSR82][JNI] Event: Unhandled event (%d)", event);
			break;
	}
}


static jboolean jsr82_startEventLoop(JNIEnv* env, jobject object)
{
    XLOGI("[JSR82][JNI] jsr82_startEventLoop");

    pthread_mutex_lock(&(nat->thread_mutex));
    
    env->GetJavaVM( &(nat->vm) );
    nat->envVer = env->GetVersion();
    nat->me = env->NewGlobalRef(object);
    
    pthread_create(&(nat->thread), NULL, jsr82_eventLoopMain, nat);
    
    pthread_mutex_unlock(&(nat->thread_mutex));
    return JNI_TRUE;
}


static void *jsr82_eventLoopMain(void *ptr)
{
    JNIEnv *env;
    int res = 0;
    fd_set readfs;
    int sockfd;
    ilm_struct ilm;

    JavaVMAttachArgs args;
    char name[] = "BT JSR82 EventLoop";

    args.version = nat->envVer;
    args.name = name;
    args.group = NULL;
    
    nat->vm->AttachCurrentThread(&env, &args);
    
    XLOGI("[JSR82][JNI] jsr82_eventLoopMain");

    XLOGI("[JSR82][JNI][EventLoop] JSR82 service loop start.");
    while (1)
    {
        sockfd = nat->jsr82srvcsock;
        FD_ZERO(&readfs);
        if(0 <= sockfd)
        {
            FD_SET(sockfd, &readfs);
        }
        else
        {
            XLOGE("[JSR82][JNI][EventLoop][ERR] nat->jsr82srvcsock == 0. exit");
        }

        res = select(sockfd+1, &readfs, NULL, NULL, NULL);
        if(0 < res)
        {
            res = recvfrom(sockfd, (void*)&ilm, sizeof(ilm_struct), 0, NULL, NULL);
            XLOGI("[JSR82][JNI][EventLoop] Recv JSR82 CNF/IND : %d", (int)ilm.msg_id);
            if(0 > res)
            {
                XLOGE("[JSR82][JNI][EventLoop][ERR] recvfrom failed : %s, %d", strerror(errno), errno);
            }
            else
            {
                btmtk_jsr82_handle_message(&ilm);

                nat->vm->GetEnv((void**)&env, nat->envVer);
            }
        }
        else if(0 == res)
        {
            XLOGW("[JSR82][JNI][EventLoop][ERR] timeout waiting indication");
            break;
        }
        else
        {
            if ( errno != EINTR ) {
                XLOGE("[JSR82][JNI][EventLoop][ERR] select failed : %s, %d", strerror(errno), errno);
                break;
            }
        }
    }
    if(res <= 0)
    {
        nat->vm->DetachCurrentThread();
    }
    return NULL;
}


static jboolean jsr82_registerSocket(JNIEnv *env, jobject object)
{
    sockaddr_un jsr82extname;
    socklen_t   jsr82extnamelen;
    struct sockaddr_un jsr82intname;
    socklen_t   jsr82intnamelen;    

    XLOGI("[JSR82][JNI] jsr82_registerSocket +++");

    //-----------------------------------------------------------------
    //           start setup socket
    //-----------------------------------------------------------------
    // Setup bt server address
    jsr82intname.sun_family = AF_UNIX;
    strcpy (jsr82intname.sun_path, /*BT_SERV_SOCK_ADDR*/BT_SOCK_NAME_INT_ADP);
    jsr82intnamelen = (offsetof (struct sockaddr_un, sun_path) + strlen (jsr82intname.sun_path) + 1);    
    // Setup JSR82 service socket
    nat->jsr82srvcsock = socket_local_server(BT_SOCK_NAME_EXT_ADP_JSR82, 
                                                                      ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
                                                                      SOCK_DGRAM);
    XLOGI("[JSR82][JNI] jsr82_registerSocket: socket_local_server() done.");
    if(0 > nat->jsr82srvcsock)
    {
        XLOGE("[JSR82][JNI][ERR] create JSR82 server socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }
    else
    {
        jsr82extnamelen = sizeof(jsr82extname.sun_path);
        jsr82extname.sun_path[0] = '\0';
	 XLOGI("[JSR82][JNI] create JSR82 server socket success");
        if (0 > getsockname(nat->jsr82srvcsock, (sockaddr*)&jsr82extname, &jsr82extnamelen))
        {
            XLOGI("[JSR82][JNI] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            XLOGI("[JSR82][JNI] Auto bind JSR82 server : len=%d, addr=%s", jsr82extnamelen, &jsr82extname.sun_path[1]);
        }
    }

    btmtk_jsr82_setExtSockAddress(&jsr82extname, jsr82extnamelen);
    XLOGI("[JSR82][JNI] jsr82_registerSocket: btmtk_jsr82_setExtSockAddress() done.");

    // Setup JSR82 api socket
    jsr82extnamelen = sizeof(short);
    nat->jsr82sock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    XLOGI("[JSR82][JNI] nat->jsr82sock==%d", nat->jsr82sock);
    if (nat->jsr82sock < 0)
    {
        XLOGE("[JSR82][JNI][ERR] create JSR82 api socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }
    if (bind (nat->jsr82sock, (struct sockaddr *) &jsr82extname, jsr82extnamelen) < 0)
    {
        XLOGE("[JSR82][JNI][ERR] bind JSR82 api socket failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }
    else
    {
        jsr82extnamelen = sizeof(jsr82extname.sun_path);
        jsr82extname.sun_path[0] = '\0';
        if (getsockname(nat->jsr82sock, (sockaddr*)&jsr82extname, &jsr82extnamelen) < 0)
        {
            XLOGE("[JSR82][JNI][ERR] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            XLOGI("[JSR82][JNI] Auto bind JSR82 api socket : len=%d, addr=%s", jsr82extnamelen, &jsr82extname.sun_path[1]);
        }
    }
    if ( connect(nat->jsr82sock, (const struct sockaddr*)&jsr82intname, jsr82intnamelen) < 0)
    {
        XLOGE("[JSR82][JNI][ERR] connect to /data/btserv failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }

    //-----------------------------------------------------------------
    //           Start receiving indication
    //-----------------------------------------------------------------
    XLOGI("[JSR82][JNI] jsr82_registerSocket: Start Event Loop.");
    jsr82_startEventLoop(env, object);

    btmtk_jsr82_setSockFd(nat->jsr82srvcsock, nat->jsr82sock);

    btmtk_jsr82_register_mmi_callback_req(btmtk_jsr82_cb_event_handler);
    btmtk_jbt_init();

    XLOGI("[JSR82][JNI] jsr82_registerSocket ---");
    return JNI_TRUE;

exit:
    if (0 <= nat->jsr82srvcsock)
    {
        close(nat->jsr82srvcsock);
        nat->jsr82srvcsock = -1;
    }
    if (0 <= nat->jsr82sock)
    {
        close(nat->jsr82sock);
        nat->jsr82sock = -1;
    }
    XLOGI("[JSR82][JNI] jsr82_registerSocket ---");
    return JNI_FALSE;
}
#endif

} /* namespace android */




