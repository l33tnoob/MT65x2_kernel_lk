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

/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: NvRAMAgent.aidl
 */

package com.mediatek.hdmi;

import android.os.IBinder;

public interface NvRAMAgent extends android.os.IInterface {   
    /** Local-side IPC implementation stub class. */
    public abstract static class Stub extends android.os.Binder implements NvRAMAgent
    {
        private static final java.lang.String DESCRIPTOR = "NvRAMAgent";
        /** Construct the stub at attach it to the interface. */
        public Stub() {
            this.attachInterface(this, DESCRIPTOR);
        }
        /**
         * Cast an IBinder object into an NvRAMAgent interface,
         * generating a proxy if needed.
         */
        public static NvRAMAgent asInterface(android.os.IBinder obj) {
            if ((obj == null)) {
                return null;
            }
            android.os.IInterface iin = (android.os.IInterface)obj.queryLocalInterface(DESCRIPTOR);
            if (((iin != null) && (iin instanceof NvRAMAgent))) {
                return ((NvRAMAgent)iin);
            }
            return new NvRAMAgent.Stub.Proxy(obj);
        }
        public android.os.IBinder asBinder() {
            return this;
        }
        public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags)
                throws android.os.RemoteException {
            switch (code) {
                case INTERFACE_TRANSACTION:
                    reply.writeString(DESCRIPTOR);
                    return true;
                case TRANSACTION_READFILE:
                    data.enforceInterface(DESCRIPTOR);
                    int myArg0;
                    myArg0 = data.readInt();
                    byte[] myResult = this.readFile(myArg0);
                    reply.writeNoException();
                    reply.writeByteArray(myResult);
                    return true;
                case TRANSACTION_WRITEFILE:
                    data.enforceInterface(DESCRIPTOR);
                    int myArg01;
                    myArg01 = data.readInt();
                    byte[] arg1;
                    arg1 = data.createByteArray();
                    int myResult2 = this.writeFile(myArg01, arg1);
                    reply.writeNoException();
                    reply.writeInt(myResult2);
                    return true;
                default:
                    break;
            }
            return super.onTransact(code, data, reply, flags);
        }
        private static class Proxy implements NvRAMAgent {
            private android.os.IBinder mRemote;
            Proxy(android.os.IBinder remote) {
                mRemote = remote;
            }
            public android.os.IBinder asBinder() {
                return mRemote;
            }
            public java.lang.String getInterfaceDescriptor() {
                return DESCRIPTOR;
            }
            public byte[] readFile(int fileLid) throws android.os.RemoteException
            {
                android.os.Parcel data = android.os.Parcel.obtain();
                android.os.Parcel reply = android.os.Parcel.obtain();
                byte[] result;
                try {
                    data.writeInterfaceToken(DESCRIPTOR);
                    data.writeInt(fileLid);
                    mRemote.transact(Stub.TRANSACTION_READFILE, data, reply, 0);
                    reply.readException();
                    result = reply.createByteArray();
                } finally {
                    reply.recycle();
                    data.recycle();
                }
                return result;
            }
            public int writeFile(int fileLid, byte[] buff) throws android.os.RemoteException
            {
                android.os.Parcel data = android.os.Parcel.obtain();
                android.os.Parcel reply = android.os.Parcel.obtain();
                int result;
                try {
                    data.writeInterfaceToken(DESCRIPTOR);
                    data.writeInt(fileLid);
                    data.writeByteArray(buff);
                    mRemote.transact(Stub.TRANSACTION_WRITEFILE, data, reply, 0);
                    reply.readException();
                    result = reply.readInt();
                } finally {
                    reply.recycle();
                    data.recycle();
                }
                return result;
            }
        }
        static final int TRANSACTION_READFILE = (IBinder.FIRST_CALL_TRANSACTION + 0);
        static final int TRANSACTION_WRITEFILE = (IBinder.FIRST_CALL_TRANSACTION + 1);
    }
    byte[] readFile(int fileLid) throws android.os.RemoteException;
    int writeFile(int fileLid, byte[] buff) throws android.os.RemoteException;
}
