/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2013. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.videoplayer;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.PixelFormat;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.BaseColumns;
import android.provider.MediaStore.MediaColumns;
import android.provider.MediaStore.Video.Media;
import android.provider.MediaStore.Video.VideoColumns;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Locale;

import com.mediatek.drm.OmaDrmStore;

public class MovieListActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "MovieListActivity";
    private static final boolean LOG = true;

    private static final Uri VIDEO_URI = Media.EXTERNAL_CONTENT_URI;
    private static final String[] PROJECTION = new String[]{
        BaseColumns._ID,
        MediaColumns.DISPLAY_NAME,
        VideoColumns.DATE_TAKEN,
        VideoColumns.DURATION,
        MediaColumns.MIME_TYPE,
        MediaColumns.DATA,
        MediaColumns.SIZE,
        Media.IS_DRM,
        MediaColumns.DATE_MODIFIED,
        Media.STEREO_TYPE,
    };
    private static final int INDEX_ID = 0;
    private static final int INDEX_DISPLAY_NAME = 1;
    private static final int INDEX_TAKEN_DATE = 2;
    private static final int INDEX_DRUATION = 3;
    private static final int INDEX_MIME_TYPE = 4;
    private static final int INDEX_DATA = 5;
    private static final int INDEX_FILE_SIZE = 6;
    private static final int INDEX_IS_DRM = 7;
    private static final int INDEX_DATE_MODIFIED = 8;
    private static final int INDEX_SUPPORT_3D = 9;
    
    private static final String ORDER_COLUMN =
        VideoColumns.DATE_TAKEN + " DESC, " + 
        BaseColumns._ID + " DESC ";
    
    private ListView mListView;
    private TextView mEmptyView;
    private ViewGroup mNoSdView;
    private MovieListAdapter mAdapter;
    
    private static final int MENU_DELETE_ALL = 1;
    private static final int MENU_DELETE_ONE = 2;
    private static final int MENU_PROPERTY = 3;
    private static final int MENU_DRM_DETAIL = 4;
    
    private static final String KEY_LOGO_BITMAP = "logo-bitmap";
    private static final String KEY_TREAT_UP_AS_BACK = "treat-up-as-back";
    private static final String EXTRA_ALL_VIDEO_FOLDER = "mediatek.intent.extra.ALL_VIDEO_FOLDER";
    private static final String EXTRA_ENABLE_VIDEO_LIST = "mediatek.intent.extra.ENABLE_VIDEO_LIST";
    private ProgressDialog mProgressDialog;
    private static String[] sExternalStoragePaths;
    
    private ThumbnailCache mThumbnailCache;
    private CachedVideoInfo mCachedVideoInfo;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        MtkLog.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_PROGRESS);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);

        setContentView(R.layout.movielist);
        final StorageManager storageManager =
            (StorageManager)getSystemService(Context.STORAGE_SERVICE);
        sExternalStoragePaths = storageManager.getVolumePaths();

        mListView = (ListView) findViewById(android.R.id.list);
        mEmptyView = (TextView) findViewById(android.R.id.empty);
        mNoSdView = (ViewGroup) findViewById(R.id.no_sdcard);
        mAdapter = new MovieListAdapter(this, R.layout.movielist_item, null, new String[]{}, new int[]{});
        mListView.setAdapter(mAdapter);
        mListView.setOnScrollListener(mAdapter);
        
        mListView.setOnItemClickListener(this);
        registerForContextMenu(mListView);
        registerStorageListener();
        refreshSdStatus(MtkUtils.isMediaMounted(MovieListActivity.this));
        
        mThumbnailCache = new ThumbnailCache(this);
        mThumbnailCache.addListener(mAdapter);
        mCachedVideoInfo = new CachedVideoInfo();
        MtkLog.v(TAG, "onCreate(" + savedInstanceState + ")");
    }
    
    private void refreshMovieList() {
        mAdapter.getQueryHandler().removeCallbacks(null);
        mAdapter.getQueryHandler().startQuery(0, null,
                VIDEO_URI,
                PROJECTION,
                null,
                null,
                ORDER_COLUMN);
    }
    
    private void registerStorageListener() {
        final IntentFilter iFilter = new IntentFilter();
        iFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        iFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        iFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        iFilter.addDataScheme("file");
        registerReceiver(mStorageListener, iFilter);
    }
    
    private final BroadcastReceiver mStorageListener = new BroadcastReceiver() {

        @Override
        public void onReceive(final Context context, final Intent intent) {
            MtkLog.v(TAG, "mStorageListener.onReceive(" + intent + ")");
            final String action = intent.getAction();
            if (Intent.ACTION_MEDIA_SCANNER_STARTED.equals(action)) {
                refreshSdStatus(MtkUtils.isMediaMounted(MovieListActivity.this));
            } else if (Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                refreshSdStatus(MtkUtils.isMediaMounted(MovieListActivity.this));
            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action) || Intent.ACTION_MEDIA_EJECT.equals(action)) {
                if (intent.hasExtra(StorageVolume.EXTRA_STORAGE_VOLUME)) {
                    final StorageVolume storage = (StorageVolume)intent.getParcelableExtra(
                            StorageVolume.EXTRA_STORAGE_VOLUME);
                    if (storage != null && storage.getPath().equals(sExternalStoragePaths[0])) {
                        refreshSdStatus(false);
                        mAdapter.changeCursor(null);
                    } // else contentObserver will listen it.
                    MtkLog.v(TAG, "mStorageListener.onReceive() eject storage="
                            + (storage == null ? "null" : storage.getPath()));
                }
            }
        };

    };
    
    private void refreshSdStatus(final boolean mounted) {
        MtkLog.v(TAG, "refreshSdStatus(" + mounted + ")");
        if (mounted) {
            if (MtkUtils.isMediaScanning(this)) {
                MtkLog.v(TAG, "refreshSdStatus() isMediaScanning true");
                showScanningProgress();
                showList();
                MtkUtils.disableSpinnerState(this);
            } else {
                MtkLog.v(TAG, "refreshSdStatus() isMediaScanning false");
                hideScanningProgress();
                showList();
                refreshMovieList();
                MtkUtils.enableSpinnerState(this);
            }
        } else {
            hideScanningProgress();
            showSdcardLost();
            MtkUtils.disableSpinnerState(this);
        }
    }
    
    private void showScanningProgress() {
        showProgress(getString(R.string.scanning), new OnCancelListener() {

            @Override
            public void onCancel(final DialogInterface dialog) {
                MtkLog.v(TAG, "mProgressDialog.onCancel()");
                hideScanningProgress();
                finish();
            }

        });
    }
    
    private void hideScanningProgress() {
        hideProgress();
    }
    
    private void showProgress(final String message, final OnCancelListener cancelListener) {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setIndeterminate(true);
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setCancelable(cancelListener != null);
            mProgressDialog.setOnCancelListener(cancelListener);
            mProgressDialog.setMessage(message);
        }
        mProgressDialog.show();
    }
    
    private void hideProgress() {
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }
    
    private void showSdcardLost() {
        mListView.setVisibility(View.GONE);
        mEmptyView.setVisibility(View.GONE);
        mNoSdView.setVisibility(View.VISIBLE);
    }
    
    private void showList() {
        mListView.setVisibility(View.VISIBLE);
        mEmptyView.setVisibility(View.GONE);
        mNoSdView.setVisibility(View.GONE);
    }
    
    private void showEmpty() {
        mListView.setVisibility(View.GONE);
        mEmptyView.setVisibility(View.VISIBLE);
        mNoSdView.setVisibility(View.GONE);
    }
    
    @Override
    public void onItemClick(final AdapterView<?> parent, final View view, final int position, final long id) {
        final Object o = view.getTag();
        ViewHolder holder = null;
        if (o instanceof ViewHolder) {
            holder = (ViewHolder) o;
            final Intent intent = new Intent(Intent.ACTION_VIEW);
            String mime = "video/*";
            if (!(holder.mMimetype == null || "".equals(holder.mMimetype.trim()))) {
                mime = holder.mMimetype;
            }
            intent.setDataAndType(ContentUris.withAppendedId(VIDEO_URI, holder.mId), mime);
            intent.putExtra(EXTRA_ALL_VIDEO_FOLDER, true);
            intent.putExtra(KEY_TREAT_UP_AS_BACK, true);
            intent.putExtra(EXTRA_ENABLE_VIDEO_LIST, true);
            intent.putExtra(KEY_LOGO_BITMAP, BitmapFactory.decodeResource(getResources(), R.drawable.ic_video_app));
            try {
                startActivity(intent);
            } catch (final ActivityNotFoundException e) {
                e.printStackTrace();
            }
        }
        MtkLog.v(TAG, "onItemClick(" + position + ", " + id + ") holder=" + holder);
    }

    @Override
    public void onCreateContextMenu(final ContextMenu menu, final View v,
            final ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        final AdapterContextMenuInfo info = (AdapterContextMenuInfo) menuInfo;
        final Object obj = info.targetView.getTag();
        ViewHolder holder = null;
        if (obj instanceof ViewHolder) {
            holder = (ViewHolder)obj;
            menu.setHeaderTitle(holder.mTitle);
            menu.add(0, MENU_DELETE_ONE, 0, R.string.delete);
            menu.add(0, MENU_PROPERTY, 0, R.string.media_detail);
            if (MtkUtils.isSupportDrm() && holder.mIsDrm) {
                menu.add(0, MENU_DRM_DETAIL, 0, com.mediatek.R.string.drm_protectioninfo_title);
            }
        }
    }
    
    @Override
    public boolean onContextItemSelected(final MenuItem item) {
        final AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
        final Object obj = info.targetView.getTag();
        ViewHolder holder = null;
        if (obj instanceof ViewHolder) {
            holder = (ViewHolder)obj;
        }
        if (holder == null) {
            MtkLog.w(TAG, "wrong context item info " + info);
            return true;
        }
        switch (item.getItemId()) {
        case MENU_DELETE_ONE:
            showDelete(holder.clone());
            return true;
        case MENU_PROPERTY:
            showDetail(holder.clone());
            return true;
        case MENU_DRM_DETAIL:
            if (MtkUtils.isSupportDrm()) {
                MtkUtils.showDrmDetails(this, holder.mData);
            }
            break;
        default:
            break;
        }
        return super.onContextItemSelected(item);
    }
    
    private void showDetail(final ViewHolder holder) {
        final DetailDialog detailDialog = new DetailDialog(this, holder);
        detailDialog.setTitle(R.string.media_detail);
        detailDialog.show();
    }
    
    private void showDelete(final ViewHolder holder) {
        MtkLog.v(TAG, "showDelete(" + holder + ")");
        new AlertDialog.Builder(this)
        .setTitle(R.string.delete)
        .setMessage(getString(R.string.delete_tip, holder.mTitle))    
        .setCancelable(true)
        .setPositiveButton(android.R.string.ok, new OnClickListener() {

            @Override
            public void onClick(final DialogInterface dialog, final int which) {
                MtkLog.v(TAG, "Delete.onClick() " + holder);
                new DeleteTask(holder).execute();
            }
            
        })
        .setNegativeButton(android.R.string.cancel, null)
        .create()
        .show();
    }
    
    public class DeleteTask extends AsyncTask<Void, Void, Void> {
        private final ViewHolder mHolder;
        
        public DeleteTask(final ViewHolder holder) {
            mHolder = holder;
        }
        
        @Override
        protected void onPreExecute() {
            showDeleteProgress(getString(R.string.delete_progress, mHolder.mTitle));
        }
        
        @Override
        protected void onPostExecute(final Void result) {
            hideDeleteProgress();
        }
        
        @Override
        protected Void doInBackground(final Void... params) {
            final ViewHolder holder = mHolder;
            if (holder == null) {
                MtkLog.w(TAG, "DeleteTask.doInBackground holder=" + holder);
            } else {
                int count = 0;
                try {
                    count = getContentResolver().delete(ContentUris.withAppendedId(VIDEO_URI, holder.mId), null, null);
                } catch (final SQLiteException e) {
                    e.printStackTrace();
                }
                MtkLog.v(TAG, "DeleteTask.doInBackground delete count=" + count);
            }
            return null; 
        }
        
    }
    
    private void showDeleteProgress(final String message) {
        showProgress(message, null);
    }
    
    private void hideDeleteProgress() {
        hideProgress();
    }

    @Override
    protected void onResume() {
        MtkLog.d(TAG, "onResume()");
        super.onResume();
        if (mAdapter != null) { //update drm icon
            mAdapter.notifyDataSetChanged();
        }
        mCachedVideoInfo.setLocale(Locale.getDefault());
    }

    @Override
    protected void onStart() {
        MtkLog.d(TAG, "onStart()");
        super.onStart();
    }

    @Override
    protected void onPause() {
        MtkLog.d(TAG, "onPause()");
        super.onPause();
    }

    @Override
    protected void onStop() {
        MtkLog.d(TAG, "onStop()");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        MtkLog.d(TAG, "onDestroy()");
        if (mAdapter != null) {
            mAdapter.clearCachedHolder();
            mAdapter.changeCursor(null);
        }
        if (mThumbnailCache != null) {
            mThumbnailCache.removeListener(mAdapter);
            mThumbnailCache.clear();
        }
        mCachedVideoInfo.setLocale(null);
        unregisterReceiver(mStorageListener);
        super.onDestroy();
    }
    
    class MovieListAdapter extends SimpleCursorAdapter implements ThumbnailCache.ThumbnailStateListener, OnScrollListener {
        private static final String TAG = "MovieListAdapter";
        private final QueryHandler mQueryHandler;
        private final ArrayList<ViewHolder> mCachedHolder = new ArrayList<ViewHolder>();
        private static final String VALUE_IS_DRM = "1";
        
        QueryHandler getQueryHandler() {
            return mQueryHandler;
        }
        
        public MovieListAdapter(final Context context, final int layout, final Cursor c,
                final String[] from, final int[] to) {
            super(context, layout, c, from, to);
            mQueryHandler = new QueryHandler(getContentResolver());
        }
        
        @Override
        public View newView(final Context context, final Cursor cursor, final ViewGroup parent) {
            final View view = super.newView(context, cursor, parent);
            final ViewHolder holder = new ViewHolder();
            holder.mIcon = (ImageView) view.findViewById(R.id.item_icon);
            holder.mTitleView = (TextView) view.findViewById(R.id.item_title);
            holder.mFileSizeView = (TextView) view.findViewById(R.id.item_date);
            holder.mDurationView = (TextView) view.findViewById(R.id.item_duration);
            int width = mThumbnailCache.getDefaultThumbnailWidth();
            int height = mThumbnailCache.getDefaultThumbnailHeight();
            holder.mFastDrawable = new FastBitmapDrawable(width, height);
            view.setTag(holder);
            mCachedHolder.add(holder);
            MtkLog.v(TAG, "newView() mCachedHolder.size()=" + mCachedHolder.size());
            return view;
        }

        public void onChanged(final long rowId, final int type, final Bitmap drawable) {
            MtkLog.v(TAG, "onChanged(" + rowId + ", " + type + ", " + drawable + ")");
            for (final ViewHolder holder : mCachedHolder) {
                if (holder.mId == rowId) {
                    refreshThumbnail(holder);
                    break;
                }
            }
        }
        
        public void clearCachedHolder() {
            mCachedHolder.clear();
        }
        
        @Override
        public void bindView(final View view, final Context context, final Cursor cursor) {
            final ViewHolder holder = (ViewHolder) view.getTag();
            holder.mId = cursor.getLong(INDEX_ID);
            holder.mTitle = cursor.getString(INDEX_DISPLAY_NAME);
            holder.mDateTaken = cursor.getLong(INDEX_TAKEN_DATE);
            holder.mMimetype = cursor.getString(INDEX_MIME_TYPE);
            holder.mData = cursor.getString(INDEX_DATA);
            holder.mFileSize = cursor.getLong(INDEX_FILE_SIZE);
            holder.mDuration = cursor.getLong(INDEX_DRUATION);
            holder.mIsDrm = VALUE_IS_DRM.equals(cursor.getString(INDEX_IS_DRM));
            holder.mDateModified = cursor.getLong(INDEX_DATE_MODIFIED);
            holder.mSupport3D = MtkUtils.isStereo3D(cursor.getInt(INDEX_SUPPORT_3D));
            
            holder.mTitleView.setText(holder.mTitle);
            holder.mFileSizeView.setText(mCachedVideoInfo.getFileSize(MovieListActivity.this, holder.mFileSize));
            holder.mDurationView.setText(mCachedVideoInfo.getDuration(holder.mDuration));
            
            refreshThumbnail(holder);
            MtkLog.v(TAG, "bindeView() " + holder);
        }
        
        private void refreshThumbnail(final ViewHolder holder) {
            Bitmap bitmap = mThumbnailCache.getCachedThumbnail(holder.mId, holder.mDateModified,
                    holder.mSupport3D, !mFling);
            if (MtkUtils.isSupportDrm() && holder.mIsDrm) {
                bitmap = MtkUtils.overlayDrmIcon(MovieListActivity.this, holder.mData, OmaDrmStore.Action.PLAY, bitmap);
            }
            holder.mFastDrawable.setBitmap(bitmap);
            holder.mIcon.setImageDrawable(holder.mFastDrawable);
            holder.mIcon.invalidate();
        }
        
        @Override
        public void changeCursor(final Cursor c) {
            super.changeCursor(c);
        }
        
        @Override
        protected void onContentChanged() {
            super.onContentChanged();
            mQueryHandler.onQueryComplete(0, null, getCursor());
        }
        
        class QueryHandler extends AsyncQueryHandler {

            QueryHandler(final ContentResolver cr) {
                super(cr);
            }
            
            @Override
            protected void onQueryComplete(final int token, final Object cookie,
                    final Cursor cursor) {
                MtkLog.v(TAG, "onQueryComplete(" + token + "," + cookie + "," + cursor + ")");
                MtkUtils.disableSpinnerState(MovieListActivity.this);
                if (cursor == null || cursor.getCount() == 0) {
                    showEmpty();
                    if (cursor != null) { //to observe database change
                        changeCursor(cursor);
                    }
                } else {
                    showList();
                    changeCursor(cursor);
                }
                if (cursor != null) {
                    MtkLog.v(TAG, "onQueryComplete() end");
                }
            }
        }

        @Override
        public void onScroll(final AbsListView view, final int firstVisibleItem,
                final int visibleItemCount, final int totalItemCount) {
            
        }

        private boolean mFling = false;
        @Override
        public void onScrollStateChanged(final AbsListView view, final int scrollState) {
            switch (scrollState) {
            case OnScrollListener.SCROLL_STATE_IDLE:
                mFling = false;
                //notify data changed to load bitmap from mediastore.
                notifyDataSetChanged();
                break;
            case OnScrollListener.SCROLL_STATE_TOUCH_SCROLL:
                mFling = false;
                break;
            case OnScrollListener.SCROLL_STATE_FLING:
                mFling = true;
                break;
            default:
                break;
            }
            MtkLog.v(TAG, "onScrollStateChanged(" + scrollState + ") mFling=" + mFling);
        }
    }
    
    public class ViewHolder {
        long mId;
        String mTitle;
        String mMimetype;
        String mData;
        long mDuration;
        long mDateTaken;
        long mFileSize;
        boolean mIsDrm;
        long mDateModified;
        boolean mSupport3D;
        ImageView mIcon;
        TextView mTitleView;
        TextView mFileSizeView;
        TextView mDurationView;
        FastBitmapDrawable mFastDrawable;
        
        @Override
        public String toString() {
            return new StringBuilder()
                    .append("ViewHolder(mId=")
                    .append(mId)
                    .append(", mTitle=")
                    .append(mTitle)
                    .append(", mDuration=")
                    .append(mDuration)
                    .append(", mIsDrm=")
                    .append(mIsDrm)
                    .append(", mData=")
                    .append(mData)
                    .append(", mFileSize=")
                    .append(mFileSize)
                    .append(", mSupport3D=")
                    .append(mSupport3D)
                    .append(")")
                    .toString();
        }
        
        /**
         * just clone info
         */
        @Override
        protected ViewHolder clone() {
            final ViewHolder holder = new ViewHolder();
            holder.mId = mId;
            holder.mTitle = mTitle;
            holder.mMimetype = mMimetype;
            holder.mData = mData;
            holder.mDuration = mDuration;
            holder.mDateTaken = mDateTaken;
            holder.mFileSize = mFileSize;
            holder.mIsDrm = mIsDrm;
            holder.mDateModified = mDateModified;
            holder.mSupport3D = mSupport3D;
            return holder;
        }
    }
}
