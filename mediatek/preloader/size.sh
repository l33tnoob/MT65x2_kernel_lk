# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
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



if [ -f "${PL_IMG}" ]; then
  PL_SIZE=$(stat -c%s "${PL_IMG}")
  if [ ${PL_SIZE} -gt 75000 ]; then
    echo "===================== building 'fail' ========================"
    echo "---------------------------------------------------------------"
    echo "image size : ${PL_SIZE} cannot be greater than 74000 bytes !!"
    echo "please reduce your code size first then compile again !!"
    echo "==============================================================="
    size `ls out/*.o` > pl-code-size-report.txt
    echo "---------------------------------------------------------------"
    echo "                      CODE SIZE REPORT                         "
    echo "---------------------------------------------------------------"
    echo "size{bytes}     file  { size > 2500 bytes }"
    echo "---------------------------------------------------------------"
    awk '$prj ~ /^[0-9]/ { if {$4>2500} print $4 "\t\t" $6}' < pl-code-size-report.txt | sort -rn
    rm ${PL_IMG}
    echo "BUILD FAIL !!!!!!!!!!!!!!!!"
    echo "BUILD FAIL !!!!!!!!!!!!!!!!"
    exit 1;
  fi
  if [ -f "${MTK_PATH_PLATFORM}/gfh_info.txt" ]; then
    echo ""
    echo "Attach GFH ... "
    echo "----------------------"
    chmod 777 bin/${PL_IMG_NAME}.bin
    mv bin/${PL_IMG_NAME}.bin bin/${PL_IMG_NAME}_NO_GFH.bin
    chmod 777 ${MTK_PATH_PLATFORM}/gfh_info.txt
    cp ${MTK_PATH_PLATFORM}/gfh_info.txt bin/${PL_IMG_NAME}.bin
    cat bin/${PL_IMG_NAME}_NO_GFH.bin >> bin/${PL_IMG_NAME}.bin
    cat ${MTK_PATH_PLATFORM}/gfh_hash.txt >> bin/${PL_IMG_NAME}.bin
    echo ""
    echo "Sign Pre-loader ... "
    echo "----------------------"
    /usr/local/wine-1.1.33-i686/bin/wine tools/PBP/PBP.exe bin/${PL_IMG_NAME}.bin
  fi
  if [ ! -d ${IMG_DIR} ];   then mkdir ${IMG_DIR};   fi
  if [ ! -d ${FLASH_DIR} ]; then mkdir ${FLASH_DIR}; fi
  echo ""
  echo ============================================
  echo "${_BOARD} preloader load"
  echo "'bin/${PL_IMG_NAME}.bin' built at"
  echo "time : ${date}"
  echo ============================================
  chmod 777 bin/${PL_IMG_NAME}.bin
  chmod 777 ${PL_IMG_NAME}.bin
  cp bin/${PL_IMG_NAME}.bin ${PL_IMG_NAME}.bin
  rm -rf ${PL_DOWNLOAD_IMG}
  chmod 777 ${PL_IMG_NAME}.bin
  cp ${PL_IMG_NAME}.bin ${PL_DOWNLOAD_IMG}
fi

