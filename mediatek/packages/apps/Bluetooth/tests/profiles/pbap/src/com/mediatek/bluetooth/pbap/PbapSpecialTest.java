package com.mediatek.bluetooth.pbap;

import android.content.ContentValues;
import android.content.Context;
import android.util.Log;

import com.android.bluetooth.pbap.BluetoothVCardComposer;
import com.android.vcard.VCardConfig;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Map;


public class PbapSpecialTest { 
    
    private static final String TAG = "[BT][PBAPUT][PbapSpecialTest]";
    /* Store result path */
    private String mResultPath = null;
    
    private String mLocalName = "PbapSpecialTestLocalName";
    
    private String mLocalNumber = "134288888";
    
    private Map<String, List<ContentValues>> mTestDataMap;
    
    /* Keep the application context */
    private Context mContext;
    
    PbapSpecialTest(Context context) {
        mContext = context;
    }
    
    public String getVcardResult() {
        return mResultPath;
    }
    
    public String testVCardPhoto(boolean vcard21, int type) {
        Class<?> classType = com.android.bluetooth.pbap.BluetoothVCardComposer.class;
        
        BluetoothVCardComposer testVCardComposer = new BluetoothVCardComposer(
                mContext,
                vcard21 ? VCardConfig.VCARD_TYPE_V21_GENERIC : VCardConfig.VCARD_TYPE_V30_GENERIC,
                true,
                ((type != BluetoothPbapPath.FOLDER_TYPE_PB) && (type != BluetoothPbapPath.FOLDER_TYPE_SIM1_PB)),
                true);
        
        StringBuilder builder = new StringBuilder();
        
        try {
            Method method = classType.getDeclaredMethod("appendPhotos", new Class[] {StringBuilder.class, Map.class});
            Log.d(TAG,"method is : " + method);
            method.setAccessible(true);
            method.invoke(testVCardComposer, new Object[]{builder, mTestDataMap});
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        
        return (builder.toString());
    }
    
    public void setTestData(Map<String, List<ContentValues>> testDataMap) {
        mTestDataMap = testDataMap;
    }
    
}
