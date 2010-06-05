#include "train.h"

/*
 * Globals
 */
typedef struct{
    DBT *data_i, *data_j;
    DBT *key;
    DBT *pkey;
    DB *tdb;
    DB *idx_db;
    char key_buf[32];
    simprof sp;
    size_t num_profs;
    int numrecs;
    size_t num_dropped;
} workspace;

DB_ENV *dbenv;
char *progname;


char *matchset_names[];

char *nonmatchset_names[];

static int load_patseq(void*, int, char**, char**);

int
main(int argc, char *argv[])
{
    int ret;
    int t_idx;
    int j;
    int have_match, have_non;
    char *home;
    clock_t start, end;
    sqlite3 *sql_db;
    char *testkey = "06836899-0";
    char sql_buf[64];
    char dbname_buf[64];
    DB *primary, *idx_db, *yes_db, *no_db, *sdb;
    DBC *cur;
    int numkeys= 0;
    int match_done = 0, non_done = 0;
    DB_BTREE_STAT *stat;

/* Workspace variables */
    workspace w;
    DBT data_i, data_j;
    DBT key, pkey;
    w.data_i = &data_i;
    w.data_j = &data_j;
    w.key = &key;
    w.key = &key;
    w.pkey = &pkey;
    w.pkey = &pkey;
    w.numrecs = 0;
    w.num_dropped = 0;

    dbenv=NULL;
    home = getenv("DB_HOME");
    if(sqlite_db_env_open(home))
        return(EXIT_FAILURE);

    printf("Timing started ...\n");
    start = clock();
    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, 0, 0, compare_uint32);
    sqlite_db_secondary_open(primary, &idx_db, "idx", 8*1024, 0, index_callback, NULL);
/*
    idx_db->cursor(idx_db, NULL, &cur, 0);

    memset(w.key, 0, sizeof(*(w.key)));
    memset(w.pkey, 0, sizeof(*(w.pkey)));
    memset(w.data_i, 0, sizeof(*(w.data_i)));

    //w.key->data=testkey;
    //w.key->size=11;

    if(DB_NOTFOUND==cur->pget(cur, w.key, w.pkey, w.data_i, DB_SET))
        printf("NOTFOUND!!! Sorry!\n");

    int i = 0;
    for(i=0; i < 100; ++i){
        cur->pget(cur, w.key, w.pkey, w.data_i, DB_NEXT);
        printf("key: %s\n", (char*)w.key->data);
    }

    cur->close(cur);
*/
    w.idx_db = idx_db;
    if(sqlite3_open("training_sets/tsetC.sqlite3", &sql_db))
        return(EXIT_FAILURE);

/*  Set these to 1 to keep from running that type of training set */
//    match_done = 1;
//    non_done = 1;

    for(t_idx=0; !(match_done && non_done); ++t_idx){
        /* Match (Positive) Training Sets */
        if(matchset_names[t_idx] == NULL)
            match_done = 1;
        if(!match_done){
            printf("Now training: %s\n", matchset_names[t_idx]);
            sprintf(dbname_buf, "%s", matchset_names[t_idx]);
            ret = sqlite_db_primary_open(&yes_db, dbname_buf, DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
            if(ret){
                printf("Could not open DB!\n");
                break;
            }

        //if((ret = sqlite_db_primary_open(&(w.tdb), matchset_names[t_idx], DB_QUEUE, 4*1024, DB_CREATE, 0, NULL)))
        //    return(EXIT_FAILURE);
            w.tdb = yes_db;
            sprintf(sql_buf, "select * from %s;", matchset_names[t_idx]);
            sqlite3_exec(sql_db, sql_buf, load_patseq, (void*)&w, NULL);
            printf("%s NUMRECS: %d\n", dbname_buf, w.numrecs);
            printf("%s NUMDROPPED: %u\n", dbname_buf, w.num_dropped);
            w.numrecs = 0;
            w.num_dropped = 0;
            //w.tdb->stat_print(w.tdb, DB_FAST_STAT);
            yes_db->stat(yes_db, NULL, &stat, 0);
            printf("numkeys: %lu\n", (u_long)(stat->bt_nkeys));
            free(stat);

            /* Index the similarity profiles according to their independent component sets */
            for(j=0; j<NUM_IDXS; ++j){
                sprintf(dbname_buf, "%s_%s", matchset_names[t_idx], idx_names[j]);
                sqlite_db_secondary_open(yes_db, &sdb, dbname_buf, 4*1024, DB_DUPSORT, idx_funcs[j], NULL);
                sdb->stat(sdb, NULL, &stat, 0);
                printf("\t%s_numkeys: %lu\n", idx_names[j], (u_long)(stat->bt_nkeys));
                free(stat);
                sdb->close(sdb,0);
            }
                
            yes_db->close(yes_db, 0);
        }

        /* Non-match (Negative) Training Sets */
        if(nonmatchset_names[t_idx] == NULL)
            non_done = 1;
        if(!non_done){ 
            printf("Now training: %s\n", nonmatchset_names[t_idx]);
            sprintf(dbname_buf, "%s", nonmatchset_names[t_idx]);
            sqlite_db_primary_open(&no_db, dbname_buf, DB_BTREE, 4*1024, DB_CREATE, 0, NULL);

            w.tdb = no_db;
            sprintf(sql_buf, "select * from %s;", nonmatchset_names[t_idx]);
            sqlite3_exec(sql_db, sql_buf, load_patseq, (void*)&w, NULL);
            printf("%s NUMRECS: %d\n", dbname_buf, w.numrecs);
            printf("%s NUMDROPPED: %u\n", dbname_buf, w.num_dropped);
            w.numrecs = 0;
            w.num_dropped=0;
            no_db->stat(no_db, NULL, &stat, 0);
            printf("numkeys: %lu\n", (u_long)stat->bt_nkeys);
            free(stat);

            /* Index the similarity profiles according to their independent component sets */
            for(j=0; j<NUM_IDXS; ++j){
                sprintf(dbname_buf, "%s_%s", nonmatchset_names[t_idx], idx_names[j]);
                sqlite_db_secondary_open(no_db, &sdb, dbname_buf, 4*1024, DB_DUPSORT, idx_funcs[j], NULL);
                sdb->stat(sdb, NULL, &stat, 0);
                printf("\t%s_numkeys: %lu\n", idx_names[j], (u_long)(stat->bt_nkeys));
                free(stat);
                sdb->close(sdb,0);
            }
         
            no_db->close(no_db, 0);
        }
    }
    
    end = clock();
    printf("Timing stopped.\n");
    printf("CPU Time: %f\n", ((double) (end - start))/CLOCKS_PER_SEC);

    sqlite3_close(sql_db);
    ret = idx_db->close(idx_db, 0);
    ret = primary->close(primary,0);
    ret = sqlite_db_env_close(); 
   
    return( ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE );
}

static int
load_patseq(void* w, int argc, char **argv, char **err){
    int ret;
    workspace* ws = (workspace*)w;
    ++ws->numrecs;
    //if(!(ws->numrecs % 1000000)) printf("\t%d records completed...\n", ws->numrecs);
    DBC *cur1, *cur2;

    (ws->idx_db)->cursor(ws->idx_db, NULL, &cur1, 0);
    (ws->idx_db)->cursor(ws->idx_db, NULL, &cur2, 0);

    memset(ws->key, 0, sizeof(*(ws->key)));
    memset(ws->pkey, 0, sizeof(*(ws->pkey)));
    memset(ws->data_i, 0, sizeof(*(ws->data_i)));
    sprintf(ws->key_buf, "%s", argv[0]);
    //printf("%s\n", ws->key_buf);

    ws->key->data=ws->key_buf;
    ws->key->size=strlen(ws->key_buf)+1;
    //printf("%u\n", ws->key->size);

    ret = cur1->pget(cur1, ws->key, ws->pkey, ws->data_i, DB_SET);
    if(ret==DB_NOTFOUND)
    {
//        printf("Missing 1: %s\n", (char*)(ws->key->data));
        ++(ws->num_dropped);
        return(0);
    }

    memset(ws->key, 0, sizeof(*(ws->key)));
    memset(ws->pkey, 0, sizeof(*(ws->pkey)));
    memset(ws->data_j, 0, sizeof(*(ws->data_j)));
    sprintf(ws->key_buf, "%s", argv[1]);
    //printf("%s\n", ws->key_buf);

    ws->key->data=ws->key_buf;
    ws->key->size=strlen(ws->key_buf)+1;

    ret = cur2->pget(cur2, ws->key, ws->pkey, ws->data_j, DB_SET);
    if(ret==DB_NOTFOUND)
    {
//        printf("Missing 2: %s\n", (char*)(ws->key->data));
        ++(ws->num_dropped);
        return(0);
    }
    compare_records(ws->data_i, ws->data_j, &(ws->sp));

    /*if(ws->sp.midname == 0){
        DbRecord_dump((DBT*)ws->data_i->data);
        DbRecord_dump((DBT*)ws->data_j->data);
    }*/

    cur1->close(cur1);
    cur2->close(cur2);

    memset(ws->key, 0, sizeof(*(ws->key)));
    memset(ws->data_i, 0, sizeof(*(ws->data_i)));

    ws->key->data=&(ws->sp);
    ws->key->size=sizeof(simprof);
    
    ws->num_profs=0;
    ws->data_i->data=&(ws->num_profs);
    ws->data_i->size=sizeof(size_t);

    if(DB_KEYEXIST == (ws->tdb)->put(ws->tdb, NULL, ws->key, ws->data_i, DB_NOOVERWRITE)){
//        printf("collision!\n");
        (ws->tdb)->get(ws->tdb, NULL, ws->key, ws->data_i, 0);
//        printf("\tnumprofs: %u\n", ws->num_profs);
//        printf("\tnum_in_db: %u\n", *(size_t*)ws->data_i->data);
        (*(size_t*)(ws->data_i->data))++;
        ret = (ws->tdb)->put(ws->tdb, NULL, ws->key, ws->data_i, 0);
        if(ret != 0) printf("something's wrong.\n");
    }

    return(0);
}
