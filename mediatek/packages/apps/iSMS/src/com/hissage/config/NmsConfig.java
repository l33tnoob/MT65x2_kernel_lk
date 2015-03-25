package com.hissage.config;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.hissage.util.log.NmsLog;
import com.hissage.util.preference.NmsPreferences;

public final class NmsConfig {

    private static final String TAG = "NmsConfig";
    public final static boolean isAndroidKitKatOnward = true;//4.4 is true ,others false

    // public static final int NMS_LANGUAGE_CHINESE = 0;
    // public static final int NMS_LANGUAGE_ENGLISH = 1;
    public static final int NMS_LANG_DEFAULT = 0;
    public static final int NMS_LANG_ENGLISH = 1; // en
    public static final int NMS_LANG_TR_CHINESE = 2; // zh_TW
    public static final int NMS_LANG_SM_CHINESE = 3; // zh_CN
    public static final int NMS_LANG_THAI = 4; // th
    public static final int NMS_LANG_SPANISH = 5; // es
    public static final int NMS_LANG_FRENCH = 6; // fr
    public static final int NMS_LANG_GERMAN = 7; // de
    public static final int NMS_LANG_RUSSIAN = 8; // ru
    public static final int NMS_LANG_ITALIAN = 9; // it
    public static final int NMS_LANG_ARABIC = 10; // ar
    public static final int NMS_LANG_PORTUGUESE = 11; // pt
    public static final int NMS_LANG_TURKISH = 12; // tr
    public static final int NMS_LANG_VIETNAMESE = 13; // vi
    public static final int NMS_LANG_INDONESIAN = 14; // id
    public static final int NMS_LANG_MALAY = 15; // ms
    public static final int NMS_LANG_HINDI = 16; // hi
    public static final int NMS_LANG_DANISH = 17; // da
    public static final int NMS_LANG_CZECH = 18; // cs
    public static final int NMS_LANG_POLISH = 19; // pl
    public static final int NMS_LANG_HUNGARIAN = 20; // hu
    public static final int NMS_LANG_FINNISH = 21; // fi
    public static final int NMS_LANG_NORWEGIAN = 22; // no
    public static final int NMS_LANG_SLOVAK = 23; // sk
    public static final int NMS_LANG_DUTCH = 24; // nl
    public static final int NMS_LANG_SWEDISH = 25; // sv
    public static final int NMS_LANG_CROATIAN = 26; // hr
    public static final int NMS_LANG_ROMANIAN = 27; // ro
    public static final int NMS_LANG_SLOVENIAN = 28; // sl
    public static final int NMS_LANG_GREEK = 29; // el
    public static final int NMS_LANG_HEBREW = 30; // he
    public static final int NMS_LANG_BULGARIAN = 31; // bg
    public static final int NMS_LANG_MARATHI = 32; // mr
    public static final int NMS_LANG_PERSIAN = 33; // fa
    public static final int NMS_LANG_URDU = 34; // ur
    public static final int NMS_LANG_TAMIL = 35; // ta
    public static final int NMS_LANG_BENGALI = 36; // bn
    public static final int NMS_LANG_PUNJABI = 37; // pa
    public static final int NMS_LANG_TELUGU = 38; // te
    public static final int NMS_LANG_UKRAINIAN = 39; // uk
    public static final int NMS_LANG_GUJARATI = 40; // gu
    public static final int NMS_LANG_KANNADA = 41; // kn
    public static final int NMS_LANG_ZULU = 42; // zu
    public static final int NMS_LANG_XHOSA = 43; // xh
    public static final int NMS_LANG_SWAHILI = 44; // sw
    public static final int NMS_LANG_AFRIKAANS = 45; // af
    public static final int NMS_LANG_LITHUANIAN = 46; // lt
    public static final int NMS_LANG_LATVIAN = 47; // lv
    public static final int NMS_LANG_ESTONIAN = 48; // et
    public static final int NMS_LANG_ARMENIAN = 49; // hy
    public static final int NMS_LANG_GEORGIAN = 50; // ka
    public static final int NMS_LANG_MOLDOVAN = 51; // ro_MD
    public static final int NMS_LANG_MALAYALAM = 52; // ml
    public static final int NMS_LANG_ORIYA = 53; // or
    public static final int NMS_LANG_ALBANIAN = 54; // sq
    public static final int NMS_LANG_ASSAMESE = 55; // as
    public static final int NMS_LANG_AZERBAIJANI = 56; // az
    public static final int NMS_LANG_HK_CHINESE = 57; // zh_HK
    public static final int NMS_LANG_CATALAN = 58; // ca
    public static final int NMS_LANG_CA_FRENCH = 59; // fr
    public static final int NMS_LANG_ICELANDIC = 60; // is
    public static final int NMS_LANG_SA_SPANISH = 61; // es
    public static final int NMS_LANG_MACEDONIAN = 62; // mk
    public static final int NMS_LANG_SA_PORTUGUESE = 63; // pt
    public static final int NMS_LANG_SERBIAN = 64; // sr
    public static final int NMS_LANG_SESOTHO = 65; // st
    public static final int NMS_LANG_TAGALOG = 66; // tl
    public static final int NMS_LANG_UK_ENGLISH = 67; // en_UK
    public static final int NMS_LANG_HAUSA = 68; // ha
    public static final int NMS_LANG_YORUBA = 69; // yo
    public static final int NMS_LANG_KAZAK = 70; // kk
    public static final int NMS_LANG_BASQUE = 71; // eu
    public static final int NMS_LANG_FILIPINO = 72; // tl_PH
    public static final int NMS_LANG_GALICIAN = 73; // gl
    public static final int NMS_LANG_IGBO = 74; // ig
    public static final int NMS_LANG_IRISH = 75; // ga
    public static final int NMS_LANG_MYANMAR = 76; // my
    public static final int NMS_LANG_LAO = 77; // lo
    public static final int NMS_LANG_KHMER = 78; // km
    public static final int NMS_LANG_KOREAN = 79; // ko

    public static volatile boolean mIsDBInitDone = false;
    public static volatile boolean mAirplaneMode = false;

    public static String getSim1IMSI(Context context) {
        // return "460002030666999";
        TelephonyManager telManager = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);

        if (null != telManager) {
            return telManager.getSubscriberId();
        } else {
            NmsLog.trace(TAG, "IMSI is null from telManger or no sim card.");
            return null;
        }
    }

    public static void clearAllCache() {
        NmsLog.trace(TAG, "all config will be delete......");
        NmsPreferences.clearAllConfig();
    }

    public static long getMaxNormalizedDate() // added by luozheng in 12.2.29
                                              // for message refresh;
    {
        return NmsPreferences.getLongValue("normalized_date");
    }

    public static void setMaxNormalizedDate(long id) // added by luozheng in
                                                     // 12.2.29 for message
                                                     // refresh;
    {
        NmsPreferences.setLongValue("normalized_date", id);
    }

    public static void setUpdataVersionFlag(int flag) {
        NmsPreferences.setIntegerValue("UpdateVersionFlag", flag);
    }

    public static int getUpdataVersionFlag() {
        return NmsPreferences.getIntegerValue("UpdateVersionFlag");
    }

    public static void setWallPaper(String wallpaperPath) {
        NmsPreferences.setStringValue("wallpaper", wallpaperPath);
    }

    public static String getWallPaper() {
        return NmsPreferences.getStringValue("wallpaper");
    }

    public static void setSendAsSMSFlag(int flag) {
        NmsPreferences.setIntegerValue("send_as_sms", flag);
    }

    public static boolean getSendAsSMSFlag() {
        return 1 != NmsPreferences.getIntegerValue("send_as_sms");
    }

    public static void setCaptionFlag(int flag) {
        NmsPreferences.setIntegerValue("caption", flag);
    }

    public static boolean getCaptionFlag() {
        return 1 != NmsPreferences.getIntegerValue("caption");
    }
    
    public static boolean getPhotoCaptionFlag(){
        return 1 != NmsPreferences.getIntegerValue("photo_caption");
    }
    
    public static void setPhotoCaptionFlag(int flag){
        NmsPreferences.setIntegerValue("photo_caption", flag);
    }
    
    public static boolean getVideoCaptionFlag(){
        return 1 != NmsPreferences.getIntegerValue("video_caption");
    }
    
    public static void setShowReadStatusFlag(int flag){
        NmsPreferences.setIntegerValue("read_status", flag);
    }
    
    public static boolean getShowReadStatusFlag(){
        return 1 == NmsPreferences.getIntegerValue("read_status");
    }
    
    public static void setShowRemindersFlag(int flag){
        NmsPreferences.setIntegerValue("show_reminders", flag);
    }
    
    public static boolean getShowRemindersFlag(){
        return 1 != NmsPreferences.getIntegerValue("show_reminders");
    }
    
    public static void setVideoCaptionFlag(int flag){
        NmsPreferences.setIntegerValue("video_caption", flag);
    }
    
    public static boolean getAudioCaptionFlag(){
        return 1 != NmsPreferences.getIntegerValue("audio_caption");
    }
    
    public static void setAudioCaptionFlag(int flag){
        NmsPreferences.setIntegerValue("audio_caption", flag);
    }
    
    public static boolean getAutoDownloadFlag(){
        return 0 != NmsPreferences.getIntegerValue("auto_download");
    }
    
    public static void setAutoDownloadFlag(int flag){
        NmsPreferences.setIntegerValue("auto_download", flag);
    }
    
    public static int getSketchColor(){
        return NmsPreferences.getIntegerValue("sketch_color");
    }
    
    public static void setSketchColor(int color){
        NmsPreferences.setIntegerValue("sketch_color", color);
    }
    
    public static int getSketchSize(){
        return NmsPreferences.getIntegerValue("sketch_size");
    }
    
    public static void setSketchSize(int size){
        NmsPreferences.setIntegerValue("sketch_size", size);
    }
    
    
    public static long getClearFlowTime(){
        return NmsPreferences.getLongValue("flow");
    }
    
    public static void setClearFlowTime(long time){
        NmsPreferences.setLongValue("flow", time);
    }
    
    public static void setPnType(String pnType) {
        NmsPreferences.setStringValue("pnType", pnType);
    }

    public static String getPnType() {
        return NmsPreferences.getStringValue("pnType");
    }
    
    public static void setSketchCustomColor(String color) {
        NmsPreferences.setStringValue("sketch_custom_color", color);
    }

    public static String getSketchCustomColor() {
        return NmsPreferences.getStringValue("sketch_custom_color");
    }
    
    public static int getFirstTimeRunForIntegration(){
        return NmsPreferences.getIntegerValue("first_run_for_integration");
    }
    
    public static void setFirstTimeRunForIntegration(){
        NmsPreferences.setIntegerValue("first_run_for_integration", 1000);
    }
    
    public static void setCurrentConversation(int id){
        NmsPreferences.setIntegerValue("current_conversation_id", id);
    }
    
    public static int getCurrentConversation(){
        return NmsPreferences.getIntegerValue("current_conversation_id");
    }
}
