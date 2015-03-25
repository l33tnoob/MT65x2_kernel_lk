/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mediatek.engineermode.hqanfc;

import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.mediatek.engineermode.R;

import java.nio.ByteBuffer;

/**
 * 
 * @author mtk54045
 */
public class NfcCommand {

    public static final int RECEIVE_DATA_SIZE = 1024;
    public static final int MAIN_MESSAGE_SIZE = 8;
    public static final int INT_SIZE = 4;
    public static final String ACTION_PRE = "com.mediatek.hqanfc.";
    public static final String MESSAGE_CONTENT_KEY = "content";
    private static final int POS_3 = 3;
    private static final int POS_4 = 4;
    private static final int POS_5 = 5;
    private static final int MOVE_BIT_8 = 8;
    private static final int MOVE_BIT_16 = 16;
    private static final int MOVE_BIT_24 = 24;
    private static final int NUMBER_OXFF = 0xff;
    private int mMessageType;
    private ByteBuffer mMessageContent;

    public NfcCommand(int msgType, ByteBuffer bufferCont) {
        mMessageType = msgType;
        mMessageContent = bufferCont;
    }

    public ByteBuffer getMessageContent() {
        return mMessageContent;
    }

    public void setMessageContent(ByteBuffer mMessageLenContent) {
        this.mMessageContent = mMessageLenContent;
    }

    public int getMessageType() {
        return mMessageType;
    }

    public void setMessageType(int messageType) {
        this.mMessageType = messageType;
    }

    public static class DataConvert {

        public static byte[] intToLH(int n) {
            byte[] b = new byte[INT_SIZE];
            b[0] = (byte) (n & NUMBER_OXFF);
            b[1] = (byte) (n >> MOVE_BIT_8 & NUMBER_OXFF);
            b[2] = (byte) (n >> MOVE_BIT_16 & NUMBER_OXFF);
            b[POS_3] = (byte) (n >> MOVE_BIT_24 & NUMBER_OXFF);
            return b;
        }

        public static int byteToInt(byte[] b) {
            int n = 0;
            for (int i = 0; i < INT_SIZE; i++) {
                int c = b[POS_3 - i] & NUMBER_OXFF;
                n = (n << MOVE_BIT_8) + c;
            }
            return n;
        }

        public static byte[] shortToLH(short n) {
            byte[] b = new byte[2];
            b[0] = (byte) (n & NUMBER_OXFF);
            b[1] = (byte) (n >> MOVE_BIT_8 & NUMBER_OXFF);
            return b;
        }

        public static byte[] getByteArr(ByteBuffer buffer) {
            byte[] b = new byte[INT_SIZE];
            buffer.get(b);
            return b;
        }

        public static String printHexString(byte[] b, int length) {
            String string = "";
            int leng = b.length;
            if (0 != length) {
                leng = length;
            }

            for (int i = 0; i < leng; i++) {
                String hex = Integer.toHexString(b[i] & 0xFF);
                if (hex.length() == 1) {
                    hex = '0' + hex;
                }
                // string = string + hex.toUpperCase() + " ";
                string = string + "0x" + hex.toUpperCase() + " ";
            }
            return string;
        }

        public static String printHexString(byte b) {
            String string = "";
            String hex = Integer.toHexString(b & NUMBER_OXFF);
            if (hex.length() == 1) {
                hex = '0' + hex;
            }
            // string = string + hex.toUpperCase() + " ";
            string = string + "0x" + hex.toUpperCase() + " ";
            return string;
        }

        public static long readUnsignedInt(byte[] bytes) {
            long b0 = ((long) (bytes[0] & NUMBER_OXFF));
            long b1 = ((long) (bytes[1] & NUMBER_OXFF)) << MOVE_BIT_8;
            long b2 = ((long) (bytes[2] & NUMBER_OXFF)) << MOVE_BIT_16;
            long b3 = ((long) (bytes[POS_3] & NUMBER_OXFF)) << MOVE_BIT_24;
            return (long) (b0 | b1 | b2 | b3);
        }
    }

    /* FOR EM */
    // def SUPPORT_EM
    public static class CommandType {
        public static final int MTK_NFC_EM_START_CMD = 100;
        public static final int MTK_NFC_EM_ALS_READER_MODE_REQ = 101;
        public static final int MTK_NFC_EM_ALS_READER_MODE_RSP = 102;
        public static final int MTK_NFC_EM_ALS_READER_MODE_OPT_REQ = 103;
        public static final int MTK_NFC_EM_ALS_READER_MODE_OPT_RSP = 104;
        public static final int MTK_NFC_EM_ALS_P2P_MODE_REQ = 105;
        public static final int MTK_NFC_EM_ALS_P2P_MODE_RSP = 106;
        public static final int MTK_NFC_EM_ALS_CARD_MODE_REQ = 107;
        public static final int MTK_NFC_EM_ALS_CARD_MODE_RSP = 108;
        public static final int MTK_NFC_EM_POLLING_MODE_REQ = 109;
        public static final int MTK_NFC_EM_POLLING_MODE_RSP = 110;
        public static final int MTK_NFC_EM_TX_CARRIER_ALS_ON_REQ = 111;
        public static final int MTK_NFC_EM_TX_CARRIER_ALS_ON_RSP = 112;
        public static final int MTK_NFC_EM_VIRTUAL_CARD_REQ = 113;
        public static final int MTK_NFC_EM_VIRTUAL_CARD_RSP = 114;
        public static final int MTK_NFC_EM_PNFC_CMD_REQ = 115;
        public static final int MTK_NFC_EM_PNFC_CMD_RSP = 116;
        public static final int MTK_NFC_EM_POLLING_MODE_NTF = 117;
        public static final int MTK_NFC_EM_ALS_READER_MODE_NTF = 118;
        public static final int MTK_NFC_EM_ALS_P2P_MODE_NTF = 119;
        public static final int MTK_NFC_EM_STOP_CMD = 120;

        public static final int MTK_NFC_TESTMODE_SETTING_REQ = 127;  // option
        public static final int MTK_NFC_TESTMODE_SETTING_RSP = 128;
        public static final int MTK_EM_LOOPBACK_TEST_REQ = 129;  // loop back test
        public static final int MTK_EM_LOOPBACK_TEST_RSP = 130;
        public static final int MTK_NFC_SW_VERSION_QUERY = 131;  // version query
        public static final int MTK_NFC_SW_VERSION_RESPONSE = 132;
        public static final int MTK_NFC_FM_SWP_TEST_REQ = 201;  // swp test
        public static final int MTK_NFC_FM_SWP_TEST_NTF = 202;
        public static final int MTK_NFC_FM_SWP_TEST_RSP = 203;
        
        public static final int MTK_NFC_EM_MSG_END = 204;
    }

    // public static class CommandType {
    // public static final int MTK_NFC_EM_SETART_CMD = 100;
    // public static final int MTK_NFC_EM_ALS_READER_MODE_REQ = 101;
    // public static final int MTK_NFC_EM_ALS_READER_MODE_RSP = 102;
    // public static final int MTK_NFC_EM_ALS_READER_MODE_OPT_REQ = 103;
    // public static final int MTK_NFC_EM_ALS_READER_MODE_OPT_RSP = 104;
    // public static final int MTK_NFC_EM_ALS_P2P_MODE_REQ = 105;
    // public static final int MTK_NFC_EM_ALS_P2P_MODE_RSP = 106;
    // public static final int MTK_NFC_EM_ALS_CARD_MODE_REQ = 107;
    // public static final int MTK_NFC_EM_ALS_CARD_MODE_RSP = 108;
    // public static final int MTK_NFC_EM_POLLING_MODE_REQ = 109;
    // public static final int MTK_NFC_EM_POLLING_MODE_RSP = 110;
    // public static final int MTK_NFC_EM_TX_CARRIER_ALS_ON_REQ = 111;
    // public static final int MTK_NFC_EM_TX_CARRIER_ALS_ON_RSP = 112;
    // public static final int MTK_NFC_EM_VIRTUAL_CARD_REQ = 113;
    // public static final int MTK_NFC_EM_VIRTUAL_CARD_RSP = 114;
    // public static final int MTK_NFC_EM_PNFC_CMD_REQ = 115;
    // public static final int MTK_NFC_EM_PNFC_CMD_RSP = 116;
    // public static final int MTK_NFC_EM_POLLING_MODE_NTF = 117;
    // public static final int MTK_NFC_EM_STOP_CMD = 118;
    // }

    public static class BitMapValue {
        public static int getTypeValue(CheckBox[] checkBoxs) {
            int result = 0;
            result |= checkBoxs[0].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_A : 0;
            result |= checkBoxs[1].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_B : 0;
            result |= checkBoxs[2].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_F : 0;
            result |= checkBoxs[POS_3].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_V : 0;
           // result |= checkBoxs[POS_4].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_BPRIME : 0;
            result |= checkBoxs[POS_4].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_KOVIO : 0;
            return result;
        }

        public static int getTypeAbDataRateValue(RadioButton[] checkBoxs) {
            int result = 0;
            result |= checkBoxs[0].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_106 : 0;
            result |= checkBoxs[1].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_212 : 0;
            result |= checkBoxs[2].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_424 : 0;
            result |= checkBoxs[POS_3].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_848 : 0;
            return result;
        }

        public static int getTypeFDataRateValue(RadioButton[] checkBoxs) {
            int result = 0;
            result |= checkBoxs[0].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_212 : 0;
            result |= checkBoxs[1].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_424 : 0;
            return result;
        }

        public static int getTypeVDataRateValue(RadioButton[] checkBoxs) {
            int result = 0;
            result |= checkBoxs[0].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_662 : 0;
            result |= checkBoxs[1].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_2648 : 0;
            return result;
        }
/*
        public static int getTypeVSubcarrier(RadioButton[] radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_SUB_CARRIER;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_DUAL_SUB_CARRIER;
                break;
            default:
                break;
            }
            return result;
        }

        public static int getTypeVCodingMode(RadioButton[] radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_CODING_MODE_4;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_CODING_MODE_256;
                break;
            default:
                break;
            }
            return result;
        }
*/

/*        public static int getTypeADataRateValue(RadioGroup radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_106;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_212;
                break;
            case 2:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_424;
                break;
            case 3:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_848;
                break;
            default:
                break;
            }
            return result;
        }

        public static int getTypeBDataRateValue(RadioGroup radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_106;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_212;
                break;
            case 2:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_424;
                break;
            case 3:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_848;
                break;
            default:
                break;
            }
            return result;
        }

        public static int getTypeFDataRateValue(RadioGroup radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_212;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_424;
                break;
            default:
                break;
            }
            return result;
        }

        public static int getTypeVDataRateValue(RadioGroup radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_662;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_SPDRATE_2648;
                break;
            default:
                break;
            }
            return result;
        }

        public static int getTypeVSubcarrier(RadioGroup radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_SUB_CARRIER;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_DUAL_SUB_CARRIER;
                break;
            default:
                break;
            }
            return result;
        }

        public static int getTypeVCodingMode(RadioGroup radioGroup) {
            int checked = -1, result = 0;
            for (int i = 0; i < radioGroup.getChildCount(); i++) {
                if (radioGroup.getChildAt(i).getId() == radioGroup.getCheckedRadioButtonId()) {
                    checked = i;
                    break;
                }
            }
            switch (checked) {
            case 0:
                result = NfcCommand.EM_ALS_READER_M_CODING_MODE_4;
                break;
            case 1:
                result = NfcCommand.EM_ALS_READER_M_CODING_MODE_256;
                break;
            default:
                break;
            }
            return result;
        }
*/
        public static int getFunctionValue(CheckBox[] checkBoxs) {
            int result = 0;
            result |= checkBoxs[0].isChecked() ? NfcCommand.EM_ENABLE_FUNC_READER_MODE : 0;
            result |= checkBoxs[1].isChecked() ? NfcCommand.EM_ENABLE_FUNC_RCARDR_MODE : 0;
            result |= checkBoxs[2].isChecked() ? NfcCommand.EM_ENABLE_FUNC_P2P_MODE : 0;
            return result;
        }

        public static int getSwioValue(CheckBox[] checkBoxs) {
            int result = 0;
            result |= checkBoxs[0].isChecked() ? NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO1 : 0;
            result |= checkBoxs[1].isChecked() ? NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO2 : 0;
            result |= checkBoxs[2].isChecked() ? NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIOSE : 0;
            return result;
        }
    }

    /* BITMAP OF EM_ALS_READER_M_TYPE */
    public static final int EM_ALS_READER_M_TYPE_A = (1 << 0);
    public static final int EM_ALS_READER_M_TYPE_B = (1 << 1);
    public static final int EM_ALS_READER_M_TYPE_F = (1 << 2);
    public static final int EM_ALS_READER_M_TYPE_V = (1 << 3);
    public static final int EM_ALS_READER_M_TYPE_BPRIME = (1 << 4);
    public static final int EM_ALS_READER_M_TYPE_KOVIO = (1 << 5);
    /* BITMAP OF EM_ALS_READER_M_SPDRATE */
    public static final int EM_ALS_READER_M_SPDRATE_106 = (1 << 0);
    public static final int EM_ALS_READER_M_SPDRATE_212 = (1 << 1);
    public static final int EM_ALS_READER_M_SPDRATE_424 = (1 << 2);
    public static final int EM_ALS_READER_M_SPDRATE_848 = (1 << 3);
    public static final int EM_ALS_READER_M_SPDRATE_662 = (1 << 4);
    public static final int EM_ALS_READER_M_SPDRATE_2648 = (1 << 5);
    /* BITMAP OF EM_ENABLE_FUNC */
    public static final int EM_ENABLE_FUNC_READER_MODE = (1 << 0);
    public static final int EM_ENABLE_FUNC_RCARDR_MODE = (1 << 1);
    public static final int EM_ENABLE_FUNC_P2P_MODE = (1 << 2);
    /* BITMAP OF EM_ALS_CARD_M_SW_NUM */
    public static final int EM_ALS_CARD_M_SW_NUM_SWIO1 = (1 << 0);
    public static final int EM_ALS_CARD_M_SW_NUM_SWIO2 = (1 << 1);
    public static final int EM_ALS_CARD_M_SW_NUM_SWIOSE = (1 << 2);
    /* BITMAP OF EM_P2P_ROLE */
    public static final int EM_P2P_ROLE_INITIATOR_MODE = (1 << 0);
    public static final int EM_P2P_ROLE_TARGET_MODE = (1 << 1);
    /* BITMAP OF EM_P2P_MODE */
    public static final int EM_P2P_MODE_PASSIVE_MODE = (1 << 0);
    public static final int EM_P2P_MODE_ACTIVE_MODE = (1 << 1);

    public static final int EM_ALS_READER_M_SUB_CARRIER = 0;
    public static final int EM_ALS_READER_M_DUAL_SUB_CARRIER = 1;
    public static final int EM_ALS_READER_M_CODING_MODE_4 = 0;
    public static final int EM_ALS_READER_M_CODING_MODE_256 = 1;

    public static class RspResult {
        public static final int SUCCESS = 0;
        public static final int FAIL = 1;
        // TAG ADD
        public static final int NFC_STATUS_NOT_SUPPORT = (0xA);
        public static final int NFC_STATUS_INVALID_NDEF_FORMAT = (0x20); // FOR
                                                                         // NDEF
                                                                         // USE
        public static final int NFC_STATUS_INVALID_FORMAT = (0x21); // FOR NDEF
                                                                    // USE
        public static final int NFC_STATUS_NDEF_EOF_REACHED = (0x22);
        public static final int NFC_STATUS_NO_SIM = (0xE1);
        public static final int NFC_STATUS_REMOVE_SE = (0xE3);
        public static final int NFC_STATUS_LINK_UP = 1;
        public static final int NFC_STATUS_LINK_DOWN = 0;
    }

    public static class EmAction {
        public static final int ACTION_START = 0;
        public static final int ACTION_STOP = 1;
        public static final int ACTION_RUNINBG = 2;
    }

    public static class PollingPhase {
        public static final int PHASE_LISTEN = 0;
        public static final int PHASE_PAUSE = 1;
    }

    public static class P2pDisableCardM {
        public static final int NOT_DISABLE = 0;
        public static final int DISABLE = 1;
    }

    public static class ReaderModeRspResult {
        public static final int CONNECT = 0;
        public static final int FAIL = 1;
        public static final int DISCONNECT = 2;
    }

    public static class ReaderModeRspNdef {
        public static final int NDEF = 0;
        public static final int NON_NDEF = 1;
    }

    public static class EmOptAction {
        public static final int READ = 0;
        public static final int WRITE = 1;
        public static final int FORMAT = 2;
    }

}
