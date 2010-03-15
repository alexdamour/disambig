#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

DB_ENV *dbenv;

int main(int argc, char** argv){
    DB* primary, block, count;
    DBC* cur;
    DBT key, data;

    DB_CLEAR(key);
    DB_CLEAR(data);
    
    sqlite_db_env_open(NULL);

    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, DB_CREATE, 0, compar_uint32);
    sqlite_db_secondary_open(primary, &block, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);
    sqlite_db_secondary_open(block, &count, "count_db", 8*1024, DB_DUPSORT, count_callback, NULL);

    

    return(0);
}

int count_callback(DB* db_secondary, const DBT* key, const DBT* data, DBT* result){
    db_recno_t count;
    db_secondary
