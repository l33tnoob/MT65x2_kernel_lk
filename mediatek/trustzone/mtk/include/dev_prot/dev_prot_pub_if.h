/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _DEV_PROT_PUB_IF_H_
#define _DEV_PROT_PUB_IF_H_

/*
    encrypt data (on SRAM or DRAM) bases on device dependent secret
    input:
        pui1_uid  - user id specified by user, 16 bytes
        pui1_src  - source buffer, dram address
        ui4_sz    - size of source/destination buffer
    output:
        pui1_des  - desitnation buffer, dram address
    return:
        0   - success
        <0  - fail
    note:
        1. slow but data can be in dram/sram
        2. mtk_dev_prot_data_encrypt and mtk_dev_prot_data_decrypt are paired
*/
extern int32_t mtk_dev_prot_data_encrypt
(
uint8_t* pui1_uid,
uint8_t* pui1_src,
uint8_t* pui1_des,
uint32_t ui4_sz
);

/*
    decrypt data (on SRAM or DRAM) bases on device dependent secret
    input:
        pui1_uid  - user id specified by user, 16 bytes
        pui1_src  - source buffer, dram address
        ui4_sz    - size of source/destination buffer
    output:
        pui1_des  - desitnation buffer, dram address
    return:
        0   - success
        <0  - fail
    note:
        1. slow but data can be in dram/sram
        2. mtk_dev_prot_data_encrypt and mtk_dev_prot_data_decrypt are paired
*/
extern int32_t mtk_dev_prot_data_decrypt
(
uint8_t* pui1_uid,
uint8_t* pui1_src,
uint8_t* pui1_des,
uint32_t ui4_sz
);

/*
    generating cmac, bases on device dependent secret, for data
    input:
        pui1_uid  - user id specified by user, 16 bytes
        pui1_src  - source buffer, dram address
        ui4_sz    - size of source buffer
    output:
        pui1_cmac - desitnation buffer, 16 bytes
    return:
        0   - success
        <0  - fail
*/
extern int32_t mtk_dev_prot_data_cmac
(
uint8_t* pui1_uid,
uint8_t* pui1_src,
uint8_t* pui1_cmac,
uint32_t ui4_sz
);

/*
    encrypt data (on DRAM) bases on device dependent secret
    input:
        pui1_uid  - user id specified by user, 16 bytes
        pui1_src  - source buffer, dram address
        ui4_sz    - size of source/destination buffer
    output:
        pui1_des - desitnation buffer, dram address
    return:
        0   - success
        <0  - fail
    note:
        1. fast but data should be in dram
        2. mtk_dev_prot_dram_data_encrypt_md1 and mtk_dev_prot_dram_data_decrypt_md1 are paired
*/
extern int32_t mtk_dev_prot_dram_data_encrypt_md1
(
uint8_t* pui1_uid,
uint8_t* pui1_src,
uint8_t* pui1_des,
uint32_t ui4_sz
);

/*
    decrypt data (on dram) bases on device dependent secret
    fast but data should be in dram
    input:
        pui1_uid  - user id specified by user, DEV_PROT_USER_ID_SZ bytes
        pui1_src  - source buffer, dram address
        ui4_sz    - size of source/destination buffer
    output:
        pui1_des - desitnation buffer, dram address
    return:
        0   - success
        <0  - fail
    note:
        1. fast but data should be in dram
        2. mtk_dev_prot_dram_data_encrypt_md1 and mtk_dev_prot_dram_data_decrypt_md1 are paired
*/
extern int32_t mtk_dev_prot_dram_data_decrypt_md1
(
uint8_t* pui1_uid,
uint8_t* pui1_src,
uint8_t* pui1_des,
uint32_t ui4_sz
);


#endif /* _DEV_PROT_PUB_IF_H_ */