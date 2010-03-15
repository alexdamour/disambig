#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"            

/*
 * Globals
 */
DB_ENV	 *dbenv;			/* Database environment */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */

struct sqlite_workspace{
    DBT key;
    DBT data;
    u_int32_t primary_key;
    DbField *f;
};

static int build_record(sqlite3_stmt*, DbRecord*, struct sqlite_workspace*);
static int DbRecord_write(DbRecord*, DB*, struct sqlite_workspace*);
static int count_blocks(DB*);

int input_load(DB *db, sqlite3* sql_db, char* sql_table){
    int ret;
    struct sqlite_workspace w;
    DB *sdb;
    DBC *cursor;
    DBT pkey;
    memset(&pkey, 0, sizeof(pkey));
    sqlite3_stmt *ppStmt;
    DbRecord recordp;
    DB_BTREE_STAT *stat;
    db_recno_t count;
    db_recno_t max = 0;
    char big_block[128];

    if ((ret = db->cursor(db, NULL, &cursor, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->cursor");
		return (1);
	}
	memset(&(w.key), 0, sizeof(w.key));
	memset(&(w.data), 0, sizeof(w.data));
/*    
    if(DB_NOTFOUND ==
      (ret = cursor->get(cursor, &(w.key), &(w.data), DB_LAST)))
        w.primary_key = 0;
    else{
        w.primary_key = *(u_int32_t*)(w.data.data);
        w.primary_key++;
    }
*/
    ret = cursor->close(cursor);

    sqlite3_prepare_v2(sql_db, sql_query, -1, &ppStmt, NULL);
    while(SQLITE_ROW == sqlite3_step(ppStmt)){
        build_record(ppStmt, &recordp, &w);
        DbRecord_write(&recordp, db, &w);
    }
    sqlite3_finalize(ppStmt);

    db->cursor(db, NULL, &cursor, 0);
    cursor->get(cursor, &(w.key), &(w.data), DB_LAST);
    printf("last key: %s\n", (char*)w.key.data);
    //DbRecord_dump(w.data.data);

    db->stat(db, NULL, &stat, 0);
    printf("primary (blocking) nkeys: %lu\n", (u_long)(stat->bt_nkeys));
    free(stat);

    //sqlite_db_secondary_open(db, &sdb, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);
    //sdb->stat(sdb, NULL, &stat, 0);
    //printf("block_idx keys: %lu\n", (u_long)(stat->bt_nkeys));
    //free(stat);
    //sdb->cursor(sdb, NULL, &cursor, 0);
    /*
    while(DB_NOTFOUND != cursor->pget(cursor, &(w.key), &pkey, &(w.data), DB_NEXT)){
        cursor->count(cursor, &count, 0);
        if (count > max){
            max = count;
            memcpy(big_block, w.key.data, (size_t)w.key.size);
            big_block[w.key.size] = '\0';
        }
    }
    cursor->close(cursor);
    printf("Biggest block: %s\n", big_block);
    printf("%u records.\n", (size_t)max);
    */
    count_blocks(db);
    //sdb->close(sdb, 0);

    //sqlite_db_secondary_open(db, &sdb, "idx", 8*1024, 0, index_callback, NULL);
    ret = sdb->stat(sdb, NULL, &stat, 0);
    printf("%d\n", ret);
    printf("idx_keys: %lu\n", (u_long)(stat->bt_nkeys));
    free(stat);
    sdb->close(sdb, 0);
    
    return(0);
}

static int count_blocks(DB *sdb){
    DBC *cursor;
    DBT key, data;
    db_recno_t dup_count;
    char *filename = "big_blocks.txt";
    FILE *fp;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    fp = fopen(filename, "w");
    sdb->cursor(sdb, NULL, &cursor, 0);
    while(DB_NOTFOUND != cursor->get(cursor, &key, &data, DB_NEXT_NODUP)){
        cursor->count(cursor, &dup_count, 0);
        if((int)dup_count > 500)
            fprintf(fp, "%s, %u\n", (char*)key.data, (size_t)dup_count);
    }
    fclose(fp);
}

static int DbRecord_write(DbRecord *recordp, DB *db, struct sqlite_workspace *w){
    int ret;
    /*
    w->key.data = &(w->primary_key);
    w->primary_key++;
    w->key.size = sizeof(u_int32_t);
    */

    w->data.data = recordp;
    w->data.size = sizeof(DbRecord);
    blocking_callback(db, &(w->data), &(w->data), &(w->key));
    
    if((ret=db->put(db, NULL, &(w->key), &(w->data), 0)) != 0){
        printf("DB Error!");
        dbenv->err(dbenv, ret,
              "DB->put: %s", (char*)w->key.data);
        //    "DB->put: %lu", (u_int32_t)(w->primary_key));
        return(1);
    }
    
    return(0);
}

static int build_record(sqlite3_stmt *ppStmt, DbRecord *recordp, struct sqlite_workspace* w){
    int i, cols = sqlite3_column_count(ppStmt); 
    for(i = 0, w->f = fieldlist; i < cols && w->f->name != NULL; (w->f)++, i++){
        //switch(sqlite3_column_type(ppStmt, i)){
        switch(w->f->type){
            case UNSIGNED_LONG:
                *(int*)((char*)recordp+(w->f->offset)) = sqlite3_column_int(ppStmt, i);
                break;
            case STRING:
                memcpy((char*)recordp+(w->f->offset), sqlite3_column_text(ppStmt, i), sqlite3_column_bytes(ppStmt, i));
                *((char*)recordp+(w->f->offset)+sqlite3_column_bytes(ppStmt, i)) = '\0';
                break;
            case DATE:
                strptime((char*)sqlite3_column_text(ppStmt, i), "%Y%m%d", 
                        (struct tm*)((char*)recordp+(w->f->offset)));
                break;
            case DOUBLE:
                *(double*)((char*)recordp+(w->f->offset)) = sqlite3_column_double(ppStmt, i);
                break;
            default:
                abort();
        }
    }
    if(i < cols){
        printf("Description file has fewer slots than sqlite query columns.");
        abort();
    }
    if(w->f->name != NULL){
        printf("Description file has more slots that sqlite query columns.");
        abort();
    }

    return(0);
}
        
        

/*
 * alloc_set_insert
 * Callback function for sqlite3. Allocate, set, and insert database records.
*/
// static int alloc_set_insert(void *vw, int argc, char **argv, char **colname){
//     int i, ret;
//     size_t len, put_len;
//     //size_t l;
//     struct sqlite_workspace* w = (struct sqlite_workspace*)vw;
//     u_int32_t *put_line=0;
// 
//     colname=colname;
//     ret = ret;
// 
//     ++(w->primary_key);
//     len = put_len = 0;
// 
//     for(i=0; i < argc; i++){
//         if(argv[i])
//             len+=strlen(argv[i]);
//         len++;
//     }
// 
//     put_len = (argc + 2) * sizeof(u_int32_t) + len * sizeof(char);
// 
//     if((put_line = (u_int32_t *)malloc(put_len)) == NULL) {
//         printf("realloc error!");
// 		dbenv->err(dbenv, errno,
// 		    "unable to allocate %lu bytes for record",
// 		    (u_long)put_len);
// 		return (1);
// 	}
// 
//     //printf("put_line_final: %lu\n", (ulong)(put_line+len));
//     
//     if(input_set_offset(put_line, argv, len, (u_int32_t)argc)){
//         printf("set error!\n");
//     }
// 
// //    for(l=0; l < len; l++)
// //        printf("%c\n", *((char*)put_line + (argc+2)*sizeof(u_int32_t)+l));
// 
//     w->key.data = &(w->primary_key);
//     w->key.size = sizeof(u_int32_t);
// 
//     w->data.data = put_line;
//     w->data.size = put_len;
// 
//     if((ret=db->put(db, NULL, &(w->key), &(w->data), 0)) != 0){
//         printf("DB Error!");
//         dbenv->err(dbenv, ret,
//             "DB->put: %lu", (u_long)(w->primary_key));
//         return(1);
//     }
// 
//     free(put_line);
// 
//     return(0);
// }

/*
 * input_set_offset --
 *	Build an offset table and record combination.
 */
// static int input_set_offset(u_int32_t *put_line,
//     char **fields, size_t len, u_int32_t field_count)
// {
// 	u_int32_t *op;
// 	u_int32_t i;
// 	//char *p/*, *cp*/;
//     //char test[256];
//     //char blank = '\0';
// 	op = put_line;
// 
// 	/* The first field is the version number. */
// 	*op++ = version;
// 
// 	*op = 0;
//     //printf("1: ");
//     for(i=0; i<field_count; i++){
//         ++op;
//         *op = (u_int32_t)(op[-1] + strlen(fields[i]) + 1);
//         memcpy((char *)put_line + (field_count+2)*sizeof(u_int32_t) + op[-1],
//                //fields[i]?fields[i]:&blank, *op-op[-1]+(fields[i]?0:1));
//                 fields[i], *op-op[-1]);
// 
//     }
//     //printf("offset_final: %lu\n", (ulong)(put_line+*op));
//     //printf("\n");
//     //*((char *)put_line + (field_count+2)*sizeof(u_int32_t) + *op +1) = '\0';
//     //printf("2: %s\n", ((char*)(put_line) + (field_count+2)*sizeof(u_int32_t)));
// 
//     assert (len==*op);
//     //assert(len == *op);
//         
// 	return 0;
// }
