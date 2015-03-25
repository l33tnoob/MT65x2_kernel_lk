



int readDefaultByNvDrv(NVRAM_CAMERA_STROBE_STRUCT* p)
{
	LogInfo("forceLoadNvramLocal+");
	int err;
	NvramDrvBase* nvDrv = NvramDrvBase::createInstance();
	err = nvDrv->readNvram( DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_DATA_STROBE, p, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	if(err!=0)
		LogInfo("forceLoadNvramLocal MError:");
	nvDrv->destroyInstance();
	LogInfo("forceLoadNvramLocal-");
	return err;
}


int clearNvBuf()
{
	LogInfo("clearNvBuf+");
	FlashMgr* pF;
	pF = FlashMgr::getInstance();
	NVRAM_CAMERA_STROBE_STRUCT* testNv;
	testNv = new NVRAM_CAMERA_STROBE_STRUCT[1];
	int i;
	char* p;
	int sz;
	sz = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	p = (char*)testNv;
	for(i=0;i<sz;i++)
		p[i]=0;
	MUINT32 realSize;
	int e;
	e = pF->cctSetNvdata(p, sz, 0, 0, &realSize);
	delete []testNv;
	return e;
}



int readDefaultByCct(NVRAM_CAMERA_STROBE_STRUCT* pMem)
{
	LogInfo("readDefaultByCct+");
	int sz;
	sz = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	FlashMgr* pF;
	pF = FlashMgr::getInstance();
	int err=0;
	int e;
	MUINT32 realSize;

	e = clearNvBuf();
	if(e!=0)
		LogErr("readDefaultByCct");

	e = pF->cctReadDefaultNvram(0, 0, 0, 0, &realSize);
	LogInfo("cctReadDefaultNvram realSize=%d",realSize);
	if(e!=0)
		LogErr("readDefaultByCct");

	e = pF->cctGetNvdata(0, 0, pMem, sz, &realSize);
	LogInfo("cctReadDefaultNvram realSize=%d",realSize);
	if(e!=0)
		LogErr("readDefaultByCct");
	if(err==0)
		err=e;
	return e;
}

int writeNvram(NVRAM_CAMERA_STROBE_STRUCT* p, const char* f)
{
	LogInfo("writeNvram+");
	LogInfo("writeNvram f=%s", f);
	FILE* fp;
	fp = fopen(f, "wb");
	LogInfo("writeNvram fp=%d", fp);
	if(fp==0)
	{
		LogErr("writeNvram fp=0");
		return -1;
	}

	//LogInfo("writeNvram %d", __LINE__);

	fwrite(p, 1, sizeof(NVRAM_CAMERA_STROBE_STRUCT), fp);
	//LogInfo("writeNvram %d", __LINE__);
	fclose(fp);
	//LogInfo("writeNvram %d", __LINE__);
	return 0;
}

int writeAcdkNvram(ACDK_STROBE_STRUCT* p, const char* f)
{
	LogInfo("writeAcdkNvram+");
	LogInfo("writeAcdkNvram f=%s", f);
	FILE* fp;
	fp = fopen(f, "wb");
	LogInfo("writeAcdkNvram fp=%d", fp);
	if(fp==0)
	{
		LogErr("writeAcdkNvram fp=0");
		return -1;
	}

	//LogInfo("writeNvram %d", __LINE__);

	fwrite(p, 1, sizeof(ACDK_STROBE_STRUCT), fp);
	//LogInfo("writeNvram %d", __LINE__);
	fclose(fp);
	//LogInfo("writeNvram %d", __LINE__);
	return 0;
}


int checkTwoMem(void* pMem, void* pMem2, int sz)
{
	int i;
	unsigned char* p;
	unsigned char* p2;
	p = (unsigned char*)pMem;
	p2 = (unsigned char*)pMem2;
	int err=0;
	for(i=0;i<sz;i++)
	{
		if(p[i]!=p2[i])
		{
			err=-1;
			break;
		}
	}
	if(err!=0)
		LogErr("checkTwoMem");
	return err;
}


int randMem(void* pMem, int sz)
{
	//int t;
	srand(time(0));
	int i;
	unsigned char* p;
	p = (unsigned char*)pMem;
	for(i=0;i<sz;i++)
	{
		p[i] = rand()%256;
	}
	return 0;
}
int AcdkToNvram(ACDK_STROBE_STRUCT* in, NVRAM_CAMERA_STROBE_STRUCT* out);
int NvramToAcdk(NVRAM_CAMERA_STROBE_STRUCT* in, ACDK_STROBE_STRUCT* out);
void testCctNv()
{
	LogInfo("testCctNv +");
	FlashMgr* pFl;
	pFl = FlashMgr::getInstance();
	int e;
	int NvSz;
	NVRAM_CAMERA_STROBE_STRUCT* defNv;
	NVRAM_CAMERA_STROBE_STRUCT* testNv;
	NVRAM_CAMERA_STROBE_STRUCT* goldenNv;
	defNv = new NVRAM_CAMERA_STROBE_STRUCT[1];
	testNv = new NVRAM_CAMERA_STROBE_STRUCT[1];
	goldenNv = new NVRAM_CAMERA_STROBE_STRUCT[1];
	NvSz = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	FlashUtil::createDir("/sdcard/flashdata");
//-----------------------------------------------
LogInfo("testCctNv line=%d",__LINE__);
	e = readDefaultByNvDrv(defNv);
	if(e!=0) LogErr("testCctNv");
	e = readDefaultByCct(testNv);
	if(e!=0) LogErr("testCctNv");
	e = checkTwoMem(defNv, testNv, NvSz);
	if(e!=0) LogErr("testCctNv");
	writeNvram(defNv, "/sdcard/flashdata/flashNv1_def.bin");
	writeNvram(testNv, "/sdcard/flashdata/flashNv1_test.bin");
//-----------------------------------------------
LogInfo("testCctNv line=%d",__LINE__);
	*goldenNv = *defNv;
	goldenNv->tuningPara[0].yTar = 99;
	MUINT32 realSize;
	e = pFl->cctSetNvdata(goldenNv, NvSz, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctGetNvdata(0, 0, testNv, NvSz, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = checkTwoMem(goldenNv, testNv, NvSz);
	if(e!=0) LogErr("testCctNv");
	writeNvram(testNv, "/sdcard/flashdata/flashNv2.bin");
//-----------------------------------------------
LogInfo("testCctNv line=%d",__LINE__);
	*goldenNv = *defNv;
	goldenNv->tuningPara[0].yTar = 105;
	//MINT32 realSize;
	e = pFl->cctSetNvdata(goldenNv, NvSz, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctWriteNvram(0, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = clearNvBuf();
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctReadNvram(0, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctGetNvdata(0, 0, testNv, NvSz, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = checkTwoMem(goldenNv, testNv, NvSz);
	if(e!=0) LogErr("testCctNv");
	writeNvram(goldenNv, "/sdcard/flashdata/flashNv3_golden.bin");
	writeNvram(testNv, "/sdcard/flashdata/flashNv3_test.bin");
//-----------------------------------------------
LogInfo("testCctNv line=%d",__LINE__);
	*goldenNv = *defNv;
	goldenNv->tuningPara[0].yTar = 166;
	//MINT32 realSize;
	e = pFl->cctSetNvdata(goldenNv, NvSz, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	char s[]="/sdcard/flashdata/nvtest.bin";
	e = pFl->cctNvdataToFile(s, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = clearNvBuf();
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctFileToNvdata(s, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctGetNvdata(0, 0, testNv, NvSz, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = checkTwoMem(goldenNv, testNv, NvSz);
	if(e!=0) LogErr("testCctNv");
	writeNvram(testNv, "/sdcard/flashdata/flashNv4.bin");

//-----------------------------------------------
LogInfo("testCctNv line=%d",__LINE__);

    ACDK_STROBE_STRUCT* nv2;
    ACDK_STROBE_STRUCT* nv3;
    nv2 = new ACDK_STROBE_STRUCT[1];
    nv3 = new ACDK_STROBE_STRUCT[1];

/*
    e = pFl->cctSetNvdata(goldenNv, NvSz, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctWriteNvram(0, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	char s5[]="/sdcard/flashdata/flashNv5-w.bin";
	e = pFl->cctNvdataToFile(s5, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	*/
	writeNvram(goldenNv, "/sdcard/flashdata/flashNv5-golden.bin");
    NvramToAcdk(goldenNv, nv2);
    writeAcdkNvram(nv2, "/sdcard/flashdata/flashNv5-goldenAcdk.bin");
    unsigned char *pMem;
    pMem = (unsigned char *)nv2;
    randMem(pMem, sizeof( ACDK_STROBE_STRUCT));
    e = pFl->cctSetNvdataMeta(pMem, sizeof(ACDK_STROBE_STRUCT), 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");




	 //FlashMgr::getInstance()->getNvram(testNv);
//writeNvram(testNv, "/sdcard/flashdata/flashNv5-1.bin");

	e = pFl->cctWriteNvram(0, 0, 0, 0, &realSize);
	if(e!=0) LogErr("testCctNv");
	e = clearNvBuf();
	if(e!=0) LogErr("testCctNv");
	e = pFl->cctReadNvramToPcMeta(0, 0, nv3, sizeof( ACDK_STROBE_STRUCT), &realSize);
	if(e!=0) LogErr("testCctNv");
	e = checkTwoMem(nv2, nv3, sizeof( ACDK_STROBE_STRUCT));
	if(e!=0) LogErr("testCctNv");

	writeAcdkNvram(nv3, "/sdcard/flashdata/flashNv5-test.bin");
	delete []nv2;
	delete []nv3;


//-----------------------------------------------
	delete []defNv;
	delete []testNv;
	delete []goldenNv;
}

/*
ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM
	puParaIn: 0
	u4ParaInLen: 0
	puParaOut:  0
	u4ParaOutLen: 0
	pu4RealParaOutLen : 0
	Description: read the default (in the code) to flash buf.


ACDK_CCT_OP_STROBE_GET_NVDATA
	puParaIn: 0
	u4ParaInLen: 0
	puParaOut:  pMem
	u4ParaOutLen: sizeof(NVRAM_CAMERA_STROBE_STRUCT)
	pu4RealParaOutLen : sizeof(NVRAM_CAMERA_STROBE_STRUCT)
	Description: read flash buf to memory


ACDK_CCT_OP_STROBE_SET_NVDATA
	puParaIn: pMem
	u4ParaInLen: sizeof(NVRAM_CAMERA_STROBE_STRUCT)
	puParaOut:  0
	u4ParaOutLen: 0
	pu4RealParaOutLen : 0
	Description: set flash buf

ACDK_CCT_OP_STROBE_WRITE_NVRAM
	puParaIn: 0
	u4ParaInLen: 0
	puParaOut:  0
	u4ParaOutLen: 0
	pu4RealParaOutLen : 0
	Description: write buf to nvram

ACDK_CCT_OP_STROBE_READ_NVRAM
	puParaIn: 0
	u4ParaInLen: 0
	puParaOut:  0
	u4ParaOutLen: 0
	pu4RealParaOutLen : 0
	Description: read nvram to buf


ACDK_CCT_OP_STROBE_NVDATA_TO_FILE
	puParaIn: fileName
	u4ParaInLen: 0
	puParaOut:  0
	u4ParaOutLen: 0
	pu4RealParaOutLen : 0
	Description: buf to file

ACDK_CCT_OP_STROBE_FILE_TO_NVDATA
	puParaIn: fileName
	u4ParaInLen: 0
	puParaOut:  0
	u4ParaOutLen: 0
	pu4RealParaOutLen : 0
	Description: file to buf



ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META
	puParaIn: 0
	u4ParaInLen: 0
	puParaOut:  pMem
	u4ParaOutLen: sizeof(NVRAM_CAMERA_STROBE_STRUCT)
	pu4RealParaOutLen : sizeof(NVRAM_CAMERA_STROBE_STRUCT)
	Description: read flash buf to memory

	*/