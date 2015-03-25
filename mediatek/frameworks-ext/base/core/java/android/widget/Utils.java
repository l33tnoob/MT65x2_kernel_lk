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

package android.widget;

import android.appwidget.AppWidgetHostView;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.ImageView;

public class Utils {

    private static final String TAG = "Utils";

    // private static final ITelephony mITelephony =
    // ITelephony.Stub.asInterface(ServiceManager.checkService("phone"));

    /**
     * Kick off an intent to initiate a call.
     */
    public static void initiateCall(Context context, CharSequence phoneNumber) {
        Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, Uri.fromParts("tel", phoneNumber
                .toString(), null));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // new add
        context.startActivity(intent);
    }

    /**
     * Kick off an intent to initiate an Sms/Mms message.
     */
    public static void initiateSms(Context context, CharSequence phoneNumber) {
        Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.fromParts("sms", phoneNumber
                .toString(), null));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // new add
        context.startActivity(intent);
    }

    /*
     * @param context
     * @param phone the phone number.
     * @param sim in the dual SIM system sim == 0 or sim == 1, in the single SIM
     * system sim == -1
     */
    public static void callPassSim(Context context, String phone, int sim) {
        /*
         * if((mITelephony != null) && ((sim == 0) || (sim == 1))){ try{
         * mITelephony.dial(phone); }catch(Exception e){
         * Xlog.e("MAppWidgetProvider", "callPassSm Error"); } }
         */
        Uri uri = Uri.fromParts("tel", phone, null);
       // Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, uri);
        Intent intent = new Intent(Intent.ACTION_DIAL, uri);
       /* if (sim != -1) {
            if (sim == 0) {
                intent.putExtra(Phone.GEMINI_SIM_ID_KEY, Phone.GEMINI_SIM_1);
            } else if (sim == 1) {
                intent.putExtra(Phone.GEMINI_SIM_ID_KEY, Phone.GEMINI_SIM_2);
            }
        }*/
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
    static Handler mClearBitmapHandle;

    public static void onDetachedFromWindowClearUp(View view) {
        if (mClearBitmapHandle == null) {
            mClearBitmapHandle = new Handler() {
                public void handleMessage(Message message) {
                    Xlog.d(TAG, "message = " + message);
                    if (message.obj instanceof View) {
                        View onDetachedView = (View) message.obj;
                        if (!hasInLancher(onDetachedView)) {
                            ViewGroup viewParent = getWidgetHostView(onDetachedView);
                            clearView(viewParent);
                            Xlog.d(TAG, "clearView imageView");
                        }else{
                            Xlog.d(TAG, "no Remove imageView");
                        }
                    }
                    //message.recycle();
                }
            };
        }
        mClearBitmapHandle.sendMessageDelayed(mClearBitmapHandle.obtainMessage(
                0, view), 100);
    }
    
    public static boolean hasInLancher(View view){
        if (view == null){
            return false;
        }
        ViewParent viewParent = view.getParent(); 
        if (viewParent == null || !(viewParent instanceof View)){
            return false;
        }
        if (viewParent.getClass().toString().indexOf("Workspace") >= 0){
            return true;
        }
        return hasInLancher((View)viewParent); 
    }
    
    public static void clearView(View view){
    	if (view == null){
    		return;
    	}
//    	view.setBackgroundDrawable(null);
    	if (view instanceof ImageView){
//    		((ImageView)view).setImageDrawable(null);
    		return;
    	}
    	if (view instanceof ViewGroup){
    		ViewGroup group = (ViewGroup)view;
    		for (int i = 0; i < group.getChildCount(); i++){
    			clearView(group.getChildAt(i));
    		}
    	}
    }
    
    public static ViewGroup getWidgetHostView(View view){
    	if (view == null || view.getParent() == null){
    		return null;
    	}
    	ViewParent viewParent = view.getParent(); 
    	if (viewParent == null || !(viewParent instanceof View)){
    		return null;
    	}
    	if (viewParent instanceof AppWidgetHostView){
    		return (ViewGroup)view;
    	}
    	return getWidgetHostView((View)viewParent); 
    }
    
    

}
