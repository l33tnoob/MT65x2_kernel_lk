/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.protocol.sip;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.sql.Date;

import com.android.org.bouncycastle.x509.X509V3CertificateGenerator;
import com.android.org.bouncycastle.jce.X509Principal;
/**
 * KeyStore manager for secure connection.
 * 
 * @author B. JOGUET
 */
@SuppressWarnings("deprecation")
public class KeyStoreManager {
    /**
     * M: Modified and added for achieving MSRPoTLS related feature. @{
     */
    /**
     * Keystore name
     */
    public final static String KEYSTORE_NAME = "server.bks";
    /**
     * Client self signed keystore
     */
    public final static String CLIENT_KEYSTORE = "client.bks";
    /**
     * Keystore password
     */
    public final static String KEYSTORE_PASSWORD = "1234567";

    /**
     * Keystore type
     */
    public final static String KEYSTORE_TYPE = KeyStore.getDefaultType();

    /**
     * Private key alias
     */
    public final static String PRIVATE_KEY_ALIAS = "MyPrivateKey";

    private static Logger logger = Logger.getLogger(KeyStoreManager.class.getName());

    private static final String X509_NAME = "CN=Mediatek, OU=None, O=None, L=None, C=None";
    private static final String SIGNATURE_ALGORITHM = "MD5WithRSAEncryption";
    private static final String DIGEST_ALGORITHM = "SHA1";
    private static final String GENERATOR_RSA = "RSA";
    private static final String AlGORITHM_SHA = "SHA-1 ";
    private static String sFingerPrint = null;
    private static char[] HEX_CHARS = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    /**
     * Get finger print.
     * 
     * @return The finger print.
     */
    public static String getFingerPrint() {
        return sFingerPrint;
    }

    /**
     * returns self-signed keystore path.
     * 
     * @return keystore path
     */
    public static String getSelfSignedKeystorePath() {
        return AndroidFactory.getApplicationContext().getFilesDir().getAbsolutePath() + "/"
                + CLIENT_KEYSTORE;
    }
    /**
     * @}
     */
    
    /**
     * returns keystore path.
     * 
     * @return keystore path
     */
    public static String getKeystorePath() {
        return AndroidFactory.getApplicationContext().getFilesDir().getAbsolutePath() + "/"
                + KEYSTORE_NAME;
    }

    /**
     * Test if a keystore is created.
     * 
     * @return true if already created.
     */
    public static boolean isKeystoreExists(String path) throws KeyStoreManagerException {
        // Test file 
        File file = new File(path);
        if ((file == null) || (!file.exists()))
            return false;
        
        // Test keystore
        FileInputStream fis = null;
        boolean result = false;
        try {
            // Try to open the keystore
            fis = new FileInputStream(path);
            KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
            ks.load(fis, KEYSTORE_PASSWORD.toCharArray());
            result = true;
        } catch (FileNotFoundException e) {
            throw new KeyStoreManagerException(e.getMessage());
        } catch (Exception e) {
            result = false;
        } finally {
            try {
                if (fis != null)
                    fis.close();
            } catch (IOException e) {
                // Intentionally blank
            }
        }
        return result;
    }

    /**
     * Create the RCS keystore.
     * 
     * @throws Exception
     */
    public static void createKeyStore() throws KeyStoreManagerException {
        File file = new File(getKeystorePath());
        if ((file == null) || (!file.exists())) {
            FileOutputStream fos = null;
            try {
                fos = new FileOutputStream(getKeystorePath());
                // Build empty keystore
                KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
                ks.load(null, KEYSTORE_PASSWORD.toCharArray());
                // Export keystore in a file
                ks.store(fos, KEYSTORE_PASSWORD.toCharArray());
            } catch (Exception e) {
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fos != null)
                        fos.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        }
    }

    /**
     * Create the RCS client self signed keystore.
     * 
     * @throws KeyStoreManagerException The key store manager exception.
     */
    public static void createClientKeyStore() throws KeyStoreManagerException {
        if (logger.isActivated()) {
            logger.debug("createClientKeyStore entry");
        }
        File file = new File(getSelfSignedKeystorePath());
        if ((file == null) || (!file.exists())) {
            FileOutputStream fos = null;
            try {
                fos = new FileOutputStream(getSelfSignedKeystorePath());
                // Build empty keystore
                KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
                ks.load(null, KEYSTORE_PASSWORD.toCharArray());
                // Export keystore in a file
                ks.store(fos, KEYSTORE_PASSWORD.toCharArray());
            } catch (Exception e) {
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fos != null) {
                        fos.close();
                    }
                } catch (IOException e) {
                    if (logger.isActivated()) {
                        logger.debug("createClientKeyStore IOException");
                    }
                    e.printStackTrace();
                }
            }
        }
        if (logger.isActivated()) {
            logger.debug("createClientKeyStore exit");
        }
    }

    /**
     * Check if a certificate is in the keystore.
     * 
     * @param path certificate path
     * @return true if available
     * @throws Exception
     */
    public static boolean isCertificateEntry(String path) throws KeyStoreManagerException {
        FileInputStream fis = null;
        boolean result = false;
        if (KeyStoreManager.isKeystoreExists(getKeystorePath())) {
            try {
                fis = new FileInputStream(getKeystorePath());
                // Open the existing keystore
                KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
                ks.load(fis, KEYSTORE_PASSWORD.toCharArray());
                // isCertificateEntry
                result = ks.isCertificateEntry(buildCertificateAlias(path));
            } catch (Exception e) {
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fis != null)
                        fis.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        }else{
            logger.debug("KeyStore do not exist");
        }
        logger.debug("result = " + result);
        return result;
    }

    /**
     * Add a certificate in the keystore
     * 
     * @param alias certificate alias
     * @param path certificate path
     * @throws Exception
     */
    public static void addCertificate(String path) throws KeyStoreManagerException {
        if (KeyStoreManager.isKeystoreExists(getKeystorePath())) {
            FileInputStream fis = null;
            FileOutputStream fos = null;
            try {
                // Open the existing keystore
                fis = new FileInputStream(getKeystorePath());
                KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
                ks.load(fis, KEYSTORE_PASSWORD.toCharArray());
    
                // Get certificate and add in keystore
                CertificateFactory cf = CertificateFactory.getInstance("X.509");
                InputStream inStream = new FileInputStream(path);
                X509Certificate cert = (X509Certificate) cf.generateCertificate(inStream);
                inStream.close();
                ks.setCertificateEntry(buildCertificateAlias(path), cert);
    
                // save the keystore
                fos = new FileOutputStream(getKeystorePath());
                ks.store(fos, KEYSTORE_PASSWORD.toCharArray());
            } catch (Exception e) {
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fis != null)
                        fis.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
                try {
                    if (fos != null)
                        fos.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        }
    }

    /**
     * M: Added to generate fingerprint for MSRPoTLS @{
     */
    private static String dumpHex(byte[] data) {
        int n = data.length;
        StringBuffer sb = new StringBuffer(n * 3 - 1);
        for (int i = 0; i < n; i++) {
            if (i > 0) {
                sb.append(':');
            }
            sb.append(HEX_CHARS[(data[i] >> 4) & 0x0F]);
            sb.append(HEX_CHARS[data[i] & 0x0F]);
        }
        return sb.toString();
    }

    /**
     * Check if a certificate is in the keystore.
     * 
     * @param inputStream certificate stream
     * @param name certificate name
     * @return true if available
     * @throws Exception
     */
    public static boolean isCertificateEntry(InputStream inputStream, String name)
            throws KeyStoreManagerException {
        FileInputStream fis = null;
        boolean result = false;
        if (KeyStoreManager.isKeystoreExists(getKeystorePath())) {
            try {
                fis = new FileInputStream(getKeystorePath());
                // Open the existing keystore
                KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
                ks.load(fis, KEYSTORE_PASSWORD.toCharArray());
                // isCertificateEntry
                result = ks.isCertificateEntry(buildCertificateAlias(name));
            } catch (IOException e) {
                logger.debug("IOException:");
                throw new KeyStoreManagerException(e.getMessage());
            } catch (KeyStoreException e) {
                logger.debug("KeyStoreException:");
                throw new KeyStoreManagerException(e.getMessage());
            } catch (NoSuchAlgorithmException e) {
                logger.debug("NoSuchAlgorithmException:");
                throw new KeyStoreManagerException(e.getMessage());
            } catch (CertificateException e) {
                logger.debug("CertificateException:");
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fis != null)
                        fis.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        } else {
            logger.debug("KeyStore do not exist");
        }
        logger.debug("result = " + result);
        return result;
    }

    /**
     * Add a certificate in the keystore
     * 
     * @param inputStream certificate stream
     * @param name certificate name
     * @throws Exception
     */
    public static void addCertificate(InputStream inputStream, String name)
            throws KeyStoreManagerException {
        if (KeyStoreManager.isKeystoreExists(getKeystorePath())) {
            FileInputStream fis = null;
            FileOutputStream fos = null;
            try {
                // Open the existing keystore
                fis = new FileInputStream(getKeystorePath());
                KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
                ks.load(fis, KEYSTORE_PASSWORD.toCharArray());

                // Get certificate and add in keystore
                CertificateFactory cf = CertificateFactory.getInstance("X.509");
                X509Certificate cert = (X509Certificate) cf.generateCertificate(inputStream);
                ks.setCertificateEntry(buildCertificateAlias(name), cert);

                // save the keystore
                fos = new FileOutputStream(getKeystorePath());
                ks.store(fos, KEYSTORE_PASSWORD.toCharArray());
            } catch (IOException e) {
                logger.debug("IOException:");
                throw new KeyStoreManagerException(e.getMessage());
            } catch (KeyStoreException e) {
                logger.debug("KeyStoreException:");
                throw new KeyStoreManagerException(e.getMessage());
            } catch (NoSuchAlgorithmException e) {
                logger.debug("NoSuchAlgorithmException:");
                throw new KeyStoreManagerException(e.getMessage());
            } catch (CertificateException e) {
                logger.debug("CertificateException:");
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fis != null)
                        fis.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
                try {
                    if (fos != null)
                        fos.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        }
    }
    /**
     * @}
     */
    
    /**
     * M: add for MSRPoTLS @{
     */
    /**
     * Build alias from the certificate name
     * 
     * @return the alias
     */
    private static String buildCertificateAlias(String name) {
        logger.debug("buildCertificateAlias(),name = " + name);
        String alias = null;
        StringBuilder stringBuilder = new StringBuilder();
        int lastDotPosition = name.lastIndexOf('.');
        if (lastDotPosition > 0) {
            stringBuilder.append(name.substring(0, lastDotPosition));
        } else {
            stringBuilder.append(name);
        }
        stringBuilder.append(R.string.rcs_core_release_number);
        alias = stringBuilder.toString();
        logger.debug("alias = " + alias);
        return alias;
    }
    /**
     * @}
     */
    
    /**
     * M: Modified for MSRPoTLS @{
     */
    /**
     * Initialize a private key with self signed certificate.
     * 
     * @throws Exception
     */
    @SuppressWarnings("deprecation")
    public static void initPrivateKeyAndSelfsignedCertificate() throws Exception {
        if (logger.isActivated()) {
            logger.debug("initPrivateKeyAndSelfsignedCertificate entry");
        }
        createClientKeyStore();
        if (KeyStoreManager.isKeystoreExists(getSelfSignedKeystorePath())) {
            if (logger.isActivated()) {
                logger.error("initPrivateKeyAndSelfsignedCertificate key store exist");
            }
            // Open the existing keystore
            KeyStore ks = KeyStore.getInstance(KEYSTORE_TYPE);
            ks.load(new FileInputStream(getSelfSignedKeystorePath()),
                    KEYSTORE_PASSWORD.toCharArray());
                // Generate Key
            KeyPairGenerator kpg = KeyPairGenerator.getInstance(GENERATOR_RSA);
                kpg.initialize(1024);
                KeyPair kp = kpg.generateKeyPair();
                // Generate certificate
                long currentTime = System.currentTimeMillis();
                X509V3CertificateGenerator v3CertGen = new X509V3CertificateGenerator();
                v3CertGen.setSerialNumber(new BigInteger(Long.toString(currentTime)));
            v3CertGen.setIssuerDN(new X509Principal(X509_NAME));
                v3CertGen.setNotBefore(new Date(currentTime - 1000L * 60 * 60 * 24 * 30));
                v3CertGen.setNotAfter(new Date(currentTime + (1000L * 60 * 60 * 24 * 365 * 10)));
            v3CertGen.setSubjectDN(new X509Principal(X509_NAME));
                v3CertGen.setPublicKey(kp.getPublic());
            v3CertGen.setSignatureAlgorithm(SIGNATURE_ALGORITHM);
                X509Certificate cert = v3CertGen.generateX509Certificate(kp.getPrivate());
                try {
                MessageDigest md = MessageDigest.getInstance(DIGEST_ALGORITHM);
                    md.update(cert.getEncoded());
                    sFingerPrint = AlGORITHM_SHA;
                    sFingerPrint = sFingerPrint + dumpHex(md.digest());
                    if (logger.isActivated()) {
                    logger.debug("initPrivateKeyAndSelfsignedCertificate exit, with sFingerPrint: "
                            + sFingerPrint);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    if (logger.isActivated()) {
                        logger.debug("initPrivateKeyAndSelfsignedCertificate exception");
                    }
                }
                // Add the private key with cert in keystore
                ks.setKeyEntry(PRIVATE_KEY_ALIAS, kp.getPrivate(), KEYSTORE_PASSWORD.toCharArray(),
                        new Certificate[] {
                            cert
                        });
                // save the keystore
            ks.store(new FileOutputStream(getSelfSignedKeystorePath()),
                    KEYSTORE_PASSWORD.toCharArray());
        } else {
            if (logger.isActivated()) {
                logger.error("initPrivateKeyAndSelfsignedCertificate key store not exist");
            }
        }
        if (logger.isActivated()) {
            logger.debug("initPrivateKeyAndSelfsignedCertificate exit");
        }
    }
    /**
     * @}
     */
}
