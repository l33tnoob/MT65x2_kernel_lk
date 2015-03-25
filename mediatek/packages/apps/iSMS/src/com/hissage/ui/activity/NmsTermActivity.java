package com.hissage.ui.activity;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;


import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.api.NmsiSMSApi;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
//M: Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImFlag;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImReadMode;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.platfrom.NmsMtkSettings;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.ui.adapter.NmsGetAssetsResourceFileAdapter;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;
//M:Activation Statistics
import com.hissage.util.statistics.NmsStatistics;


import android.R.integer;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

public class NmsTermActivity extends Activity implements OnClickListener {

    private static final String TAG = "NmsTermActivity";
    private final static int HANDLER_READ_FILE = 0;
   
    private long mSim_id = NmsConsts.INVALID_SIM_ID;
    private Button mReject = null;
    private Button mAgree = null;
    private TextView mMainTitle = null;
    private StringBuffer mStringBuffer = new StringBuffer();
    private static final int IPMSG_SERVICE_ENABLED = 0;
    private static final int IPMSG_SERVICE_DISABLED = 1;
    private static final int IPMSG_SERVICE_UNACTIVATED = 2;
    
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
        getActionBar().setTitle(R.string.STR_NMS_MENU_LICENSE_AGREEMENT);

        setContentView(R.layout.term);

        mMainTitle= (TextView) findViewById(R.id.detail_content);
       
        try{
        	getFileContentForSetText();
        }catch (Exception e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        } finally {
            //
        }
        
        Intent i = getIntent();
        if (i == null) {
            NmsLog.error(TAG, "not find sim_id at intent: " + i);
            finish();
            return;
        }
        
        mAgree = (Button)findViewById(R.id.agree);
        mAgree.setOnClickListener(this);
        mReject = (Button)findViewById(R.id.reject);
        mReject.setOnClickListener(this);

    }

    @Override
    public void onClick(View v) {
        if(v == mAgree){
			//M: Activation Statistics
            checkIpMessageServiceStatusForUserLicense();
        }else if(v == mReject){
            finish();
        }else{
            // do nothing
        }
    }
    
    private static void startMyActivity(Context context,int id){
        NmsStartActivityApi.nmsStartActivitionActivity(
                context, id, 7);
        }
   
    private class SNmsSimInfoMTK {
        long sim_id = NmsConsts.INVALID_SIM_ID;
        String simName = null;
        String phone = null;
        int color = -1;
        boolean simEnable = false;
    }

    private static SNmsSimInfoMTK[] mSimInfo = new SNmsSimInfoMTK[NmsConsts.SIM_CARD_COUNT];
    private NmsMtkSettings nmsMtkSettings = null;

    private void getAllSimInfo() {

        for (int i = 0; i < NmsConsts.SIM_CARD_COUNT; ++i) {
            if (null == mSimInfo[i]) {
                mSimInfo[i] = new SNmsSimInfoMTK();
            }
            mSimInfo[i].sim_id = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(i);
            if (mSimInfo[i].sim_id > 0) {
                mSimInfo[i].simName = NmsPlatformAdapter.getInstance(this).getSimName(
                        mSimInfo[i].sim_id);
                mSimInfo[i].color = NmsPlatformAdapter.getInstance(this).getSimColor(
                        mSimInfo[i].sim_id);
                SNmsSimInfo sim = NmsIpMessageApiNative
                        .nmsGetSimInfoViaSimId((int) mSimInfo[i].sim_id);
                if (null != sim) {
                    mSimInfo[i].simEnable = NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED == sim.status;
                    mSimInfo[i].phone = sim.number;
                } else {
                    // do nothing
                }
            } else {
                // do nothing
            }
        }
    }

    // /M: add for ipmessage for activate from UserLicense{@
    public boolean checkIpMessageServiceStatusForUserLicense() {
        getAllSimInfo();
        int simID = -1;
        nmsMtkSettings = new NmsMtkSettings(this);
        if (mSimInfo.length == 0) {
            // / M: add for IP message, No SIM
            String content = this.getResources().getString(R.string.imsp_no_sim_card);
            Toast.makeText(this, content, Toast.LENGTH_LONG).show();
        } else if (mSimInfo.length == 1) {
            // / M: add for IP message, one SIM
            simID = (int) mSimInfo[0].sim_id;
        } else {

            if (nmsMtkSettings.isDefultSimNotSet(NmsTermActivity.this)) {
                NmsAlertDialogUtils.showSelectSimCardDialog(this, this,
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                // TODO Auto-generated method stub
                                List<Map<String, Object>> entries = (List<Map<String, Object>>) NmsAlertDialogUtils
                                        .getList();
                                HashMap<String, Object> entry = (HashMap<String, Object>) entries
                                        .get(which);

                                if (((Integer) entry.get("simStatus")) == 0) {
                                    NmsTermActivity.startMyActivity(NmsTermActivity.this,
                                            ((Integer) entry.get("simID")));
                                    finish();
                                } else {
                                    Toast.makeText(
                                            NmsTermActivity.this,
                                            NmsTermActivity.this.getResources().getString(
                                                    R.string.imsp_current_sim_enabled),
                                            Toast.LENGTH_LONG).show();
                                    finish();
                                }
                            }
                        });
                return true;
            } else {
                simID = (int) nmsMtkSettings.getCurrentSimId(NmsTermActivity.this);
            }
        }
        NmsTermActivity.startMyActivity(NmsTermActivity.this, simID);
        finish();
        return true;
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
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_zh_CN.txt";
            }else if(languageCode.endsWith("zh") && countryCode.endsWith("TW")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_zh_TW.txt";
            }else if(languageCode.endsWith("en")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_en.txt";
            }else if(languageCode.endsWith("ar")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_ar.txt";
            }else if(languageCode.endsWith("eu")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_eu.txt";
            }else if(languageCode.endsWith("bg")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_bg.txt";
            }else if(languageCode.endsWith("ca")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_ca.txt";
            }else if(languageCode.endsWith("hr")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_hr.txt";
            }else if(languageCode.endsWith("cs")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_cs.txt";
            }else if(languageCode.endsWith("da")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_da.txt";
            }else if(languageCode.endsWith("de")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_de.txt";
            }else if(languageCode.endsWith("es") && countryCode.endsWith("ES")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_es_ES.txt";
            }else if(languageCode.endsWith("es") && countryCode.endsWith("US")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_es_US.txt";
            }else if(languageCode.endsWith("et")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_et.txt";
            }else if(languageCode.endsWith("fa")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_fa.txt";
            }else if(languageCode.endsWith("fi")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_fi.txt";
            }else if(languageCode.endsWith("gl")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_gl.txt";
            }else if(languageCode.endsWith("el")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_el.txt";
            }else if(languageCode.endsWith("hi")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_hi.txt";
            }else if(languageCode.endsWith("hu")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_hu.txt";
            }else if(languageCode.endsWith("in")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_in.txt";
            }else if(languageCode.endsWith("it")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_it.txt";
            }else if(languageCode.endsWith("ja")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_ja.txt";
            }else if(languageCode.endsWith("lv")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_lv.txt";
            }else if(languageCode.endsWith("lt")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_lt.txt";
            }else if(languageCode.endsWith("mk")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_mk.txt";
            }else if(languageCode.endsWith("ms")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_ms.txt";
            }else if(languageCode.endsWith("nl")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_nl.txt";
            }else if(languageCode.endsWith("nb")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_nb.txt";
            }else if(languageCode.endsWith("pl")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_pl.txt";
            }else if(languageCode.endsWith("pt") && countryCode.endsWith("BR")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_pt_BR.txt";
            }else if(languageCode.endsWith("pt") && countryCode.endsWith("PT")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_pt_PT.txt";
            }else if(languageCode.endsWith("ro")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_ro.txt";
            }else if(languageCode.endsWith("ru")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_ru.txt";
            }else if(languageCode.endsWith("sr")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_sr.txt";
            }else if(languageCode.endsWith("sk")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_sk.txt";
            }else if(languageCode.endsWith("sl")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_sl.txt";
            }else if(languageCode.endsWith("sv")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_sv.txt";
            }else if(languageCode.endsWith("th")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_th.txt";
            }else if(languageCode.endsWith("tr")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_tr.txt";
            }else if(languageCode.endsWith("uk")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_uk.txt";
            }else if(languageCode.endsWith("vi")){
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_vi.txt";
            }else{
                languageTypeFileName = "NMS_LICENSE_AGREEMENT_DETAIL_en.txt";
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
	     }finally {
	            //
	     }
		//M:Activation Statistics
        NmsStatistics.incKeyVal(NmsStatistics.KEY_OTHER_ACTIVATE_PROMPT) ;
    }
}
