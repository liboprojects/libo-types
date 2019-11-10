
#include "../src/sstring.c"
#include <stdio.h>

int main(int argc, char **argv) 
{
    sstring *str;
    if (string_new(&str, "Hello World") != SCT_OK) {
        fprintf(stderr, "Init failed\n");
        return 1;
    }
    printf("Value: %s, Length: \n", string_get_value(str), string_length(str));
    string_destroy(str);
    

    return 0;
}