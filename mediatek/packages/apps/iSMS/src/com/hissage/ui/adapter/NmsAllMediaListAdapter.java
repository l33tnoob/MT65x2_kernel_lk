package com.hissage.ui.adapter;

import java.io.File;
import java.util.List;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.imagecache.NmsImageCache;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts.NmsMessageProtocol;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.ui.activity.NmsAllMediaActivity.AllMediaIpMessage;
import com.hissage.ui.activity.NmsAllMediaDetailsActivity;
import com.hissage.ui.view.NmsAllMediaCustomGridView;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsAllMediaListAdapter extends BaseAdapter {

    private final static String TAG = "NmsAllMediaListAdapter";

    private Context mContext;
    private short mEngineContactId;
    private List<AllMediaIpMessage> mAllMediaIpMsgList;
    private NmsImageCache mImageCache;

    private static class ViewHolder {
        TextView tvTimeDivider;
        NmsAllMediaCustomGridView gvMedia;
    }

    public NmsAllMediaListAdapter(Context context, short engineContactId,
            List<AllMediaIpMessage> amIpMsgList, NmsImageCache imageCache) {
        mContext = context;
        mEngineContactId = engineContactId;
        mAllMediaIpMsgList = amIpMsgList;
        mImageCache = imageCache;
    }

    @Override
    public int getCount() {
        if (mAllMediaIpMsgList != null) {
            return mAllMediaIpMsgList.size();
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
            convertView = inflater.inflate(R.layout.all_media_list_item, null);
            holder = new ViewHolder();

            LinearLayout llAllMedia = (LinearLayout) convertView.findViewById(R.id.ll_time_divider);
            holder.tvTimeDivider = (TextView) llAllMedia.findViewById(R.id.tv_time_divider);
            holder.gvMedia = (NmsAllMediaCustomGridView) convertView.findViewById(R.id.cgv_view);

            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        if (mAllMediaIpMsgList == null || mAllMediaIpMsgList.isEmpty()) {
            NmsLog.warn(TAG, "mAllMediaIpMsgList is null/empty.");
            return convertView;
        }
        final AllMediaIpMessage amIpMsg = mAllMediaIpMsgList.get(position);
        if (amIpMsg == null) {
            NmsLog.error(TAG, "getView: amIpMsg is null!");
            return convertView;
        }

        String time = MessageUtils.allMediaTimeDividerType2String(mContext, amIpMsg.timeType);
        holder.tvTimeDivider.setText(time);
        holder.gvMedia.setAdapter(new NmsAllMediaGridAdapter(mContext, amIpMsg.ipMsgList,
                mImageCache));

        holder.gvMedia.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                TextView tvNote = (TextView) arg1.findViewById(R.id.tv_note);

                List<NmsIpMessage> ipMsgList = amIpMsg.ipMsgList;
                if (ipMsgList == null) {
                    NmsLog.error(TAG, "onItemClick: ipMsgList is null!");
                    return;
                }
                NmsIpMessage ipMsg = ipMsgList.get(arg2);
                if (ipMsg == null) {
                    NmsLog.error(TAG, "onItemClick: ipMsg is null!");
                    return;
                }
                if (ipMsg.protocol == NmsMessageProtocol.MMS) {
                    // TODO: Call show mms activitys
                    NmsLog.trace(TAG, "The MMS messag id:" + ipMsg.id);

                    String contentType = NmsSMSMMSManager.getInstance(mContext)
                            .getMmsSpecialContentType((int) ipMsg.id);
                    if (contentType == null) {
                        Intent intent = new Intent("com.android.mms.ui.SlideshowActivity");
                        intent.setClassName("com.android.mms",
                                "com.android.mms.ui.SlideshowActivity");
                        intent.setData(Uri.parse("content://mms/" + ipMsg.id));
                        mContext.startActivity(intent);
                    } else if (contentType.equals("text/x-vcard")) {
                        String vcardPath = NmsSMSMMSManager.getInstance(mContext)
                                .getMmsSpecialAttachPath((int) ipMsg.id);
                        if (vcardPath == null) {
                            NmsLog.error(TAG,
                                    " parse mms got error, vcard path is null, ipMsg id is "
                                            + ipMsg.id);
                            return;
                        }

                        File vcardFile = new File(vcardPath);
                        Uri vcardUri = Uri.fromFile(vcardFile);
                        Intent intent = new Intent();
                        intent.setAction(android.content.Intent.ACTION_VIEW);
                        intent.setDataAndType(vcardUri, "text/x-vcard");
                        try {
                            mContext.startActivity(intent);
                        } catch (ActivityNotFoundException e) {
                            NmsLog.error(TAG, "can't open vcard");
                        }
                    } else if (contentType.equals("text/x-vcalendar")) {
                        String calendarPath = NmsSMSMMSManager.getInstance(mContext)
                                .getMmsSpecialAttachPath((int) ipMsg.id);
                        if (calendarPath == null) {
                            NmsLog.error(TAG,
                                    " parse mms got error, calendar path is null, ipMsg id is "
                                            + ipMsg.id);
                            return;
                        }

                        File calendarFile = new File(calendarPath);
                        Uri calendarUri = Uri.fromFile(calendarFile);
                        Intent intent = new Intent();
                        intent.setAction(android.content.Intent.ACTION_VIEW);
                        intent.setDataAndType(calendarUri, "text/x-vcalendar");
                        try {
                            mContext.startActivity(intent);
                        } catch (ActivityNotFoundException e) {
                            NmsLog.error(TAG, "can't open calendar");
                        }
                    } else {
                        NmsLog.error(TAG, "got invalid content type when parsing mms."); 
                    }
                } else {
                    int ipDbId = Integer.parseInt(tvNote.getText().toString());
                    Intent i = new Intent(mContext, NmsAllMediaDetailsActivity.class);
                    i.putExtra("type", 1);
                    i.putExtra("ipDbId", ipDbId);
                    mContext.startActivity(i);
                }
            }
        });

        return convertView;
    }

}
