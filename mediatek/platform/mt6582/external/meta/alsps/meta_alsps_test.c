#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "meta_alsps.h"
#include "meta_alsps_para.h"
//#include "libhwm.h"
#define TAG "[MALSPS] "
#define NULL_FT {0,0}
#define MET_ERR(X) ((X == 0) ? ("OKAY") : ("FAIL"))
int main(int argc, const char** argv)
{
    int err=0;
	int i=0;
   
    if (Meta_ALSPS_Open() == false) {
        printf("[MS] open failed\n");
		return 0;
    }
    printf("main handler\n");
    
    sleep(3);   
	for(i=0; i<20; i++)
	{
	  err = Meta_ALSPS_OP();
	  sleep(1);
	}
    
	if(0 == err)
	{
	  printf("[alsps] i2c  ok\n");
	}
	else
	{
	  printf("[alsps] i2c  err\n");
	}
    
    if (Meta_ALSPS_Close() == false) {
        printf("[alsps] close failed\n");
    }
    return 0;
}
