#include "comp_engine.h"

char* has_tag(DbRecord *data){
    if(0 == strncmp(data->Invnum_N, "\0", 1))
        return NULL;
    else
        return data->Invnum_N;
}

int apply_tag(DbRecord *data, char *tagp){
    //char invnum_buf[16];
    if(tagp == NULL){
        //memset(invnum_buf, '\0', 16);
        //printf("%s", data->Invnum);
        memcpy(data->Invnum_N, data->Invnum, 16);
    }
    else
       memcpy(data->Invnum_N, tagp, 16);
    return(0);
}

int tagcmp(DbRecord *data_i, DbRecord *data_j){
    return strcmp(data_i->Invnum_N, data_j->Invnum_N);
}
