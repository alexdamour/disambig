#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

DB_ENV	 *dbenv;			/* Database environment */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */

int main(int argc, char *argv[])
{
    DB *primary;
    DBC *cur;
    DbRecord *rp;
    DbField *f;
    sqlite3 *sql_db;
    sqlite3_stmt *ppStmt;
    DBT key, data;
    int sql_ret;
    sqlite3_int64 mem_lim = 1*1024*1024;
    mem_lim *= 1024;
    u_int32_t numrecs = 0;
    //DB_BTREE_STAT *stat;
    char *sql_query = "update invpat_sort set Invnum_N = ? where Patent = ? and InvSeq = ?;";
    //char *sql_query2 = "select * from invpat where Patent = ? and InvSeq = ?;";

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, DB_CREATE, 0, compare_uint32);
    //primary->stat(primary, NULL, &stat, 0);
    //printf("numkeys in primary: %lu\n", (u_long)stat->bt_nkeys);
    //free(stat);
    sqlite3_open("sqlite_dbs/invpat.sqlite3", &sql_db);
    sqlite3_exec(sql_db, "PRAGMA synchronous=0;", NULL, NULL, NULL);
    //sqlite3_exec(sql_db, "PRAGMA cache_size=200;", NULL, NULL, NULL);

    primary->cursor(primary, NULL, &cur, 0);

    printf("Starting writes!\n");

    while(DB_NOTFOUND != cur->get(cur, &key, &data, DB_NEXT)){
        if(!(++numrecs % 1000000)){
            sqlite3_finalize(ppStmt);
            sqlite3_close(sql_db);
            sqlite3_open("sqlite_dbs/invpat.sqlite3", &sql_db);
            sqlite3_exec(sql_db, "PRAGMA synchronous=0;", NULL, NULL, NULL);
            printf("\t %lu records processed...\n", (ulong)numrecs);
            //printf("\t Using %lld bytes of memory...\n", sqlite3_memory_used());
        }
        if(strncmp(((DbRecord*)data.data)->Invnum_N, "", 1)){
            
            //printf("Invnum_N = %s\n", ((DbRecord *)data.data)->Invnum_N);
            //printf("Patent = %s\n", ((DbRecord*)data.data)->Patent);
            //printf("InvSeq = %lu\n", (u_long)((DbRecord*)data.data)->InvSeq);
            
            sql_ret = sqlite3_prepare_v2(sql_db, sql_query, -1, &ppStmt, NULL);
            //sql_ret = sqlite3_prepare_v2(sql_db, sql_query2, -1, &ppStmt, NULL);
            
            if((sql_ret = sqlite3_bind_text(ppStmt, 1, ((DbRecord *)data.data)->Invnum_N,  -1, SQLITE_STATIC))!=0)//strlen(((DbRecord *)data.data)->Invnum_N)+1
                printf("Bind complaint: %d\n", sql_ret);
            if((sql_ret = sqlite3_bind_text(ppStmt, 2, ((DbRecord *)data.data)->Patent, 8, SQLITE_STATIC))!=0)
                printf("Bind complaint: %d\n", sql_ret);
            if((sql_ret=sqlite3_bind_int(ppStmt, 3, (int)((DbRecord *)data.data)->InvSeq))!=0)
                printf("Bind complaint: %d\n", sql_ret);
            //if((sql_ret = sqlite3_step(ppStmt))!=SQLITE_ROW)
            //    printf("No results! %d\n", sql_ret);
            if((sql_ret = sqlite3_step(ppStmt))!=SQLITE_DONE)
                printf("Step complaint: %d\n", sql_ret);
            sqlite3_clear_bindings(ppStmt);
            sqlite3_reset(ppStmt);
    
       } 
    }
    sqlite3_finalize(ppStmt);
    cur->close(cur);
    primary->close(primary, 0);
    sqlite3_close(sql_db);
    return(0);
}
