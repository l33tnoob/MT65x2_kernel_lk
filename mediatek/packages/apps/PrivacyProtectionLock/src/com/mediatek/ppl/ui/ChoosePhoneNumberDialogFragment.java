package com.mediatek.ppl.ui;

import android.app.Activity;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;

import com.mediatek.ppl.R;

/**
 * Dedicated dialog fragment for SetupTrustedContactActivity and UpdateTrustedContactsActivity. It prompts a number
 * list to user if the contact user selected has multiple phone numbers.
 */
public class ChoosePhoneNumberDialogFragment extends DialogFragment {
    public static final String ARG_KEY_ITEMS = "items";
    public static final String ARG_KEY_NAME = "name";
    public static final String ARG_KEY_LINE_INDEX = "index";

    /**
     * Interface to send the user selection back.
     */
    public static interface IUpdateNumber {
        /**
         * Update the number and name of specified contact line.
         * 
         * @param number    New number.
         * @param name      New name. May be null.
         * @param index     Index to the contact line.
         */
        void changeNumber(String number, String name, int index);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Activity activity = getActivity();
        Bundle args = getArguments();
        final String[] items = args.getStringArray(ARG_KEY_ITEMS);
        final String name = args.getString(ARG_KEY_NAME);
        final int index = args.getInt(ARG_KEY_LINE_INDEX);
        Builder builder = new Builder(activity);
        builder.setTitle(R.string.title_choose_number);
        builder.setNegativeButton(android.R.string.cancel, null);
        builder.setItems(items, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                ((IUpdateNumber) activity).changeNumber(items[which], name, index);
            }
        });

        return builder.create();
    }

}
