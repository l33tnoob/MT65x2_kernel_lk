#include "libprofiler.h"
#include <string.h>
#include <time.h>

#if defined(__CC_ARM)
    typedef long long libprof_int_64;
#elif defined(__GNUC__)
    typedef long long libprof_int_64;
#else
    typedef __int64 libprof_int_64;
#endif

typedef struct
{
    libprof_uint32 peak_ticks;
    libprof_uint32 average_ticks;
    libprof_int_64 accumulate_ticks;
    libprof_uint32 prev_ticks;
    libprof_uint32 latest_ticks;//between latest start and stop
    libprof_uint32 log_counts;
    const char* id;
} libprof_private;

#ifdef ___LIBPROF_ENABLE___
static libprof_private s_prof_object[MAX_PROF_OBJECT];
#endif

void libprof_init()
{
#ifndef ___LIBPROF_ENABLE___
#else
    memset(s_prof_object, 0, sizeof(s_prof_object));
#endif
}

libprof_handle libprof_get(const char* _id)
{
#ifndef ___LIBPROF_ENABLE___
    return 0;
#else
    int i;
    for (i = 0; i < MAX_PROF_OBJECT; i++)
        if (0 == s_prof_object[i].id)
            break;
    if (MAX_PROF_OBJECT == i)
        return 0;
    memset(&s_prof_object[i], 0, sizeof(libprof_private));
    s_prof_object[i].id = _id;
    return (libprof_handle) &s_prof_object[i];
#endif
}

void libprof_start_log(libprof_handle handle)
{
#ifndef ___LIBPROF_ENABLE___
#else
    libprof_private * p_prof = (libprof_private*) handle;
    if (!p_prof)
        return;
    p_prof->prev_ticks = clock();
#endif
}

void libprof_stop_log(libprof_handle handle)
{
#ifndef ___LIBPROF_ENABLE___
#else
    libprof_private * p_prof = (libprof_private*) handle;
    if (!p_prof || !p_prof->prev_ticks)
        return;
    p_prof->latest_ticks = clock() - p_prof->prev_ticks;
    p_prof->prev_ticks = 0;
    p_prof->accumulate_ticks += p_prof->latest_ticks;
    if (p_prof->peak_ticks < p_prof->latest_ticks)
        p_prof->peak_ticks = p_prof->latest_ticks;
    p_prof->log_counts++;
    p_prof->average_ticks = p_prof->accumulate_ticks / p_prof->log_counts;
#endif
}

void libprof_dump_log(libprof_handle handle, FILE* fp)
{
#ifndef ___LIBPROF_ENABLE___
#else
    libprof_private * p_prof = (libprof_private*) handle;
    if (!p_prof)
        return;
    fprintf(fp, "***** libprof dump details *****\n"
                "id: %s\n"
                "average ticks: %u\n"
                "peak ticks: %u\n"
                "log counts: %u\n\n",
                p_prof->id, p_prof->average_ticks,
                p_prof->peak_ticks, p_prof->log_counts);
#endif
}

void libprof_return(libprof_handle handle)
{
#ifndef ___LIBPROF_ENABLE___
#else
    libprof_private * p_prof = (libprof_private*) handle;
    if (!p_prof)
        return;
    p_prof->id = 0;
#endif
}

