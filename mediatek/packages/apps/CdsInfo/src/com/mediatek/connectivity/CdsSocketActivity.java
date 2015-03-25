/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.connectivity;

import android.app.Activity;
import com.mediatek.connectivity.R;
import android.view.View;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.net.ConnectivityManager;

import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Spinner;
import android.widget.Toast;
import android.widget.EditText;
import android.view.View.OnClickListener;

import android.widget.ListView;
import android.widget.SimpleAdapter;
import java.util.ArrayList;
import java.util.HashMap;
import java.net.InetAddress;
import java.util.List;
import java.io.File;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;

public class CdsSocketActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsSocketActivity";
    private Context mContext;
    private Toast mToast;
    private static final String[] DEFAULT_CONN_LIST = new String[]{"Wi-Fi", "Mobile"};
    Spinner mConnSpinner = null;
    private int mSelectConnType = ConnectivityManager.TYPE_MOBILE;
    private EditText mReportPercent = null;
    private ConnectivityManager mConnMgr = null;
    private Button mRefreshBtn = null;
    
    private ListView mSocketListview;
    private TextView mSocketInfo;
    private SimpleAdapter mSocketAdapter;
    ArrayList<HashMap<String,String>> mSocketList = new ArrayList<HashMap<String,String>>();
        
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_socket);

        mContext = this.getBaseContext();

        mSocketListview    = (ListView)findViewById(R.id.SocketListview);
        mSocketAdapter = new SimpleAdapter(
            this,
            mSocketList,
            R.layout.simple_list_item2,
            new String[] { "name","port" },
            new int[] { android.R.id.text1, android.R.id.text2 } );
        mSocketListview.setAdapter( mSocketAdapter );
        
        mRefreshBtn = (Button)this.findViewById(R.id.btn_refresh);
        if(mRefreshBtn != null){
            mRefreshBtn.setOnClickListener(new OnClickListener() {
                public void onClick(View arg0) {
                    listSocketPort();
                }
            });
        }

        mSocketInfo = (TextView) this.findViewById(R.id.socket_info);
        Xlog.i(TAG, "CdsSocketActivity is started");
    }

    @Override
    protected void onResume() {
        listSocketPort();
        showSocketOptionInfo();
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }


    public void onClick(View v) {
        int buttonId = v.getId();
        listSocketPort();
        
    }

    private void showSocketOptionInfo(){
        
        try{
            Socket s = new Socket();
            StringBuilder builder = new StringBuilder("Default Socket Option Info:");
            builder.append("\r\nRX Buf. size:").append(s.getReceiveBufferSize()).append("\r\nTX Buf. size:").append(s.getSendBufferSize()).
            append("\r\nRX Socket Timeout:").append(s.getSoTimeout()).append("\r\nTX Socket Timeout:").append(s.getSoSndTimeout()).
            append("\r\nLinger time:").append(s.getSoLinger()).append("\r\nTCP no delay: ").append(s.getTcpNoDelay()).
            append("\r\nOOBINLINE:").append(s.getOOBInline()).append("\r\nTraffic class: ").append(s.getTrafficClass());
        
            mSocketInfo.setText(builder.toString());
        }catch(Exception e){
            e.printStackTrace();
        }
    }

    private void listSocketPort(){
        try{
            mSocketList.clear();
            accessibleListeningPorts("/proc/net/tcp", true);
            accessibleListeningPorts("/proc/net/tcp6", true);
            accessibleListeningPorts("/proc/net/udp", true);
            accessibleListeningPorts("/proc/net/udp6", true);
            mSocketAdapter.notifyDataSetChanged();
        }catch(Exception e){
            e.printStackTrace();
        }
    }

    private void accessibleListeningPorts(String procFilePath, boolean isTcp) throws IOException {
        String socketEntry = "";
        HashMap<String,String> socketItem;
        String socketName = "";
        String socketInfo = "";
        int    i = 0;
        
        List<ParsedProcEntry> entries = ParsedProcEntry.parse(procFilePath);
        for (ParsedProcEntry entry : entries) {
            String addrPort = entry.localAddress.getHostAddress() + ':' + entry.port;

            if (isPortListening(entry.state, isTcp)) {
                socketItem = new HashMap<String,String>();
                i++;
                String uidname = "" + i;

                if( entry.uid != 0){
                    uidname = mContext.getPackageManager().getNameForUid(entry.uid);
                    socketName = "uid(" + entry.uid + "):" + uidname;
                }else{
                    socketName = "System(" + i + ")";
                }
                
                socketInfo = entry.localAddress.getHostAddress() + ":" + entry.port;

                socketItem.put( "name", socketName );
                socketItem.put( "port", socketInfo );
                mSocketList.add( socketItem );

                Xlog.d(TAG, "length:" + mSocketList.size());
                
                socketEntry = "\nFound port listening on addr="
                        + entry.localAddress.getHostAddress() + ", port="
                        + entry.port + ", UID=" + entry.uid + " in "
                        + procFilePath;

                Xlog.i(TAG, socketEntry);
                
            }
        }
    }

    private boolean isPortListening(String state, boolean isTcp) throws IOException {
        // 0A = TCP_LISTEN from include/net/tcp_states.h
        String listeningState = isTcp ? "0A" : "07";
        return listeningState.equals(state);
    }

    private static class ParsedProcEntry {
        private final InetAddress localAddress;
        private final int port;
        private final String state;
        private final int uid;

        private ParsedProcEntry(InetAddress addr, int port, String state, int uid) {
            this.localAddress = addr;
            this.port = port;
            this.state = state;
            this.uid = uid;
        }


        private static List<ParsedProcEntry> parse(String procFilePath) throws IOException {

            List<ParsedProcEntry> retval = new ArrayList<ParsedProcEntry>();
            /*
            * Sample output of "cat /proc/net/tcp" on emulator:
            *
            * sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  ...
            * 0: 0100007F:13AD 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0   ...
            * 1: 00000000:15B3 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0   ...
            * 2: 0F02000A:15B3 0202000A:CE8A 01 00000000:00000000 00:00000000 00000000     0   ...
            *
            */

            File procFile = new File(procFilePath);
            Scanner scanner = null;
            try {
                scanner = new Scanner(procFile);
                while (scanner.hasNextLine()) {
                    String line = scanner.nextLine().trim();

                    // Skip column headers
                    if (line.startsWith("sl")) {
                        continue;
                    }

                    String[] fields = line.split("\\s+");
                    final int expectedNumColumns = 12;
                    if(fields.length < expectedNumColumns){
                        Xlog.e(TAG, procFilePath + " should have at least " + expectedNumColumns
                            + " columns of output " + fields);
                    }

                    String state = fields[3];
                    int uid = Integer.parseInt(fields[7]);
                    InetAddress localIp = addrToInet(fields[1].split(":")[0]);
                    int localPort = Integer.parseInt(fields[1].split(":")[1], 16);

                    retval.add(new ParsedProcEntry(localIp, localPort, state, uid));
                }
            } finally {
                if (scanner != null) {
                    scanner.close();
                }
            }
            return retval;
        }

        /**
         * Convert a string stored in little endian format to an IP address.
         */
        private static InetAddress addrToInet(String s) throws UnknownHostException {
            int len = s.length();
            if (len != 8 && len != 32) {
                throw new IllegalArgumentException(len + "");
            }
            byte[] retval = new byte[len / 2];

            for (int i = 0; i < len / 2; i += 4) {
                retval[i] = (byte) ((Character.digit(s.charAt(2*i + 6), 16) << 4)
                        + Character.digit(s.charAt(2*i + 7), 16));
                retval[i + 1] = (byte) ((Character.digit(s.charAt(2*i + 4), 16) << 4)
                        + Character.digit(s.charAt(2*i + 5), 16));
                retval[i + 2] = (byte) ((Character.digit(s.charAt(2*i + 2), 16) << 4)
                        + Character.digit(s.charAt(2*i + 3), 16));
                retval[i + 3] = (byte) ((Character.digit(s.charAt(2*i), 16) << 4)
                        + Character.digit(s.charAt(2*i + 1), 16));
            }
            return InetAddress.getByAddress(retval);
        }
    }

}