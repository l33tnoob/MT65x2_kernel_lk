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

package com.mediatek.apst.target.data.proxy.sysinfo;

import android.content.Context;
import android.os.Build;
import android.os.Environment;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.StatFs;
//import android.os.storage.IMountService;
import android.os.storage.StorageManager;
import android.telephony.TelephonyManager;

import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.entity.message.Message;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.storage.StorageManagerEx;


/**
 * Class Name: SystemInfoProxy
 * <p>
 * Package: com.mediatek.apst.target.proxy.sysinfo
 * <p>
 * Created on: 2010-8-6
 * <p>
 * <p>
 * Description:
 * <p>
 * Proxy class provides system info related database operations.
 * <p>
 * Support platform: Android 2.2(Froyo)
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public final class SystemInfoProxy extends ContextBasedProxy {
    /** Singleton instance. */
    private static SystemInfoProxy sInstance = null;

    private static StorageManager sStorageManager = null;
    
    private static TelephonyManagerEx sTelephonyManager;

    private SystemInfoProxy(Context context) {
        super(context);
        setProxyName("SystemInfoProxy");
        sStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        
        /* Gemini API refactor */
        sTelephonyManager = TelephonyManagerEx.getDefault();
    }

    /**
     * @param context
     *            The context to handle the systeminfoproxy.
     * @return An instance of the systemIfo proxy.
     */
    public static synchronized SystemInfoProxy getInstance(Context context) {
        if (null == sInstance) {
            sInstance = new SystemInfoProxy(context);
        } else {
            sInstance.setContext(context);
        }
        return sInstance;
    }

    public static String getDevice() {
        return Build.DEVICE;
    }

    public static String getFirmwareVersion() {
        return Build.VERSION.RELEASE;
    }

    public static String getManufacturer() {
        return Build.MANUFACTURER;
    }

    public static String getModel() {
        return Build.MODEL;
    }

    public static String getSdPath() {
        return Environment.getExternalStorageDirectory().getPath();
    }

    public static long getSdTotalSpace() {
        if (isSdMounted()) {
            String sdcard = Environment.getExternalStorageDirectory().getPath();
            StatFs statFs = new StatFs(sdcard);
            long totalSpace = (long) statFs.getBlockSize() * statFs.getBlockCount();

            return totalSpace;
        } else {
            return -1;
        }
    }

    public static long getSdAvailableSpace() {
        if (isSdMounted()) {
            String sdcard = Environment.getExternalStorageDirectory().getPath();
            StatFs statFs = new StatFs(sdcard);
            long availableSpace = (long) statFs.getBlockSize() * statFs.getAvailableBlocks();

            return availableSpace;
        } else {
            return -1;
        }
    }

    public static String getInternalStoragePath() {
        return Environment.getDataDirectory().getPath();
    }

    public static long getInternalTotalSpace() {
        String data = Environment.getDataDirectory().getPath();
        StatFs statFs = new StatFs(data);
        long totalSpace = (long) statFs.getBlockSize() * statFs.getBlockCount();

        return totalSpace;
    }

    public static long getInternalAvailableSpace() {
        String data = Environment.getDataDirectory().getPath();
        StatFs statFs = new StatFs(data);
        long availableSpace = (long) statFs.getBlockSize() * statFs.getAvailableBlocks();

        return availableSpace;
    }

    public static int getSimState(int simId) {
        int simState = TelephonyManager.SIM_STATE_ABSENT;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (Message.SIM1_ID == simId) {
                simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1);
            } else if (Message.SIM2_ID == simId) {
                simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2);
            } else if (Config.MTK_3SIM_SUPPORT && Message.SIM3_ID == simId) {
            	simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_3);
            } else if (Config.MTK_4SIM_SUPPORT && Message.SIM4_ID == simId) {
            	simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_4);
            }
        } else {
            if (Message.SIM_ID == simId) {
                simState = TelephonyManager.getDefault().getSimState();
            }
        }
        Debugger.logD(new Object[] { simId }, "simId=" + simId + ", simState=" + simState);
        return simState;
    }

    public static boolean isSimAccessible(int simState) {
        boolean b = false;

        switch (simState) {
        case TelephonyManager.SIM_STATE_READY:
            b = true;
            break;

        case TelephonyManager.SIM_STATE_ABSENT:
        case TelephonyManager.SIM_STATE_PIN_REQUIRED:
        case TelephonyManager.SIM_STATE_PUK_REQUIRED:
        case TelephonyManager.SIM_STATE_NETWORK_LOCKED:
            b = false;
            break;

        case TelephonyManager.SIM_STATE_UNKNOWN:
            b = false;
            break;

        default:
            b = false;
            break;
        }

        return b;
    }

    /**
     * Get whether SIM card is accessible.
     * 
     * @return True if accessible, otherwise false.
     */
    public boolean isSimAccessible() {
        TelephonyManager telMgr = (TelephonyManager) getContext().getSystemService(Context.TELEPHONY_SERVICE);

        return isSimAccessible(telMgr.getSimState());
    }

    public static boolean isSim1Accessible() {
        boolean b = false;
        if (Config.MTK_GEMINI_SUPPORT) {
            int simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1);

            b = isSimAccessible(simState);
        }
        return b;
    }

    public static boolean isSim2Accessible() {
        boolean b = false;
        if (Config.MTK_GEMINI_SUPPORT) {
            int simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2);

            b = isSimAccessible(simState);
        }
        return b;
    }

    public static boolean isSim3Accessible() {
        boolean b = false;
        if (Config.MTK_3SIM_SUPPORT) {
            int simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_3);

            b = isSimAccessible(simState);
        }
        return b;
    }

    public static boolean isSim4Accessible() {
        boolean b = false;
        if (Config.MTK_4SIM_SUPPORT) {
        	int simState = sTelephonyManager.getSimState(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_4);

            b = isSimAccessible(simState);
        }
        return b;
    }
    
    public static boolean getSimAccessibleBySlot(int slotId) {
    	boolean b = false;
    	switch (slotId) {
    	case SimDetailInfo.SLOT_ID_ONE:
    		b = isSim1Accessible();
    		break;
    		
    	case SimDetailInfo.SLOT_ID_TWO:
    		b = isSim2Accessible();
    		break;
    		
    	case SimDetailInfo.SLOT_ID_THREE:
    		b = isSim3Accessible();
    		break;
    		
    	case SimDetailInfo.SLOT_ID_FOUR:
    		b = isSim4Accessible();
    		break;
    		
    	default:
    		b = false;
    		break;
    	}
    	return b;
    }
    public static boolean isSdPresent() {
        return !(Environment.MEDIA_REMOVED.equals(Environment.getExternalStorageState()) 
                || Environment.MEDIA_BAD_REMOVAL
                .equals(Environment.getExternalStorageState()));
    }

    public static boolean isSdMounted() {
        return (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) 
                || Environment.MEDIA_MOUNTED_READ_ONLY
                .equals(Environment.getExternalStorageState()));
    }

    public static boolean isSdReadable() {
        return isSdMounted();
    }

    public static boolean isSdWriteable() {
        return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
    }

    public boolean[] checkSDCardState() {
        Debugger.logI("FeatureOption.MTK_EMMC_SUPPORT = " + FeatureOption.MTK_EMMC_SUPPORT);
        boolean[] sDstate = new boolean[2];
        String mSDCardPath = null;
        String mSDCard2Path = null;
        String[] storagePathList = sStorageManager.getVolumePaths();
        if (storagePathList != null) {
            if (storagePathList.length >= 2) {
                Debugger.logI("storagePathList.length >= 2");
                mSDCardPath = storagePathList[0];
                mSDCard2Path = storagePathList[1];
                if (null != mSDCardPath) {
                    String state = null;
                    state = sStorageManager.getVolumeState(mSDCardPath);
                    sDstate[0] = Environment.MEDIA_MOUNTED.equals(state);
                }

                if (null != mSDCard2Path) {
                    String state = null;
                    state = sStorageManager.getVolumeState(mSDCard2Path);
                    sDstate[1] = Environment.MEDIA_MOUNTED.equals(state);
                }

                if (!FeatureOption.MTK_EMMC_SUPPORT) {
                    Debugger.logI("FeatureOption.MTK_EMMC_SUPPORT = false");
                    sDstate[1] = sDstate[0];
                    sDstate[0] = false;
                }
            } else if (storagePathList.length == 1) {
                Debugger.logI("storagePathList.length == 1");
                mSDCardPath = storagePathList[0];

                if (null != mSDCardPath) {
                    if (FeatureOption.MTK_EMMC_SUPPORT) {
                        String state = null;
                        state = sStorageManager.getVolumeState(mSDCardPath);
                        sDstate[0] = Environment.MEDIA_MOUNTED.equals(state);
                    } else {
                        String state = null;
                        state = sStorageManager.getVolumeState(mSDCardPath);
                        sDstate[1] = Environment.MEDIA_MOUNTED.equals(state);
                    }
                }
            }
        }
        return sDstate;
    }

    public static String getInternalStoragePathSD() {
        //return sStorageManager.getInternalStoragePath();
        return StorageManagerEx.getInternalStoragePath();
    }

    public static String getExternalStoragePath() {
        //return sStorageManager.getExternalStoragePath();
        return StorageManagerEx.getExternalStoragePath();
    }

    public static boolean isSdSwap() {
        return FeatureOption.MTK_2SDCARD_SWAP && isExSdcardInserted();
    }

    public static boolean isExSdcardInserted() {
        /*IBinder service = ServiceManager.getService("mount");
        Debugger.logI("Util:service is " + service);
        if (service != null) {
            IMountService mountService = IMountService.Stub.asInterface(service);
            Debugger.logI("Util:mountService is " + mountService);
            if (mountService == null) {
                return false;
            }
            try {
                return mountService.isSDExist();
            } catch (RemoteException e) {
                Debugger.logI("Util:RemoteException when isSDExist: " + e);
                return false;
            }
        } else {
            return false;
        }*/
        boolean status = StorageManagerEx.getSdSwapState();
        Debugger.logI("Sdcard inserted status is " + status);
        return status;
    }
}
