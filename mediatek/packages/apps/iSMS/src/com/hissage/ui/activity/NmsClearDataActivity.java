package com.hissage.ui.activity;

import com.hissage.R;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

public class NmsClearDataActivity extends Activity {

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Toast.makeText(this, R.string.STR_NMS_CLEAR_USER_DATA, Toast.LENGTH_SHORT).show();
        finish();
    }
}
