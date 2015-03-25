#ifndef LIBPROFILER_H
#define LIBPROFILER_H

#include <stdio.h>

#define ___LIBPROF_ENABLE___
#define MAX_PROF_OBJECT 100

typedef void*           libprof_handle;
typedef unsigned int    libprof_uint32;

void libprof_init(void);
libprof_handle libprof_get(const char*);
void libprof_start_log(libprof_handle);
void libprof_stop_log(libprof_handle);
void libprof_dump_log(libprof_handle, FILE*);
void libprof_return(libprof_handle);

#endif // LIBPROFILER_H
