package com.mediatek.common.widget.tests;

import android.annotation.TargetApi;
import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.StackView;

import android.content.Context;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;

@TargetApi(11)
public class StackViewActivity extends Activity {
    
    private StackView mStackView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.stack_view);

        mStackView = (StackView) findViewById(R.id.stackView);

        ColorAdapter colorAdapter = new ColorAdapter(this, mColors);
        mStackView.setAdapter(colorAdapter);

        final Button previousButon = (Button) findViewById(R.id.previousButton);
        previousButon.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                mStackView.showPrevious();
            }
        });

        final Button nextButton = (Button) findViewById(R.id.nextButton);
        nextButton.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                mStackView.showNext();
            }
        });
    }
    
    public StackView getStackView() {
        return mStackView;
    }    

    private int[] mColors = {
            Color.BLUE, Color.CYAN, Color.GRAY, Color.GREEN, Color.RED
    };

    private class ColorAdapter extends BaseAdapter {

        private Context mContext;

        private int[] mColors;

        public ColorAdapter(Context context, int[] colors) {
            mContext = context;
            mColors = colors;
        }

        public int getCount() {
            return mColors == null ? 0 : mColors.length;
        }

        public Object getItem(int position) {
            return mColors == null ? null : mColors[position];
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(int position, View cacheView, ViewGroup parent) {
            LinearLayout.LayoutParams colorLayoutParams = new LinearLayout.LayoutParams(100, 100);

            LinearLayout colorLayout = new LinearLayout(mContext);
            colorLayout.setBackgroundColor(mColors[position]);
            colorLayout.setLayoutParams(colorLayoutParams);

            return colorLayout;
        }

    }
}
