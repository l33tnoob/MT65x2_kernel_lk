package com.mediatek.hotknotbeam;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.hotknotbeam.HotKnotBeamService;
import com.mediatek.hotknotbeam.HotKnotBeamConstants.State;

import java.util.Collection;
import java.util.ConcurrentModificationException;
import java.util.Iterator;


/**
 * Update {@link NotificationManager} to reflect current {@link DownloadInfo}
 * states. Collapses similar downloads into a single notification, and builds
 * {@link PendingIntent} that launch towards {@link DownloadReceiver}.
 */
public class DownloadNotifier {
    private final static String TAG = HotKnotBeamService.TAG;
    private final Context mContext;
    private final NotificationManager mNotifManager;

    private String mProgressFormat = null;


    public DownloadNotifier(Context context) {
        mContext = context;
        mNotifManager = (NotificationManager) context.getSystemService(
                            Context.NOTIFICATION_SERVICE);
    }

    public void cancelAll() {
        mNotifManager.cancelAll();
    }

    /**
     * Update {@link NotificationManager} to reflect the given set of
     * {@link DownloadInfo}, adding, collapsing, and removing as needed.
     */
    public void updateWith(Collection<DownloadInfo> downloads) {
        final Resources res = mContext.getResources();

        try {
            // Build notification for each cluster
            for (Iterator infoIterator = downloads.iterator(); infoIterator.hasNext();) {
                DownloadInfo info = (DownloadInfo) infoIterator.next();
                String tagName = info.getTag();
                //Log.d(TAG, "download info:" + info);

                final Notification.Builder builder = new Notification.Builder(mContext);
                builder.setContentTitle(info.getTitle());
                builder.setWhen(System.currentTimeMillis());
                builder.setPriority(Notification.PRIORITY_HIGH);

                // Show relevant icon & Build action intents
                if (info.mState == State.RUNNING || info.mState == State.CONNECTING) {
                    builder.setSmallIcon(R.drawable.stat_sys_download);
                    Log.d(TAG, "running : " + info.mId);

                    final Intent intent = new Intent(HotKnotBeamService.HOTKNOT_BEAMING);
                    intent.putExtra(HotKnotBeamService.HOTKNOT_EXTRA_BEAM_ID, info.mId);
                    PendingIntent cancelIntent = PendingIntent.getBroadcast(mContext, info.mId, intent, 0);

                    String cancelWord = mContext.getString(R.string.cancel);
                    builder.setDeleteIntent(cancelIntent);
                    builder.setAutoCancel(false);
                    builder.addAction(R.drawable.ic_action_cancel, (CharSequence) cancelWord, cancelIntent);
                } else if (info.mState == State.COMPLETE) {
                    Log.d(TAG, "complete");
                    builder.setSmallIcon(R.drawable.stat_sys_download_done_static);
                    Uri uri = info.getUri();
                    String mimeType = info.getMimeType();

                    if(info.getResult() && uri != null && mimeType != null) {
                        final Intent intent = new Intent(Intent.ACTION_VIEW);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                        intent.setDataAndTypeAndNormalize(uri, mimeType);
                        Log.d(TAG, "complete intent:" + intent);
                        builder.setContentIntent(PendingIntent.getActivity(mContext, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT));
                    } else if(mimeType == null) {
                        final Intent intent = new Intent(HotKnotBeamService.HOTKNOT_DL_COMPLETE, null, mContext, HotKnotBeamReceiver.class);
                        builder.setContentIntent(PendingIntent.getBroadcast(mContext, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT));
                    }

                    if (info.getResult()) {
                        /* Disable sound in Notificaiton*/
                        //builder.setSound(Uri.parse("android.resource://" + mContext.getPackageName() + "/" + R.raw.magic));
                        //builder.setDefaults(Notification.DEFAULT_SOUND);
                        builder.setContentText(res.getText(R.string.notification_download_complete));
                    } else {
                        builder.setSound(Uri.parse("android.resource://" + mContext.getPackageName() + "/" + R.raw.error));
                        builder.setContentText(res.getText(R.string.notification_download_failed));
                    }
                    builder.setAutoCancel(true);
                    infoIterator.remove();
                }

                // Calculate and show progress
                String percentText = null;
                if (info.mState == State.CONNECTING) {
                    percentText = res.getString(R.string.download_percent, 0);
                    builder.setProgress(100, 0, false);
                } else if (info.mState == State.RUNNING) {
                    long current = info.mCurrentBytes;
                    long total = info.mTotalBytes;

                    if (total > 0) {
                        final int percent = (int) ((current * 100) / total);
                        percentText = res.getString(R.string.download_percent, percent);
                        builder.setProgress(100, percent, false);
                    } else {
                        builder.setProgress(100, 0, true);
                    }
                }

                // Build titles and description
                final Notification notif;
                if (info.mState == State.RUNNING || info.mState == State.CONNECTING) {
                    builder.setContentText(getDowloadRemadingText(info.mCurrentBytes, info.mTotalBytes));
                    builder.setContentInfo(percentText);
                }
                notif = builder.build();
                mNotifManager.notify(tagName, 0, notif);
            }
        } catch(ConcurrentModificationException e) {
            Log.e(TAG, "err msg:" + e.getMessage());
            return;
        }
    }

    private CharSequence getDowloadRemadingText(int progress, int max) {
        if(max > HotKnotBeamConstants.MAX_MB_SIZE) {
            mProgressFormat = HotKnotBeamConstants.MAX_MB_FORMAT;
            progress = (int) progress / HotKnotBeamConstants.MAX_MB_SIZE;
            max = (int) max / HotKnotBeamConstants.MAX_MB_SIZE;
        } else if(max > HotKnotBeamConstants.MAX_KB_SIZE) {
            mProgressFormat = HotKnotBeamConstants.MAX_KB_FORMAT;
            progress = (int) progress / HotKnotBeamConstants.MAX_KB_SIZE;
            max = (int) max / HotKnotBeamConstants.MAX_KB_SIZE;
        } else {
            mProgressFormat = HotKnotBeamConstants.MAX_FORMAT;
        }

        return String.format(mProgressFormat, progress, max);
    }
}
