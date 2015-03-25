#ifndef __FLASH_UTIL_H__
#define __FLASH_UTIL_H__




class FlashUtil
{
	enum
	{
		FL_UTL_ERR_FILE_NOT_EXIST=-2000,
	};

public:
	static FlashUtil* getInstance();
	FlashUtil();
	virtual ~FlashUtil();
	//int convertPline(int type, void* out, void* in);
	//int cleanPlineConvertInterm();


	static void aaSub(void* arr, int* sub);

	static int getPropInt(const char* sId, int DefVal);
	static int getFileCount(const char* fname, int* fcnt, int defaultValue);
	static int setFileCount(const char* fname, int cnt);
	static int getMs();
	static void createDir(const char* dir);

	static int aaToBmp(void* arr, const char* aeF, const char* awbF);

	template <class T>
	static void flash_sortxy_xinc(int n, T* x, T* y)
	{
		int i;
		int j;
		for(i=0;i<n;i++)
		for(j=i+1;j<n;j++)
		{
			if(x[i]>x[j])
			{
				T tmp;
				tmp =x[i];
				x[i]=x[j];
				x[j]=tmp;
				tmp =y[i];
				y[i]=y[j];
				y[j]=tmp;
			}
		}
	}




	template <class T>
	static T flash_interp(T x1, T y1, T x2, T y2, T x)
	{
		return y1+ (y2-y1)*(x-x1)/(x2-x1);
	}

	template <class T>
	static T flash_calYFromXYTab(int n, T* xNode, T* yNode, T x)
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
				y=flash_interp(xst, yst, xed, yed, x);
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
	}
};


#endif