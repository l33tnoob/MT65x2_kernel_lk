#include <stdio.h>
#include <stdlib.h>
#include "meta_dfo.h"

typedef struct
{
    char name[32];
    int value;
    int partition;
} dfo_test;

typedef struct
{
    char name[32];    
} dfo_combo_mode_test;

typedef struct
{
    char name[32];
    int modeCount;
    dfo_combo_mode_test* modeArray;
} dfo_combo_test;

int g_dfoCount = 0;
dfo_test* g_dfoArray = NULL;
int g_curIndex = 0;

int g_comboCount = 0;
dfo_combo_test* g_comboArray = NULL;
int g_curCombo = 0;
int g_curComboMode = 0;

static void dfo_callback(void* buf, unsigned int size)
{
    if (buf == NULL)
    {
        printf("callback buffer is NULL\n");
        return;
    }
    
    FT_DFO_CNF* cnf = (FT_DFO_CNF*)buf;
    
    DFO_OP op = cnf->op;
    
    if (op == DFO_OP_READ_COUNT)
    {
        g_dfoCount = cnf->result.read_count_cnf.count;
        printf("DfoBoot count: (%d)\n", g_dfoCount);
        
        if (g_dfoCount > 0)
        {
            if (g_dfoArray)
            {
                free(g_dfoArray);
                g_dfoArray = NULL;
            }
            g_dfoArray = (dfo_test*)malloc(sizeof(dfo_test)*g_dfoCount);
        }
    }
    else if (op == DFO_OP_READ)
    {
        printf("Read(%d) name: (%s) value: (%d) partition: (%d)\n", g_curIndex,
             cnf->result.read_cnf.name, cnf->result.read_cnf.value, cnf->result.read_cnf.partition);
        strcpy(g_dfoArray[g_curIndex].name, cnf->result.read_cnf.name);
        g_dfoArray[g_curIndex].value = cnf->result.read_cnf.value;
        g_dfoArray[g_curIndex].partition = cnf->result.read_cnf.partition;
    }
    else if (op == DFO_OP_WRITE)
    {
        printf("Write result: (%d)\n", cnf->status);
    }
    else if (op == DFO_OP_COMBO_COUNT)
    {
        g_comboCount = cnf->result.combo_count_cnf.count;
        printf("Combo count: (%d)\n", g_comboCount);
        
        if (g_comboCount > 0)
        {
            if (g_comboArray)
            {
                free(g_comboArray);
                g_comboArray = NULL;
            }
            g_comboArray = (dfo_combo_test*)malloc(sizeof(dfo_combo_test)*g_comboCount);
        }
    }
    else if (op == DFO_OP_COMBO_READ)
    {
        printf("Combo Read(%d) name: (%s) mode count: (%d)\n", g_curCombo,
             cnf->result.combo_read_cnf.name, cnf->result.combo_read_cnf.modeCount);
        strcpy(g_comboArray[g_curCombo].name, cnf->result.combo_read_cnf.name);
        g_comboArray[g_curCombo].modeCount = cnf->result.combo_read_cnf.modeCount;
        g_comboArray[g_curCombo].modeArray = (dfo_combo_mode_test*)malloc(sizeof(dfo_combo_mode_test)*g_comboArray[g_curCombo].modeCount);
    }
    else if (op == DFO_OP_COMBO_MODE)
    {
        printf("Combo(%d) Mode(%d) name: (%s)\n", g_curCombo, g_curComboMode, cnf->result.combo_mode_cnf.name);
        strcpy(g_comboArray[g_curCombo].modeArray[g_curComboMode].name, cnf->result.combo_mode_cnf.name);
    }
    else if (op == DFO_OP_COMBO_UPDATE)
    {
    }
    else
    {
        printf("Wrong DFO OP: (%d)\n", op);
    }
}

int main(int argc, const char** argv)
{
    META_Dfo_SetCallback(dfo_callback);
    META_Dfo_Init();
    
    // test read count
    {
        FT_DFO_REQ req;
        memset(&req, 0, sizeof(FT_DFO_REQ));
        req.header.id = FT_DFO_REQ_ID;
        req.op = DFO_OP_READ_COUNT;
        META_Dfo_OP(&req);
    }
    
    // test read dfo
    {
        FT_DFO_REQ req;
        memset(&req, 0, sizeof(FT_DFO_REQ));
        req.header.id = FT_DFO_REQ_ID;
        req.op = DFO_OP_READ;
        
        int i = 0;
        for (; i < g_dfoCount; i++)
        {
            req.cmd.read_req.index = i;
            g_curIndex = i;
            META_Dfo_OP(&req);
        }
    }

    // test write dfo
    {
        FT_DFO_REQ req;
        memset(&req, 0, sizeof(FT_DFO_REQ));
        req.header.id = FT_DFO_REQ_ID;
        req.op = DFO_OP_WRITE;

        int i = 0;
        for (; i < g_dfoCount; i++)
        {
            printf("META Dfo write: (%s, %d)\n", g_dfoArray[i].name, 2);
            strcpy(req.cmd.write_req.name, g_dfoArray[i].name);
            req.cmd.write_req.partition = g_dfoArray[i].partition;
            if (g_dfoArray[i].partition == 0)
                req.cmd.write_req.value = 6;
            else
                req.cmd.write_req.value = 16;
            if (i == g_dfoCount - 1)
            {
                req.cmd.write_req.save = 1;
            }
            else
            {
                req.cmd.write_req.save = 0;
            }
            META_Dfo_OP(&req);
        }
    }
    
    // test read dfo combo count
    {
        FT_DFO_REQ req;
        memset(&req, 0, sizeof(FT_DFO_REQ));
        req.header.id = FT_DFO_REQ_ID;
        req.op = DFO_OP_COMBO_COUNT;
        META_Dfo_OP(&req);
    }
    
    // test read dfo combo
    {
        FT_DFO_REQ req;
        memset(&req, 0, sizeof(FT_DFO_REQ));
        req.header.id = FT_DFO_REQ_ID;
        req.op = DFO_OP_COMBO_READ;
        
        int i = 0;
        for (; i < g_comboCount; i++)
        {
            req.cmd.combo_read_req.index = i;
            g_curCombo = i;
            META_Dfo_OP(&req);
            
            // test read dfo combo mode
            FT_DFO_REQ req2;
            memset(&req2, 0, sizeof(FT_DFO_REQ));
            req2.header.id = FT_DFO_REQ_ID;
            req2.op = DFO_OP_COMBO_MODE;
            
            int j = 0;
            for (; j < g_comboArray[g_curCombo].modeCount; j++)
            {
                req2.cmd.combo_mode_req.index = i;
                req2.cmd.combo_mode_req.modeIndex = j;
                g_curComboMode = j;
                META_Dfo_OP(&req2);
            }
        }
    }
    
    // test write dfo combo
    {
    }

    META_Dfo_Deinit();
    META_Dfo_SetCallback(NULL);
    
    // TODO: free memory
    
    return 0;
}
