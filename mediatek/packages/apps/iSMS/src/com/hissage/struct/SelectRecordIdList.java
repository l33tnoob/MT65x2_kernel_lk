package com.hissage.struct;

import java.util.ArrayList;

public class SelectRecordIdList {
    static ArrayList<Short> recordIdList;
    static SelectRecordIdList mInstance;
    static int importantCount = 0;
    
    static ArrayList<Short> downloadList;
    static int downloadCount = 0;

    private SelectRecordIdList() {
        recordIdList = new ArrayList<Short>();
        
        downloadList = new ArrayList<Short>();
    }

    public static SelectRecordIdList get() {
        if (null == mInstance) {
            mInstance = new SelectRecordIdList();
        }
        return mInstance;
    }

    public void addElement(short dbId) {
        recordIdList.add(dbId);
    }

    public void removeElement(short dbId) {
        int index = recordIdList.indexOf(dbId);
        recordIdList.remove(index);
    }

    public short getElement(int index) {
        return recordIdList.get(index);
    }

    public boolean isContains(short dbId) {
        return recordIdList.contains(dbId);
    }

    public short[] getIdList() {
        int count = recordIdList.size();
        short[] list = new short[count];
        int i = 0;
        for (Short id : recordIdList) {
            list[i] = id;
            i++;
        }
        return list;
    }

    public int getSelectCount() {
        return recordIdList.size();
    }
    
    public void addImportantCount(){
        importantCount++;
    }
    
    public void setImportantCount(int count){
        importantCount = count;
    }
    
    public void reduceImportantCount(){
        importantCount--;
    }
    
    public int getImportantCount(){
        return importantCount;
    }
    public boolean show(){
        return importantCount > 0 ? true : false;
    }

    public void clearAll() {
        recordIdList.clear();
    }
    
    public void addDownloadId(short dbId){
        downloadList.add(dbId);
        downloadCount++;
    }
    
    public void removeDownloadId(short dbId){
        int index = downloadList.indexOf(dbId);
        if(index >= 0){
            downloadList.remove(index);
            downloadCount--;
        }
    }
    public boolean isDownloadContains(short dbId) {
        return downloadList.contains(dbId);
    }
    public short getDownloadId(int index){
        return downloadList.get(index);
    }
    
    public int getDownloadIndex(){
        int index = -1;
        if(downloadCount > 0){
            index = downloadList.size()-downloadCount;
        }
        return index;
    }
    
    public int getDownloadSize(){
        return downloadList.size();
    }
    public void reduceDownloadCount(short dbId){
        if(downloadList.indexOf(dbId) >= 0){
            downloadCount--;
        }
    }
    
    public int getDownloadCount(){
        return downloadCount;
    }
    
    public void clearDownloadList(){
        downloadList.clear();
        downloadCount = 0;
    }
    
    public String DownloadListString(){
        return downloadList.toString();
    }
}
