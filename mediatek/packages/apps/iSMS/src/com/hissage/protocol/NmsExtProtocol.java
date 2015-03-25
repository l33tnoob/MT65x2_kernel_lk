package com.hissage.protocol;

import java.util.ArrayList;
import java.util.List;

import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.CoreConnectionPNames;
import org.apache.http.protocol.HTTP;
import org.apache.http.util.EntityUtils;

import com.hissage.R;
import com.hissage.service.NmsService;
import com.hissage.util.log.NmsLog;


public class NmsExtProtocol {
    
    static final private String TAG = "protocol" ;
    
    static final private int NMS_STATUS_TRY_COUNT = 3 ;
    static final private int NMS_STATUS_SLEET_TIME = 10 ;
    static final private int NETWORK_TIMEOUT = 15 ;
    
    
    static private boolean doSendStatus(String imsi, String number, int clientSessionId, int serverSessionId) {
        
        try {
            List<NameValuePair> listPair = new ArrayList<NameValuePair>(4);
            listPair.add(new BasicNameValuePair("imsi", imsi));
            listPair.add(new BasicNameValuePair("phone", number));
            listPair.add(new BasicNameValuePair("clientSessionId", String.valueOf(clientSessionId)));
            listPair.add(new BasicNameValuePair("serverSessionId", String.valueOf(serverSessionId)));
            listPair.add(new BasicNameValuePair("version", NmsService.getInstance().getString(R.string.STR_NMS_VERSION_ID)));
            
            HttpPost httpPost = new HttpPost(NmsService.getInstance().getString(R.string.STR_NMS_OFFLINE_URL));
            
            httpPost.setEntity(new UrlEncodedFormEntity(listPair));

            HttpClient httpClient = new DefaultHttpClient();
            httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT, NETWORK_TIMEOUT * 1000);
            httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT, NETWORK_TIMEOUT * 1000);

            HttpResponse httpResp = httpClient.execute(httpPost);

            if (httpResp.getStatusLine().getStatusCode() != 200) {
                NmsLog.warn(TAG, "get request reuslt form server error: "
                        + httpResp.getStatusLine().getStatusCode());
                return false;
            }
            
            NmsLog.trace(TAG, "get result: " + EntityUtils.toString(httpResp.getEntity(), HTTP.UTF_8)) ;
            
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
            return false ;
        }
        
        return true ;
    }
    
    static public void sendOfflineStatusToServer(final String imsi, final String number, final int clientSessionId, final int serverSessionId) {
        
        try {
            NmsLog.trace(TAG, String.format("try to send status imsi: %s, number:%s, clientSessionId: %d, serverSessionId: %d, to server", imsi, number, clientSessionId, serverSessionId)) ;
            
            new Thread( new Runnable() {
                
                @Override
                public void run() {
                    
                    int tryTime = 0 ;
                    for (tryTime = 0; tryTime < NMS_STATUS_TRY_COUNT; tryTime++) {
                        try {
                            if (doSendStatus(imsi, number, clientSessionId, serverSessionId))
                                break ;
                        
                            Thread.sleep(NMS_STATUS_SLEET_TIME * 1000) ;
                        } catch (InterruptedException e) {
                            NmsLog.warn(TAG, "get exception in run function: " + e.toString()) ;
                        }
                    } 
                    
                    if (tryTime == NMS_STATUS_TRY_COUNT) 
                        NmsLog.warn(TAG, "failed to send status to server") ;
                }
            }
            ).start() ;
            
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
    }
    
}
