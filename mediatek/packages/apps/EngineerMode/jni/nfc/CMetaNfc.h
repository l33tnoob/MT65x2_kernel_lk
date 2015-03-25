/*
 * CMetaNfc.h
 *
 *  Created on: 2011-7-28
 *      Author: mtk80905
 */

#ifndef CMETANFC_H_
#define CMETANFC_H_

class XLock;
class SyncedElement;
struct NFC_REQ;
struct NFC_CNF;

class CMetaNfc {
	static XLock* mNewObjectLock;
	static SyncedElement** respMap;

	static int getOPIdx(int op);
public:
	static void Init();
	static void DeInit();
	static int SendCommand(NFC_REQ *req, char *peer_buff,
			unsigned short peer_len, /*IN OUT*/NFC_CNF* resp);

	static void NotifyResponse(NFC_CNF* resp) ;
};
#endif /* CMETANFC_H_ */
