package com.mediatek.engineermode.user2root;

import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;
import android.provider.Settings;
import com.mediatek.engineermode.R;

/**
 * 
 * This is a demo for User To Root.
 * If you use this app, you MUST open system properties write security.
 * 
 * Update for android ics: persist.sys.usb.config
 * 
 * @author mtk71029 Yanghui Li <yanghui.li@mediatek.com>
 *
 */
public class User2rootActivity extends Activity {
    /** Called when the activity is first created. */

    private Button mRootButton;
    private Button mUserButton;

    private static final String ANDROID_BUILD_VERSION = "ro.build.version.sdk";
    private static final int ANDROID_BUILD_ICS = 14; 

    private static final String RO_SECURE = "ro.secure";
    private static final String RO_ALLOW_MOCK_LOCATION="ro.allow.mock.location";
    private static final String RO_DEBUG = "ro.debuggable";
    private static final String ADB_ENABLE_GB = "persist.service.adb.enable";
    private static final String ADB_ENABLE_ICS = "persist.sys.usb.config";
    private static final String ATCI_USERMODE = "persist.service.atci.usermode";


    private void toRoot_gb(){
        SystemProperties.set(RO_SECURE, "0");
        SystemProperties.set(RO_ALLOW_MOCK_LOCATION,"1" );
        SystemProperties.set(RO_DEBUG, "1");
        SystemClock.sleep(200);
        SystemProperties.set(ADB_ENABLE_GB, "1");
        Toast.makeText(User2rootActivity.this, "Update to Root Success", Toast.LENGTH_LONG).show();
    }

    private void toRoot_ics(){
        SystemProperties.set(ADB_ENABLE_ICS, "none");
        //SystemProperties.set("ctl.stop", "adbd");
        SystemProperties.set(RO_SECURE, "0");
        //SystemProperties.set(RO_ALLOW_MOCK_LOCATION,"1" );
        SystemProperties.set(RO_DEBUG, "1");
        SystemProperties.set(ADB_ENABLE_ICS, "mass_storage,adb,acm");
        //SystemProperties.set("ctl.start", "adbd");
        SystemProperties.set(ATCI_USERMODE, "1");
        try {
            Process proc = Runtime.getRuntime().exec("start atcid-daemon-u");
        } catch (IOException e) {
            e.printStackTrace();
        }
        Toast.makeText(User2rootActivity.this, "Update to Root Success", Toast.LENGTH_LONG).show();
    }

    private void toUser_gb(){
        SystemProperties.set(RO_SECURE, "1");
        SystemProperties.set(RO_ALLOW_MOCK_LOCATION,"0" );
        SystemProperties.set(RO_DEBUG, "0");
        SystemProperties.set(ADB_ENABLE_GB, "0");
        Toast.makeText(User2rootActivity.this, "Update to User Success", Toast.LENGTH_LONG).show();
    }

    private void toUser_ics(){
        SystemProperties.set(ATCI_USERMODE, "0");
        try {
            Process proc = Runtime.getRuntime().exec("stop atcid-daemon-u");
        } catch (IOException e) {
            e.printStackTrace();
        }
        SystemProperties.set(ADB_ENABLE_ICS, "mass_storage");
        SystemProperties.set(RO_SECURE, "1");
        //SystemProperties.set(RO_ALLOW_MOCK_LOCATION,"0" );
        SystemProperties.set(RO_DEBUG, "0");
        
        Toast.makeText(User2rootActivity.this, "Update to User Success", Toast.LENGTH_LONG).show();
    }

    private OnClickListener mRootListener = new Button.OnClickListener(){

        @Override
        public void onClick(View v) {
            int sdkVersion = SystemProperties.getInt(ANDROID_BUILD_VERSION, 10);
            if(sdkVersion >= ANDROID_BUILD_ICS){
                toRoot_ics();
            }else{
                toRoot_gb();
            }
        }
    };

    private OnClickListener mUserListener = new Button.OnClickListener(){

        @Override
        public void onClick(View v) {
        int sdkVersion = SystemProperties.getInt(ANDROID_BUILD_VERSION, 10);
            if(sdkVersion >= ANDROID_BUILD_ICS){
                toUser_ics();
            }else{
                toUser_gb();
            }
        }
    };

    protected void findViews(){
        this.mRootButton = (Button) this.findViewById(R.id.root);
        this.mUserButton = (Button) this.findViewById(R.id.user);
    }

    protected void setActionListener() {
        this.mRootButton.setOnClickListener(this.mRootListener);
        this.mUserButton.setOnClickListener(this.mUserListener);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.user2root);
        this.findViews();
        this.setActionListener();
    }

}
