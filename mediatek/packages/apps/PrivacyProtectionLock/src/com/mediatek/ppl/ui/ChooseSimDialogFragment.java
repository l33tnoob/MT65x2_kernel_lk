package com.mediatek.ppl.ui;

import android.app.Activity;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.ListView;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.ppl.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

public class ChooseSimDialogFragment extends DialogFragment {
    public static final String ARG_KEY_ITEMS = "items";
    public static final String ARG_KEY_VALUES = "values";
    public static final String TAG = "PPL/ChooseSim";

    public static interface ISendMessage {
        void sendMessage(int simId);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Activity activity = getActivity();
        Bundle args = getArguments();
        String[] items = args.getStringArray(ARG_KEY_ITEMS);
        final int[] values = args.getIntArray(ARG_KEY_VALUES);

        List<SimInfoRecord> simItem =  SimInfoManager.getInsertedSimInfoList(this.getActivity());
        Log.i(TAG, "simItem: " + simItem.size());

        // sort the unordered list
        Collections.sort(simItem, new SimInfoComparable());

        if (simItem.size() == items.length) {
            List<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
            for (int i = 0; i < simItem.size(); i++) {
                HashMap<String, Object> map = new HashMap<String, Object>();
                map.put("ItemImage", getSimIndicator(this.getActivity(), simItem.get(i).mSimSlotId));
                map.put("ItemTitle", simItem.get(i).mDisplayName);
                map.put("Color", simItem.get(i).mColor);
                map.put("SimId", values[i]);

                Log.i(TAG, "mSimSlotId: " + simItem.get(i).mSimSlotId);
                Log.i(TAG, "mDisplayName: " + simItem.get(i).mDisplayName);
                Log.i(TAG, "mColor: " + simItem.get(i).mColor);
                Log.i(TAG, "values: " + values[i]);

                data.add(map);
            }

            ChooseDialog dialog = new ChooseDialog(this.getActivity(), data);
            return dialog;

        } else {
            Builder builder = new Builder(activity);
            builder.setTitle(R.string.title_choose_sim);

            builder.setNegativeButton(android.R.string.cancel, null);
            builder.setItems(items, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    ((ISendMessage) activity).sendMessage(values[which]);
                }
            });

            return builder.create();
        }
    }

    private class SimInfoComparable implements Comparator<SimInfoRecord> {
        @Override
         public int compare(SimInfoRecord sim1, SimInfoRecord sim2) {
            return sim1.mSimSlotId - sim2.mSimSlotId;
         }
     }

    private class ChooseDialog extends Dialog  {

        public ChooseDialog(final Context context, final List<HashMap<String, Object>> data) {
            super(context);
            setContentView(R.layout.choose_sim);
            setTitle(R.string.title_choose_sim);

            ListView listView = (ListView) findViewById(R.id.list_choose_sim);

            MyBaseAdapter adapter = new MyBaseAdapter(context, data);
            listView.setAdapter(adapter);

            listView.setOnItemClickListener(new OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> arg0, View v, int position, long id) {
                    ((ISendMessage) context).sendMessage((Integer) data.get(position).get("SimId"));
                }
            });
        }
    }

    private class MyBaseAdapter extends BaseAdapter {
        private Context mContext;
        private List<HashMap<String, Object>> mDataList;

        public MyBaseAdapter(Context context, List<HashMap<String, Object>> data) {
            mContext = context;
            mDataList = data;
        }

        @Override
        public int getCount() {
            return mDataList.size();
        }

        @Override
        public HashMap<String, Object> getItem(int position) {
            return mDataList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.list_item, null);
            ImageView image = (ImageView) convertView.findViewById(R.id.ItemImage);
            TextView title = (TextView) convertView.findViewById(R.id.ItemTitle);

            title.setText((String) getItem(position).get("ItemTitle"));
            setSimIndicatorIcon(image, (Integer) getItem(position).get("ItemImage"));
            setSimBackgroundColor(image, (Integer) getItem(position).get("Color"));

            image.setScaleType(ScaleType.CENTER);
            return convertView;
        }
    }

    private void setSimIndicatorIcon(ImageView imageView, int indicator) {
        boolean isVisible = true;
        if (indicator == PhoneConstants.SIM_INDICATOR_UNKNOWN ||
            indicator == PhoneConstants.SIM_INDICATOR_NORMAL) {
            isVisible = false;
            Log.i(TAG, "indicator = " + indicator + "unable to show indicator icon");
        } else {
            int res = getStatusResource(indicator);
            imageView.setImageResource(res);
        }
        imageView.setVisibility(isVisible ? View.VISIBLE : View.GONE);
    }

    private void setSimBackgroundColor(ImageView imageView, int colorId) {
        boolean isVisible = true;
        if (colorId >= 0) {
            int resColor = getSimColorResource(colorId);
            if (resColor >= 0) {
                imageView.setBackgroundResource(resColor);
            } else {
                isVisible = false;
                Log.i(TAG, "wrong colorId unable to get color for sim colorId = " + colorId + " resColor = " + resColor);
            }
        } else {
            isVisible = false;
            Log.i(TAG, "colorId < 0 not correct");
        }
        imageView.setVisibility(isVisible ? View.VISIBLE : View.GONE);
    }

    /**
     * Get the pic from framework according to sim indicator state 
     * @param state sim indicator state
     * @return the pic res from mediatek framework
     */
    private int getStatusResource(int state) {
        int resId;
        switch (state) {
        case PhoneConstants.SIM_INDICATOR_RADIOOFF:
            resId = com.mediatek.internal.R.drawable.sim_radio_off;
            break;
        case PhoneConstants.SIM_INDICATOR_LOCKED:
            resId = com.mediatek.internal.R.drawable.sim_locked;
            break;
        case PhoneConstants.SIM_INDICATOR_INVALID:
            resId = com.mediatek.internal.R.drawable.sim_invalid;
            break;
        case PhoneConstants.SIM_INDICATOR_SEARCHING:
            resId = com.mediatek.internal.R.drawable.sim_searching;
            break;
        case PhoneConstants.SIM_INDICATOR_ROAMING:
            resId = com.mediatek.internal.R.drawable.sim_roaming;
            break;
        case PhoneConstants.SIM_INDICATOR_CONNECTED:
            resId = com.mediatek.internal.R.drawable.sim_connected;
            break;
        case PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
            resId = com.mediatek.internal.R.drawable.sim_roaming_connected;
            break;
        default:
            resId = PhoneConstants.SIM_INDICATOR_UNKNOWN;
            break;
        }
        return resId;
    }

    /**
     * Get sim color resources
     * @param colorId sim color id
     * @return the color resource 
     */
    private int getSimColorResource(int colorId) {
        int bgColor = -1;
        if ((colorId >= 0) && (colorId < SimInfoManager.SimBackgroundDarkRes.length)) {
            bgColor = SimInfoManager.SimBackgroundDarkRes[colorId];
        } else if (colorId == SimInfoManager.SimBackgroundDarkRes.length) {
            bgColor = com.mediatek.internal.R.drawable.sim_background_sip;
        }
        return bgColor;

    }

    private int getSimIndicator(Context context, int slotId) {
        if (isAllRadioOff(context)) {
            Log.i(TAG, "isAllRadioOff=" + isAllRadioOff(context) + "slotId=" + slotId);
            return PhoneConstants.SIM_INDICATOR_RADIOOFF;
        }

        ITelephony iTelephony = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
        ITelephonyEx iTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        int indicator = PhoneConstants.SIM_INDICATOR_UNKNOWN;
        if (iTelephony != null && iTelephonyEx != null) {
            try {
                indicator = FeatureOption.MTK_GEMINI_SUPPORT ?
                        iTelephonyEx.getSimIndicatorState(slotId)
                        : iTelephony.getSimIndicatorState();
            } catch (RemoteException e) {
                Log.i(TAG, "RemoteException");
            } catch (NullPointerException ex) {
                Log.i(TAG, "NullPointerException");
            }
        }
        return indicator;
    }

    private boolean isAllRadioOff(Context context) {
        int airMode = Settings.System.getInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1);
        int dualMode = Settings.System.getInt(context.getContentResolver(), Settings.System.DUAL_SIM_MODE_SETTING, -1);
        return airMode == 1 || dualMode == 0;
    }

}
