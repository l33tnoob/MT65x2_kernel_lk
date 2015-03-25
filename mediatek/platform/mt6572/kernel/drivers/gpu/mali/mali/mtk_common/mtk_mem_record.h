#ifndef __MTK_MEM_RECORD_H__
#define __MTK_MEM_RECORD_H__

#if defined (__cplusplus)
extern "C" {
#endif

void MTKMemRecordAdd(int i32Bytes);
void MTKMemRecordRemove(int i32Bytes);
int MTKMemRecordInit(void);
void MTKMemRecordDeInit(void);


#if defined (__cplusplus)
}
#endif

#endif	/* __MTK_MEM_RECORD_H__ */

/******************************************************************************
 End of file (mtk_mem_record.h)
******************************************************************************/

