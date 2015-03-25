package android.nfc.wps;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/** @hide */
public class PasswordToken implements Parcelable{
    static final String TAG = "PasswordToken";    
    
    public static byte[] mPASSWORDTOKEN_ATTRIBUTE_ID_VERSION = {0x10, 0x4A};
    
    public static byte[] mPASSWORDTOKEN_ATTRIBUTE_ID_OOB_DEVICE_PASSWORD = {0x10, 0x2C};
    
    public static byte[] mPASSWORDTOKEN_ATTRIBUTE_ID_VENDOR_EXTENSION = {0x10, 0x49};
    
    public static byte mPASSWORDTOKEN_VERSION_10 = 0x10;
    
    public static byte mPASSWORDTOKEN_VERSION_20 = 0x20;
    
    /**
     *   Arbitrary value between 0x0010 and 0xFFFF that serves as an identifier for the device password
     *   length :: 2 bytes
     */
    private int mPwdId;
    /**
     *   First 160 bits of the SHA-256 hash of the Enrollee's public key
     *   length :: 20 bytes
     */
    private byte[] mPublicKeyHash; 
    /**
     *   A 16 - 32 octet long device password
     *   length :: 16 bytes
     */
    private byte[] mDevicePwd;
    /**
     *   A 6 octet long vendor d
     *   length :: 6 bytes
     */
    private byte[] mVendorId = new byte[0];
 
    private byte[] mVendorEx;
  
    public PasswordToken(){

    }

    public PasswordToken(Parcel in) {
        Log.d(TAG, "assemble function is called");
        mPwdId = in.readInt();
        Log.d(TAG, "mPwdId = " + mPwdId);
        mPublicKeyHash = new byte[in.readInt()];
        in.readByteArray(mPublicKeyHash);
        mDevicePwd = new byte[in.readInt()];
        in.readByteArray(mDevicePwd);
        mVendorId = new byte[in.readInt()];
        in.readByteArray(mVendorId);
        mVendorEx = new byte[in.readInt()];
        in.readByteArray(mVendorEx);      
    }
    
    public void writeToParcel(Parcel dest, int flags) {
        Log.d(TAG, "writeToParcel is called");
        dest.writeInt(mPwdId);
        Log.d(TAG, "mPwdId = " + mPwdId);
        dest.writeInt(mPublicKeyHash.length);
        dest.writeByteArray(mPublicKeyHash);
        dest.writeInt(mDevicePwd.length);
        dest.writeByteArray(mDevicePwd);
        dest.writeInt(mVendorId.length);
        dest.writeByteArray(mVendorId);
        dest.writeInt(mVendorEx.length);
        dest.writeByteArray(mVendorEx);
    }

    // Getter and setter methods
    public void setPwdId(int pwdId){
    	mPwdId = pwdId; 
    }
    
    public void setPublicKeyHash(byte[] pubKeyHash){
    	mPublicKeyHash = pubKeyHash; 
    }
    
    public void setDevPwd(byte[] devPwd){
    	mDevicePwd = devPwd; 
    }
    
    public void setVendorEx(byte[] vendorEx){
    	mVendorEx = vendorEx; 
    }
    
    public int getPwdId(){
    	return mPwdId; 
    }
    
    public byte[] getPublicKeyHash(){
    	return mPublicKeyHash; 
    }
    
    public byte[] getDevPwd(){
    	return mDevicePwd; 
    }
    
    public byte[] getVendorEx(){
    	return mVendorEx;
    }
    
    public void testPrint(){
    	
    }

    public static final Parcelable.Creator CREATOR = new Parcelable.Creator() {
        public PasswordToken createFromParcel(Parcel in) {
            return new PasswordToken(in); 
        }

        public PasswordToken[] newArray(int size) {
            return new PasswordToken[size];
        }
    };    
     
    
    public int describeContents(){
        return 0;
    }

 }
