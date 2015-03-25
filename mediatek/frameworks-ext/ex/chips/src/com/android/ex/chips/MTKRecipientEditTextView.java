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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.android.ex.chips;

import android.app.Dialog;
import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.text.Editable;
import android.text.InputType;
import android.text.Layout;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.QwertyKeyListener;
import android.text.style.ImageSpan;
import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Patterns;
import android.util.TypedValue;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
import android.view.DragEvent;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewParent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.Filterable;
import android.widget.ListAdapter;
import android.widget.ListPopupWindow;
import android.widget.ListView;
import android.widget.MultiAutoCompleteTextView;
import android.widget.ScrollView;
import android.widget.TextView;

import com.android.ex.chips.RecipientAlternatesAdapter.RecipientMatchCallback;
import com.android.ex.chips.recipientchip.DrawableRecipientChip;
import com.android.ex.chips.recipientchip.InvisibleRecipientChip;
import com.android.ex.chips.recipientchip.VisibleRecipientChip;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/// M:
import android.view.Gravity;
import android.telephony.PhoneNumberUtils;
import android.text.SpanWatcher;
import android.content.res.Configuration;
import android.view.ViewTreeObserver;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.text.BoringLayout;
import java.lang.Enum;
import android.os.Parcel;
import android.os.Parcelable;
import android.net.Uri;
import android.util.LogPrinter;
import android.os.SystemProperties;
import android.widget.FrameLayout;
import android.widget.PopupWindow;
import android.view.ViewGroup;

/**
 * RecipientEditTextView is an auto complete text view for use with applications
 * that use the new Chips UI for addressing a message to recipients.
 */
public class MTKRecipientEditTextView extends MultiAutoCompleteTextView implements
        OnItemClickListener, Callback, RecipientAlternatesAdapter.OnCheckedItemChangedListener,
        GestureDetector.OnGestureListener, OnDismissListener, OnClickListener,
        TextView.OnEditorActionListener {

    private static final char COMMIT_CHAR_COMMA = ',';

    private static final char NAME_WRAPPER_CHAR = '"';

    private static final char COMMIT_CHAR_SEMICOLON = ';';

    private static final char COMMIT_CHAR_CHINESE_COMMA = '\uFF0C';  /// M: Support chinese comma as seperator

    private static final char COMMIT_CHAR_CHINESE_SEMICOLON = '\uFF1B';  /// M: Support chinese semicolon as seperator

    private static final char COMMIT_CHAR_SPACE = ' ';

    private static final String SEPARATOR = String.valueOf(COMMIT_CHAR_COMMA)
            + String.valueOf(COMMIT_CHAR_SPACE);

    private static final String TAG = "RecipientEditTextView";

    private static final String MTKTAG = "MTKRecip"; /// M: Log tag for detecting race condition

    private static boolean DEBUG_THREADING_LOG = true; // M: Flag of whether print log for detecting race condition

    private static boolean DEBUG_LOG = true; // M: Flag of whether print normal debug log

    private static final String DEBUG_MTKRECIPIENTEDITTEXTVIEW_THREADING = "debug.MTKRecip.threading"; /// M: Property name of whether print the race condition log or not

    private static int DISMISS = "dismiss".hashCode();

    private static final long DISMISS_DELAY = 300;

    /// M: distance between chip
    private static final int CHIP_INTERVAL = 5 ;

    // TODO: get correct number/ algorithm from with UX.
    // Visible for testing.
    /*package*/ static final int CHIP_LIMIT = 2;

    private static final int MAX_CHIPS_PARSED = 100;
    
    private static final float DELTA_Y_THRESHOLD = 5; /// M: threshold of if scrolling occurs

    private static int sSelectedTextColor = -1;

    // Resources for displaying chips.
    private Drawable mChipBackground = null;
    
    private float mDownPosY = 0; /// M: record the down postion

    private Drawable mChipDelete = null;

    private Drawable mInvalidChipBackground;

    private Drawable mChipBackgroundPressed;

    private float mChipHeight;

    private float mChipFontSize;

    private float mLineSpacingExtra;

    private int mChipPadding;

    private Tokenizer mTokenizer;

    private Validator mValidator;

    private DrawableRecipientChip mSelectedChip;

    private int mAlternatesLayout;

    private Bitmap mDefaultContactPhoto;

    private ImageSpan mMoreChip;

    private TextView mMoreItem;

    // VisibleForTesting
    private final ArrayList<String> mPendingChips = new ArrayList<String>();

    private Handler mHandler;

    private int mPendingChipsCount = 0;

    private boolean mNoChips = false;

    /// M: use this flag to block doing query while creating/removing chip
    private enum PROCESSING_MODE {
        NONE, COMMIT, REMOVE, REMOVE_LAST, REPLACE, REPLACE_LAST
    }
    private PROCESSING_MODE mChipProcessingMode = PROCESSING_MODE.NONE;

    private ListPopupWindow mAlternatesPopup;

    private ListPopupWindow mAddressPopup;

    // VisibleForTesting
    private ArrayList<DrawableRecipientChip> mTemporaryRecipients;

    private ArrayList<DrawableRecipientChip> mRemovedSpans;

    private boolean mShouldShrink = true;

    // Chip copy fields.
    private GestureDetector mGestureDetector;

    private Dialog mCopyDialog;

    private String mCopyAddress;

    /**
     * Used with {@link #mAlternatesPopup}. Handles clicks to alternate addresses for a
     * selected chip.
     */
    private OnItemClickListener mAlternatesListener;

    private int mCheckedItem;

    private TextWatcher mTextWatcher;

    // Obtain the enclosing scroll view, if it exists, so that the view can be
    // scrolled to show the last line of chips content.
    private ScrollView mScrollView;

    private boolean mTriedGettingScrollView;

    private boolean mDragEnabled = false;

    private final Runnable mAddTextWatcher = new Runnable() {
        @Override
        public void run() {
            printDebugLog(TAG,"[mAddTextWatcher.run]");
            if (mTextWatcher == null) {
                mTextWatcher = new RecipientTextWatcher();
                addTextChangedListener(mTextWatcher);
            }
        }
    };

    private IndividualReplacementTask mIndividualReplacements;

    private Runnable mHandlePendingChips = new Runnable() {

        @Override
        public void run() {
            printDebugLog(TAG,"[mHandlePendingChips.run]");
            handlePendingChips();
        }

    };

    private Runnable mDelayedShrink = new Runnable() {

        @Override
        public void run() {
            printDebugLog(TAG,"[mDelayedShrink.run]");
            shrink();
        }

    };

    private int mMaxLines;

    private static int sExcessTopPadding = -1;

    private int mActionBarHeight;

    private boolean mAttachedToWindow;    

    /// M: block the rapidly incoming alternates drop down item clicking
    private boolean mHandlingAlternatesDropDown = false;

    public MTKRecipientEditTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        printDebugLog(TAG,"[MTKRecipientEditTextView] constructor");
        setChipDimensions(context, attrs);
        if (sSelectedTextColor == -1) {
            sSelectedTextColor = context.getResources().getColor(android.R.color.white);
        }
        mDefaultTextSize = getPaint().getTextSize();  // M: Save default size of text
        mAlternatesPopup = new ListPopupWindow(context);
        mAddressPopup = new ListPopupWindow(context);
        /// M: Get default vertical offset of AutoCompleteTextView which is used for adjusting position of popup window. @{
        TypedArray a = context.obtainStyledAttributes(
                attrs, com.android.internal.R.styleable.AutoCompleteTextView, com.android.internal.R.attr.autoCompleteTextViewStyle, 0);
        mDefaultVerticalOffset = (int)a.getDimension(com.android.internal.R.styleable.AutoCompleteTextView_dropDownVerticalOffset, 0.0f);
        /// @}
        mCopyDialog = new Dialog(context);
        mAlternatesListener = new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView,View view, int position,
                    long rowId) {
                /// M: block the rapidly incoming alternates drop down item clicking. @{
                if (mHandlingAlternatesDropDown) {
                    return;
                }
                mHandlingAlternatesDropDown = true;
                /// @}
                mAlternatesPopup.setOnItemClickListener(null);
                replaceChip(mSelectedChip, ((RecipientAlternatesAdapter) adapterView.getAdapter())
                        .getRecipientEntry(position));
                Message delayed = Message.obtain(mHandler, DISMISS);
                delayed.obj = mAlternatesPopup;
                mHandler.sendMessageDelayed(delayed, DISMISS_DELAY);
                clearComposingText();
            }
        };
        setInputType(getInputType() | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
        setOnItemClickListener(this);
        setCustomSelectionActionModeCallback(this);
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                if (msg.what == DISMISS) {
                    ((ListPopupWindow) msg.obj).dismiss();
                    return;
                }
                super.handleMessage(msg);
            }
        };
        mTextWatcher = new RecipientTextWatcher();
        addTextChangedListener(mTextWatcher);
        mGestureDetector = new GestureDetector(context, this);
        setOnEditorActionListener(this);
        /// M: For debuggin race condition problem. @{
        DEBUG_THREADING_LOG = SystemProperties.getBoolean(DEBUG_MTKRECIPIENTEDITTEXTVIEW_THREADING, true);
        /// @}
        /// M: For duplicate contacts replacement usage.
        mPedingReplaceChips = new ArrayList<DrawableRecipientChip>();
        mPedingReplaceEntries = new ArrayList<RecipientEntry>();
        /// M: Force scroll to bottom at the beginning
        setForceEnableBringPointIntoView(true);
    }

    @Override
    protected void onDetachedFromWindow() {
        mAttachedToWindow = false;
    }

    @Override
    protected void onAttachedToWindow() {
        mAttachedToWindow = true;
    }
    
    @Override
    public boolean onEditorAction(TextView view, int action, KeyEvent keyEvent) {
        printDebugLog(TAG,"[onEditorAction] " + keyEvent);
        if (action == EditorInfo.IME_ACTION_DONE) {
            if (commitDefault()) {
                return true;
            }
            if (mSelectedChip != null) {
                clearSelectedChip();
                return true;
            } else if (focusNext()) {
                return true;
            }
        }
        return false;
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        InputConnection connection = super.onCreateInputConnection(outAttrs);
        int imeActions = outAttrs.imeOptions&EditorInfo.IME_MASK_ACTION;
        if ((imeActions&EditorInfo.IME_ACTION_DONE) != 0) {
            // clear the existing action
            outAttrs.imeOptions ^= imeActions;
            // set the DONE action
            outAttrs.imeOptions |= EditorInfo.IME_ACTION_DONE;
        }
        if ((outAttrs.imeOptions&EditorInfo.IME_FLAG_NO_ENTER_ACTION) != 0) {
            outAttrs.imeOptions &= ~EditorInfo.IME_FLAG_NO_ENTER_ACTION;
        }
        outAttrs.actionId = EditorInfo.IME_ACTION_DONE;
        outAttrs.actionLabel = getContext().getString(R.string.done);
        return connection;
    }

    /*package*/ DrawableRecipientChip getLastChip() {
        DrawableRecipientChip last = null;
        DrawableRecipientChip[] chips = getSortedRecipients();
        if (chips != null && chips.length > 0) {
            last = chips[chips.length - 1];
        }
        return last;
    }

    @Override
    public void onSelectionChanged(int start, int end) {
        // When selection changes, see if it is inside the chips area.
        // If so, move the cursor back after the chips again.
        DrawableRecipientChip last = getLastChip();
        if (last != null && start <= getSpannable().getSpanEnd(last)) {
            // Grab the last chip and set the cursor to after it.
            setSelection(Math.min(getSpannable().getSpanEnd(last) + 1, getText().length()));
        }
        /// M: Set selection to the end of whole text when chips to be parsed is over MAX_CHIPS_PARSED now or before & RecipientEditTextView is just been expanded. @{
        if (mNoChips && mJustExpanded) {
            Editable text = getText();
            setSelection(text != null && text.length() > 0 ? text.length() : 0);
            mJustExpanded = false;
        }
        /// @}
        super.onSelectionChanged(start, end);
    }

    /**
     * M: User interface state that is stored by RecipientEditTextView for implementing View.onSaveInstanceState.
     * @hide
     */
    public static class RecipientSavedState extends BaseSavedState {
        boolean frozenWithFocus;

        RecipientSavedState(Parcelable superState) {
            super(superState);
        }

        @Override
        public void writeToParcel(Parcel out, int flags) {
            super.writeToParcel(out, flags);
            out.writeInt(frozenWithFocus ? 1 : 0);
        }

        @Override
        public String toString() {
            String str = "RecipientEditTextView.RecipientSavedState{"
                    + Integer.toHexString(System.identityHashCode(this))
                    + " frozenWithFocus=" + frozenWithFocus + ")";
            return str;
        }

        @SuppressWarnings("hiding")
        public static final Parcelable.Creator<RecipientSavedState> CREATOR
                = new Parcelable.Creator<RecipientSavedState>() {
            public RecipientSavedState createFromParcel(Parcel in) {
                return new RecipientSavedState(in);
            }

            public RecipientSavedState[] newArray(int size) {
                return new RecipientSavedState[size];
            }
        };

        private RecipientSavedState(Parcel in) {
            super(in);
            frozenWithFocus = (in.readInt() != 0);
        }
    }

    @Override
    public void onRestoreInstanceState(Parcelable state) {
        Log.d(TAG,"[onRestoreInstanceState]");
        /// M: Modify to remove text content and append all addresses back. 
        ///    Because original onRestoreInstance didn't recover mTemporaryRecipients/mRemovedSpans/.... @{
        RecipientSavedState ss = (RecipientSavedState)state;
        boolean hasFocus = ss.frozenWithFocus;

        if (!TextUtils.isEmpty(getText())) {
            super.onRestoreInstanceState(null);
        } else {
            super.onRestoreInstanceState(ss.getSuperState());
        }

        Log.d(TAG,"[onRestore] Text->" + getText());

        /// M: System help us restore RecipientChip, we don't need to restore it by ourselves. @{
        boolean doRestore = true;
        if (hasFocus) {
            DrawableRecipientChip lastChip = getLastChip();
            if (lastChip != null) {
                doRestore = false;
            }
        }
        /// @}

        if (!TextUtils.isEmpty(getText()) && doRestore) {
            Log.d(TAG,"[onRestore] Do restore process");
            if (mTextWatcher != null) {
                removeTextChangedListener(mTextWatcher);
            }
            /// M: Process text content. @{
            String text = getText().toString();
            int textLen = text.length();
            printThreadingDebugLog(MTKTAG, "[onRestoreInstanceState] delete");
            getText().delete(0, textLen);
            MTKRecipientList recipientList = new MTKRecipientList();
            int x=0;
            int tokenStart = 0;
            int tokenEnd = 0;
            while((tokenEnd = mTokenizer.findTokenEnd(text, tokenStart)) < text.length()) {
                String destination = text.substring(tokenStart, tokenEnd);
                tokenStart = tokenEnd + 2;
                recipientList.addRecipient(tokenizeName(destination), isPhoneNumber(destination) ? destination : tokenizeAddress(destination));
                x++;
            }

            appendList(recipientList);

            if (tokenStart < tokenEnd) {
                String lastToken = text.substring(tokenStart, tokenEnd);
                if (recipientList.getRecipientCount() != 0) {
                    mStringToBeRestore = lastToken; /// M: Restore the text later
                } else {
                    getText().append(lastToken); /// M: Restore the text now
                }
            }
            /// @}
            mHandler.post(mAddTextWatcher);
        }
        /// @}
    }

    @Override
    public Parcelable onSaveInstanceState() {
        Log.d(TAG,"[onSaveInstanceState]");
        // If the user changes orientation while they are editing, just roll back the selection.
        clearSelectedChip();
        /// M: Save the state whether RecipientEditTextView has focus now. @{
        Parcelable superState = super.onSaveInstanceState();
        RecipientSavedState ss = new RecipientSavedState(superState);
        if (isFocused()) {
            ss.frozenWithFocus = true;
        } else {
            ss.frozenWithFocus = false;
        }
        Log.d(TAG,"[onSave] Text ->" + getText());
        return ss;
        /// @}
    }

    /**
     * Convenience method: Append the specified text slice to the TextView's
     * display buffer, upgrading it to BufferType.EDITABLE if it was
     * not already editable. Commas are excluded as they are added automatically
     * by the view.
     */
    @Override
    public void append(CharSequence text, int start, int end) {
        /// M: Only do append instead of other procedures during batch processing of appending strings. @{
        if (mDuringAppendStrings) {
            Log.d(TAG, "[append] (mDuringAppendStrings) " + text);
            super.append(text, start, end);
            return;
        }
        Log.d(TAG, "[append] " + text);
        /// @}
        // We don't care about watching text changes while appending.
        if (mTextWatcher != null) {
            removeTextChangedListener(mTextWatcher);
        }
        /// M: Add to pendingStrings list to process in batch later
        mPendingStrings.add(text.toString());

        if (!TextUtils.isEmpty(text) && TextUtils.getTrimmedLength(text) > 0) {
            String displayString = text.toString();

            if (!displayString.trim().endsWith(String.valueOf(COMMIT_CHAR_COMMA))) {
                // We have no separator, so we should add it
                super.append(SEPARATOR, 0, SEPARATOR.length());
                displayString += SEPARATOR;
            }
            
            if (!TextUtils.isEmpty(displayString)
                    && TextUtils.getTrimmedLength(displayString) > 0) {
                mPendingChipsCount++;
                mPendingChips.add(displayString);
            }
        }
        // Put a message on the queue to make sure we ALWAYS handle pending
        // chips.
        if (mPendingChipsCount > 0) {
            postHandlePendingChips();
        }
        mHandler.post(mAddTextWatcher);
    }

    /// M:
    public void appendList(MTKRecipientList recipientList) {
        if ((recipientList == null) || (recipientList.getRecipientCount() <= 0)) {
            return;
        }
        
        int recipientCnt = recipientList.getRecipientCount();
        printDebugLog(TAG,"[appendList] Start, count: " + recipientCnt);
        String str = "";
        for (int x=0; x<recipientCnt; x++) {
            MTKRecipient recipient = recipientList.getRecipient(x);
            str += recipient.getFormatString();
        }

        // We don't care about watching text changes while appending.
        if (mTextWatcher != null) {
            removeTextChangedListener(mTextWatcher);
        }

        mDuringAppendStrings = true;
        append(str, 0, str.length());
        mDuringAppendStrings = false;

        if (Log.isLoggable(TAG, Log.DEBUG)) {
            for (int x=0; x<recipientCnt; x++) {
                MTKRecipient recipient = recipientList.getRecipient(x);
                Log.d(TAG,"[appendList] Recipient -> Name = " + recipient.getDisplayName() + " & Dest = " + recipient.getDestination());
            }
        }

        for (int x=0; x<recipientCnt; x++) {
            /// Original manipulation in append. @{
            MTKRecipient recipient = recipientList.getRecipient(x);
            String text = recipient.getFormatString();
            if (!TextUtils.isEmpty(text) && TextUtils.getTrimmedLength(text) > 0) {
                printDebugLog(TAG,"[appendList] adding pending chips, index: " + x + ", " + text);
                String displayString = text.toString();
                int separatorPos = displayString.lastIndexOf(COMMIT_CHAR_COMMA);
                // Verify that the separator pos is not within ""; if it is, look
                // past the closing quote. If there is no comma past ", this string
                // will resolve to an error chip.
                if (separatorPos > -1) {
                    String parseDisplayString = displayString.substring(separatorPos);
                    int endQuotedTextPos = parseDisplayString.indexOf(NAME_WRAPPER_CHAR);
                    if (endQuotedTextPos > separatorPos) {
                        separatorPos = parseDisplayString.lastIndexOf(COMMIT_CHAR_COMMA,
                                endQuotedTextPos);
                    }
                }
                if (!TextUtils.isEmpty(displayString)
                        && TextUtils.getTrimmedLength(displayString) > 0) {
                    mPendingChipsCount++;
                    mPendingChips.add(text.toString());
                }
            }
            /// @}
        }
        // Put a message on the queue to make sure we ALWAYS handle pending
        // chips.
        if (mPendingChipsCount > 0) {
            postHandlePendingChips();
        }
        mHandler.post(mAddTextWatcher);
        printDebugLog(TAG,"[appendList] End");
    }

    /// M: Scroll to bottom when the action popup is shown
    @Override
    public boolean performLongClick() {
        setDisableBringPointIntoView(false);
        return super.performLongClick();
    }    

    @Override
    public void onFocusChanged(boolean hasFocus, int direction, Rect previous) {
        printDebugLog(TAG,"[onFocusChanged] hasFocus: " + hasFocus);
        super.onFocusChanged(hasFocus, direction, previous);
        if (!hasFocus) {
            shrink();
        } else {
            expand();
        }
    }

    @Override
    public <T extends ListAdapter & Filterable> void setAdapter(T adapter) {
        super.setAdapter(adapter);
        ((BaseRecipientAdapter) adapter)
                .registerUpdateObserver(new BaseRecipientAdapter.EntriesUpdatedObserver() {
                    @Override
                    public void onChanged(List<RecipientEntry> entries) {
                        if (entries != null && entries.size() > 0) {
                            scrollBottomIntoView();
                        }
                    }
                });
    }

    private void scrollBottomIntoView() {
        if (mScrollView != null && mShouldShrink) {
            int[] location = new int[2];
            getLocationOnScreen(location);
            int height = getHeight();
            int currentPos = location[1] + height;
            // Desired position shows at least 1 line of chips below the action
            // bar.
            // We add excess padding to make sure this is always below other
            // content.
            int desiredPos = (int) mChipHeight + mActionBarHeight + getExcessTopPadding();
            if (currentPos > desiredPos) {
                mScrollView.scrollBy(0, currentPos - desiredPos);
            }
        }
    }

    private int getExcessTopPadding() {
        if (sExcessTopPadding == -1) {
            sExcessTopPadding = (int) (mChipHeight + mLineSpacingExtra);
        }
        return sExcessTopPadding;
    }

    @Override
    public void performValidation() {
        // Do nothing. Chips handles its own validation.
    }

    private void shrink() {
        printDebugLog(TAG,"[shrink]");
        if (mTokenizer == null) {
            return;
        }
        /// M: When chips to be parsed is over MAX_CHIPS_PARSED now or before, we just need to create moreChip (+XX) instead of executing other procedures. @{
        Editable editable = getText();
        if (mNoChips) {
            int tokenCount = countTokens(editable);
            if (tokenCount < MAX_CHIPS_PARSED) {
                mNoChips = false;
            } else {
                printDebugLog(TAG,"[shrink] mNoChips");
                createMoreChip();
                return;
            }
        }
        /// @}
        /// M: Return if text is empty, otherwise move cursor to the end of text @{
        if (editable.length() == 0) {
            printDebugLog(TAG,"[shrink] empty, return");
            return;
        } else {
            if (isPhoneQuery()) {
                setSelection(editable.length());
            }
        }
        /// @}
        long contactId = mSelectedChip != null ? mSelectedChip.getEntry().getContactId() : -1;
        if (mSelectedChip != null && contactId != RecipientEntry.INVALID_CONTACT
                && ((!isPhoneQuery() && contactId != RecipientEntry.GENERATED_CONTACT) || isPhoneQuery())) { /// M: When shrink, also clear selectedChip in phoneQuery.
            printDebugLog(TAG,"[shrink] selecting chip");
            clearSelectedChip();
        } else {
            if (getWidth() <= 0) {
                printDebugLog(TAG,"[shrink] getWidth() <= 0");
                // We don't have the width yet which means the view hasn't been drawn yet
                // and there is no reason to attempt to commit chips yet.
                // This focus lost must be the result of an orientation change
                // or an initial rendering.
                // Re-post the shrink for later.
                mHandler.removeCallbacks(mDelayedShrink);
                mHandler.post(mDelayedShrink);
                return;
            }
            // Reset any pending chips as they would have been handled
            // when the field lost focus.
            if (mPendingChipsCount > 0) {
                printDebugLog(TAG,"[shrink] mPendingChipsCount > 0");
                postHandlePendingChips();
            } else {
                printDebugLog(TAG,"[shrink] mPendingChipsCount = 0");
                /// M: To judge wheather text just have blank.
                boolean textIsAllBlank = textIsAllBlank(editable);

                int end = getSelectionEnd();
                int start = mTokenizer.findTokenStart(editable, end);
                
                DrawableRecipientChip[] chips = 
                            getSpannable().getSpans(start, end, DrawableRecipientChip.class);
                if ((chips == null || chips.length == 0)) {
                    Editable text = getText();
                    int whatEnd = mTokenizer.findTokenEnd(text, start);
                    // This token was already tokenized, so skip past the ending token.
                    if (whatEnd < text.length() && text.charAt(whatEnd) == ',') {
                        whatEnd = movePastTerminators(whatEnd);
                    }
                    // In the middle of chip; treat this as an edit
                    // and commit the whole token.
                    int selEnd = getSelectionEnd();
                    if (whatEnd != selEnd && !textIsAllBlank) { /// M: To judge wheather text just have blank,if not construct chips.
                        handleEdit(start, whatEnd);
                    } else {
                        commitChip(start, end, editable);
                    }
                }
            }
            mHandler.post(mAddTextWatcher);
        }
        createMoreChip();
    }
    
    /// M: To judge wheather text just have blank.
    private boolean textIsAllBlank(Editable e) {
        if (e != null) {
            for (int i = 0; i < e.length(); i++) {
                if (e.charAt(i) != ' ') {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    
    private void expand() {
        printDebugLog(TAG,"[expand] start, pending chip count: " + mPedingReplaceChips.size() + ", TemporaryRecipients count: "
                + (mTemporaryRecipients == null ? null : mTemporaryRecipients.size()));
        /// M: Alway force scroll to bottom when expanding
        setForceEnableBringPointIntoView(true);
        /// M: Phone has limited number of lines
        if (mShouldShrink && !isPhoneQuery()) {
            setMaxLines(Integer.MAX_VALUE);
        }
        /// M: Replace the first chip if needed. @{
        if(isPhoneQuery()) {
            DrawableRecipientChip[] recipients = getSortedRecipients();
            if (recipients != null && recipients.length > 0 && mHasEllipsizedFirstChip) {
                replaceChipOnSameTextRange(recipients[0], -1);
                mHasEllipsizedFirstChip = false;
            }
        }
        /// @}
        removeMoreChip();
        /// M: Replace duplicate contacts. @{
        if (isPhoneQuery() && mPedingReplaceChips.size() != 0 && mPedingReplaceEntries.size()!=0) {
            mDuringReplaceDupChips = true;
            int replaceCnt = mPedingReplaceChips.size();
            DrawableRecipientChip oldChip;
            RecipientEntry newEntry;
            for (int x = 0; x < replaceCnt; x++) {
                oldChip = mPedingReplaceChips.get(x);
                newEntry = mPedingReplaceEntries.get(x);
                replaceChip(oldChip, newEntry);
                printDebugLog(TAG, "[expand] Expand and replace contact from " + oldChip.getEntry().getContactId() + " to " + newEntry.getContactId());
            }
            mPedingReplaceChips.clear();
            mPedingReplaceEntries.clear();
            mDuringReplaceDupChips = false;
        }
        /// @}
        setCursorVisible(true);
        Editable text = getText();
        setSelection(text != null && text.length() > 0 ? text.length() : 0);
        // If there are any temporary chips, try replacing them now that the user
        // has expanded the field.
        if (mTemporaryRecipients != null && mTemporaryRecipients.size() > 0) {
            printDebugLog(TAG, "[expand] execute RecipientReplacementTask, mTemporaryRecipients.size: "
                    + mTemporaryRecipients.size());
            new RecipientReplacementTask().execute();
            clearTemporaryRecipients(); /// M: replaced with API with log
        } else {
            /// M: Restore if no further processing is required
            setForceEnableBringPointIntoView(false);
        }
        /// M: Scroll to bottom after expanded
        setDisableBringPointIntoView(false);
        /// M: For indicating RecipientEditTextView is just been expanded.
        if (mNoChips) {
            mJustExpanded = true;
        }
        /// @}
        printDebugLog(TAG, "[expand] end");
    }

    /// M: Add item to the mTemporaryRecipients
    private void addTemporaryRecipients(DrawableRecipientChip chip) {
        if (mTemporaryRecipients == null) {
            mTemporaryRecipients = new ArrayList<DrawableRecipientChip>();
        }
        mTemporaryRecipients.add(chip);
        printDebugLog(TAG,"[addItemToTemporaryRecipients] count: " + mTemporaryRecipients.size());
    }

    /// M: Clear the mTemporaryRecipients
    private void clearTemporaryRecipients() {
        printDebugLog(TAG,"[clearTemporaryRecipients]");
        mTemporaryRecipients = null;
    }

    private CharSequence ellipsizeText(CharSequence text, TextPaint paint, float maxWidth) {
        paint.setTextSize(mChipFontSize);
        if (maxWidth <= 0 && Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "Max width is negative: " + maxWidth);
        }
        return TextUtils.ellipsize(text, paint, maxWidth,
                TextUtils.TruncateAt.END);
    }

    private Bitmap createSelectedChip(RecipientEntry contact, TextPaint paint) {
        // Ellipsize the text so that it takes AT MOST the entire width of the
        // autocomplete text entry area. Make sure to leave space for padding
        // on the sides.
        int height = (int) mChipHeight;
        int deleteWidth = height;
        float[] widths = new float[1];
        paint.getTextWidths(" ", widths);
        CharSequence ellipsizedText = ellipsizeText(createChipDisplayText(contact), paint,
                calculateAvailableWidth() - deleteWidth - widths[0]);
        printDebugLog(TAG,"[createSelectedChip] " + ellipsizedText);

        // Make sure there is a minimum chip width so the user can ALWAYS
        // tap a chip without difficulty.
        int width = Math.max(deleteWidth * 2, (int) Math.floor(paint.measureText(ellipsizedText, 0,
                ellipsizedText.length()))
                + (mChipPadding * 2) + deleteWidth);

        // Create the background of the chip.
        Bitmap tmpBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(tmpBitmap);
        if (mChipBackgroundPressed != null) {
            mChipBackgroundPressed.setBounds(0, 0, width, height);
            mChipBackgroundPressed.draw(canvas);
            paint.setColor(sSelectedTextColor);
            // Vertically center the text in the chip.
            canvas.drawText(ellipsizedText, 0, ellipsizedText.length(), mChipPadding,
                    getTextYOffset((String) ellipsizedText, paint, height), paint);
            // Make the delete a square.
            Rect backgroundPadding = new Rect();
            mChipBackgroundPressed.getPadding(backgroundPadding);
            mChipDelete.setBounds(width - deleteWidth + backgroundPadding.left,
                    0 + backgroundPadding.top,
                    width - backgroundPadding.right,
                    height - backgroundPadding.bottom);
            mChipDelete.draw(canvas);
        } else {
            Log.w(TAG, "Unable to draw a background for the chips as it was never set");
        }
        return tmpBitmap;
    }

    private Bitmap createUnselectedChip(RecipientEntry contact, TextPaint paint,
            boolean leaveBlankIconSpacer) {
        // Ellipsize the text so that it takes AT MOST the entire width of the
        // autocomplete text entry area. Make sure to leave space for padding
        // on the sides.
        int height = (int) mChipHeight;
        int iconWidth = height;
        float[] widths = new float[1];
        paint.getTextWidths(" ", widths);
        /// M: Limit ellipsizedText in some case (ex. moreChip)
        CharSequence ellipsizedText = ellipsizeText(createChipDisplayText(contact), paint,
                (mLimitedWidthForSpan == -1) ? (calculateAvailableWidth() - iconWidth - widths[0]) : (mLimitedWidthForSpan - iconWidth - widths[0]));
        printDebugLog(TAG,"[createUnselectedChip] start, " + ellipsizedText + ", ID: " + contact.getContactId());
        // Make sure there is a minimum chip width so the user can ALWAYS
        // tap a chip without difficulty.
        
        /// M: Only leave space if icon exists. @{
        boolean hasIcon = false;
        int ellipsizedTextWidth = (int) Math.floor(paint.measureText(ellipsizedText, 0, ellipsizedText.length()));
        int width = ellipsizedTextWidth + (mChipPadding * 2);
        /// @}

        // Create the background of the chip.
        Bitmap tmpBitmap = null;
        Drawable background = getChipBackground(contact);
        if (background != null) {
            Canvas canvas = null; /// M: Only leave space if icon exists	
            Bitmap photo = null;
            Matrix matrix = null;

            // Don't draw photos for recipients that have been typed in OR generated on the fly.
            long contactId = contact.getContactId();
            boolean drawPhotos = isPhoneQuery() ?
                    contactId != RecipientEntry.INVALID_CONTACT
                    : (contactId != RecipientEntry.INVALID_CONTACT
                            && (contactId != RecipientEntry.GENERATED_CONTACT &&
                                    !TextUtils.isEmpty(contact.getDisplayName())));
            if (drawPhotos) {
                byte[] photoBytes = contact.getPhotoBytes();
                // There may not be a photo yet if anything but the first contact address
                // was selected.
                if (photoBytes == null && contact.getPhotoThumbnailUri() != null) {
                    // TODO: cache this in the recipient entry?
                    ((BaseRecipientAdapter) getAdapter()).fetchPhoto(contact, contact
                            .getPhotoThumbnailUri());
                    photoBytes = contact.getPhotoBytes();
                }

                 if (photoBytes != null) {
                    photo = BitmapFactory.decodeByteArray(photoBytes, 0, photoBytes.length);
                } else {
                    // TODO: can the scaled down default photo be cached?
                    photo = mDefaultContactPhoto;
                }
                // Draw the photo on the left side.
                if (photo != null) {
                    /// M: Only leave space if icon exists. @{
                    hasIcon = true;
                    width = ellipsizedTextWidth + (mChipPadding * 2) + iconWidth; 
                    /// @}
                    RectF src = new RectF(0, 0, photo.getWidth(), photo.getHeight());
                    Rect backgroundPadding = new Rect();
                    mChipBackground.getPadding(backgroundPadding);
                    RectF dst = new RectF(width - iconWidth + backgroundPadding.left,
                            0 + backgroundPadding.top,
                            width - backgroundPadding.right,
                            height - backgroundPadding.bottom);
                    matrix = new Matrix();
                    matrix.setRectToRect(src, dst, Matrix.ScaleToFit.FILL);
                }
            } else if (!leaveBlankIconSpacer || isPhoneQuery()) {
                iconWidth = 0;
            }

            tmpBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            canvas = new Canvas(tmpBitmap);

            background.setBounds(0, 0, width, height);
            background.draw(canvas);
            if (photo != null && matrix != null) {
                canvas.drawBitmap(photo, matrix, paint);
            }
            
            paint.setColor(getContext().getResources().getColor(android.R.color.black));
            // Vertically center the text in the chip.
            int xPositionOfText = hasIcon ? mChipPadding : (mChipPadding + (width - mChipPadding*2 - ellipsizedTextWidth)/2); /// M: Horizontally center the text in the chip
            canvas.drawText(ellipsizedText, 0, ellipsizedText.length(), xPositionOfText,
                    getTextYOffset((String)ellipsizedText, paint, height), paint);
        } else {
            Log.w(TAG, "Unable to draw a background for the chips as it was never set");
        }

        if (tmpBitmap == null) {
            tmpBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        }
        printDebugLog(TAG,"[createUnselectedChip] end");
        return tmpBitmap;
    }

    /// M: Calculate the width of an unselected chip @{
    private int calculateUnselectedChipWidth(RecipientEntry entry) {
        TextPaint paint = getPaint();
        int height = (int) mChipHeight;
        float[] widths = new float[1];
        paint.getTextWidths(" ", widths);
        long contactId = entry.getContactId();
        boolean drawIcon = isPhoneQuery() ?
                    contactId != RecipientEntry.INVALID_CONTACT
                    : (contactId != RecipientEntry.INVALID_CONTACT
                            && (contactId != RecipientEntry.GENERATED_CONTACT &&
                                    !TextUtils.isEmpty(entry.getDisplayName())));
        int iconWidth = drawIcon ? height : 0;
        float tempTextSize = paint.getTextSize();
        CharSequence ellipsizedText = ellipsizeText(createChipDisplayText(entry), paint,
                (mLimitedWidthForSpan == -1) ? (calculateAvailableWidth() - iconWidth - widths[0]) : 
                (mLimitedWidthForSpan - iconWidth - widths[0]));
        int ellipsizedTextWidth = (int) Math.floor(paint.measureText(ellipsizedText, 0, ellipsizedText.length()));
        int width = ellipsizedTextWidth + (mChipPadding * 2) + iconWidth;
        paint.setTextSize(tempTextSize);
        return width;
    }
    /// @}

    /**
     * Get the background drawable for a RecipientChip.
     */
    // Visible for testing.
    /* package */Drawable getChipBackground(RecipientEntry contact) {
        /// M: Also consider the judgement of validator
        return contact.isValid() && isValid(contact.getDestination()) ?
                mChipBackground : mInvalidChipBackground;
    }
    
    private static float getTextYOffset(String text, TextPaint paint, int height) {
        Rect bounds = new Rect();
        paint.getTextBounds(text, 0, text.length(), bounds);
        int textHeight = bounds.bottom - bounds.top - (int)paint.descent(); /// M: Vertically center the text in the chip (rollback to ICS2)
        return height - ((height - textHeight) / 2);
    }

    private DrawableRecipientChip constructChipSpan(RecipientEntry contact, boolean pressed,
            boolean leaveIconSpace) throws NullPointerException {
        printDebugLog(TAG,"[constructChipSpan] pressed: " + pressed);    
        if (mChipBackground == null) {
            throw new NullPointerException(
                    "Unable to render any chips as setChipDimensions was not called.");
        }

        TextPaint paint = getPaint();
        float defaultSize = paint.getTextSize();
        int defaultColor = paint.getColor();

        Bitmap tmpBitmap;
        if (pressed) {
            tmpBitmap = createSelectedChip(contact, paint);

        } else {
            tmpBitmap = createUnselectedChip(contact, paint, leaveIconSpace);
        }

        // Pass the full text, un-ellipsized, to the chip.
        Drawable result = new BitmapDrawable(getResources(), tmpBitmap);
        result.setBounds(0, 0, tmpBitmap.getWidth(), tmpBitmap.getHeight());
        DrawableRecipientChip recipientChip = new VisibleRecipientChip(result, contact);
        // Return text to the original size.
        paint.setTextSize(defaultSize);
        paint.setColor(defaultColor);
        return recipientChip;
    }

    /**
     * Calculate the bottom of the line the chip will be located on using:
     * 1) which line the chip appears on
     * 2) the height of a chip
     * 3) padding built into the edit text view
     */
    private int calculateOffsetFromBottom(int line) {
        // Line offsets start at zero.
        int actualLine = getLineCount() - (line + 1);
        return -((actualLine * ((int) mChipHeight) + getPaddingBottom()) + getPaddingTop())
                + getDropDownVerticalOffset();
    }

    /**
     * Get the max amount of space a chip can take up. The formula takes into
     * account the width of the EditTextView, any view padding, and padding
     * that will be added to the chip.
     */
    private float calculateAvailableWidth() {
        return getWidth() - getPaddingLeft() - getPaddingRight() - (mChipPadding * 2);
    }


    private void setChipDimensions(Context context, AttributeSet attrs) {
        TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.RecipientEditTextView, 0,
                0);
        Resources r = getContext().getResources();
        mChipBackground = a.getDrawable(R.styleable.RecipientEditTextView_chipBackground);
        if (mChipBackground == null) {
            mChipBackground = r.getDrawable(R.drawable.chip_background);
        }
        mChipBackgroundPressed = a
                .getDrawable(R.styleable.RecipientEditTextView_chipBackgroundPressed);
        if (mChipBackgroundPressed == null) {
            mChipBackgroundPressed = r.getDrawable(R.drawable.chip_background_selected);
        }
        mChipDelete = a.getDrawable(R.styleable.RecipientEditTextView_chipDelete);
        if (mChipDelete == null) {
            mChipDelete = r.getDrawable(R.drawable.chip_delete);
        }
        mChipPadding = a.getDimensionPixelSize(R.styleable.RecipientEditTextView_chipPadding, -1);
        if (mChipPadding == -1) {
            mChipPadding = (int) r.getDimension(R.dimen.chip_padding);
        }
        mAlternatesLayout = a.getResourceId(R.styleable.RecipientEditTextView_chipAlternatesLayout,
                -1);
        if (mAlternatesLayout == -1) {
            mAlternatesLayout = R.layout.chips_alternate_item;
        }

        mDefaultContactPhoto = BitmapFactory.decodeResource(r, R.drawable.ic_contact_picture);

        mMoreItem = (TextView) LayoutInflater.from(getContext()).inflate(R.layout.more_item, null);

        mChipHeight = a.getDimensionPixelSize(R.styleable.RecipientEditTextView_chipHeight, -1);
        if (mChipHeight == -1) {
            mChipHeight = r.getDimension(R.dimen.chip_height);
        }
        mChipFontSize = a.getDimensionPixelSize(R.styleable.RecipientEditTextView_chipFontSize, -1);
        if (mChipFontSize == -1) {
            mChipFontSize = r.getDimension(R.dimen.chip_text_size);
        }
        mInvalidChipBackground = a
                .getDrawable(R.styleable.RecipientEditTextView_invalidChipBackground);
        if (mInvalidChipBackground == null) {
            mInvalidChipBackground = r.getDrawable(R.drawable.chip_background_invalid);
        }
        mLineSpacingExtra =  context.getResources().getDimension(R.dimen.line_spacing_extra);
        mMaxLines = r.getInteger(R.integer.chips_max_lines);
        TypedValue tv = new TypedValue();
        if (context.getTheme().resolveAttribute(android.R.attr.actionBarSize, tv, true)) {
            mActionBarHeight = TypedValue.complexToDimensionPixelSize(tv.data, getResources()
                    .getDisplayMetrics());
        }
        a.recycle();
    }

    // Visible for testing.
    /* package */ void setMoreItem(TextView moreItem) {
        mMoreItem = moreItem;
    }


    // Visible for testing.
    /* package */ void setChipBackground(Drawable chipBackground) {
        mChipBackground = chipBackground;
    }

    // Visible for testing.
    /* package */ void setChipHeight(int height) {
        mChipHeight = height;
    }

    /**
     * Set whether to shrink the recipients field such that at most
     * one line of recipients chips are shown when the field loses
     * focus. By default, the number of displayed recipients will be
     * limited and a "more" chip will be shown when focus is lost.
     * @param shrink
     */
    public void setOnFocusListShrinkRecipients(boolean shrink) {
        mShouldShrink = shrink;
    }

    @Override
    public void onSizeChanged(int width, int height, int oldw, int oldh) {
        super.onSizeChanged(width, height, oldw, oldh);
        printDebugLog(TAG,"[onSizeChanged] w: " + width + ", h: " + height + ", oldw: " + oldw + ", oldh: " + oldh);
        if (width != 0 && height != 0) {
            if (mPendingChipsCount > 0) {
                postHandlePendingChips();
            } else {
                checkChipWidths();
            }
        }
        // Try to find the scroll view parent, if it exists.
        if (mScrollView == null && !mTriedGettingScrollView) {
            ViewParent parent = getParent();
            while (parent != null && !(parent instanceof ScrollView)) {
                parent = parent.getParent();
            }
            if (parent != null) {
                mScrollView = (ScrollView) parent;
            }
            mTriedGettingScrollView = true;
        }
    }

    private void postHandlePendingChips() {
        printDebugLog(TAG,"[postHandlePendingChips] count: " + mPendingChipsCount);
        mHandler.removeCallbacks(mHandlePendingChips);
        mHandler.post(mHandlePendingChips);
    }

    private void checkChipWidths() {
        // Check the widths of the associated chips.
        DrawableRecipientChip[] chips = getSortedRecipients();
        if (chips != null) {
            Rect bounds;
            for (DrawableRecipientChip chip : chips) {
                bounds = chip.getBounds();
                if (getWidth() > 0 && bounds.right - bounds.left > (getWidth() - getPaddingLeft() - getPaddingRight())) { /// M: Modified to take padding into consideration
                    // Need to redraw that chip.
                    replaceChip(chip, chip.getEntry());
                }
            }
        }
    }

    // Visible for testing.
    /*package*/ void handlePendingChips() {
        printDebugLog(TAG,"[handlePendingChips] Start, pending chips count: " + mPendingChipsCount);
        /// M: Append strings in batch processing
        appendPendingStrings();
        /// M: Force scroll to bottom
        setForceEnableBringPointIntoView(true);

        if (getViewWidth() <= 0) {
            printDebugLog(TAG,"[handlePendingChips] getViewWidth() <= 0, return");
            // The widget has not been sized yet.
            // This will be called as a result of onSizeChanged
            // at a later point.
            return;
        }
        if (mPendingChipsCount <= 0) {
            return;
        }

        synchronized (mPendingChips) {
            Editable editable = getText();
            int prevTokenEnd = 0; /// M: Record previous tokenEnd as starting of next substring
            // Tokenize!
            if (mPendingChipsCount <= MAX_CHIPS_PARSED) {
                /// M: We don't care about watching text and span changes while in the middle of handling pending chips. @{
                watcherProcessor wp = null;
                wp = new watcherProcessor();
                wp.initWatcherProcessor();
                wp.removeSpanWatchers();
                /// @}
                for (int i = 0; i < mPendingChips.size(); i++) {
                    /// M: Add text and span watchers back before handling last chip. @{
                    if (i == mPendingChips.size() - 1) {
                        printDebugLog(TAG,"[handlePendingChips] handling last pending chip"); 
                        if(wp != null) {
                            wp.addSpanWatchers();
                        }
                        /// M: Let text and span watchers work corectly by reset selection and layout. @{
                        setSelection(getText().length());
                        requestLayout();
                        /// @}
                    }
                    /// @}                    
                    String current = mPendingChips.get(i);
                    int tokenStart = editable.toString().indexOf(current, prevTokenEnd); /// M: Get substring from previous tokenEnd to end of current string
                    int tokenEnd = tokenStart + current.length() - 1;
                    printDebugLog(TAG,"[handlePendingChips] index: "+i+", "+current+", tokenStart:"+tokenStart+", tokenEnd:"+tokenEnd); 
                    if (tokenStart >= 0) {
                        // When we have a valid token, include it with the token
                        // to the left.
                        if (tokenEnd < editable.length() - 2
                                && editable.charAt(tokenEnd) == COMMIT_CHAR_COMMA) {
                            tokenEnd++;
                        }
                        createReplacementChip(tokenStart, tokenEnd, editable, i < CHIP_LIMIT || !mShouldShrink);
                    }
                    /// M: Get previous tokenEnd as starting of next substring. @{
                    DrawableRecipientChip[] chips = getSpannable().getSpans(tokenStart, editable.length(), DrawableRecipientChip.class);
                    if ((chips != null && chips.length > 0)) {
                        boolean prevTokenEndSet = false;
                        for(int x = 0; x < chips.length; x++) {
                            if (getChipStart(chips[x]) == tokenStart) {
                                prevTokenEnd = getChipEnd(chips[x]);
                                prevTokenEndSet = true;
                                break;
                            }
                        }
                        if (!prevTokenEndSet) {
                            prevTokenEnd = 0;
                        }
                    } else {
                        prevTokenEnd = 0;
                    }
                    /// @}
                    mPendingChipsCount--;
                }
                /// M: skip space
                sanitizeBetween() ;
                sanitizeEnd() ;
                /// M: Let text and span watchers work corectly by reset selection and layout. @{
                recoverLayout();
                /// @}
            } else {
                mNoChips = true;
            }

            /// M: Restore part of text after handling chips in case it be sanitized. @{
            if (mStringToBeRestore != null) {
                Log.d(TAG,"[handlePendingChips] Restore text ->" + mStringToBeRestore);
                getText().append(mStringToBeRestore);
                mStringToBeRestore = null;
            }
            /// @}

            printDebugLog(TAG, "[handlePendingChips] phase 1 completed, mTemporaryRecipients.size: " + 
                    (mTemporaryRecipients == null ? 0 : mTemporaryRecipients.size()));
            if (mTemporaryRecipients != null && mTemporaryRecipients.size() > 0
                    && mTemporaryRecipients.size() <= RecipientAlternatesAdapter.MAX_LOOKUPS) {
                if (hasFocus() || mTemporaryRecipients.size() < CHIP_LIMIT) {
                    printDebugLog(TAG,"[handlePendingChips] execute RecipientReplacementTask, count: " + mTemporaryRecipients.size());
                    new RecipientReplacementTask().execute();
                    clearTemporaryRecipients(); /// M: replaced with API with log
                } else {
                    // Create the "more" chip
                    /// M: Calculate how many chips can be accommodated. @{
                    int numChipsCanShow = 0;
                    if (isPhoneQuery()) {
                        numChipsCanShow = calculateNumChipsCanShow();
                    } else {
                        numChipsCanShow = CHIP_LIMIT;
                    }
                    /// @}
                    /// M: 
                    DrawableRecipientChip[] chips = getSortedRecipients();
                    if (chips != null && chips.length <= numChipsCanShow) {
                        printDebugLog(TAG,"[handlePendingChips] execute RecipientReplacementTask, count: " + mTemporaryRecipients.size());
                        new RecipientReplacementTask().execute();
                        clearTemporaryRecipients(); /// M: replaced with API with log
                    } else {
                        printDebugLog(TAG,"[handlePendingChips] execute IndividualReplacementTask, count: " + mTemporaryRecipients.size()
                                + ", canShow: " + numChipsCanShow);
                        mIndividualReplacements = new IndividualReplacementTask();
                        mIndividualReplacements.execute(new ArrayList<DrawableRecipientChip>(
                                mTemporaryRecipients.subList(0, numChipsCanShow)));
                        if (mTemporaryRecipients.size() > numChipsCanShow) {
                            mTemporaryRecipients = new ArrayList<DrawableRecipientChip>(
                                    mTemporaryRecipients.subList(numChipsCanShow,
                                            mTemporaryRecipients.size()));
                            printDebugLog(TAG,"[handlePendingChips] update mTemporaryRecipients count: " + mTemporaryRecipients.size()
                                + ", canShow: " + numChipsCanShow);
                        } else {
                            clearTemporaryRecipients(); /// M: replaced with API with log
                        }
                        createMoreChip();
                    }                    
                }
            } else {
                printDebugLog(TAG,"[handlePendingChips] fall back to show addresses. count: " 
                        + (mTemporaryRecipients == null ? 0 : mTemporaryRecipients.size()));
                // There are too many recipients to look up, so just fall back
                // to showing addresses for all of them.
                clearTemporaryRecipients(); /// M: replaced with API with log
                /// M: Only create moreChip (+XX) when RecipientEditTextView lost focus. @{
                if (!hasFocus() && mMoreChip != null) {
                    createMoreChip();
                }
                /// @}
                /// M: Restore force scrolling if no more processing
                setForceEnableBringPointIntoView(false);
            }
            mPendingChipsCount = 0;
            mPendingChips.clear();
        }
        printDebugLog(TAG,"[handlePendingChips] End"); 
    }

    // Visible for testing.
    /*package*/ int getViewWidth() {
        return getWidth();
    }

    /**
     * Remove any characters after the last valid chip.
     */
    // Visible for testing.
    /*package*/ void sanitizeEnd() {
        // Don't sanitize while we are waiting for pending chips to complete.
        if (mPendingChipsCount > 0) {
            return;
        }
        // Find the last chip; eliminate any commit characters after it.
        DrawableRecipientChip[] chips = getSortedRecipients();
        Spannable spannable = getSpannable();
        if (chips != null && chips.length > 0) {
            int end;
            mMoreChip = getMoreChip();
            if (mMoreChip != null) {
                end = spannable.getSpanEnd(mMoreChip);
            } else {
                end = getSpannable().getSpanEnd(getLastChip());
            }
            Editable editable = getText();
            int length = editable.length();
            if (length > end) {
                // See what characters occur after that and eliminate them.
                if (Log.isLoggable(TAG, Log.DEBUG)) {
                    Log.d(TAG, "There were extra characters after the last tokenizable entry."
                            + editable);
                }
                printThreadingDebugLog(MTKTAG, "[sanitizeEnd] delete");
                editable.delete(end + 1, length);
            }
        }
    }

    /**
     * Create a chip that represents just the email address of a recipient. At some later
     * point, this chip will be attached to a real contact entry, if one exists.
     */
     // VisibleForTesting
    private void createReplacementChip(int tokenStart, int tokenEnd, Editable editable, 
                boolean visible) {
        printDebugLog(TAG,"[createReplacementChip] tokenStart:"+tokenStart+", tokenEnd:"+tokenEnd+", visible:"+visible);
        if (alreadyHasChip(tokenStart, tokenEnd)) {
            // There is already a chip present at this location.
            // Don't recreate it.
            return;
        }
        String token = editable.toString().substring(tokenStart, tokenEnd);
        final String trimmedToken = token.trim();
        int commitCharIndex = trimmedToken.lastIndexOf(COMMIT_CHAR_COMMA);
        if (commitCharIndex != -1 && commitCharIndex == trimmedToken.length() - 1) {
            token = trimmedToken.substring(0, trimmedToken.length() - 1);
        }
        RecipientEntry entry = createTokenizedEntry(token);
        if (entry != null) {
            DrawableRecipientChip chip = null;
            /// M: Set the flag to COMMIT mode
            setChipProcessingMode(PROCESSING_MODE.COMMIT);
            try {
                if (!mNoChips) {
                    /* leave space for the contact icon if this is not just an email address */
                    boolean leaveSpace = TextUtils.isEmpty(entry.getDisplayName())
                            || TextUtils.equals(entry.getDisplayName(),
                                    entry.getDestination());
                    chip = visible ?
                            constructChipSpan(entry, false, leaveSpace)
                            : new InvisibleRecipientChip(entry);
                }
            } catch (NullPointerException e) {
                Log.e(TAG, e.getMessage(), e);
            }
            printThreadingDebugLog(MTKTAG, "[createReplacementChip] replace");
            editable.setSpan(chip, tokenStart, tokenEnd, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            // Add this chip to the list of entries "to replace"
            if (chip != null) {
                /// M: let original text contain "," ,to be the same as it in JB2.  @{
                chip.setOriginalText(trimmedToken);
                /// @}
                addTemporaryRecipients(chip); /// M: replaced with API with log
            }
            setChipProcessingMode(PROCESSING_MODE.NONE);
        }
    }

    private static boolean isPhoneNumber(String number) {
        // TODO: replace this function with libphonenumber's isPossibleNumber (see
        // PhoneNumberUtil). One complication is that it requires the sender's region which
        // comes from the CurrentCountryIso. For now, let's just do this simple match.
        if (TextUtils.isEmpty(number)) {
            return false;
        }

        /// M: MTK Version for ALPS00934864
        Matcher match = Patterns.PHONE_PATTERN_MTK.matcher(number);
        return match.matches();
    }

    /**
     * VisibleForTesting
     * M:Remove keyword final in parameter for MTK Version of ALPS00934864
     */
    private RecipientEntry createTokenizedEntry(String token) {
        printDebugLog(TAG,"[createTokenizedEntry] token:"+token); 
        if (TextUtils.isEmpty(token)) {
            return null;
        }
        if (isPhoneQuery() && isPhoneNumber(token)) {
            /// M: MTK Version for ALPS00934864
            token = token.trim();
            if(token.endsWith(","))
               token = token.substring(0, token.length()-1);
            return RecipientEntry
                    .constructFakeEntry(token);
        }
        Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(token);
        String display = null;
        boolean isValid = isValid(token);
        if (isValid && tokens != null && tokens.length > 0) {
            // If we can get a name from tokenizing, then generate an entry from
            // this.
            display = tokens[0].getName();
            if (!TextUtils.isEmpty(display)) {
                printDebugLog(TAG,"[createTokenizedEntry] RecipientEntry.constructGeneratedEntry()"); 
                return RecipientEntry.constructGeneratedEntry(display, tokens[0].getAddress(), isValid);
            } else {
                display = tokens[0].getAddress();
                if (!TextUtils.isEmpty(display)) {
                    printDebugLog(TAG,"[createTokenizedEntry] RecipientEntry.constructFakeEntry()"); 
                    return RecipientEntry.constructFakeEntry(display, isValid);
                }
            }
        }
        // Unable to validate the token or to create a valid token from it.
        // Just create a chip the user can edit.
        String validatedToken = null;
        if (mValidator != null && !isValid) {
            // Try fixing up the entry using the validator.
            validatedToken = mValidator.fixText(token).toString();
            if (!TextUtils.isEmpty(validatedToken)) {
                if (validatedToken.contains(token)) {
                    // protect against the case of a validator with a null domain,
                    // which doesn't add a domain to the token
                    Rfc822Token[] tokenized = Rfc822Tokenizer.tokenize(validatedToken);
                    if (tokenized.length > 0) {
                        validatedToken = tokenized[0].getAddress();
                        isValid = true;
                    }
                } else {
                    // We ran into a case where the token was invalid and removed
                    // by the validator. In this case, just use the original token
                    // and let the user sort out the error chip.
                    validatedToken = null;
                    isValid = false;
                }
            }
        }
        // Otherwise, fallback to just creating an editable email address chip.
        printDebugLog(TAG,"[createTokenizedEntry] RecipientEntry.constructFakeEntry()"); 
        return RecipientEntry.constructFakeEntry(
                    !TextUtils.isEmpty(validatedToken) ? validatedToken : token, isValid);
    }

    private boolean isValid(String text) {
        return mValidator == null ? true : mValidator.isValid(text);
    }

    /**
     * M: Get name after parsing destination by Rfc822Tokenizer. If it's null, return whole destination instead.
     */
    private String tokenizeName(String destination) {
        Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(destination);
        if (tokens != null && tokens.length > 0) {
            return tokens[0].getName();
        }
        return destination;
    }

    private static String tokenizeAddress(String destination) {
        Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(destination);
        if (tokens != null && tokens.length > 0) {
            return tokens[0].getAddress();
        }
        return destination;
    }

    @Override
    public void setTokenizer(Tokenizer tokenizer) {
        mTokenizer = tokenizer;
        super.setTokenizer(mTokenizer);
    }

    @Override
    public void setValidator(Validator validator) {
        mValidator = validator;
        super.setValidator(validator);
    }

    /**
     * We cannot use the default mechanism for replaceText. Instead,
     * we override onItemClickListener so we can get all the associated
     * contact information including display text, address, and id.
     */
    @Override
    protected void replaceText(CharSequence text) {
        return;
    }

    /**
     * Dismiss any selected chips when the back key is pressed.
     */
    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && mSelectedChip != null) {
            clearSelectedChip();
            return true;
        }
        return super.onKeyPreIme(keyCode, event);
    }

    /**
     * Monitor key presses in this view to see if the user types
     * any commit keys, which consist of ENTER, TAB, or DPAD_CENTER.
     * If the user has entered text that has contact matches and types
     * a commit key, create a chip from the topmost matching contact.
     * If the user has entered text that has no contact matches and types
     * a commit key, then create a chip from the text they have entered.
     */
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        printDebugLog(TAG,"[onKeyUp] " + event);
        switch (keyCode) {
            case KeyEvent.KEYCODE_TAB:
                if (event.hasNoModifiers()) {
                    if (mSelectedChip != null) {
                        clearSelectedChip();
                    } else {
                        commitDefault();
                    }
                }
                break;
        }
        return super.onKeyUp(keyCode, event);
    }

    private boolean focusNext() {
        View next = focusSearch(View.FOCUS_DOWN);
        if (next != null) {
            next.requestFocus();
            return true;
        }
        return false;
    }

    /**
     * Create a chip from the default selection. If the popup is showing, the
     * default is the first item in the popup suggestions list. Otherwise, it is
     * whatever the user had typed in. End represents where the the tokenizer
     * should search for a token to turn into a chip.
     * @return If a chip was created from a real contact.
     */
    private boolean commitDefault() {
        // If there is no tokenizer, don't try to commit.
        if (mTokenizer == null) {
            return false;
        }
        setDisableBringPointIntoView(true);  /// M: Don't scroll view when commitDefault chip
        Editable editable = getText();
        int end = getSelectionEnd();
        int start = mTokenizer.findTokenStart(editable, end);

        boolean shouldCreate = shouldCreateChip(start, end);
        printDebugLog(TAG,"[commitDefault] start: " + start + ", end: " + end + ", shouldCreateChip: " + shouldCreate); 

        if (shouldCreate) {
            int whatEnd = mTokenizer.findTokenEnd(getText(), start);
            // In the middle of chip; treat this as an edit
            // and commit the whole token.
            whatEnd = movePastTerminators(whatEnd);
            if (whatEnd != getSelectionEnd()) {
                handleEdit(start, whatEnd);
                return true;
            }
            return commitChip(start, end , editable);
        }
        return false;
    }

    private void commitByCharacter() {
        // We can't possibly commit by character if we can't tokenize.
        if (mTokenizer == null) {
            return;
        }
        Editable editable = getText();
        int end = getSelectionEnd();
        int start = mTokenizer.findTokenStart(editable, end);

        boolean shouldCreate = shouldCreateChip(start, end);
        printDebugLog(TAG,"[commitByCharacter] start: " + start + ", end: " + end + ", shouldCreateChip: " + shouldCreate); 
        
        if (end - start > 1 && shouldCreate) {
            commitChip(start, end, editable);
        }
        setSelection(getText().length());
    }

    private boolean commitChip(int start, int end, Editable editable) {
        ListAdapter adapter = getAdapter();
        if (adapter != null && adapter.getCount() > 0 && enoughToFilter()
                && end == getSelectionEnd() && !isPhoneQuery()) {
            printDebugLog(TAG,"[commitChip] submit 1st item, start: " + start + ", end: " + end);         
            // choose the first entry.
            submitItemAtPosition(0);
            dismissDropDown();
            return true;
        } else {
            int tokenEnd = mTokenizer.findTokenEnd(editable, start);
            if (editable.length() > tokenEnd + 1) {
                char charAt = editable.charAt(tokenEnd + 1);
                if (charAt == COMMIT_CHAR_COMMA || charAt == COMMIT_CHAR_SEMICOLON || charAt == COMMIT_CHAR_CHINESE_COMMA || charAt == COMMIT_CHAR_CHINESE_SEMICOLON) {
                    tokenEnd++;
                }
            }
            String text = editable.toString().substring(start, tokenEnd).trim();
            printDebugLog(TAG,"[commitChip] trying to match item. text: " + text + ", start: " + start + ", end: " + end);     
            /// M: Auto choose first match contact. @{
            if (isPhoneQuery() && adapter != null && adapter.getCount() > 0 && enoughToFilter() && end == getSelectionEnd()){
                int adapterCount = getAdapter().getCount();
                /// M: Check if there's fully match displayName or destination.
                for (int itemCnt = 0; itemCnt < adapterCount; itemCnt++){
                    RecipientEntry entry = (RecipientEntry)getAdapter().getItem(itemCnt);
                    String displayName = entry.getDisplayName().toLowerCase();
                    String destination = entry.getDestination(); 
                    if (text.equals(destination) || text.toLowerCase().equals(displayName)){
                        printDebugLog(TAG,"[commitChip] submit item: " + itemCnt);     
                        submitItemAtPosition(itemCnt);
                        dismissDropDown();
                        return true;
                    }
                }
                /// M: Check if there's match normanlized destination.
                for (int itemCnt = 0; itemCnt < adapterCount; itemCnt++){
                    RecipientEntry entry = (RecipientEntry)getAdapter().getItem(itemCnt);
                    String displayName = entry.getDisplayName().toLowerCase();
                    String destination = entry.getDestination();
                    if (entry.getDestinationKind() == RecipientEntry.ENTRY_KIND_PHONE) {
                        String currentNumber = PhoneNumberUtils.normalizeNumber(text);
                        String queryNumber = PhoneNumberUtils.normalizeNumber(destination);
                        if (PhoneNumberUtils.compare(currentNumber, queryNumber)) {
                            printDebugLog(TAG,"[commitChip] match normalized destination. submit item: " + itemCnt);     
                            submitItemAtPosition(itemCnt);
                            dismissDropDown();
                            return true;
                        }
                     }
                }
            }
            /// @}
            clearComposingText();
            if (text != null && text.length() > 0 && !text.equals(" ")) {
                RecipientEntry entry = createTokenizedEntry(text);
                if (entry != null) {
                    /// M: set the flag to block doing query
                    setChipProcessingMode(PROCESSING_MODE.COMMIT);
                    QwertyKeyListener.markAsReplaced(editable, start, end, "");
                    CharSequence chipText = createChip(entry, false);
                    if (chipText != null && start > -1 && end > -1) {
                        printThreadingDebugLog(MTKTAG, "[commitChip] replace");
                        /// M: Try to remove duplicated ", " if there's one
                        if (end+1 < editable.length() && TextUtils.equals(editable.toString().substring(end, end+2), ", ")) {
                            end += 2;
                        }
                        editable.replace(start, end, chipText);
                    }
                    dismissDropDown();
                    setChipProcessingMode(PROCESSING_MODE.NONE);
                }
                // Only dismiss the dropdown if it is related to the text we
                // just committed.
                // For paste, it may not be as there are possibly multiple
                // tokens being added.
                if (end == getSelectionEnd()) {
                    dismissDropDown();
                }
                sanitizeBetween();
                return true;
            }
        }
        printDebugLog(TAG,"[commitChip] do nothing");     
        return false;
    }

    // Visible for testing.
    /* package */ void sanitizeBetween() {
        // Don't sanitize while we are waiting for content to chipify.
        if (mPendingChipsCount > 0) {
            return;
        }
        // Find the last chip.
        DrawableRecipientChip[] recips = getSortedRecipients();
        if (recips != null && recips.length > 0) {
            DrawableRecipientChip last = recips[recips.length - 1];
            DrawableRecipientChip beforeLast = null;
            if (recips.length > 1) {
                beforeLast = recips[recips.length - 2];
            }
            int startLooking = 0;
            int end = getSpannable().getSpanStart(last);
            if (beforeLast != null) {
                startLooking = getSpannable().getSpanEnd(beforeLast);
                Editable text = getText();
                if (startLooking == -1 || startLooking > text.length() - 1) {
                    // There is nothing after this chip.
                    return;
                }
                if (text.charAt(startLooking) == ' ') {
                    startLooking++;
                }
            }
            if (startLooking >= 0 && end >= 0 && startLooking < end) {
                printThreadingDebugLog(MTKTAG,"[sanitizeBetween] delete");
                printDebugLog(TAG,"[sanitizeBetween] delete, start: " + startLooking + ", end: " + end); 
                getText().delete(startLooking, end);
            }
        }
    }

    private boolean shouldCreateChip(int start, int end) {
        return !mNoChips && hasFocus() && enoughToFilter() && !alreadyHasChip(start, end);
    }

    private boolean alreadyHasChip(int start, int end) {
        if (mNoChips) {
            return true;
        }
        DrawableRecipientChip[] chips = 
                    getSpannable().getSpans(start, end, DrawableRecipientChip.class);
        if ((chips == null || chips.length == 0)) {
            return false;
        }
        return true;
    }

    private void handleEdit(int start, int end) {
        printDebugLog(TAG,"[handleEdit] start: " + start + ", end: " + end); 
        if (start == -1 || end == -1) {
            // This chip no longer exists in the field.
            dismissDropDown();
            return;
        }
        // This is in the middle of a chip, so select out the whole chip
        // and commit it.
        Editable editable = getText();
        setSelection(end);
        String text = getText().toString().substring(start, end);
        if (!TextUtils.isEmpty(text)) {
            RecipientEntry entry = RecipientEntry.constructFakeEntry(text, isValid(text));
            QwertyKeyListener.markAsReplaced(editable, start, end, "");
            CharSequence chipText = createChip(entry, false);
            int selEnd = getSelectionEnd();
            /// M: Try to remove duplicated ", " if there's one
            if (end+1 < editable.length() && TextUtils.equals(editable.toString().substring(end, end+2), ", ")) {
                end += 2;
            }
            if (chipText != null && start > -1 && selEnd > -1) {
                printThreadingDebugLog(MTKTAG,"[handleEdit] replace");
                editable.replace(start, selEnd, chipText);
            }
        }
        dismissDropDown();
    }

    /**
     * If there is a selected chip, delegate the key events
     * to the selected chip.
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_ENTER && event.hasNoModifiers()) {
            return true;
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_ENTER:
            case KeyEvent.KEYCODE_DPAD_CENTER:
                if (event.hasNoModifiers()) {
                    if (commitDefault()) {
                        return true;
                    }
                    if (mSelectedChip != null) {
                        clearSelectedChip();
                        return true;
                    } else if (focusNext()) {
                        return true;
                    }
                }
                break;
        }    

        return super.onKeyDown(keyCode, event);
    }

    // Visible for testing.
    /* package */ Spannable getSpannable() {
        return getText();
    }

    private int getChipStart(DrawableRecipientChip chip) {
        return getSpannable().getSpanStart(chip);
    }

    private int getChipEnd(DrawableRecipientChip chip) {
        return getSpannable().getSpanEnd(chip);
    }

    /**
     * Instead of filtering on the entire contents of the edit box,
     * this subclass method filters on the range from
     * {@link Tokenizer#findTokenStart} to {@link #getSelectionEnd}
     * if the length of that range meets or exceeds {@link #getThreshold}
     * and makes sure that the range is not already a Chip.
     */
    @Override
    protected void performFiltering(CharSequence text, int keyCode) {
        /// M: Do not do query while creating / removing chip
        if (mChipProcessingMode != PROCESSING_MODE.NONE) {
            return;
        }
        
        boolean isCompletedToken = isCompletedToken(text);
        if (enoughToFilter() && text != null && !isCompletedToken) {
            int end = getSelectionEnd();
            int start = mTokenizer.findTokenStart(text, end);
            // If this is a RecipientChip, don't filter
            // on its contents.
            Spannable span = getSpannable();
            DrawableRecipientChip[] chips = span.getSpans(start, end, DrawableRecipientChip.class);
            if (chips != null && chips.length > 0) {
                /// M: close the drop down if no need to query
                dismissDropDown();
                return;
            }
        } else if (isCompletedToken) {
            /// M: close the drop down if no need to query
            dismissDropDown();
            return;
        }
        super.performFiltering(text, keyCode);
    }

    /// M: When touch after paste,dismiss filter popup. @{
    public void onFilterComplete(int count) {
        if (!bTouchedAfterPasted) {
            super.onFilterComplete(count);
        }
        bPasted = false;
        bTouchedAfterPasted = false;
    }
    /// M: }@

    // Visible for testing.
    /*package*/ boolean isCompletedToken(CharSequence text) {
        if (TextUtils.isEmpty(text)) {
            return false;
        }
        // Check to see if this is a completed token before filtering.
        int end = text.length();
        int start = mTokenizer.findTokenStart(text, end);
        String token = text.toString().substring(start, end).trim();
        if (!TextUtils.isEmpty(token)) {
            char atEnd = token.charAt(token.length() - 1);
            return atEnd == COMMIT_CHAR_COMMA || atEnd == COMMIT_CHAR_SEMICOLON || atEnd == COMMIT_CHAR_CHINESE_COMMA || atEnd == COMMIT_CHAR_CHINESE_SEMICOLON;
        }
        return false;
    }

    private void clearSelectedChip() {
        if (mSelectedChip != null) {
            unselectChip(mSelectedChip);
        }
        setCursorVisible(true);
    }
    /// M: check if scroll action occurs. @{
    private boolean mHasScrolled = false;
    ///@}
    /**
     * Monitor touch events in the RecipientEditTextView.
     * If the view does not have focus, any tap on the view
     * will just focus the view. If the view has focus, determine
     * if the touch target is a recipient chip. If it is and the chip
     * is not selected, select it and clear any other selected chips.
     * If it isn't, then select that chip.
     */
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        /// M: check if scroll action occurs. @{
        switch(event.getAction()) {
            case MotionEvent.ACTION_DOWN :
                mDownPosY = event.getY();
                mHasScrolled = false;
                break;
            case MotionEvent.ACTION_MOVE :
                float yDelta = Math.abs(mDownPosY - event.getY());
                if(yDelta > DELTA_Y_THRESHOLD){
                    mHasScrolled = true;
                }
                break;
        }/// @}
        if (!isFocused()) {
            // Ignore any chip taps until this view is focused.
            return super.onTouchEvent(event);
        }

        /// M: When touch after paste, don't care filter popup.
        if (bPasted) {
            bTouchedAfterPasted = true;
            dismissDropDown();
        }

        /// M: Temporary disable chip touching functionality during processing 
        if (mForceEnableBringPointIntoView) {
            printDebugLog(TAG, "[onTouchEvent] blocked by mForceEnableBringPointIntoView");
            return super.onTouchEvent(event);
        }

        /// M: Get currentChip if touch point is in chip. Only show soft input when currentChip is INVALID_CONTACT or GENERATED_CONTACT. @{
        float x = -1;
        float y = -1;
        int offset = -1;
        DrawableRecipientChip currentChip = null;
        int action = event.getAction();
        boolean shouldShowSoftInput = true;

        /// M: use the flag "outOfView" to ignore unnecessary UP event
        boolean outOfView = false;
        if (action == MotionEvent.ACTION_UP) {
            x = event.getX();
            y = event.getY();

            if (x < getTotalPaddingLeft() || getWidth() - getTotalPaddingRight() < x 
                    || y < getTotalPaddingTop() || getHeight() - getTotalPaddingBottom() < y) {
                setDisableBringPointIntoView(true);
                outOfView = true;
                printDebugLog(TAG, "[onTouchEvent] out of view, X: " + x + ", Y: " + y);
            }

            if (mCopyAddress == null && !outOfView) {
                offset = putOffsetInRange(x, y);
                /// M: Fix misrecognize a touch event is located in a chip while it's actually out side of the chip.
                boolean inChipHor = isTouchPointInChip(x, y);
                boolean inChipVer = isTouchPointInChipVertical(y);
                currentChip = (inChipHor && inChipVer) ? (findChip(offset)) : (null); 
                
                if (currentChip != null) {
                    shouldShowSoftInput = shouldShowEditableText(currentChip);
                    if (!shouldShowSoftInput) {
                        super.setShowSoftInputOnFocus(false);
                    }
                }

                /// M: do not scroll to bottom if hit a chip or a chip is selected
                setDisableBringPointIntoView(inChipHor || mSelectedChip != null);
            }
        }
        /// @}

        boolean handled = super.onTouchEvent(event);
        boolean chipWasSelected = false;
        if (mSelectedChip == null) {
            mGestureDetector.onTouchEvent(event);
        }

        /// M: Don't handle the release after a long press, because it will
        /// move the selection away from whatever the menu action was
        /// trying to affect.@{ 
        if (getEnableDiscardNextActionUp() && action == MotionEvent.ACTION_UP) {
            setEnableDiscardNextActionUp(false);
            /// M: Reset ShowSoftInputOnFocus in TextView. @{
            if (!shouldShowSoftInput) {
                super.setShowSoftInputOnFocus(true);
            }
            /// @}
            return handled;
        }
        /// @} 

        if (mCopyAddress == null && action == MotionEvent.ACTION_UP) {
            printDebugLog(TAG, "[onTouchEvent] ACTION_UP");
            /// M: Do nothing after scrolling in case trigger scroll view to the end of text afterwards. @{
            if (isPhoneQuery() && mMoveCursorToVisible) {
                mMoveCursorToVisible = false;
                /// M: Reset ShowSoftInputOnFocus in TextView. @{
                if (!shouldShowSoftInput) {
                    super.setShowSoftInputOnFocus(true);
                }
                /// @}
                return true;
            }
            /// @}
            
            if (!outOfView && currentChip != null) {
                if (action == MotionEvent.ACTION_UP && !mHasScrolled) {
                    if (mSelectedChip != null && mSelectedChip != currentChip) {
                        clearSelectedChip();
                        mSelectedChip = selectChip(currentChip);
                    } else if (mSelectedChip == null) {
                        setSelection(getText().length());
                        commitDefault();
                        mSelectedChip = selectChip(currentChip);
                        /// M: Disable the scrolling caused by commit chip (if some texts are input)
                        setDisableBringPointIntoView(true);
                    } else {
                        onClick(mSelectedChip, offset, x, y);
                    }
                    /// M: Save the line on which the selected chip is located
                    if (mSelectedChip != null) {
                        mLineOfSelectedChip = getLineOfChip(mSelectedChip);
                    }
                }
                chipWasSelected = true;
                handled = true;
            } else if (mSelectedChip != null && shouldShowEditableText(mSelectedChip)) {
                chipWasSelected = true;
            }
        }
        if (action == MotionEvent.ACTION_UP && !chipWasSelected) {
            clearSelectedChip();
        }
        /// M: clear select chip when scrolling happens . @{
        if((action == MotionEvent.ACTION_CANCEL || mHasScrolled)
        		&& mSelectedChip != null) {
        	clearSelectedChip();
        }
        /// @}
        
        /// M: Reset ShowSoftInputOnFocus in TextView. @{
        if (!shouldShowSoftInput) {
            super.setShowSoftInputOnFocus(true);
        }
        /// @}
        return handled;
    }

    private void scrollLineIntoView(int line) {
        if (mScrollView != null) {
            mScrollView.smoothScrollBy(0, calculateOffsetFromBottom(line));
        }
    }

    AsyncTask<Void, Void, ListAdapter> mShowAlternatesTask;
    
    /// M: add a anchorview in viewhierarchy for ALPS01247954 @{  
    private View mAnchorView = null ;
    
    private void adjustAnchorView(final DrawableRecipientChip currentChip){
        if(mAnchorView == null){
	        mAnchorView = new View(getContext());
	    	mAnchorView.setVisibility(View.INVISIBLE);
	    	ViewGroup rootView = (ViewGroup)getRootView();  	
	    	rootView.addView(mAnchorView);
        }
    	int[] anchorPos = new int[2]; 
        getLocationOnScreen(anchorPos);        
    	int line = getLineOfChip(currentChip);
    	int offsetFromWindowTop =anchorPos[1] + getPaddingTop() + getLayout().getLineTop(line) - getScrollY();
    	int x = anchorPos[0] + getPaddingLeft() + (int)(getLayout().getPrimaryHorizontal(getChipStart(currentChip)))  - getScrollX(); 
    	
    	FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(1, (int)mChipHeight + (-2 * mDefaultVerticalOffset),Gravity.FILL_VERTICAL);
    	lp.setMargins(x-1, offsetFromWindowTop - mDefaultVerticalOffset, 0, 0); 
    	mAnchorView.setLayoutParams(lp);
    }
    /// @}
    private void showAlternates(final DrawableRecipientChip currentChip,
            final ListPopupWindow alternatesPopup, final int width) {
        printDebugLog(TAG, "[showAlternates] " + (currentChip == null ? null : currentChip.getValue()));
        /// M: Cancel task for showing alternates popup @{    
        if (mShowAlternatesTask != null) {
            mShowAlternatesTask.cancel(true);
        }
        /// M: adjust anchorview positon @{
        adjustAnchorView(currentChip);
        /// @}
        mShowAlternatesTask = new AsyncTask<Void, Void, ListAdapter>() {
            @Override
            protected ListAdapter doInBackground(final Void... params) {
                return createAlternatesAdapter(currentChip);
            }

            @Override
            protected void onPostExecute(final ListAdapter result) {
                if (!mAttachedToWindow) {
                    printDebugLog(TAG, "[mShowAlternatesTask][onPostExecute] !mAttachedToWindow, return");
                    return;
                }
                printDebugLog(TAG, "[mShowAlternatesTask][onPostExecute]");
                
                mHandlingAlternatesDropDown = false; /// M: reset this flag when the alternates popup is being shown
                alternatesPopup.setWidth(width);
                alternatesPopup.setAnchorView( mAnchorView ); /// M: change the anchorView from "this" to mAnchorView 
                alternatesPopup.setAdapter(result);
                alternatesPopup.setOnItemClickListener(mAlternatesListener);
                // Clear the checked item.
                mCheckedItem = -1;
                alternatesPopup.show();
                ListView listView = alternatesPopup.getListView();
                listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
                // Checked item would be -1 if the adapter has not
                // loaded the view that should be checked yet. The
                // variable will be set correctly when onCheckedItemChanged
                // is called in a separate thread.
                if (mCheckedItem != -1) {
                    listView.setItemChecked(mCheckedItem, true);
                    mCheckedItem = -1;
                }

                mShowAlternatesTask = null;
            }
        };        
        mShowAlternatesTask.execute((Void[]) null);
    }    

    private ListAdapter createAlternatesAdapter(DrawableRecipientChip chip) {
        if (isPhoneQuery()) {
            /// M: Show phone number and email simutaneously when select chip in phoneQuery
            RecipientAlternatesAdapter adapter = new RecipientAlternatesAdapter(getContext(), chip.getContactId(), chip.getDataId(),
                ((BaseRecipientAdapter)getAdapter()).getQueryType(), this, ((BaseRecipientAdapter)getAdapter()).getShowPhoneAndEmail());
            return adapter;
        } else { 
            return new RecipientAlternatesAdapter(getContext(), chip.getContactId(), chip.getDataId(),
                ((BaseRecipientAdapter)getAdapter()).getQueryType(), this);    
        }
    }

    private ListAdapter createSingleAddressAdapter(DrawableRecipientChip currentChip) {
        return new SingleRecipientArrayAdapter(getContext(), mAlternatesLayout, currentChip
                .getEntry());
    }

    @Override
    public void onCheckedItemChanged(int position) {
        ListView listView = mAlternatesPopup.getListView();
        if (listView != null && listView.getCheckedItemCount() == 0) {
            listView.setItemChecked(position, true);
        }
        mCheckedItem = position;
    }

    private int putOffsetInRange(final float x, final float y) {
        final int offset;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
            offset = getOffsetForPosition(x, y);
        } else {
            offset = supportGetOffsetForPosition(x, y);
        }

        return putOffsetInRange(offset);
    }    

    // TODO: This algorithm will need a lot of tweaking after more people have used
    // the chips ui. This attempts to be "forgiving" to fat finger touches by favoring
    // what comes before the finger.
    private int putOffsetInRange(int o) {
        int offset = o;
        Editable text = getText();
        int length = text.length();
        // Remove whitespace from end to find "real end"
        int realLength = length;
        for (int i = length - 1; i >= 0; i--) {
            if (text.charAt(i) == ' ') {
                realLength--;
            } else {
                break;
            }
        }

        // If the offset is beyond or at the end of the text,
        // leave it alone.
        if (offset >= realLength) {
            return offset;
        }
        Editable editable = getText();
        while (offset >= 0 && findText(editable, offset) == -1 && findChip(offset) == null) {
            // Keep walking backward!
            offset--;
        }
        return offset;
    }

    private static int findText(Editable text, int offset) {
        if (text.charAt(offset) != ' ') {
            return offset;
        }
        return -1;
    }

    private DrawableRecipientChip findChip(int offset) {
        DrawableRecipientChip[] chips = getSpannable().getSpans(0, getText().length(), DrawableRecipientChip.class);
        // Find the chip that contains this offset.
        for (int i = 0; i < chips.length; i++) {
            DrawableRecipientChip chip = chips[i];
            int start = getChipStart(chip);
            int end = getChipEnd(chip);
            if (offset >= start && offset <= end) {
                return chip;
            }
        }
        return null;
    }

    // Visible for testing.
    // Use this method to generate text to add to the list of addresses.
    /* package */String createAddressText(RecipientEntry entry) {
        String display = entry.getDisplayName();
        String address = entry.getDestination();
        if (TextUtils.isEmpty(display) || TextUtils.equals(display, address)) {
            display = null;
        }
        String trimmedDisplayText;
        if (isPhoneQuery() && isPhoneNumber(address)) {
            trimmedDisplayText = address.trim();
        } else {
            if (address != null) {
                // Tokenize out the address in case the address already
                // contained the username as well.
                Rfc822Token[] tokenized = Rfc822Tokenizer.tokenize(address);
                if (tokenized != null && tokenized.length > 0) {
                    address = tokenized[0].getAddress();
                }
            }
            Rfc822Token token = new Rfc822Token(display, address, null);
            trimmedDisplayText = token.toString().trim();
        }
        int index = trimmedDisplayText.indexOf(",");
        return mTokenizer != null && !TextUtils.isEmpty(trimmedDisplayText)
                && index < trimmedDisplayText.length() - 1 ? (String) mTokenizer
                .terminateToken(trimmedDisplayText) : trimmedDisplayText;
    }

    // Visible for testing.
    // Use this method to generate text to display in a chip.
    /*package*/ String createChipDisplayText(RecipientEntry entry) {
        String display = entry.getDisplayName();
        String address = entry.getDestination();
        if (TextUtils.isEmpty(display) || TextUtils.equals(display, address)) {
            display = null;
        }
        if (!TextUtils.isEmpty(display)) {
            return display;
        } else if (!TextUtils.isEmpty(address)){
            /// M: Do tokenizing to get rid of other string 
            address = address.replaceAll("([, ]+$)|([; ]+$)","");
            if (!Patterns.PHONE_PATTERN_MTK.matcher(address).matches()) {
                Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(address);
                address = tokens.length > 0 ? tokens[0].getAddress() : address;
            }
            return address;
        } else {
            return new Rfc822Token(display, address, null).toString();
        }
    }

    private CharSequence createChip(RecipientEntry entry, boolean pressed) {
        String displayText = createAddressText(entry);
        printDebugLog(TAG,"[createChip] displayText: " + displayText + ", pressed: " + pressed);     
        if (TextUtils.isEmpty(displayText)) {
            return null;
        }
        SpannableString chipText = null;
        // Always leave a blank space at the end of a chip.
        int textLength = displayText.length() - 1;
        chipText = new SpannableString(displayText);
        if (!mNoChips) {
            try {
                DrawableRecipientChip chip = constructChipSpan(entry, pressed,
                        false /* leave space for contact icon */);
                chipText.setSpan(chip, 0, textLength,
                        Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                chip.setOriginalText(chipText.toString());
            } catch (NullPointerException e) {
                Log.e(TAG, e.getMessage(), e);
                return null;
            }
        }
        return chipText;
    }

    /**
     * When an item in the suggestions list has been clicked, create a chip from the
     * contact information of the selected item.
     */
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (position < 0) {
            return;
        }
        submitItemAtPosition(position);
    }

    private void submitItemAtPosition(int position) {
        RecipientEntry entry = createValidatedEntry(
                (RecipientEntry)getAdapter().getItem(position));
        if (entry == null) {
            return;
        }
        clearComposingText();

        int end = getSelectionEnd();
        int start = mTokenizer.findTokenStart(getText(), end);

        Editable editable = getText();
        QwertyKeyListener.markAsReplaced(editable, start, end, "");
        CharSequence chip = createChip(entry, false);
        if (chip != null && start >= 0 && end >= 0) {
            printThreadingDebugLog(MTKTAG, "[submitItemAtPosition] replace");
            editable.replace(start, end, chip);
        }
        sanitizeBetween();
        /// M: Replace current contact if there's a duplicate address previous to current one (sort by alphabet).
        if (isPhoneQuery()) {
            DrawableRecipientChip[] currChips = getSpannable().getSpans(start, start + chip.length() - 1, DrawableRecipientChip.class);
            if (currChips == null || currChips.length == 0 ) {
                return;
            }
            DuplicateContactReplacementTask task = new DuplicateContactReplacementTask();
            task.execute(currChips[0]);
        }
        /// @}
    }

    private RecipientEntry createValidatedEntry(RecipientEntry item) {
        if (item == null) {
            return null;
        }
        final RecipientEntry entry;
        // If the display name and the address are the same, or if this is a
        // valid contact, but the destination is invalid, then make this a fake
        // recipient that is editable.
        String destination = item.getDestination();
        if (item.getContactId() == RecipientEntry.GENERATED_CONTACT) { /// M: Let phone also can constructGeneratedEntry
            entry = RecipientEntry.constructGeneratedEntry(item.getDisplayName(),
                    destination, item.isValid());
        } else if (RecipientEntry.isCreatedRecipient(item.getContactId())
                && (TextUtils.isEmpty(item.getDisplayName())
                        || TextUtils.equals(item.getDisplayName(), destination)
                        || (mValidator != null && !mValidator.isValid(destination)))) {
            entry = RecipientEntry.constructFakeEntry(destination, item.isValid());
        } else {
            entry = item;
        }
        return entry;
    }

    /** Returns a collection of contact Id for each chip inside this View. */
    /* package */ Collection<Long> getContactIds() {
        final Set<Long> result = new HashSet<Long>();
        DrawableRecipientChip[] chips = getSortedRecipients();
        if (chips != null) {
            for (DrawableRecipientChip chip : chips) {
                result.add(chip.getContactId());
            }
        }
        return result;
    }


    /** Returns a collection of data Id for each chip inside this View. May be null. */
    /* package */ Collection<Long> getDataIds() {
        final Set<Long> result = new HashSet<Long>();
        DrawableRecipientChip [] chips = getSortedRecipients();
        if (chips != null) {
            for (DrawableRecipientChip chip : chips) {
                result.add(chip.getDataId());
            }
        }
        return result;
    }

    // Visible for testing.
    /* package */DrawableRecipientChip[] getSortedRecipients() {
        /// M: For print TempDebugLog. @{
        Object[] recipientsObj = getSpannable()
                .getSpans(0, getText().length(), DrawableRecipientChip.class);
        boolean printLog = false;
        for (Object currObj : recipientsObj) {
            if(!(currObj instanceof DrawableRecipientChip)) {
                printLog = true;
            }
        }
        if (printLog) {
            for (Object currObj : recipientsObj) {
                tempLogPrint("getSortedRecipients",currObj);
            }
            printLog = false;
        }
        /// @}
        DrawableRecipientChip[] recips = null;
        try {
            recips = getSpannable()
                .getSpans(0, getText().length(), DrawableRecipientChip.class);
        } catch (ArrayStoreException e) {
            /// M: Add for getting more information when ArrayStoreException occurs. @{
            Log.d(TAG,"[getSortedRecipients] ArrayStoreException occurs.");
            Log.e(TAG, e.getMessage(), e);
            LogPrinter lp = new LogPrinter(Log.DEBUG, TAG);
            lp.println("[getSortedRecipients] spans:");
            TextUtils.dumpSpans(getText(), lp, "  ");
            throw new ArrayStoreException();
            /// @}
        }
        ArrayList<DrawableRecipientChip> recipientsList = new ArrayList<DrawableRecipientChip>(Arrays
                .asList(recips));
        final Spannable spannable = getSpannable();
        Collections.sort(recipientsList, new Comparator<DrawableRecipientChip>() {

            @Override
            public int compare(DrawableRecipientChip first, DrawableRecipientChip second) {
                int firstStart = spannable.getSpanStart(first);
                int secondStart = spannable.getSpanStart(second);
                if (firstStart < secondStart) {
                    return -1;
                } else if (firstStart > secondStart) {
                    return 1;
                } else {
                    return 0;
                }
            }
        });
        printThreadingDebugLog("MTKRec", "[getSortedRecipients] end");
        return recipientsList.toArray(new DrawableRecipientChip[recipientsList.size()]);
    }

    @Override
    public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
        return false;
    }

    @Override
    public void onDestroyActionMode(ActionMode mode) {
    }

    @Override
    public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
        return false;
    }

    /**
     * No chips are selectable.
     */
    @Override
    public boolean onCreateActionMode(ActionMode mode, Menu menu) {
        return false;
    }

    // Visible for testing.
    /* package */ImageSpan getMoreChip() {
        MoreImageSpan[] moreSpans = getSpannable().getSpans(0, getText().length(),
                MoreImageSpan.class);
        return moreSpans != null && moreSpans.length > 0 ? moreSpans[0] : null;
    }

    private MoreImageSpan createMoreSpan(int count) {
        printDebugLog(TAG,"[createMoreSpan] count: " + count); 
        String moreText = String.format(mMoreItem.getText().toString(), count);
        TextPaint morePaint = new TextPaint(getPaint());
        morePaint.setTextSize(mMoreItem.getTextSize());
        morePaint.setColor(mMoreItem.getCurrentTextColor());
        int width = (int)morePaint.measureText(moreText) + mMoreItem.getPaddingLeft()
                + mMoreItem.getPaddingRight();
        /// M: Save current textSize and set textSize to defaultSize in case bitmap size of MoreChip is incorrect. @{
        float TempTextSize = getPaint().getTextSize();
        getPaint().setTextSize(mDefaultTextSize);
        /// @}
        int height = getLineHeight();
        Bitmap drawable = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(drawable);
        int adjustedHeight = height;
        Layout layout = getLayout();
        if (layout != null) {
            adjustedHeight -= layout.getLineDescent(0);
        }
        canvas.drawText(moreText, 0, moreText.length(), 0, adjustedHeight, morePaint);

        Drawable result = new BitmapDrawable(getResources(), drawable);
        result.setBounds(0, 0, width, height);
        getPaint().setTextSize(TempTextSize);  /// M: Reset textSize back.
        return new MoreImageSpan(result);
    }

    // Visible for testing.
    /*package*/ void createMoreChipPlainText() {
        // Take the first <= CHIP_LIMIT addresses and get to the end of the second one.
        Editable text = getText();
        int start = 0;
        int end = start;
        for (int i = 0; i < CHIP_LIMIT; i++) {
            end = movePastTerminators(mTokenizer.findTokenEnd(text, start));
            start = end; // move to the next token and get its end.
        }
        // Now, count total addresses.
        start = 0;
        int tokenCount = countTokens(text);
        MoreImageSpan moreSpan = createMoreSpan(tokenCount - CHIP_LIMIT);
        SpannableString chipText = new SpannableString(text.subSequence(end, text.length()));
        chipText.setSpan(moreSpan, 0, chipText.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        printThreadingDebugLog(MTKTAG, "[createMoreChipPlainText] replace");
        text.replace(end, text.length(), chipText);
        mMoreChip = moreSpan;
    }

    // Visible for testing.
    /* package */int countTokens(Editable text) {
        int tokenCount = 0;
        int start = 0;
        while (start < text.length()) {
            start = movePastTerminators(mTokenizer.findTokenEnd(text, start));
            tokenCount++;
            if (start >= text.length()) {
                break;
            }
        }
        return tokenCount;
    }

    /**
     * Create the more chip. The more chip is text that replaces any chips that
     * do not fit in the pre-defined available space when the
     * RecipientEditTextView loses focus.
     */
    // Visible for testing.
    /* package */ void createMoreChip() {
        printDebugLog(TAG,"[createMoreChip] Start");
        
        if (mNoChips) {
            printDebugLog(TAG,"[createMoreChip] mNoChips, return"); 
            createMoreChipPlainText();
            return;
        }

        if (!mShouldShrink) {
            printDebugLog(TAG,"[createMoreChip] !mShouldShrink, return"); 
            return;
        }
        /// M: we call "removeMoreChip()" to remove morechip and put all removed chips back. 
        /// For the case that "hanlePendingChips()--->createMoreChip()" is called in shrink mode , if we don't 
        /// put these removed chips back here , it will lost forever. Because the "getSortedRecipients()" cannot get them since they was remove in last "createMoreChip()" @{
        removeMoreChip();
        /// @}
        DrawableRecipientChip[] recipients = getSortedRecipients();
        printDebugLog(TAG,"[createMoreChip] recipients count: " + (recipients == null ? 0 : recipients.length)); 
        /// M: There's different criterion for phoneQuery & non-phoneQuery.
        if (recipients == null || (!isPhoneQuery() && recipients.length <= CHIP_LIMIT) || (isPhoneQuery() && recipients.length <= 1) ) {
            mMoreChip = null;
            printDebugLog(TAG,"[createMoreChip] no chip or all chips can be shown, return"); 
            return;
        }
        Spannable spannable = getSpannable();
        int numRecipients = recipients.length;
        /// M: Calculate overage. @{
        int overage = 0;
        if (isPhoneQuery()) {
            overage = numRecipients - calculateNumChipsCanShow() ;
            if (overage <= 0) {
                mMoreChip = null;
                printDebugLog(TAG,"[createMoreChip] overage <= 0, return"); 
                return;
            }
        } else {
            overage = numRecipients - CHIP_LIMIT;
        }
        /// @}
        MoreImageSpan moreSpan = createMoreSpan(overage);
        mRemovedSpans = new ArrayList<DrawableRecipientChip>();
        int totalReplaceStart = 0;
        int totalReplaceEnd = 0;
        Editable text = getText();
        /// M: Remove watchers. @{
        watcherProcessor wp = null;
        wp = new watcherProcessor();
        wp.initWatcherProcessor();
        wp.removeSpanWatchers();
        /// @}
        for (int i = numRecipients - overage; i < recipients.length; i++) {
            mRemovedSpans.add(recipients[i]);
            if (i == numRecipients - overage) {
                totalReplaceStart = spannable.getSpanStart(recipients[i]);
            }
            if (i == recipients.length - 1) {
                totalReplaceEnd = spannable.getSpanEnd(recipients[i]);
            }
            if (mTemporaryRecipients == null || !mTemporaryRecipients.contains(recipients[i])) {
                int spanStart = spannable.getSpanStart(recipients[i]);
                int spanEnd = spannable.getSpanEnd(recipients[i]);
                recipients[i].setOriginalText(text.toString().substring(spanStart, spanEnd));
            }
            spannable.removeSpan(recipients[i]);
        }
        /// M: Add watchers back. @{
        if(wp != null) {
            wp.addSpanWatchers();
        }
        recoverLayout();
        /// @}
        if (totalReplaceEnd < text.length()) {
            totalReplaceEnd = text.length();
        }
        int end = Math.max(totalReplaceStart, totalReplaceEnd);
        int start = Math.min(totalReplaceStart, totalReplaceEnd);
        SpannableString chipText = new SpannableString(text.subSequence(start, end));
        chipText.setSpan(moreSpan, 0, chipText.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        printThreadingDebugLog(MTKTAG, "[createMoreChip] replace");
        printDebugLog(TAG,"[createMoreChip] do replace, start: " + start + ", end: " + end); 
        text.replace(start, end, chipText);
        mMoreChip = moreSpan;
        // If adding the +more chip goes over the limit, resize accordingly.
        if (!isPhoneQuery() && getLineCount() > mMaxLines) {
            setMaxLines(getLineCount());
        }
        printDebugLog(TAG,"[createMoreChip] End"); 
    }

    /**
     * Replace the more chip, if it exists, with all of the recipient chips it had
     * replaced when the RecipientEditTextView gains focus.
     */
    // Visible for testing.
    /*package*/ void removeMoreChip() {
        printDebugLog(TAG,"[removeMoreChip], more chip span count: " + (mRemovedSpans == null ? null : mRemovedSpans.size())); 
        if (mMoreChip != null) {
            Spannable span = getSpannable();
            /// M: we set the fromIndex to the start of morechip instead of google's code because we found 
            /// that handPendingChips() may be called in shrink mode . And then there are some replacement chips have been
            /// added , it may cause formIndex incorrect @{
            int fromIndex = span.getSpanStart(mMoreChip);
            /// @}
            span.removeSpan(mMoreChip);
            mMoreChip = null;
            // Re-add the spans that were removed.
            if (mRemovedSpans != null && mRemovedSpans.size() > 0) {
                // Recreate each removed span.
                DrawableRecipientChip[] recipients = getSortedRecipients();
                // Start the search for tokens after the last currently visible
                // chip.
                if (recipients == null || recipients.length == 0) {
                    return;
                }
                Editable editable = getText();
                printDebugLog(TAG,"[removeMoreChip], text = "+ editable.toString());
                /// M: Remove watchers. @{
                watcherProcessor wp = null;
                wp = new watcherProcessor();
                wp.initWatcherProcessor();
                wp.removeSpanWatchers();
                /// @}
                for (DrawableRecipientChip chip : mRemovedSpans) {
                    int chipStart;
                    int chipEnd;
                    String token;
                    // Need to find the location of the chip, again.
                    token = (String) chip.getOriginalText();
                    printDebugLog(TAG,"[removeMoreChip], token = " + token);
                    // As we find the matching recipient for the remove spans,
                    // reduce the size of the string we need to search.
                    // That way, if there are duplicates, we always find the correct
                    // recipient.
                    chipStart = editable.toString().indexOf(token, fromIndex);
                    fromIndex = chipEnd = Math.min(editable.length(), chipStart + token.length());
                    // Only set the span if we found a matching token.
                    if (chipStart != -1) {
                        printThreadingDebugLog(MTKTAG, "[removeMoreChip] setSpan chipStart = " + chipStart +" chipEnd =" + chipEnd);
                        editable.setSpan(chip, chipStart, chipEnd,
                                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                    }
                }
                /// M: Add watchers back. @{
                if(wp != null) {
                    wp.addSpanWatchers();
                }
                recoverLayout();
                /// @}
                mRemovedSpans.clear();
            }
        }
    }

    /**
     * Show specified chip as selected. If the RecipientChip is just an email address,
     * selecting the chip will take the contents of the chip and place it at
     * the end of the RecipientEditTextView for inline editing. If the
     * RecipientChip is a complete contact, then selecting the chip
     * will change the background color of the chip, show the delete icon,
     * and a popup window with the address in use highlighted and any other
     * alternate addresses for the contact.
     * @param currentChip Chip to select.
     * @return A RecipientChip in the selected state or null if the chip
     * just contained an email address.
     */
    private DrawableRecipientChip selectChip(DrawableRecipientChip currentChip) {
        printDebugLog(TAG,"[selectChip] " + (currentChip == null ? null : currentChip.getValue())); 
        if (shouldShowEditableText(currentChip)) {
            CharSequence text = currentChip.getValue();
            Editable editable = getText();
            Spannable spannable = getSpannable();
            int spanStart = spannable.getSpanStart(currentChip);
            int spanEnd = spannable.getSpanEnd(currentChip);
            /// M: Remove trailing space characters @{
            int toDelete = spanEnd;
            while (toDelete >= 0 && toDelete < editable.length() && editable.charAt(toDelete) == ' ') {
                toDelete++;
            }
            /// M: @}
            spannable.removeSpan(currentChip);
            editable.delete(spanStart, toDelete);
            setCursorVisible(true);
            setSelection(editable.length());
            editable.append(text);
            setDisableBringPointIntoView(false); /// M: Scroll view when select INVALID_CONTACT or GENERATED_CONTACT
            return null; /// M: Just return null if we select fake chip, the text will be commited as chip later
        } else if (currentChip.getContactId() == RecipientEntry.GENERATED_CONTACT
                || currentChip.isGalContact() || currentChip.getContactId() == 0) { /// M: Deal with this situation that contact id is 0
            int start = getChipStart(currentChip);
            int end = getChipEnd(currentChip);
            getSpannable().removeSpan(currentChip);
            DrawableRecipientChip newChip;
            try {
                if (mNoChips) {
                    return null;
                }
                newChip = constructChipSpan(currentChip.getEntry(), true, false);
            } catch (NullPointerException e) {
                Log.e(TAG, e.getMessage(), e);
                return null;
            }
            Editable editable = getText();
            QwertyKeyListener.markAsReplaced(editable, start, end, "");
            if (start == -1 || end == -1) {
                Log.d(TAG, "The chip being selected no longer exists but should.");
            } else {
                printThreadingDebugLog(MTKTAG, "[selectChip] setSpan");
                editable.setSpan(newChip, start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            newChip.setSelected(true);
            if (shouldShowEditableText(newChip)) {
                scrollLineIntoView(getLayout().getLineForOffset(getChipStart(newChip)));
            }
            showAddress(newChip, mAddressPopup, getWidth());
            setCursorVisible(false);
            return newChip;
        } else {
            int start = getChipStart(currentChip);
            int end = getChipEnd(currentChip);
            getSpannable().removeSpan(currentChip);
            DrawableRecipientChip newChip;
            try {
                newChip = constructChipSpan(currentChip.getEntry(), true, false);
            } catch (NullPointerException e) {
                Log.e(TAG, e.getMessage(), e);
                return null;
            }
            Editable editable = getText();
            QwertyKeyListener.markAsReplaced(editable, start, end, "");
            if (start == -1 || end == -1) {
                Log.d(TAG, "The chip being selected no longer exists but should.");
            } else {
                printThreadingDebugLog(MTKTAG, "[selectChip] setSpan");
                editable.setSpan(newChip, start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            newChip.setSelected(true);
            if (shouldShowEditableText(newChip)) {
                scrollLineIntoView(getLayout().getLineForOffset(getChipStart(newChip)));
            }
            showAlternates(newChip, mAlternatesPopup, getWidth());
            setCursorVisible(false);
            return newChip;
        }
    }

    private boolean shouldShowEditableText(DrawableRecipientChip currentChip) {
        long contactId = currentChip.getContactId();
        return contactId == RecipientEntry.INVALID_CONTACT
                || (!isPhoneQuery() && contactId == RecipientEntry.GENERATED_CONTACT);
    }

    private void showAddress(final DrawableRecipientChip currentChip, final ListPopupWindow popup,
            int width) {
        printDebugLog(TAG,"[showAddress] " + (currentChip == null ? null : currentChip.getValue()));
        if (!mAttachedToWindow) {            
            return;
        }
        int line = getLayout().getLineForOffset(getChipStart(currentChip));
        int bottom = getOffsetFromBottom(line); /// M: Locate drop-down list at proper position
        // Align the alternates popup with the left side of the View,
        // regardless of the position of the chip tapped.
        popup.setWidth(width);
        popup.setAnchorView(this);
        popup.setVerticalOffset(bottom + mDefaultVerticalOffset); /// M: Adjust position of popup window
        popup.setAdapter(createSingleAddressAdapter(currentChip));
        popup.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                unselectChip(currentChip);
                popup.dismiss();
            }
        });
        popup.show();
        ListView listView = popup.getListView();
        listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        listView.setItemChecked(0, true);
    }

    /**
     * Remove selection from this chip. Unselecting a RecipientChip will render
     * the chip without a delete icon and with an unfocused background. This is
     * called when the RecipientChip no longer has focus.
     */
    private void unselectChip(DrawableRecipientChip chip) {
        printDebugLog(TAG,"[unselectChip] " + (chip == null ? null : chip.getValue())); 
        int start = getChipStart(chip);
        int end = getChipEnd(chip);
        Editable editable = getText();
        mSelectedChip = null;
        if (start == -1 || end == -1) {
            Log.w(TAG, "The chip doesn't exist or may be a chip a user was editing");
            setSelection(editable.length());
            commitDefault();
        } else {
            getSpannable().removeSpan(chip);
            QwertyKeyListener.markAsReplaced(editable, start, end, "");
            editable.removeSpan(chip);
            try {
                if (!mNoChips) {
                    printThreadingDebugLog(MTKTAG, "[unSelectChip] setSpan");
                    editable.setSpan(constructChipSpan(chip.getEntry(), false, false),
                            start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                }
            } catch (NullPointerException e) {
                Log.e(TAG, e.getMessage(), e);
            }
        }
        setCursorVisible(true);
        setSelection(editable.length());
        if (mAlternatesPopup != null && mAlternatesPopup.isShowing()) {
            mAlternatesPopup.dismiss();
        }
        /// M: cancel the async task to show pop @{
        if (mShowAlternatesTask != null) {
            mShowAlternatesTask.cancel(true);
        }
        /// @}
        /// M: Dismiss mAddressPopup. @{
        if (mAddressPopup != null && mAddressPopup.isShowing()) {
            mAddressPopup.dismiss();
        }
        /// @}
    }

    /**
     * Return whether a touch event was inside the delete target of
     * a selected chip. It is in the delete target if:
     * 1) the x and y points of the event are within the
     * delete assset.
     * 2) the point tapped would have caused a cursor to appear
     * right after the selected chip.
     * @return boolean
     */
    private boolean isInDelete(DrawableRecipientChip chip, int offset, float x, float y) {
        // Figure out the bounds of this chip and whether or not
        // the user clicked in the X portion.
        // TODO: Should x and y be used, or removed?
        return chip.isSelected() && offset == getChipEnd(chip);
    }

    /**
     * Remove the chip and any text associated with it from the RecipientEditTextView.
     */
    // Visible for testing.
    /*pacakge*/ void removeChip(DrawableRecipientChip chip) {
        printDebugLog(TAG,"[removeChip] " + (chip == null ? null : chip.getValue())); 
        Spannable spannable = getSpannable();
        int spanStart = spannable.getSpanStart(chip);
        int spanEnd = spannable.getSpanEnd(chip);
        Editable text = getText();
        int toDelete = spanEnd;
        /// M: set the flag to block doing query
        setChipProcessingMode(spanEnd == (text.length() - 1) ? 
                PROCESSING_MODE.REMOVE_LAST : PROCESSING_MODE.REMOVE);
        /// M: Keep the line count before chip removed
        int preLineCount = getLayout() != null ? getLayout().getLineCount() : 0;
        boolean wasSelected = chip == mSelectedChip;
        // Clear that there is a selected chip before updating any text.
        if (wasSelected) {
            mSelectedChip = null;
        }
        // Always remove trailing spaces when removing a chip.
        while (toDelete >= 0 && toDelete < text.length() && text.charAt(toDelete) == ' ') {
            toDelete++;
        }
        /// M: No need to adopt accelerate mechanism if removed last chip. @{
        boolean needAccelerate = true;
        DrawableRecipientChip lastChip = getLastChip();
        if ((getChipStart(chip) == getChipStart(lastChip)) && (getChipEnd(chip) == getChipEnd(lastChip))) {
            needAccelerate = false;
        }
        /// @}
        spannable.removeSpan(chip);
        /// M: No need to adopt acclerate mechanism if no chip left. @{
        DrawableRecipientChip[] chips = getSortedRecipients();
        if ((chips == null || chips.length == 0)) {
            needAccelerate = false;
        }
        /// @}
        /// M: Temporarily remove all the chips after removedChip, and save them for later use. @{
        RecipientChipProcessor rcp = new RecipientChipProcessor();
        if (needAccelerate) {
            /// M: Get start index of removed chips
            int index = 0;
            for (index = 0; index < chips.length; index++) {
                if (getChipStart(chips[index]) >= spanStart) {
                    break;
                }
            }
            rcp.removeChipsWithoutNotification(index, chips.length);
            mDuringAccelerateRemoveChip = true;
        }
        /// @}
        if (spanStart >= 0 && toDelete > 0) {
            printThreadingDebugLog(MTKTAG, "[removeChip] delete");
            text.delete(spanStart, toDelete);
        }
        /// M: Add all the temporarily removed chips back. @{
        if (needAccelerate) {
            mDuringAccelerateRemoveChip = false;
            rcp.addChipsBackWithoutNotification(toDelete - spanStart);
        }

        /// @}
        if (wasSelected) {
            clearSelectedChip();
            /// M: Check for these 2 conditions:
            /// 1. The line count changed after chip removed
            /// 2. The removed chip is on the last line or the previous line above last
            /// If 1 & 2 are true, the last chip must has moved to previous line 
            /// and we should scroll the view to the bottom @{
            if (mLineOfSelectedChip >= preLineCount - 2) {
                int postLineCount = getLayout() != null ? getLayout().getLineCount() : 0;
                if (preLineCount != postLineCount) {
                    setDisableBringPointIntoView(false);
                    scrollBottomIntoView(); //// this line necessary???
                }
            }
            /// @}
        }

        /// M: Cancel task for showing alternates popup @{
        if (mShowAlternatesTask != null) {
            mShowAlternatesTask.cancel(true);
        }
        /// M:@}
        setChipProcessingMode(PROCESSING_MODE.NONE);
    }

    /**
     * Replace this currently selected chip with a new chip
     * that uses the contact data provided.
     */
    // Visible for testing.
    /*package*/ void replaceChip(DrawableRecipientChip chip, RecipientEntry entry) {
        printDebugLog(TAG, "[replaceChip] start");
        boolean wasSelected = chip == mSelectedChip;
        if (wasSelected) {
            mSelectedChip = null;
        }
        int start = getChipStart(chip);
        int end = getChipEnd(chip);
        DrawableRecipientChip[] currChips = getSortedRecipients();
        printDebugLog(TAG, "[replaceChip] start: " + start + ", end: " + end + ", chip: " + chip.getEntry());
        /// M: Check if the first chip is what we are look for
        /// The first chip may be updated due to screen rotation. @{
        if (start == -1 || end == -1) {            
            RecipientEntry entry1 = chip.getEntry();
            if (currChips != null && currChips.length > 0) {
                RecipientEntry entry2 = currChips[0].getEntry();
                if (compareEntries(entry1, entry2)){
                    printDebugLog(TAG, "[replaceChip] The first chip is changed, update to the new one");
                    chip = currChips[0];
                    start = getChipStart(chip);
                    end = getChipEnd(chip);
                }                
            }
            // If not match the first match, try to find it in the mRemovedSpans
            // This case if is the chip to be replaced (post by IndividualReplacementTask) is moved to mRemovedSpans
            // due to screen rotation from landscape to portrait
            if (start == -1 || end == -1) {
                if (mRemovedSpans != null && mRemovedSpans.size() > 0) {
                    int MAX_SEARCH_COUNT = 5;
                    int searchCount = Math.min(MAX_SEARCH_COUNT, mRemovedSpans.size());
                    for (int idx = 0; idx < searchCount; idx++) {
                        RecipientEntry entry3 = mRemovedSpans.get(idx).getEntry();
                        if (compareEntries(entry1, entry3)){
                            printDebugLog(TAG, "[replaceChip] Found in mRemovedSpans, index:"+idx+", ignore this replacing action");
                            return;
                        }
                    }
                }
            }    
        }
        /// @}
        getSpannable().removeSpan(chip);
        Editable editable = getText();
        if (!mDuringReplaceDupChips) {
            setChipProcessingMode(end == (editable.length() - 1) ? 
                PROCESSING_MODE.REPLACE_LAST : PROCESSING_MODE.REPLACE);
        }
        CharSequence chipText = createChip(entry, false);
        printDebugLog(TAG, "[replaceChip] start: " + start + ", end: " + end + ", chip: " + chip.getEntry() + ", chipText: " + chipText);
        /// M: No need to adopt accelerate mechanism if replaced last chip. @{
        boolean needAccelerate = true;
        DrawableRecipientChip lastChip = getLastChip();
        if ((getChipStart(chip) == getChipStart(lastChip)) && (getChipEnd(chip) == getChipEnd(lastChip))) {
            needAccelerate = false;
        }
        /// @}
        /// M: Temporarily remove all the chips after replacedChip, and save them for later use. @{
        RecipientChipProcessor rcp = new RecipientChipProcessor();
        currChips = getSortedRecipients();
        Spannable spannable = getSpannable();
        if (needAccelerate) {
            /// M: Get start index of removed chips
            int index = 0;
            for (index = 0; index < currChips.length; index++) {
                if (getChipStart(currChips[index]) >= start) {
                    break;
                }
            }
            rcp.removeChipsWithoutNotification(index, currChips.length);
            mDuringAccelerateRemoveChip = true;
        }
        /// @}
        if (chipText != null) {
            if (start == -1 || end == -1) {
                Log.e(TAG, "[WARNING] The chip to replace does not exist but should.");
                /// M: Do not insert the chip as error handling, this only makes it worse
                //editable.insert(0, chipText);
            } else {
                if (!TextUtils.isEmpty(chipText)) {
                    // There may be a space to replace with this chip's new
                    // associated space. Check for it
                    int toReplace = end;
                    while (toReplace >= 0 && toReplace < editable.length()
                            && editable.charAt(toReplace) == ' ') {
                        toReplace++;
                    }
                    printThreadingDebugLog(MTKTAG, "[replaceChip] replace");
                    editable.replace(start, toReplace, chipText);
                }
            }
        }
        /// M: Add all the temporarily removed chips back. @{
        if (needAccelerate) {
            mDuringAccelerateRemoveChip = false;
            final int chipTextLength = chipText != null ? chipText.length() : 0;
            rcp.addChipsBackWithoutNotification((end - start + 1) - chipTextLength);
        }
        /// @}
        setCursorVisible(true);
        if (wasSelected) {
            clearSelectedChip();
        }
        setChipProcessingMode(PROCESSING_MODE.NONE);
        printDebugLog(TAG, "[replaceChip] end");
    }

    /// M: Comapre the ID, name, destination of 2 entries
    boolean compareEntries(RecipientEntry entry1, RecipientEntry entry2) {
        if (entry1 == null || entry2 == null) {
            return false;
        }
        return entry1.getContactId() == entry2.getContactId()
                && entry1.getDisplayName().equals(entry2.getDisplayName())
                && entry1.getDestination().equals(entry2.getDestination());
    }

    /**
     * Handle click events for a chip. When a selected chip receives a click
     * event, see if that event was in the delete icon. If so, delete it.
     * Otherwise, unselect the chip.
     */
    public void onClick(DrawableRecipientChip chip, int offset, float x, float y) {
        printDebugLog(TAG,"[onClick] " + (chip == null ? null : chip.getValue())); 
        if (chip.isSelected()) {
            if (isInDelete(chip, offset, x, y)) {
                removeChip(chip);
            } else {
                clearSelectedChip();
            }
        }
    }

    private boolean chipsPending() {
        return mPendingChipsCount > 0 || (mRemovedSpans != null && mRemovedSpans.size() > 0);
    }

    @Override
    public void removeTextChangedListener(TextWatcher watcher) {
        mTextWatcher = null;
        super.removeTextChangedListener(watcher);
    }

    private class RecipientTextWatcher implements TextWatcher {

        @Override
        public void afterTextChanged(Editable s) {
            printDebugLog(TAG,"[RecipientTextWatcher.afterTextChanged]"); 
            /// M: Always set cursor to true if there's any text changed in case cursor disappear.
            setCursorVisible(true);
            // If the text has been set to null or empty, make sure we remove
            // all the spans we applied.
            if (TextUtils.isEmpty(s)) {
                printDebugLog(TAG,"[RecipientTextWatcher.afterTextChanged] text is empty"); 
                // Remove all the chips spans.
                Spannable spannable = getSpannable();
                DrawableRecipientChip[] chips = spannable.getSpans(0, getText().length(),
                            DrawableRecipientChip.class);
                for (DrawableRecipientChip chip : chips) {
                    spannable.removeSpan(chip);
                }
                if (mMoreChip != null) {
                    spannable.removeSpan(mMoreChip);
                }
                /// M: Reset the no chip flag
                mNoChips = false;

                /// M: text is empty. If mTemporaryRecipients is not empty, something is going wrong
                if (mTemporaryRecipients != null && mTemporaryRecipients.size() > 0) {
                    printDebugLog(TAG,"[RecipientTextWatcher.afterTextChanged] mTemporaryRecipients is not empty, count: "
                            + mTemporaryRecipients.size()); 
                    clearTemporaryRecipients(); /// M: replaced with API with log
                }
                return;
            }
            // Get whether there are any recipients pending addition to the
            // view. If there are, don't do anything in the text watcher.
            if (chipsPending()) {
                return;
            }
            // If the user is editing a chip, don't clear it.
            if (mSelectedChip != null) {
                if (!shouldShowEditableText(mSelectedChip)) {
                    setCursorVisible(true);
                    setSelection(getText().length());
                    clearSelectedChip();
                } else {
                    return;
                }
            }
            /// M:
            if (mDuringAccelerateRemoveChip) {
                return;
            }
            /// @}
            int length = s.length();
            // Make sure there is content there to parse and that it is
            // not just the commit character.
            if (length > 1) {
                if (lastCharacterIsCommitCharacter(s)) {
                    commitByCharacter();
                    return;
                }
                char last;
                int end = getSelectionEnd() == 0 ? 0 : getSelectionEnd() - 1;
                int len = length() - 1;
                if (end != len) {
                    last = s.charAt(end);
                } else {
                    last = s.charAt(len);
                }
                if (last == COMMIT_CHAR_CHINESE_COMMA || last == COMMIT_CHAR_CHINESE_SEMICOLON) {
                    /// M: Replace chinese comma or semiconlon to english one. @{
                    if (last == COMMIT_CHAR_CHINESE_COMMA) {
                        getText().replace(end, end + 1, Character.toString(COMMIT_CHAR_COMMA));
                        return;
                    }
                    else if (last == COMMIT_CHAR_CHINESE_SEMICOLON) {
                        getText().replace(end, end + 1, Character.toString(COMMIT_CHAR_SEMICOLON));
                        return;
                    }
                    /// @}
                } else if (last == COMMIT_CHAR_SPACE) {
                    if (!isPhoneQuery()) {
                        // Check if this is a valid email address. If it is,
                        // commit it.
                        String text = getText().toString();
                        int tokenStart = mTokenizer.findTokenStart(text, getSelectionEnd());
                        String sub = text.substring(tokenStart, mTokenizer.findTokenEnd(text,
                                tokenStart));
                        if (!TextUtils.isEmpty(sub) && mValidator != null && mValidator.isValid(sub)) {
                            commitByCharacter();
                        }
                    }
                }
            }
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            printDebugLog(TAG,"[RecipientTextWatcher.onTextChanged] start: " + start + ", before: " + before 
                    + ", count: " + count + ", processing mode: " + mChipProcessingMode); 
            printDebugLog(TAG,"[RecipientTextWatcher.onTextChanged] text: " + s);
            /// M: Don't scroll view when unselect chip
            setDisableBringPointIntoView(mChipProcessingMode != PROCESSING_MODE.NONE
                    && mChipProcessingMode != PROCESSING_MODE.COMMIT
                    && mChipProcessingMode != PROCESSING_MODE.REMOVE_LAST
                    && mChipProcessingMode != PROCESSING_MODE.REPLACE_LAST);
            // The user deleted some text OR some text was replaced; check to
            // see if the insertion point is on a space
            if (before > count) {
                printDebugLog(TAG,"[RecipientTextWatcher.onTextChanged] deleting case");
                // If the item deleted is a space, and the thing before the
                // space is a chip, delete the entire span.
                int selStart = getSelectionStart();
                DrawableRecipientChip[] repl = getSpannable().getSpans(selStart, selStart,
                            DrawableRecipientChip.class);
                printDebugLog(TAG,"[RecipientTextWatcher.onTextChanged] selStart: "+selStart+", repl.length: " + repl.length);
                if (repl.length > 0) {
                    // There is a chip there! Just remove it.
                    Editable editable = getText();
                    // Add the separator token.
                    int tokenStart = mTokenizer.findTokenStart(editable, selStart);
                    int tokenEnd = mTokenizer.findTokenEnd(editable, tokenStart);
                    tokenEnd = tokenEnd + 1;
                    if (tokenEnd > editable.length()) {
                        tokenEnd = editable.length();
                    }
                    /// M: Dismiss drop-down list & clear slected chip if necessary. @{
                    if (mSelectedChip != null) {
                        printDebugLog(TAG,"[RecipientTextWatcher.onTextChanged] mSelectedChip != null");
                        if (tokenStart == getChipStart(mSelectedChip) && tokenEnd == getChipEnd(mSelectedChip)) {
                            if (mAlternatesPopup != null && mAlternatesPopup.isShowing()) {
                                mAlternatesPopup.dismiss();
                            }
                            if (mAddressPopup != null && mAddressPopup.isShowing()) {
                                mAddressPopup.dismiss();
                            }
                            mSelectedChip = null;
                        }
                    }
                    /// @}
                    printThreadingDebugLog(MTKTAG, "[onTextChanged] delete");
                    printDebugLog(TAG,"[RecipientTextWatcher.onTextChanged] delete a chip, tokenStart:"+tokenStart+", tokenEnd:"+tokenEnd);
                    editable.delete(tokenStart, tokenEnd);
                    getSpannable().removeSpan(repl[0]);
                }
            } else if (count > before && !mDisableBringPointIntoView) { /// M: scroll when enable.
                // Only scroll when the user is adding text, not clearing text.
                scrollBottomIntoView();
                if (mSelectedChip != null
                    && shouldShowEditableText(mSelectedChip)) {
                    if (lastCharacterIsCommitCharacter(s)) {
                        commitByCharacter();
                        return;
                    }
                }
            }
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            // Do nothing.
        }
    }

    public boolean lastCharacterIsCommitCharacter(CharSequence s) {
        char last;
        int end = getSelectionEnd() == 0 ? 0 : getSelectionEnd() - 1;
        int len = length() - 1;
        if (end != len) {
            last = s.charAt(end);
        } else {
            last = s.charAt(len);
        }
        return last == COMMIT_CHAR_COMMA || last == COMMIT_CHAR_SEMICOLON;
    }

    /// M: Get the line on which the chip is located
    private int getLineOfChip(DrawableRecipientChip chip) {
        return getLayout() == null ? 0 : getLayout().getLineForOffset(getChipStart(chip));
    }
    
    /**
     * Handles pasting a {@link ClipData} to this {@link RecipientEditTextView}.
     */
    private void handlePasteClip(ClipData clip) {
        /// M: Ignore this paste action in shrink state
        if (!hasFocus() || mMoreChip != null) {
            printDebugLog(TAG,"[handlePasteClip] in shrink state, return"); 
            return;
        }
        printDebugLog(TAG,"[handlePasteClip] start");
        removeTextChangedListener(mTextWatcher);
        /// M: Remove white spaces at the end of text. @{
        do {
            int index = 0;
            DrawableRecipientChip lastChip = getLastChip();
            if (lastChip != null) {
                index = getChipEnd(lastChip) + 1;
            }
            int selEnd = getSelectionEnd();
            if (selEnd <= index || selEnd == 0) {
                /// M: No extra space
                break;
            }
            int x = selEnd;
            String text = getText().toString();
            while (x > index && text.charAt(x - 1) == ' ') {
                x--;
            }
            if ((x - 1) > index && x < (text.length() - 1) && x <= (selEnd - 1) && text.charAt(x) == ' ') {
                /// M: Leave one space if needed
                x++;
            }
            printThreadingDebugLog(MTKTAG, "[handlePasteClip] delete");
            getText().delete(x, selEnd);
        } while (false);
        /// @}

        if (clip != null && clip.getDescription().hasMimeType(ClipDescription.MIMETYPE_TEXT_PLAIN)){
            for (int i = 0; i < clip.getItemCount(); i++) {
                CharSequence paste = clip.getItemAt(i).getText();
                if (paste != null && !TextUtils.isEmpty(paste)) {
                    printDebugLog(TAG,"[handlePasteClip] paste text: " + paste);
                    paste = filterInvalidCharacter(paste);  /// M: replace the invalid character refer to android2.3 Email Chips.          
                    int start = getSelectionStart();
                    int end = getSelectionEnd();
                    printDebugLog(TAG,"[handlePasteClip] filtered text: " + paste + ", start: " + start + ", end: " + end);
                    Editable editable = getText();
                    /// M: This case is when users try to paste something at the space char of previos token
                    /// we adjust the index to the right by 1
                    if (start == end && end < editable.length() && editable.charAt(start) == COMMIT_CHAR_SPACE) {
                        ++start;
                        ++end;
                    }
                    /// M: scroll to bottom after doing paste action
                    setDisableBringPointIntoView(false);
                    if (start >= 0 && end >= 0 && start != end) {
                        /// M: replace the selected text. set cursor to the end. @{                         
                        printThreadingDebugLog(MTKTAG, "[handlePasteClip] replace");
                        editable.replace(start, end, paste);                        
                        setSelection(editable.length());                        
                        /// @}
                    } else {
                        editable.insert(end, paste);
                    }
                    handlePasteAndReplace();
                } else {
                    printDebugLog(TAG,"[handlePasteClip] pasted text is empty, ignore");
                }
            }
        }

        mHandler.post(mAddTextWatcher);
        printDebugLog(TAG,"[handlePasteClip] end");
    }

    /** M: filter invalid character from the string.
     * replace '\n' to ' '
     * replace the one or more ' '(white space) in the beginning of a string to ""
     * A string contains "0 or more ' '(white space) following a ','(comma)" repeat one or more will be replaced to a ','
     * @param source string.
     * @return the processed string.
     */
    private CharSequence filterInvalidCharacter(CharSequence source) {
        String result = source.toString();
        /// M: The '\n' in the middle of the span which cause IndexOutOfBoundsException.
        result = result.replaceAll("\n", " ");
        /// M: String contains chinese comma and semicolon will be replaced to a ','
        result = result.replace(COMMIT_CHAR_CHINESE_COMMA, COMMIT_CHAR_COMMA);
        /// M: String contains chinese semicolon will be replaced to a ';'
        result = result.replace(COMMIT_CHAR_CHINESE_SEMICOLON, COMMIT_CHAR_SEMICOLON);
        /// M: Replace the "0 or more ' '(white space) following a ','(comma)" repeat one or more in the beginning of a string to ""
        result = result.replaceAll("^( *,)+", "");
        /// M: String contains "0 or more ' '(white space) following a ','(comma)" repeat one or more will be replaced to a ','
        result = result.replaceAll("( *,)+", ",");
        result = result.replaceAll("(, *)+", ", ");
        /// M: Replace the "0 or more ' '(white space) following a ';'(semicolon)" repeat one or more in the beginning of a string to ""
        result = result.replaceAll("^( *;)+", "");
        /// M: String contains "0 or more ' '(white space) following a ';'(semicolon)" repeat one or more will be replaced to a ';'
        result = result.replaceAll("( *;)+", ";");
        result = result.replaceAll("(; *)+", "; ");
        /// M: Trim white spaces at the beginning of string
        result = result.replaceAll("^\\s+","");
        return result;
    }

    @Override
    public boolean onTextContextMenuItem(int id) {
        if (id == android.R.id.paste) {
            /// M: When touch after paste, don't care filter popup.
            this.bPasted = true;

            ClipboardManager clipboard = (ClipboardManager) getContext().getSystemService(
                    Context.CLIPBOARD_SERVICE);
            handlePasteClip(clipboard.getPrimaryClip());
            return true;
        }
        return super.onTextContextMenuItem(id);
    }

    private void handlePasteAndReplace() {
        printDebugLog(TAG,"[handlePasteAndReplace]"); 
        ArrayList<DrawableRecipientChip> created = handlePaste();
        if (created != null && created.size() > 0) {
            // Perform reverse lookups on the pasted contacts.
            /// M: IndividualReplacementTask will check the mIndividualReplacements when doing its job
            mIndividualReplacements = new IndividualReplacementTask();
            mIndividualReplacements.execute(created);
        }
    }

    // Visible for testing.
    /* package */ArrayList<DrawableRecipientChip> handlePaste() {
        printDebugLog(TAG,"[handlePaste]"); 
        String text = getText().toString();
        int originalTokenStart = mTokenizer.findTokenStart(text, getSelectionEnd());
        String lastAddress = text.substring(originalTokenStart);
        int tokenStart = originalTokenStart;
        int prevTokenStart = 0;
        DrawableRecipientChip findChip = null;
        ArrayList<DrawableRecipientChip> created = new ArrayList<DrawableRecipientChip>();
        if (tokenStart != 0) {
            // There are things before this!
            while (tokenStart != 0 && findChip == null && tokenStart != prevTokenStart) {
                prevTokenStart = tokenStart;
                tokenStart = mTokenizer.findTokenStart(text, tokenStart);
                findChip = findChip(tokenStart);
                if (tokenStart == originalTokenStart && findChip == null) {
                    break;
                }
                /// M: Stop searching for tokenStart. @{
                if (prevTokenStart == tokenStart) {
                    break;
                }
                /// @}
            }
            if (tokenStart != originalTokenStart) {
                if (findChip != null) {
                    tokenStart = prevTokenStart;
                }
                int tokenEnd;
                DrawableRecipientChip createdChip;
                /// M: Fix lost chip problem (strings stay in string state instead of becoming chips) @{
                int parseEnd = originalTokenStart;
                int offsetFromLastString = text.length() - originalTokenStart;
                /// @}
                while (tokenStart < parseEnd) { 
                    tokenEnd = movePastTerminators(mTokenizer.findTokenEnd(getText().toString(),
                            tokenStart));
                    commitChip(tokenStart, tokenEnd, getText());
                    createdChip = findChip(tokenStart);
                    if (createdChip == null) {
                        break;
                    }
                    // +1 for the space at the end.
                    tokenStart = getSpannable().getSpanEnd(createdChip) + 1;
                    created.add(createdChip);
                    /// M: Fix lost chip problem (strings stay in string state instead of becoming chips) 
                    parseEnd = getText().length() - offsetFromLastString;
                }
            }
        }
        // Take a look at the last token. If the token has been completed with a
        // commit character, create a chip.
        if (isCompletedToken(lastAddress)) {
            Editable editable = getText();
            tokenStart = editable.toString().indexOf(lastAddress, tokenStart); /// M: Use tokenStart after text processing in case wrong index value lead to JE.
            commitChip(tokenStart, editable.length(), editable);
            created.add(findChip(tokenStart));
        }
        return created;
    }

    // Visible for testing.
    /* package */int movePastTerminators(int tokenEnd) {
        if (tokenEnd >= length()) {
            return tokenEnd;
        }
        char atEnd = getText().toString().charAt(tokenEnd);
        if (atEnd == COMMIT_CHAR_COMMA || atEnd == COMMIT_CHAR_SEMICOLON) {
            tokenEnd++;
        }
        // This token had not only an end token character, but also a space
        // separating it from the next token.
        if (tokenEnd < length() && getText().toString().charAt(tokenEnd) == ' ') {
            tokenEnd++;
        }
        return tokenEnd;
    }

    private class RecipientReplacementTask extends AsyncTask<Void, Void, Void> {
        private DrawableRecipientChip createFreeChip(RecipientEntry entry) {
            printDebugLog(TAG,"[RecipientReplacementTask.createFreeChip]"); 
            try {
                if (mNoChips) {
                    return null;
                }
                return constructChipSpan(entry, false, false /*leave space for contact icon */);
            } catch (NullPointerException e) {
                Log.e(TAG, e.getMessage(), e);
                return null;
            }
        }

        @Override
        protected void onPreExecute() {
            // Ensure everything is in chip-form already, so we don't have text that slowly gets
            // replaced
            final List<DrawableRecipientChip> originalRecipients =
                    new ArrayList<DrawableRecipientChip>();
            final DrawableRecipientChip[] existingChips = getSortedRecipients();
            printDebugLog(TAG,"[RecipientReplacementTask.onPreExecute] start, recipient count: " + existingChips.length); 
            for (int i = 0; i < existingChips.length; i++) {
                originalRecipients.add(existingChips[i]);
            }
            if (mRemovedSpans != null) {
                originalRecipients.addAll(mRemovedSpans);
            }

            final List<DrawableRecipientChip> replacements =
                    new ArrayList<DrawableRecipientChip>(originalRecipients.size());

            for (final DrawableRecipientChip chip : originalRecipients) {
                if (RecipientEntry.isCreatedRecipient(chip.getEntry().getContactId())
                        && getSpannable().getSpanStart(chip) != -1) {
                    replacements.add(createFreeChip(chip.getEntry()));
                } else {
                    replacements.add(null);
                }
            }

            processReplacements(originalRecipients, replacements);
            printDebugLog(TAG,"[RecipientReplacementTask.onPreExecute] end"); 
        }

        @Override
        protected Void doInBackground(Void... params) {
            if (mIndividualReplacements != null) {
                printDebugLog(TAG,"[RecipientReplacementTask.doInBackground] mIndividualReplacements.cancel()"); 
                mIndividualReplacements.cancel(true);
                mIndividualReplacements = null;
            }
            // For each chip in the list, look up the matching contact.
            // If there is a match, replace that chip with the matching
            // chip.
            final ArrayList<DrawableRecipientChip> recipients = 
                        new ArrayList<DrawableRecipientChip>();
            DrawableRecipientChip[] existingChips = getSortedRecipients();
            printDebugLog(TAG,"[RecipientReplacementTask.doInBackground] start, recipient count: " + existingChips.length); 
            for (int i = 0; i < existingChips.length; i++) {
                recipients.add(existingChips[i]);
            }
            if (mRemovedSpans != null) {
                printDebugLog(TAG,"[RecipientReplacementTask.doInBackground] removed recipient count: " + mRemovedSpans.size()); 
                recipients.addAll(mRemovedSpans);
            }
            ArrayList<String> addresses = new ArrayList<String>();
            DrawableRecipientChip chip;
            for (int i = 0; i < recipients.size(); i++) {
                chip = recipients.get(i);
                if (chip != null) {
                    addresses.add(createAddressText(chip.getEntry()));
                }
            }
            final BaseRecipientAdapter adapter = (BaseRecipientAdapter) getAdapter();
            RecipientAlternatesAdapter.getMatchingRecipients(getContext(), adapter, addresses, 
                    adapter.getAccount(), new RecipientMatchCallback() {

                        @Override
                        public void matchesFound(Map<String, RecipientEntry> entries) {
                            printDebugLog(TAG,"[RecipientReplacementTask.doInBackground][matchesFound] start, recipients count: " 
                                    + recipients.size() + ", entries size: " + entries.size()); 
                            final ArrayList<DrawableRecipientChip> replacements = 
                                    new ArrayList<DrawableRecipientChip>();
                            for (final DrawableRecipientChip temp : recipients) {
                                RecipientEntry entry = null;
                                if (temp != null) {
                                    long contactID = temp.getEntry().getContactId();
                                    int spanStart = getSpannable().getSpanStart(temp);
                                    printDebugLog(TAG,"[matchesFound] chip: " + temp.getValue() + ", contactID: " + contactID 
                                            + ", spanStart: " + spanStart); 
                                    if (RecipientEntry.isCreatedRecipient(contactID)) {
                                        // Replace this.
                                        /// M: Query with normalized number again if there's no fully matched number. @{
                                        if (isPhoneQuery()) {
                                            String tokenizedAddress = tokenizeAddress(temp.getEntry().getDestination());
                                            entry = entries.get(tokenizedAddress);
                                            printDebugLog(TAG,"[matchesFound] phone case, entry: " + entry); 
                                            if (entry == null && tokenizedAddress != null && !tokenizedAddress.contains("@")) {
                                                entry = RecipientAlternatesAdapter.getRecipientEntryByPhoneNumber(getContext(), tokenizedAddress);
                                            }
                                        } else { /// @}
                                            entry = createValidatedEntry(entries.get(tokenizeAddress(temp.getEntry()
                                                    .getDestination())));
                                            printDebugLog(TAG,"[matchesFound] email case, entry: " + entry); 
                                        }
                                        /// M: If the RecipientEditTextiView went to shrink state before reaching here
                                        /// put the replacing chips back to mTemporaryRecipients, they will be handled on next expanding
                                        if (spanStart == -1) {
                                            addTemporaryRecipients(createFreeChip(entry));
                                            entry = null;
                                        }
                                    }
                                    if (entry != null) {
                                        replacements.add(createFreeChip(entry));
                                    } else {
                                        replacements.add(null);
                                    }
                                }
                            }
                            processReplacements(recipients, replacements);
                            printDebugLog(TAG,"[RecipientReplacementTask.doInBackground][matchesFound] end"); 
                        }

                        @Override
                        public void matchesNotFound(Set<String> unfoundAddresses) {
                            printDebugLog(TAG,"[RecipientReplacementTask.doInBackground][matchesNotFound] start, unfound count: " 
                                    + unfoundAddresses.size()); 
                            if (unfoundAddresses.size() > 0) {
                                final List<DrawableRecipientChip> replacements = 
                                        new ArrayList<DrawableRecipientChip>(unfoundAddresses.size());
                                for (final DrawableRecipientChip temp : recipients) {
                                    if (temp != null && RecipientEntry.isCreatedRecipient(temp.getEntry().getContactId())
                                            && getSpannable().getSpanStart(temp) != -1) {
                                        if (unfoundAddresses.contains(temp.getEntry().getDestination())) {
                                            replacements.add(createFreeChip(temp.getEntry()));
                                        } else {
                                            replacements.add(null);
                                        }
                                    } else {
                                        replacements.add(null);
                                    }
                                }
                                processReplacements(recipients, replacements);
                            }    
                            printDebugLog(TAG,"[RecipientReplacementTask.doInBackground][matchesNotFound] end"); 
                        }
                    });
            /// M: Restore force scrolling
            setForceEnableBringPointIntoView(false);
            printDebugLog(TAG,"[RecipientReplacementTask.doInBackground] end");
            return null;
        }

        private void processReplacements(final List<DrawableRecipientChip> recipients,
                final List<DrawableRecipientChip> replacements) {
            printDebugLog(TAG,"[RecipientReplacementTask.processReplacements] start");
            if (replacements != null && replacements.size() > 0) {
                final Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        final Editable text = new SpannableStringBuilder(getText());
                        int i = 0;
                        /// M: We don't care about watching text and span changes while in the middle of handling pending chips. @{
                        int totalChips = replacements.size();
                        printDebugLog(TAG,"[RecipientReplacementTask.processReplacements][run] start, recipients count: " + recipients.size()
                                + ", replacements count: " + replacements.size()); 
                        watcherProcessor wp = null;
                        if (totalChips > 0) {
                            wp = new watcherProcessor();
                            wp.initWatcherProcessor();
                            wp.removeSpanWatchers();
                        }
                        /// @}
                        for (final DrawableRecipientChip chip : recipients) {
                            /// M: Add text and span watchers back before handling last chip. @{
                            if (i == (totalChips - 1)) {
                                if(wp != null) {
                                    wp.addSpanWatchers();
                                }
                                /// M: Let text and span watchers work corectly by reset selection and layout. @{
                                setSelection(getText().length());
                                requestLayout();
                                /// @}
                            }
                            /// @}
                            final DrawableRecipientChip replacement = replacements.get(i);
                            if (replacement != null) {
                                final RecipientEntry oldEntry = chip.getEntry();
                                final RecipientEntry newEntry = replacement.getEntry();
                                final boolean isBetter =
                                        RecipientAlternatesAdapter.getBetterRecipient(
                                                oldEntry, newEntry) == newEntry;
                                printDebugLog(TAG,"[run] index: " + i + ", old: " + oldEntry + ", new: " + newEntry + ", isBetter: " + isBetter 
                                        + ", spanStart: " + text.getSpanStart(chip)); 
                                if (isBetter) {
                                    DrawableRecipientChip targetChip = chip;
                                    // Find the location of the chip in the text currently shown.
                                    int start = text.getSpanStart(targetChip);
                                    /// M: The target chip is gone, it may be replaced by a new chip
                                    if (start == -1) {
                                        printDebugLog(TAG,"[run] Can't find the chip to be replaced!!"); 
                                        DrawableRecipientChip[] currentChips = getSortedRecipients();
                                        for (DrawableRecipientChip c : currentChips) {
                                            if (compareEntries(targetChip.getEntry(), c.getEntry())) {
                                                printDebugLog(TAG,"[run] Found the missing chip"); 
                                                targetChip = c;
                                                start = text.getSpanStart(targetChip);
                                                break;
                                            }
                                        }
                                    }
                                    if (start != -1) {
                                        // Replacing the entirety of what the chip represented,
                                        // including the extra space dividing it from other chips.
                                        final int end =
                                                Math.min(text.getSpanEnd(targetChip) + 1, text.length());
                                        text.removeSpan(targetChip);
                                        // Make sure we always have just 1 space at the end to
                                        // separate this chip from the next chip.
                                        String addressText = createAddressText(replacement.getEntry());
                                        if (addressText != null)  {
                                            final SpannableString displayText =
                                                    new SpannableString(addressText.trim() + " ");
                                            displayText.setSpan(replacement, 0,
                                                    displayText.length() - 1,
                                                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                                            // Replace the old text we found with with the new display
                                            // text, which now may also contain the display name of the
                                            // recipient.
                                            /// M: Correct the end index for replacing for some async cases, 
                                            ///      make sure the replace range will not overlap the nexe chip
                                            int toReplace = text.charAt(Math.min(end, text.length() - 1)) == ' ' ? 
                                                    Math.min(end + 1, text.length()) : end;
                                            printDebugLog(TAG,"[run] replace text, start: " + start + ", end: " + end + ", text: " + displayText); 
                                            text.replace(start, toReplace, displayText);
                                            replacement.setOriginalText(displayText.toString());
                                            replacements.set(i, null);
                                            recipients.set(i, replacement);
                                        }
                                    } else if (!hasFocus()) {
                                        /// M: Fix part of chips didn't correctly be replaced to the chip with contact info. @{
                                        /// M: Recipient control is not focused now, add the chip back to mTemporaryRecipients
                                        printDebugLog(TAG,"[run] !hasFocus, add chip back to mTemporaryRecipients");
                                        addTemporaryRecipients(replacement); /// M: replaced with API with log                                        
                                        /// @}
                                    }
                                }
                            } else {
                                printDebugLog(TAG,"[run] index: " + i + ", replacement is null");
                            }
                            i++;
                        }
                        setText(text);
                        /// M: Let text and span watchers work corectly by reset selection and layout. @{
                        if (totalChips > 0) {
                            recoverLayout();
                        }
                        /// @}
                        printDebugLog(TAG,"[RecipientReplacementTask.processReplacements][run] end"); 
                    }
                };

                if (Looper.myLooper() == Looper.getMainLooper()) {
                    printDebugLog(TAG,"[processReplacements] call runnable.run()");
                    runnable.run();
                } else {
                    printDebugLog(TAG,"[processReplacements] post runnable");
                    mHandler.post(runnable);
                }
            }
            printDebugLog(TAG,"[RecipientReplacementTask.processReplacements] end");
        }
        /// M: add to notify add all the chips have been processed , workaround for ALPS01268328  @{
        @Override
        protected void onPostExecute(Void v){
        	if(mChipProcessListener != null){
        		mChipProcessListener.onChipProcessDone();
        	}
        }
        /// @}
    }
    /// M: add to notify add all the chips have been processed , workaround for ALPS01268328  @{
    static public interface ChipProcessListener{
    	void onChipProcessDone();
    }
    
    private ChipProcessListener mChipProcessListener = null;
    
    public void setChipProcessListener(ChipProcessListener l){
    	mChipProcessListener = l;
    }
    /// @}
    private class IndividualReplacementTask extends AsyncTask<ArrayList<DrawableRecipientChip>, Void, Void> {
        @SuppressWarnings("unchecked")
        @Override
        protected Void doInBackground(ArrayList<DrawableRecipientChip>... params) {
            // For each chip in the list, look up the matching contact.
            // If there is a match, replace that chip with the matching
            // chip.
            final ArrayList<DrawableRecipientChip> originalRecipients = params[0];
            printDebugLog(TAG,"[IndividualReplacementTask.doInBackground] start, recipient count: " + originalRecipients.size()); 
            ArrayList<String> addresses = new ArrayList<String>();
            DrawableRecipientChip chip;
            for (int i = 0; i < originalRecipients.size(); i++) {
                chip = originalRecipients.get(i);
                if (chip != null) {
                    addresses.add(createAddressText(chip.getEntry()));
                }
            }
            final BaseRecipientAdapter adapter = (BaseRecipientAdapter) getAdapter();
            if (adapter == null) {
                return null;
            }
            RecipientAlternatesAdapter.getMatchingRecipients(getContext(), adapter, addresses,
                    adapter.getAccount(), new RecipientMatchCallback() {
                    
                        @Override
                        public void matchesFound(Map<String, RecipientEntry> entries) {
                            printDebugLog(TAG,"[IndividualReplacementTask.doInBackground][matchesFound] entries size: " + entries.size()); 
                            for (String des : entries.keySet()) {
                                printDebugLog(TAG,"entry: " + entries.get(des));
                            }
                            int index = 0;
                            for (DrawableRecipientChip temp : originalRecipients) {
                                if (temp != null && RecipientEntry.isCreatedRecipient(temp.getEntry().getContactId())) {
                                    /// M: The first chip may be temporarily removed in replaceChipOnSameTextRange,
                                    ///      so using the mEllipsizedChipLock to wait until the first chip setting done @{
                                    if (getSpannable().getSpanStart(temp) == -1) {
                                        synchronized (mEllipsizedChipLock) {
                                            DrawableRecipientChip[] recipients = getSortedRecipients();
                                            if (recipients != null) {
                                                boolean found = false;
                                                for (int idx = 0; idx < recipients.length && idx < originalRecipients.size(); idx++) {
                                                    if (compareEntries(temp.getEntry(), recipients[idx].getEntry())) {
                                                        temp = recipients[idx];
                                                        found = true;
                                                        printDebugLog(TAG,"[matchesFound] Chip may be replaced due to replaceChipOnSameTextRange()");
                                                        break;
                                                    }
                                                }
                                                if (!found) {
                                                    printDebugLog(TAG,"[matchesFound] [WARNING] Can't find the chip to replace");
                                                    continue;
                                                }
                                            }
                                        }                                        
                                    }
                                    /// @}
                                    // Replace this.
                                    /// M: If destination is a normal phone number (which means without "<",...), use it directly. @{
                                    String destination = temp.getEntry().getDestination().toLowerCase();
                                    RecipientEntry entry = null;
                                    if (!isPhoneNumber(destination)) {
                                        destination = tokenizeAddress(destination);
                                    }
                                    entry = createValidatedEntry(entries.get(destination));
                                    printDebugLog(TAG, "[matchesFound] destination: " + destination + ", entry created: " + entry);
                                    if (isPhoneQuery() && entry == null && destination != null && !destination.contains("@")) {
                                        ///M: Query with normalized number again if there's no fully matched number.
                                        entry = RecipientAlternatesAdapter.getRecipientEntryByPhoneNumber(getContext(), destination);
                                        printDebugLog(TAG, "[matchesFound] entry by phone lookup: " + entry);
                                    }
                                    ///@}
                                    // If we don't have a validated contact match, just use the
                                    // entry as it existed before.
                                    if (entry == null) {  /// M: if there is no result ,replace the invisible chip
                                        entry = temp.getEntry();
                                    }
                                    final RecipientEntry tempEntry = entry;
                                    printDebugLog(TAG, "[matchesFound] post runnable, old: " + temp.getEntry() + ", new: " + tempEntry);
                                    if (tempEntry != null) {
                                        final DrawableRecipientChip chip = temp;
                                        final boolean isFirstChip = (index == 0);
                                        mHandler.post(new Runnable() {
                                            @Override
                                            public void run() {
                                                printDebugLog(TAG,"[IndividualReplacementTask.doInBackground][matchesFound.run] old: " 
                                                        + chip.getEntry() + ", new: " + tempEntry); 
                                                if (mIndividualReplacements != null) {
                                                    replaceChip(chip, tempEntry);
                                                    /// M: If the width of replaced chip is larger than maximum chip space,
                                                    ///      it means the chip must be an ellipsized one with very long name
                                                    ///      and we have to resize it again @{
                                                    if (isFirstChip && mHasEllipsizedFirstChip) {
                                                        int innerWidth = (int) calculateAvailableWidth();
                                                        int moreSpanWidth = getMeasuredMoreSpanWidth(mRemovedSpans.size());
                                                        int chipsSpace = innerWidth - moreSpanWidth;
                                                        DrawableRecipientChip[] allChips = getSortedRecipients();
                                                        if (allChips != null && allChips.length > 0 && getChipWidth(allChips[0]) > chipsSpace) {
                                                            printDebugLog(TAG,"The first chip must be ellipsized again");
                                                            replaceChipOnSameTextRange(allChips[0], chipsSpace);
                                                        }
                                                    }
                                                    /// @}
                                                }
                                            }
                                        });
                                    }
                                }
                                index++;
                            }
                        }

                        @Override
                        public void matchesNotFound(Set<String> unfoundAddresses) {
                            // No acton required
                            printDebugLog(TAG,"[IndividualReplacementTask.doInBackground][matchesNotFound]"); 
                        }
                    });            
            /// M: Restore force scrolling
            setForceEnableBringPointIntoView(false);
            printDebugLog(TAG,"[IndividualReplacementTask.doInBackground] end"); 
            return null;
        }
        /// M: add to notify add all the chips have been processed , workaround for ALPS01268328  @{
        @Override
        protected void onPostExecute(Void v){
        	if(mChipProcessListener != null){
        		mChipProcessListener.onChipProcessDone();
        	}
        }
        /// @}
    }


    /**
     * MoreImageSpan is a simple class created for tracking the existence of a
     * more chip across activity restarts/
     */
    private class MoreImageSpan extends ImageSpan {
        public MoreImageSpan(Drawable b) {
            super(b);
        }
    }

    @Override
    public boolean onDown(MotionEvent e) {
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        // Do nothing.
        return false;
    }

    @Override
    public void onLongPress(MotionEvent event) {
        if (mSelectedChip != null) {
            return;
        }
        float x = event.getX();
        float y = event.getY();
        final int offset = putOffsetInRange(x, y);
        /// M: Fix misrecognize a touch event is located in a chip while it's actually out side of the chip.
        DrawableRecipientChip currentChip = (isTouchPointInChip(x,y)) ? (findChip(offset)) : (null); 
        if (currentChip != null) {
            if (mDragEnabled) {
                // Start drag-and-drop for the selected chip.
                startDrag(currentChip);
            } else {
                // Copy the selected chip email address.
                showCopyDialog(currentChip.getEntry().getDestination());
            }
        }
    }

    // The following methods are used to provide some functionality on older versions of Android
    // These methods were copied out of JB MR2's TextView
    /////////////////////////////////////////////////
    private int supportGetOffsetForPosition(float x, float y) {
        if (getLayout() == null) return -1;
        final int line = supportGetLineAtCoordinate(y);
        final int offset = supportGetOffsetAtCoordinate(line, x);
        return offset;
    }

    private float supportConvertToLocalHorizontalCoordinate(float x) {
        x -= getTotalPaddingLeft();
        // Clamp the position to inside of the view.
        x = Math.max(0.0f, x);
        x = Math.min(getWidth() - getTotalPaddingRight() - 1, x);
        x += getScrollX();
        return x;
    }

    private int supportGetLineAtCoordinate(float y) {
        y -= getTotalPaddingLeft();
        // Clamp the position to inside of the view.
        y = Math.max(0.0f, y);
        y = Math.min(getHeight() - getTotalPaddingBottom() - 1, y);
        y += getScrollY();
        return getLayout().getLineForVertical((int) y);
    }

    private int supportGetOffsetAtCoordinate(int line, float x) {
        x = supportConvertToLocalHorizontalCoordinate(x);
        return getLayout().getOffsetForHorizontal(line, x);
    }
    /////////////////////////////////////////////////

    /**
     * Enables drag-and-drop for chips.
     */
    public void enableDrag() {
        mDragEnabled = true;
    }

    /**
     * Starts drag-and-drop for the selected chip.
     */
    private void startDrag(DrawableRecipientChip currentChip) {
        String address = currentChip.getEntry().getDestination();
        ClipData data = ClipData.newPlainText(address, address + COMMIT_CHAR_COMMA);

        // Start drag mode.
        startDrag(data, new RecipientChipShadow(currentChip), null, 0);

        // Remove the current chip, so drag-and-drop will result in a move.
        // TODO (phamm): consider readd this chip if it's dropped outside a target.
        removeChip(currentChip);
    }

    /**
     * Handles drag event.
     */
    @Override
    public boolean onDragEvent(DragEvent event) {
        switch (event.getAction()) {
            case DragEvent.ACTION_DRAG_STARTED:
                // Only handle plain text drag and drop.
                return event.getClipDescription().hasMimeType(ClipDescription.MIMETYPE_TEXT_PLAIN);
            case DragEvent.ACTION_DRAG_ENTERED:
                requestFocus();
                return true;
            case DragEvent.ACTION_DROP:
                handlePasteClip(event.getClipData());
                return true;
        }
        return false;
    }

    /**
     * Drag shadow for a {@link RecipientChip}.
     */
    private final class RecipientChipShadow extends DragShadowBuilder {
        private final DrawableRecipientChip mChip;

        public RecipientChipShadow(DrawableRecipientChip chip) {
            mChip = chip;
        }

        @Override
        public void onProvideShadowMetrics(Point shadowSize, Point shadowTouchPoint) {
            Rect rect = mChip.getBounds();
            shadowSize.set(rect.width(), rect.height());
            shadowTouchPoint.set(rect.centerX(), rect.centerY());
        }

        @Override
        public void onDrawShadow(Canvas canvas) {
            mChip.draw(canvas);
        }
    }

    private void showCopyDialog(final String address) {
        printDebugLog(TAG,"[showCopyDialog] address: " + address); 
        if (!mAttachedToWindow) {
            return;
        }        
        mCopyAddress = address;
        /// M: Set the title show on single line. @{
        TextView title = (TextView)mCopyDialog.findViewById(android.R.id.title);
        title.setSingleLine(true);
        /// @}
        mCopyDialog.setTitle(address);
        mCopyDialog.setContentView(R.layout.copy_chip_dialog_layout);
        mCopyDialog.setCancelable(true);
        mCopyDialog.setCanceledOnTouchOutside(true);
        Button button = (Button)mCopyDialog.findViewById(android.R.id.button1);
        button.setOnClickListener(this);
        int btnTitleId;
        if (isPhoneQuery()) {
            btnTitleId = R.string.copy_number;
        } else {
            btnTitleId = R.string.copy_email;
        }
        String buttonTitle = getContext().getResources().getString(btnTitleId);
        button.setText(buttonTitle);
        mCopyDialog.setOnDismissListener(this);
        mCopyDialog.show();
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
        // Do nothing.
        return false;
    }

    @Override
    public void onShowPress(MotionEvent e) {
        // Do nothing.
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e) {
        // Do nothing.
        return false;
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        mCopyAddress = null;
    }

    @Override
    public void onClick(View v) {
        // Copy this to the clipboard.
        ClipboardManager clipboard = (ClipboardManager) getContext().getSystemService(
                Context.CLIPBOARD_SERVICE);
        clipboard.setPrimaryClip(ClipData.newPlainText("", mCopyAddress));
        mCopyDialog.dismiss();
    }

    protected boolean isPhoneQuery() {
        return getAdapter() != null
                && ((BaseRecipientAdapter) getAdapter()).getQueryType()
                    == BaseRecipientAdapter.QUERY_TYPE_PHONE;
    }

    /**
     * M: set whether to scroll when adding text
     * @param enable whether to scroll when adding text
     * @hide
     */
    public void setScrollAddText(boolean enable) {
        mEnableScrollAddText = enable;
    }
    
    /**
     * M: whether enable to scroll when adding text
     * @hide
     */
    public boolean isScrollAddText() {
        return mEnableScrollAddText;
    }
    
    /// M: whether enable scroll when adding text.
    private boolean mEnableScrollAddText = true;
    
    /**
     * M: Set whether discard next action up.
     * @param enable true is enable; false is disable.
     * @hide
     */
    protected void setEnableDiscardNextActionUp(boolean enable) {
        mRETVDiscardNextActionUp = enable;
    }
    
    /**
     * M: Get whether discard next action up.
     * @hide
     */
    protected boolean getEnableDiscardNextActionUp() {
        return mRETVDiscardNextActionUp;
    }
    

    /// M: Whether discard next action up.
    private boolean mRETVDiscardNextActionUp =false;

    /// M: When touch after paste, don't care filter popup.
    private boolean bPasted = false;
    private boolean bTouchedAfterPasted = false;


    /// M: Limit width for construct chip span
    private int mLimitedWidthForSpan = -1;
    /// M: Whether first chip has been ellipsized before
    private boolean mHasEllipsizedFirstChip = false;
    
    /**
     * M: Get width of the chip.
     * @param chip 
     * @hide
     */
    private int getChipWidth(DrawableRecipientChip chip){
        int width = chip.getBounds().width();
        /// M: InvisibleRecipientChip does not have width, we have to calculate it now. @{
        if (width == 0) {
            width = calculateUnselectedChipWidth(chip.getEntry());
        }
        /// @}
        return width;
    }

    /**
     * M: Get interval between chips.
     * @hide
     */
    private int getChipInterval() {
        /// M: ChipInterval should be a fixed value no matter what the text size is
        return CHIP_INTERVAL ;
    }

    /**
     * M: Get width of moreSpan approximately.
     * @param count how many chip is in moreChip
     * @hide
     */
    private int getMeasuredMoreSpanWidth(int count){
        String moreText = String.format(mMoreItem.getText().toString(), count);
        TextPaint morePaint = new TextPaint(getPaint());
        morePaint.setTextSize(mMoreItem.getTextSize());
        return (int)morePaint.measureText(moreText) + mMoreItem.getPaddingLeft()+ mMoreItem.getPaddingRight();
    }

    /**
     * M: Replace currentChip from its start position to its end position with same contact but new chip.
     * @param currentChip chip to be replaced
     * @param newChipWidth the width of new chip if need to do replacement
     * @hide
     */
    private final Object mEllipsizedChipLock = new Object();
    private void replaceChipOnSameTextRange(DrawableRecipientChip currentChip, int newChipWidth){
        printDebugLog(TAG,"[replaceChipOnSameTextRange]"); 
        int start = getChipStart(currentChip);
        int end = getChipEnd(currentChip);
        mLimitedWidthForSpan = newChipWidth;
        RecipientEntry entry = currentChip.getEntry();
        DrawableRecipientChip ellipsizeRecipient = constructChipSpan(currentChip.getEntry(), false, 
                                                             TextUtils.isEmpty(entry.getDisplayName()) || 
                                                             TextUtils.equals(entry.getDisplayName(),entry.getDestination()));
        mLimitedWidthForSpan = -1;
        synchronized (mEllipsizedChipLock) {
            getSpannable().removeSpan(currentChip);
            Editable text = getText();
            QwertyKeyListener.markAsReplaced(text, start, end, "");
            printThreadingDebugLog(MTKTAG, "[replaceChipOnSameTextRange] setSpan");
            text.setSpan(ellipsizeRecipient, start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
    }

    /**
     * M: Calculate how many chips can be accommodated in one line (need to take moreChip into consideration).
     */
    private int calculateNumChipsCanShow() {
        DrawableRecipientChip[] recipients = getSortedRecipients();
        if (recipients == null || recipients.length == 0) {
            return 0;
        }

        /// M: no need to add mChipPadding when computing innerWidth
        int innerWidth = (int) calculateAvailableWidth() ;
        int numRecipients = recipients.length;
        int overage = 0;

        /// M: Get chip interval
        int chipInterval = getChipInterval();

        boolean canShowAll = true;
        int occupiedSpace = 0;
        int index=0;
        for (index=0; index < numRecipients; index++) {
            occupiedSpace += getChipWidth(recipients[index]) + chipInterval;
            /// M: mChipPadding must be added back temporarily when checking occupiedSpace
            if (occupiedSpace > innerWidth + (mChipPadding * 2)) {
                canShowAll = false;
                break;
            }
        }

        if (canShowAll) {
            return numRecipients;
        }

        if ((index == numRecipients) && !canShowAll) {
            index--;
        }

        int moreSpanWidth = getMeasuredMoreSpanWidth(numRecipients - index); /// M: For measuring approximate moreSpan width
        int chipsSpace = innerWidth - moreSpanWidth;

        int j=0;
        for (j=index; j>=0; j--) {
            occupiedSpace -= getChipWidth(recipients[j]) + chipInterval;

            if (occupiedSpace < chipsSpace) {
                break;
            }
        }

        if (j==0) {
            if (getChipWidth(recipients[0]) > chipsSpace) {
                /// M: need to ellipsize 1st chip becauese space is not enough, then replace the original one
                replaceChipOnSameTextRange(recipients[0],chipsSpace);
                mHasEllipsizedFirstChip = true;
            }
            return 1;
        } else {
            return j;
        }
    }

    /// M: Whether to bring point into view when manipulating. Default is disabled
    private boolean mMoveCursorToVisible = false;
    private boolean mDisableBringPointIntoView = true;
    private boolean mForceEnableBringPointIntoView = false;
    private int mLineOfSelectedChip = 0;

    /**
     * M: Overide to disable moveCursorToVisibleOffset in certain case.
     */
    @Override
    public boolean moveCursorToVisibleOffset() {
        if (isPhoneQuery() && !mMoveCursorToVisible) {
            mMoveCursorToVisible = true;
            return false;
        } else {
            return super.moveCursorToVisibleOffset();
        }
        
    }

    /**
     * M: Set whether to disable bringPointIntoView.
     * @param disableBringPointIntoView 
     */  
    private void setDisableBringPointIntoView(boolean disableBringPointIntoView) {
        printDebugLog(TAG, "[setDisableBringPointIntoView] " + disableBringPointIntoView);
        mDisableBringPointIntoView = disableBringPointIntoView;
    }

    /**
     * M: Temporary force enable bringPointIntoView
     * @param value
     */  
    private void setForceEnableBringPointIntoView(boolean value) {
        printDebugLog(TAG, "[setForceEnableBringPointIntoView] " + value);
        mForceEnableBringPointIntoView = value;
    }
    
    /**
     * M: Overide to let bringPointIntoView only be triggered in certain cases.
     * @param offset 
     */
    @Override
    public boolean bringPointIntoView(int offset) {
        if (mForceEnableBringPointIntoView) {
            /// M: This case is for during expand or handlePendingChips
            /// force to scroll to botton since and temporary disable the chip touching functionality
            return super.bringPointIntoView(offset);
        } else if (mDisableBringPointIntoView || !isShown()) {
            return false;
        } else {
            return super.bringPointIntoView(offset);
        }
    }

    /**
     * M: Override to reset settings of bringPointIntoView.
     */
    @Override
    public boolean onPreDraw() {
        boolean changed = super.onPreDraw();  
        /// M: After one manipulation, reset it to true.
        setDisableBringPointIntoView(true);  
        return changed;
    }

    /// M: For appending strings in batch processing
    private ArrayList<String> mPendingStrings = new ArrayList<String>();
    private boolean mDuringAppendStrings = false;

    /**
     * M: Append strings in batch processing.
     */
    private void appendPendingStrings() {
        printDebugLog(TAG,"[Debug] appendPendingStrings-start");  
        int pendingStringsCount = (mPendingStrings != null)? (mPendingStrings.size()): 0;
        if (pendingStringsCount <= 0) {
            printDebugLog(TAG,"[Debug] appendPendingStrings-end (null)"); 
            return;
        }
        mDuringAppendStrings = true;
        String str = "";
        for (int x=0; x<pendingStringsCount ; x++) {
            str += mPendingStrings.get(x);
        }
        append(str, 0, str.length());
        mPendingStrings.clear();
        mDuringAppendStrings = false; 
        printDebugLog(TAG,"[Debug] appendPendingStrings-end"); 
    }

    /**
     * M: Manipulate removing and adding span watchers for improving performance.
     */
    private class watcherProcessor{
        private SpanWatcher[] mSpanWatchers;
        private int[] mSpanFlags;
        private int mSpanWatchersNum;

        public watcherProcessor(){
            printDebugLog(TAG,"[watcherProcessor] constructor"); 
            mSpanWatchers = null;    
            mSpanFlags = null;
            mSpanWatchersNum = 0;
        }

        public void initWatcherProcessor(){
            printDebugLog(TAG,"[watcherProcessor.initWatcherProcessor]"); 
            mSpanWatchers = getSpannable().getSpans(0, getText().length(), SpanWatcher.class);
            mSpanWatchersNum = mSpanWatchers.length; 
            mSpanFlags = new int[mSpanWatchersNum];
        }

        public void removeSpanWatchers(){
            printDebugLog(TAG,"[watcherProcessor.removeSpanWatchers]"); 
            for (int x=0; x<mSpanWatchersNum ; x++) {
                tempLogPrint("removeSpanWatchers",mSpanWatchers[x]); 
                mSpanFlags[x] = getSpannable().getSpanFlags(mSpanWatchers[x]);
                if (mSpanWatchers[x] instanceof TextWatcher) {
                    tempLogPrint("removeSpanWatchers, remove - ",mSpanWatchers[x]); 
                    getSpannable().removeSpan(mSpanWatchers[x]);
                } 
            }
        }

        public void addSpanWatchers(){
            printDebugLog(TAG,"[watcherProcessor.addSpanWatchers]"); 
            for (int x=0; x<mSpanWatchersNum ; x++) {
                if (mSpanWatchers[x] instanceof TextWatcher) {
                    printThreadingDebugLog(MTKTAG, "[WatchProcessor.addSpan] setSpan");
                    getSpannable().setSpan(mSpanWatchers[x], 0, getText().length(),mSpanFlags[x]);
                    tempLogPrint("addSpanWatchers, add - ",mSpanWatchers[x]); 
                }
            } 
        }        
    }

    /**
     * M: Get offset from bottom to show drop-down list in proper position.
     * @param line The line in which selected chip located
     */
    private int getOffsetFromBottom(int line) {
        int bottom;
        if (line == getLineCount() -1) {
            bottom = 0;
        } else {
            int offsetFromTop = getPaddingTop() + getLayout().getLineTop(line+1);
            bottom = - (getHeight() - (offsetFromTop - getScrollY()));
        }
        return bottom;
    }

    /**
     * M: Check whether cursor is after last chip or not.
     */
    private boolean isEndChip() {
        CharSequence text = getText();
        int end = getSelectionEnd();
        int i = end - 1;
        char c;
        
        while (i > 0 && ((c = text.charAt(i)) != ',' && c != ';')) {
            --i;
        }

        if ((end - i) <= 2 && i != 0 ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * M: Override to avoid showing unnecessary drop-down list.
     */
    @Override
    public void showDropDown() {
        if (isPhoneQuery()) {
            if (!isEndChip()) {
                super.showDropDown();
            } else {
                dismissDropDown();
            }
        } else {
            super.showDropDown();
        }
    }

    /**
     * M: Get whether the touch point is located in chip.
     * @param posX Touch position in x coordinate
     * @param posY Touch position in y coordinate
     * @hide
     */
    protected boolean isTouchPointInChip(float posX, float posY) {
        boolean isInChip = true;
        Layout layout = getLayout();
        if (layout != null)
        {
            int offsetForPosition = getOffsetForPosition(posX, posY);
            int line = layout.getLineForOffset(offsetForPosition);
            float maxX = layout.getPrimaryHorizontal(layout.getLineEnd(line) - 1);
            float currentX = posX - getTotalPaddingLeft();
            currentX = Math.max(0.0f, currentX);
            currentX = Math.min(getWidth() - getTotalPaddingRight() - 1, currentX);
            currentX += getScrollX();
            if(currentX > maxX)
            {
                isInChip = false;
            }
        }
        return isInChip;
    }

    /**
     * M: Return true if touch point is not in the empty space between two lines.
     * All line space is at the top area. Layout of a chip in one single line:
     *  _______________
     *      line space
     *   ______________
     *  |
     *  |   [CHIP]
     *  |______________
     *  _______________
     * @param posY Touch position in y coordinate
     * @hide
     */
    protected boolean isTouchPointInChipVertical(float posY) {
        Layout layout = getLayout();
        if (layout != null)
        {
            int lineHeight = layout.getLineBottom(0) - layout.getLineTop(0);
            float currentY = posY - getTotalPaddingTop();
            float lineSpace = lineHeight - mChipHeight;
            float y = (currentY + getScrollY()) % lineHeight;
            if (y <= lineSpace) {
                return false;
            }
        }
        return true;
    }
    
    /// M: For dealing with configuration changed
    private OnGlobalLayoutListener mGlobalLayoutListener = null;
    private int mCurrentWidth = 0;

    /**
     * M: When configuration changed, if focused, clear selectedChip and dismiss drop-down list if focused.
     *    When configuration changed, if unfocused and during phone query, update moreChip.
     * @hide
     */
    @Override 
    protected void onConfigurationChanged(Configuration newConfig) {
        printDebugLog(TAG, "[onConfigurationChanged] current view width="+ getWidth() +", height="+ getHeight() +", line count="+ getLineCount());

        if (isPhoneQuery()) {
            registerGlobalLayoutListener();
        }

        if (isFocused()) {
            if (mSelectedChip != null && !shouldShowEditableText(mSelectedChip)) {
                clearSelectedChip();
            }
            dismissDropDown();
        }
    }

    /**
     * M: RegisterGlobalLayoutListener to deal with moreChip when configuration changed.
     */
    private void registerGlobalLayoutListener() {
        ViewTreeObserver viewTreeObs = getViewTreeObserver();
        if( mGlobalLayoutListener == null ){
            mGlobalLayoutListener = new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    printDebugLog(TAG, "[onGlobalLayout] current view width="+ getWidth() +", height="+ getHeight() +", line count="+ getLineCount());

                    do {
                        if (mCurrentWidth == getWidth()) {
                            /// M: Width of view haven't been updated
                            break;
                        }
                        mCurrentWidth = getWidth();
                        if (isFocused()) {
                            /// M: Do not need to relayout while under focused
                            break;
                        }
                        boolean isPortrait = (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT);
                        if (isPortrait) {
                            rotateToPortrait();
                        } else {
                            rotateToLandscape();
                        }
                        requestLayout();
                    } while (false);

                    /// M: Post an action to scroll to bottom on screen rotated
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            printDebugLog(TAG, "[onGlobalLayout][run]");
                            setDisableBringPointIntoView(false);
                            bringPointIntoView(getSelectionStart());
                        }
                    });
                    
                    unRegisterGlobalLayoutListener();
                }
            };
            viewTreeObs.addOnGlobalLayoutListener(mGlobalLayoutListener);
        }
    }

    /**
     * M: UnRegisterGlobalLayoutListener.
     */
    private void unRegisterGlobalLayoutListener() {
        if (mGlobalLayoutListener != null) {
            ViewTreeObserver viewTreeObs = getViewTreeObserver();
            viewTreeObs.removeGlobalOnLayoutListener(mGlobalLayoutListener);
            mGlobalLayoutListener = null;
        }
    }

    /**
     * M: Configuration changed from landscape mode to portrait mode.
     */
    private void rotateToPortrait() {
        printDebugLog(TAG, "[rotateToPortrait] current view width="+ getWidth() +", height="+ getHeight() +", line count="+ getLineCount());
        DrawableRecipientChip[] recipients = getSortedRecipients();
        int numRecipients = recipients.length;

        if (recipients == null || numRecipients== 0) {
            return;
        }

        if (mMoreChip == null) {
            createMoreChip();
            return;
        }

        int innerWidth = (int) calculateAvailableWidth();
        int moreSpanWidth = getMeasuredMoreSpanWidth(mRemovedSpans.size()); /// M: For measuring approximate moreSpan width
        int chipInterval = getChipInterval();
        /// M: The availableSpace calculation should sync with calculateNumChipsCanShow()
        int availableSpace = innerWidth - moreSpanWidth;
        int leftSpace = availableSpace;

        int currIndex = 0;
        /// M: Get how many chips can be accommodated in one line
        for (currIndex = 0; currIndex < numRecipients; currIndex++) {
            leftSpace -= (getChipWidth(recipients[currIndex]) + chipInterval);
            if (leftSpace <= 0) {
                break;
            }
        }

        if (currIndex == numRecipients) {
            if (leftSpace >= 0) {
                /// M: Remain same layout as landscape mode
                return;
            } else {
                currIndex -= 1;
            }
        }

        if (numRecipients == 1) {
            if ((currIndex == 0) && (leftSpace < 0)) {
                /// M: Need to ellipsize 1st chip becauese space is not enough, then replace the original one
                replaceChipOnSameTextRange(recipients[0],availableSpace);
                mHasEllipsizedFirstChip = true;
            }
            return;
        } else {
            if (currIndex == 0) {
                currIndex ++;
                if (leftSpace < 0) {
                    /// M: need to ellipsize 1st chip becauese space is not enough, then replace the original one
                    replaceChipOnSameTextRange(recipients[0],availableSpace);
                    mHasEllipsizedFirstChip = true;
                }
            }

            /// M: Update mMoreChip
            Spannable spannable = getSpannable();
            Editable text = getText();
            int recipientSpanStart = spannable.getSpanStart(recipients[currIndex]);
            int moreSpanEnd = spannable.getSpanEnd(mMoreChip);
            int j = 0;
            for (int i = currIndex; i < numRecipients; i++) {
                mRemovedSpans.add(j++, recipients[i]);
                if (mTemporaryRecipients == null || !mTemporaryRecipients.contains(recipients[i])) {
                    int spanStart = spannable.getSpanStart(recipients[i]);
                    int spanEnd = spannable.getSpanEnd(recipients[i]);
                    recipients[i].setOriginalText(text.toString().substring(spanStart, spanEnd));
                }
                spannable.removeSpan(recipients[i]);
            }
            spannable.removeSpan(mMoreChip);
            MoreImageSpan moreSpan = createMoreSpan(mRemovedSpans.size());
            SpannableString chipText = new SpannableString(text.subSequence(recipientSpanStart, moreSpanEnd));
            chipText.setSpan(moreSpan, 0, chipText.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            printThreadingDebugLog(MTKTAG, "[rotateToPortrait] replace");
            text.replace(recipientSpanStart, moreSpanEnd, chipText);
            mMoreChip = moreSpan;
        }
    }
    /**
     * M: try to convert an invisible chip to visible. If chip is visible ,return the original one.
     */ 
    private DrawableRecipientChip convertToVisibleChip(DrawableRecipientChip chip){
    	if(chip instanceof InvisibleRecipientChip) {
    		RecipientEntry entry = chip.getEntry();
            boolean leaveSpace = TextUtils.isEmpty(entry.getDisplayName())
                    || TextUtils.equals(entry.getDisplayName(),
                            entry.getDestination());
            chip = constructChipSpan(entry, false, leaveSpace);               
    	}
    	return chip;
    }
    /**
     * M: Configuration changed from portrait mode to landscape mode.
     */
    private void rotateToLandscape() {
        printDebugLog(TAG, "[rotateToLandscape] current view width="+ getWidth() +", height="+ getHeight() +", line count="+ getLineCount());
        DrawableRecipientChip[] recipients = getSortedRecipients();
        int numRecipients = recipients.length;

        if (recipients == null || numRecipients == 0) {
            return;
        }

        /// M: Expand first chip
        replaceChipOnSameTextRange(recipients[0], -1);
        mHasEllipsizedFirstChip = false;
        recipients = getSortedRecipients(); /// M: Get newest recipients

        if (mMoreChip == null) {
            return;
        }

        int innerWidth = (int) calculateAvailableWidth();
        int moreSpanWidth = getMeasuredMoreSpanWidth(mRemovedSpans.size()); /// M: For measuring approximate moreSpan width
        int chipInterval = getChipInterval();
        int availableSpace = innerWidth; /// M: Don't minus mMoreChip size now
        int leftSpace = availableSpace;

        if (numRecipients == 1) {
            availableSpace -= (moreSpanWidth + chipInterval);
            if ((availableSpace - getChipWidth(recipients[0])) < 0) {
                replaceChipOnSameTextRange(recipients[0], availableSpace);
                mHasEllipsizedFirstChip = true;
                return;
            }
        }

        int currIndex = 0;
        /// M: Minus all existing chip's width
        for (currIndex = 0; currIndex < numRecipients; currIndex++) {
            leftSpace -= (getChipWidth(recipients[currIndex]) + chipInterval);
            if (leftSpace <= 0) {
                break;
            }
        }

        /// M: Check if left space can accommodate all chips in mRemovedSpans
        int i = 0;
        for (i = 0; i < mRemovedSpans.size(); i++) {
            leftSpace -= (getChipWidth(mRemovedSpans.get(i)) + chipInterval);
            if (leftSpace <= 0) {
                break;
            }
        }

        if (i == mRemovedSpans.size()) {
            if (leftSpace >= 0) {
                /// M: All the chips can be shown
                expand();
                return;
            } else {
                i--;
            }
        }

        /// M: Get how many chips can be accommodated in one line (including mMoreChip)
        leftSpace -= moreSpanWidth;
        int j = 0;
        for (j = i; j >= 0; j--) {
            leftSpace += (getChipWidth(mRemovedSpans.get(j)) + chipInterval);
            if (leftSpace >= 0) {
                break;
            }
        }  

        /// M: Add removedSpan back & remove mMoreChip
        Spannable spannable = getSpannable();
        Editable editable = getText();
        int moreSpanStart = spannable.getSpanStart(mMoreChip);
        int moreSpanEnd = spannable.getSpanEnd(mMoreChip);
        spannable.removeSpan(mMoreChip);
        int end = spannable.getSpanEnd(recipients[numRecipients- 1]);
        int chipStart = 0;;
        int chipEnd = end; /// M: Starts from the end of current last recipientChip (the chip just before mMoreChip)
        String token;
        DrawableRecipientChip chip = null;
        ArrayList<DrawableRecipientChip> newChips = new ArrayList<DrawableRecipientChip>();
        for (int iteration = 0;  iteration < j; iteration++) {
            chip = mRemovedSpans.get(0); /// M: Always get first removedSpan
            // Need to find the location of the chip, again.
            token = (String) chip.getOriginalText();
            // As we find the matching recipient for the remove spans,
            // reduce the size of the string we need to search.
            // That way, if there are duplicates, we always find the correct
            // recipient.
            chipStart = editable.toString().indexOf(token, chipEnd);
            chipEnd = chipStart + token.length();
            // Only set the span if we found a matching token.
            if (chipStart != -1) {
                printThreadingDebugLog(MTKTAG, "[rotateToLandscape] setSpan");
                /// M: there more chips can be shown in landscape mode ,so we convert invisible replacement chip to visible @{
                chip = convertToVisibleChip(chip);
                /// @}
                editable.setSpan(chip, chipStart, chipEnd,
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                newChips.add(chip);
            }
            mRemovedSpans.remove(0); /// M: Always remove first removedSpan
        }

        if (newChips.size() > 0) {
            printDebugLog(TAG, "[rotateToLandscape] execute IndividualReplacementTask, count: " + newChips.size());
            mIndividualReplacements = new IndividualReplacementTask();
            mIndividualReplacements.execute(newChips);
        }

        /// M: Update mMoreChip
        MoreImageSpan moreSpan = createMoreSpan(mRemovedSpans.size());
        SpannableString chipText = new SpannableString(getText().subSequence(chipEnd + 1, moreSpanEnd));
        chipText.setSpan(moreSpan, 0, chipText.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        printThreadingDebugLog(MTKTAG, "[rotateToLandscape] replace");
        getText().replace(chipEnd + 1, moreSpanEnd, chipText);
        mMoreChip = moreSpan;
    }
    
    /**
     * M: Construct chip for long pressed at last normal text.
     * @hide
     */
    public void constructPressedChip() {
        Editable editable = getText();
        setSelection(editable != null && editable.length() > 0 ? editable.length() : 0);
        boolean textIsAllBlank = textIsAllBlank(editable);
        
        int end = getSelectionEnd();
        int start = mTokenizer.findTokenStart(editable, end);
        DrawableRecipientChip[] chips = getSpannable().getSpans(start, end, DrawableRecipientChip.class);
        if ((chips == null || chips.length == 0)) {
            Editable text = getText();
            int whatEnd = mTokenizer.findTokenEnd(text, start);
            if (whatEnd < text.length() && text.charAt(whatEnd) == ',') {
                whatEnd++;
            }
            int selEnd = getSelectionEnd();
            if (whatEnd != selEnd && !textIsAllBlank) {
                handleEdit(start, whatEnd);
            } else {
                commitChip(start, end, editable);
            }
        }
    }
    
    /**
     * M: UpdatePressedChipType for deciding how to update pressed chip.
     * @hide
     */
    public enum UpdatePressedChipType{
        ADD_CONTACT, UPDATE_CONTACT, DELETE_CONTACT
    }

    /**
     * M: Update pressed chip which just added into contact.
     * @param posX Touch position of pressed chip in x coordinate
     * @param posY Touch position of pressed chip in y coordinate
     * @param updateType How to update pressed chip
     * @hide
     */
    public void updatePressedChip(float posX, float posY, UpdatePressedChipType updateType) {
        printDebugLog(TAG, "[updatePressedChip] posX: " + posX + ", posY: " + posY + ", type: " + updateType);
        int offset = putOffsetInRange(getOffsetForPosition(posX, posY));
        final DrawableRecipientChip currentChip = (isTouchPointInChip(posX, posY)) ? (findChip(offset)) : (null);
        ArrayList<String> addresses = new ArrayList<String>();
        if (currentChip == null) {
            printDebugLog(TAG, "[updatePressedChip] Can't find any chip in this position, return");
            return;
        }

        /// M: To collect the chip to be updated @{
        ArrayList<DrawableRecipientChip> chipCandidates = new ArrayList<DrawableRecipientChip>();
        DrawableRecipientChip [] chips = getSortedRecipients();
        long targetID = currentChip.getContactId();
        if (chips != null) {
            for (DrawableRecipientChip chip : chips) {
                if (chip.getContactId() == targetID) {
                    chipCandidates.add(chip);
                    addresses.add(createAddressText(chip.getEntry()));
                }
            }
            printDebugLog(TAG, "[updatePressedChip] chips count: " + chipCandidates.size());
        }
        /// @}

        if (updateType == UpdatePressedChipType.DELETE_CONTACT) {
            /// M: handle the case of deleting contact
            for (int idx = 0; idx < chipCandidates.size(); idx++) {
                DrawableRecipientChip chip = chipCandidates.get(idx);
                RecipientEntry entry = createTokenizedEntry(chip.getValue().toString());
                if (entry != null) {
                    Editable editable = getText();
                    int start = getChipStart(chip);
                    int end = getChipEnd(chip);
                    QwertyKeyListener.markAsReplaced(editable, start, end, "");
                    CharSequence chipText = createChip(entry, false);
                    if (chipText != null && start > -1 && end > -1) {
                        printDebugLog(TAG, "[updatePressedChip] do remove");
                        setChipProcessingMode(end == (editable.length() - 1) ? 
                                PROCESSING_MODE.REMOVE_LAST : PROCESSING_MODE.REMOVE);
                        editable.replace(start, end + 1, chipText);
                        setChipProcessingMode(PROCESSING_MODE.NONE);
                    }
                }
            }
        } else {
            /// M: Query new chip infos & replace the old chips @{
            final UpdatePressedChipType updateTypeFinal = updateType;
            final ArrayList<DrawableRecipientChip> chipCandidatesFinal = chipCandidates;
            final BaseRecipientAdapter adapter = (BaseRecipientAdapter) getAdapter();
            RecipientAlternatesAdapter.getMatchingRecipients(getContext(), adapter, addresses, 
                    adapter.getAccount(), new RecipientMatchCallback() {

                            @Override
                            public void matchesFound(Map<String, RecipientEntry> entries) {
                                for (int idx = 0; idx < chipCandidatesFinal.size(); idx++) {
                                    final DrawableRecipientChip chip = chipCandidatesFinal.get(idx);
                                    if ((RecipientEntry.isCreatedRecipient(chip.getEntry().getContactId()) || (updateTypeFinal == UpdatePressedChipType.UPDATE_CONTACT))
                                            && getSpannable().getSpanStart(chip) != -1) {    
                                        String destination = chip.getEntry().getDestination();
                                        RecipientEntry entry = null;
                                        if (isPhoneNumber(destination)) {
                                            entry = createValidatedEntry(entries.get(destination));
                                        } else {
                                            entry = createValidatedEntry(entries.get(tokenizeAddress(destination.toLowerCase())));
                                        }
                                        if (entry == null && !isPhoneQuery()) {
                                            entry = chip.getEntry();
                                        }
                                        final RecipientEntry tempEntry = entry;
                                        if (tempEntry != null) {
                                            /// M: Update photo cache map
                                            Uri photoThumbnailUri = entry.getPhotoThumbnailUri();
                                            if (photoThumbnailUri != null) {
                                                ((BaseRecipientAdapter) getAdapter()).updatePhotoCacheByUri(photoThumbnailUri);
                                            }
                                            /// @}
                                            mHandler.post(new Runnable() {
                                                @Override
                                                public void run() {
                                                    printDebugLog(TAG, "[updatePressedChip][run] " 
                                                            + (chip == null ? null : chip.getValue()));
                                                    replaceChip(chip, tempEntry);
                                                }
                                            });
                                        }
                                    }
                                }
                            }
                        
                            @Override
                            public void matchesNotFound(Set<String> unfoundAddresses) {
                                // No action required
                            }
                        });
            /// @}  
            /// M: If the control is under shrink state, expand after updating info in case the chip width changed @{
            if (!hasFocus()) {
                mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            requestFocus();
                        }
                    });
            }
            /// @}
        }
    }

    /// M: Set set processing mode
    private void setChipProcessingMode(PROCESSING_MODE mode) {
        printDebugLog(TAG, "[setChipProcessingMode] from: " + mChipProcessingMode + ", to: " + mode);
        mChipProcessingMode = mode;
    }

    /// M: Default size of text
    private float mDefaultTextSize = 0;

    /// M: For indicating RecipientEditTextView is just been expanded, especially used when chips to be parsed is over MAX_CHIPS_PARSED.
    private boolean mJustExpanded = false;

    /**
     * M: Handle pending strings immediately to let getText() get all text including pending strings.
     *    This API is to prevent getting wrong content due to getText is called before handlePendingStrings.
     * @hide
     */
    public Editable handleAndGetText() {
        appendPendingStrings();
        return getText();
    }

    private void tempLogPrint(String logTitle, Object obj) {
        int spanStart = getSpannable().getSpanStart(obj);
        int spanEnd = getSpannable().getSpanEnd(obj);
        int spanFlag = getSpannable().getSpanFlags(obj);
        String spanName = obj.getClass().getName();
        int spanID = obj.hashCode();
        Log.d(TAG, "[Debug] "+logTitle+ " ---> spanStart=" + spanStart + ", spanEnd="+ spanEnd + ", spanFlag=" + spanFlag + 
                   ", spanID=" + spanID + ", spanName=" + spanName);            
    }
    
    /// M: To indicate it's during accelrattion of removing chip, preventing unnecessary afterTextChanged action.
    private boolean mDuringAccelerateRemoveChip = false;

    /*
     * M: Let text and span watchers work corectly by reset selection and layout.
     */
    private void recoverLayout() {
        printDebugLog(TAG,"[recoverLayout]");  
        if (mIsAutoTesting) {
            return;
        }
        
        setSelection(getText().length());
        int hintWant = getLayout() == null ? 0 : getLayout().getWidth();
        makeNewLayout(hintWant, hintWant, new BoringLayout.Metrics(), new BoringLayout.Metrics(),
                      getWidth() - getCompoundPaddingLeft() - getCompoundPaddingRight(), false);
        requestLayout();
        invalidate();
    }

    private class RecipientChipProcessor{
        private ArrayList<DrawableRecipientChip> mChips;
        private ArrayList<Integer> mSpanStart;
        private ArrayList<Integer> mSpanEnd;
        private ArrayList<Integer> mSpanFlags;

        public RecipientChipProcessor() {
            mChips = new ArrayList<DrawableRecipientChip>();
            mSpanStart = new ArrayList<Integer>();
            mSpanEnd = new ArrayList<Integer>();
            mSpanFlags = new ArrayList<Integer>();
        }

        public void removeChipsWithoutNotification(int startIndex, int endIndex) {
            DrawableRecipientChip[] chips = getSortedRecipients();
            if ((chips == null || chips.length == 0)) {
                return;
            }
            Log.e(TAG, "[removeChipsWithoutNotification] startIndex:" + startIndex + ", endIndex:" + endIndex);
            /// M: Remove watchers
            watcherProcessor wp = null;
            wp = new watcherProcessor();
            wp.initWatcherProcessor();
            wp.removeSpanWatchers();
            /// M: Remove chip spans
            for (int x = startIndex; x < endIndex; x++) {
                mChips.add(chips[x]);
                mSpanStart.add(getChipStart(chips[x]));
                mSpanEnd.add(getChipEnd(chips[x]));
                mSpanFlags.add(getSpannable().getSpanFlags(chips[x]));
                getSpannable().removeSpan(chips[x]);
            }
            /// M: Add watchers back
            if(wp != null) {
                wp.addSpanWatchers();
            }
            recoverLayout();
        }

        public void addChipsBackWithoutNotification(int offset) {
            if (mChips == null || mChips.size() == 0) {
                return;
            }
            printDebugLog(TAG, "[addChipsBackWithoutNotification]");
            /// M: Remove watchers
            watcherProcessor wp = null;
            wp = new watcherProcessor();
            wp.initWatcherProcessor();
            wp.removeSpanWatchers();
            /// M: Add chip spans back
            int textLen = getText().length();
            int chipsLen = mChips.size();
            for (int x=0; x< chipsLen; x++) {
                if (x == (chipsLen -1)) {
                    /// M: Add watchers back
                    if(wp != null) {
                        wp.addSpanWatchers();
                    }
                    /// M: Let text and span watchers work corectly by reset selection and layout. @{
                    setSelection(textLen);
                    requestLayout();
                    /// @}
                }
                int chipStart = mSpanStart.get(x) - offset;
                int chipEnd = mSpanEnd.get(x) - offset;
                if (chipStart < 0 || chipEnd > textLen) {
                    printDebugLog(TAG,"[addChipsBackWithoutNotification] New position of chip is wrong while resetting span back.");
                    continue;
                }
                if (!alreadyHasChip(chipStart, chipEnd)) {
                    printThreadingDebugLog(MTKTAG, "[RecipChipProcessor.addChipsBack] setSpan");
                    getSpannable().setSpan(mChips.get(x), chipStart, chipEnd, mSpanFlags.get(x));
                }
            }
            recoverLayout();
        }
    }

    /// M: Default vertical offset of AutoCompleteTextView which is used for adjusting position of popup window
    private int mDefaultVerticalOffset = 0;

    /// M: Text to be restore later instead of restoring at onRestoreInstanceState phase
    private String mStringToBeRestore = null;

    /// M: For replacing duplcate contact
    private ArrayList<DrawableRecipientChip> mPedingReplaceChips;
    private ArrayList<RecipientEntry> mPedingReplaceEntries;
    private boolean mDuringReplaceDupChips = false;

    private class DuplicateContactReplacementTask extends AsyncTask<Object, Void, Void> {
        @Override
        protected Void doInBackground(Object... params) {
            printDebugLog(TAG,"[DuplicateContactReplacementTask] start.");
            final DrawableRecipientChip chip = (DrawableRecipientChip) params[0];
            final RecipientEntry currEntry = chip.getEntry();
            final String address = currEntry.getDestination();
            ArrayList<String> addresses = new ArrayList<String>();
            addresses.add(address);
            
            final BaseRecipientAdapter adapter = (BaseRecipientAdapter) getAdapter();
            RecipientAlternatesAdapter.getMatchingRecipients(getContext(), adapter, addresses, 
                adapter.getAccount(), new RecipientMatchCallback() {
                            @Override
                            public void matchesFound(Map<String, RecipientEntry> entries) {
                                RecipientEntry entry = null;
                                entry = entries.get(address);
                                final RecipientEntry newEntry = entry;
                                if ((newEntry != null) && (newEntry.getContactId() != currEntry.getContactId())) {
                                    printDebugLog(TAG,"[DuplicateContactReplacement] Post handleReplaceDuplicateChip.");
                                    mHandler.post(new Runnable() {
                                            @Override
                                            public void run() {
                                                handleReplaceDuplicateChip(chip, newEntry);
                                            }
                                        });
                                }
                            }

                            @Override
                            public void matchesNotFound(Set<String> unfoundAddresses) {
                                // No action required
                            }
                        });
            
            printDebugLog(TAG,"[DuplicateContactReplacementTask] end.");
            return null;
        }
    }

    private void handleReplaceDuplicateChip(DrawableRecipientChip chip, RecipientEntry newEntry){
        if (hasFocus() || (mRemovedSpans == null) || (mRemovedSpans.size() == 0)) {
            replaceChip(chip, newEntry);
            printDebugLog(TAG,"[DuplicateContactReplacement] Replace contact from " + chip.getEntry().getContactId() + " to " + newEntry.getContactId());
        } else {
            mPedingReplaceChips.add(chip);
            mPedingReplaceEntries.add(newEntry);
            printDebugLog(TAG,"[DuplicateContactReplacement] Replace contact later.");
        }
    }

    /// M: MTK debug log for detecting race condition
    private void printThreadingDebugLog(String logTag, String logContent) {
        if (DEBUG_THREADING_LOG) {
            Log.d(logTag, logContent);
        }
    }

    /// M: MTK normal debug log
    private void printDebugLog(String logTag, String logContent) {
        if (DEBUG_LOG) {
            Log.d(logTag, logContent);
        }
    }

    /// M: To indicate we are in the auto testing
    private boolean mIsAutoTesting = false;
    protected void setIsAutoTesting() {
        mIsAutoTesting = true;
    }

    protected boolean getIsAutoTesting() {
        return mIsAutoTesting;
    }
}
