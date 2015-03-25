#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

#define MTK_WDT_LENGTH_TIME_OUT              (0xffe0)
#define MTK_WDT_LENGTH_KEY           (0x0008)

int main()
{
	char str[8], buf[256];
        int i, sec, temp;
	unsigned int timeout, setting;
	
	//disable WDT
	system("echo 0 29 30 0 0 > /proc/wdk");
	
	//user input
	printf("Please set Count Down Timer between 1 ~ 30 sec\n");
	printf("Set Count Down Timer (sec) : ");
	scanf("%d", &sec);
	if( sec > 30 )
	{	
		sec=30;
		printf("MAX value = %d \n",sec);
		printf("Set Count Down Timer = 30 \n");
	}
	else if(sec<1)
	{
		sec=1;
		printf("MIN value = %d \n",sec);
		printf("Set Count Down Timer = 1 \n");
	}
        timeout = ( sec * (1 << 6) );
        setting = ((timeout<<5)&MTK_WDT_LENGTH_TIME_OUT)|MTK_WDT_LENGTH_KEY;
        //printf("setting = %x\n",setting);


	//set count down sec
	system("echo 0xF0000004 > /sys/bus/platform/drivers/register_access/addr");
	sprintf(buf, "echo 0x%x > /sys/bus/platform/drivers/register_access/val", setting); 
	system(buf);

	//enable WDT
	system("echo 0xF0000000 > /sys/bus/platform/drivers/register_access/addr");
	system("echo 0x2200007D > /sys/bus/platform/drivers/register_access/val");

	//refresh restart reg
	system("echo 0xF0000008 > /sys/bus/platform/drivers/register_access/addr");
	system("echo 0x1971 >  /sys/bus/platform/drivers/register_access/val");


	//count down
	i=sec;
	printf("===== Count Down =====\n");
	do
	{
	printf("%d\n", i);
	sleep(1);	
	i--;
	}while(i>-1);

	return 0;

}

