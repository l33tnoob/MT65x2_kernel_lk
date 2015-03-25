#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "meta_msensor.h"
#include "meta_msensor_para.h"
//#include "libhwm.h"
#define TAG "[MSTST] "
#define NULL_FT {0,0}
#define MET_ERR(X) ((X == 0) ? ("OKAY") : ("FAIL"))
int main(int argc, const char** argv)
{
    int err=0;
   
    if (Meta_MSensor_Open() == false) {
        printf("[MS] open failed\n");
		return 0;
    }
    printf("main handler\n");
    
    sleep(5);   
    err = Meta_MSensor_OP();
	if(0 == err)
	{
	  printf("[MS] i2c  ok\n");
	}
	else
	{
	  printf("[MS] i2c  err\n");
	}
    
    
    
    if (Meta_MSensor_Close() == false) {
        printf("[MS] close failed\n");
    }
    return 0;
}
