#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "meta_common.h"
#include "libnvram.h"
#include "FT_Public.h"
#include "Meta_APEditor_Para.h"
#include "errno.h"


//-----------------------------------------------------------------------------
bool META_Editor_Init(void)
{
	return true;
}
//-----------------------------------------------------------------------------
bool META_Editor_Deinit(void)
{
	return true;
}
//-----------------------------------------------------------------------------
bool META_Editor_ReadFile_OP(FT_AP_Editor_read_req *pReq)
{
	FT_AP_Editor_read_cnf kCnf;
	F_INFO kFileInfo;
	int iNvmRecSize = 0, iReadSize;
	F_ID iFD;
	char* pBuffer = NULL;
	bool IsRead =true;

	memset(&kCnf, 0, sizeof(FT_AP_Editor_read_cnf));
	kCnf.header.id 		= pReq->header.id + 1;
	kCnf.header.token 	= pReq->header.token;
	kCnf.file_idx 		= pReq->file_idx;
	kCnf.para 			= pReq->para;
	
	iFD = NVM_GetFileDesc(pReq->file_idx,&(kFileInfo.i4RecSize),&(kFileInfo.i4RecNum),IsRead);
	if (iFD.iFileDesc == -1) 
    {
		NVRAM_LOG("Error AP_Editor_ReadFile can't open file: file index-%d, %d\n", pReq->file_idx, iNvmRecSize);
		kCnf.status = META_FAILED;
		WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), NULL, 0);
		return false;		
	}
	iNvmRecSize = kFileInfo.i4RecSize;
    
	if (pReq->para > kFileInfo.i4RecNum) 
    {
		NVRAM_LOG("Error AP_Editor_ReadFile para: %d, %d\n", pReq->file_idx, pReq->para);
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), NULL, 0);
		return false;
	}

    /* Open NVRAM realted files */
	pBuffer = (char*)malloc(iNvmRecSize);
	lseek(iFD.iFileDesc, (pReq->para - 1) * iNvmRecSize, SEEK_SET);
	iReadSize=read(iFD.iFileDesc, pBuffer, iNvmRecSize);
	if(iNvmRecSize != iReadSize){
		NVRAM_LOG("Error AP_Editor_ReadFile :Read size not match:iReadSize(%d),iNvmRecSize(%d),error:%s\n",iReadSize,iNvmRecSize,strerror(errno));
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), NULL, 0);
		return false;
	}
		
	NVM_CloseFileDesc(iFD);

	kCnf.read_status = META_STATUS_SUCCESS;
	kCnf.status = META_SUCCESS;

	WriteDataToPC(&kCnf, sizeof(FT_AP_Editor_read_cnf), pBuffer, iNvmRecSize);

	NVRAM_LOG("AP_Editor_ReadFile result: file_idx ~ %d para ~ %d read ~ %d\n", pReq->file_idx, pReq->para, iReadSize);
	free(pBuffer);

	return true;
}
//-----------------------------------------------------------------------------
FT_AP_Editor_write_cnf	META_Editor_WriteFile_OP(
	FT_AP_Editor_write_req *pReq,
    char *peer_buf,
    unsigned short peer_len)
{
	FT_AP_Editor_write_cnf kCnf;
	F_INFO kFileInfo;
	int iNvmRecSize = 0, iWriteSize;
	F_ID iFD;
	bool IsRead = false;

	memset(&kCnf, 0, sizeof(FT_AP_Editor_write_cnf));
	kCnf.file_idx 		= pReq->file_idx;
	kCnf.para 			= pReq->para;
	
	if ((peer_buf == NULL) || (peer_len == 0)) {
		NVRAM_LOG("Error AP_Editor_WriteFile Peer Buffer Error\n");
		kCnf.status = META_FAILED;
		return kCnf;
	}
	
	iFD = NVM_GetFileDesc(pReq->file_idx,&(kFileInfo.i4RecSize),&(kFileInfo.i4RecNum),IsRead);
	if (iFD.iFileDesc == -1) {
		NVRAM_LOG("Error AP_Editor_WriteFile can't open file: file index-%d, %d\n", 
			pReq->file_idx, iNvmRecSize);
		kCnf.status = META_FAILED;
		return kCnf;
	}
	iNvmRecSize = kFileInfo.i4RecSize;
	if ((pReq->para > kFileInfo.i4RecNum) || (peer_len > kFileInfo.i4RecSize)) {
		NVRAM_LOG("Error AP_Editor_WriteFile para: %d, %d, %d\n", pReq->file_idx, pReq->para, peer_len);
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		return kCnf;

	}
	lseek(iFD.iFileDesc, (pReq->para - 1) * iNvmRecSize, SEEK_SET);
	iWriteSize = write(iFD.iFileDesc, peer_buf, iNvmRecSize);
	if(iNvmRecSize != iWriteSize){
		NVRAM_LOG("Error AP_Editor_WriteFile :Write size not match:iWriteSize(%d),iNvmRecSize(%d),error:%s\n",iWriteSize,iNvmRecSize,strerror(errno));
		NVM_CloseFileDesc(iFD);
		kCnf.status = META_FAILED;
		return kCnf;
	}
	NVM_CloseFileDesc(iFD);

	kCnf.write_status = META_STATUS_SUCCESS;
	kCnf.status = META_SUCCESS;

	NVRAM_LOG("AP_Editor_WriteFile result: file_idx-%d para-%d write-%d\n", pReq->file_idx, pReq->para, iWriteSize);
	NVRAM_LOG("AddBackupFileNum Begin");
	NVM_AddBackupFileNum(pReq->file_idx);
	NVRAM_LOG("AddBackupFileNum End");
	return kCnf;
}
//-----------------------------------------------------------------------------
FT_AP_Editor_reset_cnf	META_Editor_ResetFile_OP(FT_AP_Editor_reset_req *pReq)
{
	FT_AP_Editor_reset_cnf kCnf;

	memset(&kCnf, 0, sizeof(FT_AP_Editor_reset_cnf));
	if (!NVM_ResetFileToDefault(pReq->file_idx)) 
    {
		printf("Error AP_Editor_ResetFile\n");
		kCnf.status = META_FAILED;
		return kCnf;
	}

	kCnf.status = META_SUCCESS;
	return kCnf;
}
//-----------------------------------------------------------------------------
FT_AP_Editor_reset_cnf	META_Editor_ResetAllFile_OP(FT_AP_Editor_reset_req *pReq)
{
	int i;
	FT_AP_Editor_reset_cnf kCnf;
	FT_AP_Editor_reset_req kReq;
	
	F_INFO kFileInfo = NVM_ReadFileVerInfo(0);

	memset(&kCnf, 0, sizeof(FT_AP_Editor_reset_cnf));
	memset(&kReq, 0, sizeof(FT_AP_Editor_reset_req));
    
	if ((pReq->file_idx != 0xFCCF) || (pReq->reset_category != 0xFC)) 
    {
		kCnf.status = META_FAILED;
		NVRAM_LOG("Error AP_Editor_ResetAllFile para is wrong - %d", pReq->file_idx);
		return kCnf;
	}

	for (i = 0; i < kFileInfo.i4MaxFileLid; ++i) 
    {
		kReq.file_idx = i;
		kCnf = META_Editor_ResetFile_OP(&kReq);
		if (kCnf.status == META_FAILED) {
			NVRAM_LOG("Error AP_Editor_ResetAllFile: file_idx-%d\n", kReq.file_idx);
			return kCnf;			
		}
	}

	return kCnf;
}
//-----------------------------------------------------------------------------
