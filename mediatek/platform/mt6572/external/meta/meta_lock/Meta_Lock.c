#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<sys/mount.h>
#include "Meta_Lock.h"
#include "FT_Public.h"
#include "WM2Linux.h"
#include "../../../../kernel/include/mtd/mtd-abi.h"
//#include "../../../../../external/mtd_util/mtd_utilc.h"


#define INVALID_HANDLE  -1

static int META_Lock_WriteFlash(unsigned int iMagicNum)
{
      int fd = INVALID_HANDLE;
      int iWriteSize = 0;
	  int iRealWriteSize = 0;
      int result = 0;
      unsigned int iReadValue=0;
      int *tempBuf=NULL;
      struct mtd_info_user info;
      struct erase_info_user erase_info;
	  META_LOG("[META_LOCK]:enter write flash\r\n");

      META_LOG("[META_LOCK]:open logo partition: '/dev/logo'\n");
	  fd=open("/dev/logo",O_RDWR);
      if(fd<0)
	  {
	      META_LOG("[META_LOCK]:mtd open error\r\n");
	      return 0;
	  }
	  
      result=ioctl(fd,MEMGETINFO,&info);
      if(result<0)
      {
          META_LOG("[META_LOCK]:mtd get info error\r\n");
          goto end;
      }
      iWriteSize=info.writesize;
      
      erase_info.start=END_BLOCK;
      erase_info.length=BLOCK_SIZE;
      result=ioctl(fd, MEMERASE, &erase_info);
      if(result<0)
      {
        META_LOG("[META_LOCK]:mtd erase error\r\n");
        goto end;
      }
	  
      tempBuf=(int*)malloc(iWriteSize);
      
      if(tempBuf==NULL)
      {
      	META_LOG("[META_LOCK]:malloc error\r\n");
	      goto end;
      }
      memset(tempBuf,0xFF,iWriteSize);
      iRealWriteSize=sizeof(unsigned int);
      memcpy(tempBuf,&iMagicNum,iRealWriteSize);
	  
      result=lseek(fd,END_BLOCK,SEEK_SET);
      if(result!=(END_BLOCK))
	  {
          META_LOG("[META_LOCK]:mtd first lseek error\r\n");
          goto end;
	  }
      result=write(fd,tempBuf,iWriteSize);
      if(result!=iWriteSize)
	    {
          META_LOG("[META_LOCK]:mtd write error,iWriteSize:%d\r\n",iWriteSize);
          goto end;
      	}
      memset(tempBuf,0,iWriteSize);
      result=lseek(fd,END_BLOCK,SEEK_SET);
      if(result!=(END_BLOCK))
	    {
          META_LOG("[META_LOCK]:mtd second lseek error\r\n");
          goto end;
	    }
      result=read(fd,tempBuf,iRealWriteSize);
      if(result!=iRealWriteSize)
          {
           META_LOG("[META_LOCK]:mtd read error\r\n");
           goto end;
          }
      memcpy(&iReadValue,tempBuf,iRealWriteSize);
      if(tempBuf!=NULL)
  	  {
      free(tempBuf);
		  tempBuf = NULL;
  	  }
	  if (fd != INVALID_HANDLE)
      {
          close(fd);
		  fd = INVALID_HANDLE;
	  }
	  
      if(iReadValue!=iMagicNum)
	    {   
	     META_LOG("[META_LOCK]:mtd readed value error,iReadValue:%d,iMagicNum:%d\r\n",iReadValue,iMagicNum);
	     return 0;
	    }
	  
      return 1;
  end:
  	  if(tempBuf!=NULL)
  	  {
  	      free(tempBuf);
		  tempBuf = NULL;
  	  }
  	  if (fd != INVALID_HANDLE)
      {
  	      close(fd);
		  fd = INVALID_HANDLE;
	  }
  	  return 0;
}


unsigned char META_Lock_OP()
{
   unsigned int iMagicNum=META_LOCK_MAGIC_NUM;
   int result=0;
   META_LOG("[META_LOCK]:enter meta_lock_op\r\n");
   result=META_Lock_WriteFlash(iMagicNum);
   if(result==0)
     return META_FAILED;
   return META_SUCCESS;
   
}
