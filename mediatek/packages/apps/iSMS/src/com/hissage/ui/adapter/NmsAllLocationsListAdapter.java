package com.hissage.ui.adapter;

import java.util.List;

import android.content.Context;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.location.NmsLocationFormat;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.util.data.NmsDateUtils;
import com.hissage.util.log.NmsLog;

public class NmsAllLocationsListAdapter extends BaseAdapter {

    private final static String TAG = "NmsAllLocationsListAdapter"; 

    private Context mContext;
    private short mEngineContactId;
    private List<NmsIpLocationMessage> mIpLocMsgList;

    private static class ViewHolder {
        TextView tvOne;
        TextView tvTwo;
        TextView tvDate;
    }

    public NmsAllLocationsListAdapter(Context context, short engineContactId,
            List<NmsIpLocationMessage> ipLocMsgList) {
        mContext = context;
        mEngineContactId = engineContactId;
        mIpLocMsgList = ipLocMsgList;
    }

    @Override
    public int getCount() {
        if (mIpLocMsgList != null) {
            return mIpLocMsgList.size();
        } else {
            return 0;
        }
    }

    @Override
    public Object getItem(int position) {
        if (mIpLocMsgList == null || mIpLocMsgList.isEmpty()) {
            NmsLog.error(TAG, "getItem. mIpLocMsgList is null/empty");
            return null;
        }

        return mIpLocMsgList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;

        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.all_locations_list_item, null);

            holder = new ViewHolder();
            holder.tvOne = (TextView) convertView.findViewById(R.id.tv_one);
            holder.tvTwo = (TextView) convertView.findViewById(R.id.tv_two);
            holder.tvDate = (TextView) convertView.findViewById(R.id.tv_date);

            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        if (mIpLocMsgList == null || mIpLocMsgList.isEmpty()) {
            NmsLog.warn(TAG, "getView. mIpLocMsgList is null/empty");
            return convertView;
        }
        NmsIpLocationMessage ipLocMsg = mIpLocMsgList.get(position);
        if (ipLocMsg == null) {
            NmsLog.error(TAG, "getView. ipLocMsg is null!");
            return convertView;
        }

        String lineOne = NmsLocationFormat.getDetailAddr(ipLocMsg.address,
                NmsLocationFormat.ADDR_TYPE_NAME);
        String lineTwo = NmsLocationFormat.getDetailAddr(ipLocMsg.address,
                NmsLocationFormat.ADDR_TYPE_VICINITY);
        String date = NmsDateUtils.getAllLocationsFormatTime(mContext, (long) ipLocMsg.time * 1000);

        if (TextUtils.isEmpty(lineOne)) {
            holder.tvOne.setText(mContext.getString(R.string.STR_NMS_ALL_LOCATIONS_LOC));
        } else {
            holder.tvOne.setText(lineOne);
        }
        holder.tvTwo.setText(lineTwo);
        holder.tvDate.setText(date);

        return convertView;
    }

}
