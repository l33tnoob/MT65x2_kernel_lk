package com.mediatek.datatransfer;

import android.graphics.drawable.Drawable;

public class AppSnippet{
    private Drawable mIcon;
    private CharSequence mName;
    private String mPackageName;
    public String mFileName; //only for restore
    
    public AppSnippet(Drawable icon, CharSequence name, String packageName){
        mIcon = icon;
        mName = name;
        mPackageName = packageName;
    }
    
    public Drawable getIcon(){
        return mIcon;
    }
    
    public CharSequence getName(){
        return mName;
    }
    
    public String getPackageName(){
        return mPackageName;
    }
    
    public void SetFileName(String filename){
        mFileName = filename;
    }
    
    public String getFileName(){
        return mFileName;
    }
}
