package com.mediatek.ppl.ui;

import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;

import com.mediatek.ppl.R;

/**
 * Dedicated dialog fragment for ControlPanelActivity to ask user to confirm the disable action.
 */
public class ConfirmDisableDialogFragment extends DialogFragment {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Builder builder = new Builder(getActivity());
        builder.setMessage(R.string.description_disable_confirm);
        builder.setNegativeButton(android.R.string.cancel, null);
        builder.setPositiveButton(android.R.string.ok, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                ControlPanelActivity activity = (ControlPanelActivity) ConfirmDisableDialogFragment.this.getActivity();
                activity.executeDisable();
            }
        });
        return builder.create();
    }

}
