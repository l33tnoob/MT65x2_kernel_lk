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

package com.mediatek.datatransfer.modules;

import android.accounts.Account;
import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.Context;
import android.content.OperationApplicationException;
import android.net.Uri;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.ContactsContract;
import com.android.vcard.VCardConfig;
import com.android.vcard.VCardEntry;
import com.android.vcard.VCardEntryCommitter;
import com.android.vcard.VCardEntryConstructor;
import com.android.vcard.VCardEntryHandler;
import com.android.vcard.VCardInterpreter;
import com.android.vcard.VCardParser;
import com.android.vcard.VCardParser_V21;
import com.android.vcard.VCardParser_V30;
import com.android.vcard.exception.VCardException;
import com.android.vcard.exception.VCardNestedException;
import com.android.vcard.exception.VCardNotSupportedException;
import com.android.vcard.exception.VCardVersionException;
import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.String;
import java.util.ArrayList;

public class ContactRestoreComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreService";
    private int mIndex;
    private int mCount;
    private InputStream mInputStream;
    public ContactRestoreComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_CONTACT;
    }

    public int getCount() {
        MyLogger.logD(CLASS_TAG, "getCount():" + mCount);
        return mCount;
    }

    public boolean init() {
        boolean result = false;
        MyLogger.logD(CLASS_TAG, "begin init:" + System.currentTimeMillis());
        try {
            mCount = getContactCount();
            result = true;
        } catch (Exception e) {
        }

        MyLogger.logD(CLASS_TAG, "end init:" + System.currentTimeMillis());
        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + mCount);
        return result;
    }

    public boolean isAfterLast() {
        boolean result = (mIndex >= mCount) ? true : false;
        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    public boolean composeOneEntity() {
        return implementComposeOneEntity();
    }

    public static boolean isTablet() {
        String characteristics = SystemProperties.get("ro.build.characteristics");
        MyLogger.logD(CLASS_TAG, "[isTablet]  characteristics = " + characteristics);
        if (characteristics != null && characteristics.toLowerCase().equals("tablet")) {
            return true;
        }

        return false;
    }

    public boolean implementComposeOneEntity() {
        boolean result = false;

        ++mIndex;
        if (mIndex == 1) {
            if (mInputStream != null) {
                // Account account = new Account("Phone",
                // AccountType.ACCOUNT_TYPE_LOCAL_PHONE);
                //Account account = new Account("Phone", "Local Phone Account");
                Account account = isTablet()? new Account("Tablet", "Local Phone Account"):new Account("Phone", "Local Phone Account");
                final VCardEntryConstructor constructor = new VCardEntryConstructor(VCardConfig.VCARD_TYPE_V21_GENERIC,
                                                                                    account);
                final RestoreVCardEntryCommitter committer = new RestoreVCardEntryCommitter(mContext.getContentResolver());
                constructor.addEntryHandler(committer);
                final int[] possibleVCardVersions = new int[] { VCardConfig.VCARD_TYPE_V21_GENERIC,
                                                                VCardConfig.VCARD_TYPE_V30_GENERIC };
                result = readOneVCard(mInputStream, VCardConfig.VCARD_TYPE_V21_GENERIC, constructor, possibleVCardVersions);
            }
        } else {
            result = true;
        }

        MyLogger.logD(CLASS_TAG, "implementComposeOneEntity()" + ",result:" + result);
        return result;
    }

    private boolean deleteAllContact() {
        if (mContext != null) {
            MyLogger.logD(CLASS_TAG, "begin delete:" + System.currentTimeMillis());

            int count = mContext.getContentResolver().delete(
                    Uri.parse(ContactsContract.RawContacts.CONTENT_URI.toString() + "?"
                            + ContactsContract.CALLER_IS_SYNCADAPTER + "=true"),
                    ContactsContract.RawContacts._ID + ">0", null);

            MyLogger.logD(CLASS_TAG, "end delete:" + System.currentTimeMillis());

            MyLogger.logD(CLASS_TAG, "deleteAllContact()," + count + " records deleted!");

            return true;
        }

        return false;
    }

    private boolean readOneVCard(InputStream is, int vcardType, final VCardInterpreter interpreter,
                                 final int[] possibleVCardVersions) {
        boolean successful = false;
        final int length = possibleVCardVersions.length;
        VCardParser vcardParser;

        for (int i = 0; i < length; i++) {
            final int vcardVersion = possibleVCardVersions[i];
            try {
                if (i > 0 && (interpreter instanceof VCardEntryConstructor)) {
                    // Let the object clean up internal temporary objects,
                    ((VCardEntryConstructor) interpreter).clear();
                }

                // We need synchronized block here,
                // since we need to handle mCanceled and mVCardParser at once.
                // In the worst case, a user may call cancel() just before
                // creating
                // mVCardParser.
                synchronized (this) {
                    vcardParser = (vcardVersion == VCardConfig.VCARD_TYPE_V21_GENERIC) ? new VCardParser_V21(
                            vcardType) : new VCardParser_V30(vcardType);
                }

                vcardParser.parse(is, interpreter);
                successful = true;
                break;
            } catch (IOException e) {
                e.printStackTrace();
            } catch (VCardNestedException e) {
                e.printStackTrace();
            } catch (VCardNotSupportedException e) {
                e.printStackTrace();
            } catch (VCardVersionException e) {
                e.printStackTrace();
            } catch (VCardException e) {
                e.printStackTrace();
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                    }
                }
            }
        }

        MyLogger.logD(CLASS_TAG, "readOneVCard() " + successful);
        return successful;
    }

    public void onStart() {
        super.onStart();
        // deleteAllContact();
        try {
            String fileName = mParentFolderPath + File.separator +
                ModulePath.FOLDER_CONTACT + File.separator + ModulePath.NAME_CONTACT;
            mInputStream =new FileInputStream(fileName);
        } catch(Exception e) {
            mInputStream = null;
        }

        MyLogger.logD(CLASS_TAG, " onStart()");
    }

    public void onEnd() {
        super.onEnd();
        if(mInputStream != null) {
            try {
                mInputStream.close();
            } catch(IOException e) {
            } catch(Exception e) {
            }
        }

        MyLogger.logD(CLASS_TAG, " onEnd()");
    }

    private int getContactCount() {
        int count = 0;
        try {
            String fileName = mParentFolderPath + File.separator +
                ModulePath.FOLDER_CONTACT + File.separator + ModulePath.NAME_CONTACT;
            InputStream instream = new FileInputStream(fileName);
            InputStreamReader inreader = new InputStreamReader(instream);
            BufferedReader buffreader = new BufferedReader(inreader);
            String line = null;
            while ((line = buffreader.readLine()) != null) {
                if (line.contains("END:VCARD")) {
                    ++count;
                }
            }
            instream.close();
        } catch(IOException e) {
        } catch(Exception e) {
        }

        return count;
    }

    /**
     * Describe class <code>RestoreVCardEntryCommitter</code> here.
     *
     */
    private class RestoreVCardEntryCommitter extends VCardEntryCommitter {

        /**
         * Creates a new <code>RestoreVCardEntryCommitter</code> instance.
         *
         * @param resolver a <code>ContentResolver</code> value
         */
        public RestoreVCardEntryCommitter(ContentResolver resolver) {
            super(resolver);
        }

        /**
         * Describe <code>onEntryCreated</code> method here.
         *
         * @param vcardEntry a <code>VCardEntry</code> value
         */
        public void onEntryCreated(final VCardEntry vcardEntry) {
            super.onEntryCreated(vcardEntry);
            increaseComposed(true);
        }
    }

    // private class RestoreVCardEntryCommitter implements VCardEntryHandler {
    //     private final ContentResolver mContentResolver;
    //     // private long mTimeToCommit;
    //     // private int mCounter;
    //     private ArrayList<ContentProviderOperation> mOperationList;
    //     private final ArrayList<Uri> mCreatedUris = new ArrayList<Uri>();

    //     public RestoreVCardEntryCommitter(ContentResolver resolver) {
    //         mContentResolver = resolver;
    //     }

    //     @Override
    //     public void onStart() {
    //     }

    //     @Override
    //     public void onEnd() {
    //         if (mOperationList != null) {
    //             mCreatedUris.add(pushIntoContentResolver(mOperationList));
    //         }
    //     }

    //     @Override
    //     public void onEntryCreated(final VCardEntry vcardEntry) {
    //         // final long start = System.currentTimeMillis();
    //         mOperationList = vcardEntry.constructInsertOperations(mContentResolver, mOperationList);
    //         if (mOperationList != null
    //                 && mOperationList.size() >= Constants.NUMBER_IMPORT_CONTACTS_EACH) {
    //             mCreatedUris.add(pushIntoContentResolver(mOperationList));
    //             mOperationList = null;
    //         }
    //         // mTimeToCommit += System.currentTimeMillis() - start;
    //         increaseComposed(true);
    //     }

    //     private Uri pushIntoContentResolver(ArrayList<ContentProviderOperation> operationList) {
    //         try {
    //             final ContentProviderResult[] results = mContentResolver.applyBatch(
    //                     ContactsContract.AUTHORITY, operationList);

    //             // the first result is always the raw_contact. return it's uri
    //             // so that it can be found later. do null checking for badly
    //             // behaving
    //             // ContentResolvers
    //             return ((results == null || results.length == 0 || results[0] == null) ? null
    //                     : results[0].uri);
    //         } catch (RemoteException e) {
    //             e.printStackTrace();
    //             return null;
    //         } catch (OperationApplicationException e) {
    //             e.printStackTrace();
    //             return null;
    //         }
    //     }

    //     /**
    //      * Returns the list of created Uris. This list should not be modified by
    //      * the caller as it is not a clone.
    //      */
    //     public ArrayList<Uri> getCreatedUris() {
    //         return mCreatedUris;
    //     }
    // }

}
