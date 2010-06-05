#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

datatype block_type = STRING;

int blocking_callback(DB* db_secondary, const DBT* key, const DBT* data, DBT* result){
    char* fname_lname;
    size_t len_lname;
    size_t len_fname;
    DbRecord *recordp = (DbRecord*)data->data;

    db_secondary=db_secondary;
    key=key;

    len_fname = strlen(recordp->Block1);
    //printf("%s, %u, ", recordp->Block1, len_fname);
    len_lname = strlen(recordp->Block2);
    //printf("%s, %u, ", recordp->Block2, len_lname);
    fname_lname = (char*) malloc(sizeof(char)*len_lname+sizeof(char)*len_fname+2);
    
    memcpy(fname_lname,recordp->Block1, len_fname);
    fname_lname[len_fname]='.';
    memcpy(fname_lname+len_fname+1, recordp->Block2, len_lname);
    fname_lname[len_lname+len_fname+1]='\0';
    //printf("%s\n", fname_lname);
    
    result->data = fname_lname;
    result->size = sizeof(char)*len_lname+sizeof(char)*len_fname+2;
    result->flags = result->flags | DB_DBT_APPMALLOC;
    return(0);
}
