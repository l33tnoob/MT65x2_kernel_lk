#ifndef _CFG_STS_FILE_H
#define _CFG_STS_FILE_H

typedef struct
{
	unsigned int Array[128];
}File_Sts_Custom_Struct;

#define CFG_FILE_STS_CUSTOM_REC_SIZE    sizeof(File_Sts_Custom_Struct)
#define CFG_FILE_STS_CUSTOM_REC_TOTAL   1

#endif
