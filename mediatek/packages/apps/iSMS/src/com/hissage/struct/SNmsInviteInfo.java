package com.hissage.struct;

import android.content.Context;
import android.util.Log;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsConfig;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.db.NmsDBUtils;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;

public class SNmsInviteInfo {
    public static final int mReminderInvalid = 0;
    public static final int mReminderInvite = 1;
    public static final int mReminderActivate = 2;
    public static final int mReminderSwitch = 3;
    public static final int mReminderEnable = 4;

    private static final String TAG = "SNmsInviteInfo";
    public short recdId = -1;
    public int last_contact = 0;
    public int contact_count = 0;
    public int count_today = 0;
    public int later_time = 0;

    private final int TEN_DAYS = 30;
    private final int THREE_DAYS = 3;
    private final int COUNT_TIME = 3;

    private final int FIVE_DAYS = 5;
    private final int COUNT_ALL = 10;

    public static int getCurrentDay() {
        long param = 1000 * 60 * 60 * 24;
        return (int) (System.currentTimeMillis() / param);
    }

    public void updateInvite(Context context, short contactId) {
        int currentDay = getCurrentDay();
        SNmsInviteInfo info = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(contactId);
        if (null == info) {
            info = new SNmsInviteInfo();
            info.recdId = contactId;
        }
        if (info.later_time <= currentDay) {
            if (info.last_contact == currentDay) {
                info.count_today++;
            } else {
                info.count_today = 1;
            }

            if (info.last_contact + 1 == currentDay) {
                info.contact_count++;
            } else if(info.last_contact  == currentDay){
            }else{
                info.contact_count = 1;
            }
            info.last_contact = currentDay;
            NmsDBUtils.getDataBaseInstance(context).nmsAddInviteRecd(info);
        }
        
        SNmsInviteInfo allMessageInfo = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(
                (short) 0);
        if (null == allMessageInfo) {
            allMessageInfo = new SNmsInviteInfo();
            allMessageInfo.recdId = 0;
        }
        if (allMessageInfo.later_time <= currentDay) {
            if (allMessageInfo.last_contact == currentDay) {
                allMessageInfo.count_today++;
            } else {
                allMessageInfo.count_today = 1;
            }
            if (allMessageInfo.last_contact + 1 == currentDay) {
                allMessageInfo.contact_count++;
            } else if (allMessageInfo.last_contact == currentDay) {
            } else {
                allMessageInfo.contact_count = 1;
            }
            allMessageInfo.last_contact = currentDay;
            NmsDBUtils.getDataBaseInstance(context).nmsAddInviteRecd(allMessageInfo);
        }
    }

    public boolean needShowInviteDlg(Context context, short contactId) {
        boolean isComposeMessage = true ;
        if (null == context || contactId < 0) {
            NmsLog.error(TAG, "needShowInviteDlg param error: " + context + ", contactId: "
                    + contactId);
            return false;
        }else if(null == context || contactId == 0){
            isComposeMessage =false;
        }
        long simId1 = NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_1);
        long simId2 = NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_2);

        if ((simId1 <= 0 || NmsIpMessageApiNative.nmsGetActivationStatus((int) simId1) < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                && (simId2 <= 0 || NmsIpMessageApiNative.nmsGetActivationStatus((int) simId2) < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)) {
            NmsLog.trace(TAG, "no sim card activited, can not show invite dlg.");
            return false;
        }
        NmsContact contact  = null;
           if(isComposeMessage){
               contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);

               if (null == contact) {
                   NmsLog.trace(TAG, "can not get NmsContact via contactId: " + contactId);
                   return false;
               }

               if (contact.getType() != NmsContactType.NOT_HISSAGE_USER) {
                   NmsLog.trace(TAG, "this user type is: " + contact.getType());
                   return false;
               }

               SNmsInviteInfo info = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(
                       contact.getId());
               if (null == info) {
                   NmsLog.trace(TAG, "get invite info error, may be first contact to this people.");
                   return false;
               }

               if (info.later_time > getCurrentDay()) {
                   NmsLog.trace(TAG, "user set later last time: " + NmsConsts.SDF3.format(info.later_time));
                   return false;
               }

               if (info.contact_count >= THREE_DAYS || info.count_today >= COUNT_TIME) {
                   NmsLog.trace(TAG, "needShowInviteDlg return true");
                   return true;
               }
            }else{

               contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId((short)0);

               if (null == contact) {
                   NmsLog.trace(TAG, "can not get NmsContact via contactId: " + contactId);
                   return false;
               }

//               if (contact.getType() != NmsContactType.NOT_HISSAGE_USER) {
//                   NmsLog.trace(TAG, "this user type is: " + contact.getType());
//                   return false;
//               }

               SNmsInviteInfo info = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(
                       contact.getId());
               if (null == info) {
                   NmsLog.trace(TAG, "get invite info error, may be first contact to this people.");
                   return false;
               }

               if (info.later_time > getCurrentDay()) {
                   NmsLog.trace(TAG, "user set later last time: " + NmsConsts.SDF3.format(info.later_time));
                   return false;
               }
            
               if (info.contact_count >= FIVE_DAYS || info.count_today >= COUNT_ALL) {
                   NmsLog.trace(TAG, "needShowInviteDlg return true");
                   return true;
               }
            }
        return false;
    }

    public int needShowReminderDlg(Context context, short contactId) {
        long simId1 = 0, simId2 = 0, currentSimId = 0;
        long simStatus1 = 0, simStatus2 = 0, currentStatus = 0;
        boolean isComposeMessage = true ;

        if(NmsConfig.getShowRemindersFlag() == false){
 
            return mReminderInvalid;
        }
        if (null == context || contactId < 0) {
            NmsLog.error(TAG, "needShowReminderDlg param error: " + context + ", contactId: "
                    + contactId);
            return mReminderInvalid;
        }else if(null == context ||  contactId == 0){
            isComposeMessage =false;
        }

        simId1 = NmsPlatformAdapter.getInstance(context)
                .getSimIdBySlotId(NmsConsts.SIM_CARD_SLOT_1);
        simId2 = NmsPlatformAdapter.getInstance(context)
                .getSimIdBySlotId(NmsConsts.SIM_CARD_SLOT_2);

        if (simId1 <= 0 && simId2 <= 0) {
            return mReminderInvalid;
        }
        SNmsInviteInfo info = null;
         if(isComposeMessage){
             NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
             if (null == contact) {
                 NmsLog.error(TAG, "can not get NmsContact via contactId: " + contactId);
                 return mReminderInvalid;
             }

              info = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(
                     contact.getId());
              if (null == info) {
                  NmsLog.trace(TAG, "get invite info error, may be first contact to this people.");
                  return mReminderInvalid;
              }

              NmsLog.trace(TAG, "needShowReminderDlg: " + "contact_count: " + info.contact_count
                      + " count_today: " + info.count_today);

              if (info.later_time > getCurrentDay()) {
                  NmsLog.trace(TAG, "user set later last time: " + NmsConsts.SDF3.format(info.later_time));
                  return mReminderInvalid;
              }
              if (info.contact_count >= THREE_DAYS || info.count_today >= COUNT_TIME) {
                  simStatus1 = NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST;
                  simStatus2 = NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST;
                  if (simId1 > 0) {
                      simStatus1 = NmsIpMessageApiNative.nmsGetActivationStatus((int) simId1);
                  }

                  if (simId2 > 0) {
                      simStatus2 = NmsIpMessageApiNative.nmsGetActivationStatus((int) simId2);
                  }

                  if (simStatus1 < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED
                          && simStatus2 < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Activate reminder");
                      return SNmsInviteInfo.mReminderActivate;
                  }

                  currentSimId = NmsPlatformAdapter.getInstance(context).getCurrentSimId();
                  currentStatus = NmsIpMessageApiNative.nmsGetActivationStatus((int) currentSimId);

                  if (currentStatus == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED
                          && contact.getType() == NmsContactType.NOT_HISSAGE_USER) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Invite reminder");
                      return SNmsInviteInfo.mReminderInvite;
                  }

                  if (currentStatus == NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED
                          && contact.getType() == NmsContactType.HISSAGE_USER) {

                      NmsLog.trace(TAG, "needShowReminderDlg return Enable reminder");
                      return SNmsInviteInfo.mReminderEnable;
                  }

                  if (currentStatus < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED
                          && contact.getType() == NmsContactType.HISSAGE_USER
                          && (simStatus1 >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED || simStatus2 >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Switch reminder");
                      return SNmsInviteInfo.mReminderSwitch;
                  }
              }

          }else {
              info = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(
                      (short)0);
              if (null == info) {
                  NmsLog.trace(TAG, "get invite info error, may be first contact to this people.");
                  return mReminderInvalid;
              }
              if (info.later_time > getCurrentDay()) {
                  NmsLog.trace(TAG, "user set later last time: " + NmsConsts.SDF3.format(info.later_time));
                  return mReminderInvalid;
              }
              NmsLog.trace(TAG, "needShowReminderDlg: " + "contact_count: " + info.contact_count
                      + " count_today: " + info.count_today);
              if (info.contact_count >= FIVE_DAYS || info.count_today >= COUNT_ALL) {
                  simStatus1 = NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST;
                  simStatus2 = NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST;
                  if (simId1 > 0) {
                      simStatus1 = NmsIpMessageApiNative.nmsGetActivationStatus((int) simId1);
                  }

                  if (simId2 > 0) {
                      simStatus2 = NmsIpMessageApiNative.nmsGetActivationStatus((int) simId2);
                  }

                  if (simStatus1 < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED
                          && simStatus2 < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Activate reminder");
                      return SNmsInviteInfo.mReminderActivate;
                  }

                  currentSimId = NmsPlatformAdapter.getInstance(context).getCurrentSimId();
                  currentStatus = NmsIpMessageApiNative.nmsGetActivationStatus((int) currentSimId);

                  if (currentStatus == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Invite reminder");
                      return SNmsInviteInfo.mReminderInvite;
                  }

                  if (currentStatus == NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Enable reminder");
                      return SNmsInviteInfo.mReminderEnable;
                  }

                  if (currentStatus < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED
                          && (simStatus1 >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED || simStatus2 >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)) {
                      NmsLog.trace(TAG, "needShowReminderDlg return Switch reminder");
                      
                      return SNmsInviteInfo.mReminderSwitch;
                  }
              }
          }
        return mReminderInvalid;
    }

    public boolean setLater(Context context, short contactId) {
        if (null == context || contactId < 0) {
            NmsLog.error(TAG, "setLater param error: " + context + ", contactId: " + contactId);
            return false;
        }
        SNmsInviteInfo info =null;
        if(contactId == 0){
            info = new SNmsInviteInfo();
            info.recdId = 0;
        }else{
            NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
            if (null == contact) {
                NmsLog.trace(TAG, "can not get NmsContact via contactId: " + contactId);
                return false;
            }
          info = NmsDBUtils.getDataBaseInstance(context).nmsGetInviteInfo(
                    contact.getId());
            if (null == info) {
                NmsLog.error(TAG, "can not get this contact invite info, so can not set later");
                return false;
            }  
        }
        info.later_time = getCurrentDay() + TEN_DAYS;
        info.contact_count = 0;
        info.count_today = 0;
        info.last_contact = 0;

        return NmsDBUtils.getDataBaseInstance(context).nmsAddInviteRecd(info) > 0;
    }

    public boolean setInvite(Context context, short contactId) {
        if (null == context || contactId <= 0) {
            NmsLog.error(TAG, "setInvite param error: " + context + ", contactId: " + contactId);
            return false;
        }
        return setLater(context, contactId);
    }

}
