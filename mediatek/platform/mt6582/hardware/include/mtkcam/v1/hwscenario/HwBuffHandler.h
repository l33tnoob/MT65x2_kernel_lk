#ifndef HW_BUFF_HANDLER_H
#define HW_BUFF_HANDLER_H

/******************************************************************************
 *
 ******************************************************************************/
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <mtkcam/common.h>
#include <mtkcam/drv/imem_drv.h>
//
#include <vector>
using namespace std;
//
/******************************************************************************
 *
 ******************************************************************************/
class HwBuffer : public IImgBuf
{

/******************************************************************************
*   Inheritance from IMemBuf.
*******************************************************************************/
public:
    virtual int64_t             getTimestamp()              const       { return mTimestamp; }
    virtual void                setTimestamp(int64_t const timestamp)   { mTimestamp = timestamp; }
    virtual size_t              getBufSize()                const       { return mbufInfo.size; }
    virtual void*               getVirAddr()                const       { return (void*)mbufInfo.virtAddr; }
    virtual void*               getPhyAddr()                const       { return (void*)mbufInfo.phyAddr; }
    virtual const char*         getBufName()                const       { return mName; }              
    virtual int                 getIonFd()                  const       { return mbufInfo.memID; }

    
/******************************************************************************
*   Inheritance from IImgBuf.
*******************************************************************************/
public:     
  
   //pass from construction   
    virtual uint32_t            getImgWidth()               const       { return mWidth;  }
    virtual uint32_t            getImgHeight()              const       { return mHeight; }
    virtual String8 const&      getImgFormat()              const       { return ms8format; }

    //query by camera format
    virtual uint32_t            getImgWidthStride(uint_t const uPlaneIndex = 0) const     
    { 
        return FmtUtils::queryImgWidthStride(ms8format.string(), getImgWidth(), uPlaneIndex);   
    }
    virtual uint32_t            getBitsPerPixel()           const       { return mBpp; }

/******************************************************************************
*   Initialization.
*******************************************************************************/
public:
    HwBuffer( uint32_t _w = 0, 
              uint32_t _h = 0, 
              const char* _format = "",
              const char* _name = "")
        : IImgBuf()
        , mWidth(_w)
        , mHeight(_h)
        , ms8format(String8(_format))
        , mBpp(FmtUtils::queryBitsPerPixel(_format))
        , mbufSize(FmtUtils::queryImgBufferSize(_format, mWidth, mHeight))
        , mTimestamp(0)
        , mName(_name)
        , mpIMemDrv(NULL)
    {
        createBuffer();
    }

    virtual                     ~HwBuffer()                             {  destroyBuffer(); }
    bool                        operator!()                 const       { return mbufInfo.virtAddr != 0; }

/******************************************************************************
*   Initialization.
*******************************************************************************/    
private:    
    virtual void                createBuffer();   
    virtual void                destroyBuffer();    
    
private:
    uint32_t mWidth;
    uint32_t mHeight;
    String8  ms8format;    
    uint32_t mBpp;
    uint32_t mbufSize;
    int64_t  mTimestamp;
    const char* mName;
    IMemDrv *mpIMemDrv;
    IMEM_BUF_INFO mbufInfo;
};


/******************************************************************************
* 
*******************************************************************************/
class HwBuffProvider : public RefBase
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Initialization.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    
                    HwBuffProvider();
    virtual        ~HwBuffProvider();
    bool            operator!()                             const       { return mvHwBufQ.size()!=0; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual void    addBuf(sp<HwBuffer> const &buf);
    virtual void    removeBuf();
    virtual void    deque(sp<IImgBuf> &buf);
    virtual void    enque(sp<IImgBuf> const &buf);
    
  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
private:    
    sp<IImgBuf>const&   getBuf(int i)                       const       { return mvHwBufQ.at(i); }
    int                 getBufSize()                        const       { return mvHwBufQ.size(); }

private:
    int32_t                mbufCnt;
    Mutex                  mIdxMtx;
    vector< sp<IImgBuf> > mvHwBufQ;     //original bag
    vector< sp<IImgBuf> > mvDequedBuf;  //(1)hold sp, (2)can do de/enque during the time
};


#endif
