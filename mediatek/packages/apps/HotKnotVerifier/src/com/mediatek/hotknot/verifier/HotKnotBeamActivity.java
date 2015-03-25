package com.mediatek.hotknot.verifier;

import android.app.Activity;
import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.net.Uri;
import android.provider.MediaStore;
import android.text.format.Time;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Toast;


import com.mediatek.hotknot.HotKnotAdapter;
import com.mediatek.hotknot.HotKnotAdapter.CreateHotKnotBeamUrisCallback;
import com.mediatek.storage.StorageManagerEx;

import com.mediatek.hotknot.verifier.R;

import java.lang.StringBuffer;
import java.io.File;
import java.util.Random;

public class HotKnotBeamActivity extends Activity implements View.OnClickListener, HotKnotAdapter.CreateHotKnotBeamUrisCallback {
    private static final String TAG = HotKnotAppListActivity.TAG;


    private static final String STRING_TABLE= "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    private static final String NON_SUPPORT = "HotKnot is not available on this device.";
    private static final int MAX_FILE_NUM = 10;

    private String   mMimeType = null;

    private TextView mFileList = null;
    private EditText mEditData = null;
    private Context mContext;
    private HotKnotAdapter mHotKnotAdapter = null;

    private static final int MESSAGE_SENT = 1;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hotknot_test_beam_app);


        ((Button) findViewById(R.id.sendBtn)).setOnClickListener(this);
        ((Button) findViewById(R.id.sendCbBtn)).setOnClickListener(this);
        ((Button) findViewById(R.id.resetBtn)).setOnClickListener(this);

        mFileList = (TextView) findViewById(R.id.appFileList);
        mEditData = (EditText) findViewById(R.id.inputText);

        mContext = this.getBaseContext();


        mHotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
        if(mHotKnotAdapter == null) {
            Log.d(TAG, "mHotKnotAdapter is null");
            mEditData.setText(NON_SUPPORT);
        } else {
            Log.d(TAG, "Get HotKnot Adapter");
            mEditData.setText(StorageManagerEx.getDefaultPath() + File.separator + "testimg.jpg");
        }
    }

    boolean isHotKnotSupported() {
        return mHotKnotAdapter != null;
    }

    @Override
    public void onClick(View v) {

        if(!isHotKnotSupported()) {
            Toast.makeText(mContext, NON_SUPPORT, Toast.LENGTH_SHORT).show();
            return;
        }

        int id = v.getId();

        switch(id) {
        case R.id.sendBtn:
            sendData();
            break;
        case R.id.sendCbBtn:
            sendCbData();
            break;
        case R.id.resetBtn:
            resetData();
            break;
        default:
            Log.e(TAG, "No match button id");
            break;
        }
    }

    public void sendData() {
        String data = mEditData.getText().toString();

        Uri[] uris = getUris();
        if(uris == null){
            return;
        }        
        mHotKnotAdapter.setHotKnotBeamUrisCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUris(uris, this);
    }

    public void sendCbData() {
        Uri[] uris = getUris();
        if(uris == null){
            return;
        }
        mHotKnotAdapter.setHotKnotBeamUrisCallback(this, this);
    }

    public void resetData(){
        Log.d(TAG, "resetData callback");
        mFileList.setText("");
        mHotKnotAdapter.setHotKnotMessage(null, this);
        mHotKnotAdapter.setHotKnotMessageCallback(null, this);
        mHotKnotAdapter.setOnHotKnotCompleteCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUrisCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUris(null, this);
    }

    private Uri[] getUris() {
        File folder = null;
        StringBuilder fileList = new StringBuilder("Sending File List:\r\n");
        Uri[] uris = null;
        String data = mEditData.getText().toString();
        Log.d(TAG, "config callback:" + data);

        try{
            folder = new File(data).getCanonicalFile();
        }catch(Exception e){
            e.printStackTrace();
        }

        if(folder != null && folder.isDirectory()) {
            Log.d(TAG, "Send by folder:" + folder);
            try {                
                File[] files = folder.listFiles();
                int count = 0;
                File[] fileUris = new File[MAX_FILE_NUM];
                if(files != null){
                    for (File inFile : files) {
                        if (!inFile.isDirectory()) {
                            count++;
                            fileUris[count-1] = inFile;
                            Log.d(TAG, "File name(" + count + "):" + inFile.getName());
                        }
                        if(count >= MAX_FILE_NUM) {
                            Log.d(TAG, "Reach maximum file number:" + MAX_FILE_NUM);
                            break;
                        }
                    }
                }else{
                    Log.e(TAG, "No list files");
                }
                
                if(count > 0) {
                    uris = new Uri[count];
                    for(int i = 0; i < count ; i++) {
                        uris[i] = Uri.fromFile(fileUris[i]);
                        fileList.append(uris[i] + "\r\n");
                    }
                    Log.d(TAG, "File counts:" + count);
                    
                    mFileList.setText(fileList.toString());
                    return uris;
                }
            } catch(Exception e) {
                e.printStackTrace();
            }
        }

        uris = new Uri[1];
        try {
            uris[0] = Uri.parse("file://" + data);
        } catch(Exception e) {
            e.printStackTrace();
            uris[0] = Uri.parse("file://mnt/sdcard/testimg.jpg");
        }

        for(Uri uri : uris){
            File f = getFilePathFromUri(uri);
            if(f == null){                
                Toast.makeText(mContext, uri + ": 1. is not existed or 2. file size is zero or 3. file format is directorory", Toast.LENGTH_SHORT).show();
                mFileList.setText("");
                return null;
            }
            fileList.append(uri + "\r\n");
        }
        
        mFileList.setText(fileList.toString());
        return uris;
    }

    /**
    * Implementation for the CreateHotKnotBeamCallback interface
    */
    @Override
    public Uri[] createHotKnotBeamUris() {
        return getUris();
    }
    
    private File getFilePathFromUri(Uri uri){
        File inputFile = null;
        String filePath = "";
        
        if(uri == null){
           Log.e(TAG, "File Uri must not be null");
           throw new IllegalArgumentException("File Uri must not be null");
        }

        String scheme = uri.getScheme();

        if(scheme != null && scheme.equalsIgnoreCase("content")){
            Cursor cursor = mContext.getContentResolver().query(uri, null, null, null, null);          
            if(cursor!= null){
                int index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                cursor.moveToFirst();
                filePath = cursor.getString(index);
                cursor.close();                
            }else{
                Log.e(TAG, "Cursor is null");
            }
        } else if(scheme != null && scheme.equalsIgnoreCase("file")){ 
        	    filePath = uri.getPath();
        }

        if(filePath.length() == 0){
           Log.e(TAG, "File path is empty");
           return null;
        }

        Log.d(TAG, "The sending path is " + filePath);
        inputFile = new File(filePath);

        if(!inputFile.exists() || inputFile.isDirectory()){
           Log.e(TAG, "File size is zero or a directory");
           return null;
        }

        return inputFile;
    }
}