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

package com.mediatek.apst.target.data.proxy.contacts;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IIccPhoneBook;
import com.android.internal.telephony.ITelephony;
import com.mediatek.common.telephony.UsimGroup;
import com.android.internal.telephony.gsm.UsimPhoneBookManager;

import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;

import java.util.List;

public class USIMUtils {

    private static final String TAG = "APST";

    public static final String SIMPHONEBOOK_SERVICE = "simphonebook";
    public static final String SIMPHONEBOOK2_SERVICE = "simphonebook2";
    public static final String SIMPHONEBOOK3_SERVICE = "simphonebook3";
    public static final String SIMPHONEBOOK4_SERVICE = "simphonebook4";
    public static final int SINGLE_SLOT = 0;
    public static final int GEMINI_SLOT1 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1;
    public static final int GEMINI_SLOT2 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2;
    public static final int GEMINI_SLOT3 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_3;
    public static final int GEMINI_SLOT4 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_4;
    public static final int SLOT_COUNT = Config.MTK_GEMINI_SUPPORT ? (Config.MTK_3SIM_SUPPORT ? 
    		(Config.MTK_4SIM_SUPPORT ? 4 : 3 ) : 2) : 1;

    private static final int[] MAX_USIM_GROUP_NAME_LENGTH = { -1, -1, -1, -1 };
    private static final int[] MAX_USIM_GROUP_COUNT = { -1, -1, -1, -1 };

    // The following lines are provided and maintained by Mediatek inc.
    // Added Local Account Type
    public static final String ACCOUNT_TYPE_SIM = "SIM Account";
    public static final String ACCOUNT_TYPE_USIM = "USIM Account";
    public static final String ACCOUNT_TYPE_LOCAL_PHONE = "Local Phone Account";

    // Added Local Account Name - For Sim/Usim Only
    public static final String ACCOUNT_NAME_SIM = "SIM" + SimSlot.SLOT_ID1;
    public static final String ACCOUNT_NAME_SIM2 = "SIM" + SimSlot.SLOT_ID2;
    public static final String ACCOUNT_NAME_SIM3 = "SIM" + SimSlot.SLOT_ID3;
    public static final String ACCOUNT_NAME_SIM4 = "SIM" + SimSlot.SLOT_ID4;
    
    public static final String ACCOUNT_NAME_USIM = "USIM" + SimSlot.SLOT_ID1;
    public static final String ACCOUNT_NAME_USIM2 = "USIM" + SimSlot.SLOT_ID2;
    public static final String ACCOUNT_NAME_USIM3 = "USIM" + SimSlot.SLOT_ID3;
    public static final String ACCOUNT_NAME_USIM4 = "USIM" + SimSlot.SLOT_ID4;
    public static final String ACCOUNT_NAME_LOCAL_PHONE = "Phone";

    /**
     * @author The interface for sim type.
     */
    public interface SimType {
        String SIM_TYPE_USIM_TAG = "USIM";

        int SIM_TYPE_SIM = 0;
        int SIM_TYPE_USIM = 1;
    }

    /**
     * The interface for sim slot.
     * 
     */
    public interface SimSlot {
        int SLOT_NONE = -1;
        int SLOT_SINGLE = 0;
        int SLOT_ID1 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1;
        int SLOT_ID2 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2;
        int SLOT_ID3 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_3;
        int SLOT_ID4 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_4;
    }

    /**
     * @param slotId
     *            The slot id of the sim.
     * @return The sim type.
     */
    public static int getSimTypeBySlot(int slotId) {
//        final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
//                .getService(Context.TELEPHONY_SERVICE));
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        int simType = SimType.SIM_TYPE_SIM;
        if (iTel == null) {
            Debugger.logE(new Object[] { slotId }, "iTel is null, may be phone service hasn't int done!");
            return simType;
        }
        try {
            if (Config.MTK_GEMINI_SUPPORT) {
                if (SimType.SIM_TYPE_USIM_TAG.equals(iTel
                        .getIccCardType(slotId))) {
                    simType = SimType.SIM_TYPE_USIM;
                }
            } else {
                if (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType(0))) {
                    simType = SimType.SIM_TYPE_USIM;
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        Debugger.logI(new Object[] { slotId }, "simType : " + simType);
        return simType;
    }

    /**
     * @param slotId
     *            The slot id of the sim.
     * @return Whether is sim inserted in slot.
     */
    public static boolean isSimInserted(int slotId) {
//        final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
//                .getService(Context.TELEPHONY_SERVICE));
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        if (iTel == null) {
            Debugger.logE(new Object[] { slotId }, "iTel is null, may be phone service hasn't int done!");
            return false;
        }
        boolean isSimInsert = false;
        try {
            if (iTel != null) {
                if (Config.MTK_GEMINI_SUPPORT) {
                    isSimInsert = iTel.hasIccCard(slotId);
                } else {
                    isSimInsert = iTel.hasIccCard(0);
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            isSimInsert = false;
        }
        return isSimInsert;
    }

    // The following lines are provided and maintained by Mediatek inc.
    /**
     * @param slotId
     *            The slot id.
     * @return The account type.
     */
    public static String getAccountTypeBySlot(int slotId) {
        Debugger.logI("getAccountTypeBySlot()+ - slotId:" + slotId);
        if (slotId < SimSlot.SLOT_ID1 || slotId > SimSlot.SLOT_ID4) {
            Debugger.logE("Error! - slot id error. slotid:" + slotId);
            return null;
        }
        int simtype = SimType.SIM_TYPE_SIM;
        String simAccountType = ACCOUNT_TYPE_SIM;

        if (isSimInserted(slotId)) {
            simtype = getSimTypeBySlot(slotId);
            if (SimType.SIM_TYPE_USIM == simtype) {
                simAccountType = ACCOUNT_TYPE_USIM;
            }
        } else {
            Debugger.logE("Error! getAccountTypeBySlot - slotId:" + slotId
                    + " no sim inserted!");
            simAccountType = null;
        }
        Debugger.logI("getAccountTypeBySlot()- - slotId:" + slotId
                + " AccountType:" + simAccountType);
        return simAccountType;
    }

    /**
     * @param slotId
     *            The the slot id.
     * @return The sim account name.
     */
    public static String getSimAccountNameBySlot(int slotId) {
        String retSimName = null;
        int simType = SimType.SIM_TYPE_SIM;

        Debugger.logI("getSimAccountNameBySlot()+ slotId:" + slotId);
        if (!isSimInserted(slotId)) {
            Debugger.logE("getSimAccountNameBySlot Error! - SIM not inserted!");
            return retSimName;
        }

        simType = getSimTypeBySlot(slotId);
        Debugger.logI("getSimAccountNameBySlot() slotId:" + slotId
                + " simType(0-SIM/1-USIM):" + simType);

        if (SimType.SIM_TYPE_SIM == simType) {
            retSimName = ACCOUNT_NAME_SIM;
            if (SimSlot.SLOT_ID2 == slotId) {
                retSimName = ACCOUNT_NAME_SIM2;
            } else if (SimSlot.SLOT_ID3 == slotId) {
                retSimName = ACCOUNT_NAME_SIM3;
            } else if (SimSlot.SLOT_ID4 == slotId) {
                retSimName = ACCOUNT_NAME_SIM4;
            }
        } else if (SimType.SIM_TYPE_USIM == simType) {
            retSimName = ACCOUNT_NAME_USIM;
            if (SimSlot.SLOT_ID2 == slotId) {
                retSimName = ACCOUNT_NAME_USIM2;
            } else if (SimSlot.SLOT_ID3 == slotId) {
                retSimName = ACCOUNT_NAME_USIM3;
            } else if (SimSlot.SLOT_ID4 == slotId) {
                retSimName = ACCOUNT_NAME_USIM4;
            }
        } else {
            Debugger
                    .logE("getSimAccountNameBySlot() Error!  get SIM Type error! simType:"
                            + simType);
        }

        Debugger.logI("getSimAccountNameBySlot()- slotId:" + slotId
                + " SimName:" + retSimName);
        return retSimName;
    }

    /**
     * There are some differences with iccprovider
     * 
     * @param sourceLoacation
     *            The sourceLoacation.
     * @return The Usim type according to the slot id.
     */
    public static boolean isSimUsimType(int sourceLoacation) {
//        final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
//                .getService(Context.TELEPHONY_SERVICE));
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        if (iTel == null) {
            Debugger.logE(new Object[] { sourceLoacation }, "iTel is null, may be phone service hasn't int done!");
            return false;
        }
        boolean isUsim = false;
        try {
            if (sourceLoacation == 0) {
                if (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType(0))) {
                    isUsim = true;
                }
            } else if (sourceLoacation > 0) {
                // this sourceLoacation in deamon equals real slotId + 1
                if (SimType.SIM_TYPE_USIM_TAG.equals(iTel
                        .getIccCardType(sourceLoacation - 1))) {
                    isUsim = true;
                }
            } else {
                Debugger.logE("sourceLoacation < 0");
            }
        } catch (RemoteException e) {
            Debugger.logE("catched exception.");
            e.printStackTrace();
        }
        Debugger.logI(new Object[] { sourceLoacation }, "isUsim : " + isUsim);
        return isUsim;
    }

    // ------------------XXXX
    // ----------------XXXX--------------------XXXX---------------

    /**
     * @param slotId
     *            The slot id.
     * @return The IIccPhoneBook of the slot id.
     */
    public static IIccPhoneBook getIIccPhoneBook(int slotId) {
        Log.d(TAG, "[getIIccPhoneBook]slotId:" + slotId);
        String serviceName;
        if (Config.MTK_GEMINI_SUPPORT) {
            serviceName = SIMPHONEBOOK_SERVICE;
            
            if (slotId == GEMINI_SLOT2) {
            	serviceName = SIMPHONEBOOK2_SERVICE;
            } else if (slotId == GEMINI_SLOT3) {
            	serviceName = SIMPHONEBOOK3_SERVICE;
            } else if (slotId == GEMINI_SLOT4) {
            	serviceName = SIMPHONEBOOK4_SERVICE;
            }
        } else {
            serviceName = SIMPHONEBOOK_SERVICE;
        }
        final IIccPhoneBook iIccPhb = IIccPhoneBook.Stub
                .asInterface(ServiceManager.getService(serviceName));
        Log.d(TAG, "[getIIccPhoneBook]iIccPhb:" + iIccPhb);
        return iIccPhb;
    }

    /**
     * @param slot
     *            The slot of the sim.
     * @return The max length of the group name in USIM.
     */
    public static int getUSIMGrpMaxNameLen(int slot) {
        if (slot < SimSlot.SLOT_ID1 || slot > SimSlot.SLOT_ID4) {
            Log.d(TAG, "slot:" + slot);
            return -1;
        }
        Log.d(TAG, "[getUSIMGrpMaxNameLen]slot:" + slot + "|maxNameLen:"
                + MAX_USIM_GROUP_NAME_LENGTH[slot]);
        if (MAX_USIM_GROUP_NAME_LENGTH[slot] < 0) {
            try {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slot);
                if (iIccPhb != null) {
                    MAX_USIM_GROUP_NAME_LENGTH[slot] = iIccPhb
                            .getUsimGrpMaxNameLen();
                }
            } catch (android.os.RemoteException e) {
                Log.d(TAG, "catched exception.");
                MAX_USIM_GROUP_NAME_LENGTH[slot] = -1;
            }
        }
        Log.d(TAG, "[getUSIMGrpMaxNameLen]end slot:" + slot + "|maxNameLen:"
                + MAX_USIM_GROUP_NAME_LENGTH[slot]);
        return MAX_USIM_GROUP_NAME_LENGTH[slot];
    }

    // Framework interface, here should be change in future.
    /**
     * @param slotId
     *            The slot id.
     * @param grpName
     *            The group name.
     * @return Whether the group exist.
     * @throws RemoteException
     */
    public static int hasExistGroup(int slotId, String grpName)
            throws RemoteException {
        int grpId = -1;
        final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
        Log.d(TAG, "grpName:" + grpName + "|iIccPhb:" + iIccPhb);
        if (TextUtils.isEmpty(grpName) || iIccPhb == null) {
            return grpId;
        }
        List<UsimGroup> uList = iIccPhb.getUsimGroups();
        if(null != uList)
        {
        	for (UsimGroup ug : uList) {
                String gName = ug.getAlphaTag();
                int gIndex = ug.getRecordIndex();
                if (!TextUtils.isEmpty(gName) && gIndex > 0) {
                    Log.d(TAG, "[hasExistGroup]gName:" + gName + "||gIndex:"
                            + gIndex);
                    if (gName.equals(grpName)) {
                        grpId = gIndex;
                    }
                }
            }
        }
        
        return grpId;
    }

    /**
     * If a USIM group is created, it should indicate which USIM it creates on.
     * 
     * @param slotId
     *            The slot id.
     * @param name
     *            The group name for creating.
     * @return The group id of the created group.
     * @throws RemoteException
     * @throws USIMGroupException
     */
    public static int createUSIMGroup(int slotId, String name)
            throws RemoteException, USIMGroupException {
        int nameLen = 0;
        try {
            nameLen = name.getBytes("GBK").length;
        } catch (java.io.UnsupportedEncodingException e) {
            nameLen = name.length();
        }
        if (nameLen > getUSIMGrpMaxNameLen(slotId)) {
            throw new USIMGroupException(
                    USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                    USIMGroupException.GROUP_NAME_OUT_OF_BOUND, slotId);
        }
        final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
        int grpId = -1;
        if (iIccPhb != null) {
            grpId = iIccPhb.insertUsimGroup(name);
        }
        Log.i(TAG, "[createUSIMGroup]inserted grpId:" + grpId);
        if (grpId > 0) {
            UsimGroup usimGroup = new UsimGroup(grpId, name);
        } else {
            switch (grpId) {
            case USIMGroupException.USIM_ERROR_GROUP_COUNT: 
                throw new USIMGroupException(
                        USIMGroupException.ERROR_STR_GRP_COUNT_OUTOFBOUND,
                        USIMGroupException.GROUP_NUMBER_OUT_OF_BOUND, slotId);
                // Name len has been check before new group.
                // However, do protect here just for full logic.
            case USIMGroupException.USIM_ERROR_NAME_LEN: 
                throw new USIMGroupException(
                        USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                        USIMGroupException.GROUP_NAME_OUT_OF_BOUND, slotId);
            default:
                break;
            }
        }
        return grpId;
    }

    /**
     * @param slotId
     *            The slot id of the sim.
     * @param nGasId
     * @param name
     *            The group name .
     * @return
     * @throws RemoteException
     * @throws USIMGroupException
     */
    public static int updateUSIMGroup(int slotId, int nGasId, String name)
            throws RemoteException, USIMGroupException {
        int nameLen = 0;
        try {
            nameLen = name.getBytes("GBK").length;
        } catch (java.io.UnsupportedEncodingException e) {
            nameLen = name.length();
        }
        if (nameLen > getUSIMGrpMaxNameLen(slotId)) {
            throw new USIMGroupException(
                    USIMGroupException.ERROR_STR_GRP_NAME_OUTOFBOUND,
                    USIMGroupException.GROUP_NAME_OUT_OF_BOUND, slotId);
        }
        final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
        int grpId = -1;
        if (iIccPhb != null) {
            grpId = iIccPhb.updateUsimGroup(nGasId, name);
        }

        return grpId;
    }

    /**
     * @param grpName
     *            The group name in sim.
     * @return The result of the delete group in sim.
     */
    public static int[] syncUSIMGroupDeleteDualSim(String grpName) {

        int[] errFlag = new int[SLOT_COUNT];
        int flag = -2;
        if (Config.MTK_GEMINI_SUPPORT) {
            flag = deleteUSIMGroup(GEMINI_SLOT1, grpName);
            if (flag > 0) {
                errFlag[GEMINI_SLOT1] = flag;
            }
            flag = deleteUSIMGroup(GEMINI_SLOT2, grpName);
            if (flag > 0) {
                errFlag[GEMINI_SLOT2] = flag;
            }
            if (Config.MTK_3SIM_SUPPORT) {
            	flag = deleteUSIMGroup(GEMINI_SLOT3, grpName);
                if (flag > 0) {
                    errFlag[GEMINI_SLOT3] = flag;
                }
            }
            if (Config.MTK_4SIM_SUPPORT) {
            	flag = deleteUSIMGroup(GEMINI_SLOT4, grpName);
                if (flag > 0) {
                    errFlag[GEMINI_SLOT4] = flag;
                }
            }
        } else {
            flag = deleteUSIMGroup(SINGLE_SLOT, grpName);
            if (flag > 0) {
                errFlag[SINGLE_SLOT] = flag;
            }
        }
        return errFlag;
    }

    /**
     * @param slotId
     *            The slot id of the slot to delete group.
     * @param name
     *            The group name to delete.
     * @return The result of the delete.
     */
    public static int deleteUSIMGroup(int slotId, String name) {

        final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
        int errCode = -2;
        try {
            int grpId = hasExistGroup(slotId, name);
            if (grpId > 0) {
                if (iIccPhb.removeUsimGroupById(grpId)) {
                    // ugrpListArray.removeItem(slotId, grpId);
                    errCode = 0;
                } else {
                    errCode = -1;
                }
            }
        } catch (android.os.RemoteException e) {
            Log.d(TAG, "catched exception");
        }
        return errCode;

    }

    /**
     * @param slotId
     *            The slot id of the slot to add group member.
     * @param simIndex
     *            The sim index.
     * @param grpId
     *            The group id to add.
     * @return Whether succeed to add group member in USIM.
     */
    public static boolean addUSIMGroupMember(int slotId, int simIndex, int grpId) {
        boolean succFlag = false;
        try {
            if (grpId > 0) {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
                if (iIccPhb != null) {
                    succFlag = iIccPhb.addContactToGroup(simIndex, grpId);
                    succFlag = true; // Only for test, should be removed after
                    // framework is ready.
                }
            }
        } catch (android.os.RemoteException e) {
            Log.d(TAG, "catched exception");
            succFlag = false;
        }
        Log.d(TAG, "[addUSIMGroupMember]succFlag" + succFlag);
        return succFlag;
    }

    /**
     * @param slotId
     *            The slot id of the slot to delete the group member.
     * @param simIndex
     *            The sim index.
     * @param grpId
     *            The group id .
     * @return Whether succeed to delete group member in USIM.
     */
    public static boolean deleteUSIMGroupMember(int slotId, int simIndex,
            int grpId) {
        Log.i(TAG, slotId + "-----deleteUSIMGroupMember[slotId]");
        Log.i(TAG, simIndex + "-----deleteUSIMGroupMember[simIndex]");
        Log.i(TAG, grpId + "-----deleteUSIMGroupMember[grpId]");
        boolean succFlag = false;
        try {
            if (grpId > 0) {
                final IIccPhoneBook iIccPhb = getIIccPhoneBook(slotId);
                if (iIccPhb != null) {
                    succFlag = iIccPhb.removeContactFromGroup(simIndex, grpId);
                    succFlag = true;
                }
            }
        } catch (android.os.RemoteException e) {
            Log.d(TAG, "catched exception.");
            succFlag = false;
        }
        return succFlag;
    }

    /**
     * @param accountName 
     *            The group accountName
     * @return The slot id.
     */
    public static int getSlotIdByAccountName(String accountName) {
        int slotId = 0;
        if (ACCOUNT_NAME_USIM.endsWith(accountName)) {
        	slotId = 0;
        } else if (ACCOUNT_NAME_USIM2.endsWith(accountName)) {
        	slotId = 1;
        } else if (ACCOUNT_NAME_USIM3.endsWith(accountName)) {
        	slotId = 2;
        } else if (ACCOUNT_NAME_USIM4.endsWith(accountName)) {
        	slotId = 3;
        }
        return slotId;
    }
    public static class USIMGroupException extends Exception {

        private static final long serialVersionUID = 1L;

        public static final String ERROR_STR_GRP_NAME_OUTOFBOUND = "Group name out of bound";
        public static final String ERROR_STR_GRP_COUNT_OUTOFBOUND = "Group count out of bound";
        public static final int GROUP_NAME_OUT_OF_BOUND = 1;
        public static final int GROUP_NUMBER_OUT_OF_BOUND = 2;
        // Exception type definination in framework.
        public static final int USIM_ERROR_NAME_LEN = UsimPhoneBookManager.USIM_ERROR_NAME_LEN;
        public static final int USIM_ERROR_GROUP_COUNT = UsimPhoneBookManager.USIM_ERROR_GROUP_COUNT;

        int mErrorType;
        int mSlotId;

        /**
         * Empty constructor.
         */
        USIMGroupException() {
            super();
        }

        /**
         * @param msg
         *            The messge about the exception.
         */
        USIMGroupException(String msg) {
            super(msg);
        }

        /**
         * @param msg
         *            The message about the exception.
         * @param errorType
         *            The error type.
         * @param slotId
         *            The slot id.
         */
        USIMGroupException(String msg, int errorType, int slotId) {
            super(msg);
            mErrorType = errorType;
            mSlotId = slotId;
        }

        /**
         * @return The error type.
         */
        public int getErrorType() {
            return mErrorType;
        }

        /**
         * @return the error slot id.
         */
        public int getErrorSlotId() {
            return mSlotId;
        }

        @Override
        public String getMessage() {
            return "Details message: errorType:" + mErrorType + "\n"
                    + super.getMessage();
        }
    }

}
