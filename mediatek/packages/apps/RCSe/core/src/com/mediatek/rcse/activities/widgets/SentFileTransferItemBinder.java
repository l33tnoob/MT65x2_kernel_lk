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
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment.FileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;

import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * This is an item binder for sent message
 */
public class SentFileTransferItemBinder extends AbsItemBinder implements OnClickListener {
    public static final String TAG = "SentFileTransferItemBinder";
    private static final String COLON = ":";
    private static final String WHITE_SPACE = " ";
    private FileTransfer mFileTransfer = null;
    boolean mIsPause = false;

    public SentFileTransferItemBinder(FileTransfer fileTransfer) {
        mFileTransfer = fileTransfer;
    }


	@Override
    public void bindView(View itemView) {
        FileStruct fileStruct = mFileTransfer.getFileStruct();
        Logger.v(TAG, "bindView: fileStruct is " + fileStruct);
        if (null != fileStruct) {
            Status status = mFileTransfer.getStatue();
            switch (status) {
                case PENDING:
                case WAITING:
                case TRANSFERING:
                    bindProgressFile(fileStruct, itemView);
                    break;
                case CANCEL:
                    bindCancelView(itemView);
                    break;
                case CANCELED:
                    bindCanceledView(itemView);
                    break;
                case FAILED:
                    bindFailedView(itemView);
                    break;
                case TIMEOUT:
                    bindTimeoutView(itemView);
                    break;
                case REJECTED:
                    bindRejectView(itemView);
                    break;
                case FINISHED:
                    bindSendFileFinished(fileStruct, itemView);
                    break;
                default:
                    Logger.e(TAG, "bindView() unknown status type");
                    break;
            }
        }
    }

    @Override
    public int getLayoutResId() {
        if (mFileTransfer != null) {
            Status status = mFileTransfer.getStatue();
            switch (status) {
                case FAILED:
                case CANCELED:
                case TIMEOUT:
                case CANCEL:
                    return R.layout.chat_item_file_transfer_terminated;
                case REJECTED:
                    // The sender was rejected by remote, then should show
                    // resend view
                    return R.layout.chat_item_resent_file_transfer;
                default:
                    return R.layout.chat_item_sent_file_transfer;
            }
        } else {
            Logger.v(TAG, "mFileTransfer is null");
            return 0;
        }
    }

    @Override
    public int getItemType() {
        if (mFileTransfer != null) {
            Status status = mFileTransfer.getStatue();
            switch (status) {
                case PENDING:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_PENDING;
                case CANCELED:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_CANCELED;
                case CANCEL:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_CANCEL;
                case FAILED:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_FAILED;
                case REJECTED:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_REJECT;
                case FINISHED:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_FINISHED;
                case TIMEOUT:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_TIMEOUT;
                default:
                    return ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER;
            }
        } else {
            Logger.v(TAG, "mFileTransfer is null");
            return 0;
        }
    }

    private void bindProgressFile(FileStruct fileStruct, View itemView) {
        Logger.d(TAG, "bindProgressFile: itemView = " + itemView + ", fileStruct = " + fileStruct);
        if (null == itemView || null == fileStruct) {
            return;
        }

        DateView date = (DateView) itemView.findViewById(R.id.chat_item_date);
        date.setTime(fileStruct.mDate);
        FileTransferView fileTransferView = (FileTransferView) itemView
                .findViewById(R.id.file_transfer_view);
        if (fileTransferView != null) {
            fileTransferView.setFile(fileStruct.mFilePath);
        } else {
            Logger.w(TAG, "bindProgressFile(), fileTransferView is null!");
        }
        Status status = mFileTransfer.getStatue();
        TextView fileSize = (TextView) itemView.findViewById(R.id.file_transfer_size);
        RelativeLayout statusLayout = (RelativeLayout) itemView
                .findViewById(R.id.file_transfer_layout_status);
        TextView statusText = (TextView) itemView.findViewById(R.id.file_transfer_status);
        ProgressBar progressBar = (ProgressBar) itemView
                .findViewById(R.id.file_transfer_progress_bar);
        if (statusLayout != null) {
            statusLayout.setVisibility(View.VISIBLE);
        } else {
            Logger.e(TAG, "bindWaitingFile, statusLayout is null!");
        }
        ImageView playPauseView = (ImageView) itemView.findViewById(R.id.file_transfer_btn_play_pause);
        if(playPauseView != null){
        	playPauseView.setOnClickListener(this);
        } else {
            Logger.w(TAG, "playPauseView is null.");
        }

        if (progressBar != null) {
            switch (status) {
                case PENDING:
                    fileSize.setVisibility(View.GONE);
                    progressBar.setVisibility(View.INVISIBLE);
                    playPauseView.setVisibility(View.INVISIBLE);
                    statusText.setText(R.string.file_transfer_status_waiting);
                    break;
                case WAITING:
                    progressBar.setVisibility(View.VISIBLE);
                    statusText.setVisibility(View.VISIBLE);
                    playPauseView.setVisibility(View.INVISIBLE);
                    fileSize.setText(Utils.formatFileSizeToString(fileStruct.mSize, 0));
                    progressBar.setMax((int) fileStruct.mSize);
                    progressBar.setProgress(0);
                    statusText.setText(R.string.file_transfer_status_waiting);
                    break;
                case TRANSFERING:
                    progressBar.setVisibility(View.VISIBLE);
                    playPauseView.setVisibility(View.VISIBLE);
                    statusText.setVisibility(View.VISIBLE);
                    fileSize.setText(Utils.formatFileSizeToString(fileStruct.mSize, 0));
                    double numerator = (double) mFileTransfer.getProgress();
                    double denominator = (double) fileStruct.mSize;
                    String percent = NumberFormat.getPercentInstance().format(
                            numerator / denominator);
                    progressBar.setMax((int) fileStruct.mSize);
                    progressBar.setProgress((int) mFileTransfer.getProgress());
                    statusText.setText(percent);
                    break;
                default:
                    break;
            }
        } else {
            Logger.e(TAG, "bindWaitingFile, progressBar is null!");
        }

        ImageView cancelView = (ImageView) itemView.findViewById(R.id.file_transfer_btn_cancel);
        if (cancelView != null) {
            cancelView.setOnClickListener(this);
        } else {
            Logger.w(TAG, "cancelView is null.");
        }
    }

    private void bindCancelView(View itemView) {
        Context context = ApiManager.getInstance().getContext();
        Logger.v(TAG, "bindCancelView: itemView = " + itemView + ", context = " + context);
        if (itemView == null || context == null) {
            return;
        }

        if (mFileTransfer != null) {
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.append(context.getString(R.string.file_transfer_you));
            stringBuilder.append(WHITE_SPACE);
            stringBuilder.append(context.getString(R.string.file_transfer_canceled));
            stringBuilder.append(WHITE_SPACE);
            FileStruct fileStruct = mFileTransfer.getFileStruct();
            if (fileStruct != null) {
                stringBuilder.append(fileStruct.mName);
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(context.getString(R.string.file_transfer_at));
                stringBuilder.append(WHITE_SPACE);
                Date date = fileStruct.mDate;
                DateView dateView = new DateView(context);
                String timeString = dateView.convertTime(date);
                stringBuilder.append(timeString);
            } else {
                Logger.w(TAG, "fileStruct is null");
            }

            TextView displayTextView = (TextView) itemView
                    .findViewById(R.id.file_transfer_terminated);
            if (displayTextView != null) {
                displayTextView.setText(stringBuilder.toString());
            } else {
                Logger.w(TAG, "displayTextView is null");
            }
        } else {
            Logger.w(TAG, "mFileTransfer is null");
        }
    }

    // Create view when a file transfer was canceled by remote
    private void bindCanceledView(View itemView) {
        Context context = ApiManager.getInstance().getContext();
        Logger.v(TAG, "bindCanceledView: itemView = " + itemView + ", context = " + context);
        if (itemView == null || context == null) {
            return;
        }

        if (mFileTransfer != null) {
            StringBuilder stringBuilder = new StringBuilder();
            if(mFileTransfer instanceof One2OneChatFragment.SentFileTransfer)
            {
                stringBuilder.append(((One2OneChatFragment.SentFileTransfer)mFileTransfer).getContactName());
            }
            stringBuilder.append(WHITE_SPACE);
            stringBuilder.append(context.getString(R.string.file_transfer_canceled));
            stringBuilder.append(WHITE_SPACE);
            FileStruct fileStruct = mFileTransfer.getFileStruct();
            if (fileStruct != null) {
                stringBuilder.append(fileStruct.mName);
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(context.getString(R.string.file_transfer_at));
                stringBuilder.append(WHITE_SPACE);
                Date date = fileStruct.mDate;
                DateView dateView = new DateView(context);
                String timeString = dateView.convertTime(date);
                stringBuilder.append(timeString);
            } else {
                Logger.w(TAG, "fileStruct is null");
            }

            TextView displayTextView = (TextView) itemView
                    .findViewById(R.id.file_transfer_terminated);
            if (displayTextView != null) {
                displayTextView.setText(stringBuilder.toString());
            } else {
                Logger.w(TAG, "displayTextView is null");
            }
        } else {
            Logger.w(TAG, "mFileTransfer is null");
        }
    }

    private void bindFailedView(View itemView) {
        Context context = ApiManager.getInstance().getContext();
        Logger.v(TAG, "bindFailedView: itemView = " + itemView + ", context = " + context);
        if (itemView == null || context == null) {
            return;
        }

        TextView failedTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        String receiverOfflineText = context.getString(R.string.file_transfer_receiver_offline);
        if (failedTextView != null) {
            failedTextView.setText(receiverOfflineText);
        } else {
            Logger.w(TAG, "bindFailedView(), failedTextView is null");
        }
    }

    private void bindTimeoutView(View itemView) {
        Logger.v(TAG, "bindFailedView() entry");
        if (itemView == null) {
            Logger.w(TAG, "bindFailedView(), itemView is null!");
            return;
        }
        TextView timeoutTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        String timeoutText = null;
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            timeoutText = context.getString(R.string.file_transfer_receiver_timeout);
            if (timeoutTextView != null) {
                timeoutTextView.setText(timeoutText);
            } else {
                Logger.w(TAG, "bindFailedView(), failedTextView is null");
            }
        } else {
            Logger.w(TAG, "bindFailedView(), context is null");
        }
    }

    private void bindRejectView(View itemView) {
        Logger.v(TAG, "bindRejectView: itemView = " + itemView);
        if (null == itemView) {
            return;
        }

        DateView date = (DateView) itemView.findViewById(R.id.chat_item_date);
        Button resendView = (Button) itemView.findViewById(R.id.file_transfer_resent);
        if (resendView != null) {
            resendView.setOnClickListener(this);
        } else {
            Logger.e(TAG, "resendView is null!");
        }
        TextView textView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        setRejectText(textView);
        FileStruct fileStruct = null;
        if (mFileTransfer != null && mFileTransfer.getFileStruct() != null) {
            fileStruct = mFileTransfer.getFileStruct();
            date.setTime(fileStruct.mDate);
            FileTransferView fileTransferView = (FileTransferView) itemView
                    .findViewById(R.id.file_transfer_view);
            if (fileTransferView != null) {
                fileTransferView.setFile(fileStruct.mFilePath);
            } else {
                Logger.w(TAG, "bindRejectView(), fileTransferView is null ");
            }
        } else {
            Logger.w(TAG, "mFileTransfer is null, or fileStruct is null ");
        }
    }

    private void setRejectText(TextView textView) {
        Logger.v(TAG, "setRejectText()");
        Context context = ApiManager.getInstance().getContext();
        String rejectTextString = null;
        if (context != null) {
            rejectTextString = context.getString(R.string.file_transfer_rejected);
        } else {
            Logger.w(TAG, "context is null");
        }
        if (textView != null) {
            StringBuilder stringBuilder = new StringBuilder();
            if (mFileTransfer != null) {
            	 if(mFileTransfer instanceof One2OneChatFragment.SentFileTransfer)
                 {
                    stringBuilder.append(((One2OneChatFragment.SentFileTransfer)mFileTransfer).getContactName());
                 }
                stringBuilder.append(WHITE_SPACE);
                stringBuilder.append(rejectTextString);
                stringBuilder.append(WHITE_SPACE);
                FileStruct fileStruct = mFileTransfer.getFileStruct();
                if (fileStruct != null) {
                    stringBuilder.append(fileStruct.mName);
                    stringBuilder.append(WHITE_SPACE);
                    stringBuilder.append(context.getString(R.string.file_transfer_at));
                    stringBuilder.append(WHITE_SPACE);
                    Date date = fileStruct.mDate;
                    DateView dateView = new DateView(context);
                    String timeString = dateView.convertTime(date);
                    Logger.d(TAG, "setRejectText the time is " + timeString);
                    stringBuilder.append(timeString);
                } else {
                    Logger.w(TAG, "fileStruct is null");
                }
                String textString = stringBuilder.toString();
                Logger.v(TAG, "textString = " + textString);
                textView.setText(textString);
            } else {
                Logger.w(TAG, "mFileTransfer is null");
            }
        } else {
            Logger.e(TAG, "textView is null!");
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            case R.id.file_transfer_btn_cancel:
                Logger.v(TAG, "Sender cancel the file transfer.");
                handleCancelFileTransfer();
                break;
            case R.id.file_transfer_resent:
                Logger.v(TAG, "Sender resend the file.");
                handleResendFileTransfer();
                break;
                
            case R.id.file_transfer_btn_status:
            	Logger.v(TAG, "Sender press status button on file view");
                handleStatusFileTransfer(v.getContext());
            	break;
            	
            case R.id.file_transfer_btn_play_pause:
                Logger.v(TAG, "Sender play/pause the file transfer.");
                handlePlayPauseFileTransfer(v);
                break;
            default:
                break;
        }
    }

    private void handleCancelFileTransfer() {
        Logger.v(TAG, "handleCancelFileTransfer: mFileTransfer = " + mFileTransfer);
        if (mFileTransfer != null) {
            mFileTransfer.setStatus(Status.CANCEL);
            FileStruct fileStruct = mFileTransfer.getFileStruct();
            if (fileStruct != null) {
                Message message = ControllerImpl.getInstance().obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_CANCEL, mFileTransfer.getTag(),
                        fileStruct.mFileTransferTag);
                if (message != null) {
                    ControllerImpl.getInstance().sendMessage(message);
                } else {
                    Logger.w(TAG, "message is null.");
                }
            } else {
                Logger.w(TAG, "fileStruct is null.");
            }
        }
    }

    private void handlePlayPauseFileTransfer(View v) {
        Logger.v(TAG, "handlePlayPauseFileTransfer: mFileTransfer = " + mFileTransfer);
        if (mFileTransfer != null) {
            //mFileTransfer.setStatus(Status.CANCEL); 
            FileStruct fileStruct = mFileTransfer.getFileStruct();
            if (fileStruct != null) {
            	if(mIsPause == false)
            	{
	                Message message = ControllerImpl.getInstance().obtainMessage(
	                        ChatController.EVENT_FILE_TRANSFER_PAUSE, mFileTransfer.getTag(),
	                        fileStruct.mFileTransferTag);
	                message.arg1 = 0; //sent file case
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
	                        ChatController.EVENT_FILE_TRANSFER_RESUME, mFileTransfer.getTag(),
	                        fileStruct.mFileTransferTag);
	                message.arg1 = 0; //sent file case
	                if (message != null) {
	                    ControllerImpl.getInstance().sendMessage(message);
	                } else {
	                    Logger.w(TAG, "message is null.");
	                }
	                mIsPause = false;
	                //set pause drawable
	                v.setBackgroundResource(android.R.drawable.ic_media_pause);
            	}
            } else {
                Logger.w(TAG, "fileStruct is null.");
            }
        }
    }

    private void handleStatusFileTransfer(Context context) {
        Logger.v(TAG, "handleStatusFileTransfer: mFileTransfer = " + mFileTransfer);
        ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
    	List<Participant> participants= ((GroupChatWindow) window).getmGroupChatFragment().getParticipants();
    	
    	Intent intent = new Intent(InvitationDialog.ACTION);
        //intent.putExtra(RcsNotification.CONTACT, mFile.getContactNum());
        //intent.putExtra(RcsNotification.SESSION_ID, mFileStruct.mFileTransferTag.toString());
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_GROUP_FILE_VIEW_STATUS);
        //intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, true);
        //String content = context.getString(R.string.file_size_warning_contents);
        //intent.putExtra(RcsNotification.NOTIFY_CONTENT, content);
        //intent.putExtra(RcsNotification.NOTIFY_SIZE, String.valueOf(fileSize));
        //intent.putExtra("participants", participants);
        intent.putParcelableArrayListExtra(Utils.CHAT_PARTICIPANTS, (ArrayList<Participant>)participants);
        context.startActivity(intent);
    }

    private void bindSendFileFinished(FileStruct fileStruct, View itemView) {
        Logger.v(TAG, "bindSendFileFinished: itemView = " + itemView + ", fileStruct = "
                + fileStruct);
        if (null == itemView || null == fileStruct) {
            return;
        }

        DateView date = (DateView) itemView.findViewById(R.id.chat_item_date);
        FileTransferView fileTransferView = (FileTransferView) itemView
                .findViewById(R.id.file_transfer_view);
        if (fileTransferView != null) {
            fileTransferView.setFile(fileStruct.mFilePath);
        } else {
            Logger.w(TAG, "bindSendFileFinished(), fileTransferView is null!");
        }
        RelativeLayout statusLayout = (RelativeLayout) itemView
                .findViewById(R.id.file_transfer_layout_status);
        statusLayout.setVisibility(View.GONE);
        date.setTime(fileStruct.mDate);
        ImageView statusView = (ImageView) itemView.findViewById(R.id.file_transfer_btn_status);
        if (statusView != null) {
        	statusView.setOnClickListener(this);
        } else {
            Logger.w(TAG, "cancelView is null.");
        }
    }

    private void handleResendFileTransfer() {
        if (mFileTransfer != null) {
            FileStruct fileStruct = mFileTransfer.getFileStruct();
            if (fileStruct != null) {
                Message message = ControllerImpl.getInstance().obtainMessage(
                        ChatController.EVENT_FILE_TRANSFER_RESENT, mFileTransfer.getTag(),
                        fileStruct.mFileTransferTag);
                if (message != null) {
                    ControllerImpl.getInstance().sendMessage(message);
                } else {
                    Logger.w(TAG, "message is null.");
                }
            } else {
                Logger.w(TAG, "fileStruct is null.");
            }
        } else {
            Logger.w(TAG, "mFileTransfer is null.");
        }
    }
}
