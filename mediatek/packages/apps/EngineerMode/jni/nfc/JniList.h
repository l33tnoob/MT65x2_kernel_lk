// JniList.h: interface for the JniList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JNILIST_H__B7EFA548_657D_4483_8A2C_5C7BDF6F8534__INCLUDED_)
#define AFX_JNILIST_H__B7EFA548_657D_4483_8A2C_5C7BDF6F8534__INCLUDED_


#include <jni.h>

class JniList  
{
public:
	JniList(JNIEnv*, jobject);
	virtual ~JniList();
public:
	jobject Get(jint idx);
	jobject Set(jint idx, jobject& newVal);
	jboolean Add(jobject obj);
	jint Size();
private:
	JniList();
	JNIEnv* m_env;
	jobject m_list_obj;
	jclass m_list_class;
};



JniList::JniList(JNIEnv* env, jobject obj)
: m_env(env)
, m_list_obj(obj)
, m_list_class(0)
{
	m_list_class = m_env->GetObjectClass(obj);
	m_list_obj = m_env->NewGlobalRef(obj);
}

JniList::~JniList()
{
	m_env->DeleteGlobalRef(m_list_obj);
}


inline jobject JniList::Get(jint idx)
{
	jmethodID id = m_env->GetMethodID(m_list_class, "get", "(I)Ljava/lang/Object;");
	return (m_env->CallObjectMethod(m_list_obj, id, idx));
}


inline jobject JniList::Set(jint idx, jobject& newVal)
{
	jmethodID id = m_env->GetMethodID(m_list_class, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");
	return m_env->CallObjectMethod(m_list_obj, id, idx, newVal);
}

inline jboolean JniList::Add(jobject obj)
{
	jmethodID id = m_env->GetMethodID(m_list_class, "add", "(Ljava/lang/Object;)Z");
	return m_env->CallBooleanMethod(m_list_obj, id, obj);
}

inline jint JniList::Size()
{
	jmethodID id = m_env->GetMethodID(m_list_class, "size", "()I");
	return m_env->CallIntMethod(m_list_obj, id);

}

#endif // !defined(AFX_JNILIST_H__B7EFA548_657D_4483_8A2C_5C7BDF6F8534__INCLUDED_)
