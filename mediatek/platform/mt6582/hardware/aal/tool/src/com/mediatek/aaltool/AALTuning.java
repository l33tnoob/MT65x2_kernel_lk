package com.mediatek.aaltool;

import android.os.Bundle;
import android.provider.Settings;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.AbsoluteLayout;
import android.widget.Button;
import android.content.ContentResolver;
import android.content.SharedPreferences;
import android.content.Context;
import android.graphics.Color;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.Map;

public class AALTuning extends Activity implements OnSeekBarChangeListener {
    private static final String TAG = "AALTool";
    private static final String PREFS_NAME = "aal";
    private static final String FILE_NAME = "aal.cfg";
    private SeekBar mBrightnessBar;
    private SeekBar mDarkeningSpeedBar;
    private SeekBar mBrighteningSpeedBar;
    private SeekBar mSmartBacklightBar;
    private SeekBar mToleranceRatioBar;
    private SeekBar mReadabilityBar;
    private TextView mBrightnessText;
    private TextView mDarkeningSpeedText;
    private TextView mBrighteningSpeedText;
    private TextView mSmartBacklightText;
    private TextView mToleranceRatioText;
    private TextView mReadabilityText;
    private TextView mDarkeningSpeedTitle;
    private TextView mBrighteningSpeedTitle;
    private TextView mToleranceRatioTitle;
    private TextView mReadabilityTitle;
    private int mBrightnessLevel = 5;
    private int mDarkeningSpeedLevel = 2;
    private int mBrighteningSpeedLevel = 5;
    private int mSmartBacklightLevel = 5;
    private int mToleranceRatioLevel = 0;
    private int mReadabilityLevel = 5;
    private Button mSaveButton;
    private SharedPreferences mPreferences;
    private int mBrightness = 255;
    private int mBrightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC;
    private int mPreBrightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;
    
    //the content resolver used as a handle to the system's settings  
    private ContentResolver mContentResolver;  
    //a window object, that will store a reference to the current window  
    private Window mWindow;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.aal_tuning);        
        
        Log.d(TAG, "onCreate...");
        
        
        //get the content resolver  
        mContentResolver = getContentResolver();
        //get the current window  
        mWindow = getWindow();
        try {
             mPreBrightnessMode = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, mBrightnessMode);
		
        
        mSaveButton = (Button)findViewById(R.id.buttonSave);
        mSaveButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                saveToFile();
            }
        });
        
        mPreferences = getSharedPreferences(PREFS_NAME, 0);
        loadPreference();

        mBrightnessText = (TextView) this.findViewById(R.id.textBrightnessLevel); 
        mBrightnessText.setText("level: " + Integer.toString(mBrightnessLevel));
        mBrightnessBar = (SeekBar)findViewById(R.id.seekBarBrightness); // make seekbar object
        mBrightnessBar.setOnSeekBarChangeListener(this); // set seekbar listener.
        
        mDarkeningSpeedText = (TextView) this.findViewById(R.id.textDarkeningSpeedLevel); 
        mDarkeningSpeedText.setText("level: " + Integer.toString(mDarkeningSpeedLevel));
        mDarkeningSpeedBar = (SeekBar)findViewById(R.id.seekBarDarkeningSpeed); // make seekbar object
        mDarkeningSpeedBar.setOnSeekBarChangeListener(this); // set seekbar listener. 
        
        mBrighteningSpeedText = (TextView) this.findViewById(R.id.textBrighteningSpeedLevel); 
        mBrighteningSpeedText.setText("level: " + Integer.toString(mBrighteningSpeedLevel));
        mBrighteningSpeedBar = (SeekBar)findViewById(R.id.seekBarBrighteningSpeed); // make seekbar object
        mBrighteningSpeedBar.setOnSeekBarChangeListener(this); // set seekbar listener.       

        mSmartBacklightText = (TextView) this.findViewById(R.id.textSmartBacklightLevel); 
        mSmartBacklightText.setText("level: " + Integer.toString(mSmartBacklightLevel));
        mSmartBacklightBar = (SeekBar)findViewById(R.id.seekBarSmartBacklight); // make seekbar object
        mSmartBacklightBar.setOnSeekBarChangeListener(this); // set seekbar listener.

        mToleranceRatioText = (TextView) this.findViewById(R.id.textToleranceRatioLevel); 
        mToleranceRatioText.setText("level: " + Integer.toString(mToleranceRatioLevel));
        mToleranceRatioBar = (SeekBar)findViewById(R.id.seekBarToleranceRatio); // make seekbar object
        mToleranceRatioBar.setOnSeekBarChangeListener(this); // set seekbar listener.       
        
        mReadabilityText = (TextView) this.findViewById(R.id.textReadabilityLevel); 
        mReadabilityText.setText("level: " + Integer.toString(mReadabilityLevel));
        mReadabilityBar = (SeekBar)findViewById(R.id.seekBarReadability); // make seekbar object
        mReadabilityBar.setOnSeekBarChangeListener(this); // set seekbar listener.

        mDarkeningSpeedTitle = (TextView) this.findViewById(R.id.textDarkeningSpeed); 
        mBrighteningSpeedTitle = (TextView) this.findViewById(R.id.textBrighteningSpeed); 
        mToleranceRatioTitle = (TextView) this.findViewById(R.id.textToleranceRatio); 
        mReadabilityTitle = (TextView) this.findViewById(R.id.textReadability); 

        enableSeekBar((mBrightnessLevel > 0), mDarkeningSpeedBar, mDarkeningSpeedTitle);
        enableSeekBar((mBrightnessLevel > 0), mBrighteningSpeedBar, mBrighteningSpeedTitle);
        enableSeekBar((mBrightnessLevel > 0), mReadabilityBar, mReadabilityTitle);
        enableSeekBar((mSmartBacklightLevel > 0), mToleranceRatioBar, mToleranceRatioTitle);

        mBrightnessBar.setProgress(mBrightnessLevel);
        mDarkeningSpeedBar.setProgress(mDarkeningSpeedLevel);
        mBrighteningSpeedBar.setProgress(mBrighteningSpeedLevel);
        mSmartBacklightBar.setProgress(mSmartBacklightLevel);
        mToleranceRatioBar.setProgress(mToleranceRatioLevel);
        mReadabilityBar.setProgress(mReadabilityLevel);
    }

    private void setScreenBrightness() {
        try {
             mBrightness = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS);
        } catch (Exception e) {
            e.printStackTrace();
        }
        nSetBacklight(mBrightness);
    }
    
    private void loadPreference() {
        Map<String,?> keys = mPreferences.getAll();
        for(Map.Entry<String,?> entry : keys.entrySet()) {
            Log.d(TAG, "map values " + entry.getKey() + ": " + entry.getValue().toString());
            int value = Integer.parseInt(entry.getValue().toString());
            if (entry.getKey().equals("Brightness"))
                mBrightnessLevel = value;
            if (entry.getKey().equals("DarkeningSpeed")) 
                mDarkeningSpeedLevel = value;
            if (entry.getKey().equals("BrighteningSpeed")) 
                mBrighteningSpeedLevel = value;
            if (entry.getKey().equals("SmartBacklight")) 
                mSmartBacklightLevel = value;
            if (entry.getKey().equals("ToleranceRatio")) 
                mToleranceRatioLevel = value;
            if (entry.getKey().equals("Readability"))
                mReadabilityLevel = value;
        }
    }
    private void saveToFile() {
        try {
            
            FileOutputStream fos = openFileOutput(FILE_NAME, Context.MODE_PRIVATE);
            PrintWriter pw = new PrintWriter(fos); 
            pw.println("Brightness=" + mBrightnessLevel);
            pw.println("DarkeningSpeed=" + mDarkeningSpeedLevel);
            pw.println("BrighteningSpeed=" + mBrighteningSpeedLevel);
            pw.println("SmartBacklight=" + mSmartBacklightLevel);
            pw.println("ToleranceRatio=" + mToleranceRatioLevel);
            pw.println("Readability=" + mReadabilityLevel);
            pw.close();
            fos.close();
        } catch(Exception e){
            e.printStackTrace();
        }
    }

    private void enableSeekBar(boolean enable, SeekBar seekbar, TextView titletext) {
        if (enable) {
            seekbar.setEnabled(true);
            titletext.setTextColor(Color.rgb(0, 0, 0));
        }
        else {
            seekbar.setEnabled(false);
            titletext.setTextColor(Color.rgb(156, 156, 156));
        }	
    }
    	
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.aal_tuning, menu);
        return true;
    }

    @Override
    public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {        
        String key = "";
        if (arg0 == mBrightnessBar) {
            Log.d(TAG, "Brightness level = " + arg1);
            if (nSetBrightnessLevel(arg1)) {
                key = "Brightness";
                mBrightnessLevel = arg1;
                mBrightnessText.setText("level: " + Integer.toString(mBrightnessLevel));
				
                enableSeekBar((mBrightnessLevel > 0), mDarkeningSpeedBar, mDarkeningSpeedTitle);
                enableSeekBar((mBrightnessLevel > 0), mBrighteningSpeedBar, mBrighteningSpeedTitle);
                enableSeekBar((mBrightnessLevel > 0), mReadabilityBar, mReadabilityTitle);
            }
        }
        if (arg0 == mDarkeningSpeedBar) {
            Log.d(TAG, "set Darkening Speed level = " + arg1);
            if (nSetDarkeningSpeedLevel(arg1)) {
                key = "DarkeningSpeed";
                mDarkeningSpeedLevel = arg1;
                mDarkeningSpeedText.setText("level: " + Integer.toString(mDarkeningSpeedLevel));
            }
        }
        if (arg0 == mBrighteningSpeedBar) {
            Log.d(TAG, "set Brightening Speed level = " + arg1);
            if (nSetBrighteningSpeedLevel(arg1)) {
                key = "BrighteningSpeed";
                mBrighteningSpeedLevel = arg1;
                mBrighteningSpeedText.setText("level: " + Integer.toString(mBrighteningSpeedLevel));
            }
        }
        if (arg0 == mSmartBacklightBar) {
            Log.d(TAG, "set SmartBacklight level = " + arg1);
            if (nSetSmartBacklightLevel(arg1)) {
                key = "SmartBacklight";
                mSmartBacklightLevel = arg1;
                mSmartBacklightText.setText("level: " + Integer.toString(mSmartBacklightLevel));

                enableSeekBar((mSmartBacklightLevel > 0), mToleranceRatioBar, mToleranceRatioTitle);
            }
        }
        if (arg0 == mToleranceRatioBar) {
            Log.d(TAG, "set Tolerance Ratio level = " + arg1);
            if (nSetToleranceRatioLevel(arg1)) {
                key = "ToleranceRatio";
                mToleranceRatioLevel = arg1;
                mToleranceRatioText.setText("level: " + Integer.toString(mToleranceRatioLevel));
            }
        }
        if (arg0 == mReadabilityBar) {
            Log.d(TAG, "set Readability level = " + arg1);
            if (nSetReadabilityLevel(arg1)) {
                key = "Readability";
                mReadabilityLevel = arg1;
                mReadabilityText.setText("level: " + Integer.toString(mReadabilityLevel));
            }
        }
		
        setScreenBrightness();

        if (key.length() > 0) {
            SharedPreferences.Editor editor = mPreferences.edit();
            editor.putInt(key, arg1);
            editor.commit();
        }

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
        
    }
    
    @Override
    protected void onResume() {
        Log.d(TAG, "onResume..., restore brightness mode to " + mBrightnessMode);
        try {
             mPreBrightnessMode = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, mBrightnessMode);
		
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause..., restore brightness mode to " + mPreBrightnessMode);
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, mPreBrightnessMode);
        super.onPause();
    }
	
    static {
        System.loadLibrary("aaltool_jni");
    }

    private native boolean nSetBrightnessLevel(int level);
    private native boolean nSetDarkeningSpeedLevel(int level);
    private native boolean nSetBrighteningSpeedLevel(int level);
    private native boolean nSetSmartBacklightLevel(int level);
    private native boolean nSetToleranceRatioLevel(int level);
    private native boolean nSetReadabilityLevel(int level);
    private native boolean nSetBacklight(int level);
    
}
