package com.mediatek.ppl.test;

import android.content.Context;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.ControlData;

public class MockPplAgent extends IPplAgent.Stub {
    private static final String TAG = "PPL/MockPplAgent";

    private final Context mContext;
    private ControlData mControlData;

    public MockPplAgent(Context context) {
        mContext = context;
        mControlData = new ControlData();
    }

    @Override
    public byte[] readControlData() throws RemoteException {
        return mControlData.encode();
    }

    @Override
    public int writeControlData(byte[] data) throws RemoteException {
        mControlData.decode(data);
        Log.d(TAG, "writeControlData: " + mControlData);
        return 1;
    }

}
