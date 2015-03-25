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

import java.util.List;

import android.app.Dialog;
import android.os.Bundle;
import android.preference.PreferenceActivity;

import com.mediatek.bluetooth.util.BtLog;

public class AssembledPreferenceActivity extends PreferenceActivity {

    public interface AssemblyPreference {

        public int getPreferenceResourceId();
        public void onCreate( PreferenceActivity parentActivity );
        public void onDestroy();
        public void onResume();
        public void onSaveInstanceState( Bundle outState );
        public void onRestoreInstanceState( Bundle savedInstanceState );

        public Dialog onCreateDialog( int id );
    }

    /**
     * keep the all registered DockablePreference objects
     */
    private List<AssemblyPreference> registeredPreference;


    /**
     * Constructor
     *
     * @param registeredPreference
     */
    protected AssembledPreferenceActivity( List<AssemblyPreference> registeredPreference ){

        this.registeredPreference = registeredPreference;
    }

    @Override
    protected void onCreate( Bundle savedInstanceState ){

        BtLog.d( "onCreate()[+]" );

        super.onCreate( savedInstanceState );

        // create preferences and call onCreate()
        for( AssemblyPreference dp : this.registeredPreference ){

            // combine all resource from registered DockablePreference
            this.addPreferencesFromResource( dp.getPreferenceResourceId() );

            // let DockablePreference init itself
            dp.onCreate( this );
        }
    }

    @Override
    protected void onResume(){

        BtLog.d( "onResume()[+]" );
        /*parent's onResume has to run first, or else MAP settings will be influenced*/
        super.onResume();

        for( AssemblyPreference dp : this.registeredPreference ){
            dp.onResume();
        }

    }

    @Override
    protected void onDestroy(){

        BtLog.d( "onDestroy()[+]" );

        // destroy all preferences
        for( AssemblyPreference dp : this.registeredPreference ){
            dp.onDestroy();
        }

        super.onDestroy();
    }

    @Override
    protected void onRestoreInstanceState(Bundle state) {

        BtLog.d( "onRestoreInstanceState()[+]" );
        for( AssemblyPreference dp : this.registeredPreference ){
            dp.onRestoreInstanceState(state);
        }
                super.onRestoreInstanceState(state);

    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {

        BtLog.d( "onSaveInstanceState()[+]" );
        for( AssemblyPreference dp : this.registeredPreference ){
            dp.onSaveInstanceState(outState);
        }
                super.onSaveInstanceState(outState);

    }
}
