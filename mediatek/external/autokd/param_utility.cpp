#include <cstdio>
#include <cstring>
#include "autok.h"
#define BUF_LEN     1024

int serilize_predata(struct autok_predata *predata, char **buf)
{
    char pack[BUF_LEN*2] = "";
    char *temp;
    int offset = 0;
    int i, j;
    int width;
    int vol_count = predata->vol_count;
    int param_count = predata->param_count;
    pack[0] = vol_count;
    pack[1] = param_count;
    offset += 2;
    temp = (char*)predata->vol_list;
    width = sizeof(unsigned int)/sizeof(char);
    for(i=0; i<vol_count*width; i++){
        pack[offset+i] = temp[i];
    }
    offset+=vol_count*width;
    width = sizeof(U_AUTOK_INTERFACE_DATA);
    for(i=0; i<vol_count; i++){
        temp = (char*)predata->ai_data[i];
        for(j=0; j<param_count*width; j++){
            pack[offset+j] = temp[j];
        }
        offset += (width * param_count);
    }
    *buf = (char*)malloc(offset*sizeof(char));
    
    memcpy(*buf, pack, offset);
    /*for(i=0; i<offset; i++){
        printf("%02x", pack[i]);    
    }
    printf("\n");
    */
    return offset;
}


// Test Only If we want to gererate a special param to autok_predata
int pack_param(struct autok_predata *test_predata, unsigned int *vol_list, int vol_count, unsigned int *param_list, int param_count)
{
    //int vol_count = VCORE_NO;
    int i, j;
    //int param_count = param_len;
    //test_predata = malloc(sizeof(struct autok_predata));
    U_AUTOK_INTERFACE_DATA **ai_data;
    test_predata->vol_count = vol_count;
    test_predata->param_count = param_count;
    test_predata->vol_list = (unsigned int*)malloc(sizeof(unsigned int)*vol_count);
    for(i=0; i<vol_count; i++){
        test_predata->vol_list[i] = vol_list[i];
    }
    ai_data = (U_AUTOK_INTERFACE_DATA**)malloc(sizeof(U_AUTOK_INTERFACE_DATA*)*vol_count);
    for(i=0; i<vol_count; i++){
        ai_data[i] = (U_AUTOK_INTERFACE_DATA*)malloc(sizeof(U_AUTOK_INTERFACE_DATA)*param_count);
        for(j=0; j<param_count; j++){
            //ai_data[i][j].version = 100;
            //ai_data[i][j].freq = 100;
            //printf("param[%d][%d]=%d\n", i, j, param_list[i*param_count + j]);
            ai_data[i][j].data.sel = param_list[i*param_count + j];
        }
    }
    test_predata->ai_data = ai_data;
    return 0;
}


int get_param_data_from_buf(struct autok_predata *test_predata, char *buf)
{
    int i, j, width;
    int offset;
    int vol_count, param_count;
    offset = 0;
    vol_count = buf[0];
    param_count = buf[1];
    test_predata->vol_count = vol_count;
    test_predata->param_count = param_count;
    offset += 2;
    //printf("vol_count:%d, param_count:%d", vol_count, param_count);
    test_predata->vol_list = (unsigned int*)malloc(vol_count*sizeof(unsigned int));
    width = sizeof(unsigned int)/sizeof(char);
    for(i=0; i<vol_count; i++){
        memcpy((char*)&test_predata->vol_list[i], buf+offset, width);
        offset += width;
    }
    
    test_predata->ai_data = (U_AUTOK_INTERFACE_DATA**)malloc(sizeof(U_AUTOK_INTERFACE_DATA*)*vol_count);
    width = sizeof(U_AUTOK_INTERFACE_DATA)/sizeof(char);
    //return offset;
    for(i=0; i<vol_count; i++){
        test_predata->ai_data[i] = (U_AUTOK_INTERFACE_DATA *)malloc(param_count*width);
        for(j=0; j<param_count; j++){
            memcpy((char*)&test_predata->ai_data[i][j], buf+offset, width);
            //printf("ai_data[%d][%d]:%d\n", i, j, test_predata->ai_data[i][j]);
            offset += width;
        }
    }
    
    return offset;
}
struct autok_predata get_param(char *filename)
{
    char *data_buf;
    int data_count;
    int i;
    //struct autok_predata *predata = (struct autok_predata*)malloc(sizeof(struct autok_predata));
    struct autok_predata predata;
    //char devnode[BUF_LEN]="";
    //snprintf(devnode, BUF_LEN, "%s/%d", filename, id);
    get_node_data(filename, &data_buf, &data_count);
    /*printf("GET_PARAM:%s\n", data_buf);
    for(i=0; i<data_count; i++){
        printf("%02x", data_buf[i]);
    }
    printf("\n");
    */
    if(get_param_data_from_buf(&predata, data_buf)<=0){
        //free(predata);
        //predata = NULL;   
        printf("Predata error\n");
    }
    //printf("[vol:param] = [%d:%d]\n", predata->vol_count, predata->param_count);
EXIT_GET_PARAM:
    free(data_buf);    
    return predata;
}

void release_predata(struct autok_predata *predata)
{
    int i;
    //struct autok_predata *predata = *test_predata;
    free(predata->vol_list);
    for(i=0; i<predata->vol_count; i++){
        free(predata->ai_data[i]);     
    }   
    free(predata->ai_data);
    //free(predata);
}