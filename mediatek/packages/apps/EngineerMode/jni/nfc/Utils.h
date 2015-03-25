// Utils.h: interface for the Utils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILS_H__83577125_C3F4_428B_A2ED_DE6793346790__INCLUDED_)
#define AFX_UTILS_H__83577125_C3F4_428B_A2ED_DE6793346790__INCLUDED_

#include <jni.h>

typedef enum _OBJTYPE{
		ENUM_BOOLEAN = 0,
		ENUM_BYTE,
		ENUM_CHAR,
		ENUM_SHORT,
		ENUM_INT,
		ENUM_LONG,
		ENUM_FLOAT,
		ENUM_DOUBLE,
		ENUM_OBJECT,
		ENMU_TOTAL
}OBJ_TYPE;

typedef union _JVALUE
{
	jboolean bool_val;
	jbyte	 byte_val;
	jchar	 char_val;
	jshort	 short_val;
	jint	 int_val;
	jlong    long_val;
	jfloat   float_val;
	jdouble  double_val;
	jobject  object_val;
}JVALUE;


jfieldID XGetField(JNIEnv*& env, jobject& obj, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val);

jfieldID XSetField(JNIEnv*& env, jobject& obj, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val);

jfieldID XGetStaticField(JNIEnv*& env, const char* cls_name, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val);

jfieldID XSetStaticField(JNIEnv*& env, const char* cls_name, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val);

jsize XGetArrayLength(JNIEnv*& env, jobject& obj);
void XCopyByteArray2ByteBuf(JNIEnv*& env, char* buf, jobject& obj);
jobject XByteBuf2NewByteArray(JNIEnv*& env, char* buf, jsize sz);

void XCopyShortArray2ShortBuf(JNIEnv*& env, short* buf, jobject& obj);
jobject XShortBuf2NewShortArray(JNIEnv*& env, short* buf, jsize sz);

jint  XGetEnumOrdinal(JNIEnv*& env,  jobject& obj);
void  XSetEnumField(JNIEnv*& env, jobject& obj, jint ordinal);

jobject XNewElement(JNIEnv*& env,  const char* ele_sig);

void XGet2DArrayDimension(JNIEnv*& env, jobject& obj, /*IN OUT*/ jint* m, /*IN OUT*/ jint* n);
void XGet2DShortFromArray(JNIEnv*& env, jobject& obj, jshort* data);
jobjectArray XMake2DShortArray(JNIEnv*& env, jshort* data, jint m, jint n);//data[m][n]

void Check_exception(JNIEnv*& env, int line, const char* append_str="#");



#endif // !defined(AFX_UTILS_H__83577125_C3F4_428B_A2ED_DE6793346790__INCLUDED_)
