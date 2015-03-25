#include "icusb_util.h"
#include "icusb_storage.h"

static char *vdc_cmd="/system/bin/vdc" ;

static char *unmount_stor1_argv[]={"vdc", "volume", "unmount", "icusb_storage1","force_and_revert", 0};

static char *unmount_stor2_argv[]={"vdc", "volume", "unmount", "icusb_storage2","force_and_revert", 0};


int icusb_storage_do_unmomunt()
{
	int ret ;
	icusb_print(PRINT_WARN, "[ICUSB][INFO] ====> icusb_storage_do_unmomunt .\n");
	
	ret = icusb_do_exec(vdc_cmd, unmount_stor1_argv) ;

	icusb_print(PRINT_WARN, "[ICUSB][INFO] unmomunt icusb_storage1, return %d \n", ret);
	
	ret = icusb_do_exec(vdc_cmd, unmount_stor2_argv) ;

	icusb_print(PRINT_WARN, "[ICUSB][INFO] unmomunt icusb_storage2, return %d \n", ret);
	
	icusb_print(PRINT_WARN, "[ICUSB][INFO] <==== icusb_storage_do_unmomunt\n");
	
	return ret ; 

}

