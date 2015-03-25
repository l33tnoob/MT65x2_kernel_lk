#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>

#include "bootloader.h"
#include "common.h"
#include "mtdutils/mtdutils.h"
#include "roots.h"
#include "install.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "sec"
#endif
#include "cutils/log.h"

#include "sec.h"


/******************************************************************************
 *  DEBUG OPTIONS
 ******************************************************************************/
//#define SEC_UI_MESSAGE

/******************************************************************************
 *  INTERNAL DEFINITION
 ******************************************************************************/
#define MOD                                     "SEC"

/******************************************************************************
 *  INTERNAL DATA STRUCTURE
 ******************************************************************************/
#define RB_SEC_VERSION                          "2012/02/06"

/******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
bool bSecEnabled                                = false;

/******************************************************************************
 *  EXTERNAL FUNCTION
 ******************************************************************************/
extern void LOG_INFO(const char *msg, ...);
extern void LOG_ERROR(const char *msg, ...);
extern void LOG_HEX(const char *str, const char *p, int len);

//#define SEC_UI_MESSAGE
//#define SEC_LOG_MESSAGE
#ifdef SEC_UI_MESSAGE
#define ui_print_info   ui_print
#define ui_print_err   ui_print
#elif SEC_LOG_MESSAGE
#define ui_print_info   LOG_INFO
#define ui_print_err   LOG_ERROR
#else
#define ui_print_info(...)
#define ui_print_err(...)
#endif


/******************************************************************************
 *  SECURE INIT
 ******************************************************************************/
bool sec_init(bool bDebug)
{
    int ret = SEC_OK;

    if(bDebug)
    {
        ui_print_info("[%s] %s\n", __func__, RB_SEC_VERSION);
    }

    /* =================================== */
    /* init boot info                      */
    /* =================================== */
    ret = sec_boot_init(false,false);

    if((ret == SEC_SBOOT_NOT_ENABLED) || (ret == SEC_SUSBDL_NOT_ENABLED))
    {
        printf("[%s] no check. ret 0x%x\n",MOD,ret);
        bSecEnabled = false;
        goto _end;
    }
    
    if(SEC_OK != ret)
    {   
        goto _err;
    }

    bSecEnabled = true;

_end:        
    return ret;

_err:
    ui_print_err("[%s] init fail. ret '0x%x'\n",MOD,ret);
    return ret;
}

/******************************************************************************
 *  CHECK IMAGE INFO
 ******************************************************************************/
int sec_verify_img_info (ZipArchive *zip,bool bDebug)
{
    int ret = SEC_OK;
    int ok = 0;
    int fd = 0;
    FILE *fp = NULL; 

    /* =================================== */
    /* check if security is enabled        */
    /* =================================== */
    if(false == bSecEnabled)
    {
        goto _end;
    }

    /* =================================== */
    /* open version file                   */
    /* =================================== */
    char *fname = "/tmp/SEC_VER";
    
    const ZipEntry *sec_ver_entry = mzFindZipEntry(zip, "SEC_VER.txt");
    
    char *r0 = 0;
    char *r1 = 0;
    char *r2 = 0;
    char file_buf[1024];
    bool bCheckCustName = false;
    
    if(bDebug) 
    { 
        ui_print_info("sec check image version\n"); 
    }     

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    /* If using MOTA, must ask MOTA owner to add SEC_VER.txt into diff package  */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    if (sec_ver_entry == NULL) 
    {
        siu_invalid();
        ret = INSTALL_SECURE_INFO_NOT_FOUND; 
        goto _err;
    }
    
    unlink(fname);
    
    fd = creat(fname, 0755);
    if (fd < 0) 
    {
        mzCloseZipArchive(zip);
        LOGE("Can't make %s\n", fname);
        ui_print_err("Error: Can't make %s\n", fname);
        goto _err;        
    }

    ok = mzExtractZipEntryToFile(zip, sec_ver_entry, fd);
    close(fd);

    if (!ok) 
    {
        ui_print_err("Error: File system fail\n");
        goto _err;
    }
    
    fp = fopen(fname, "r");
    if(!fp) 
    {
        ui_print_err("Error: Open type fail\n");
        unlink(fname);    
        goto _err;
    }
    
    /* =================================== */
    /* check security information          */
    /* =================================== */
    
    while(fgets(file_buf, sizeof(file_buf), fp) != 0)
    {    
        r0 = strtok(file_buf," ");
        r1 = strtok(0," ");
        r2 = strtok(0,"\n");
        
        if(false == bCheckCustName)
        {
            //ui_print_info("[%s] cust name '%s' '%d' \n",MOD,r2,strlen(r2));
            if(SEC_OK != (ret = siu_verify_cust_name((unsigned char*)r2,strlen(r2))))
            {            
                goto _err;
            }
            
            bCheckCustName = true;            
        }
        else
        {
            if(SEC_OK != (ret = siu_verify_img_ver((unsigned char*)r0,atoi(r2))))
            {            
                goto _err;
            }
            
            //ui_print_info("[%s] image '%s' version '%d' ok\n",MOD,r0,atoi(r2));
        }        
    }

_end:    
    return ret;

_err:
    siu_invalid();
    ui_print_err("[%s] check image info fail '0x%x'\n",MOD,ret);
    return ret;
}

/******************************************************************************
 *  CHECK IMAGE INFO FOR FOTA
 ******************************************************************************/
int sec_verify_img_info_fota(const char *fname,bool bDebug)
{
    int ret = SEC_OK;
    int ok = 0;
    int fd = 0;
    FILE *fp = NULL; 

    /* =================================== */
    /* check if security is enabled        */
    /* =================================== */
    if(false == bSecEnabled)
    {
        goto _end;
    }

    /* =================================== */
    /* open version file                   */
    /* =================================== */
    char *r0 = 0;
    char *r1 = 0;
    char *r2 = 0;
    char file_buf[1024];
    bool bCheckCustName = false;
    
    if(bDebug) 
    { 
        ui_print_info("sec check image version\n"); 
    }     

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    /* If using MOTA, must ask MOTA owner to add SEC_VER1.txt SEC_VER2.txt into diff package  */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    if (fname == NULL) 
    {
        siu_invalid();
        ret = INSTALL_SECURE_INFO_NOT_FOUND; 
        goto _err;
    }

   
    fp = fopen(fname, "r");
    if(!fp) 
    {
        ui_print_err("Error: Open type fail\n");
        unlink(fname);    
        goto _err;
    }
    
    /* =================================== */
    /* check security information          */
    /* =================================== */
    
    while(fgets(file_buf, sizeof(file_buf), fp) != 0)
    {    
        r0 = strtok(file_buf," ");
        r1 = strtok(0," ");
        r2 = strtok(0,"\n");
        
        if(false == bCheckCustName)
        {
            //ui_print_info("[%s] cust name '%s' '%d' \n",MOD,r2,strlen(r2));
            if(SEC_OK != (ret = siu_verify_cust_name((unsigned char*)r2,strlen(r2))))
            {            
                goto _err;
            }
            
            bCheckCustName = true;            
        }
        else
        {
            if(SEC_OK != (ret = siu_verify_img_ver((unsigned char*)r0,atoi(r2))))
            {            
                goto _err;
            }
            
            //ui_print_info("[%s] image '%s' version '%d' ok\n",MOD,r0,atoi(r2));
        }        
    }

_end:    
    return ret;

_err:
    siu_invalid();
    ui_print_err("[%s] check image info fail '0x%x'\n",MOD,ret);
    return ret;
}


/******************************************************************************
 *  MARK STATUS
 ******************************************************************************/
int sec_mark_status(bool bDebug)
{
    int ret = SEC_OK;
    
    if(bDebug) { ui_print_info("sec mark status\n"); }     


    /* =================================== */
    /* check if sec is enabled             */
    /* =================================== */
    if(false == bSecEnabled)
    {
        goto _end;
    }

    /* =================================== */
    /* mark status                         */
    /* =================================== */
    if(SEC_OK != (ret = siu_mark_status()))
    {
        goto _err;
    }

_end:    
    return ret;

_err:
    ui_print_err("[%s] mark status fail '0x%x'\n",MOD,ret);
    //assert(0);        
    return ret;
}



/******************************************************************************
 *  SECURE UPDATE
 ******************************************************************************/
int sec_update(bool bDebug)
{
    int ret = SEC_OK;

    if(bDebug) 
    { 
        ui_print_info("sec update\n"); 
    }     

    /* =================================== */
    /* check if sec is enabled             */
    /* =================================== */
    if(false == bSecEnabled)
    {
        goto _end;
    }

    /* =================================== */
    /* recovery update                     */
    /* =================================== */
    if(SEC_OK != (ret = siu_recovery_update()))
    {
        goto _err;
    }     

_end:    
    return ret;

_err:
    ui_print_err("[%s] recovery update fail '0x%x'\n",MOD,ret);
    //assert(0);        
    return ret;
}

