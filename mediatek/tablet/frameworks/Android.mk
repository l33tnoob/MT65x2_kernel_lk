# limitations under the License.
#

# This makefile shows how to build your own shared library that can be
# shipped on the system of a phone, and included additional examples of
# including JNI code with the library and writing client applications against it.

ifeq ($(strip $(MTK_TABLET_PLUGIN_BUILD)),yes)

    LOCAL_PATH := $(call my-dir)

    # MediaTek tablet library.
    # =============================================================
    include $(CLEAR_VARS)

    LOCAL_MODULE_TAGS := optional

    LOCAL_MODULE := mediatek-tablet

    LOCAL_SRC_FILES := $(call all-java-files-under,java)

    LOCAL_JAVA_LIBRARIES := framework mediatek-framework telephony-common

    include $(BUILD_JAVA_LIBRARY)
endif
