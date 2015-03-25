LOCAL_DIR := $(GET_LOCAL_DIR)

PLATFORM := mediatek

MODULES += \
        dev/keys \
    lib/ptable

include out/lk/rules_platform.mk

