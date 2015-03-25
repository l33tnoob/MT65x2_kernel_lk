package com.hissage.util.data;

import java.util.Hashtable;

import com.hissage.R;

import android.content.Context;
import android.text.TextUtils;
import android.widget.Toast;

public class NmsApiErrorCode {
    private static NmsApiErrorCode mInstance;
    private static Hashtable<Integer, String> table;

    private Context context;

    private NmsApiErrorCode(Context context) {
        this.context = context;
        table = new Hashtable<Integer, String>();
        table.put(-1, context.getString(R.string.STR_NMS_ERR_GENERAL));
        table.put(-2, context.getString(R.string.STR_NMS_ERR_INVALID_ARGUMENT));
        table.put(-3, context.getString(R.string.STR_NMS_ERR_NETWORK_BLOCK));
        table.put(-4, context.getString(R.string.STR_NMS_ERR_PHONE_NOT_ACTIVATED));
        table.put(-5, context.getString(R.string.STR_NMS_ERR_SIM_NOT_ACTIVATED));
        table.put(-6, context.getString(R.string.STR_NMS_ERR_INVALID_MSG_DB_ID));
        table.put(-7, context.getString(R.string.STR_NMS_ERR_ADD_MSG_DB_FAILED));
        table.put(-8, context.getString(R.string.STR_NMS_ERR_UPDATE_MSG_DB_FAILED));
        table.put(-9, context.getString(R.string.STR_NMS_ERR_GET_MSG_DB_DATA_FAILED));
        table.put(-10, context.getString(R.string.STR_NMS_ERR_INVALID_CONTACT_DB_ID));
        table.put(-11, context.getString(R.string.STR_NMS_ERR_ADD_CONTACT_DB_FAILED));
        table.put(-12, context.getString(R.string.STR_NMS_ERR_UPDATE_CONTACT_DB_FAILED));
        table.put(-13, context.getString(R.string.STR_NMS_ERR_GET_CONTACT_DB_DATA_FAILED));
        table.put(-14, context.getString(R.string.STR_NMS_ERR_NEW_IP_MSG_ERROR_FORMAT));
        table.put(-15, context.getString(R.string.STR_NMS_ERR_NEW_IP_MSG_INVALID_TO));
        table.put(-16, context.getString(R.string.STR_NMS_ERR_GNEW_IP_MSG_WITH_UNKNOWN_USER));
        table.put(-17, context.getString(R.string.STR_NMS_ERR_NEW_IP_MSG_GCHAT_NO_MBMBER));
        table.put(-18, context.getString(R.string.STR_NMS_ERR_GNEW_IP_MSG_GCHAT_DEAD));
        table.put(-19, context.getString(R.string.STR_NMS_ERR_NEW_IP_MSG_GCHAT_INVALID_SIM_ID));
        table.put(-20, context.getString(R.string.STR_NMS_ERR_RESEND_IP_MSG_FOR_SYS_MSG));
        table.put(-21, context.getString(R.string.STR_NMS_ERR_RESEND_IP_MSG_INVALID_STATUS));
        table.put(-22, context.getString(R.string.STR_NMS_ERR_GCHAT_IS_DEAD));
        table.put(-23, context.getString(R.string.STR_NMS_ERR_GCHAT_INVALID_MEMBERS));
        table.put(-24, context.getString(R.string.STR_NMS_ERR_GCHAT_INVALID_MEMBER_WITH_SELF));
        table.put(-25, context.getString(R.string.STR_NMS_ERR_GCHAT_EXCEED_MAX_MEMBER_COUNT));
        table.put(-26, context.getString(R.string.STR_NMS_ERR_GCHAT_INVALID_NAME));
        table.put(-27, context.getString(R.string.STR_NMS_ERR_GCHAT_SIM_IS_NOT_ACTIVATED));
        table.put(-28, context.getString(R.string.STR_NMS_ERR_NEW_IP_MSG_GCHAT_INVALID_SIM_ID));
        table.put(-29, context.getString(R.string.STR_NMS_ERR_GET_DOWNLOAD_URL_NOT_INBOX_MSG));
        table.put(-30, context.getString(R.string.STR_NMS_ERR_GET_DOWNLOAD_URL_INVALID_URL));
        table.put(-31, context.getString(R.string.STR_NMS_ERR_GET_ATTACH_NAME_INVALID_BODY_FORMAT));
        table.put(-32,
                context.getString(R.string.STR_NMS_ERR_SET_DOWNLOADED_FILE_INVALID_FILE_NAME));
        table.put(-33, context.getString(R.string.STR_NMS_ERR_SET_DOWNLOADED_FILE_NOT_EXIST));
        table.put(-34, context.getString(R.string.STR_NMS_ERR_SET_DOWNLOADED_FILE_NOT_INBOX_MSG));
        table.put(-35, context.getString(R.string.STR_NMS_ERR_SIM_INFO_NOT_FOUND));
        table.put(-36, context.getString(R.string.STR_NMS_ERR_SIM_IN_INVALID_STATUS));
        table.put(-37, context.getString(R.string.STR_NMS_ERR_CMD_PROCESSING));
        table.put(-38, context.getString(R.string.STR_NMS_ERR_CMD_GENERAL));

        table.put(-41, context.getString(R.string.STR_OPER_FAILED_FOR_NOT_DEFAULT_SMS_APP));

        table.put(-1000, context.getString(R.string.STR_NMS_ERR_CODE_END));

    }

    public static NmsApiErrorCode get(Context context) {
        if (null == mInstance) {
            mInstance = new NmsApiErrorCode(context);
        }
        return mInstance;
    }

    public void showApiResultToast(int errorCode) {
        String s = table.get(errorCode);
        if (!TextUtils.isEmpty(s)) {
            Toast.makeText(context, s, Toast.LENGTH_SHORT).show();
        }
    }
}
