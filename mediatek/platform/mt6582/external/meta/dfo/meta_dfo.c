#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <DfoBoot.h>
#include <DfoBootDefault.h>
#include <libnvram.h>
#include <CFG_Dfo_File.h>
#include <CFG_Dfo_Default.h>
#include <Custom_NvRam_LID.h>
#include "meta_dfo.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "META_DFO"

static int g_dfoBootCount = 0;
static bool g_hasDfoBoot = false;
static tag_dfo_boot g_dfoBoot;
static ap_nvram_dfo_config_struct g_dfoNvram;
static DFO_CNF_CB g_callback = NULL;

void META_Dfo_SetCallback(DFO_CNF_CB callback)
{
    g_callback = callback;
}

bool    META_Dfo_Init(void)
{
#ifdef NOT_SUPPORT_DFO
    return false;
#endif
    // Read DFO from NVRAM
    F_ID fid;
    int rec_size = 0;
    int rec_num = 0;
    int dfo_file_lid = AP_CFG_CUSTOM_FILE_DFO_LID;
    bool isread = true;
    
    fid = NVM_GetFileDesc(dfo_file_lid, &rec_size, &rec_num, isread);
    if (fid.iFileDesc < 0)
    {
        ALOGE("NVRAM wrong fid: (%d)", fid.iFileDesc);
    }
    int nSize = read(fid.iFileDesc, &g_dfoNvram, rec_size);
    if (nSize != rec_size)
    {
        ALOGE("NVRAM wrong size: (%d)", nSize);
    }
    else
    {
        int i = 0;
        for (; i < g_dfoNvram.count; i++)
        {
            ALOGE("NVRAM DFO(%d): name(%s)  value(%d)", i, g_dfoNvram.name[i], g_dfoNvram.value[i]);
        }
    }

    NVM_CloseFileDesc(fid);
    
    // Read DFO from MISC
    FILE* fp = fopen("/proc/lk_env", "r");
        
    if (fp)
    {
        // get lk_env size
        char tmp[512] = {0};
        int rtn = 0;
        int lkSize = 0;
        while ((rtn = fread(tmp, 1, 512, fp)) > 0)
        {
            lkSize += rtn;
            if (rtn != 512)
                break;
        }
        fseek(fp, 0, SEEK_SET);
    
        if (lkSize > 0)
        {
            char* buffer = (char*) malloc(sizeof(char)*lkSize + 1);
            size_t result = fread(buffer, 1, lkSize, fp);
            buffer[lkSize] = '\0';
            
            ALOGI("Read env: (%s)", buffer);

            int remain = lkSize + 1;
            char* ptr = buffer;
            
            do
            {
                int len = strlen(ptr);   
                char* name = strtok(ptr, "=");
                
                if (strcmp(name, "DFO") == 0)
                {
                    const char* dfoDel = ",";
                    char* values = strtok(NULL, "=");
                    g_dfoBootCount = DFO_BOOT_COUNT;
                    
                    int i = 0;
                    for (; i < g_dfoBootCount; i++)
                    {
                        char* dfoName = NULL;
                        if (i == 0)
                            dfoName = strtok(values, dfoDel);
                        else
                            dfoName = strtok(NULL, dfoDel);
                        char* dfoValue = strtok(NULL, dfoDel);
                        ALOGI("MISC DFO(%d): name(%s) value(%s)", i, dfoName, dfoValue);
                        strcpy(g_dfoBoot.name[i], dfoName);
                        g_dfoBoot.value[i] = atoi(dfoValue);
                    }
                    
                    g_hasDfoBoot = true;
                    break;
                }
                else
                {
                    ptr += (len + 1);
                    remain -= (len + 1);
                }
            } while (remain > 0);
            
            if (!g_hasDfoBoot)
            {
                memcpy(&g_dfoBoot, &dfo_boot_default, sizeof(tag_dfo_boot));
                g_dfoBootCount = DFO_BOOT_COUNT;
            }
            
            free(buffer);
        }
        fclose(fp);
    }
    
    return true;
}

void    META_Dfo_Deinit(void)
{
    g_dfoBootCount = 0;
    g_hasDfoBoot = false;
}

unsigned char    Dfo_ReadCount_OP(Dfo_read_count_cnf* cnf)
{
    cnf->count = g_dfoBootCount + g_dfoNvram.count;
    return META_SUCCESS;
}

unsigned char    Dfo_ReadValue_OP(Dfo_read_req* req, Dfo_read_cnf* cnf)
{
    if (req->index < g_dfoNvram.count)
    {
        strcpy(cnf->name, g_dfoNvram.name[req->index]);
        cnf->value = g_dfoNvram.value[req->index];
        cnf->partition = 0; // NVRAM
    }
    else
    {
        int index = req->index - g_dfoNvram.count;
        strcpy(cnf->name, g_dfoBoot.name[index]);
        cnf->value = g_dfoBoot.value[index];
        cnf->partition = 1; // MISC
    }
    return META_SUCCESS;
}

unsigned char    Dfo_WriteValue_OP(Dfo_write_req* req)
{
    if (req->partition == 0) // NVRAM
    {
        int i = 0;
        for (; i < g_dfoNvram.count; i++)
        {
            if (strcmp(g_dfoNvram.name[i], req->name) == 0)
            {
                // find it
                ALOGI("[NVRAM] Dfo write: (%s, %d)", req->name, req->value);
                g_dfoNvram.value[i] = req->value;
                break;
            }
        }
    }
    else    // MISC, partition == 1
    {
        int i = 0;
        for (; i < g_dfoBootCount; i++)
        {
            if (strcmp(g_dfoBoot.name[i], req->name) == 0)
            {
                // find it
                ALOGI("[MISC] Dfo write: (%s, %d)", req->name, req->value);
                g_dfoBoot.value[i] = req->value;
                break;
            }
        }
    }
    
    if (req->save)
    {
        // save to NVRAM
        F_ID fid;
        int rec_size = 0;
        int rec_num = 0;
        int dfo_file_lid = AP_CFG_CUSTOM_FILE_DFO_LID;
        bool isread = false;

        fid = NVM_GetFileDesc(dfo_file_lid, &rec_size, &rec_num, isread);
        if (fid.iFileDesc < 0)
        {
            ALOGE("NVRAM wrong fid: (%d)", fid.iFileDesc);
        }

        int nSize = write(fid.iFileDesc, &g_dfoNvram, rec_size);
        if (nSize != rec_size)
        {
            ALOGE("NVRAM wrong size: (%d)", nSize);
        }

        NVM_CloseFileDesc(fid);
        
        // save to MISC
        FILE* fp = fopen("/proc/lk_env", "w");
        char tmp[16];
        int size = 4;  // "dfo="

        // name and value
        int i = 0;
        for (; i < g_dfoBootCount; i++)
        {
            sprintf(tmp, "%d", g_dfoBoot.value[i]);
            if (i != 0)
                size += 1;     // ','
            size += strlen(g_dfoBoot.name[i]);
            size += 1;     // ','
            size += strlen(tmp);
        }

        size += 1;         // '\0'

        char* buffer = (char*) malloc(sizeof(char)*size);
        sprintf(buffer, "DFO=");
        buffer[size - 1] = '\0';
        char* ptr = buffer + 4;
        int remain = size - 4;
        int rtn = 0;

        // write name and value
        for (i = 0; i < g_dfoBootCount; i++)
        {
            if (i != 0)
                rtn = snprintf(ptr, remain, ",%s,%d", g_dfoBoot.name[i], g_dfoBoot.value[i]);
            else
                rtn = snprintf(ptr, remain, "%s,%d", g_dfoBoot.name[i], g_dfoBoot.value[i]);
            ptr += rtn;
            remain -= rtn;
        }

        ALOGI("Dfo write string: (%s)", buffer);

        fwrite(buffer, 1, size, fp);
        fclose(fp);
        free(buffer);
    }

    return META_SUCCESS;
}

unsigned char    Dfo_ComboCount_OP(Dfo_combo_count_cnf* cnf)
{
    // TODO: implement this function
    cnf->count = 3;
    return META_SUCCESS;
}

unsigned char    Dfo_ComboRead_OP(Dfo_combo_read_req* req, Dfo_combo_read_cnf* cnf)
{
    // TODO: implement this function
    sprintf(cnf->name, "COMBO%d", req->index + 1);
    cnf->modeCount = req->index + 1;
    return META_SUCCESS;
}

unsigned char    Dfo_ComboMode_OP(Dfo_combo_mode_req* req, Dfo_combo_mode_cnf* cnf)
{
    sprintf(cnf->name, "COMBO%d_MODE%d", req->index + 1, req->modeIndex);
    return META_SUCCESS;
}

unsigned char    Dfo_ComboUpdate_OP(Dfo_combo_update_req* req)
{
    // TODO: implement this function
    return META_SUCCESS;
}

void    META_Dfo_OP(FT_DFO_REQ* req)
{
    LOGD("req->op:%d\n", req->op);
    FT_DFO_CNF dfo_cnf;
    memset(&dfo_cnf, 0, sizeof(FT_DFO_CNF));
    dfo_cnf.header.token = req->header.token;
    dfo_cnf.header.id = FT_DFO_CNF_ID;
    dfo_cnf.op = req->op;
    
    switch (req->op) {
    case DFO_OP_QUERY_STATUS:
        break;
    case DFO_OP_READ_COUNT:
        dfo_cnf.status = Dfo_ReadCount_OP(&(dfo_cnf.result.read_count_cnf));
        break;
    case DFO_OP_READ:
        dfo_cnf.status = Dfo_ReadValue_OP(&(req->cmd.read_req), &(dfo_cnf.result.read_cnf));
        break;
    case DFO_OP_WRITE:
        dfo_cnf.status = Dfo_WriteValue_OP(&(req->cmd.write_req));
        break;
    case DFO_OP_COMBO_COUNT:
        dfo_cnf.status = Dfo_ComboCount_OP(&(dfo_cnf.result.combo_count_cnf));
        break;
    case DFO_OP_COMBO_READ:
        dfo_cnf.status = Dfo_ComboRead_OP(&(req->cmd.combo_read_req), &(dfo_cnf.result.combo_read_cnf));
        break;
    case DFO_OP_COMBO_MODE:
        dfo_cnf.status = Dfo_ComboMode_OP(&(req->cmd.combo_mode_req), &(dfo_cnf.result.combo_mode_cnf));
        break;
    case DFO_OP_COMBO_UPDATE:
        dfo_cnf.status = Dfo_ComboUpdate_OP(&(req->cmd.combo_update_req));
        break;
    default:	
        LOGE("Error: unsupport DFO_OP code = %d\n", req->op);	  	
        break;
    }
    
    if (g_callback)
        g_callback(&dfo_cnf, sizeof(FT_DFO_CNF));
    else
        WriteDataToPC(&dfo_cnf, sizeof(FT_DFO_CNF), NULL, 0);
}
