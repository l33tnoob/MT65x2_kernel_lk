package com.mediatek.voicecommand.adapter;

public interface IVoiceAdapter {

    /*
     * Judge whether the native is prepared for voice training, recognition and
     * UI
     *
     * @return true if prepared
     */
    boolean isNativePrepared();

    /*
     * Start voice recognition when power on
     *
     * @param patternpath Path used for native
     *
     * @param ubmpath Path used for native
     *
     * @param processname Record the process who start the voice recognition
     *
     * @param pid Record the process pid because maybe the process was died
     * during voice recognition starting
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int startVoicePWRecognition(String patternpath, String ubmpath,
            String processname, int pid);

    /*
     * Stop voice recognition when power off or time out
     *
     * @param processname Used to check if the process has start this business
     *
     * @param pid Used to check if the pid is the same as before
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int stopVoicePWRecognition(String processname, int pid);

    /*
     * Start voice training
     *
     * @param pwdpath Used for native thread
     *
     * @param patternpath Used for native thread
     *
     * @param featurepath Used for native thread
     *
     * @param umbpath Used for native thread
     *
     * @param commandid Used for native thread and notify App when recognition
     *
     * @param processname Record the process who start the voice recognition
     *
     * @param pid Record the process pid because maybe the process was died
     * during voice recognition starting
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int startVoiceTraining(String pwdpath, String patternpath,
            String featurepath, String umbpath, int commandid,
            int[] commandMask, String processname, int pid);

    /*
     * Start voice training
     *
     * @param pwdpath Used for native thread
     *
     * @param patternpath Used for native thread
     *
     * @param featurepath Used for native thread
     *
     * @param commandid Used for native thread and notify App when recognition
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int resetVoiceTraining(String pwdpath, String patternpath,
            String featurepath, int commandid);

    /*
     * Stop voice training when app unregister from service or send the stopping
     * command
     *
     * @param processname Used to check if the process has start this business
     *
     * @param pid Used to check if the pid is the same as before
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int stopVoiceTraining(String processname, int pid);

    /*
     * Start voice ui business
     *
     * @param modelpath Used for native thread
     *
     * @param patternpath Used for native thread
     *
     * @param processname Used for native thread
     *
     * @param pid Used for native thread
     *
     * @param languageid Notify native thread the current supprot language
     *
     * @return
     */
    int startVoiceUI(String modelpath, String patternpath, String processname,
            int pid, int languageid);

    /*
     * Stop voice ui business
     *
     * @param processname Used to check if the process has start this business
     *
     * @param pid Used to check if the pid is the same as before
     *
     * @return
     */
    int stopVoiceUI(String processname, int pid);

    /*
     * Start voice contacts business
     *
     * @param processname Used for native thread
     *
     * @param pid Used for native thread
     *
     * @param screenOrientation Screen Orientation from contacts App
     *
     * @return
     */
    int startVoiceContacts(String processname, int pid, int screenOrientation);

    /*
     * Stop voice contacts business
     *
     * @param processname Used to check if the process has start this business
     *
     * @param pid Used to check if the pid is the same as before
     *
     * @return
     */
    int stopVoiceContacts(String processname, int pid);

    /*
     * Send all voice contacts name
     *
     * @param modelpath Used for native thread
     *
     * @param contactsdbpath Used for native thread
     *
     * @param allContactsName Used to send all contacts name
     *
     */
    int sendContactsName(String modelpath, String contactsdbpath, String[] allContactsName);

    /*
     * Send learning voice contacts name
     *
     * @param selectName Used to send learning contacts name
     *
     */
    int sendContactsSelected(String selectName);

    /*
     * Send required contacts result count from contacts app
     *
     * @param searchCnt Used to send search voice contacts count
     *
     */
    int sendContactsSearchCnt(int searchCnt);

    /*
     * Send screen orientation from contacts app
     *
     * @param screenOrientation Used to send screen orientation
     *
     */
    int sendContactsOrientation(int screenOrientation);

    /*
     * Get the intensity from native
     *
     * @param bundle
     *
     * @return
     */
    int getNativeIntensity();

    /*
     * Stop native thread business if needed when process died
     *
     * @param processname
     *
     * @param pid
     */
    void stopCurMode(String processname, int pid);

    /*
     * set current Headset status
     *
     * @param isPlugin
     *
     * @return
     */
    void setCurHeadsetMode(boolean isPlugin);

    /*
     * Release all the native memory when service is called onDestroy
     */
    void release();

}
