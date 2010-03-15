#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

/*
 * Globals
 */
DB_ENV	 *dbenv;			/* Database environment */
DB	 *db;				/* Primary database */
char	 *progname;			/* Program name */
datatype block_type;

int main(int argc, char** argv){
    int ret;
    int i;
 //   db_recno_t dups;
//    char start_year[5] = "1990\0";
    argc = argc;
    argv = argv;
    
//    DB_ENV* dbenv=NULL;
//    DB* db=NULL;
    //u_int32_t claims = 13;
    DB* sdb = NULL;
    DBC *cur_i=NULL, *cur_j = NULL;
    DBT data_i, key_i, pkey_i;
    DBT data_j, key_j, pkey_j;

    memset(&data_i, 0, sizeof(data_i));
    memset(&key_i, 0, sizeof(key_i));
    memset(&pkey_i, 0, sizeof(pkey_i));

    memset(&data_j, 0, sizeof(data_j));
    memset(&key_j, 0, sizeof(key_j));
    memset(&pkey_j, 0, sizeof(pkey_j));

//    key.data = &claims;
//    key.size = sizeof(u_int32_t);

    ret=sqlite_db_env_open(NULL, 1);
//    ret=sqlite_db_blockidx_open(&sdb);
    ret=sqlite_db_secondary_open(&sdb, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_ulong);

    ret = sdb->stat_print(sdb,0);

    ret = sdb->cursor(sdb, NULL, &cur_i, 0);

    //ret = cur1->get(cur1, &key, &data, DB_SET);
//    ret = cur2->get(cur2, &key, &data, DB_SET);
   // key_i.data=&claims;
    //key_i.size=sizeof(u_int32_t);
   
    for(i=0; i < 20; i++){ 
        ret = cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_NEXT_NODUP);
        printf("KEY: %s\n", (char*)key_i.data);
        DbRecord_dump(data_i.data);
        if(DB_NOTFOUND != cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_NEXT_DUP)){
            printf("DUP KEY: %s\n", (char*)key_i.data);
            DbRecord_dump(data_i.data);
        }
    }

    key_i.data="J.SMITH";
    key_i.size=8*sizeof(char);

    ret = cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_SET);
    if(!ret){
        i=0;
        DbRecord_dump(data_i.data);
        while(DB_NOTFOUND != cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_NEXT_DUP)){
            printf("KEY: %s\n", (char*)key_i.data);
            DbRecord_dump(data_i.data);
            ++i;
        }
        printf("Number of J.Smiths: %d\n", i);
    }
/*
    while(ret==0){
        printf("%s: %lu\n", (char*)key.data, *(ulong *)pkey.data);
        ret = cur3->pget(cur3, &key, &pkey, &data, DB_NEXT);
    }
*/

    switch(ret){
        case 0:
            break;
        case DB_NOTFOUND:
            printf("DB_NOTFOUND\n");
            break;
        case DB_REP_HANDLE_DEAD:
            printf("DB_REP_HANDLE_DEAD\n");
            break;
        case DB_SECONDARY_BAD:
            printf("DB_SECONDARY_BAD\n");
            break;
        default:
            printf("OTHER ERROR!\n");
    }
            
/*
    while(limit < 100 && ret != DB_NOTFOUND){
        printf("%lu: %s\n", *(ulong*)pkey.data, (char*)key.data);
        ret = cur1->pget(cur1, &key, &pkey, &data, DB_NEXT);
        ++limit;
    }
*/

    cur_i->close(cur_i);
    //cur_j->close(cur_j);
    //cur3->close(cur3);
    //join_cur->close(join_cur);

    sdb->close(sdb,0);
    db->close(db, 0);

    dbenv->close(dbenv, 0);

    return(ret);
}
