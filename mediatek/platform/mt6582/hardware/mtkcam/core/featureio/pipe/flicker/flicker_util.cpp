#include <ctype.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flicker_util.h"
#include "time.h"


int FlickerUtil::getMs()
{
	//	max:
	//	2147483648 digit
	//	2147483.648 second
	//	35791.39413 min
	//	596.5232356 hour
	//	24.85513481 day
	//int t;
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);

	int t;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	t = (ts.tv_sec*1000+ts.tv_nsec/1000000);

	return t;
}

static int isInt(const char* s)
{
  bool isInt = true;
  int slen;
  slen = strlen(s);
    for(int i = 0; i < slen; i++)
	{
      if(!isdigit(s[i]))
	  {
        isInt = false;
		break;
	  }
    }
  return isInt;
}

int FlickerUtil::getFileCount(const char* fname, int* fcnt, int defaultValue)
{
	int err=0;
	int v;
	FILE* fp;
	fp = fopen(fname, "rb");
	if(fp==NULL)
	{
		err = FILE_COUNT_FILE_NOT_FOUND;
	}
	else
	{
		char s[101];
		fscanf(fp,"%100s",s);
		fclose(fp);
		int bNum;
		if(strlen(s)>1)
		  bNum=1;

		//bNum = isInt(s);
		if(bNum==(int)true)
		{
			v=atoi(s);
		}
		else
		{
			err = FILE_COUNT_FILE_NOT_CONTAIN_INT;
		}
	}
	if(err==0)
		*fcnt = v;
	else
		*fcnt = defaultValue;

	return err;
}

int FlickerUtil::setFileCount(const char* fname, int cnt)
{
	int err=0;
	FILE* fp;
	fp = fopen(fname, "wt");
	if(fp==NULL)
	{
		err = FILE_COUNT_FILE_NOT_FOUND;
	}
	else
	{
		fprintf(fp,"%d",cnt);
		fclose(fp);
	}
	return err;
}



void FlickerUtil::createDir(const char* dir)
{
#ifdef WIN32
	CreateDirectory(dir,0);
#else
	mkdir(dir,S_IRWXU | S_IRWXG | S_IRWXO);

#endif

}

#include <cutils/properties.h>
int FlickerUtil:: getPropInt(const char* sId, int DefVal)
{
	int ret;
	char ss[PROPERTY_VALUE_MAX];
	char sDef[20];
	sprintf(sDef,"%d",DefVal);
	property_get(sId, ss, sDef);
	ret = atoi(ss);
	return ret;
}