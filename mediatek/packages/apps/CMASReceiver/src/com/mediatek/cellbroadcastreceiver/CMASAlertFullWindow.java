package com.mediatek.cellbroadcastreceiver;

import android.app.Activity;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Typeface;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.TelephonyManager;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Iterator;

import com.mediatek.cmas.ext.ICmasMessageInitiation;

public class CMASAlertFullWindow {

    private static CMASAlertFullWindow sInstance = null;
    public static ArrayList<ViewInfo> sShowingView = null;

    private static final String TAG = "[CMAS][CMASAlertFullWindow]";
    private static final int PRESIDENT_ALERT_ID = 4370;

    protected Context mContext;
    ArrayList<CellBroadcastMessage> mMessageList;

    /** Handler to add and remove screen on flags for emergency alerts. */
    private final ScreenOffHandler mScreenOffHandler = new ScreenOffHandler();

    //wheather this dialog is start autoly(true) or start from message list(false)
    boolean mStartAuto;

    private static WindowManager sWindowManager;

    private TelephonyManager mTm = null;

    private static final int CLEAR_SCREEN_FLAG = 1;

    private static final int ALERT_TIME_LENGTH = 10500;//10.5s

    private class ScreenOffHandler extends Handler {

        /** Package local constructor (called from outer class). */
        ScreenOffHandler() {}

        /** Clear the screen on window flags. */
        private void clearWindowFlags(ViewInfo viewInfo) {
            Log.i(TAG, "enter clearWindowFlags");
            WindowManager.LayoutParams lp = viewInfo.getLayoutParams();
            if (lp != null && lp != null && isMsgShowing(viewInfo.getMsgRowId())) {
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON));
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));

                try {
                    sWindowManager.updateViewLayout(viewInfo.getView(), lp);
                } catch (IllegalArgumentException e) {
                    // TODO: handle exception
                    e.printStackTrace();
                    Log.e(TAG, "updateViewLayout 1 exception, view not attached");
                }
            }
        }
        
        private void clearWindowFlags(View view, WindowManager.LayoutParams lp) {
            Log.i(TAG, "clearWindowFlags ++");
            if (lp != null && lp != null) {
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON));
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
                
                try {
                    sWindowManager.updateViewLayout(view, lp);
                } catch (IllegalArgumentException e) {
                    // TODO: handle exception
                    e.printStackTrace();
                    Log.e(TAG, "updateViewLayout 2 exception, view not attached");
                }

            }
        }

        @Override
        public void handleMessage(Message msg) {
            int msgWhat = msg.what;
            Log.i(TAG, "HandleMessage msgWhat = " + msgWhat);
            switch (msgWhat) {
                case CLEAR_SCREEN_FLAG:
                    clearWindowFlags((ViewInfo)msg.obj);
                    break;

                default:
                    break;
            }
        }
    }

    private BroadcastReceiver mBr = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            Log.i(TAG, "onReceiver: action = " + intent.getAction());
            if (intent.getAction().equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                TelephonyManager tm = (TelephonyManager)context.getSystemService(context.TELEPHONY_SERVICE);
                int callState = tm.getCallState();
                Log.i(TAG, "onReceiver: callState = " + callState);
                if (callState == TelephonyManager.CALL_STATE_IDLE) {
                    //update all showing views to not touchable
                    updateShowingView(false);
                } else {
                    //update all showing views to touchable
                    updateShowingView(true);
                }
            }
        }
    };

    public static synchronized CMASAlertFullWindow getInstance(Context ctx) {
        if (sInstance == null) {
            Log.i(TAG, "mInstance == null");
            sInstance = new CMASAlertFullWindow(ctx);
        } else {
            Log.i(TAG, "mInstance != null");
        }

        if (sShowingView == null) {
            sShowingView = new ArrayList<ViewInfo>();
        }

        return sInstance;
    }

    public CMASAlertFullWindow(Context ctx) {
        mContext = ctx;

        sWindowManager = (WindowManager)ctx.getSystemService(Activity.WINDOW_SERVICE);

        if (mTm == null) {
            mTm = (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        ctx.registerReceiver(mBr, filter);
    }

    private CMASAlertLinearLayout initViewToShow(final CellBroadcastMessage message, final long msgRowId, final boolean bAutoStart) {

        LayoutInflater inflater = LayoutInflater.from(mContext);
        final CMASAlertLinearLayout view = (CMASAlertLinearLayout) inflater.inflate(getLayoutResId(), null);

        view.setOnKeyListener(new View.OnKeyListener() {
            
            public boolean onKey(View arg0, int arg1, KeyEvent arg2) {
                // TODO Auto-generated method stub
                Log.i(TAG, "onKey:: arg1 = " + arg1);
                return false;
            }
        });
        ((Button)view.findViewById(R.id.dismissButton)).setOnClickListener(new View.OnClickListener() {

            public void onClick(View arg0) {
                Log.i(TAG, "ok button is clicked, msgRowId = " + msgRowId);
                clearNotification(bAutoStart);
                //clearViewFlag(view);
                dismissAndMarkRead(view, message, msgRowId);
                ((Button)view.findViewById(R.id.dismissButton)).setOnClickListener(null);

                rmViewByRowId(msgRowId);
                if (!sShowingView.isEmpty()) {
                    Log.i(TAG, "after dismiss current dialog, there are still some other window showing");
                    //get the highest priority view and set top property
                    final ViewInfo viewInfo = getHighestPriorityShowingView();

                    if (viewInfo != null) {
                        Log.i(TAG, "get highest view to show");
                        sWindowManager.removeView(viewInfo.getView());
                        viewInfo.getLayoutParams().type = WindowManager.LayoutParams.TYPE_TOP_MOST;
                        WindowManager.LayoutParams lp  = initLayoutParams(WindowManager.LayoutParams.TYPE_TOP_MOST);

                        //update viewinfo's parameters stored in arraylist
                        int index = sShowingView.indexOf(viewInfo);
                        sShowingView.remove(viewInfo);
                        viewInfo.setLayoutParams(lp);
                        sShowingView.add(index, viewInfo);

                        sWindowManager.addView(viewInfo.getView(), lp);
                    }
                }
            }
        });

        updateAlertText(view, message, msgRowId);
        updateAlertIcon(view, message);

        return view;
    }

    public void showView(final CellBroadcastMessage message, final long msgRowId, final boolean bAutoStart) {
        Log.i(TAG, "showView ++, msgid = " + message.getServiceCategory() + " msgRowId = " + msgRowId);

        if (message == null) {
            Log.i(TAG, "showView: mMessageList == null, return");
            return;
        }

        //if the alert message already showing
        if (isMsgShowing(msgRowId)) {
            Log.i(TAG, "message already showing");
            return;
        }

        WindowManager.LayoutParams lp;
        if (!sShowingView.isEmpty()) {
            //there is some dialog already showing
            Log.i(TAG, "some other msgs already showing");
            ViewInfo topViewInfo = getTopShowedViewInfo(sShowingView);
            if (CMASUtils.getMsgPriority(message.getServiceCategory()) <= topViewInfo.getPriority()) {
                Log.i(TAG, "new come message priority lower than top showed message");
                lp = initLayoutParams(WindowManager.LayoutParams.TYPE_PRIORITY_PHONE);
            } else {
                Log.i(TAG, "new come message priority higher than top showed message");
                lp = initLayoutParams(WindowManager.LayoutParams.TYPE_TOP_MOST);
            }
        } else {
            //there is no dialog showing
            lp = initLayoutParams(WindowManager.LayoutParams.TYPE_TOP_MOST);
        }

        int callState = TelephonyManager.CALL_STATE_IDLE;
        if (mTm != null) {
            callState = mTm.getCallState();
            Log.i(TAG, "call callState = " + callState);
        }
        if (allowAnswerCallBeforeAlertDismmss() && callState != TelephonyManager.CALL_STATE_IDLE) {
            Log.i(TAG, "phone is activate, and allow operation on phone before CMAS alert dismiss");
            lp = updateViewLpToTouchable(lp, true);
        }

        CMASAlertLinearLayout view = initViewToShow(message, msgRowId, bAutoStart);

        ViewInfo vInfo = new ViewInfo(msgRowId, view, CMASUtils.getMsgPriority(message.getServiceCategory()), lp);
        sShowingView.add(vInfo);

        if (sWindowManager == null) {
            sWindowManager = (WindowManager)mContext.getSystemService(Activity.WINDOW_SERVICE);
        }

        sWindowManager.addView(view, lp);

        Message msg = new Message();
        msg.what = CLEAR_SCREEN_FLAG;
        msg.obj = vInfo;
        mScreenOffHandler.sendMessageDelayed(msg, ALERT_TIME_LENGTH);
    }

    private WindowManager.LayoutParams initLayoutParams(int type) {
        Log.i(TAG, "initLayoutParams ++ new");
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.type = type;

        lp.flags = WindowManager.LayoutParams.FLAG_FULLSCREEN
            | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
            | WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON
            | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH
            //|WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
            //|WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
            | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;

//        mLayoutParams.width = WindowManager.LayoutParams.MATCH_PARENT;
        lp.width = getMin(sWindowManager.getDefaultDisplay().getWidth(), sWindowManager.getDefaultDisplay()
                .getHeight()) - 64;
        lp.height = WindowManager.LayoutParams.WRAP_CONTENT;
        lp.gravity = Gravity.CENTER | Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL;
        lp.alpha = 1;
        lp.setTitle("title: " + SystemClock.uptimeMillis());

         return lp;
    }

    private void clearViewFlag(View view) {
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.type = WindowManager.LayoutParams.TYPE_TOP_MOST;
        lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_FULLSCREEN))
            & (~(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED))
            & (~(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON))
            & (~(WindowManager.LayoutParams.FLAG_SPLIT_TOUCH))
            & (~(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));

        try {
            sWindowManager.updateViewLayout(view, lp);
        } catch (IllegalArgumentException e) {
            // TODO: handle exception
            e.printStackTrace();
        }

    }

    private void updateShowingView(boolean bTouchable) {
        Log.i(TAG, "updateShowingView ++, bTouchable = " + bTouchable);
        if (allowAnswerCallBeforeAlertDismmss() && sShowingView != null && !sShowingView.isEmpty()) {
            Iterator<ViewInfo> iterator = sShowingView.iterator();
            while (iterator.hasNext()) {
                CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
                WindowManager.LayoutParams lp = viewInfo.getLayoutParams();
                lp = updateViewLpToTouchable(lp, bTouchable);
                sWindowManager.updateViewLayout(viewInfo.getView(), lp);
            }
        }
    }

    private WindowManager.LayoutParams updateViewLpToTouchable(WindowManager.LayoutParams lp, boolean bTouchable) {
        Log.i(TAG, "updateViewLpToTouchable:: bTouchable = " + bTouchable);
        if (bTouchable) {
            lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_SPLIT_TOUCH));

            lp.flags = lp.flags | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
        } else {
            lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE))
            & (~(WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL));

        lp.flags = lp.flags | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH;
        }

        return lp;
    }

    private ViewInfo getTopShowedViewInfo(ArrayList<ViewInfo> viewInfos) {
        if (viewInfos == null || viewInfos.isEmpty()) {
            return null;
        }
        Iterator<ViewInfo> iterator = viewInfos.iterator();
        if (iterator.hasNext() && isAllPriorityTheSame()) {
            return iterator.next();
        } else {
            return getHighestPriorityShowingView();
        }
    }

    private void rmViewByRowId(long msgId) {
        if (sShowingView == null || sShowingView.isEmpty()) {
            Log.i(TAG, "rmViewContains::nothing to remove");
            return;
        }

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getMsgRowId() == msgId) {
                Log.i(TAG, "rmViewByRowId::remove msgid = " + msgId);
                sShowingView.remove(viewInfo);
                return;
            }
        }
        return;
    }

    private ViewInfo getHighestPriorityShowingView() {
        if (sShowingView == null || sShowingView.isEmpty()) {
            Log.i(TAG, "getHighestPriorityShowingView return null");
            return null;
        }

        ViewInfo ret = null;
        int priority = -1;

        //precondition: not all view's priority is the same
        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getPriority() > priority) {
                priority = viewInfo.getPriority();
                ret = viewInfo;
            }
        }

        return ret;
    }

    private boolean isAllPriorityTheSame() {
        int priority = 0;
        if (sShowingView == null || sShowingView.isEmpty()) {
            return true;
        }
        
        Iterator<ViewInfo> iterator = sShowingView.iterator();
        if (iterator.hasNext()) {
            priority = iterator.next().getPriority();
        }
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getPriority() != priority) {
                return false;
            }
        }

        return true;
    }

    public void dismissAll() {
        Log.i(TAG, "dismissAll ++, sShowingView.size() = " + sShowingView.size());
        mContext.stopService(new Intent(mContext, CellBroadcastAlertAudio.class));

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            sWindowManager.removeView(viewInfo.getView());
        }

        sShowingView.clear();
    }

    /** Returns the resource ID for either the full screen or dialog layout. */
    protected int getLayoutResId() {
//        return R.layout.cell_broadcast_alert_fullscreen;
        return R.layout.cell_broadcast_alert;
    }

    private void updateAlertText(View view, CellBroadcastMessage message, long msgId) {
        Log.i(TAG, "enter updateAlertText");

        int titleId = CellBroadcastResources.getDialogTitleResource(message);

        TextView titleTextView = (TextView) view.findViewById(R.id.alertTitle);
        if (Comparer.getUpdateNumOfCb(message) > 0) { 
            // >0 update message
            Log.i(TAG, "updateAlertText::this is update message");
            titleTextView.setText(mContext.getString(titleId) + " "
                    + mContext.getString(R.string.have_updated));
        } else {
            Log.i(TAG, "updateAlertIcon::this is normal message");
            titleTextView.setText(titleId);
        }

        TextView textViewMsgBody = (TextView) view.findViewById(R.id.message);
        String content;
        if(getShowMsgId()) {
            //only TMO need to show MsgId, and did not support show hyper link. for TMO, getSetTextViewAutoLink return false
            content = "ID: " + CMASUtils.convertMsgId2Str(message.getServiceCategory()) + "\r\n" 
                + CellBroadcastResources.getMessageDetails(mContext, message) + "\r\n"
                + message.getMessageBody();
        } else {
            content = CellBroadcastResources.getMessageDetails(mContext, message) + "\r\n"
            + message.getMessageBody();
        }

        //set typeface
        textViewMsgBody.setTypeface(Typeface.SANS_SERIF);

        //plugin set textview autolink
        Log.i(TAG, "before set autolink");
        if (getSetTextViewAutoLink() || message.getServiceCategory() == PRESIDENT_ALERT_ID) {
            setTextAutoLink(textViewMsgBody, content, view, message, msgId);
        } else {
            textViewMsgBody.setText(content);
        }

    }

    private void setTextAutoLink(TextView tv, String msgContent, final View view, final CellBroadcastMessage msg,
            final long msgId) {
        //plugin
        IAutoLinkClick autoLinkClick = new IAutoLinkClick() {

            public void onAutoLinkClicked() {
                // dismiss top dialog
                Log.i(TAG, "autolink is clicked, dismiss the dialog and mark read");
                dismissAndMarkRead(view, msg, msgId);

                //remove view
                rmViewByRowId(msgId);
            }
        };

        SpannableString text = new SpannableString(msgContent);
        CMASLinkify.addLinks(text, CMASLinkify.ALL, autoLinkClick);

        tv.setText(text);
        tv.setMovementMethod(LinkMovementMethod.getInstance());
    }

    private void updateAlertIcon(View view, CellBroadcastMessage message) {
        ImageView alertIcon = (ImageView)view.findViewById(R.id.icon);

        if (Comparer.getUpdateNumOfCb(message) > 0) { 
            // >0 update message
            Log.i(TAG, "updateAlertIcon::this is update message");
            alertIcon.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_dialog_update_alarm));
        } else {
            Log.i(TAG, "updateAlertIcon::this is normal message");
            alertIcon.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_dialog_alarm));
        }

    }

    //get boolean value whether we need to set TextView from plug-in
    private boolean getSetTextViewAutoLink() {

        ICmasMessageInitiation optSetAutoLink = (ICmasMessageInitiation)
        CellBroadcastPluginManager.getCellBroadcastPluginObject(
        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        if (optSetAutoLink != null) {
            return optSetAutoLink.initMessage();
        }

        return false;
    }

    private boolean getShowMsgId(){
        ICmasMessageInitiation initMsg = (ICmasMessageInitiation)
        CellBroadcastPluginManager.getCellBroadcastPluginObject(
        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        if (initMsg != null) {
            return initMsg.showMsgId();
        }

        return false;
    }

    private boolean allowAnswerCallBeforeAlertDismmss() {
        Log.i(TAG, "allowAnswerCallBeforeAlertDismmss ++ ");
        ICmasMessageInitiation optAllAnswerCallBeforeCMASDismiss = (ICmasMessageInitiation)
        CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        if (optAllAnswerCallBeforeCMASDismiss != null) {
            return optAllAnswerCallBeforeCMASDismiss.allowAnswerCallBeforeCMASdismiss();
        }

        return false;
    }

    /** Returns the currently displayed message. */
    CellBroadcastMessage getLatestMessage() {
        int index = mMessageList.size() - 1;
        if (index >= 0) {
            return mMessageList.get(index);
        } else {
            return null;
        }
    }

    /** Removes and returns the currently displayed message. */
    private CellBroadcastMessage removeLatestMessage() {
        int index = mMessageList.size() - 1;
        if (index >= 0) {
            return mMessageList.remove(index);
        } else {
            return null;
        }
    }

    private void dismissAndMarkRead(View view, CellBroadcastMessage msg, long msgRowId) {
        Log.i(TAG, "dismissAndMarkRead ++, msgRowId = " + msgRowId);
        mContext.stopService(new Intent(mContext, CellBroadcastAlertAudio.class));

        // Mark the alert as read.
        markRead(msg);
        Log.i(TAG, "dismissAndMarkRead::sShowingView.size() = " + sShowingView.size());
        /*if(isMsgShowing(msgRowId)) {
            Log.i(TAG, "dismissAndMarkRead:: before remove review");
            mWindowManager.removeView(view);
        }*/

        try {
            Log.i(TAG, "dismissAndMarkRead:: before remove review");
            sWindowManager.removeView(view);
        } catch (IllegalArgumentException e) {
            // TODO: handle exception
            Log.i(TAG, "dismissAndMarkRead:: removeView Exception");
        }

        return;
    }

    private boolean isMsgShowing(long msgRowId) {
        if (sShowingView == null || sShowingView.isEmpty()) {
            return false;
        }

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getMsgRowId() == msgRowId) {
                return true;
            }
        }

        return false;
    }

    private void markRead(final CellBroadcastMessage msg) {
        // Mark broadcast as read on a background thread.
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(mContext.getContentResolver())
                .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                    @Override
                    public boolean execute(CellBroadcastContentProvider provider) {
                        return provider.markBroadcastRead(
                                Telephony.CellBroadcasts.DELIVERY_TIME, msg.getDeliveryTime());
                    }
                });

        return;
    }

    private void clearNotification(boolean bClear) {
        Log.i(TAG, "enter clearNotification");
        if (bClear) {
            Log.i(TAG, "Dimissing notification");
            NotificationManager notificationManager = (NotificationManager) mContext
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.cancel(CMASPresentationService.NOTIFICATION_ID);
        }
    }

    private int getMin(int width, int height) {
        if (width < height) {
            return width;
        }

        return height;
    }

    private class ViewInfo {
        private long mMsgRowId;
        private View mView;
        private int mPriority;
        private WindowManager.LayoutParams mLp;

        public ViewInfo(long msgId, View view, int priority, WindowManager.LayoutParams lp) {
            this.mMsgRowId = msgId;
            this.mView = view;
            this.mPriority = priority;
            this.mLp = lp;
        }

        public long getMsgRowId() {
            return this.mMsgRowId;
        }

        public View getView() {
            return this.mView;
        }
        
        public int getPriority() {
            return this.mPriority;
        }

        public WindowManager.LayoutParams getLayoutParams() {
            return this.mLp;
        }

        public void setLayoutParams(WindowManager.LayoutParams lp) {
            this.mLp = lp;
        }
    }

    private class ExtView extends View {

        public ExtView(Context context) {
            super(context);
            // TODO Auto-generated constructor stub
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent keyEvent) {
            Log.i(TAG, "onkeydown, keycode =" + keyCode);
            super.onKeyDown(keyCode, keyEvent);
            
            return true;
        }
    }

    public interface IAutoLinkClick {
        void onAutoLinkClicked();//dismiss the dialog
    }
}
