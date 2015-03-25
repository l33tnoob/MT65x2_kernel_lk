package com.mediatek.common.widget.tests;

import android.app.Activity;
import android.app.DatePickerDialog;
import android.os.Bundle;
import android.widget.DatePicker;

public class DatePickerDialogActivity extends Activity implements DatePickerDialog.OnDateSetListener{
	
    public DatePickerDialog mDatePickerDialog;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        mDatePickerDialog = new DatePickerDialog(DatePickerDialogActivity.this, this, 2012, 12, 21);
        mDatePickerDialog.show();
    }
	
    public DatePickerDialog getDialog() {
        return mDatePickerDialog;
    }
	
    @Override
    public void onDateSet(DatePicker view, int year, int monthOfYear,
        int dayOfMonth) {
        // TODO Auto-generated method stub
        // Do nothing
    }
}
