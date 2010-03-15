#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

datatype block_type = STRING;

int blocking_callback(DB* db_secondary, const DBT* key, const DBT* data, DBT* result){
    char* finit_lname;
    size_t len_lname;
    DbRecord *recordp = (DbRecord*)data->data;

    db_secondary=db_secondary;
    key=key;

    len_lname = strlen(recordp->Lastname);
    finit_lname = (char*) malloc(sizeof(char)*len_lname+3);
    
    finit_lname[0]=recordp->Firstname[0];
    finit_lname[1]='.';
    memcpy(finit_lname+2, recordp->Lastname, len_lname);
    finit_lname[len_lname+2]='\0';
    
    result->data = finit_lname;
    result->size = sizeof(char)*len_lname+3;
    result->flags = result->flags | DB_DBT_APPMALLOC;
    return(0);
}
