
package android.app;

import android.os.Binder;
import android.os.Debug;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.util.Singleton;

class ANRManagerProxy implements IANRManager
{
    public ANRManagerProxy(IBinder remote)
    {
        mRemote = remote;
    }

    public IBinder asBinder()
    {
        return mRemote;
    }

    /// M: WNR Debugging Mechanism @{
    public boolean notifyWNR(int pid, String reason) throws RemoteException {
        Log.i("ANRManager", "notifyWNR  pid= " + pid);
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        data.writeInterfaceToken(IANRManager.descriptor);
        data.writeInt(pid);
        data.writeString(reason);
        mRemote.transact(NOTIFY_WNR_TRANSACTION, data, reply, 0);
        reply.readException();
        boolean result = reply.readInt() != 0;
        reply.recycle();
        data.recycle();
        return result;
    }

    /// M: Enhance keydispatching predump
    public void notifyLightWeightANR(int pid, String reason, int message) throws RemoteException {
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        data.writeInterfaceToken(IActivityManager.descriptor);
        data.writeInt(pid);
        data.writeString(reason);
        /// M: Enhance keydispatching predump
        data.writeInt(message);
        mRemote.transact(NOTIFY_LIGHTWEIGHT_ANR_TRANSACTION, data, reply, 0);
        reply.readException();
        reply.recycle();
        data.recycle();
    }
    /// @}
    /// M: Message history v1.5 @{
    public void informMessageDump(String msgInfo, int pid) throws RemoteException {
        Parcel data = Parcel.obtain();
        data.writeInterfaceToken(IANRManager.descriptor);
        data.writeString(msgInfo);
        data.writeInt(pid);
        mRemote.transact(INFORM_MESSAGE_DUMP_TRANSACTION, data, null, IBinder.FLAG_ONEWAY);
        data.recycle();
    }
    /// M: Message history v1.5 @}
    private IBinder mRemote;
}
