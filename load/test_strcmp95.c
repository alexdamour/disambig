#include <stdlib.h>
#include <stdio.h>
#include "strcmp95.h"

#define MAX(X,Y) ((X > Y) ? X : Y)

int main(int argc, char** argv){
    char* s1 = "DAVID";
    char* s2 = "DAVID L.";
    size_t len = MAX(strlen(s2)+1, strlen(s1)+1);
    printf("%u\n", len);
    int *jw_opts[] = {0,0};
    double res = 0;
    res = strcmp95(s1, s2, (long)len, jw_opts);
    printf("res: %f\n", res);
    return(0);
}
