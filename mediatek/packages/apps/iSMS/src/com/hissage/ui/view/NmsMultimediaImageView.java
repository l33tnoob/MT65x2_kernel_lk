package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Handler;
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
import com.hissage.util.log.NmsLog;

public class NmsMultimediaImageView extends NmsMultimediaBaseView {

    private Handler mHandler;
    private Context mContext;
    private View mConvertView;
    private ImageView v;
    private Bitmap bitmap;
    private TextView tvTime;
    private NmsIpImageMessage mSession;
    private final static String TAG = "NmsMultimediaBaseView";

    public NmsMultimediaImageView(Context context, NmsIpImageMessage session) {
        super(context);
        mContext = context;
        this.mSession = session;

        LayoutInflater inflater = LayoutInflater.from(context);
        mConvertView = inflater.inflate(R.layout.all_media_image, this, true);

        v = (ImageView) mConvertView.findViewById(R.id.iv_image);

        tvTime = (TextView) mConvertView.findViewById(R.id.tv_media_name);
    }

    public void setMediaImage() {
        setTimeStamp(tvTime, mSession);
        if (bitmap == null) {
            isSetTime = true;
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager wmg = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
            wmg.getDefaultDisplay().getMetrics(dm);

            bitmap = NmsBitmapUtils.getBitmapByPath(mSession.path,
                    NmsBitmapUtils.getOptions(mSession.path), dm.widthPixels, dm.heightPixels);
            if (bitmap == null) {
                try {
                    bitmap = NmsBitmapUtils.getBitmapByPath(mSession.thumbPath,
                            NmsBitmapUtils.getOptions(mSession.thumbPath), dm.widthPixels,
                            dm.heightPixels);
                } catch (Exception e) {
                    NmsLog.warn(TAG, "BitmapFactory.decodeFile failed, ipMsg.ipDbId: "
                            + mSession.ipDbId);
                }
            }

            if (null == bitmap) {
                v.setImageResource(R.drawable.isms_media_failed_big);
            } else {
                v.setImageBitmap(bitmap);
            }
        }
    }

    public void destoryMediaImage() {
        if (bitmap != null) {
            v.setImageBitmap(null);
            bitmap.recycle();
            bitmap = null;
        }
    }

    public NmsMultimediaImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }
}
