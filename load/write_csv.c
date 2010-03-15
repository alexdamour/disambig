#include "comp_engine.h"

DB_ENV *dbenv;
char *progname;

int
main(int argc, char *argv[]){
    DB *ldb;
    DB *match_idx;

    DBC *match_cur;
    
    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&pairs, "lik_db", DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
    sqlite_db_
