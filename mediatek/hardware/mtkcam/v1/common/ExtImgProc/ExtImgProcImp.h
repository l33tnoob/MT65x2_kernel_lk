#ifndef EXT_IMG_PROC_IMP_H
#define EXT_IMG_PROC_IMP_H
//-----------------------------------------------------------------------------
class ExtImgProcImp : public ExtImgProc
{
    protected:
        ExtImgProcImp();
        virtual ~ExtImgProcImp();
    //
    public:
        static ExtImgProc*  getInstance(void);
        virtual void        destroyInstance(void);
        //
        virtual MBOOL       init(void);
        virtual MBOOL       uninit(void);
        virtual MUINT32     getImgMask(void);
        virtual MBOOL       doImgProc(ImgInfo& img);
   //
   private:
        mutable Mutex   mLock;
        volatile MINT32 mUser;
        MUINT32         mImgMask;
};
//-----------------------------------------------------------------------------
#endif

