/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <utils/Log.h>

#include "srs_types.h"
#include "srs_params.h"

#undef LOG_TAG
#define LOG_TAG "SRS_ProcPA"

namespace android {
	
unsigned int SRS_ParamBlock::CRC_Bank[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D };
	
unsigned int SRS_ParamBlock::CalcCRC(unsigned char* pData, unsigned int dataLen){
	unsigned int outCRC = 0xFFFFFFFF;
	unsigned int i;

	for (i=0; i<dataLen; i++,pData++)	// Standard CRC32 bit mangling...
        outCRC = (outCRC>>8)^CRC_Bank[(outCRC^*pData)&0x000000FF];

	return outCRC;
}

void SRS_ParamBlock::FillPreCalcs(){
	ConfigFlags |= SRS_PBFS_FILLED;
	
	int i, j;
	for (i=0; i<BankCount; i++){
		pBanks[i].PrefixLen = strlen(pBanks[i].pPrefix);
		
		pBanks[i].PrefixCRC = CalcCRC((unsigned char*)pBanks[i].pPrefix, pBanks[i].PrefixLen);
		
		SRS_Param* pPMs = pBanks[i].pParams;
		int tPCount = pBanks[i].ParamCount;
		for (j=0; j<tPCount; j++)
			pPMs[j].KeyCRC = CalcCRC((unsigned char*)pPMs[j].pKey, strlen(pPMs[j].pKey));
	}
}

SRS_Param* SRS_ParamBlock::FindParam(const char* pKey, int& bank, int& param){
	int i, j;
	
	bank = -1;
	param = -1;
	
	if (pKey == NULL) return NULL;
	
	// Parse the key for useful info (bank prefix, indexing, etc)	
	const char* pSeekKey = pKey;
	
	int tKeyLen = 0;
	
	const char* pSub = NULL;
	int tSubLen = 0;
	
	const char* pKeyIdx = NULL;
	int tKeyIdxLen = 0;
	int tKeyIdx = -1;
	
	const char* pSubIdx = NULL;
	int tSubIdxLen = 0;
	int tSubIdx = -1;	
	
	while (*pSeekKey){	// Scan for : or [
		if (*pSeekKey == ':') break;
		if (*pSeekKey == '['){ pSeekKey++; pKeyIdx = pSeekKey; break; }
		tKeyLen++;
		pSeekKey++;
	}
	
	while (*pSeekKey){	// Scan for : or ]
		if (*pSeekKey == ':') break;
		if (*pSeekKey == ']'){ pSeekKey++; break; }
		tKeyIdxLen++;
		pSeekKey++;
	}
	
	while (*pSeekKey){	// Scan for : (any chars after ] will be ignored)
		if (*pSeekKey == ':'){ pSeekKey++; break; }
		pSeekKey++;
	}
	
	pSub = pSeekKey;
	while (*pSeekKey){	// Scan SubKey (and SubKeyIdx if found)
	if (*pSeekKey == '['){ pSeekKey++; pSubIdx = pSeekKey; break; }
		tSubLen++;
		pSeekKey++;
	}
	
	while (*pSeekKey){	// Scan for : or ]
		if (*pSeekKey == ']'){ pSeekKey++; break; }
		tSubIdxLen++;
		pSeekKey++;
	}
	
	if (tKeyLen <= 0) return NULL;
	if (tSubLen <= 0) return NULL;
	
	unsigned int tKeyCRC = CalcCRC((unsigned char*)pKey, tKeyLen);
	unsigned int tSubCRC = CalcCRC((unsigned char*)pSub, tSubLen);
	
	for (i=0; i<BankCount; i++){
		if (tKeyCRC == pBanks[i].PrefixCRC){
			bank = i;	// May be more than 1 bank with the same name...
			
			SRS_Param* pPMs = pBanks[i].pParams;
			int tPCount = pBanks[i].ParamCount;
			bool bMatched = false;	// Did we match (external to loop to clean up ALIAS handling)
			for (j=0; j<tPCount; j++){
				//if (bDoit) SRS_LOG("Searching Param %d", j);
				if (tSubCRC == pPMs[j].KeyCRC) bMatched = true;
				if (bMatched == false) continue;
	
				if (pPMs[j].Type == SRS_PTYP_ALIAS) continue;	// We matched, but it's an alias - so keep going...
				
				param = j;
				return pPMs+j;
			}
		}
	}
	
	bank = -1;
	param = -1;
	return NULL;
}

const char* SRS_ParamBlock::GetParamCache(){
	if ((ConfigFlags&SRS_PBFS_PARAMSCACHED) == SRS_PBFS_PARAMSCACHED)
		return pParamCache;
		
	/*	
	String8 bankSplit, paramSplit, subSplit;
	bankSplit = "^";
	paramSplit = "?";
	subSplit = "&";
	char hold[512];
	
	ParamCache = "";
	int i;
	for (i=0; i<BankCount; i++){
		String8 bankStr;
		
		SRS_ParamBank* pB = pBanks+i;
		
		bankStr = pB->pPrefix;	// Bank Header
		bankStr += subSplit;
		bankStr += pB->pType;
		bankStr += subSplit;
		bankStr += pB->pInfo;
		
		int j;
		for (j=0; j<pB->ParamCount; j++){
			SRS_Param* pP = pB->pParams+j;
			String8 paramStr;
			
			paramStr = pP->pKey;
			paramStr += subSplit;
			paramStr += pP->pName;
			paramStr += subSplit;
			paramStr += pP->pInfo;
			
			sprintf(hold, "&%d&%d&%d", pP->Type, pP->EnumID, pP->Format);
			paramStr += hold;
			
			switch (pP->Format){
			case SRS_PFMT_STATIC: {
				// Statics just use pInfo, which is already sent...
			} break;
			case SRS_PFMT_BOOL: {
				snprintf(hold, sizeof(hold), "&%d", (int)(pP->DefValue));
				paramStr += hold;
			} break;
			case SRS_PFMT_INTARRAY: {
				snprintf(hold, sizeof(hold), "&%d&%d&%d&%s", (int)(pP->DefValue), (int)(pP->Min), (int)(pP->Max), pP->pEnums);	// pEnums is a string for the length of array
				paramStr += hold;
			} break;
			case SRS_PFMT_INT: {
				snprintf(hold, sizeof(hold), "&%d&%d&%d", (int)(pP->DefValue), (int)(pP->Min), (int)(pP->Max));
				paramStr += hold;
			} break;
			case SRS_PFMT_FLOATARRAY: {
				snprintf(hold, sizeof(hold), "&%4.3f&%4.3f&%4.3f&%s", pP->DefValue, pP->Min, pP->Max, pP->pEnums);	// pEnums is a string for the length of array
				paramStr += hold;
			} break;
			case SRS_PFMT_FLOAT: {
				snprintf(hold, sizeof(hold), "&%4.3f&%4.3f&%4.3f", pP->DefValue, pP->Min, pP->Max);
				paramStr += hold;
			} break;
			case SRS_PFMT_ENUM: {
				snprintf(hold, sizeof(hold), "&%d&", (int)(pP->DefValue));
				paramStr += hold;
				paramStr += pP->pEnums;
			} break;
			}

			bankStr += paramSplit;
			bankStr += paramStr;
		}
		
		if (ParamCache == "") ParamCache = bankStr;
		else {
			ParamCache += bankSplit;
			ParamCache += bankStr;
		}
	}*/
	
	pParamCache = NULL;
		
	ConfigFlags |= SRS_PBFS_PARAMSCACHED;
	return pParamCache;
}

// String Parsing Helpers (not hard to do, just cleaner this way)
bool HELP_ParamIn::GetBool(const char* pV){
	if (pV[0] == '1') return true;
	if (pV[0] == 't') return true;
	return false;
}

int HELP_ParamIn::GetInt(const char* pV){
	int tR = 0;
	sscanf(pV, "%d", &tR);
	return tR;
}

float HELP_ParamIn::GetFloat(const char* pV){
	float tR = 0.0f;
	sscanf(pV, "%f", &tR);
	return tR;
}

char HELP_ParamOut::Workspace[512] = { 0 };

const char* HELP_ParamOut::FromBool(bool tV){
	return (tV)?"1":"0"; 
}

const char* HELP_ParamOut::FromInt(int tV){
	snprintf(Workspace, sizeof(Workspace), "%d", tV);
	return Workspace;
}

const char* HELP_ParamOut::FromFloat(float tV){
	snprintf(Workspace, sizeof(Workspace), "%4.3f", tV);
	return Workspace;
}

bool SRS_ParamBlock::ConfigRead(const char* pPath, SRS_ParamSource* pSource, uint32_t typeMask){
	FILE* pF = fopen(pPath, "rb");
	if (pF == NULL) return false;
	
	char data[128*1024];
	int len = fread(data, 1, sizeof(data)-1, pF);
	fclose(pF);
	
	if (len < 1) return false;
	
	data[len] = 0;
	
	SRS_LOG("Reading CFG - %s", pPath);	
	return ConfigParse(data, pSource, typeMask);
}

bool SRS_ParamBlock::ConfigParse(char* data, SRS_ParamSource* pSource, uint32_t typeMask){	
	char* pHead = data;
	char* pNext = data;
	
	while (1){
		char* pEq = NULL;
		char* pCom = NULL;
		int ComHits = 0;
		
		while (*pNext){	// Next Line End... (watch for comment/equal)
			if (*pNext == 0x0A) break;	// Line feed?
			if (*pNext == 0x0D) break;	// ?
			if (pCom == NULL){
				if (*pNext == '/'){ 
					ComHits++;
					if (ComHits == 2) pCom = pNext-1;
				} else ComHits = 0;
				if ((*pNext == '=') && (pEq == NULL)) pEq = pNext;
			}
			pNext++;
		}
		
		bool bAtEnd = false;
		if (*pNext == 0) bAtEnd = true;
		
		char* pTail = pNext;
		if (pCom != NULL) pTail = pCom;
		
		if (pEq != NULL){	// Valid line in format XX = YY?
			char* pKHead = pHead;
			char* pKTail = pEq-1;
			
			while (pKHead < pKTail){	// Key - Left WS Scan...
				if (*pKHead == 0x09){ pKHead++; continue; }
				if (*pKHead == ' '){ pKHead++; continue; }
				break;
			}
			while (pKTail >= pKHead){	// Key - Right WS Scan...
				if (*pKTail == 0x09){ pKTail--; continue; }
				if (*pKTail == 0x0A){ pKTail--; continue; }
				if (*pKTail == 0x0D){ pKTail--; continue; }
				if (*pKTail == ' '){ pKTail--; continue; }
				break;
			}
			
			if (pKTail >= pKHead){	// Valid Key?
				char* pVHead = pEq+1;
				char* pVTail = pTail-1;
			
				while (pVHead < pVTail){	// Value - Left WS Scan...
					if (*pVHead == 0x09){ pVHead++; continue; }
					if (*pVHead == ' '){ pVHead++; continue; }
					break;
				}
				
				while (pVTail >= pVHead){	// Value - Right WS Scan...
					if (*pVTail == 0x09){ pVTail--; continue; }
					if (*pVTail == 0x0A){ pVTail--; continue; }
					if (*pVTail == 0x0D){ pVTail--; continue; }
					if (*pVTail == ' '){ pVTail--; continue; }
					break;
				}
				
				if (pVTail >= pVHead){	// Valid Value?
					pKTail[1] = 0;
					pVTail[1] = 0;
					
					int bankId, paramId;
					SRS_Param* pP = FindParam(pKHead, bankId, paramId);
					if (pP != NULL){
						if ((pP->Type >= SRS_PTYP_CFG) && (typeMask&(1<<pP->Type))){	// CFG is the first writeable param type
	    					pSource->SetParam(this, pSource, bankId, paramId, pVHead);
						}
					} else
						SRS_LOG("Unk KEYVALUE: %s = %s", pKHead, pVHead);
				}
			}
		}

		if (!bAtEnd){
			pNext++;	// Skip other clear lines, etc...
			while (true){
				if (*pNext == 0){ bAtEnd = true; break; }
				if (*pNext == 0x0A){ pNext++; continue; }
				if (*pNext == 0x0D){ pNext++; continue; }
				break;
			}
		}
			
		if (bAtEnd) break;
		pHead = pNext;
	}
	
	return true;
}

void SRS_ParamBlock::ConfigWrite(const char* pPath, SRS_ParamSource* pSource, uint32_t typeMask){
	FILE* pF = fopen(pPath, "wb");
	if (pF == NULL) return;

	char hold[512];
	
	int i, j, k;
	for (i=0; i<BankCount; i++){
		SRS_ParamBank* pB = pBanks+i;
		
		for (j=0; j<pB->ParamCount; j++){
			SRS_Param* pP = pB->pParams+j;
			
			if (typeMask&(1<<pP->Type)) break;	// We'd be written - break the loop early
		}
		
		if (j >= pB->ParamCount) continue;	// No values passed the typeMask
		
		snprintf(hold, sizeof(hold), "//=-=-=-=-=-=-=-=-=-=-=-=-\n//BLOCK: (%s) %s - %s\n//=-=-=-=-=-=-=-=-=-=-=-=-\n", pB->pPrefix, pB->pType, pB->pInfo);
		fwrite(hold, 1, strlen(hold), pF);
		
		for (j=0; j<pB->ParamCount; j++){
			SRS_Param* pP = pB->pParams+j;
			
			if (pP->Type == SRS_PTYP_ALIAS) continue;		// Don't write (later add //ALIAS: line)
			if ((typeMask&(1<<pP->Type)) == 0) continue;	// Don't write this type...
			
			// =-=-=-=-=- ALIAS OUTPUT HANDLING - START -=-=-=-=-= //
			if ((typeMask&(1<<SRS_PTYP_ALIAS)) != 0){
				k = j;			// Scan back into the list for Aliases...
				while (k >= 1){
					SRS_Param* pAl = pB->pParams+(k-1);
					if (pAl->Type != SRS_PTYP_ALIAS) break;
					//SRS_LOG("CFG Out ALIAS: %s - for %s", pAl->pKey, pP->pKey);
					k--;
				}
				
				while (k < j){	// Now scan forward through the Aliases...
					SRS_Param* pAl = pB->pParams+k;
					snprintf(hold, sizeof(hold), "//ALIAS: \t%s:%s\t// %s\n", pB->pPrefix, pAl->pKey, pAl->pName);
					fwrite(hold, 1, strlen(hold), pF);
					k++;
				}
			}
			// =-=-=-=-=- ALIAS OUTPUT HANDLING - END -=-=-=-=-= //
			
			const char* pVal = pSource->GetParam(this, pSource, i, j);
			if (pVal == NULL) continue;
			
			const char* pPfx = "";
			if (pP->Type == SRS_PTYP_INFO) pPfx = "//INFO: ";
			else if (pP->Type == SRS_PTYP_DEBUG) pPfx = "//DEBUG: ";
			

			if (pP->Format == SRS_PFMT_STATIC)
				snprintf(hold, sizeof(hold), "%s\t%s:%s =\t%s\t// %s\n", pPfx, pB->pPrefix, pP->pKey, pVal, pP->pName);
			else if (pP->Format == SRS_PFMT_ENUM)
				snprintf(hold, sizeof(hold), "%s\t%s:%s =\t%s\t// %s - %s (%s)\n", pPfx, pB->pPrefix, pP->pKey, pVal, pP->pName, pP->pInfo, pP->pEnums);
			else
				snprintf(hold, sizeof(hold), "%s\t%s:%s =\t%s\t// %s - %s\n", pPfx, pB->pPrefix, pP->pKey, pVal, pP->pName, pP->pInfo);
			
			fwrite(hold, 1, strlen(hold), pF);
		}
		
		snprintf(hold, sizeof(hold), "\n\n");
		fwrite(hold, 1, strlen(hold), pF);
	}
	
	fclose(pF);
	
	SRS_LOG("Wrote CFG - %s", pPath);
}

};

