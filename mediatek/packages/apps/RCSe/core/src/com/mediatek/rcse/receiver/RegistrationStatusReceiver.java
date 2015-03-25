package com.mediatek.rcse.receiver;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.widget.Toast;

import com.mediatek.rcse.activities.RegistrationStatusActivity;
import com.orangelabs.rcs.utils.logger.Logger;


/**
 * @author MTK33296
 * This Class used to receive broadcast when registration is failed first time on wifi.
 */
public class RegistrationStatusReceiver extends BroadcastReceiver {

    private Logger logger = Logger.getLogger(this.getClass().getName());

    @Override
    public void onReceive(Context context, Intent intent) {
        if (logger.isActivated()) {
            logger.debug("Registration before config Broadcast Receievd");
        }
        try {
            Intent intent_status = new Intent(context,
                    RegistrationStatusActivity.class);
            intent_status.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent_status);
        } catch (Exception e) {
            e.printStackTrace();
            logger.debug(e.getMessage());
        }
    }
}

