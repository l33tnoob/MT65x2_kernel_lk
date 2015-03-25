#if 1//def JPEG_THREAD_SUPPORT
#ifdef USE_WIN32_THREAD
#define _WIN32_WINNT 0x500
#include <windows.h>
#endif //USE_WIN32_THREAD

#define MCU_ROWS_IN_PER_THREAD	1

enum {
	RUNNING,
    RUNNING_IDLE,
    STOP_DONE,
    STOP_DIE
};

#ifdef USE_WIN32_THREAD
typedef struct
{
    void *handle;
    void *(*func)( void* arg );
    void *arg;
    void *ret;
} Thread_t;
#define Thread_attr_t int

/* the conditional variable api for windows 6.0+ uses critical sections and not mutexes */
typedef CRITICAL_SECTION Thread_mutex_t;
#define JPEGTHREAD_MUTEX_INITIALIZER {0}
#define Thread_mutexattr_t int

/* This is the CONDITIONAL_VARIABLE typedef for using Window's native conditional variables on kernels 6.0+.
 * MinGW does not currently have this typedef. */
typedef struct
{
    void *ptr;
} Thread_cond_t;
#define Thread_condattr_t int
#else //USE_WIN32_THREAD
/****** Pthread define******/
typedef long Thread_t;
typedef struct
{
    unsigned int flags;
    void * stack_base;
    unsigned int stack_size;
    unsigned int guard_size;
    unsigned int sched_policy;
    unsigned int sched_priority;
} Thread_attr_t;

typedef struct
{
    int volatile value;
} Thread_mutex_t;
typedef long Thread_mutexattr_t;

typedef struct
{
    int volatile value;
} Thread_cond_t;
typedef long Thread_condattr_t;
/****** End of Pthread define******/
#endif //USE_WIN32_THREAD

typedef struct
{
    Thread_t *self;
    Thread_mutex_t *mutex[2];
    Thread_cond_t *cond[2];
    void *parent;
    int index;
    int state;
	//int rows_num;
    //u32 loading;
    //u32 retval;
} ThreadContext_t;

int  JPEGThreading_init( void );
void JPEGThreading_destroy( void );
int JPEGThread_create( Thread_t *thread, const Thread_attr_t *attr, void *(*start_routine)( void* ), void *arg );
int JPEGThread_join( Thread_t thread, void **value_ptr );
int JPEGThread_mutex_init( Thread_mutex_t *mutex, const Thread_mutexattr_t *attr );
int JPEGThread_mutex_destroy( Thread_mutex_t *mutex );
int JPEGThread_mutex_lock( Thread_mutex_t *mutex );
int JPEGThread_mutex_unlock( Thread_mutex_t *mutex );
int JPEGThread_cond_init( Thread_cond_t *cond, const Thread_condattr_t *attr );
int JPEGThread_cond_destroy( Thread_cond_t *cond );
int JPEGThread_cond_broadcast( Thread_cond_t *cond );
int JPEGThread_cond_wait( Thread_cond_t *cond, Thread_mutex_t *mutex );
int JPEGThread_cond_signal( Thread_cond_t *cond );

#ifdef USE_WIN32_THREAD
#define JPEGThread_attr_init(a) 0
#define JPEGThread_attr_destroy(a) 0
#else //USE_WIN32_THREAD
int JPEGthread_attr_init( Thread_attr_t *attr );
int JPEGthread_attr_destroy(Thread_attr_t *attr );
int JPEGBindingCore( Thread_t ThreadHandle, int CPUid );
#endif //USE_WIN32_THREAD

#endif //JPEG_THREAD_SUPPORT
