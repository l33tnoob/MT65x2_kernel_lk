###################################################################
# flag
###################################################################

MOD_SRC  = 
MOD_OBJ  = $(MTK_ROOT_OUT)/PRELOADER_OBJ/obj

###################################################################
# source files
# - add your file here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
###################################################################

MOD_SRC  := \
sec.c \
sec_boot.c \
sec_ctrl.c \
sec_key.c \
sec_rom_info.c \
sec_secroimg.c \
sec_util.c \
sec_region.c \
sec_flashtool_cfg.c \

ifeq ($(CFG_TRUSTONIC_TEE_SUPPORT),1)
SUBDIRS += trustzone
endif

###################################################################
# target
###################################################################

include $(COMMON_FILE_MK)

###################################################################
# include sub dir
###################################################################

ifeq ($(CUSTOM_SEC_AUTH_SUPPORT), yes)
SUBDIRS += $(MTK_ROOT_CUSTOM)/common/security/preloader
else
SUBDIRS += auth
endif

include $(COMMON_DIR_MK)
