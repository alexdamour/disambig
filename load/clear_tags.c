#include "comp_engine.h"

DB_ENV	 *dbenv;			/* Database environment */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */

int main(int argc, char *argv[])
{
    char *tagp;
    DB *primary;
    DBC *cur;
    DbRecord *rp;
    DbField *f;
    sqlite3 *sql_db;
    sqlite3_stmt *ppStmt;
    DBT key, data;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, DB_CREATE, 0, compare_uint32);
    //primary->stat(primary, NULL, &stat, 0);
    //printf("numkeys in primary: %lu\n", (u_long)stat->bt_nkeys);
    //free(stat);
    sqlite3_open("sqlite_dbs/invpat.sqlite3", &sql_db);

    primary->cursor(primary, NULL, &cur, 0);

    while(DB_NOTFOUND != cur->get(cur, &key, &data, DB_NEXT)){
        if((tagp = has_tag((DbRecord*)data.data))!=NULL){
//            printf("found one!\n");
            memset(tagp, '\0', 16);
            primary->put(primary, NULL, &key, &data, 0);
        }
    }
    cur->close(cur);
    primary->close(primary, 0);
    return(0);
}
