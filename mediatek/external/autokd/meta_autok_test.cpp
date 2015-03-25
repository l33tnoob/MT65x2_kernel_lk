//#include <stdio.h> 
//#include <stdarg.h>
//#include <string.h>
//#include <stdlib.h>
#include <cstdio>
#include "meta_autok_para.h"
//#include <fcntl.h>
//#include <unistd.h>
//#include <sys/ioctl.h>
//#include <sys/stat.h>
//#include <linux/rtc.h>
//#include <sys/mman.h>
//#include <utils/Log.h>
#include "autok.h"

unsigned int g_autok_vcore[] = {1187500, 1237500, 1281250};
int VCORE_NO = (sizeof(g_autok_vcore)/sizeof(unsigned int));

#if 0
int test_only()
{
    char *p_data;
    int offset;
    int result = 0;
    int i, j;
    unsigned int *test_pattern;
    struct autok_predata test_predata;
    int PARAM_NO = 0;
    PARAM_NO = get_param_count();
    
    test_pattern = (unsigned int*)malloc(sizeof(unsigned int)*PARAM_NO*VCORE_NO);
    for(i=0; i<PARAM_NO*VCORE_NO; i++){
        test_pattern[i] = i%3;    
    }    
    result = pack_param(&test_predata, g_autok_vcore, VCORE_NO, test_pattern, PARAM_NO);
    if(result != 0)
        return -1;

    //offset = serilize_predata(&test_predata, &p_data);
    //printf("PDATA:%s\n", p_data);
    //result = set_node_data(STAGE2_DEVNODE, p_data, offset);
    set_stage2(2, &test_predata);
    set_debug(1);
    printf("debug:%d", get_debug());
    set_debug(0);
    printf("debug:%d", get_debug());
    //get_param(STAGE2_DEVNODE);
    get_stage2(2);
terminate: 
    //free(test_predata);
    free(p_data);
    return result;
    //close(fd_wr);
}
#endif

int main(int argc, char *argv[])
{
    int result = 0;
    printf("test only\n");
    printf("test only\n");    
    
    /*g_nv_buf = (unsigned char*)malloc(sizeof(ap_nvram_sdio_config_struct));
    if((result = sdio_read_nvram(g_nv_buf))!=0)
        return result;
    write_file_to_nvram("/data/autok_2_1187500", 2);
    write_file_to_nvram("/data/autok_2_1237500", 2);
    int length;
    unsigned char *temp_str;
    read_from_nvram(2, 1237500, &temp_str, &length);
    */
    
    /*int i;
    printf("length:%d\n", length);
    for(i=0; i<length; i++){
        printf("%02x", temp_str[i]);    
    }
    printf("\n");
    */
    //free(temp_str);
    init_autok_nvram();
    
    autok_flow();
    close_nvram();
	return result;
}
