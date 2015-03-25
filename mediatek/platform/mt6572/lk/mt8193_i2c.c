/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <platform/mt_typedefs.h>
#include <platform/mt_i2c.h>
#include <platform/mt8193.h>


#define MT8193_CHIP_ADDR                0x3A

#if 0

static U32 _mt8193_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = mt_i2c_write(I2C2, chip, cmdBuffer, cmdBufferLen, 0);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = mt_i2c_read(I2C2, chip, dataBuffer, dataBufferLen, 0);

    dbg_print("[_mt8193_i2c_read] Done\n");

    return ret_code;
}

static U32 _mt8193_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {   return -1;
    }

    //write_data[0] = cmd;
    //write_data[1] = writeData;

    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmdBuffer[cmdIndex];
        cmdIndex++;
        i++;
    }

    while(dataIndex < dataBufferLen)
    {
        write_data[i] = dataBuffer[dataIndex];
        dataIndex++;
        i++;
    }

    /* dump write_data for check */
    for( i=0 ; i < transfer_len ; i++ )
    {
        dbg_print("[mt8193_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = mt_i2c_write(I2C2, chip, write_data, transfer_len, 0);

    dbg_print("[mt8193_i2c_write] Done\n");
    
    return ret_code;
}

u8 mt8193_i2c_read8(u16 addr)
{
    U8 chip_slave_address = MT8193_CHIP_ADDR;
    U8 cmd = addr;
	U32 result_tmp;
	int cmd_len = 1;
	U8 data = 0xFF;
    int data_len = 1;

	cmd = addr;
	result_tmp = _mt8193_i2c_read(MT8193_CHIP_ADDR, cmd, cmd_len, &data, data_len);
	
    return result_tmp;
}

int mt8193_i2c_write8(u16 addr, u8 value)
{
    U8 chip_slave_address = MT8193_CHIP_ADDR;
    U8 cmd = addr;
    int cmd_len = 1;
    U8 data = value;
    int data_len = 1;	
    U32 result_tmp;

    cmd = addr;	

    result_tmp = _mt8193_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //check 
    result_tmp = _mt8193_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
	
	printf("[mt8193_i2c_write] Reg[0x%x]=0x%x\n", addr, data);

	return 0;
}

u16 mt8193_i2c_read16(u16 addr)
{
    U8 chip_slave_address = MT8193_CHIP_ADDR;
    U16 cmd = addr;
	U32 result_tmp;
	int cmd_len = 2;
	U16 data = 0xFFFF;
    int data_len = 2;

	cmd = addr;
	result_tmp = _mt8193_i2c_read(MT8193_CHIP_ADDR, cmd, cmd_len, &data, data_len);
	
    return result_tmp;
}

int mt8193_i2c_write16(u16 addr, u16 value)
{
    U8 chip_slave_address = MT8193_CHIP_ADDR;
    U16 cmd = addr;
    int cmd_len = 2;
    U16 data = value;
    int data_len = 2;	
    U32 result_tmp;

    cmd = addr;	

    result_tmp = _mt8193_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //check 
    result_tmp = _mt8193_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
	
	printf("[mt8193_i2c_write] Reg[0x%x]=0x%x\n", addr, data);

	return 0;
}

u32 mt8193_i2c_read32(u16 addr)
{
    U8 chip_slave_address = MT8193_CHIP_ADDR;
    U16 cmd = addr;
	U32 result_tmp;
	int cmd_len = 2;
	U32 data = 0xFFFFFFFF;
    int data_len = 4;

	cmd = addr;
	result_tmp = _mt8193_i2c_read(MT8193_CHIP_ADDR, cmd, cmd_len, &data, data_len);
	
    return result_tmp;
}

int mt8193_i2c_write32(u16 addr, u32 value)
{
    U8 chip_slave_address = MT8193_CHIP_ADDR;
    U16 cmd = addr;
    int cmd_len = 2;
    U32 data = value;
    int data_len = 4;	
    U32 result_tmp;

    cmd = addr;	

    result_tmp = _mt8193_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //check 
    result_tmp = _mt8193_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
	
	printf("[mt8193_i2c_write] Reg[0x%x]=0x%x\n", addr, data);

	return 0;
}



#endif


u8 mt8193_i2c_read8(u16 addr)
{
    u8 rxBuf[8] = {0};
    u8 lens;
    U32 ret_code = 0;
    u8 data;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        rxBuf[0] = (addr >> 8) & 0xFF;
        lens = 1;
    }
    else // 16 bit : noraml mode
    {
        rxBuf[0] = ( addr >> 8 ) & 0xFF;
        rxBuf[1] = addr & 0xFF;     
        lens = 2;
    }

    ret_code = mt_i2c_write(I2C2, MT8193_CHIP_ADDR, rxBuf, lens, 0);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    lens = 1;
    ret_code = mt_i2c_read(I2C2, MT8193_CHIP_ADDR, rxBuf, lens, 0);
    if (ret_code != I2C_OK)
    {
        return ret_code;
    }
    
    data = rxBuf[0]; //LSB fisrt
    
    return data;
    
}

int mt8193_i2c_write8(u16 addr, u8 data)
{
    u8 buffer[8];
    u8 lens;
    u32 ret_code = 0;
    u32 result_tmp = 0;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = data & 0xFF;
        lens = 2;
    }
    else // 16 bit : noraml mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = addr & 0xFF;
        buffer[2] = data & 0xFF;        
        lens = 3;
    }

    ret_code = mt_i2c_write(I2C2, MT8193_CHIP_ADDR, buffer, lens, 0); // 0:I2C_PATH_NORMAL
    if (ret_code != 0)
    {
        return ret_code;
    }
    
    //check 
    result_tmp = mt8193_i2c_read8(addr);
    
    printf("[mt8193_i2c_write] Reg[0x%x]=0x%x\n", addr, data);

    return 0;
}

u16 mt8193_i2c_read16(u16 addr)
{
    
    u8 rxBuf[8] = {0};
    u8 lens;
    U32 ret_code = 0;
    u16 data;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        rxBuf[0] = (addr >> 8) & 0xFF;
        lens = 1;
    }
    else // 16 bit : noraml mode
    {
        rxBuf[0] = ( addr >> 8 ) & 0xFF;
        rxBuf[1] = addr & 0xFF;     
        lens = 2;
    }

    ret_code = mt_i2c_write(I2C2, MT8193_CHIP_ADDR, rxBuf, lens, 0);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    lens = 2;
    ret_code = mt_i2c_read(I2C2, MT8193_CHIP_ADDR, rxBuf, lens, 0);
    if (ret_code != I2C_OK)
    {
        return ret_code;
    }
    
    data = (rxBuf[1] << 8) | (rxBuf[0]); //LSB fisrt
    
    return data;
    
}

int mt8193_i2c_write16(u16 addr, u16 data)
{
    u8 buffer[8];
    u8 lens;
    u32 ret_code = 0;
    u32 result_tmp = 0;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = (data >> 8) & 0xFF;
        buffer[2] = data & 0xFF;
        lens = 3;
    }
    else // 16 bit : noraml mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = addr & 0xFF;
        buffer[2] = (data >> 8) & 0xFF;
        buffer[3] = data & 0xFF;        
        lens = 4;
    }

    ret_code = mt_i2c_write(I2C2, MT8193_CHIP_ADDR, buffer, lens, 0); // 0:I2C_PATH_NORMAL
    if (ret_code != 0)
    {
        return ret_code;
    }
    
    //check 
    result_tmp = mt8193_i2c_read16(addr);
    
    printf("[mt8193_i2c_write] Reg[0x%x]=0x%x\n", addr, data);

    return 0;
}

u32 mt8193_i2c_read32(u16 addr)
{
    
    u8 rxBuf[8] = {0};
    u8 lens;
    U32 ret_code = 0;
    u32 data;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        rxBuf[0] = (addr >> 8) & 0xFF;
        lens = 1;
    }
    else // 16 bit : noraml mode
    {
        rxBuf[0] = ( addr >> 8 ) & 0xFF;
        rxBuf[1] = addr & 0xFF;     
        lens = 2;
    }

    ret_code = mt_i2c_write(I2C2, MT8193_CHIP_ADDR, rxBuf, lens, 0);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    lens = 4;
    ret_code = mt_i2c_read(I2C2, MT8193_CHIP_ADDR, rxBuf, lens, 0);
    if (ret_code != I2C_OK)
    {
        return ret_code;
    }
    
    data = (rxBuf[3] << 24) | (rxBuf[2] << 16) | (rxBuf[1] << 8) | (rxBuf[0]); //LSB fisrt

    return data;
    
}

int mt8193_i2c_write32(u16 addr, u32 data)
{
    u8 buffer[8];
    u8 lens;
    u32 ret_code = 0;
    u32 result_tmp = 0;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = (data >> 24) & 0xFF;
        buffer[2] = (data >> 16) & 0xFF;
        buffer[3] = (data >> 8) & 0xFF;
        buffer[4] = data & 0xFF;
        lens = 5;
    }
    else // 16 bit : noraml mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = addr & 0xFF;
        buffer[2] = (data >> 24) & 0xFF;
        buffer[3] = (data >> 16) & 0xFF;
        buffer[4] = (data >> 8) & 0xFF;
        buffer[5] = data & 0xFF;        
        lens = 6;
    }

    ret_code = mt_i2c_write(I2C2, MT8193_CHIP_ADDR, buffer, lens, 0); // 0:I2C_PATH_NORMAL
    if (ret_code != 0)
    {
        return ret_code;
    }
    
    //check 
    result_tmp = mt8193_i2c_read32(addr);
    
    printf("[mt8193_i2c_write] Reg[0x%x]=0x%x\n", addr, data);

    return 0;
}



