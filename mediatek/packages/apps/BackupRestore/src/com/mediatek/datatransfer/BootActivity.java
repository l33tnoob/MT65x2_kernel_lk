package com.mediatek.datatransfer;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle; 

public class BootActivity extends Activity {
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = new Intent(this, MainActivity.class);
        this.startActivity(intent);
        this.finish();
    }
}
