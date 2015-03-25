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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_H_


typedef enum mtk_camera_metadata_section {
    MTK_COLOR_CORRECTION,
    MTK_CONTROL,
    MTK_DEMOSAIC,
    MTK_EDGE,
    MTK_FLASH,
    MTK_FLASH_INFO,
    MTK_GEOMETRIC,
    MTK_HOT_PIXEL,
    MTK_HOT_PIXEL_INFO,
    MTK_JPEG,
    MTK_LENS,
    MTK_LENS_INFO,
    MTK_NOISE_REDUCTION,
    MTK_QUIRKS,
    MTK_REQUEST,
    MTK_SCALER,
    MTK_SENSOR,
    MTK_SENSOR_INFO,
    MTK_SHADING,
    MTK_STATISTICS,
    MTK_STATISTICS_INFO,
    MTK_TONEMAP,
    MTK_LED,
    MTK_INFO,
    MTK_BLACK_LEVEL,
    MTK_IOPIPE_INFO, 
    MTK_PROCESSOR_PRIVATE_INFO,
    MTK_SECTION_COUNT,
} mtk_camera_metadata_section_t;

/**
 * Hierarchy positions in enum space. All vendor extension tags must be
 * defined with tag >= VENDOR_SECTION_START
 */
 
typedef enum mtk_camera_metadata_section_start {
    MTK_COLOR_CORRECTION_START = MTK_COLOR_CORRECTION  << 16,
    MTK_CONTROL_START          = MTK_CONTROL           << 16,
    MTK_DEMOSAIC_START         = MTK_DEMOSAIC          << 16,
    MTK_EDGE_START             = MTK_EDGE              << 16,
    MTK_FLASH_START            = MTK_FLASH             << 16,
    MTK_FLASH_INFO_START       = MTK_FLASH_INFO        << 16,
    MTK_GEOMETRIC_START        = MTK_GEOMETRIC         << 16,
    MTK_HOT_PIXEL_START        = MTK_HOT_PIXEL         << 16,
    MTK_HOT_PIXEL_INFO_START   = MTK_HOT_PIXEL_INFO    << 16,
    MTK_JPEG_START             = MTK_JPEG              << 16,
    MTK_LENS_START             = MTK_LENS              << 16,
    MTK_LENS_INFO_START        = MTK_LENS_INFO         << 16,
    MTK_NOISE_REDUCTION_START  = MTK_NOISE_REDUCTION   << 16,
    MTK_QUIRKS_START           = MTK_QUIRKS            << 16,
    MTK_REQUEST_START          = MTK_REQUEST           << 16,
    MTK_SCALER_START           = MTK_SCALER            << 16,
    MTK_SENSOR_START           = MTK_SENSOR            << 16,
    MTK_SENSOR_INFO_START      = MTK_SENSOR_INFO       << 16,
    MTK_SHADING_START          = MTK_SHADING           << 16,
    MTK_STATISTICS_START       = MTK_STATISTICS        << 16,
    MTK_STATISTICS_INFO_START  = MTK_STATISTICS_INFO   << 16,
    MTK_TONEMAP_START          = MTK_TONEMAP           << 16,
    MTK_LED_START              = MTK_LED               << 16,
    MTK_INFO_START             = MTK_INFO              << 16,
    MTK_BLACK_LEVEL_START      = MTK_BLACK_LEVEL       << 16,
    MTK_IOPIPE_INFO_START      = MTK_IOPIPE_INFO       << 16, 
    MTK_PROCESSOR_PRIVATE_INFO_START = MTK_PROCESSOR_PRIVATE_INFO << 16,
} mtk_camera_metadata_section_start_t;



/**
 * Main enum for defining camera metadata tags.  New entries must always go
 * before the section _END tag to preserve existing enumeration values.  In
 * addition, the name and type of the tag needs to be added to
 * ""
 */
typedef enum mtk_camera_metadata_tag {
    MTK_COLOR_CORRECTION_MODE  = MTK_COLOR_CORRECTION_START,
    MTK_COLOR_CORRECTION_TRANSFORM,
    MTK_COLOR_CORRECTION_GAINS,
    MTK_COLOR_CORRECTION_END,
    
    MTK_CONTROL_AE_ANTIBANDING_MODE = MTK_CONTROL_START,
    MTK_CONTROL_AE_EXPOSURE_COMPENSATION,
    MTK_CONTROL_AE_LOCK,
    MTK_CONTROL_AE_MODE,
    MTK_CONTROL_AE_REGIONS,
    MTK_CONTROL_AE_TARGET_FPS_RANGE,
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER,
    MTK_CONTROL_AF_MODE,
    MTK_CONTROL_AF_REGIONS,
    MTK_CONTROL_AF_TRIGGER,
    MTK_CONTROL_AWB_LOCK,
    MTK_CONTROL_AWB_MODE,
    MTK_CONTROL_AWB_REGIONS,
    MTK_CONTROL_CAPTURE_INTENT,
    MTK_CONTROL_EFFECT_MODE,
    MTK_CONTROL_MODE,
    MTK_CONTROL_SCENE_MODE,
    MTK_CONTROL_VIDEO_STABILIZATION_MODE,
    MTK_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES,
    MTK_CONTROL_AE_AVAILABLE_MODES,
    MTK_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES,
    MTK_CONTROL_AE_COMPENSATION_RANGE,
    MTK_CONTROL_AE_COMPENSATION_STEP,
    MTK_CONTROL_AF_AVAILABLE_MODES,
    MTK_CONTROL_AVAILABLE_EFFECTS,
    MTK_CONTROL_AVAILABLE_SCENE_MODES,
    MTK_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES,
    MTK_CONTROL_AWB_AVAILABLE_MODES,
    MTK_CONTROL_MAX_REGIONS,
    MTK_CONTROL_SCENE_MODE_OVERRIDES,
    MTK_CONTROL_AE_PRECAPTURE_ID,
    MTK_CONTROL_AE_STATE,
    MTK_CONTROL_AF_STATE,
    MTK_CONTROL_AF_TRIGGER_ID,
    MTK_CONTROL_AWB_STATE,
    MTK_CONTROL_END,

    MTK_DEMOSAIC_MODE          = MTK_DEMOSAIC_START,
    MTK_DEMOSAIC_END,

    MTK_EDGE_MODE              = MTK_EDGE_START,
    MTK_EDGE_STRENGTH,
    MTK_EDGE_END,

    MTK_FLASH_FIRING_POWER     = MTK_FLASH_START,
    MTK_FLASH_FIRING_TIME,
    MTK_FLASH_MODE,
    MTK_FLASH_COLOR_TEMPERATURE,
    MTK_FLASH_MAX_ENERGY,
    MTK_FLASH_STATE,
    MTK_FLASH_END,

    MTK_FLASH_INFO_AVAILABLE   = MTK_FLASH_INFO_START,
    MTK_FLASH_INFO_CHARGE_DURATION,
    MTK_FLASH_INFO_END,

    MTK_GEOMETRIC_MODE         = MTK_GEOMETRIC_START,
    MTK_GEOMETRIC_STRENGTH,
    MTK_GEOMETRIC_END,

    MTK_HOT_PIXEL_MODE         = MTK_HOT_PIXEL_START,
    MTK_HOT_PIXEL_END,

    MTK_HOT_PIXEL_INFO_MAP     = MTK_HOT_PIXEL_INFO_START,
    MTK_HOT_PIXEL_INFO_END,

    MTK_JPEG_GPS_COORDINATES   = MTK_JPEG_START,
    MTK_JPEG_GPS_PROCESSING_METHOD,
    MTK_JPEG_GPS_TIMESTAMP,
    MTK_JPEG_ORIENTATION,
    MTK_JPEG_QUALITY,
    MTK_JPEG_THUMBNAIL_QUALITY,
    MTK_JPEG_THUMBNAIL_SIZE,
    MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES,
    MTK_JPEG_MAX_SIZE,
    MTK_JPEG_SIZE,
    MTK_JPEG_END,

    MTK_LENS_APERTURE          = MTK_LENS_START,
    MTK_LENS_FILTER_DENSITY,
    MTK_LENS_FOCAL_LENGTH,
    MTK_LENS_FOCUS_DISTANCE,
    MTK_LENS_OPTICAL_STABILIZATION_MODE,
    MTK_LENS_OPTICAL_AXIS_ANGLE,
    MTK_LENS_POSITION,
    MTK_LENS_FOCUS_RANGE,
    MTK_LENS_STATE,
    MTK_LENS_END,

    MTK_LENS_INFO_AVAILABLE_APERTURES
                                   = MTK_LENS_INFO_START,
    MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES,
    MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS,
    MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION,
    MTK_LENS_INFO_GEOMETRIC_CORRECTION_MAP,
    MTK_LENS_INFO_GEOMETRIC_CORRECTION_MAP_SIZE,
    MTK_LENS_INFO_HYPERFOCAL_DISTANCE,
    MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE,
    MTK_LENS_INFO_SHADING_MAP_SIZE,
    MTK_LENS_INFO_END,

    MTK_NOISE_REDUCTION_MODE   = MTK_NOISE_REDUCTION_START,
    MTK_NOISE_REDUCTION_STRENGTH,
    MTK_NOISE_REDUCTION_END,

    MTK_QUIRKS_METERING_CROP_REGION
                                   = MTK_QUIRKS_START,
    MTK_QUIRKS_TRIGGER_AF_WITH_AUTO,
    MTK_QUIRKS_USE_ZSL_FORMAT,
    MTK_QUIRKS_END,

    MTK_REQUEST_FRAME_COUNT    = MTK_REQUEST_START,
    MTK_REQUEST_ID,
    MTK_REQUEST_INPUT_STREAMS,
    MTK_REQUEST_METADATA_MODE,
    MTK_REQUEST_OUTPUT_STREAMS,
    MTK_REQUEST_TYPE,
    MTK_REQUEST_MAX_NUM_OUTPUT_STREAMS,
    MTK_REQUEST_MAX_NUM_REPROCESS_STREAMS,
    MTK_REQUEST_END,

    MTK_SCALER_CROP_REGION     = MTK_SCALER_START,
    MTK_SCALER_AVAILABLE_FORMATS,
    MTK_SCALER_AVAILABLE_JPEG_MIN_DURATIONS,
    MTK_SCALER_AVAILABLE_JPEG_SIZES,
    MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM,
    MTK_SCALER_AVAILABLE_PROCESSED_MIN_DURATIONS,
    MTK_SCALER_AVAILABLE_PROCESSED_SIZES,
    MTK_SCALER_AVAILABLE_RAW_MIN_DURATIONS,
    MTK_SCALER_AVAILABLE_RAW_SIZES,
    MTK_SCALER_END,

    MTK_IOPIPE_INFO_AVAILABLE_IN_PORT_INFO
                               = MTK_IOPIPE_INFO_START, 
    MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO, 
    MTK_IOPIPE_INFO_PORT_ID, 
    MTK_IOPIPE_INFO_TRANSFORM, 
    MTK_IOPIPE_INFO_CROP,
    MTK_IOPIPE_INFO_SCALE_DOWN_RATIO, 
    MTK_IOPIPE_INFO_SCALE_UP_RATIO,     
    MTK_IOPIPE_INFO_LINEBUFFER,    
    MTK_IOPIPE_INFO_AVAILABLE_FORMATS, 
    MTK_IOPIPE_INFO_END, 

    MTK_SENSOR_EXPOSURE_TIME   = MTK_SENSOR_START,
    MTK_SENSOR_FRAME_DURATION,
    MTK_SENSOR_SENSITIVITY,
    MTK_SENSOR_BASE_GAIN_FACTOR,
    MTK_SENSOR_BLACK_LEVEL_PATTERN,
    MTK_SENSOR_CALIBRATION_TRANSFORM1,
    MTK_SENSOR_CALIBRATION_TRANSFORM2,
    MTK_SENSOR_COLOR_TRANSFORM1,
    MTK_SENSOR_COLOR_TRANSFORM2,
    MTK_SENSOR_FORWARD_MATRIX1,
    MTK_SENSOR_FORWARD_MATRIX2,
    MTK_SENSOR_MAX_ANALOG_SENSITIVITY,
    MTK_SENSOR_NOISE_MODEL_COEFFICIENTS,
    MTK_SENSOR_REFERENCE_ILLUMINANT1,
    MTK_SENSOR_REFERENCE_ILLUMINANT2,
    MTK_SENSOR_TIMESTAMP,
    MTK_SENSOR_TEMPERATURE,    
    MTK_SENSOR_END,

    MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION
                               = MTK_SENSOR_INFO_START,
    MTK_SENSOR_INFO_SENSITIVITY_RANGE ,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT,
    MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE,
    MTK_SENSOR_INFO_MAX_FRAME_DURATION,
    MTK_SENSOR_INFO_PHYSICAL_SIZE,
    MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE,
    MTK_SENSOR_INFO_WHITE_LEVEL,
    MTK_SENSOR_INFO_ORIENTATION,
    MTK_SENSOR_INFO_FACING, 
    MTK_SENSOR_INFO_PACKAGE,
    MTK_SENSOR_INFO_DEV,
    MTK_SENSOR_INFO_SCENARIO_ID,   
    MTK_SENSOR_INFO_FRAME_RATE,
    MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, 
    MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, 
    MTK_SENSOR_INFO_END,

    MTK_SHADING_MODE           = MTK_SHADING_START,
    MTK_SHADING_STRENGTH,
    MTK_SHADING_END,

    MTK_STATISTICS_FACE_DETECT_MODE
                                   = MTK_STATISTICS_START,
    MTK_STATISTICS_HISTOGRAM_MODE,
    MTK_STATISTICS_SHARPNESS_MAP_MODE,
    MTK_STATISTICS_FACE_IDS,
    MTK_STATISTICS_FACE_LANDMARKS,
    MTK_STATISTICS_FACE_RECTANGLES,
    MTK_STATISTICS_FACE_SCORES,
    MTK_STATISTICS_HISTOGRAM,
    MTK_STATISTICS_SHARPNESS_MAP,
    MTK_STATISTICS_LENS_SHADING_MAP,
    MTK_STATISTICS_PREDICTED_COLOR_GAINS,
    MTK_STATISTICS_PREDICTED_COLOR_TRANSFORM,
    MTK_STATISTICS_SCENE_FLICKER,
    MTK_STATISTICS_LENS_SHADING_MAP_MODE,    
    MTK_STATISTICS_END,

    MTK_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES
                                   = MTK_STATISTICS_INFO_START,
    MTK_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT,
    MTK_STATISTICS_INFO_MAX_FACE_COUNT,
    MTK_STATISTICS_INFO_MAX_HISTOGRAM_COUNT,
    MTK_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE,
    MTK_STATISTICS_INFO_SHARPNESS_MAP_SIZE,
    MTK_STATISTICS_INFO_END,

    MTK_TONEMAP_CURVE_BLUE     = MTK_TONEMAP_START,
    MTK_TONEMAP_CURVE_GREEN,
    MTK_TONEMAP_CURVE_RED,
    MTK_TONEMAP_MODE,
    MTK_TONEMAP_MAX_CURVE_POINTS,
    MTK_TONEMAP_END,

    MTK_LED_TRANSMIT           = MTK_LED_START,
    MTK_LED_AVAILABLE_LEDS,
    MTK_LED_END,
    
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL = MTK_INFO_START,
    MTK_INFO_END,

    MTK_BLACK_LEVEL_LOCK       = MTK_BLACK_LEVEL_START,
    MTK_BLACK_LEVEL_END,

} mtk_camera_metadata_tag_t;

/**
 * Enumeration definitions for the various entries that need them
 */

// MTK_COLOR_CORRECTION_MODE
typedef enum mtk_camera_metadata_enum_android_color_correction_mode {
    MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX,
    MTK_COLOR_CORRECTION_MODE_FAST,
    MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_color_correction_mode_t;

// MTK_CONTROL_AE_ANTIBANDING_MODE
typedef enum mtk_camera_metadata_enum_android_control_ae_antibanding_mode {
    MTK_CONTROL_AE_ANTIBANDING_MODE_OFF,
    MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ,
    MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ,
    MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO,
} mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t;

// MTK_CONTROL_AE_LOCK
typedef enum mtk_camera_metadata_enum_android_control_ae_lock {
    MTK_CONTROL_AE_LOCK_OFF,
    MTK_CONTROL_AE_LOCK_ON,
} mtk_camera_metadata_enum_android_control_ae_lock_t;

// MTK_CONTROL_AE_MODE
typedef enum mtk_camera_metadata_enum_android_control_ae_mode {
    MTK_CONTROL_AE_MODE_OFF,
    MTK_CONTROL_AE_MODE_ON,
    MTK_CONTROL_AE_MODE_ON_AUTO_FLASH,
    MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH,
    MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE,
} mtk_camera_metadata_enum_android_control_ae_mode_t;

// MTK_CONTROL_AE_PRECAPTURE_TRIGGER
typedef enum mtk_camera_metadata_enum_android_control_ae_precapture_trigger {
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE,
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START,
} mtk_camera_metadata_enum_android_control_ae_precapture_trigger_t;

// MTK_CONTROL_AF_MODE
typedef enum mtk_camera_metadata_enum_android_control_af_mode {
    MTK_CONTROL_AF_MODE_OFF,
    MTK_CONTROL_AF_MODE_AUTO,
    MTK_CONTROL_AF_MODE_MACRO,
    MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO,
    MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
    MTK_CONTROL_AF_MODE_EDOF,
} mtk_camera_metadata_enum_android_control_af_mode_t;

// MTK_CONTROL_AF_TRIGGER
typedef enum mtk_camera_metadata_enum_android_control_af_trigger {
    MTK_CONTROL_AF_TRIGGER_IDLE,
    MTK_CONTROL_AF_TRIGGER_START,
    MTK_CONTROL_AF_TRIGGER_CANCEL,
} mtk_camera_metadata_enum_android_control_af_trigger_t;

// MTK_CONTROL_AWB_LOCK
typedef enum mtk_camera_metadata_enum_android_control_awb_lock {
    MTK_CONTROL_AWB_LOCK_OFF,
    MTK_CONTROL_AWB_LOCK_ON,
} mtk_camera_metadata_enum_android_control_awb_lock_t;

// MTK_CONTROL_AWB_MODE
typedef enum mtk_camera_metadata_enum_android_control_awb_mode {
    MTK_CONTROL_AWB_MODE_OFF,
    MTK_CONTROL_AWB_MODE_AUTO,
    MTK_CONTROL_AWB_MODE_INCANDESCENT,
    MTK_CONTROL_AWB_MODE_FLUORESCENT,
    MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT,
    MTK_CONTROL_AWB_MODE_DAYLIGHT,
    MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT,
    MTK_CONTROL_AWB_MODE_TWILIGHT,
    MTK_CONTROL_AWB_MODE_SHADE,
} mtk_camera_metadata_enum_android_control_awb_mode_t;

// MTK_CONTROL_CAPTURE_INTENT
typedef enum mtk_camera_metadata_enum_android_control_capture_intent {
    MTK_CONTROL_CAPTURE_INTENT_CUSTOM,
    MTK_CONTROL_CAPTURE_INTENT_PREVIEW,
    MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE,
    MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD,
    MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT,
    MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG,
} mtk_camera_metadata_enum_android_control_capture_intent_t;

// MTK_CONTROL_EFFECT_MODE
typedef enum mtk_camera_metadata_enum_android_control_effect_mode {
    MTK_CONTROL_EFFECT_MODE_OFF,
    MTK_CONTROL_EFFECT_MODE_MONO,
    MTK_CONTROL_EFFECT_MODE_NEGATIVE,
    MTK_CONTROL_EFFECT_MODE_SOLARIZE,
    MTK_CONTROL_EFFECT_MODE_SEPIA,
    MTK_CONTROL_EFFECT_MODE_POSTERIZE,
    MTK_CONTROL_EFFECT_MODE_WHITEBOARD,
    MTK_CONTROL_EFFECT_MODE_BLACKBOARD,
    MTK_CONTROL_EFFECT_MODE_AQUA,
} mtk_camera_metadata_enum_android_control_effect_mode_t;

// MTK_CONTROL_MODE
typedef enum mtk_camera_metadata_enum_android_control_mode {
    MTK_CONTROL_MODE_OFF,
    MTK_CONTROL_MODE_AUTO,
    MTK_CONTROL_MODE_USE_SCENE_MODE,
} mtk_camera_metadata_enum_android_control_mode_t;

// MTK_CONTROL_SCENE_MODE
typedef enum mtk_camera_metadata_enum_android_control_scene_mode {
    MTK_CONTROL_SCENE_MODE_UNSUPPORTED                      = 0,
    MTK_CONTROL_SCENE_MODE_FACE_PRIORITY,
    MTK_CONTROL_SCENE_MODE_ACTION,
    MTK_CONTROL_SCENE_MODE_PORTRAIT,
    MTK_CONTROL_SCENE_MODE_LANDSCAPE,
    MTK_CONTROL_SCENE_MODE_NIGHT,
    MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
    MTK_CONTROL_SCENE_MODE_THEATRE,
    MTK_CONTROL_SCENE_MODE_BEACH,
    MTK_CONTROL_SCENE_MODE_SNOW,
    MTK_CONTROL_SCENE_MODE_SUNSET,
    MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
    MTK_CONTROL_SCENE_MODE_FIREWORKS,
    MTK_CONTROL_SCENE_MODE_SPORTS,
    MTK_CONTROL_SCENE_MODE_PARTY,
    MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
    MTK_CONTROL_SCENE_MODE_BARCODE,
} mtk_camera_metadata_enum_android_control_scene_mode_t;

// MTK_CONTROL_VIDEO_STABILIZATION_MODE
typedef enum mtk_camera_metadata_enum_android_control_video_stabilization_mode {
    MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF,
    MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON,
} mtk_camera_metadata_enum_android_control_video_stabilization_mode_t;

// MTK_CONTROL_AE_STATE
typedef enum mtk_camera_metadata_enum_android_control_ae_state {
    MTK_CONTROL_AE_STATE_INACTIVE,
    MTK_CONTROL_AE_STATE_SEARCHING,
    MTK_CONTROL_AE_STATE_CONVERGED,
    MTK_CONTROL_AE_STATE_LOCKED,
    MTK_CONTROL_AE_STATE_FLASH_REQUIRED,
    MTK_CONTROL_AE_STATE_PRECAPTURE,
} mtk_camera_metadata_enum_android_control_ae_state_t;

// MTK_CONTROL_AF_STATE
typedef enum mtk_camera_metadata_enum_android_control_af_state {
    MTK_CONTROL_AF_STATE_INACTIVE,
    MTK_CONTROL_AF_STATE_PASSIVE_SCAN,
    MTK_CONTROL_AF_STATE_PASSIVE_FOCUSED,
    MTK_CONTROL_AF_STATE_ACTIVE_SCAN,
    MTK_CONTROL_AF_STATE_FOCUSED_LOCKED,
    MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED,
} mtk_camera_metadata_enum_android_control_af_state_t;

// MTK_CONTROL_AWB_STATE
typedef enum mtk_camera_metadata_enum_android_control_awb_state {
    MTK_CONTROL_AWB_STATE_INACTIVE,
    MTK_CONTROL_AWB_STATE_SEARCHING,
    MTK_CONTROL_AWB_STATE_CONVERGED,
    MTK_CONTROL_AWB_STATE_LOCKED,
} mtk_camera_metadata_enum_android_control_awb_state_t;


// MTK_DEMOSAIC_MODE
typedef enum mtk_camera_metadata_enum_android_demosaic_mode {
    MTK_DEMOSAIC_MODE_FAST,
    MTK_DEMOSAIC_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_demosaic_mode_t;


// MTK_EDGE_MODE
typedef enum mtk_camera_metadata_enum_android_edge_mode {
    MTK_EDGE_MODE_OFF,
    MTK_EDGE_MODE_FAST,
    MTK_EDGE_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_edge_mode_t;


// MTK_FLASH_MODE
typedef enum mtk_camera_metadata_enum_android_flash_mode {
    MTK_FLASH_MODE_OFF,
    MTK_FLASH_MODE_SINGLE,
    MTK_FLASH_MODE_TORCH,
} mtk_camera_metadata_enum_android_flash_mode_t;

// MTK_FLASH_STATE
typedef enum mtk_camera_metadata_enum_android_flash_state {
    MTK_FLASH_STATE_UNAVAILABLE,
    MTK_FLASH_STATE_CHARGING,
    MTK_FLASH_STATE_READY,
    MTK_FLASH_STATE_FIRED,
} mtk_camera_metadata_enum_android_flash_state_t;



// MTK_GEOMETRIC_MODE
typedef enum mtk_camera_metadata_enum_android_geometric_mode {
    MTK_GEOMETRIC_MODE_OFF,
    MTK_GEOMETRIC_MODE_FAST,
    MTK_GEOMETRIC_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_geometric_mode_t;


// MTK_HOT_PIXEL_MODE
typedef enum mtk_camera_metadata_enum_android_hot_pixel_mode {
    MTK_HOT_PIXEL_MODE_OFF,
    MTK_HOT_PIXEL_MODE_FAST,
    MTK_HOT_PIXEL_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_hot_pixel_mode_t;




// MTK_LENS_OPTICAL_STABILIZATION_MODE
typedef enum mtk_camera_metadata_enum_android_lens_optical_stabilization_mode {
    MTK_LENS_OPTICAL_STABILIZATION_MODE_OFF,
    MTK_LENS_OPTICAL_STABILIZATION_MODE_ON,
} mtk_camera_metadata_enum_android_lens_optical_stabilization_mode_t;

// MTK_LENS_FACING
typedef enum mtk_camera_metadata_enum_android_lens_facing {
    MTK_LENS_FACING_FRONT,
    MTK_LENS_FACING_BACK,
} mtk_camera_metadata_enum_android_lens_facing_t;

// MTK_LENS_STATE
typedef enum mtk_camera_metadata_enum_android_lens_state {
    MTK_LENS_STATE_STATIONARY,
} mtk_camera_metadata_enum_android_lens_state_t;



// MTK_NOISE_REDUCTION_MODE
typedef enum mtk_camera_metadata_enum_android_noise_reduction_mode {
    MTK_NOISE_REDUCTION_MODE_OFF,
    MTK_NOISE_REDUCTION_MODE_FAST,
    MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_noise_reduction_mode_t;



// MTK_REQUEST_METADATA_MODE
typedef enum mtk_camera_metadata_enum_android_request_metadata_mode {
    MTK_REQUEST_METADATA_MODE_NONE,
    MTK_REQUEST_METADATA_MODE_FULL,
} mtk_camera_metadata_enum_android_request_metadata_mode_t;

// MTK_REQUEST_TYPE
typedef enum mtk_camera_metadata_enum_android_request_type {
    MTK_REQUEST_TYPE_CAPTURE,
    MTK_REQUEST_TYPE_REPROCESS,
} mtk_camera_metadata_enum_android_request_type_t;

// MTK_IOPIPE_INFO_CROP
typedef enum mtk_camera_metadata_enum_android_iopipe_info_crop {
    MTK_IOPIPE_INFO_CROP_NOT_SUPPORT, 
    MTK_IOPIPE_INFO_CROP_SYMMETRIC, 
    MTK_IOPIPE_INFO_CROP_ASYMMETRIC, 
} mtk_camera_metadata_enum_android_iopipe_info_crop_t;

// MTK_SENSOR_REFERENCE_ILLUMINANT1
typedef enum mtk_camera_metadata_enum_android_sensor_reference_illuminant1 {
    MTK_SENSOR_REFERENCE_ILLUMINANT1_DAYLIGHT               ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_FLUORESCENT            ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_TUNGSTEN               ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_FLASH                  ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_FINE_WEATHER           ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_CLOUDY_WEATHER         ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_SHADE                  ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_DAYLIGHT_FLUORESCENT   ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_DAY_WHITE_FLUORESCENT  ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_COOL_WHITE_FLUORESCENT ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_WHITE_FLUORESCENT      ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_A             ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_B             ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_C             ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D55                    ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D65                    ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D75                    ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D50                    ,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_ISO_STUDIO_TUNGSTEN    ,
} mtk_camera_metadata_enum_android_sensor_reference_illuminant1_t;


// MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT
typedef enum mtk_camera_metadata_enum_android_sensor_info_color_filter_arrangement {
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_RGGB,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_GRBG,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_GBRG,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_BGGR,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_RGB,
} mtk_camera_metadata_enum_android_sensor_info_color_filter_arrangement_t;

// MTK_SENSOR_INFO_SCENARIO_ID
typedef enum mtk_camera_metadata_enum_android_sensor_info_scenario_id {
    MTK_SENSOR_INFO_SCENARIO_ID_ZSD, 
    MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_PREVIEW, 
    MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_CAPTURE, 
    MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_VIDEO, 
    /**************************************************************************
     * All unnamed scenario id for a specific sensor must be started with 
     * values >= MTK_SENSOR_INFO_SCENARIO_ID_UNNAMED_START.
     **************************************************************************/
    MTK_SENSOR_INFO_SCENARIO_ID_UNNAMED_START = 0x100, 
} mtk_camera_metadata_enum_android_sensor_info_scenario_id_t;

// MTK_SHADING_MODE
typedef enum mtk_camera_metadata_enum_android_shading_mode {
    MTK_SHADING_MODE_OFF,
    MTK_SHADING_MODE_FAST,
    MTK_SHADING_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_shading_mode_t;


// MTK_STATISTICS_FACE_DETECT_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_face_detect_mode {
    MTK_STATISTICS_FACE_DETECT_MODE_OFF,
    MTK_STATISTICS_FACE_DETECT_MODE_SIMPLE,
    MTK_STATISTICS_FACE_DETECT_MODE_FULL,
} mtk_camera_metadata_enum_android_statistics_face_detect_mode_t;

// MTK_STATISTICS_HISTOGRAM_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_histogram_mode {
    MTK_STATISTICS_HISTOGRAM_MODE_OFF,
    MTK_STATISTICS_HISTOGRAM_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_histogram_mode_t;

// MTK_STATISTICS_SHARPNESS_MAP_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_sharpness_map_mode {
    MTK_STATISTICS_SHARPNESS_MAP_MODE_OFF,
    MTK_STATISTICS_SHARPNESS_MAP_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_sharpness_map_mode_t;

// ANDROID_STATISTICS_SCENE_FLICKER
typedef enum mtk_camera_metadata_enum_android_statistics_scene_flicker {
    MTK_STATISTICS_SCENE_FLICKER_NONE,
    MTK_STATISTICS_SCENE_FLICKER_50HZ,
    MTK_STATISTICS_SCENE_FLICKER_60HZ,
} mtk_camera_metadata_enum_android_statistics_scene_flicker_t;

// ANDROID_STATISTICS_LENS_SHADING_MAP_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_lens_shading_map_mode {
    MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF,
    MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_lens_shading_map_mode_t;

// MTK_TONEMAP_MODE
typedef enum mtk_camera_metadata_enum_android_tonemap_mode {
    MTK_TONEMAP_MODE_CONTRAST_CURVE,
    MTK_TONEMAP_MODE_FAST,
    MTK_TONEMAP_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_tonemap_mode_t;


// MTK_LED_TRANSMIT
typedef enum mtk_camera_metadata_enum_android_led_transmit {
    MTK_LED_TRANSMIT_OFF,
    MTK_LED_TRANSMIT_ON,
} mtk_camera_metadata_enum_android_led_transmit_t;

// MTK_LED_AVAILABLE_LEDS
typedef enum mtk_camera_metadata_enum_android_led_available_leds {
    MTK_LED_AVAILABLE_LEDS_TRANSMIT,
} mtk_camera_metadata_enum_android_led_available_leds_t;

// MTK_INFO_SUPPORTED_HARDWARE_LEVEL
typedef enum mtk_camera_metadata_enum_android_info_supported_hardware_level {
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL_LIMITED,
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL_FULL,
} mtk_camera_metadata_enum_android_info_supported_hardware_level_t;


// MTK_BLACK_LEVEL_LOCK
typedef enum mtk_camera_metadata_enum_android_black_level_lock {
    MTK_BLACK_LEVEL_LOCK_OFF,
    MTK_BLACK_LEVEL_LOCK_ON,
} mtk_camera_metadata_enum_android_black_level_lock_t;

#endif
