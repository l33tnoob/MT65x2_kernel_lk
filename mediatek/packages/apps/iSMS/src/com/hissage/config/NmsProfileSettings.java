package com.hissage.config;

import java.io.File;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.TextUtils;

import com.hissage.util.log.NmsLog;

/**
 * The Class NmsProfileSettings, include isms user avatar\name etc.
 */
public class NmsProfileSettings {
    
    private static final String TAG = "NmsProfileSettings";
    
    /** The user name. */
    public String name;
    
    public String signature;
    
    /** The phone number. */
    public String phoneNum;
    
    /** The avatar file name. */
    public String fileName;

	/** The device imsi .*/
	public String imsi;

    public static final String userDefaultImgPath = "drawable/default_userphoto.png";
    
    public static final int CMD_SET_HESINE_INFO_ACK = 0x11c;
    
    public static final int CMD_GET_HESINE_INFO_ACK = 0x12a;
    
    public static final String CMD_RESULT_KEY = "cmd_result_key";
    
    public static final String CMD_CODE = "cmd_code";

    /**
     * Sets the avatar file to profile
     *
     * @param img the avatar byte array.
     */
    public synchronized void setImgPath(byte img[]) {
        try {

            if (img != null && img.length > 0) {
                File f = new File(fileName);
                if (f.exists()) {
                    NmsLog.trace(TAG, "file [" + fileName + "] is exists,now move it.");
                    f.delete();
                }
                NmsCommonUtils.nmsStream2File(img, fileName);
            } else {
                NmsLog.trace(TAG, "set user icon,but length is 0");
            }
        } catch (Exception e) {
            NmsLog.trace(TAG, "save user icon error,exception:" + NmsLog.nmsGetStactTrace(e));

        }
    }

    public Bitmap getProfileSettingsAvatar() {
        if (TextUtils.isEmpty(fileName)) {
            NmsLog.error(TAG, "fileName is empty");
            return null;
        }

        Bitmap result = null;
        try {
            result = BitmapFactory.decodeFile(fileName);
        } catch (Exception e) {
            NmsLog.warn(TAG, "BitmapFactory.decodeFile failed(self bitmap)");
        }

        return result;
    }
}
