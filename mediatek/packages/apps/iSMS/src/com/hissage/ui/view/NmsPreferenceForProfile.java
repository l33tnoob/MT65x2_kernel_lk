package com.hissage.ui.view;

import com.hissage.ui.activity.NmsProfileSettingsActivity;

import android.R;
import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;

public class NmsPreferenceForProfile extends Preference {

    private Context mContext = null;

    public NmsPreferenceForProfile(Context context) {
        super(context);
        mContext = context;
    }

    public NmsPreferenceForProfile(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
    }

    public NmsPreferenceForProfile(Context context, AttributeSet attrs) {
        super(context, attrs, 0);
        mContext = context;
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        ImageView imageView = (ImageView) view.findViewById(R.id.icon);
        if (imageView != null) {
            imageView.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    Intent i = new Intent(mContext, NmsProfileSettingsActivity.class);
                    mContext.startActivity(i);

                }
            });
        }

    }

}
