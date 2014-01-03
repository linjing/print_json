#include "print_json.h"

const int one_level = 1;
const int two_level = 2;
int main ()
{
  VP_JSON (one_level,
           G("int","%d", 1),
           G("char", "%c", 'x'),
           G("long long", "%lf", (double) 1.1),
           G("string", "%s", "I'am string"),
           G("s_string", "%s", "[\\] [\"]"),
           NULL);
  print_json (one_level,
              G("int_str_longlong","x %d x %s x %lld", 5, "yy", (long long)7),
              G("int_str_longlong","x %d x %s x %lld", 5, "yy", (long long)7),
              NULL);
  print_json (two_level,
              G("int_str_longlong","x %d x %s x %lld", 5, "yy", (long long)7),
              G("long long","%lld", (long long) 7),
              NULL);
  P_JSON (two_level,
          G("test NULL ended","%s", "2010 9 4"),
          G("no NULL ended","test %d", 1));
  return 0;
}
