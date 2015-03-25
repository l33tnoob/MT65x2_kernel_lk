package com.mediatek.smsreg;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.HashMap;

import android.os.FileUtils;


import android.util.Log;

public class BlackListUnit {
    public final String TAG = "SmsReg/BlackListUnit";
    public final String blackFilePath ="/data/data/com.mediatek.smsreg/files/blackList.txt";
    public final String blackFileDir ="/data/data/com.mediatek.smsreg/files";
    public static String SLOT_ONE_KEY = "SLOT_1";
    public static String SLOT_TWO_KEY = "SLOT_2";
    private static BlackListUnit sBlackListUnit = null;
    
    public static BlackListUnit getInstance() {
        if (sBlackListUnit == null) {
            sBlackListUnit = new BlackListUnit();
        }
        return sBlackListUnit;
    }
    
    public void isBlackFileReady() {
        try {
            File fblackFilePath = new File(blackFilePath);
            File dir = new File(blackFileDir);
            if (!fblackFilePath.exists()) {
                Log.i(TAG,"!fblackFilePath.exists()");
                if (!dir.exists()) {
                    Log.d(TAG, "there is no /files dir in dm folder");
                    if (dir.mkdir()) {
                        // chmod for recovery access?
                        FileUtils.setPermissions(dir, FileUtils.S_IRWXU | FileUtils.S_IRWXG | FileUtils.S_IXOTH, -1, -1);
                    } else {
                        throw new Error("Failed to create folder in data folder.");
                    }
                }
                HashMap<String,String> saveMap = new HashMap<String,String>();
                saveMap.put(BlackListUnit.SLOT_ONE_KEY, "");
                saveMap.put(BlackListUnit.SLOT_TWO_KEY, "");
                FileOutputStream outStream = new FileOutputStream(blackFilePath);   
                ObjectOutputStream objectOutputStream = new ObjectOutputStream(outStream);
                objectOutputStream.writeObject(saveMap);   
                outStream.close();
                objectOutputStream.close();
            }
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } 
    }
    public void writeObjectToFile(HashMap<String,String> map) {
        try {      
                FileOutputStream outStream = new FileOutputStream(blackFilePath);   
                ObjectOutputStream objectOutputStream = new ObjectOutputStream(outStream);
                objectOutputStream.writeObject(map);   
                outStream.close();   
                objectOutputStream.close();
                Log.i(TAG,"writeObjectToFile successful");  
            } catch (FileNotFoundException e){   
                e.printStackTrace();  
            } catch (IOException e) {   
                e.printStackTrace();  
            } 
    }  

    public HashMap<String,String> readObjectFromfile() {
        
        HashMap<String,String> map = new HashMap<String,String>();
        try {   
                FileInputStream freader;
                freader = new FileInputStream(blackFilePath);   
                ObjectInputStream objectInputStream = new ObjectInputStream(freader);   
                map = (HashMap<String, String>) objectInputStream.readObject();
                objectInputStream.close();
                Log.i(TAG,"The name 1 is " + map.get(SLOT_ONE_KEY));
                Log.i(TAG,"The name 2 is " + map.get(SLOT_TWO_KEY));
        } catch (FileNotFoundException e) {   
                // TODO Auto-generated catch block   
                e.printStackTrace();  
        } catch (IOException e) {   
                // TODO Auto-generated catch block   
                e.printStackTrace();  
        } catch (ClassNotFoundException e) {   
                // TODO Auto-generated catch block  
                e.printStackTrace();  
        }
        return map;
    }
}
