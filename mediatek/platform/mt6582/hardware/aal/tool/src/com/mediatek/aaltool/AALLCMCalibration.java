package com.mediatek.aaltool;

import android.os.Bundle;
import android.provider.Settings;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Button;
import android.widget.TextView.OnEditorActionListener;
import android.graphics.Color;
import java.util.Map;
import java.util.TreeMap;
import java.util.Comparator;
import java.util.Iterator;
import java.io.FileOutputStream;
import java.io.PrintWriter;

public class AALLCMCalibration extends Activity {
    private static final String TAG = "AALTool";    
    private static final String PREFS_NAME = "lcm.cal";
    private static final String FILE_NAME = "aal_lcm_cal.cfg";
    private static final int mBrightnessStep = 8;
    private static final int mGreycodeStep = 8;
    private TextView mPatternText;
    private TextView mGreycodeText;
    private TextView mBrightnessText;
    private EditText mLuminanceText;
    private TextView mRecordDataText;
    private View mView;
    private Button mSaveButton;
    private Button mIncButton;
    private Button mDecButton;
    private SharedPreferences mPreferences;
    private TreeMap mSortedMap;

    private float mLuminance = 0;
    private int mPreBrightness = 255;
    private int mBrightness = 255;
    private int mGreycode = 255;

    private AlertDialog mAlertDialog;

    //the content resolver used as a handle to the system's settings  
    private ContentResolver mContentResolver;  
    //a window object, that will store a reference to the current window  
    private Window mWindow;

    
    class MyComparator2 implements Comparator<String>{
    	@Override
    	public int compare(String s1, String s2) {
    	    return Float.compare(Float.parseFloat(s1), Float.parseFloat(s2));
    	}
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.aal_lcm_calibration);
        
        //get the content resolver  
        mContentResolver = getContentResolver();
        //get the current window  
        mWindow = getWindow();
        
        Log.d(TAG, "onCreate...");
        mView = (View) this.findViewById(R.id.view);
        mView.setBackgroundColor(0xffffffff);
        
        
        mRecordDataText = (TextView) this.findViewById(R.id.recordData);
        mRecordDataText.setText("");
        mBrightnessText = (TextView) this.findViewById(R.id.brightness);
        mBrightnessText.setText("Brightness: " + mBrightness);

        try {
             mPreBrightness = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS, mBrightness);
        //preview brightness changes at this window
        //get the current window attributes
        LayoutParams layoutpars = mWindow.getAttributes();
        //set the brightness of this window
        layoutpars.screenBrightness = mBrightness / (float)255;
        //apply attribute changes to this window
        mWindow.setAttributes(layoutpars);

        mLuminanceText = (EditText) this.findViewById(R.id.editTextLuminance);
        mLuminanceText.setOnEditorActionListener(new OnEditorActionListener() {
               @Override
            public boolean onEditorAction(TextView v, int actionId,
                    KeyEvent event) {
                if ((event != null && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER)) ||
                    (actionId == EditorInfo.IME_ACTION_NEXT || actionId == EditorInfo.IME_ACTION_DONE)) {
                    InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                    imm.hideSoftInputFromWindow(mLuminanceText.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
                    saveLuminance();
                    // Must return true here to consume event
                    return true;
                }
                return false;
            }
        });

        mPatternText = (TextView) this.findViewById(R.id.pattern);
        mPatternText.setText("Greycode: ");

        mGreycodeText = (TextView) this.findViewById(R.id.greycode);
        setGreycode();


        // create alert dialog
        mAlertDialog = new AlertDialog.Builder(this).create();
        mAlertDialog.setTitle("Warning");
        mAlertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Cancel", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog,int id) {
                // if this button is clicked, just close
                // the dialog box and do nothing
                dialog.cancel();
            }
        });
/*        
        mAlertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog,int id) {
                // if this button is clicked, close
                // current activity
            }
          });
*/        
        mSaveButton = (Button)findViewById(R.id.buttonSave);
        mSaveButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                //saveLuminance();
                saveToFile();
            }
        });

        mIncButton = (Button)findViewById(R.id.buttonInc);
        mIncButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                incGreycode();
            }
        });

        mDecButton = (Button)findViewById(R.id.buttonDec);
        mDecButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                decGreycode();
            }
        });


        mPreferences = getSharedPreferences(PREFS_NAME, 0);
        mSortedMap = new TreeMap(new MyComparator2());
        mSortedMap.putAll(mPreferences.getAll());
        dumpRecord();
    }

    private void dumpRecord() {
        Iterator iterator = mSortedMap.entrySet().iterator();
        //mRecordDataText.setText("");
        while (iterator.hasNext()) {
            Map.Entry entry = (Map.Entry) iterator.next();
            Log.d(TAG, "greycode=" + entry.getKey()  + ",luminance=" + entry.getValue().toString());
            //mRecordDataText.setText("Greycode=" + entry.getKey() + ", luminance=" + entry.getValue().toString() + 
            //        "\n" + mRecordDataText.getText().toString());
        }
    }

    private void saveLuminance(){
        if (mLuminanceText.getText().length() > 0) {
            mLuminance = Float.parseFloat(mLuminanceText.getText().toString());
            Log.d(TAG, "saveLuminance: Greycode=" + mGreycode + ", luminance=" + String.valueOf(mLuminance));
            mRecordDataText.setText("Greycode=" + mGreycode + ", luminance=" + String.valueOf(mLuminance));
            SharedPreferences.Editor editor = mPreferences.edit();
            String key = String.valueOf(mGreycode);
            editor.putFloat(key, mLuminance);
            editor.commit();
            mSortedMap.put(key, mLuminance);
            mLuminanceText.getText().clear();
            dumpRecord();
        }
    }
    
    private boolean checkInvalidData() {
        Iterator iterator = mSortedMap.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry entry = (Map.Entry) iterator.next();
            Float current = new Float(entry.getValue().toString());
            // get submap from first key to target
            Iterator i = mSortedMap.subMap(mSortedMap.firstKey(), entry.getKey()).entrySet().iterator();
            Log.d(TAG, "greycode=" + entry.getKey()  + ", luminance=" + current);
            while (i.hasNext()) {
                Map.Entry e = (Map.Entry) i.next();
                Float previous = new Float(e.getValue().toString());

                if (previous.compareTo(current) >= 0) {
                    
                    Log.d(TAG, "==> compare current=" + current + ", previous=" + previous);
                    
                    mAlertDialog.setMessage("Invalid data!" +
                        "\ngreycode=" + e.getKey() + ", luminance=" + previous +
                        "\ngreycode=" + entry.getKey() + ", luminance=" + current);
                                
                    mAlertDialog.show();
                    return false;
                }
            }
        }

        return true;
    }
    
    private void saveToFile() {
        if (!checkInvalidData())
            return;
        
        Iterator iterator = mSortedMap.entrySet().iterator();
        try {
            
            FileOutputStream fos = openFileOutput(FILE_NAME, Context.MODE_PRIVATE);
            PrintWriter pw = new PrintWriter(fos);        
            while (iterator.hasNext()) {
                Map.Entry entry = (Map.Entry) iterator.next();
                Log.d(TAG, "greycode=" + entry.getKey()  + ",luminance=" + entry.getValue().toString());
                pw.println("greycode=" + entry.getKey()  + ",luminance=" + entry.getValue().toString());
            }
            pw.close();
            fos.close();
        } catch(Exception e){
            e.printStackTrace();
        }
    }
    

    private void incGreycode()
    {
        mGreycode = Math.min(mGreycode + mGreycodeStep, 255);
        setGreycode();
    }


    private void decGreycode()
    {
        if (mGreycode == 255)
            mGreycode -= (mGreycodeStep - 1);
        else
            mGreycode = Math.max(mGreycode - mGreycodeStep, 0);
        setGreycode();
    }


    private void setGreycode()
    {   
        if (mGreycode < 128) {
            mPatternText.setTextColor(Color.WHITE);
            mGreycodeText.setTextColor(Color.WHITE);
            mBrightnessText.setTextColor(Color.WHITE);
            mLuminanceText.setTextColor(Color.WHITE);
        }
        else {
            mPatternText.setTextColor(Color.BLACK);
            mGreycodeText.setTextColor(Color.BLACK);
            mBrightnessText.setTextColor(Color.BLACK);
            mLuminanceText.setTextColor(Color.BLACK);
        }
        
        mGreycodeText.setText(Integer.toString(mGreycode));

        int color = 0xFF000000 | (mGreycode << 16) | (mGreycode << 8) | mGreycode; 
        Log.d(TAG, "setGreycode: GreyCode=" + mGreycode + ", color=" + Integer.toHexString(color));
        mView.setBackgroundColor(color);
        mView.invalidate();
    }
    
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.aal_lcm_calibration, menu);
        return true;
    }
    
    @Override
    protected void onResume() {
        Log.d(TAG, "onResume..., restore brightness to " + mBrightness);
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS, mBrightness);
        //preview brightness changes at this window
        //get the current window attributes
        LayoutParams layoutpars = mWindow.getAttributes();
        //set the brightness of this window
        layoutpars.screenBrightness = mBrightness / (float)255;
        //apply attribute changes to this window
        mWindow.setAttributes(layoutpars);        
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause..., restore brightness to " + mPreBrightness);
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS, mPreBrightness);
        //preview brightness changes at this window
        //get the current window attributes
        LayoutParams layoutpars = mWindow.getAttributes();
        //set the brightness of this window
        layoutpars.screenBrightness = mPreBrightness / (float)255;
        //apply attribute changes to this window
        mWindow.setAttributes(layoutpars);    
        super.onPause();
    }

}
