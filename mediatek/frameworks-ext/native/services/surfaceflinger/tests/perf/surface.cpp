/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "test-perf"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <cutils/properties.h>

#include <ui/DisplayInfo.h>

#include "transaction.h"

using namespace android;


#define MAX_FRAME_COUNT		(32)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


enum {
	wms_transaction,
	wms_orientation,
	transaction,
};

enum {
	tr_size,
	tr_position,
	tr_matrix,
	tr_alpha,
};

struct arg_opt {
	const char *name;
	int val;
};

static void usage(void)
{
	printf(
		"Usage: test-perf [options]\n\n"
		"Options:\n"
		" -m | --mode\t\t\t[transaction (default)]\n"
		" -t | --transaction_mode\t[size/position/matrix/alpha]\n"
		" -c | --transaction_complexity\t[layer count (1~32, default:1)]\n"
		" -d | --delay\t\t\t[delay time between frame refresh (unit:ms, default:0)]\n"
		" -s | --sync\t\t\t[sync for transaction]\n\n"
	);
}

int main(int argc, char** argv)
{
	int mode = transaction;
	int transaction_mode = -1;
	int layer_cnt = 1;
	int delay = 0;
	bool sync = false;
	unsigned int i;

	/* analyze the arguments */
	if (argc <= 1) {
		/* no argument */
		usage();
		return 0;
	}

	/* options structures */
	const char shortOptions[] = "m:t:c:d:s";
	const struct option longOptions[] = {
		{"mode", required_argument, NULL, 'm'},
		{"transaction_mode", required_argument, NULL, 't'},
		{"transaction_complexity", required_argument, NULL, 'c'},
		{"delay", required_argument, NULL, 'd'},
		{"sync", no_argument, NULL, 's'},
		{0, 0, 0, 0}
	};

	/* mode options parse */
	const struct arg_opt mode_opts[] = {
		{"transaction", transaction},
	};

	/* transaction mode options parse */
	const struct arg_opt transaction_mode_opts[] = {
		{"size", tr_size},
		{"position", tr_position},
		{"matrix", tr_matrix},
		{"alpha", tr_alpha},
	};

	while (1) {
		int c = getopt_long(argc, argv, shortOptions, longOptions, NULL);

		if (-1 == c)
			break;

		switch (c) {
		case 0:
			break;

		case 'm':
			for (i = 0; i < ARRAY_SIZE(mode_opts); i++) {
				if (!strcmp(optarg, mode_opts[i].name)) {
					mode = mode_opts[i].val;
					break;
				}
			}

			if (i >= ARRAY_SIZE(mode_opts)) {
				usage();
				return 0;
			}

			break;

		case 't':
			for (i = 0; i < ARRAY_SIZE(transaction_mode_opts); i++) {
				if (!strcmp(optarg, transaction_mode_opts[i].name)) {
					transaction_mode = transaction_mode_opts[i].val;
					break;
				}
			}

			if (i >= ARRAY_SIZE(transaction_mode_opts)) {
				usage();
				return 0;
			}

			break;

		case 'c':
			layer_cnt = atoi(optarg);
			if ((layer_cnt <= 0) || (layer_cnt > MAX_FRAME_COUNT)) {
				usage();
				return 0;
			}

			break;

		case 'd':
			delay = atoi(optarg);
			if (delay < 0) {
				usage();
				return 0;
			}

			break;

		case 's':
			sync = true;
			break;

		default:
			usage();
			return 0;
		}
	}

	/* check and print input test parameters */
	switch (mode) {
	case transaction:
		for (i = 0; i < ARRAY_SIZE(transaction_mode_opts); i++) {
			if (transaction_mode == transaction_mode_opts[i].val) {
				break;
			}
		}

		if (i >= ARRAY_SIZE(transaction_mode_opts)) {
			usage();
			return 0;
		}

		printf("mode:transaction[%s] complexity:%d  delay:%dms\n",
				transaction_mode_opts[i].name, layer_cnt, delay);

		break;

	default:
		usage();
		return 0;
	}

    /* set up the thread-pool */
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

	/* create a client to surfaceflinger */
	sp<SurfaceComposerClient> client = new SurfaceComposerClient();
	DisplayInfo dinfo;
	sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
			ISurfaceComposer::eDisplayIdMain);
	SurfaceComposerClient::getDisplayInfo(display, &dinfo);
	uint32_t dispw = dinfo.w;
	uint32_t disph = dinfo.h;

	/* create backgound surface */
	sp<SurfaceControl> bg_surfaceControl = client->createSurface(
			String8("test-bg-surface"), dispw, disph, PIXEL_FORMAT_RGBX_8888);

	sp<Surface> bg_surface = bg_surfaceControl->getSurface();

	/* set background layer z-order */
	SurfaceComposerClient::openGlobalTransaction();
	bg_surfaceControl->setLayer(BG_LAYER_ORDER);
	SurfaceComposerClient::closeGlobalTransaction();

	/* clear background layer black */
    ANativeWindow_Buffer anwb;
	bg_surface->lock(&anwb, NULL);
	ssize_t bpr = anwb.stride * bytesPerPixel(anwb.format);
	android_memset32((uint32_t*)anwb.bits, 0xFF000000, bpr * anwb.height);
	bg_surface->unlockAndPost();

	struct TestParamInput param;
	param.layerCount = layer_cnt;
	param.dispWidth = dispw;
	param.dispHeight = disph;

	sp<TransactionTest> testSurfaces[MAX_FRAME_COUNT];

	/* start to test */
	switch (mode) {
	case transaction:
		switch (transaction_mode) {
		case tr_size:
			for (int layer = 0; layer < layer_cnt; layer++) {
				param.index = layer;
				testSurfaces[layer] = new SizeTest(client, param);
			}
			break;
		case tr_position:
			for (int layer = 0; layer < layer_cnt; layer++) {
				param.index = layer;
				testSurfaces[layer] = new PositionTest(client, param);
			}
			break;
		case tr_matrix:
			for (int layer = 0; layer < layer_cnt; layer++) {
				param.index = layer;
				testSurfaces[layer] = new MatrixTest(client, param);
			}
			break;
		case tr_alpha:
			for (int layer = 0; layer < layer_cnt; layer++) {
				param.index = layer;
				testSurfaces[layer] = new AlphaTest(client, param);
			}
			break;
		default:
			return 0;
		}
		break;

	default:
		return 0;
	}

	/* initialize each surface */
	for (int layer = 0; layer < layer_cnt; layer++) {
		if (testSurfaces[layer]->Init()) {
			printf("Init for surface%d failed\n", layer);
			return 0;
		}
	}

	printf("Press [CTRL+C] to stop test.\n");

	while (1) {
		SurfaceComposerClient::openGlobalTransaction();
		for (int layer = 0; layer < layer_cnt; layer++) {
			testSurfaces[layer]->Test();
		}
		SurfaceComposerClient::closeGlobalTransaction(sync);

		if (delay > 0)
			usleep(delay * 1000);
	}

	IPCThreadState::self()->joinThreadPool();

    return 0;
}
