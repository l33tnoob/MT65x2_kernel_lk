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

//#define LOG_TAG "libmtkhdmi_jni"
#define LOG_TAG "hdmi"
/*
typedef unsigned char       u8;
typedef signed char         s8;
typedef unsigned short      u16;
typedef signed short        s16;
typedef unsigned int        u32;
typedef signed int          s32;
typedef unsigned long long  u64;
typedef signed long long    s64;
*/

#define EDIDNUM 2

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <utils/misc.h>
#include <utils/Log.h>

#include "jni.h"
#include "JNIHelp.h"

#include <linux/hdmitx.h>
#include <linux/mtkfb_info.h>

#if defined (MTK_DRM_KEY_MNG_SUPPORT)
#include "keyblock.h"
#endif

using namespace android;

#ifdef __cplusplus
    extern "C" {
#endif

#if defined(MTK_HDMI_SUPPORT)
static jboolean hdmi_ioctl(int code, int value)
{
    int fd = open("/dev/hdmitx", O_RDONLY, 0);
    int ret;
    if (fd >= 0) {
        ret = ioctl(fd, code, value);
        if (ret == -1) {
            ALOGE("[HDMI] [%s] failed. ioctlCode: %d, errno: %d",
                 __func__, code, errno);
            return 0;
        }
        close(fd);
    } else {
        ALOGE("[HDMI] [%s] open mtkfb failed. errno: %d", __func__, errno);
        return 0;
    }
    return ret;
}
#endif


static jboolean enableHDMI(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE, enable);
#endif

	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableHDMI(%d)\n", enable);
    return ret == 0;
}


static jboolean IPOPowerONHDMI(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    if(enable)
    {
        ret = hdmi_ioctl(MTK_HDMI_IPO_POWERON, 1);
    }
    else
    {
        ret = hdmi_ioctl(MTK_HDMI_IPO_POWERON, 0);
    }    

#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableHDMIIPO(%d)\n", enable);

    return ret;
}

static jboolean hdmiPowerEnable(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    if(enable)
    {
        ret = hdmi_ioctl(MTK_HDMI_POWER_ENABLE, 1);
    }
    else
    {
        ret = hdmi_ioctl(MTK_HDMI_POWER_ENABLE, 0);
    }    

#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableHDMIPOWER(%d)\n", enable);

    return ret;
}

static jboolean hdmiPortraitEnable(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    if(enable)
    {
        ret = hdmi_ioctl(MTK_HDMI_PORTRAIT_ENABLE, 1);
    }
    else
    {
        ret = hdmi_ioctl(MTK_HDMI_PORTRAIT_ENABLE, 0);
    }    

#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableHDMIPortrait(%d)\n", enable);

    return ret;
}

static jboolean enableVideo(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_VIDEO_ENABLE, enable);
#endif

	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableVideo(%d)\n", enable);
    return ret == 0;
}

static jboolean enableCEC(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_CEC_ENABLE, enable);
#endif

	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableCEC(%d)\n", enable);
    return ret;
}

static jboolean enableHDCP(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_MT8193_HDCP_SUPPORT)||defined (MTK_HDMI_HDCP_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_ENABLE_HDCP, enable);
#endif

	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableHDCP(%d)\n", enable);
    return ret;
}

static jboolean enableAudio(JNIEnv *env, jobject clazz, jboolean enable) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_AUDIO_ENABLE, enable);
#endif

	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.enableAudio(%d)\n", enable);
    return ret;
}




static jboolean GetDeviceStatus(JNIEnv* env, jobject clazz, jboolean is_audio_enabled, jboolean is_video_enabled ) {
    bool ret = false;
    hdmi_device_status h;

#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_GET_DEVICE_STATUS, (int)&h);
#endif

	is_audio_enabled = h.is_audio_enabled;
	is_video_enabled = h.is_video_enabled;

	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMI.GetDeviceStatus(%d %d)\n", is_audio_enabled, is_video_enabled);
    return ret;
}

static jintArray getEDID(JNIEnv* env, jobject clazz) {
    bool ret = false;
    jint cResult[EDIDNUM];
    jintArray jResult = NULL;
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)||defined (MTK_INTERNAL_MHL_SUPPORT)
    HDMI_EDID_T edid;        
    ret = hdmi_ioctl(MTK_HDMI_GET_EDID, (int)&edid);
    if (!ret) {
        jResult = env->NewIntArray(EDIDNUM);
        ALOGI("[HDMI] edid.ui4_ntsc_resolution %4X\n", edid.ui4_ntsc_resolution);
        ALOGI("[HDMI] edid.ui4_pal_resolution %4X\n", edid.ui4_pal_resolution);
        //ALOGI("[HDMI] edid.ui1_sink_rgb_color_bit  %4X\n", edid.ui1_sink_rgb_color_bit);
        //ALOGI("[HDMI] edid.ui1_sink_ycbcr_color_bit  %4X\n", edid.ui1_sink_ycbcr_color_bit);
        //ALOGI("[HDMI] edid.ui2_sink_colorimetry  %4X\n", edid.ui2_sink_colorimetry);
        cResult[0] = edid.ui4_ntsc_resolution;
        cResult[1] = edid.ui4_pal_resolution;
        //cResult[1] = (unsigned int)edid.ui1_sink_rgb_color_bit;
        //cResult[2] = (unsigned int)edid.ui1_sink_ycbcr_color_bit;
        //cResult[3] = (unsigned int)edid.ui2_sink_colorimetry;
        env->SetIntArrayRegion(jResult, 0, EDIDNUM, cResult);
    }
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.getEDID\n");
	return jResult;
}

static jcharArray getCECAddr(JNIEnv* env, jobject clazz) {
    bool ret = false;
    jchar cResult[2];
    jcharArray jResult = NULL;
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)
    CEC_ADDRESS_IO cecAddr;
    ret = hdmi_ioctl(MTK_HDMI_GET_CECADDR, (int)&cecAddr);
    if (!ret) {
        jResult = env->NewCharArray(2);
        ALOGI("[HDMI] cecAddr.ui1_la %4X\n", cecAddr.ui1_la);
        ALOGI("[HDMI] cecAddr.ui2_pa %4X\n", cecAddr.ui2_pa);
        cResult[0] = cecAddr.ui1_la;
        cResult[1] = cecAddr.ui2_pa;
        env->SetCharArrayRegion(jResult, 0, 2, cResult);
    }    
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.getCECAddr\n");
	return jResult;
}

static jintArray getCECCmd(JNIEnv* env, jobject clazz) {
    bool ret = false;    
    int i;
    jint cResult[22];
    jintArray jResult = NULL;
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)    
    CEC_FRAME_DESCRIPTION_IO cecCmd;    
    ret = hdmi_ioctl(MTK_HDMI_GET_CECCMD, (int)&cecCmd);
    if (!ret) {
        jResult = env->NewIntArray(22);
        ALOGI("[HDMI] cecCmd.size %d\n", cecCmd.size);
        ALOGI("[HDMI] cecCmd.sendidx %d\n", cecCmd.sendidx);
        ALOGI("[HDMI] cecCmd.reTXcnt %d\n", cecCmd.size);
        ALOGI("[HDMI] cecCmd.blocks.header.destination %d\n", cecCmd.blocks.header.destination);
        ALOGI("[HDMI] cecCmd.blocks.header.initiator %d\n", cecCmd.blocks.header.initiator);
        ALOGI("[HDMI] cecCmd.blocks.opcode %d\n", cecCmd.blocks.opcode);
        for (i = 0; i < 15; i++) {
            ALOGI("[HDMI] cecCmd.blocks.operand[%d] %d\n", i, cecCmd.blocks.operand[i]);
        }
        cResult[0] = cecCmd.size;
        cResult[1] = cecCmd.sendidx;
        cResult[2] = cecCmd.reTXcnt;
        cResult[3] = (unsigned int)cecCmd.txtag;
        cResult[4] = cecCmd.blocks.header.destination;
        cResult[5] = cecCmd.blocks.header.initiator;
        cResult[6] = cecCmd.blocks.opcode;
        for (i = 0; i < 15; i++) {
            cResult[i + 7] = cecCmd.blocks.operand[i];
        }
        env->SetIntArrayRegion(jResult, 0, 22, cResult);
    }
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.getCECCmd\n");
	return jResult;
}


static jboolean setVideoConfig(JNIEnv* env, jobject clazz, jint vformat) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_VIDEO_CONFIG, vformat);
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setVideoConfig(%d)\n", vformat);
    return ret == 0;
}

static jboolean setAudioConfig(JNIEnv* env, jobject clazz, jint aformat) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_AUDIO_CONFIG, aformat);
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setAudioConfig(%d)\n", aformat);
    return ret;
}

static jboolean setDeepColor(JNIEnv* env, jobject clazz, jint colorSpace, jint deepColor) {
    bool ret = false;			
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)
    hdmi_para_setting h;
	h.u4Data1 = colorSpace;
	h.u4Data2 = deepColor;
	ret = hdmi_ioctl(MTK_HDMI_COLOR_DEEP, (int)&h);
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setDeepColor(%d,%d)\n", colorSpace, deepColor);
	return ret == 0;
}

static jboolean setHDCPKey(JNIEnv* env, jobject clazz, jbyteArray key) {
    bool ret = false;	
#if defined (MTK_MT8193_HDCP_SUPPORT)||defined (MTK_HDMI_HDCP_SUPPORT)
    jbyte* hKeyTemp;
	hdmi_hdcp_key hKey;
    int i;
	hKeyTemp = env->GetByteArrayElements(key, NULL);
	memcpy(hKey.u1Hdcpkey, (unsigned char*)hKeyTemp, sizeof(hKey.u1Hdcpkey));
    for (i=0; i<287; i++) {
        ALOGI("[HDMI] JNI setHDCPKey key[%d] = %d\n", i, hKey.u1Hdcpkey[i]);
    }
	ret = hdmi_ioctl(MTK_HDMI_HDCP_KEY, (int)&hKey);
	env->ReleaseByteArrayElements(key, hKeyTemp, 0);
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setHDCPKey\n");
	return ret;
}

static jboolean setHDMIDRMKey(JNIEnv* env, jobject clazz) {
    bool ret = false;	
#if defined (MTK_HDMI_SUPPORT)
#if defined (MTK_DRM_KEY_MNG_SUPPORT)


    ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setHDMIDRMKey end\n");
	hdmi_hdcp_drmkey hKey;
    int i;
    int ret_temp = 0;
    unsigned char* enckbdrm = NULL;
    unsigned int inlength = 0;


    ret_temp = get_encrypt_drmkey(HDCP_1X_TX_ID,&enckbdrm,&inlength);    
        if(ret_temp !=0 ) {
            ALOGI("[HDMI] JNI setHDMIDRMKey get_encrypt_drmkey failed %d", ret_temp);
            return ret;
        }
    
	memcpy(hKey.u1Hdcpkey, (unsigned char*)enckbdrm, sizeof(hKey.u1Hdcpkey));
    for (i=0; i<sizeof(hKey.u1Hdcpkey); i++) {
        ALOGI("[HDMI] JNI setHDMIDRMKey key[%d] = %x\n", i, hKey.u1Hdcpkey[i]);
    }
	ret = hdmi_ioctl(MTK_HDMI_HDCP_KEY, (int)&hKey);
    ALOGI("[HDMI] JNI setHDMIDRMKey ret = %d\n",ret);
    free_encrypt_drmkey(enckbdrm);
#endif
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setHDMIDRMKey end\n");
	return ret;
}

static jboolean NeedSwDrm(JNIEnv *env, jobject clazz) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    int needSwDrmProtect = 0;
    hdmi_ioctl(MTK_HDMI_GET_DRM_ENABLE, (unsigned int)&needSwDrmProtect);
    ret = (needSwDrmProtect)? true : false;
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.needSwDrmProtect()\n");
    return ret;
}

static jboolean setCECAddr(JNIEnv* env, jobject clazz, jbyte laNum, jbyteArray la, jchar pa, jchar svc) {
    bool ret = false;	
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)
    jbyte* laTemp;
    CEC_DRV_ADDR_CFG CECAddr;
    laTemp = env->GetByteArrayElements(la, NULL);
    CECAddr.ui1_la_num = laNum;
    memcpy(CECAddr.e_la, (unsigned char*)laTemp, sizeof(CECAddr.e_la));
    CECAddr.ui2_pa = pa;
    CECAddr.h_cecm_svc = svc;
    ALOGI("[HDMI] JNI setCECAddr CECAddr.ui1_la_num = %d\n", CECAddr.ui1_la_num);
    ALOGI("[HDMI] JNI setCECAddr CECAddr.e_la[0] = %d\n", CECAddr.e_la[0]);
    ALOGI("[HDMI] JNI setCECAddr CECAddr.e_la[1] = %d\n", CECAddr.e_la[1]);
    ALOGI("[HDMI] JNI setCECAddr CECAddr.e_la[2] = %d\n", CECAddr.e_la[2]);
    ALOGI("[HDMI] JNI setCECAddr CECAddr.ui2_pa = %d\n", CECAddr.ui2_pa);
    ALOGI("[HDMI] JNI setCECAddr CECAddr.h_cecm_svc = %d\n", CECAddr.h_cecm_svc);
    ret = hdmi_ioctl(MTK_HDMI_SETLA, (int)&CECAddr);
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setCECAddr\n");
	return ret;
}

static jboolean setCECCmd(JNIEnv* env, jobject clazz, jbyte initAddr, jbyte destAddr, jchar opCode, jbyteArray operand, jint size, jbyte enqueueOk) {
    bool ret = false;
    int i;
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)
    jbyte* operandTemp;
    CEC_SEND_MSG CECCmd;
    operandTemp = env->GetByteArrayElements(operand, NULL);
    CECCmd.t_frame_info.ui1_init_addr = initAddr;
    CECCmd.t_frame_info.ui1_dest_addr = destAddr;
    CECCmd.t_frame_info.ui2_opcode = opCode;
    memcpy(CECCmd.t_frame_info.aui1_operand, (unsigned char*)operandTemp, sizeof(CECCmd.t_frame_info.aui1_operand));
    CECCmd.t_frame_info.z_operand_size = size;
    CECCmd.b_enqueue_ok = enqueueOk;
    ALOGI("[HDMI] JNI setCECAddr CECCmd.t_frame_info.ui1_init_addr = %d\n", CECCmd.t_frame_info.ui1_init_addr);
    ALOGI("[HDMI] JNI setCECAddr CECCmd.t_frame_info.ui1_dest_addr = %d\n", CECCmd.t_frame_info.ui1_dest_addr);
    ALOGI("[HDMI] JNI setCECAddr CECCmd.t_frame_info.ui2_opcode = %d\n", CECCmd.t_frame_info.ui2_opcode);
    for (i = 0; i < 14; i++) {
        ALOGI("[HDMI] JNI setCECAddr CECCmd.t_frame_info.aui1_operand[%d] = %d\n", i, CECCmd.t_frame_info.aui1_operand[i]);
    }
    ALOGI("[HDMI] JNI setCECAddr CECCmd.t_frame_info.z_operand_size = %d\n", CECCmd.t_frame_info.z_operand_size);
    ALOGI("[HDMI] JNI setCECAddr CECCmd.b_enqueue_ok = %d\n", CECCmd.b_enqueue_ok);
    ret = hdmi_ioctl(MTK_HDMI_SET_CECCMD, (int)&CECCmd);
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.setCECCmd\n");
	return ret;
}

static jboolean isHdmiForceAwake(JNIEnv *env, jobject clazz) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    int isforceawake = 0;
    hdmi_ioctl(MTK_HDMI_IS_FORCE_AWAKE, (unsigned int)&isforceawake);
    ret = (isforceawake)? true : false;
#endif
	ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.isHdmiForceAwake( )\n");
    return ret;
}

static jboolean notifyOtgState(JNIEnv* env, jobject clazz, jint aformat) {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_USBOTG_STATUS, aformat);
#endif
    ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.notifyOtgState(%d)\n", aformat);
    return ret;
}

static jint getDisplayType(JNIEnv* env, jobject clazz) {
    bool ret = false;
    jint result = 0;
#if defined (MTK_HDMI_SUPPORT)
    mtk_dispif_info_t hdmi_info;
    ret = hdmi_ioctl(MTK_HDMI_GET_DEV_INFO, (int)&hdmi_info);
    if (!ret) {
        if (hdmi_info.displayType == HDMI_SMARTBOOK) {
            result = 1;
        } else if (hdmi_info.displayType == MHL) {
            result = 2;
        }
    }
#endif
    ALOGI("[HDMI] JNI com.mediatek.hdmi.HDMINative.getDisplayType: %d\n", result);
    return result;
}

// --------------------------------------------------------------------------

static JNINativeMethod gNotify[] = {
    { "enableHdmiNative", 	    "(Z)Z",       (void*)enableHDMI},
    { "enableHdmiIpoNative", 	    "(Z)Z",       (void*)IPOPowerONHDMI},
    { "enableVideoNative", 	    "(Z)Z",       (void*)enableVideo},
    { "enableAudioNative", 	    "(Z)Z",       (void*)enableAudio},
    { "enableCecNative", 	        "(Z)Z",       (void*)enableCEC},
    { "enableHdcpNative", 	    "(Z)Z",       (void*)enableHDCP},
//    { "GetDeviceStatus","(ZZ)Z", 	(void*)GetDeviceStatus},
    { "setVideoConfigNative",     "(I)Z",       (void*)setVideoConfig},
    { "setAudioConfigNative",     "(I)Z",       (void*)setAudioConfig},
    { "setDeepColorNative",       "(II)Z",      (void*)setDeepColor},
    { "setHdcpKeyNative",         "([B)Z",      (void*)setHDCPKey},
    { "setHdmiDrmKeyNative",   "()Z", 	      (void*)setHDMIDRMKey},
    { "setCecAddrNative",         "(B[BCC)Z",   (void*)setCECAddr},
    { "setCecCmdNative",          "(BBC[BIB)Z", (void*)setCECCmd},
    { "getEdidNative",            "()[I",       (void*)getEDID},
    { "getCecAddrNative",         "()[C",       (void*)getCECAddr},
    { "getCecCmdNative",          "()[I",       (void*)getCECCmd},
    { "hdmiPowerEnableNative",    "(Z)Z",       (void*)hdmiPowerEnable},
    { "hdmiPortraitEnableNative", "(Z)Z",       (void*)hdmiPortraitEnable},
    { "isHdmiForceAwakeNative",   "()Z", 	      (void*)isHdmiForceAwake},
    { "getDisplayTypeNative",     "()I",        (void*)getDisplayType},
    { "needSwDrmProtectNative",   "()Z",        (void*)NeedSwDrm},
    { "notifyOtgStateNative",     "(I)Z",       (void*)notifyOtgState},
};

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!");
        return result;
    }
    ALOG_ASSERT(env, "[HDMI] Could not retrieve the env!");

    int ret = jniRegisterNativeMethods(
        env, "com/mediatek/hdmi/HDMINative", gNotify, NELEM(gNotify));

    if (ret) {
        ALOGE("[HDMI] call jniRegisterNativeMethods() failed, ret:%d\n", ret);
    }
    
    return JNI_VERSION_1_4;
}

#ifdef __cplusplus
    }
#endif

