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

package com.mediatek.activity;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.widget.Toast;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 *
 * @param <ServiceInterface>
 *
 * help the implementation of Activity that needs to connect to Service via binder (ServiceInterface).
 *
 * 1. onCreate()    => bindService(this);
 * 2. onDestroy()    => unbindService(this);
 * 3. onResume()    => refreshUi(this);
 * 4. onCreateDialog()    => createBusyDialog( id, this );
 */
public class ServiceActivityHelper<ServiceInterface> {

    public static final int DIALOG_BUSY = -55158;

    // let Activity implements this interface and interact with Service
    public interface ServiceActivity<ServiceInterface> {

        public String getServiceAction();    // action to start the service
        public ServiceInterface asInterface( IBinder service );
        public void onServiceConnected();    // callback when service connected
        public void onServiceDisconnected();    // callback when service disconnected
        public void refreshActivityUi();    // update ui according updated properties
    }

    protected ServiceActivity<ServiceInterface> serviceActivity;
    public ServiceInterface service;
    protected Boolean isServiceAvailable = false;
    protected ProgressDialog busyDialog;

    /**
     * Constructor
     *
     * @param serviceActivity
     */
    public ServiceActivityHelper( ServiceActivity<ServiceInterface> serviceActivity ){

        this.serviceActivity = serviceActivity;
    }

    /**
     * acquire service lock: after acquiring lock, refreshUi() will show a busy dialog
     */
    public void acquireServiceLock(){

        synchronized( this.isServiceAvailable ){

            this.isServiceAvailable = false;
        }
    }

    /**
     * release service lock: after releasing lock, the activity will allow user interaction.
     */
    public void releaseServiceLock(){

        synchronized( this.isServiceAvailable ){

            this.isServiceAvailable = true;
        }
    }

    /**
     * should be called when activity's properties changed ( or at onResume() )
     *
     * @param activity
     */
    public void refreshUi( Activity activity ){

        synchronized( this.isServiceAvailable ){

            if( !this.isServiceAvailable ){

                // show busy dialog
                activity.showDialog( DIALOG_BUSY );
            }
            else {
                // update ui (by subclass)
                this.serviceActivity.refreshActivityUi();

                //  dismiss busy dialog
                if( this.busyDialog != null && this.busyDialog.isShowing() ){

                    activity.dismissDialog( DIALOG_BUSY );
                }
            }
        }
    }

    /**
     * service connection
     */
    private ServiceConnection serviceConn = new ServiceConnection(){

        public void onServiceConnected( ComponentName className, IBinder service ){

            BtLog.i( "onServiceConnected()[+]" );
            ServiceActivityHelper.this.service = ServiceActivityHelper.this.serviceActivity.asInterface( service );
            ServiceActivityHelper.this.serviceActivity.onServiceConnected();
        }

        // is called when a connection to the Service has been lost.
        // this typically happens when the process hosting the service has "crashed" or been "killed".
        public void onServiceDisconnected( ComponentName className ){

            BtLog.w( "onServiceDisconnected()[+] component[" + className + "]" );
            ServiceActivityHelper.this.serviceActivity.onServiceDisconnected();
            ServiceActivityHelper.this.service = null;

            // show message and finish Activity if it's
            if( serviceActivity instanceof Activity ){

                Activity activity = (Activity)serviceActivity;
                Toast.makeText( activity, R.string.bt_base_activity_service_disconnected, Toast.LENGTH_LONG );
                activity.finish();
            }
        }
    };

    /**
     * try to connect to service, usually be called at activity's onCreate()
     *
     * @param context
     */
    public boolean bindService( Context context ){

        BtLog.d( "bindService()[+]" );

        this.isServiceAvailable = false;
        boolean res = context.bindService( new Intent( this.serviceActivity.getServiceAction() ), serviceConn,
                Context.BIND_NOT_FOREGROUND | Context.BIND_AUTO_CREATE );
        if( !res ){
            BtLog.e( "bind service failed: action[" + this.serviceActivity.getServiceAction() + "]" );
        }
        return res;
    }

    /**
     * unbind connected service, usually be called at activity's onDestory()
     *
     * @param context
     */
    public void unbindService( Context context ){

        BtLog.d( "unbindService()[+]" );

        // unbind service: don't set serviceConn = null or reuse Helper will get NPE
        context.unbindService( this.serviceConn );
        this.service = null;
        this.busyDialog = null;
    }

    /**
     * create dialog for service activity, e.g. BusyDialog
     *
     * @param id
     * @param context
     * @return
     */
    public Dialog createBusyDialog( int id, Context context ){

        BtLog.d( "createBusyDialog()[+]" );

        // check dialog id
        if( id != DIALOG_BUSY )    return null;

        // create busy dialog only
        this.busyDialog = new ProgressDialog( context );
        this.busyDialog.setMessage( context.getString( R.string.bt_base_activity_busy_message ) );
        this.busyDialog.setIndeterminate( true );
        this.busyDialog.setCancelable( false );
        return this.busyDialog;
    }
}
