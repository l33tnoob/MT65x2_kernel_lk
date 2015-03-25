package com.mediatek.mtkthermalstress;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;

public class PatternActivity extends Activity{
	private CheckBox cbCpuMD5 = null;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_pattern);
		
		cbCpuMD5 = (CheckBox)findViewById(R.id.checkBox_cpu);
		if (Utility.ENABLE_CPU_MD5 == Utility.readIntData(this, R.string.pattern_cpu_key)){
			cbCpuMD5.setChecked(true);			
		}else{
			cbCpuMD5.setChecked(false);
		}
	}
	
	@Override
	public void onBackPressed(){
		super.onBackPressed();
		//TODO: Save settings here
		Utility.ToastText(this, "Leave PatternActivity");
	}
	
	public void onCheckboxClicked(View view){
		boolean checked = ((CheckBox) view).isChecked();
		
		switch(view.getId()){
		case R.id.checkBox_cpu:
			if (checked){				
				Utility.ToastText(this,"CPU MD5 is enable");
				Utility.saveIntData(this, R.string.pattern_cpu_key, Utility.ENABLE_CPU_MD5);
			}
			else{
				Utility.ToastText(this,"CPU MD5 is disable");
				Utility.saveIntData(this, R.string.pattern_cpu_key, Utility.DISABLE_CPU_MD5);
			}
			break;
		}
	}
}
