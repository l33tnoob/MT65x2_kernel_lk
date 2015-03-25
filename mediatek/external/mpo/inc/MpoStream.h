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

#ifndef MPO_STREAM_H
#define MPO_STREAM_H


#include "mpo_type.h"

class MpoStream {
public:
    virtual ~MpoStream(){};
    //write the rest of src Stream in to this stream
    virtual bool skip(int bytes)=0;
    //tell the current location in the stream
    virtual int tell()=0;
    virtual void rewind()=0;
    virtual bool end()=0;
    //go to the end of stream
    virtual bool gotoEnd()=0;
};

class MpoInputStream :public MpoStream{
public:
    virtual ~MpoInputStream(){};

    //change the current two bytes of stream into UINT16,
    //the output depends upon bigEndian property
    virtual int readUint16(bool bigEndian)=0;
    //change the current four bytes of stream into UINT32,
    //the output depends upon bigEndian property
    virtual int readUint32(bool bigEndian)=0;
    //read stream content into buffer
    virtual int read(char* buffer, int size)=0;
    //write the rest of src Stream in to this stream
    virtual bool skip(int bytes)=0;
    //tell the current location in the stream
    virtual int tell()=0;
    virtual void rewind()=0;
    virtual bool end()=0;
    //go to the end of stream
    virtual bool gotoEnd()=0;
};

class MpoFileInputStream: public MpoInputStream {
public:
    MpoFileInputStream(const char* path);
    MpoFileInputStream(const char* path,const char* mode);

    virtual ~MpoFileInputStream();

    //read stream content into buffer
    virtual int read(char* buffer, int size);
    //change the current two bytes of stream into UINT16,
    //the output depends upon bigEndian property
    virtual int readUint16(bool bigEndian);
    //change the current four bytes of stream into UINT32,
    //the output depends upon bigEndian property
    virtual int readUint32(bool bigEndian);
    //write the rest of src Stream in to this stream
    virtual bool skip(int bytes);
    //tell the current location in the stream
    virtual int tell();
    virtual void rewind();
    virtual bool end();
    //go to the end of stream
    virtual bool gotoEnd();
protected:
    
    FILE* mFp;
};

class MpoMemoryInputStream: public MpoInputStream {
public:
    // construct a memory stream by malloc memory
    MpoMemoryInputStream(int length);
    // if copyData is true, the stream makes a private copy of the data
    MpoMemoryInputStream(const void* data, int length, bool copyData = false);

    virtual ~MpoMemoryInputStream();

    //read stream content into buffer
    virtual int read(char* buffer, int size);
    int read(char* buffer, int offset, int size);
    //change the current two bytes of stream into UINT16,
    //the output depends upon bigEndian property
    virtual int readUint16(bool bigEndian);
    //change the current four bytes of stream into UINT32,
    //the output depends upon bigEndian property
    virtual int readUint32(bool bigEndian);
    //write the rest of src Stream in to this stream
    virtual bool skip(int bytes);
    //tell the current location in the stream
    virtual int tell();
    virtual void rewind();
    virtual bool end();
    //go to the end of stream
    virtual bool gotoEnd();
protected:
    char * mBuffer;
    int mPos;
    int mCount;
    bool mWeOwnTheData;
};

class MpoOutputStream :public MpoStream{
public:
    virtual ~MpoOutputStream(){};

    //write content of srcStream to this output Stream
    virtual bool write(MpoInputStream* srcStream,int bufferSize = 4096*16)=0;
    //write buffer into this output stream
    virtual int write(char* buffer, int offset, int size)=0;
    //output the UINT16 current into the stream,
    //the output depends upon bigEndian property
    virtual bool writeUint16(int uint16, bool bigEndian)=0;
    //output the UINT16 current into the stream,
    //the output depends upon bigEndian property
    virtual bool writeUint32(int uint32, bool bigEndian)=0;
    //skip some bytes
    virtual bool skip(int bytes)=0;
    //tell the current location in the stream
    virtual int tell()=0;
    virtual void rewind()=0;
    virtual bool end()=0;
    //go to the end of stream
    virtual bool gotoEnd()=0;
    //flush content of output stream
    virtual bool flush()=0;

    //record current position of stream
    virtual void mark() {};
    //check whether the stream support mark operation
    virtual bool markSupported() {return false;}
    //reset the position of stream to marked position
    virtual void reset() {};
};

class MpoFileOutputStream: public MpoOutputStream {
public:
    MpoFileOutputStream(const char* path,const char* mode);
    virtual ~MpoFileOutputStream();

    //write content of srcStream to this output Stream
    virtual bool write(MpoInputStream* srcStream,int bufferSize = 4096*16);
    //write buffer into this output stream
    virtual int write(char* buffer, int offset, int size);
    //output the UINT16 current into the stream,
    //the output depends upon bigEndian property
    virtual bool writeUint16(int uint16, bool bigEndian);
    //output the UINT16 current into the stream,
    //the output depends upon bigEndian property
    virtual bool writeUint32(int uint32, bool bigEndian);
    //skip some bytes
    virtual bool skip(int bytes);
    //tell the current location in the stream
    virtual int tell();
    virtual void rewind();
    virtual bool end();
    //go to the end of stream
    virtual bool gotoEnd();
    //flush content of output stream
    virtual bool flush();

    //record current position of stream
    virtual void mark();
    //check whether the stream support mark operation
    virtual bool markSupported() {return true;}
    //reset the position of stream to marked position
    virtual void reset();
protected:    
    FILE* mFp;
    //marked position
    int mMarkedPosition;
};

class MpoBufferedOutputStream : public MpoOutputStream {
public:
    //MpoBufferedOutputStream(MpoOutputStream* out);
    MpoBufferedOutputStream(MpoOutputStream* out, int size=8192);
    virtual ~MpoBufferedOutputStream();

    //write content of srcStream to this output Stream
    virtual bool write(MpoInputStream* srcStream,int bufferSize);
    //write buffer into this output stream
    virtual int write(char* buffer, int offset, int size);
    //output the UINT16 current into the stream,
    //the output depends upon bigEndian property
    virtual bool writeUint16(int uint16, bool bigEndian);
    //output the UINT16 current into the stream,
    //the output depends upon bigEndian property
    virtual bool writeUint32(int uint32, bool bigEndian);
    //skip some bytes
    virtual bool skip(int bytes);
    //tell the current location in the stream
    virtual int tell();
    virtual void rewind();
    virtual bool end();
    //go to the end of stream
    virtual bool gotoEnd();
    //flush content of output stream
    virtual bool flush();

    //record current position of stream
    virtual void mark();
    //check whether the stream support mark operation
    virtual bool markSupported();
    //reset the position of stream to marked position
    virtual void reset();
private:
    void flushInternal();
    //void 
protected:
    //target output stream which cached data will be flushed to
    MpoOutputStream* mOutputStream;
    //buffer to hold cached data
    char * mInternalBuf;
    //size of internal buffer
    int mBufSize;
    //count of cached data in Bytes
    int mCount;
};

class MpoMemoryOutputStream: public MpoOutputStream {
public:
    // construct a memory stream by malloc memory
    MpoMemoryOutputStream(int length);
    // if copyData is true, the stream makes a private copy of the data
    MpoMemoryOutputStream(const unsigned char * data, int length, bool copyData = false);
//    MpoMemoryOutputStream(const void * data, int length, bool copyData = false);

    virtual ~MpoMemoryOutputStream();

    //write stream content into buffer
    //virtual int write(char* buffer, int size);
    virtual int write(char* buffer, int offset, int size);
    //write content of srcStream to this output Stream
    virtual bool write(MpoInputStream* srcStream, int bufferSize);
    //change the current two bytes of stream into UINT16,
    //the output depends upon bigEndian property
    virtual bool writeUint16(int uint16, bool bigEndian);
    //change the current four bytes of stream into UINT32,
    //the output depends upon bigEndian property
    virtual bool writeUint32(int uint32, bool bigEndian);
    //skip some bytes
    virtual bool skip(int bytes);
    //tell the current location in the stream
    virtual int tell();
    virtual void rewind();
    virtual bool end();
    //go to the end of stream
    virtual bool gotoEnd();
    //flush content of output stream
    virtual bool flush();

    //record current position of stream
    virtual void mark();
    //check whether the stream support mark operation
    virtual bool markSupported();
    //reset the position of stream to marked position
    virtual void reset();
protected:
    char * mBuffer;
    int mPos;
    int mCount;
    bool mWeOwnTheData;
    //marked position
    int mMarkedPosition;
};

#endif

