package com.hissage.vcard;

import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;

import com.hissage.db.NmsContentResolver;
import com.hissage.message.ip.NmsIpMessageConsts.NmsUpdateSystemContactAction;
import com.hissage.timer.NmsTimer;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class NmsContactObserver extends ContentObserver {

    private static ContentResolver mRecolver;
    private final static Uri uriDirtyContacts = RawContacts.CONTENT_URI;
    private static NmsContactObserver mObserver = null;
    private static boolean mStartFlag = true;
    private static Context mContext = null;

    private static ArrayList<Integer> contactList = new ArrayList<Integer>();

    public static void initContactList() {
        contactList.clear();
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(mContext.getContentResolver(),
                    ContactsContract.Contacts.CONTENT_URI,
                    new String[] { ContactsContract.Contacts._ID }, null, null, null);
            while (null != cursor && cursor.moveToNext()) {
                contactList
                        .add(cursor.getInt(cursor.getColumnIndex(ContactsContract.Contacts._ID)));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    public static void getDeletedContact(OutputStreamWriter writer) throws IOException {
        Cursor csrIn = null;
        try {
            csrIn = NmsContentResolver.query(mContext.getContentResolver(),
                    ContactsContract.Contacts.CONTENT_URI,
                    new String[] { ContactsContract.Contacts._ID }, null, null, null);
            if ((null != csrIn) && (csrIn.getCount() > 0) && (csrIn.moveToFirst())) {
                int i = 0;
                for (i = 0; i < csrIn.getCount();) {
                    csrIn.moveToPosition(i);
                    int currentId = csrIn.getInt(csrIn
                            .getColumnIndex(ContactsContract.Contacts._ID));
                    if (contactList.size() > i) {
                        int sourceId = contactList.get(i);
                        if (sourceId == currentId) {
                            i++;
                            continue;
                        } else {
                            NmsLog.trace(HissageTag.vcard, "export deleted contact id: " + sourceId);
                            writer.write("BEGIN:VCARD\r\nUID:" + sourceId + "\r\nEND:VCARD\r\n");
                            contactList.remove(i);
                            continue;
                        }
                    } else {
                        contactList.add(currentId);
                        i++;
                        continue;
                    }
                }
                if (contactList.size() > i) {
                    for (int j = contactList.size(); j > i; j--) {
                        int dirtyId = contactList.get(i);
                        NmsLog.trace(HissageTag.vcard, "export deleted contact id: " + dirtyId);
                        writer.write("BEGIN:VCARD\r\nUID:" + dirtyId + "\r\nEND:VCARD\r\n");
                        contactList.remove(i);
                    }
                }
            } else {
                for (int i = 0; i < contactList.size(); ++i) {
                    int dirtyId = contactList.get(i);
                    NmsLog.trace(HissageTag.vcard, "export deleted contact id: " + dirtyId);
                    writer.write("BEGIN:VCARD\r\nUID:" + dirtyId + "\r\nEND:VCARD\r\n");
                }
                contactList.clear();
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != csrIn) {
                csrIn.close();
            }
        }

    }

    public NmsContactObserver() {
        super(null);
        // TODO Auto-generated constructor stub
    }

    public static void registerContentObserver(Context context, boolean flag) {
        mContext = context;
        if (null == mRecolver) {
            mRecolver = context.getContentResolver();
        }
        if (null == mObserver) {
            mObserver = new NmsContactObserver();
            mRecolver.registerContentObserver(uriDirtyContacts, true, mObserver);
            initContactList();
        }
        NmsLog.trace(HissageTag.vcard, "registerContentObserver, flag: " + flag);

        mStartFlag = flag;
    }

    public static void unregisterContentObserver(boolean flag) {
        NmsLog.trace(HissageTag.vcard, "unregisterContentObserver, flag: " + flag);
        if (flag) {
            mRecolver.unregisterContentObserver(mObserver);
            mObserver = null;
        }
        mStartFlag = false;
    }

    @Override
    public boolean deliverSelfNotifications() {
        return super.deliverSelfNotifications();
    }

    @Override
    public void onChange(boolean selfChange) {
        NmsLog.error(HissageTag.vcard,
                "contacts changed, so we reset contact observer timer.start flag:" + mStartFlag);
        sendRefreshContactBroadcast();
        if (mStartFlag) {
            NmsTimer.NmsKillTimer(NmsTimer.NMS_TIMERID_CONTACT);
            NmsTimer.NmsSetTimer(NmsTimer.NMS_TIMERID_CONTACT, 5);
        }
    }

    private void sendRefreshContactBroadcast() {
        Intent intent = new Intent();
        intent.setAction(NmsUpdateSystemContactAction.NMS_UPDATE_CONTACT);
        mContext.sendBroadcast(intent);
    }

}