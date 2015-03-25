package com.mediatek.mtkthermalstress;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.RandomAccessFile;

import android.content.Context;
import android.content.SharedPreferences;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

public class Utility {
	final public static int ENABLE_CPU_MD5 = 1;
	final public static int DISABLE_CPU_MD5 = 0;
	final public static int ENABLE_MTHROTTLE = 1;
	final public static int DISABLE_MTHROTTLE = 0;
	final public static String mTag = "ThermalStress";

	public static void saveIntData(Context context, int resId, int value){
		String key = context.getString(resId);
		SharedPreferences sharedPref = context.getSharedPreferences(key, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = sharedPref.edit();
		editor.putInt(key, value);
		editor.commit();
	}

	public static int readIntData(Context context, int resId){
		String key = context.getString(resId);
		SharedPreferences sharedPref = context.getSharedPreferences(key, Context.MODE_PRIVATE);
		int defaultValue = R.string.def_data;
		int testValue = sharedPref.getInt(key, defaultValue);

		return testValue;
	}

	public static void saveStringData(Context context, int resId, String value){
		String key = context.getString(resId);
		SharedPreferences sharedPref = context.getSharedPreferences(key, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = sharedPref.edit();
		editor.putString(key, value);
		editor.commit();
	}

	public static String readStringData(Context context, int resId){
		String key = context.getString(resId);
		SharedPreferences sharedPref = context.getSharedPreferences(key, Context.MODE_PRIVATE);
		String defaultValue = "";
		String testValue = sharedPref.getString(key, defaultValue);

		return testValue;
	}

	public static void ToastText(Context context, String str){
		Toast.makeText(context, str, Toast.LENGTH_SHORT).show();
		Xlog.d(mTag, str);
	}

	public static String readFile(String path) {
    	String s = "";
         try {
            RandomAccessFile reader = new RandomAccessFile(path, "r");
            String line = null;
            while ((line = reader.readLine()) != null){
            	s += line;
            }
            reader.close();
        } catch (IOException ex) {
        	//Xlog.e(mTag, "readFile exception", ex);
            //ex.printStackTrace();
        }

        return s;
    }

    public static String readLineFromFile(String path) {
    	String s = "";
         try {
            RandomAccessFile reader = new RandomAccessFile(path, "r");
            String line = null;
            if ((line = reader.readLine()) != null){
            	s += line;
            }
            reader.close();
        } catch (IOException ex) {
        	//Xlog.e(mTag, "readLineFromFile exception", ex);
            //ex.printStackTrace();
        }

        return s;
    }

    public static void executeShellCommand(String shellCommand){
    	Runtime runtime = Runtime.getRuntime();
    	Process proc = null;
    	OutputStreamWriter osw = null;

    	Xlog.d(Utility.mTag, "Shell cmd: " + shellCommand);

    	try
    	{
	    	proc = runtime.exec(shellCommand);
	    	osw = new OutputStreamWriter(proc.getOutputStream());
	    	if (null != osw){
	    		osw.write(shellCommand);
	    		osw.write("\n");
	    		osw.write("exit\n");
	    		osw.flush();
	    		osw.close();
	    	}
    	}
    	catch (IOException ex)
    	{
    		Xlog.e(Utility.mTag, "execCommandLine() IO Exception: ", ex);
    		return;
    	}
    	finally
    	{
    		if (null != osw){
    			try
    			{
    				osw.close();
    			}
    			catch (IOException e){}
    		}
    	}

    	try
    	{
    		proc.waitFor();
    	}
    	catch (InterruptedException e){}

    	if (proc.exitValue() != 0)
    	{
    		Xlog.e(Utility.mTag, "execCommandLine() Err exit code: " + proc.exitValue());
    	}
    }

    public static void writeFile(String filePath, String line)
    {
    	Xlog.d(Utility.mTag, "Writing " + line + " to " + filePath);
    	File a = new File(filePath);
    	if (a.exists())
    	{
    		try {
    			FileOutputStream fs = new FileOutputStream(a);
    			DataOutputStream ds = new DataOutputStream(fs);
    			ds.write(line.getBytes());
    			ds.flush();
    			ds.close();
    			fs.close();
    		}
    		catch (Exception ex) {
    			Xlog.e("MTKThermalManagerActivity", "writeFile() Exception: " + filePath, ex);
    		}
    	}
    	else
    	{
    		Xlog.d("MTKThermalManagerActivity", "writeFile() File not exist: " + filePath);
    		try {
    			if (a.createNewFile())
    			{
    				Xlog.d("MTKThermalManagerActivity", "writeFile() File created: " + filePath);
    				try {
    					FileOutputStream fs = new FileOutputStream(a);
    					DataOutputStream ds = new DataOutputStream(fs);
    					ds.write(line.getBytes());
    					ds.flush();
    					ds.close();
    					fs.close();
    				}
    				catch (Exception ex) {
    					Xlog.e("MTKThermalManagerActivity", "writeFile() Exception: " + filePath);
    				}
    			}
    			else
    			{
    				Xlog.d("MTKThermalManagerActivity", "writeFile() Create file fail: " + filePath);
    			}
    		}
    		catch (IOException e)
    		{
    			Xlog.e("MTKThermalManagerActivity", "writeFile() creatFile Exception: " + filePath, e);
    		}
    	}
    }
}
