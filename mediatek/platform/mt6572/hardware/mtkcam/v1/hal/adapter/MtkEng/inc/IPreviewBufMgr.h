#ifndef I_PRV_BUF_MGR
#define I_PRV_BUF_MGR
//
#include <utils/String8.h>
//
//
namespace android {
namespace NSMtkEngCamAdapter {
//
/******************************************************************************
*
*******************************************************************************/
class IPreviewBufMgrHandler : public virtual RefBase
{
public:    
    virtual                ~IPreviewBufMgrHandler() {}

public:
    virtual bool            dequeBuffer(int ePort, ImgBufQueNode &node) = 0;
    virtual bool            enqueBuffer(ImgBufQueNode const& node)= 0;
    virtual void            allocBuffer(int w, int h, const char* format, int cnt) = 0; 
    virtual void            freeBuffer() = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
class IPreviewBufMgr : public IPreviewBufMgrHandler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Member Enum
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
public:
    enum EBufProvider{
        eBuf_Unknown,
        eBuf_Pass1,
        eBuf_Disp,
        eBuf_AP,
        eBuf_FD,
        eBuf_Rec
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPreviewCmdQueThreadHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    
    
    virtual bool            dequeBuffer(int ePort, ImgBufQueNode &node) = 0;
    virtual bool            enqueBuffer(ImgBufQueNode const& node)= 0;
    virtual void            allocBuffer(int w, int h, const char* format, int cnt) = 0; 
    virtual void            freeBuffer() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
public:

    static IPreviewBufMgr*  createInstance(sp<ImgBufProvidersManager>& rImgBufProvidersMgr);
    virtual void            destroyInstance() = 0;
    virtual                 ~IPreviewBufMgr(){};
};

};  // namespace NSMtkEngCamAdapter
};  // namespace android

#endif
