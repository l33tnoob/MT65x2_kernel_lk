package com.mediatek.bluetooth.pbap;
 
import java.util.ArrayList; 
import java.util.HashMap;
 
public class ContactInfo { 

    private static final String TAG = "[BT][PBAPUT][ContactInfo]";
    private String mContactId;
      
    private HashMap<String, String> mCiName = new HashMap<String, String>(); 
    private HashMap<String, Integer> mCiFlagInfo = new HashMap<String, Integer>();
    private String mCiNickName;
    private ArrayList<String> mNoteList = new ArrayList<String>();
    private ArrayList<String> mWebSiteList = new ArrayList<String>();
    private ArrayList<PhoneNumberWithType> mPhoneNumsList = new ArrayList<PhoneNumberWithType>(); 
    private ArrayList<EmailWithType> mEmailList = new ArrayList<EmailWithType>(); 
    private ArrayList<PostalWithType> mPostalList = new ArrayList<PostalWithType>(); 
    private ArrayList<ImWithType> mImList = new ArrayList<ImWithType>(); 
    private ArrayList<EventWithType> mEventList = new ArrayList<EventWithType>();
    private ArrayList<OrganizationWithType> mOrganzationList = new ArrayList<OrganizationWithType>();
     
    public String getContactId() { 
        return mContactId; 
    } 
    public void setContactId(String contactId) { 
        this.mContactId = contactId; 
    } 
    
    public String getCiNickName() {
        return mCiNickName;
    }
    public void setCiNickName(String nickName) {
        mCiNickName = nickName;
    }

    public void setCiNote(String note) {        
        mNoteList.add(note);
    } 
    public ArrayList<String> getCiNoteList() {
        return mNoteList;
    }
    
    public void setCiWebsite(String address) {        
        mWebSiteList.add(address);
    } 
    public ArrayList<String> getCiWebsiteList() {
        return mWebSiteList;
    }
    
    public void setCiFlagInfo(String index, int flagInfo) {
        mCiFlagInfo.put(index, flagInfo);
    }
    public HashMap<String, Integer> getCiFlagInfo() {
        return mCiFlagInfo;
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
        PhoneNumberWithType setPhoneNumber = new PhoneNumberWithType(number, type, lable);
        mPhoneNumsList.add(setPhoneNumber);
        setPhoneNumber = null;
    } 
    public ArrayList<PhoneNumberWithType> getCiPhoneNumList() {
        return mPhoneNumsList;
    }
        
    public void setCiEmail(String address, int type, String lable) { 
        EmailWithType setMailAddress = new EmailWithType(address, type, lable);
        mEmailList.add(setMailAddress);
        setMailAddress = null;
    } 
    public  ArrayList<EmailWithType> getCiEmailList() {
        return mEmailList;
    }
    
    public void setCiPostal(String address, int type, String lable) { 
        PostalWithType setPostal = new PostalWithType(address, type, lable);
        mPostalList.add(setPostal);
        setPostal = null;
    } 
    public ArrayList<PostalWithType> getCiPostalList() {
        return mPostalList;
    }

    public ImWithType getCiIm(int index) { 
        return mImList.get(index);
    } 
    public void setCiIm(String address, int type, String lable) { 
        ImWithType setIm = new ImWithType(address, type, lable);
        mImList.add(setIm);
        setIm = null;
    } 
    public ArrayList<ImWithType> getCiImList() {
        return mImList;
    }
    
    public void setCiEvent(String startDate, int type, String lable) { 
        EventWithType setEvent = new EventWithType(startDate, type, lable);
        mEventList.add(setEvent);
        setEvent = null;
    } 
    public ArrayList<EventWithType> getCiEventList() {
        return mEventList;
    }
    
    public void setCiOrganization(String company, int type, String lable, String title) { 
        OrganizationWithType setOrganization = new OrganizationWithType(company, type, lable, title);
        mOrganzationList.add(setOrganization);
        setOrganization = null;
    } 
    public ArrayList<OrganizationWithType> getCiOrganizationList() {
        return mOrganzationList;
    }

    //inert classes
    public class PhoneNumberWithType {
        private int mType;
        private String mNumber;
        private String mLabel;
        
        public PhoneNumberWithType(String number, int type, String lable) {
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

    public class EmailWithType {
        private int mType;
        private String mAddress;
        private String mLabel;
        
        public EmailWithType(String address, int type, String lable) {
            mType = type;
            mAddress = address;
            mLabel = lable;
        }

        public int getType() {
            return mType;
        }

        public String getAddress() {
            return mAddress;
        }

        public String getLable() {
            return mLabel;
        }
    }

    public class PostalWithType {
        private int mType;
        private String mAddress;
        private String mLabel;
        
        public PostalWithType(String address, int type, String lable) {
            mType = type;
            mAddress = address;
            mLabel = lable;
        }

        public int getType() {
            return mType;
        }

        public String getAddress() {
            return mAddress;
        }

        public String getLable() {
            return mLabel;
        }
    }

    public class ImWithType {
        private int mType;
        private String mAddress;
        private String mLabel;
        
        public ImWithType(String address, int type, String lable) {
            mType = type;
            mAddress = address;
            mLabel = lable;
        }

        public int getType() {
            return mType;
        }

        public String getAddress() {
            return mAddress;
        }

        public String getLable() {
            return mLabel;
        }
    }
    
    public class EventWithType {
        private int mType;
        private String mStartDate;
        private String mLabel;
        
        public EventWithType(String startDate, int type, String lable) {
            mType = type;
            mStartDate = startDate;
            mLabel = lable;
        }

        public int getType() {
            return mType;
        }

        public String getStartDate() {
            return mStartDate;
        }

        public String getLable() {
            return mLabel;
        }
    }
    
    public class OrganizationWithType {
        private int mType;
        private String mCompany;
        private String mLabel;
        private String mTitle;
        
        public OrganizationWithType(String company, int type, String lable, String title) {
            mType = type;
            mCompany = company;
            mLabel = lable;
            mTitle = title;
        }

        public int getType() {
            return mType;
        }

        public String getCompany() {
            return mCompany;
        }

        public String getLable() {
            return mLabel;
        }

        public String getTitle() {
            return mTitle;
        }
    }
 
} 
