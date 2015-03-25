// Utils.cpp: implementation of the Utils class.
//
//////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include "comDef.h"
#include "stdio.h"
#define LOG_TAG "nfc"
#include <cutils/xlog.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//jclass cls = env->GetObjectClass(obj);
//jclass cls = env->FindClass("java/lang/String");
char* g_type_sig[ENMU_TOTAL] = {
		(char*)"Z",
		(char*)"B",
		(char*)"C",
		(char*)"S",
		(char*)"I",
		(char*)"J",
		(char*)"F",
		(char*)"D",
		(char*)"N/A"
};

jfieldID XGetField(JNIEnv*& env, jobject& obj, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val)
{
	jclass cls_obj = env->GetObjectClass(obj);
	Check_exception(env, __LINE__);
	jfieldID id = NULL;

	switch(field_type)
	{
		case ENUM_BOOLEAN:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.bool_val = env->GetBooleanField(obj, id);
			break;
		case ENUM_BYTE:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.byte_val = env->GetByteField(obj, id);
			break;
		case ENUM_CHAR:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.char_val = env->GetCharField(obj, id);
			break;
		case ENUM_SHORT:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.short_val = env->GetShortField(obj, id);
			break;
		case ENUM_INT:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.int_val = env->GetIntField(obj, id);
			break;
		case ENUM_LONG:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.long_val = env->GetLongField(obj, id);
			break;
		case ENUM_FLOAT:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.float_val = env->GetFloatField(obj, id);
			break;
		case ENUM_DOUBLE:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.double_val = env->GetDoubleField(obj, id);
			break;
		case ENUM_OBJECT:
			id = env->GetFieldID(cls_obj, field_name, str_field_sig);
			val.object_val = env->GetObjectField(obj, id);
			break;
		default:
			break;
	}
	Check_exception(env, __LINE__, field_name);
	return id;
}


jfieldID XSetField(JNIEnv*& env, jobject& obj, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val)
{
	jclass cls_obj = env->GetObjectClass(obj);
	Check_exception(env, __LINE__);
	jfieldID id = NULL;

	switch(field_type)
	{
		case ENUM_BOOLEAN:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetBooleanField(obj, id, val.bool_val);
			break;
		case ENUM_BYTE:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetByteField(obj, id, val.byte_val);
			break;
		case ENUM_CHAR:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetCharField(obj, id, val.char_val);
			break;
		case ENUM_SHORT:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetShortField(obj, id, val.short_val);
			break;
		case ENUM_INT:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetIntField(obj, id, val.int_val);
			break;
		case ENUM_LONG:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetLongField(obj, id, val.long_val);
			break;
		case ENUM_FLOAT:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetFloatField(obj, id, val.float_val);
			break;
		case ENUM_DOUBLE:
			id = env->GetFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetDoubleField(obj, id, val.double_val);
			break;
		case ENUM_OBJECT:
			id = env->GetFieldID(cls_obj, field_name, str_field_sig);
			env->SetObjectField(obj, id, val.object_val);
			break;
		default:
			break;
	}
	Check_exception(env, __LINE__, field_name);
	return id;
}


jfieldID XGetStaticField(JNIEnv*& env, const char* cls_name, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val)
{
	jclass cls_obj = env->FindClass(cls_name);
	Check_exception(env, __LINE__);
	jfieldID id = NULL;

	switch(field_type)
	{
		case ENUM_BOOLEAN:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.bool_val = env->GetStaticBooleanField(cls_obj, id);
			break;
		case ENUM_BYTE:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.byte_val = env->GetStaticByteField(cls_obj, id);
			break;
		case ENUM_CHAR:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.char_val = env->GetStaticCharField(cls_obj, id);
			break;
		case ENUM_SHORT:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.short_val = env->GetStaticShortField(cls_obj, id);
			break;
		case ENUM_INT:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.int_val = env->GetStaticIntField(cls_obj, id);
			break;
		case ENUM_LONG:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.long_val = env->GetStaticLongField(cls_obj, id);
			break;
		case ENUM_FLOAT:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.float_val = env->GetStaticFloatField(cls_obj, id);
			break;
		case ENUM_DOUBLE:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			val.double_val = env->GetStaticDoubleField(cls_obj, id);
			break;
		case ENUM_OBJECT:
			id = env->GetStaticFieldID(cls_obj, field_name, str_field_sig);
			val.object_val = env->GetStaticObjectField(cls_obj, id);
			break;
		default:
			break;
	}
	Check_exception(env, __LINE__, field_name);
	return id;
}


jfieldID XSetStaticField(JNIEnv*& env, const char* cls_name, const char* field_name, OBJ_TYPE field_type, const char* str_field_sig, JVALUE& val)
{
	jclass cls_obj = env->FindClass(cls_name);
	Check_exception(env, __LINE__);
	jfieldID id = NULL;

	switch(field_type)
	{
		case ENUM_BOOLEAN:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticBooleanField(cls_obj, id, val.bool_val);
			break;
		case ENUM_BYTE:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticByteField(cls_obj, id, val.byte_val);
			break;
		case ENUM_CHAR:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticCharField(cls_obj, id, val.char_val);
			break;
		case ENUM_SHORT:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticShortField(cls_obj, id, val.short_val);
			break;
		case ENUM_INT:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticIntField(cls_obj, id, val.int_val);
			break;
		case ENUM_LONG:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticLongField(cls_obj, id, val.long_val);
			break;
		case ENUM_FLOAT:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticFloatField(cls_obj, id, val.float_val);
			break;
		case ENUM_DOUBLE:
			id = env->GetStaticFieldID(cls_obj, field_name, g_type_sig[field_type]);
			env->SetStaticDoubleField(cls_obj, id, val.double_val);
			break;
		case ENUM_OBJECT:
			id = env->GetStaticFieldID(cls_obj, field_name, str_field_sig);
			env->SetStaticObjectField(cls_obj, id, val.object_val);
			break;
		default:
			break;
	}
	Check_exception(env, __LINE__, field_name);
	return id;
}


jsize XGetArrayLength(JNIEnv*& env, jobject& obj)
{
	jsize sz = env->GetArrayLength((jarray&)obj);

	Check_exception(env, __LINE__);
	return sz;
}

void XCopyByteArray2ByteBuf(JNIEnv*& env, char* buf, jobject& obj)
{
	jsize sz = env->GetArrayLength((jarray&)obj);
	env->GetByteArrayRegion((jbyteArray)obj, 0, sz, (signed char*)buf);
	Check_exception(env, __LINE__);
	return;
}

jobject XByteBuf2NewByteArray(JNIEnv*& env, char* buf, jsize sz)
{
	jbyteArray new_array = env->NewByteArray(sz);
	env->SetByteArrayRegion(new_array, 0, sz, (const signed char*)buf);

	Check_exception(env, __LINE__);
	return new_array;
}
void XCopyShortArray2ShortBuf(JNIEnv*& env, short* buf, jobject& obj)
{
	jsize sz = env->GetArrayLength((jarray&)obj);
	env->GetShortArrayRegion((jshortArray)obj, 0, sz, (signed short*)buf);
	Check_exception(env, __LINE__);
	return;
}

jobject XShortBuf2NewShortArray(JNIEnv*& env, short* buf, jsize sz)
{
	jshortArray new_array = env->NewShortArray(sz);
	env->SetShortArrayRegion(new_array, 0, sz, (const signed short*)buf);

	Check_exception(env, __LINE__);
	return new_array;
}

jint  XGetEnumOrdinal(JNIEnv*& env,  jobject& obj)
{
	jclass obj_class = env->GetObjectClass(obj);
	jmethodID id = env->GetMethodID(obj_class, "getOrdinal", "()I");

	jint i = env->CallIntMethod(obj, id);
	Check_exception(env, __LINE__);
	return i;
}

void XSetEnumField(JNIEnv*& env, jobject& obj, jint ordinal)
{
	jclass obj_class = env->GetObjectClass(obj);
	jmethodID id = env->GetMethodID(obj_class, "SetType", "(I)V");

	env->CallObjectMethod(obj, id, ordinal);
	Check_exception(env, __LINE__);
	return;
}

jobject XNewElement(JNIEnv*& env,  const char* ele_sig)
{
	XLOGD("Enter XNewElement signature %s", ele_sig);
	jclass obj_class = env->FindClass(ele_sig);
	XLOGD("XNewElement obj_class %d", obj_class);
	jmethodID id = env->GetMethodID(obj_class, "<init>", "()V");
	
	XLOGD("XNewElement jmethodID %d", id);
	jobject obj = env->NewObject(obj_class, id);
	
	XLOGD("XNewElement jobject %x", obj);
	Check_exception(env, __LINE__, ele_sig);
	return obj;
}


jobjectArray XMake2DShortArray(JNIEnv*& env, jshort* data, jint m, jint n)//data[m][n]

{	
    jobjectArray result;	
    jclass intArrCls = env->FindClass("[S");
	
    result = env->NewObjectArray(m, intArrCls, NULL);	
	
    for (int i=0; i<m; i++)
	{
        jshortArray iarr = env->NewShortArray(n);
		
        env->SetShortArrayRegion(iarr, 0, n, data+i*n);
		
        env->SetObjectArrayElement(result, i, iarr);
		
        env->DeleteLocalRef(iarr);		
    }
	
    return result;	
}

void XGet2DArrayDimension(JNIEnv*& env, jobject& obj, /*IN OUT*/ jint* m, /*IN OUT*/ jint* n)
{
	*m = env->GetArrayLength((jobjectArray)obj);	
	jobjectArray colArray = (jobjectArray)env->GetObjectArrayElement((jobjectArray)obj, 0);	
	*n =env->GetArrayLength(colArray);
}

void XGet2DShortFromArray(JNIEnv*& env, jobject& obj, jshort* data)
{
	int m = env->GetArrayLength((jobjectArray)obj);	
	jobjectArray colArray = (jobjectArray)env->GetObjectArrayElement((jobjectArray)obj, 0);	
	int n =env->GetArrayLength(colArray);
	
	for (int i=0; i<m; i++)		
	{	
		colArray = (jobjectArray)env->GetObjectArrayElement((jobjectArray)obj, i);		
		
		jshort* colData = env->GetShortArrayElements((jshortArray)colArray, 0 );		
		
		for (int j=0; j<n; j++)
		{
			data[i*n +j] = colData[j];
		}
		env->ReleaseShortArrayElements((jshortArray)colArray, colData, 0);
	}
	return;
}

void Check_exception(JNIEnv*& env, int line, const char* append_str)
{
	if(env->ExceptionCheck())
	{
		char buf[128];
		sprintf(buf, "Exception in %s line:%d. Info:%s", __FILE__, line, append_str);
		env->ThrowNew(env->FindClass("android/util/AndroidException")
			, buf);		
	}
}
