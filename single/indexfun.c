#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"
#include <math.h>

/*
 * Globals
 */

int index_callback(DB* db_secondary, const DBT*  key, const DBT* data, DBT* result){
    char *pat_seq;
    size_t len;
    DbRecord *rec = (DbRecord*)data->data;
    pat_seq = (char*)malloc(sizeof(char)*strlen(rec->Invnum)+1);
    len = strlen(rec->Invnum)+1;
    memcpy(pat_seq, rec->Invnum, len);
    
    result->data = pat_seq;
    result->size = len;
    result->flags = DB_DBT_APPMALLOC;
    return(0);
}
