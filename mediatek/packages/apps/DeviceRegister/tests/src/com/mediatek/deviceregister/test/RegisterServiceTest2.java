package com.mediatek.deviceregister.test;

import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.test.AndroidTestCase;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.mediatek.common.dm.DmAgent;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.deviceregister.Const;
import com.mediatek.deviceregister.RegisterService;

public class RegisterServiceTest2 extends AndroidTestCase {

    private final String SUB_TAG = Const.TAG + "RegisterServiceTest";

    public void testFirstRegister() throws Exception {
        Log.d(SUB_TAG, "testFirstRegister");
        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent agent = DmAgent.Stub.asInterface(binder);
        Log.d(SUB_TAG, "set register flag to 0");
        String registerFlag = "0";
        agent.setRegisterFlag(registerFlag.getBytes(), registerFlag.getBytes().length);

        Intent feasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        Log.d(SUB_TAG, "start service");
        feasibleIntent.setClass(mContext, RegisterService.class);
        mContext.startService(feasibleIntent);

        // wait for check the register result
        Log.d(SUB_TAG, "sleep 3 min...");
        Thread.sleep(2 * 60 * 1000);
        Log.d(SUB_TAG, "read register flag");
        String registerFlagRead = new String(agent.readRegisterFlag());
        assertEquals("1", registerFlagRead);
    }

    public void testHaveRegistered() throws Exception {
        Log.d(SUB_TAG, "testHaveRegistered");
        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent agent = DmAgent.Stub.asInterface(binder);
        String registerFlag = "1";
        agent.setRegisterFlag(registerFlag.getBytes(), registerFlag.getBytes().length);

        if (!CommonFunction.writeImsiToDevice()) {
            fail(SUB_TAG + ":write imsi failed!");

        }

        if (!CommonFunction.writeEsnToUIM()) {
            fail(SUB_TAG + ":write pEsn failed!");
        }

        Intent feasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        Log.d(SUB_TAG, "start service");
        feasibleIntent.setClass(mContext, RegisterService.class);
        mContext.startService(feasibleIntent);

        // wait for check the register result
        Thread.sleep(2 * 60 * 1000);
        String registerFlagRead = new String(agent.readRegisterFlag());
        assertEquals("1", registerFlagRead);
    }

    public void testImsiNotSame() throws Exception {
        Log.d(SUB_TAG, "testImsiNotSame");
        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent agent = DmAgent.Stub.asInterface(binder);
        String registerFlag = "1";
        agent.setRegisterFlag(registerFlag.getBytes(), registerFlag.getBytes().length);
        String imsi1 = "012345676543210";
        agent.writeImsi1(imsi1.getBytes(), imsi1.getBytes().length);
        Intent feasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        Log.d(SUB_TAG, "start service");
        feasibleIntent.setClass(mContext, RegisterService.class);
        mContext.startService(feasibleIntent);
        // wait for check the register result
        Thread.sleep(3 * 60 * 1000);
        String registerFlagRead = new String(agent.readRegisterFlag());
        assertEquals("1", registerFlagRead);
        String imsiRead = new String(agent.readImsi1());
        String[] imsiArr = CommonFunction.getImsiFromUIM();
        assertEquals(imsiRead, imsiArr[0]);
    }

    public void testMeidNotSame() throws Exception {
        Log.d(SUB_TAG, "testMeidNotSame");
        IBinder binder = ServiceManager.getService("DmAgent");
        DmAgent agent = DmAgent.Stub.asInterface(binder);
        String registerFlag = "1";
        agent.setRegisterFlag(registerFlag.getBytes(), registerFlag.getBytes().length);
        
        int writeCommand = 222;
        int fileId = 0x6F38;
        String path = "3F007F25";
        String pEsn = "04000000";
        Log.d(SUB_TAG, "write a fake pEsn:"+pEsn);
        ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        byte[] writeReturn = iTel.transmitIccSimIoEx(fileId, writeCommand, 0, 0, 8, path, pEsn, null, 0);
        Log.d(SUB_TAG, "Write pEsn result:" + CommonFunction.bytesToHexString(writeReturn));
        
        Intent feasibleIntent = new Intent(Const.ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE);
        Log.d(SUB_TAG, "start service");
        feasibleIntent.setClass(mContext, RegisterService.class);
        mContext.startService(feasibleIntent);

        // wait for check the register result
        Thread.sleep(3 * 60 * 1000);
        String registerFlagRead = new String(agent.readRegisterFlag());
        assertEquals("1", registerFlagRead);
        String pESN = CommonFunction.meidToESN(CommonFunction.getDeviceId());
        String meidUIM = CommonFunction.getEsnFromUIM();
        assertEquals(pESN, meidUIM);
    }
    
}
