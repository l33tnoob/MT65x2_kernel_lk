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
package com.mediatek.common.telephony.gsm;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * {@hide}
 */
public class FemtoCellInfo implements Parcelable {
    /* This CSG is Not in EFACSGL nor in EFOCSGL */
    public static final int CSG_ICON_TYPE_NOT_ALLOWED = 0;	
	
	/* This CSG is in Allowed CSG List (EFACSGL) */
    public static final int CSG_ICON_TYPE_ALLOWED = 1;
    
    /* This CSG is in Operator CSG List (EFOCSGL) */
    public static final int CSG_ICON_TYPE_OPERATOR = 2;

    /* This CSG is in Operator CSGs. However, it is unauthorized, meaning that UE's registration has been rejected by cause #25.*/
    public static final int CSG_ICON_TYPE_OPERATOR_UNAUTHORIZED = 3;    

    private int csgId;
    private int csgIconType; /* FemtoCellInfo.CSG_ICON_TYPE_xxx */	
    private String homeNodeBName;
    private String operatorNumeric; /* ex: "46000" */
    private String operatorAlphaLong; /* ex: "China Mobile" */	
    private int rat = 0; /* ServiceState.RIL_RADIO_TECHNOLOGY_xxx */

    public int getCsgId() {
        return csgId;
    }

    public int getCsgIconType() {
        return csgIconType;
    }	
	
    public String getHomeNodeBName() {
        return homeNodeBName;
    }

    public int getCsgRat() {
        return rat;
    }
	
    public String getOperatorNumeric() {
        return operatorNumeric;
    }

    public String getOperatorAlphaLong() {
        return operatorAlphaLong;
    }	
	
    public FemtoCellInfo(int csgId, int csgIconType, String homeNodeBName, String operatorNumeric,String operatorAlphaLong,int rat) {
        this.csgId = csgId;
        this.csgIconType = csgIconType;
        this.homeNodeBName = homeNodeBName;
        this.operatorNumeric = operatorNumeric;		
        this.operatorAlphaLong = operatorAlphaLong;				
        this.rat = rat;
    }

    public String toString() {
        return "FemtoCellInfo " + csgId
                + "/" + csgIconType
                + "/" + homeNodeBName
                + "/" + operatorNumeric
                + "/" + operatorAlphaLong                
                + "/" + rat;
    }

    /**
     * Parcelable interface implemented below.
     * This is a simple effort to make FemtoCellInfo parcelable rather than
     * trying to make the conventional containing object (AsyncResult),
     * implement parcelable.  
     */

    public int describeContents() {
        return 0;
    }

    /**
     * Implement the Parcelable interface.
     * Method to serialize a FemtoCellInfo object.
     */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(csgId);
        dest.writeInt(csgIconType);
        dest.writeString(homeNodeBName);
        dest.writeString(operatorNumeric);		
        dest.writeString(operatorAlphaLong);				
        dest.writeInt(rat);        				
    }

    /**
     * Implement the Parcelable interface
     * Method to deserialize a FemtoCellInfo object, or an array thereof.
     */
    public static final Creator<FemtoCellInfo> CREATOR =
        new Creator<FemtoCellInfo>() {
            public FemtoCellInfo createFromParcel(Parcel in) {
                FemtoCellInfo femtoCellInfo = new FemtoCellInfo(
                        in.readInt(), /*csgId*/
                        in.readInt(), /*csgIconType*/
                        in.readString(), /*homeNodeBName*/    
                        in.readString(), /*operatorNumeric*/                   
                        in.readString(), /*operatorAlphaLong*/                            
                        in.readInt()); /*rat*/                                                
                return femtoCellInfo;
            }

            public FemtoCellInfo[] newArray(int size) {
                return new FemtoCellInfo[size];
            }
        };
}

