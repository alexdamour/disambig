#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

datatype block_type = STRING;

int blocking_callback(DB* db_secondary, const DBT* key, const DBT* data, DBT* result){
    DbRecord *recordp = (DbRecord*)data->data;

    //quite compiler
    db_secondary=db_secondary;
    key=key;

    result->data = ;
    result->size = ;
    result->flags = result->flags
                   //| DB_DBT_APPMALLOC
                   //Include above flag if you malloc'd the object returned in
                   //"result" data field.
    return(0);
}
