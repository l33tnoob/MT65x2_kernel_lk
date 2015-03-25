/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "EMMODULES"
#include "Modules.h"
#include "RPCClient.h"
#include <cutils/xlog.h>

#include "ModuleBasebandRegDump.h"
#include "ModuleCpuFreqTest.h"
#include "ModuleFB0.h"
#include "ModuleCpuStress.h"
#include "ModuleSensor.h"
#include "ModuleDfo.h"
#include "ModuleMsdc.h"

AFMModules::AFMModules()
{
}
AFMModules::~AFMModules()
{
}

void AFMModules::Execute(int feature_id, RPCClient* msgSender)
{
	switch(feature_id)
	{

	case FUNCTION_EM_BASEBAND:
		ModuleBaseband::BasebandEntry(msgSender);
		break;
	case FUNCTION_EM_CPU_FREQ_TEST_START:
		ModuleCpuFreqTest::StartTest(msgSender);
		break;
	case FUNCTION_EM_CPU_FREQ_TEST_STOP:
		ModuleCpuFreqTest::StopTest(msgSender);
		break;
	case FUNCTION_EM_CPU_FREQ_TEST_CURRENCT:
		ModuleCpuFreqTest::SetCurrent(msgSender);
		break;
	case FUNCTION_EM_FB0_IOCTL:
		ModuleFB0::FB0_IOCTL(msgSender);
		break;
	case FUNCTION_EM_CPUSTRESS_APMCU:
		ModuleCpuStress::ApMcu(msgSender);
		break;
	case FUNCTION_EM_CPUSTRESS_SWCODEC:
		ModuleCpuStress::SwCodec(msgSender);
		break;
	case FUNCTION_EM_CPUSTRESS_BACKUP:
		ModuleCpuStress::BackupRestore(msgSender);
		break;
	case FUNCTION_EM_CPUSTRESS_THERMAL:
		ModuleCpuStress::ThermalUpdate(msgSender);
		break;
	case FUNCTION_EM_SENSOR_DO_CALIBRATION:
		ModuleSensor::doCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_CLEAR_CALIBRATION:
		ModuleSensor::clearCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_SET_THRESHOLD:
		ModuleSensor::setThreshold(msgSender);
		break;
	case FUNCTION_EM_SENSOR_DO_GSENSOR_CALIBRATION:
		ModuleSensor::doGsensorCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_GET_GSENSOR_CALIBRATION:
		ModuleSensor::getGsensorCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_CLEAR_GSENSOR_CALIBRATION:
		ModuleSensor::clearGsensorCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_DO_GYROSCOPE_CALIBRATION:
		ModuleSensor::doGyroscopeCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_GET_GYROSCOPE_CALIBRATION:
		ModuleSensor::getGyroscopeCalibration(msgSender);
		break;
	case FUNCTION_EM_SENSOR_CLEAR_GYROSCOPE_CALIBRATION:
		ModuleSensor::clearGyroscopeCalibration(msgSender);
		break;
#ifdef EM_DFO_SUPPORT
	case FUNCTION_EM_DFO_INIT:
		ModuleDfo::init(msgSender);
		break;
	case FUNCTION_EM_DFO_READ_COUNT:
		ModuleDfo::readCount(msgSender);
		break;
	case FUNCTION_EM_DFO_READ:
		ModuleDfo::read(msgSender);
		break;
	case FUNCTION_EM_DFO_WRITE:
		ModuleDfo::write(msgSender);
		break;
	case FUNCTION_EM_DFO_DEINIT:
		ModuleDfo::deinit(msgSender);
		break;
	case FUNCTION_EM_DFO_PROPERTY_SET:
		ModuleDfo::propertySet(msgSender);
		break;
	case FUNCTION_EM_DFO_GET_DEFAULT_SIZE:
		ModuleDfo::getDefaultSize(msgSender);
		break;
#endif
    case FUNCTION_EM_MSDC_SET_CURRENT:
        ModuleMsdc::setCurrent(msgSender);
        break;
    case FUNCTION_EM_MSDC_GET_CURRENT:
        ModuleMsdc::getCurrent(msgSender);
        break;
    case FUNCTION_EM_MSDC_SET30_MODE:
        ModuleMsdc::setSd30Mode(msgSender);
        break;
	default:
		msgSender->PostMsg((char*)"This Feature Is Not Supported.");
		XLOGE("unsupported feature id = %d", feature_id);
		break;
	}

	return;
}
