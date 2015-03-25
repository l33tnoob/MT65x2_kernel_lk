#ifndef __DRM_PLAYREADY_PLUGIN_H__
#define __DRM_PLAYREADY_PLUGIN_H__

#include <stdlib.h>
#include <strings.h>

#include <map>
#include <DrmEngineBase.h>
#include "drmmanager.h"

namespace android {

typedef struct
{   
    DRM_APP_CONTEXT * pdrmAppContext;
    DRM_DECRYPT_CONTEXT * pdrmDecryptContext;	
    DRM_BYTE * pOpaqueBuffer;
    int       u4OpaqueBufferSize;	
    DRM_BYTE * pbRevocationBuffer ;	
}DRM_PLAYREADY_SESSION;

typedef struct PLAYREADY_IVDATA {
    unsigned long long qwInitializationVector;
    unsigned long long qwBlockOffset;
    unsigned long  bByteOffset;
} PLAYREADY_IVDATA;

typedef struct PLAYREADY_ENCINFO{
    char                                  role[100];	
    unsigned int                   dataSize;                  //total enc buffer size	
    unsigned int                   segNum;                  //trunk number	
    PLAYREADY_IVDATA    iv[10];                      //IV data for each trunk
    unsigned int                   offset[10];               //pointer to an integer array, each element describe clear data size
    unsigned int                   length[10];              //pointer to an integer array, each element describe enc data size
    unsigned int                   dstHandle;              //true : dstData is a handle; false : dstData is a buffer;    
}PLAYREADY_ENCINFO;

class DrmPlayreadyPlugIn : public DrmEngineBase {

public:
    DrmPlayreadyPlugIn();
    virtual ~DrmPlayreadyPlugIn();

protected:
    DrmConstraints* onGetConstraints(int uniqueId, const String8* path, int action);

    DrmMetadata* onGetMetadata(int uniqueId, const String8* path);

    status_t onInitialize(int uniqueId);

    status_t onSetOnInfoListener(int uniqueId, const IDrmEngine::OnInfoListener* infoListener);

    status_t onTerminate(int uniqueId);

    bool onCanHandle(int uniqueId, const String8& path);

    DrmInfoStatus* onProcessDrmInfo(int uniqueId, const DrmInfo* drmInfo);

    status_t onSaveRights(int uniqueId, const DrmRights& drmRights,
            const String8& rightsPath, const String8& contentPath);

    DrmInfo* onAcquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest);

    String8 onGetOriginalMimeType(int uniqueId, const String8& path, int fd);

    int onGetDrmObjectType(int uniqueId, const String8& path, const String8& mimeType);

    int onCheckRightsStatus(int uniqueId, const String8& path, int action);

    status_t onConsumeRights(int uniqueId, DecryptHandle* decryptHandle, int action, bool reserve);

    status_t onSetPlaybackStatus(
            int uniqueId, DecryptHandle* decryptHandle, int playbackStatus, int64_t position);

    bool onValidateAction(
            int uniqueId, const String8& path, int action, const ActionDescription& description);

    status_t onRemoveRights(int uniqueId, const String8& path);

    status_t onRemoveAllRights(int uniqueId);

    status_t onOpenConvertSession(int uniqueId, int convertId);

    DrmConvertedStatus* onConvertData(int uniqueId, int convertId, const DrmBuffer* inputData);

    DrmConvertedStatus* onCloseConvertSession(int uniqueId, int convertId);

    DrmSupportInfo* onGetSupportInfo(int uniqueId);

    status_t onOpenDecryptSession(
            int uniqueId, DecryptHandle* decryptHandle, int fd, off64_t offset, off64_t length);

    status_t onOpenDecryptSession(
            int uniqueId, DecryptHandle* decryptHandle,   const char* uri);


    status_t onOpenDecryptSession(
            int uniqueId, DecryptHandle* decryptHandle, const DrmBuffer& buf, const String8& mimeType);

    status_t onCloseDecryptSession(int uniqueId, DecryptHandle* decryptHandle);

    status_t onInitializeDecryptUnit(int uniqueId, DecryptHandle* decryptHandle,
            int decryptUnitId, const DrmBuffer* headerInfo);

    status_t onDecrypt(int uniqueId, DecryptHandle* decryptHandle, int decryptUnitId,
            const DrmBuffer* encBuffer, DrmBuffer** decBuffer, DrmBuffer* IV);

    status_t onFinalizeDecryptUnit(int uniqueId, DecryptHandle* decryptHandle, int decryptUnitId);

    ssize_t onPread(int uniqueId, DecryptHandle* decryptHandle,
            void* buffer, ssize_t numBytes, off64_t offset);

private:
    DecryptHandle* openDecryptSessionImpl();
	    
    DRM_CONST_STRING drmStore;
    static std::map<int, DRM_PLAYREADY_SESSION *> *m_pDrmPlayreadySession;		
};

};

#endif /* __DRM_PLAYREADY_PLUGIN_H__ */

