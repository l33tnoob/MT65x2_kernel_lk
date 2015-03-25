package com.mediatek.bluetooth.pbap;
 
import java.util.ArrayList; 
import java.util.HashMap;
 
public class ContactSimInfo {

    private static final String TAG = "[BT][PBAPUT][ContactInfo]";
    private String mContactId;
      
    private HashMap<String, String> mCiName = new HashMap<String, String>(); 
    private ArrayList<PhoneNumberSimWithType> mPhoneNumsList = new ArrayList<PhoneNumberSimWithType>();
     
    public String getContactId() { 
        return mContactId; 
    } 
    public void setContactId(String contactId) { 
        this.mContactId = contactId; 
    } 
    
    public String getCiName(String index) { 
        return mCiName.get(index); 
    } 
    public void setCiName(String index, String ciName) { 
        mCiName.put(index, ciName);
    } 
    public int getCiNameSize() { 
        return mCiName.size();
    } 
    public HashMap<String, String> getCiNameMapObj() {
        return mCiName;
    }

    public void setCiPhoneNums(String number, int type, String lable) { 
        PhoneNumberSimWithType setPhoneNumber = new PhoneNumberSimWithType(number, type, lable);
        mPhoneNumsList.add(setPhoneNumber);
        setPhoneNumber = null;
    } 
    public ArrayList<PhoneNumberSimWithType> getCiPhoneNumList() {
        return mPhoneNumsList;
    }

    //inert classes
    public class PhoneNumberSimWithType {
        private int mType;
        private String mNumber;
        private String mLabel;
        
        public PhoneNumberSimWithType(String number, int type, String lable) {
            mType = type;
            mNumber = number;
            mLabel = lable;
        }

        public int getType() {
            return mType;
        }

        public String getNumber() {
            return mNumber;
        }

        public String getLable() {
            return mLabel;
        }

    }
    
 
} 
