#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <alloca.h>
#include <string.h>
#include "json.h"

static void print_depth_shift(int depth)
{
        int j;
        for (j=0; j < depth; j++) {
                //printf(" ");
        }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.object.length;
        for (x = 0; x < length; x++) {
                print_depth_shift(depth);
                //printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
                process_value(value->u.object.values[x].value, depth+1);
        }
}

static void process_array(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.array.length;
        //printf("array\n");
        for (x = 0; x < length; x++) {
                process_value(value->u.array.values[x], depth);
        }
}

static void process_value(json_value* value, int depth)
{
        int j;
        if (value == NULL) {
                return;
        }
        if (value->type != json_object) {
                print_depth_shift(depth);
        }
        switch (value->type) {
                case json_none:
                        //printf("none\n");
                        break;
                case json_object:
                        process_object(value, depth+1);
                        break;
                case json_array:
                        process_array(value, depth+1);
                        break;
                case json_integer:
                        //printf("int: %10" PRId64 "\n", value->u.integer);
                        break;
                case json_double:
                        //printf("double: %f\n", value->u.dbl);
                        break;
                case json_string:
                        //printf("string: %s\n", value->u.string.ptr);
                        break;
                case json_boolean:
                        //printf("bool: %d\n", value->u.boolean);
                        break;
        }
}

#ifdef USE_FUZZ
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size){

        char* code;


	if(Size > 1){
        json_value * res = json_parse((char *)Data , Size);
	//process_value(res, 0);
	json_value_free(res);
	}

        return 0;
}
#endif
#ifndef USE_FUZZ
int main(int argc, char **argv) {
		json_value * res = json_parse("[1,2,3,4,5]" , strlen("[1,2,3,4,5]"));
		process_value(res, 0);
		json_value_free(res);
		json_value * res2 = json_parse("tru" , 3);
		process_value(res2, 0);
		json_value_free(res2);
return 0;
}
#endif
