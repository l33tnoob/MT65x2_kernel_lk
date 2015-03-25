#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_MTK_PATH:= mediatek/frameworks/base/voicecommand/cfg/

ifeq ($(strip $(MTK_VOICE_UI_SUPPORT)),yes)
    PRODUCT_COPY_FILES += \
       $(LOCAL_MTK_PATH)/command/alarm/1/0.ogg:system/etc/voicecommand/command/alarm/1/0.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/1/1.ogg:system/etc/voicecommand/command/alarm/1/1.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/1/2.ogg:system/etc/voicecommand/command/alarm/1/2.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/1/3.ogg:system/etc/voicecommand/command/alarm/1/3.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/2/0.ogg:system/etc/voicecommand/command/alarm/2/0.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/2/1.ogg:system/etc/voicecommand/command/alarm/2/1.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/2/2.ogg:system/etc/voicecommand/command/alarm/2/2.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/2/3.ogg:system/etc/voicecommand/command/alarm/2/3.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/3/0.ogg:system/etc/voicecommand/command/alarm/3/0.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/3/1.ogg:system/etc/voicecommand/command/alarm/3/1.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/3/2.ogg:system/etc/voicecommand/command/alarm/3/2.ogg \
       $(LOCAL_MTK_PATH)/command/alarm/3/3.ogg:system/etc/voicecommand/command/alarm/3/3.ogg \
       $(LOCAL_MTK_PATH)/command/camera/1/0.ogg:system/etc/voicecommand/command/camera/1/0.ogg \
       $(LOCAL_MTK_PATH)/command/camera/1/1.ogg:system/etc/voicecommand/command/camera/1/1.ogg \
       $(LOCAL_MTK_PATH)/command/camera/2/0.ogg:system/etc/voicecommand/command/camera/2/0.ogg \
       $(LOCAL_MTK_PATH)/command/camera/2/1.ogg:system/etc/voicecommand/command/camera/2/1.ogg \
       $(LOCAL_MTK_PATH)/command/camera/3/0.ogg:system/etc/voicecommand/command/camera/3/0.ogg \
       $(LOCAL_MTK_PATH)/command/camera/3/1.ogg:system/etc/voicecommand/command/camera/3/1.ogg \
       $(LOCAL_MTK_PATH)/command/phone/1/0.ogg:system/etc/voicecommand/command/phone/1/0.ogg \
       $(LOCAL_MTK_PATH)/command/phone/1/1.ogg:system/etc/voicecommand/command/phone/1/1.ogg \
       $(LOCAL_MTK_PATH)/command/phone/2/0.ogg:system/etc/voicecommand/command/phone/2/0.ogg \
       $(LOCAL_MTK_PATH)/command/phone/2/1.ogg:system/etc/voicecommand/command/phone/2/1.ogg \
       $(LOCAL_MTK_PATH)/command/phone/3/0.ogg:system/etc/voicecommand/command/phone/3/0.ogg \
       $(LOCAL_MTK_PATH)/command/phone/3/1.ogg:system/etc/voicecommand/command/phone/3/1.ogg
endif

ifeq ($(strip $(MTK_VOICE_UNLOCK_SUPPORT)),yes)
    PRODUCT_COPY_FILES += \
       $(LOCAL_MTK_PATH)/64.dat:system/etc/voicecommand/training/ubmfile/64.dat \
       $(LOCAL_MTK_PATH)/128.dat:system/etc/voicecommand/training/ubmfile/128.dat
endif

ifeq ($(strip $(MTK_VOICE_UI_SUPPORT)),yes)
    PRODUCT_COPY_FILES += \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/a_c_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/a_c_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/a_c_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/a_c_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/a_c_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/a_c_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/a_c_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/a_c_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/c_c_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/c_c_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/c_c_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/c_c_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/c_c_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/c_c_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/c_c_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/c_c_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/p_c_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/p_c_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/p_c_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/p_c_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/p_c_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/p_c_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Mandarin/p_c_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/Chinese-Mandarin/p_c_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/a_t_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/a_t_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/a_t_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/a_t_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/a_t_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/a_t_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/a_t_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/a_t_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/c_t_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/c_t_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/c_t_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/c_t_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/c_t_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/c_t_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/c_t_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/c_t_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/p_t_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/p_t_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/p_t_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/p_t_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/p_t_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/p_t_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/Chinese-Taiwan/p_t_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/Chinese-Taiwan/p_t_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/English/a_e_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/English/a_e_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/English/a_e_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/English/a_e_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/English/a_e_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/English/a_e_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/English/a_e_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/English/a_e_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/English/c_e_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/English/c_e_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/English/c_e_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/English/c_e_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/English/c_e_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/English/c_e_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/English/c_e_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/English/c_e_dic.bin.p \
       $(LOCAL_MTK_PATH)/voicepattern/English/p_e_dic.bin.f:system/etc/voicecommand/voiceui/uipattern/English/p_e_dic.bin.f \
       $(LOCAL_MTK_PATH)/voicepattern/English/p_e_dic.bin.l:system/etc/voicecommand/voiceui/uipattern/English/p_e_dic.bin.l \
       $(LOCAL_MTK_PATH)/voicepattern/English/p_e_dic.bin.t:system/etc/voicecommand/voiceui/uipattern/English/p_e_dic.bin.t \
       $(LOCAL_MTK_PATH)/voicepattern/English/p_e_dic.bin.p:system/etc/voicecommand/voiceui/uipattern/English/p_e_dic.bin.p \
       $(LOCAL_MTK_PATH)/modefile/commandfilr3.dic:system/etc/voicecommand/voiceui/modefile/commandfilr3.dic  \
       $(LOCAL_MTK_PATH)/modefile/commandfilr2.dic:system/etc/voicecommand/voiceui/modefile/commandfilr2.dic  \
       $(LOCAL_MTK_PATH)/modefile/commandfilr.dic:system/etc/voicecommand/voiceui/modefile/commandfilr.dic  \
       $(LOCAL_MTK_PATH)/modefile/GMMModel1.bin:system/etc/voicecommand/voiceui/modefile/GMMModel1.bin  \
       $(LOCAL_MTK_PATH)/modefile/GMMModel2.bin:system/etc/voicecommand/voiceui/modefile/GMMModel2.bin  \
       $(LOCAL_MTK_PATH)/modefile/GMMModel3.bin:system/etc/voicecommand/voiceui/modefile/GMMModel3.bin  \
       $(LOCAL_MTK_PATH)/modefile/Model1.bin:system/etc/voicecommand/voiceui/modefile/Model1.bin  \
       $(LOCAL_MTK_PATH)/modefile/Model2.bin:system/etc/voicecommand/voiceui/modefile/Model2.bin  \
       $(LOCAL_MTK_PATH)/modefile/Model3.bin:system/etc/voicecommand/voiceui/modefile/Model3.bin  \
       $(LOCAL_MTK_PATH)/modefile/Model4.bin:system/etc/voicecommand/voiceui/modefile/Model4.bin  \
       $(LOCAL_MTK_PATH)/modefile/Model5.bin:system/etc/voicecommand/voiceui/modefile/Model5.bin  \
       $(LOCAL_MTK_PATH)/modefile/FModel1.bin:system/etc/voicecommand/voiceui/modefile/FModel1.bin  \
       $(LOCAL_MTK_PATH)/modefile/FModel2.bin:system/etc/voicecommand/voiceui/modefile/FModel2.bin  \
       $(LOCAL_MTK_PATH)/modefile/FModel3.bin:system/etc/voicecommand/voiceui/modefile/FModel3.bin  \
       $(LOCAL_MTK_PATH)/modefile/Model_M_gmmfea39d.dat:system/etc/voicecommand/voiceui/modefile/Model_M_gmmfea39d.dat
endif

ifeq ($(strip $(MTK_VOICE_CONTACT_SEARCH_SUPPORT)),yes)
PRODUCT_COPY_FILES += \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/commandfilr5.dic:system/etc/voicecommand/voicecontacts/modefile/commandfilr5.dic  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/commandfilr4.dic:system/etc/voicecommand/voicecontacts/modefile/commandfilr4.dic  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/commandfilr3.dic:system/etc/voicecommand/voicecontacts/modefile/commandfilr3.dic  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/commandfilr2.dic:system/etc/voicecommand/voicecontacts/modefile/commandfilr2.dic  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/commandfilr.dic:system/etc/voicecommand/voicecontacts/modefile/commandfilr.dic  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/GMMModel1.bin:system/etc/voicecommand/voicecontacts/modefile/GMMModel1.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/GMMModel2.bin:system/etc/voicecommand/voicecontacts/modefile/GMMModel2.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/GMMModel3.bin:system/etc/voicecommand/voicecontacts/modefile/GMMModel3.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/Model1.bin:system/etc/voicecommand/voicecontacts/modefile/Model1.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/Model2.bin:system/etc/voicecommand/voicecontacts/modefile/Model2.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/Model3.bin:system/etc/voicecommand/voicecontacts/modefile/Model3.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/Model4.bin:system/etc/voicecommand/voicecontacts/modefile/Model4.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/Model5.bin:system/etc/voicecommand/voicecontacts/modefile/Model5.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/FModel1.bin:system/etc/voicecommand/voicecontacts/modefile/FModel1.bin  \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/Model_M_gmmfea39d.dat:system/etc/voicecommand/voicecontacts/modefile/Model_M_gmmfea39d.dat \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/allInOne.dic:system/etc/voicecommand/voicecontacts/modefile/allInOne.dic \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/metaData:system/etc/voicecommand/voicecontacts/modefile/metaData \
       $(LOCAL_MTK_PATH)/voicecontacts/modefile/rcdSet.lst:system/etc/voicecommand/voicecontacts/modefile/rcdSet.lst
endif
