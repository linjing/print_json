#include <assert.h>
#include <printf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
#  include <sys/syscall.h>
#endif
#include <sys/types.h>
#include <unistd.h>


#include "print_json.h"
#ifndef FORMAT_DETAIL_DEBUG
#  define log_dbg(fmt,...)
//#  define log_dbg(fmt,...) do { fprintf (stderr, fmt, ##__VA_ARGS__);fflush (stdout); } while (0)
#else
#  define log_dbg(fmt,...) do { fprintf (stderr, fmt, ##__VA_ARGS__);fflush (stdout); } while (0)
#endif

int gettid()
{
    return (int) syscall( __NR_gettid );
}

// IMPL LIMITS
#define ARG_MAX_LEN 20
#define MAX_LEN 1<<11
//static const int MAX_LEN = 1<<11;
// error: variably modified ‘buf’ at file scope. when to define buffer_t

static bool format_impl (int level, va_list al);
static bool json_escape (char *buffer, int length, int buf_size);

void print_json (int level, ...)
{
    va_list al;
    va_start (al, level);
    format_impl (level, al);
    va_end (al);
}

static bool is_blank (const char ch) {
  if (ch == '\t' || ch == ' ' || ch == '\n' || ch == '\r' || ch == '\f'
      || ch == '\v')
    return true;
  else
    return false;
}

static bool json_escape (char *buffer, int length, int buf_size)
{
    char tmp[MAX_LEN];
    strcpy (tmp, buffer);
    int i;
    int idx = 0;

    for (i = 0; i < length; ++i) {
        if (tmp[i] == '"' || tmp[i] == '\\') {
            if (idx + 3 >= buf_size)
                return false;
            buffer[idx++] = '\\';
            buffer[idx++] = tmp[i];
        } else if (is_blank (tmp[i])) {
            // skip blanks, eat more than one blank
            if (idx != 0 && buffer[idx-1] != ' ')
                buffer[idx++] = ' ';
        } else {
            if (idx + 2 >= buf_size)
                return false;
            buffer[idx++] = tmp[i];
        }
    }
    // terminating NULL
    buffer[idx] = '\0';
    return true;
}

typedef struct buffer_s {
  char buf[MAX_LEN];
  int used;
  int size;
} buffer_t;

bool add_json_header (buffer_t *buf)
{
  assert (buf != 0);
  if (buf->used + 1 >= buf->size)
    return false;
  buf->buf[buf->used] = '{';
  buf->used += 1;
  buf->buf[buf->used] = '\0';
  return true;
}

bool add_json_end (buffer_t *buf)
{
  assert (buf != 0);
  if (buf->buf[buf->used - 1] == ',') {
    buf->buf[buf->used - 1] = '}';
  } else {
    if (buf->used + 1 >= buf->size)
      return false;
    buf->buf[buf->used] = '}';
    buf->used += 1;
  }
  buf->buf[buf->used] = '\0';
  return true;
}

bool add_json_item (buffer_t *buf, const char *key, const char *value)
{
  snprintf (buf->buf + buf->used, buf->size - buf->used,
            "\"%s\":\"%s\",", key, value);
  buf->used = strlen (buf->buf);
  return buf->used < buf->size - 1;
}

bool add_json_next_level_start (buffer_t *buf, const char *key)
{
  snprintf (buf->buf + buf->used, buf->size - buf->used, "\"%s\":{", key);
  buf->used = strlen (buf->buf);
  return buf->used < buf->size - 1;
}

static bool format_impl (int level, va_list al)
{
  assert (level == 1 || level == 2);
  buffer_t buf = {.used = 0, .size = MAX_LEN};
  buf.buf[buf.size - 1] = '\0';


  log_dbg ("=level: %d\n", level);

  if (!add_json_header (&buf)) return false;

  if (level == 2) {
    if (!add_json_item (&buf, "first_level", "TODO")) return false;
    log_dbg ("==add fix key [%s]\n", buf.buf);
    // TODO, add one level
    if (!add_json_next_level_start (&buf, "second_level_data")) return false;
    log_dbg ("==start seconde level [%s]\n", buf.buf);
  }

  while (1) {
    const char * key = NULL;
    const char * fmt = NULL;
    char value[MAX_LEN];
    int args[ARG_MAX_LEN];
    int arg_no;

    key = va_arg (al, const char *);
    if (!key) break;

    fmt = va_arg (al, const char *);
    if (!fmt) break;

    va_list ap_tmp;
    va_copy (ap_tmp, al);
    vsnprintf (value, buf.size - 1, fmt, ap_tmp);
    va_end (ap_tmp);
    json_escape (value, strlen (value), MAX_LEN);

    log_dbg ("===debug: key [%s] fmt [%s]  value [%s]\n", key, fmt, value);
    add_json_item (&buf, key, value);

    // forwart __VA_ARGS__
    arg_no = parse_printf_format (fmt, ARG_MAX_LEN, args);
    log_dbg ("=== debug arg no is %d\n", arg_no);
    int i;

    for (i = 0; i < arg_no; ++i) {
      log_dbg ("args[i] & ~PA_FLAG_MASK %d %d\n", args[i] & ~PA_FLAG_MASK,
          args[i] & PA_FLAG_MASK);
      switch (args[i] & ~PA_FLAG_MASK) {
        case PA_INT:
          if ((args[i] & PA_FLAG_MASK) == PA_FLAG_LONG_LONG)
            (void) va_arg (al, long long);
          else if ((args[i] & PA_FLAG_MASK) == PA_FLAG_LONG)
            (void) va_arg (al, long);
          else
            (void) va_arg (al, int);
          break;
        case PA_CHAR:
          (void) va_arg (al, int);
          break;
        case PA_STRING:
          (void) va_arg (al, char *);
          break;
        case PA_POINTER:
          (void) va_arg (al, void *);
          break;
        case PA_FLAG_LONG_LONG:
          (void) va_arg (al, long long);
          break;
        case PA_FLOAT:
          // FALLTHROUGH
        case PA_DOUBLE: // long is double type base
          (void) va_arg (al, double);
          break;
        default:
          ;
          // log_dbg ("args[i] & ~PA_FLAG_MASK %d\n", args[i] & ~PA_FLAG_MASK);
      }
    }
  }
  log_dbg ("== all item finished [%s]\n", buf.buf);
  if (level == 2) {
    if (!add_json_end (&buf)) return false;
  }
  if (!add_json_end (&buf)) return false;
  printf ("%s\n", buf.buf);
  log_dbg ("=log end\n");
  return true;
}

