package com.mediatek.hotknot.verifier;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.hotknot.HotKnotAdapter;

import com.mediatek.hotknot.verifier.R;

import java.lang.StringBuffer;

public class HotKnotActivityRxA extends Activity{
    private static final String TAG = HotKnotAppListActivity.TAG;

    private TextView mShowData = null;
    private Context mContext;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hotknot_test_rx_app);

        ((TextView) findViewById(R.id.appLabel)).setText(this.getClass().getName());
        mShowData = (TextView) findViewById(R.id.appInfo);

        mContext = this.getBaseContext();
        
        Log.d(TAG, "onCreate: " + this.getClass().getName());
        tryHotKnotIntent(getIntent());
    }

    @Override
    public void onNewIntent(Intent intent) {
        tryHotKnotIntent(intent);
    }

    @Override  
    public boolean onTouchEvent(MotionEvent event) {  
        this.finish();
        return true;
    }


    private void tryHotKnotIntent(Intent intent) {
        if (intent.getAction().equals(HotKnotAdapter.ACTION_MESSAGE_DISCOVERED)) {
            String mimeType = intent.getType();
            String data = new String(intent.getByteArrayExtra(HotKnotAdapter.EXTRA_DATA));
            Toast.makeText(this, "Received Done", Toast.LENGTH_SHORT).show();
            Log.d(TAG, "tryHotKnotIntent: " + mimeType + ":" + data);
            
            ((TextView) findViewById(R.id.appRxLabel)).setText(mimeType);                        
            mShowData.setText(data);
        }
    }

}