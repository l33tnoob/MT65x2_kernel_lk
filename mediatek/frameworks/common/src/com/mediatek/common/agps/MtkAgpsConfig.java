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

package com.mediatek.common.agps;

import android.os.Parcel;
import android.os.Parcelable;

public final class MtkAgpsConfig implements Parcelable {

    public int siMode;          // 0 MA, 1 MB, 2 Standalone
    public int setId;           // 0 IMSI, 1 IPv4
    public int qopHacc;         // 
    public int qopVacc;         // 
    public int qopAge;          // 
    public int qopDelay;        // 
    public int notifyTimeout;   // 
    public int verifyTimeout;   // 
    public int niEnable;        // 0 disable, 1 enable
    public int agpsProtocol;    // 0 up, 1 cp, 2 evdo, 3 temp, 4 test
    public String extAddress;       // 
    public int extAddressEnable;    // 0 disable, 1 enable
    public String mlcNum;           // 
    public int mlcNumEnable;        // 0 disable, 1 enable
    public int suplPosProtocol;  // 0 RRLP, 1 RRC, 2 RRLP+RRC
    public int cpMolrType;            // For CP, 0 Location Estimate, 1 Assistance Data
    public int log2file;            // 0 disable, 1 enable
    public int supl2file;           // 0 disable, 1 enable
    public int log2uart;            // 0 disable, 1 enable
    public int niIot;               // 0 disable, 1 enable
    public int logFileMaxSize;      //  
    public int simIdPref;           // 1 SIM1, 2 SIM2, 3 SIM3, 4 SIM4
    public int roaming;             // 0 disable, 1 enable
    public int caEnable;            // 0 disable, 1 enable
    public int emEnable;            // 0 disable, 1 enable, PS: not set to agpsd
    public int niTimer;             // 0 disable, 1 enable, PS: not set to agpsd
    public int eCidEnable;          // 0 disable, 1 enable
    public int pmtk9975;            // 0 disable, 1 enable
    public int gpevt;               // 0 disable, 1 enable
    public int dedicatedAPN;        // 0 disable, 1 SUPL APN only when data connection is on, 2 SUPL APN always

    //==================== new for SUPL2.0 ====================
    public int suplVersion;         // 1 SUPL1.0, 2 SUPL2.0
    public int enable3Party;        // 0 disable, 1 enable
    public int posMethod;           // AREA_EVENT_TYPE defined in MtkAgpsManager
    
    public int periodicNumOfFix;    //times (range: 1 to 8639999)
    public int periodicInterval;    //second (range: 1 to 8639999)
    public int periodicStartTime;   //second (range: 0 to 2678400)

    public int eventStartTime;      //second (range: 0 to 2678400)
    public int eventStopTime;       //second (range: 0 to 11318399)
    public int eventType;           //0 entering, 1 inside, 2 outside, 3 leaving, AREA_EVENT_TYPE defined in MtkAgpsManager
    public int eventInterval;       //second Range: (1..604800)
    public int eventMaxNumReport;   //times Range: (1..1024)

    public Parcelable[] targetAreaList;

    //==================== new for CDMA ====================
    public int evdoAgpsPrefer;

    
    public static final Parcelable.Creator<MtkAgpsConfig> CREATOR = 
        new Parcelable.Creator<MtkAgpsConfig>() {
        public MtkAgpsConfig createFromParcel(Parcel in) {
            MtkAgpsConfig profile = new MtkAgpsConfig();
            profile.readFromParcel(in);
            return profile;
        }
        public MtkAgpsConfig[] newArray(int size) {
            return new MtkAgpsConfig[size];
        }
    };

    public MtkAgpsConfig() {}
    public MtkAgpsConfig(int i, int s, int h, int v, int m, int d, int tn, int tv,int ni, 
        int mt,String exa,int exa_enable,String mlcnum,int mlcnum_enable, int st,
        int mlt, int ltf,int lstf, int ltu, int niiot, int lfmn, 
        int simid, int nt_r, int ca, int emEnable, int niTimer, int ecid, int pmtk9975, int gpevt,
        int suplVersion, int enable3Party, int posMethod, int periodicNumOfFix, int periodicInterval, int periodicStartTime,
        int eventStartTime, int eventStopTime, int eventType, int eventInterval, int eventMaxNumReport,
        Parcelable[] targetAreaList, int evdoAgpsPrefer, int dedicatedAPN) {
    
        this.siMode             = i;
        this.setId              = s;
        this.qopHacc            = h;
        this.qopVacc            = v;
        this.qopAge             = m;
        this.qopDelay           = d;
        this.notifyTimeout      = tn;
        this.verifyTimeout      = tv;
        this.niEnable           = ni;
        this.agpsProtocol       = mt;
        this.extAddress         = exa;
        this.extAddressEnable   = exa_enable;
        this.mlcNum             = mlcnum;
        this.mlcNumEnable       = mlcnum_enable;
        this.suplPosProtocol    = st;
        this.cpMolrType         = mlt;
        this.log2file           = ltf;
        this.supl2file          = lstf;
        this.log2uart           = ltu;
        this.niIot              = niiot;
        this.logFileMaxSize     = lfmn;
        this.simIdPref          = simid;
        this.roaming            = nt_r;
        this.caEnable           = ca;
        this.emEnable           = emEnable;
        this.niTimer            = niTimer;
        this.eCidEnable         = ecid;
        this.pmtk9975           = pmtk9975;
        this.gpevt              = gpevt;
        this.dedicatedAPN       = dedicatedAPN;

        this.suplVersion        = suplVersion;
        this.enable3Party       = enable3Party;
        this.posMethod          = posMethod;

        this.periodicNumOfFix   = periodicNumOfFix;
        this.periodicInterval   = periodicInterval;
        this.periodicStartTime  = periodicStartTime;

        this.eventStartTime     = eventStartTime;
        this.eventStopTime      = eventStopTime;
        this.eventType          = eventType;
        this.eventInterval      = eventInterval;
        this.eventMaxNumReport  = eventMaxNumReport;
        
        this.targetAreaList     = targetAreaList;

        this.evdoAgpsPrefer     = evdoAgpsPrefer;
    }

    //@Override
    public int describeContents() {
        return 0;
    }

    //@Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(siMode);
        out.writeInt(setId);
        out.writeInt(qopHacc);
        out.writeInt(qopVacc);
        out.writeInt(qopAge);
        out.writeInt(qopDelay);
        out.writeInt(notifyTimeout);
        out.writeInt(verifyTimeout);
        out.writeInt(niEnable);
        out.writeInt(agpsProtocol);
        out.writeString(extAddress);
        out.writeInt(extAddressEnable);
        out.writeString(mlcNum);
        out.writeInt(mlcNumEnable);
        out.writeInt(suplPosProtocol);
        out.writeInt(cpMolrType);
        out.writeInt(log2file);
        out.writeInt(supl2file);
        out.writeInt(log2uart);
        out.writeInt(niIot);
        out.writeInt(logFileMaxSize);
        out.writeInt(simIdPref);
        out.writeInt(roaming);
        out.writeInt(caEnable);
        out.writeInt(emEnable);
        out.writeInt(niTimer);
        out.writeInt(eCidEnable);
        out.writeInt(pmtk9975);
        out.writeInt(gpevt);
        out.writeInt(dedicatedAPN);

        out.writeInt(suplVersion);
        out.writeInt(enable3Party);
        out.writeInt(posMethod);
        
        out.writeInt(periodicNumOfFix);
        out.writeInt(periodicInterval);
        out.writeInt(periodicStartTime);
    
        out.writeInt(eventStartTime);
        out.writeInt(eventStopTime);
        out.writeInt(eventType);
        out.writeInt(eventInterval);
        out.writeInt(eventMaxNumReport);

        out.writeParcelableArray(targetAreaList, 0);

        
        out.writeInt(evdoAgpsPrefer);
    }

    //@Override
    public void readFromParcel(Parcel in) {
        siMode              = in.readInt();
        setId               = in.readInt();
        qopHacc             = in.readInt();
        qopVacc             = in.readInt();
        qopAge              = in.readInt();
        qopDelay            = in.readInt();
        notifyTimeout       = in.readInt();
        verifyTimeout       = in.readInt();
        niEnable            = in.readInt();
        agpsProtocol        = in.readInt();
        extAddress          = in.readString();
        extAddressEnable    = in.readInt();
        mlcNum              = in.readString();
        mlcNumEnable        = in.readInt(); 
        suplPosProtocol     = in.readInt();
        cpMolrType          = in.readInt();
        log2file            = in.readInt();
        supl2file           = in.readInt();
        log2uart            = in.readInt();
        niIot               = in.readInt();
        logFileMaxSize      = in.readInt();
        simIdPref           = in.readInt();
        roaming             = in.readInt();
        caEnable            = in.readInt();
        emEnable            = in.readInt();
        niTimer             = in.readInt();
        eCidEnable          = in.readInt();
        pmtk9975            = in.readInt();
        gpevt               = in.readInt();
        dedicatedAPN        = in.readInt();

        suplVersion         = in.readInt();
        enable3Party        = in.readInt();
        posMethod           = in.readInt();

        periodicNumOfFix    = in.readInt();
        periodicInterval    = in.readInt();
        periodicStartTime   = in.readInt();

        eventStartTime      = in.readInt();
        eventStopTime       = in.readInt();
        eventType           = in.readInt();

        eventInterval       = in.readInt();
        eventMaxNumReport   = in.readInt();
        
        targetAreaList      = in.readParcelableArray(MtkAgpsTargetArea.class.getClassLoader());

        evdoAgpsPrefer      = in.readInt();
    }

    public String toString() {
        String str = new String();
        
        str = "-----------------------MtkAgpsConfig--------------------------\n" +
            " siMode=" + siMode + " (0:MA 1:MB 2:Standalone)\n" +
            " setId=" + setId + " (0:IMSI 1:IPv4)\n" +
            " qopHacc=" + qopHacc + "\n" +
            " qopVacc=" +  qopVacc + "\n" +
            " qopAge=" + qopAge + "\n" +
            " qopDelay=" + qopDelay + "\n" +
            " notifyTimeout=" + notifyTimeout + "\n" +
            " verifyTimeout=" + verifyTimeout + "\n" +
            " niEnable=" + niEnable + "\n" +
            " agpsProtocol=" + agpsProtocol + " (0:UP 1:CP)\n"+
            " extAddressEnable=" + extAddressEnable + "\n" +
            " extAddress=" + extAddress + "\n" +
            " mlcNumEnable=" + mlcNumEnable + "\n"+
            " mlcNum=" + mlcNum + "\n" +
            " suplPosProtocol=" + suplPosProtocol + " (0:RRLP 1:RRC 2:RRLP+RRC)\n" + 
            " cpMolrType=" + cpMolrType + " (0:LocationEstimate 1:AssistanceData)\n"+
            " niIot=" + niIot + "\n"+
            " logFileMaxSize=" + logFileMaxSize + "\n"+
            " simIdPref=" + simIdPref +"\n"+
            " roaming=" + roaming +"\n"+
            " caEnable=" + caEnable + "\n" +
            " log2file=" + log2file + "\n" +
            " supl2file=" + supl2file + "\n" +
            " log2uart=" + log2uart + "\n" +
            " emEnable=" + emEnable + "\n" +
            " niTimer=" + niTimer + "\n" +
            " eCidEnable=" + eCidEnable + "\n" + 
            " pmtk9975=" + pmtk9975 + "\n" +
            " gpevt=" + gpevt + "\n" +
            " dedicatedAPN=" + dedicatedAPN + "\n" +

            " suplVersion=" + suplVersion + "\n" +
            " enable3Party=" + enable3Party + "\n" +
            " posMethod=" + posMethod + "\n" +

            " periodicNumOfFix=" + periodicNumOfFix + "\n" +
            " periodicInterval=" + periodicInterval + "\n" +
            " periodicStartTime=" + periodicStartTime + "\n" +

            " eventStartTime=" + eventStartTime + "\n" + 
            " eventStopTime=" + eventStopTime + "\n" + 
            " eventType=" + eventType + "\n" +
            " eventInterval=" + eventInterval + "\n" +
            " eventMaxNumReport=" + eventMaxNumReport + "\n";
        
        if(targetAreaList != null) {
            str += " targetAreaList length=" + targetAreaList.length + "\n";
            
            for(int i = 0; i < targetAreaList.length; i++) {
                MtkAgpsTargetArea area = (MtkAgpsTargetArea)targetAreaList[i];
                str += " area type=" + area.type + "\n";
                str += " area radius=" + area.radius + "\n";
                str += " area latSign=" + area.latSign + "\n";
                str += " area lat=" + area.lat + "\n";
                str += " area lng=" + area.lng + "\n";
            }
        } else {
            str += " targetAreaList is null\n";
        }

        str += " evdoAgpsPrefer=" + evdoAgpsPrefer + "\n";
        
        str += "------------------------------------------------------------";
        return str;
    }
}

