/*
 * Copyright (C) 2006 The Android Open Source Project
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
 */

package com.mediatek.rcse.plugin.phone;


import android.util.Log;

import com.android.services.telephony.common.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Connection;

/**
 * "Call card" UI element: the in-call screen contains a tiled layout of call
 * cards, each representing the state of a current "call" (ie. an active call, a
 * call on hold, or an incoming call.)
 */
public class RCSeUtils {

    private static final String LOG_TAG = "RCSeUtils";
    private static final boolean DBG = true;
    private static Call mFgCall = null;

    public static Call getmFgCall() {
    	if (DBG) {
            log("getmfgCall" + mFgCall);
        }
		return mFgCall;
	}

	public static void setmFgCall(Call mfgCall) {
		if(mfgCall != null)
		{
			mFgCall = mfgCall;
			if(mFgCall.getState() == Call.State.DISCONNECTED)
				RCSeInCallUIExtension.getInstance().onDisconnect(mFgCall.getNumber());
		}
		if (DBG) {
            log("setmfgCall" + mfgCall);
        }
	}

    public static boolean canShare(CallManager cm) {
        if (!canShareFromCallState(cm)) {
			if (DBG) {
                log("sharing canShare is false");
            }
            return false;
        }
        String number = getRCSePhoneNumber(cm);
        if (null != number) {
            return RCSeInCallUIExtension.isCapabilityToShare(number);
            //return true;
        } else {
            if (DBG) {
                log("get rcse phone number failed, return false");
            }
            return true;
        }
    }

    public static boolean canShareImage(CallManager cm) {
        if (!canShareFromCallState(cm)) {
			if (DBG) {
                log("sharing canShare is false");
            }
            return false;
        }
        String number = getRCSePhoneNumber(cm);
        if (null != number) {
            return RCSeInCallUIExtension.isCapabilityToShareImage(number);
        } else {
            if (DBG) {
                log("sharing get rcse phone number failed in canShareImage, return false");
            }
            return false;
        }
    }
    
    public static boolean canShareVideo(CallManager cm) {
        if (!canShareFromCallState(cm)) {
        	if (DBG) {
                log("canShareVideo return false");
            }
            return false;
        }
        String number = getRCSePhoneNumber(cm);
        if (null != number) {
            return RCSeInCallUIExtension.isCapabilityToShareVideo(number);
        } else {
            if (DBG) {
                log("get rcse phone number failed in canShareVideo, return false");
            }
            return false;
        }
    }
    
    public static boolean canShareFromCallState(CallManager cm) {
    	if (DBG) {
            log("can share from call state entry" + mFgCall);
        }
    	if(mFgCall!=null)
    	{
        if (mFgCall.getState() == Call.State.DIALING || mFgCall.getState() == Call.State.INCOMING
        		||mFgCall.getState() == Call.State.REDIALING||mFgCall.getState() == Call.State.INVALID
        		||mFgCall.getState() == Call.State.DISCONNECTED ||mFgCall.getState() == Call.State.DISCONNECTING) {
            if (DBG) {
                log("can not share for ring call is active");
            }
            return false;
        }
        if (cm.hasActiveBgCall()) {
            if (DBG) {
                log("can not share for background call is active");
            }
            return true;
        }
        /*
         * if (cm.getForegroundCalls().size() > 1) { if (DBG)
         * log("can not share for foreground call count > 1"); return false; }
         */
        if (cm.getFgCallConnections().size() > 1) {
            if (DBG) {
                log("can not share for foreground call connection > 1");
            }
            return false;
        }
            
        
        if (mFgCall.getState() != Call.State.ACTIVE) {
            if (DBG) {
                log("can not share for foreground call state is not ACTIVE");
            }
            return false;
        }
        
        if (mFgCall.isVideoCall()) {
            if (DBG) {
                log("can not share for latest connection is video type");
            }
            return false;
        }
    	}
        return true;
    }

    public static String getRCSePhoneNumber(CallManager cm) {
       if(mFgCall!=null)
       {
    	String number = mFgCall.getNumber();
        if (DBG) {
            log("getRCSePhoneNumber(), call is " + mFgCall + "number" + number);
        }        
        
        return number;
       }
       return null;
    }

    public static boolean shouldStop(CallManager cm) {
        
        if(mFgCall!=null)
        {
        if (mFgCall.getState() != Call.State.ACTIVE) {
            if (DBG) {
                log("should stop sharing for foreground call state is not ACTIVE");
            }
            return true;
        }
        if (mFgCall.isVideoCall()) {
            if (DBG) {
                log("should stop for latest connection is video type");
            }
            return true;
        }
		}
        return false;
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

}
