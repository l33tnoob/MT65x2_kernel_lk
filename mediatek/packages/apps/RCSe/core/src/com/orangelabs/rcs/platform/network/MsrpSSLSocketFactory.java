/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.orangelabs.rcs.platform.network;

import com.orangelabs.rcs.core.ims.protocol.sip.KeyStoreManager;
import com.orangelabs.rcs.utils.logger.Logger;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;

import org.apache.http.conn.ConnectTimeoutException;
import org.apache.http.conn.scheme.LayeredSocketFactory;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

/**
 * A sub class of {@link LayeredSocketFactory} helps to create SSL socket.
 */
public class MsrpSSLSocketFactory implements LayeredSocketFactory {

    private static final String PROTOCOL = "TLS";
    private static final String CLIENT_KEY_ALGORITHM = "X509";
    private static final String CLIENT_KEY_KEYSTORE_TYPE = "BKS";
    private static Logger sLogger = Logger.getLogger(MsrpSSLSocketFactory.class.getName());
    private SSLContext mSslcontext = null;

    /**
     * Add a extra custom trusted key store. {@link MsrpSSLSocketFactory} allow
     * you to set only one custom trusted key store. Notice that please call
     * this method before construct a {@link MsrpSSLSocketFactory} object.
     * 
     * @param keyStore A custom trusted key store.
     */
    public static void setCustomTrustedKeyStore(KeyStore keyStore) {
        sLogger.debug("setCustomTrustedKeyStore(), keyStore = " + keyStore);
        MsrpX509TrustManager.setCustomTrustedKeyStore(keyStore);
    }
    
    /** M: Add a IP address of the server. It is used to compare with
     *  server certificate Common Name.Notice that please call
     * this method before construct a {@link MsrpX509TrustManager} object.
     * @{T-Mobile */
    public static void setServerIP(String serverIP){
    	sLogger.debug("setServerIP(), serverIP = " + serverIP);
        MsrpX509TrustManager.setServerIP(serverIP);
    }
    /** T-Mobile@}*/

    /**
     * M: Modify the createMsrpSSLContext(),make it can create success without
     * the key store which contains client certificates used to be verified by
     * server and server certificates used to verify server.Notice that in this
     * function,it will be more clear if we create two key stores,one used to
     * store client certificates,the other one used to store server certificates
     * 
     * @{T-Mobile
     */
    private static SSLContext createMsrpSSLContext() throws IOException {
        sLogger.debug("createMsrpSSLContext()");
        try {
            // identify whether key store contains client certificates is null
            boolean clientKeyStoreIsNull = false;
            // Obtain a SSL context
            SSLContext sslContext = SSLContext.getInstance(PROTOCOL);
            // Obtain a KeyManagerFactory
            KeyManagerFactory keyManagerFactory = KeyManagerFactory
                    .getInstance(CLIENT_KEY_ALGORITHM);
            // Obtain a KeyStore
            KeyStore keyStore = KeyStore.getInstance(CLIENT_KEY_KEYSTORE_TYPE);
            String keySotreName = KeyStoreManager.getSelfSignedKeystorePath();

            try {
                // Open a key store file
                FileInputStream keyStoreStream = new FileInputStream(keySotreName);
                // Add key store to custom key store
                setCustomTrustedKeyStore(keyStore);
                // Load the stream into key store
                keyStore.load(keyStoreStream, KeyStoreManager.KEYSTORE_PASSWORD.toCharArray());
                // Initialize key manager
                keyManagerFactory.init(keyStore, KeyStoreManager.KEYSTORE_PASSWORD.toCharArray());
            } catch (IOException e) {
                clientKeyStoreIsNull = true;
                if (sLogger.isActivated()) {
                    sLogger.debug("Initialize client key sotre faild.Debug.....for test purpose ,accept null client key store");
                }
            }
            // Initialize the SSL context
            if (sLogger.isActivated()) {
                sLogger.debug("Initialize a ssl context, with keymanager");
            }
            if (clientKeyStoreIsNull == true) {
                sslContext.init(null, new TrustManager[] {
                    new MsrpX509TrustManager(null)
                }, null);
            } else {
                sslContext.init(keyManagerFactory.getKeyManagers(), new TrustManager[] {
                    new MsrpX509TrustManager(null)
                }, null);
            }
            return sslContext;
        } catch (NoSuchAlgorithmException e) {
            throw new IOException(e.getMessage());
        } catch (KeyManagementException e) {
            throw new IOException(e.getMessage());
        } catch (KeyStoreException e) {
            throw new IOException(e.getMessage());
        } catch (CertificateException e) {
            throw new IOException(e.getMessage());
        } catch (UnrecoverableKeyException e) {
            throw new IOException(e.getMessage());
        }
    }

    /** T-Mobile@} */

    private SSLContext getSSLContext() throws IOException {
        if (mSslcontext == null) {
            mSslcontext = createMsrpSSLContext();
        }
        return mSslcontext;
    }

    /**
     * M: Code refactoring. @{
     */
    /**
     * Connect to the specify socket server
     * 
     * @param socket The specified socket connect to
     * @param remoteHost The remote host
     * @param remotePort The remote port
     * @param localAddr The local address
     * @param localPort The local port
     * @param httpParams The HTTP parameters
     * @return The connected SSLSocket instance
     */
    public Socket connectSocket(Socket socket, String remoteHost, int remotePort,
            InetAddress localAddr, int localPort, HttpParams httpParams) throws IOException,
            UnknownHostException, ConnectTimeoutException {
        int httpConnTimeout = HttpConnectionParams.getConnectionTimeout(httpParams);
        int httpSoTimeout = HttpConnectionParams.getSoTimeout(httpParams);

        InetSocketAddress remoteAddress = new InetSocketAddress(remoteHost, remotePort);
        SSLSocket sslSocket = null;
        if (socket != null) {
            sslSocket = (SSLSocket) socket;
        } else {
            sslSocket = (SSLSocket) createSocket();
        }

        if ((localAddr != null) || (localPort > 0)) {
            if (localPort < 0) {
                // It means any port
                localPort = 0;
            }
            InetSocketAddress inetSocketAddress = new InetSocketAddress(localAddr, localPort);
            // bind explicitly
            sslSocket.bind(inetSocketAddress);
        }

        sslSocket.connect(remoteAddress, httpConnTimeout);
        sslSocket.setSoTimeout(httpSoTimeout);
        return sslSocket;
    }

    /**
     * Create a Socket instance by using the socket factory
     * 
     * @return Socket the created socket instance
     */
    public Socket createSocket() throws IOException {
        SSLSocketFactory factory = getSSLContext().getSocketFactory();
        if (factory != null) {
    	/** M: Add a IP address of the server. It is used to compare with
         *  server certificate Common Name.Notice that please call
         * this method before construct a {@link MsrpX509TrustManager} object.
         * @{T-Mobile */
    	setServerIP("");
    	/**@{*/
            return factory.createSocket();
        } else {
            sLogger.error("createSocket(), factory is null");
            return null;
        }
    }

    /**
     * Check whether an Socket instance is secure socket
     * 
     * @param socket The socket to be check
     * @return true for secure socket, otherwise return false
     */
    public boolean isSecure(Socket socket) throws IllegalArgumentException {
        if (socket instanceof SSLSocket) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Create an SSLSocket instance over the specified socket that is connected
     * 
     * @param connectedSocket The specified socket that is connected
     * @param remoteHost The remote host
     * @param remotePort The remote port
     * @param autoClose whether the socket supports auto close
     * @return The socket instance
     */
    public Socket createSocket(Socket connectedSocket, String remoteHost, int remotePort, boolean autoClose)
            throws IOException, UnknownHostException {
        SSLSocketFactory factory = getSSLContext().getSocketFactory();
        if (factory != null) {
    	/** M: Add a IP address of the server. It is used to compare with
         *  server certificate Common Name.Notice that please call
         * this method before construct a {@link MsrpX509TrustManager} object.
         * @{T-Mobile */
    	setServerIP(remoteHost);
    	/**@{*/
            return factory.createSocket(connectedSocket, remoteHost, remotePort, autoClose);
        } else {
            sLogger.error("createSocket(Socket,String,int,boolean)-factory is null");
            return null;
        }
    }

    /**
     * Create an SSLSocket instance
     * 
     * @param remoteHost The remote host
     * @param remoteport The remote port
     * @return The socket instance
     */
    public Socket createSocket(String remoteHost, int remotePort) throws IOException,
            UnknownHostException {
        sLogger.debug("createSocket(), remoteHost = " + remoteHost + ", " + "remotePort = "
                + remotePort);
        SSLSocketFactory factory = getSSLContext().getSocketFactory();
        if (factory != null) {
        /** M: Add a IP address of the server. It is used to compare with
         *  server certificate Common Name.Notice that please call
         * this method before construct a {@link MsrpX509TrustManager} object.
         * @{T-Mobile */
    	setServerIP(remoteHost);
    	/**@{*/
            return factory.createSocket(remoteHost, remotePort);
        } else {
            sLogger.error("createSocket(String,int)-factory is null");
            return null;
        }
    }
    /**
     * @}
     */

    public boolean equals(Object obj) {
        return ((obj != null) && obj.getClass().equals(MsrpSSLSocketFactory.class));
    }

    public int hashCode() {
        return MsrpSSLSocketFactory.class.hashCode();
    }
}
