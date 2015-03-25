#ifndef RESOURCE_LOCK_IMP_H
#define RESOURCE_LOCK_IMP_H
//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------
class ResourceLockImp : public ResourceLock
{
    protected:
        ResourceLockImp();
        virtual ~ResourceLockImp();
    //
    public:
        static ResourceLock* GetInstance(void);
        virtual MVOID       DestroyInstance(void);
        virtual MBOOL       Init(void);
        virtual MBOOL       Uninit(void);
        virtual MBOOL       SetMode(ECamAdapter type);
        virtual MBOOL       Lock(
            ECamAdapter     Type,
            MUINT32         Timeout = 1000);
        virtual MBOOL       Unlock(ECamAdapter type);
        //
    private:
        virtual MBOOL       GetResMgr(
            ResourceLock::ECamAdapter   Type,
            RES_MGR_DRV_MODE_STRUCT&    Dst);
        virtual MBOOL       GetPipeMgr(
            ResourceLock::ECamAdapter   Type,
            MUINT32&                    Mode);
     private:
        PipeMgrDrv* mpPipeMgr;
        ResMgrDrv*  mpResMgr;
        //
        mutable Mutex mLock;
        volatile MINT32 mUser;
};
//-----------------------------------------------------------------------------
#endif

