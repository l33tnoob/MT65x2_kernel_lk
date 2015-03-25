package com.mediatek.common.dm;

/**
 * Agent to set DM flag and status
 * @hide
 */
interface DmAgent {
    /**
     * @deprecated
     * @hide
     */
    byte[] readDmTree();

    /**
     * @deprecated
     * @hide
     */
    boolean writeDmTree(in byte[] tree);

    /**
     * @return <code>true</code>if the DM lock flag is set, <code>false</code> otherwise
     * @internal
     */

    boolean isLockFlagSet();

    /**
     * Set DM lock flag
     * @param lockType The DM lock type to set
     * @return <code>true</code> if DM lock flag set success, <code>false</code> otherwise
     * @internal
     */
    boolean setLockFlag(in byte[] lockType);

    /**
     * Clear DM lock flag
     * @return <code>true</code> if DM lock flag cleared success, <code>false</code> otherwise
     * @internal
     */
    boolean clearLockFlag();

    /**
     * Read the saved IMSI
     * @return the value of the registered SIM card's IMSI
     * @internal
     */
    byte[] readImsi();

    /**
     * Save the registered SIM card's IMSI
     * @param imsi The registered SIM card's IMSI
     * @return <code>true</code> if IMSI written success, <code>false</code> otherwise
     * @internal
     */
    boolean writeImsi(in byte[] imsi);

    /**
     * @deprecated
     * @hide
     */
    byte[] readOperatorName();

    /**
     * Read Device Register switch value
     * @return current Device Register switch status,<code>'1'</code>Device Register enabled, <code>'0'</code> otherwise
     * @internal
     */
    byte[] getRegisterSwitch();

    /**
     * Set Device Register switch
     * @param registerSwitch Device Register switch, only can set <code>'0'</code> or <code>'1'</code>
     * @return <code>true</code> if Device Register switch set success, <code>false</code> otherwise
     * @internal
     */
    boolean setRegisterSwitch(in byte[] registerSwitch);

    /**
     * Set reboot flag
     * @return <code>true</code> if reboot flag set success, <code>false</code> otherwise.
     * @internal
     */
    boolean setRebootFlag();

    /**
     * Get DM lock Type
     * @return <code>-1</code> DM unlock, <code>0</code> DM partially lock, <code>1</code> DM fully lock.
     * @internal
     */
    int getLockType();

    /**
     * @deprecated
     * @hide
     */
    int getOperatorId();

    /**
     * @deprecated
     * @hide
     */
    byte[] getOperatorName();

    /**
     * @return <code>true</code> if MO Call is NOT allowed, <code>false</code> otherwise.
     * @hide
     * @internal
     */
    boolean isHangMoCallLocking();

    /**
     * @return <code>true</code> if MT Call is NOT allowed, <code>false</code> otherwise.
     * @hide
     * @internal
     */
    boolean isHangMtCallLocking();

    /**
     * Clear reboot flag
     * @return <code>true</code> if reboot flag cleared success, <code>false</code> otherwise.
     * @internal
     */
    boolean clearRebootFlag();

    /**
     * @deprecated
     * @hide
     */
    boolean isBootRecoveryFlag();

    /**
     * @deprecated
     * @hide
     */
    int getUpgradeStatus();

    /**
     * Restart Android
     * @return <code>true</code> if restart success, <code>false</code> otherwise.
     * @internal
     */
    int restartAndroid();

    /**
     * @return <code>true</code> if DM Wipe flag is set, <code>false</code> otherwise.
     * @hide
     * @internal
     */
    boolean isWipeSet();

    /**
     * Set DM Wipe flag
     * @return <code>true</code> if DM Wipe flag set success, <code>false</code> otherwise.
     * @hide
     * @internal
     */
    boolean setWipeFlag();

    /**
     * Clear DM Wipe flag
     * @return <code>true</code> if DM Wipe flag cleared success, <code>false</code> otherwise.
     * @hide
     * @internal
     */
    boolean clearWipeFlag();

    /**
     * Get OTA upgrade result.
     * @return <code>1</code> if OTA upgrade success, <code>0</code> otherwise.
     * @internal
     */
    int readOtaResult();

    /**
     * Clear OTA upgrade result
     * @return <code>1</code> if clear OTA result success, <code>0</code> otherwise.
     * @internal
     */
    int clearOtaResult();
    
    /**
     * Get MediatekDM Productive/Test switch value.
     * @return <code>'1'</code> for Productive Environment, <code>'0'</code> for Test Environment.
     */
    byte[] getSwitchValue();
    
    /**
     * Set MediatekDM Productive/Test switch value.
     * @param <code>'1'</code> for Productive Environment, <code>'0'</code> for Test Environment.
     * @return <code>true</code> if success, <code>false</code> otherwise
     * @internal
     */
    boolean setSwitchValue(in byte[] registerSwitch);
    
    /**
     * Get the pending flag of Productive/Test switch for DM. DM should clear this flag after the
     * necessary action has been performed.
     * @return <code>'1'</code> if there is pending action for DM, <code>'0'</code> otherwise.
     */
    byte[] getDmSwitchValue();
    
    /**
     * Set the pending flag of Productive/Test switch for DM.
     * @param <code>'1'</code> for Productive Environment, <code>'0'</code> for Test Environment.
     * @return <code>true</code> if success, <code>false</code> otherwise
     * @internal
     */
    boolean setDmSwitchValue(in byte[] registerSwitch);
    
    /**
     * Get the pending flag of Productive/Test switch for SmsReg. SmsReg should clear this flag after
     * the necessary action has been performed.
     * @return <code>'1'</code> if there is pending action for SmsReg, <code>'0'</code> otherwise.
     */
    byte[] getSmsRegSwitchValue();
    
    /**
     * Set the pending flag of Productive/Test switch for SmsReg.
     * @param <code>'1'</code> for Productive Environment, <code>'0'</code> for Test Environment.
     * @return <code>true</code> if success, <code>false</code> otherwise
     * @internal
     */
    boolean setSmsRegSwitchValue(in byte[] registerSwitch);


    /**
     * Set registered value for Smsreg.
     * @return write size of the imsi value
     * @internal
     */
    boolean setRegisterFlag(in byte[] flag, in int size);

    /**
     * Read registered value for Smsreg.
     * @return write size of the imsi value
     * @internal
     */
    byte[] readRegisterFlag();

    /**
     * Save register imsi value for simcard1.
     * @return write size of the imsi value
     * @internal
     */
    boolean writeImsi1(in byte[] imsi, in int size);

    /**
     * Save register imsi value for simcard2.
     * @return write size of the imsi value
     * @internal
     */
    boolean writeImsi2(in byte[] imsi, in int size);

    /**
     * Read register imsi value for simcard1.
     * @return write size of the imsi value
     * @internal
     */
    byte[] readImsi1();

    /**
     * Read register imsi value for simcard2.
     * @return write size of the imsi value
     * @internal
     */
    byte[] readImsi2();
}
