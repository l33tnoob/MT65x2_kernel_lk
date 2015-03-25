package com.hissage.ui.adapter;

import java.io.IOException;
import java.io.InputStream;

import android.content.res.AssetManager;
import android.content.res.Resources;

public class NmsGetAssetsResourceFileAdapter {
	 public static Resources resources; 
	    public NmsGetAssetsResourceFileAdapter(Resources resources) 
	    { 
	        NmsGetAssetsResourceFileAdapter.resources = resources; 
	    } 
	    public InputStream getFile(String name){ 
	        AssetManager am = NmsGetAssetsResourceFileAdapter.resources.getAssets(); 
	        try { 
	            return am.open(name); 
	        } catch (IOException e) { 
	            e.printStackTrace(); 
	            return null; 
	        } 
	    } 
}
