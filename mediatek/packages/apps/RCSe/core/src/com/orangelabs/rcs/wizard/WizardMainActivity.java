package com.orangelabs.rcs.wizard;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;

import java.util.List;

import org.apache.http.HttpHost;
import org.apache.http.HttpVersion;
import org.apache.http.client.CookieStore;
import org.apache.http.client.protocol.ClientContext;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.params.ConnRoutePNames;
import org.apache.http.impl.client.BasicCookieStore;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.SingleClientConnManager;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.HttpContext;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Proxy;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.provider.Settings;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.view.Menu;
import android.view.Surface;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import android.os.ServiceManager;
import android.view.IWindowManager;
import android.widget.Toast;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.client.ClientApiIntents;
import com.orangelabs.rcs.service.api.client.ImsApiIntents;
import com.orangelabs.rcs.utils.logger.Logger;
import android.os.SystemProperties;

public class WizardMainActivity extends Activity {
  
	
   private ProgressDialog mprogressDialog = null;
   
   private static Handler mUIHandler = null;
   
   private NetworkInfo mCurrentNetworkInfo = null;
   /**
    * The logger
    */
   private Logger logger = Logger.getLogger(this.getClass().getName());
  
   private boolean mSimExist = false;
   
   
   /**
    * rcs wizard service related 
    * */
   Messenger mService = null;
   boolean mIsBound;
   final Messenger mMessenger = new Messenger(new WizardUIhandler());
   
   
   private ServiceConnection mConnection = new ServiceConnection() {
	
	@Override
	public void onServiceDisconnected(ComponentName name) {
		// TODO Auto-generated method stub
		 // This is called when the connection with the service has been unexpectedly disconnected - process crashed.
        mService = null;
	}
	
	@Override
	public void onServiceConnected(ComponentName name, IBinder service) {
		// TODO Auto-generated method stub
		 mService = new Messenger(service);
		 
		 //send the message to the service
		 try{
			 
			 //show processing screen
			 showProcessingScreen();

			 
			 sendMessageToService(RcsWizardManagerService.MSG_INITIALIZE_JOYN);
			

		 }catch(RemoteException e){
			 //the service has crashed
		 }
		 
	}
};

   private void CheckIfServiceIsRunning() {
    //If the service is running when the activity starts, we want to automatically bind to it.
    
	   if(!RcsWizardManagerService.isRunning()){
	   startWizardService();
	   }
	
  }

   private void startWizardService() {
    boolean status ;
    status = getApplicationContext().bindService(new Intent(this, RcsWizardManagerService.class), mConnection, Context.BIND_AUTO_CREATE);
    mIsBound = true;
   }

   void doUnbindService() {
	   boolean status = false;
       if (mIsBound) {
           // If we have received the service, and hence registered with it, then now is the time to unregister.
           if (mService != null) {
               try {
                   
            	   
            	  // status = stopService(new Intent(this,RcsWizardManagerService.class));
            	   sendMessageToService(RcsWizardManagerService.MSG_STOP_SERVICE);
            	  
               } catch (Exception e) {
                   // There is nothing special we need to do if the service has crashed.
               }
           }
           // Detach our existing connection.
           
           getApplicationContext().unbindService(mConnection);
           mIsBound = false;
           
           //status = RcsWizardManagerService.isRunning();
       }
   }
   
   
   private void sendMessageToService(int message) throws RemoteException{
       if (mIsBound) {
           if (mService != null) {
               try {
                   Message msg = Message.obtain(null, message, 0, 0);
                   msg.replyTo = mMessenger;
                   mService.send(msg);
               } catch (RemoteException e) {
            	   throw e;
               }
           }
       }
   }
	@Override
  protected void onResume() {
	// TODO Auto-generated method stub
	super.onResume();
	
}

	@Override
   protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		//Diable window rotate for phone.  
       //setWindowRotation(false);
       
		/**TODO disable the sttaic bar  **/
     
		
		 //check if sim exists
		 mSimExist = isSimExist();
			
	      // Set the connectivity manager
		  ConnectivityManager networkConnection = (ConnectivityManager)getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
		  NetworkInfo networkInfo = networkConnection.getActiveNetworkInfo();
			
	      // If network connected then
		  if(mSimExist && (networkInfo != null)){
	        	mCurrentNetworkInfo = networkInfo;
	        	
	    		CheckIfServiceIsRunning();
				
	      }
	      //if no network
	      else{
	    	  
	    	  
	    	  // TODO : update system provider entery
	    	    //finish rcs oobe wizard
	    		finishRcsOOBE(false);
	      }
		  
      

		
		// TODO : first run system provider entery
		/**
		 mIsFirstRun = Settings.System.getInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 0) == 0;
	        if (mIsFirstRun) {
	            Settings.System.putInt(getContentResolver(),
	                Settings.System.OOBE_DISPLAY, Settings.System.OOBE_DISPLAY_ON);
	            Settings.Global.putInt(getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 0);
	            Settings.Secure.putInt(getContentResolver(), Settings.Secure.USER_SETUP_COMPLETE, 0);
	            enableStatusBar(false);
	        }
	        
	      */  
	}

	@Override
   public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
	
   
   //if sim is present in the phone 
   private boolean isSimExist() {
       boolean simExist = false;
       List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(this);
       if (simList != null) {
           simExist = simList.size() > 0;
       }
       return simExist;
   }

   /**
    * SHOW PROCESSING SCREEN
    * 
    * */
   private void showProcessingScreen(){
	 
	   //EXECUTE THE PROGRESS DIALOG AT THE RUN TIME
	   AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
			
			@Override
			protected void onPreExecute() {
				
				 if(mprogressDialog==null){
					   mprogressDialog = new ProgressDialog(WizardMainActivity.this);
					   mprogressDialog.setIcon(R.drawable.rcs_icon);
					   mprogressDialog.setTitle(R.string.processing_dialog_title);   
					   mprogressDialog.setMessage(getString(R.string.processing_dialog_message));			
					   mprogressDialog.setCancelable(false);
					   mprogressDialog.setIndeterminate(true);
				    }
				    mprogressDialog.show();			
     	  }

			@Override
			protected Void doInBackground(Void... params) {
				try{
				 
				}
				catch(Exception e){
					
				}
				// TODO Auto-generated method stub
				return null;
			}
	 
	}.execute();

   }
   
   private void hideProcessingScreen(){
	   if(mprogressDialog!=null)
		   mprogressDialog.dismiss();
	       mprogressDialog = null;
    }
   
   /**
    * Whether the device is tablet
    * @return true if the device is tablet, false if the device is phone.
    */
   private boolean isTablet() {
       String sDeviceInfo = SystemProperties.get("ro.build.characteristics");   
       if (logger.isActivated()) {
			logger.debug("devide info: " + sDeviceInfo);
		}  
       return "tablet".equals(sDeviceInfo);
   }
   //finish rcs OOBE process
   private void finishRcsOOBE(boolean state) {
   	
   	
  
       //disable RCSWizardActivity
       PackageManager pm = getPackageManager();
       ComponentName name = new ComponentName(this, RcsWizardActivity.class);
       int enabledState = pm.getComponentEnabledSetting(name);
       if (enabledState != PackageManager.COMPONENT_ENABLED_STATE_DISABLED) {
           pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                   PackageManager.DONT_KILL_APP);
       }
    
	   
	   
     //set the wizard state to true
     // RcsWizardManager.getInstance().setWizardState(false);
     		
       /*
        * if (mIsFirstRun) {
       
           if (isTablet()) {
               Utils.startLauncher(this);
           } else {
               // start quick start guide if OOBE is first run
               Intent intent = new Intent(ACTION_QUICK_START_GUIDE);
               intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
               intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
               intent.putExtra("mIsFirstRun", true);
               startActivity(intent);
           }

           //set device provisioned, oobe has run.
           Settings.Global.putInt(getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 1);
           Settings.Secure.putInt(getContentResolver(), Settings.Secure.USER_SETUP_COMPLETE, 1);
           Settings.System.putInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 1);
           //set oobe display finish
           Settings.System.putInt(getContentResolver(),
               Settings.System.OOBE_DISPLAY, Settings.System.OOBE_DISPLAY_DEFAULT);
       }
        */
       
       finish();
   }
   
   //set window roatation
   private void setWindowRotation(boolean enable) {
       if (!isTablet()) {
           try {
               IWindowManager wm = IWindowManager.Stub.asInterface(
                       ServiceManager.getService(Context.WINDOW_SERVICE));
               if (enable) {
                   wm.thawRotation();
               } else {
                   wm.freezeRotation(Surface.ROTATION_0);
               }
           } catch (RemoteException e) {
           	if (logger.isActivated()) {
       			logger.debug("New conference event notification received");
       		}  
           }
       }
   }

   /**
    * SHOW JOYN INTRODUCION
    */
   private void showJoynIntroduction(){
   	//Toast.makeText(this, "JOYN INTRoduction!!! ", Toast.LENGTH_LONG).show();
 
   	//start joyn introduction
 	Intent intent = new Intent(this, JoynIntroduction.class);
	startActivityForResult(intent, 1);
	
    overridePendingTransition(R.anim.slide_right_in, R.anim.slide_left_out);
   }
      
   /**
    * SHOW JOYN INTRODUCION VIDEO
    */
   private void showJoynIntroductionVideo(){
   	//Toast.makeText(this, "JOYN INTRoduction!!! ", Toast.LENGTH_LONG).show();
 
   	//start joyn introduction
 	Intent intent = new Intent(this, JoynIntroductionVideo.class);
	startActivityForResult(intent, 1);
	
    overridePendingTransition(R.anim.slide_right_in, R.anim.slide_left_out);
   }
   
   
   protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
      // Xlog.d(TAG, "onActivityResult resultCode = " + resultCode);
       switch (resultCode) {
           case Utils.RESULT_CODE_NEXT:	   
        	   showJoynIntroductionVideo();
        	   //showProvisioningsucess();
               break;
           case Utils.RESULT_CODE_FINISH_VIDEO:
        	     doUnbindService();
        	     finishRcsOOBE(true);
           default:
               break;
       }
   }

   
   //show dialog to close the other joyn client and then start our own client
   private void askregistration(final Context appContext){
   	AlertDialog.Builder alertDialog = new AlertDialog.Builder(appContext);
		 
       // Setting Dialog Title
       alertDialog.setTitle(R.string.title_registration_confirm_dialog);

       // Setting Dialog Message
       alertDialog.setMessage(R.string.message_registration_confirm_dialog);

       alertDialog.setCancelable(true);
       
       //negative button
       alertDialog.setNegativeButton(R.string.skip_button_registration_confirm, new DialogInterface.OnClickListener() {
           public void onClick(DialogInterface dialog,int which) {		 
        	  // RcsWizardManager.skipRegistration();
           }
       });
       
       //continue
       alertDialog.setPositiveButton(R.string.continue_button_registration_confirm, new DialogInterface.OnClickListener() {
           public void onClick(DialogInterface dialog,int which) {		 
           //	RcsWizardManager.getInstance().startRegistration();
           }
       });
       
       alertDialog.show();
   }
    
   
   /**
    * ui HANDLER FOR GETTING UI UPDATES BASED ON MESSAGE
    * */
   private class WizardUIhandler extends Handler{
   	public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
		  switch(msg.what){
		  case RcsWizardManagerService.MSG_SHOW_PROCESSING :  
			  showProcessingScreen();
			  return;
		  case RcsWizardManagerService.MSG_SHOW_JOYN_INTRO : 
			   //hideProcessing screen
			   hideProcessingScreen();
			   
			   //show introduction of joyn
			   showJoynIntroduction();
		  return ; 
		 /* case RcsWizardManager.MSG_SHOW_PROVISION_SUCCESS :  
			  showProvisioningsucess();
		  return ; 
			
		  case RcsWizardManager.MSG_SHOW_REGISTRATION_ASK_DIALOG :  
			  askregistration();
	      return ; 
		
		  case RcsWizardManager.MSG_REGISTER_REGISTRATION_RECEIVER :  
			  registerRegistrationListner();
	      return ;
           */
		  
		  case RcsWizardManagerService.MSG_PROVISON_FAIURE : 
			  hideProcessingScreen();
			  doUnbindService();
			  finishRcsOOBE(false);
		      return ; 
		  case RcsWizardManagerService.MSG_RCS_OOBE_FINISH :  
		      finishRcsOOBE(true);
	      return ; 
	      default: 
	    	  super.handleMessage(msg);
		  }
		}
   }
}
