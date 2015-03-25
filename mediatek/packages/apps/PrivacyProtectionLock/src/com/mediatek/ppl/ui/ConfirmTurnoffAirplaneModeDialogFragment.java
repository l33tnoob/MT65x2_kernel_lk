package com.mediatek.ppl.ui;

import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;

import com.mediatek.ppl.R;

/**
 * Dialog fragment to ask user to turn off airplane mode.
 */
public class ConfirmTurnoffAirplaneModeDialogFragment extends DialogFragment {
    /**
     * Interface to commit the action of turning off airplane mode.
     */
    public static interface ITurnOffAirplaneMode {
        void turnoffAirplaneMode();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Builder builder = new Builder(getActivity());
        builder.setMessage(R.string.description_turnoff_airplane_mode_confirm);
        builder.setNegativeButton(android.R.string.cancel, null);
        builder.setPositiveButton(android.R.string.ok, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                ITurnOffAirplaneMode activity =
                        (ITurnOffAirplaneMode) ConfirmTurnoffAirplaneModeDialogFragment.this.getActivity();
                activity.turnoffAirplaneMode();
            }
        });
        return builder.create();
    }

}
