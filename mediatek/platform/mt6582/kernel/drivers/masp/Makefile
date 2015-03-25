#######################################
# Sepcify source files
#######################################
include $(MTK_ROOT_BUILD)/Makefile
MASP_CORE_DRIVER_DIR := $(MTK_PATH_PLATFORM)/drivers/masp

asf_link := asf
lib_obj := module/sec_mod.o


ifeq ($(MTK_SECURITY_SW_SUPPORT), yes)

include $(MASP_CORE_DRIVER_DIR)/asf/Makefile.core

ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
ccflags-y += -I$(MTK_PATH_SOURCE)/../platform/${ARCH_MTK_PLATFORM}/kernel/core/include/trustzone/sec/Tlsec/inc
endif

ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/gud/MobiCoreKernelApi/include \
             -I$(MTK_PATH_PLATFORM)/drivers/gud/MobiCoreKernelApi/public 

mach_obj += mach/hacc_tee_req.o
mach_obj += mach/hacc_tee.o
mach_obj += mach/hacc_service.o
ccflags-y += -DTRUSTONIC_TEE_SUPPORT
else
include $(MASP_CORE_DRIVER_DIR)/mach/Makefile.mach
endif

ifeq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
CCCI_CUSTOM_DRIVER_DIR := $(MTK_PATH_CUSTOM)/$(call lc,$(MTK_PROJECT))/kernel/ccci
lib_obj += $(CCCI_CUSTOM_DRIVER_DIR)/cust_auth.o
else
include $(MASP_CORE_DRIVER_DIR)/asf/Makefile.crypto
lib_obj += $(crypto_obj)
lib_obj += $(asf_link)/auth/sec_wrapper.o
endif

else
ccflags-y += -DMTK_SECURITY_MODULE_LITE
core_obj += $(asf_link)/core/sec_ops.o
core_obj += $(asf_link)/core/alg_sha1.o
mach_obj += mach/sec_uid.o
EXTRA_CFLAGS += -I$(src)/asf/asf_inc
endif

# ALWAYS BUILD-IN
obj-y += sec.o

# HEADER FILES
EXTRA_CFLAGS += -I$(src)/module

# INCLUDE ASP LIBRARY
sec-y += $(lib_obj) $(core_obj) $(mach_obj)
