#ifndef RESOURCE_LOCK_H
#define RESOURCE_LOCK_H
//-----------------------------------------------------------------------------
class ResourceLock
{
    public:
        enum ECamAdapter
        {
            eMTK_NONE,
            eMTKCAM_IDLE,
            eMTKPHOTO_PRV,
            eMTKPHOTO_CAP,
            eMTKVIDEO_PRV,
            eMTKVIDEO_REC,
            eMTKVIDEO_VSS,
            eMTKZSDNCC_PRV,
            eMTKZSDNCC_CAP,
            eMTKZSDCC_PRV,
            eMTKZSDCC_CAP,
            eMTK_VT,
            eMTK_ATV,
        };
    //
    protected:
        virtual ~ResourceLock() {};
    //
    public:
        static ResourceLock* CreateInstance(void);
        virtual MVOID       DestroyInstance(void) = 0;
        virtual MBOOL       Init(void) = 0;
        virtual MBOOL       Uninit(void) = 0;
        virtual MBOOL       SetMode(ECamAdapter type) = 0;
        virtual MBOOL       Lock(
            ECamAdapter     Type,
            MUINT32         Timeout = 3000) = 0;
        virtual MBOOL       Unlock(ECamAdapter type) = 0;
};
//-----------------------------------------------------------------------------
#endif

