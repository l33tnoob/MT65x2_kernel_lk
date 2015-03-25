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

package com.mediatek.datatransfer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.Constants.LogTag;

public class ResultDialog {
    
    public static final int RESULT_TYPE_BACKUP = 1;
    public static final int RESULT_TYPE_RESTRORE = 2;

    public static AlertDialog createResultDlg(final Context context, final int titleId,
                                final Bundle args, final DialogInterface.OnClickListener listener) {
        AlertDialog dialog = new AlertDialog.Builder(context).setCancelable(false)
                .setTitle(titleId).setPositiveButton(android.R.string.ok, listener)
                .setAdapter(createEmptyAdapter(context), null)
                .create();

        return dialog;
    }
    
    private static ArrayAdapter<String> createEmptyAdapter(final Context context){
        
        return new ArrayAdapter<String>(context, 0, new String[]{});
    }

    public static ResultDialogAdapter createResultAdapter(final Context context, final Bundle args) {
        List<Map<String, Object>> dataMapList = new ArrayList<Map<String, Object>>();
        List<ResultEntity> list = args.getParcelableArrayList(Constants.RESULT_KEY);
        if(list == null){
            Log.e(LogTag.LOG_TAG, "ResultDialogAdapter: error: list is null");
            return null;
        }
        for (ResultEntity item : list) {
            Map<String, Object> map = new HashMap<String, Object>();
            map.put(Constants.ITEM_NAME, ModuleType.getModuleStringFromType(context, item.mType));
            int resId = R.string.result_success;
            switch (item.mResult) {
            case ResultEntity.NO_CONTENT:
                resId = R.string.result_no_content;
                break;
                
            case ResultEntity.FAIL:
                resId = R.string.result_fail;
                break;
                
            default:
                resId = R.string.result_success;
                break;
            }
            map.put(Constants.ITEM_RESULT, context.getString(resId));
            dataMapList.add(map);
        }
        String[] from = new String[] { Constants.ITEM_NAME, Constants.ITEM_RESULT };
        int[] to = new int[] { R.id.module_name, R.id.result };
        return new ResultDialogAdapter(context, dataMapList, R.layout.result_list_item, from, to);
    }
    
    public static ResultDialogAdapter createAppResultAdapter(final List<AppSnippet> appSnippetList, 
                                                         final Context context, final Bundle args, final int resultType) {
        List<Map<String, Object>> dataMapList = new ArrayList<Map<String, Object>>();
        List<ResultEntity> list = args.getParcelableArrayList("result");
        if(list == null){
            Log.e(LogTag.LOG_TAG, "createAppResultAdapter: error: list is null");
            return null;
        }
        for (ResultEntity item : list) {
            Map<String, Object> map = new HashMap<String, Object>();
            CharSequence name = null;
            for(AppSnippet appSnippet : appSnippetList){
                String key = null;
                if(RESULT_TYPE_BACKUP == resultType){
                    key = appSnippet.getPackageName();
                }else{
                    key = appSnippet.getFileName();
                }
                if(key.equalsIgnoreCase(item.mKey)){
                    name = appSnippet.getName();
                    break;
                }
            }
            if(name == null){
                name = context.getString(R.string.unknown);
            }
            map.put(Constants.ITEM_NAME, name);
            int resId = R.string.result_success;
            if(item.mResult == ResultEntity.FAIL) {
                resId = R.string.result_fail;
            }
            map.put(Constants.ITEM_RESULT, context.getString(resId));
            dataMapList.add(map);
        }
        String[] from = new String[] { Constants.ITEM_NAME, Constants.ITEM_RESULT };
        int[] to = new int[] { R.id.module_name, R.id.result };
        return new ResultDialogAdapter(context, dataMapList, R.layout.result_list_item, from, to);
    }
    
    

    public static class ResultEntity implements Parcelable {
        public static final int ENTITY_PERSONAL_DATA_ITEM = 0;
        public static final int ENTITY_APP_ITEM = 0;
        public static final int SUCCESS  = 0;
        public static final int FAIL = -1;
        public static final int NO_CONTENT = -2;
        
        private int mType;
        private String mKey;
        private int mResult;

        public ResultEntity(final int type, final int result) {
            mType = type;
            mResult = result;
        }
        
        public void setKey(String key){
            mKey = key;
        }
        

        private ResultEntity(Parcel in) {
            mType = in.readInt();
            mResult = in.readInt();
            mKey = in.readString();
        }

        public void setResult(int result) {
            this.mResult = result;
        }

        public int getResult() {
            return mResult;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(final Parcel dest, final int flags) {
            dest.writeInt(mType);
            dest.writeInt(mResult);
            dest.writeString(mKey);
        }

        public static final Parcelable.Creator<ResultEntity> CREATOR = new Parcelable.Creator<ResultEntity>() {

            @Override
            public ResultEntity createFromParcel(Parcel in) {
                return new ResultEntity(in);
            }

            @Override
            public ResultEntity[] newArray(int size) {
                return new ResultEntity[size];
            }
        };

    }

    private static class ResultDialogAdapter extends SimpleAdapter {
        private List<? extends Map<String, ?>> mDataMapList;
        private Context mContext;

        public ResultDialogAdapter(Context context, List<? extends Map<String, ?>> data,
                int resource, String[] from, int[] to) {
            super(context, data, resource, from, to);
            mDataMapList = data;
            mContext = context;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            View view = super.getView(position, convertView, parent);
            TextView result = (TextView) view.findViewById(R.id.result);
            if (mDataMapList.get(position).get(Constants.ITEM_RESULT)
                    .equals(mContext.getResources().getString(R.string.result_success))) {
                result.setTextColor(mContext.getResources().getColor(R.color.result_success));
            } else {
                result.setTextColor(mContext.getResources().getColor(R.color.result_fail));
            }
            return view;
        }
    }
}
