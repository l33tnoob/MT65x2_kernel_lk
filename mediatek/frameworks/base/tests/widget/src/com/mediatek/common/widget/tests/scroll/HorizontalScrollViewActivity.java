package com.mediatek.common.widget.tests.scroll;


import android.app.Activity;
import android.os.Bundle;
import android.widget.HorizontalScrollView;
import android.view.View;

import com.mediatek.common.widget.tests.R;

public class HorizontalScrollViewActivity extends Activity {
    
    private HorizontalScrollView mHorizontalScrollView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.horizontal_scroll_view);
        
        mHorizontalScrollView = (HorizontalScrollView) findViewById(R.id.hsv);
        
    }
    
    public HorizontalScrollView getHorizontalScrollView() {
        return mHorizontalScrollView;
    }
    
    public View getButton4() {
        return findViewById(R.id.button4);
    }
}
