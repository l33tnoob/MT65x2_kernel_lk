#include <DpStream.h>
#include <DpChannel.h>
#include <DpDataType.h>

#ifndef _DP_G2D_STREAM_H_
#define _DP_G2D_STREAM_H_


class DpG2DStream
{
public:
    DpG2DStream();
    ~DpG2DStream();

    // for normal case
    void setSrcBuffer(void* addr, unsigned int size, int type = 0);

    void setSrcConfig(int w, int h, DpColorFormat color, DpRect* ROI = 0);

    // for normal case
    void setDstBuffer(void* addr, unsigned int size, int type = 0);

    void setDstConfig(int w, int h, DpColorFormat color, DpRect* ROI = 0);

    bool invalidate();
    
    // for dump register
    void enableDumpReg(unsigned int flags){mDumpRegFlags = flags;}


private:

    DpStream    *mStream;
    DpChannel   *mChannel;
    DpBufferPool *mSrcPool;
    DpBufferPool *mDstPool;
    DpPortOption *mSrcPort;
    DpPortOption *mDstPort;

    int mSrcBufferId;
    int mDstBufferId;
    
    unsigned int mDumpRegFlags;
};

#endif
