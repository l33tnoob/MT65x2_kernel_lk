package com.mediatek.hotknot.verifier;


import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.hotknot.verifier.R;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;

public class HotKnotAppListActivity extends Activity implements OnItemClickListener {
    protected static final String TAG = "HotKnotVerifier";
    protected   static final String PACKAGE_NAME = "com.mediatek.hotknot.verifier";

    private   static final String ITEM_STRINGS[] = { "Test HotKnot", "Test HotKnot Beam"};
    private   static final String ITEM_INTENT_STRING[] = {"HotKnotActivityA", "HotKnotBeamActivity"};
    
    private   ListView mMenuListView;
    private   TextView mAppVersion;
    private   List<String> mListData;
 
     @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hotknot_app_list);

        mMenuListView = (ListView) findViewById(R.id.ListViewHotKnotAppList);
        mMenuListView.setOnItemClickListener(this);

        mAppVersion = (TextView) findViewById(R.id.appLabel);

        mAppVersion.setText(mAppVersion.getText().toString() + ":" + getBuildDate());
        Log.i(TAG, "onCreate in HotKnotAppListActivity");
    }   

    @Override
    protected void onResume() {
        super.onResume();

        mListData = getData();
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.simple_list_item_1, mListData);
        mMenuListView.setAdapter(adapter);
    }
    
    public void onItemClick(AdapterView<?> arg0, View view, int menuId, long arg3) {

        try {
            int i = 0;
            Intent intent = new Intent();

            for(i = 0 ; i < ITEM_STRINGS.length; i++) {
                if (ITEM_STRINGS[i] == mListData.get(menuId)) {
                    intent.setClassName(this, PACKAGE_NAME + "." + ITEM_INTENT_STRING[i]);
                    Log.i(TAG, "Start activity:" + ITEM_STRINGS[i] + " inent:" + ITEM_INTENT_STRING[i]);
                    break;
                }
            }

            this.startActivity(intent);

        } catch(Exception e) {
            e.printStackTrace();

        }
    }

    private List<String> getData() {
        List<String> items = new ArrayList<String>();

        for (int i = 0; i < ITEM_STRINGS.length; i++) {
            if(i == 4)  //Remove Background Data Usage from BSP package
               continue;
            items.add(ITEM_STRINGS[i]);
        }

        return items;
    }    
    
    private String getBuildDate(){
        String buildDate = "";

        try{
            ApplicationInfo ai = getPackageManager().getApplicationInfo(getPackageName(), 0);
            ZipFile zf = new ZipFile(ai.sourceDir);
            ZipEntry ze = zf.getEntry("classes.dex");
            long time = ze.getTime();
            SimpleDateFormat sdFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
            buildDate = sdFormat.format(new java.util.Date(time));

        }catch(Exception e){
            e.printStackTrace();
        } 
        
        return buildDate;
    }
}