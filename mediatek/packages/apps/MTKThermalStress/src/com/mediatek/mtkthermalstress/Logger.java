package com.mediatek.mtkthermalstress;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import com.mediatek.xlog.Xlog;

public class Logger {
	final String mFolder = "/data/MTKThermalStress";
	private FileWriter mFile = null;
	private ThermalControl mThermalCtl;
	private CPUControl mCpuCtl;
	private String mFileName;

	Logger(ThermalControl thermalCtl, CPUControl cpuCtl, String extraStr){
		mThermalCtl = thermalCtl;
		mCpuCtl = cpuCtl;
		setup(extraStr);
	}

	public String getFilename(){
		return mFileName;
	}

	SimpleDateFormat mDateFormat = new SimpleDateFormat("HH:mm:ss");
	public void logNow(){
		if(mFile == null)
			return;

		try {
			StringBuffer infoStr = new StringBuffer();

			// Current time
			Calendar c = Calendar.getInstance();
			infoStr.append(mDateFormat.format(c.getTime()));
			infoStr.append(",");

			// Temperature of each sensor
			for (int i = 0; i < mThermalCtl.getThermalCount(); ++i){
				infoStr.append(mThermalCtl.getThermalTemp(i));
				infoStr.append(",");
			}

			// CPU frequency and usage
			float cpuUsage[] = mCpuCtl.getCpuUsage();
			for(int i = 0; i < mCpuCtl.getCpuCount(); ++i){
				infoStr.append(String.valueOf(mCpuCtl.getCpuFreq(i)));
				infoStr.append(",");
				infoStr.append(String.valueOf(cpuUsage[i]));
				infoStr.append(",");
			}

            // Add new info below
            // infroStr += ...
            // Add new info above

			infoStr.append("\n");

		    mFile.write(infoStr.toString());
		} catch (IOException e) {
			//e.printStackTrace();
		}
	}

	public void logData(String strData){
		if(mFile == null)
			return;

		try{
			mFile.write(strData);
		} catch(IOException e){
			Xlog.e(Utility.mTag, "logData exception");
		}
	}

	private void setup(String extraStr){
		Calendar c = Calendar.getInstance();

		mFileName = c.get(Calendar.YEAR) + "-" + (c.get(Calendar.MONTH) + 1) + "-" +
		            c.get(Calendar.DAY_OF_MONTH) + "-" + c.get(Calendar.HOUR_OF_DAY) + "-" +
		            c.get(Calendar.MINUTE) + "-" + c.get(Calendar.SECOND) + ".csv";

        try {
            File mDataPath = new File(mFolder);

            if(!mDataPath.exists())
            	mDataPath.mkdirs();

            mFile = new FileWriter( mFolder +"/"+ mFileName);

            String infoStr = extraStr + "\n";
            infoStr += "Time,";

            int themCount = mThermalCtl.getThermalCount();

            for(int i = 0; i < themCount; ++i)
           	 	infoStr += mThermalCtl.getThermalType(i) + " temp,";

            for(int i = 0; i < mCpuCtl.getCpuCount(); ++i){
           	 	infoStr += "CPU" + i + " freq,";
           	 	infoStr += "CPU" + i + " usage,";
            }
            // Add new info below
            // infroStr += ...
            // Add new info above
            infoStr += "\n";
			mFile.write(infoStr);
        } catch (IOException e) {
        	e.printStackTrace();
        }
	}

	public void flush(){
		if(mFile == null) return;
		try {
			mFile.flush();
		} catch (IOException e) {
			//e.printStackTrace();
		}
	}

	public void close(){
		if(mFile == null) return;
		try {
			mFile.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
