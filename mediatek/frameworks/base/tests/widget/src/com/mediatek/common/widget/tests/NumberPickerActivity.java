package com.mediatek.common.widget.tests;

import android.os.Bundle;
import android.app.Activity;
import android.widget.NumberPicker;

public class NumberPickerActivity extends Activity {

    NumberPicker mNumberPicker;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.numberpicker_layout);
        mNumberPicker = (NumberPicker)findViewById(R.id.testNumberPicker);
        mNumberPicker.setMaxValue(20);
        mNumberPicker.setMinValue(0);
        mNumberPicker.setValue(1);
    }
}
