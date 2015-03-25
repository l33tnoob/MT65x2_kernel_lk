ifneq ($(filter arm x86 mips,$(TARGET_ARCH)),)
include $(call all-subdir-makefiles)
endif

