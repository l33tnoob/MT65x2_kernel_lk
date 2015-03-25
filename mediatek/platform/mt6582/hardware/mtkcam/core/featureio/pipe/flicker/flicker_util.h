#ifndef __FLICKER_UTIL_H__
#define __FLICKER_UTIL_H__

class FlickerUtil
{
public:
  static int setFileCount(const char* fname, int cnt);
  static int getFileCount(const char* fname, int* fcnt, int defaultValue=0);
  static void createDir(const char* dir);
  static int getPropInt(const char* sId, int DefVal);
  static int getMs();

  enum
  {
  	FILE_COUNT_FILE_NOT_FOUND = -1,
  	FILE_COUNT_FILE_NOT_CONTAIN_INT = -2,
  };

};

#endif  //#ifndef __FLICKER_UTIL_H__