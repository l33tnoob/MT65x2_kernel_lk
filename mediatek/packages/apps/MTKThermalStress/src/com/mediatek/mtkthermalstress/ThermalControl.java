package com.mediatek.mtkthermalstress;

import java.io.File;

import android.content.Context;

import com.mediatek.xlog.Xlog;

public class ThermalControl {
	private String[] mTzType = null;
	private String[] mTzTempFilePath = null;
	private int mZoneCount = 0;
	CPUControl mCpuctl = null;

	public ThermalControl(){
		mCpuctl = new CPUControl();

	    mZoneCount = getZoneCount();
	    mTzTempFilePath = new String[mZoneCount];
	    mTzType = new String[mZoneCount];
	}

	public static void turnOffThermalProtection(Context context)
	{
		 turnOnThermalProtection(context.getString(R.string.tp_protect));
	}

	 public static void turnOnThermalProtection(String confFile)
	 {
		 Xlog.d(Utility.mTag, "Set thermal policy \"" + confFile + "\"");
		 Utility.executeShellCommand("/system/bin/thermal_manager " + confFile);
	 }

	public String getThermalType(int id){
   		if (null == mTzType[id]){
   			String path = "/sys/class/thermal/" + "thermal_zone" + id + "/type";
   			mTzType[id] = Utility.readFile(path);
   			String filePath = "/proc/mtktz/" + mTzType[id];
       		File f = new File(filePath);
       		if (f.exists())
       			mTzTempFilePath[id] = filePath;
       		else
       			mTzTempFilePath[id] = "/sys/class/thermal/" + "thermal_zone" + id + "/temp";
   		}
        return mTzType[id];
    }

	public String getThermalTemp(int id){
		if (null == mTzTempFilePath[id]) {
			getThermalType(id);
		}
		String strRet = Utility.readLineFromFile(mTzTempFilePath[id]);
		if (strRet.equals(""))
			return "0";
		else
			return strRet;
    }

	private int getZoneCount(){
        int zoneCount = 0;
    	File f = new File("/sys/class/thermal/");

    	if(!f.exists())
    		return 0;

    	File[] files = f.listFiles();
    	for(File file : files){
    		if(file.getName().startsWith("thermal_zone"))
    		    ++zoneCount;
    	}
    	return zoneCount;
    }

	public int getThermalCount(){
		return mZoneCount;
	}
}
