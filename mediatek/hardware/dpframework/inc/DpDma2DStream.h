#include <DpStream.h>
#include <DpChannel.h>
#include <DpDataType.h>

#ifndef _DP_DMA2D_STREAM_H_
#define _DP_DMA2D_STREAM_H_

struct DpDmaBufferInfo;

class DpDma2DStream
{
public:
    DpDma2DStream();
    ~DpDma2DStream();

    void setSrcBuffer(void* addr, unsigned int size, unsigned int stride, bool addrIsPhy);
    void setDstBuffer(void* addr, unsigned int size, unsigned int stride, bool addrIsPhy);
    void setCopyArea(unsigned int width, unsigned int height);
    bool execute(void);

private:
    DpDmaBufferInfo *mSrcBufferInfo;
    DpDmaBufferInfo *mDstBufferInfo;
    unsigned int copyWidth;
    unsigned int copyHeight;
};

#endif
