package com.hissage.ui.adapter;

import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.location.NmsLocation;
import com.hissage.util.log.NmsLog;

public class NmsLocationListAdapter extends BaseAdapter {

    private final static String TAG = "NmsLocationListAdapter";

    private Context mContext;
    private List<NmsLocation> mListData;

    private static class ViewHolder {
        TextView tvOne;
        TextView tvTwo;
        ImageView ivPin;
    }

    public NmsLocationListAdapter(Context context, List<NmsLocation> listData) {
        mContext = context;
        mListData = listData;
    }

    @Override
    public int getCount() {
        if (mListData == null) {
            NmsLog.error(TAG, "mListData is null");
            return 0;
        }

        return mListData.size();
    }

    @Override
    public Object getItem(int position) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public long getItemId(int position) {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;

        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.location_list_item, null);

            holder = new ViewHolder();
            holder.tvOne = (TextView) convertView.findViewById(R.id.tv_one);
            holder.tvTwo = (TextView) convertView.findViewById(R.id.tv_two);
            holder.ivPin = (ImageView) convertView.findViewById(R.id.iv_pin);

            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        NmsLocation data = mListData.get(position);

        holder.tvOne.setText(data.getName());
        holder.tvTwo.setText(data.getVicinity());
        if (data.isSelected()) {
            holder.ivPin.setVisibility(View.VISIBLE);
        } else {
            holder.ivPin.setVisibility(View.GONE);
        }

        return convertView;
    }

}
