package com.hissage.receiver.app;

import com.hissage.R;
import com.hissage.struct.SNmsAssertAlertMsgData;
import com.hissage.util.log.NmsLog;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class NmsAssertAlertReceiver extends BroadcastReceiver {

    public static final String INTENT_ACTION = "com.hissage.assertalert";
    public static final String INTENT_KEY = "data";

    @Override
    public void onReceive(Context context, Intent intent) {
        try {
            if (intent.getAction().equals(NmsAssertAlertReceiver.INTENT_ACTION)) {
                SNmsAssertAlertMsgData data = (SNmsAssertAlertMsgData) intent.getExtras().get(
                        NmsAssertAlertReceiver.INTENT_KEY);
                if (data != null) {
                    LayoutInflater inflater = (LayoutInflater) context
                            .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                    View layout = inflater.inflate(R.layout.assert_alert_layout, null);

                    TextView text = (TextView) layout.findViewById(R.id.assert_text);
                    text.setText("Assert in " + data.fileName + " line " + data.line);

                    Toast toast = new Toast(context);
                    toast.setGravity(Gravity.CENTER_VERTICAL, 0, 0);
                    toast.setDuration(Toast.LENGTH_LONG);
                    toast.setView(layout);
                    toast.show();
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }
}
