package com.mediatek.deviceregister;

import android.os.Build;
import android.util.Log;

import java.util.zip.CRC32;

public class RegisterMessage {

    private static final String SUB_TAG = Const.TAG + "RegisterMessage";
    private byte mProtocolVersion = Const.PROTOCOL_VERSION_MEID;
    private byte mCommandType = Const.COMMAND_TYPE_SEND;
    private byte mDataLength = 0;
    private byte mFillByte = 0x0;
    private String mData = "";
    private String mChecksum = "";
    private RegisterService mService;

    public RegisterMessage(RegisterService service) {
        mService = service;
    }

    public byte[] getRegisterMessage() {
        Log.d(SUB_TAG, "getRegisterMessage");
        mData = generateMessageData();
        mDataLength = (byte) mData.length();

        int byteArrayLenth = 4 + mDataLength;
        byte[] message = new byte[byteArrayLenth];
        message[0] = mProtocolVersion;
        message[1] = mCommandType;
        message[2] = mDataLength;
        message[3] = mFillByte;
        byte[] dataByte = mData.getBytes();
        int i = 4;
        for (int j = 0; j < dataByte.length; j++) {
            message[i] = dataByte[j];
            i++;
        }

        mChecksum = generateChecksum(message);
        Log.d(SUB_TAG, "checksum:" + mChecksum);
        byte[] crcByte = mChecksum.getBytes();

        byte[] messageFinal = new byte[message.length + Const.CHECKSUM_LENGTH];

        int k = 0;
        for (int j = 0; j < message.length; j++) {
            messageFinal[k] = message[j];
            k++;
        }

        for (int j = 0; j < crcByte.length; j++) {
            messageFinal[k] = crcByte[j];
            k++;
        }
        return messageFinal;
    }

    private String generateMessageData() {
        Log.d(SUB_TAG, "generateMessageData");
        String beginTag = "<a1>";
        String endTag = "</a1>";
        String modelBeginTag = "<b1>";
        String modelEndTag = "</b1>";
        String meidBeginTag = "<b2>";
        String meidEndTag = "</b2>";
        String imsiBeginTag = "<b3>";
        String imsiEndTag = "</b3>";
        String softwareVersionBeginTag = "<b4>";
        String softwareVersionEndTag = "</b4>";
        StringBuffer data = new StringBuffer();
        data.append(beginTag);
        data.append(modelBeginTag).append(getModel()).append(modelEndTag);
        data.append(meidBeginTag).append(getMeid()).append(meidEndTag);
        data.append(imsiBeginTag).append(getIMSI()).append(imsiEndTag);
        data.append(softwareVersionBeginTag).append(getSoftwareVersion()).append(softwareVersionEndTag);
        data.append(endTag);
        Log.d(SUB_TAG, "message:" + data.toString());
        return data.toString();
    }

    private String getModel() {
        String globleModel = Build.MODEL;
        int index = globleModel.indexOf(" ");
        if (index < 0 || index == globleModel.length()) {
            Log.w(SUB_TAG, "Model in Build.MODEL may be error!!, globleModel = " + globleModel);
            return globleModel;
        }

        String manufacturer = globleModel.substring(0, index);
        // String manufacturer = Customization.getManufacturer();
        Log.d(SUB_TAG, "manufacturer:" + manufacturer);
        if (manufacturer.length() > Const.MANUFACTURE_MAX_LENGTH) {
            Log.w(SUB_TAG, "Manufacturer length > " + Const.MANUFACTURE_MAX_LENGTH + ", cut it!");
            manufacturer = manufacturer.substring(0, Const.MANUFACTURE_MAX_LENGTH);
        }

        // String model = Customization.getModel();
        String model = globleModel.substring(index + 1, globleModel.length());
        Log.d(SUB_TAG, "model:" + model);
        model = model.replaceAll("-", " ");
        if (model.indexOf(manufacturer) != -1) {
            model = model.replaceFirst(manufacturer, "");
        }

        String result = manufacturer + "-" + model;
        if (result.length() > Const.MODEL_MAX_LENGTH) {
            Log.w(SUB_TAG, "Model length > " + Const.MODEL_MAX_LENGTH + ", cut it!");
            result = result.substring(0, Const.MODEL_MAX_LENGTH);
        }

        return result;
    }

    private String getMeid() {
        return mService.getDeviceId();
    }

    private String getIMSI() {
        return mService.getCurrentLogonImsi();
    }

    private String getSoftwareVersion() {
        return Build.DISPLAY;
    }

    private String generateChecksum(byte[] data) {
        CRC32 checksum = new CRC32();
        checksum.update(data);
        long value = checksum.getValue();
        // return Long.toHexString(value);
        String crcString = Long.toHexString(value);
        int crcStringLength = crcString.length();
        if (crcStringLength < Const.CHECKSUM_LENGTH) {
            String prefix = "";
            for (int i = crcStringLength; i < Const.CHECKSUM_LENGTH; i++) {
                prefix += "0";
            }
            crcString = prefix + crcString;
        }
        return crcString;
    }

}
