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

import com.orangelabs.rcs.utils.logger.Logger;

import java.security.InvalidAlgorithmParameterException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertPath;
import java.security.cert.CertPathValidator;
import java.security.cert.CertPathValidatorException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.PKIXParameters;
import java.security.cert.TrustAnchor;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Set;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

/**
 * A sub class of {@link X509TrustManager}, helps to add some special key store
 * to trust manager.
 */
public class MsrpX509TrustManager implements X509TrustManager {

    private X509TrustManager mDefaultX509TrustManager = null;
    private static final String ALGORITHM_NAME = "PKIX";
    private static final String CERTIFICATION_TYPE = "X509";
    private PKIXParameters mCustomPKIXParams = null;
    private static KeyStore sCustomTrustedKeyStore = null;
    /** M: Add a data member to contains IP address of the server. @{T-Mobile */
    private static String mServerIP;
    /** @}*/
    /**
     * The logger
     */
    private static Logger sLogger = Logger.getLogger(X509TrustManager.class.getName());

    /**
     * The constructor for EasyX509TrustManager.
     * 
     * @param keystore The trusted key store
     */
    public MsrpX509TrustManager(KeyStore keystore) throws NoSuchAlgorithmException,
            KeyStoreException {
        super();
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory
                .getDefaultAlgorithm());
        trustManagerFactory.init(keystore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();
        if (trustManagers.length == 0) {
            throw new NoSuchAlgorithmException("No trusted manager be found");
        }
        this.mDefaultX509TrustManager = (X509TrustManager) trustManagers[0];
        readCustomTrustedKeyStore();
    }

    /**
     * The constructor for EasyX509TrustManager.
     * 
     * @param sysKeyStore The system default key store, always be null.
     * @param customerKeyStore A custom trusted key store.
     * @throws CertificateException
     */
    public MsrpX509TrustManager(KeyStore sysKeyStore, KeyStore customerKeyStore)
            throws NoSuchAlgorithmException, KeyStoreException, CertificateException {
        super();
        sLogger.debug("MsrpX509TrustManager(KeyStore sysKeyStore, KeyStore customerKeyStore) entry. customerKeyStore = "
                + customerKeyStore);
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory
                .getDefaultAlgorithm());
        trustManagerFactory.init(sysKeyStore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();
        if (trustManagers.length == 0) {
            sLogger.error("no trust manager found");
            throw new NoSuchAlgorithmException("no trust manager found");
        }
        mDefaultX509TrustManager = (X509TrustManager) trustManagers[0];
        readCustomTrustedKeyStore();
    }
    
    /**
     * Check whether the client is trusted
     * 
     * @param certificateList The certificate list
     * @param certificateType The certificate type
     */
    public void checkClientTrusted(X509Certificate[] certificateList, String certificateType)
            throws CertificateException {
        mDefaultX509TrustManager.checkClientTrusted(certificateList, certificateType);
    }

    /**
     * Check whether the server is trusted. Firstly use system's trust key
     * store, if fail then use a custom trusted key store
     * 
     * @param certificateList The certificate list
     * @param certificateType The certificate type
     */
    public void checkServerTrusted(X509Certificate[] certificateList, String certificateType)
            throws CertificateException {
        sLogger.debug("checkServerTrusted() entry()");
        if (certificateList != null && certificateList.length > 0) {
            sLogger.debug("checkServerTrusted():certificateList[0] = " + certificateList[0].toString()
                    + ", certificateType = " + certificateType);
        }
        try {
            mDefaultX509TrustManager.checkServerTrusted(certificateList, certificateType);
            sLogger.debug("The server was verified by system.");
            return;
        } catch (CertificateException e) {
            sLogger.debug("The server was not verified by system, then check the custom keystore.");
        }
        // System key store verified failed, check custom trusted key store
        CertPathValidator certPathValidator;
        try {
            certPathValidator = CertPathValidator.getInstance(ALGORITHM_NAME);
            CertificateFactory certificateFactory = CertificateFactory
                    .getInstance(CERTIFICATION_TYPE);
            CertPath certPath = certificateFactory.generateCertPath(Arrays.asList(certificateList));
            if (mCustomPKIXParams == null) {
                // key store is empty
                sLogger.debug("Custom keystore is empty");
                /**
                 * M: For test purpose ,when custom key store is null,also makes
                 * code continue executing @{T-Mobile
                 * so we need to comment the next sentence:
                 */
                // throw new CertificateException("Custom keystore is empty");
                /** T-Mobile@} */
                throw new CertificateException("Custom keystore is empty");
            }
            try {
                certPathValidator.validate(certPath, mCustomPKIXParams);
                sLogger.debug("The server's certificate is in our custom keystore. It means the server is secure.");
                return;
            } catch (CertPathValidatorException e) {
                sLogger.warn("CertPathValidatorException: " + e.getMessage());
            } catch (InvalidAlgorithmParameterException e) {
                sLogger.warn("InvalidAlgorithmParameterException: " + e.getMessage());
            }
            if (certificateList.length >= 1) {
                byte[] codeArray = certificateList[0].getEncoded();
                /**
                 * M:For test purpose, when PKIXParameters is null(cased by null
                 * custom key store),also make it trust the server @{T-Mobile
                 */
//                if (mCustomPKIXParams == null) {
//                    sLogger.debug("mCustomPKIXParameters is null. Debug..... so trust the server");
//                    return;
//                }
                /** T-Mobile@} */
                Set<TrustAnchor> trustAnchors = mCustomPKIXParams.getTrustAnchors();
                sLogger.debug("Trust anchors:" + Integer.toString(trustAnchors.size()));
                for (TrustAnchor anchor : trustAnchors) {
                    byte[] trusted = anchor.getTrustedCert().getEncoded();
                    if (Arrays.equals(codeArray, trusted)) {
                        sLogger.debug("trust manager verified, that is, the server is trusted by special key store");
                        return;
                    }
                }
                sLogger.debug("trust manager do not verified");
            }
        } catch (NoSuchAlgorithmException e) {
            sLogger.error("NoSuchAlgorithmException: " + e.getMessage());
            e.printStackTrace();
        }
        sLogger.error("checkServerTrusted failed, that is, the server is not secure.");
        // For debug vf test server so delete the below line
        sLogger.error("For debug vf test server, then trust the server.");
        // throw new CertificateException("Untrust server key");
    }

    /**
     *Get the accepted X509 certificate list
     * 
     *@return The accepted X509 certificate list
     */
    public X509Certificate[] getAcceptedIssuers() {
        return mDefaultX509TrustManager.getAcceptedIssuers();
    }

    /**
     * Load a custom key store which contains a special certificate such as
     * vodafone's CA certificate.
     */
    private void readCustomTrustedKeyStore() {
        sLogger.debug("readCustomTrustedKeyStore() entry");
        Set<TrustAnchor> trusted = new HashSet<TrustAnchor>();
        try {
            sLogger.debug("sCustomTrustedKeyStore = " + sCustomTrustedKeyStore);
            if (sCustomTrustedKeyStore == null) {
                sLogger.error("sExtraTrustKeyStore is null");
                return;
            }
            for (Enumeration<String> en = sCustomTrustedKeyStore.aliases(); en.hasMoreElements();) {
                final String alias = en.nextElement();
                sLogger.debug("alias = " + alias);
                final X509Certificate cert = (X509Certificate) sCustomTrustedKeyStore
                        .getCertificate(alias);
                if (cert != null) {
                    sLogger.debug("add certificate to trust");
                    trusted.add(new TrustAnchor(cert, null));
                } else {
                    sLogger.debug("certificate is null");
                }
            }
            mCustomPKIXParams = new PKIXParameters(trusted);
            mCustomPKIXParams.setRevocationEnabled(false);
        } catch (KeyStoreException e) {
            sLogger.error("KeyStoreException: " + e.getMessage());
            e.printStackTrace();
        } catch (InvalidAlgorithmParameterException e) {
            sLogger.error("InvalidAlgorithmParameterException: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Add a extra custom trusted key store. {@link MsrpX509TrustManager} allow
     * you to set only one custom trusted key store. Notice that please call
     * this method before construct a {@link MsrpX509TrustManager} object.
     * 
     * @param keyStore A custom trusted key store.
     */
    public static void setCustomTrustedKeyStore(KeyStore keyStore) {
        sLogger.debug("setCustomTrustedKeyStore(), keyStore = " + keyStore);
        sCustomTrustedKeyStore = keyStore;
    }

    /** M: Add a IP address of the server. It is used to compare with
     *  server certificate Common Name.Notice that please call
     * this method before construct a {@link MsrpX509TrustManager} object.
     * @{T-Mobile */
    public static void setServerIP(String serverIP){
    	mServerIP = serverIP;	
    }
    /** T-Mobile@}*/

}
