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
 */

package android.appwidget;

import android.app.AppGlobals;
import android.content.Context;
import android.content.pm.IPackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.SystemProperties;
import android.os.RemoteException;
import android.os.UserHandle;
import android.text.TextUtils;
import android.util.AtomicFile;
import android.util.Log;
import android.util.Slog;
import android.util.Xml;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import android.os.Build;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

/**
 * Determine AppWidget Context
 */
public final class AppWidgetContext {
    private static final String TAG = "AppWidgetContext";
    private static final String SETTINGS_FILENAME = "/system/etc/appwidget/3dwidgets.xml";
    private static ArrayList<String> m3DWidgetsList = new ArrayList<String>();
    static {
        m3DWidgetsList.add("com.mediatek.weather3dwidget");
        m3DWidgetsList.add("com.mediatek.videofavorites");
        loadConfigFile();
    }

    private AppWidgetContext() {}

    /*
     * @hide
     */
    public static Context newWidgetContext(Context context, String packageName, boolean hasUsedCustomerView)
                 throws PackageManager.NameNotFoundException {
        int contextPermission = Context.CONTEXT_RESTRICTED;

        Log.v(TAG, "package name: " + packageName);
        if (m3DWidgetsList.contains(packageName)) {
            Log.v(TAG, "context permission changed");
            contextPermission = Context.CONTEXT_INCLUDE_CODE
                    | Context.CONTEXT_IGNORE_SECURITY;
            if (hasUsedCustomerView) {
                ensurePackageDexOpt(packageName);
            }
        }

        Context theirContext = context.createPackageContext(packageName, contextPermission);
        return theirContext;
    }

    /*
     * @hide
     */
    public static Context newWidgetContextAsUser(Context context, String packageName, boolean hasUsedCustomerView,
                  UserHandle user) throws PackageManager.NameNotFoundException {
        int contextPermission = Context.CONTEXT_RESTRICTED;

        Log.v(TAG, "package name: " + packageName);
        if (m3DWidgetsList.contains(packageName)) {
            Log.v(TAG, "context permission changed");
            contextPermission = Context.CONTEXT_INCLUDE_CODE
                    | Context.CONTEXT_IGNORE_SECURITY;
            if (hasUsedCustomerView) {
                ensurePackageDexOpt(packageName);
            }
        } else {
            Log.v(TAG, "context permission not changed");
        }

        Context theirContext = context.createPackageContextAsUser(packageName, contextPermission, user);
        return theirContext;
    }

    static void loadConfigFile() {
        File configFile = getSettingsFile();
        if(configFile == null){
            return ;
        }
        AtomicFile file = new AtomicFile(configFile);
        try {
            FileInputStream stream = file.openRead();
            parseConfigFile(stream);

            if (stream != null) {
                try {
                    stream.close();
                } catch (IOException e) {
                    Slog.w(TAG, "Failed to close state FileInputStream " + e);
                }
            }
        } catch (FileNotFoundException e) {
            Slog.w(TAG, "Failed to read state: " + e);
        }
    }

    @SuppressWarnings("unused")
    static void parseConfigFile(FileInputStream stream) {
        boolean success = false;
        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(stream, null);

            int type;
            do {
                type = parser.next();
                if (type == XmlPullParser.START_TAG) {
                    String tag = parser.getName();
                    if ("item".equals(tag)) {
                        String pkg = parser.getAttributeValue(null, "pkg");
                        Log.v(TAG, "package name = " + pkg);
                        if(!TextUtils.isEmpty(pkg) && !m3DWidgetsList.contains(pkg)){
                            m3DWidgetsList.add(pkg.trim());
                        }
                    }
                }
            } while (type != XmlPullParser.END_DOCUMENT);
        } catch (NullPointerException e) {
            Slog.w(TAG, "failed parsing " + e);
        } catch (NumberFormatException e) {
            Slog.w(TAG, "failed parsing " + e);
        } catch (XmlPullParserException e) {
            Slog.w(TAG, "failed parsing " + e);
        } catch (IOException e) {
            Slog.w(TAG, "failed parsing " + e);
        } catch (IndexOutOfBoundsException e) {
            Slog.w(TAG, "failed parsing " + e);
        }
    }

    private static File getSettingsFile() {
        File file = new File(SETTINGS_FILENAME);
        if(file.exists()){
            return file;
        }
        return null;
    }

    private static void ensurePackageDexOpt(String packageName) {
        /* This action is only for eng build. */
        if (!Build.TYPE.equals("eng")) {
            return;
        }

        IPackageManager ipm = AppGlobals.getPackageManager();
        try {
            if (ipm.enforceDexOpt(packageName)) {
                Log.d(TAG, "AppWidgetContext performDexOpt done, " + packageName);
            } else {
                Log.d(TAG, "AppWidgetContext performDexOpt not work, " + packageName);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "plugin performDexOpt exception occur");
        }
    }
}
