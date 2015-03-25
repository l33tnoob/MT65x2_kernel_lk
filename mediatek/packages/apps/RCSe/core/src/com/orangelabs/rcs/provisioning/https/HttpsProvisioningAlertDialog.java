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

package com.orangelabs.rcs.provisioning.https;

import com.orangelabs.rcs.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.widget.EditText;
import android.widget.TextView;
import android.view.View;
/**
 * HTTPS provisioning - MSISDN Pop-up activity
 *
 * @author Orange
 */
public class HttpsProvisioningAlertDialog extends Activity {

	  private static volatile Object objInstance = new Object();
	  //private static boolean finalStatus = false;
	  private static Handler dialogHandler = null; 
	  
	  private boolean showErrorFlag = false;
	  private boolean showOTPInput = false;
	  
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        showErrorFlag = getIntent().getBooleanExtra("showerrormessage",false);
        showOTPInput = getIntent().getBooleanExtra("showOtpDialog",false);
        
        showRelevenatDialog();
        
	}
    
    private void showRelevenatDialog(){
    	

        if(showErrorFlag){
        	showErrorMessage();
        }
        else if(showOTPInput){
        	showOTPInputDialog();
        }
        else{
        	showInputDialog("");   	
        }
        
    }
    
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		cleanHandler();
		super.onDestroy();		
	} 
   

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		 
		 showRelevenatDialog();
	}
	
    
	private void cleanHandler(){
		if(dialogHandler!= null){
			dialogHandler.removeCallbacksAndMessages(null);
			dialogHandler = null;
		}
	}
    
	private void showErrorMessage(){
	AlertDialog.Builder alert = new AlertDialog.Builder(this);
    
    /**
     * M : added for chaning UI according to specs
     */
    alert.setTitle(R.string.rcs_joyn_name);

    // Setting Icon to Dialog
    alert.setIcon(R.drawable.rcs_icon);
    
    // Setting Dialog Message
    alert.setMessage(R.string.label_msisdn_error_dialog);

    /**
     * @ 
     * */
    alert.setPositiveButton(R.string.label_ok, new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface dialog, int whichButton) {
            finish();
            HttpsProvionningMSISDNInput.getInstance().errorMsgResponseAcknowledged();
        }
    });
    
	alert.show();
}


 private void showInputDialog(String number){
	 
	 cleanHandler();
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        
        /**
         * M : added for chaning UI according to specs
         */
        alert.setTitle(R.string.rcs_joyn_name);

        // Setting Icon to Dialog
        alert.setIcon(R.drawable.rcs_icon);
        
        // Setting Dialog Message
        alert.setMessage(R.string.label_edit_msisdn_config);

        /**
         * @ 
         * */
        
        // Set an EditText view to get user input
        final EditText input = new EditText(this);
     input.setText(number);
        alert.setView(input);

        alert.setPositiveButton(R.string.label_ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                String value = input.getText().toString();
             
             showAlertDialog(value);

            }
        });

		alert.setNegativeButton(R.string.label_cancel, new DialogInterface.OnClickListener() {
		  public void onClick(DialogInterface dialog, int whichButton) {
			  HttpsProvionningMSISDNInput.getInstance().responseReceived(null);
			  finish();
		  }
		});

		final AlertDialog dial = alert.show();

		if(dialogHandler == null) {
				
			dialogHandler =	 new Handler();
			dialogHandler.postDelayed(new Runnable() {
		    public void run() {
				        //HttpsProvisioningAlertDialog.objInstance.notify();
				    	
				HttpsProvionningMSISDNInput.getInstance().responseReceived(null);
						
						if(dial.isShowing()){
				dial.dismiss();
						}
				finish();
		    }
		}, HttpsProvisioningUtils.INPUT_MSISDN_TIMEOUT);
	}
		
		
 }
private void showAlertDialog(String number){
    	
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        
        /**
         * M : added for chaning UI according to specs
         */
        alert.setTitle(R.string.rcs_joyn_name);

        // Setting Icon to Dialog
        alert.setIcon(R.drawable.rcs_icon);
        
        
        String msg = number;
        msg += R.string.label_msisdn_confirm_dialog;
        // Setting Dialog Message
        alert.setMessage(msg);

        
        final TextView input = new TextView(this);
        input.setText(number);
        input.setVisibility(View.INVISIBLE);
        alert.setView(input);
        
        
        /**
         * @ 
         * */
    	alert.setNegativeButton(R.string.label_cancel, new DialogInterface.OnClickListener() {
  		  public void onClick(DialogInterface dialog, int whichButton) {  			  			 
  			 // dialog.dismiss(); 
  			String value = input.getText().toString(); 
  			showInputDialog(value);
  			
  			// HttpsProvisioningAlertDialog.finalStatus = false;
  			// HttpsProvisioningAlertDialog.objInstance.notify();
  		  }
  		});

        alert.setPositiveButton(R.string.label_ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {     	
            	
            	 String value = input.getText().toString();
            	 HttpsProvionningMSISDNInput.getInstance().responseReceived(value);
                 //dialog.dismiss();   
                 finish();
              //  HttpsProvisioningAlertDialog.objInstance.notify();
                
            }
        });
        
        final AlertDialog alertDial = alert.show();
    }

private void showOTPInputDialog(){
	AlertDialog.Builder alert = new AlertDialog.Builder(this);
    
    /**
     * M : added for chaning UI according to specs
     */
    alert.setTitle(R.string.rcs_joyn_name);

    // Setting Icon to Dialog
    alert.setIcon(R.drawable.rcs_icon);
    
    // Setting Dialog Message
    alert.setMessage("enter OTP ");

    /**
     * @ 
     * */
    
    // Set an EditText view to get user input
    final EditText input = new EditText(this);
    alert.setView(input);

    alert.setPositiveButton(R.string.label_ok, new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface dialog, int whichButton) {
            String value = input.getText().toString();
            
            HttpsProvionningMSISDNInput.getInstance().updateOTPDialog(value);

        }
    });

		alert.setNegativeButton(R.string.label_cancel, new DialogInterface.OnClickListener() {
		  public void onClick(DialogInterface dialog, int whichButton) {
			  HttpsProvionningMSISDNInput.getInstance().updateOTPDialog(null);
			  finish();
		  }
		});

		final AlertDialog dial = alert.show();
}

}
