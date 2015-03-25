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

#ifndef MTK_PERF_TRANSACTION_TEST_H
#define MTK_PERF_TRANSACTION_TEST_H

#include <android/native_window.h>

#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <utils/RefBase.h>


#define BG_LAYER_ORDER		(200000)
#define START_LAYER_ORDER	(BG_LAYER_ORDER + 1)


namespace android {
// ----------------------------------------------------------------------------

struct TestParamInput {
	int index;
	int layerCount;
	uint32_t dispWidth;
	uint32_t dispHeight;
};

class TransactionTest : public virtual RefBase {
public:
	virtual int Init() = 0;
	virtual void Test() = 0;

protected:
	TransactionTest(const sp<SurfaceComposerClient>& client,
			const struct TestParamInput& input);
	~TransactionTest();

	sp<SurfaceControl> mSurfaceControl;
	sp<Surface> mSurface;

	int mIndex;			/* the index of this test layer */
	int mLayerCount;	/* the total count of test layers */

	uint32_t mDispWidth;
	uint32_t mDispHeight;
};


class SizeTest : public TransactionTest {
public:
	SizeTest(const sp<SurfaceComposerClient>& client,
		 const struct TestParamInput& input);
	~SizeTest();
	virtual int Init();
	virtual void Test();

private:
	uint32_t mWidth;
	uint32_t mHeight;

	uint32_t mMaxWidth;
	uint32_t mMaxHeight;

	const int mSteps;
	int mCurrStep;

	enum {
		SHRINK = 0,
		ENLARGE = 1,
	};

	int mDirection;

	const int mBufWidth;
	const int mBufHeight;

	bool mConnected;
};


class PositionTest : public TransactionTest {
public:
	PositionTest(const sp<SurfaceComposerClient>& client,
		     const struct TestParamInput& input);
	~PositionTest();
	virtual int Init();
	virtual void Test();

private:
	uint32_t mWidth;
	uint32_t mHeight;

	uint32_t mPositionRight;
	uint32_t mPositionBottom;

	uint32_t mCurrX;
	uint32_t mCurrY;

	const int mSteps;
	int mCurrStep;

	enum {
		DIR_UP = 0,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
	};

	int mDirection;
};


class MatrixTest : public TransactionTest {
public:
	MatrixTest(const sp<SurfaceComposerClient>& client,
		   const struct TestParamInput& input);
	~MatrixTest();
	virtual int Init();
	virtual void Test();

private:
	uint32_t mWidth;
	uint32_t mHeight;

	const float mMaxAngle;
	float mAngle;

	const int mSteps;
	int mCurrStep;

	enum {
		COUNTER_CLOCKWISE = 0,
		CLOCKWISE = 1,
	};

	int mDirection;
};


class AlphaTest : public TransactionTest {
public:
	AlphaTest(const sp<SurfaceComposerClient>& client,
		  const struct TestParamInput& input);
	~AlphaTest();
	virtual int Init();
	virtual void Test();

private:
	uint32_t mWidth;
	uint32_t mHeight;

	uint32_t mLeft;
	uint32_t mTop;

	float mAlpha;

	const int mSteps;
	int mCurrStep;

	enum {
		TRANSPARENT = 0,
		OPAQUE = 1,
	};

	int mDirection;
};


// ----------------------------------------------------------------------------
}; // namespace android

#endif
