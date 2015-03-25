package com.hissage.jni;

import com.hissage.config.NmsProfileSettings;
import com.hissage.contact.NmsContact;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.struct.SNmsImg;
import com.hissage.struct.SNmsSystemConfig;
import com.hissage.struct.SNmsSystemStatus;

public final class engineadapter {

    public enum msgtype {
        NMS_ENG_MSG_INIT_REQ, NMS_ENG_MSG_ACTIVATE_REQ, NMS_ENG_MSG_IMSI_SELECTED, NMS_ENG_MSG_IAP_SELECTED, NMS_ENG_MSG_NUMBER_INPUT, NMS_ENG_MSG_SYNC_MAIL_REQ, // engine
                                                                                                                                                                  // internal
        NMS_ENG_MSG_CONNECT_REQ, NMS_ENG_MSG_DISCONNECT_REQ, NMS_ENG_MSG_DATA_READ_IND, // engine
                                                                                        // internal,
                                                                                        // but
                                                                                        // platform
                                                                                        // shall
                                                                                        // provide
                                                                                        // it
        NMS_ENG_MSG_DATA_WRITE_IND, NMS_ENG_MSG_NW_ERROR_IND, NMS_ENG_MSG_STORAGE_SELECTED, NMS_ENG_MSG_NUM_IND, // engine
                                                                                                                 // internal
        NMS_ENG_MSG_NEW_SERVER, // engine internal
        NMS_ENG_MSG_UPDATE_VCARD, NMS_ENG_MSG_ADD_CONTACT_ACK, NMS_ENG_MSG_ALL_CONTACTS_READY, NMS_ENG_MSG_UPDATE_CACHE, // engine
                                                                                                                         // internal
        NMS_ENG_MSG_STORAGE_CHANGE_ACK, NMS_ENG_MSG_FETCH_REQ, NMS_ENG_MSG_UPDATE_SNS, NMS_ENG_MSG_SYNC_VCARD, NMS_ENG_MSG_CHANGE_LANGUAGE, NMS_ENG_MSG_USB_CONNECTING, NMS_ENG_MSG_FETCH_PREVIEW, NMS_ENG_MSG_SEND_CFG_CMD, NMS_ENG_MSG_OTHER_LOGINED_SELECTED, NMS_ENG_MSG_NUMBER_COUNTY_INPUT, NMS_ENG_MSG_CHANGE_ACTUAL_IMSI_ACK, NMS_ENG_MSG_CHANGE_VIRTUAL_IMSI_ACK, NMS_ENG_MSG_USER_ALLOW_SEND_REGSMS, NMS_ENG_MSG_RESTORE_MSG, NMS_ENG_MSG_SEND_HESINE_STATUS_CMD, NMS_ENG_MSG_SIM_STATUS_CHANGED, NMS_ENG_MSG_ENABLE_SIM, NMS_ENG_MSG_DISABLE_SIM, NMS_ENG_MSG_UDP_REFLEX, // engine
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // internal
        NMS_ENG_MSG_TCP_REFLEX, // engine internal
        NMS_ENG_MSG_RESUME_VCARD, // engine internal
        NMS_ENG_MSG_END,
    };

    private static engineadapter m_instance = null;

    private engineadapter() {
    }

    public static void Initialize() {
        if (m_instance == null) {
            m_instance = new engineadapter();
        }
    }

    public static engineadapter get() {
        Initialize();
        return m_instance;
    }

    public native void nmsUploadCUL();

    public native void nmsSaveAllVCard2JNI();

    public native String nmsGetCachePath();

    public native int nmsSendMsgToEngine(int mid, byte[] pMsg, int msgLen); // do
                                                                            // OCR
                                                                            // over
                                                                            // the
                                                                            // image
                                                                            // in
                                                                            // the
                                                                            // api
                                                                            // buffers

    public native void nmsSendTimerMsgToEngine(int mid);

    public native int nmsInputNumberForActivation(String phoneNubmer, int sim_id);

    public native int nmsSendMsgToEngine(int mid, int msg, int msgLen);

    public native int nmsSetAirplaneMode(int simId, int flag); // 1 AirplaneMode
                                                               // is On, 0
    // is OFF.

    // msg api
    public native String nmsUIGetMsgDownloadAttachName(int recdId);

    // public native SNmsCategory nmsUIGetMsgCategory(int index);

    public native int nmsUISetMsgCategory(byte categoryId, int topIndex, int order);

    public native NmsIpSessionMessage nmsUIGetMsgSummary(int index);

    public native NmsIpMessage nmsUIGetMsgKey(short recordId);

    // public native int nmsUISendMsgInDraft(SNmsMsgKey msgKey, SNmsMsgCont
    // msgCont);

    // public native String nmsUIGetMsgCategoryImg(byte categoryId);

    public native SNmsImMsgCountInfo nmsUISetImMode(int contactId, int updateData, int flag,
            int readMode);

    public native int nmsUISetMsgSearchString(String pSearchString, int order);

    public native int nmsUIDeleteMsg(short[] recordId, int number, int isDelSavedMsg, int isDelMsgInSmsDb);

    public native int nmsUISetMsgFlag(short[] recordId, int num, short flag);

    public native int nmsUICancelMsgFlag(short[] recordId, int num, short flag);

    /*
     * groupId for normal and multi-send msg is 0, for group chat msg is the
     * group id
     */
    public native int nmsUISendNewMsg(NmsIpMessage pCont, int sendMsgMode);

    public native int nmsUISaveMsgIntoDraft(NmsIpMessage pCont);

    public native int nmsUIDeleteMsgViaContactRecId(short contactRecId, int isDelSavedMsg, int isDelHolderMsg, int isDelMsgInSmsDb);

    public native int nmsUIGetIsHesineAccount(String number);

    public native int nmsUIGetContactDraftMsgId(short contactRecId);

    public native int nmsUIDeleteContactDraftMsg(short contactRecId, int isDeleteSysDraftMsg);

    public native String nmsUIGetContactPhoneNumber(short contactRecId);

    public native String nmsUIGetSignature(short contactRecId);

	public native String nmsUIGetImsiViaNumber(String number);

	public native String nmsUIGetUnifyPhoneNumber(String number);

    public native int nmsUIGetContactId(String number);

    public native int nmsUIGetHesineContactId(String[] numbers, int count);

    public native SNmsImg nmsUIGetContactImg(short recordId);

    public native NmsContact nmsUIGetContact(short recordId);

    public native int nmsUIBlockContacts(short[] recordId, int num);

    public native int nmsUIUnBolckContacts(short[] recordId, int num);

    public native int nmsUISaveContactsMsgs(short[] recordId, int num);

    public native int nmsUIUnSaveContactsMsgs(short[] recordId, int num);

    // config
    public native SNmsSystemConfig nmsUIGetSystemConfig();

    public native SNmsSystemStatus nmsUIGetSystemStatus();

    public native int nmsUISetConnMode(byte connMode);

    public native int nmsUISetPollTime(byte heartbeat);

    public native int nmsUIGetLanguageChange();

    // account
    public native int nmsUIUpdateSns();

    public native NmsProfileSettings nmsUIGetUserInfo();

	public native NmsProfileSettings nmsUIGetUserInfoViaImsi(String imsi);

    public native int nmsUISetUserInfo(NmsProfileSettings hesineInfo);

    public native int nmsUIIsHesineActivated();

	public native int nmsUIIsHesineActivatedViaSimId(int simId);

    public native int nmsUIResendMsg(int simId, short msgRecordId, int sendBySms);

    public native int nmsUIUpdateMsgKeyOfContact(String phoneNumber);

    public native String nmsUIGetActionListFileName();

    public native String nmsUIGetLogPath();

    public native int nmsUISendSpecialMsg(String to, String body, String attachFile);

    public native String nmsUIRemoveCountryCode(String phoneNumber);

    public native String nmsUIGetCountryCode();

    public native int nmsUIClearFlow();

    public native int nmsUISetLogPriority(int priority);

    public native short nmsUICreateGroup(int simId, String name, String members);

    public native int nmsUIModifyGroupName(short groupId, String name);

    public native int nmsUIAddMembersToGroup(short groupId, String members);

    public native int nmsUIDelMembersFromGroup(short groupId, String members);

    public native int nmsUIQuitFromGroup(short groupId, int isClearMsg);

    public native short[] nmsUIGetGroupList();

    public native int nmsUIGetGroupLatestMsgId(short groupId);

    public native int nmsUIGetGroupCreaterId(short groupId);

    public native boolean nmsUIIsGroupChatCreater(short groupId) ;

	public native boolean nmsUIIsIpMsgSendable(short cotactId) ;

    // public native int nmsUIGetMsgLeftCount();

    // SMS
    // When the sms/mms db updated, this function will be called 
    public native int nmsProcessInterceptedSms(String from, String to, String content, int smsType,
            int smsId, int source, int haveAttach, int readed, int date, int isLocked,
            String threadNumber, int simId);

    public native int nmsProcessUpdateSms(String from, String to, String content, int smsType,
            int smsId, int source, int haveAttach, int readed, int date, int isLocked,
            String threadNumber, int simId);

    public native int nmsSetServiceOn(int flag);

    // sms/mms was deleted and then update it in ip db
    public native int nmsSmsDeletedFromSystem(int smsType, int smsId);

    public native int nmsUpdateIpMsgStatus(int msgId, int status);

    public native int nmsUpdateIpMsgReaded(int msgId);

    public native int nmsUpdateIpMsgSaved(int msgId, int isSaved);

    public native void nmsUIEnterIMStatus(short contactId);

    public native void nmsUISendIMStatus(short contactId, int status);

    public native void nmsUIExitIMStatus(short contactId);

    public native String nmsUIGetMsgDownloadUrl(int msgId);

    public native int nmsUISetMsgDownloadedAttachPath(int msgId, String path);

    public native int nmsContactKeyRecordId(short messageId);

    public native boolean nmsUIIsActivated();

    public native int nmsUIEnableSimService(int simId);

    public native int nmsUIDisableSimService(int simId);

    public native int nmsUIActivateSimCard(int simId);

	public native String nmsUIGetNeedActiveSimIdList();
    
    public native void nmsUICancelActivation() ;

    public native SNmsSimInfo nmsUIGetImsiInfoViaSimId(int simId);

    public native int nmsUIGetSaveMsgCountInContact(short contactId);

    public native int nmsUISetContactMsgReaded(short contactId);

    public native int nmsUIResendFailedMsgInContact(int simId, short recordId);

    public native short[] nmsUIGetSelfContactIds();
    
    // value of flag please refer to NmsGetContactMsgRecordIdListFlag
    public native short[] nmsUIGetMsgRecrodIdListViaContactId(short contactId, int flag,
            boolean isIncludeGroupCfg);

    public native int[] nmsUIGetIpMsgPlatformIds(short recordId[], int num);
    
    public native int nmsGetOsId() ;

    public native int nmsGetChannelId() ;
    
    public native int nmsGetDevicelId() ;
    
    public native String nmsGetImsiList() ;
    
    public native String nmsGetPhoneNumberList() ;
    
    public native int nmsGetMainImsiIndex() ;
    
    public native void nmsUISetShowReadStatus(int isShow) ;
    
    public native int  nmsUIGetShowReadStatus() ;
    
    public native int  nmsUIGetContactIpMsgCount(short contactId) ;
    
    public native int nmsUIGetContactIPMsgCountOfReadMode(short contactId, int readModes);
    
    public native String nmsUIGetNqNocPrefix() ;
    
    public native short[] nmsUIGetServiceContactList(); //add for ODM virtual number

    public native String nmsGetDNSHost() ;
    
    public native void nmsProcessDnsResult(int ip) ;
    
    public native void nmsUIHandlePNNewRegId() ;
    
    public native void nmsUIHandlePNNotification() ;
    
    public native int nmsUIIsSelfRegSms(String body) ;
    
	//M: Activation Statistics
    public native void nmsUISetUserActivteType(int type) ;

    public native void nmsSetTraffic(int type , int connsimid);
    
    /* stub functions for unit test */
    public native void nmsTestSetNetworkStatus(int isNetworkOK);

    public native void nmsTestSetPhoneStorageStatus(int isPhoneStorageOK);

    public native void nmsTestSetSDCardStatus(int isSdcardOK);

    public native void nmsTestSetMemoryStatus(int isMemoryOK);

    public native void nmsTestSetActivatedStatus(int status);

    public native String nmsTestGetSelfNumber();

    public native void nmsTestClearLog();

    public native int nmsTestGetCurLogPosition();

    public native int nmsTestSearchInLog(int pos, String str);
    
    public native byte[] nmsTestGetSimInfo(int getHistory) ;

    public native void nmsSetDiskStatus(int busy);
    
    public native void nmsCheckDefaultSmsAppChanged(); 
	
    static {
        // System.loadLibrary("hpe");
        System.loadLibrary("hissage");
    }

}
