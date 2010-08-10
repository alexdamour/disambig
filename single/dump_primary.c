#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

DB_ENV	 *dbenv;			/* Database environment */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */

char *table = "invpat_3_sort";
char *database = "invpatC.v3.sqlite3";

int main(int argc, char *argv[])
{
    DB *primary, *block_db;
    DBC *cur, *tmp_cur;
    DbRecord *rp;
    DbField *f;
    sqlite3 *sql_db;
    sqlite3_stmt *ppStmt;
    DBT key, pkey, data, uid;
    int sql_ret;
    sqlite3_int64 mem_lim = 1*1024*1024;
    mem_lim *= 1024;
    u_int32_t numrecs = 0, numblocks=0;
    //DB_BTREE_STAT *stat;
    //char sql_query[256];
    //sprintf(sql_query, "update %s set Invnum_N = ? where invnum = ?;", table);
    char * sql_query = "update invpat_8_sort set Invnum_N = ? where invnum = ?;";
    //char sql_file[128];
    //sprintf(sql_file, "sqlite_dbs/%s", database);
    char *sql_file = "sqlite_dbs/invpatC.v8.sqlite3";
    //char *sql_query2 = "select * from invpat where Patent = ? and InvSeq = ?;";
    db_recno_t dup_count;

    int custom_block=0;
    int has_max = 0;
    u_int32_t start_rec = 0, end_block = 0;
    char ch;

    memset(&key, 0, sizeof(key));
    memset(&pkey, 0, sizeof(pkey));
    memset(&data, 0, sizeof(data));
    memset(&uid, 0, sizeof(uid));

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, DB_CREATE, 0, compare_uint32);
    sqlite_db_secondary_open(primary, &block_db, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);
    //primary->stat(primary, NULL, &stat, 0);
    //printf("numkeys in primary: %lu\n", (u_long)stat->bt_nkeys);
    //free(stat);
    sqlite3_open(sql_file, &sql_db);
    sqlite3_exec(sql_db, "PRAGMA synchronous=0;", NULL, NULL, NULL);
    //sqlite3_exec(sql_db, "PRAGMA cache_size=200;", NULL, NULL, NULL);

    block_db->cursor(block_db, NULL, &cur, 0);

    printf("Starting writes!\n");

    while ((ch = getopt(argc, argv, "b:n:x:")) != EOF)
		switch (ch) {
		case 'b':
            key.data = optarg;
            key.size = strlen(optarg)+1;
            printf("%s, %u\n", key.data, key.size);
            custom_block = 1;
            if(DB_NOTFOUND == cur->pget(cur, &key, &pkey, &data, DB_SET)){
                printf("Block not found\n");
                return(EXIT_FAILURE);
            }
			break;
        case 'n':
            start_rec = (u_int32_t)atol(optarg);
            break;
        case 'x':
            end_block = (u_int32_t)atol(optarg);
            has_max = 1;
            break;
        }
    argc -= optind;
	argv += optind;

    if (*argv != NULL)
		return (EXIT_FAILURE);

    sql_ret = sqlite3_prepare_v2(sql_db, sql_query, -1, &ppStmt, NULL);

    while(DB_NOTFOUND != cur->pget(cur, &key, &pkey, &data,
                (custom_block? DB_SET : DB_NEXT_NODUP))){
        if(has_max && numblocks++ >= end_block) break;
        do{
            if(!(++numrecs % 10000)){
                primary->sync(primary, 0);
                block_db->sync(primary, 0);
                dbenv->memp_sync(dbenv, NULL);
                cur->dup(cur, &tmp_cur, DB_POSITION);
                tmp_cur->pget(tmp_cur, &key, &pkey, &data, DB_CURRENT);
                cur->close(cur);
                cur = tmp_cur;
                sqlite3_finalize(ppStmt);
                sqlite3_close(sql_db);
                sqlite3_open(sql_file, &sql_db);
                sqlite3_exec(sql_db, "PRAGMA synchronous=0;", NULL, NULL, NULL);
                sql_ret = sqlite3_prepare_v2(sql_db, sql_query, -1, &ppStmt, NULL);
                printf("\t %lu records processed...\n", (ulong)numrecs);
                //printf("\t Using %lld bytes of memory...\n", sqlite3_memory_used());
            }

            cur->count(cur, &dup_count, 0);
            if(numrecs < start_rec) continue;

            if(custom_block || (strncmp(((DbRecord*)data.data)->Invnum_N, "", 1) &&
               strcmp(((DbRecord*)data.data)->Invnum_N, ((DbRecord*)data.data)->Invnum))){
                
                //printf("Invnum_N = %s\n", ((DbRecord *)data.data)->Invnum_N);
                //printf("Patent = %s\n", ((DbRecord*)data.data)->Patent);
                //printf("InvSeq = %lu\n", (u_long)((DbRecord*)data.data)->InvSeq);
                
                sql_ret = sqlite3_prepare_v2(sql_db, sql_query, -1, &ppStmt, NULL);
                //sql_ret = sqlite3_prepare_v2(sql_db, sql_query2, -1, &ppStmt, NULL);
                /*if(numrecs > 920000){
                    if(has_tag((DbRecord*)data.data)==NULL)
                        printf("HUZZAH!\n");
                    printf("Invnum_N: %s\n", has_tag((DbRecord *)data.data));
                    printf("%d\n", sql_ret);
                }*/
                if((sql_ret = sqlite3_bind_text(ppStmt, 1, ((DbRecord *)data.data)->Invnum_N,  -1, SQLITE_STATIC))!=0)//strlen(((DbRecord *)data.data)->Invnum_N)+1
                    printf("Bind complaint: %d\n", sql_ret);
                index_callback(NULL, NULL, &data, &uid);
                if((sql_ret = sqlite3_bind_text(ppStmt, 2, (char*)uid.data, -1, SQLITE_STATIC))!=0)
                    printf("Bind complaint: %d\n", sql_ret);
                //if((sql_ret=sqlite3_bind_int(ppStmt, 3, (int)((DbRecord *)data.data)->InvSeq))!=0)
                //    printf("Bind complaint: %d\n", sql_ret);
                //if((sql_ret = sqlite3_step(ppStmt))!=SQLITE_ROW)
                //    printf("No results! %d\n", sql_ret);
                if((sql_ret = sqlite3_step(ppStmt))!=SQLITE_DONE)
                    printf("Step complaint: %d\n", sql_ret);
                sqlite3_clear_bindings(ppStmt);
                sqlite3_reset(ppStmt);
        
            } 
        } while(DB_NOTFOUND !=
            cur->pget(cur, &key, &pkey, &data, DB_NEXT_DUP));
        if(custom_block) break;
    }
    sqlite3_finalize(ppStmt);
    cur->close(cur);
    block_db->close(block_db, 0);
    primary->close(primary, 0);
    sqlite3_close(sql_db);
    return(0);
}
