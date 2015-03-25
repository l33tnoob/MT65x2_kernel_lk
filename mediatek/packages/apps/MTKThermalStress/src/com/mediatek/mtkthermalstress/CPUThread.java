package com.mediatek.mtkthermalstress;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import com.mediatek.xlog.Xlog;

public class CPUThread extends Thread{
	private final static String mAlgorithm = "MD5";
	MessageDigest mDigester;
	byte[] mDigest;
	boolean isStop;

	CPUThread(){
		mDigester = null;
		try{
			mDigester = MessageDigest.getInstance(mAlgorithm);
		}
		catch(NoSuchAlgorithmException e){
			Xlog.d(Utility.mTag, "Does not support " + mAlgorithm, e);
		}
		isStop = false;
	}

	@Override
	public void run(){
		byte[] bytes = new byte[8192];
		int loopCount = 100;

		if (mDigester == null){
			Xlog.d(Utility.mTag, "Thread ends due to no Digester");
			return;
		}

		Xlog.d(Utility.mTag, "Thread is started");
		while(isStop == false){
			//TODO: Remove this while loop if the thread doesn't have to do anything once a while
			while(loopCount > 0){
				mDigester.update(bytes);
				--loopCount;
			}
			mDigest = mDigester.digest();
		}

		Xlog.d(Utility.mTag, "Thread is terminated");
	}

	public void setStop(){
		isStop = true;
		Xlog.d(Utility.mTag, "Cancelling thread...");
	}
}
