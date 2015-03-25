package com.mediatek.bluetooth.pbap; 
 
public class CallHistory { 
 
    private int mChCallTpye; 
    private String mChCalledNumber;
    
    private long mChCallDate; 
    private long mChCallDuration;
    private boolean mIfNew;
    
    private String mChNumberName; 
    private int mChNumberType; 
    private String mChNumberLable; 
    
    private boolean mIsRead;
     
    public CallHistory() { 
        
    }
    
    public int getChCallType() { 
        return mChCallTpye; 
    } 
    public void setChCallType(int callType) { 
        this.mChCallTpye = callType; 
    } 
    
    public String getChCalledNumber() { 
        return mChCalledNumber; 
    } 
    public void setChCalledNumber(String chCalledNumber) { 
        this.mChCalledNumber = chCalledNumber; 
    } 
    
    public long getChCalledDate() { 
        return mChCallDate; 
    } 
    public void setChCalledDate(long chCalledDate) { 
        this.mChCallDate = chCalledDate; 
    } 
    
    public long getChCalledDuration() { 
        return mChCallDuration; 
    } 
    public void setChCalledDuration(long duration) { 
        this.mChCallDuration = duration; 
    } 
    
    public boolean getChCalledNew() { 
        return mIfNew; 
    } 
    public void setChCalledNew(boolean ifNew) { 
        this.mIfNew = ifNew; 
    }
    
    public String getChNumberName() { 
        return mChNumberName; 
    } 
    public void setChNumberName(String name) { 
        this.mChNumberName = name; 
    } 
    
    public int getChNumberType() {
        return mChNumberType;
    }
    
    public void setChNumberType(int type) {
        mChNumberType = type;
    }
    
    public String getChNumberLable() { 
        return mChNumberLable; 
    } 
    public void setChNumberLable(String lable) { 
        this.mChNumberLable = lable; 
    } 
    
    public boolean getChCalledIsRead() { 
        return mIsRead; 
    } 
    public void setChCalledIsRead(boolean isRead) { 
        this.mIsRead = isRead; 
    }
    

} 
