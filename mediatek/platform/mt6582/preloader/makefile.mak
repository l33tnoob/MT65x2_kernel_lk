###################################################################
# Include Files Directery
###################################################################

#include $(D_ROOT)/mtk_cust.mak

###################################################################
# Using GCC
###################################################################

CROSS_COMPILE = arm-linux-androideabi-

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC)-E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB
THUMB_MODE = TRUE


###################################################################
# Initialize GCC Compile Parameter 
###################################################################
DEFINE           = -D$(MTK_PLATFORM) 
OBJCFLAGS 	 = --gap-fill=0xff
AFLAGS_DEBUG 	 = -Wa,-gstabs,
STRIP_SYMBOL	 = -fdata-sections -ffunction-sections
INCLUDE_FILE     =  \
    -I$(MTK_ROOT_OUT)/PRELOADER_OBJ/inc \
    -I$(MTK_PATH_PLATFORM)/src/security/inc \
    -I$(MTK_PATH_PLATFORM)/src/security/trustzone \
    -I$(MTK_PATH_PLATFORM)/src/drivers/inc \
    -I$(MTK_PATH_PLATFORM)/src/core/inc \
    -I$(MTK_PATH_PLATFORM)/src/init/inc \
    -I$(MTK_PATH_PLATFORM)/src/security/inc \
    -I$(MTK_ROOT_OUT)/EMIGEN/inc \
    -I$(MTK_PATH_CUSTOM)/inc \
    -I$(D_ROOT)/custom/common/inc \
    -I$(D_ROOT)/inc/$(_CHIP) \
    -I$(MTK_ROOT_CUSTOM)/$(MTK_PROJECT)/common \
    -I$(MTK_ROOT_CUSTOM_OUT)/kernel/dct \
    -I$(MTK_ROOT_OUT)/PTGEN/common \
    -I$(MTK_ROOT_OUT)/NANDGEN/common


###################################################################
# GCC Compile Options 
###################################################################

ifeq ($(CREATE_SEC_LIB),TRUE)

INCLUDE_FILE     +=  \
    -I$(MTK_PATH_PLATFORM)/src/secure_lib/ \
    -I$(MTK_PATH_PLATFORM)/src/secure_lib/inc \
    -I$(MTK_PATH_PLATFORM)/src/secure_lib/crypto_lib \
    -I$(MTK_PATH_PLATFORM)/src/secure_lib/crypto_lib/opt \

# if it's security.lib, we must remove gcc debug message
C_OPTION	 := -Os -fdata-sections -ffunction-sections -fno-strict-aliasing -fno-common -ffixed-r8 -fno-builtin -ffreestanding -pipe -mno-thumb-interwork -Wstrict-prototypes -march=armv7-a $(DEFINE) -c $(INCLUDE_FILE) -msoft-float -D__ASSEMBLY__  -DPRELOADER_HEAP
C_OPTION_OPTIMIZE	 := -Os -fdata-sections -ffunction-sections -fno-strict-aliasing -fno-common -ffixed-r8 -fno-builtin -ffreestanding -pipe -mno-thumb-interwork -Wstrict-prototypes -march=armv7-a $(DEFINE) -c $(INCLUDE_FILE) -msoft-float -D__ASSEMBLY__  -DPRELOADER_HEAP
AFLAGS 		 := -c -march=armv7-a -g
AFLAGS_OPTIMIZE	 := -c -march=armv7-a -g

else

C_OPTION	    := -Os $(STRIP_SYMBOL) -fno-strict-aliasing -fno-common -ffixed-r8 -fno-builtin -ffreestanding -pipe -Wstrict-prototypes -march=armv7-a $(DEFINE) -c $(INCLUDE_FILE) -msoft-float -D__ASSEMBLY__ -g
C_OPTION_OPTIMIZE   := -Os $(STRIP_SYMBOL) -fno-strict-aliasing -fno-common -ffixed-r8 -fno-builtin -ffreestanding -pipe -Wstrict-prototypes -march=armv7-a $(DEFINE) -c $(INCLUDE_FILE) -msoft-float -D__ASSEMBLY__ -g
	
ifeq ($(THUMB_MODE),TRUE)
#thumb 
C_OPTION            += -mthumb-interwork -mthumb 
C_OPTION_OPTIMIZE   += -mthumb-interwork -mthumb 
else
C_OPTION	    += -mno-thumb-interwork
C_OPTION_OPTIMIZE   += -mno-thumb-interwork
endif

AFLAGS 		    := -c -march=armv7-a -g
AFLAGS_OPTIMIZE	    := -c -march=armv7-a -g

endif

#priority mode>cust>default
include $(MTK_PATH_PLATFORM)/default.mak

include ${MTK_ROOT_CUSTOM}/${MTK_PROJECT}/preloader/cust_bldr.mak

ifdef PL_MODE
C_OPTION += -D$(PL_MODE)
C_OPTION_OPTIMIZE += -D$(PL_MODE)
include $(MTK_PATH_PLATFORM)/mode/$(PL_MODE).mak
endif

ifeq ("$(MTK_SECURITY_SW_SUPPORT)","yes")	
	C_OPTION += -DMTK_SECURITY_SW_SUPPORT
endif

include $(MTK_PATH_PLATFORM)/feature.mak

MTK_CDEFS := $(PL_MTK_CDEFS)
MTK_ADEFS := $(PL_MTK_ADEFS)

C_OPTION += $(MTK_CFLAGS) $(MTK_CDEFS) $(MTK_INC)
AFLAGS   += $(MTK_AFLAGS) 

###################################################################
# gcc link descriptor
###################################################################

ifeq ($(findstring fpga, $(MTK_PROJECT)),fpga)
LDSCRIPT	:= $(MTK_PATH_PLATFORM)/link_descriptor_fpga.ld
else
LDSCRIPT	:= $(MTK_PATH_PLATFORM)/link_descriptor.ld
endif

LINKFILE	:= $(LD)
LINK		:= $(LINKFILE) -Bstatic -T $(LDSCRIPT) --gc-sections

###################################################################
# Object File
###################################################################

export All_OBJS
