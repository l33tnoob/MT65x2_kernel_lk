package com.mediatek.aaltool;

import android.os.Bundle;
import android.provider.Settings;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.EditText;
import android.widget.Button;
import android.view.KeyEvent;
import android.content.SharedPreferences;
import java.util.Map;
import java.util.TreeMap;
import java.util.Comparator;
import java.util.Iterator;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.DataOutputStream;

public class AALALSCalibration extends Activity implements SensorEventListener {
    private static final String TAG = "AALTool";
    private static final String PREFS_NAME = "als.cal";
    private static final String FILE_NAME = "aal_als_cal.cfg";
    private SensorManager mSensorManager;
    private Sensor mLightSensor;
    private TextView mLightSensorText;
    private TextView mLightSensorDataText;
    private TextView mRecordDataText;
    private EditText mLuminanceText;
    private Button mSaveButton;
    private Button mClearButton;	
    private SharedPreferences mPreferences;
    private TreeMap mSortedMap;
    private int mALSRawData = 0;
    private float mALS = 0.0f;
    private float mLuminance = 0;
    private AlertDialog mAlertDialog;
    
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
        setContentView(R.layout.aal_als_calibration);
        
        //get the current window  
        mWindow = getWindow();
        
        Log.d(TAG, "onCreate...");
        
        changePerm();
        
        mSensorManager = (SensorManager) this.getSystemService(SENSOR_SERVICE);
        mLightSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
        mLightSensorText = (TextView) this.findViewById(R.id.lightsensor);
        if (mLightSensor == null){
            mLightSensorText.setText("No Light Sensor!");
        } else{
            mLightSensorText.setText(mLightSensor.getName());
        }
        mLightSensorDataText = (TextView) this.findViewById(R.id.lightsensorData);
        mRecordDataText = (TextView) this.findViewById(R.id.recordData);
        
        mLuminanceText = (EditText) this.findViewById(R.id.editTextLuminance);
        mLuminanceText.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
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

        mClearButton = (Button)findViewById(R.id.buttonClear);
        mClearButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                mSortedMap.clear();
                mRecordDataText.setText("");
                SharedPreferences.Editor editor = mPreferences.edit();
                editor.clear();
                editor.commit();
            }
        });

        mPreferences = getSharedPreferences(PREFS_NAME, 0);
        mSortedMap = new TreeMap(new MyComparator2());
        mSortedMap.putAll(mPreferences.getAll());
        dumpRecord();
    }

    private void changePerm() {
        Process chperm;
        try {
            chperm = Runtime.getRuntime().exec("su");
            DataOutputStream os =  new DataOutputStream(chperm.getOutputStream());
            os.writeBytes("chmod 777 /dev/als_ps\n");
            os.flush();

            os.writeBytes("exit\n");
            os.flush();
            chperm.waitFor();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void dumpRecord() {
        Iterator iterator = mSortedMap.entrySet().iterator();
        mRecordDataText.setText("");
        while (iterator.hasNext()) {
            Map.Entry entry = (Map.Entry) iterator.next();
            Log.d(TAG, "als=" + entry.getKey()  + ",value=" + entry.getValue().toString());
            mRecordDataText.setText("ALS=" + entry.getKey() + ", luminance=" + entry.getValue().toString() + 
                    "\n" + mRecordDataText.getText().toString());
        }
    }

    private void saveLuminance() {
        if (mLuminanceText.getText().length() > 0) {
            mLuminance = Float.parseFloat(mLuminanceText.getText().toString());
            Log.d(TAG, "saveLuminance: ALS=" + mALSRawData+ ", luminance=" +  String.valueOf(mLuminance));
            SharedPreferences.Editor editor = mPreferences.edit();
            String key = String.valueOf(mALSRawData);
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
            Log.d(TAG, "als=" + entry.getKey()  + ", value=" + current);
            while (i.hasNext()) {
                Map.Entry e = (Map.Entry) i.next();
                Float previous = new Float(e.getValue().toString());

                if (previous.compareTo(current) >= 0) {
                    
                    Log.d(TAG, "==> compare current=" + current + ", previous=" + previous);
                    
                    mAlertDialog.setMessage("Invalid data!" +
                        "\nals=" + e.getKey() + ", value=" + previous +
                        "\nals=" + entry.getKey() + ", value=" + current);

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
                Log.d(TAG, "als=" + entry.getKey()  + ",value=" + entry.getValue().toString());
                pw.println("als=" + entry.getKey()  + ",value=" + entry.getValue().toString());
            }
            pw.close();
            fos.close();
        } catch(Exception e){
            e.printStackTrace();
        }
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.aal_als_calibration, menu);
        return true;
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "registerListener...");
        if (mLightSensor != null) {
            mSensorManager.registerListener(this, mLightSensor, 
                    SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "unregisterListener...");
        if (mLightSensor != null) {
            mSensorManager.unregisterListener(this, mLightSensor);
        }
    }
    
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        // TODO Auto-generated method stub
        mALS = event.values[0];
        mALSRawData = nGetALSRawData();
        if (mALSRawData >= 0)
            mLightSensorDataText.setText("ALS RAW = "+ mALSRawData + ". Value = " + event.values[0]);
        else
            mLightSensorDataText.setText("Fail to get RAW data! Value = " + event.values[0]);
    }

    static {
        System.loadLibrary("aaltool_jni");
    }

    private native int nGetALSRawData();
        
}
