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

//#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_INFO_INL_
//#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_INFO_INL_


/******************************************************************************
 *  
 ******************************************************************************/
#ifndef _IMP_SECTION_INFO_

    template <mtk_camera_metadata_section section> struct MetadataSectionInfo {};

    #define _IMP_SECTION_INFO_(_section_, _name_) \
        template <> struct MetadataSectionInfo<_section_> { \
            enum \
            { \
                SECTION_START = _section_##_START, \
                SECTION_END   = _section_##_END, \
            }; \
        };

#endif


/******************************************************************************
 *  
 ******************************************************************************/
_IMP_SECTION_INFO_(MTK_COLOR_CORRECTION,    "mtk.colorCorrection")
_IMP_SECTION_INFO_(MTK_CONTROL,             "mtk.control")
_IMP_SECTION_INFO_(MTK_DEMOSAIC,            "mtk.demosaic")
_IMP_SECTION_INFO_(MTK_EDGE,                "mtk.edge")
_IMP_SECTION_INFO_(MTK_FLASH,               "mtk.flash")
_IMP_SECTION_INFO_(MTK_FLASH_INFO,          "mtk.flash.info")
_IMP_SECTION_INFO_(MTK_GEOMETRIC,           "mtk.geometric")
_IMP_SECTION_INFO_(MTK_HOT_PIXEL,           "mtk.hotPixel")
_IMP_SECTION_INFO_(MTK_HOT_PIXEL_INFO,      "mtk.hotPixel.info")
_IMP_SECTION_INFO_(MTK_JPEG,                "mtk.jpeg")
_IMP_SECTION_INFO_(MTK_LENS,                "mtk.lens")
_IMP_SECTION_INFO_(MTK_LENS_INFO,           "mtk.lens.info")
_IMP_SECTION_INFO_(MTK_NOISE_REDUCTION,     "mtk.noiseReduction")
_IMP_SECTION_INFO_(MTK_QUIRKS,              "mtk.quirks")
_IMP_SECTION_INFO_(MTK_REQUEST,             "mtk.request")
_IMP_SECTION_INFO_(MTK_SCALER,              "mtk.scaler")
_IMP_SECTION_INFO_(MTK_SENSOR,              "mtk.sensor")
_IMP_SECTION_INFO_(MTK_SENSOR_INFO,         "mtk.sensor.info")
_IMP_SECTION_INFO_(MTK_SHADING,             "mtk.shading")
_IMP_SECTION_INFO_(MTK_STATISTICS,          "mtk.statistics")
_IMP_SECTION_INFO_(MTK_STATISTICS_INFO,     "mtk.statistics.info")
_IMP_SECTION_INFO_(MTK_TONEMAP,             "mtk.tonemap")
_IMP_SECTION_INFO_(MTK_LED,                 "mtk.led")
_IMP_SECTION_INFO_(MTK_INFO,                "mtk.info")
_IMP_SECTION_INFO_(MTK_BLACK_LEVEL,         "mtk.blacklevel")
_IMP_SECTION_INFO_(MTK_IOPIPE_INFO,         "mtk.iopipe.info")
#undef  _IMP_SECTION_INFO_


/******************************************************************************
 *  
 ******************************************************************************/
#ifndef _IMP_TAG_INFO_

    template <mtk_camera_metadata_tag tag> struct MetadataTagInfo {};

    #define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
        template <> struct MetadataTagInfo<_tag_> { \
            typedef _type_ type; \
        };

#endif


/******************************************************************************
 *  
 ******************************************************************************/
_IMP_TAG_INFO_( MTK_COLOR_CORRECTION_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_COLOR_CORRECTION_TRANSFORM, 
                MRational,     "transform")
_IMP_TAG_INFO_( MTK_COLOR_CORRECTION_GAINS , 
                MFLOAT,     "gains")                
//
_IMP_TAG_INFO_( MTK_CONTROL_AE_ANTIBANDING_MODE, 
                MUINT8,     "aeAntibandingMode")
_IMP_TAG_INFO_( MTK_CONTROL_AE_EXPOSURE_COMPENSATION, 
                MINT32,     "aeExposureCompensation")
_IMP_TAG_INFO_( MTK_CONTROL_AE_LOCK, 
                MUINT8,     "aeLock")
_IMP_TAG_INFO_( MTK_CONTROL_AE_MODE, 
                MUINT8,     "aeMode")
_IMP_TAG_INFO_( MTK_CONTROL_AE_REGIONS, 
                MINT32,     "aeRegions")
_IMP_TAG_INFO_( MTK_CONTROL_AE_TARGET_FPS_RANGE, 
                MINT32,     "aeTargetFpsRange")
_IMP_TAG_INFO_( MTK_CONTROL_AE_PRECAPTURE_TRIGGER, 
                MUINT8,     "aePrecaptureTrigger")
_IMP_TAG_INFO_( MTK_CONTROL_AF_MODE, 
                MUINT8,     "afMode")
_IMP_TAG_INFO_( MTK_CONTROL_AF_REGIONS, 
                MINT32,     "afRegions")
_IMP_TAG_INFO_( MTK_CONTROL_AF_TRIGGER, 
                MUINT8,     "afTrigger")
_IMP_TAG_INFO_( MTK_CONTROL_AWB_LOCK, 
                MUINT8,     "awbLock")
_IMP_TAG_INFO_( MTK_CONTROL_AWB_MODE, 
                MUINT8,     "awbMode")
_IMP_TAG_INFO_( MTK_CONTROL_AWB_REGIONS, 
                MINT32,     "awbRegions")
_IMP_TAG_INFO_( MTK_CONTROL_CAPTURE_INTENT, 
                MUINT8,     "captureIntent")
_IMP_TAG_INFO_( MTK_CONTROL_EFFECT_MODE, 
                MUINT8,     "effectMode")
_IMP_TAG_INFO_( MTK_CONTROL_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_CONTROL_SCENE_MODE, 
                MUINT8,     "sceneMode")
_IMP_TAG_INFO_( MTK_CONTROL_VIDEO_STABILIZATION_MODE, 
                MUINT8,     "videoStabilizationMode")
_IMP_TAG_INFO_( MTK_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES, 
                MUINT8,     "aeAvailableAntibandingModes")
_IMP_TAG_INFO_( MTK_CONTROL_AE_AVAILABLE_MODES, 
                MUINT8,     "aeAvailableModes")
_IMP_TAG_INFO_( MTK_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, 
                MINT32,     "aeAvailableTargetFpsRanges")
_IMP_TAG_INFO_( MTK_CONTROL_AE_COMPENSATION_RANGE, 
                MINT32,     "aeCompensationRange")
_IMP_TAG_INFO_( MTK_CONTROL_AE_COMPENSATION_STEP, 
                MRational,  "aeCompensationStep")
_IMP_TAG_INFO_( MTK_CONTROL_AF_AVAILABLE_MODES, 
                MUINT8,     "afAvailableModes")
_IMP_TAG_INFO_( MTK_CONTROL_AVAILABLE_EFFECTS, 
                MUINT8,     "availableEffects")
_IMP_TAG_INFO_( MTK_CONTROL_AVAILABLE_SCENE_MODES, 
                MUINT8,     "availableSceneModes")
_IMP_TAG_INFO_( MTK_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES, 
                MUINT8,     "availableVideoStabilizationModes")
_IMP_TAG_INFO_( MTK_CONTROL_AWB_AVAILABLE_MODES, 
                MUINT8,     "awbAvailableModes")
_IMP_TAG_INFO_( MTK_CONTROL_MAX_REGIONS, 
                MINT32,     "maxRegions")
_IMP_TAG_INFO_( MTK_CONTROL_SCENE_MODE_OVERRIDES, 
                MUINT8,     "sceneModeOverrides")
_IMP_TAG_INFO_( MTK_CONTROL_AE_PRECAPTURE_ID, 
                MINT32,     "aePrecaptureId")
_IMP_TAG_INFO_( MTK_CONTROL_AE_STATE, 
                MUINT8,     "aeState")
_IMP_TAG_INFO_( MTK_CONTROL_AF_STATE, 
                MUINT8,     "afState")
_IMP_TAG_INFO_( MTK_CONTROL_AF_TRIGGER_ID, 
                MINT32,     "afTriggerId")
_IMP_TAG_INFO_( MTK_CONTROL_AWB_STATE, 
                MUINT8,     "awbState")
//
_IMP_TAG_INFO_( MTK_DEMOSAIC_MODE, 
                MUINT8,     "mode")
//
_IMP_TAG_INFO_( MTK_EDGE_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_EDGE_STRENGTH, 
                MUINT8,     "strength")
//
_IMP_TAG_INFO_( MTK_FLASH_FIRING_POWER, 
                MUINT8,     "firingPower")
_IMP_TAG_INFO_( MTK_FLASH_FIRING_TIME, 
                MINT64,     "firingTime")
_IMP_TAG_INFO_( MTK_FLASH_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_FLASH_COLOR_TEMPERATURE, 
                MUINT8,     "colorTemperature")
_IMP_TAG_INFO_( MTK_FLASH_MAX_ENERGY, 
                MUINT8,     "maxEnergy")
_IMP_TAG_INFO_( MTK_FLASH_STATE, 
                MUINT8,     "state")
//
_IMP_TAG_INFO_( MTK_FLASH_INFO_AVAILABLE, 
                MUINT8,     "available")
_IMP_TAG_INFO_( MTK_FLASH_INFO_CHARGE_DURATION, 
                MINT64,     "chargeDuration")
//
_IMP_TAG_INFO_( MTK_GEOMETRIC_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_GEOMETRIC_STRENGTH, 
                MUINT8,     "strength")
//
_IMP_TAG_INFO_( MTK_HOT_PIXEL_MODE, 
                MUINT8,     "mode")
//
_IMP_TAG_INFO_( MTK_HOT_PIXEL_INFO_MAP, 
                MINT32,     "map")
//
_IMP_TAG_INFO_( MTK_JPEG_GPS_COORDINATES, 
                MDOUBLE,    "gpsCoordinates")
_IMP_TAG_INFO_( MTK_JPEG_GPS_PROCESSING_METHOD, 
                MUINT8,     "gpsProcessingMethod")
_IMP_TAG_INFO_( MTK_JPEG_GPS_TIMESTAMP, 
                MINT64,     "gpsTimestamp")
_IMP_TAG_INFO_( MTK_JPEG_ORIENTATION, 
                MINT32,     "orientation")
_IMP_TAG_INFO_( MTK_JPEG_QUALITY, 
                MUINT8,     "quality")
_IMP_TAG_INFO_( MTK_JPEG_THUMBNAIL_QUALITY, 
                MUINT8,     "thumbnailQuality")
_IMP_TAG_INFO_( MTK_JPEG_THUMBNAIL_SIZE, 
                MSize,      "thumbnailSize")
_IMP_TAG_INFO_( MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES, 
                MSize,      "availableThumbnailSizes")
_IMP_TAG_INFO_( MTK_JPEG_MAX_SIZE, 
                MINT32,      "maxSize")
_IMP_TAG_INFO_( MTK_JPEG_SIZE, 
                MSize,      "size")
//
_IMP_TAG_INFO_( MTK_LENS_APERTURE, 
                MFLOAT,     "aperture")
_IMP_TAG_INFO_( MTK_LENS_FILTER_DENSITY, 
                MFLOAT,     "filterDensity")
_IMP_TAG_INFO_( MTK_LENS_FOCAL_LENGTH, 
                MFLOAT,     "focalLength")
_IMP_TAG_INFO_( MTK_LENS_FOCUS_DISTANCE, 
                MFLOAT,     "focusDistance")
_IMP_TAG_INFO_( MTK_LENS_OPTICAL_STABILIZATION_MODE, 
                MUINT8,     "opticalStabilizationMode")
_IMP_TAG_INFO_( MTK_LENS_OPTICAL_AXIS_ANGLE, 
                MFLOAT,     "opticalAxisAngle")
_IMP_TAG_INFO_( MTK_LENS_POSITION, 
                MFLOAT,     "position")
_IMP_TAG_INFO_( MTK_LENS_FOCUS_RANGE, 
                MFLOAT,     "focusRange")
_IMP_TAG_INFO_( MTK_LENS_STATE, 
                MUINT8,     "state")
//
_IMP_TAG_INFO_( MTK_LENS_INFO_AVAILABLE_APERTURES, 
                MFLOAT,     "availableApertures")
_IMP_TAG_INFO_( MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES, 
                MFLOAT,     "availableFilterDensities")
_IMP_TAG_INFO_( MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, 
                MFLOAT,     "availableFocalLengths")
_IMP_TAG_INFO_( MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION, 
                MUINT8,     "availableOpticalStabilization")
_IMP_TAG_INFO_( MTK_LENS_INFO_GEOMETRIC_CORRECTION_MAP, 
                MFLOAT,     "geometricCorrectionMap")
_IMP_TAG_INFO_( MTK_LENS_INFO_GEOMETRIC_CORRECTION_MAP_SIZE, 
                MINT32,     "geometricCorrectionMapSize")
_IMP_TAG_INFO_( MTK_LENS_INFO_HYPERFOCAL_DISTANCE, 
                MFLOAT,     "hyperfocalDistance")
_IMP_TAG_INFO_( MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE, 
                MFLOAT,     "minimumFocusDistance")
_IMP_TAG_INFO_( MTK_LENS_INFO_SHADING_MAP_SIZE, 
                MSize,      "shadingMapSize")
//
_IMP_TAG_INFO_( MTK_NOISE_REDUCTION_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_NOISE_REDUCTION_STRENGTH, 
                MUINT8,     "strength")
//
_IMP_TAG_INFO_( MTK_QUIRKS_METERING_CROP_REGION, 
                MRect,      "meteringCropRegion")
_IMP_TAG_INFO_( MTK_QUIRKS_TRIGGER_AF_WITH_AUTO, 
                MUINT8,     "triggerAfWithAuto")
_IMP_TAG_INFO_( MTK_QUIRKS_USE_ZSL_FORMAT, 
                MUINT8,     "useZslFormat")
//
_IMP_TAG_INFO_( MTK_REQUEST_FRAME_COUNT, 
                MINT32,     "frameCount")
_IMP_TAG_INFO_( MTK_REQUEST_ID, 
                MINT32,     "id")
_IMP_TAG_INFO_( MTK_REQUEST_INPUT_STREAMS, 
                MINT32,     "inputStreams")
_IMP_TAG_INFO_( MTK_REQUEST_METADATA_MODE, 
                MUINT8,     "metadataMode")
_IMP_TAG_INFO_( MTK_REQUEST_OUTPUT_STREAMS, 
                MINT32,     "outputStreams")
_IMP_TAG_INFO_( MTK_REQUEST_TYPE, 
                MUINT8,     "type")
_IMP_TAG_INFO_( MTK_REQUEST_MAX_NUM_OUTPUT_STREAMS, 
                MINT32,     "maxNumOutputStreams")
_IMP_TAG_INFO_( MTK_REQUEST_MAX_NUM_REPROCESS_STREAMS, 
                MINT32,     "maxNumReprocessStreams")
//
_IMP_TAG_INFO_( MTK_SCALER_CROP_REGION, 
                MRect,      "cropRegion")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_FORMATS, 
                MINT32,     "availableFormats")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_JPEG_MIN_DURATIONS, 
                MINT64,     "availableJpegMinDurations")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_JPEG_SIZES, 
                MINT32,     "availableJpegSizes")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM, 
                MFLOAT,     "availableMaxDigitalZoom")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_PROCESSED_MIN_DURATIONS, 
                MINT64,     "availableProcessedMinDurations")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_PROCESSED_SIZES, 
                MSize,      "availableProcessedSizes")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_RAW_MIN_DURATIONS, 
                MINT64,     "availableRawMinDurations")
_IMP_TAG_INFO_( MTK_SCALER_AVAILABLE_RAW_SIZES, 
                MSize,      "availableRawSizes")
//
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_AVAILABLE_IN_PORT_INFO, 
                IMetadata,  "availableInPortInfo")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO, 
                IMetadata,  "availableOutPortInfo")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_PORT_ID, 
                MINT32,     "portId")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_TRANSFORM, 
                MINT32,     "transform")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_CROP, 
                MINT32,     "crop")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_SCALE_DOWN_RATIO, 
                MFLOAT,     "scale_down")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_SCALE_UP_RATIO, 
                MFLOAT,     "scale_up")                
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_LINEBUFFER, 
                MINT32,     "linebuffer")
_IMP_TAG_INFO_( MTK_IOPIPE_INFO_AVAILABLE_FORMATS, 
                MINT32,     "availableFormats")
//
_IMP_TAG_INFO_( MTK_SENSOR_EXPOSURE_TIME, 
                MINT64,     "exposureTime")
_IMP_TAG_INFO_( MTK_SENSOR_FRAME_DURATION, 
                MINT64,     "frameDuration")
_IMP_TAG_INFO_( MTK_SENSOR_SENSITIVITY, 
                MINT32,     "sensitivity")
_IMP_TAG_INFO_( MTK_SENSOR_BASE_GAIN_FACTOR, 
                MRational,  "baseGainFactor")
_IMP_TAG_INFO_( MTK_SENSOR_BLACK_LEVEL_PATTERN, 
                MINT32,     "blackLevelPattern")
_IMP_TAG_INFO_( MTK_SENSOR_CALIBRATION_TRANSFORM1, 
                MRational,  "calibrationTransform1")
_IMP_TAG_INFO_( MTK_SENSOR_CALIBRATION_TRANSFORM2, 
                MRational,  "calibrationTransform2")
_IMP_TAG_INFO_( MTK_SENSOR_COLOR_TRANSFORM1, 
                MRational,  "colorTransform1")
_IMP_TAG_INFO_( MTK_SENSOR_COLOR_TRANSFORM2, 
                MRational,  "colorTransform2")
_IMP_TAG_INFO_( MTK_SENSOR_FORWARD_MATRIX1, 
                MRational,  "forwardMatrix1")
_IMP_TAG_INFO_( MTK_SENSOR_FORWARD_MATRIX2, 
                MRational,  "forwardMatrix2")
_IMP_TAG_INFO_( MTK_SENSOR_MAX_ANALOG_SENSITIVITY, 
                MINT32,     "maxAnalogSensitivity")
_IMP_TAG_INFO_( MTK_SENSOR_NOISE_MODEL_COEFFICIENTS, 
                MFLOAT,     "noiseModelCoefficients")
_IMP_TAG_INFO_( MTK_SENSOR_REFERENCE_ILLUMINANT1, 
                MUINT8,     "referenceIlluminant1")
_IMP_TAG_INFO_( MTK_SENSOR_REFERENCE_ILLUMINANT2, 
                MUINT8,     "referenceIlluminant2")
_IMP_TAG_INFO_( MTK_SENSOR_TIMESTAMP, 
                MINT64,     "timestamp")
_IMP_TAG_INFO_( MTK_SENSOR_TEMPERATURE , 
                MFLOAT,     "temperature")                
//
_IMP_TAG_INFO_( MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, 
                MRect,      "activeArrayRegion")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_SENSITIVITY_RANGE, 
                MINT32,     "sensitivityRange")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, 
                MUINT8,     "colorFilterArrangement")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE, 
                MINT64,     "exposureTimeRange")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_MAX_FRAME_DURATION, 
                MINT64,     "maxFrameDuration")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_PHYSICAL_SIZE, 
                MFLOAT,     "physicalSize")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, 
                MSize,      "pixelArraySize")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_WHITE_LEVEL, 
                MINT32,     "whiteLevel")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_ORIENTATION, 
                MINT32,     "orientation")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_FACING, 
                MUINT8,     "facing")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_PACKAGE, 
                IMetadata,  "package")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_DEV, 
                MINT32,     "sensorDev")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_SCENARIO_ID, 
                MINT32,     "scenarioID")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_FRAME_RATE, 
                MINT32,     "frameRate")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, 
                MSize,      "realOutputSize")
_IMP_TAG_INFO_( MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, 
                MRect,      "outputRegionOnActiveArray")
//
_IMP_TAG_INFO_( MTK_SHADING_MODE, 
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_SHADING_STRENGTH, 
                MUINT8,     "strength")
//
_IMP_TAG_INFO_( MTK_STATISTICS_FACE_DETECT_MODE, 
                MUINT8,     "faceDetectMode")
_IMP_TAG_INFO_( MTK_STATISTICS_HISTOGRAM_MODE, 
                MUINT8,     "histogramMode")
_IMP_TAG_INFO_( MTK_STATISTICS_SHARPNESS_MAP_MODE, 
                MUINT8,     "sharpnessMapMode")
_IMP_TAG_INFO_( MTK_STATISTICS_FACE_IDS, 
                MINT32,     "faceIds")
_IMP_TAG_INFO_( MTK_STATISTICS_FACE_LANDMARKS, 
                MINT32,     "faceLandmarks")
_IMP_TAG_INFO_( MTK_STATISTICS_FACE_RECTANGLES, 
                MRect,      "faceRectangles")
_IMP_TAG_INFO_( MTK_STATISTICS_FACE_SCORES, 
                MUINT8,     "faceScores")
_IMP_TAG_INFO_( MTK_STATISTICS_HISTOGRAM, 
                MINT32,     "histogram")
_IMP_TAG_INFO_( MTK_STATISTICS_SHARPNESS_MAP, 
                MINT32,     "sharpnessMap")
//
_IMP_TAG_INFO_( MTK_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES,
                MUINT8,     "availableFaceDetectModes")
_IMP_TAG_INFO_( MTK_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT,
                MINT32,     "histogramBucketCount")
_IMP_TAG_INFO_( MTK_STATISTICS_INFO_MAX_FACE_COUNT,
                MINT32,     "maxFaceCount")
_IMP_TAG_INFO_( MTK_STATISTICS_INFO_MAX_HISTOGRAM_COUNT,
                MINT32,     "maxHistogramCount")
_IMP_TAG_INFO_( MTK_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE,
                MINT32,     "maxSharpnessMapValue")
_IMP_TAG_INFO_( MTK_STATISTICS_INFO_SHARPNESS_MAP_SIZE,
                MSize,      "sharpnessMapSize")
_IMP_TAG_INFO_( MTK_STATISTICS_LENS_SHADING_MAP,
                MFLOAT,     "lensShadingMap")
_IMP_TAG_INFO_( MTK_STATISTICS_PREDICTED_COLOR_GAINS,
                MFLOAT,     "predictedColorGains")
_IMP_TAG_INFO_( MTK_STATISTICS_PREDICTED_COLOR_TRANSFORM,
                MRational,  "predictedColorTransform")
_IMP_TAG_INFO_( MTK_STATISTICS_SCENE_FLICKER,
                MUINT8,     "sceneFlicker")
_IMP_TAG_INFO_( MTK_STATISTICS_LENS_SHADING_MAP_MODE,
                MUINT8,     "lensShadingMapMode")            
//
_IMP_TAG_INFO_( MTK_TONEMAP_CURVE_BLUE, 
                MFLOAT,     "curveBlue")
_IMP_TAG_INFO_( MTK_TONEMAP_CURVE_GREEN,
                MFLOAT,     "curveGreen")
_IMP_TAG_INFO_( MTK_TONEMAP_CURVE_RED,
                MFLOAT,     "curveRed")
_IMP_TAG_INFO_( MTK_TONEMAP_MODE,
                MUINT8,     "mode")
_IMP_TAG_INFO_( MTK_TONEMAP_MAX_CURVE_POINTS,
                MINT32,     "maxCurvePoints")
//
_IMP_TAG_INFO_( MTK_LED_TRANSMIT, 
                MUINT8,     "transmit")
_IMP_TAG_INFO_( MTK_LED_AVAILABLE_LEDS,
                MUINT8,     "availableLeds")
//
_IMP_TAG_INFO_( MTK_INFO_SUPPORTED_HARDWARE_LEVEL, 
                MUINT8,     "supportedHardwareLevel")
//
_IMP_TAG_INFO_( MTK_BLACK_LEVEL_LOCK,
                MUINT8,     "lock")
//               
#undef  _IMP_TAG_INFO_


/******************************************************************************
 *  
 ******************************************************************************/
//#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_INFO_INL_
