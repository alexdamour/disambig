#include "comp_engine.h"

/*
 * Globals
 */
#ifndef ADAPTIVE_PRIOR
#define ADAPTIVE_PRIOR 1
#endif

#ifndef TRIPLET_ON
#define TRIPLET_ON 1
#endif

#ifndef PR_M
#define PR_M 0.05
#endif

#ifndef PR_T
#define PR_T 0.5
#endif


#define DEBUG 1

DB_ENV	 *dbenv;			/* Database environment */
DB	 *db;				/* Primary database */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */
double Pr_M=0.;

//char* on_blocks[] = {"J.SMITH", "D.MORRIS", "J.LEE", NULL};

int main(int argc, char** argv){
    //This should be updated soon to use batch reads and writes
    //Should increase caching performance substantially.
    clock_t start, end, block_start, block_end;
    char ch;
    int big_block = 0;
    int ret, i, freeme, done, first_time;
    int one=1;
    int custom_block = 0;
    u_int32_t block_num = 0;
    u_int32_t trip = 0;
    size_t num_digs;
    double likelihood;
    db_recno_t dup_count, max_matches;
    FILE* fp;
    char* filename = "skipped_blocks.txt";
    fp = fopen(filename, "w");
    fclose(fp);

    void* multp;

    u_int32_t matches, num_comps, total_matches = 0;

    simprof sp;
    char* spkey_buf, **my_block;
    DB_BTREE_STAT *stat;

    DB  *db, *block_db, *stat_db, *sp_db, *rdb, *ldb, *first, *second, *lfreq, *match;
    DBC *cur, *tmp_cur, *cur_i, *cur_j, *r_cur, *tri_cur, *tag_cur, *ldb_cur;
    DBT data_i, key_i, pkey_i;
    DBT data_j, key_j, pkey_j;
    DBT data_sp, key_sp;
    DBT key_lik, data_lik;
    DBT lik_val, lik_cnt;

    memset(&data_i, 0, sizeof(data_i));
    memset(&key_i, 0, sizeof(data_i));
    memset(&pkey_i, 0, sizeof(data_i));

    memset(&data_j, 0, sizeof(data_j));
    memset(&key_j, 0, sizeof(data_j));
    memset(&pkey_j, 0, sizeof(data_j));

    memset(&data_sp, 0, sizeof(data_sp));
    memset(&key_sp, 0, sizeof(key_sp));

    DBT_CLEAR(key_lik);
    DBT_CLEAR(data_lik);

    DBT_CLEAR(lik_val);
    DBT_CLEAR(lik_cnt);
    
    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&db, "primary", DB_BTREE, 32*1024, 0, 0, compare_uint32);
    sqlite_db_secondary_open(db, &block_db, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);
    sqlite_db_primary_open(&rdb, "ratios", DB_BTREE, 4*1024, 0, 0, NULL);
    sqlite_db_primary_open(&stat_db, "stat_db", DB_BTREE, 32*1024, DB_CREATE, 0, NULL);
    if(DEBUG){
        ret = sqlite_db_primary_open(&lfreq, "lik_freq", DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
        printf("ret: %d\n", ret);
    }

    ret = db->cursor(db, NULL, &cur, 0);
    ret = block_db->cursor(block_db, NULL, &cur_i, 0);
    ret = rdb->cursor(rdb, NULL, &r_cur, 0);


//Ready the sp_db key buffer
    ret = cur->get(cur, &key_i, &data_i, DB_LAST);
    
    num_digs = (size_t)(log10((double)*(u_int32_t*)key_i.data))+1;
    printf("max_len: %u\n", num_digs);
    spkey_buf = malloc(sizeof(char)*num_digs*2+2);
    
    cur->close(cur);
//Ready to go!
    printf("ready to go!\n");

    memset(&data_i, 0, sizeof(data_i));
    memset(&key_i, 0, sizeof(key_i));
    memset(&pkey_i, 0, sizeof(pkey_i));

    while ((ch = getopt(argc, argv, "b:")) != EOF)
		switch (ch) {
		case 'b':
            key_i.data = optarg;
            key_i.size = strlen(optarg)+1;
            printf("%s, %u\n", key_i.data, key_i.size);
            custom_block = 1;
			break;
        }

    argc -= optind;
	argv += optind;

    if (*argv != NULL)
		return (EXIT_FAILURE);

    //my_block = on_blocks;
    printf("Timing started...\n");
    start = clock();
/*
    while(my_block != NULL){
        printf("block: %s\n", *my_block);
        key_i.data=*my_block;
        key_i.size=strlen(*my_block)+1;
        ++my_block;
      if(custom_block){
            if((ret = cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_SET)))
                return(1);
        }
            
        else
            if((ret = cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_FIRST)))
                return(1);
*/
    i=0;
    while(custom_block && DB_NOTFOUND !=
            cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, (i ? DB_NEXT_DUP : DB_SET))){
        //printf("Clearing...\n");
        if(strncmp(((DbRecord *)data_i.data)->Invnum_N, "\0", 1)){
            memset(((DbRecord*)data_i.data)->Invnum_N, '\0', 16);
            db->put(db, 0, &pkey_i, &data_i, 0);
        }
        i=1;
    }

    /* Initialize the comparison environment. */
    comp_env_init();

    while(DB_NOTFOUND !=
      cur_i->pget(cur_i, &key_i, &pkey_i, &data_i,
                  (custom_block ? DB_SET : DB_NEXT_NODUP))){//block loop 
        big_block = 0;
        block_start = clock();
        cur_i->dup(cur_i, &tri_cur, DB_POSITION);
        cur_i->dup(cur_i, &tag_cur, DB_POSITION);
        cur_i->count(cur_i, &dup_count, 0);
        if((int)dup_count > 100 || custom_block){
            printf("Big block: %s, %u records\n", (char*)key_i.data, (size_t)dup_count);
        //    big_block = 1;
    //        continue;
        }
        
        if(!custom_block && (int)dup_count > 2000){
            printf("Block too big: %s, %u\n", (char*)key_i.data, (size_t)dup_count);
            fp = fopen(filename, "a");
            fprintf(fp, "%s, %u\n", (char*)key_i.data, (size_t)dup_count);
            fclose(fp);
            continue;
        }

        Pr_M = MIN(PR_M, 100/((double)dup_count));

        if(!(++block_num % 10000) || (block_num < 1000 && !(block_num % 10))) {
            db->sync(db, 0);
            dbenv->memp_sync(dbenv, NULL);
            //Reinitialize cursor to clear cursor cache
            cur_i->dup(cur_i, &tmp_cur, DB_POSITION);
            tmp_cur->pget(tmp_cur, &key_i, &pkey_i, &data_i, DB_CURRENT);
            cur_i->close(cur_i);
            cur_i = tmp_cur;
            printf("%lu blocks processed with %lu needing triplet correction and %lu matches...\n", (ulong)block_num, (ulong)trip, (ulong)total_matches);
            
        }
        if(strncmp(((DbRecord *)data_i.data)->Invnum_N, "\0", 1)){
            //printf("Skipped! %s\n", ((DbRecord *)data_i.data)->Invnum_N);
            
            if(custom_block){
                memset(((DbRecord*)data_i.data)->Invnum_N, '\0', 16);
                //printf("Block already has a tag. Clear tags and try again.\n");
                ;
            }
            
            continue;
        }
        //else printf("Blocks previously processed: %lu\n", (ulong)block_num); 
        done = 0;
        first_time=1;

        while(!done){
            cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_SET);
            matches = 0;
            num_comps = 0;
            
            //printf("again!\n");
            //Compare by block and create likelihoods immediately
            //Only cache in the simprof DB for debugging
            
        /*
            else{
                printf("key_i: %s\n", (char*)key_i.data);
                printf("num_recs: %lu\n", (u_long)dup_count);
            }
        */
            ret = sqlite_db_primary_open(&sp_db, NULL, DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
            //ret = sqlite_db_primary_open(&sp_db, "simprof", DB_BTREE, 16*1024, DB_CREATE, 0, NULL);
            ret = sqlite_db_primary_open(&ldb, NULL, DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
            //ret = sqlite_db_primary_open(&ldb, "lik_db", DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
            ret = sqlite_db_secondary_open(ldb, &first, NULL, 4*1024, DB_DUPSORT, first_index, NULL);
            //ret = sqlite_db_secondary_open(ldb, &first, "first_idx", 16*1024, DB_DUPSORT, first_index, NULL);
            ret = sqlite_db_secondary_open(ldb, &second, NULL, 4*1024, DB_DUPSORT, second_index, NULL);
            //ret = sqlite_db_secondary_open(ldb, &second, "second_idx", 16*1024, DB_DUPSORT, second_index, NULL);
            if(ret)
                printf("DB open problem! %d\n", ret);
            do{
                
                cur_i->dup(cur_i, &cur_j, DB_POSITION);
                while(DB_NOTFOUND != 
                  cur_j->pget(cur_j, &key_j, &pkey_j, &data_j, DB_NEXT_DUP)){
                    sprintf(spkey_buf, "%lu-%lu", (ulong)*(u_int32_t*)pkey_i.data, (ulong)*(u_int32_t*)pkey_j.data);
                    //printf("spkey_buf: %s\n", spkey_buf);
                    key_sp.data = spkey_buf;
                    key_sp.size = strlen(spkey_buf);

                    if(!stop_comp((DbRecord*)data_i.data, (DbRecord*)data_j.data)){
                        ret = compare_records(&data_i, &data_j, &sp);
                        
                        data_sp.data = &sp;
                        data_sp.size = sizeof(sp);

                        //DEBUG
                        //ret = sp_db->put(sp_db, NULL, &key_sp, &data_sp, 0);
                        //END DEBUG
                        key_lik.data = data_sp.data;
                        key_lik.size = data_sp.size; 
                        ret = r_cur->get(r_cur, &key_lik, &data_lik, DB_SET);
                        if(ret){
                            printf("simprof missing: ");
                            simprof_dump((simprof*)key_lik.data);
                        }
                        likelihood = 1/(1+(1-Pr_M)/(Pr_M*(*(double*)(data_lik.data))));

                        /*DEBUGGING ONLY*/
                        if(DEBUG && first_time){
                            lik_val.data = &likelihood;
                            lik_val.size = sizeof(double);
                            if(DB_NOTFOUND == lfreq->get(lfreq, NULL, &lik_val, &lik_cnt, 0)){
                                lik_cnt.data = &(one);
                                lik_cnt.size = sizeof(int);
                            }
                            else{
                                *((int*)lik_cnt.data)+=1;
                                lik_cnt.size = sizeof(int);
                            }
                            lfreq->put(lfreq, NULL, &lik_val, &lik_cnt, 0);
                        }
                        /*END DEBUGGING ONLY*/
                        
                        if(likelihood > PR_T){
                            ++matches;
                        }
                            
                            /*if(strcmp(((DbRecord*)data_i.data)->Invnum,"04933465-2")==0 ||
                                strcmp(((DbRecord*)data_j.data)->Invnum,"04933465-2")==0){*/
                        
                            if(custom_block && likelihood > PR_T && strcmp(((DbRecord*)data_i.data)->Lastname,"JOHNSON")==0&&
                                                                    strcmp(((DbRecord*)data_j.data)->Lastname,"JOHNSTON")==0)
                                    //(strcmp(((DbRecord*)data_i.data)->Invnum, "D0571716-0")==0 ||
                                    //strcmp(((DbRecord*)data_j.data)->Invnum, "D0571716-0")==0))
                            {
                                DbRecord_dump((DbRecord*)data_i.data);
                                DbRecord_dump((DbRecord*)data_j.data);
                                
                                simprof_dump(&sp);
                                printf("likelihood: %g\n\n", likelihood);
                            }
                            
                            
                        //}
                    }
                    else{
                        likelihood = 0;
                    }

                    data_lik.data = &likelihood;
                    data_lik.size = sizeof(double);
         
                    ret = ldb->put(ldb, NULL, &key_sp, &data_lik, 0);
                       
                }
         //       printf("%lu matches in block.\n", (u_long)matches);
                cur_j->close(cur_j);
            } while(DB_NOTFOUND !=
                cur_i->pget(cur_i, &key_i, &pkey_i, &data_i, DB_NEXT_DUP));
            first_time=0;
            num_comps = (dup_count*(dup_count-1))/2;
            //printf("matches: %lu\n", (u_long)matches);
            if(ADAPTIVE_PRIOR && dup_count > 15 && num_comps != 0 && fabs(((double)(Pr_M*num_comps-matches))/((double)matches)) > 0.001){
                if(custom_block)
                    printf("Pr_M: %g, Emp: %g\n", Pr_M, (double)matches/(double)num_comps);
                Pr_M = (double)matches/(double)num_comps;//(double)matches/(double)dup_count;
            }
            else{
                if(custom_block)
                    printf("Pr_M: %g, Final Emp: %g\n", Pr_M, (double)matches/(double)num_comps);
                done = 1;
            }

            if(TRIPLET_ON && done && fabs((double)matches/(double)num_comps-1) > 0.00000001 && matches > 0){
                //printf("Mode: %d\n", ((double)matches/(double)num_comps) < 0.5 ? HI : LO);
                ret = triplet_correct(tri_cur, ldb, first, second,
                        ((double)matches/(double)num_comps) < 0.5 ? HI : LO);
                tri_cur->close(tri_cur);
                total_matches += matches;
                ++trip;
            }
            //sqlite_db_secondary_open(ldb, &match, "match_idx", 8*1024, DB_DUPSORT, match_index, NULL);
            
            /*
            second->stat(second, NULL, &stat, 0);
            printf("second_idx_nkeys: %lu\n", (u_long)(stat->bt_nkeys));
            printf("int_convert: %d\n", (int)(stat->bt_nkeys));
            if((int)stat->bt_nkeys == 1){
                printf("true!\n");
                ldb->cursor(ldb, NULL, &ldb_cur, 0);
                while(DB_NOTFOUND != ldb_cur->get(ldb_cur, &key_i, &data_i, DB_NEXT)){
                    printf("%s\n", (char*)key_i.data);
                    printf("%g\n", *(double*)data_i.data);
                }
                ldb_cur->close(ldb_cur);
            }
            free(stat);
            */
    //        match->stat(match, NULL, &stat, 0);
    //        printf("match_idx_nkeys: %lu\n", (u_long)(stat->bt_nkeys));
    //        free(stat);
            if(done){
                clump(tag_cur, ldb, first, second, match, db);
                analyze(stat_db, &key_i, ldb, db);
                tag_cur->close(tag_cur);
            }
            //match->close(match,0);
            first->close(first, 0);
            second->close(second, 0);
            ldb->close(ldb, 0);
            sp_db->close(sp_db, 0);

            //if(dbenv->dbremove(dbenv, NULL, "first_idx", NULL, 0))
            //    printf("complain!\n");
            //if(dbenv->dbremove(dbenv, NULL, "second_idx", NULL, 0))
            //    printf("complain!\n");
            //if(dbenv->dbremove(dbenv, NULL, "match_idx", NULL, 0))
            //    printf("complain!\n");
            //if(dbenv->dbremove(dbenv, NULL, "lik_db", NULL, 0))
            //    printf("complain!\n");
            //if(dbenv->dbremove(dbenv, NULL, "simprof", NULL, 0))
            //    printf("complain!\n");
        }
        block_end = clock();
        if(big_block)
            printf("Block time: %f\n", ((double) (block_end-block_start))/CLOCKS_PER_SEC);
        if(custom_block)
            break;
    }
    free(spkey_buf);

    /*Clean up the comparison environment. */
    comp_env_clean();

    end = clock();

    //ldb->stat_print(ldb, DB_STAT_ALL);
    printf("CPU TIME: %f\n", ((double) (end-start))/CLOCKS_PER_SEC);

    //sp_db->cursor(sp_db, NULL, &cur, 0);

    //cur->close(cur);
    cur_i->close(cur_i); 
    //cur_j->close(cur_j);
    r_cur->close(r_cur);
    //sp_db->close(sp_db, 0);
    rdb->close(rdb, 0);
    //first->close(first, 0);
    //second->close(second, 0);
    //ldb->close(ldb, 0);
    if(DEBUG)
        lfreq->close(lfreq,0);
    stat_db->close(stat_db, 0);
    block_db->close(block_db, 0);
    db->close(db,0);
    dbenv->close(dbenv,0);

    return(ret);
}
