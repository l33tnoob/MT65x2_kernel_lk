package com.mediatek.ppl.ui;

import android.content.Intent;

/**
 * Container of activity result. This activity need to apply the result after PplService is connected.
 */
public class PendingActivityResult {
    public int requestCode;
    public int resultCode;
    public Intent data;
    
    public PendingActivityResult(int requestCode, int resultCode, Intent data) {
        this.requestCode = requestCode;
        this.resultCode = resultCode;
        this.data = data;
    }
}