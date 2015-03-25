#
#  Copyright Statement:
#  --------------------
#  This software is protected by Copyright and the information contained
#  herein is confidential. The software may not be copied and the information
#  contained herein may not be used or disclosed except with the written
#  permission of MediaTek Inc. (C) 2005
#
#  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
#  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
#  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
#  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
#  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
#  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
#  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
#  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
#  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
#  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
#
#  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
#  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
#  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
#  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
#  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
#  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
#  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
#  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
#  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
#  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
#
#*************************************************************************

LIB_NAME        = ape
EXE_NAME        = ape_demo

####### path setting #######
PATHC_ADS_1_2	  = C:/Progra~1/ARM/ADSv1_2/bin
PATHC_RVCT_3_1  = C:/Progra~1/ARM/RVCT/Programs/3.1/569/win_32-pentium
PATHC_RVCT_2_2	= C:/Progra~1/ARM/RVCT/Programs/2.2/349/win_32-pentium
PATH_LIB        = lib
PATH_OBJ        = obj
PATH_DEMO       = demo
PATH_INC        = inc
PATH_SRC        = src

####### PC ##########
ifeq ($(PLATFORM),pc)
SUFFIX_EXE    = exe
SUFFIX_OBJ    = obj
SUFFIX_LIB    = lib

CC		          = cl.exe 
CFLAGS          = -c -nologo -I$(PATH_SRC) -I$(PATH_INC) -Idemo\lib -D__VC6=1 -DSTATIC_DECLARE= -DEXTERN=extern
CRULE           = @$(CC) $(CFLAGS) $< -Fo$@

WLINK		= link.exe
W_LFLAG	= -lib -nologo
W_EFLAG	= -nologo -I$(PATH_INC)

LINK_EXE_RULE = $(CC) $(W_EFLAG) $^	/Fe$@
LINK_LIB_RULE = $(WLINK) $(W_LFLAG) -out:$@ $^

endif ###############

####### ARM #######
ifeq ($(PLATFORM),arm)

#---- configuration ----
ifeq ($(CONFIG),arm7ads1.2)
PATH_COMPILER = $(PATHC_ADS_1_2)
COMPILER_DEP_FLAG = -cpu ARM7TDMI -g+ -O2
endif
ifeq ($(CONFIG),arm9ads1.2)
PATH_COMPILER = $(PATHC_ADS_1_2)
COMPILER_DEP_FLAG = -cpu ARM9EJ-S -g+ -O2
endif
ifeq ($(CONFIG),arm9rvct2.2)
PATH_COMPILER = $(PATHC_RVCT_2_2)
COMPILER_DEP_FLAG = --cpu ARM9EJ-S -O3 -Otime --split_sections
endif
ifeq ($(CONFIG),arm7rvct3.1)
PATH_COMPILER = $(PATHC_RVCT_3_1)
COMPILER_DEP_FLAG = --cpu ARM7EJ-S -O3 -Otime --split_sections
endif
ifeq ($(CONFIG),arm9rvct3.1)
PATH_COMPILER = $(PATHC_RVCT_3_1)
COMPILER_DEP_FLAG = --cpu ARM926EJ-S -O3 -Otime --split_sections
endif
ifeq ($(CONFIG),arm11rvct3.1)
PATH_COMPILER = $(PATHC_RVCT_3_1)
COMPILER_DEP_FLAG = --cpu ARM1136J-S -O3 -Otime --split_sections
endif 
#--- configuration end ---

SUFFIX_EXE    = axf
SUFFIX_OBJ    = o
SUFFIX_LIB    = a

CC		        = $(PATH_COMPILER)/armcc.exe 
CFLAGS        = -c $(COMPILER_DEP_FLAG) -Otime -I$(PATH_SRC) -I$(PATH_INC) -Idemo\lib -D__CC_ARM=1 -D_ARM_ASSEM_=1 -DSTATIC_DECLARE= -DEXTERN=extern 
ifeq ($(CONFIG),arm11rvct3.1)
CFLAGS        += -D__ARMv6=1  
endif
CRULE         = @$(CC) $(CFLAGS) -o$@ $<

AR            = $(PATH_COMPILER)/armar.exe
LINK          = $(PATH_COMPILER)/armlink.exe
LINK_EXE_RULE = $(LINK) -o $@ $^
LINK_LIB_RULE = $(AR) --create $@ $^

endif 
################

####### src lists ##########
EXE_SRC = \
    cue_parser.c\
    wavwrite.c\
    demac.c\
    libprofiler.c\
    parser.c

LIB_SRC	=	\
    predictor.c\
    filter_16_11.c\
    filter_32_10.c\
    filter_64_11.c\
    filter_256_13.c\
    filter_1280_15.c\
    entropy.c\
    crc.c\
    decoder.c\
    ape_decoder_swip.c

ifeq ($(CONFIG),arm11rvct3.1)
LIB_ASM_SRC = \
    ARMv6_unaligned_dot_and_add_64.s\
    ARMv6_unaligned_dot_and_sub_64.s\
    ARMv6_aligned_dot_and_add_64.s\
    ARMv6_aligned_dot_and_sub_64.s\
    ARMv6_dot_product.s\
    ARMv6_predictor.s
else
LIB_ASM_SRC = \
    unaligned_dot_and_add_64.s\
    unaligned_dot_and_sub_64.s\
    dot_product_arm9m.s\
    aligned_dot_and_add_64.s\
    aligned_dot_and_sub_64.s
endif

####### object lists #######
TARGET_EXE      = $(EXE_NAME)_$(CONFIG).$(SUFFIX_EXE)
TARGET_LIB      = $(PATH_LIB)/$(LIB_NAME)_$(CONFIG).$(SUFFIX_LIB)
DEMO_OBJECTS    = $(subst /,\,$(EXE_SRC:%.c=$(PATH_OBJ)/%_$(CONFIG).$(SUFFIX_OBJ)))
LIB_OBJECTS     = $(subst /,\,$(LIB_SRC:%.c=$(PATH_OBJ)/%_$(CONFIG).$(SUFFIX_OBJ)))
LIB_ASM_OBJECTS = $(subst /,\,$(LIB_ASM_SRC:%.s=$(PATH_OBJ)/%_$(CONFIG).$(SUFFIX_OBJ)))
####### link-time dependency #######
all: $(PATH_LIB) $(PATH_OBJ) $(TARGET_EXE)

$(PATH_LIB):
	mkdir $(PATH_LIB)
    
$(PATH_OBJ):
	mkdir $(PATH_OBJ)

$(TARGET_EXE) : $(TARGET_LIB) $(DEMO_OBJECTS)
	$(LINK_EXE_RULE)

ifeq ($(PLATFORM),arm)
$(TARGET_LIB) : $(LIB_OBJECTS) $(LIB_ASM_OBJECTS)
	$(LINK_LIB_RULE)
endif

ifeq ($(PLATFORM),pc)
$(TARGET_LIB) : $(LIB_OBJECTS)
	$(LINK_LIB_RULE)
endif
	    
clean: $(PATH_LIB) $(PATH_OBJ)
	del /Q $(PATH_OBJ)\*
	del /Q $(PATH_LIB)\*
	del /Q *.axf
	del /Q *.exe
	del /Q *.dxe

####### compile-time depencency
include ape.dep
