package com.hissage.util.data;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
//M:Activation Statistics
import com.hissage.util.statistics.NmsStatistics;

public class NmsAlertDialogUtils {

    private Context mContext;
    private long selectSimId;

    class MyURLSpan extends ClickableSpan {
        @Override
        public void onClick(View widget) {
            Intent intent = new Intent();
            intent.setAction(NmsIpMessageConsts.ACTION_TERM);
            intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                    NmsIpMessageConsts.CLASS_NAME_TERM);
            intent.putExtra(NmsConsts.SIM_ID, selectSimId);
            mContext.startActivity(intent);
        }
    }

    public static void showDialog(Context context, int rsTitle, int rsIcon, int rsMsg, int rsOK,
            int rsCancel, DialogInterface.OnClickListener listener,
            DialogInterface.OnClickListener cancellistener) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(rsTitle).setIcon(rsIcon).setMessage(rsMsg)
                .setPositiveButton(rsOK, listener).setNegativeButton(rsCancel, cancellistener)
                .create().show();
    }

    public static void showDialog(Context context, CharSequence rsTitle, int rsIcon,
            CharSequence rsMsg, CharSequence rsOK, CharSequence rsCancel,
            DialogInterface.OnClickListener listener, DialogInterface.OnClickListener cancellistener) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(rsTitle).setIcon(rsIcon).setMessage(rsMsg)
                .setPositiveButton(rsOK, listener).setNegativeButton(rsCancel, cancellistener)
                .create().show();
    }

    public static boolean checkSimCardActivty(final Context context, final Context nmsContext,
            final int simId, final int requestCode, int always) {
        SNmsSimInfo info = NmsIpMessageApi.getInstance(context).nmsGetSimInfoViaSimId(simId);

        // enable current sim card
        if (info != null && info.status == NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED) {
            NmsAlertDialogUtils.showDialog(context,
                    nmsContext.getText(R.string.STR_NMS_ISMS_ENABLE_TITLE), 0,
                    nmsContext.getText(R.string.STR_NMS_ISMS_ENABLE_CONTENT),
                    nmsContext.getText(R.string.STR_NMS_ENABLE),
                    nmsContext.getText(R.string.STR_NMS_CANCEL),
                    new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            NmsIpMessageApi.getInstance(context).nmsEnableIpService(simId);
                            NmsStartActivityApi.nmsStartCreateGroupChatActivity(context,
                                    requestCode, simId);
                        }
                    }, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            dialog.dismiss();
                        }
                    });
            return true;
        }

        // Current not activated and another activated
        if (info != null && info.status <= NmsSimActivateStatus.NMS_SIM_STATUS_NOT_ACTIVATED) {
            if (always == -1) {
                NmsAlertDialogUtils activity = new NmsAlertDialogUtils();
                activity.showActivitionDlg(context, nmsContext, simId);
                // NmsStartActivityApi.nmsStartActivitionActivity(context,
                // simId);
                // M:Activation Statistics
                NmsIpMessageApi.getInstance(context).nmsAddActivatePromptStatistics(
                        NmsIpMessageConsts.NmsUIActivateType.OTHER);
                return true;
            }
            int sim_Id = 0;
            sim_Id = (int) NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                    NmsConsts.SIM_CARD_SLOT_2);
            if (simId == sim_Id) {
                sim_Id = (int) NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                        NmsConsts.SIM_CARD_SLOT_1);
            }

            SNmsSimInfo info_sim2 = NmsIpMessageApi.getInstance(context).nmsGetSimInfoViaSimId(
                    simId);

            if (info_sim2 != null
                    && info_sim2.status == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                final int nextSimId = sim_Id;
                NmsAlertDialogUtils.showDialog(context,
                        nmsContext.getText(R.string.STR_NMS_SWITCH_TITLE), 0,
                        nmsContext.getText(R.string.STR_NMS_SWITCH_MESSAGE),
                        nmsContext.getText(R.string.STR_NMS_SWITCH),
                        nmsContext.getText(R.string.STR_NMS_CANCEL),
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                // TODO Auto-generated method stub
                                NmsPlatformAdapter.getInstance(context).setCurrentSimId(nextSimId);
                                NmsStartActivityApi.nmsStartCreateGroupChatActivity(context,
                                        requestCode, nextSimId);
                            }
                        }, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                // TODO Auto-generated method stub
                                dialog.dismiss();
                            }
                        });
                return true;
            } else if (info_sim2 != null
                    && info_sim2.status == NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED) {
                // M:Activation Statistics
                NmsStartActivityApi.nmsStartActivitionActivity(context, simId,
                        NmsIpMessageConsts.NmsUIActivateType.OTHER);
                return true;
            }
        }

        return false;
    }

    protected void showActivitionDlg(final Context context, final Context nmsContext,
            final long sim_id) {
        mContext = context;
        selectSimId = sim_id;

        LayoutInflater factory = LayoutInflater.from(nmsContext);
        final View view = factory.inflate(R.layout.alert_dialog_text_view, null);

        TextView textView = (TextView) view.findViewById(R.id.term_textview);

        String termContent = nmsContext.getString(R.string.STR_NMS_TERM_WARN_ACTIVATE);
        SpannableString ss = new SpannableString(termContent);

        ss.setSpan(new URLSpan("noting"),
                termContent.indexOf(nmsContext.getString(R.string.STR_NMS_MENU_LICENSE_AGREEMENT)),
                termContent.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

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
        // M:Activation Statistics
        NmsStatistics.incKeyVal(NmsStatistics.KEY_OTHER_ACTIVATE_PROMPT);
        final AlertDialog mActivationDlg = new AlertDialog.Builder(context)
                .setTitle(nmsContext.getString(R.string.STR_NMS_ACTIVE))
                .setView(view)
                .setPositiveButton(nmsContext.getString(R.string.STR_NMS_AGREE_AND_CONTINUE),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                // M:Activation Statistics
                                NmsStartActivityApi.nmsStartActivitionActivity(context, sim_id,
                                        NmsIpMessageConsts.NmsUIActivateType.OTHER);
                            }
                        }).setOnCancelListener(new OnCancelListener() {

                    @Override
                    public void onCancel(DialogInterface dialog) {

                    }
                }).create();
        mActivationDlg.show();
    }
    public static List<Map<String, Object>> entries =null;
    public static List<Map<String, Object>>  getList(){
        return entries;
    }
    public static void showSelectSimCardDialog(Context context, Context nmsContext,
            DialogInterface.OnClickListener listener) {
        entries = new ArrayList<Map<String, Object>>();
        int simId1 = (int) NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_1);
        int simId2 = (int) NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_2);

        String shortNumber;
        SNmsSimInfo info1 = NmsIpMessageApi.getInstance(context).nmsGetSimInfoViaSimId(simId1);
        SNmsSimInfo info2 = NmsIpMessageApi.getInstance(context).nmsGetSimInfoViaSimId(simId2);
        HashMap<String, Object> entry1 = new HashMap<String, Object>();
        if (info1 != null) {
            entry1.put("simID",simId1);
            entry1.put("simIcon", NmsPlatformAdapter.getInstance(context).getSimColor(simId1));
            entry1.put("simStatus",
                    info1.status == SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED ? 1
                            : 0);
            entry1.put("simName", NmsPlatformAdapter.getInstance(context).getSimName(simId1));

            if (info1.number.length() <= 4) {
                shortNumber = info1.number;
            } else {
                shortNumber = info1.number.substring(info1.number.length() - 4);
            }

            entry1.put("simShortNumber", shortNumber);
            entry1.put("simNumber", info1.number);
            entries.add(entry1);
        }
        if (info2 != null) {
            HashMap<String, Object> entry2 = new HashMap<String, Object>();
            entry2.put("simID",simId2);
            entry2.put("simIcon", NmsPlatformAdapter.getInstance(context).getSimColor(simId2));
            entry2.put("simStatus",
                    info2.status == SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED ? 1
                            : 0);
            entry2.put("simName", NmsPlatformAdapter.getInstance(context).getSimName(simId2));
            if (info2.number.length() <= 4) {
                shortNumber = info2.number;
            } else {
                shortNumber = info2.number.substring(info2.number.length() - 4);
            }
            entry2.put("simShortNumber", shortNumber);
            entry2.put("simNumber", info2.number);
            entries.add(entry2);
        }

        final SimpleAdapter a = new SimpleAdapter(nmsContext, entries, R.layout.sim_selector,
                new String[] { "simIcon", "simStatus", "simName", "simShortNumber", "simNumber" },
                new int[] { R.id.sim_icon, R.id.sim_status, R.id.sim_name, R.id.sim_number_short,
                        R.id.sim_number });
        SimpleAdapter.ViewBinder viewBinder = new SimpleAdapter.ViewBinder() {

            @Override
            public boolean setViewValue(View view, Object data, String textRepresentation) {
                // TODO Auto-generated method stub
                if (view instanceof ImageView) {
                    if (view.getId() == R.id.sim_icon) {
                        Integer a = (Integer) data;
                        ImageView simicon = (ImageView) view.findViewById(R.id.sim_icon);
                        simicon.setBackgroundResource(a);
                    } else if (view.getId() == R.id.sim_status) {
                        ImageView simstatus = (ImageView) view.findViewById(R.id.sim_status);
                        if (((Integer) data).intValue() == 1) {
                            simstatus.setVisibility(View.VISIBLE);
                        } else {
                            simstatus.setVisibility(View.GONE);
                        }
                    }
                    return true;
                }
                return false;
            }
        };

        a.setViewBinder(viewBinder);
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(nmsContext.getText(R.string.STR_NMS_SIM_SELECT_TITLE));
        builder.setCancelable(true);
        builder.setAdapter(a, listener);
        builder.create().show();
    }
}
