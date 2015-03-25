package com.mediatek.mtkthermalstress;
import java.io.IOException;
import java.io.RandomAccessFile;

import com.mediatek.xlog.Xlog;

public class MicroThrottlingControl {
	public static void turnOff(){
		//Set low-limit to 110000 means turning micro throttling off
		//int[] limits = new int[]{110000, 120000};
		//turnOn(limits);
		Utility.writeFile("/proc/cpufreq/cpufreq_ptpod_temperature_limit", "110000 120000");
	}
	public static void turnOn(int[] limits){
		Xlog.d(Utility.mTag, "Set micro-throttling " + limits[0]+"-"+limits[1]);
		//String cmd = "echo "+limits[0]+" "+limits[1]+" > /proc/cpufreq/cpufreq_ptpod_temperature_limit";
		//Utility.executeShellCommand(cmd);
		Utility.writeFile("/proc/cpufreq/cpufreq_ptpod_temperature_limit", ""+limits[0]+" "+limits[1]);
	}
	public static int[] getFreqInfo(){
		Xlog.d(Utility.mTag, "getFreqInfo");
		int[] ret = new int[2];
		int lowlimit = 0;
		int highlimit = 0;
		try{
			RandomAccessFile reader = new RandomAccessFile("/proc/cpufreq/cpufreq_downgrade_freq_info", "r");
			String line;
			do{
				line = reader.readLine();
				if (line == null)
					break;
				//Xlog.d(Utility.mTag, line);
				if (line.contains("mt_cpufreq_ptpod_temperature_limit_1")){
					String[] array = line.split("\\s+");
					lowlimit = Integer.parseInt(array[2]);
					//Xlog.d(Utility.mTag, "lowlimit is " + lowlimit);
				}
				if (line.contains("mt_cpufreq_ptpod_temperature_limit_2")){
					String[] array = line.split("\\s+");
					highlimit = Integer.parseInt(array[2]);
					//Xlog.d(Utility.mTag, "highlimit is " + highlimit);
				}
			}while ((lowlimit == 0)||(highlimit == 0));
			reader.close();
		}catch(IOException ex){
			Xlog.e(Utility.mTag, "GetFreqInfo failed", ex);
		}
		ret[0] = lowlimit;
		ret[1] = highlimit;
		return ret;
	}
}
