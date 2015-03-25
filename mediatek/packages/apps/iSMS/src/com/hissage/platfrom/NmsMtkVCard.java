package com.hissage.platfrom;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import android.text.TextUtils;

import android.content.Context;
import android.provider.ContactsContract.Contacts;
import com.android.vcard.VCardComposer;
import com.android.vcard.VCardConfig;

import com.hissage.config.NmsCommonUtils;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;
import com.hissage.vcard.NmsVcardUtils;

public class NmsMtkVCard extends NmsPlatformBase {

    private static final String TAG = "NmsMtkVCard";
    private Class<?> mSystemVCardComposerClass = null;
    private Context mContext = null;
    private static final String CLASS_PATH = "com.android.vcard.VCardComposer";

    public NmsMtkVCard(Context context) {
        super(context);
        try {
            mContext = context;
            mSystemVCardComposerClass = Class.forName(CLASS_PATH);
            mPlatfromMode = NMS_INTEGRATION_MODE;

        } catch (Exception e) {
            mPlatfromMode = NMS_STANDEALONE_MODE;
            NmsLog.warn(TAG, e.toString());
        }
    }

    private String nmsGetVcfViaMTKPlatform(Context context, String contactsId) {
        if (null == context || TextUtils.isEmpty(contactsId)) {
            NmsLog.error(HissageTag.vcard, "get vcf file error, param: "
                    + context + ", contactId: " + contactsId);
            return null;
        }

        String path = NmsCommonUtils.getCachePath(context) + System.currentTimeMillis() + ".vcf";

        try {
            File file = NmsVcardUtils.createVcardFile(path);

            OutputStreamWriter writer = null;
            VCardComposer composer = null;

            try {
                writer = new OutputStreamWriter(new FileOutputStream(file),
                        "UTF-8");

                composer = new VCardComposer(context);
                if (!composer.init(Contacts._ID + " IN (" + contactsId + " )",
                        null)) {
                    throw new Exception("can not get vcf from MTK FW.");
                }

                while (composer.isAfterLast()) {
                    writer.write(composer.createOneEntry());
                }

            } finally {

                if (composer != null) {
                    composer.terminate();
                }

                if (writer != null) {
                    writer.close();
                }

            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
            path = null;
        }
        return path;
    }

    public String nmsGetVcfViaSysContactId(Context context, long[] contactsId) {
        if (null == context || contactsId == null
                || contactsId.length <= 0) {
            NmsLog.error(HissageTag.vcard,
                    "get vcf file error, param is/are null");
            return null;
        }

        String contactStr = "";
        for (int i = 0; i < contactsId.length; ++i) {
            if (contactsId[i] <= 0) {
                NmsLog.error(TAG, "contactId <= 0, index=" + i);
                return null;
            }
            if (TextUtils.isEmpty(contactStr)) {
                contactStr = String.valueOf(contactsId[i]);
            } else {
                contactStr += "," + contactsId[i];
            }
        }
        
        if (null != mSystemVCardComposerClass
                && NMS_INTEGRATION_MODE == mPlatfromMode) {
            try {

                String path = NmsCommonUtils.getCachePath(context)
                        + System.currentTimeMillis() + "_" + contactsId.length
                        + ".vcf";

                File file = NmsVcardUtils.createVcardFile(path);

                OutputStreamWriter writer = new OutputStreamWriter(
                        new FileOutputStream(file), "UTF-8");

                Class param[] = new Class[1];
                param[0] = Context.class;

                Object objs[] = new Object[1];
                objs[0] = context;

                Constructor constructor = mSystemVCardComposerClass
                        .getConstructor(param);

                Object instance = constructor.newInstance(objs);

                Method methodInit = mSystemVCardComposerClass.getMethod("init",
                        String.class, String[].class);
                Method methodIsAfterLast = mSystemVCardComposerClass
                        .getMethod("isAfterLast");

                Method methodCreateOneEntry = mSystemVCardComposerClass
                        .getMethod("createOneEntry");

                Method methodTerminate = mSystemVCardComposerClass
                        .getMethod("terminate");

                if (!(Boolean) (methodInit.invoke(instance, Contacts._ID
                        + " IN (" + contactStr + ")", null))) {
                    throw new Exception("can not get vcf from MTK FW.");
                }

                while (!(Boolean) (methodIsAfterLast.invoke(instance))) {
                    writer.write((String) (methodCreateOneEntry
                            .invoke((instance))));
                }

                methodTerminate.invoke(instance);
                writer.close();

                NmsLog.trace(TAG,
                        "NmsMtkVCard get on contact vcf from mtk FW, contactsId:"
                                + contactStr);

                return path;

            } catch (Exception e) {
                return nmsGetVcfViaMTKPlatform(context, contactStr);
            }
        } else {
            return nmsGetVcfViaMTKPlatform(context, contactStr);
        }
    }
}

