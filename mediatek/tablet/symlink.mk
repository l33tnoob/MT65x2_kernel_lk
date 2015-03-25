ifneq ($(MTK_TABLET_HARDWARE), )

ifeq ($(MTK_PLATFORM), MT6572)
SYMLIST := mt8312c
else ifeq ($(MTK_PLATFORM), MT6577)
SYMLIST := mt8377 mt8317
else ifeq ($(MTK_PLATFORM), MT6589)
SYMLIST := mt8389 mt8125
MTK_HWC_CHIP := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
LOCAL_MODULE := $(subst .so,,$(LOCAL_MODULE))
else ifeq ($(MTK_PLATFORM), MT6582)
SYMLIST := mt8382 mt8312 mt8121 mt8111
else ifeq ($(MTK_PLATFORM), MT6592)
SYMLIST := mt8392 mt8388
endif

SYMLINKS := $(foreach M,$(SYMLIST),$(if $(filter-out $(MTK_HWC_CHIP),$(M)),$(LOCAL_MODULE_PATH)/$(basename $(LOCAL_MODULE)).$(M).so))

$(SYMLINKS): TARGET := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	$(hide) echo "Symlink: $@ -> $(TARGET).so"
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -rf $@
	$(hide) ln -sf $(TARGET).so $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

ALL_MODULES.$(LOCAL_MODULE).INSTALLED :=\
        $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)

M :=
SYMLIST :=
SYMLINKS :=
TARGET :=

endif
