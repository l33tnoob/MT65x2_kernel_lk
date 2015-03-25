package com.hissage.api;

import java.io.InputStream;
import java.util.ArrayList;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.PhoneLookup;
import android.text.TextUtils;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.config.NmsProfileSettings;
import com.hissage.contact.NmsBroadCastContact;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.contact.NmsUIContact;
import com.hissage.db.NmsContentResolver;
import com.hissage.jni.engineadapter;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsImg;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsContactApi {

    private static String TAG = "NmsContactApi";

    class SystemContactSummary {
        public long contactId;
        public String name;
        public String number;
    }

    private static NmsContactApi mInstance = null;
    private static Context mContext;
    private Bitmap mDefaultBitmap;
    private Bitmap mBlankBitmap;
    private Bitmap mBroadCastBitmap;

    private NmsContactApi() {
        mDefaultBitmap = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.contact_default_avatar);
        mBlankBitmap = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.contact_blank_avatar);
        mBroadCastBitmap = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.contact_broadcast_avatar);
    }

    public synchronized static NmsContactApi getInstance(Context context) {
        if (context == null) {
            if (NmsService.getInstance() != null)
                mContext = NmsService.getInstance().getApplicationContext();
        } else {
            mContext = context.getApplicationContext();
        }

        if (mInstance == null) {
            mInstance = new NmsContactApi();
        }

        return mInstance;
    }

    public boolean isMyselfEngineContactId(short engContactId) {
        if (engContactId <= 0) {
            NmsLog.error(TAG, "engContactId <= 0");
            return false;
        }

        short[] ids = engineadapter.get().nmsUIGetSelfContactIds();
        if (ids == null) {
            NmsLog.warn(TAG, "engContactIds is null");
            return false;
        }

        boolean result = false;

        for (short id : ids) {
            if (id == engContactId) {
                result = true;
                break;
            }
        }

        return result;
    }

    public boolean isHissageNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "isHissageNumber. number is empty!");
            return false;
        }
        if (number.contains("@")) {
            return false;
        }

        String cutNumber = NmsCommonUtils.nmsGetStandardPhoneNum(number);
        if (TextUtils.isEmpty(cutNumber)) {
            NmsLog.warn(TAG, "isHisssageNumber. cutNumber is empty!");
        }

        return engineadapter.get().nmsUIGetIsHesineAccount(cutNumber) == 0 ? true : false;
    }


	public NmsUIContact getContactInfoViaEngineContactId(short engineContactId) {		
	    NmsUIContact contact = new NmsUIContact();
		NmsContact tmp = engineadapter.get().nmsUIGetContact(engineContactId);
		contact.setEngineContactId(engineContactId);
		contact.setSystemContactId(engineContactId);
		contact.setType(NmsContactType.HISSAGE_USER);
		contact.setName(tmp.getName());
		contact.setNumberOrEmail(tmp.getNumber());
		return contact;
	}

    public boolean isIpMessageSrvNumber(String number) {
        NmsLog.trace(TAG, "isIpMessageSrvNumber. number is "+ number);
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "isIpMessageSrvNumber. number is empty!");
            return false;
        }
        if (number.contains("@")) {
            NmsLog.error(TAG, "isIpMessageSrvNumber. number is include contain @");
            return false;
        }

		short[] ids = engineadapter.get().nmsUIGetServiceContactList();
		if (ids != null) {
			for (int i=0; i<ids.length; ++i) {
		        NmsLog.trace(TAG, "hesine customer service id is "+ids[i]);
				NmsUIContact contact = getContactInfoViaEngineContactId(ids[i]);
				if (contact != null && contact.getNumberOrEmail().equals(number)) {
					NmsLog.trace(TAG, "hesine customer service id name "+contact.getName());
					return true;
				}
			}
		}else{
			NmsLog.trace(TAG, "hesine customer service num is null");
		}
        return false;
    }

    public String getIpMessageSrvNumberName(String number) {
        NmsLog.trace(TAG, "getIpMessageSrvNumberName. number is "+ number);
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "getIpMessageSrvNumberName. number is empty!");
            return null;
        }
        if (number.contains("@")) {
            NmsLog.error(TAG, "getIpMessageSrvNumberName. number is include contain @");
            return null;
        }

		short[] ids = engineadapter.get().nmsUIGetServiceContactList();
		if (ids != null) {
			for (int i=0; i<ids.length; ++i) {
		        NmsLog.trace(TAG, "hesine customer service id is "+ids[i]);
				NmsUIContact contact = getContactInfoViaEngineContactId(ids[i]);
				if (contact != null && contact.getNumberOrEmail().equals(number)) {
					NmsLog.trace(TAG, "hesine customer service id name "+contact.getName());
					return contact.getName();
				}
			}
		}else{
			NmsLog.trace(TAG, "hesine customer service num is null");
		}
        return null;
    }

    public boolean isExistSystemContactViaNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "isExistSystemContact. number is empty!");
            return false;
        }

        if (getSystemContactSummaryViaNumber(number) != null) {
            return true;
        } else {
            return false;
        }
    }

    public boolean isExistSystemContactViaEmail(String email) {
        if (TextUtils.isEmpty(email)) {
            NmsLog.error(TAG, "isExistSystemContactViaEmail. email is empty!");
            return false;
        }
        if (!email.contains("@")) {
            NmsLog.error(TAG, "isExistSystemContactViaEmail. email is invalid");
            return false;
        }

        String encodeEmail = Uri.encode(email);
        if (TextUtils.isEmpty(encodeEmail)) {
            NmsLog.error(TAG, "isExistSystemContactViaEmail. encodeEmail is empty!");
            return false;
        }
        Cursor cursor = null;
        boolean result = false;
        try {
            Uri lookupUri = Uri.withAppendedPath(Email.CONTENT_LOOKUP_URI, encodeEmail);
            cursor = NmsContentResolver.query(mContext.getContentResolver(), lookupUri,
                    new String[] { Email.CONTACT_ID }, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                result = true;
            }

        } catch (Exception e) {
            NmsLog.error(TAG, "email: " + email + ". encodeEmail: " + encodeEmail);
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return result;
    }

    public long getSystemContactIdViaNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "getSystemContactIdViaNumber. number is empty!");
            return -1;
        }

        long result = -1;

        SystemContactSummary cs = getSystemContactSummaryViaNumber(number);
        if (cs != null) {
            result = cs.contactId;
        }
        return result;
    }

    public String getSystemNameViaNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "getSystemNameViaNumber. number is empty!");
            return null;
        }

        String result = "";

        SystemContactSummary cs = getSystemContactSummaryViaNumber(number);
        if (cs != null) {
            result = cs.name;
            // NmsLog.error("TAG", "test!!! number=" + number + ", name=" +
            // result);
        }
        return result == null ? "" : result;
    }

    public String getSystemNumberViaNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "getSystemNumberViaNumber. number is empty!");
            return null;
        }

        String result = "";

        SystemContactSummary cs = getSystemContactSummaryViaNumber(number);
        if (cs != null) {
            result = cs.number;
        }
        return result == null ? "" : result;
    }

    private SystemContactSummary getSystemContactSummaryViaNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "getSystemContactSummaryViaNumber. number is empty!");
            return null;
        }

        String encodeNumber = Uri.encode(number);
        if (TextUtils.isEmpty(encodeNumber)) {
            NmsLog.error(TAG, "getSystemContactSummaryViaNumber. encodeNumber is empty!");
            return null;
        }

        SystemContactSummary result = null;
        Cursor cursor = null;
        try {
            Uri lookupUri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, encodeNumber);
            cursor = NmsContentResolver.query(mContext.getContentResolver(), lookupUri,
                    new String[] { PhoneLookup._ID, PhoneLookup.DISPLAY_NAME, PhoneLookup.NUMBER },
                    null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                result = new SystemContactSummary();
                result.contactId = cursor.getLong(cursor.getColumnIndex(PhoneLookup._ID));
                result.name = cursor.getString(cursor.getColumnIndex(PhoneLookup.DISPLAY_NAME));
                result.number = NmsCommonUtils.nmsGetStandardPhoneNum(cursor.getString(cursor
                        .getColumnIndex(PhoneLookup.NUMBER)));
            }

        } catch (Exception e) {
            NmsLog.error(TAG, "number: " + number + ". encodeNumber: " + encodeNumber);
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return result;
    }

    public String getSystemNameViaSystemContactId(long systemContactId) {
        if (systemContactId <= 0) {
            NmsLog.error(TAG, "getSystemNameViaSystemContactId: sysContactId is invalid!");
            return null;
        }

        String result = "";

        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(mContext.getContentResolver(), Contacts.CONTENT_URI,
                    new String[] { Contacts.DISPLAY_NAME }, Contacts._ID + "=?",
                    new String[] { String.valueOf(systemContactId) }, null);
            if (cursor != null && cursor.moveToFirst()) {
                result = cursor.getString(cursor.getColumnIndex(Contacts.DISPLAY_NAME));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return result == null ? "" : result;
    }

    public long getSystemContactIdViaContactUri(Uri uri) {
        if (uri == null) {
            NmsLog.error(TAG, "getSystemContactIdViaContactUri is invalid!");
            return -1;
        }

        long result = -1;
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(mContext.getContentResolver(), uri,
                    new String[] { Contacts._ID }, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                result = cursor.getLong(cursor.getColumnIndex(Contacts._ID));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        return result;
    }

    public short getEngineContactIdViaSystemThreadId(long threadId) {
        if (threadId <= 0) {
            NmsLog.error(TAG, "threadId2engineId. threadId is invalid!");
            return -1;
        }

        return NmsSMSMMSManager.getInstance(mContext).getEngineContactIdViaThreadId(threadId);
    }

    public short getEngineContactIdViaSystemMsgId(long msgId) {
        if (msgId <= 0) {
            NmsLog.error(TAG, "msgId2engineId. msgId is invalid!");
            return -1;
        }

        long threadId = NmsSMSMMSManager.getInstance(mContext).getThreadViaSysMsgId(msgId);
        if (threadId <= 0) {
            NmsLog.error(TAG, "msgId2engineId. threadId is invalid!");
            return -1;
        }

        return getEngineContactIdViaSystemThreadId(threadId);
    }

    public short getEngineContactIdViaNumber(String number) {
        String formatNumber = null;

        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "number2engineId. number is empty!");
            return -1;
        }
        if (number.contains(",")) {// for broadcast
            formatNumber = number;
        } else {
            if (number.length() > NmsCustomUIConfig.PHONENUM_MAX_LENGTH
                    && !NmsGroupChatContact.isGroupChatContactNumber(number)) {
                NmsLog.error(TAG, "number2engineId. number is too long!");
                return -1;
            }

            if (!NmsCommonUtils.isPhoneNumberValid(number)) {
                NmsLog.error(TAG, "number2engineId. number(" + number + ") is invalid!");
                return -1;
            }

            formatNumber = NmsCommonUtils.nmsGetStandardPhoneNum(number);
            if (TextUtils.isEmpty(formatNumber)) {
                NmsLog.error(TAG, "number2engineId. formatNumber is invalid!");
            }
        }

        return (short) engineadapter.get().nmsUIGetContactId(formatNumber);
    }

    public Bitmap getSystemAvatarViaSystemContactId(long systemContactId) {
        if (systemContactId <= 0) {
            NmsLog.error(TAG, "getSystemAvatarViaSystemContactId. sysContactId is invalid!");
            return null;
        }

        Bitmap result = null;
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(mContext.getContentResolver(), Contacts.CONTENT_URI,
                    new String[] { Contacts.PHOTO_ID }, Contacts._ID + "=?",
                    new String[] { String.valueOf(systemContactId) }, null);
            if (cursor != null && cursor.moveToFirst()) {
                long photoId = cursor.getLong(cursor.getColumnIndex(Contacts.PHOTO_ID));
                if (photoId > 0) {
                    Uri uri = ContentUris.withAppendedId(Contacts.CONTENT_URI, systemContactId);
                    InputStream input = Contacts.openContactPhotoInputStream(
                            mContext.getContentResolver(), uri);
                    result = BitmapFactory.decodeStream(input);
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return result;
    }

    public Bitmap getSystemAvatarViaNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "getSystemAvatarViaNumber. number is empty!");
            return null;
        }

        long systemContactId = getSystemContactIdViaNumber(number);
        if (systemContactId <= 0) {
            NmsLog.trace(TAG, "this number is not in System Contact. number:" + number);
            return null;
        }

        return getSystemAvatarViaSystemContactId(systemContactId);
    }

    public Bitmap getEngineAvatarViaEngineContactId(short engineContactId) {
        if (engineContactId <= 0) {
            NmsLog.error(TAG, "getEngineAvatarViaEngineContactId. engineContactId is invalid!");
            return null;
        }

        Bitmap result = null;

        if (isMyselfEngineContactId(engineContactId)) {
			NmsContact contact = engineadapter.get().nmsUIGetContact(engineContactId);
            NmsProfileSettings userProfile = engineadapter.get().nmsUIGetUserInfoViaImsi(engineadapter.get().nmsUIGetImsiViaNumber(contact.getNumber()));
            if (userProfile != null) {
                result = userProfile.getProfileSettingsAvatar();
            }
        } else {
            SNmsImg photoPath = engineadapter.get().nmsUIGetContactImg(engineContactId);
            if (photoPath != null) {
                if (photoPath.imgPath != null) {
                    try {
                        result = BitmapFactory.decodeFile(photoPath.imgPath);
                    } catch (Exception e) {
                        NmsLog.warn(TAG, "BitmapFactory.decodeFile failed, engineContactId: "
                                + engineContactId);
                    }
                } else if (photoPath.byteImg != null) {
                    result = BitmapFactory.decodeByteArray(photoPath.byteImg, 0,
                            photoPath.byteImg.length);
                }
            }
        }

        return result;
    }

    public Bitmap getAvatarViaEngineContactId(short engineContactId) {
        if (engineContactId <= 0) {
            NmsLog.error(TAG, "getAvatarViaEngineContactId. engineContactId <= 0");
            return null;
        }

        Bitmap result = null;

        NmsContact contact = engineadapter.get().nmsUIGetContact(engineContactId);
        if (contact != null) {
            if (contact instanceof NmsGroupChatContact) {
                result = getGroupChatContactAvatar((NmsGroupChatContact) contact);
            } else if (contact instanceof NmsBroadCastContact) {
                result = mBroadCastBitmap;
            } else {
                result = getSystemAvatarViaNumber(contact.getNumber());
                if (result == null) {
                    result = getEngineAvatarViaEngineContactId(contact.getId());
                }
            }
        }

        return result;
    }

    public short getMyselfEngineContactIdViaSimId(int simId) {
        if (simId < 0) {
            NmsLog.error(TAG, "simId < 0.");
            return -1;
        }

        SNmsSimInfo simInfo = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(simId);
        if (simInfo == null) {
            NmsLog.error(TAG, "simInfo is null.");
            return -1;
        }

        return getEngineContactIdViaNumber(simInfo.number);
    }

    private Bitmap getMyselfAvatarViaSimId(int simId) {
        if (simId < 0) {
            NmsLog.error(TAG, "simId < 0..");
            return null;
        }

        SNmsSimInfo simInfo = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(simId);
        if (simInfo == null) {
            NmsLog.error(TAG, "simInfo is null");
            return null;
        }

        Bitmap result = getSystemAvatarViaNumber(simInfo.number);
        if (result == null) {
            NmsProfileSettings userProfile = engineadapter.get().nmsUIGetUserInfoViaImsi(simInfo.imsi);
            if (userProfile != null) {
                result = userProfile.getProfileSettingsAvatar();
            }
        }

        return result;
    }

    private ArrayList<Bitmap> fillBitmapList(int count, ArrayList<Bitmap> bitmapList,
            Bitmap defaultBitmap, Bitmap blankBitmap) {
        if (count < 0 || bitmapList == null || defaultBitmap == null || blankBitmap == null) {
            NmsLog.error(TAG, "fillBitmapList. parm is error");
            return null;
        }

        int size = bitmapList.size();

        if (count == 0) {
            if (size == 0) {
                bitmapList.add(blankBitmap);
                bitmapList.add(blankBitmap);
                bitmapList.add(blankBitmap);
            } else {
                NmsLog.error(TAG, "count==0, size>0");
                return null;
            }
        } else if (count == 1) {
            if (size == 0) {
                bitmapList.add(defaultBitmap);
                bitmapList.add(blankBitmap);
                bitmapList.add(blankBitmap);
            } else if (size == 1) {
                bitmapList.add(blankBitmap);
                bitmapList.add(blankBitmap);
            } else {
                NmsLog.error(TAG, "count==1, size>1");
                return null;
            }
        } else if (count == 2) {
            if (size == 0) {
                bitmapList.add(defaultBitmap);
                bitmapList.add(defaultBitmap);
                bitmapList.add(blankBitmap);
            } else if (size == 1) {
                bitmapList.add(defaultBitmap);
                bitmapList.add(blankBitmap);
            } else if (size == 2) {
                bitmapList.add(blankBitmap);
            } else {
                NmsLog.error(TAG, "count==2, size>2");
                return null;
            }
        } else if (count >= 3) {
            if (size == 0) {
                bitmapList.add(defaultBitmap);
                bitmapList.add(defaultBitmap);
                bitmapList.add(defaultBitmap);
            } else if (size == 1) {
                bitmapList.add(defaultBitmap);
                bitmapList.add(defaultBitmap);
            } else if (size == 2) {
                bitmapList.add(defaultBitmap);
            } else if (size == 3) {
            } else {
                NmsLog.error(TAG, "count>=3, size>3");
                return null;
            }
        } else {
            NmsLog.error(TAG, "count<0");
            return null;
        }

        return bitmapList;
    }

    public Bitmap getGroupChatContactAvatar(NmsGroupChatContact groupChatContact) {
        if (groupChatContact == null) {
            NmsLog.error(TAG, "groupChatContact is invalid!");
            return null;
        }

        ArrayList<Bitmap> bitmapList = new ArrayList<Bitmap>();

        short[] memberIds = groupChatContact.getMemberIds();

        if (memberIds != null) {
            for (int index = 0; index < memberIds.length && bitmapList.size() < 3; ++index) {
                Bitmap avatar = getAvatarViaEngineContactId(memberIds[index]);
                if (avatar != null) {
                    bitmapList.add(avatar);
                }
            }
        } else {
            NmsLog.warn(TAG, "memberIds is null.");
        }

        if (bitmapList.size() < 3 && groupChatContact.isAlive()) {
            // myself bitmap
            Bitmap avatar = getMyselfAvatarViaSimId(groupChatContact.getSimId());
            if (avatar == null) {
                avatar = mDefaultBitmap;
            }
            bitmapList.add(avatar);
        }

        bitmapList = fillBitmapList(groupChatContact.getMemberCount(), bitmapList, mDefaultBitmap,
                mBlankBitmap);

        Bitmap result = null;
        if (bitmapList != null && bitmapList.size() == 3) {
            result = NmsCommonUtils.stitchBitmap(bitmapList.get(0), bitmapList.get(1),
                    bitmapList.get(2));
        }

        return result;
    }

}
