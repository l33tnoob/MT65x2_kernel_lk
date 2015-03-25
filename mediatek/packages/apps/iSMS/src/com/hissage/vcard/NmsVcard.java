package com.hissage.vcard;

import java.io.UnsupportedEncodingException;

import android.content.Context;
import android.database.Cursor;
import android.provider.Contacts;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Data;
import android.text.TextUtils;

import com.hissage.db.NmsContentResolver;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class NmsVcard {

    private static String mContactId = null;
    private static Context mContext = null;
    // private static ContactStruct contact;
    private static NmsContactStruct Montact;

    public NmsVcard(Context context, NmsContactStruct struct, String cId) {
        mContext = context;
        Montact = struct;
        mContactId = cId;
    }

    public void initAllData(boolean isAll) {
        Montact.UID = mContactId;
        try {
            getPhone();
            getFormartName();
            if (isAll) {
                getEmail();
                getTimeStamp();
                getOthers();
                getPhoto();
                getIm();
                getUrl();
            }

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private void getOthers() {
        Montact.others = "";
        getPostal();
        getOrganization();
        getNote();
    }

    private void getTimeStamp() {

        // Cursor c =
        // context.getContentResolver().query(RawContacts.CONTENT_URI, new
        // String[]{RawContacts.VERSION},
        // ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = " +
        // contactId,
        // null, null);
        // while (c.moveToNext()) {
        // int index = c.getColumnIndex(ContactsContract.RawContacts.VERSION);
        // contact.TimeStamp =
        // NmsConverter.int2String(NmsTimer.NmsGetSystemTime());
        // }
        // c.close();
    }

    private void getUrl() {
        Cursor url = NmsContentResolver.query(mContext.getContentResolver(), Data.CONTENT_URI,
                null, ContactsContract.CommonDataKinds.Website.CONTACT_ID + " = " + mContactId
                        + " and " + Data.MIMETYPE + "='"
                        + ContactsContract.CommonDataKinds.Website.CONTENT_ITEM_TYPE + "'", null,
                null);
        if (url != null) {
            while (url.moveToNext()) {
                Montact.others += "URL:"
                        + url.getString(url
                                .getColumnIndex(ContactsContract.CommonDataKinds.Website.URL))
                        + "\r\n";
            }
            url.close();
        }
    }

    public NmsContactStruct getContact() {
        mContext = null;
        mContactId = null;
        return Montact;
    }

    private void getPhone() {
        Cursor phones = null;
        try{
            phones = NmsContentResolver.query(mContext.getContentResolver(),
                ContactsContract.CommonDataKinds.Phone.CONTENT_URI, new String[] {
                        ContactsContract.CommonDataKinds.Phone.NUMBER,
                        ContactsContract.CommonDataKinds.Phone.TYPE,
                        ContactsContract.CommonDataKinds.Phone.LABEL,
                        ContactsContract.CommonDataKinds.Phone.IS_PRIMARY },
                ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = " + mContactId, null, null);
        while (phones.moveToNext()) {
            String phoneNumber = phones.getString(phones
                    .getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
            int phoneType = phones.getInt(phones
                    .getColumnIndex(ContactsContract.CommonDataKinds.Phone.TYPE));
            String phoneLabel = phones.getString(phones
                    .getColumnIndex(ContactsContract.CommonDataKinds.Phone.LABEL));
            String phoneIsPrimary = phones.getString(phones
                    .getColumnIndex(ContactsContract.CommonDataKinds.Phone.IS_PRIMARY));

            phoneNumber = replaceExceptionalChars(phoneNumber);
            Montact.addPhone(phoneType, phoneNumber, phoneLabel, phoneIsPrimary.equals("0") ? false
                    : true);
        }
        }catch(Exception e){
            NmsLog.nmsPrintStackTrace(e);
        }finally{
            if(phones != null){
                phones.close();
            }
        }

    }

    private void getEmail() {
        Cursor emails = NmsContentResolver.query(mContext.getContentResolver(),
                ContactsContract.CommonDataKinds.Email.CONTENT_URI, new String[] {
                        ContactsContract.CommonDataKinds.Email.DATA,
                        ContactsContract.CommonDataKinds.Email.TYPE,
                        ContactsContract.CommonDataKinds.Email.IS_PRIMARY },
                ContactsContract.CommonDataKinds.Email.CONTACT_ID + " = " + mContactId, null, null);
        if (emails != null) {
            while (emails.moveToNext()) {
                String emailAddress = emails.getString(emails
                        .getColumnIndex(ContactsContract.CommonDataKinds.Email.DATA));
                int emailType = emails.getInt(emails
                        .getColumnIndex(ContactsContract.CommonDataKinds.Email.TYPE));
                String emailIsPrimary = emails.getString(emails
                        .getColumnIndex(ContactsContract.CommonDataKinds.Email.IS_PRIMARY));
                Montact.addContactmethod(Contacts.KIND_EMAIL, emailType, emailAddress, "",
                        emailIsPrimary.equals("0") ? false : true);
            }
            emails.close();
        }
    }

    private void getIm() {
        Cursor ims = NmsContentResolver.query(mContext.getContentResolver(), Data.CONTENT_URI,
                new String[] { ContactsContract.CommonDataKinds.Im.TYPE,
                        ContactsContract.CommonDataKinds.Im.DATA,
                        ContactsContract.CommonDataKinds.Im.LABEL },
                ContactsContract.CommonDataKinds.Im.CONTACT_ID + "=" + mContactId + " and "
                        + Data.MIMETYPE + "='"
                        + ContactsContract.CommonDataKinds.Im.CONTENT_ITEM_TYPE + "'", null, null);
        if (ims != null) {
            while (ims.moveToNext()) {
                int ImType = ims.getInt(ims
                        .getColumnIndex(ContactsContract.CommonDataKinds.Im.TYPE));
                String ImData = ims.getString(ims
                        .getColumnIndex(ContactsContract.CommonDataKinds.Im.DATA));
                String ImLabel = ims.getString(ims
                        .getColumnIndex(ContactsContract.CommonDataKinds.Im.LABEL));

                Montact.others += (TextUtils.isEmpty(ImLabel) ? "X-IM" : ImLabel) + ":" + ImData
                        + "\r\n";
            }
            ims.close();
        }
    }

    private void getPostal() {
        String addrWhere = ContactsContract.Data.CONTACT_ID + " = ? AND "
                + ContactsContract.Data.MIMETYPE + " = ?";
        String[] addrWhereParams = new String[] { mContactId,
                ContactsContract.CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE };
        Cursor addrCur = null;
        try{
            addrCur = NmsContentResolver.query(mContext.getContentResolver(),

                ContactsContract.Data.CONTENT_URI, null, addrWhere, addrWhereParams, null);
        while (addrCur != null && addrCur.moveToNext()) {
            String postal = "ADR;";
            int type = addrCur.getInt(addrCur
                    .getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.TYPE));

            if (type == ContactsContract.CommonDataKinds.StructuredPostal.TYPE_WORK) {
                postal += "WORK;";
            } else {
                postal += "HOME;";
            }
            postal += "ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:";
            try {
                String poBox = addrCur.getString(addrCur
                        .getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.POBOX));
                postal += ConvertString2VCardUTF8(poBox) + ";";

                String street = addrCur.getString(addrCur
                        .getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.STREET));
                postal += ConvertString2VCardUTF8(street) + ";";

                String city = addrCur.getString(addrCur
                        .getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.CITY));
                postal += ConvertString2VCardUTF8(city) + ";";

                String state = addrCur.getString(addrCur
                        .getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.REGION));
                postal += ConvertString2VCardUTF8(state) + ";";

                String country = addrCur.getString(addrCur
                        .getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.COUNTRY));
                postal += ConvertString2VCardUTF8(country) + ";\r\n";
                Montact.others += postal;
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
            }
        }
        }catch(Exception e){
            NmsLog.nmsPrintStackTrace(e);
        }finally{
            if(addrCur != null){
        addrCur.close();
            }
        }

    }

    private void getOrganization() {
        Cursor organization = null;
        try {
            organization = NmsContentResolver
                    .query(mContext.getContentResolver(),
                            Data.CONTENT_URI,
                            new String[] { ContactsContract.CommonDataKinds.Organization.TYPE,
                                    ContactsContract.CommonDataKinds.Organization.COMPANY,
                                    ContactsContract.CommonDataKinds.Organization.PHONETIC_NAME,
                                    ContactsContract.CommonDataKinds.Organization.TITLE,
                                    ContactsContract.CommonDataKinds.Organization.IS_PRIMARY },
                            ContactsContract.CommonDataKinds.Organization.CONTACT_ID
                                    + " = "
                                    + mContactId
                                    + " and "
                                    + Data.MIMETYPE
                                    + "='"
                                    + ContactsContract.CommonDataKinds.Organization.CONTENT_ITEM_TYPE
                                    + "'", null, null);
            if (organization != null) {
                while (organization.moveToNext()) {
                    int type = organization.getInt(organization
                            .getColumnIndex(ContactsContract.CommonDataKinds.Organization.TYPE));

                    String company = organization.getString(organization
                            .getColumnIndex(ContactsContract.CommonDataKinds.Organization.COMPANY));

                    String positionName = organization
                            .getString(organization
                                    .getColumnIndex(ContactsContract.CommonDataKinds.Organization.PHONETIC_NAME));
                    String title = organization.getString(organization
                            .getColumnIndex(ContactsContract.CommonDataKinds.Organization.TITLE));
                    company = company + " " + title;

                    String isPrimary = organization
                            .getString(organization
                                    .getColumnIndex(ContactsContract.CommonDataKinds.Organization.IS_PRIMARY));

                    if (!TextUtils.isEmpty(company)) {
                        try {
                            company = "ORG;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:"
                                    + ConvertString2VCardUTF8(company) + "\r\n";
                            Montact.others = Montact.others + company;
                        } catch (UnsupportedEncodingException e) {
                            // TODO Auto-generated catch block
                            NmsLog.nmsPrintStackTrace(e);
                        }
                    }
                    if (!TextUtils.isEmpty(positionName)) {
                        try {
                            positionName = "ADR;WORK;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:"
                                    + ConvertString2VCardUTF8(positionName) + "\r\n";
                            Montact.others = Montact.others + positionName;
                        } catch (UnsupportedEncodingException e) {
                            // TODO Auto-generated catch block
                            NmsLog.nmsPrintStackTrace(e);
                        }
                    }
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != organization) {
                organization.close();
            }
        }
    }

    private void getNote() {
        Cursor note = NmsContentResolver
                .query(mContext.getContentResolver(), Data.CONTENT_URI,
                        new String[] { ContactsContract.CommonDataKinds.Note.NOTE },
                        ContactsContract.CommonDataKinds.Note.CONTACT_ID + " = " + mContactId
                                + " and " + Data.MIMETYPE + "='"
                                + ContactsContract.CommonDataKinds.Note.CONTENT_ITEM_TYPE + "'",
                        null, null);

        if (note != null) {
            while (note.moveToNext()) {
                String notes = note.getString(note
                        .getColumnIndex(ContactsContract.CommonDataKinds.Note.NOTE));
                if (!TextUtils.isEmpty(notes)) {
                    try {
                        notes = "NOTE;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:"
                                + ConvertString2VCardUTF8(notes) + "\r\n";
                    } catch (UnsupportedEncodingException e) {
                        // TODO Auto-generated catch block
                        NmsLog.nmsPrintStackTrace(e);
                    }
                    Montact.others += notes;
                }
            }
            note.close();
        }
    }

    public static byte ConverAsc2Char(byte aChar) {
        if (aChar >= 0xA && aChar <= 0xF)
            return (byte) (aChar + 65 - 10);
        else if (aChar >= 0 && aChar <= 9)
            return (byte) (aChar + 48);
        else {
            return 0;
        }
    }

    public static String ConvertString2VCardUTF8(String str) throws UnsupportedEncodingException {
        if (TextUtils.isEmpty(str))
            return "";
        byte[] pStrUtf8 = str.getBytes("UTF-8");
        byte[] pStrVCardUtf8 = new byte[pStrUtf8.length * 4];
        int j = 0;

        for (int i = 0; i < pStrUtf8.length; i++) {

            pStrVCardUtf8[j] = (byte) '=';
            j++;

            byte temp = pStrUtf8[i];
            temp = (byte) (temp >> 4);
            byte temp1 = (byte) 0x0f;

            temp = (byte) (temp & temp1);

            pStrVCardUtf8[j] = temp;
            pStrVCardUtf8[j] = ConverAsc2Char(pStrVCardUtf8[j]);
            j++;

            pStrVCardUtf8[j] = (byte) (pStrUtf8[i] & temp1);
            pStrVCardUtf8[j] = ConverAsc2Char(pStrVCardUtf8[j]);
            j++;

        }
        return new String(pStrVCardUtf8, 0, j);
    }

    private void getFormartName() {
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(mContext.getContentResolver(),
                    ContactsContract.Contacts.CONTENT_URI,
                    new String[] { ContactsContract.Contacts.DISPLAY_NAME },
                    ContactsContract.Contacts._ID + "=" + mContactId, null, null);
            while (cursor != null && cursor.moveToNext()) {
                Montact.name = cursor.getString(cursor
                        .getColumnIndex(ContactsContract.Contacts.DISPLAY_NAME));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        if (Montact.name != null) {
            // here should delete
            Montact.name = Montact.name.trim();
            Montact.sourceName = Montact.name;
            String temp = "ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:";
            try {
                temp += ConvertString2VCardUTF8(Montact.name);
                Montact.name = temp;

            } catch (UnsupportedEncodingException e) {
                NmsLog.nmsPrintStackTrace(e);
            }
        }
    }

    private void getPhoto() {
        Cursor photo = null;
        try {
            photo = NmsContentResolver.query(mContext.getContentResolver(), Data.CONTENT_URI,

            null, ContactsContract.CommonDataKinds.Photo.CONTACT_ID + " = " + mContactId + " and "
                    + Data.MIMETYPE + "='"
                    + ContactsContract.CommonDataKinds.Photo.CONTENT_ITEM_TYPE + "'", null, null);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (photo != null) {
                while (photo.moveToNext()) {
                    Montact.photoBytes = photo.getBlob(photo
                            .getColumnIndex(ContactsContract.CommonDataKinds.Photo.PHOTO));
                }
                photo.close();
            }
        }
    }

    private String replaceExceptionalChars(String sourceString) {

        String desString = sourceString.replaceAll("[^+0-9]", "");
        NmsLog.trace(HissageTag.vcard, "finish to replace excetional chars,target String:"
                + desString.toString() + ", source String:" + sourceString);
        return desString;
    }
}
