/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.activities.widgets;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.os.Message;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.webkit.MimeTypeMap;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedFileTransfer;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;

import java.text.NumberFormat;
import java.util.Date;
import java.util.List;

/**
 * This is an item binder for receive file transfer invitation
 */
public class ReceivedFileTransferItemBinder extends AbsItemBinder implements OnClickListener {
    public static final String TAG = "ReceivedFileTransferItemBinder";
    public static final String COLON = ":";
    public static final String DOT = ".";
    public static final String FILEPATH_NO_SPACE = "sdcard no space";
    private static final String WHITE_SPACE = " ";
    private static final double FILE_SIZE_ZERO = 0.0;
    private static final String PERCENT_ZERO = "0%";
    private ReceivedFileTransfer mFile = null;
    private FileStruct mFileStruct;
    boolean mIsPause = false;

    public ReceivedFileTransferItemBinder(ReceivedFileTransfer file) {
        Logger.v(TAG, "ReceivedFileTransferItemBinder: ReceiveFileTransfer = " + file);
        mFile = file;
        if (mFile != null) {
            mFileStruct = mFile.getFileStruct();
        }
    }

    @Override
    public void bindView(View itemView) {
        Logger.v(TAG, "bindView: itemView = " + itemView + ", mFileStruct = " + mFileStruct);
        if (null == itemView || null == mFileStruct) {
            return;
        }

        Status status = mFile.getStatue();
        switch (status) {
            case WAITING:
                bindReceiveInvitationView(itemView);
                break;
            case FAILED:
                bindFailedView(itemView);
                break;
            case REJECTED:
                bindRejectView(itemView);
                break;
            case CANCEL:
                bindCancelView(itemView);
                break;
            case CANCELED:
                bindCanceledView(itemView);
                break;
            case TRANSFERING:
                bindAcceptView(itemView);
                break;
            case FINISHED:
                bindFinishedView(itemView);
                break;
            default:
                Logger.e(TAG, "bindView() unknown status type");
                break;
        }
    }

    @Override
    public int getLayoutResId() {
        Logger.v(TAG, "bindView: mFileStruct = " + mFileStruct);

        if (null == mFile || null == mFileStruct) {
            return 0;
        }

        Status status = mFile.getStatue();
        switch (status) {
            case WAITING:
                return R.layout.chat_item_received_file_transfer;
            case FAILED:
            case REJECTED:
            case CANCELED:
            case CANCEL:
                return R.layout.chat_item_file_transfer_terminated;
            case TRANSFERING:
                return R.layout.chat_item_received_file_transfer_accept;
            case FINISHED:
                return R.layout.chat_item_received_file_transfer_finished;
            default:
                Logger.e(TAG, "bindView() unknown status type");
                return 0;
        }

    }

    @Override
    public int getItemType() {
        Logger.v(TAG, "getItemType: mFileStruct = " + mFileStruct);

        if (null == mFileStruct) {
            return 0;
        }

        Status status = mFile.getStatue();
        switch (status) {
            case WAITING:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER;
            case FAILED:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER_FAILED;
            case REJECTED:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER_REJECT;
            case CANCEL:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER_CANCEL;
            case CANCELED:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER_CANCELED;
            case TRANSFERING:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER_ACCEPT;
            case FINISHED:
                return ChatAdapter.ITEM_TYPE_RECEIVED_FILE_TRANSFER_FINISHED;
            default:
                Logger.e(TAG, "bindView() unknown status type");
                return 0;
        }

    }

    private View bindReceiveInvitationView(View itemView) {
        AsyncAvatarView avatarImageView = (AsyncAvatarView) itemView.findViewById(R.id.peer_avatar);
        TextView displayTextView = (TextView) itemView.findViewById(R.id.file_transfer_display);
        ImageView imageType = (ImageView) itemView.findViewById(R.id.image_type);
        TextView fileSize = (TextView) itemView.findViewById(R.id.file_transfer_size);
        DateView dateTextView = (DateView) itemView.findViewById(R.id.file_transfer_time);
        Button acceptBtn = (Button) itemView.findViewById(R.id.file_transfer_accept);
        Button rejectBtn = (Button) itemView.findViewById(R.id.file_transfer_reject);
        acceptBtn.setOnClickListener(this);
        rejectBtn.setOnClickListener(this);

        Context context = ApiManager.getInstance().getContext();
        if (null == context) {
            Logger.e(TAG, "Get system context is null!");
            return itemView;
        }

        Date date = mFileStruct.mDate;
        String phoneNum = mFile.getContactNum();
        if (phoneNum != null) {
            avatarImageView.setAsyncContact(phoneNum);
        } else {
            Logger.w(TAG, "bindReceiveInvitationView, phoneNum is null!");
        }
        avatarImageView.setVisibility(View.VISIBLE);

        displayTextView.setText(mFileStruct.mName);
        String mimeType = MediaFile.getMimeTypeForFile(mFileStruct.mName);
        if (mimeType == null) {
            mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                    Utils.getFileExtension(mFileStruct.mName));
        }
        String type = context.getString(R.string.file_type_file);
        if (mimeType != null) {
            if (mimeType.contains(Utils.FILE_TYPE_IMAGE)) {
                type = context.getString(R.string.file_type_image);
            } else if (mimeType.contains(Utils.FILE_TYPE_AUDIO)) {
                type = context.getString(R.string.file_type_audio);
            } else if (mimeType.contains(Utils.FILE_TYPE_VIDEO)) {
                type = context.getString(R.string.file_type_video);
            } else if (mimeType.contains(Utils.FILE_TYPE_TEXT)) {
                type = context.getString(R.string.file_type_text);
            } else if (mimeType.contains(Utils.FILE_TYPE_APP)) {
                type = context.getString(R.string.file_type_app);
            }
        }
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(Utils.getFileNameUri(mFileStruct.mName), mimeType);
        PackageManager packageManager = context.getPackageManager();
        List<ResolveInfo> list = packageManager.queryIntentActivities(intent,
                PackageManager.MATCH_DEFAULT_ONLY);
        int size = list.size();
        Drawable drawable = context.getResources()
                .getDrawable(R.drawable.rcs_ic_ft_default_preview);
        if (size > 0) {
            drawable = list.get(0).activityInfo.loadIcon(packageManager);
        }
        String thumbNail= mFileStruct.mThumbnail;
        if(thumbNail != null)
        {
        imageType.setImageDrawable(Drawable.createFromPath(thumbNail));
        }
        else
        {
        	imageType.setImageDrawable(drawable);
        }
        fileSize.setText(context.getString(R.string.file_transfer_size)
                + Utils.formatFileSizeToString(mFileStruct.mSize, 0));

        dateTextView.setTime(date);

        return itemView;
    }

    private View bindFailedView(View itemView) {
        TextView failedTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        setFailedText(failedTextView);
        return itemView;
    }

    private void setFailedText(TextView textView) {
        Logger.v(TAG, "setFailedText entry: textView = " + textView);
        if (null == textView) {
            return;
        }

        Context context = ApiManager.getInstance().getContext();
        String failedText = null;
        String timedOutText = null;
        if (context != null) {
            failedText = context.getString(R.string.file_transfer_failed);
            timedOutText = context.getString(R.string.invitation_timeout_title);
        }

        if (null != mFile) {
            StringBuilder stringBuilder = new StringBuilder();
            FileStruct fileStruct = mFile.getFileStruct();
            if (null != fileStruct) {
                stringBuilder.append(fileStruct.mName);
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(failedText);
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(context.getString(R.string.file_transfer_at));
                stringBuilder.append(WHITE_SPACE);
                Date date = fileStruct.mDate;
                DateView dateView = new DateView(context);
                String time = dateView.convertTime(date);
                stringBuilder.append(time);
                stringBuilder.append(DOT);
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(timedOutText);
            }

            String text = stringBuilder.toString();
            Logger.d(TAG, "setFailedText exit: text = " + text);
            textView.setText(text);
        }
    }

    private View bindRejectView(View itemView) {
        TextView displayTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        setRejectText(displayTextView);
        return itemView;
    }

    private void setRejectText(TextView textView) {
        Logger.v(TAG, "setRejectText entry: textView = " + textView);
        if (null == textView) {
            return;
        }

        Context context = ApiManager.getInstance().getContext();
        String rejectTextString = null;
        String you = null;
        if (context != null) {
            rejectTextString = context.getString(R.string.file_transfer_rejected);
            you = context.getString(R.string.file_transfer_you);
        }

        if (null != mFile) {
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.append(you);
            stringBuilder.append(WHITE_SPACE);
            stringBuilder.append(rejectTextString);
            stringBuilder.append(WHITE_SPACE);

            FileStruct fileStruct = mFile.getFileStruct();
            if (null != fileStruct) {
                stringBuilder.append(fileStruct.mName);
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(context.getString(R.string.file_transfer_at));
                stringBuilder.append(WHITE_SPACE);
                Date date = fileStruct.mDate;
                DateView dateView = new DateView(context);
                String timeString = dateView.convertTime(date);
                stringBuilder.append(timeString);
            }

            String textString = stringBuilder.toString();
            Logger.v(TAG, "setRejectText exit: textString = " + textString);
            textView.setText(textString);
        }
    }

    private View bindAcceptView(View itemView) {
        AsyncAvatarView avatarImageView = (AsyncAvatarView) itemView.findViewById(R.id.peer_avatar);
        TextView displayTextView = (TextView) itemView.findViewById(R.id.file_transfer_size);

        Button cancelBtn = (Button) itemView.findViewById(R.id.file_transfer_cancel);
        Button playPauseBtn = (Button) itemView.findViewById(R.id.file_transfer_play_pause);
        ProgressBar progressBar = (ProgressBar) itemView.findViewById(R.id.progress);
        cancelBtn.setOnClickListener(this);
        playPauseBtn.setOnClickListener(this);

        if (mFile != null && mFileStruct != null) {
            double numerator = (double) mFile.getProgress();
            double denominator = (double) mFileStruct.mSize;
            double percentvalue;
            String percent = null;
            if (numerator == FILE_SIZE_ZERO || denominator == FILE_SIZE_ZERO) {
                percent = PERCENT_ZERO;
            } else {
                percentvalue = numerator/denominator;
                if(percentvalue > 1.00)
                percent = "100%";
                else
                percent = NumberFormat.getPercentInstance().format(numerator / denominator);
            }
            String phoneNum = mFile.getContactNum();
            if (phoneNum != null) {
                avatarImageView.setAsyncContact(phoneNum);
            }

            avatarImageView.setVisibility(View.VISIBLE);
            displayTextView.setText(percent);
            if (progressBar != null) {
                progressBar.setMax((int) mFileStruct.mSize);
                if (mFile.getProgress() != -1) {
                    progressBar.setProgress((int) mFile.getProgress());
                } else {
                    progressBar.setProgress(0);
                }
            }
        }

        return itemView;
    }

    private View bindFinishedView(View itemView) {
        AsyncAvatarView avatarImageView = (AsyncAvatarView) itemView.findViewById(R.id.peer_avatar);
        DateView dateView = (DateView) itemView.findViewById(R.id.chat_item_date);
        Logger.d(TAG, "bindFinishedView: avatarImageView  = " + avatarImageView + ", dateView = "
                + dateView);
        if (avatarImageView != null && dateView != null) {
            String phoneNum = mFile.getContactNum();
            if (phoneNum != null) {
                avatarImageView.setAsyncContact(phoneNum);
            }

            avatarImageView.setVisibility(View.VISIBLE);
            FileTransferView fileTransferView = (FileTransferView) itemView
                    .findViewById(R.id.file_transfer_view);
            Logger.d(TAG, "bindFinishedView: fileTransferView = " + fileTransferView);
            if (fileTransferView != null && mFileStruct != null) {
                dateView.setTime(mFileStruct.mDate);
                fileTransferView.setFile(mFileStruct.mFilePath);
            }
            return itemView;
        }

        return itemView;
    }

    private void bindCancelView(View itemView) {
        Context context = ApiManager.getInstance().getContext();
        Logger.v(TAG, "bindCancelView entry: itemView = " + itemView + ", context = " + context
                + ", mFileTransfer = " + mFile);
        if (itemView == null || null == context || null == mFile) {
            return;
        }

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(context.getString(R.string.file_transfer_you));
        stringBuilder.append(WHITE_SPACE);
        stringBuilder.append(context.getString(R.string.file_transfer_canceled));
        stringBuilder.append(WHITE_SPACE);
        stringBuilder.append(mFile.getContactName());
        stringBuilder.append(WHITE_SPACE);
        stringBuilder.append(context.getString(R.string.file_transfer_at));
        stringBuilder.append(WHITE_SPACE);
        FileStruct fileStruct = mFile.getFileStruct();
        Logger.d(TAG, "bindCancelView: fileStruct = " + fileStruct);
        if (fileStruct != null) {
            Date date = fileStruct.mDate;
            DateView dateView = new DateView(context);
            String timeString = dateView.convertTime(date);
            stringBuilder.append(timeString);
            if (FILEPATH_NO_SPACE.equals(fileStruct.mFilePath)) {
                Logger.d(TAG, "bindCancelView: sdcard has no enough space");
                fileStruct.mFilePath = null;
                String noSpaceText = context.getResources().getString(
                        R.string.file_transfer_receiver_no_space);
                Toast toast = Toast.makeText(context, noSpaceText, Toast.LENGTH_LONG);
                toast.show();
            } else {
                Logger.d(TAG, "bindCancelView: sdcard has enough space");
            }
        }

        TextView displayTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        if (displayTextView != null) {
            displayTextView.setText(stringBuilder.toString());
        }
    }

    // Create view when a file transfer was canceled by remote
    private void bindCanceledView(View itemView) {
        Context context = ApiManager.getInstance().getContext();
        Logger.v(TAG, "bindCanceledView entry: itemView = " + itemView + ", context = " + context
                + ", mFile = " + mFile);
        if (null == itemView || null == context || null == mFile) {
            return;
        }

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(mFile.getContactName());
        stringBuilder.append(WHITE_SPACE);
        stringBuilder.append(context.getString(R.string.file_transfer_canceled));
        stringBuilder.append(WHITE_SPACE);
        FileStruct fileStruct = mFile.getFileStruct();
        Logger.d(TAG, "bindCancelView: fileStruct = " + fileStruct);
        if (fileStruct != null) {
            stringBuilder.append(fileStruct.mName);
            stringBuilder.append(WHITE_SPACE);
            stringBuilder.append(context.getString(R.string.file_transfer_at));
            stringBuilder.append(WHITE_SPACE);
            Date date = fileStruct.mDate;
            DateView dateView = new DateView(context);
            String timeString = dateView.convertTime(date);
            stringBuilder.append(timeString);
        }

        TextView displayTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        if (displayTextView != null) {
            displayTextView.setText(stringBuilder.toString());
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.file_transfer_accept:
                onUserAccept(v.getContext());
                break;
            case R.id.file_transfer_reject:
                handleReceiverReject();
                break;
            case R.id.file_transfer_cancel:
                handleCancelFileTransfer();
                break;
            case R.id.file_transfer_play_pause:
                handlePlayPauseFileTransfer(v);
                break;
            default:
                break;
        }
    }

    private void handleReceiverReject() {
        Logger.v(TAG, "handleReceiverReject entry: mFile = " + mFile);
        if (mFile == null) {
            return;
        }

        mFile.setStatus(Status.REJECTED);
        ControllerImpl controller = ControllerImpl.getInstance();
        if (null != controller) {
            Message controllerMessage = controller.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_RECEIVER_REJECT, mFile.getTag(),
                    mFileStruct.mFileTransferTag);
            controllerMessage.sendToTarget();
        }
    }

    private void onUserAccept(Context context) {
        long maxFileSize = ApiManager.getInstance().getMaxSizeforFileThransfer();
        long warningFileSize = ApiManager.getInstance().getWarningSizeforFileThransfer();
        Logger.v(TAG, "onUserAccept entry: maxFileSize = " + maxFileSize + ", warningFileSize = "
                + warningFileSize);

        long fileSize = mFileStruct.mSize;
        if (fileSize >= warningFileSize && warningFileSize != 0) {
            SharedPreferences sPrefer = PreferenceManager.getDefaultSharedPreferences(context);
            Boolean isRemind = sPrefer.getBoolean(SettingsFragment.RCS_REMIND, false);
            Logger.w(TAG, "WarningDialog: isRemind = " + isRemind);
            if (isRemind) {
                Intent intent = new Intent(InvitationDialog.ACTION);
                intent.putExtra(RcsNotification.CONTACT, mFile.getContactNum());
                intent.putExtra(RcsNotification.SESSION_ID, mFileStruct.mFileTransferTag.toString());
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_FILE_TRANSFER_SIZE_WARNING);
                intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, true);
                String content = context.getString(R.string.file_size_warning_contents);
                intent.putExtra(RcsNotification.NOTIFY_CONTENT, content);
                intent.putExtra(RcsNotification.NOTIFY_SIZE, String.valueOf(fileSize));
                context.startActivity(intent);
            } else {
                handleReceiverAccept();
            }
        } else {
            handleReceiverAccept();
        }

        Logger.v(TAG, "onUserAccept exit");
    }

    private void handleReceiverAccept() {
        Logger.v(TAG, "handleReceiverAccept entry: mFile = " + mFile);
        if (null == mFile) {
            return;
        }

        mFile.setStatus(Status.TRANSFERING);
        ControllerImpl controller = ControllerImpl.getInstance();
        if (null != controller) {
            Message controllerMessage = controller.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_RECEIVER_ACCEPT, mFile.getTag(),
                    mFileStruct.mFileTransferTag);
            controllerMessage.sendToTarget();
        }
    }

    private void handleCancelFileTransfer() {
        Logger.v(TAG, "handleCancelFileTransfer entry: mFile = " + mFile);
        if (null == mFile) {
            return;
        }

        mFile.setStatus(Status.CANCEL);
        FileStruct fileStruct = mFile.getFileStruct();
        Logger.d(TAG, "handleCancelFileTransfer: fileStruct = " + fileStruct);
        if (fileStruct != null) {
            ControllerImpl controller = ControllerImpl.getInstance();
            if (null != controller) {
                Message message = controller.obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_CANCEL, mFile.getTag(),
                        fileStruct.mFileTransferTag);
                ControllerImpl.getInstance().sendMessage(message);
            }
        }
    }
    
    private void handlePlayPauseFileTransfer(View v) {
        Logger.v(TAG, "handlePlayPauseFileTransfer entry: mFile = " + mFile);
        if (null == mFile) {
            return;
        }

        //mFile.setStatus(Status.CANCEL);
        FileStruct fileStruct = mFile.getFileStruct();
        Logger.d(TAG, "handlePlayPauseFileTransfer: fileStruct = " + fileStruct);
        if (fileStruct != null) {
        	if(mIsPause == false)
        	{
                Message message = ControllerImpl.getInstance().obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_PAUSE, mFile.getTag(),
                        fileStruct.mFileTransferTag);
                message.arg1 = 1; //received file case
                if (message != null) {
                    ControllerImpl.getInstance().sendMessage(message);
                } else {
                    Logger.w(TAG, "message is null.");
                }
                mIsPause = true;
                //set resume drawable
                v.setBackgroundResource(android.R.drawable.ic_media_play);
        	}
        	else
        	{
        		Message message = ControllerImpl.getInstance().obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_RESUME, mFile.getTag(),
                        fileStruct.mFileTransferTag);
                message.arg1 = 1; //received file case
                if (message != null) {
                    ControllerImpl.getInstance().sendMessage(message);
                } else {
                    Logger.w(TAG, "message is null.");
                }
                mIsPause = false;
                //set pause drawable
                v.setBackgroundResource(android.R.drawable.ic_media_pause);
        	}
        }
    }
}
