
#include <cstdio>
#include "autok.h"
#include "CFG_SDIO_File.h"
#include "CFG_file_lid.h"
#include "libnvram.h"
#include "CFG_file_lid.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <cutils/properties.h>
#define LOG_DBG printf
#define LOG_ERR printf
#define LOG_WAN printf
unsigned char *g_nv_buf;
#define EACH_FILE_SIZE (200)
#define MAX_COUNT (19)
struct autok_single_element
{
    unsigned char vol_count; //always 1
    unsigned char param_count;
    unsigned int voltage;       //1 voltage only
    U_AUTOK_INTERFACE_DATA *ai_data;
};

int sdio_read_nvram(unsigned char *ucNvRamData)
{
    F_ID sdio_nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;
    int sdio_cfgfile_fd = -1;
    ap_nvram_sdio_config_struct sdio_nvram;
    
    int nvram_ready_retry = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    
    /* Sync with Nvram daemon ready */
    do {
        property_get("nvram_init", nvram_init_val, NULL);
        if(0 == strcmp(nvram_init_val, "Ready"))
            break;
        else {
            nvram_ready_retry ++;
            usleep(50000);
        }
    } while(nvram_ready_retry < 10);
    
    LOG_DBG("Get NVRAM ready retry %d\n", nvram_ready_retry);
    if (nvram_ready_retry >= 10){
        LOG_ERR("Get NVRAM restore ready fails!\n");
        return -1;
    }
    
    /* Try Nvram first */
    sdio_nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_SDIO_LID, &rec_size, &rec_num, ISWRITE);
    if(sdio_nvram_fd.iFileDesc >= 0){
        if(rec_num != 1){
            LOG_ERR("Unexpected record num %d", rec_num);
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }
        
        if(rec_size != sizeof(ap_nvram_sdio_config_struct)){
            LOG_ERR("Unexpected record size %d ap_nvram_sdioradio_struct %d",
                    rec_size, sizeof(ap_nvram_sdio_config_struct));
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }
        
        if(read(sdio_nvram_fd.iFileDesc, &sdio_nvram, rec_num*rec_size) < 0){
            LOG_ERR("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }
        
        NVM_CloseFileDesc(sdio_nvram_fd);
    }
    
    memcpy(ucNvRamData, &sdio_nvram, sizeof(ap_nvram_sdio_config_struct));
    
    return 0;
}


int read_from_nvram(int id, int voltage, unsigned char **file_data, int *length)
{
    int i;
    int file_length = 0;
    int file_count = 0;
    int result = -1;    
    ap_nvram_sdio_config_struct *nv_data = (ap_nvram_sdio_config_struct*)g_nv_buf;   
    
    
    file_count = nv_data->file_count;
    printf("file_count[%d]\n", file_count);
    for(i=0; i<file_count; i++){
        if(nv_data->id[i] == id){
            struct autok_single_element autok_ptr;
            // = (struct autok_single_element*)&nv_data->data[i*EACH_FILE_SIZE];
            //for(int j=0; j<10; j++)
            //printf("%02x", nv_data->data[i*EACH_FILE_SIZE+j]);
            char *ptr = &nv_data->data[i*EACH_FILE_SIZE];
            memcpy(&autok_ptr.vol_count, ptr, 1);
            memcpy(&autok_ptr.param_count, ptr+1, 1);
            memcpy(&autok_ptr.voltage, ptr+2, 4);
            //printf("voltage[%x][%d]", autok_ptr.voltage, voltage);
            if(autok_ptr.voltage == voltage){
                file_length = nv_data->file_length[i];
                //printf("filelength:%d\n", file_length);
                *file_data = (unsigned char*)malloc(sizeof(unsigned char)*file_length);
                memcpy(*file_data, &nv_data->data[i*EACH_FILE_SIZE], file_length);
                *length = file_length;
                //free(nv_buf);
                return 0;
            }
        }
    }
    //free(nv_buf);
    file_data = NULL;
    return 0;
}

int write_nvram(unsigned char* write_buf, int length, int id, int file_idx, int file_count)
{
    F_ID sdio_nvram_fd;
    int rec_size = 0;
    int rec_num = 0;
    //int sdio_cfgfile_fd = -1;
    //ap_nvram_sdio_config_struct sdio_nvram;
    
    sdio_nvram_fd = NVM_GetFileDesc(AP_CFG_RDCL_FILE_SDIO_LID, &rec_size, &rec_num, ISWRITE);
    if(sdio_nvram_fd.iFileDesc >= 0){
        // Write to data
        lseek(sdio_nvram_fd.iFileDesc, 1+MAX_COUNT+(MAX_COUNT*sizeof(unsigned int))+(file_idx*EACH_FILE_SIZE), SEEK_SET);
        if(write(sdio_nvram_fd.iFileDesc, write_buf, length)<0){
            LOG_ERR("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }        
        // Write to file_length
        lseek(sdio_nvram_fd.iFileDesc, 1+MAX_COUNT+(file_idx*sizeof(unsigned int)), SEEK_SET);
        if(write(sdio_nvram_fd.iFileDesc, &length, sizeof(unsigned char))<0){
            LOG_ERR("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }
        // Write to host_id
        lseek(sdio_nvram_fd.iFileDesc, 1+file_idx, SEEK_SET);
        if(write(sdio_nvram_fd.iFileDesc, (unsigned char*)&id, sizeof(unsigned char))<0){
            LOG_ERR("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }
        // Write to file_count
        lseek(sdio_nvram_fd.iFileDesc, 0, SEEK_SET);
        if(write(sdio_nvram_fd.iFileDesc, (unsigned char*)&file_count, sizeof(unsigned char))<0){
            LOG_ERR("Read NVRAM fails errno %d\n", errno);
            NVM_CloseFileDesc(sdio_nvram_fd);
            return -1;
        }
        NVM_CloseFileDesc(sdio_nvram_fd);
    }
    return 0;
}

int write_dev_to_nvram(char *filename, int id)
{
    F_ID sdio_nvram_fd;
    int rec_size = 0;
    int rec_num = 0;
    int sdio_cfgfile_fd = -1;   
    ap_nvram_sdio_config_struct sdio_nvram;
    unsigned char *data_buf;
    int file_count;
    FILE * inFile;
    int lSize;
    int i;
    
    int data_count;
    FILE * pFile;
    get_node_data(filename, (char**)&data_buf, &lSize);
        
    struct autok_single_element *in_buf = (struct autok_single_element*)malloc(sizeof(struct autok_single_element));
    ap_nvram_sdio_config_struct *nv_data = (ap_nvram_sdio_config_struct*)g_nv_buf;   
    file_count = nv_data->file_count;
    memcpy(&in_buf->voltage, &data_buf[2], sizeof(unsigned int));
    // file update
    for(i=0; i<file_count; i++){
        if(nv_data->id[i] == id){
            struct autok_single_element *autok_ptr = (struct autok_single_element *)malloc(sizeof(struct autok_single_element));
            //; = (struct autok_single_element*)&nv_data->data[i*EACH_FILE_SIZE];
            memcpy(&autok_ptr->voltage, (&nv_data->data[i*EACH_FILE_SIZE])+2, sizeof(unsigned int));
            printf("nv_voltage[%d] buf_voltage[%d]\n", autok_ptr->voltage, in_buf->voltage);
            if(autok_ptr->voltage == in_buf->voltage){
                printf("file_update:%d", file_count);
                nv_data->file_length[i] = lSize;
                memcpy(&nv_data->data[i*EACH_FILE_SIZE], data_buf, lSize);
                write_nvram(data_buf, lSize, id, i, file_count);
                free(autok_ptr);
                break;
            }
            free(autok_ptr);
        }
    }
    free(in_buf);
    // new file(no found in table)
    if(i==file_count){
        printf("add new file with size:%d", lSize);
        nv_data->id[file_count] = id;
        nv_data->file_length[file_count] = lSize;
        memcpy(&nv_data->data[file_count*EACH_FILE_SIZE], data_buf, lSize);        
        write_nvram(data_buf, lSize, id, file_count, file_count+1);
        nv_data->file_count += 1;   // zero based   
    }    
    
    free (data_buf);
    return 0;
}

int write_file_to_nvram(char *filename, int id)
{
    F_ID sdio_nvram_fd;
    int rec_size = 0;
    int rec_num = 0;
    int sdio_cfgfile_fd = -1;   
    ap_nvram_sdio_config_struct sdio_nvram;
    unsigned char *data_buf;
    int file_count;
    FILE * inFile;
    long lSize;
    int i;
    
    inFile = fopen (filename, "rb");
    if (inFile==NULL) {
        printf("File error"); 
        return -1;
    }
    fseek (inFile , 0 , SEEK_END);
    lSize = ftell (inFile);
    rewind (inFile);
    data_buf = (unsigned char*) malloc (sizeof(char)*lSize);
    fread (data_buf, 1, lSize, inFile);
    fclose (inFile);
        
    struct autok_single_element *in_buf = (struct autok_single_element *)data_buf;
    ap_nvram_sdio_config_struct *nv_data = (ap_nvram_sdio_config_struct*)g_nv_buf;   
    file_count = nv_data->file_count;
    
    // file update
    for(i=0; i<file_count; i++){
        if(nv_data->id[i] == id){
            struct autok_single_element *autok_ptr = (struct autok_single_element*)&nv_data->data[i*EACH_FILE_SIZE];
            if(autok_ptr->voltage == in_buf->voltage){
                printf("file_update:%d", file_count);
                nv_data->file_length[i] = lSize;
                memcpy(autok_ptr, data_buf, lSize);
                write_nvram(data_buf, lSize, id, i, file_count);
                break;
            }
        }
    }
    
    // new file(no found in table)
    if(i==file_count){
        printf("add new file with size:%d", lSize);
        nv_data->id[file_count] = id;
        nv_data->file_length[file_count] = lSize;
        memcpy(&nv_data->data[file_count], data_buf, lSize);        
        write_nvram(data_buf, lSize, id, file_count, file_count+1);
        nv_data->file_count += 1;   // zero based   
    }    
    
    free (data_buf);
    return 0;
}

int is_nvram_data_exist(int id, int voltage)
{
    int file_count;
    int i;
    ap_nvram_sdio_config_struct *nv_data = (ap_nvram_sdio_config_struct*)g_nv_buf;   
    file_count = nv_data->file_count;
    printf("file_count[%d]\n", file_count);
    for(i=0; i<file_count; i++){
        if(nv_data->id[i] == id){
            struct autok_single_element autok_ptr;;
            char *ptr = &nv_data->data[i*EACH_FILE_SIZE];
            memcpy(&autok_ptr.voltage, ptr+2, 4);
            if(autok_ptr.voltage == voltage){
                return 1;
            }
        }
    }
    return 0;
}

int get_nvram_param_count(int id)
{
    int file_count;
    int i;
    ap_nvram_sdio_config_struct *nv_data = (ap_nvram_sdio_config_struct*)g_nv_buf;   
    file_count = nv_data->file_count;
    printf("file_count[%d]\n", file_count);
    for(i=0; i<file_count; i++){
        if(nv_data->id[i] == id){
            char *ptr = &nv_data->data[i*EACH_FILE_SIZE];
            return (int)((char)ptr[1]);
        }
    }
    return 0;
}

int init_autok_nvram()
{
    int result = 0;
    g_nv_buf = (unsigned char*)malloc(sizeof(ap_nvram_sdio_config_struct));
    if((result = sdio_read_nvram(g_nv_buf))!=0)
        return result;
    return result;
}

int close_nvram()
{
    free(g_nv_buf);
    return 0;
}