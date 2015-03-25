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

package com.orangelabs.rcs.core.ims.security.cert;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.sql.Date;
import java.math.BigInteger;
import com.android.org.bouncycastle.x509.X509V3CertificateGenerator;
import com.android.org.bouncycastle.jce.X509Principal;


import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

import java.security.KeyPair;
import java.security.KeyPairGenerator;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.TrustManagerFactory;

import com.orangelabs.rcs.utils.logger.Logger;

import android.content.Context;

/**
 * Keystore manager for certificates
 * 
 * @author B. JOGUET
 * @author Deutsche Telekom AG
 */
public class KeyStoreManager {
/**
     * M: Modified and added for achieving MSRPoTLS related feature. @{
     */
/**
     * Client self signed keystore
     */
    public final static String CLIENT_KEYSTORE = "client.bks";
    /**
     * Keystore name
     */
    private final static String KEYSTORE_NAME = "server.bks";

    /**
     * Keystore password
     */
    private final static String KEYSTORE_PASSWORD = "1234567";
    
    
    /**
     * M: Added to achieve the SDPoTLS and MSRPoTLS implementation. @{
     */
    /**
     * Certificate name in asset
     */
    
    
       
    
    
    public static final String CERTIFICATE_NAME = "vodafone_reg_server";
    public static final String CERTIFICATE_SUFFIXE = ".crt";
    public static final int CERTIFICATE_NUM = 4;
    /**
     * Keystore type
     */
    public final static String KEYSTORE_TYPE = KeyStore.getDefaultType();

    /**
     * @}
     */
    
    
    
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
        logger.debug("getFingerPrint" + sFingerPrint);
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
     * Load the keystore manager
     * 
     * @throws KeyStoreManagerException
     * @throws IOException 
     * @throws KeyStoreException 
     */
    public static void loadKeyStore() throws KeyStoreManagerException, IOException, KeyStoreException {    
        // Create the keystore if not present
        if (!KeyStoreManager.isKeystoreExists(KeyStoreManager.getKeystorePath())) {
            KeyStoreManager.createKeyStore();
        }
        
        // Add certificates if not present
        String certRoot = RcsSettings.getInstance().getTlsCertificateRoot();
        if ((certRoot != null) && (certRoot.length() > 0)) {
            if (!KeyStoreManager.isCertificateEntry(certRoot)) {
            	KeyStoreManager.addCertificates(certRoot);
            }
        }
        String certIntermediate = RcsSettings.getInstance().getTlsCertificateIntermediate();
        if ((certIntermediate != null) && (certIntermediate.length() > 0)) {
            if (!KeyStoreManager.isCertificateEntry(certIntermediate)) {
                KeyStoreManager.addCertificates(certIntermediate);
            }
        }
        
        /**
         * M: Modified to achieve the TLS related implementation-Get
         * vodafone's self-signed certificate and do sip register over
         * TLS. @{
         */
       
        Context context = AndroidFactory.getApplicationContext();
        if (context != null) {
            //Add all custom certificate to keystore
            for(int i=0 ; i < CERTIFICATE_NUM; ++i){
                InputStream is = null;
                try{
                    is = context.getAssets().open(
                            CERTIFICATE_NAME + i + CERTIFICATE_SUFFIXE);
                    if (!KeyStoreManager.isCertificateEntry(is,CERTIFICATE_NAME + i + CERTIFICATE_SUFFIXE)) {
                        KeyStoreManager.addCertificate(is,CERTIFICATE_NAME + i + CERTIFICATE_SUFFIXE);
                    }
                }catch (IOException e) {
                }finally{
                    if(is != null){
                        is.close();
                    }
                }
            }
        } 
        /**
         * @}
         */
        
        
        
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
     * @param inputStream certificate stream
     * @param name certificate name
     * @return true if available
     * @throws KeyStoreException 
     * @throws Exception
     */
    public static boolean isCertificateEntry(InputStream inputStream, String name)
            throws KeyStoreManagerException, KeyStoreException {
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
                throw new KeyStoreManagerException(e.getMessage());
            } catch (NoSuchAlgorithmException e) {
                throw new KeyStoreManagerException(e.getMessage());
            } catch (CertificateException e) {
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fis != null)
                        fis.close();
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        } 
        return result;
    }

    /**
     * Add a certificate in the keystore
     * 
     * @param inputStream certificate stream
     * @param name certificate name
     * @throws KeyStoreException 
     * @throws Exception
     */
    public static void addCertificate(InputStream inputStream, String name)
            throws KeyStoreManagerException, KeyStoreException {
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
                throw new KeyStoreManagerException(e.getMessage());
            } catch (NoSuchAlgorithmException e) {
                throw new KeyStoreManagerException(e.getMessage());
            } catch (CertificateException e) {
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
     * Returns the keystore type
     * 
     * @return Type
     */
    public static String getKeystoreType() {
    	return KeyStore.getDefaultType();
    }
    
    /**
     * Returns the keystore password
     * 
     * @return Password
     */
    public static String getKeystorePassword() {
    	return KEYSTORE_PASSWORD;
    }
    	
    /**
     * Returns the keystore path
     * 
     * @return Keystore path
     */
    public static String getKeystorePath() {
        return AndroidFactory.getApplicationContext().getFilesDir().getAbsolutePath() + "/"
                + KEYSTORE_NAME;
    }

    /**
     * Test if a keystore is created
     * 
     * @return True if already created
     * @throws KeyStoreManagerException
     */
    private static boolean isKeystoreExists(String path) throws KeyStoreManagerException {
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
            KeyStore ks = KeyStore.getInstance(getKeystoreType());
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
     * Create the RCS keystore
     * 
     * @throws KeyStoreManagerException
     */
    private static void createKeyStore() throws KeyStoreManagerException {
        File file = new File(getKeystorePath());
        if ((file == null) || (!file.exists())) {
            FileOutputStream fos = null;
            try {
                fos = new FileOutputStream(getKeystorePath());
                
                // Build empty keystore
                KeyStore ks = KeyStore.getInstance(getKeystoreType());
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
     * Check if a certificate is in the keystore
     * 
     * @param path Certificate path
     * @return True if available
     * @throws KeyStoreManagerException
     */
    private static boolean isCertificateEntry(String path) throws KeyStoreManagerException {
        FileInputStream fis = null;
        boolean result = false;
        if (KeyStoreManager.isKeystoreExists(getKeystorePath())) {
            try {
                fis = new FileInputStream(getKeystorePath());
                
                // Open the existing keystore
                KeyStore ks = KeyStore.getInstance(getKeystoreType());
                ks.load(fis, KEYSTORE_PASSWORD.toCharArray());
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
        } 
        return result;
    }

    /**
     * Add a certificate or all certificates in folder in the keystore
     *
     * @param path certificates path
     * @throws KeyStoreManagerException
     */
    private static void addCertificates(String path) throws KeyStoreManagerException {
        if (KeyStoreManager.isKeystoreExists(getKeystorePath())) {
            FileInputStream fis = null;
            FileOutputStream fos = null;
            try {
                // Open the existing keystore
                fis = new FileInputStream(getKeystorePath());
                KeyStore ks = KeyStore.getInstance(getKeystoreType());
                ks.load(fis, KEYSTORE_PASSWORD.toCharArray());

                // Open certificates path
                File pathFile = new File(path);
                if (pathFile.isDirectory()) {
                    // The path is a folder, add all certificates
                    File[] certificates = pathFile.listFiles(new FilenameFilter() {
                        public boolean accept(File dir, String filename) {
                            return filename.endsWith(RcsSettingsData.CERTIFICATE_FILE_TYPE);
                        }
                    });

                    if (certificates != null) {
                        for (File file : certificates) {
                            // Get certificate and add in keystore
                            CertificateFactory cf = CertificateFactory.getInstance("X.509");
                            InputStream inStream = new FileInputStream(file);
                            X509Certificate cert = (X509Certificate) cf.generateCertificate(inStream);
                            inStream.close();
                            ks.setCertificateEntry(buildCertificateAlias(path), cert);

                            // Save the keystore
                            fos = new FileOutputStream(getKeystorePath());
                            ks.store(fos, KEYSTORE_PASSWORD.toCharArray());
                            fos.close();
                            fos = null;
                        }
                    }
                } else {
                    // The path is a file, add certificate
                    if (path.endsWith(RcsSettingsData.CERTIFICATE_FILE_TYPE)) {
                        // Get certificate and add in keystore
                        CertificateFactory cf = CertificateFactory.getInstance("X.509");
                        InputStream inStream = new FileInputStream(path);
                        X509Certificate cert = (X509Certificate) cf.generateCertificate(inStream);
                        inStream.close();
                        ks.setCertificateEntry(buildCertificateAlias(path), cert);

                        // Save the keystore
                        fos = new FileOutputStream(getKeystorePath());
                        ks.store(fos, KEYSTORE_PASSWORD.toCharArray());
                    }
                }
            } catch (Exception e) {
                throw new KeyStoreManagerException(e.getMessage());
            } finally {
                try {
                    if (fis != null) {
                        fis.close();
                    }
                } catch (IOException e) {
                    // Intentionally blank
                }
                try {
                    if (fos != null) {
                        fos.close();
                    }
                } catch (IOException e) {
                    // Intentionally blank
                }
            }
        }
    }

    /**
     * Build alias from path
     * 
     * @param path File path
     * @return Alias
     */
    private static String buildCertificateAlias(String path) {
        String alias = "";
        File file = new File(path);
        String filename = file.getName();
        long lastModified = file.lastModified();
        int lastDotPosition = filename.lastIndexOf('.');
        if (lastDotPosition > 0)
            alias = filename.substring(0, lastDotPosition) + lastModified;
        else
            alias = filename + lastModified;
        return alias;
    }
    
    /**
     * Returns the fingerprint of a certificate
     * 
     * @param cert Certificate
     * @return String as xx:yy:zz
     */
    public static String getCertFingerprint(Certificate cert) {
    	try {
		    MessageDigest md = MessageDigest.getInstance("SHA-1");
		    byte[] der = cert.getEncoded();
		    md.update(der);
		    byte[] digest = md.digest();
		    return hexify(digest);
    	} catch(Exception e) {
    		return null;
    	}
	}


    /**
     * Hexify a byte array 
     * 
     * @param bytes Byte array
     * @return String
     */
	private static String hexify(byte bytes[]) {
	    char[] hexDigits = {'0', '1', '2', '3', '4', '5', '6', '7', 
	                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	    StringBuffer buf = new StringBuffer(bytes.length * 2);
	    for (int i = 0; i < bytes.length; ++i) {
	    	if (i != 0) {
	    		buf.append(":");	
	    	}
	    	buf.append(hexDigits[(bytes[i] & 0xf0) >> 4]);
	        buf.append(hexDigits[bytes[i] & 0x0f]);
	    }
	    return buf.toString();
	}
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
			logger.debug("initPrivateKeyAndSelfsignedCertificate 0");
                // Generate Key
            KeyPairGenerator kpg = KeyPairGenerator.getInstance(GENERATOR_RSA);
                kpg.initialize(1024);
                KeyPair kp = kpg.generateKeyPair();
                // Generate certificate
                long currentTime = System.currentTimeMillis();
                X509V3CertificateGenerator v3CertGen = new X509V3CertificateGenerator();
                v3CertGen.setSerialNumber(new BigInteger(Long.toString(currentTime)));
            v3CertGen.setIssuerDN(new X509Principal(X509_NAME));
			logger.debug("initPrivateKeyAndSelfsignedCertificate 1");
                v3CertGen.setNotBefore(new Date(currentTime - 1000L * 60 * 60 * 24 * 30));
                v3CertGen.setNotAfter(new Date(currentTime + (1000L * 60 * 60 * 24 * 365 * 10)));
            v3CertGen.setSubjectDN(new X509Principal(X509_NAME));
                v3CertGen.setPublicKey(kp.getPublic());
            v3CertGen.setSignatureAlgorithm(SIGNATURE_ALGORITHM);
                X509Certificate cert = v3CertGen.generateX509Certificate(kp.getPrivate());
				logger.debug("initPrivateKeyAndSelfsignedCertificate 2");
                try {
                MessageDigest md = MessageDigest.getInstance(DIGEST_ALGORITHM);
                    md.update(cert.getEncoded());
                    sFingerPrint = AlGORITHM_SHA;
					logger.debug("initPrivateKeyAndSelfsignedCertificate 3");
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
				logger.debug("initPrivateKeyAndSelfsignedCertificate 4");
                // Add the private key with cert in keystore
                ks.setKeyEntry(PRIVATE_KEY_ALIAS, kp.getPrivate(), KEYSTORE_PASSWORD.toCharArray(),
                        new Certificate[] {
                            cert
                        });
                // save the keystore
                logger.debug("initPrivateKeyAndSelfsignedCertificate 5");
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
