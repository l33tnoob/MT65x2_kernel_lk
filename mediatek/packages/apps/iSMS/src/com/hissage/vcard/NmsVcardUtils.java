package com.hissage.vcard;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;

import com.hissage.config.NmsCommonUtils;
import com.hissage.db.NmsContentResolver;
import com.hissage.jni.engineadapter;
import com.hissage.service.NmsService;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class NmsVcardUtils {

    private static AtomicInteger threadGuard = new AtomicInteger(0);

    public static String nmsGetVcfViaSysContactsId(Context context, long[] contactsId) {
        if (null == context || contactsId == null) {
            NmsLog.error(HissageTag.vcard, "get vcf file error, param is/are null");
            return null;
        }

        String path = NmsCommonUtils.getCachePath(context) + System.currentTimeMillis() + "_"
                + contactsId.length + ".vcf";

        File file = createVcardFile(path);

        OutputStreamWriter writer = null;
        try {
            writer = new OutputStreamWriter(new FileOutputStream(file), "UTF-8");
        } catch (Exception e) {
            NmsLog.error(HissageTag.vcard,
                    "new OutputStreamWriter error:" + NmsLog.nmsGetStactTrace(e));
            return null;
        }

        for (int i = 0; i < contactsId.length; ++i) {
            NmsContactStruct contact = new NmsContactStruct();
            NmsVcard vcard = new NmsVcard(context, contact, "" + contactsId[i]);
            vcard.initAllData(true);

            try {
                VCardComposer composer = new VCardComposer();

                writer.write(composer.createVCard(contact, VCardComposer.VERSION_VCARD21_INT));
                writer.write("\n"); // add empty lines between contacts
            } catch (Exception e) {
                NmsLog.error(HissageTag.vcard, "NmsGetVcfViaSysContactId exception, name: "
                        + contact.sourceName);
                path = null;
                break;
            }

        }

        try {
            if (null != writer) {
                writer.close();
            }
        } catch (IOException e) {
            NmsLog.error(HissageTag.vcard,
                    "close OutputStreamWriter error:" + NmsLog.nmsGetStactTrace(e));
        }

        return path;
    }

    public static void nmsExportContactFromNative2File(Context context, String fileName)
            throws Exception {

        if (!threadGuard.compareAndSet(0, 1)) {
            NmsLog.warn(HissageTag.vcard,
                    "a vcard export to file thread was running now, just discard current request");
            return;
        }

        NmsLog.trace(HissageTag.vcard, "run the vacrd export to file thread");

        File file = createVcardFile(fileName);
        OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(file), "UTF-8");
        VCardComposer composer = new VCardComposer();
        ArrayList<NmsContactStruct> vcardList = listContactFromAndroid(context);
        if (vcardList != null && vcardList.size() > 0) {
            for (NmsContactStruct contact : vcardList) {
                try {
                    writer.write(composer.createVCard(contact, VCardComposer.VERSION_VCARD21_INT));
                    writer.write("\n"); // add empty lines between contacts
                } catch (Exception e) {
                    NmsLog.error(HissageTag.vcard, "some contact is error, name: "
                            + contact.sourceName);
                }
            }
        }
        writer.close();

        NmsContactObserver.unregisterContentObserver(false);
        ContentValues values = new ContentValues();
        values.put(RawContacts.DIRTY, 0);
        NmsContentResolver.update(context.getContentResolver(), RawContacts.CONTENT_URI, values,
                RawContacts.DELETED + " = 0 and " + RawContacts.DIRTY + " = 1", null);
        NmsLog.trace(HissageTag.vcard, "write vcard file [" + file.getPath() + "] success!");
        NmsContactObserver.registerContentObserver(context, true);

        NmsLog.trace(HissageTag.vcard, "finish the vacrd export to file thread");

        threadGuard.set(0);
    }

    public static ArrayList<NmsContactStruct> listContactFromAndroid(Context context) {
        ArrayList<NmsContactStruct> list = new ArrayList<NmsContactStruct>();
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(context.getContentResolver(),
                    ContactsContract.Contacts.CONTENT_URI,
                    new String[] { ContactsContract.Contacts._ID }, null, null, null);
            while (null != cursor && cursor.moveToNext()) {
                NmsContactStruct contact = new NmsContactStruct();
                String contactId = cursor.getString(cursor
                        .getColumnIndex(ContactsContract.Contacts._ID));
                NmsVcard vcard = new NmsVcard(context, contact, contactId);
                vcard.initAllData(false);
                list.add(vcard.getContact());
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return list;
    }

    public static ArrayList<NmsContactStruct> listDirtyContactsFromAndroid(Context context) {
        ArrayList<NmsContactStruct> list = new ArrayList<NmsContactStruct>();
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(context.getContentResolver(),
                    RawContacts.CONTENT_URI, new String[] { RawContacts.CONTACT_ID },
                    RawContacts.DELETED + " = 0 and " + RawContacts.DIRTY + " = 1", null, null);
            while (null != cursor && cursor.moveToNext()) {
                NmsContactStruct contact = new NmsContactStruct();
                String contactId = cursor.getString(cursor.getColumnIndex(RawContacts.CONTACT_ID));
                NmsVcard vcard = new NmsVcard(context, contact, contactId);
                vcard.initAllData(false);
                list.add(vcard.getContact());
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return list;
    }

    public static void nmsDeleteContactViaUID(String strData, Context context) {
        String[] pars = strData.split(" ");
        NmsContactObserver.unregisterContentObserver(false);
        if (pars != null && pars.length > 0) {
            ContentResolver ConRes = context.getContentResolver();
            for (String s : pars) {
                NmsContentResolver.delete(ConRes, RawContacts.CONTENT_URI, RawContacts.DELETED
                        + " = 0 and " + RawContacts.CONTACT_ID + " = " + s, null);
            }
        }
        NmsContactObserver.registerContentObserver(context, true);
    }

    public static void notifyEngineContactChanged() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                String filepath = engineadapter.get().nmsGetCachePath() + "_vcard_for_java_CUL.tmp";
                nmsExportDirtyContactFromNative2File(NmsService.getInstance(), filepath);
                engineadapter.get().nmsUploadCUL();
            }
        }).start();
    }

    public static void nmsImportContactFromFile2Native(Context context, int msg, String fileName)
            throws Exception {
        File vcard = new File(fileName);
        if (!vcard.exists()) {
            NmsLog.trace(HissageTag.vcard, "Exception: Import vcard file " + fileName
                    + " is not exists!");
            return;
        }
        ImportVcardThread importVcardthread = new ImportVcardThread(fileName, context, msg);
        importVcardthread.start();
        NmsLog.trace(HissageTag.vcard, "parse the vcard and add to the contacts!");
    }

    private static boolean nmsExportDeletedContacts(OutputStreamWriter writer, Context context)
            throws IOException {
        Cursor cursor = null;
        boolean ret = false;
        try {
            cursor = NmsContentResolver.query(context.getContentResolver(),
                    RawContacts.CONTENT_URI, new String[] { RawContacts.CONTACT_ID },
                    RawContacts.DELETED + " = 1 and " + RawContacts.DIRTY + " = 1", null, null);

            while (null != cursor && cursor.moveToNext()) {
                int contactId = cursor.getInt(cursor.getColumnIndex(RawContacts.CONTACT_ID));
                if (contactId > 0) {
                    writer.write("BEGIN:VCARD\r\nUID:" + contactId + "\r\nEND:VCARD\r\n");
                    NmsLog.trace(HissageTag.vcard, "export deleted contact id: " + contactId);
                    ret = true;
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        NmsContactObserver.unregisterContentObserver(false);
        ContentValues values1 = new ContentValues();
        values1.put(RawContacts.DIRTY, 0);
        NmsContentResolver.update(context.getContentResolver(), RawContacts.CONTENT_URI, values1,
                RawContacts.DELETED + " = 1 and " + RawContacts.DIRTY + " = 1", null);
        return ret;
    }

    public static void nmsExportDirtyContactFromNative2File(Context context, String fileName) {
        File file = createVcardFile(fileName);

        OutputStreamWriter writer;
        try {
            writer = new OutputStreamWriter(new FileOutputStream(file), "UTF-8");

            VCardComposer composer = new VCardComposer();
            ArrayList<NmsContactStruct> vcardList = listDirtyContactsFromAndroid(context);
            if (vcardList != null && vcardList.size() > 0) {
                for (NmsContactStruct contact : vcardList) {
                    if (contact.name == null || contact.name.trim().equals("")) {
                        continue;
                    }
                    writer.write(composer.createVCard(contact, VCardComposer.VERSION_VCARD21_INT));
                    writer.write("\n"); // add empty lines between contacts
                }
            }

            if (nmsExportDeletedContacts(writer, context) != true) {// if delete
                                                                    // flag ok,
                                                                    // we just
                                                                    // need
                                                                    // re-init
                                                                    // contact
                                                                    // list
                NmsContactObserver.getDeletedContact(writer);
            } else {
                NmsContactObserver.initContactList();
            }

            writer.close();

            ContentValues values = new ContentValues();
            values.put(RawContacts.DIRTY, 0);
            NmsContentResolver.update(context.getContentResolver(), RawContacts.CONTENT_URI,
                    values, RawContacts.DELETED + " = 0 and " + RawContacts.DIRTY + " = 1", null);
            NmsLog.trace(HissageTag.vcard, "write drity vcard file [" + file.getPath()
                    + "] success!");
        } catch (UnsupportedEncodingException e) {
            NmsLog.nmsPrintStackTrace(e);
        } catch (FileNotFoundException e) {
            NmsLog.nmsPrintStackTrace(e);
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
        } catch (VCardException e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        NmsContactObserver.registerContentObserver(context, true);
    }

    public static void nmsExportDeleteContactFromNative2File(Context context, String fileName) {

        File vcardFile = NmsVcardUtils.createVcardFile(fileName);
        Cursor cursor = null;
        try {
            OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(vcardFile),
                    "UTF-8");

            cursor = NmsContentResolver.query(context.getContentResolver(),
                    RawContacts.CONTENT_URI, new String[] { RawContacts.CONTACT_ID },
                    RawContacts.DELETED + " = 1 and " + RawContacts.DIRTY + " = 1", null, null);
            while (null != cursor && cursor.moveToNext()) {
                String contactId = cursor.getString(cursor.getColumnIndex(RawContacts.CONTACT_ID));
                writer.write(contactId + " ");
                NmsLog.trace(HissageTag.vcard, "export deleted contact id: " + contactId);
            }
            cursor.close();
            writer.close();
        } catch (Exception e) {
            NmsLog.error(HissageTag.vcard, "==File not found exception:" + e);
        }finally{
            if(cursor != null){
                cursor.close();
            }
        }
        NmsContactObserver.unregisterContentObserver(false);
        ContentValues values = new ContentValues();
        values.put(RawContacts.DIRTY, 0);
        NmsContentResolver.update(context.getContentResolver(), RawContacts.CONTENT_URI, values,
                RawContacts.DELETED + " = 1 and " + RawContacts.DIRTY + " = 1", null);
        NmsLog.trace(HissageTag.vcard, "write vcard delete list file [" + vcardFile.getPath()
                + "] success!");
        NmsContactObserver.registerContentObserver(context, true);
    }

    public static File createVcardFile(String path) {
        File file = new File(path);
        if (!file.exists()) {
            file.getParentFile().mkdirs();
        } else {
            file.delete();
        }

        try {
            file.createNewFile();
        } catch (Exception e) {
            NmsLog.error(HissageTag.vcard,
                    "Failed to create vcard file:" + NmsLog.nmsGetStactTrace(e));
        }
        return file;
    }
}
