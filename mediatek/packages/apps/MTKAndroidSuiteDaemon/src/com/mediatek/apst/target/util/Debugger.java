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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.apst.target.util;

import com.mediatek.xlog.Xlog;

/**
 * Class Name: Debugger
 * <p>
 * Package: com.mediatek.apst.target.util
 * <p>
 * Created on: 2010-6-17
 * <p>
 * <p>
 * Description:
 * <p>
 * A helper class for Android PC Sync Tool debugging.
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public abstract class Debugger {
    // ==============================================================
    // Constants
    // ==============================================================

    public static final String LOG_TAG = "APST";

    private static final boolean ENABLE_LOG = true;
    private static final boolean ENABLE_E = true;
    private static final boolean ENABLE_W = true;
    private static final boolean ENABLE_I = true;
    private static final boolean ENABLE_D = true;
    private static final boolean ENABLE_V = true;

    // ==============================================================
    // Fields
    // ==============================================================

    private static long sStartTime;
    private static long sStopTime;

    public static void logStartTime(final String msg) {
        if (ENABLE_LOG && ENABLE_D) {
            sStartTime = System.currentTimeMillis();
            Xlog.d(LOG_TAG, "<" + msg + "> start: " + sStartTime);
        }
    }

    public static void logStopTime(final String msg) {
        if (ENABLE_LOG && ENABLE_D) {
            sStopTime = System.currentTimeMillis();
            Xlog.d(LOG_TAG, "<" + msg + "> stop: " + sStopTime);
            Xlog.d(LOG_TAG, "<" + msg + "> cost: " + (sStopTime - sStartTime)
                    + "ms");
        }
    }

    public static void logE(final String className, final String methodName, final Object[] args,
            final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(className, methodName, args, msg), tr);
        }
    }

    public static void logE(final Object[] args, final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(args, msg), tr);
        }
    }

    public static void logE(final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(msg), tr);
        }
    }

    public static void logE(final Throwable tr) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(null), tr);
        }
    }

    public static void logE(final String className, final String methodName, final Object[] args,
            final String msg) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(className, methodName, args, msg));
        }
    }

    public static void logE(final Object[] args, final String msg) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(args, msg));
        }
    }

    public static void logE(final String msg) {
        if (ENABLE_LOG && ENABLE_E) {
            Xlog.e(LOG_TAG, mkLog(msg));
        }
    }

    public static void logW(final String className, final String methodName, final Object[] args,
            final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(className, methodName, args, msg), tr);
        }
    }

    public static void logW(final Object[] args, final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(args, msg), tr);
        }
    }

    public static void logW(final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(msg), tr);
        }
    }

    public static void logW(final Throwable tr) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(null), tr);
        }
    }

    public static void logW(final String className, final String methodName, final Object[] args,
            final String msg) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(className, methodName, args, msg));
        }
    }

    public static void logW(final Object[] args, final String msg) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(args, msg));
        }
    }

    public static void logW(final String msg) {
        if (ENABLE_LOG && ENABLE_W) {
            Xlog.w(LOG_TAG, mkLog(msg));
        }
    }

    public static void logI(final String className, final String methodName, final Object[] args,
            final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(className, methodName, args, msg), tr);
        }
    }

    public static void logI(final Object[] args, final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(args, msg), tr);
        }
    }

    public static void logI(final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(msg), tr);
        }
    }

    public static void logI(final Throwable tr) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(null), tr);
        }
    }

    public static void logI(final String className, final String methodName, final Object[] args,
            final String msg) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(className, methodName, args, msg));
        }
    }

    public static void logI(final Object[] args, final String msg) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(args, msg));
        }
    }

    public static void logI(final String msg) {
        if (ENABLE_LOG && ENABLE_I) {
            Xlog.i(LOG_TAG, mkLog(msg));
        }
    }

    public static void logD(final String className, final String methodName, final Object[] args,
            final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(className, methodName, args, msg), tr);
        }
    }

    public static void logD(final Object[] args, final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(args, msg), tr);
        }
    }

    public static void logD(final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(msg), tr);
        }
    }

    public static void logD(final Throwable tr) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(null), tr);
        }
    }

    public static void logD(final String className, final String methodName, final Object[] args,
            final String msg) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(className, methodName, args, msg));
        }
    }

    public static void logD(final Object[] args, final String msg) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(args, msg));
        }
    }

    public static void logD(final String msg) {
        if (ENABLE_LOG && ENABLE_D) {
            Xlog.d(LOG_TAG, mkLog(msg));
        }
    }

    public static void logV(final String className, final String methodName, final Object[] args,
            final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(className, methodName, args, msg), tr);
        }
    }

    public static void logV(final Object[] args, final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(args, msg), tr);
        }
    }

    public static void logV(final String msg, final Throwable tr) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(msg), tr);
        }
    }

    public static void logV(final Throwable tr) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(null), tr);
        }
    }

    public static void logV(final String className, final String methodName, final Object[] args,
            final String msg) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(className, methodName, args, msg));
        }
    }

    public static void logV(final Object[] args, final String msg) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(args, msg));
        }
    }

    public static void logV(final String msg) {
        if (ENABLE_LOG && ENABLE_V) {
            Xlog.v(LOG_TAG, mkLog(msg));
        }
    }

    protected static String mkLog(final String className, final String methodName,
            final Object[] args, final String msg) {
        final StringBuffer strLog = new StringBuffer();
        if (null != className) {
            strLog.append('[').append(className).append(']').append(' ');
        }
        if (null != methodName) {
            strLog.append(methodName);
            if (null != args) {
                strLog.append('(');
                for (int i = 0; i < args.length; i++) {
                    strLog.append(args[i]);
                    if (i != args.length - 1) {
                        strLog.append(',').append(' ');
                    }
                }
                strLog.append(')');
            }
            strLog.append(':').append(' ');
        }
        if (null != msg) {
            strLog.append(msg);
        }

        return strLog.toString();
    }

    protected static String mkLog(final Object[] args, final String msg) {
        final StackTraceElement stack[] = new Throwable().getStackTrace();
        // First, search back to a method in the "Debugger" class.
        int ix = 0;
        while (ix < stack.length) {
            if (stack[ix].getClassName().equals(
                    "com.mediatek.apst.target.util.Debugger")) {
                break;
            }
            ix++;
        }
        // Now search for the first frame before the "Debugger" class.
        while (ix < stack.length) {
            final StackTraceElement frame = stack[ix];
            final String className = frame.getClassName();
            if (!className.equals("com.mediatek.apst.target.util.Debugger")) {
                // We've found the relevant frame.
                final int lastSegment = className.lastIndexOf('.');
                return mkLog(className.substring(lastSegment + 1), frame
                        .getMethodName(), args, msg);
            }
            ix++;
        }
        return "Debugger get stack trace error!";
    }

    protected static String mkLog(final String msg) {
        return mkLog(null, msg);
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================

}
