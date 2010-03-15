#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

DB_ENV *dbenv;
int verbose;
char *progname;

int main(int argc, char** argv){
    DB* db, *sdb, *new, *bqueue;
    DBC* cur;
    DBT key,pkey,data;
    db_recno_t dup_count;
    DB_QUEUE_STAT* dbq;
    int ret, numblocks;

    DBT_CLEAR(key);
    DBT_CLEAR(pkey);
    DBT_CLEAR(data);
    
    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&db, "primary", DB_BTREE, PRIMARY_PAGE_KB*1024, DB_CREATE, 0, compare_uint32);
    sqlite_db_primary_open(&bqueue, "block_queue_2", DB_QUEUE, 4*1024, DB_CREATE, 0, NULL);
    sqlite_db_secondary_open(db, &sdb, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);
    sqlite_db_primary_open(&new, "primary2", DB_BTREE, PRIMARY_PAGE_KB*1024, DB_CREATE, 0, compare_uint32);

    sdb->cursor(sdb, NULL, &cur, 0);
    cur->pget(cur, &key, &pkey, &data, DB_FIRST);
    printf("block: %s\n", (char*)data.data);

    bqueue->stat(bqueue, NULL, &dbq, 0);
    printf("Queue length: %lu\n", (ulong)dbq->qs_nkeys);
    
    while(DB_NOTFOUND != bqueue->get(bqueue, NULL, &key, &data, DB_CONSUME)){
        printf("block: %s\n", (char*)data.data);
        data.size=strlen(data.data)+1;
        cur->pget(cur, &data, &pkey, &key, DB_SET);
        do {
            new->put(new, NULL, &pkey, &key, 0);
        } while(DB_NOTFOUND != cur->pget(cur, &data, &pkey, &key, DB_NEXT_DUP));
    }
    
    new->close(new,0);
    bqueue->close(bqueue, 0);
    cur->close(cur);
    sdb->close(sdb, 0);
    db->close(db,0);
    dbenv->close(dbenv, 0);

    return(0);
}
