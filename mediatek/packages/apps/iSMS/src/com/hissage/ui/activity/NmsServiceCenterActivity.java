package com.hissage.ui.activity;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.app.ActionBar;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Paint;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.Bundle;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsCommonUtils;
//M:Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsServiceCenterActivity extends NmsBaseActivity {

    private ListView mList;
    private Context mContext;

    SNmsSimInfo sim1 = null;
    SNmsSimInfo sim2 = null;

    private ServiceCenterAdapter mServiceCenterAdapter;
    private final static String TAG = "NmsServiceCenterActivity";
    private static final int[] resId = { R.layout.service_center_audio_item,
            R.layout.service_center_video_item, R.layout.service_center_activate_item };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.service_center_activity);
        mContext = this;
        initialize();
    }

    @Override
    public void onResume() {
        super.onResume();
        getAllSimInfo();
        if (mServiceCenterAdapter != null) {
            mServiceCenterAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            openConversationList();
            finish();
        }
        return super.onMenuItemSelected(featureId, item);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    private void initialize() {
        initResourceRefs();
        getAllSimInfo();
        initActionBar();
        initList();
    }

    private void initResourceRefs() {
        mList = (ListView) findViewById(R.id.lv_list);

    }

    private void getAllSimInfo() {

        long sim_id = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(0);
        if (sim_id > 0) {
            sim1 = NmsIpMessageApiNative.nmsGetSimInfoViaSimId((int) sim_id);
        }
        long sim_id2 = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(1);
        if (sim_id2 > 0) {
            sim2 = NmsIpMessageApiNative.nmsGetSimInfoViaSimId((int) sim_id2);
        }
    }

    private void initActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowHomeEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayUseLogoEnabled(true);
        actionBar.setTitle(R.string.STR_NMS_SERVICE_TITLE);
        actionBar.setLogo(R.drawable.isms_service);
    }

    private void initList() {
        mServiceCenterAdapter = new ServiceCenterAdapter();
        mList.setAdapter(mServiceCenterAdapter);
        mList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (position == 1) {
                    Intent i = new Intent(mContext, NmsVideoPlayActivity.class);
                    Uri uri = Uri.parse("android.resource://com.hissage/raw/video");
                    i.setData(uri);
                    mContext.startActivity(i);
                } else if (position == 0) {
                    Intent intent = new Intent();
                    intent.setAction(android.content.Intent.ACTION_VIEW);
                    Uri uri = getAudioUri();
                    if (null == uri) {
                        return;
                    }
                    intent.setDataAndType(uri, "audio/*");
                    mContext.startActivity(Intent.createChooser(intent, "Choose share method."));
                }
            }
        });
    }

    private Uri getAudioUri() {
/*        if (NmsCommonUtils.getSDCardStatus()) {
            String cachePath = NmsCommonUtils.getCachePath(mContext);
            File f = new File(cachePath);
            if (!f.exists()) {
                f.mkdirs();
            }
        } else {
            Toast.makeText(mContext, R.string.STR_NMS_LOSE_SDCARD, Toast.LENGTH_SHORT).show();
            return null;
        }
        String dest = NmsCommonUtils.getCachePath(mContext) + "introduction.amr";
        File out = new File(dest);
        InputStream stream;// = getResources().openRawResource(R.raw.audio);
        FileOutputStream outStream = null;
        try {
            outStream = new FileOutputStream(out);
        } catch (FileNotFoundException e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
            return null;
        }

        byte buffer[] = new byte[256];
        int length = 0;
        try {
            while ((length = stream.read(buffer)) != -1) {
                outStream.write(buffer, 0, length);
            }
            outStream.close();
        } catch (IOException e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
            return null;
        }
        return Uri.fromFile(new File(dest));*/
    	return null;
    }

    private class ServiceCenterAdapter extends BaseAdapter {
        private LayoutInflater inflater;
        private Button mActivation;
        private TextView mTermLink;
        private TextView mAudioDuration;
        private ImageView mVideoContent;

        public ServiceCenterAdapter() {
            inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        private int getLayoutResIdViaType(int type) {
            if (type < 0 || type > resId.length) {
                NmsLog.error(TAG, "invalid display type:" + type);
                return 0;
            }
            return resId[type];
        }

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return resId.length;
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        public int getViewTypeCount() {
            return resId.length;
        }

        @Override
        public int getItemViewType(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            int type = getItemViewType(position);
            if (convertView == null) {
                int resId = getLayoutResIdViaType(type);
                convertView = inflater.inflate(resId, null);
                convertView.setTag(convertView);
            } else {
                convertView = (View) convertView.getTag();
            }
            mVideoContent = (ImageView) convertView.findViewById(R.id.iv_video_content);
            if (mVideoContent != null) {
                final BitmapFactory.Options options = new BitmapFactory.Options();
                options.inJustDecodeBounds = true;
                BitmapFactory.decodeResource(getResources(), R.drawable.introduction, options);
                Bitmap bt = NmsBitmapUtils.decodeSampledBitmapFromResource(getResources(),
                        R.drawable.introduction, options.outWidth, options.outHeight);
                mVideoContent.setImageBitmap(bt);
            }
            mAudioDuration = (TextView) convertView.findViewById(R.id.tv_audio_info);
            if (mAudioDuration != null) {
                int duration = 0;
                MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                Uri uri = Uri.parse("android.resource://com.hissage/raw/audio");
                try {
                    retriever.setDataSource(mContext, uri);
                    String dur = retriever
                            .extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
                    if (dur != null) {
                        duration = Integer.parseInt(dur);
                        duration = duration / 1000 == 0 ? 1 : duration / 1000;
                    }
                } catch (Exception ex) {
                    NmsLog.error(TAG,
                            "MediaMetadataRetriever failed to get duration for " + uri.getPath());
                } finally {
                    retriever.release();
                }
                String text = MessageUtils.formatAudioTime(duration);
                mAudioDuration.setText(text);
            }

            mActivation = (Button) convertView.findViewById(R.id.btn_activation);
            mTermLink = (TextView) convertView.findViewById(R.id.tv_terms_content);
            if (mActivation != null) {
                if (needShowActivition()) {
                    mActivation.setVisibility(View.VISIBLE);
                    mActivation.setOnClickListener(new OnClickListener() {

                        @Override
                        public void onClick(View v) {
                            showActivitionDlg(
									//M:Activation Statistics
                                    NmsPlatformAdapter.getInstance(NmsServiceCenterActivity.this)
                                            .getCurrentSimId(), 1, NmsIpMessageConsts.NmsUIActivateType.OTHER);
                        }
                    });
                } else {
                    mActivation.setVisibility(View.GONE);
                }
            }
            if (null != mTermLink) {
                if (needShowActivition()) {
                    mTermLink.getPaint().setFlags(Paint.UNDERLINE_TEXT_FLAG);
                    mTermLink.getPaint().setAntiAlias(true);
                    SpannableString styles = setSpan(mTermLink.getText());
                    mTermLink.setText(styles);
                    mTermLink.setMovementMethod(LinkMovementMethod.getInstance());
                    mTermLink.setFocusable(false);
                    mTermLink.setClickable(false);
                    mTermLink.setLongClickable(false);
                } else {
                    mTermLink.setVisibility(View.GONE);
                }

            }
            return convertView;
        }

        private boolean needShowActivition() {
            if (null == sim1) {
                if (null == sim2) {
                    return false;
                } else {
                    if ((sim2.status < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                            && (sim2.status > NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST)) {
                        return true;
                    } else {
                        return false;
                    }
                }
            } else {
                if (null == sim2) {
                    if ((sim1.status < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                            && (sim1.status > NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST)) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    if (((sim1.status < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) && (sim1.status > NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST))
                            && ((sim2.status < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) && (sim2.status > NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST))) {
                        return true;
                    } else {
                        return false;
                    }
                }
            }
        }

        private SpannableString setSpan(CharSequence text) {
            if (text == null) {
                return null;
            }
            SpannableString style = new SpannableString(text);
            MyURLSpan myURLSpan = new MyURLSpan();
            style.setSpan(myURLSpan, 0, text.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            return style;
        }

        private class MyURLSpan extends ClickableSpan {

            @Override
            public void onClick(View widget) {
                Intent i = new Intent(mContext, NmsTermActivity.class);
                i.putExtra(NmsConsts.SIM_ID,
                        NmsPlatformAdapter.getInstance(NmsServiceCenterActivity.this)
                                .getCurrentSimId());
                startActivity(i);
            }
        }
    }
}
