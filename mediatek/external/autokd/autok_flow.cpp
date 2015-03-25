#include <unistd.h>
#include <sys/stat.h>
#include "autok.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <list>
#include <cutils/properties.h>
#define BUF_LEN     1024
#define PROP_VALUE_MAX  92
//#include "property_service.h"
#include "errno.h"
//#include <linux/syswait.h>
extern std::list<struct host_progress*> get_ready();
int is_file_valid(char *fname)
{
    struct stat fstat;
    if (lstat(fname,&fstat)==-1)
        return -ENFILE;
    if (!S_ISREG(fstat.st_mode))
        return -ENOENT;
    if(!fstat.st_size)
        return -ENOSPC;
    return fstat.st_size;
}


int is_nvram_mode()
{
#if 1    
    int get_time = 5;
    char bootmode[32];
    char tmp[PROP_VALUE_MAX];
    while(get_time--!=0 && property_get("ro.bootmode", tmp, NULL)){
        usleep(100000);
    }
    strlcpy(bootmode, tmp, sizeof(bootmode));
    if (!strcmp(bootmode,"factory")){
        return 1;
    }
    return 0;   
#else
    int get_time = 5;
    char bootmode[32];
    char tmp[PROP_VALUE_MAX];
    while(get_time--!=0 && property_get("autok_bootmode", tmp, NULL)){
        usleep(100000);
    }
    strlcpy(bootmode, tmp, sizeof(bootmode));
    if (!strcmp(bootmode,"factory")){
        return 1;
    }
    return 0;
#endif    
}

struct res_data{
    int id;
    unsigned int voltage;
    char filepath[256];  
};

int do_host_autok(int id)
{
    int i, j;
    int param_count;
    int vol_count;
    //std::list<unsigned int> undo_vol_list;
    struct autok_predata predata;
    unsigned int *param_list;
    unsigned int *vol_list;
    struct autok_predata full_data;
    std::list<struct res_data*> res_list;
    std::list<struct res_data*>::iterator it_res;
    struct res_data *p_res;
    char devnode[BUF_LEN]=""; 
    char resnode[BUF_LEN]="";
    char data_node[BUF_LEN]="";
    int need_to_recover = 0;
    param_count = get_param_count();
    
    printf("Param_count:%d\n", param_count);

    if(get_nvram_param_count(id)!=param_count){
        system("rm -rf /data/autok_*");
        system("rm -rf /data/nvram/APCFG/APRDCL/SDIO");
        close_nvram();
        init_autok_nvram();
        need_to_recover = 1;
    }
    //duplicate nvram data partition
    //if(!is_nvram_mode()){
        for(i=0; i<VCORE_NO; i++){
            //char nvram_node[BUF_LEN]="";
            //snprintf(nvram_node, BUF_LEN, "%s/%s_%d_%d", AUTOK_NVRAM_PATH, RESULT_FILE_PREFIX, id, g_autok_vcore[i]);
            snprintf(data_node, BUF_LEN, "%s/%s_%d_%d", AUTOK_RES_PATH, RESULT_FILE_PREFIX, id, g_autok_vcore[i]);
            // Check environment
            // 1. nvram data partition should exist
            // 2. autok parameter in data partion should not exist
            // 3. param_count should be the same 
            if(is_nvram_data_exist(id, g_autok_vcore[i])>0 && is_file_valid(data_node)<=0){
                //data_copy(nvram_node, data_node);
                int data_length;
                char *data_buf;
                read_from_nvram(id, g_autok_vcore[i], (unsigned char**)&data_buf, &data_length);
                if(write_to_file(data_node, data_buf, data_length)<0){
                    free(data_buf);
                    return -1;
                }
                free(data_buf);
                printf("duplicata from [%s] to [%s]\n", AUTOK_NVRAM_PATH, data_node);
            }
        }
    //}
    
    
    //check autok folder
    for(i=0; i<VCORE_NO; i++){
        // [FIXME] Normal mode & factory mode should store to different path
        snprintf(resnode, BUF_LEN, "%s/%s_%d_%d", AUTOK_RES_PATH, RESULT_FILE_PREFIX, id, g_autok_vcore[i]);
        printf("1-0 done\n");
        //if(!is_nvram_mode()){
            //std::string res_str(resnode);
            p_res = (struct res_data*)malloc(sizeof(struct res_data));
            p_res->id = id;
            p_res->voltage = g_autok_vcore[i];
            strcpy(p_res->filepath, resnode);
            res_list.push_back(p_res);
        //} 
        /*else {
            snprintf(resnode, BUF_LEN, "%s/%s_%d_%d", AUTOK_RES_PATH, RESULT_FILE_PREFIX, id, g_autok_vcore[i]);
            write_file_to_nvram(resnode, id);
            
        }*/
        struct timespec tstart={0,0}, tend={0,0};
        printf("1-1 done\n");
        if(is_file_valid(resnode)<=0){
            //undo_vol_list.push_back(g_autok_vcore[i]);
            set_stage1_voltage(id, g_autok_vcore[i]);
            set_stage1_done(id, 0);
            printf("[%s] set done to 0\n", resnode);
            // prepare zero data to drive autok algorithm 
            param_list = (unsigned int*)malloc(sizeof(unsigned int)*param_count);
            for(j=0; j<param_count; j++){
                param_list[j] = 0;        
            }
            vol_count = 1;            
            vol_list = (unsigned int*)malloc(sizeof(unsigned int)*vol_count); 
            vol_list[0] = g_autok_vcore[i];                  
            pack_param(&predata, vol_list, vol_count, param_list, param_count);
            clock_gettime(CLOCK_MONOTONIC, &tstart);
            set_stage1_params(id, &predata);
            
            printf("operation col_count[%d] vcore[%d] param_count[%d]\n", vol_count, vol_list[0], param_count);     
            release_predata(&predata);
            printf("release col_count[%d] param_count[%d]\n", vol_count, param_count); 
            // Wait for autok stage1 for a specific voltage done
            // [FIXME] can switch to uevent?
            while(1){
                if(get_stage1_done(id))
                    break;
                usleep(10*1000); 
            }
            clock_gettime(CLOCK_MONOTONIC, &tend);
            printf("autok once %.5f seconds\n",((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
            set_debug(0);
            snprintf(devnode, BUF_LEN, "%s/%d/%s", STAGE1_DEVNODE, id, "PARAMS");
            if(!is_nvram_mode()){
                printf("[NON_NV]From dev[%s] to res[%s]\n", devnode, resnode);
                from_dev_to_data(devnode, resnode);
                // For recover different version param_count
                if(need_to_recover){
                    write_dev_to_nvram(devnode, id);
                }
            } else {
                printf("[NV]From dev[%s] to nvram\n", devnode);
                from_dev_to_data(devnode, resnode);
                write_dev_to_nvram(devnode, id);
            }
        }
        printf("1-2 done\n");
    }
    
    // Set stage2 data to apply autok parameter
    //if(!is_nvram_mode()){
        // Merge Phase
        full_data.vol_count = VCORE_NO;
        full_data.param_count = param_count;
        full_data.vol_list = (unsigned int*)malloc(sizeof(unsigned int)*VCORE_NO);
        full_data.ai_data = (U_AUTOK_INTERFACE_DATA**)malloc(sizeof(U_AUTOK_INTERFACE_DATA*)*VCORE_NO);
        i = 0;
        for(it_res=res_list.begin(); it_res!=res_list.end(); ++it_res){
            full_data.ai_data[i] = (U_AUTOK_INTERFACE_DATA*)malloc(sizeof(U_AUTOK_INTERFACE_DATA)*param_count);
            struct res_data *p_temp_res = *it_res;
            predata = get_param((char*)p_temp_res->filepath);
            full_data.vol_list[i] = p_temp_res->voltage;
            memcpy(full_data.ai_data[i], predata.ai_data[0], sizeof(U_AUTOK_INTERFACE_DATA)*param_count);
            release_predata(&predata);
            free(p_temp_res);
            i++;
        }
        printf("1-3 done\n");
        set_stage2(id, &full_data);
        release_predata(&full_data);
        printf("1-4 done\n");
    /*}else{
        set_ready(id);
    }*/
    return 0;
}

int wait_autok_done(int id)
{
    int is_stage2_done = 0;
    std::list<struct host_progress*>::iterator it_prog;
    std::list<struct host_progress*> host_prog;
    while(!is_stage2_done){
        host_prog = get_ready();
        for (it_prog=host_prog.begin(); it_prog!=host_prog.end() ; ++it_prog){
            struct host_progress *prog = *it_prog;
            if(prog->host_id==id){
                if(prog->is_done == 1){
                    is_stage2_done = 1;
                }
                break;
            }
            free(prog);
        }
        usleep(300*1000);
    }
    return 0;
}

int autok_flow()
{
    std::list<struct host_progress*>::iterator it_prog;
    std::list<struct host_progress*> host_prog;
    struct timespec tstart={0,0}, tend={0,0};
    
    while(1){
        // [FIXME] uevent to trigger host autok operation?        
        host_prog = get_ready();
        for (it_prog=host_prog.begin(); it_prog!=host_prog.end() ; ++it_prog){
            struct host_progress *prog = *it_prog;
            //printf("ID(%d) IS_DONE(%d)\n", prog->host_id, prog->is_done);
            //set_ready(2);
            //system("rm -rf /data/autok_*");
            if(prog->host_id>=0 && !prog->is_done){
                printf("[AUTOKD]Autok runs at host %d\n", prog->host_id);
                clock_gettime(CLOCK_MONOTONIC, &tstart);
                if(do_host_autok(prog->host_id)==-1)
                    return -1;
                if(wait_autok_done(prog->host_id)==-1)
                    return -1;
                clock_gettime(CLOCK_MONOTONIC, &tend);                
                printf("autok computation took about %.5f seconds\n",((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
                /*if(is_nvram_mode()){
                    system("rm -rf /data/autok_*");
                }*/
            }
            free(prog);
        }
        usleep(1000*1000);
    }
    return 0;
}