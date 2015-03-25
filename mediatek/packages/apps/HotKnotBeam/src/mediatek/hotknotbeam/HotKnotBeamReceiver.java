package com.mediatek.hotknotbeam;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.hotknot.HotKnotAdapter;

import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.provider.DocumentsContract.Document;
import android.provider.DocumentsContract.Root;
import android.provider.DocumentsProvider;


public class HotKnotBeamReceiver extends BroadcastReceiver {
    static final String TAG = "HotKnotBeamReceiver";
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {

        Intent serviceIntent = new Intent(context, HotKnotBeamService.class);
        mContext = context;
        

        // TODO Auto-generated method stub
        if(intent.getAction().equalsIgnoreCase(WifiManager.WIFI_STATE_CHANGED_ACTION))
        {
            int state = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_DISABLED);
            Log.d(TAG, "Start/Stop hotknotbeam service on HotKnot enable/disable:" + state);

            if(state == WifiManager.WIFI_STATE_DISABLED) {
                context.stopService(serviceIntent);
            } else if(state == WifiManager.WIFI_STATE_ENABLED) {
                context.startService(serviceIntent);
            }
        } else if(intent.getAction().equalsIgnoreCase(HotKnotAdapter.ACTION_ADAPTER_STATE_CHANGED)) {

            int state = intent.getIntExtra(HotKnotAdapter.EXTRA_ADAPTER_STATE, HotKnotAdapter.STATE_DISABLED);
            Log.d(TAG, "Start/Stop hotknotbeam service on HotKnot enable/disable:" + state);

            if(state == HotKnotAdapter.STATE_DISABLED) {
                context.stopService(serviceIntent);
            } else if(state == HotKnotAdapter.STATE_ENABLED) {
                context.startService(serviceIntent);
            }
        } else if(intent.getAction().equalsIgnoreCase(HotKnotBeamService.HOTKNOT_DL_COMPLETE)) {
            Log.d(TAG, "Show can't open toast");
            //Toast.makeText(context, R.string.download_no_application_title, Toast.LENGTH_SHORT).show();
            showDocumentUI();
        }
    }


    private void showDocumentUI(){
        String path = ".";
        final Intent intent = new Intent(DocumentsContract.ACTION_MANAGE_ROOT);
        intent.setData(DocumentsContract.buildRootUri(HotKnotBeamConstants.STORAGE_AUTHORITY, HotKnotBeamConstants.STORAGE_ROOT_ID));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(Intent.EXTRA_LOCAL_ONLY, true);
        intent.putExtra(DocumentsContract.EXTRA_SHOW_ADVANCED, false);
        mContext.startActivity(intent);
    }    
}