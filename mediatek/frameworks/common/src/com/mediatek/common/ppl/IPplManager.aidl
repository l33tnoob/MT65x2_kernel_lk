package com.mediatek.common.ppl;

interface IPplManager {
	/*
	 * Reset the password and send the new password to trusted phone number via SMS.
	 */
	void resetPassword();

	/*
	 * Whether Keyguard should lock the phone according to lock flag and sim status.
	 *
	 * @return 0 - no need to lock
	 *         1 - lock flag is set, need to lock
	 *         2 - sim info does not match, need to lock 
	 */
	int needLock();

	/*
	 * Lock the phone.
	 */
	void lock();

	/*
	 * Try to unlock the phone.
	 *
	 * @return true  - password is accepted.
	 *         false - password is incorrect.
	 */
	boolean unlock(String password);
}
