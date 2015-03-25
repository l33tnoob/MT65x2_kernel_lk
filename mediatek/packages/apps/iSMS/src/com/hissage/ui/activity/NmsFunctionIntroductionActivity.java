package com.hissage.ui.activity;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Locale;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.MenuItem;
import android.widget.TextView;

import com.hissage.ui.adapter.NmsGetAssetsResourceFileAdapter;
import com.hissage.util.log.NmsLog;
import com.hissage.R;

public class NmsFunctionIntroductionActivity extends Activity{

		private static final String TAG = null;
		private TextView mMainTitle;
		private final static int HANDLER_READ_FILE = 0;
	    private StringBuffer mStringBuffer = new StringBuffer();
	    
	    private Handler mHandler = new Handler() {
	        @Override
	        public void handleMessage(Message msg) {
	            super.handleMessage(msg);

	            NmsLog.trace(TAG, "handler received msg type: " + msg.what);
	            switch (msg.what) {
	            case HANDLER_READ_FILE:
	                mMainTitle.setText(mStringBuffer);
	                return;
	            default:
	            	//do nothing
	                break;
	            }
	        }

	    };
	    
	 @Override
	    public boolean onMenuItemSelected(int featureId, MenuItem item) {
	        int id = item.getItemId();
	        switch (id) {
	        case android.R.id.home: {
	            finish();
	            break;
	        }
	        }
	        return super.onMenuItemSelected(featureId, item);
	    }


	    public void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
	        getActionBar().setDisplayHomeAsUpEnabled(true);
	        getActionBar().setDisplayShowHomeEnabled(true);
	        getActionBar().setLogo(R.drawable.isms);
	        getActionBar().setTitle(R.string.STR_NMS_MENU_FUNCTION_INTRODUCTION);
	        
	        setContentView(R.layout.function_introduction);
	        mMainTitle= (TextView) findViewById(R.id.function_content);
	        try{
	        	getFileContentForSetText();
	        }catch (Exception e) {
	            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
	        } finally {
	            //
	        }
	        
	    }
	    private void getFileContentForSetText() {
	        NmsLog.trace(TAG, "thread to get file content");

	        new Thread() {
	            @Override
	            public void run() {
	                super.run();
	                
	                NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_READ_FILE);
	                readFileContent();
                	mHandler.sendEmptyMessage(HANDLER_READ_FILE);
	                /*
	                if(mMainTitle != null && mMainTitle.getWidth()>0 && mMainTitle.getHeight()>0){
	                	readFileContent();
	                	mHandler.sendEmptyMessage(HANDLER_READ_FILE);
	                }else{
	            		mHandler.postDelayed(this, 50);
	    			}
	    			*/
	                return;
	            }
	        }.start();
	    }
	    
	    private void readFileContent(){
	    	try{
	    		   Locale locale = getResources().getConfiguration().locale;
	   	        String languageCode = locale.getLanguage();
	   	        String countryCode = locale.getCountry();
	   	        String languageTypeFileName = null;
	   	        if (languageCode.endsWith("zh") && countryCode.endsWith("CN")){ // zh_CN not include zh_TW.
	   	        	languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_zh_CN.txt";
	   	        }else if(languageCode.endsWith("zh") && countryCode.endsWith("TW")){
	   	        	languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_zh_TW.txt";
	   	        }else if(languageCode.endsWith("en")){
	   	        	languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_en.txt";
	   	        }else if(languageCode.endsWith("ar")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_ar.txt";
                }else if(languageCode.endsWith("eu")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_eu.txt";
                }else if(languageCode.endsWith("bg")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_bg.txt";
                }else if(languageCode.endsWith("ca")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_ca.txt";
                }else if(languageCode.endsWith("hr")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_hr.txt";
                }else if(languageCode.endsWith("cs")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_cs.txt";
                }else if(languageCode.endsWith("da")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_da.txt";
                }else if(languageCode.endsWith("de")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_de.txt";
                }else if(languageCode.endsWith("es") && countryCode.endsWith("ES")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_es_ES.txt";
                }else if(languageCode.endsWith("es") && countryCode.endsWith("US")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_es_US.txt";
                }else if(languageCode.endsWith("et")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_et.txt";
                }else if(languageCode.endsWith("fa")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_fa.txt";
                }else if(languageCode.endsWith("fi")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_fi.txt";
                }else if(languageCode.endsWith("gl")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_gl.txt";
                }else if(languageCode.endsWith("el")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_el.txt";
                }else if(languageCode.endsWith("hi")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_hi.txt";
                }else if(languageCode.endsWith("hu")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_hu.txt";
                }else if(languageCode.endsWith("in")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_in.txt";
                }else if(languageCode.endsWith("it")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_it.txt";
                }else if(languageCode.endsWith("ja")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_ja.txt";
                }else if(languageCode.endsWith("lv")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_lv.txt";
                }else if(languageCode.endsWith("lt")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_lt.txt";
                }else if(languageCode.endsWith("mk")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_mk.txt";
                }else if(languageCode.endsWith("ms")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_ms.txt";
                }else if(languageCode.endsWith("nl")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_nl.txt";
                }else if(languageCode.endsWith("nb")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_nb.txt";
                }else if(languageCode.endsWith("pl")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_pl.txt";
                }else if(languageCode.endsWith("pt") && countryCode.endsWith("BR")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_pt_BR.txt";
                }else if(languageCode.endsWith("pt") && countryCode.endsWith("PT")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_pt_PT.txt";
                }else if(languageCode.endsWith("ro")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_ro.txt";
                }else if(languageCode.endsWith("ru")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_ru.txt";
                }else if(languageCode.endsWith("sr")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_sr.txt";
                }else if(languageCode.endsWith("sk")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_sk.txt";
                }else if(languageCode.endsWith("sl")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_sl.txt";
                }else if(languageCode.endsWith("sv")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_sv.txt";
                }else if(languageCode.endsWith("th")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_th.txt";
                }else if(languageCode.endsWith("tr")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_tr.txt";
                }else if(languageCode.endsWith("uk")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_uk.txt";
                }else if(languageCode.endsWith("vi")){
                    languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_vi.txt";
                }else{
	   	        	languageTypeFileName = "NMS_FUNCTION_INTRODUCTION_CONTENT_en.txt";
	   	        }
	   	        NmsGetAssetsResourceFileAdapter gLFile = new NmsGetAssetsResourceFileAdapter(getResources());
	   	        BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(gLFile.getFile(languageTypeFileName)));
	               String res = null;
	               while ((res = bufferedReader.readLine()) != null) {
	               	mStringBuffer.append(res);
	               	mStringBuffer.append("\n");
	               }
		        }catch (Exception e) {
		            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
		        } finally {
		            //
		        }
	    }

}
