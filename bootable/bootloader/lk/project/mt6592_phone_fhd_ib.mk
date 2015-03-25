#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := mediatek

MODULES += app/mt_boot \
           app/shell \
           out/lk

DEBUG := 0

#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1

