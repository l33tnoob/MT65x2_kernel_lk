package com.mediatek.hotknot.verifier;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.format.Time;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;


import com.mediatek.hotknot.HotKnotAdapter;
import com.mediatek.hotknot.HotKnotAdapter.CreateHotKnotMessageCallback;
import com.mediatek.hotknot.HotKnotAdapter.OnHotKnotCompleteCallback;
import com.mediatek.hotknot.HotKnotMessage;

import com.mediatek.hotknot.verifier.R;

import java.lang.StringBuffer;
import java.util.Random;

public class HotKnotActivityA extends Activity implements View.OnClickListener, HotKnotAdapter.OnHotKnotCompleteCallback, HotKnotAdapter.CreateHotKnotMessageCallback,
            RadioGroup.OnCheckedChangeListener{
    private static final String TAG = HotKnotAppListActivity.TAG;
    
    
    private static final String STRING_TABLE= "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    private static final String NON_SUPPORT = "HotKnot is not available on this device.";

    private String   mMimeType = null;
        
    private EditText mNumData = null;
    private EditText mEditData = null;    
    private Context mContext;
    private HotKnotAdapter mHotKnotAdapter = null;

    private static final int MESSAGE_SENT = 1;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hotknot_test_app);
        
        mMimeType = HotKnotAppListActivity.PACKAGE_NAME + "/" + this.getClass().getName() + "A";
        
        ((TextView) findViewById(R.id.appLabel)).setText(mMimeType);
        ((TextView) findViewById(R.id.appActionLabel)).setText("Aciton:None");
        
        ((Button) findViewById(R.id.fillBtn)).setOnClickListener(this);
        ((Button) findViewById(R.id.sendBtn)).setOnClickListener(this);
        ((Button) findViewById(R.id.sendCbBtn)).setOnClickListener(this);
        ((Button) findViewById(R.id.resetBtn)).setOnClickListener(this);

        ((RadioGroup) findViewById(R.id.testGroup)).setOnCheckedChangeListener(this);
        
        mNumData  = (EditText) findViewById(R.id.inputNum);
        mEditData = (EditText) findViewById(R.id.inputText);

        mNumData.setText("50");

        mContext = this.getBaseContext();
                

        //Load default values
        generateData();
        
        mHotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
        if(mHotKnotAdapter == null){
            Log.d(TAG, "mHotKnotAdapter is null");
            mEditData.setText(NON_SUPPORT);            
        }else{
            Log.d(TAG, "Get HotKnot Adapter");            
            mHotKnotAdapter.setOnHotKnotCompleteCallback(this, this);            
        }
    }

    boolean isHotKnotSupported(){
        return mHotKnotAdapter != null;
    }
    
    @Override
    public void onClick(View v){
        
        if(!isHotKnotSupported()){
            Toast.makeText(mContext, NON_SUPPORT, Toast.LENGTH_SHORT).show();
            return;
        }
                        
        int id = v.getId();    
        
        switch(id){
            case R.id.fillBtn:
                generateData();
            break;
            case R.id.sendBtn:
                sendData();
                ((TextView) findViewById(R.id.appActionLabel)).setText("Aciton:Send");
            break;
            case R.id.sendCbBtn:
                sendCbData();
                ((TextView) findViewById(R.id.appActionLabel)).setText("Aciton:Send by CallBack");
            break;
            case R.id.resetBtn:
                resetData();
                ((TextView) findViewById(R.id.appActionLabel)).setText("Aciton:None");
            break;
            default:
                Log.e(TAG, "No match button id");
            break;            
        }
    }
    
    public void sendData(){
       String data = mEditData.getText().toString();
       Log.d(TAG, "sendData:" + data);
       Toast.makeText(mContext, mMimeType, Toast.LENGTH_SHORT).show();
       mHotKnotAdapter.setHotKnotMessageCallback(null, this);
       mHotKnotAdapter.setHotKnotMessage(new HotKnotMessage(mMimeType, data.getBytes()), this);
        mHotKnotAdapter.setOnHotKnotCompleteCallback(this, this);
    }
    
    public void sendCbData(){
        mHotKnotAdapter.setHotKnotMessageCallback(this, this);
        mHotKnotAdapter.setOnHotKnotCompleteCallback(this, this);
    }
    
    public void resetData(){
        Log.d(TAG, "resetData callback");
        mEditData.setText("");
        mHotKnotAdapter.setHotKnotMessage(null, this);
        mHotKnotAdapter.setHotKnotMessageCallback(null, this);
        mHotKnotAdapter.setOnHotKnotCompleteCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUrisCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUris(null, this);        
    }

    public void generateData(){
        Random r = new Random();

        int totalNum = 50;
        String numTxt = ((EditText) findViewById(R.id.inputNum)).getText().toString();

        if(numTxt.length() <= 0){
            Toast.makeText(mContext, "The size of content is not in range (0-1024)", Toast.LENGTH_SHORT).show();
            Log.e(TAG, "The size of content is not in range (0-1024)");
            return;            
        }

        try{
            totalNum = Integer.parseInt(numTxt);            
        }catch(Exception e){
            e.printStackTrace();            
        }
        
        if(totalNum > 1024 || totalNum < 0){
            Toast.makeText(mContext, "The size of content is not in range (0-1024)", Toast.LENGTH_SHORT).show();
            Log.e(TAG, "The size of content is not in range (0-1024)");
            return;
        }
        
        StringBuffer str = new StringBuffer("");
        for(int i = 0; i < totalNum; i++){
            str.append(STRING_TABLE.charAt(r.nextInt(62)));
        }
        mEditData.setText(str.toString());
    }
    
    /**
    * Implementation for the CreateHotKnotMessageCallback interface
     */
    @Override
    public HotKnotMessage createHotKnotMessage() {
        Log.d(TAG, "createHotKnotMessage");
        Time time = new Time();
        time.setToNow();
        String text = ("Beam me up!\n\n" + "Beam Time: " + time.format("%H:%M:%S"));
        HotKnotMessage msg = new HotKnotMessage(mMimeType, text.getBytes());        
        return msg;
    }
    
    /**
    * Implementation for the OnNdefPushCompleteCallback interface
    */
    @Override
    public void onHotKnotComplete(int reason) {
        // A handler is needed to send messages to the activity when this
        // callback occurs, because it happens from a binder thread
        Log.d(TAG, "onHotKnotComplete:" + reason);
        Message msg = mHandler.obtainMessage();
        msg.what = MESSAGE_SENT;
        msg.arg1 = reason;
        mHandler.sendMessage(msg);
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId){
        switch(checkedId){
            case R.id.testA:
                mMimeType = HotKnotAppListActivity.PACKAGE_NAME + "/" + this.getClass().getName() + "A";
                break;
            case R.id.testB:
                mMimeType = HotKnotAppListActivity.PACKAGE_NAME + "/" + this.getClass().getName() + "B";
                break;
            case R.id.testC:
                mMimeType = HotKnotAppListActivity.PACKAGE_NAME + "/" + this.getClass().getName() + "C";
               break;                
            case R.id.testD:
               mMimeType =  HotKnotAppListActivity.PACKAGE_NAME + "/" + this.getClass().getName() + "D";
               break;            
            default:                
            break;
        }

        ((TextView) findViewById(R.id.appLabel)).setText(mMimeType);
        Log.d(TAG, "Mime Type is:" + mMimeType);
    }
    
    /** This handler receives a message from onHotKnotComplete */
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                 case MESSAGE_SENT:
                   int reason = msg.arg1;
                   if(reason == 0) {
                       Toast.makeText(getApplicationContext(), "Message sent!", Toast.LENGTH_LONG).show();
                   } else {
                       Toast.makeText(getApplicationContext(), "Message can't be sent", Toast.LENGTH_LONG).show();
                   }
                     break;
                   }
            }
        };            
}