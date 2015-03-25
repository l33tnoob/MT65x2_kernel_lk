#!/bin/bash
# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

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
# Program:
# Program to create ALPS preloader binary
#

function build_preloader () {

    if [ "$1" != "" ]; then export TARGET_PRODUCT=$1; fi
	#set -e
    source ../../mediatek/build/shell.sh ../../ preloader
    CUR_DIR=`pwd`

    ##############################################################
    # Variable Initialization
    #
    if [ -z ${PL_MODE} ]; then 
		PL_IMAGE=${PRELOADER_OUT}/bin/preloader_${MTK_PROJECT}.bin ;
		PL_ELF_IMAGE=${PRELOADER_OUT}/bin/preloader_${MTK_PROJECT}.elf
	else
		PL_IMAGE=${PRELOADER_OUT}/bin/preloader_${MTK_PROJECT}_${PL_MODE}.bin
		PL_ELF_IMAGE=${PRELOADER_OUT}/bin/preloader_${MTK_PROJECT}_${PL_MODE}.elf
	fi
	


    ##############################################################
    # Binary Generation
    #

    make ${FORCE_CFG}

    if [ ! -f "${PL_IMAGE}" ]; then echo "BUILD FAIL."; exit 1; fi

    PL_FUN_MAP=${PRELOADER_OUT}/function.map
    
   source ${MTK_PATH_PLATFORM}/check_size.sh
}

function ns_chip_ini () {

    chmod u+w ${PL_IMAGE}
    cp -f ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}

    ##############################################################
    # PROCESS BOOT LOADER
    #

    chmod 777 ${PBP_TOOL}/PBP
    
    ${PBP_TOOL}/PBP -g ${GFH_PATH}/GFH_CONFIG.ini ${PL_IMAGE}
    if [ $? -eq 0 ] ; then
	echo "${PBP_TOOL}/PBP pass !!!!"
    else
	echo "===BUILD FAIL. ${PBP_TOOL}/PBP return fail==="
	exit 1;
    fi	
}

function ns_chip_legacy () {

    if [ "${MTK_EMMC_SUPPORT}" == "yes" ]; then
	GFH_INFO=${GFH_PATH}/GFH_INFO_EMMC.txt
    else
	GFH_INFO=${GFH_PATH}/GFH_INFO.txt
    fi
    GFH_HASH=${GFH_PATH}/GFH_HASH.txt

    ##############################################################
    # ATTACH GFH
    #

    echo ""      
    echo "[ Attach ${MTK_PLATFORM} GFH ]"
    echo "============================================"
    echo " : GFH_INFO             - ${GFH_INFO}"
    echo " : GFH_HASH             - ${GFH_HASH}"

    chmod u+w ${PL_IMAGE}
    mv -f ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cp -f ${GFH_INFO} ${PL_IMAGE}	    
    chmod u+w ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cat ${PL_IMAGE/%.bin/_NO_GFH.bin} >> ${PL_IMAGE}
    cat ${GFH_HASH} >> ${PL_IMAGE}

    ##############################################################
    # PROCESS BOOT LOADER
    #

    chmod 777 ${PBP_TOOL}/PBP
    
    ${PBP_TOOL}/PBP ${PL_IMAGE}
    if [ $? -eq 0 ] ; then
	echo "${PBP_TOOL}/PBP pass !!!!"
    else
	echo "===BUILD FAIL. ${PBP_TOOL}/PBP return fail==="
	exit 1;
    fi	
}

function ns_chip () {

    ##############################################################
    # Only Support Non-Secure Chip
    #        

    echo ""
    echo "[ Only for Non-Secure Chip ]"
    echo "============================================"
    
    GFH_PATH=${CHIP_CONFIG_PATH}/ns/  

    ##############################################################
    # INITIALIZE GFH
    #
   
    if [ -e ${GFH_PATH}/GFH_CONFIG.ini ]; then
	echo "INI_GFH_GEN=YES"
	ns_chip_ini;
    else
	echo "INI_GFH_GEN=NO"
	ns_chip_legacy;
    fi
}

function s_chip_ini () {

    chmod u+w ${PL_IMAGE}
    cp -f ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}

    ##############################################################
    # PROCESS BOOT LOADER
    #

    chmod 777 ${PBP_TOOL}/PBP
    
    ${PBP_TOOL}/PBP -m ${CHIP_CONFIG} -i ${CHIP_KEY} -g ${GFH_PATH}/GFH_CONFIG.ini ${PL_IMAGE}
    if [ $? -eq 0 ] ; then
	echo "${PBP_TOOL}/PBP pass !!!!"
    else
	echo "===BUILD FAIL. ${PBP_TOOL}/PBP return fail==="
	exit 1;
    fi	
}

function s_chip_legacy () {
    
    if [ "${MTK_EMMC_SUPPORT}" == "yes" ]; then
        GFH_INFO=${GFH_PATH}/GFH_INFO_EMMC.txt
    else
    	GFH_INFO=${GFH_PATH}/GFH_INFO.txt
    fi
    GFH_SEC_KEY=${GFH_PATH}/GFH_SEC_KEY.txt
    GFH_ANTI_CLONE=${GFH_PATH}/GFH_ANTI_CLONE.txt
    GFH_HASH_SIGNATURE=${GFH_PATH}/GFH_HASH_AND_SIG.txt
    GFH_PADDING=${GFH_PATH}/GFH_PADDING.txt

    source ${CONFIG_PATH}/SECURE_JTAG_CONFIG.ini
    if [ "${SECURE_JTAG_ENABLE}" == "TRUE" ]; then
        SECURE_JTAG_GFH=${GFH_PATH}/GFH_SEC_CFG_JTAG_ON.txt
        echo " : SECURE_JTAG_ENABLE - TRUE"
    elif [ "${SECURE_JTAG_ENABLE}" == "FALSE" ]; then
        SECURE_JTAG_GFH=${GFH_PATH}/GFH_SEC_CFG_JTAG_OFF.txt
        echo " : SECURE_JTAG_ENABLE - FALSE"
    else
        echo "BUILD FAIL. SECURE_JTAG_ENABLE not defined in ${CONFIG_PATH}/SECURE_JTAG_CONFIG.ini"
        exit 1;
    fi

    ##############################################################
    # ATTACH GFH
    #

    echo ""      
    echo "[ Attach ${MTK_PLATFORM} GFH ]"
    echo "============================================"
    echo " : GFH_INFO             - ${GFH_INFO}"
    echo " : GFH_SEC_KEY          - ${GFH_SEC_KEY}"
    echo " : GFH_ANTI_CLONE       - ${GFH_ANTI_CLONE}"
    echo " : GFH_JTAG_CFG         - ${SECURE_JTAG_GFH}"
    echo " : GFH_PADDING          - ${GFH_PADDING}"
    echo " : GFH_HASH_SIGNATURE   - ${GFH_HASH_SIGNATURE}"

    chmod u+w ${PL_IMAGE}
    mv -f ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cp -f ${GFH_INFO} ${PL_IMAGE}	    
    chmod 777 ${PL_IMAGE}
    cat ${GFH_SEC_KEY} >> ${PL_IMAGE}
    cat ${GFH_ANTI_CLONE} >> ${PL_IMAGE}
    cat ${SECURE_JTAG_GFH} >> ${PL_IMAGE}
    cat ${GFH_PADDING} >> ${PL_IMAGE}
    chmod u+w ${PL_IMAGE} ${PL_IMAGE/%.bin/_NO_GFH.bin}
    cat ${PL_IMAGE/%.bin/_NO_GFH.bin} >> ${PL_IMAGE}
    cat ${GFH_HASH_SIGNATURE} >> ${PL_IMAGE}

    echo ""
    echo "[ Load Configuration ]"
    echo "============================================"
    echo " : CONFIG               - ${CHIP_CONFIG}"
    echo " : RSA KEY              - ${CHIP_KEY}"	
    echo " : AC_K                 - ${CHIP_KEY}"

    ##############################################################
    # PROCESS BOOT LOADER
    #

    chmod 777 ${PBP_TOOL}/PBP

    ${PBP_TOOL}/PBP -m ${CHIP_CONFIG} -i ${CHIP_KEY} ${PL_IMAGE}
    if [ $? -eq 0 ] ; then
        echo "${PBP_TOOL}/PBP pass !!!!"
    else
        echo "===BUILD FAIL. ${PBP_TOOL}/PBP return fail==="
        exit 1;
    fi

}

function s_chip_support () {

    ##############################################################
    # Can Support Secure Chip
    #

    echo ""
    echo "[ Enable Secure Chip Support ]"
    echo "============================================"

    GFH_PATH=${CHIP_CONFIG_PATH}/s/gfh    
    CONFIG_PATH=${CHIP_CONFIG_PATH}/s/cfg
    KEY_PATH=${CHIP_CONFIG_PATH}/s/key
    
    ##############################################################
    # INITIALIZE CONFIG and KEY
    #
    
    CHIP_CONFIG=${CONFIG_PATH}/CHIP_CONFIG.ini
    CHIP_KEY=${KEY_PATH}/CHIP_TEST_KEY.ini
    
    if [ -e ${GFH_PATH}/GFH_CONFIG.ini ]; then
	echo "INI_GFH_GEN=YES"
	s_chip_ini;
    else
	echo "INI_GFH_GEN=NO"
	s_chip_legacy;
    fi
} 

function key_encode () {

    ##############################################################
    # Encode Key
    #

    KEY_ENCODE_TOOL=tools/ke/KeyEncode
    chmod 777 ${KEY_ENCODE_TOOL}
    if [ -e ${KEY_ENCODE_TOOL} ]; then    

        ./${KEY_ENCODE_TOOL} ${PL_IMAGE} ${PRELOADER_OUT}/KEY_ENCODED_PL
        
        if [ $? -eq 0 ] ; then
            echo "${KEY_ENCODE_TOOL} pass !!!!"
        else
            echo "===BUILD FAIL. ${KEY_ENCODE_TOOL} return fail==="
            exit 1;
        fi 
        
        if [ -e ${PRELOADER_OUT}/KEY_ENCODED_PL ]; then    
            rm ${PL_IMAGE}
            mv  ${PRELOADER_OUT}/KEY_ENCODED_PL ${PL_IMAGE}
        fi
    fi
}

function post_process () {

    ##############################################################
    # Binary Secure Postprocessing
    #        

    PBP_TOOL=tools/pbp
    CUSTOM_PATH=${MTK_ROOT_CUSTOM}/${MTK_PROJECT}/security

    if [ -e ${PBP_TOOL}/PBP ]; then
        echo ""
        echo "[ Pre-loader Post Processing ]"
        echo "============================================"

        ##############################################################
        # ENCODE KEY FIRST
        #        
        key_encode;

	echo ""
	echo "[ Load Chip Config. ]"
	echo "============================================"                        
	echo " : MTK_SEC_CHIP_SUPPORT - ${MTK_SEC_CHIP_SUPPORT}"                 

	if [ "${MTK_SEC_CHIP_SUPPORT}" == "no" ]; then
	    CHIP_CONFIG_PATH=${MTK_PATH_PLATFORM}/gfh/default
	    ns_chip;
	elif [ "${MTK_SEC_CHIP_SUPPORT}" == "yes" ]; then
	    CHIP_CONFIG_PATH=${CUSTOM_PATH}/chip_config
	    s_chip_support;
	else
	    echo "BUILD FAIL. MTK_SEC_CHIP_SUPPORT not defined in ProjectConfig.mk"
	    exit 1;
	fi
    fi
}

function dump_build_info () {

    ##############################################################
    # Dump Message
    #

    echo ""          
    echo "============================================"
    echo "${MTK_PROJECT} preloader load"
    echo "${PL_IMAGE} built at"
    echo "time : $(date)"
    echo "img  size : $(stat -c%s "${PL_IMAGE}")" byte
    echo "bss  size : 0x$(readelf -SW "${PL_ELF_IMAGE}" | grep "bss" | awk '{if (NF==11) print $6; else print $7;}')" byte
    echo "dram size : 0x$(readelf -SW "${PL_ELF_IMAGE}" | grep "dram" | awk '{if (NF==11) print $6; else print $7;}')" byte
    echo "============================================"

    PL_ELF_IMAGE=bin/preloader_${MTK_PROJECT}.elf

    chmod a+w ${PL_IMAGE}
    cp -f ${PL_IMAGE} .
	
	if [ -z != ${PL_MODE} ]; then 
		cp -f ${PL_IMAGE}  ${PRELOADER_OUT}/bin/preloader_${MTK_PROJECT}.bin
	fi

}

function copy_binary () {

    ##############################################################
    # Copy Binary to Output Direcory
    #
    copy_to_legacy_download_flash_folder   ${PL_IMAGE}

}


##############################################################
# Main Flow
#
PRELOADER_OUT=../../${MTK_ROOT_OUT}/PRELOADER_OBJ;
build_preloader;
post_process;
dump_build_info;
copy_binary;
