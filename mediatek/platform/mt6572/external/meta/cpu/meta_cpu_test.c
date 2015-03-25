#include <stdio.h>

#include <WM2Linux.h>
#include "meta_cpu_para.h"

int main(int argc, char *argv[])
{
	WatchDog_REQ req;
	WatchDog_CNF cnf;

	cnf = META_RTCRead_OP(req);
	if (cnf.status) {
		META_LOG("rtc: time is %u/%02u/%02u (%u) %02u:%02u:%02u\n",
		         cnf.rtc_year, cnf.rtc_mon, cnf.rtc_day, cnf.rtc_wday,
		         cnf.rtc_hour, cnf.rtc_min, cnf.rtc_sec);
	} else {
		META_LOG("rtc: META_RTCRead_OP() failed\n");
	}

	return 0;
}
