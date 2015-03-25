package com.mediatek.mtkthermalstress;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;
import android.view.Menu;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.mediatek.xlog.Xlog;

public class MainActivity extends Activity implements OnItemClickListener{
	private boolean mIsStartStress = false;
	private boolean mIsStartUpdate = false;
	private CPUControl mCpuCtl = null;
	private String mOriginalTP;
	private int[] mOriginalMTLimit;
	private int mOriginalCpuFreq;
	private CPUThread[] mCPUThreads = null;
	private int mThreadNum;
	private long mStartTime;
	private long mEndTime;
	private Logger mLogger = null;
	private int mData = 0;
	private int mLevel = 0;
	private WakeLock mWakeLock = null;
	private boolean mIsUserPressButton = false;

	static
	{
		System.loadLibrary("thermalstress_jni");
	}
	public static native int getData();
	public static native int getLevel();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		ListView lv_main = (ListView) findViewById(R.id.lv_main);
		List<String> items = new ArrayList<String>();
		items.add(getString(R.string.lv_condition));
		items.add(getString(R.string.lv_pattern));
		ArrayAdapter<String> mArrayAdap = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, items){
			@Override
			public boolean isEnabled(int position){
				if(mIsStartStress == true){
					return false;
				}else{
					return true;
				}
			}
		};
		lv_main.setAdapter(mArrayAdap);
		lv_main.setOnItemClickListener(this);

		mIsStartStress = false;
		mIsUserPressButton = false;
		mCpuCtl = new CPUControl();
		mThreadNum = mCpuCtl.getCpuCount() * 2;
		mStartTime = mEndTime = 0;
		mOriginalTP = Utility.readFile("/data/.tp.settings");
		Xlog.d(Utility.mTag, "mOriginalTP is " + mOriginalTP);
		mOriginalMTLimit = MicroThrottlingControl.getFreqInfo();
		mOriginalCpuFreq = mCpuCtl.getCpuMaxFreq();
		mData = getData();
		mLevel = getLevel();
		new LoadDefaultTask().execute(mLevel);
	}

	@Override
	protected void onResume(){
		super.onResume();
		mTimerHandler.post(timerTask);
		if (mWakeLock == null){
			//if (mIsUserPressButton == true)
			{
				PowerManager pMgr = (PowerManager) getSystemService(Context.POWER_SERVICE);
		        mWakeLock = pMgr.newWakeLock(PowerManager.FULL_WAKE_LOCK, "MTKThermalStress");
		        mWakeLock.acquire();
		        mWakeLock.setReferenceCounted(false);
			}
		}
	}

	@Override
	public void onPause(){
		super.onPause();
		mTimerHandler.removeCallbacks(timerTask);
		//stopStress();
		if (mWakeLock != null){
			mWakeLock.release();
			mWakeLock = null;
		}
	}

	public void onClickButton(View view){
		boolean on = ((ToggleButton) view).isChecked();

		if (on){
			mIsStartUpdate = true;
			mIsUserPressButton = true;
		}else{
			stopStress();
		}
	}

	private synchronized void startStress(){
		if (this.mIsStartStress == true){
			Xlog.d(Utility.mTag, "Start stress abort since this.mIsStartStress = true");
			return;
		}
		if (this.mIsUserPressButton == false){
			Xlog.d(Utility.mTag, "Start stress abort since mIsUserPressButton = false");
			return;
		}

		Xlog.d(Utility.mTag, "Start stress");
		int CpuNum = Utility.readIntData(this, R.string.cpunum_key);
		int CpuMaxFreq = Integer.parseInt(Utility.readStringData(this, R.string.cpufreq_key));
		String thermalPolicy = Utility.readStringData(this, R.string.tp_key);
		int mThrottling = Utility.readIntData(this, R.string.mt_key);
		mLogger = new Logger(new ThermalControl(), this.mCpuCtl,
				"Initial condition:\n"
				+ "\nCPU number: " + CpuNum
				+ "\nCPU Max freq. is " + CpuMaxFreq
				+ "\nThermal policy is " + thermalPolicy
				+ "\nMicro throttling is " + mThrottling
				+ "\nData is " + mData);

		this.mCpuCtl.setCpuOnlineCores(CpuNum);
		this.mCpuCtl.setCpuMaxFreq(CpuMaxFreq);

		ThermalControl.turnOnThermalProtection(thermalPolicy);

		switch(mThrottling){
		case Utility.DISABLE_MTHROTTLE:
			MicroThrottlingControl.turnOff();
			break;
		case Utility.ENABLE_MTHROTTLE:
			MicroThrottlingControl.turnOn(this.mOriginalMTLimit);
			break;
		}
		
		mThreadNum = CpuNum * 2;

		if (Utility.ENABLE_CPU_MD5 == Utility.readIntData(this, R.string.pattern_cpu_key)){
			mCPUThreads = new CPUThread[mThreadNum];
			for (int i = 0; i < mCPUThreads.length; ++i){
				mCPUThreads[i] = new CPUThread();
				mCPUThreads[i].start();
			}
		}
		mCpuCtl.getCpuUsage();
		mStartTime = SystemClock.elapsedRealtime();
		Utility.saveIntData(this, R.string.starttemp_key, 0);
		Utility.saveIntData(this, R.string.lasttemp_key, 0);

		/*
		PowerManager pMgr = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = pMgr.newWakeLock(PowerManager.FULL_WAKE_LOCK, "MTKThermalStress");
        mWakeLock.acquire();
        */
        
        Utility.saveStringData(this, R.string.stop_reason_key, "User Stop");

		mIsStartStress = true;
		mLogger.logNow();
	}

	int calculateResult(String stopReason){
		int isPass = 0;

		if (stopReason.equals("Reach target temperature")){
			//Fail
			return 1;
		}else if (stopReason.equals("Timeout")){
			//Pass
			return 2;
		}

		return isPass;
	}

	private synchronized void stopStress(){
		if (this.mIsStartStress == false){
			return;
		}
		
		mIsUserPressButton = false;

		String logName = "";
		String stopReason = Utility.readStringData(this, R.string.stop_reason_key);
		Xlog.d(Utility.mTag, "Stop stress: " + stopReason);

		if (mLogger != null){
			//TODO: also log stopReason here
			mLogger.logData(stopReason);

			mLogger.flush();
			mLogger.close();
			logName = mLogger.getFilename();
			Xlog.d(Utility.mTag, "Close log: " + logName);
			mLogger = null;
		}

		if (mCPUThreads != null){
			for (int i = 0; i < mCPUThreads.length; ++i){
				mCPUThreads[i].setStop();
			}
		}
		mEndTime = SystemClock.elapsedRealtime();

		//FIXME: Should set to the tp read from settings
		//ThermalControl.turnOnThermalProtection(mOriginalTP.trim());
		Utility.executeShellCommand("/system/bin/thermal_manager /etc/.tp/thermal.conf");

		MicroThrottlingControl.turnOn(this.mOriginalMTLimit);
		this.mCpuCtl.restoreCpuOnlineCores();

		this.mCpuCtl.setCpuMaxFreq(mOriginalCpuFreq);

		mIsStartStress = false;
		mIsStartUpdate = false;
		ToggleButton btStress = (ToggleButton) findViewById(R.id.toggleButton_stress);
		btStress.setChecked(false);

		int isPass = calculateResult(stopReason);

		AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
		StringBuilder strResult = new StringBuilder();
		if (isPass == 2){
			strResult.append("Result is PASSED\n");
		}else if (isPass == 1){
			strResult.append("Result is FAILED\n");
		}else if (isPass == 0){
			strResult.append("Stress is not finished\n");
		}
		strResult.append("(log: " + logName + ")");
		builder.setMessage(strResult.toString());
		builder.setPositiveButton("OK", new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface dialog, int id){
				//Do nothing for this
			}
		});
		AlertDialog dialog = builder.create();
		dialog.show();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3){
		Intent intent = new Intent();
		switch(arg2){
		case 0:
			intent.setClass(this, ConditionActivity.class);
			this.startActivity(intent);
			break;
		case 1:
			intent.setClass(this, PatternActivity.class);
			this.startActivity(intent);
			break;
		}
	}

	private class execInfo{
		private CPUControl mCpuCtl = null;
		private boolean mIsStartUpdate = false;
		private boolean mIsStartStress = false;
		private long mStartTime = 0;
		private Logger mLogger = null;

		public CPUControl getCpuCtl(){
			return mCpuCtl;
		}

		public void setInfo(CPUControl CpuCtl, boolean IsStartUpdate, boolean IsStartStress, long StartTime, Logger Logger){
			mCpuCtl = CpuCtl;
			mIsStartUpdate = IsStartUpdate;
			mIsStartStress = IsStartStress;
			mStartTime = StartTime;
			mLogger = Logger;
		}

		public boolean isStartUpdate(){
			return mIsStartUpdate;
		}

		public boolean isStartStress(){
			return mIsStartStress;
		}

		public long getStartTime(){
			return mStartTime;
		}

		public Logger getLogger(){
			return mLogger;
		}
	}
	
	private UpdateTextTask mUpdateTask = new UpdateTextTask(this);
	private execInfo mUpdateInfo = new execInfo();
	private Handler mTimerHandler = new Handler();
	private Runnable timerTask = new Runnable(){
		@Override
		public void run(){
			mUpdateInfo.setInfo(mCpuCtl, mIsStartUpdate, mIsStartStress, mStartTime, mLogger);
			if (mIsStartUpdate == true){
				if (mUpdateTask.getStatus()==AsyncTask.Status.PENDING){
					mUpdateTask.cancel(false);
					Xlog.d(Utility.mTag, "cancel pending update task");
				}
				
				//new UpdateTextTask(MainActivity.this).execute(mUpdateInfo);
				if (mUpdateTask.getStatus()==AsyncTask.Status.FINISHED)
					mUpdateTask = null;
					mUpdateTask = new UpdateTextTask(MainActivity.this);
					mUpdateTask.execute(mUpdateInfo);
			}

			mTimerHandler.postDelayed(timerTask, 500);
		}
	};

	private class DefaultSettings{
		public int initTemp;
		public int endTemp;
		public int period;
		public int level;
		public int templimit;
	}

	List<DefaultSettings> mListDefSetting = new ArrayList<DefaultSettings>();
	private class LoadDefaultTask extends AsyncTask<Integer, Object, Object>{

		@Override
		protected Object doInBackground(Integer... arg0) {			
			int level = arg0[0];
			
			try{
				RandomAccessFile reader = new RandomAccessFile("/system/etc/.tp/thermalstress.cfg", "r");
				String line;
				DefaultSettings setting = null;
				do{
					line = reader.readLine();
					Xlog.d(Utility.mTag, "Get cfg line: " + line);
					if (line == null){
						break;
					}
					String[] array = line.split("\\s+");
					String key = "";
					if (array.length >= 2)
						key = array[1];
					if (key.equalsIgnoreCase("setting")){
						setting = new DefaultSettings();
					}else if (key.equalsIgnoreCase("initial")){
						setting.initTemp = Integer.parseInt(reader.readLine().trim());
					}else if (key.equalsIgnoreCase("end")){
						setting.endTemp = Integer.parseInt(reader.readLine().trim());
					}else if (key.equalsIgnoreCase("period")){
						setting.period = Integer.parseInt(reader.readLine().trim());
					}else if (key.equalsIgnoreCase("templimit")){
						setting.templimit = Integer.parseInt(reader.readLine().trim());
					}else if (key.equalsIgnoreCase("level")){
						setting.level = Integer.parseInt(reader.readLine().trim());
						mListDefSetting.add(setting);
						Xlog.d(Utility.mTag, "Add a new setting");
					}
				}while (true);
				reader.close();
			}catch(IOException ex){
				Xlog.e(Utility.mTag, "GetFreqInfo failed", ex);
			}

			int defIndex = 0;
			for (int i = 0; i < mListDefSetting.size();++i){
				if (mListDefSetting.get(i).level == level){
					defIndex = i;
				}
			}

			Utility.saveStringData(MainActivity.this, R.string.tp_key, getString(R.string.tp_ht120));
			Utility.saveIntData(MainActivity.this, R.string.mt_key, Utility.DISABLE_MTHROTTLE);
			Utility.saveIntData(MainActivity.this, R.string.pattern_cpu_key, Utility.ENABLE_CPU_MD5);
			Utility.saveIntData(MainActivity.this, R.string.inittemp_key, mListDefSetting.get(defIndex).initTemp);
			Utility.saveIntData(MainActivity.this, R.string.endtemp_key, mListDefSetting.get(defIndex).endTemp);
			Utility.saveIntData(MainActivity.this, R.string.expecttime_key, mListDefSetting.get(defIndex).period);
			Utility.saveIntData(MainActivity.this, R.string.templimit_key, mListDefSetting.get(defIndex).templimit);
			Utility.saveStringData(MainActivity.this, R.string.cpufreq_key, String.valueOf(mCpuCtl.getCpuMaxFreq()));
			Utility.saveIntData(MainActivity.this, R.string.cpunum_key, mCpuCtl.getCpuCount());

			return null;
		}
	}

	private class UpdateTextTask extends AsyncTask<execInfo, Object, Integer>{
		private TextView tvInfo;
		private StringBuffer strMsg;
		private boolean isReady2Stress = false;
		private Context MainContext = null;

		public UpdateTextTask(Context context){
			MainContext = context;
		}

		@Override
		protected void onPreExecute(){
			super.onPreExecute();
			tvInfo = (TextView) findViewById(R.id.textView_msg);
			strMsg = new StringBuffer();
		}

		@Override
		protected Integer doInBackground(execInfo... arg0) {
			execInfo info = arg0[0];
			CPUControl cpuCtrl = info.getCpuCtl();
			Integer isStressEnd = 0;
			Xlog.d(Utility.mTag, "UpdateTextTask: isStartStress=" + info.isStartStress() 
					+ ", isStartUpdate=" + info.isStartUpdate());

			if (info.isStartUpdate()){
				ThermalControl tmCtl = new ThermalControl();

				int count = cpuCtrl.getCpuCount();
				String str = String.format(Locale.US, "CPU\nCore number: %1d\n", count);
				strMsg.append(str);
				strMsg.append("Frequency (Unit: KHz)\n");
				float usage[] = cpuCtrl.getCpuUsage();
				int i = 0;
				for (;i<count;++i){
					str = String.format(Locale.US, "CPU%1d:%8d  Usage:%3.3f%%\n",
							i, cpuCtrl.getCpuFreq(i), usage[i]);
					strMsg.append(str);
				}
				Integer cpuTemp = 0;
				strMsg.append("\n");
				strMsg.append("Temperature (Unit: 1/1000 degree C)\n");

				int initTemp = Utility.readIntData(MainContext, R.string.inittemp_key);
				int endTemp = Utility.readIntData(MainContext, R.string.endtemp_key);
				int startTemp = Utility.readIntData(MainContext, R.string.starttemp_key);
				for (i = 0; i < tmCtl.getThermalCount(); ++i){
					if (tmCtl.getThermalType(i).equalsIgnoreCase("mtktscpu")){
						str = String.format(Locale.US, "%-12s:%s\n",
								tmCtl.getThermalType(i), tmCtl.getThermalTemp(i));
						strMsg.append(str);

						cpuTemp = Integer.parseInt(tmCtl.getThermalTemp(i));
						if (cpuTemp < initTemp*1000){
							isReady2Stress = true;
						}else{
							Xlog.d(Utility.mTag, "cpuTemp=" + cpuTemp + " is higher than required temp(" + initTemp + ")");
						}

						if (info.isStartStress() == true){
							long curTime = SystemClock.elapsedRealtime();
							
							long stopTemp = startTemp + endTemp*1000;

							if (cpuTemp >= stopTemp){
								isStressEnd = 1;
								if (0 == Utility.readIntData(MainContext, R.string.lasttemp_key)){
									Utility.saveIntData(MainContext, R.string.lasttemp_key, cpuTemp);
									Utility.saveStringData(MainContext, R.string.stop_reason_key, "Reach target temperature");
								}
							}else{
								Xlog.d(Utility.mTag, "cpuTemp=" + cpuTemp + " hasn't reach stopTemp(" + stopTemp + ")");
							}

							long expectTime = Utility.readIntData(MainContext, R.string.expecttime_key) * 1000;
							if ((curTime - info.getStartTime()) > expectTime){
								isStressEnd = 1;
								if (0 == Utility.readIntData(MainContext, R.string.lasttemp_key)){
									Utility.saveIntData(MainContext, R.string.lasttemp_key, cpuTemp);
									Utility.saveStringData(MainContext, R.string.stop_reason_key, "Timeout");
								}
							}
						}
					}
				}
				strMsg.append("\n");
				if (info.isStartStress() == true){
					long curTime = SystemClock.elapsedRealtime();
					strMsg.append("Stress time (ms): " + (curTime - info.getStartTime()) +"\n");
					info.getLogger().logNow();
				}else{
					if (isReady2Stress == true){
						strMsg.append("mtktscpu is " + cpuTemp +"\n");
						strMsg.append("Ready to stress\n");
						Xlog.d(Utility.mTag, "Ready to stress(mtktscpu: " + cpuTemp + ")");
						startStress();
						if (0 == Utility.readIntData(MainContext, R.string.starttemp_key)){
							Utility.saveIntData(MainContext, R.string.starttemp_key, cpuTemp);
						}
					}else{
						strMsg.append("Warning: mtktscpu is " + cpuTemp + "\n");
						strMsg.append("Stress would start after it is less than " +
								Utility.readIntData(MainContext, R.string.inittemp_key));
					}
				}
			}

			return isStressEnd;
		}

		@Override
		protected void onPostExecute(Integer result){
			tvInfo.setText(strMsg);

			if (result == 1){
				stopStress();
			}
		}
	}
}
