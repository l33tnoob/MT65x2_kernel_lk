package com.hissage.util.data;

import java.util.ArrayList;

public class NmsImportantList {
    static ArrayList<Short> recordIdList;
    static NmsImportantList mInstance;

    private NmsImportantList() {
        recordIdList = new ArrayList<Short>();
    }

    public static NmsImportantList get() {
        if (null == mInstance) {
            mInstance = new NmsImportantList();
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

    public ArrayList<Short> getIdList() {
        return recordIdList;
    }

    public int size() {
        return recordIdList.size();
    }

    public void clearAll() {
        recordIdList.clear();
    }
}
