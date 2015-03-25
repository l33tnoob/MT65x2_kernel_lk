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

#include <utils/Log.h>
#include <cutils/xlog.h>

#include "./inc/MpoStream.h"


#undef LOG_TAG
#define LOG_TAG "MPO_Stream"


////////////////For MpoFileInputStream/////////////////

MpoFileInputStream::MpoFileInputStream(const char* path)
{
    mFp = fopen(path, "rb");
}

MpoFileInputStream::MpoFileInputStream(const char* path,const char* mode)
{
    mFp = fopen(path, mode);
}

MpoFileInputStream::~MpoFileInputStream()
{
    if (NULL == mFp)
        return;
    fclose(mFp);
    mFp = NULL;
}

int MpoFileInputStream::read(char* buffer, int size)
{
    if (NULL == mFp)
        return 0;
    return fread(buffer, 1, size, mFp);
}

int MpoFileInputStream::readUint16(bool bigEndian)
{
    int ret = 0;
    char buffer[2];
    if (2 != read(buffer,2)) {
        XLOGE("MpoFileInputStream:readUint16 failed to read 2 Bytes from file");
    }
    if (bigEndian) {
        ret = buffer[1] | (buffer[0] << 8);
    } else {
        ret = buffer[0] | (buffer[1] << 8);
    }
    return ret;
}

int MpoFileInputStream::readUint32(bool bigEndian)
{
    int ret = 0;
    char buffer[4];
    if (4 != read(buffer,4)) {
        XLOGE("MpoFileInputStream:readUint32 failed to read 4 Bytes from file");
    }
    if (bigEndian) {
        ret = buffer[3] | (buffer[2] << 8) | (buffer[1] << 16) | (buffer[0] << 24);
    } else {
        ret = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    }
    return ret;
}

bool MpoFileInputStream::skip(int bytes)
{
    if (NULL == mFp)
        return false;
    if (0 != fseek(mFp, bytes, SEEK_CUR))
        return false;
    else
        return true;
}

int MpoFileInputStream::tell()
{
    if (NULL == mFp)
        return -1;
    return ftell(mFp);
}

void MpoFileInputStream::rewind()
{
    if (NULL == mFp)
        return;
    fseek(mFp,0L, SEEK_SET);
}

bool MpoFileInputStream::end()
{
    if (NULL == mFp)
        return true;
    return feof(mFp);
}

bool MpoFileInputStream::gotoEnd()
{
    if (NULL == mFp)
        return false;
    if (0 == fseek(mFp,0L, SEEK_END))
        return true;
    else
        return false;
}

////////////////For MpoMemoryInputStream/////////////////

// construct a memory stream by malloc memory
MpoMemoryInputStream::MpoMemoryInputStream(int length)
:mBuffer(NULL), mPos(0), mCount(0)
{
//temporarily NO NEED! 
}

// if copyData is true, the stream makes a private copy of the data
MpoMemoryInputStream::MpoMemoryInputStream(const void* data,
                                    int length, bool copyData)
:mBuffer(NULL), mPos(0), mCount(0), mWeOwnTheData(false)
{
    if (NULL == data || length <= 0) {
        XLOGE("Failed to construct MpoMemoryInputStream:data=0x%x, length=%d",(unsigned)data,length);
        return;
    }
    //check if we need to malloc memory ourselves
    if (copyData) {
        mBuffer = new char[length];
        if (NULL == mBuffer) {
            XLOGE("malloc memory for %d Bytes failed!",length);
            return;
        }
        memcpy(mBuffer, data, length);
        mWeOwnTheData = true;
    } else {
        //we use data as our buffer
        mBuffer = (char*) data;
    }
    //record buffer size
    mCount = length;
}

MpoMemoryInputStream::~MpoMemoryInputStream()
{
    if (true == mWeOwnTheData) {
        delete mBuffer;
        mBuffer = NULL;
        mWeOwnTheData = false;
    }
    mCount = 0;
    mPos = 0;
}

int MpoMemoryInputStream::read(char* buffer, int size)
{
    return read(buffer, 0, size);
}

int MpoMemoryInputStream::read(char* buffer, int offset, int size)
{
    if (NULL == buffer) {
        XLOGE("buffer to hold data is NULL when read");
        return -1;
    }
    //check interger overflow 
    if ((offset | size) < 0) {
        XLOGE("error Params: offset=%d, size=%d",offset, size);
        return -1;
    }
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when read buffer");
        return -1;
    }
    //check whether Stream is ended
    if (mPos >= mCount) {
        return -1;
    }
    if (0 == size) return 0;
    //calculate byte num that should be copyed
    int copyLen = mCount - mPos < size ?
                  mCount - mPos : size;
    //copy data to buffer
    memcpy(buffer+offset, mBuffer+mPos, copyLen);
    mPos += copyLen;
    return copyLen;
}

int MpoMemoryInputStream::readUint16(bool bigEndian)
{
    int ret = 0;
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when readUint16 buffer");
        return ret;
    }
    if (mCount - mPos < 2) {
        XLOGE("MpoMemoryInputStream:readUint16 failed to read 2 Bytes from buffer");
        return ret;
    }

    if (bigEndian) {
        ret = (mBuffer[mPos] << 8) | mBuffer[mPos+1];
    } else {
        ret = mBuffer[mPos] | (mBuffer[mPos+1] << 8);
    }
    mPos += 2;
    return ret;
}

int MpoMemoryInputStream::readUint32(bool bigEndian)
{
    int ret = 0;
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when readUint32 buffer");
        return ret;
    }
    if (mCount - mPos < 4) {
        XLOGE("MpoMemoryInputStream:readUint16 failed to read 4 Bytes from buffer");
        return ret;
    }
    if (bigEndian) {
        ret = (mBuffer[mPos] << 24) | (mBuffer[mPos+1] << 16) | (mBuffer[mPos+2] << 8) | mBuffer[mPos+3];
    } else {
        ret = mBuffer[mPos] | (mBuffer[mPos+1] << 8) | (mBuffer[mPos+2] << 16) | (mBuffer[mPos+3] << 24);
    }
    mPos += 4;
    return ret;
}

bool MpoMemoryInputStream::skip(int bytes)
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when skip(%d)",bytes);
        return false;
    }
    //move current position
    if (bytes > 0) {
        mPos = mPos + bytes <= mCount ?
               mPos + bytes : mCount;
    } else if (bytes < 0) {
        mPos = mPos + bytes >= 0 ?
               mPos + bytes : 0;
    }
    return true;
}

int MpoMemoryInputStream::tell()
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when tell()");
        return -1;
    }
    return mPos;
}

void MpoMemoryInputStream::rewind()
{
    mPos = 0;
}

bool MpoMemoryInputStream::end()
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when end()");
        return true;
    }
    return mPos == mCount ;
}

bool MpoMemoryInputStream::gotoEnd()
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryInputStream is invalid when gotoEnd()");
        return false;
    }
    mPos = mCount;
    return true;
}

////////////////For MpoFileOutputStream/////////////////

MpoFileOutputStream::MpoFileOutputStream(const char* path,const char* mode)
{
    mFp = fopen(path, mode);
    mMarkedPosition = 0;
}

MpoFileOutputStream::~MpoFileOutputStream()
{
    if (NULL == mFp)
        return;
    fclose(mFp);
    mFp = NULL;
}

int MpoFileOutputStream::write(char* buffer, int offset, int size)
{
    if (NULL == mFp)
        return 0;
    return fwrite(buffer, 1, size, mFp);
}

bool MpoFileOutputStream::write(MpoInputStream* srcStream,int bufferSize)
{
    if (NULL == mFp || NULL == srcStream)
        return false;
    //return fwrite(buffer, 1, size, mFp);
    int srcPos = srcStream->tell();
    //goto the end of JPEG file
    if (false == srcStream->gotoEnd()) {
        XLOGE("goto end of source stream failed");
        return false;
    }
    int endBytePos = srcStream->tell();
    int srcLength = endBytePos - srcPos + 1;
    //return to original postion
    srcStream->rewind();
    if (false == srcStream->skip(srcPos)) {
        XLOGE("MpoFileOutputStream:src stram skip %d Bytes failed",srcPos);
        return false;
    }
    //prepare for buffer
    char * buffer = new char[bufferSize];
    //begin dumping
    while (srcLength > bufferSize) {
        //read content from jpeg source
        if (bufferSize != srcStream->read(buffer,bufferSize)) {
            XLOGE("read content of %d Bytes from src stream failed",bufferSize);
            delete buffer;
            return false;
        }
        //write content to stream
        if (bufferSize != write(buffer, 0, bufferSize)) {
            XLOGE("write content of %d Bytes to stream failed",bufferSize);
            delete buffer;
            return false;
        }
        srcLength -= bufferSize;
    }
    //read rest content from jpeg source
    if ((srcLength-1)  != srcStream->read(buffer,(srcLength-1) )) {
        XLOGE("read content of %d Bytes from src stream failed",(srcLength-1) );
        delete buffer;
        return false;
    }
    //write rest content to stream
    if ((srcLength-1) != write(buffer, 0, (srcLength-1) )) {
        XLOGE("write content of %d Bytes to stream failed",(srcLength-1) );
        delete buffer;
        return false;
    }
    return true;
}

bool MpoFileOutputStream::writeUint16(int uint16, bool bigEndian)
{
    bool ret = false;
    char buffer[2];
    if (bigEndian) {
        buffer[0] = (uint16 >> 8) & 0xFF;
        buffer[1] =  uint16       & 0xFF;
    } else {
        buffer[1] = (uint16 >> 8) & 0xFF;
        buffer[0] =  uint16       & 0xFF;
    }
    if (2 != write(buffer, 0, 2)) {
        XLOGE("MpoFileOutputStream:writeUint16 failed to write 2 Bytes to file");
        ret = false;
    } else {
        ret = true;
    }
    return ret;

}

bool MpoFileOutputStream::writeUint32(int uint32, bool bigEndian)
{
    bool ret = false;
    char buffer[4];
    if (bigEndian) {
        buffer[0] = (uint32 >> 24) & 0xFF;
        buffer[1] = (uint32 >> 16) & 0xFF;
        buffer[2] = (uint32 >>  8) & 0xFF;
        buffer[3] =  uint32        & 0xFF;
    } else {
        //ret = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
        buffer[3] = (uint32 >> 24) & 0xFF;
        buffer[2] = (uint32 >> 16) & 0xFF;
        buffer[1] = (uint32 >>  8) & 0xFF;
        buffer[0] =  uint32        & 0xFF;
    }
    if (4 != write(buffer, 0, 4)) {
        XLOGE("MpoFileOutputStream:writeUint32 failed to write 4 Bytes to file");
        ret = false;
    } else {
        ret = true;
    }
    return ret;
}

bool MpoFileOutputStream::skip(int bytes)
{
    if (NULL == mFp)
        return false;
    if (0 != fseek(mFp, bytes, SEEK_CUR))
        return false;
    else
        return true;
}

int MpoFileOutputStream::tell()
{
    if (NULL == mFp)
        return -1;
    return ftell(mFp);
}

void MpoFileOutputStream::rewind()
{
    if (NULL == mFp)
        return;
    fseek(mFp, 0L, SEEK_SET);
}

bool MpoFileOutputStream::end()
{
    if (NULL == mFp)
        return true;
    return feof(mFp);
}

bool MpoFileOutputStream::gotoEnd()
{
    if (NULL == mFp)
        return false;
    if (0 == fseek(mFp,0L, SEEK_END))
        return true;
    else
        return false;
}

bool MpoFileOutputStream::flush()
{
    if (NULL == mFp)
        return false;
    if (0 == fflush(mFp))
        return true;
    else
        return false;
}

void MpoFileOutputStream::mark()
{
    mMarkedPosition = tell();
}

void MpoFileOutputStream::reset()
{
    if (NULL == mFp)
        return;
    if (mMarkedPosition >= 0)
        fseek(mFp, mMarkedPosition, SEEK_SET);
}

//////////////For MpoBufferedOutputStream //////////////////

MpoBufferedOutputStream::MpoBufferedOutputStream(MpoOutputStream* out, int size)
:mOutputStream(NULL), mInternalBuf(NULL), mBufSize(0), mCount(0)
{
    //record output stream
    if (NULL == out) {
        XLOGE("NULL MpoOutputStream passed in in MpoBufferedOutputStream!");
        return;
    } else {
        mOutputStream = out;
    }

    //malloc internal buffer
    if (size <= 0) {
        XLOGE("Invalid buffer size: %d",size);
        return;
    } else {
        mBufSize = size;
        mInternalBuf = new char[mBufSize];
    }

    //reset internal buffer
    mCount = 0;
}

MpoBufferedOutputStream::~MpoBufferedOutputStream()
{
    //temporarily, we do not release proxy output stream in our code
    //if (NULL != mOutputStream) {
    //    delete mOutputStream;
    //}

    //free internal buffer
    if (NULL != mInternalBuf) {
        delete [] mInternalBuf;
    }

    //reset internal buffer
    mCount = 0;
}

int MpoBufferedOutputStream::write(char* buffer, int offset, int size)
{
    //XLOGD("MpoBufferedOutputStream::write(buffer, offset=%d, size=%d)//mCount=%d,mBufSize=%d",offset,size,mCount,mBufSize);
    //check whether internal buffer is valid
    if (NULL == mInternalBuf) {
        XLOGE("Internal Buffer of MpoBufferedOutputStream is NULL!");
        return 0;
    }

    //check whether source buffer is valid
    if (NULL == buffer) {
        XLOGE("Invalid source buffer!");
        return 0;
    }

    //check whether parameters are valid
    if (offset < 0 || size <= 0) {
        XLOGE("Invalid params:offset=%d, size=%d when writes buffer",offset,size);
        return 0;
    }

    //check whether we need to cache the content of this writing
    if (size >= mBufSize) {
        //if content size is larger than internal buffer size,
        //no need to cache
        //XLOGD("MpoBufferedOutputStream:write, size(%d)>=mBufSize(%d):call flushInternal()..",size,mBufSize);
        flushInternal();
        if (NULL != mOutputStream) {
            return mOutputStream->write(buffer, offset, size);
        } else {
            XLOGE("mOutputStream is NULL when MpoBufferedOutputStream writes buffer!");
            return 0;
        }
    }

    //flush internal buffer contents if we don't have enough space
    if (size >= mBufSize - mCount) {
        //XLOGD("Before internal flust in write, size = %d, mBufSize=%d, mCount=%d)",size,mBufSize,mCount);
        flushInternal();
    }

    //then, the length is always less than empty space in internal buffer,
    //so memory copy is safe.
    memcpy(mInternalBuf+mCount, buffer+offset, size);

    //updates status of this buffered stream
    mCount += size;

    return size;
}

bool MpoBufferedOutputStream::write(MpoInputStream* srcStream,int bufferSize)
{
    //flush internal buffer
    flushInternal();

    if (NULL == mOutputStream || NULL == srcStream)
        return false;
    return mOutputStream->write(srcStream, bufferSize);
}

bool MpoBufferedOutputStream::writeUint16(int uint16, bool bigEndian)
{
    //check whether internal buffer is valid
    if (NULL == mInternalBuf) {
        XLOGE("Internal Buffer of MpoBufferedOutputStream is NULL!");
        return false;
    }

    //flush internal buffer contents if we don't have enough space
    if (2 >= mBufSize - mCount) {
        flushInternal();
    }

    if (bigEndian) {
        mInternalBuf[mCount++] = (uint16 >> 8) & 0xFF;
        mInternalBuf[mCount++] =  uint16       & 0xFF;
    } else {
        mInternalBuf[mCount++] =  uint16       & 0xFF;
        mInternalBuf[mCount++] = (uint16 >> 8) & 0xFF;
    }

    return true;

}

bool MpoBufferedOutputStream::writeUint32(int uint32, bool bigEndian)
{
    //check whether internal buffer is valid
    if (NULL == mInternalBuf) {
        XLOGE("Internal Buffer of MpoBufferedOutputStream is NULL!");
        return false;
    }

    //flush internal buffer contents if we don't have enough space
    if (4 >= mBufSize - mCount) {
        flushInternal();
    }

    if (bigEndian) {
        mInternalBuf[mCount++] = (uint32 >> 24) & 0xFF;
        mInternalBuf[mCount++] = (uint32 >> 16) & 0xFF;
        mInternalBuf[mCount++] = (uint32 >>  8) & 0xFF;
        mInternalBuf[mCount++] =  uint32        & 0xFF;
    } else {
        mInternalBuf[mCount++] =  uint32        & 0xFF;
        mInternalBuf[mCount++] = (uint32 >>  8) & 0xFF;
        mInternalBuf[mCount++] = (uint32 >> 16) & 0xFF;
        mInternalBuf[mCount++] = (uint32 >> 24) & 0xFF;
    }

    return true;
}

bool MpoBufferedOutputStream::skip(int bytes)
{
    //flush internal buffer
    flushInternal();

    if (NULL == mOutputStream) {
        XLOGE("mOutputStream is NULL when MpoBufferedOutputStream skip %d bytes",bytes);
        return false;
    }

    return mOutputStream->skip(bytes);
}

int MpoBufferedOutputStream::tell()
{
    //flush internal buffer
    flushInternal();

    if (NULL == mOutputStream) {
        XLOGE("mOutputStream is NULL when MpoBufferedOutputStream tell");
        return -1;
    }
    return mOutputStream->tell();
}

void MpoBufferedOutputStream::rewind()
{
    //flush internal buffer
    flushInternal();

    if (NULL == mOutputStream) {
        XLOGE("mOutputStream is NULL when MpoBufferedOutputStream rewind");
        return;
    }

    mOutputStream->rewind();
}

bool MpoBufferedOutputStream::end()
{
    //flush internal buffer
    flushInternal();

    if (NULL == mOutputStream) {
        XLOGE("mOutputStream is NULL when MpoBufferedOutputStream end()");
        return true;
    }
    return mOutputStream->end();
}

bool MpoBufferedOutputStream::gotoEnd()
{
    //flush internal buffer
    flushInternal();

    if (NULL == mOutputStream) {
        XLOGE("mOutputStream is NULL when MpoBufferedOutputStream gotoEnd");
        return false;
    }

    return mOutputStream->gotoEnd();
}

void MpoBufferedOutputStream::flushInternal() {
    //XLOGV("MpoBufferedOutputStream:flushInternal()//mCount=%d,mOutputStream=%d,mInternalBuf=%d",mCount,mOutputStream,mInternalBuf);
    if (mCount > 0 && NULL != mOutputStream && NULL != mInternalBuf) {
        //XLOGV("MpoBufferedOutputStream:flushInternal:mOutputStream->write(mInternalBuf, 0, %d",mCount);
        mOutputStream->write(mInternalBuf, 0, mCount);
        mCount = 0;
    }
}

bool MpoBufferedOutputStream::flush() {
    //flush internal buffer
    flushInternal();

    if (NULL != mOutputStream)
        return mOutputStream->flush();
    else {
        XLOGE("mOutputStream is NULL when flush Buffered output stream!");
        return true;
    }
}

void MpoBufferedOutputStream::mark()
{
    //flush internal buffer
    flushInternal();

    if (NULL != mOutputStream)
        mOutputStream->mark();
}

bool MpoBufferedOutputStream::markSupported()
{
    if (NULL != mOutputStream)
        return mOutputStream->markSupported();
    else
        return false;
}

void MpoBufferedOutputStream::reset()
{
    //flush internal buffer
    flushInternal();

    if (NULL != mOutputStream)
        mOutputStream->reset();
}

////////////////For MpoMemoryOutputStream/////////////////

// construct a memory stream by malloc memory
MpoMemoryOutputStream::MpoMemoryOutputStream(int length)
:mBuffer(NULL), mPos(0), mCount(0)
{
//temporarily NO NEED!
}

// if copyData is true, the stream makes a private copy of the data
MpoMemoryOutputStream::MpoMemoryOutputStream(const unsigned char* data, int length, bool copyData)
:mBuffer(NULL), mPos(0), mCount(0), mWeOwnTheData(false)
{
    if (NULL == data || length <= 0) {
        //XLOGE("Failed to construct MpoMemoryOutputStream:data=0x%x, length=%d",data,length);
        return;
    }
    //check if we need to malloc memory ourselves
    if (copyData) {
        mBuffer = new char[length];
        if (NULL == mBuffer) {
            XLOGE("malloc memory for %d Bytes failed!",length);
            return;
        }
        memcpy(mBuffer, data, length);
        mWeOwnTheData = true;
    } else {
        //we use data as our buffer
        mBuffer = (char*) data;
    }
    //record buffer size
    mCount = length;
}

MpoMemoryOutputStream::~MpoMemoryOutputStream()
{
    if (true == mWeOwnTheData) {
        delete mBuffer;
        mBuffer = NULL;
        mWeOwnTheData = false;
    }
    mCount = 0;
    mPos = 0;
}


int MpoMemoryOutputStream::write(char* buffer, int offset, int size)
{
    if (NULL == buffer) {
        XLOGE("buffer to hold data is NULL when write");
        return -1;
    }
    //check interger overflow
    if ((offset | size) < 0) {
        XLOGE("error Params: offset=%d, size=%d",offset, size);
        return -1;
    }
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryOutputStream is invalid when write buffer");
        return -1;
    }
    //check whether Stream is ended
    if (mPos >= mCount) {
        return -1;
    }
    if (0 == size) return 0;
    //calculate byte num that should be written
    int copyLen = mCount - mPos < size ?
                  mCount - mPos : size;
    memcpy(mBuffer+mPos, buffer+offset, copyLen);
    mPos += copyLen;
    return copyLen;
}

bool MpoMemoryOutputStream::write(MpoInputStream* srcStream, int bufferSize)
{
    if (NULL == srcStream)
        return false;
    //return fwrite(buffer, 1, size, mFp);
    int srcPos = srcStream->tell();
    //goto the end of JPEG file
    if (false == srcStream->gotoEnd()) {
        XLOGE("goto end of source stream failed");
        return false;
    }
    int endBytePos = srcStream->tell();
    int srcLength = endBytePos - srcPos + 1;
    srcStream->rewind();
    if (false == srcStream->skip(srcPos)) {
        XLOGE("MpoFileOutputStream:src stram skip %d Bytes failed",srcPos);
        return false;
    }
    //prepare for buffer
    char * buffer = new char[bufferSize];
    //begin dumping
    while (srcLength > bufferSize) {
        //read content from jpeg source
        if (bufferSize != srcStream->read(buffer,bufferSize)) {
            XLOGE("read content of %d Bytes from src stream failed",bufferSize);
            delete mBuffer;
            return false;
        }
        //write content to stream
        if (bufferSize != write(buffer, 0, bufferSize)) {
            XLOGE("write content of %d Bytes to stream failed",bufferSize);
            delete mBuffer;
            return false;
        }
        srcLength -= bufferSize;
    }
    //read rest content from jpeg source
    if ((srcLength-1)  != srcStream->read(mBuffer,(srcLength-1) )) {
        XLOGE("read content of %d Bytes from src stream failed",(srcLength-1) );
        delete mBuffer;
        return false;
    }
    //write rest content to stream
    if ((srcLength-1) != write(mBuffer, 0, (srcLength-1) )) {
        XLOGE("write content of %d Bytes to stream failed",(srcLength-1) );
        delete mBuffer;
        return false;
    }

	return true;
}

bool MpoMemoryOutputStream::writeUint16(int uint16, bool bigEndian)
{
    bool ret = false;
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        //XLOGE("MpoMemoryOutputStream is invalid when writeUint16 buffer");
        return ret;
    }
    if (mCount - mPos < 2) {
        //XLOGE("MpoMemoryOutputStream:writeUint16 failed to write 2 Bytes to buffer");
        return ret;
    }
    if (bigEndian) {
        mBuffer[mPos] = (uint16 >> 8) & 0xFF;
        mBuffer[mPos+1] =  uint16       & 0xFF;
    } else {
        mBuffer[mPos+1] = (uint16 >> 8) & 0xFF;
        mBuffer[mPos] =  uint16       & 0xFF;
    }
    mPos += 2;
    return true;
}

bool MpoMemoryOutputStream::writeUint32(int uint32, bool bigEndian)
{
    bool ret = false;
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        //XLOGE("MpoMemoryOutputStream is invalid when writeUint32 buffer");
        return ret;
    }
    if (mCount - mPos < 4) {
        //XLOGE("MpoMemoryOutputStream:writeUint16 failed to write 4 Bytes to buffer");
        return ret;
    }

    if (bigEndian) {
        mBuffer[mPos] = (uint32 >> 24) & 0xFF;
        mBuffer[mPos+1] = (uint32 >> 16) & 0xFF;
        mBuffer[mPos+2] = (uint32 >>  8) & 0xFF;
        mBuffer[mPos+3] =  uint32        & 0xFF;
    } else {
        mBuffer[mPos+3] = (uint32 >> 24) & 0xFF;
        mBuffer[mPos+2] = (uint32 >> 16) & 0xFF;
        mBuffer[mPos+1] = (uint32 >>  8) & 0xFF;
        mBuffer[mPos] =  uint32        & 0xFF;
    }
    mPos += 4;
    return true;
}

bool MpoMemoryOutputStream::skip(int bytes)
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryOutputStream is invalid when skip(%d)",bytes);
        return false;
    }
    //move current position
    if (bytes > 0) {
        mPos = mPos + bytes <= mCount ?
               mPos + bytes : mCount;
    } else if (bytes < 0) {
        mPos = mPos + bytes >= 0 ?
               mPos + bytes : 0;
    }
    return true;
}

int MpoMemoryOutputStream::tell()
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryOutputStream is invalid when tell()");
        return -1;
    }
    return mPos;
}

void MpoMemoryOutputStream::rewind()
{
    mPos = 0;
}

bool MpoMemoryOutputStream::end()
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryOutputStream is invalid when end()");
        return true;
    }
    return mPos == mCount ;
}

bool MpoMemoryOutputStream::gotoEnd()
{
    //check whether Stream is valid
    if (NULL == mBuffer || mCount <= 0) {
        XLOGE("MpoMemoryOutputStream is invalid when gotoEnd()");
        return false;
    }
    mPos = mCount;
    return true;
}

bool MpoMemoryOutputStream::flush()
{
	return true;
}

void MpoMemoryOutputStream::mark()
{
	if (NULL == mBuffer) {
	    XLOGE("MpoMemoryOutputStream is NULL when mark memory");
            return;
	}
	mMarkedPosition = tell();
}

bool MpoMemoryOutputStream::markSupported()
{
	return true;
}

void MpoMemoryOutputStream::reset()
{
	if (NULL == mBuffer) {
	    XLOGE("MpoMemoryOutputStream is NULL when reset memory");
	    return;
	}
	if (mMarkedPosition >=0) {
            mPos = mMarkedPosition;
	}
}
