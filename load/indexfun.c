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
    size_t length = strlen(rec->Patent) + (size_t)log10(MAX((ulong)rec->InvSeq, 1)) + 3;
    
    pat_seq = (char*)malloc(length);
    sprintf(pat_seq, "%s-%lu", rec->Patent, (ulong)rec->InvSeq);
    pat_seq[length-1] = '\0';
    if(length > 14){
        DbRecord_dump(rec);
        printf("invseq: %lu\n", (ulong)rec->InvSeq);
        printf("length: %u\n", (unsigned int)length);
        printf("idx_entry: %s\n", pat_seq);
    }

    result->data = pat_seq;
    result->size = length;
    result->flags = result->flags | DB_DBT_APPMALLOC;
    return(0);
}
