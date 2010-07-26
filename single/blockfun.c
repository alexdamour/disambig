#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

datatype block_type = STRING;

int blocking_callback(DB* db_secondary, const DBT* key, const DBT* data, DBT* result){
    char* invnum_N; 
    DbRecord *recordp = (DbRecord*)data->data;

    db_secondary=db_secondary;
    key=key;
    
    result->data = recordp->Block1;
    result->size = strlen(recordp->Block1)+1;
    result->flags = result->flags;
    return(0);
}
