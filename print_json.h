#ifndef INCLUDED_print_json_H
#define INCLUDED_print_json_H

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__cplusplus)
extern "C" {
#endif

int gettid();

// TODO, now only discard key-value pair when buffer is not enough

// define macro "G" (short for group)
// marco "G" can used to test fmt and va_args whether is right,
//   for building DEBUG version. G helps to reduce error
#ifdef NDEBUG
#  define G(key,fmt,...) key, fmt, ##__VA_ARGS__
#else
#  define G(key,fmt,...) (char*) ((fprintf(stderr, fmt, ##__VA_ARGS__) & 0) | (long)key), fmt, ##__VA_ARGS__
#endif

// args, support multi groups and end with NULL
//    G(key-1, value-fmt-1, value-1.1,...),
//    G(key-2, value-fmt-2, value-2.1,...),
//    NULL
// as:
//    print_json (1_or_2,                      /* level */
//          "action", "%s" , "vddb_query",       /* one group */
//          "error_code", "%d", 1234,            /* one group */
//          "msg", "qe error. cost %.4f", 23.9,  /* one group */
//          NULL);                               /* end with NULL */
void print_json (int level, ...);

// marco start with V will add fields (FILE LINE TID) in json
#define P_JSON(level,...) print_json (level, __VA_ARGS__, NULL)
#define VP_JSON(level,...) print_json (level, "FILE", "%s", __FILE__,\
    "LINE", "%d", __LINE__, "TID", "%d", gettid(),\
    __VA_ARGS__, NULL)

#if defined(__cplusplus)
} // end: extern C {
#endif

#endif // end: HEADER
