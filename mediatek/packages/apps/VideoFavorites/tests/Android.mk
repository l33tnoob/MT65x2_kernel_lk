# Add appropriate copyright banner here
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
 
# We only want this apk build for tests.
LOCAL_MODULE_TAGS := tests
  
# Include all test java files.
LOCAL_SRC_FILES := $(call all-java-files-under, src)
   
# Notice that we don't have to include the src files of ApiDemos because, by
# running the tests using an instrumentation targeting ApiDemos, we
# automatically get all of its classes loaded into our environment.
    
LOCAL_PACKAGE_NAME := VideoFavoritesTests

LOCAL_JAVA_LIBRARIES := android.test.runner
LOCAL_STATIC_JAVA_LIBRARIES := libjunitreport robotium-solo

LOCAL_INSTRUMENTATION_FOR := VideoFavorites
      
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

##################################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libjunitreport:libs/android-junit-report-dev.jar robotium-solo:libs/robotium-solo-4.3.1.jar
include $(BUILD_MULTI_PREBUILT)
