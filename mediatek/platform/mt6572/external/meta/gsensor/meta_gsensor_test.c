#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "meta_gsensor.h"
#include "meta_gsensor_para.h"
#include "libhwm.h"
#define TAG "[GSTST] "
#define NULL_FT {0,0}
#define GET_ERR(X) ((X == 0) ? ("OKAY") : ("FAIL"))

static void gs_cnf_handler(GS_CNF *cnf)
{
    printf("-------------------------------------------------------------------\n");
    if (cnf->op == GS_OP_CALI) {
        printf("OP_CALI: %d %d %s\n", cnf->gs_err, cnf->status, GET_ERR(cnf->status));
        printf("(%+6d, %+6d, %+6d) => (%+9.4f, %+9.4f, %+9.4f)\n", 
                cnf->ack.cali.x, cnf->ack.cali.y, cnf->ack.cali.z,
                LSB_TO_GRA(cnf->ack.cali.x), LSB_TO_GRA(cnf->ack.cali.y), LSB_TO_GRA(cnf->ack.cali.z));
    } else if (cnf->op == GS_OP_READ_RAW) {
        printf("OP_READ_RAW: %d %d %s\n", cnf->gs_err, cnf->status, GET_ERR(cnf->status));
        printf("(%+6d, %+6d, %+6d) => (%+9.4f, %+9.4f, %+9.4f)\n", 
                cnf->ack.readraw.x, cnf->ack.cali.y, cnf->ack.cali.z,
                LSB_TO_GRA(cnf->ack.readraw.x), LSB_TO_GRA(cnf->ack.cali.y), LSB_TO_GRA(cnf->ack.cali.z));    
    } else if (cnf->op == GS_OP_WRITE_NVRAM) {
        printf("GS_OP_WRITE_NVRAM: %d %d %s\n", cnf->gs_err, cnf->status, GET_ERR(cnf->status));
    } else if (cnf->op == GS_OP_READ_NVRAM) {
        printf("OP_READ_NVRAM: %d %d %s\n", cnf->gs_err, cnf->status, GET_ERR(cnf->status));
        printf("(%+6d, %+6d, %+6d) => (%+9.4f, %+9.4f, %+9.4f)\n", 
                cnf->ack.readnv.x, cnf->ack.readnv.y, cnf->ack.readnv.z,
                LSB_TO_GRA(cnf->ack.readnv.x), LSB_TO_GRA(cnf->ack.readnv.y), LSB_TO_GRA(cnf->ack.readnv.z));        
    }
}

int main(int argc, const char** argv)
{
    int id, idx = 0;
    GS_REQ req;

    printf("[VERSION] %s %s\n", __DATE__, __TIME__);
    for (idx = 0; idx < argc; idx++)
        printf("[%2d] %s\n", idx, argv[idx]);

    memset(&req, 0, sizeof(GS_REQ));

    if (argc >= 2) {
        id = atoi(argv[1]);
        id = (id < GS_OP_END) ? (id) : (GS_OP_END);
    } else {
        printf("Usage: %s [ID] [ARGS..]\n", argv[0]);
        printf("       %s 0 [DELAY] [NUM] [TOLERANCE]: calibration\n",argv[0]);
        printf("       %s 1 [NUM]: read raw data\n",argv[0]);
        printf("       %s 2 [X] [Y] [Z]: write nvram\n",argv[0]);
        printf("       %s 3 [X] [Y] [Z]: read  nvram\n",argv[0]);
        return 0;
    }

    printf("register: %p\n", gs_cnf_handler);
    Meta_GSensor_Register(gs_cnf_handler);
    if (Meta_GSensor_Open() == false) {
        printf("[GS] open failed\n");
        goto exit;
    }
    printf("main handler\n");
    if ((id == GS_OP_CALI) || (id == GS_OP_END)) {
        req.cmd.cali.delay = 50;
        req.cmd.cali.num = 20;
        req.cmd.cali.tolerance = 20;
        req.op = GS_OP_CALI;
        if (argc >= 3)
            req.cmd.cali.delay = atoi(argv[2]);
        if (argc >= 4)
            req.cmd.cali.num   = atoi(argv[3]);
        if (argc >= 5)
            req.cmd.cali.tolerance = atoi(argv[4]);
        
        Meta_GSensor_OP(&req, NULL, 0);
    }
    if ((id == GS_OP_READ_RAW) || (id == GS_OP_END)) {
        int idx, num = 1;
        req.cmd.readraw.dummy = 0;
        req.op = GS_OP_READ_RAW;
        if (argc >= 3)
            num = atoi(argv[2]);
        for (idx = 0; idx < num; idx++)        
            Meta_GSensor_OP(&req, NULL, 0);
    }
    if ((id == GS_OP_WRITE_NVRAM) || (id == GS_OP_END)) {
        req.op = GS_OP_WRITE_NVRAM;
        if (argc >= 5) {
            req.cmd.writenv.x = atoi(argv[2]);
            req.cmd.writenv.y = atoi(argv[3]);
            req.cmd.writenv.z = atoi(argv[4]);            
        }
        Meta_GSensor_OP(&req, NULL, 0);
    } 
    if ((id == GS_OP_READ_NVRAM) || (id == GS_OP_END)) {
        req.op = GS_OP_READ_NVRAM;
        Meta_GSensor_OP(&req, NULL, 0);
    } 
    
    if (Meta_GSensor_Close() == false) {
        printf("[GS] close failed\n");
        goto exit;
    }

exit:
    Meta_GSensor_Register(NULL);
    return 0;
}
