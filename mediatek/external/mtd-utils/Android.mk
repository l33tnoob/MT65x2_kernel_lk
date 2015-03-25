LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(TARGET_USERIMAGES_USE_UBIFS)),true)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	lib/libcrc32.c \
	mkfs.ubifs/compr.c \
	mkfs.ubifs/crc16.c \
	mkfs.ubifs/devtable.c \
	mkfs.ubifs/hashtable/hashtable.c \
	mkfs.ubifs/hashtable/hashtable_itr.c \
	mkfs.ubifs/lpt.c \
	mkfs.ubifs/mkfs.ubifs.c \
	ubi-utils/libubi.c \
	lz4k_encoder/lz4k_compress.c \
	lz4k_encoder/lz4k_decompress.c \
	lz4k_encoder/lz4k_matcher.c \
	lz4k_encoder/divsufsort.c \
	lz4k_encoder/sssort.c \
	lz4k_encoder/trsort.c \
	../../../external/e2fsprogs/lib/uuid/gen_uuid.c \
	../../../external/e2fsprogs/lib/uuid/pack.c \
	../../../external/e2fsprogs/lib/uuid/unpack.c \
	../../../external/e2fsprogs/lib/uuid/unparse.c \
	../lzo/src/lzo1x_9x.c \

LOCAL_C_INCLUDES := \
	external/e2fsprogs/lib \
	mediatek/external/mtd-utils/include \
	mediatek/external/mtd-utils/ubi-utils/include \
	mediatek/external/lzo/include \

LOCAL_CFLAGS := -O2 -Wall \
	-DHAVE_INTTYPES_H \
	-DHAVE_UNISTD_H \
	-DHAVE_ERRNO_H \
	-DHAVE_NETINET_IN_H \
	-DHAVE_SYS_IOCTL_H \
	-DHAVE_SYS_MMAN_H \
	-DHAVE_SYS_MOUNT_H \
	-DHAVE_SYS_PRCTL_H \
	-DHAVE_SYS_RESOURCE_H \
	-DHAVE_SYS_SELECT_H \
	-DHAVE_SYS_STAT_H \
	-DHAVE_SYS_TYPES_H \
	-DHAVE_STDLIB_H \
	-DHAVE_STRDUP \
	-DHAVE_MMAP \
	-DHAVE_UTIME_H \
	-DHAVE_GETPAGESIZE \
	-DHAVE_LSEEK64 \
	-DHAVE_LSEEK64_PROTOTYPE \
	-DHAVE_EXT2_IOCTLS \
	-DHAVE_LINUX_FD_H \
	-DHAVE_TYPE_SSIZE_T \
	-DHAVE_SYS_TIME_H \
	-DHAVE_SYSCONF

LOCAL_STATIC_LIBRARIES := libz

LOCAL_MODULE := mkfs_ubifs
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	lib/libcrc32.c \
	ubi-utils/dictionary.c \
	ubi-utils/libiniparser.c \
	ubi-utils/libubigen.c \
	ubi-utils/ubinize.c \
	ubi-utils/ubiutils-common.c \

LOCAL_C_INCLUDES := \
	mediatek/external/mtd-utils/include \
	mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubinize
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)

endif


include $(CLEAR_VARS)

LOCAL_SRC_FILES := lib/libmtd.c \
                   lib/libmtd_legacy.c \
                   lib/libcrc32.c \
                   lib/libfec.c \

LOCAL_C_INCLUDES := \
        mediatek/external/mtd-utils/include \
        mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := libmtd

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := nandwrite.c

LOCAL_C_INCLUDES := \
        mediatek/external/mtd-utils/include \
        mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := nandwrite

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd 

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := flash_erase.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := flash_erase
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubiformat.c \
		ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \
		ubi-utils/libscan.c \
		ubi-utils/libubigen.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \
		    $(MTK_ROOT_OUT)/PTGEN/common

		    
LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubiformat
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubiformat.c \
		ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \
		ubi-utils/libscan.c \
		ubi-utils/libubigen.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubiformat_static
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libc libstdc++ libmtd
LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubi_ota_update
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubinfo.c \
		ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubinfo
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubiattach.c \
		ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubiattach
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubidetach.c \
		ubi-utils/libubi.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubidetach
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubimkvol.c \
		ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubimkvol
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubirmvol.c \
		ubi-utils/libubi.c \


LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubirmvol
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/ubiupdatevol.c \
		ubi-utils/libubi.c \


LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := ubiupdatevol
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ubi-utils/mtdinfo.c \
		ubi-utils/libubi.c \
		ubi-utils/ubiutils-common.c \
		ubi-utils/libubigen.c \

LOCAL_C_INCLUDES := mediatek/external/mtd-utils/include \
                    mediatek/external/mtd-utils/ubi-utils/include \

LOCAL_CFLAGS := -O2 -Wall

LOCAL_MODULE := mtdinfo
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES := libmtd

include $(BUILD_EXECUTABLE)
