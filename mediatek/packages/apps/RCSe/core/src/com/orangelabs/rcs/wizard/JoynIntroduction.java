package com.orangelabs.rcs.wizard;


import com.orangelabs.rcs.R;
import com.orangelabs.rcs.R.layout;
import com.orangelabs.rcs.R.menu;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Spinner;

public class JoynIntroduction extends Activity implements OnClickListener {

	private Button mNextbtn;
	
	private Button mSkipbtn;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_joyn_introduction);
		
		initLayout();
	}
	
	
	private void initLayout(){	
		   mNextbtn = (Button) findViewById(R.id.next);
	       mNextbtn.setOnClickListener(this);
	       
	       
	       mSkipbtn = (Button) findViewById(R.id.skip);
	       mSkipbtn.setOnClickListener(this);
	}



	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		 if (v == mNextbtn) {
			  nextStep(true);
	     }
		 else if(v == mSkipbtn){
			 nextStep(false);
			 
		 }
	}
	
	private void nextStep(boolean flag){
		
		int result;
		if(flag){
			result = Utils.RESULT_CODE_NEXT;	
		}
		else{
		    result = Utils.RESULT_CODE_FINISH_VIDEO;	
		}
		
        finishActivityByResult(result);
		
	}

	   /**
     * Set result code and finish
     * @param resultCode true to start next step, false to start last step
     */
    private void finishActivityByResult(int resultCode) {
       // Xlog.d(TAG, "finishActivityByResult, resultCode: " + resultCode);
        Intent intent = new Intent();
        setResult(resultCode, intent);
        finish();
    }

}
