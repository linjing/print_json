cc -shared -Iinclude -MMD -DNDEBUG  -g -Werror -Wall -fPIC -O2 -c -o print_json.o print_json.c
cc -shared -o libprint_json.so print_json.o  -Llib 
ar rs libprint_json.a print_json.o
cc -shared -Iinclude -MMD -DNDEBUG  -g -Werror -Wall -fPIC -c -o test.o test.c
cc -o test test.o libprint_json.a -L.
