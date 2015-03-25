package com.hissage.ui.view;



import com.hissage.ui.activity.NmsFunctionIntroductionActivity;

import com.hissage.R;
import android.content.Context;
import android.content.Intent;
import android.preference.Preference;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.text.style.UnderlineSpan;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TextView;

public class NmsPreferenceSettingComent extends Preference {
	private Context context = null;

	public NmsPreferenceSettingComent(Context context, AttributeSet attrs,
			int defStyle) {
		super(context, attrs, defStyle);
		// TODO Auto-generated constructor stub
		this.context = context;
//		setLayoutResource(R.layout.setting_comment);
	}

	public NmsPreferenceSettingComent(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
		this.context = context;
//		setLayoutResource(R.layout.setting_comment);
	}

	public NmsPreferenceSettingComent(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
		this.context = context;
//		setLayoutResource(R.layout.setting_comment);
	}

	@Override
	protected void onBindView(View view) {
		// TODO Auto-generated method stub
		super.onBindView(view);
		TextView tv = (TextView) view.findViewById(R.id.comment_tv);
		String sContent = context.getResources().getString(R.string.STR_NMS_SETTING_COMMENT) ;
		
		SpannableString ss = new SpannableString(sContent) ;
		ss.setSpan(new URLSpan("noting"), sContent.indexOf(context.getResources().getString(R.string.STR_NMS_KNOW_MORE)), sContent.length(), 
				Spanned.SPAN_EXCLUSIVE_EXCLUSIVE) ;
		ss.setSpan(new UnderlineSpan(), sContent.indexOf(context.getResources().getString(R.string.STR_NMS_KNOW_MORE)), sContent.length(), 
				Spanned.SPAN_EXCLUSIVE_EXCLUSIVE) ;
		tv.setText(ss) ;
		tv.setMovementMethod(LinkMovementMethod.getInstance()) ;
		
		CharSequence text = tv.getText();
		if (text instanceof Spannable) {
			int end = text.length();
			Spannable sp = (Spannable) tv.getText();
			URLSpan[] urls = sp.getSpans(0, end, URLSpan.class);
			SpannableStringBuilder style = new SpannableStringBuilder(text);
			style.clearSpans();// should clear old spans
			for (URLSpan url : urls) {
				MyURLSpan myURLSpan = new MyURLSpan();
				style.setSpan(myURLSpan, sp.getSpanStart(url),
						sp.getSpanEnd(url), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
			}
			tv.setText(style);
		}

		
		
		
	}
	
	
	private class MyURLSpan extends ClickableSpan {
		@Override
        public void onClick(View widget) {
        
            Intent functionIntroductionIntent = new Intent(context, NmsFunctionIntroductionActivity.class);
            context.startActivity(functionIntroductionIntent);

        }
	

	}

}
