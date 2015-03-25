package com.mediatek.smsreg.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.CheckBox;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.smsreg.R;
import com.mediatek.smsreg.SmsRegConst;
import com.mediatek.smsreg.SmsRegReceiver;
import com.mediatek.smsreg.SmsRegService;
import com.mediatek.xlog.Xlog;

public class SendMessageAlertActivity extends Activity {
    
    private static final String TAG = "SmsReg/SendMessageAlertActivity";
    private static final int SEND_MESSAGE_NOTIFY = 1;
    private NotificationManager mNotificationManager = null;
    Notification mNotification = null;
    TextView msgTxt = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        //createDialog();
        
        mNotificationManager = (NotificationManager) this.getSystemService(Context.NOTIFICATION_SERVICE);
        showDialog(SEND_MESSAGE_NOTIFY);
        Xlog.d(TAG, "SmsReg createDialog" );
        //showDialog(id);
    }
    @Override
    protected Dialog onCreateDialog(int id) {
        Log.d(TAG, "onCreateDialog with id " + id);
        Dialog dialog = null;
        switch (id) {
            case SEND_MESSAGE_NOTIFY:
                dialog = sendMessageDialog();
                Log.d(TAG, "sendMessageDialog" + id);
                notifysendMessageNotification();
                Log.d(TAG, "notifysendMessageNotification" + id);
                break;
        }
        return dialog;
    }
    
    private void notifysendMessageNotification() {
        mNotification = new Notification();  
        mNotification.icon = R.drawable.perm_sent_mms;   
        mNotification.tickerText = getResources().getString(R.string.send_message_notification_tickerText);
        mNotification.flags = Notification.FLAG_NO_CLEAR;
        mNotification.audioStreamType= android.media.AudioManager.ADJUST_LOWER;  
        Intent intent = getIntent();
        intent.setClass(this, SendMessageAlertActivity.class); 
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);  
        mNotification.setLatestEventInfo(this, 
                getResources().getString(R.string.send_message_notification_title),
                getResources().getString(R.string.send_message_notification_msg), 
                pendingIntent);
        mNotificationManager.notify(SmsRegConst.ID_NOTIFICATION_SEND_MSG_DIALOG, mNotification);
    }
    private Dialog sendMessageDialog() {
        Log.d(TAG, "sendMessageDialog()");
        Dialog dialog = null;
        LayoutInflater inflater = LayoutInflater.from(this);
        View layout = inflater.inflate(R.layout.notify_dialog_customview, null);
        msgTxt = (TextView)layout.findViewById(R.id.message);
        msgTxt.setText(R.string.send_message_dlg_mgs);
        
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setCancelable(false);
        builder.setTitle(R.string.send_message_dlg_title);
        builder.setView(layout);  
        builder.setPositiveButton(R.string.alert_dlg_ok_button, new OnClickListener() {  
           @Override 
           public void onClick(DialogInterface dialog, int which) {
               Xlog.d(TAG, "SmsReg yes" );
               //todo update black list
               mNotificationManager.cancel(SmsRegConst.ID_NOTIFICATION_SEND_MSG_DIALOG);
               dialog.dismiss();
               responseSendMsg(true);
               SendMessageAlertActivity.this.finish();
               
               
           }
        });   
        builder.setNegativeButton(R.string.alert_dlg_cancel_button, new OnClickListener() {  
            @Override 
            public void onClick(DialogInterface dialog, int which) {
                Xlog.d(TAG, "SmsReg no" );
                //todo :send message update black list
                dialog.dismiss();
                AlertDialog.Builder confirmBuilder = new AlertDialog.Builder(SendMessageAlertActivity.this);
                confirmBuilder.setCancelable(false);
                confirmBuilder.setTitle(R.string.confirm_dlg_title);
                confirmBuilder.setMessage(R.string.confirm_dlg__msg);  

                confirmBuilder.setPositiveButton(R.string.alert_dlg_ok_button, new OnClickListener() {  
                    @Override 
                    public void onClick(DialogInterface dialog, int which) {  
                        Xlog.d(TAG, "notify yes" );
                        mNotificationManager.cancel(SmsRegConst.ID_NOTIFICATION_SEND_MSG_DIALOG);
                        dialog.dismiss();
                        responseSendMsg(false);
                        SendMessageAlertActivity.this.finish();
                    } 
                });
                confirmBuilder.setNegativeButton(R.string.alert_dlg_cancel_button, new OnClickListener() {  
                    @Override 
                    public void onClick(DialogInterface dialog, int which) {  
                        Xlog.i(TAG, "notify no" );
                        mNotificationManager.cancel(SmsRegConst.ID_NOTIFICATION_SEND_MSG_DIALOG);
                        dialog.dismiss();
                        Intent intent = getIntent();
                        SendMessageAlertActivity.this.finish();
                        intent.setAction(SmsRegConst.ACTION_CONFIRM_DIALOG_START);
                        startActivity(intent);
                        
                    } 
                });
                confirmBuilder.show();
                Xlog.d(TAG, "confirmBuilder.show();" );
                
            }
        });
        builder.setCancelable(false);
        dialog =builder.create();
        return dialog;
    }
    public void responseSendMsg(boolean result) {
        Intent intent = getIntent();
        intent.putExtra(SmsRegConst.EXTRA_IS_NEED_SEND_MSG, result);
        int slotid = intent.getIntExtra(SmsRegConst.EXTRA_SLOT_ID, -1);
        Xlog.d(TAG, " slot id is :"  + slotid);
        intent.setAction(SmsRegConst.ACTION_CONFIRM_DIALOG_END);
        intent.setClass(this, SmsRegReceiver.class);
        sendBroadcast(intent);
    }
}
