/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.mediatek.videofavorites;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.ContactsContract;
import android.view.View;
import android.widget.RemoteViews;
import android.widget.RemoteViewsService;

import com.mediatek.xlog.Xlog;

import java.io.File;
import java.util.ArrayList;
import java.util.Locale;

public class WidgetAdapter extends RemoteViewsService {

    public static final String KEY_NAME = "com.mediatek.videofavorites.NAME";
    private static final String PROVIDER_KEY_NAME = VideoFavoritesProviderValues.Columns.NAME;
    public static final int LARGE_MAX_NUM_VIDEOS = 4;

    private static final String TAG = "WidgetAdapter";
    private static final String SORT_ASCENDING = " ASC";
    private static final String SORT_DESCENDING = " DESC";
    private static final String [] PROJECTION = new String [] {
        VideoFavoritesProviderValues.Columns._ID,
        VideoFavoritesProviderValues.Columns.CONTACT_URI,
        VideoFavoritesProviderValues.Columns.VIDEO_URI,
        VideoFavoritesProviderValues.Columns.NAME,
        VideoFavoritesProviderValues.Columns.STORAGE
    };

    private static final int CONTACT_UPDATE_DATA_DELAY = 2000;
    private static final int MSG_CONTACT_DATA_CHANGED = 1;

    private static final String [] CONTACT_PROJECTION = new String [] {
        ContactsContract.Contacts.DISPLAY_NAME_PRIMARY,
    };

    private RemoteViewsFactory mViewFactory;

    // for mornitoring data changing in database of contacts
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_CONTACT_DATA_CHANGED && mViewFactory != null) {
                if (mViewFactory instanceof ViewFactory) {
                    ((ViewFactory) mViewFactory).onContactDataChanged();
                }
            } else {
                super.handleMessage(msg);
            }
        }
    };

    public RemoteViewsFactory onGetViewFactory(Intent intent) {
        if (mViewFactory == null) {
            ViewFactory vf = new ViewFactory(R.layout.list_item);
            vf.init(getBaseContext());
            mViewFactory = vf;
        }
        return mViewFactory;
    }

    // bit map utilities
    /////////////////

    private Rect getSourceRect(int srcWidth, int srcHeight, int maxWidth, int maxHeight) {
        float rSrc = (float) srcWidth / srcHeight;
        float rMax = (float) maxWidth / maxHeight;

        int targetWidth;
        int targetHeight;
        int marginX = 0;
        int marginY = 0;

        // crop and scale

        if (rSrc < rMax) {
            targetWidth = srcWidth;
            targetHeight = targetWidth * maxHeight / maxWidth;
            marginY = (srcHeight - targetHeight) / 2;
        } else {
            targetHeight = srcHeight;
            targetWidth = targetHeight * maxWidth / maxHeight;
            marginX = (srcWidth - targetWidth) / 2;
        }

        return new Rect(marginX , marginY, marginX + targetWidth, marginY + targetHeight);
    }


    /**
     * Create a video thumbnail for a video. May return null if the video is
     * corrupt or the format is not supported.
     *
     * @param filePath the path of video file
     */
    public static Bitmap createVideoThumbnail(Context context, Uri uri) {
        Bitmap bitmap = null;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(context, uri);
            bitmap = retriever.getFrameAtTime(0, MediaMetadataRetriever.OPTION_CLOSEST);
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file
            Xlog.e(TAG, "Exception:" + ex);

            // CSOFF: IllegalCatch - see comment below about why catching this exception
        } catch (RuntimeException rex) {
            // CSON:

            // sometimes we'll have some very strage format videos being inserted
            // so we catch runtime exception to prevent it being fail.
            Xlog.e(TAG, "Runtime Exception occure getting thumbnal: " + rex);
            bitmap = null;
        } finally {
            retriever.release();
        }

        if (bitmap == null) {
            return null;
        }

        // Scale down the bitmap if it's too large.
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int max = Math.max(width, height);
        if (max > 512) {
            float scale = 512f / max;
            int w = Math.round(scale * width);
            int h = Math.round(scale * height);
            bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
        }

        return bitmap;
    }


    private void sendRefreshBroadcast() {
        Intent i = new Intent(AbsVideoFavoritesWidget.ACTION_REFRESH);
        getBaseContext().sendBroadcast(i);
    }

    private Bitmap getThumbnail(ContentResolver resolver, Uri uri, int maxWidth, int maxHeight) {
        if (uri == null) {
            return null;
        }

        if ("file".equals(uri.getScheme())) {
            File f = new File(uri.getPath());
            if (!f.exists()) {
                Xlog.e(TAG, "File not found: " + uri);
                return null;
            }
        } else if (!Storage.isAvailable(getBaseContext())) {
            Xlog.e(TAG, "Storage is not ready");
            return null;
        }

        Bitmap videoThumb = createVideoThumbnail(getBaseContext(), uri);

        if (videoThumb == null) {
            Xlog.e(TAG, "bitmap create failed!!");
            return null;
        }

        int srcWidth = videoThumb.getWidth();
        int srcHeight = videoThumb.getHeight();

        Rect srcRect = getSourceRect(srcWidth, srcHeight, maxWidth, maxHeight);
        Rect dstRect = new Rect(0, 0, maxWidth, maxHeight);
        Bitmap b = Bitmap.createBitmap(maxWidth, maxHeight, videoThumb.getConfig());
        if (b == null) {
            Xlog.e(TAG, "bitmap create failed!!");
            return null;
        }
        b.eraseColor(Color.TRANSPARENT);
        Canvas canvas = new Canvas(b);
        canvas.drawBitmap(videoThumb, srcRect, dstRect, null);
        videoThumb.recycle();

        return b;
    }

    private class ViewFactory implements RemoteViewsFactory {

        private String mPackageName;
        private ContentResolver mResolver;
        private int mIndexContactUri;
        private int mIndexVideoUri;
        private int mIndexName;
        private int mIndexId;
        private int mIndexStorage;
        private final int mLayoutId;
        private final ContentObserver mContactObserver = new AddressObserver(mHandler);

        // try not always create a remoteView
        private RemoteViews mAddView;

        // we use local lists to cache the data from cursor

        private class FavoriteEntry {
            public String videoUri;
            public String name;
            public String contactUri;
            public int dbId;

            FavoriteEntry(String videoUriStr, String nameStr, String contactUriStr, int id) {
                videoUri = videoUriStr;
                name = nameStr;
                contactUri = contactUriStr;
                dbId = id;
            }
        }

        private final ArrayList<FavoriteEntry> mFavoriteEntries =
            new ArrayList<FavoriteEntry>();

        public ViewFactory(int layoutId) {
            mLayoutId = layoutId;
        }

        public void init(Context context) {
            mPackageName = context.getPackageName();
            mResolver = context.getContentResolver();

            mAddView = new RemoteViews(mPackageName, R.layout.list_item_new);
            mAddView.setOnClickFillInIntent(R.id.favorite_new, new Intent(Intent.ACTION_PICK));
        }

        // provider related methods
        ///////////////////////////

        protected String selection() {
            return null;
        }

        // TODO: sort by date.
        protected String sortOrder() {
            return null;
        }

        protected String [] projection() {
            return PROJECTION;
        }

        // regular interface methods
        public int getCount() {
            return LARGE_MAX_NUM_VIDEOS; // for new design we always return 4 item
        }

        public long getItemId(int position) {
            return position;
        }

        public RemoteViews getLoadingView() {
            return null;
        }

        private RemoteViews getAddIcon() {
            return mAddView;
        }

        private RemoteViews getEmptyView() {
            return new RemoteViews(mPackageName, R.layout.list_item_empty);
        }

        private Intent getFillInIntentFor(Uri contactUri, int entryId) {
            Intent i = new Intent(Intent.ACTION_VIEW, contactUri);
            i.putExtra(VideoFavoritesProviderValues.Columns._ID, entryId);
            return i;
        }

        private Intent getFillInIntentOfDeleteIcon(int id, String name) {
            Uri uri = Uri.parse(String.format(Locale.US, "%s/%d",
                                              VideoFavoritesProviderValues.Columns.CONTENT_URI.toString() , id));
            Intent intent = new Intent(Intent.ACTION_DELETE, uri);
            // put name string into extra, for confirm dialog.
            intent.setAction(Intent.ACTION_DELETE).putExtra(KEY_NAME, name);

            return intent;
        }

        private Intent getFillInIntentForPickContact(Uri videoUri, int entryId) {
            Intent intent = new Intent(Intent.ACTION_PICK, videoUri);
            intent.putExtra(VideoFavoritesProviderValues.Columns._ID, entryId);

            int pickType = WidgetActionActivity.CODE_PICK_CONTACT;
            if (videoUri == null || "".equals(videoUri.toString())) {
                pickType = WidgetActionActivity.CODE_PICK_BOTH;
            }
            intent.putExtra(WidgetActionActivity.KEY_ACTION_PICK_TYPE, pickType);

            return intent;
        }

        private Intent getFillInIntentForPickVideo(Uri contactUri, int entryId) {
            Intent intent = new Intent(Intent.ACTION_PICK, contactUri);
            intent.putExtra(VideoFavoritesProviderValues.Columns._ID, entryId);
            intent.putExtra(WidgetActionActivity.KEY_ACTION_PICK_TYPE,
                            WidgetActionActivity.CODE_PICK_VIDEO);
            return intent;
        }

        public RemoteViews getViewAt(int position) {
            RemoteViews rv;
            if (position < mFavoriteEntries.size()) {
                rv = new RemoteViews(mPackageName, mLayoutId);
                // force invisible since framework will cache the value.
                rv.setViewVisibility(R.id.video, View.INVISIBLE);

                FavoriteEntry entry = mFavoriteEntries.get(position);
                Xlog.v(TAG, "getViewAt(): " + position + ", totalFavorites:"
                       + mFavoriteEntries.size()
                       + " (" + entry.dbId + ", " + entry.name + ", " + entry.videoUri + ", "
                       + entry.contactUri + ")");

                Bitmap bmp = getThumbnail(mResolver,
                                          Uri.parse(entry.videoUri),
                                          getResources().getInteger(R.integer.thumb_width),
                                          getResources().getInteger(R.integer.thumb_height));
                if (bmp == null) {
                    rv.setImageViewResource(R.id.thumb, R.drawable.wd_video_delete);
                    rv.setString(R.id.video, "setVideoUriWithoutOpenVideo", "");
                } else {
                    rv.setImageViewBitmap(R.id.thumb, bmp);
                    rv.setString(R.id.video, "setVideoUriWithoutOpenVideo", entry.videoUri);
                    rv.setBoolean(R.id.video, "setAudioMute", true);
                }

                Intent favoirteClickIntent;
                if ("".equals(entry.name)) {
                    favoirteClickIntent = getFillInIntentForPickContact(
                                              (bmp == null) ? null : Uri.parse(entry.videoUri),
                                              entry.dbId);
                } else if (bmp == null) {
                    // TODO: see if it is possible to check whether video is really gone
                    // or it is simply the media server not ready yet.
                    favoirteClickIntent = getFillInIntentForPickVideo(Uri.parse(entry.contactUri),
                                          entry.dbId);
                } else {
                    favoirteClickIntent = getFillInIntentFor(Uri.parse(entry.contactUri),
                                          entry.dbId);
                }
                rv.setTextViewText(R.id.caption, entry.name);

                rv.setOnClickFillInIntent(R.id.favorite, favoirteClickIntent);

                rv.setOnClickFillInIntent(R.id.icon_delete,
                                          getFillInIntentOfDeleteIcon(entry.dbId, entry.name));
                rv.setViewVisibility(R.id.frame_delete, View.INVISIBLE);
            } else if (position == mFavoriteEntries.size()) {
                Xlog.v(TAG, "getViewAt(): " + position + ", totalFavorites:"
                       + mFavoriteEntries.size() + ": getAddIcon()");
                rv = getAddIcon();
            } else {
                Xlog.v(TAG, "getViewAt(): " + position + ", totalFavorites:"
                       + mFavoriteEntries.size() + ": getEmptyView()");
                rv = getEmptyView();
            }

            return rv;
        }


        public int getViewTypeCount() {
            // NOTE: remember to change test code if the value here is changed
            return 3;
        }

        public boolean hasStableIds() {
            return false;
        }


        private void updateColumnIndex(Cursor c) {
            if (c == null) {
                return;
            }
            mIndexContactUri = c.getColumnIndex(
                                   VideoFavoritesProviderValues.Columns.CONTACT_URI);
            mIndexVideoUri = c.getColumnIndex(VideoFavoritesProviderValues.Columns.VIDEO_URI);
            mIndexName = c.getColumnIndex(VideoFavoritesProviderValues.Columns.NAME);
            mIndexId = c.getColumnIndex(VideoFavoritesProviderValues.Columns._ID);
            mIndexStorage = c.getColumnIndex(VideoFavoritesProviderValues.Columns.STORAGE);
        }


        public void loadEntries() {
            refreshContactdata();
            synchronized (mFavoriteEntries) {
                Cursor c = mResolver.query(VideoFavoritesProviderValues.Columns.CONTENT_URI,
                                           PROJECTION, selection(), null, sortOrder());
                c.moveToFirst();
                updateColumnIndex(c);

                final int count = c.getCount();

                mFavoriteEntries.clear();
                for (int i = 0; i < count; i++) {
                    // to support sd hot swap, we may have to modify the drive name to correct uri.
                    String realVideoUri = VideoFavoritesProvider.getRealVideoURI(
                                              c.getString(mIndexVideoUri), c.getInt(mIndexStorage));

                    FavoriteEntry fe = new FavoriteEntry(realVideoUri,
                                                         c.getString(mIndexName), c.getString(mIndexContactUri), c.getInt(mIndexId));
                    Xlog.v(TAG, "adding entry: (" + fe.dbId + ", " + fe.name + ", "
                           + fe.videoUri + ", " + fe.contactUri + ")");
                    mFavoriteEntries.add(fe);
                    c.moveToNext();
                }
                Xlog.d(TAG, "Entry loaded: " + count);
                c.close();
            }
        }

        /**
         * Called when your factory is first constructed.
         */
        public void onCreate() {
            mResolver.registerContentObserver(
                ContactsContract.Contacts.CONTENT_URI, true, mContactObserver);
            loadEntries();
        }

        /**
         * Called when notify DataSetChanged() is triggered on the remote adapter
         */
        public void onDataSetChanged() {
            loadEntries();
        }

        /**
         * Called when the last RemoteViewsAdapter that is associated with this factory is unbound
         */
        public void onDestroy() {
            mResolver.unregisterContentObserver(mContactObserver);
        }

        // TODO: consider to move to broadcast receiver
        private class AddressObserver extends ContentObserver {
            Handler mHandler;
            public AddressObserver(Handler handler) {
                super(handler);
                mHandler = handler;
            }

            @Override
            public void onChange(boolean selfChange) {
                mHandler.removeMessages(MSG_CONTACT_DATA_CHANGED);
                mHandler.sendEmptyMessageDelayed(MSG_CONTACT_DATA_CHANGED,
                                                 CONTACT_UPDATE_DATA_DELAY);
            }

            @Override
            public boolean deliverSelfNotifications() {
                return true;
            }
        }

        private String selectionContactUri(ArrayList<String> uriStrings) {
            if (uriStrings == null) {
                return null;
            }
            final String or = " OR ";
            StringBuilder sb = new StringBuilder();
            final String prefix = VideoFavoritesProviderValues.Columns.CONTACT_URI + "='";
            final int size = uriStrings.size();

            for (int i = 0; i < size; i++) {
                sb.append(prefix);
                sb.append(uriStrings.get(i));
                sb.append("'");
                if (i + 1 < size) {
                    sb.append(or);
                }
            }
            return sb.toString();
        }


        public boolean refreshContactdata() {
            ArrayList<String> uriStringList = null;
            boolean nameChanged = false;

            synchronized (mFavoriteEntries) {
                if (mFavoriteEntries.size() <= 0) {
                    return false;
                }

                final int count = mFavoriteEntries.size();

                for (int i = 0; i < count; i++) {
                    FavoriteEntry fe = mFavoriteEntries.get(i);
                    if (fe.contactUri.length() == 0) {
                        continue;
                    }

                    Cursor c = mResolver.query(Uri.parse(fe.contactUri), CONTACT_PROJECTION,
                                               null, null, null);

                    if (c == null || c.getCount() == 0) {
                        Xlog.v(TAG, "a contact url is gone");
                        if (uriStringList == null) {
                            uriStringList = new ArrayList<String>();
                        }
                        uriStringList.add(fe.contactUri);
                    } else {
                        c.moveToFirst();
                        String name = c.getString(c.getColumnIndex(
                                                      ContactsContract.Contacts.DISPLAY_NAME_PRIMARY));
                        if ((fe.name == null && name != null)
                                || (fe.name != null && !fe.name.equals(name))) {
                            Xlog.v(TAG, "name changed from " + fe.name + " to " + name);
                            Uri target = Uri.parse(String.format(Locale.US, "%s/%d",
                                                                 VideoFavoritesProviderValues.Columns.CONTENT_URI.toString(),
                                                                 fe.dbId));
                            ContentValues v = new ContentValues(1);
                            v.put(PROVIDER_KEY_NAME, name);
                            mResolver.update(target, v, null, null);
                            nameChanged = true;
                        }
                    }

                    if (c != null) {
                        c.close();
                    }
                }

                if (uriStringList != null) {
                    ContentValues v = new ContentValues();
                    v.put(VideoFavoritesProviderValues.Columns.CONTACT_URI, "");
                    v.put(VideoFavoritesProviderValues.Columns.NAME, "");

                    mResolver.update(VideoFavoritesProviderValues.Columns.CONTENT_URI,
                                     v, selectionContactUri(uriStringList), null);
                }
            }
            // assume someone is observing the resolver, so we don't send msg ourselves.
            return (nameChanged || uriStringList != null);
        }

        // there are 2 possibilities, so we handle both
        // 1. a contact is deleted/added
        // 2. the content of a contact is changed.
        public void onContactDataChanged() {
            if (refreshContactdata()) {
                sendRefreshBroadcast();
            }
        }
    }
}
