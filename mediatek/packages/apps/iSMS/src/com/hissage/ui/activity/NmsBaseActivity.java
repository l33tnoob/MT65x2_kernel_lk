package com.hissage.ui.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
//M: Activation Statistics
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.config.NmsCommonUtils;
//M: Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.data.NmsConsts;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;

public class NmsBaseActivity extends Activity {

    public static final int WELCOME = 0;
    public static final int ACTIVATE = 0;

    private AlertDialog mActivationDlg = null;
    private long mSimId = NmsConsts.INVALID_SIM_ID;
    protected boolean mFollowSysScrOri = true;

    public void onCreate(Bundle savedInstanceState) {
        // TODO jhnie, follow this config via System or SMS settings.
        // if(NmsConfig.NmsQueryLockRotateScrFlag()){
        // setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        // }else{
        // setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR);
        // }
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        // TODO jhnie, follow this config via System settings.
        // if(NmsConfig.NmsQueryLockRotateScrFlag() || !mFollowSysScrOri){
        // setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        // }else{
        // setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR);
        // }
        super.onResume();

        NmsUpgradeManager.tryToShowUpgradePrompt(this);
    }

    protected boolean checkSdSimNetworkStatus(boolean needCheckSDCard) {
        if (!isSimCardReady()) {
            Toast.makeText(this, R.string.STR_NMS_NO_SIM, Toast.LENGTH_SHORT).show();
            return false;
        }

        if (!NmsCommonUtils.isNetworkReady(this)) {
            Toast.makeText(this, R.string.STR_NMS_NO_CONNECTION, Toast.LENGTH_SHORT).show();
            return false;
        }

        if (needCheckSDCard && !isSDCardReady()) {
            Toast.makeText(this, R.string.STR_NMS_NO_SDCARD, Toast.LENGTH_SHORT).show();
            return false;
        }
        return true;
    }
    
    protected boolean isNetworkReady(){
        return NmsCommonUtils.isNetworkReady(this);
    }

    protected boolean isSDCardReady() {
        String sdStatus = Environment.getExternalStorageState();

        if (TextUtils.isEmpty(sdStatus)) {
            return false;
        }

        return sdStatus.equals(android.os.Environment.MEDIA_MOUNTED);
    }

    protected boolean isSimCardReady() {
        for (int i = 0; i < NmsConsts.SIM_CARD_COUNT; ++i) {
            long simId = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(i);
            if (simId > 0) {
                return true;
            }
        }
        return false;
    }

    private class MyURLSpan extends ClickableSpan {
        @Override
        public void onClick(View widget) {
            if (mActivationDlg != null) {
                mActivationDlg.dismiss();
                mActivationDlg = null;
            }
            Intent i = new Intent(NmsBaseActivity.this, NmsTermActivity.class);
            i.putExtra(NmsConsts.SIM_ID, mSimId);
            startActivity(i);
        }
    }
	//M: Activation Statistics
    protected void showActivitionDlg(final long sim_id, int mode, final int type) {
        mSimId = sim_id;

        LayoutInflater factory = LayoutInflater.from(this);
        final View view = factory.inflate(R.layout.alert_dialog_text_view, null);

        TextView textView = (TextView) view.findViewById(R.id.term_textview);

        String termContent = getString(WELCOME == mode ? R.string.STR_NMS_TERM_WARN_WELCOME
                : R.string.STR_NMS_TERM_WARN_ACTIVATE);
        SpannableString ss = new SpannableString(termContent);

        ss.setSpan(new URLSpan("noting"),
                termContent.indexOf(getString(R.string.STR_NMS_MENU_LICENSE_AGREEMENT)), termContent.length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

        ((TextView) textView).setText(ss);

        textView.setMovementMethod(LinkMovementMethod.getInstance());
        CharSequence text = textView.getText();
        if (text instanceof Spannable) {
            int end = text.length();
            Spannable sp = (Spannable) textView.getText();
            URLSpan[] urls = sp.getSpans(0, end, URLSpan.class);
            SpannableStringBuilder style = new SpannableStringBuilder(text);
            style.clearSpans();// should clear old spans
            for (URLSpan url : urls) {
                MyURLSpan myURLSpan = new MyURLSpan();
                style.setSpan(myURLSpan, sp.getSpanStart(url), sp.getSpanEnd(url),
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            textView.setText(style);
        }
        //M: Activation Statistics
        NmsIpMessageApiNative.nmsAddActivatePromptStatistics(type) ;

        mActivationDlg = new AlertDialog.Builder(this)
                .setTitle(
                        WELCOME == mode ? R.string.STR_NMS_WELCOME_ACTIVE : R.string.STR_NMS_ACTIVE)
                .setView(view)
                .setPositiveButton(R.string.STR_NMS_AGREE_AND_CONTINUE,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
								//M: Activation Statistics
                                NmsStartActivityApi.nmsStartActivitionActivity(
                                        NmsBaseActivity.this, (int) sim_id, type);
                            }
                        }).setOnCancelListener(new OnCancelListener() {

                    @Override
                    public void onCancel(DialogInterface dialog) {
                        cancelActivate();
                    }
                }).create();
        mActivationDlg.show();

    }

    public void cancelActivate() {

    }

    public void openConversationList() {
        if (NmsSMSMMSManager.getInstance(this).isExtentionFieldExsit() == 1) {
            Intent intent = new Intent();
            intent.setClassName("com.android.mms", "com.android.mms.ui.ConversationList");
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    }
}
