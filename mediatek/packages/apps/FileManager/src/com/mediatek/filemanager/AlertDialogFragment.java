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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.filemanager;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.os.Bundle;
import android.os.Vibrator;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextWatcher;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;

import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.ToastHelper;

import java.io.UnsupportedEncodingException;

public class AlertDialogFragment extends DialogFragment implements
        OnClickListener {
    public static final String TAG = "AlertDialogFragment";

    private static final String TITLE = "title";
    private static final String CANCELABLE = "cancelable";
    private static final String ICON = "icon";
    private static final String MESSAGE = "message";
    private static final String LAYOUT = "layout";
    private static final String NEGATIVE_TITLE = "negativeTitle";
    private static final String POSITIVE_TITLE = "positiveTitle";

    public static final int INVIND_RES_ID = -1;

    protected OnClickListener mDoneListener;
    protected OnDismissListener mDismissListener = null;
    protected ToastHelper mToastHelper = null;
    private OnDialogDismissListener mDialogDismissListener;

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putAll(getArguments());
        super.onSaveInstanceState(outState);
    }

    public static class AlertDialogFragmentBuilder {
        protected final Bundle mBundle = new Bundle();

        /**
         * This method creates AlertDialogFragment with parameter of mBundle.
         * 
         * @return AlertDialogFragment
         */
        public AlertDialogFragment create() {
            AlertDialogFragment f = new AlertDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        /**
         * This method sets TITLE for AlertDialogFragmentBuilder, which responds to title of dialog.
         * 
         * @param resId resource id of title
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setTitle(int resId) {
            mBundle.putInt(TITLE, resId);
            return this;
        }

        /**
         * This method sets LAYOUT for AlertDialogFragmentBuilder, which responds to layout of
         * dialog.
         * 
         * @param resId resource id of layout
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setLayout(int resId) {
            mBundle.putInt(LAYOUT, resId);
            return this;
        }

        /**
         * This method sets CANCELABLE for AlertDialogFragmentBuilder (default value is true), which
         * responds to weather dialog can be canceled.
         * 
         * @param cancelable true for can be canceled, and false for can not be canceled
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setCancelable(boolean cancelable) {
            mBundle.putBoolean(CANCELABLE, cancelable);
            return this;
        }

        /**
         * This method sets ICON for AlertDialogFragmentBuilder.
         * 
         * @param resId resource id of icon
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setIcon(int resId) {
            mBundle.putInt(ICON, resId);
            return this;
        }

        /**
         * This method sets MESSAGE for AlertDialogFragmentBuilder, which is a string.
         * 
         * @param resId resource id of message
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setMessage(int resId) {
            mBundle.putInt(MESSAGE, resId);
            return this;
        }

        /**
         * This method sets NEGATIVE_TITLE for AlertDialogFragmentBuilder, which responds to title
         * of negative button.
         * 
         * @param resId resource id of title
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setCancelTitle(int resId) {
            mBundle.putInt(NEGATIVE_TITLE, resId);
            return this;
        }

        /**
         * This method sets POSITIVE_TITLE for AlertDialogFragmentBuilder, which responds to title
         * of positive button.
         * 
         * @param resId resource id of title
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setDoneTitle(int resId) {
            mBundle.putInt(POSITIVE_TITLE, resId);
            return this;
        }
    }

    /**
     * This method sets doneListenser for AlertDialogFragment
     * 
     * @param listener doneListenser for AlertDialogFragment, which will response to press done
     *            button
     */
    public void setOnDoneListener(OnClickListener listener) {
        mDoneListener = listener;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (mDoneListener != null) {
            mDoneListener.onClick(dialog, which);
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);
        return builder.create();
    }

    /**
     * This method gets a instance of AlertDialog.Builder
     * 
     * @param savedInstanceState information for AlertDialog.Builder
     * @return
     */
    protected Builder createAlertDialogBuilder(Bundle savedInstanceState) {
        Bundle args = null;
        if (savedInstanceState == null) {
            args = getArguments();
        } else {
            args = savedInstanceState;
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        if (args != null) {
            int title = args.getInt(TITLE, INVIND_RES_ID);
            if (title != INVIND_RES_ID) {
                builder.setTitle(title);
            }

            int icon = args.getInt(ICON, INVIND_RES_ID);
            if (icon != INVIND_RES_ID) {
                builder.setIcon(icon);
            }

            int message = args.getInt(MESSAGE, INVIND_RES_ID);
            int layout = args.getInt(LAYOUT, INVIND_RES_ID);
            if (layout != INVIND_RES_ID) {
                View view = getActivity().getLayoutInflater().inflate(layout,
                        null);
                builder.setView(view);
            } else if (message != INVIND_RES_ID) {
                builder.setMessage(message);
            }

            int cancel = args.getInt(NEGATIVE_TITLE, INVIND_RES_ID);

            if (cancel != INVIND_RES_ID) {
                builder.setNegativeButton(cancel, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });
            }

            int done = args.getInt(POSITIVE_TITLE, INVIND_RES_ID);
            if (done != INVIND_RES_ID) {
                builder.setPositiveButton(done, this);
            }

            mToastHelper = new ToastHelper(getActivity());
            boolean cancelable = args.getBoolean(CANCELABLE, true);
            builder.setCancelable(cancelable);
        }
        return builder;
    }

    /**
     * This method sets dismissListener for AlertDialogFragment, which will response to
     * dismissDialog
     * 
     * @param listener OnDismissListener for AlertDialogFragment
     */
    public void setDismissListener(OnDismissListener listener) {
        mDismissListener = listener;
    }

    /**
     * This method sets dismissListener for AlertDialogFragment, which will
     * response to dismissDialog
     *
     * @param listener
     *            OnDismissListener for AlertDialogFragment
     */
    public void setOnDialogDismissListener(OnDialogDismissListener listener) {
        mDialogDismissListener = listener;
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        if (mDismissListener != null) {
            mDismissListener.onDismiss(dialog);
        }
        if (mDialogDismissListener != null) {
            mDialogDismissListener.onDialogDismiss();
        }
        super.onDismiss(dialog);
    }

    public static class EditDialogFragmentBuilder extends
            AlertDialogFragmentBuilder {
        @Override
        public EditTextDialogFragment create() {
            EditTextDialogFragment f = new EditTextDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        /**
         * This method sets default string and default selection for EditTextDialogFragment.
         * 
         * @param defaultString default string to show on EditTextDialogFragment
         * @param defaultSelection resource id for default selection
         * @return EditDialogFragmentBuilder
         */
        public EditDialogFragmentBuilder setDefault(String defaultString,
                int defaultSelection) {
            mBundle.putString(EditTextDialogFragment.DEFAULT_STRING,
                    defaultString);
            mBundle.putInt(EditTextDialogFragment.DEFAULT_SELCTION,
                    defaultSelection);
            return this;
        }
    }

    public static class EditTextDialogFragment extends AlertDialogFragment {
        public static final String TAG = "EditTextDialogFragment";
        public static final String DEFAULT_STRING = "defaultString";
        public static final String DEFAULT_SELCTION = "defaultSelection";
        private EditText mEditText;
        private EditTextDoneListener mEditTextDoneListener;

        public interface EditTextDoneListener {
            /**
             * This method is used to overwrite by its implement
             * 
             * @param text text on EditText when done button is pressed
             */
            void onClick(String text);
        }

        @Override
        public void onSaveInstanceState(Bundle outState) {
            getArguments().putString(DEFAULT_STRING,
                    mEditText.getText().toString());
            getArguments().putInt(DEFAULT_SELCTION,
                    mEditText.getSelectionStart());
            super.onSaveInstanceState(outState);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            this.setOnDoneListener(this);
            AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);
            Bundle args = null;
            if (savedInstanceState == null) {
                args = getArguments();
            } else {
                args = savedInstanceState;
            }
            if (args != null) {
                String defaultString = args.getString(DEFAULT_STRING, "");
                int selction = args.getInt(DEFAULT_SELCTION, 0);
                View view = getActivity().getLayoutInflater().inflate(
                        R.layout.dialog_edit_text, null);
                builder.setView(view);
                mEditText = (EditText) view.findViewById(R.id.edit_text);
                if (mEditText != null) {
                    mEditText.setText(defaultString);
                    mEditText.setSelection(selction);
                }
            }
            return builder.create();
        }

        @Override
        public void onResume() {
            super.onResume();
            if (mEditText != null && mEditText.getText().length() == 0) {
                final Button button = ((AlertDialog) getDialog())
                        .getButton(DialogInterface.BUTTON_POSITIVE);
                if (button != null) {
                    button.setEnabled(false);
                }
            }
            getDialog().getWindow().setSoftInputMode(
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
            setTextChangedCallback(mEditText, (AlertDialog) getDialog());
        }

        /**
         * This method is used to set filter to EditText which is used for user entering filename.
         * This filter will ensure that the inputed filename wouldn't be too long. If so, the
         * inputed info would be rejected.
         * 
         * @param edit The EditText for filter to be registered.
         * @param maxLength limitation of length for input text
         */
        private void setEditTextFilter(final EditText edit, final int maxLength) {
            InputFilter filter = new InputFilter.LengthFilter(maxLength) {
                boolean mHasToasted = false;
                private static final int VIBRATOR_TIME = 100;

                public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                        int dstart, int dend) {
                    String oldText = null;
                    String newText = null;
                    int oldSize = 0;
                    int newSize = 0;
                    if (mEditText != null) {
                        oldText = mEditText.getText().toString();
                        //oldSize = oldText.length();
                        try {
                            oldSize = oldText.getBytes("UTF-8").length;
                            LogUtils.d(TAG, "filter,oldSize=" + oldSize + ",oldText=" + oldText);
                        } catch (UnsupportedEncodingException e) {
                            e.printStackTrace();
                            oldSize = oldText.length();
                        }
                    }
                    if (source != null) {
                        newText = source.toString();
                       // newSize = newText.length();
                        try {
                            newSize = newText.getBytes("UTF-8").length;
                            LogUtils.d(TAG, "filter,newSize=" + newSize + ",newText =" + newText);
                        } catch (UnsupportedEncodingException e) {
                            e.printStackTrace();
                            newSize = newText.length();
                        }
                    }
                    
                    if (source != null && source.length() > 0 && (oldSize + newSize) > maxLength) {
                        LogUtils.d(TAG, "oldSize + newSize) > maxLength,source.length()="
                                + source.length());
                        Vibrator vibrator = (Vibrator) getActivity().getSystemService(
                                Context.VIBRATOR_SERVICE);
                        boolean hasVibrator = vibrator.hasVibrator();
                        if (hasVibrator) {
                            vibrator.vibrate(new long[] { VIBRATOR_TIME, VIBRATOR_TIME },
                                    INVIND_RES_ID);
                        }
                        LogUtils.w(TAG, "input out of range,hasVibrator:" + hasVibrator);
                        return "";
                    }
                    if (source != null && source.length() > 0 && !mHasToasted
                            && dstart == 0) {
                        if (source.charAt(0) == '.') {
                            mToastHelper.showToast(R.string.create_hidden_file);
                            mHasToasted = true;
                        }
                    }
                    return super.filter(source, start, end, dest, dstart, dend);
                }
            };
            edit.setFilters(new InputFilter[] { filter });
        }

        /**
         * This method register callback and set filter to Edit, in order to make sure that user
         * input is legal. The input can't be illegal filename and can't be too long.
         * 
         * @param editText EditText, which user type on
         * @param dialog dialog, which EditText associated with
         */
        protected void setTextChangedCallback(EditText editText,
                final AlertDialog dialog) {
            setEditTextFilter(editText, FileInfo.FILENAME_MAX_LENGTH);
            editText.addTextChangedListener(new TextWatcher() {

                @Override
                public void afterTextChanged(Editable arg0) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start,
                        int count, int after) {
                }

                @Override
                public void onTextChanged(CharSequence s, int start,
                        int before, int count) {
                    if (s.toString().length() <= 0
                            || s.toString().matches(".*[/\\\\:*?\"<>|\t].*")) {
                        // characters not allowed
                        if (s.toString().matches(".*[/\\\\:*?\"<>|\t].*")) {
                            mToastHelper
                                    .showToast(R.string.invalid_char_prompt);
                        }
                        Button botton = dialog
                                .getButton(DialogInterface.BUTTON_POSITIVE);
                        if (botton != null) {
                            botton.setEnabled(false);
                        }
                    } else {
                        Button botton = dialog
                                .getButton(DialogInterface.BUTTON_POSITIVE);
                        if (botton != null) {
                            botton.setEnabled(true);
                        }
                    }
                }
            });
        }

        /**
         * This method gets EditText's content on EditTextDialogFragment
         * 
         * @return content of EditText
         */
        public String getText() {
            if (mEditText != null) {
                return mEditText.getText().toString().trim();
            }
            return null;
        }

        /**
         * This method sets EditTextDoneListener for EditTextDialogFragment
         * 
         * @param listener EditTextDoneListener, which will response press done button
         */
        public void setOnEditTextDoneListener(EditTextDoneListener listener) {
            mEditTextDoneListener = listener;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (mEditTextDoneListener != null) {
                mEditTextDoneListener.onClick(getText());
            }
        }
    }

    public static class ChoiceDialogFragmentBuilder extends
            AlertDialogFragmentBuilder {
        @Override
        public ChoiceDialogFragment create() {
            ChoiceDialogFragment f = new ChoiceDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        /**
         * This method sets default choice and array for ChoiceDialogFragment.
         * 
         * @param arrayId resource id for array
         * @param defaultChoice resource id for default choice
         * @return ChoiceDialogFragmentBuilder
         */
        public ChoiceDialogFragmentBuilder setDefault(int arrayId,
                int defaultChoice) {
            mBundle.putInt(ChoiceDialogFragment.DEFAULT_CHOICE, defaultChoice);
            mBundle.putInt(ChoiceDialogFragment.ARRAY_ID, arrayId);
            return this;
        }
    }

    public static class ChoiceDialogFragment extends AlertDialogFragment {
        public static final String CHOICE_DIALOG_TAG = "ChoiceDialogFragment";
        public static final String DEFAULT_CHOICE = "defaultChoice";
        public static final String ARRAY_ID = "arrayId";
        public static final String ITEM_LISTENER = "itemlistener";
        private int mArrayId;
        private int mDefaultChoice;
        private OnClickListener mItemLinster = null;

        /**
         * This method sets clickListener for ChoiceDialogFragment
         * 
         * @param listener onClickListener, which will response press cancel button
         */
        public void setItemClickListener(OnClickListener listener) {
            mItemLinster = listener;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            LogUtils.d(CHOICE_DIALOG_TAG, "Show alertSortDialog");
            AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);

            Bundle args = null;
            if (savedInstanceState == null) {
                args = getArguments();
            } else {
                args = savedInstanceState;
            }
            if (args != null) {
                mDefaultChoice = args.getInt(DEFAULT_CHOICE);
                mArrayId = args.getInt(ARRAY_ID);
            }
            builder.setSingleChoiceItems(mArrayId, mDefaultChoice, this);
            return builder.create();
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (mItemLinster != null) {
                mItemLinster.onClick(dialog, which);
            }
        }
    }

    public interface OnDialogDismissListener {
        void onDialogDismiss();
    }
}
