/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mediatek.engineermode.nfc;

import com.mediatek.engineermode.Elog;

/**
 * 
 * @author mtk80905
 */
public class NfcNativeCallClass {
    public static final String TAG = "nfc";

    static {
        System.loadLibrary("em_nfc_jni");
    }

    /**
     * init nfc driver
     * 
     * @return 0: success, 1: failed
     */
    public static native int initNfcDriver();

    /**
     * de Init nfc driver, when back to main view of em call this
     */
    public static native void deinitNfcDriver();

    /**
     * send nfc setting request and get the rsp
     * 
     * @param req
     *            : nfc setting req
     * @return nfc setting rsp
     */
    public static native nfc_setting_response getSettings(
        nfc_setting_request req);

    /**
     * send nfc notify req and get the rsp
     * 
     * @param req
     *            nfc notify req
     * @return nfc notify rsp
     */
    public static native nfc_reg_notif_response getRegisterNotif(
        nfc_reg_notif_request req);

    /**
     * Set se req to chip and get the rsp
     * 
     * @param req
     *            set se req
     * @return set se rsp
     */
    public static native nfc_se_set_response setSEOption(nfc_se_set_request req);

    /**
     * send discovery notify req and get the rsp fron the chip
     * 
     * @param req
     *            discovery notify req
     * @return discovery notify rsp
     */
    public static native nfc_dis_notif_response getDiscoveryNotif(
        nfc_dis_notif_request req);

    /**
     * send the tag read req and get the rsp
     * 
     * @param req
     *            tag read req
     * @return tag read rsp
     */
    public static native nfc_tag_read_response readTag(nfc_tag_read_request req);

    /**
     * send nfc test req and get the rsp
     * 
     * @param req
     *            nfc test req
     * @return nfc test rsp
     */
    public static native nfc_test_response testEntry(nfc_test_request req);

    // ========================================================
    // ====Define NFC Service Handler Return Setting/Bitmap ===
    // ========================================================
    public static final int MTK_NFC_DIS_NOTIF_DURATION = 500;

    public static final int MIFARE4K_LEN = 16;
    public static final int MIFARE1K_LEN = 4;
    public static final int ISO15693_LEN = 4;

    public static final int MIFARE1K_PAGE = 4;
    public static final int UID_DATA_LEN = 8;
    public static final int TAG_RAW_DATA_LEN = 256;

    public static final int NDEF_DATA_LEN = 256; // Need to Check (1024)
    public static final int RAW_COMM_DATA_LEN = 256;

    public static final int NFC_HEADER_LENGTH = 8;
    // Reader/card mode/Register_notification Bitmap
    public static final int BM_MODE_MIFAREUL = 0;
    public static final int BM_MODE_MIFARESTD = 1;
    public static final int BM_MODE_ISO1443_4A = 2;
    public static final int BM_MODE_ISO1443_4B = 3;
    public static final int BM_MODE_JEWEL = 4;
    public static final int BM_MODE_NFC = 5;
    public static final int BM_MODE_FELICA = 6;
    public static final int BM_MODE_ISO15693 = 7;

    // secure element bitmap
    public static final int BM_START_OF_TRANSACTION = 0;
    public static final int BM_END_OF_TRANSACTION = 1;
    public static final int BM_TRANSACTION = 2;
    public static final int BM_RF_FIELD_ON = 3;
    public static final int BM_RF_FIELD_OFF = 4;
    public static final int BM_CONNECTIVITY = 5;

    // Discovery notification bitmap
    public static final int BM_DN_ISO1443_4A = 0;
    public static final int BM_DN_ISO1443_4B = 1;
    public static final int BM_DN_FELICA212 = 2;
    public static final int BM_DN_FELICA424 = 3;
    public static final int BM_DN_ISO15693 = 4;
    public static final int BM_DN_NFC_ACTIVE = 5;
    public static final int BM_DN_DISCARD_CE = 6;
    public static final int BM_DN_DISABLE_P2P = 7;

    // Magic number
    private static final int NUM_4 = 4;
    private static final int NUM_16 = 16;
    private static final int NUM_15 = 15;
    private static final int NUM_999 = 999;
    private static final int NUM_888 = 888;
    private static final int NUM_111 = 111;
    private static final int NUM_222 = 222;
    private static final int NUM_333 = 333;

    // Settings
    public static class nfc_setting_request {

        public int nfc_enable; /*
                                * feature enable or disable , 0: disable,
                                * 1:enable
                                */

        public int debug_enable; /*
                                  * debug enable or disable , 0: disable,
                                  * 1:enable
                                  */

        public int sw_protocol; /* SW protocol, 0: SW stack, 1: raw data mode */

        public int get_capabilities; /*
                                      * get chip capability 0: no request, 1:
                                      * request
                                      */

        public short[] test1 = new short[NUM_16];
        public short[][] test2 = new short[NUM_4][NUM_4];

        nfc_setting_request() {
            test1[0] = NUM_999;
            test1[NUM_15] = NUM_888;
            test2[0][0] = NUM_111;
            test2[1][0] = NUM_222;
            test2[3][3] = NUM_333;
        }
    };

    public static class nfc_setting_response {

        public int status; /* return setting result */

        public int nfc_enable; /* return feature enable or disable */

        public int debug_enable; /* return debug enable or disable */

        public int sw_protocol; /* return SW protocol, */

        public int get_capabilities; /* return chip capability */

        public int sw_ver; /* return software version */

        public int hw_ver; /* return hardware version */

        public int fw_ver; /* return firmware version */

        public int reader_mode; /* return support format bitmap */

        public int card_mode; /* return support format bitmap */

        public short[] test1;
        public short[][] test2;
    };

    // NFC Register Notification
    public static class nfc_reg_notif_request {

        public int reg_type; /* register notification bitmap */

    };

    public static class nfc_reg_notif_response {

        public int status; /* return setting result,0 success, other: fail */

        public int se; /*
                        * secure element detect, 0: no detect, 1: se typ1 , 2:
                        * se type 2
                        */

        public int se_status; /*
                               * secure element status, 0:off, 1:Virtual,
                               * 2:Wired
                               */

        public int se_type; /* secure element type, bitmap */

        public int length; /* length of data */

        public byte data[]; /* ascii data returned. */

    };

    // NFC Secure Element
    public static class nfc_se_set_request {

        public int set_SEtype; /* setting se type, 0:off, 1:Virtual, 2:Wired */

    };

    public static class nfc_se_set_response {

        public int status; /* return setting result, 0:sucess */

    };

    // NFC Discovery Notification
    public static class nfc_dis_notif_request {

        public int dis_type; /* discovery notification bitmap */

        public int duration; /* set duration,(unit:ms) */

    };

    public static class nfc_dis_notif_response {

        public int status; /* return setting result, 0:sucess */

        // public int which; /* tag or p2p was detected or nothing.*/

        public Object target;

    };

    // NFC Tag Detected Response
    public static class nfc_tag_det_response {

        public int tag_type; /* return detected tag type */

        public int card_type; /* card type, 1: Mifare classic 1K */
        /* 2: Mifare classic 4K */
        /* 3: NDEF */

        public byte[] uid; /* card Uid */

        public int sak; /* card sak */

        public int atag; /* card atag */

        public int appdata; /* card appdata */

        public int maxdatarate; /* card maxdatarate */

        /**
         * print the member for debug
         */
        public void printMember() {
            String msg =
                String
                    .format(
                        "tag_type %d, card_type %d, uid %s, sak %d, atag %d, appdata %d, maxdatarate %d",
                        tag_type, card_type, new String(uid), sak, atag,
                        appdata, maxdatarate);
            Elog.i(TAG, msg);
        }

    };

    // NFC Peer to Peer Detected Response
    public static class nfc_p2p_det_response {

        public int p2p_type; /* return detected tag type */

    };

    // NFC Peer to Peer Detected Response
    public static class nfc_none_det_response {

        public int dummy;

    };

    // NFC Peer to Peer communication
    // public static class nfc_p2p_com_request {
    //
    // public int action; /* 1 : send, 2 receive */
    //
    // public int length; /* length of */
    //
    // public byte data[]; /* ascii data returned. */
    //
    // };

    // public static class nfc_p2p_com_response {
    //
    // public int status; /* 0:success */
    //
    // public int length; /* length of */
    //
    // public byte data[]; /* ascii data returned. */
    //
    // };

    // NFC raw data communication //not used
    // public static class nfc_rd_com_request {
    //
    // public int action; /* 1:start test, 0:stop test */
    //
    // public int length; /* length of */
    //
    // public byte data[]; /* ascii data returned. */
    //
    // };

    // public static class nfc_rd_com_response {
    //
    // public int status; /* Test result, 0:success */
    //
    // public int length; /* length of */
    //
    // public byte data[]; /* ascii data returned. */
    //
    // };

    // /NFC test mode
    public static class nfc_script_request {

        public int type; /* Test type: currently, always set 1 */

        public int action; /* Test action, 1: start test, 0:stop test */

    };

    public static class nfc_script_response {

        public int result; /* Test result,0 :success */

    };

    // NFC test mode
    public static class nfc_script_uid_request {

        public int type; /* Test type: currently, always set 1 */

        public int action; /* Test action, 1: start test, 0:stop test */

        public int uid_type; /* 1: uid 4bytes, 2 : uid 7bytes */

        public byte[] data = new byte[UID_DATA_LEN]; /* uid content */

    };

    public static class nfc_script_uid_response {

        public int result; /* Test result, 0: success */

        public int uid_type; /* 1: uid 4bytes, 2 : uid 7bytes */

        public byte[] data;// = new short[UID_DATA_LEN]; /* uid content */

    };

    public static class nfc_tx_alwayson_request {
        public int type; /* Test type: currently, always set 1 */
        public int action; /* Test action, 1: start test, 0:stop test */
        public byte modulation_type; /*
                                      * 0:type A, 1:type B, 2:type F, 3:No
                                      * Modulation
                                      */
        public byte bitrate; /* 0:106kbps, 1:212kbps, 2:424kbps */
    };

    public static class nfc_card_emulation_request {
        public int type; /* Test type: currently, always set 1 */
        public int action; /* Test action, 1: start test, 0:stop test */
        public short technology; /*
                                  * bitmask: MifareUL=bit 0, MifareStd=bit1,
                                  * ISO14443_4A=bit 2, ISO14443_4B=bit 3,
                                  * Jewel=bit 4, Felica=bit 5, ISO15693=bit 6
                                  */
        public short protocols; /*
                                 * bitmask: Iso14443A=bit 0, Iso14443B=bit 1,
                                 * Felica212=bit 2, Felica424=bit 3,
                                 * Iso15693=bit 4
                                 */
    };

    public static class nfc_test_request {
        public int which; // which test
        public Object target; // test request. like nfc_tx_alwayson_request..
    }

    public static class nfc_test_response {
        public Object target; // test request. like nfc_script_uid_response..
    }

    // NFC Tag Read Request
    public static class nfc_tag_read_request {

        public int read_type; /* which type want to read */
        /* 1 for Mifare classic 1K */
        /* 2 for Mifare classic 4K */
        /* 3 for NDEF type */

        public int address; /* for Mifare classic 1K used */

        public int sector; /* for Mifare classic 4K used */

        public int block; /* for Mifare classic 4K used */

    };

    // !!!NDEF type no parameter

    // start NFC Tag Read Response
    public static class nfc_tag_read_response {

        public int status; /* return read status, 0 success */

        public Object target; /*
                               * 1 : Mifare classic 1K ,2: Mifare classic 4K, 3:
                               * NDEF
                               */

    };

    public static class nfc_tag_read_Mifare1K {

        public int address; /*  */

        public short[] data;// = new short[MIFARE1K_LEN];
    };

    public static class nfc_tag_read_Mifare4K {

        public int sector; /*  */

        public int block;
        public short[] data;// = new short[MIFARE4K_LEN];
    };

    public static class nfc_tag_read_ndef {

        public int ndef_type;
        public int recordFlage;
        public int recordId;
        public int recordInfo;
        public int length;
        public short[] dataHex; // = new short[NDEF_DATA_LEN];
        public char[] dataAscii; // = new short[NDEF_DATA_LEN];
    };

    // end NFC Tag Read Response

    // start NFC Tag Write Request
    // public static class nfc_tag_write_request {
    //
    // public int write_type; /* which type want to write */
    //
    // public Object target;
    //
    // };

    // public static class nfc_tag_write_typeA {
    //
    // public int address; /* */
    //
    // public short[] data = new short[NUM_4];
    // };

    // public static class nfc_tag_write_typeB {
    //
    // public int sector; /* */
    //
    // public int block;
    // public short[] data = new short[NUM_16];
    // };

    // public static class nfc_tag_write_typeMifare {
    // public int sector; /* */
    // public int block;
    // public byte[] data = new byte[MIFARE4K_LEN];
    // };

    // public static class nfc_tag_write_typeISO15693 {
    // public int sector; /* */
    // public int block;
    // public byte[] data = new byte[MIFARE4K_LEN];
    // };

    // public static class nfc_tag_write_ndef {
    // public int ndef_type;
    // public int length;
    // public byte[] data = new byte[NDEF_DATA_LEN];
    // };

    // end NFC Tag Write Request

    // NFC Tag Read Request

    // NFC Tag Write Response
    /* Tag TYPE & card type */
    public static final int TAG_TYPE_DEFAULT = 0;
    public static final int TAG_TYPE_MIFARE_UL = 1;
    public static final int TAG_TYPE_MIFARE_STD = 2;
    public static final int TAG_TYPE_ISO1443_4A = 3;
    public static final int TAG_TYPE_ISO1443_4B = 4;
    public static final int TAG_TYPE_JEWWL = 5;
    public static final int TAG_TYPE_NFC = 6;
    public static final int TAG_TYPE_FELICA = 7;
    public static final int TAG_TYPE_ISO15693 = 8;
    public static final int TAG_TYPE_NDEF = 9;

    // public static class nfc_tag_write_response {
    //
    // public int status; /* return read status, 0 success */
    //
    // public int wrtie_type; /* return writed Tag TYPE */
    //
    // };

    // /NFC Tag Disconnect Request
    // public static class nfc_tag_disconnect_request {
    //
    // public int action; /* 1: disconnect, */
    //
    // };

    // /NFC Tag Disconnect Response
    // public static class nfc_tag_disconnect_response {
    //
    // public int status; /* 0: success */
    //
    // };

    // NFC Tag format tp Ndef format Request
    // public static class nfc_tag_fromat2Ndef_request {
    //
    // public int action;
    //
    // ; /* 1: format to Ndef, */
    //
    // };

    // NFC Tag format tp Ndef format Response

    // public static class nfc_tag_fromat2Ndef_response {
    //
    // public int status; /* 0: success */
    //
    // };

    /* ==========NFC Tag raw command Request=========== */
    // public static class nfc_tag_raw_com_req_typeA {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // public static class nfc_tag_raw_com_req_typeB {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // public static class nfc_tag_raw_com_req_typeJewel {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // public static class nfc_tag_raw_com_req_typeFelica {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // nfc_tag_raw_com_request
    // public static class nfc_tag_raw_com_request {
    // public int type; /* TAG TYPE */
    // public int length; /*
    // * Length of written data. Unit is byte. The data is
    // * stored in the Meta peer buffer
    // */
    // public Object req_data;
    // };

    /* NFC Tag Disconnect Response */

    // public static class nfc_tag_raw_com_rsp_typeA {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // public static class nfc_tag_raw_com_rsp_typeB {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // public static class nfc_tag_raw_com_rsp_typeJewel {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // public static class nfc_tag_raw_com_rsp_typeFelica {
    // public int length;
    // public byte[] data = new byte[TAG_RAW_DATA_LEN];
    // };

    // nfc_tag_raw_com_response
    // public static class nfc_tag_raw_com_response {
    // public int type; /* TAG TYPE */
    // public int status; /* 0 : success */
    // public Object rsp_data;
    // };

}
