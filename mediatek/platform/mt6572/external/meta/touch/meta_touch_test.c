#include <stdio.h>
#include "meta_touch.h"

// ---------------------------------------------------------------------------

int main(int argc, const char** args)
{
	Touch_REQ req;
	char peer_buff[100]={0};
	unsigned short peer_len=0;

   	printf("Meta Test Touch AP test : START\n");

	if (false == Meta_Touch_Init())
    {
        printf("Meta_Touch_Init() fail\n");
        return -1;
    }

	Meta_Touch_OP(&req, peer_buff, peer_len);
	
	if (false == Meta_Touch_Deinit())
    {
        printf("Meta_Touch_Deinit() fail\n");
		
        return -1;
    }

	printf("Meta Test Touch AP test : END\n");
	
    return 0;
}
