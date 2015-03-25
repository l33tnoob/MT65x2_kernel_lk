/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.network;


import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.widget.EditText;
import android.widget.ToggleButton;

/**
 * HTTPS provisioning - MSISDN Pop-up activity
 *
 * @author Orange
 */
public class OnlyApnRoamingAlertDialog extends Activity {
    
	
	public static boolean roamingState = true;

	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final CharSequence[] items = {" Roaming OFF "," Roaming ON "};
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        alert.setTitle(R.string.label_edit_msisdn);
        alert.setSingleChoiceItems(items, 0,
	        new DialogInterface.OnClickListener() {
	            public void onClick(DialogInterface dialog, int item) {
	
	                switch (item) {
	                    case 0:
	                    setRcsRoaming(true);
	                    
	                    break;
	                    case 1: 
	                    setRcsRoaming(false);
	                
	                    break; 
	
	                    } 
	                }
	           }); 
        // Set an EditText view to get user input
      //  final EditText input = new EditText(this);
      

        alert.setPositiveButton(R.string.label_ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                Boolean value = roamingState;
                OnlyApnRoamingState.getInstance().responseReceived(value);
                finish();
            }
        });

		alert.setNegativeButton(R.string.label_cancel, new DialogInterface.OnClickListener() {
		  public void onClick(DialogInterface dialog, int whichButton) {
			  Boolean value = roamingState;
			  OnlyApnRoamingState.getInstance().responseReceived(value);
			  
			  finish();
		  }
		});

		final AlertDialog dial = alert.show();
		new Handler().postDelayed(new Runnable() {
		    public void run() {
		    	Boolean value = roamingState;
				OnlyApnRoamingState.getInstance().responseReceived(value);
				dial.dismiss();
				finish();
		    }
		}, 3000);
	}
    
    
    public void setRcsRoaming(Boolean value){
    	RcsSettings.getInstance().setRcsOnlyRoamingAuthorizationState(false);
    	roamingState= value;
    	return;
    }
   
   
    
}
