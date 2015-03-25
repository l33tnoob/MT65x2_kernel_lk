#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "meta_wifi.h"

static void wifi_info_callback(FT_WM_WIFI_CNF *cnf, void *buf, unsigned int size)
{
    unsigned int i;
    char *type[] = { "WIFI_CMD_SET_OID", "WIFI_CMD_QUERY_OID" };
    OID_STRUC *poid;

    printf("[META_WIFI] <CNF> %s, Drv Status: %d, Status: %d\n", type[cnf->type],     
        cnf->drv_status, cnf->status);

    if (buf) {
        poid = (OID_STRUC *)buf;
        printf("META_WIFI] <CNF> OID: %d, data len: %d\n",
            poid->QueryOidPara.oid, poid->QueryOidPara.dataLen);
        for (i = 0; i < poid->QueryOidPara.dataLen; i++) {
            printf("META_WIFI] <CNF> Data[%d] = 0x%x\n",
                i, poid->QueryOidPara.data[i]);            
        }
    }
}

int main(int argc, const char** argv)
{
    FT_WM_WIFI_REQ req;

    memset(&req, 0, sizeof(FT_WM_WIFI_REQ));

    META_WIFI_Register(wifi_info_callback);

    if (META_WIFI_init(&req) == false) {
        printf("WLAN init failed\n");
        return -1;
    }

    #if 0
    req.type = WIFI_CMD_SET_OID;
    META_WIFI_OP(&req, NULL, 0);

    req.type = WIFI_CMD_QUERY_OID;
    META_WIFI_OP(&req, NULL, 0);
    #endif

    META_WIFI_deinit();
    META_WIFI_Register(NULL);
    
    return 0;
}

