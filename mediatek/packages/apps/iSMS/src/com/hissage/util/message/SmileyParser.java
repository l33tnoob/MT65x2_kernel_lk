package com.hissage.util.message;

import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.ImageSpan;

import com.hissage.R;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.util.log.NmsLog;

/**
 * A class for annotating a CharSequence with spans to convert textual emoticons
 * to graphical ones.
 */
public class SmileyParser {
    // Singleton stuff
    private static SmileyParser sInstance;

    public static SmileyParser getInstance() {
        return sInstance;
    }

    public static void init(Context context) {
        sInstance = new SmileyParser(context);
    }

    private final Context mContext;
    private final String[] mSmileyTexts;
    private final Pattern mPattern;
    private final HashMap<String, Integer> mSmileyToRes;
    private final HashMap<String, Integer> mLargeCnRes;
    private final HashMap<String, Integer> mLargeEnRes;
    private final HashMap<String, Integer> mDynamicEnRes;
    private final HashMap<String, Integer> mDynamicCnRes;
    private final HashMap<String, Integer> mAdEnRes;
    private final HashMap<String, Integer> mAdCnRes;
    private final HashMap<String, Integer> mXmEnRes;
    private final HashMap<String, Integer> mXmCnRes;
    private final String[] mLargeCnTexts;
    private final String[] mLargeEnTexts;
    private final String[] mDynamicEnTexts;
    private final String[] mDynamicCnTexts;
    private final String[] mAdCnTexts;
    private final String[] mAdEnTexts;
    private final String[] mXmEnTexts;
    private final String[] mXmCnTexts;

    private SmileyParser(Context context) {
        mContext = context;
        mSmileyTexts = mContext.getResources().getStringArray(DEFAULT_SMILEY_TEXTS);
        mLargeCnTexts = mContext.getResources().getStringArray(LARGE_SMILEY_CN);
        mLargeEnTexts = mContext.getResources().getStringArray(LARGE_SMILEY_EN);
        mDynamicEnTexts = mContext.getResources().getStringArray(DYNAMIC_SMIPEY_EN);
        mDynamicCnTexts = mContext.getResources().getStringArray(DYNAMIC_SMIPEY_CN);
        mAdCnTexts = mContext.getResources().getStringArray(AD_SMIPEY_CN);
        mAdEnTexts = mContext.getResources().getStringArray(AD_SMIPEY_EN);
        mXmCnTexts = mContext.getResources().getStringArray(XM_SMIPEY_CN);
        mXmEnTexts = mContext.getResources().getStringArray(XM_SMIPEY_EN);
        mSmileyToRes = buildSmileyToRes();
        mLargeCnRes = buildLargeCnRes();
        mLargeEnRes = buildLargeEnRes();
        mDynamicEnRes = buildDynamicEnRes();
        mDynamicCnRes = buildDynamicCnRes();
        mAdCnRes = buildAdCnRes();
        mAdEnRes = buildAdEnRes();
        mXmEnRes = buildXmEnRes();
        mXmCnRes = buildXmCnRes();
        mPattern = buildPattern();
    }

    static class Smileys {
        private static final int[] sIconIds = { R.drawable.emo_small_01, R.drawable.emo_small_02,
                R.drawable.emo_small_03, R.drawable.emo_small_04, R.drawable.emo_small_05,
                R.drawable.emo_small_06, R.drawable.emo_small_07, R.drawable.emo_small_08,
                R.drawable.emo_small_09, R.drawable.emo_small_10, R.drawable.emo_small_11,
                R.drawable.emo_small_12, R.drawable.emo_small_13, R.drawable.emo_small_14,
                R.drawable.emo_small_15, R.drawable.emo_small_16, R.drawable.emo_small_17,
                R.drawable.emo_small_18, R.drawable.emo_small_19, R.drawable.emo_small_20,
                R.drawable.emo_small_21, R.drawable.emo_small_22, R.drawable.emo_small_23,
                R.drawable.emo_small_24, R.drawable.emo_small_25, R.drawable.emo_small_26,
                R.drawable.emo_small_27, R.drawable.emo_small_28, R.drawable.emo_small_29,
                R.drawable.emo_small_30, R.drawable.emo_small_31, R.drawable.emo_small_32,
                R.drawable.emo_small_33, R.drawable.emo_small_34, R.drawable.emo_small_35,
                R.drawable.emo_small_36, R.drawable.emo_small_37, R.drawable.emo_small_38,
                R.drawable.emo_small_39, R.drawable.emo_small_40, R.drawable.good, R.drawable.no,
                R.drawable.ok, R.drawable.victory, R.drawable.seduce, R.drawable.down,
                R.drawable.rain, R.drawable.lightning, R.drawable.sun, R.drawable.microphone,
                R.drawable.clock, R.drawable.email, R.drawable.candle, R.drawable.birthday_cake,
                R.drawable.gift, R.drawable.star, R.drawable.heart, R.drawable.brokenheart,
                R.drawable.bulb, R.drawable.music, R.drawable.shenma, R.drawable.fuyun,
                R.drawable.rice, R.drawable.roses, R.drawable.film, R.drawable.aeroplane,
                R.drawable.umbrella, R.drawable.caonima, R.drawable.penguin, R.drawable.pig };

        public static int HAPPY = 0;
        public static int WINKING = 1;
        public static int SAD = 2;
        public static int CRYING = 3;
        public static int COOL = 4;
        public static int KISSING = 5;
        public static int TONGUE = 6;
        public static int YRLLING = 7;
        public static int ANGEL = 8;
        public static int SURPRISED = 9;
        public static int EMBARRASSED = 10;
        public static int MONRY = 11;
        public static int WRONG = 12;
        public static int UNDECIDED = 13;
        public static int LAUGHING = 14;
        public static int CONFUSED = 15;
        public static int LIPSARESEALED = 16;
        public static int HEART = 17;
        public static int MAD = 18;
        public static int SMIRK = 19;
        public static int POKER = 20;
        public static int SLEEP = 21;
        public static int VOMIT = 22;
        public static int CHARMING = 23;
        public static int SPEECHLESS = 24;
        public static int DEMONS = 25;
        public static int GRIEVANCE = 26;
        public static int ABSORBED = 27;
        public static int CUTE = 28;
        public static int SLEEPY = 29;
        public static int STRUGGLE = 30;
        public static int ANGRY = 31;
        public static int HORROR = 32;
        public static int FAINT = 33;
        public static int SLOBBER = 34;
        public static int BADLOL = 35;
        public static int GOBALLISTIC = 36;
        public static int BOMB = 37;
        public static int DOMINEERING = 38;
        public static int DEPRESSED = 39;
        public static int GOOD = 40;
        public static int NO = 41;
        public static int OK = 42;
        public static int VICROTY = 43;
        public static int SEDUCE = 44;
        public static int DOWN = 45;
        public static int RAIN = 46;
        public static int LIGHTNING = 47;
        public static int SUN = 48;
        public static int MICROPHONE = 49;
        public static int CLOCK = 50;
        public static int EMAIL = 51;
        public static int CANDLE = 52;
        public static int BIRTHDAYCAKE = 53;
        public static int GIFT = 54;
        public static int STAR = 55;
        public static int REDHEART = 56;
        public static int BROKENHEART = 57;
        public static int BULB = 58;
        public static int MUSIC = 59;
        public static int SHENMA = 60;
        public static int FUYUN = 61;
        public static int RICE = 62;
        public static int ROSES = 63;
        public static int FILM = 64;
        public static int AEROPLANE = 65;
        public static int UMBRELLA = 66;
        public static int CAONIMA = 67;
        public static int PENGUIN = 68;
        public static int PIG = 69;

        public static int getSmileyResource(int which) {
            return sIconIds[which];
        }
    }

    static class LargeSmileys {
        private static final int[] sIconIds = { R.drawable.emo_praise, R.drawable.emo_gift,
                R.drawable.emo_kongfu, R.drawable.emo_shower, R.drawable.emo_scare,
                R.drawable.emo_ill, R.drawable.emo_rich, R.drawable.emo_fly, R.drawable.emo_angry,
                R.drawable.emo_approve, R.drawable.emo_boring, R.drawable.emo_cry,
                R.drawable.emo_driving, R.drawable.emo_eating, R.drawable.emo_happy,
                R.drawable.emo_hold, R.drawable.emo_holiday, R.drawable.emo_love,
                R.drawable.emo_pray, R.drawable.emo_pressure, R.drawable.emo_sing,
                R.drawable.emo_sleep, R.drawable.emo_sports, R.drawable.emo_swimming };

        public static int PRAISE = 0;
        public static int GIFT = 1;
        public static int KONGFU = 2;
        public static int SHOWER = 3;
        public static int SCARE = 4;
        public static int ILL = 5;
        public static int RICH = 6;
        public static int FLY = 7;
        public static int ANGRY = 8;
        public static int APPROVE = 9;
        public static int BORING = 10;
        public static int CRY = 11;
        public static int DRIVING = 12;
        public static int EATING = 13;
        public static int HAPPY = 14;
        public static int HOLD = 15;
        public static int HOLIDAY = 16;
        public static int LOVE = 17;
        public static int PRAY = 18;
        public static int PRESSURE = 19;
        public static int SING = 20;
        public static int SLEEP = 21;
        public static int SPORTS = 22;
        public static int SWIMMING = 23;

        public static int getSmileyResource(int which) {
            return sIconIds[which];
        }
    }

    static class DynamicSmileys {
        private static final int[] sIconIds = { R.drawable.emo_dynamic_01,
                R.drawable.emo_dynamic_02, R.drawable.emo_dynamic_03, R.drawable.emo_dynamic_04,
                R.drawable.emo_dynamic_05, R.drawable.emo_dynamic_06, R.drawable.emo_dynamic_07,
                R.drawable.emo_dynamic_08, R.drawable.emo_dynamic_09, R.drawable.emo_dynamic_10,
                R.drawable.emo_dynamic_11, R.drawable.emo_dynamic_12, R.drawable.emo_dynamic_13,
                R.drawable.emo_dynamic_14, R.drawable.emo_dynamic_15, R.drawable.emo_dynamic_16,
                R.drawable.emo_dynamic_17, R.drawable.emo_dynamic_18, R.drawable.emo_dynamic_19,
                R.drawable.emo_dynamic_20, R.drawable.emo_dynamic_21, R.drawable.emo_dynamic_22,
                R.drawable.emo_dynamic_23, R.drawable.emo_dynamic_24 };

        public static int HAPPY = 0;
        public static int CLAPPING = 1;
        public static int LOVE = 2;
        public static int PROUDLY = 3;
        public static int DISDAIN = 4;
        public static int IMPATIENT = 5;
        public static int SLAP = 6;
        public static int ANGRY = 7;
        public static int CURSE = 8;
        public static int HOWEVER = 9;
        public static int TANGLE = 10;
        public static int CRY = 11;
        public static int DISCOURAGING = 12;
        public static int ANGRY_TOO = 13;
        public static int AROUND = 14;
        public static int PASSING = 15;
        public static int WORSHIP = 16;
        public static int PURE = 17;
        public static int BYE = 18;
        public static int INNOCENT = 19;
        public static int AMAZING = 20;
        public static int HUNGRY = 21;
        public static int SLEEP = 22;
        public static int WISHES = 23;

        public static int getSmileyResource(int which) {
            return sIconIds[which];
        }
    }

    static class AdSmileys {
        private static final int[] sIconIds = { R.drawable.ad01, R.drawable.ad02, R.drawable.ad03,
                R.drawable.ad04, R.drawable.ad05, R.drawable.ad06, R.drawable.ad07,
                R.drawable.ad08, R.drawable.ad09, R.drawable.ad10, R.drawable.ad11,
                R.drawable.ad12, R.drawable.ad13, R.drawable.ad14, R.drawable.ad15,
                R.drawable.ad16, R.drawable.ad17, R.drawable.ad18, R.drawable.ad19,
                R.drawable.ad20, R.drawable.ad21, R.drawable.ad22, R.drawable.ad23, R.drawable.ad24 };

        public static int BRETING = 0;
        public static int SIDESHOW = 1;
        public static int CRY = 2;
        public static int HALO = 3;
        public static int HEARTBEAT = 4;
        public static int DOZING = 5;
        public static int LAUGH = 6;
        public static int IMPATIENCE = 7;
        public static int RUNNING = 8;
        public static int CURSE = 9;
        public static int SWEAT = 10;
        public static int COMPLACENT = 11;
        public static int SNEEZE = 12;
        public static int DANCING = 13;
        public static int ANGRY = 14;
        public static int JUMPING = 15;
        public static int PEEP = 16;
        public static int PRATFALL = 17;
        public static int EATING = 18;
        public static int HOOP = 19;
        public static int PASSING = 20;
        public static int SHAKE = 21;
        public static int BYE = 22;
        public static int ASTRICTION = 23;

        public static int getSmileyResource(int which) {
            return sIconIds[which];
        }
    }

    static class XmSmileys {
        private static final int[] sIconIds = { R.drawable.xm01, R.drawable.xm02, R.drawable.xm03,
                R.drawable.xm04, R.drawable.xm05, R.drawable.xm06, R.drawable.xm07,
                R.drawable.xm08, R.drawable.xm09, R.drawable.xm10, R.drawable.xm11,
                R.drawable.xm12, R.drawable.xm13, R.drawable.xm14, R.drawable.xm15,
                R.drawable.xm16, R.drawable.xm17, R.drawable.xm18, R.drawable.xm19,
                R.drawable.xm20, R.drawable.xm21, R.drawable.xm22, R.drawable.xm23, R.drawable.xm24 };

        public static int WINK = 0;
        public static int QUESTION = 1;
        public static int VOMIT = 2;
        public static int BADLUCK = 3;
        public static int LIKE = 4;
        public static int CUTE = 5;
        public static int DIZZY = 6;
        public static int TIRED = 7;
        public static int CONFUSED = 8;
        public static int KISS = 9;
        public static int LOVE = 10;
        public static int CURSE = 11;
        public static int MONEY = 12;
        public static int SMILE = 13;
        public static int CRY = 14;
        public static int ANGRY = 15;
        public static int PLEASED = 16;
        public static int NAUGHTY = 17;
        public static int THRILLER = 18;
        public static int DAZE = 19;
        public static int EMBARRASSED = 20;
        public static int SLEEP = 21;
        public static int DISAPPEAR = 22;
        public static int GRIEVANCE = 23;

        public static int getSmileyResource(int which) {
            return sIconIds[which];
        }
    }

    // NOTE: if you change anything about this array, you must make the
    // corresponding change
    // to the string arrays: default_smiley_texts and default_smiley_names in
    // res/values/arrays.xml
    public static final int[] DEFAULT_SMILEY_RES_IDS = { Smileys.getSmileyResource(Smileys.HAPPY), // 0
            Smileys.getSmileyResource(Smileys.WINKING), // 1
            Smileys.getSmileyResource(Smileys.SAD), // 2
            Smileys.getSmileyResource(Smileys.CRYING), // 3
            Smileys.getSmileyResource(Smileys.COOL), // 4
            Smileys.getSmileyResource(Smileys.KISSING), // 5
            Smileys.getSmileyResource(Smileys.TONGUE), // 6
            Smileys.getSmileyResource(Smileys.YRLLING), // 7
            Smileys.getSmileyResource(Smileys.ANGEL), // 8
            Smileys.getSmileyResource(Smileys.SURPRISED), // 9
            Smileys.getSmileyResource(Smileys.EMBARRASSED), // 10
            Smileys.getSmileyResource(Smileys.MONRY), // 11
            Smileys.getSmileyResource(Smileys.WRONG), // 12
            Smileys.getSmileyResource(Smileys.UNDECIDED), // 13
            Smileys.getSmileyResource(Smileys.LAUGHING), // 14
            Smileys.getSmileyResource(Smileys.CONFUSED), // 15
            Smileys.getSmileyResource(Smileys.LIPSARESEALED), // 16
            Smileys.getSmileyResource(Smileys.HEART), // 17
            Smileys.getSmileyResource(Smileys.MAD), // 18
            Smileys.getSmileyResource(Smileys.SMIRK), // 19
            Smileys.getSmileyResource(Smileys.POKER), // 20
            Smileys.getSmileyResource(Smileys.SLEEP), // 21
            Smileys.getSmileyResource(Smileys.VOMIT), // 22
            Smileys.getSmileyResource(Smileys.CHARMING), // 23
            Smileys.getSmileyResource(Smileys.SPEECHLESS), // 24
            Smileys.getSmileyResource(Smileys.DEMONS), // 25
            Smileys.getSmileyResource(Smileys.GRIEVANCE), // 26
            Smileys.getSmileyResource(Smileys.ABSORBED), // 27
            Smileys.getSmileyResource(Smileys.CUTE), // 28
            Smileys.getSmileyResource(Smileys.SLEEPY), // 29
            Smileys.getSmileyResource(Smileys.STRUGGLE), // 30
            Smileys.getSmileyResource(Smileys.ANGRY), // 31
            Smileys.getSmileyResource(Smileys.HORROR), // 32
            Smileys.getSmileyResource(Smileys.FAINT), // 33
            Smileys.getSmileyResource(Smileys.SLOBBER), // 34
            Smileys.getSmileyResource(Smileys.BADLOL), // 35
            Smileys.getSmileyResource(Smileys.GOBALLISTIC), // 36
            Smileys.getSmileyResource(Smileys.BOMB), // 37
            Smileys.getSmileyResource(Smileys.DOMINEERING), // 38
            Smileys.getSmileyResource(Smileys.DEPRESSED), // 39
            Smileys.getSmileyResource(Smileys.GOOD), // 40
            Smileys.getSmileyResource(Smileys.NO), // 41
            Smileys.getSmileyResource(Smileys.OK), // 42
            Smileys.getSmileyResource(Smileys.VICROTY), // 43
            Smileys.getSmileyResource(Smileys.SEDUCE), // 44
            Smileys.getSmileyResource(Smileys.DOWN), // 45
            Smileys.getSmileyResource(Smileys.RAIN), // 46
            Smileys.getSmileyResource(Smileys.LIGHTNING), // 47
            Smileys.getSmileyResource(Smileys.SUN), // 48
            Smileys.getSmileyResource(Smileys.MICROPHONE), // 49
            Smileys.getSmileyResource(Smileys.CLOCK), // 50
            Smileys.getSmileyResource(Smileys.EMAIL), // 51
            Smileys.getSmileyResource(Smileys.CANDLE), // 52
            Smileys.getSmileyResource(Smileys.BIRTHDAYCAKE), // 53
            Smileys.getSmileyResource(Smileys.GIFT), // 54
            Smileys.getSmileyResource(Smileys.STAR), // 55
            Smileys.getSmileyResource(Smileys.REDHEART), // 56
            Smileys.getSmileyResource(Smileys.BROKENHEART), // 57
            Smileys.getSmileyResource(Smileys.BULB), // 58
            Smileys.getSmileyResource(Smileys.MUSIC), // 59
            Smileys.getSmileyResource(Smileys.SHENMA), // 60
            Smileys.getSmileyResource(Smileys.FUYUN), // 61
            Smileys.getSmileyResource(Smileys.RICE), // 62
            Smileys.getSmileyResource(Smileys.ROSES), // 63
            Smileys.getSmileyResource(Smileys.FILM), // 64
            Smileys.getSmileyResource(Smileys.AEROPLANE), // 65
            Smileys.getSmileyResource(Smileys.UMBRELLA), // 66
            Smileys.getSmileyResource(Smileys.CAONIMA), // 67
            Smileys.getSmileyResource(Smileys.PENGUIN), // 68
            Smileys.getSmileyResource(Smileys.PIG), // 69

    };

    public static final int[] LARGE_SMILEY_RES_IDS = {
            LargeSmileys.getSmileyResource(LargeSmileys.PRAISE), // 0
            LargeSmileys.getSmileyResource(LargeSmileys.GIFT), // 1
            LargeSmileys.getSmileyResource(LargeSmileys.KONGFU), // 2
            LargeSmileys.getSmileyResource(LargeSmileys.SHOWER), // 3
            LargeSmileys.getSmileyResource(LargeSmileys.SCARE), // 4
            LargeSmileys.getSmileyResource(LargeSmileys.ILL), // 5
            LargeSmileys.getSmileyResource(LargeSmileys.RICH), // 6
            LargeSmileys.getSmileyResource(LargeSmileys.FLY), // 7
            LargeSmileys.getSmileyResource(LargeSmileys.ANGRY), // 8
            LargeSmileys.getSmileyResource(LargeSmileys.APPROVE), // 9
            LargeSmileys.getSmileyResource(LargeSmileys.BORING), // 10
            LargeSmileys.getSmileyResource(LargeSmileys.CRY), // 11
            LargeSmileys.getSmileyResource(LargeSmileys.DRIVING), // 12
            LargeSmileys.getSmileyResource(LargeSmileys.EATING), // 13
            LargeSmileys.getSmileyResource(LargeSmileys.HAPPY), // 14
            LargeSmileys.getSmileyResource(LargeSmileys.HOLD), // 15
            LargeSmileys.getSmileyResource(LargeSmileys.HOLIDAY), // 16
            LargeSmileys.getSmileyResource(LargeSmileys.LOVE), // 17
            LargeSmileys.getSmileyResource(LargeSmileys.PRAY), // 18
            LargeSmileys.getSmileyResource(LargeSmileys.PRESSURE), // 19
            LargeSmileys.getSmileyResource(LargeSmileys.SING), // 20
            LargeSmileys.getSmileyResource(LargeSmileys.SLEEP), // 21
            LargeSmileys.getSmileyResource(LargeSmileys.SPORTS), // 22
            LargeSmileys.getSmileyResource(LargeSmileys.SWIMMING), // 23
    };

    public static final int[] DYNAMIC_SMILEY_RES_IDS = {
            DynamicSmileys.getSmileyResource(DynamicSmileys.HAPPY), // 0
            DynamicSmileys.getSmileyResource(DynamicSmileys.CLAPPING), // 1
            DynamicSmileys.getSmileyResource(DynamicSmileys.LOVE), // 2
            DynamicSmileys.getSmileyResource(DynamicSmileys.PROUDLY), // 3
            DynamicSmileys.getSmileyResource(DynamicSmileys.DISDAIN), // 4
            DynamicSmileys.getSmileyResource(DynamicSmileys.IMPATIENT), // 5
            DynamicSmileys.getSmileyResource(DynamicSmileys.SLAP), // 6
            DynamicSmileys.getSmileyResource(DynamicSmileys.ANGRY), // 7
            DynamicSmileys.getSmileyResource(DynamicSmileys.CURSE), // 8
            DynamicSmileys.getSmileyResource(DynamicSmileys.HOWEVER), // 9
            DynamicSmileys.getSmileyResource(DynamicSmileys.TANGLE), // 10
            DynamicSmileys.getSmileyResource(DynamicSmileys.CRY), // 11
            DynamicSmileys.getSmileyResource(DynamicSmileys.DISCOURAGING), // 12
            DynamicSmileys.getSmileyResource(DynamicSmileys.ANGRY_TOO), // 13
            DynamicSmileys.getSmileyResource(DynamicSmileys.AROUND), // 14
            DynamicSmileys.getSmileyResource(DynamicSmileys.PASSING), // 15
            DynamicSmileys.getSmileyResource(DynamicSmileys.WORSHIP), // 16
            DynamicSmileys.getSmileyResource(DynamicSmileys.PURE), // 17
            DynamicSmileys.getSmileyResource(DynamicSmileys.BYE), // 18
            DynamicSmileys.getSmileyResource(DynamicSmileys.INNOCENT), // 19
            DynamicSmileys.getSmileyResource(DynamicSmileys.AMAZING), // 20
            DynamicSmileys.getSmileyResource(DynamicSmileys.HUNGRY), // 21
            DynamicSmileys.getSmileyResource(DynamicSmileys.SLEEP), // 22
            DynamicSmileys.getSmileyResource(DynamicSmileys.WISHES), // 23
    };

    public static final int[] AD_SMILEY_RES_IDS = { AdSmileys.getSmileyResource(AdSmileys.BRETING), // 0
            AdSmileys.getSmileyResource(AdSmileys.SIDESHOW), // 1
            AdSmileys.getSmileyResource(AdSmileys.CRY), // 2
            AdSmileys.getSmileyResource(AdSmileys.HALO), // 3
            AdSmileys.getSmileyResource(AdSmileys.HEARTBEAT), // 4
            AdSmileys.getSmileyResource(AdSmileys.DOZING), // 5
            AdSmileys.getSmileyResource(AdSmileys.LAUGH), // 6
            AdSmileys.getSmileyResource(AdSmileys.IMPATIENCE), // 7
            AdSmileys.getSmileyResource(AdSmileys.RUNNING), // 8
            AdSmileys.getSmileyResource(AdSmileys.CURSE), // 9
            AdSmileys.getSmileyResource(AdSmileys.SWEAT), // 10
            AdSmileys.getSmileyResource(AdSmileys.COMPLACENT), // 11
            AdSmileys.getSmileyResource(AdSmileys.SNEEZE), // 12
            AdSmileys.getSmileyResource(AdSmileys.DANCING), // 13
            AdSmileys.getSmileyResource(AdSmileys.ANGRY), // 14
            AdSmileys.getSmileyResource(AdSmileys.JUMPING), // 15
            AdSmileys.getSmileyResource(AdSmileys.PEEP), // 16
            AdSmileys.getSmileyResource(AdSmileys.PRATFALL), // 17
            AdSmileys.getSmileyResource(AdSmileys.EATING), // 18
            AdSmileys.getSmileyResource(AdSmileys.HOOP), // 19
            AdSmileys.getSmileyResource(AdSmileys.PASSING), // 20
            AdSmileys.getSmileyResource(AdSmileys.SHAKE), // 21
            AdSmileys.getSmileyResource(AdSmileys.BYE), // 22
            AdSmileys.getSmileyResource(AdSmileys.ASTRICTION), // 23
    };

    public static final int[] XM_SMILEY_RES_IDS = { XmSmileys.getSmileyResource(XmSmileys.WINK), // 0
            XmSmileys.getSmileyResource(XmSmileys.QUESTION), // 1
            XmSmileys.getSmileyResource(XmSmileys.VOMIT), // 2
            XmSmileys.getSmileyResource(XmSmileys.BADLUCK), // 3
            XmSmileys.getSmileyResource(XmSmileys.LIKE), // 4
            XmSmileys.getSmileyResource(XmSmileys.CUTE), // 5
            XmSmileys.getSmileyResource(XmSmileys.DIZZY), // 6
            XmSmileys.getSmileyResource(XmSmileys.TIRED), // 7
            XmSmileys.getSmileyResource(XmSmileys.CONFUSED), // 8
            XmSmileys.getSmileyResource(XmSmileys.KISS), // 9
            XmSmileys.getSmileyResource(XmSmileys.LOVE), // 10
            XmSmileys.getSmileyResource(XmSmileys.CURSE), // 11
            XmSmileys.getSmileyResource(XmSmileys.MONEY), // 12
            XmSmileys.getSmileyResource(XmSmileys.SMILE), // 13
            XmSmileys.getSmileyResource(XmSmileys.CRY), // 14
            XmSmileys.getSmileyResource(XmSmileys.ANGRY), // 15
            XmSmileys.getSmileyResource(XmSmileys.PLEASED), // 16
            XmSmileys.getSmileyResource(XmSmileys.NAUGHTY), // 17
            XmSmileys.getSmileyResource(XmSmileys.THRILLER), // 18
            XmSmileys.getSmileyResource(XmSmileys.DAZE), // 19
            XmSmileys.getSmileyResource(XmSmileys.EMBARRASSED), // 20
            XmSmileys.getSmileyResource(XmSmileys.SLEEP), // 21
            XmSmileys.getSmileyResource(XmSmileys.DISAPPEAR), // 22
            XmSmileys.getSmileyResource(XmSmileys.GRIEVANCE), // 23
    };

    public static final int DEFAULT_SMILEY_TEXTS = R.array.emoticon_name;
    public static final int LARGE_SMILEY_CN = R.array.large_emoticon_name_ch;
    public static final int LARGE_SMILEY_EN = R.array.large_emoticon_name_en;
    public static final int DYNAMIC_SMIPEY_EN = R.array.dynamic_emoticon_name_en;
    public static final int DYNAMIC_SMIPEY_CN = R.array.dynamic_emoticon_name_ch;
    public static final int AD_SMIPEY_EN = R.array.ad_emoticon_name_en;
    public static final int AD_SMIPEY_CN = R.array.ad_emoticon_name_ch;
    public static final int XM_SMIPEY_EN = R.array.xm_emoticon_name_en;
    public static final int XM_SMIPEY_CN = R.array.xm_emoticon_name_ch;

    /**
     * Builds the hashtable we use for mapping the string version of a smiley
     * (e.g. ":-)") to a resource ID for the icon version.
     */
    private HashMap<String, Integer> buildSmileyToRes() {
        if (DEFAULT_SMILEY_RES_IDS.length != mSmileyTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mSmileyTexts.length);
        for (int i = 0; i < mSmileyTexts.length; i++) {
            smileyToRes.put(mSmileyTexts[i], DEFAULT_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildLargeCnRes() {
        if (LARGE_SMILEY_RES_IDS.length != mLargeCnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mLargeCnTexts.length);
        for (int i = 0; i < mLargeCnTexts.length; i++) {
            smileyToRes.put(mLargeCnTexts[i], LARGE_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildLargeEnRes() {
        if (LARGE_SMILEY_RES_IDS.length != mLargeEnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mLargeEnTexts.length);
        for (int i = 0; i < mLargeEnTexts.length; i++) {
            smileyToRes.put(mLargeEnTexts[i], LARGE_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildDynamicEnRes() {
        if (DYNAMIC_SMILEY_RES_IDS.length != mDynamicEnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mDynamicEnTexts.length);
        for (int i = 0; i < mDynamicEnTexts.length; i++) {
            smileyToRes.put(mDynamicEnTexts[i], DYNAMIC_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildDynamicCnRes() {
        if (DYNAMIC_SMILEY_RES_IDS.length != mDynamicCnTexts.length) {
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mDynamicCnTexts.length);
        for (int i = 0; i < mDynamicCnTexts.length; i++) {
            smileyToRes.put(mDynamicCnTexts[i], DYNAMIC_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildAdCnRes() {
        if (AD_SMILEY_RES_IDS.length != mAdCnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mAdCnTexts.length);
        for (int i = 0; i < mAdCnTexts.length; i++) {
            smileyToRes.put(mAdCnTexts[i], AD_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildAdEnRes() {
        if (AD_SMILEY_RES_IDS.length != mAdEnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mAdEnTexts.length);
        for (int i = 0; i < mAdEnTexts.length; i++) {
            smileyToRes.put(mAdEnTexts[i], AD_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildXmCnRes() {
        if (XM_SMILEY_RES_IDS.length != mXmCnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mXmCnTexts.length);
        for (int i = 0; i < mXmCnTexts.length; i++) {
            smileyToRes.put(mXmCnTexts[i], XM_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    private HashMap<String, Integer> buildXmEnRes() {
        if (XM_SMILEY_RES_IDS.length != mXmEnTexts.length) {
            // Throw an exception if someone updated DEFAULT_SMILEY_RES_IDS
            // and failed to update arrays.xml
            throw new IllegalStateException("Smiley resource ID/text mismatch");
        }

        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(mXmEnTexts.length);
        for (int i = 0; i < mXmEnTexts.length; i++) {
            smileyToRes.put(mXmEnTexts[i], XM_SMILEY_RES_IDS[i]);
        }

        return smileyToRes;
    }

    /**
     * Builds the regular expression we use to find smileys in
     * {@link #addSmileySpans}.
     */
    private Pattern buildPattern() {
        // Set the StringBuilder capacity with the assumption that the average
        // smiley is 3 characters long.
        StringBuilder patternString = new StringBuilder(mSmileyTexts.length * 3);

        // Build a regex that looks like (:-)|:-(|...), but escaping the smilies
        // properly so they will be interpreted literally by the regex matcher.
        patternString.append('(');
        for (String s : mSmileyTexts) {
            patternString.append(Pattern.quote(s));
            patternString.append('|');
        }
        // Replace the extra '|' with a ')'
        patternString.replace(patternString.length() - 1, patternString.length(), ")");

        return Pattern.compile(patternString.toString());
    }

    /**
     * Adds ImageSpans to a CharSequence that replace textual emoticons such as
     * :-) with a graphical version.
     * 
     * @param text
     *            A CharSequence possibly containing emoticons
     * @return A CharSequence annotated with ImageSpans covering any recognized
     *         emoticons.
     */
    public CharSequence addSmileySpans(CharSequence text) {
        SpannableStringBuilder builder = new SpannableStringBuilder(text);
        Matcher matcher = mPattern.matcher(text);
        while (matcher.find()) {
            int resId = mSmileyToRes.get(matcher.group());
            Drawable drawable = mContext.getResources().getDrawable(resId);
            int bound = mContext.getResources()
                    .getDimensionPixelOffset(R.dimen.emoticon_bound_size);
            drawable.setBounds(0, 0, bound, bound);
            builder.setSpan(new ImageSpan(drawable), matcher.start(), matcher.end(),
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }

        return builder;
    }

    public int addSmileySpans(Editable text) {
        int SpansSize = 0;
        ImageSpan[] emoticonList = text.getSpans(0, text.length(), ImageSpan.class);
        if (emoticonList.length != 0) {
            // for (int i = 0; i < emoticonList.length; i++) {
            // try {
            // text.removeSpan(emoticonList[i]);
            // } catch (Exception e) {
            // Log.i("addSmileySpans", "list len: " + emoticonList.length +
            // " index:" + i);
            // e.printStackTrace();
            // }
            // }

            for (int i = emoticonList.length - 1; i >= 0; i--) {
                try {
                    text.removeSpan(emoticonList[i]);
                } catch (Exception e) {
                    NmsLog.trace("addSmileySpans", "list len: " + emoticonList.length + " index:"
                            + i);
                    e.printStackTrace();
                }
            }
        }
        Matcher matcher = mPattern.matcher(text);
        while (matcher.find()) {
            if (SpansSize >= NmsCustomUIConfig.EMOTICONS_MAX_COUNT) {
                break;
            }
            int resId = mSmileyToRes.get(matcher.group());
            Drawable drawable = mContext.getResources().getDrawable(resId);
            int bound = mContext.getResources()
                    .getDimensionPixelOffset(R.dimen.emoticon_bound_size);
            drawable.setBounds(0, 0, bound, bound);
            text.setSpan(new ImageSpan(drawable), matcher.start(), matcher.end(),
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            SpansSize++;
        }
        return SpansSize;
    }

    public int getLargeRes(String text) {
        if (TextUtils.isEmpty(text)) {
            return 0;
        }
        Integer resId = mLargeCnRes.get(text);
        if (null == resId) {
            resId = mLargeEnRes.get(text);
            if (null == resId) {
                return 0;
            }
        }
        return resId;
    }

    public int getDynamicRes(String text) {
        if (TextUtils.isEmpty(text)) {
            return 0;
        }
        Integer resId = mDynamicCnRes.get(text);
        if (null == resId) {
            resId = mDynamicEnRes.get(text);
            if (null == resId) {
                return 0;
            }
        }
        return resId;
    }

    public int getAdRes(String text) {
        if (TextUtils.isEmpty(text)) {
            return 0;
        }
        Integer resId = mAdCnRes.get(text);
        if (null == resId) {
            resId = mAdEnRes.get(text);
            if (null == resId) {
                return 0;
            }
        }
        return resId;
    }

    public int getXmRes(String text) {
        if (TextUtils.isEmpty(text)) {
            return 0;
        }
        Integer resId = mXmCnRes.get(text);
        if (null == resId) {
            resId = mXmEnRes.get(text);
            if (null == resId) {
                return 0;
            }
        }
        return resId;
    }
}
