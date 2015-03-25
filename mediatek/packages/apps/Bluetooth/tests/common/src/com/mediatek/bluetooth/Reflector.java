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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.bluetooth;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Reflects all of the target contructors, fields and methods, include the private.
 * @author Xin Gang Sun
 *
 */
public class Reflector {

    private Reflector() {

        /* hide the constructor */
    }

    @SuppressWarnings("unchecked")
    public static Field getField(Object obj, String name) throws NoSuchFieldException {
        Class<?> cls = obj instanceof Class ? (Class<?>) obj : obj.getClass() ;
        while (cls != null) {
            try {
                Field field = cls.getDeclaredField(name);
                field.setAccessible(true);
                return field;
            } catch (NoSuchFieldException e) {
                cls = cls.getSuperclass();
            }
        }
        throw new NoSuchFieldException();
    }

    /* Getters */

    /**
     * Gets the field by reflection. If base type, use getXxx(Object, String) such as
     * {@link #getBoolean(Object, String)}, {@link #getInt(Object, String)}.
     * @param obj the object want to reflect. It must be the {@link Class} instance if static field.
     * @param name the field name.
     * @return the field instance.
     * @throws NoSuchFieldException if no such field.
     * @see #set(Object, String, Object)
     */
    public static Object get(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).get(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static boolean getBoolean(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getBoolean(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static byte getByte(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getByte(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static short getShort(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getShort(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static int getInt(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getInt(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static long getLong(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getLong(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static float getFloat(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getFloat(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static double getDouble(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getDouble(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static char getChar(Object obj, String name) throws NoSuchFieldException {
        try {
            return getField(obj, name).getChar(obj);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    /* Setters */

    /**
     * Sets the field by reflection. If base type, use the setXxx(Object, String, xxx) such as
     * {@link #setBoolean(Object, String, boolean)}, {@link #setInt(Object, String, int)}
     * @param obj the object want to reflect. It must be the {@link Class} instance if static field.
     * @param name the field name.
     * @param value the new field value.
     * @throws NoSuchFieldException if no such field.
     * @see #get(Object, String)
     */
    public static void set(Object obj, String name, Object value) throws NoSuchFieldException {
        try {
            getField(obj, name).set(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setBoolean(Object obj, String name, boolean value) throws NoSuchFieldException {
        try {
            getField(obj, name).setBoolean(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setByte(Object obj, String name, byte value) throws NoSuchFieldException {
        try {
            getField(obj, name).setByte(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setShort(Object obj, String name, short value) throws NoSuchFieldException {
        try {
            getField(obj, name).setShort(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setInt(Object obj, String name, int value) throws NoSuchFieldException {
        try {
            getField(obj, name).setInt(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setLong(Object obj, String name, long value) throws NoSuchFieldException {
        try {
            getField(obj, name).setLong(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setFloat(Object obj, String name, float value) throws NoSuchFieldException {
        try {
            getField(obj, name).setFloat(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setDouble(Object obj, String name, double value) throws NoSuchFieldException {
        try {
            getField(obj, name).setDouble(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    public static void setChar(Object obj, String name, char value) throws NoSuchFieldException {
        try {
            getField(obj, name).setChar(obj, value);
        } catch (IllegalAccessException e) {
            throw new IllegalAccessError();
        }
    }

    /* Inner class */

    /**
     * The {@link Class} {@link Object} mapping.
     */
    public static class Parameter {
        private final Class<?> mType;
        private final Object mParam;

        /**
         * Constructs a {@link Parameter} instance.
         * @param mType the object class instance.
         * @param mParam the object, may be null.
         */
        public Parameter(Class<?> paraType, Object param) {
            mType = paraType;
            mParam = param;
        }
    }

    /* Constructors */

    /**
     * News an instance by reflection.
     * @param <T> the class type.
     * @param cls the class instance.
     * @param params the class-object mappings.
     * It means the constructor define and the constructor params.
     * @throws NoSuchMethodException if no such method.
     * @throws Exception if error occars.
     * @see Class#getConstructor(Class...)
     * @see Constructor#newInstance(Object...)
     */
    public static <T> T newInstance(Class<T> cls, Parameter... params) throws Exception {
        if (params == null || params.length == 0) {
            try {
                Constructor<T> constructor = cls.getDeclaredConstructor();
                constructor.setAccessible(true);
                return constructor.newInstance();
            } catch (IllegalAccessException e) {
                throw new IllegalAccessError();
            } catch (InvocationTargetException e) {
                //throw e.getTargetException();
                e.printStackTrace();
                return null;
            }
        } else {
            Class<?>[] types = new Class<?>[params.length];
            Object[] args = new Object[params.length];
            for (int index = 0; index < params.length; ++index) {
                Parameter param = params[index];
                types[index] = param.mType;
                args[index] = param.mParam;
            }

            try {
                Constructor<T> constructor = cls.getDeclaredConstructor(types);
                constructor.setAccessible(true);
                return constructor.newInstance(args);
            } catch (IllegalAccessException e) {
                throw new IllegalAccessError();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
                return null;
                //throw e.getTargetException();
            }
        }
    }

    /* Methods */

    /**
     * Invokes a method by reflection.
     * @param obj the object want to reflect. It must be {@link Class} instance if static method.
     * @param name the method name.
     * @param params the class-object mappings.
     * It means the method define and the method params.
     * @return the method return, null if void.
     * @throws NoSuchMethodException if no such method.
     * @throws Exception if error occars.
     * @see Class#getDeclaredField(String)
     * @see Method#invoke(Object, Object...)
     */
    @SuppressWarnings("unchecked")
    public static Object invoke(Object obj, String name, Parameter... params) {
        Class<?> cls = obj instanceof Class ? (Class<?>) obj : obj.getClass() ;
        if (params == null || params.length == 0) {
            try {
                while (cls != null) {
                    try {
                        Method method = cls.getDeclaredMethod(name);
                        method.setAccessible(true);
                        return method.invoke(obj);
                    } catch (NoSuchMethodException e) {
                        cls = cls.getSuperclass();
                    }
                }
                throw new NoSuchMethodException();
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
                return null;
            } catch (IllegalAccessException e) {
                e.printStackTrace();
                return null;
                //throw new IllegalAccessError();
            } catch (InvocationTargetException e) {
                //throw e.getTargetException();
                e.printStackTrace();
                return null;
            }
        } else {
            Class<?>[] types = new Class<?>[params.length];
            Object[] args = new Object[params.length];
            for (int index = 0; index < params.length; ++index) {
                Parameter param = params[index];
                types[index] = param.mType;
                args[index] = param.mParam;
            }

            try {
                while (cls != null) {
                    try {
                        Method method = cls.getDeclaredMethod(name, types);
                        method.setAccessible(true);
                        return method.invoke(obj, args);
                    } catch (NoSuchMethodException e) {
                        cls = cls.getSuperclass();
                    }
                }
                throw new NoSuchMethodException();
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
                return null;
            }catch (IllegalAccessException e) {
                e.printStackTrace();
                return null;
                //throw new IllegalAccessError();
            } catch (InvocationTargetException e) {
                //throw e.getTargetException();
                e.printStackTrace();
                return null;
            }
        }
    }
}
