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
import com.hissage.config.NmsBitmapUtils;
import com.hissage.imagecache.NmsImageCache;
import com.hissage.imageworker.NmsMessageMediaWorker;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpMessageConsts.NmsMessageProtocol;
import com.hissage.util.log.NmsLog;

public class NmsAllMediaGridAdapter extends BaseAdapter {

    private final static String TAG = "NmsAllMediaGridAdapter";

    private Context mContext;
    private List<NmsIpMessage> mIpMsgList;
    private int mThumbSize;
    private NmsMessageMediaWorker mMsgMediaMmsWorker;
    private NmsMessageMediaWorker mMsgMediaImageWorker;
    private NmsMessageMediaWorker mMsgMediaVideoWorker;

    private static class ViewHolder {
        ImageView ivThumbPath;
        ImageView ivThumbBorder;
        ImageView ivIcon;
        TextView tvNote;
    }

    public NmsAllMediaGridAdapter(Context context, List<NmsIpMessage> ipMsgList,
            NmsImageCache imageCache) {
        mContext = context;
        mIpMsgList = ipMsgList;
        mThumbSize = mContext.getResources().getDimensionPixelSize(R.dimen.all_media_thumb_width);
        mMsgMediaMmsWorker = new NmsMessageMediaWorker(mContext,
                NmsBitmapUtils.decodeSampledBitmapFromResource(mContext.getResources(),
                        R.drawable.all_media_mms, mThumbSize), imageCache, mThumbSize);
        mMsgMediaImageWorker = new NmsMessageMediaWorker(mContext,
                NmsBitmapUtils.decodeSampledBitmapFromResource(mContext.getResources(),
                        R.drawable.all_media_image, mThumbSize), imageCache, mContext
                        .getResources().getDimensionPixelSize(R.dimen.all_media_thumb_width));
        mMsgMediaVideoWorker = new NmsMessageMediaWorker(mContext,
                NmsBitmapUtils.decodeSampledBitmapFromResource(mContext.getResources(),
                        R.drawable.all_media_video, mThumbSize), imageCache, mThumbSize);
    }

    @Override
    public int getCount() {
        if (mIpMsgList != null) {
            return mIpMsgList.size();
        } else {
            return 0;
        }
    }

    @Override
    public Object getItem(int position) {
        return null;
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
            convertView = inflater.inflate(R.layout.all_media_grid_item, null);

            holder = new ViewHolder();
            holder.ivThumbPath = (ImageView) convertView.findViewById(R.id.iv_thumb_path);
            holder.ivThumbBorder = (ImageView) convertView.findViewById(R.id.iv_thumb_border);
            holder.ivIcon = (ImageView) convertView.findViewById(R.id.iv_icon);
            holder.tvNote = (TextView) convertView.findViewById(R.id.tv_note);

            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        NmsIpMessage ipMsg = mIpMsgList.get(position);
        if (ipMsg == null) {
            NmsLog.error(TAG, "getView: ipMsg is null");
            return null;
        }

        if (ipMsg.protocol == NmsMessageProtocol.MMS) {
            mMsgMediaMmsWorker.loadImage((short) ipMsg.ipDbId, holder.ivThumbPath);
            holder.ivIcon.setImageResource(R.drawable.all_media_mms_icon);
            holder.ivIcon.setVisibility(View.VISIBLE);
        } else {
            if (ipMsg.type == NmsIpMessageType.PICTURE) {
                mMsgMediaImageWorker.loadImage((short) ipMsg.ipDbId, holder.ivThumbPath);
            } else if (ipMsg.type == NmsIpMessageType.SKETCH) {
                mMsgMediaImageWorker.loadImage((short) ipMsg.ipDbId, holder.ivThumbPath);
                holder.ivThumbBorder.setVisibility(View.VISIBLE);
            } else if (ipMsg.type == NmsIpMessageType.VOICE) {
                holder.ivThumbPath.setImageBitmap(NmsBitmapUtils.decodeSampledBitmapFromResource(
                        mContext.getResources(), R.drawable.all_media_voice, mThumbSize));
            } else if (ipMsg.type == NmsIpMessageType.VIDEO) {
                mMsgMediaVideoWorker.loadImage((short) ipMsg.ipDbId, holder.ivThumbPath);
                holder.ivIcon.setVisibility(View.VISIBLE);
            } else {
                NmsLog.error(TAG, "ipMsg type(" + ipMsg.type + ") is not meida!");
            }
        }

        holder.tvNote.setText("" + ipMsg.ipDbId);

        return convertView;
    }

}
