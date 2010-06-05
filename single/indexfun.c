#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"
#include <math.h>

#define MAX(X,Y) (X>Y)?X:Y

/*
 * Globals
 */

int index_callback(DB* db_secondary, const DBT*  key, const DBT* data, DBT* result){
    char *pat_seq;
    DbRecord *rec = (DbRecord*)data->data;
    
    result->data = rec->Invnum;
    result->size = strlen(rec->Invnum)+1;
    return(0);
}
