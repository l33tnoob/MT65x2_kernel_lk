#ifndef _CFG_PS_CALI_D_H
#define _CFG_PS_CALI_D_H
//BEGIN<ps cali><add ps cali lid><20131216>yinhuiyog
typedef struct
{
	int ps_cali_data[2];
	int reserved_r[126];
}CUSTOM_PS_CALI_DATA;
#define CFG_CUSTOM_FILE_PS_CALI_SIZE sizeof(CUSTOM_PS_CALI_DATA)
#define CFG_CUSTOM_FILE_PS_CALI_TOTAL 1
//END<ps cali><add ps cali lid><20131216>yinhuiyog
#endif
