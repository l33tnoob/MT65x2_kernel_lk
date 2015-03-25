/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.test;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.text.StaticLayout;

import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ViewImpl;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;

/**
 * This class provide some useful reflecting methods for test case
 */
public class Utils {
    /**
     * Sleep for that: ActivityInstrumentationTestCase2.tearDown() function call
     * Activity.finish(), but it is async, the next testcase call getActivity()
     * may get last activity instance which will be destoryed.
     */
    public static int TEAR_DOWN_SLEEP_TIME = 1000;
    private static final String TAG = "Utils";
    public static final Uri SMS_CONTENT_URI = Uri.parse("content://sms");

    private Utils(){}
    /**
     * Get a private/protected field from a declared class
     * @param clazz The class where you need to get private/protected field
     * @param filedName The name of this field
     * @return The field required
     * @throws NoSuchFieldException
     */
    public static Field getPrivateField(Class clazz, String filedName) throws NoSuchFieldException {
        Field field = clazz.getDeclaredField(filedName);
        field.setAccessible(true);
        return field;
    }

    /**
     * Get a private/protected method from a declared class
     * @param clazz The class where you need to get private/protected method
     * @param methodName The name of this method
     * @return The field method
     * @throws NoSuchMethodException
     */
    public static Method getPrivateMethod(Class clazz, String methodName, Class<?>... parameterTypes)
            throws NoSuchMethodException {
        Method method = clazz.getDeclaredMethod(methodName, parameterTypes);
        method.setAccessible(true);
        return method;
    }

    /**
     * Get a private/protected constructor from a declared class
     * @param clazz The class where you need to get private/protected constructor
     * @param parameterTypes
     * @return The constructor
     * @throws NoSuchMethodException
     */
    public static Constructor getPrivateConstructor(Class clazz, Class<?>... parameterTypes) throws NoSuchMethodException {
        Constructor constructor = clazz.getDeclaredConstructor(parameterTypes);
        constructor.setAccessible(true);
        return constructor;
    }

    /**
     * Get a private class from specific class
     * @param clazz 
     * @param className The name of the private class
     * @return The target class
     */
    public static Class getPrivateClass(Class clazz, String className) {
        Class targetClass = null;
        Class[] classes = clazz.getDeclaredClasses();
        for (Class eachClass : classes) {
            if (className.equals(eachClass.getSimpleName())) {
                targetClass = eachClass;
            }
        }
        return targetClass;
    }

    /**
     * Call this method to clear any data in ModelImpl and ViewImpl
     * @throws NoSuchFieldException 
     * @throws IllegalAccessException 
     * @throws IllegalArgumentException 
     */
    public static void clearAllStatus() throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        try {
            Constructor<ModelImpl> constructor = ModelImpl.class.getConstructor();
            constructor.setAccessible(true);
            ModelImpl modelInstance = (ModelImpl) constructor.newInstance();
            Field instanceField = getPrivateField(ModelImpl.class, "INSTANCE");
            instanceField.set(null, modelInstance);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        Field instanceField = getPrivateField(ViewImpl.class, "sInstance");
        instanceField.set(null, null);
        instanceField = getPrivateField(ControllerImpl.class, "sControllerImpl");
        instanceField.set(null, null);
    }

    /**
     * @param context A context
     * @return A picture path
     */
    public static String getAPictureFilePath(Context context) {
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        return filePath;
    }

    /**
     * Get a image file path from database
     */
    public static String getFilePath(Context context, Uri mediaUri) {
        Logger.v(TAG, "getFilePath()");
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(mediaUri, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor.getColumnIndex(MediaStore.MediaColumns.DATA));
            } else {
                Logger.v(TAG, "getFilePath(), curosr is null!");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.v(TAG, "getFilePath() out, filePath is " + filePath);
        return filePath;
    }

}
