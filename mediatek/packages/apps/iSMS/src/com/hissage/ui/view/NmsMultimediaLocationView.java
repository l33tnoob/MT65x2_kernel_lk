package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.util.log.NmsLog;

public class NmsMultimediaLocationView extends NmsMultimediaBaseView {

    private Context mContext;
    private View mConvertView;
    private ImageView v;
    private Bitmap mapPic;

    private TextView tvTime;
    private NmsIpLocationMessage session;

    private final static String TAG = "NmsMultimediaBaseView";

    public NmsMultimediaLocationView(Context context, NmsIpLocationMessage session) {
        super(context);
        mContext = context;
        this.session = session;

        LayoutInflater inflater = LayoutInflater.from(context);
        mConvertView = inflater.inflate(R.layout.all_media_location, this, true);

        v = (ImageView) mConvertView.findViewById(R.id.iv_image);

        TextView tvTimeTitle = (TextView) mConvertView.findViewById(R.id.tv_media_title);
        tvTimeTitle.setText(((NmsIpLocationMessage) session).address);

        tvTime = (TextView) mConvertView.findViewById(R.id.tv_media_name);
    }

    public void setMediaImage() {
        setTimeStamp(tvTime, session);
        if (mapPic == null) {
            isSetTime = true;
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager wmg = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
            wmg.getDefaultDisplay().getMetrics(dm);

            String path = ((NmsIpLocationMessage) session).path;
            if (!TextUtils.isEmpty(path)) {
                try {
                    mapPic = BitmapFactory.decodeFile(path);
                } catch (Exception e) {
                    NmsLog.warn(TAG, "BitmapFactory.decodeFile failed, path=" + path);
                }
            }
            if (mapPic == null) {
                path = ((NmsIpLocationMessage) session).thumbPath;
                if (!TextUtils.isEmpty(path)) {
                    try {
                        mapPic = BitmapFactory.decodeFile(path);
                    } catch (Exception e) {
                        NmsLog.warn(TAG, "BitmapFactory.decodeFile failed, path=" + path);
                    }
                }
            }

            if (mapPic == null) {
                v.setImageResource(R.drawable.default_map_large);
            } else {
                v.setImageBitmap(mapPic);
            }
        }
    }

    public void destoryMediaImage() {
        if (mapPic != null) {
            v.setImageBitmap(null);
            mapPic.recycle();
            mapPic = null;
        }
    }

    public NmsMultimediaLocationView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
}
