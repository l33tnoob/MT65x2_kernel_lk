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

package com.mediatek.apst.target.data.proxy;

import android.database.Cursor;

import com.mediatek.android.content.AsyncCursorParser;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;

import java.nio.ByteBuffer;

public class FastCursorParser extends AsyncCursorParser {
    private static final int DEFAULT_BUFFER_SIZE = Global.DEFAULT_BUFFER_SIZE;
    private byte[] mBlock;
    private ByteBuffer mBuffer;
    private int mBlockSize;
    private boolean mBufferFull;
    private IRawBlockConsumer mConsumer;

    /**
     * @param cursor
     *            The cursor to parse .
     * @param consumer
     *            The consumer to initial mConsumer.
     * @param buffer
     *            The buffer to initial mBuffer.
     */
    public FastCursorParser(Cursor cursor, IRawBlockConsumer consumer,
            ByteBuffer buffer) {
        super(cursor);
        this.mConsumer = consumer;
        this.mBuffer = buffer;
        if (null == mBuffer) {
            Debugger.logW(new Object[] { cursor, consumer, buffer },
                    "ByteBuffer is null. Auto allocate " + DEFAULT_BUFFER_SIZE
                            + " bytes now.");
            this.mBuffer = ByteBuffer.allocate(DEFAULT_BUFFER_SIZE);
        }
    }

    /**
     * @param cursor
     *            The cursor to parse.
     * @param consumer The consumer to handle the block.
     */
    public FastCursorParser(Cursor cursor, IRawBlockConsumer consumer) {
        this(cursor, consumer, ByteBuffer.allocate(DEFAULT_BUFFER_SIZE));
    }

    /**
     * prepare new block.
     */
    private void prepareNewBlock() {
        // Clear the buffer for reading next block
        mBuffer.clear();
        // The first 4 bytes representing items count in this block(block size).
        // So reserve 4 bytes first, and we will set the right value later.
        mBuffer.putInt(0);
        // Reset block size
        mBlockSize = 0;
        mBufferFull = false;
    }

    /* (non-Javadoc)
     * @see com.mediatek.android.content.AsyncCursorParser#onParseStart()
     */
    @Override
    protected void onParseStart() {
        prepareNewBlock();
    }

    /* (non-Javadoc)
     * @see com.mediatek.android.content.AsyncCursorParser#isBlockReady()
     */
    @Override
    public boolean isBlockReady() {
        return (mBufferFull || (getCursorPosition() + 1 == getCount()));
    }

    /**
     * Override it
     * 
     * @param cursor The cursor to parse.
     * @param buffer The save the content.
     * @return IRawBufferWritable.RESULT_FAIL.
     */
    public int onParseCursorToRaw(Cursor cursor, ByteBuffer buffer) {
        return IRawBufferWritable.RESULT_FAIL;
    }

    /* (non-Javadoc)
     * @see com.mediatek.android.content.AsyncCursorParser#onNewRow(android.database.Cursor)
     */
    @Override
    protected void onNewRow(Cursor c) {
        if (c != null) {
            int status = onParseCursorToRaw(c, mBuffer);
            switch (status) {
            case IRawBufferWritable.RESULT_SUCCESS:
                mBlockSize++;
                break;

            case IRawBufferWritable.RESULT_NOT_ENOUGH_BUFFER:
                mBufferFull = true;
                // TODO Catch exception?
                // Move backwards to reread it next time
                c.move(-1);
                break;

            case IRawBufferWritable.RESULT_FAIL:
                break;

            default:
                break;
            }
        }
    }

    /* (non-Javadoc)
     * @see com.mediatek.android.content.AsyncCursorParser#onBlockReady()
     */
    @Override
    protected void onBlockReady() {
        // Set the first 4 bytes of the buffer with the right value now
        mBuffer.putInt(0, mBlockSize);
        // Flip buffer for reading
        mBuffer.flip();
        // Generate the block
        mBlock = new byte[mBuffer.limit()];
        mBuffer.get(mBlock);
        // Pass block to consumer
        Debugger.logD("Block " + (getCursorPosition() + 1) + " ready!");
        consume(mBlock, getCursorPosition() + 1, getCount());
        // Do reset work for new block
        prepareNewBlock();
    }

    /* (non-Javadoc)
     * @see com.mediatek.android.content.AsyncCursorParser#onBlockReadyForEx()
     */
    @Override
    protected void onBlockReadyForEx() {
        // Set the first 4 bytes of the buffer with the right value now
        mBuffer.putInt(0, mBlockSize);
        // Flip buffer for reading
        mBuffer.flip();
        // Generate the block
        mBlock = new byte[mBuffer.limit()];
        mBuffer.get(mBlock);
        // Pass block to consumer
        Debugger.logD("Block " + (getCursorPosition() + 1) + " ready!");
        consume(mBlock, getCursorPosition() + 1, getCursorPosition() + 1);
        // Do reset work for new block
        prepareNewBlock();
    }

    /* (non-Javadoc)
     * @see com.mediatek.android.content.AsyncCursorParser#onParseOver()
     */
    @Override
    protected void onParseOver() {
        if (0 == getCount()) {
            // If total is 0, send a empty block
            byte[] emptyBlock = new byte[4];
            consume(emptyBlock, 0, 0);
        }
        mBuffer = null;
    }

    /**
     * @param block
     * @param progress
     * @param total
     */
    private void consume(byte[] block, int progress, int total) {
        if (null != mConsumer) {
            mConsumer.consume(block, progress, total);
        } else {
            Debugger.logE(new Object[] { block, progress, total },
                    "Consumer is null!");
        }
    }
}
