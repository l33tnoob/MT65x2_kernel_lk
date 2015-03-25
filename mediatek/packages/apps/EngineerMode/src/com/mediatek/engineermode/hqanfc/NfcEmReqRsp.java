package com.mediatek.engineermode.hqanfc;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.hqanfc.NfcCommand.DataConvert;

import java.nio.BufferOverflowException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

/**
 * 
 * @author mtk54045
 */
public class NfcEmReqRsp {

    private static final int TAG_WRITE_MAXDATA = 512;

    // typedef struct mtk_nfc_sw_version_rsp{
    // CHAR mv_ver[19];
    // UINT16 fw_ver;
    // UINT16 hw_ver;
    // }mtk_nfc_sw_version_rsp_t;
    public static class NfcEmVersionRsp implements RawOperation {
        private static final int DATA_LENGTH = 19;
        public static final int CONTENT_SIZE = 2 + 2 + DATA_LENGTH;
        
        public byte[] mMwVersion;
        public int mFwVersion;
        public int mHwVersion;
    
        public NfcEmVersionRsp() {
            mMwVersion = new byte[DATA_LENGTH];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            buffer.get(mMwVersion, 0, DATA_LENGTH);
            int version = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mFwVersion = (int) (version & 0xFFFF);
            mHwVersion = (int) ((version >> 16) & 0xFFFF);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(mMwVersion, 0, DATA_LENGTH);
            buffer.put(DataConvert.intToLH(mFwVersion));
            buffer.put(DataConvert.intToLH(mHwVersion));
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct s_mtk_nfc_test_mode_setting_req{
    // UINT16 forceDownload; // 1: enable
    // UINT16 tagAutoPresenceChk; // 1: enable
    // }s_mtk_nfc_test_mode_setting_req_t;
    public static class NfcEmOptionReq extends NfcEmReq {
        public static final int CONTENT_SIZE = 4;
        public short mForceDownload;
        public short mAutoCheck;

        public NfcEmOptionReq() {
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            int option = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mForceDownload = (short) (option & 0xFFFF);
            mAutoCheck = (short) ((option >> 16) & 0xFFFF);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.shortToLH(mForceDownload));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_test_mode_setting_req forceDownload: " + mForceDownload);
            buffer.put(DataConvert.shortToLH(mAutoCheck));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_test_mode_setting_req tagAutoPresenceChk: " + mAutoCheck);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct s_mtk_nfc_test_mode_setting_rsp{
    // UINT16 result;
    // }s_mtk_nfc_test_mode_setting_rsp_t;
    public static class NfcEmOptionRsp {
        public static final int CONTENT_SIZE = 1;
        public byte mResult;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = buffer.get();
            Elog.d(NfcMainPage.TAG, "NfcEmOptionRsp result: " + mResult);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(mResult);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct s_mtk_nfc_loopback_test_req{
    // CHAR action;
    // }s_mtk_nfc_test_mode_setting_req_t;
    public static class NfcEmLoopbackReq extends NfcEmReq {
        public static final int CONTENT_SIZE = 1;
        public byte mAction;

        public NfcEmLoopbackReq() {
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = buffer.get();
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(mAction);
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_loopback_test_req action: " + mAction);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct s_mtk_nfc_loopback_test_rsp{
    // CHAR result;
    // }s_mtk_nfc_loopback_test_rsp_t;
    public static class NfcEmLoopbackRsp{
        public static final int CONTENT_SIZE = 1;
        public byte mResult;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = buffer.get();
            Elog.d(NfcMainPage.TAG, "NfcEmLoopbackRsp result: " + mResult);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(mResult);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct s_mtk_nfc_fm_swp_test_req{
    // INT32 action;
    // }s_mtk_nfc_fm_swp_test_req_t;
    public static class NfcEmSwpReq extends NfcEmReq {
        public static final int CONTENT_SIZE = 4;
        public int mAction;

        public NfcEmSwpReq() {
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_fm_swp_test_req action: " + mAction);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct s_mtk_nfc_fm_swp_test_rsp{
    // INT32 result;
    // }s_mtk_nfc_fm_swp_test_rsp_t;
    public static class NfcEmSwpRsp extends NfcEmRsp {
    }

    // typedef struct mtk_nfc_em_pnfc_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // unsigned int datalen;
    // unsigned char data[256];
    // }s_mtk_nfc_em_pnfc_req;
    public static class NfcEmPnfcReq extends NfcEmReq {

        public static final int DATA_MAX_LENGTH = 256;
        public static final int CONTENT_SIZE = DATA_MAX_LENGTH + NfcCommand.INT_SIZE * 2;
        public int mAction;
        public int mDataLen;
        public byte[] mData;

        public NfcEmPnfcReq() {
            mData = new byte[DATA_MAX_LENGTH];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mDataLen = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            buffer.get(mData, 0, mDataLen);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_pnfc_req action: " + mAction);
            buffer.put(DataConvert.intToLH(mDataLen));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_pnfc_req datalen: " + mDataLen);
            buffer.put(mData, 0, mDataLen);
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_pnfc_req data: " + new String(mData));
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }

    }

    // typedef struct mtk_nfc_em_pnfc_rsp{
    // unsigned int result; /*0:Success,1:Fail*/
    // unsigned int datalen;
    // unsigned char data[256];
    // }s_mtk_nfc_em_pnfc_rsp;
    public static class NfcEmPnfcRsp implements RawOperation {
        private static final int DATA_LENGTH = 256;
        public static final int CONTENT_SIZE = 4 + 4 + DATA_LENGTH;
        
        public int mResult;
        public int mLength;
        public byte[]mData;

        public NfcEmPnfcRsp() {
            mData = new byte[DATA_LENGTH];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mLength = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            
            buffer.get(mData, 0, mLength);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {

            buffer.put(DataConvert.intToLH(mResult));
            buffer.put(DataConvert.intToLH(mLength));
            buffer.put(mData, 0, DATA_LENGTH);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_virtual_card_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // unsigned int supporttype; /* supporttype, please refer BITMAP of
    // EM_ALS_READER_M_TYPE*/
    // //unsigned int typeA_datarate; /* TypeA,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // //unsigned int typeB_datarate; /* TypeB,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int typeF_datarate; /* TypeF,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // }s_mtk_nfc_em_virtual_card_req;
    public static class NfcEmVirtualCardReq extends NfcEmReq {

        public static final int CONTENT_SIZE = 12;
        public int mAction;
        public int mSupportType;
        // public int mTypeADataRate;
        // public int mTypeBDataRate;
        public int mTypeFDataRate;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mSupportType = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            // mTypeADataRate =
            // DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            // mTypeBDataRate =
            // DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeFDataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_victual_card_req action: " + mAction);
            buffer.put(DataConvert.intToLH(mSupportType));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_victual_card_req supporttype: "
                    + mSupportType);
            // buffer.put(DataConvert.intToLH(mTypeADataRate));
            // Elog.d(NfcMainPage.TAG,
            // "[NfcEmReqRsp]mtk_nfc_em_victual_card_req typeA_datarate: "
            // + mTypeADataRate);
            // buffer.put(DataConvert.intToLH(mTypeBDataRate));
            // Elog.d(NfcMainPage.TAG,
            // "[NfcEmReqRsp]mtk_nfc_em_victual_card_req typeB_datarate: "
            // + mTypeBDataRate);
            buffer.put(DataConvert.intToLH(mTypeFDataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_victual_card_req typeF_datarate: "
                    + mTypeFDataRate);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_victual_card_rsp{
    // unsigned int result; /*0:Success,1:Fail*/
    // }s_mtk_nfc_em_victual_card_rsp;
    public static class NfcEmVirtualCardRsp extends NfcEmRsp {
    }

    // typedef struct mtk_nfc_em_tx_carr_als_on_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // }s_mtk_nfc_em_tx_carr_als_on_req;
    public static class NfcEmTxCarrAlsOnReq extends NfcEmReq {

        public static final int CONTENT_SIZE = 4;
        public int mAction;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_tx_carr_als_on_req action: " + mAction);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_tx_carr_als_on_rsp{
    // unsigned int result; /*0:Success,1:Fail*/
    // }s_mtk_nfc_em_tx_carr_als_on_rsp;
    public static class NfcEmTxCarrAlsOnRsp extends NfcEmRsp {
    }

    // typedef struct mtk_nfc_em_polling_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // unsigned int phase; /*0:Listen phase, 1:Pause phase*/
    // unsigned int Period;
    // unsigned int enablefunc; /*enablefunc, please refer BITMAP of
    // EM_ENABLE_FUNC*/
    // s_mtk_nfc_em_als_p2p_req p2pM;
    // s_mtk_nfc_em_als_cardm_req cardM;
    // s_mtk_nfc_em_als_readerm_req readerM;
    // }s_mtk_nfc_em_polling_req;
    public static class NfcEmPollingReq extends NfcEmReq {

        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE * 4
                + NfcEmAlsP2pReq.CONTENT_SIZE + NfcEmAlsCardmReq.CONTENT_SIZE
                + NfcEmAlsReadermReq.CONTENT_SIZE;
        public int mAction;
        public int mPhase;
        public int mPeriod;
        public int mEnableFunc;
        public NfcEmAlsP2pReq mP2pmReq;
        public NfcEmAlsCardmReq mCardmReq;
        public NfcEmAlsReadermReq mReadermReq;

        public NfcEmPollingReq() {
            mP2pmReq = new NfcEmAlsP2pReq();
            mCardmReq = new NfcEmAlsCardmReq();
            mReadermReq = new NfcEmAlsReadermReq();
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mPhase = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mPeriod = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mEnableFunc = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mP2pmReq.readRaw(buffer);
            mCardmReq.readRaw(buffer);
            mReadermReq.readRaw(buffer);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req action: " + mAction);
            buffer.put(DataConvert.intToLH(mPhase));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req phase: " + mPhase);
            buffer.put(DataConvert.intToLH(mPeriod));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req period: " + mPeriod);
            buffer.put(DataConvert.intToLH(mEnableFunc));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req enablefunc: "
                    + mEnableFunc);
            mP2pmReq.writeRaw(buffer);
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req write p2p.");
            mCardmReq.writeRaw(buffer);
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req write card mode.");
            mReadermReq.writeRaw(buffer);
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_req write reader mode.");
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_polling_rsp{
    // int result; /*0:Success,1:Fail*/
    // }s_mtk_nfc_em_polling_rsp;
    public static class NfcEmPollingRsp extends NfcEmRsp {
    }

    // typedef union {
    // s_mtk_nfc_em_als_p2p_ntf p2p;
    // s_mtk_nfc_em_als_cardm_rsp card;
    // s_mtk_nfc_em_als_readerm_ntf reader;
    // } s_mtk_nfc_em_polling_func_ntf;
    //
    // typedef struct mtk_nfc_em_polling_ntf{
    // int detecttype; /*enablefunc, please refer ENUM of EM_ENABLE_FUNC*/
    // s_mtk_nfc_em_polling_func_ntf ntf;
    // }s_mtk_nfc_em_polling_ntf;
    public static class NfcEmPollingNty implements RawOperation {
        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE + NfcEmAlsReadermNtf.CONTENT_SIZE;
        public int mDetectType;
        public byte[] mData;

        public NfcEmPollingNty() {
            mData = new byte[NfcEmAlsReadermNtf.CONTENT_SIZE];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mDetectType = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_polling_ntf detecttype: "
                    + mDetectType);
            buffer.get(mData, 0, NfcEmAlsReadermNtf.CONTENT_SIZE);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mDetectType));
            buffer.put(mData, 0, NfcEmAlsReadermNtf.CONTENT_SIZE);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_p2p_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // unsigned int supporttype; /* supporttype, please refer BITMAP of
    // EM_ALS_READER_M_TYPE*/
    // unsigned int typeA_datarate; /* TypeA,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int typeF_datarate; /* TypeV,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int mode; /* BITMAPS bit0: Passive mode, bit1: Active mode*/
    // unsigned int role; /* BITMAPS bit0: Initator, bit1: Target*/
    // unsigned int isDisableCardM; /* 0: , 1: disable card mode*/
    // }s_mtk_nfc_em_als_p2p_req;
    public static class NfcEmAlsP2pReq extends NfcEmReq {

        public static final int CONTENT_SIZE = 28;
        public int mAction;
        public int mSupportType;
        public int mTypeADataRate;
        public int mTypeFDataRate;
        public int mMode;
        public int mRole;
        public int mIsDisableCardM;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mSupportType = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeADataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeFDataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mMode = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mRole = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mIsDisableCardM = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req action: " + mAction);
            buffer.put(DataConvert.intToLH(mSupportType));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req supporttype: "
                    + mSupportType);
            buffer.put(DataConvert.intToLH(mTypeADataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req typeA_speedrate: "
                    + mTypeADataRate);
            buffer.put(DataConvert.intToLH(mTypeFDataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req typeV_speedrate: "
                    + mTypeFDataRate);
            buffer.put(DataConvert.intToLH(mMode));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req mode: " + mMode);
            buffer.put(DataConvert.intToLH(mRole));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req role: " + mRole);
            buffer.put(DataConvert.intToLH(mIsDisableCardM));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_p2p_req isDisableCardM: "
                    + mIsDisableCardM);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_p2p_ntf{
    // int link_status; /*1:llcp link is up,0:llcp link is down*/
    // //unsigned int datalen;
    // //unsigned char data[256];
    // }s_mtk_nfc_em_als_p2p_ntf;
    public static class NfcEmAlsP2pNtf implements RawOperation {

        public static final int DATA_MAX_LENGTH = 256;
//        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE * 2 + DATA_MAX_LENGTH;
        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE;
        public int mResult;
//        public int mDataLen;
//        public byte[] mData;

        public NfcEmAlsP2pNtf() {
//            mData = new byte[DATA_MAX_LENGTH];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_p2p_ntf result: " + mResult);
//            mDataLen = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
//            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_p2p_ntf datalen: " + mDataLen);
//            buffer.get(mData, 0, mDataLen);
//            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_p2p_ntf data: "
//                    + new String(mData));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mResult));
//            buffer.put(DataConvert.intToLH(mDataLen));
//            buffer.put(mData, 0, mDataLen);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_p2p_rsp{
    // int result; /*0:Success,1:Fail*/
    // }s_mtk_nfc_em_als_p2p_rsp;
    public static class NfcEmAlsP2pRsp extends NfcEmRsp {
    }

    // typedef struct mtk_nfc_em_als_cardm_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // unsigned int SWNum; /* SWNum, please refer BITMAP of
    // EM_ALS_CARD_M_SW_NUM*/
    // unsigned int supporttype; /* supporttype, please refer BITMAP of
    // EM_ALS_READER_M_TYPE*/
    // unsigned int fgvirtualcard; /* 1:enable virtual card, 0:disable virtual
    // card(default) */
    // }s_mtk_nfc_em_als_cardm_req;
    public static class NfcEmAlsCardmReq extends NfcEmReq {

        public static final int CONTENT_SIZE = 16;
        public int mAction;
        public int mSwNum;
        public int mSupportType;
        public int mFgVirtualCard;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mSwNum = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mSupportType = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_cardm_req action: " + mAction);
            buffer.put(DataConvert.intToLH(mSwNum));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_cardm_req SWNum: " + mSwNum);
            buffer.put(DataConvert.intToLH(mSupportType));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_cardm_req supporttype: "
                    + mSupportType);
            buffer.put(DataConvert.intToLH(mFgVirtualCard));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_cardm_req fgvirtualcard: "
                    + mFgVirtualCard);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_cardm_rsp{
    // int result; /*0:Success,1:Fail*/
    // }s_mtk_nfc_em_als_cardm_rsp;
    public static class NfcEmAlsCardmRsp extends NfcEmRsp {
    }

    // typedef struct mtk_nfc_em_als_readerm_req{
    // unsigned int action; /*Action, please refer ENUM of EM_ACTION*/
    // unsigned int supporttype; /* supporttype, please refer BITMAP of
    // EM_ALS_READER_M_TYPE*/
    // unsigned int typeA_datarate; /* TypeA,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int typeB_datarate; /* TypeB,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int typeV_datarate; /* TypeV,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int typeF_datarate; /* TypeF,datarate, please refer BITMAP of
    // EM_ALS_READER_M_SPDRATE*/
    // unsigned int typeV_subcarrier; /* 0: subcarrier, 1 :dual subcarrier*/
    // }s_mtk_nfc_em_als_readerm_req;
    public static class NfcEmAlsReadermReq extends NfcEmReq {

        public static final int CONTENT_SIZE = 32;
        public int mAction;
        public int mSupportType;
        public int mTypeADataRate;
        public int mTypeBDataRate;
        public int mTypeVDataRate;
        public int mTypeFDataRate;
        public int mTypeVSubcarrier;
        public int mTypeVCodingMode;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mSupportType = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeADataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeBDataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeVDataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeFDataRate = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeVSubcarrier = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTypeVCodingMode = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req action: " + mAction);
            buffer.put(DataConvert.intToLH(mSupportType));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req supporttype: "
                    + mSupportType);
            buffer.put(DataConvert.intToLH(mTypeADataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req typeA_datarate: "
                    + mTypeADataRate);
            buffer.put(DataConvert.intToLH(mTypeBDataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req typeB_datarate: "
                    + mTypeBDataRate);
            buffer.put(DataConvert.intToLH(mTypeVDataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req typeV_datarate: "
                    + mTypeVDataRate);
            buffer.put(DataConvert.intToLH(mTypeFDataRate));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req typeF_datarate: "
                    + mTypeFDataRate);
            buffer.put(DataConvert.intToLH(mTypeVSubcarrier));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req typeV_subcarrier: "
                    + mTypeVSubcarrier);
            buffer.put(DataConvert.intToLH(mTypeVCodingMode));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_req typeV_codingmode: "
                    + mTypeVCodingMode);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_readerm_ntf{
    // int result; /*0:Success,Tag connected, 1: Fail, 2:Tag disconnected*/
    // unsigned int isNDEF; /*1:NDEF, 0: Non-NDEF, 2: NDEF with Read function
    // only*/
    // unsigned int UidLen;
    // unsigned char Uid[10];
    // }s_mtk_nfc_em_als_readerm_ntf;
    public static class NfcEmAlsReadermNtf implements RawOperation {

        public static final int DATA_MAX_LENGTH = 10;
        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE * 3 + DATA_MAX_LENGTH;
        public int mResult;
        public int mIsNdef;
        public int mUidLen;
        public byte[] mUid;

        public NfcEmAlsReadermNtf() {
            mUid = new byte[DATA_MAX_LENGTH];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_readerm_ntf result: " + mResult);
            mIsNdef = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_readerm_ntf isNDEF: " + mIsNdef);
            mUidLen = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_readerm_ntf UidLen: " + mUidLen);
            buffer.get(mUid, 0, mUidLen);
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]s_mtk_nfc_em_als_readerm_ntf mUid: "
                    + new String(mUid));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mResult));
            buffer.put(DataConvert.intToLH(mIsNdef));
            buffer.put(DataConvert.intToLH(mUidLen));
            buffer.put(mUid, 0, mUidLen);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_readerm_rsp{
    // int result; /*0:Success, 1: Fail,*/
    // }s_mtk_nfc_em_als_readerm_rsp;
    public static class NfcEmAlsReadermRsp extends NfcEmRsp {
    }

    // typedef struct mtk_nfc_em_als_readerm_opt_req{
    // int action; /*Action, please refer ENUM of EM_OPT_ACTION*/
    // s_mtk_nfc_tag_write_ndef ndef_write;
    // }s_mtk_nfc_em_als_readerm_opt_req;
    public static class NfcEmAlsReadermOptReq extends NfcEmReq {

        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE + NfcTagWriteNdef.CONTENT_SIZE;
        public int mAction;
        public NfcTagWriteNdef mTagWriteNdef;

        public NfcEmAlsReadermOptReq() {
            mTagWriteNdef = new NfcTagWriteNdef();
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mAction = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mTagWriteNdef.readRaw(buffer);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mAction));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_opt_req action: "
                    + mAction);
            mTagWriteNdef.writeRaw(buffer);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    // typedef struct mtk_nfc_em_als_readerm_opt_rsp{
    // int result; /*0:Success,1:Fail*/
    // s_mtk_nfc_tag_read_ndef ndef_read;
    // }s_mtk_nfc_em_als_readerm_opt_rsp;
    public static class NfcEmAlsReadermOptRsp implements RawOperation {

        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE + NfcTagReadNdef.CONTENT_SIZE;
        public int mResult;
        public NfcTagReadNdef mTagReadNdef;

        public NfcEmAlsReadermOptRsp() {
            mTagReadNdef = new NfcTagReadNdef();
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "[NfcEmReqRsp]mtk_nfc_em_als_readerm_opt_rsp result: "
                    + mResult);
            mTagReadNdef.readRaw(buffer);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mResult));
            mTagReadNdef.writeRaw(buffer);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }
    
//    typedef struct mtk_nfc_tag_read_ndef {
//        e_mtk_nfc_ndef_type  ndef_type;
//        unsigned  char lang[3];
//        unsigned  char recordFlags;
//        unsigned  char recordId[32];
//        unsigned  char recordTnf;
//        unsigned  int  length;      
//        unsigned  char data[NDEF_DATA_LEN];
//      } s_mtk_nfc_tag_read_ndef;
      
    public static class NfcTagReadNdef implements RawOperation {
        private static final int LANG_LENGTH = 3;
        private static final int RECORD_ID_LENGTH = 32;
        private static final int DATA_LENGTH = 512;
        public static final int CONTENT_SIZE = 2 + NfcNdefType.CONTENT_SIZE * 2
                + LANG_LENGTH + RECORD_ID_LENGTH + DATA_LENGTH;
        public NfcNdefType mNdefType;
        public byte[] mLang;
        public byte mRecordFlags;
        public byte[] mRecordId;
        public byte mRecordTnf;
        public int mLength;
        public byte[] mData;

        public NfcTagReadNdef() {
            mNdefType = new NfcNdefType();
            mLang = new byte[LANG_LENGTH];
            mRecordId = new byte[RECORD_ID_LENGTH];
            mData = new byte[DATA_LENGTH];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {

            mNdefType.readRaw(buffer);
            buffer.get(mLang, 0, LANG_LENGTH);
            mRecordFlags = buffer.get();
            buffer.get(mRecordId, 0, RECORD_ID_LENGTH);
            mRecordTnf = buffer.get();
            mLength = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            buffer.get(mData, 0, DATA_LENGTH);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            mNdefType.readRaw(buffer);
            buffer.put(mLang, 0, LANG_LENGTH);
            buffer.put(mRecordFlags);
            buffer.put(mRecordId, 0, RECORD_ID_LENGTH);
            buffer.put(mRecordTnf);
            buffer.put(DataConvert.intToLH(mLength));
            buffer.put(mData, 0, DATA_LENGTH);
            mNdefType.writeRaw(buffer);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }

    }

    public static class NfcTagWriteNdef implements RawOperation {

        public static final int CONTENT_SIZE = NfcCommand.INT_SIZE + NfcNdefType.CONTENT_SIZE
                + NfcNdefLangType.CONTENT_SIZE + NfcTagWriteNdefData.CONTENT_SIZE;
        public NfcNdefType mNdefType;
        public NfcNdefLangType mNdefLangType;
        public int mLength;
        public NfcTagWriteNdefData mNdefData;

        public NfcTagWriteNdef() {
            mNdefType = new NfcNdefType();
            mNdefLangType = new NfcNdefLangType();
            mNdefData = new NfcTagWriteNdefData();
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mNdefType.readRaw(buffer);
            mNdefLangType.readRaw(buffer);
            mLength = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            mNdefData.readRaw(buffer);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            mNdefType.writeRaw(buffer);
            mNdefLangType.writeRaw(buffer);
            buffer.put(DataConvert.intToLH(mLength));
            mNdefData.writeRaw(buffer);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    public static class NfcNdefType implements RawOperation {
        public static final int CONTENT_SIZE = 4;
        public static final int URI = 0;
        public static final int TEXT = 1;
        public static final int SP = 2;
        public static final int OTHERS = 3;

        public int mEnumValue;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mEnumValue = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mEnumValue));
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    public static class NfcNdefLangType implements RawOperation {
        public static final int CONTENT_SIZE = 4;
        public static final int DEFAULT = 0;
        public static final int DE = 1;
        public static final int EN = 2;
        public static final int FR = 3;

        public int mEnumValue;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mEnumValue = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mEnumValue));
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    public static class NfcTagWriteNdefData implements RawOperation {
        public static final int CONTENT_SIZE = EXTTagT.CONTENT_SIZE;
        public byte[] mData;

        public NfcTagWriteNdefData() {
            mData = new byte[CONTENT_SIZE];
        }

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            buffer.get(mData, 0, CONTENT_SIZE);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(mData, 0, CONTENT_SIZE);
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }
    }

    public static class SmartPosterT {
        public static final int CONTENT_SIZE = 132;
        public static final int COMPANY_LENGTH = 64;
        public static final int COMPANY_URL_LENGTH = 64;
        public byte[] mCompany;
        public short mCompanyLength;
        public byte[] mCompanyUrl;
        public short mCompanyUrlLength;

        public SmartPosterT() {
            mCompany = new byte[COMPANY_LENGTH];
            mCompanyUrl = new byte[COMPANY_URL_LENGTH];
        }

        public byte[] getByteArray() {
            byte[] array = new byte[CONTENT_SIZE];
            System.arraycopy(mCompany, 0, array, 0, mCompany.length);
            byte[] shortArray = DataConvert.shortToLH(mCompanyLength);
            System.arraycopy(shortArray, 0, array, COMPANY_LENGTH, shortArray.length);
            System.arraycopy(mCompanyUrl, 0, array, COMPANY_LENGTH + shortArray.length,
                    mCompanyUrl.length);
            shortArray = DataConvert.shortToLH(mCompanyUrlLength);
            System.arraycopy(shortArray, 0, array, COMPANY_LENGTH + shortArray.length
                    + COMPANY_URL_LENGTH, shortArray.length);
            return array;
        }

    }

    public static class VcardT {
        public static final int CONTENT_SIZE = 576;
        public static final int NAME_LENGTH = 64;
        public static final int COMPANY_LENGTH = 64;
        public static final int TITLEP_LENGTH = 64;
        public static final int TEL_LENGTH = 32;
        public static final int EMAIL_LENGTH = 64;
        public static final int ADDRESS_LENGTH = 128;
        public static final int POSTAL_CODE_LENGTH = 32;
        public static final int CITY_LENGTH = 64;
        public static final int COMPANY_URL_LENGTH = 64;
        public byte[] mName;
        public byte[] mCompany;
        public byte[] mTitlep;
        public byte[] mTel;
        public byte[] mEmail;
        public byte[] mAddress;
        public byte[] mPostalCode;
        public byte[] mCity;
        public byte[] mCompanyUrl;

        public VcardT() {
            mName = new byte[NAME_LENGTH];
            mCompany = new byte[COMPANY_LENGTH];
            mTitlep = new byte[TITLEP_LENGTH];
            mTel = new byte[TEL_LENGTH];
            mEmail = new byte[EMAIL_LENGTH];
            mAddress = new byte[ADDRESS_LENGTH];
            mPostalCode = new byte[POSTAL_CODE_LENGTH];
            mCity = new byte[CITY_LENGTH];
            mCompanyUrl = new byte[COMPANY_URL_LENGTH];
        }
    }

    public static class TextT {
        public static final int CONTENT_SIZE = TAG_WRITE_MAXDATA + 2;
        public static final int DATA_LENGTH = TAG_WRITE_MAXDATA;
        public byte[] mData;
        public short mDataLength;

        public TextT() {
            mData = new byte[DATA_LENGTH];
        }

        public byte[] getByteArray() {

            byte[] array = new byte[CONTENT_SIZE];
            System.arraycopy(mData, 0, array, 0, mData.length);
            byte[] shortArray = DataConvert.shortToLH(mDataLength);
            System.arraycopy(shortArray, 0, array, DATA_LENGTH, shortArray.length);
            return array;
        }
    }

    public static class UrlT {
        public static final int CONTENT_SIZE = 66;
        public static final int DATA_LENGTH = 64;
        public byte[] mUrlData;
        public short mUrlDataLength;

        public UrlT() {
            mUrlData = new byte[DATA_LENGTH];
        }

        public byte[] getByteArray() {
            byte[] array = new byte[CONTENT_SIZE];
            System.arraycopy(mUrlData, 0, array, 0, mUrlData.length);
            byte[] shortArray = DataConvert.shortToLH(mUrlDataLength);
            System.arraycopy(shortArray, 0, array, DATA_LENGTH, shortArray.length);
            return array;
        }

    }

    public static class EXTTagT {
        public static final int CONTENT_SIZE = TAG_WRITE_MAXDATA + UrlT.CONTENT_SIZE;
        public static final int DATA_LENGTH = 64;
        public byte[] mExtTagType;
        public byte[] mExtData;
        public short mExtLength;

        public EXTTagT() {
            mExtTagType = new byte[DATA_LENGTH];
            mExtData = new byte[TAG_WRITE_MAXDATA];
        }
    }

    // //////

    public abstract static class NfcEmReq implements RawOperation {

        public static final int CONTENT_SIZE = 0;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }

    }

    public abstract static class NfcEmRsp implements RawOperation {

        public static final int CONTENT_SIZE = 4;
        public int mResult;

        public void readRaw(ByteBuffer buffer) throws NullPointerException,
                BufferUnderflowException {
            mResult = DataConvert.byteToInt(DataConvert.getByteArr(buffer));
            Elog.d(NfcMainPage.TAG, "NfcEmRsp result: " + mResult);
        }

        public void writeRaw(ByteBuffer buffer) throws NullPointerException,
                BufferOverflowException {
            buffer.put(DataConvert.intToLH(mResult));
        }

        public int getContentSize() {
            return CONTENT_SIZE;
        }

    }

    interface RawOperation {
        void readRaw(ByteBuffer buffer) throws NullPointerException, BufferUnderflowException;

        void writeRaw(ByteBuffer buffer) throws NullPointerException, BufferOverflowException;

        int getContentSize();
    }
}
