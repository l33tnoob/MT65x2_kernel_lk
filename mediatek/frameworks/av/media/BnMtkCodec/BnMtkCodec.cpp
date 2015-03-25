#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include "BnMtkCodec.h"
#include "CAPEWrapper.h"
#define LOG_TAG "BnMtkCodec"

BnMtkCodec::BnMtkCodec():mCodecId(0)
{
    ALOGD("Ctor");
}

int BnMtkCodec::instantiate()
{
    ALOGD("MtkCodec Service initiate");
    int iresult = defaultServiceManager()->addService(String16("mtk.codecservice"), new BnMtkCodec());
    return iresult;
}

status_t BnMtkCodec::onTransact( uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case CREATE:
            {
                const char *pmime = data.readCString();
                CODECTYPE pcodectype = (CODECTYPE)data.readInt32();
                IMtkCodec *pCodec = fn_CreateCodec(pmime, pcodectype);
                mCodecId++;
                mvCodec.add(mCodecId,pCodec);
                reply->writeInt32(mCodecId);
                ALOGD("Create,type:%s,id:%d",pmime,mCodecId);
                pCodec->Create(data, reply);
                return OK;
            }
            break;
            
        case DESTROY:
            {
                ALOGD("Destroy");    
                int pCodecid = data.readInt32();
                IMtkCodec *pMtkCodec=mvCodec.valueFor(pCodecid);	
                status_t iresult = pMtkCodec->Destroy(data);
                delete pMtkCodec;
		mvCodec.removeItem(pCodecid);
                reply->writeInt32(iresult);
                return OK;
            }
            break;
            
        case INIT:
            {
                ALOGD("Init");
                int pCodecid = data.readInt32();
                IMtkCodec *pMtkCodec=mvCodec.valueFor(pCodecid);	
                status_t iresult = pMtkCodec->Init(data);
                reply->writeInt32(iresult);
                return 0;
            }
            break;
            
        case RESET:
            {
                ALOGD("RESET");
                int pCodecid = data.readInt32();
                IMtkCodec *pMtkCodec=mvCodec.valueFor(pCodecid);	
                status_t iresult = pMtkCodec->Reset(data);
                reply->writeInt32(iresult);
                return 0;
            }
            break;
            
        case DEINIT:
            {
                ALOGD("DEINIT");
                int pCodecid = data.readInt32();
                IMtkCodec *pMtkCodec=mvCodec.valueFor(pCodecid);	
                status_t iresult = pMtkCodec->DeInit(data);
                reply->writeInt32(iresult);
                return 0;
            }
            break;
            
        case DOCODEC:
            {
                ALOGD("Decode");
                int pCodecid = data.readInt32();
                IMtkCodec *pMtkCodec=mvCodec.valueFor(pCodecid);	
                status_t iresult = pMtkCodec->DoCodec(data,reply);
                reply->writeInt32(iresult);
                return 0;
            }
            break;
            
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
              
}

IMtkCodec * BnMtkCodec::fn_CreateCodec(const char *pmime,  CODECTYPE pcodectype)
{
    IMtkCodec *pmtkCodec = NULL;
    if (!strcmp(pmime, "audio/ape") && pcodectype == DECODE)
    {
        pmtkCodec = new CAPEWrapper;
    }
    return pmtkCodec;
}
