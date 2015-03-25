# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2011. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

##############################################################
# Including Neccesary Files
#

include ../build/Makefile
PRELOADER_OUT := ../../$(MTK_ROOT_OUT)/PRELOADER_OBJ

$(call codebase-path,preloader,$(abspath $(CURDIR)/$(call to-root)))

##############################################################
# Secure Library Building Control
#

export CREATE_SEC_LIB := FALSE
export CREATE_HW_CRYPTO_LIB := FALSE

##############################################################
# Variable Initialization
#

export D_ROOT           := $(CURDIR)
export W_ROOT           := $(subst /,\,$(CURDIR))
ALL                     ?= clean show_title show_internal_feature show_feature

#
# Image Auth
#
CUSTOM_PATH             := ${MTK_ROOT_CUSTOM}/${MTK_PROJECT}/security
IMAGE_AUTH_CFG_FILE     := ${PRELOADER_OUT}/inc/KEY_IMAGE_AUTH.h
IMAGE_AUTH_CONFIG_PATH  := ${CUSTOM_PATH}/image_auth
IMAGE_AUTH_KEY          := ${IMAGE_AUTH_CONFIG_PATH}/IMG_AUTH_KEY.ini
IMAGE_AUTH_CFG          := ${IMAGE_AUTH_CONFIG_PATH}/IMG_AUTH_CFG.ini
IMAGE_AUTH_KEY_EXIST    := $(shell if [ -f $(IMAGE_AUTH_KEY) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(IMAGE_AUTH_KEY_EXIST)","TRUE")
ALL                     += img_auth_info
endif

#
# PL Version
#
PL_CFG_FILE		:= ${CUSTOM_PATH}/pl_auth/PL_CFG.ini
PL_CFG_HDR		:= ${PRELOADER_OUT}/inc/PL_CFG.h
PL_CFG_EXIST		:= $(shell if [ -f $(PL_CFG_FILE) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(PL_CFG_EXIST)","TRUE")
ALL			+= pl_cfg_info
endif

#   
# SML Encode
#
SML_ENCODE_CFG_FILE     := ${PRELOADER_OUT}/inc/KEY_SML_ENCODE.h
SML_CONFIG_PATH         := ${CUSTOM_PATH}/sml_auth
SML_ENCODE_KEY          := ${SML_CONFIG_PATH}/SML_ENCODE_KEY.ini
SML_ENCODE_CFG          := ${SML_CONFIG_PATH}/SML_ENCODE_CFG.ini
SML_ENCODE_KEY_EXIST    := $(shell if [ -f $(SML_ENCODE_KEY) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(SML_ENCODE_KEY_EXIST)","TRUE")
ALL                     += sml_encode_info
endif

#
# SML Auth
#
SML_AUTH_CFG_FILE       := ${PRELOADER_OUT}/inc/KEY_SML_AUTH.h
SML_AUTH_PATH           := ${CUSTOM_PATH}/sml_auth
SML_AUTH_KEY            := ${SML_CONFIG_PATH}/SML_AUTH_KEY.ini
SML_AUTH_CFG            := ${SML_CONFIG_PATH}/SML_AUTH_CFG.ini
SML_AUTH_KEY_EXIST      := $(shell if [ -f $(SML_AUTH_KEY) ]; then echo "TRUE"; else echo "FALSE"; fi;)
ifeq ("$(SML_AUTH_KEY_EXIST)","TRUE")
ALL                     += sml_auth_info
endif

#
# preloader extension
#
PLATFORM                := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
PRELOADER_EXT_BIN       := ${MTK_ROOT_CUSTOM}/${PLATFORM}/preloader/preloader_ext.bin
PRELOADER_EXT_BIN_EXIST := $(shell if [ -f $(PRELOADER_EXT_BIN) ]; then echo "TRUE"; else echo "FALSE"; fi;)

#
# Tool
#
TOOL_PATH               := ../build/tools
SIGN_TOOL               := ${TOOL_PATH}/SignTool/SignTool_PL
CIPHER_TOOL             := ${TOOL_PATH}/CipherTool/CipherTool 

ifdef PL_MODE
PL_IMAGE_NAME           := preloader_$(MTK_PROJECT)_$(PL_MODE)
else 
PL_IMAGE_NAME           := preloader_$(MTK_PROJECT)
endif

HW_CRYPTO_LIB           := $(MTK_PATH_PLATFORM)/src/HwCryptoLib.a
SECURITY_LIB            := $(MTK_PATH_PLATFORM)/src/SecLib.a
DA_VERIFY_LIB           := $(MTK_PATH_PLATFORM)/src/security/auth/DaVerifyLib.a
SEC_PLAT_LIB            := $(MTK_PATH_PLATFORM)/src/SecPlat.a
ALL                     += build_info project_info $(SUBDIRS)
COMMON_DIR_MK           := $(D_ROOT)/build/common-dir.mak
COMMON_FILE_MK          := $(D_ROOT)/build/common.mak
export COMMON_DIR_MK COMMON_FILE_MK

ifeq ($(CREATE_SEC_LIB),TRUE)
    ALL                 += $(SECURITY_LIB) $(DA_VERIFY_LIB) $(SEC_PLAT_LIB)
    SUBDIRS             := $(MTK_PATH_PLATFORM)/src/
else
    ALL                 += $(D_BIN)/$(PL_IMAGE_NAME).elf $(PRELOADER_OUT)/System.map
    SUBDIRS             := $(MTK_PATH_PLATFORM)/src/ $(MTK_PATH_CUSTOM)/
endif

PL_MTK_CDEFS            := $(call mtk.custom.generate-macros)
PL_MTK_ADEFS            := $(call mtk.custom.generate-macros)
export PL_MTK_CDEFS PL_MTK_ADEFS

include build/debug.in build/setting.in 
include $(MTK_PATH_PLATFORM)/makefile.mak

.PHONY: $(ALL)
all: $(ALL)

##############################################################
# ELF Generation
#

$(D_BIN)/$(PL_IMAGE_NAME).elf:
	$(LD) --gc-sections -Bstatic -T$(LDSCRIPT) \
	$(sort $(wildcard $(D_OBJ)/*)) $(SECURITY_LIB) \
	$(shell if [ -f $(DA_VERIFY_LIB) ]; then echo $(DA_VERIFY_LIB); else echo ""; fi;) \
	$(shell if [ -f $(SEC_PLAT_LIB) ]; then echo $(SEC_PLAT_LIB); else echo ""; fi;) \
	$(shell if [ -f $(HW_CRYPTO_LIB) ]; then echo $(HW_CRYPTO_LIB); else echo ""; fi;) \
	-Map $(PRELOADER_OUT)/system.map -o $(D_BIN)/$(PL_IMAGE_NAME).elf
	@$(OBJCOPY) -R .dram $(D_BIN)/$(PL_IMAGE_NAME).elf -O elf32-littlearm $(D_BIN)/preloader.elf 
	@$(OBJCOPY) ${OBJCFLAGS} $(D_BIN)/$(PL_IMAGE_NAME).elf -O binary $(D_BIN)/$(PL_IMAGE_NAME).bin 
ifeq ("$(PRELOADER_EXT_BIN_EXIST)","TRUE")
	cat $(PRELOADER_EXT_BIN) >> $(D_BIN)/$(PL_IMAGE_NAME).bin
endif
	@./zero_padding.sh $(D_BIN)/$(PL_IMAGE_NAME).bin 4
	@tools/ft/FeatureTool encode feature_hex
	@if [ -a feature_hex ]; then cat feature_hex >> $(D_BIN)/$(PL_IMAGE_NAME).bin; rm -f feature_hex; else echo ""; fi;
	@./alignment_check.sh $(MTK_ROOT_OUT)/EMIGEN/MTK_Loader_Info.tag 4
	@cat $(MTK_ROOT_OUT)/EMIGEN/MTK_Loader_Info.tag >> $(D_BIN)/$(PL_IMAGE_NAME).bin
	@readelf -s $@ | awk -F':' '/Num/ {print $$2}' > $(PRELOADER_OUT)/report-codesize.txt
	@readelf -s $@ | awk -F':' '{if ($$1>0) print $$2}' | awk -F' ' '{if ($$2>0) print $$0}' | sort +0 -1 >> $(PRELOADER_OUT)/report-codesize.txt
	@cat $(PRELOADER_OUT)/report-codesize.txt | sed -r 's/[ ]+/,/g' > $(PRELOADER_OUT)/report-codesize.csv

##############################################################
# Security Library Generation
#

$(SECURITY_LIB):
	rm -rf $(SECURITY_LIB)
	@echo AR  $(D_OBJ)/*
	$(AR) -r $(SECURITY_LIB)  $(D_OBJ)/*
	@echo =================================================================
	@echo Security Library
	@echo '$(SECURITY_LIB)' built at
	@echo time : $(shell date )
	@echo =================================================================

$(DA_VERIFY_LIB):
	rm -rf $(DA_VERIFY_LIB)
	@echo AR $(PRELOADER_OUT)/out_da_verify/*
	$(AR) -r $(DA_VERIFY_LIB) $(PRELOADER_OUT)/out_da_verify/*
	@echo =================================================================
	@echo DA Verify Library
	@echo '$(DA_VERIFY_LIB)' built at
	@echo time : $(shell date )
	@echo =================================================================

$(SEC_PLAT_LIB):
	rm -rf $(SEC_PLAT_LIB)
	@echo AR $(PRELOADER_OUT)/out_plat/*
	$(AR) -r $(SEC_PLAT_LIB) $(PRELOADER_OUT)/out_plat/*
	@echo =================================================================
	@echo Security Library
	@echo '$(SEC_PLAT_LIB)' built at
	@echo time : $(shell date )
	@echo =================================================================

##############################################################
# File for Debugging
#

$(PRELOADER_OUT)/System.map: $(D_BIN)/$(PL_IMAGE_NAME).elf
	@$(NM) $< | \
	grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
	sort > $(PRELOADER_OUT)/function.map

##############################################################
# Dump Configurations
#

show_title:
	@echo =================================================================
	@echo Building Configuration:
	@echo Project              = $(MTK_PROJECT)
	@echo Platform             = $(MTK_PLATFORM)
	@echo Buildspec            = buildspec.mak
	@echo Create SecLib        = $(CREATE_SEC_LIB)
	@echo Image Auth key exist = $(IMAGE_AUTH_KEY_EXIST)
	@echo SML Encode key exist = $(SML_ENCODE_KEY_EXIST)
	@echo SML Auth key exist   = $(SML_AUTH_KEY_EXIST)
	@echo Preloader EXT exist  = $(PRELOADER_EXT_BIN_EXIST)
	@echo SECRO AC support     = $(MTK_SEC_SECRO_AC_SUPPORT)
	@echo =================================================================

show_internal_feature:
	@echo =================================================================
	@echo Internal Feature: 
	@echo HW_INIT_ONLY         = $(HW_INIT_ONLY) 
	@echo CFG_MDJTAG_SWITCH= $(CFG_MDJTAG_SWITCH)
	@echo CFG_MDMETA_DETECT= $(CFG_MDMETA_DETECT)
	@echo CFG_HW_WATCHDOG= $(CFG_HW_WATCHDOG)
	@echo CFG_APWDT_DISABLE= $(CFG_APWDT_DISABLE)
	@echo CFG_MDWDT_DISABLE= $(CFG_MDWDT_DISABLE)
	@echo CFG_SYS_STACK_SZ= $(CFG_SYS_STACK_SZ)
	@echo CFG_MMC_ADDR_TRANS= $(CFG_MMC_ADDR_TRANS)
	@echo CFG_BOOT_ARGUMENT= $(CFG_BOOT_ARGUMENT)
	@echo CFG_RAM_CONSOLE= $(CFG_RAM_CONSOLE)
	@echo =================================================================

show_feature:
	@if [ -e "${MTK_ROOT_CUSTOM}/${MTK_PROJECT}/preloader/cust_bldr.mak" ]; then \
	echo  =========================================== ; \
	echo Platform Feature: ; \
	echo 'CFG_FPGA_PLATFORM'= $(CFG_FPGA_PLATFORM) ; \
	echo 'CFG_EVB_PLATFORM'= $(CFG_EVB_PLATFORM) ; \
	echo 'CFG_BATTERY_DETECT'= $(CFG_BATTERY_DETECT) ; \
	echo 'CFG_PMT_SUPPORT'= $(CFG_PMT_SUPPORT) ; \
	echo  =========================================== ; \
	echo Communication Feature: ; \
	echo 'CFG_UART_TOOL_HANDSHAKE'= $(CFG_UART_TOOL_HANDSHAKE) ; \
	echo 'CFG_USB_TOOL_HANDSHAKE'= $(CFG_USB_TOOL_HANDSHAKE) ; \
	echo 'CFG_USB_DOWNLOAD'= $(CFG_USB_DOWNLOAD) ; \
	echo 'CFG_LOG_BAUDRATE'= $(CFG_LOG_BAUDRATE) ; \
	echo 'CFG_META_BAUDRATE'= $(CFG_META_BAUDRATE) ; \
	echo 'CFG_UART_LOG'= $(CFG_UART_LOG) ; \
	echo 'CFG_UART_META'= $(CFG_UART_META) ; \
	echo 'CFG_EMERGENCY_DL_SUPPORT'= $(CFG_EMERGENCY_DL_SUPPORT) ; \
	echo 'CFG_EMERGENCY_DL_TIMEOUT_MS'= $(CFG_EMERGENCY_DL_TIMEOUT_MS) ; \
	echo 'CFG_USB_UART_SWITCH'= $(CFG_USB_UART_SWITCH) ; \
	echo  =========================================== ; \
	echo Image Loading: ; \
	echo 'CFG_LOAD_UBOOT'= $(CFG_LOAD_UBOOT) ; \
	echo 'CFG_LOAD_AP_ROM'= $(CFG_LOAD_AP_ROM) ; \
	echo 'CFG_LOAD_MD_ROM'= $(CFG_LOAD_MD_ROM) ; \
	echo 'CFG_LOAD_MD_RAMDISK'= $(CFG_LOAD_MD_RAMDISK) ; \
	echo 'CFG_LOAD_CONN_SYS'= $(CFG_LOAD_CONN_SYS) ; \
	echo 'CFG_UBOOT_MEMADDR'= $(CFG_UBOOT_MEMADDR) ; \
	echo 'CFG_AP_ROM_MEMADDR'= $(CFG_AP_ROM_MEMADDR) ; \
	echo 'CFG_MD1_ROM_MEMADDR'= $(CFG_MD1_ROM_MEMADDR) ; \
	echo 'CFG_MD1_RAMDISK_MEMADDR'= $(CFG_MD1_RAMDISK_MEMADDR) ; \
	echo 'CFG_MD2_ROM_MEMADDR'= $(CFG_MD2_ROM_MEMADDR) ; \
	echo 'CFG_MD2_RAMDISK_MEMADDR'= $(CFG_MD2_RAMDISK_MEMADDR) ; \
	echo 'CFG_CONN_SYS_MEMADDR'= $(CFG_CONN_SYS_MEMADDR) ; \
	echo 'ONEKEY_REBOOT_NORMAL_MODE_PL'= $(ONEKEY_REBOOT_NORMAL_MODE_PL) ; \
	echo 'KPD_USE_EXTEND_TYPE'= $(KPD_USE_EXTEND_TYPE) ; \
	echo 'KPD_PMIC_LPRST_TD'= $(KPD_PMIC_LPRST_TD) ; \
	echo 'MTK_PMIC_RST_KEY'= $(MTK_PMIC_RST_KEY) ; \
	echo 'CFG_TRUSTONIC_TEE_SUPPORT'= $(CFG_TRUSTONIC_TEE_SUPPORT) ; \
	echo 'CFG_TEE_DRAM_SIZE'= $(CFG_TEE_DRAM_SIZE) ; \
	fi
##############################################################
# Adding Build Time
#

build_info:
	@echo // Auto generated. Build Time Information > $(PRELOADER_OUT)/inc/preloader.h
	@echo '#'define BUILD_TIME '"'$(shell date +%Y%m%d-%H%M%S)'"' >> $(PRELOADER_OUT)/inc/preloader.h


##############################################################
# Adding Project Configuration
#

project_info:
	@echo // Auto generated. Import ProjectConfig.mk > $(PRELOADER_OUT)/inc/proj_cfg.h
	@echo '#'define CUSTOM_SUSBDL_CFG $(MTK_SEC_USBDL) >> $(PRELOADER_OUT)/inc/proj_cfg.h
	@echo '#'define CUSTOM_SBOOT_CFG $(MTK_SEC_BOOT) >> $(PRELOADER_OUT)/inc/proj_cfg.h
	@echo '#'define MTK_SEC_MODEM_AUTH $(MTK_SEC_MODEM_AUTH) >> $(PRELOADER_OUT)/inc/proj_cfg.h
        ifdef MTK_SEC_SECRO_AC_SUPPORT
	@echo '#'define MTK_SEC_SECRO_AC_SUPPORT $(MTK_SEC_SECRO_AC_SUPPORT) >> $(PRELOADER_OUT)/inc/proj_cfg.h
        endif

##############################################################
# Generate Key Info File
#

img_auth_info: $(SIGN_TOOL) $(IMAGE_AUTH_KEY) $(IMAGE_AUTH_CFG)
	@touch $(IMAGE_AUTH_CFG_FILE)
	@chmod 777 $(IMAGE_AUTH_CFG_FILE)
	@./$(SIGN_TOOL) $(IMAGE_AUTH_KEY) $(IMAGE_AUTH_CFG) $(IMAGE_AUTH_CFG_FILE) IMG

sml_encode_info: $(CIPHER_TOOL) $(SML_ENCODE_KEY) $(SML_ENCODE_CFG)
	@touch $(SML_ENCODE_CFG_FILE)
	@chmod 777 $(SML_ENCODE_CFG_FILE)
	@./$(CIPHER_TOOL) GEN_HEADER $(SML_ENCODE_KEY) $(SML_ENCODE_CFG) $(SML_ENCODE_CFG_FILE) SML

sml_auth_info: $(SIGN_TOOL) $(SML_AUTH_KEY) $(SML_AUTH_CFG)
	@touch $(SML_AUTH_CFG_FILE)
	@chmod 777 $(SML_AUTH_CFG_FILE)
	@./$(SIGN_TOOL) $(SML_AUTH_KEY) $(SML_AUTH_CFG) $(SML_AUTH_CFG_FILE) SML

pl_cfg_info: $(PL_CFG_FILE)
	@touch $(PL_CFG_HDR)
	@chmod 777 $(PL_CFG_HDR)
	@echo '#'define $(shell cat $(PL_CFG_FILE) | sed -s 's/=//g') > $(PL_CFG_HDR)

$(SUBDIRS):
	@$(MAKE) -C $@ --no-print-directory -s

##############################################################
# Clean 
#

clean:
	@rm -rf $(PRELOADER_OUT)/obj
	@mkdir -p $(PRELOADER_OUT)/obj
	@rm -rf $(PRELOADER_OUT)/inc
	@mkdir -p $(PRELOADER_OUT)/inc
	@rm -rf $(PRELOADER_OUT)/bin
	@mkdir -p $(PRELOADER_OUT)/bin
	@rm -rf $(PRELOADER_OUT)/out_da_verify
	@mkdir -p $(PRELOADER_OUT)/out_da_verify
	@rm -rf $(PRELOADER_OUT)/out_plat
	@mkdir -p $(PRELOADER_OUT)/out_plat
	@rm -rf $(PRELOADER_OUT)/out_hw_crypto_lib
	@mkdir -p $(PRELOADER_OUT)/out_hw_crypto_lib

