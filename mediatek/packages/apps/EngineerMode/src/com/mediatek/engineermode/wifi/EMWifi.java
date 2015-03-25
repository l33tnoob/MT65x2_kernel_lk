/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.wifi;

public class EMWifi {

    public static boolean sIsInitialed = false;
    public static boolean sEMWifiWorked = false;
    public static boolean sIs5GNeeded = true;

    /**
     * Initial wifi
     * 
     * @return Chip ID, like 0x6620 or 0x5921
     */
    public static native int initial();

    /**
     * Uninitial wifi after test complete
     * 
     * @return ERROR_RFTEST_XXXX
     */
    public static native int unInitial();

    /**
     * Get Xtal Trim value from RFCR
     * 
     * @param value
     *            Frequency trim value pointer
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getXtalTrimToCr(long[] value);

    /**
     * Set wifi chip to test mode
     * 
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setTestMode();

    /**
     * Set the driver to normal mode
     * 
     * @return 0, if successful, -1, otherwise
     */
    public static native int setNormalMode();

    /**
     * stop BB continuous Tx mode
     * 
     * @return 0, if successful -1, Failed to set stop pattern
     */
    public static native int setStandBy();

    /**
     * Set EEPROM size
     * 
     * @param i4EepromSz
     *            Size of the EEPROM
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setEEPRomSize(long i4EepromSz);

    /**
     * Burn EEPROM by reading the data from file
     * 
     * @param atcFileName
     *            Null terminated string indicate the file to be read
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setEEPRomFromFile(String atcFileName);

    /**
     * Retrieve TX Power from EEPROM
     * 
     * @param i4ChnFreg
     *            Frequency
     * @param i4Rate
     *            Rate in unit of 500K
     * @param powerStatus
     *            Power status array
     * @param arraylen
     *            Power status array length
     * @return ERROR_RFTEST_XXXX
     */
    public static native int readTxPowerFromEEPromEx(long i4ChnFreg,
            long i4Rate, long[] powerStatus, int arraylen);

    /**
     * Update EEPROM checksum
     * 
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setEEPromCKSUpdated();

    /**
     * Get packet TX status
     * 
     * @param pktStatus
     *            Result array
     * @param arraylen
     *            Array length
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getPacketTxStatusEx(long[] pktStatus, int arraylen);

    /**
     * Query Rx status
     * 
     * @param i4Init
     *            Result array
     * @param arraylen
     *            Array length
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getPacketRxStatus(long[] i4Init, int arraylen);

    /**
     * Enable continuous tx. PAU disabled. Manually controlled the TX_RX, TX_PE,
     * PA_PE, TR_SW on, RX_PE off.
     * 
     * @param i4Rate
     *            Rate
     * @param i4TxPwrGain
     *            Power gain
     * @param i4TxAnt
     *            Ant index
     * @return 0, if successful
     */
    public static native int setOutputPower(long i4Rate, long i4TxPwrGain,
            int i4TxAnt);

    /**
     * Set local frequency
     * 
     * @param i4TxPwrGain
     *            Power gain
     * @param i4TxAnt
     *            Ant index
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setLocalFrequecy(long i4TxPwrGain, long i4TxAnt);

    /**
     * Set carrier suppression
     * 
     * @param i4Modulation
     *            Mode type
     * @param i4TxPwrGain
     *            Power gain
     * @param i4TxAnt
     *            Ant index
     * @return 0, if successful
     */
    public static native int setCarrierSuppression(long i4Modulation,
            long i4TxPwrGain, long i4TxAnt);

    /**
     * Set country
     * 
     * @param acChregDomain
     *            Country
     * @return 0, if successful
     */
    public static native int setOperatingCountry(String acChregDomain);

    /**
     * Set RF channel
     * 
     * @param i4ChFreqkHz
     *            Channel frequence
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setChannel(long i4ChFreqkHz);

    /**
     * Get support rate
     * 
     * @param pu2RateBuf
     *            Result buffer
     * @param i4MaxNum
     *            Buffer size
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getSupportedRates(int[] pu2RateBuf, long i4MaxNum);

    /**
     * This function sets the particular pin as an output one and the output
     * level
     * 
     * @param i4PinIndex
     *            Pin index
     * @param i4OutputLevel
     *            Output for band
     * @return 0 - success negative fails
     */
    public static native int setOutputPin(long i4PinIndex, long i4OutputLevel);

    /**
     * Read 16-bit data to EEPRom
     * 
     * @param u4Offset
     *            Index of the EEPRom offset
     * @param pu4Value
     *            Value read from the EEPRom offset (size: 2 Byte)
     * @return ERROR_RFTEST_XXXX
     */
    public static native int readEEPRom16(long u4Offset, long[] pu4Value);

    /**
     * Read data from EEPROM
     * 
     * @param u4Offset
     *            Index of the EEPROM offset
     * @param pu4Value
     *            Value read from the EEPROM offset
     * @return ERROR_RFTEST_XXXX
     */
    public static native int readSpecEEPRom16(long u4Offset, long[] pu4Value);

    /**
     * Read 16-bit data from EEPROM
     * 
     * @param u4Offset
     *            Index of the EEPROM offset
     * @param u4Value
     *            Value read from EEPROM
     * @return ERROR_RFTEST_XXXX
     */
    public static native int writeEEPRom16(long u4Offset, long u4Value);

    /**
     * Read string data from EEPROM
     * 
     * @param u4Addr
     *            Index of the EEPROM offset
     * @param u4Length
     *            Data length
     * @param paucStr
     *            String get from the EEPROM offset
     * @return ERROR_RFTEST_XXXX
     */
    public static native int eepromReadByteStr(long u4Addr, long u4Length,
            byte[] paucStr);

    /**
     * Write string data to EEPROM
     * 
     * @param u4Addr
     *            index of the EEPROM offset
     * @param u4Length
     *            Data length
     * @param paucStr
     *            Value to set
     * @return ERROR_RFTEST_XXXX
     */
    public static native int eepromWriteByteStr(long u4Addr, long u4Length,
            String paucStr);

    /**
     * Set AT parameter
     * 
     * @param u4FuncIndex
     *            Function index
     * @param u4FuncData
     *            Function data to set
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setATParam(long u4FuncIndex, long u4FuncData);

    /**
     * Get AT parameter
     * 
     * @param u4FuncIndex
     *            Function index
     * @param pu4FuncData
     *            Function data
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getATParam(long u4FuncIndex, long[] pu4FuncData);

    /**
     * Set Xtal Trim value to RFCR
     * 
     * @param u4Value
     *            Frequency trim value
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setXtalTrimToCr(long u4Value);

    /**
     * Query Adapter's thermo information
     * 
     * @param pi4Enable
     *            Thermo value
     * @param len
     *            The thermo data length
     * @return ERROR_RFTEST_XXXX
     */
    public static native int queryThermoInfo(long[] pi4Enable, int len);

    /**
     * Set Adapter's thermo function enable/disable
     * 
     * @param i4Enable
     *            0: thermo disable. 1: thermo enable
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setThermoEn(long i4Enable);

    /**
     * check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
     * 
     * @param value
     *            size of the EEPRom
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getEEPRomSize(long[] value);

    /**
     * Get EEPROM data
     * 
     * @param value
     *            Data buffer
     * @return ERROR_RFTEST_XXXX
     */
    public static native int getSpecEEPRomSize(long[] value);

    /**
     * Set PNP power
     * 
     * @param i4PowerMode
     *            Power mode
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setPnpPower(long i4PowerMode);

    /**
     * Set Adapter to enable/disable Anritsu test support
     * 
     * @param i4Enable
     *            0: disable. 1: enable
     * @return ERROR_RFTEST_XXXX
     */
    public static native int setAnritsu8860bTestSupportEn(long i4Enable);

    /**
     * Write 32-bit data to MCR
     * 
     * @param offset
     *            Address offset of the MCR
     * @param value
     *            Value set to the MCR
     * @return ERROR_RFTEST_XXXX
     */
    public static native int writeMCR32(long offset, long value);

    /**
     * Read 32-bit data from MCR
     * 
     * @param offset
     *            Address offset of the MCR
     * @param value
     *            Value read from the MCR
     * @return RROR_RFTEST_XXXX
     */
    public static native int readMCR32(long offset, long[] value);

    /**
     * Get DPD length, added by mtk80758 2010-11-5
     * 
     * @param value
     *            Result buffer
     * @return RROR_RFTEST_XXXX
     */
    public static native int getDPDLength(long[] value);

    /**
     * Read 32-bit data from DPD parameters
     * 
     * @param offset
     *            Address offset of the DPD
     * @param value
     *            Value read from the DPD parameters (size: 4 Byte)
     * @return RROR_RFTEST_XXXX
     */
    public static native int readDPD32(long offset, long[] value);

    /**
     * Write 32-bit data to DPD
     * 
     * @param offset
     *            Address offset of the DPD
     * @param value
     *            Value set to the DPD
     * @return ERROR_RFTEST_XXXX
     */
    public static native int writeDPD32(long offset, long value);

    /**
     * Set DPD parameters from file
     * 
     * @param atcFileName
     *            File path
     * @return RROR_RFTEST_XXXX
     */
    public static native int setDPDFromFile(String atcFileName);

    /**
     * Get support channel list. Added by mtk54046 @ 2012-01-05 for get support
     * channel list
     * 
     * @param value
     *            Channel buffer
     * @return RROR_RFTEST_XXXX
     */
    public static native int getSupportChannelList(long[] value);
    
    /**
     * CTIA test setting. Added by mtk54046 @ 2012-11-15 for CTIA test
     * 
     * @param id
     *            Address
     * @param value
     *            Value to set
     * @return RROR_RFTEST_XXXX
     */
    public static native int doCTIATestSet(long id, long value);
    
    /**
     * CTIA test getting. Added by mtk54046 @ 2012-11-15 for CTIA test
     * 
     * @param id
     *            Address
     * @param value
     *            Buffer to get
     * @return RROR_RFTEST_XXXX
     */
    public static native int doCTIATestGet(long id, long[] value);

    static {
        System.loadLibrary("em_wifi_jni");
    }

}
