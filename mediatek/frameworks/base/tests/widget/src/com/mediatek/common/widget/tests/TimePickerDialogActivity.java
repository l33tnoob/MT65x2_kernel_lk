package com.mediatek.common.widget.tests;

import android.app.Activity;
import android.app.TimePickerDialog;
import android.os.Bundle;
import android.widget.TimePicker;

public class TimePickerDialogActivity extends Activity implements TimePickerDialog.OnTimeSetListener{
	
    public TimePickerDialog mTimePickerDialog;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        mTimePickerDialog = new TimePickerDialog(TimePickerDialogActivity.this, this, 12, 12, false);
        mTimePickerDialog.show();
    }
	
    public TimePickerDialog getDialog() {
        return mTimePickerDialog;
    }
	
    @Override
    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
        // TODO Auto-generated method stub
        // Do nothing
    }
}
