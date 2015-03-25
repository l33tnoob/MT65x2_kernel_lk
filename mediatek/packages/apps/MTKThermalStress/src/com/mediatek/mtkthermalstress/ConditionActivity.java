package com.mediatek.mtkthermalstress;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import com.mediatek.xlog.Xlog;

public class ConditionActivity extends Activity {
	private EditText eT_inittemp = null;
	private EditText eT_endtemp = null;
	private Spinner tp_spinner = null;
	private Spinner mt_spinner = null;
	private Spinner cpufreq_spinner = null;
	private SeekBar mSBCpuNumber = null;
	private TextView tvCpuNumber = null;
	long[] mArrayFreq = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_condition);
		CPUControl CPUCtl = new CPUControl();
		int Cpunum = Utility.readIntData(this, R.string.cpunum_key);

		eT_inittemp = (EditText) findViewById(R.id.edittext_init_temp);
		eT_endtemp = (EditText) findViewById(R.id.edittext_end_temp);

		tvCpuNumber = (TextView) findViewById(R.id.textView_cpunum);
		tvCpuNumber.setText("Set CPU online core numbers: " + Cpunum);
		mSBCpuNumber = (SeekBar) findViewById(R.id.seekBar_cputnumber);
		mSBCpuNumber.setMax(CPUCtl.getCpuCount());
		mSBCpuNumber.setProgress(Cpunum);
		mSBCpuNumber.setOnSeekBarChangeListener(new OnSeekBarChangeListener(){
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser){
				tvCpuNumber.setText("Set CPU online core numbers: " + progress);
				if (progress != 0)
					Utility.saveIntData(ConditionActivity.this, R.string.cpunum_key, progress);
				else{
					SeekBar sb = (SeekBar)findViewById(R.id.seekBar_cputnumber);
					sb.setProgress(1);
					Utility.saveIntData(ConditionActivity.this, R.string.cpunum_key, 1);
				}
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {}

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {}
		});

		// Initialize spinner for thermal policy
		tp_spinner = (Spinner) findViewById(R.id.tp_spinner);
		setupSpinner(tp_spinner, R.array.policy_array);
		tp_spinner.setOnItemSelectedListener(new TPSpinnerClass());

		// Initialize spinner for micro-throttle
		mt_spinner = (Spinner) findViewById(R.id.mt_spinner);
		setupSpinner(mt_spinner, R.array.mt_array);
		mt_spinner.setOnItemSelectedListener(new MTSpinnerClass());

		// Initialize spinner for cpu freq.
		cpufreq_spinner = (Spinner) findViewById(R.id.spinner_cpufreq);
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		mArrayFreq = CPUCtl.getCpuFreqSettings();
		String defFreq = Utility.readStringData(this, R.string.cpufreq_key);
		for(int i = 0;i<mArrayFreq.length;++i){
			adapter.add(String.valueOf(mArrayFreq[i]));
		}
		cpufreq_spinner.setAdapter(adapter);
		cpufreq_spinner.setOnItemSelectedListener(new CPUSpinnerClass(mArrayFreq));
		cpufreq_spinner.setSelection(adapter.getPosition(defFreq));

		setupEditText(eT_inittemp, Utility.readIntData(this, R.string.inittemp_key));
		setupEditText(eT_endtemp, Utility.readIntData(this, R.string.endtemp_key));
	}

	@Override
	protected void onResume(){
		super.onResume();
	}

	@Override
	public void onPause(){
		super.onPause();

		Utility.saveIntData(this, R.string.inittemp_key, Integer.parseInt(eT_inittemp.getText().toString()));
		Utility.saveIntData(this, R.string.endtemp_key, Integer.parseInt(eT_endtemp.getText().toString()));
		Utility.saveIntData(this, R.string.cpunum_key, mSBCpuNumber.getProgress());
	}

	@Override
	public void onBackPressed(){
		int initTemp = Integer.parseInt(eT_inittemp.getText().toString());
		int endTemp = Integer.parseInt(eT_endtemp.getText().toString());
		int tempLimit = Utility.readIntData(this, R.string.templimit_key);

		if (initTemp + endTemp >= tempLimit){
			new AlertDialog.Builder(this)
			.setMessage("Initial temp + end temp is too high(>"+tempLimit+")\nPlease lower any one of them")
			.setPositiveButton(android.R.string.yes, null).create().show();
		}else if(endTemp == 0){
			new AlertDialog.Builder(this)
			.setMessage("Please set end temp other than 0")
			.setPositiveButton(android.R.string.yes, null).create().show();
		}else{
			super.onBackPressed();
		}
	}

	private void setupEditText(EditText editText, int def){
		editText.setText(String.valueOf(def));
		editText.setOnEditorActionListener(new OnEditorActionListener(){
			@Override
			public boolean onEditorAction(TextView view, int actionId, KeyEvent event) {
				boolean handled = false;
				switch(actionId){
				case EditorInfo.IME_ACTION_DONE:
					handled = true;
					InputMethodManager m_imm =
							(InputMethodManager)view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
					m_imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
					break;
				default:
					Xlog.e(Utility.mTag,"Unknown action " + actionId);
					break;
				};
				return handled;
			}
		});
	}

	private void setupSpinner(Spinner spinner, int text_array){
		ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
				text_array, android.R.layout.simple_spinner_item);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		spinner.setAdapter(adapter);
	}

	class TPSpinnerClass implements OnItemSelectedListener{
		public void onItemSelected(AdapterView<?> parent, View view, int pos, long id){
			//StringBuilder stringBuilder = new StringBuilder();
			//stringBuilder.append("Policy is set to ");
			//stringBuilder.append(parent.getItemAtPosition(pos));
			//Utility.ToastText(view.getContext(), stringBuilder.toString());

			switch(pos){
			case 0:
				Utility.saveStringData(ConditionActivity.this, R.string.tp_key, getString(R.string.tp_ht120));
				break;
			case 1:
				Utility.saveStringData(ConditionActivity.this, R.string.tp_key, getString(R.string.tp_protect));
				break;
			case 2:
				Utility.saveStringData(ConditionActivity.this, R.string.tp_key, getString(R.string.tp_def));
				break;
			};
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0) {
		}
	}

	class MTSpinnerClass implements OnItemSelectedListener{
		public void onItemSelected(AdapterView<?> parent, View view, int pos, long id){
			//StringBuilder stringBuilder = new StringBuilder();
			//stringBuilder.append("Micro-throttling is ");
			//stringBuilder.append(parent.getItemAtPosition(pos));
			//Utility.ToastText(view.getContext(), stringBuilder.toString());

			switch(pos){
			case 0:
				Utility.saveIntData(ConditionActivity.this, R.string.mt_key, Utility.DISABLE_MTHROTTLE);
				break;
			case 1:
				Utility.saveIntData(ConditionActivity.this, R.string.mt_key, Utility.ENABLE_MTHROTTLE);
				break;
			};
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0) {
		}
	}

	class CPUSpinnerClass implements OnItemSelectedListener{
		private long[] arrayFreq = null;

		CPUSpinnerClass(long[] arrayFreq){
			this.arrayFreq = arrayFreq;
		}
		public void onItemSelected(AdapterView<?> parent, View view, int pos, long id){
			//StringBuilder stringBuilder = new StringBuilder();
			//stringBuilder.append("CPU frequency is set to ");
			//stringBuilder.append(parent.getItemAtPosition(pos));
			//Utility.ToastText(view.getContext(), stringBuilder.toString());

			Utility.saveStringData(ConditionActivity.this, R.string.cpufreq_key, String.valueOf(this.arrayFreq[pos]));
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0) {
		}
	}


}
