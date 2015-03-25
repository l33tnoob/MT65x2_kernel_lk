package com.mediatek.mtkthermalstress;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.xlog.Xlog;

public class CPUControl {
	private static final int CPU_IDLE = 3;
	private int mCpuStatParmCnt = 0;
	private float[] mCpuUsage = null;
	private long[][] mCpuLastStat = null;
	private long[][] mCpuCurStat = null;
	private int mCpucount = 0;

	public CPUControl(){
		int cpuCount = getCpuCount();

		mCpuStatParmCnt = getCpuStatCount();
		mCpuUsage = new float[cpuCount];
	    mCpuLastStat = new long[cpuCount][];
	    mCpuCurStat = new long[cpuCount][];
	    for(int i = 0; i < cpuCount; ++i){
	    	mCpuLastStat[i] = new long[mCpuStatParmCnt];
	    	mCpuCurStat[i] = new long[mCpuStatParmCnt];
	    }

	    getCpuStat(mCpuLastStat);
	}

    public int getCpuFreq(int cpuId){
    	String path = "/sys/devices/system/cpu/cpu" + cpuId + "/cpufreq/scaling_cur_freq";
    	String res = Utility.readFile(path);
    	if(res.isEmpty()) return 0;
    	else return Integer.parseInt(res);
    }

    public float[] getCpuUsage(){
    	getCpuStat(mCpuCurStat);
    	for(int i = 0; i < getCpuCount(); ++i){
        	int delta = 0;
        	int idleDelta = 0;
        	mCpuUsage[i] = 0;

    		idleDelta = (int) (mCpuCurStat[i][CPU_IDLE] - mCpuLastStat[i][CPU_IDLE]);
    		for(int j = 0; j < mCpuStatParmCnt; j++){
    		    delta += mCpuCurStat[i][j] - mCpuLastStat[i][j];
    		    mCpuLastStat[i][j] = mCpuCurStat[i][j];
    		}
    		if(delta != 0)
    			mCpuUsage[i] = (1 - ((float)idleDelta / (float)delta)) * 100;
    	}

    	return mCpuUsage;
    }


	private int getCpuStatCount(){
		try{
			RandomAccessFile reader = new RandomAccessFile("/proc/stat", "r");
            String line = null;
            line = reader.readLine();
            if (line == null) {
            	reader.close();
				return 0;
			}
            line = reader.readLine();
            reader.close();
            return line.split(" ").length - 1;

		}catch (IOException ex){
			ex.printStackTrace();
			return 0;
		}
	}

	private void getCpuStat(long[][] stat){
		try {
			FileReader fr = new FileReader("/proc/stat");
			BufferedReader reader = new BufferedReader(new FileReader("/proc/stat"));
            String line = null;
            line = reader.readLine();

            int cpuIdx = 0, parsedCnt = 0;
            String token[];
            while (((line = reader.readLine()) != null)
            	 && (parsedCnt < getCpuCount())){
                token = line.split(" ");
                if (!token[0].startsWith("cpu") || token[0].length() <= 3)
                	break;
                cpuIdx = token[0].charAt(3) - '0';

                for(int i = 1; i <= mCpuStatParmCnt && i < token.length; ++i){
                	stat[cpuIdx][i - 1] = Long.parseLong(token[i].trim());
                }
                parsedCnt++;
            }
            reader.close();
            fr.close();

        } catch (IOException ex) {
            ex.printStackTrace();
        }
	}

    public int getCpuCount(){
    	if (0 == mCpucount){
	        String res = Utility.readFile("/sys/devices/system/cpu/present");
	        if(res.length() == 1) return 1;
	        String tokens[] = res.split("-");
	        mCpucount = Integer.parseInt(tokens[1].trim()) + 1;
	        //Xlog.d(Utility.mTag, "getCputCount() returns " + mCpucount);
    	}

    	return mCpucount;
    }

    public void setCpuOnlineCores(int number){
    	//Disable CPU hotplug
    	Xlog.d(Utility.mTag, "setCpuOnlineCores " + number);
    	Utility.writeFile("/sys/devices/system/cpu/cpufreq/hotplug/is_cpu_hotplug_disable", "1");
    	int CpuCount = getCpuCount();
    	int i = 1;
    	//Turn on the 1~(number-1) CPU core
    	for(;i < number;++i){
    		Utility.writeFile("/sys/devices/system/cpu/cpu" + i + "/online", "1");
    	}
    	//Turn off the rest
    	for(;i < CpuCount;++i){
    		Utility.writeFile("/sys/devices/system/cpu/cpu" + i + "/online", "0");
    	}
    }

    public void restoreCpuOnlineCores(){
    	//Enable CPU hotplug
    	Utility.writeFile("/sys/devices/system/cpu/cpufreq/hotplug/is_cpu_hotplug_disable", "0");
    }

    public long[] getCpuFreqSettings(){
    	List<Integer> listFreq = new ArrayList<Integer>();

		try{
			RandomAccessFile reader = new RandomAccessFile("/sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state", "r");
			String line;
			do{
				line = reader.readLine();
				if (line == null)
					break;
				String[] array = line.split("\\s+");
				Xlog.d(Utility.mTag, "Get " + array[0] + " from " + "\"" + line + "\"");
				listFreq.add(Integer.parseInt(array[0]));
			}while (true);
			reader.close();
		}catch(IOException ex){
			Xlog.e(Utility.mTag, "GetFreqInfo failed", ex);
		}

		long[] ret = new long[listFreq.size()];
		for(int i = 0;i < listFreq.size(); ++i){
			ret[i] = listFreq.get(i);
			//Xlog.d(Utility.mTag, "ret[i] is " + ret[i]);
		}

    	return ret;
    }

    public void setCpuMaxFreq(int freq){
    	Xlog.d(Utility.mTag, "setCpuMaxFreq " + freq);
    	Utility.writeFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", String.valueOf(freq));
    }

    public int getCpuMaxFreq(){
    	int ret = 0;

    	try{
			RandomAccessFile reader = new RandomAccessFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
			String line;
			do{
				line = reader.readLine();
				if (line == null)
					break;
				String[] array = line.split("\\s+");
				Xlog.d(Utility.mTag, "getCpuMaxFreq " + array[0]);
				ret = Integer.parseInt(array[0]);
			}while (true);
			reader.close();
		}catch(IOException ex){
			Xlog.e(Utility.mTag, "GetFreqInfo failed", ex);
		}

    	return ret;
    }
}
