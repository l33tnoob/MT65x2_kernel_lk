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

package com.mediatek.stk.tests;

import android.test.AndroidTestCase;
import android.test.ServiceTestCase;
import android.content.Intent;
import android.content.Context;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import  com.android.stk.StkAppService;

import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.CatService;

public class StkAppServiceTestCase extends
             ServiceTestCase<StkAppService> {
    
    // constants
    static final String OPCODE = "op";
    static final String CMD_MSG = "cmd message";
    static final String RES_ID = "response id";
    static final String EVDL_ID = "downLoad event id";
    static final String MENU_SELECTION = "menu selection";
    static final String INPUT = "input";
    static final String HELP = "help";
    static final String CONFIRMATION = "confirm";

    // operations ids for different service functionality.
    static final int OP_CMD = 1;
    static final int OP_RESPONSE = 2;
    static final int OP_LAUNCH_APP = 3;
    static final int OP_END_SESSION = 4;
    static final int OP_BOOT_COMPLETED = 5;
    static final int OP_EVENT_DOWNLOAD = 6;
    private static final int OP_DELAYED_MSG = 7;

    private static final int OP_RESPONSE_IDLE_TEXT = 8;
    // Response ids
    static final int RES_ID_MENU_SELECTION = 11;
    static final int RES_ID_INPUT = 12;
    static final int RES_ID_CONFIRM = 13;
    static final int RES_ID_DONE = 14;

    static final int RES_ID_TIMEOUT = 20;
    static final int RES_ID_BACKWARD = 21;
    static final int RES_ID_END_SESSION = 22;
    static final int RES_ID_EXIT = 23;
    
    // DownLoad event ids
    static final int EVDL_ID_USER_ACTIVITY = 0x04;
    static final int EVDL_ID_IDLE_SCREEN_AVAILABLE = 0x05;
    static final int EVDL_ID_LANGUAGE_SELECT = 0x07;
    static final int EVDL_ID_BROWSER_TERMINATION = 0x08;
    
    public static final int GEMINI_SIM_1 = 0;
    public static final int GEMINI_SIM_2 = 1;
    
    private static final int TIME_LONG = 100;
    public static final String LOG_TAG = "StkAppServiceTestCase";    
    
    
    public StkAppServiceTestCase() {
        super(StkAppService.class);
    }
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testcase01_Startable() {
        try {
        	  Log.d(LOG_TAG, "testcase01_Startable: start service.");
            Intent startIntent = new Intent();
            startIntent.setClass(getContext(), StkAppService.class);
            startService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void testcase02_Bindable() {
        try {
        	  Log.d(LOG_TAG, "testcase02_Bindable: bindService.");
            Intent startIntent = new Intent();
            startIntent.setClass(getContext(), StkAppService.class);
            IBinder service = bindService(startIntent);            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void testcase03_StopService() {
        try {
        	  Log.d(LOG_TAG, "testcase03_StopService.");
            Intent startIntent = new Intent();
            startService(startIntent);
            StkAppService service = getService();
            service.stopService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    /*
    public void testcase04_SetupMenu() {
        try {
        	  Log.d(LOG_TAG, "testcase04_SetupMenu.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SET_UP_MENU.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
            
            //response
            Bundle args1 = new Bundle();
            args1.putInt(OPCODE, OP_RESPONSE);
            args1.putInt(RES_ID, RES_ID_MENU_SELECTION);
            args1.putInt(MENU_SELECTION, 0);
            args1.putBoolean(HELP, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args1));
            
            //confirm
            Bundle args2 = new Bundle();
            args2.putInt(OPCODE, OP_RESPONSE);
            args2.putInt(RES_ID, RES_ID_CONFIRM);
            args2.putBoolean(CONFIRMATION, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args2));
            
            //end session
            Bundle args3 = new Bundle();
            args3.putInt(OPCODE, OP_RESPONSE);
            args3.putInt(RES_ID, RES_ID_END_SESSION);
            args3.putInt(MENU_SELECTION, 0);
            args3.putBoolean(HELP, false);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args3));
                    	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase05_DispalyText() {
        try {
        	  Log.d(LOG_TAG, "testcase05_DispalyText.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.DISPLAY_TEXT.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        	  //confirm
        	  Bundle args2 = new Bundle();
            args2.putInt(OPCODE, OP_RESPONSE);
            args2.putInt(RES_ID, RES_ID_CONFIRM);
            args2.putBoolean(CONFIRMATION, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args2));
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase06_SelectItem() {
        try {
        	  Log.d(LOG_TAG, "testcase06_SelectItem.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SELECT_ITEM.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        	  //response
            Bundle args1 = new Bundle();
            args1.putInt(OPCODE, OP_RESPONSE);
            args1.putInt(RES_ID, RES_ID_MENU_SELECTION);
            args1.putInt(MENU_SELECTION, 0);
            args1.putBoolean(HELP, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args1));
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase07_GetInput() {
        try {
        	  Log.d(LOG_TAG, "testcase07_GetInput.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.GET_INPUT.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
            
            //input
            Bundle args1 = new Bundle();
            args1.putInt(OPCODE, OP_RESPONSE);
            args1.putInt(RES_ID, RES_ID_INPUT);
            args1.putString(INPUT, "YES");
            args1.putBoolean(HELP, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args1));
        	  
        	  //timeout
        	  Bundle args2 = new Bundle();
            args2.putInt(OPCODE, OP_RESPONSE);
            args2.putInt(RES_ID, RES_ID_TIMEOUT);
            args2.putString(INPUT, null);
            args2.putBoolean(HELP, false);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args2));
            
            //backward
            Bundle args3 = new Bundle();
            args3.putInt(OPCODE, OP_RESPONSE);
            args3.putInt(RES_ID, RES_ID_BACKWARD);
            args3.putString(INPUT, null);
            args3.putBoolean(HELP, false);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args3));
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase08_GetInkey() {
        try {
        	  Log.d(LOG_TAG, "testcase08_GetInkey.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.GET_INKEY.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
     public void testcase09_SetupIdleModeText() {
        try {
        	  Log.d(LOG_TAG, "testcase09_SetupIdleModeText.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SET_UP_IDLE_MODE_TEXT.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
     public void testcase10_SendDTMF() {
        try {
        	  Log.d(LOG_TAG, "testcase10_SendDTMF.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SEND_DTMF.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
     public void testcase11_SendSMS() {
        try {
        	  Log.d(LOG_TAG, "testcase11_SendSMS.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SEND_SMS.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase12_SendSS() {
        try {
        	  Log.d(LOG_TAG, "testcase12_SendSS.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SEND_SS.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase13_SendUSSD() {
        try {
        	  Log.d(LOG_TAG, "testcase13_SendUSSD.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SEND_USSD.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase14_LaunchBrowser() {
        try {
        	  Log.d(LOG_TAG, "testcase14_LaunchBrowser.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.LAUNCH_BROWSER.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        	  //confirm
        	  Bundle args2 = new Bundle();
            args2.putInt(OPCODE, OP_RESPONSE);
            args2.putInt(RES_ID, RES_ID_CONFIRM);
            args2.putBoolean(CONFIRMATION, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args2));
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase15_SetupCall() {
        try {
        	  Log.d(LOG_TAG, "testcase15_SetupCall.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SET_UP_CALL.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        	  //confirm
        	  Bundle args2 = new Bundle();
            args2.putInt(OPCODE, OP_RESPONSE);
            args2.putInt(RES_ID, RES_ID_CONFIRM);
            args2.putBoolean(CONFIRMATION, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args2));
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase16_PlayTone() {
        try {
        	  Log.d(LOG_TAG, "testcase16_PlayTone.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.PLAY_TONE.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        	  //done
        	  Bundle args1 = new Bundle();
            args1.putInt(OPCODE, OP_RESPONSE);
            args1.putInt(RES_ID, RES_ID_DONE);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args1));
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase17_OpenChannel() {
        try {
        	  Log.d(LOG_TAG, "testcase17_OpenChannel.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.OPEN_CHANNEL.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        	  //confirm
        	  Bundle args1 = new Bundle();
            args1.putInt(OPCODE, OP_RESPONSE);
            args1.putInt(RES_ID, RES_ID_CONFIRM);
            args1.putBoolean(CONFIRMATION, true);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args1));
            
            //backward
        	  Bundle args2 = new Bundle();
            args2.putInt(OPCODE, OP_RESPONSE);
            args2.putInt(RES_ID, RES_ID_BACKWARD);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args2));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase18_CloseChannnel() {
        try {
        	  Log.d(LOG_TAG, "testcase18_CloseChannnel.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.CLOSE_CHANNEL.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase19_ReceiveData() {
        try {
        	  Log.d(LOG_TAG, "testcase19_ReceiveData.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.RECEIVE_DATA.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase20_SendData() {
        try {
        	  Log.d(LOG_TAG, "testcase20_SendData.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.SEND_DATA.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase21_GetChannelStatus() {
        try {
        	  Log.d(LOG_TAG, "testcase21_GetChannelStatus.");
        	  CommandDetails cmdDet = new CommandDetails();
        	  cmdDet.compRequired = true;
        	  cmdDet.commandNumber = 1;
        	  cmdDet.typeOfCommand = AppInterface.CommandType.GET_CHANNEL_STATUS.value();
        	  cmdDet.commandQualifier = 1;
        	  
        	  CatCmdMessage msg = new CatCmdMessage(new CommandParams(cmdDet));
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_CMD);
            args.putParcelable(CMD_MSG, msg);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));
        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase22_LaunchApp() {
        try {
        	  Log.d(LOG_TAG, "testcase22_LaunchApp.");
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_LAUNCH_APP);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase23_EndSession() {
        try {
        	  Log.d(LOG_TAG, "testcase23_EndSession.");
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_END_SESSION);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase24_BootCompleted() {
        try {
        	  Log.d(LOG_TAG, "testcase24_BootCompleted.");
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_BOOT_COMPLETED);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase25_UserActivity() {
        try {
        	  Log.d(LOG_TAG, "testcase25_UserActivity.");
        	  Bundle args = new Bundle();
            args.putInt(OPCODE, OP_EVENT_DOWNLOAD);
            args.putInt(EVDL_ID, EVDL_ID_USER_ACTIVITY);
            startService(new Intent (getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase26_LanguageSelect() {
        try {
        	  Log.d(LOG_TAG, "testcase26_LanguageSelect.");
        	  Bundle args = new Bundle();
            args.putInt(EVDL_ID, EVDL_ID_LANGUAGE_SELECT);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase27_BrowserTermination() {
        try {
        	  Log.d(LOG_TAG, "testcase27_BrowserTermination.");
        	  Bundle args = new Bundle();
            args.putInt(EVDL_ID, EVDL_ID_BROWSER_TERMINATION);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    
    public void testcase28_IdleScreenAvailable() {
        try {
        	  Log.d(LOG_TAG, "testcase28_IdleScreenAvailable.");
        	  Bundle args = new Bundle();
            args.putInt(EVDL_ID, EVDL_ID_IDLE_SCREEN_AVAILABLE);
            startService(new Intent(getContext(), StkAppService.class).putExtras(args));        	  
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        try {
            Thread.sleep(TIME_LONG);
        } catch (InterruptedException e) {
            Log.d(LOG_TAG, "wait time InterruptedException: " + e.getMessage());
        }
    }
    */    
}
