/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.common;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.net.Uri;
import android.content.ContentResolver;
import android.content.Context;
import android.content.pm.Signature;
import android.graphics.Bitmap;
import android.os.SystemClock;
import android.util.Log;

/* for importing the factory tables */
import com.mediatek.common.CommonInterface;
import com.mediatek.common.OperatorInterface;
import com.mediatek.common.TabletInterface;


import com.mediatek.common.pluginmanager.IPluginManager;
import com.mediatek.common.util.IPatterns;
import com.mediatek.common.util.IWebProtocolNames;

public final class MediatekClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekClassFactory";
    private static final String mOPFactoryName = "com.mediatek.op.MediatekOPClassFactory";
    private static Method mOpGetIfClassName = null;
    private static boolean mOpChecked = false;
    private static final String mTBFactoryName = "com.mediatek.tb.MediatekTBClassFactory";
    private static Method mTbGetIfClassName = null;
    private static boolean mTbChecked = false;

    /**
     * Use factory method design pattern.
     * 
     * @throws Exception
     */
    public static <T> T createInstance(Class<?> clazz, Object... args) {

        if (DEBUG_PERFORMANCE) {
            Log.d(TAG, "createInstance(): Begin = "
                    + SystemClock.uptimeMillis());
        }

        String ifClassName = null;
        Object obj = null;

        if (DEBUG_GETINSTANCE) {
            Log.d(TAG, "create Instance with :  " + clazz);
        }

        if (CommonInterface.getContainsKey(clazz)) {

            ifClassName = CommonInterface.getClass(clazz);

            if (DEBUG_GETINSTANCE) {
                Log.d(TAG,
                        "create Instance from mediatek-framework library :  "
                                + ifClassName);
            }

            obj = getInstanceHelper(ifClassName, args);
        } else if (OperatorInterface.getContainsKey(clazz)) {

            ifClassName = getOpIfClassName(clazz);

            if (DEBUG_GETINSTANCE) {
                Log.d(TAG, "create Instance from operator library :  "
                        + ifClassName);
            }
            obj = getInstanceHelper(ifClassName, args);
        }else if (TabletInterface.getContainsKey(clazz)) {
            ifClassName = getTbIfClassName(clazz);

            if (DEBUG_GETINSTANCE) {
                Log.d(TAG, "create Instance from tablet library :  "
                        + ifClassName);
            }

            obj = getInstanceHelper(ifClassName, args);        	
        } else if (clazz == IPluginManager.class) {
            Object pluginMgr = null;
            try {
                Class<?> clz = Class
                        .forName("com.mediatek.pluginmanager.PluginManager");
                if (clz != null && args.length > 0) {
                    Log.d(TAG, "PluginManager args length: " + args.length);
                    if (args[0].equals(IPluginManager.CREATE)) {
                        Log.d(TAG, "IPluginManager.CREATE");
                        /*
                         * we only allow partial API 1. create(Context context,
                         * String pluginIntent, String version) 2.
                         * create(Context context, String pluginIntent, String
                         * version, Signature signature)
                         */
                        if (args.length == 4) {
                            Method method = clz.getMethod(
                                    IPluginManager.CREATE, Context.class,
                                    String.class, String.class,
                                    Signature[].class);
                            if (method != null) {
                                pluginMgr = method.invoke(null, args[1],
                                        args[2], args[3], null);
                            }
                        } else if (args.length == 5) {
                            Method method = clz.getMethod(
                                    IPluginManager.CREATE, Context.class,
                                    String.class, String.class,
                                    Signature[].class);
                            if (method != null) {
                                pluginMgr = method.invoke(null, args[1],
                                        args[2], args[3], args[4]);
                            }
                        }
                    } else if (args[0]
                            .equals(IPluginManager.CREATE_PLUGIN_OBJECT)) {
                        Log.d(TAG, "IPluginManager.CREATE_PLUGIN_OBJECT");
                        /*
                         * we only allow partial API 1.
                         * createPluginObject(Context context, String
                         * pluginIntent, String version, String metaName) 2.
                         * createPluginObject(Context context, String
                         * pluginIntent, String version, String metaName,
                         * Signature signatures)
                         */
                        if (args.length == 5) {
                            Method method = clz.getMethod(
                                    IPluginManager.CREATE_PLUGIN_OBJECT,
                                    Context.class, String.class, String.class,
                                    String.class, Signature[].class);
                            if (method != null) {
                                pluginMgr = method.invoke(null, args[1],
                                        args[2], args[3], args[4], null);
                            }
                        } else if (args.length == 6) {
                            Method method = clz.getMethod(
                                    IPluginManager.CREATE_PLUGIN_OBJECT,
                                    Context.class, String.class, String.class,
                                    String.class, Signature[].class);
                            if (method != null) {
                                pluginMgr = method.invoke(null, args[1],
                                        args[2], args[3], args[4], args[5]);
                            }
                        }
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "createInstance:got exception for PluginManager");
                e.printStackTrace();
            }
            obj = pluginMgr;
        }else {
            Log.e(TAG, "Unsupported class: " + clazz);
            if (DEBUG_PERFORMANCE) {
                Log.d(TAG, "createInstance(): End = "
                        + SystemClock.uptimeMillis());
            }
        }

        if (DEBUG_PERFORMANCE) {
            Log.d(TAG, "createInstance(): End = " + SystemClock.uptimeMillis());
        }

        // Cannot return null object.
        if (obj == null) {
            Log.d(TAG, "null object during finding :  " + clazz);
            throw new RuntimeException();
        }
        return (T) obj;

    }

    public static Object getInstanceHelper(String className, Object[] args) {

        if (className == null) {
            Log.e(TAG, "Interface full class name is null");
            return null;
        }

        try {
            Class<?> clz = Class.forName(className);

            if (args.length == 0) {
                // Default constructor.
                return clz.getConstructor().newInstance();
            }

            // More than one parameters. Look for compatible constructor to the
            // input arguments.
            Constructor<?> ctorList[] = clz.getConstructors();
            for (int i = 0; i < ctorList.length; i++) {
                boolean matched = true;
                Constructor<?> ct = ctorList[i];
                Class<?> paramTypes[] = ct.getParameterTypes();
                if (paramTypes.length != args.length) {
                    continue;
                }

                for (int j = 0; j < paramTypes.length; j++) {
                    Class paramType = paramTypes[j];
                    Class actualType = args[j].getClass();

                    Log.d(TAG, "getInstanceHelper: paramType=" + paramType
                            + ", actualType=" + actualType);

                    if (!paramType.isAssignableFrom(actualType)
                            && !(paramType.isPrimitive() && primitiveMap.get(
                                    paramType).equals(actualType))) {
                        Log.d(TAG, "Parameter not matched, skip");
                        matched = false;
                        break;
                    }

                    Log.d(TAG, "Parameter matched");
                }

                // All parameter matched. Create the instance from the
                // constructor.
                if (matched) {
                    Log.d(TAG, "Constructor matched");
                    return ct.newInstance(args);
                }
            }
        } catch (Exception e) {
            Log.w(TAG, "Exception: " + e.getClass().getName());
        }

        return null;
    }

    private static String getOpIfClassName(Class<?> clazz) {
        String ifClassName = null;

        // the mOpGetIfClassName will be cached
        if (mOpGetIfClassName == null) {
            // try to find the OpCreateInst and cache it
            // we only need the getInstance method, don't need to check the
            // number of argument
            try {
                Class<?> clz = Class.forName(mOPFactoryName);
                mOpGetIfClassName = clz.getMethod("getOpIfClassName",
                        Class.class);
            } catch (ClassNotFoundException e) {
                Log.w(TAG, "OP not exist!, Get obj from default class");
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "Not Such Method Exception: "
                        + e.getClass().getName());
            }
        }

        // get the class name from operator's class factory
        if (mOpGetIfClassName != null) {
            try {
                ifClassName = (String) mOpGetIfClassName.invoke(null, clazz);
            } catch (IllegalAccessException e) {
                Log.w(TAG, "IllegalAccessException Exception: "
                        + e.getClass().getName());
            } catch (InvocationTargetException e) {
                Log.w(TAG, "InvocationTargetException Exception: "
                        + e.getClass().getName());
            }
        }

        if (ifClassName == null) {
            // give the default class name
            ifClassName = OperatorInterface.getClass(clazz);
        }

        return ifClassName;
    }

    private static String getTbIfClassName(Class<?> clazz) {
        String ifClassName = null;

        // the mOpGetIfClassName will be cached
        if (mTbGetIfClassName == null) {
            // try to find the OpCreateInst and cache it
            // we only need the getInstance method, don't need to check the
            // number of argument
            try {
                Class<?> clz = Class.forName(mTBFactoryName);
                mTbGetIfClassName = clz.getMethod("getTbIfClassName",
                        Class.class);
            } catch (ClassNotFoundException e) {
                Log.w(TAG, "Tablet not exist!, Get obj from default class");
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "Not Such Method Exception: "
                        + e.getClass().getName());
            }
        }

        // get the class name from operator's class factory
        if (mTbGetIfClassName != null) {
            try {
                ifClassName = (String) mTbGetIfClassName.invoke(null, clazz);
            } catch (IllegalAccessException e) {
                Log.w(TAG, "IllegalAccessException Exception: "
                        + e.getClass().getName());
            } catch (InvocationTargetException e) {
                Log.w(TAG, "InvocationTargetException Exception: "
                        + e.getClass().getName());
            }
        }

        if (ifClassName == null) {
            // give the default class name
            ifClassName = TabletInterface.getClass(clazz);
        }

        return ifClassName;
    }

    // Primitive type map used for parameter type matching.
    private static Map<Class, Class> primitiveMap = new HashMap<Class, Class>();
    static {
        primitiveMap.put(boolean.class, Boolean.class);
        primitiveMap.put(byte.class, Byte.class);
        primitiveMap.put(char.class, Character.class);
        primitiveMap.put(short.class, Short.class);
        primitiveMap.put(int.class, Integer.class);
        primitiveMap.put(long.class, Long.class);
        primitiveMap.put(float.class, Float.class);
        primitiveMap.put(double.class, Double.class);
    }
}
