

#include <jni.h>
#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>

#include "SkBitmap.h"
#include "GraphicsJNI.h"
#include "SkPixelRef.h"

//#include "utils/Log.h"

//#include "GraphicsJNI.h"
#include <cutils/xlog.h>


#define LOG_TAG "AGIF_ENCODER_JNI"


//static const char *classPathName = "com/mediatek/mpo/MpoDecoder";
//static jclass       mpoDecoder_class;
//static jmethodID    mpoDecoder_constructorMethodID;
//static jfieldID     mpoDecoder_nativeInstanceID;

#if 0
#include "SkMovie.h"
#include "SkStream.h"
#include "GraphicsJNI.h"
#include "SkTemplates.h"
#include "SkUtils.h"
//#include "CreateJavaOutputStreamAdaptor.h"
#endif

#include "AGifEncoder.h"

#include <androidfw/Asset.h>
#include <androidfw/ResourceTypes.h>
#include <netinet/in.h>
#include "utils/Log.h"

//==================================================================================

static jmethodID    gOutputStream_writeMethodID;
static jmethodID    gOutputStream_flushMethodID;

#define RETURN_NULL_IF_NULL(value) \
    do { if (!(value)) { SkASSERT(0); return NULL; } } while (false)


class GifSkJavaOutputStream : public SkWStream {
public:
    GifSkJavaOutputStream(JNIEnv* env, jobject stream, jbyteArray storage)
        : fEnv(env), fJavaOutputStream(stream), fJavaByteArray(storage) {
        fCapacity = env->GetArrayLength(storage);
    }

	virtual bool write(const void* buffer, size_t size) {
        JNIEnv* env = fEnv;
        jbyteArray storage = fJavaByteArray;

        while (size > 0) {
            size_t requested = size;
            if (requested > fCapacity) {
                requested = fCapacity;
            }

            env->SetByteArrayRegion(storage, 0, requested,
                                    reinterpret_cast<const jbyte*>(buffer));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                SkDebugf("--- write:SetByteArrayElements threw an exception\n");
                return false;
            }

            fEnv->CallVoidMethod(fJavaOutputStream, gOutputStream_writeMethodID,
                                 storage, 0, requested);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                SkDebugf("------- write threw an exception\n");
                return false;
            }

            buffer = (void*)((char*)buffer + requested);
            size -= requested;
        }
        return true;
    }

    virtual void flush() {
        fEnv->CallVoidMethod(fJavaOutputStream, gOutputStream_flushMethodID);
    }

private:
    JNIEnv*     fEnv;
    jobject     fJavaOutputStream;  // the caller owns this object
    jbyteArray  fJavaByteArray;     // the caller owns this object
    size_t      fCapacity;
};

SkWStream* GifCreateJavaOutputStreamAdaptor(JNIEnv* env, jobject stream,
                                         jbyteArray storage) {
    static bool gInited;

    if (!gInited) 
    {
    XLOGI("AGIFE::GifCreateJavaOutputStreamAdaptor Init %d, go L:%d!!\n",gInited, __LINE__);
        jclass outputStream_Clazz = env->FindClass("java/io/OutputStream");
        RETURN_NULL_IF_NULL(outputStream_Clazz);

        gOutputStream_writeMethodID = env->GetMethodID(outputStream_Clazz,
                                                       "write", "([BII)V");
        RETURN_NULL_IF_NULL(gOutputStream_writeMethodID);
        gOutputStream_flushMethodID = env->GetMethodID(outputStream_Clazz,
                                                       "flush", "()V");
        RETURN_NULL_IF_NULL(gOutputStream_flushMethodID);

        gInited = true;
    }

    return new GifSkJavaOutputStream(env, stream, storage);
}


//=======================================================================================



static jclass       gAGifEncoder_class;
static jmethodID    gAGifEncoder_constructorMethodID;
static jfieldID     gAGifEncoder_nativeInstanceID;

jobject create_jmovie(JNIEnv* env, AGifEncoder* encoder, int width, int height) {
    //XLOGI("AGIFE::create_jmovie, go L:%d!!\n", __LINE__);
    if (NULL == encoder) {
        XLOGI("AGIFE::create_jmovie create fail, go L:%d!!\n", __LINE__);
        return NULL;
    }
#if 1
    jobject obj = env->AllocObject(gAGifEncoder_class);
    if (obj) {
        env->CallVoidMethod(obj, gAGifEncoder_constructorMethodID, (jint)encoder, (jint)width, (jint)height );
    }
    return obj;
#else    
    return env->NewObject(gAGifEncoder_class, gAGifEncoder_constructorMethodID,
            static_cast<jint>(reinterpret_cast<uintptr_t>(encoder)));
#endif            
}


static AGifEncoder* J2AGifEncoder(JNIEnv* env, jobject movie) {
    SkASSERT(env);
    SkASSERT(movie);
    SkASSERT(env->IsInstanceOf(movie, gAGifEncoder_class));
    AGifEncoder* m = (AGifEncoder*)env->GetIntField(movie, gAGifEncoder_nativeInstanceID);
    SkASSERT(m);
    
    XLOGI("AGIFE::J2AGifEncoder get encoder %x->%x, go L:%d!!\n", (unsigned int)movie, (unsigned int)m,__LINE__);
    return m;
}

///////////////////////////////////////////////////////////////////////////////

static int agifenc_width(JNIEnv* env, jobject movie, AGifEncoder* enc) {
    XLOGI("AGIFE::agifenc_width, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    if(enc != NULL)
      return enc->width();
    else
      return 0 ;
}

static int agifenc_height(JNIEnv* env, jobject movie, AGifEncoder* enc) {
    XLOGI("AGIFE::agifenc_height, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    if(enc != NULL)
      return enc->height();
    else
      return 0 ;    
    
}

static int agifenc_duration(JNIEnv* env, jobject movie, AGifEncoder* enc) {
    XLOGI("AGIFE::agifenc_duration, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    if(enc != NULL)
      return enc->duration();
    else
      return 0 ; 
}

static jboolean agifenc_setDuration(JNIEnv* env, jobject movie, AGifEncoder* enc, int ms) {
    XLOGI("AGIFE::agifenc_setDuration, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    if(enc!=NULL)
      return enc->setFrameDuration(ms);
    else
      return false ;
    //return enc->setFrameDuration(ms);
    
}

static jboolean agifenc_setWidth(JNIEnv* env, jobject movie, int width) {
    XLOGI("AGIFE::agifenc_setWidth, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    AGifEncoder* m = J2AGifEncoder(env, movie);
    //XLOGI("AGIFE::agifenc_setWidth encoder %x, go L:%d!!\n", (unsigned int)m,__LINE__);     
    return m->setWidth(width);
}

static jboolean agifenc_setHeight(JNIEnv* env, jobject movie, int height) {
    XLOGI("AGIFE::agifenc_setHeight, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    AGifEncoder* m = J2AGifEncoder(env, movie);
    //XLOGI("AGIFE::agifenc_setHeight encoder %x, go L:%d!!\n", (unsigned int)m,__LINE__);     
    return m->setHeight(height);
}



//static jboolean agifenc_gifFrameBitmap(JNIEnv* env, jobject movie, jintArray pixelArray/*, SkBitmap* bitmap*/) 
//static jboolean agifenc_gifFrameBitmap(JNIEnv* env, jobject movie, AGifEncoder* enc, jintArray pixelArray) 
static jboolean agifenc_gifFrameBitmap(JNIEnv* env, jobject movie, AGifEncoder* enc, jintArray pixelArray, jobject jstream, jbyteArray jstorage) 
{
   bool ret = false ;


    jint* dst = env->GetIntArrayElements(pixelArray, NULL);
    SkWStream* strm = GifCreateJavaOutputStreamAdaptor(env, jstream, jstorage);

    if(dst == NULL){
      XLOGI("AGIFE::agifenc_gifFrameBitmap NULL bitmap, go L:%d!!\n",__LINE__);      
      return false;
    }
    if(strm != NULL){
      
      XLOGI("AGIFE::agifenc_gifFrameBitmap %x, go L:%d!!\n", dst,__LINE__);
      ret = enc->encodeBitmap((unsigned char *)dst, strm);
      env->ReleaseIntArrayElements(pixelArray, dst, 0);   
      delete strm; 
      //return ret;
    } 
    return ret ;
    

}

static jobject agifenc_CreateAGifEncoder(JNIEnv* env, jobject movie, int width, int height, jobject jstream, jbyteArray jstorage) 
{

    jobject obj ;
    XLOGI("AGIFE::agifenc_CreateAGifEncoder, go L:%d!!\n", __LINE__);
    
    AGifEncoder* encoder = new AGifEncoder();       
    encoder->setWidth(width);
    encoder->setHeight(height);
    encoder->setFrameDuration(100);
    //encoder->setEncodeStream(strm);

    obj = create_jmovie(env, encoder, width, height);
       
{
#if 0
    NPE_CHECK_RETURN_ZERO(env, obj);
    XLOGI("AGIFE::create_jmovie check encoder, go L:%d!!\n", __LINE__);     
    AGifEncoder* m = J2AGifEncoder(env, obj);
    XLOGI("AGIFE::create_jmovie check encoder %x, go L:%d!!\n", (unsigned int)m,__LINE__);     
#endif
}       
       
    XLOGI("AGIFE::create_jmovie check obj %x, go L:%d!!\n", (unsigned int)obj,__LINE__);     
   
       
    return obj ;
   
}


static jobject agifenc_CreateEncoder(JNIEnv* env, jobject movie, int width, int height, jobject jstream, jbyteArray jstorage) 
{

    jobject obj ;
    //XLOGI("AGIFE::agifenc_CreateEncoder, go L:%d!!\n", __LINE__);
    //SkWStream* strm = GifCreateJavaOutputStreamAdaptor(env, jstream, jstorage);
    


       //XLOGI("AGIFE::create_jmovie, go L:%d!!\n", __LINE__);       
       AGifEncoder* encoder = new AGifEncoder();       
       XLOGI("AGIFE::create_jmovie encoder %x, go L:%d!!\n", encoder,__LINE__);     
       encoder->setWidth(width);
       encoder->setHeight(height);
       encoder->setFrameDuration(100);
       //encoder->setEncodeStream(strm);

       //create_jmovie(env, encoder);
       //delete strm;
       obj = create_jmovie(env, encoder, width,height);
#if 0       
{
       NPE_CHECK_RETURN_ZERO(env, obj);
       XLOGI("AGIFE::create_jmovie check encoder, go L:%d!!\n", __LINE__);     
       AGifEncoder* m = J2AGifEncoder(env, obj);
       XLOGI("AGIFE::create_jmovie check encoder %x, go L:%d!!\n", (unsigned int)m,__LINE__);    
}       
#endif       
       //XLOGI("AGIFE::create_jmovie check obj %x, go L:%d!!\n", (unsigned int)obj,__LINE__);     
   
       
       return obj ;
   
}


static jboolean agifenc_setOutputStream(JNIEnv* env, jobject movie, AGifEncoder* enc, jobject jstream, jbyteArray jstorage) 
{

    XLOGI("AGIFE::agifenc_setOutputStream obj %x, enc %x, go L:%d!!\n",movie, enc,__LINE__);  
    SkWStream* strm = GifCreateJavaOutputStreamAdaptor(env, jstream, jstorage);
    if (NULL != strm) {

       XLOGI("AGIFE::setOutputStream encoder %x, go L:%d!!\n", (unsigned int)enc,__LINE__);       
       
       enc->setEncodeStream(strm);//(height);
       delete strm;
       
       return true ;

        //return false;
    }
    return true ;

   
}



static int agifenc_encodeFrameCount(JNIEnv* env, jobject movie) {

    XLOGI("AGIFE::agifenc_encodeFrameCount, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_ZERO(env, movie);
    AGifEncoder* m = J2AGifEncoder(env, movie);
//LOGE("Movie:movie_getEncodeFrameCount: frame count %d", m->getGifTotalFrameCount());
    return m->getGifTotalFrameCount();
}   

static void agifenc_closeGif(JNIEnv* env, jobject movie, jobject jstream, jbyteArray jstorage) {

    XLOGI("AGIFE::agifenc_closeGif, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_VOID(env, movie);
    AGifEncoder* m = J2AGifEncoder(env, movie);
    SkWStream* strm = GifCreateJavaOutputStreamAdaptor(env, jstream, jstorage);
    
    //XLOGI("AGIFE::agifenc_closeGif, strm %x, go L:%d!!\n", strm,__LINE__);
    if (NULL != strm) {

      m->setEncodeStream(strm);
      m->closeGif();    
      delete strm;
      delete m;
    }    
}

static void agifenc_closeStream(JNIEnv* env, jobject movie, AGifEncoder* enc, jobject jstream, jbyteArray jstorage) {

    XLOGI("AGIFE::agifenc_closeStream, go L:%d!!\n", __LINE__);
    NPE_CHECK_RETURN_VOID(env, movie);
    //AGifEncoder* m = J2AGifEncoder(env, movie);
    
    SkWStream* strm = GifCreateJavaOutputStreamAdaptor(env, jstream, jstorage);
    
    if(enc != NULL){
      XLOGI("AGIFE::agifenc_closeStream stream %x, go L:%d!!\n", (unsigned int)enc,__LINE__);
      enc->setEncodeStream(strm);
      enc->closeGif();          
      delete enc;
    }

    if (NULL != strm){
      delete strm;
    } 
    env->SetIntField(movie,gAGifEncoder_nativeInstanceID,0);
}

// for add gif end



static void agifenc_destructor(JNIEnv* env, jobject movie, AGifEncoder* enc) {
   
    XLOGI("AGIFE::agifenc_destructor, go L:%d!!\n", __LINE__);
   if(enc != NULL){   
     XLOGI("AGIFE::agifenc_destructor delete encoder %x, go L:%d!!\n", (unsigned int)enc,__LINE__);   
     delete enc;
   }
   env->SetIntField(movie,gAGifEncoder_nativeInstanceID,0);
}

//////////////////////////////////////////////////////////////////////////////////////////////

#include <android_runtime/AndroidRuntime.h>

static JNINativeMethod gMethods[] = {
    {   "nativeWidth"    ,    "(I)I",  (void*)agifenc_width  },
    {   "nativeHeight"   ,    "(I)I",  (void*)agifenc_height  },
    {   "nativeDuration" ,    "(I)I",  (void*)agifenc_duration  },
    //{   "setWidth" ,    "(I)Z",  (void*)agifenc_setWidth  },
    //{   "setHeight" ,   "(I)Z",  (void*)agifenc_setHeight  },

    {   "nativeSetDuration" ,    "(II)Z",  (void*)agifenc_setDuration  },
    {   "nativeEncodeFrameCount" ,   "()I", (void*)agifenc_encodeFrameCount  },    
    //{   "encodeBitmap"      ,   "(Landroid/graphics/Bitmap;)Z", (void*)agifenc_gifFrameBitmap  },
    //{   "nativeEncodeBitmap"      ,   "(I)Z", (void*)agifenc_gifFrameBitmap  },    
    //{   "nativeEncodeBitmap"      ,   "(I[I)Z", (void*)agifenc_gifFrameBitmap  },    
    {   "nativeEncodeBitmap"      ,   "(I[ILjava/io/OutputStream;[B)Z", (void*)agifenc_gifFrameBitmap  },    
    //{   "nativeEncodeBitmap"      ,   "([ILjava/io/OutputStream;[B)Z", (void*)agifenc_gifFrameBitmap  },    
    {   "nativeCloseStream"            ,   "(ILjava/io/OutputStream;[B)Z" , (void*)agifenc_closeStream  },
    {   "nativeCloseGif"            ,   "(Ljava/io/OutputStream;[B)Z" , (void*)agifenc_closeGif  },
    { "nativeDestructor"      ,  "(I)V", (void*)agifenc_destructor },
    { "nativeSetOutputStream"       , "(ILjava/io/OutputStream;[B)Z", (void*)agifenc_setOutputStream },
    { "nativeCreateEncoder"       , "(IILjava/io/OutputStream;[B)Lcom/mediatek/agifencoder/AGifEncoder;", (void*)agifenc_CreateEncoder },
    { "nativeCreateAGifEncoder"       , "(II)Lcom/mediatek/agifencoder/AGifEncoder;", (void*)agifenc_CreateAGifEncoder },
    

//    { "nativeSetOutputStream"       , "(Ljava/io/OutputStream;)Z", (void*)agifenc_setOutputStream },
//    {   "isOpaque" ,    "()Z",  (void*)movie_isOpaque  },
//    {   "setTime"  ,   "(I)Z",  (void*)movie_setTime  },
//    {   "draw"     ,     "(Landroid/graphics/Canvas;FFLandroid/graphics/Paint;)V", (void*)movie_draw  },
//    {   "gifFrameDuration"    ,     "(I)I", (void*)movie_gifFrameDuration  },
//    {   "gifFrameBitmap"      ,   "(I)Landroid/graphics/Bitmap;", (void*)agifenc_gifFrameBitmap  },
//    { "decodeMarkedStream"    ,  "(Ljava/io/InputStream;)Landroid/graphics/Movie;", (void*)movie_decodeStream },
//    { "decodeByteArray"       ,  "([BII)Landroid/graphics/Movie;", (void*)movie_decodeByteArray },
};


static const char *kClassPathName = "com/mediatek/agifencoder/AGifEncoder"; 
//#define kClassPathName  "com/mediatek/agifencoder/AGifEncoder"

#define RETURN_ERR_IF_NULL(value)   do { if (!(value)) { assert(0); return -1; } } while (false)


/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    XLOGI("JNI_register, go L:%d!!\n", __LINE__);
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        XLOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        XLOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


int register_android_graphics_AGifEncoder(JNIEnv* env);
int register_android_graphics_AGifEncoder(JNIEnv* env)
{
    XLOGI("JNI_register, go L:%d!!\n", __LINE__);
    gAGifEncoder_class = env->FindClass(kClassPathName);
    RETURN_ERR_IF_NULL(gAGifEncoder_class);
    gAGifEncoder_class = (jclass)env->NewGlobalRef(gAGifEncoder_class);

    //XLOGI("JNI_register, go L:%d!!\n", __LINE__);
    gAGifEncoder_constructorMethodID = env->GetMethodID(gAGifEncoder_class, "<init>", "(III)V");
    RETURN_ERR_IF_NULL(gAGifEncoder_constructorMethodID);

    //XLOGI("JNI_register, go L:%d!!\n", __LINE__);
    gAGifEncoder_nativeInstanceID = env->GetFieldID(gAGifEncoder_class, "mNativeAGifEncoder", "I");
    RETURN_ERR_IF_NULL(gAGifEncoder_nativeInstanceID);

    //XLOGI("JNI_register, go L:%d!!\n", __LINE__);
    //return android::AndroidRuntime::registerNativeMethods(env, kClassPathName, gMethods, 
    //                                                      SK_ARRAY_COUNT(gMethods));

    if (!registerNativeMethods(env, kClassPathName, gMethods,
                               sizeof(gMethods) / sizeof(gMethods[0])) ) {
        return JNI_FALSE;
    }    
    XLOGI("JNI_register, go L:%d!!\n", __LINE__);
    return JNI_TRUE ;
}



/*
 * This is called by the VM when the shared library is first loaded.
 */
 
typedef union {  
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    
    XLOGI("JNI_OnLoad");
    
    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        XLOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (register_android_graphics_AGifEncoder(env) != JNI_TRUE) {
        XLOGE("ERROR: registerNatives failed");
        goto bail;
    }
    result = JNI_VERSION_1_4;
    
bail:
    return result;
}


