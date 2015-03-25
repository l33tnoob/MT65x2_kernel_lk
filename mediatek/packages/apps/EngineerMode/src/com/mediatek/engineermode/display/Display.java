/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.display;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;
import com.mediatek.xlog.Xlog;

import java.io.IOException;

/**
 * Test LCD & LCM ON AND OFF, SET duty number.
 * 
 * @author mtk54040
 * 
 */
public class Display extends Activity implements OnClickListener {
    private static final String TAG = "EM/Display";

    // private String mount = "mount -t debugfs none /sys/kernel/debug";
    // private String umount = "umount /sys/kernel/debug";
    private static final String CMD_LCD_ON = "echo 255 > /sys/class/leds/lcd-backlight/brightness";
    private static final String CMD_LCD_OFF = "echo 0 > /sys/class/leds/lcd-backlight/brightness";
    // private String lcmCmdON =
    // "echo lcd:on > /sys/kernel/debug/mtkfb && echo lcm:on > /sys/kernel/debug/mtkfb";

    // private String lcmCmdOFF =
    // "echo lcd:off > /sys/kernel/debug/mtkfb && echo lcm:off > /sys/kernel/debug/mtkfb";
    private static final String DUTY_FILE = "/sys/class/leds/lcd-backlight/duty";
    private static final String FAIL_STRING = "FFFFFFFF";
    private static final String SHELL_CMD = "/system/bin/sh";
    private static final String SHELL_CAT = "-c";
    private static final String SHELL_ECHO = " echo ";
    private static final String SHELL_DIRECTION = " > ";

    private Button mBtnLcdON;
    private Button mBtnLcdOFF;
    private Button mBtnLcmON;
    private Button mBtnLcmOFF;
    private Button mBtnSet;
    private EditText mEdit;

    // const number
    private static final int MAX_NUM = 63;
    private static final int MAX_LENGTH = 2;

    // command index
    private static final int FB0_LCM_POWER_ON = 4;
    private static final int FB0_LCM_POWER_OFF = 5;
    // return flag
    private static final int RETURN_FAIL = -1;
    private static final int RETURN_SUCCESS = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
//        Xlog.v(TAG, "-->onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.display);

        // Initialize the UI component
        mBtnLcdON = (Button) findViewById(R.id.Display_lcd_on);
        mBtnLcdOFF = (Button) findViewById(R.id.Display_lcd_off);
        mBtnLcmON = (Button) findViewById(R.id.Display_lcm_on);
        mBtnLcmOFF = (Button) findViewById(R.id.Display_lcm_off);

        mBtnSet = (Button) findViewById(R.id.Display_set);
        mEdit = (EditText) findViewById(R.id.Display_Edit_Value);

        mBtnSet.setOnClickListener(this);
        mBtnLcdON.setOnClickListener(this);
        mBtnLcmON.setOnClickListener(this);
        mBtnLcdOFF.setOnClickListener(this);
        mBtnLcmOFF.setOnClickListener(this);

    }
    
    


    /* (non-Javadoc)
     * @see android.view.View.OnClickListener#onClick(android.view.View)
     */
    public void onClick(View arg0) {  
        Xlog.v(TAG, "-->onClick");
        boolean sucessFlag = false;
        if (arg0.equals(mBtnSet)) {
            String editString = mEdit.getText().toString();
            boolean falseFlag = false;
            Xlog.v(TAG, "-->onClick editString = " + editString);
            // empty or wrong input
            if (null == editString || editString.isEmpty()
                    || editString.length() > MAX_LENGTH) {
                falseFlag = true;
            }
            // invalid input
            try {
                int tmpNum = Integer.valueOf(editString);
                if (tmpNum > MAX_NUM) {
                    falseFlag = true;
                }
            } catch (NumberFormatException e) {
                Xlog.v(TAG, "-->onClick editString = "  + e.getMessage());
            }

            if (falseFlag) {
                Toast.makeText(this, R.string.Display_input_invalid,
                        Toast.LENGTH_SHORT).show();
                return;
            }


            String[] cmd = { SHELL_CMD, SHELL_CAT,
                    SHELL_ECHO + editString + SHELL_DIRECTION + DUTY_FILE };

            int ret = RETURN_FAIL;
            try {
                ret = ShellExe.execCommand(cmd);
            } catch (IOException e) {
                Xlog.d(TAG, cmd.toString() + e.getMessage());
            }
            if (RETURN_SUCCESS == ret) {
                sucessFlag = true;
            } 

        } else if (arg0.equals(mBtnLcdON)) {
            String[] cmd = { SHELL_CMD, SHELL_CAT, CMD_LCD_ON };

            int ret = RETURN_FAIL;
            try {
                ret = ShellExe.execCommand(cmd);
            } catch (IOException e) {
                Xlog.d(TAG, cmd.toString() + e.getMessage());
            }
            if (RETURN_SUCCESS == ret) {
                sucessFlag = true;
            } 

        } else if (arg0.equals(mBtnLcdOFF)) {
            String[] cmd = { SHELL_CMD, SHELL_CAT, CMD_LCD_OFF };

            int ret = RETURN_FAIL;
            try {
                ret = ShellExe.execCommand(cmd);
            } catch (IOException e) {
                Xlog.d(TAG, cmd.toString() + e.getMessage());
            }
            if (RETURN_SUCCESS == ret) {
                sucessFlag = true;
            } 

        } else if (arg0.equals(mBtnLcmON)) {
            
            Xlog.v(TAG, "-->onClick  BtnLcmON");
            if (RETURN_SUCCESS == onLcmPower()) {
                sucessFlag = true;
            } 

        } else if (arg0.equals(mBtnLcmOFF)) {
            if (RETURN_SUCCESS == offLcmPower()) {
                sucessFlag = true;
            }
        }
        if (sucessFlag) {
            Toast.makeText(this, R.string.Display_set_ok,
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, R.string.Display_set_fail,
                    Toast.LENGTH_SHORT).show();
        }

    }

    /**
     * Set LCM POWER ON
     * 
     * @return
     */
    private static int onLcmPower() {
        return controlFb0(FB0_LCM_POWER_ON);
    }

    /**
     * Set LCM POWER OFF
     * 
     * @return
     */
    private static int offLcmPower() {
        return controlFb0(FB0_LCM_POWER_OFF);
    }

    /**
     * TRANSMIT command to driver file
     * 
     * @return
     */
    private static int controlFb0(int... param) {
        int valueRet = RETURN_FAIL;
        AFMFunctionCallEx fb0Ctrl = new AFMFunctionCallEx();
        // boolean result = fb0Ctrl
        // .startCallFunctionStringReturn(AFMFunctionCallEx.FUNCTION_EM_FB0_IOCTL);
        //
        // if (!result) {
        // return RETURN_FAIL;
        // }

        if (fb0Ctrl
                .startCallFunctionStringReturn(AFMFunctionCallEx.FUNCTION_EM_FB0_IOCTL)) {
            fb0Ctrl.writeParamNo(param.length);
            for (int i : param) {
                fb0Ctrl.writeParamInt(i);
            }

            FunctionReturn resultStr;
            do {
                resultStr = fb0Ctrl.getNextResult();               
                // if (resultStr.returnString == "") {
                if (resultStr.mReturnString.isEmpty()) {
                    break;
                } else {
                    if (resultStr.mReturnString.equalsIgnoreCase(FAIL_STRING)) {
                        valueRet = RETURN_FAIL;
                        break;
                    }
                    try {
                        valueRet = Integer.valueOf(resultStr.mReturnString);
                    } catch (NumberFormatException e) {
                        Xlog.d(TAG, resultStr.mReturnString);
                        valueRet = RETURN_FAIL;
                    }
                }

            } while (resultStr.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);

            if (resultStr.mReturnCode == AFMFunctionCallEx.RESULT_IO_ERR) {
                // error
                valueRet = RETURN_FAIL;
            }
        }
        return valueRet;

    }

}
