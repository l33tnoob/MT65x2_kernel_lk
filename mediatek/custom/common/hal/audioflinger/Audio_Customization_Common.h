#ifndef AUDIO_CUSTOMIZATION_COMMON_H
#define AUDIO_CUSTOMIZATION_COMMON_H

/****************************************************
* Define Volume Range of  sound & Voice.
*****************************************************/
#define DEVICE_MAX_VOLUME           (12)
#define DEVICE_VOICE_MAX_VOLUME     (12)
#define DEVICE_MIN_VOLUME           (-32)
#define DEVICE_VOICE_MIN_VOLUME     (-32)
#define DEVICE_VOLUME_RANGE     (64)
#define DEVICE_VOLUME_STEP (256)

/***************************************************
*adjust boot animation volume. the volume range is from 0 to 1.
*****************************************************/
#define BOOT_ANIMATION_VOLUME       (0.25)

/***************************************************
*(1)->Use Ref Mic as main mic; (0)->Use original main mic.
*****************************************************/
#define USE_REFMIC_IN_LOUDSPK       (0)

/****************************************************
* Define this will enable audio compensation filter for loudspeaker
*Please see ACF Document for detail.
*****************************************************/
#define ENABLE_AUDIO_COMPENSATION_FILTER

/***************************************************
* Define this will enable DRC for loudspeaker.
* If defined MTK_HD_AUDIO_ARCHITECTURE , don't support DRC in HAL, AudioFlinger will do DRC
* If undefined MTK_HD_AUDIO_ARCHITECTURE , support DRC in HAL, AudioFlinger won't do DRC
*****************************************************/
#ifndef MTK_HD_AUDIO_ARCHITECTURE
#define ENABLE_AUDIO_DRC_SPEAKER
#endif



/***************************************************
* Define this will enable headphone compensation filter.
*Please see HCF Document for detail.
*****************************************************/
#define ENABLE_HEADPHONE_COMPENSATION_FILTER
#define HEADPHONE_COMPENSATION_FLT_MODE (4)


/***************************************************
*Define this will enable SW stereo to mono on LCH & RCH
*If not define this, HW stereo to mono (only LCH) will be applied.
*****************************************************/
#define ENABLE_AUDIO_SW_STEREO_TO_MONO


/***************************************************
*Define this will enable high samplerate record.
*****************************************************/
#define ENABLE_HIGH_SAMPLERATE_RECORD


/****************************************************
* WARNING: this macro is now obsolete, please change
* the property value ro.camera.sound.forced=1 to take effect.
*the property is defined in alps\mediatek\config\YOUR_PROJECT
*\system.prop.
*****************************************************/
//#define FORCE_CAMERA_SHUTTER_SOUND_AUDIBLE


/****************************************************
* Define this  , speaker output will not do stero to mono,
*keep in stereo format,because stereo output can apply
*on more than 1 speaker.
*****************************************************/
//#define ENABLE_STEREO_SPEAKER


/****************************************************
* Define this will enable Voice  to use  VoiceBuffer
*when using speaker and headphone in incall mode.
*****************************************************/
//#define ALL_USING_VOICEBUFFER_INCALL


/****************************************************
*Define this, audioflinger will use first active stream samplerate
*as hardware setting. it is only used for verifying hardware
*****************************************************/
//#define AUDIO_HQA_SUPPORT

/****************************************************
* Define this  , Audio Policy will apply ro.camera.sound.forced setting in mediatek/config/project_name/system.prop 
* (Add this optionn because JB2 load is invalid
*****************************************************/
//#define ENABLE_CAMERA_SOUND_FORCED_SET

#define AUDIO_DROP_FRAME_COUNT_NORMAL 5
#define AUDIO_DROP_FRAME_COUNT_RECORD 5
#define AUDIO_DROP_FRAME_COUNT_CTS 5

/***************************************************
*(0)->copyright is asserted; (1)->no copyright is asserted.
*****************************************************/
#define CHANNEL_STATUS_COPY_BIT (1)

/***************************************************
*Specify category code (1 byte).
*****************************************************/
#define CHANNEL_STATUS_CATEGORY_CODE  (0x00)


/****************************************************
*Define this, HD Rec will use this for default param if no mode is set
*****************************************************/
#define DEFAULT_HDRecordEnhanceParas \
	0, 479, 16388, 36892, 37124, 8192,  768, 0,  4048, 2245, 611, 0, 0, 0, 0, 8192


/****************************************************
*Define MagiLoudness_TE_mode
*bit 0 earpiece DRC2.0  on=> bits 0 = 1
*bit 1 headset  DRC2.0  on=> bits 1 = 1
*bit 2 speaker  DRC2.0  on=> bits 2 = 1
*****************************************************/
#define MagiLoudness_TE_mode (0x0)


#define DEFAULT_HDRecordCompenFilter \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0, \
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0
#endif
