ifeq ($(strip $(MTK_AUTO_TEST)), yes) 
    LOCAL_PATH:= $(call my-dir)
    include $(CLEAR_VARS)

    # We only want this apk build for tests.
    LOCAL_MODULE_TAGS := tests

    LOCAL_JAVA_LIBRARIES := android.test.runner
    LOCAL_JAVA_LIBRARIES += mediatek-framework
    
    #Add for juint report
    LOCAL_STATIC_JAVA_LIBRARIES := libjunitreport-for-tests
    #End add

    # Include all test java files.
    LOCAL_SRC_FILES := $(call all-java-files-under, src)

    LOCAL_PACKAGE_NAME := DeviceRegisterTests

    LOCAL_CERTIFICATE := platform

    LOCAL_INSTRUMENTATION_FOR := DeviceRegister

    include $(BUILD_PACKAGE)

    # Use the following include to make our test apk.
    include $(call all-makefiles-under,$(LOCAL_PATH))
    
    #Add for junit report    
    include $(CLEAR_VARS)
    LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libjunitreport-for-tests:android-junit-report-1.2.6.jar
    include $(BUILD_MULTI_PREBUILT)
    #Edd add
endif
