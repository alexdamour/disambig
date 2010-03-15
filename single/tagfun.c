#include "comp_engine.h"

char* has_tag(DbRecord *data){
    if(0 == strncmp(data->Invnum_N, "\0", 1))
        return NULL;
    else
        return data->Invnum_N;
}

int apply_tag(DbRecord *data, char *tagp){
    char invnum_buf[16];
    if(tagp == NULL){
        memset(invnum_buf, '\0', 16);
        sprintf(invnum_buf, "%s-%lu", data->Patent, (u_long)data->InvSeq);
        memcpy(data->Invnum_N, invnum_buf, 16);
    }
    else
       memcpy(data->Invnum_N, tagp, 16);
    return(0);
}
