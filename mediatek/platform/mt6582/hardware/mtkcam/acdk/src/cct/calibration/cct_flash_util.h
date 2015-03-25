#ifndef __CCT_FLASH_UTIL_H__
#define __CCT_FLASH_UTIL_H__

class mtkRaw
{
public:
	int w;
	int h;
	int depth;
	void* raw;
	int colorOrder;
public:
	mtkRaw();
	virtual ~mtkRaw();
	void createBuf(int w, int h, int depth);
	float mean(int x, int y, int w, int h);
	int toBmp(const char* f);
	void mean4(int div_x, int div_y, int div_w, int div_h, float* rggb);
	void mean4Center(float* rggb);
};

class flcUtil
{
public:
  enum
	{
	  FILE_COUNT_FILE_NOT_FOUND = -1,
	  FILE_COUNT_FILE_NOT_CONTAIN_INT = -2,
		FL_UTL_ERR_FILE_NOT_EXIST=-2000,
	};
public:
  template <class T>
  static T interp(T x1, T y1, T x2, T y2, T x)
  {
  	return y1+ (y2-y1)*(x-x1)/(x2-x1);
  };

  template <class T>
  static T calYFromXYTab(int n, T* xNode, T* yNode, T x)
  {
  	T y=yNode[0];
  	int i;
  	T xst;
  	T yst;
  	T xed;
  	T yed;
  	xst=xNode[0];
  	yst=yNode[0];
  	if(x<xNode[0])
  		x=xNode[0];
  	else if(x>xNode[n-1])
  		x=xNode[n-1];

  	for(i=1;i<n;i++)
  	{
  		xed=xNode[i];
  		yed=yNode[i];

  		if(x<=xNode[i])
  		{
  			y=interp(xst, yst, xed, yed, x);
  			break;
  		}
  		xst=xed;
  		yst=yed;
  	}
  	if(x<=xNode[0])
  	    y=yNode[0];
  	else if(x>=xNode[n-1])
  	    y=yNode[n-1];
  	return y;
  };

  static int getMs();
  static int getFileCount(const char* fname, int* fcnt, int defaultValue);
	static int setFileCount(const char* fname, int cnt);


};

#endif


