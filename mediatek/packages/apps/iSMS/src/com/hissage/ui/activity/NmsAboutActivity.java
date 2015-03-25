package com.hissage.ui.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnCancelListener;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.text.style.URLSpan;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.api.NmsStartActivityApi;
import com.mediatek.mms.ipmessage.IpMessageConsts;

public class NmsAboutActivity extends ListActivity{

	private AlertDialog mVersionDlg = null;
    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        switch (id) {
        case android.R.id.home: {
            finish();
            break;
        }
        }
        return super.onMenuItemSelected(featureId, item);
    }

    protected void showActivitionDlg() {

    	LayoutInflater factory = LayoutInflater.from(this);
    	final View view = factory.inflate(R.layout.alert_dialog_text_view, null);
    	TextView textView = (TextView) view.findViewById(R.id.term_textview);
    	textView.setText(R.string.STR_NMS_MENU_VERSION_INFORMATION_DETAIL);
    	textView.setGravity(Gravity.CENTER);
  
    	mVersionDlg = new AlertDialog.Builder(this)
                .setTitle(R.string.STR_NMS_MENU_VERSION_INFORMATION)
                .setView(view)
                .setPositiveButton(R.string.STR_NMS_OK,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                return;
                            }
                        }).setOnCancelListener(new OnCancelListener() {

                    @Override
                    public void onCancel(DialogInterface dialog) {

                    }
                }).create();
    	mVersionDlg.show();

    }
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setDisplayShowHomeEnabled(true);
        getActionBar().setLogo(R.drawable.isms);
        setContentView(R.layout.about);	

        String[] aboutOptionList = new String[] {getResources().getString(R.string.STR_NMS_MENU_FUNCTION_INTRODUCTION),
              getResources().getString(R.string.STR_NMS_MENU_LICENSE_AGREEMENT),
              getResources().getString(R.string.STR_NMS_MENU_VERSION_INFORMATION)};
        	  setListAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, aboutOptionList));
        		
       
	
	}
	@Override
    protected void onListItemClick(ListView arg0, View arg1, int arg2, long arg3) {
      
            switch (arg2) {
            case 0:
                Intent functionIntroductionIntent = new Intent(NmsAboutActivity.this, NmsFunctionIntroductionActivity.class);
                startActivity(functionIntroductionIntent);
                break;
            case 1:
                Intent licenseAgreementIntent = new Intent(NmsAboutActivity.this, NmsLicenseAgreementActivity.class);
                startActivity(licenseAgreementIntent);
                break;
            case 2:
            	/*
                Intent versionInformationIntent = new Intent(NmsAboutActivity.this,
                		NmsNewEditGroupChatActivity.class);
                startActivity(versionInformationIntent);
                */
            	this.showActivitionDlg();
                break;
            default:
                break;
            }
        }
   
}
