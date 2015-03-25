#include <stdio.h>
#include <stdlib.h>

#include <WM2Linux.h>
#include "meta_keypadbk_para.h"

int main(int argc, char *argv[])
{
	KeypadBK_REQ req;
	KeypadBK_CNF cnf;
	BOOL r;

	if (argc < 3)
		return 0;

	req.onoff = atoi(argv[1]);
	req.DIV = atoi(argv[2]);
	req.DUTY = atoi(argv[3]);

	r = Meta_KeypadBK_Init();
	if (!r) {
		META_LOG("kpd: Meta_KeypadBK_Init() failed\n");
		return 0;
	}

	cnf = Meta_KeypadBK_OP(req);
	if (cnf.status)
		META_LOG("kpd: Meta_KeypadBK_OP() successful\n");
	else
		META_LOG("kpd: Meta_KeypadBK_OP() failed\n");

	Meta_KeypadBK_Deinit();

	return 0;
}
