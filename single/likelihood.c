#include "comp_engine.h"

DB_ENV *dbenv;
char *progname;

#define ALMOST_ZERO -0.000000000001
#define ALMOST_ONE   1.000000000001

#ifndef LIK_WT
#define LIK_WT 4.0
#endif


//double Pr_M = 1./100;
u_int32_t records = 0;
u_int32_t triangles = 0;
u_int32_t corrected = 0;
u_int32_t blocks = 0;
/*
int triplet_correct(DB*, DB*, DB*);
int fetch_lik(DB*, DBC**, DBC*, DBC*, DBT*, DBT*, DBT*, DBT*);
int first_index(DB*, const DBT*, const DBT*, DBT*);
int second_index(DB*, const DBT*, const DBT*, DBT*);
int match_index(DB*, const DBT*, const DBT*, DBT*);
*/

/* Global DB_ENV */
DB_ENV	 *dbenv;

int write_csv(DB*);

int triplet_correct(DBC* orig, DB* lik, DB* first, DB* second, int mode){
    double w = LIK_WT;
    double ij, ik, jk;
    double big1_hat, big2_hat, small_hat;
    double delta = 0;
    int i = 0;
    u_int32_t records;

    DB* block_db, *prim_db;
    DBC *prim_cur_i;
    DBC *prim_cur_j, *prim_cur_k, *cur_i, *cur_j, *lik_cur;
    DBC *cur_ij, *cur_jk, *cur_ik;
    
    DBT dummy_key, dummy_data;
    DBT prim_key_i, prim_key_j, prim_key_k;
    DBT ij_key, jk_key, ik_key;
    DBT ij_data, jk_data, ik_data;
    DBT *small, *big1, *big2;

    //printf("triplets!\n");
    
    DBT_CLEAR(dummy_key);
    DBT_CLEAR(dummy_data);

    DBT_CLEAR(prim_key_i);
    DBT_CLEAR(prim_key_j);
    DBT_CLEAR(prim_key_k);

    DBT_CLEAR(ij_key);
    DBT_CLEAR(jk_key);
    DBT_CLEAR(ik_key);
    DBT_CLEAR(ij_data);
    DBT_CLEAR(jk_data);
    DBT_CLEAR(ik_data);

    ij_data.data = &ij;
    ij_data.ulen = sizeof(double);
    ij_data.flags = DB_DBT_USERMEM;
    ik_data.data = &ik;
    ik_data.ulen = sizeof(double);
    ik_data.flags = DB_DBT_USERMEM;
    jk_data.data = &jk;
    jk_data.ulen = sizeof(double);
    jk_data.flags = DB_DBT_USERMEM; 

    //sqlite_db_primary_open(&prim_db, "primary", DB_BTREE, 32*1024, 0, 0, compare_ulong);
    //sqlite_db_secondary_open(prim_db, &block_db, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_ulong);
    //block_db->cursor(block_db, NULL, &prim_cur_i, 0);
    first->cursor(first, NULL, &cur_i, 0);
    second->cursor(second, NULL, &cur_j, 0);
    lik->cursor(lik, NULL, &lik_cur, 0);
    
    //dummy_key.data = "J.SMITH";
    //dummy_key.size = 8;

    //orig->dup(orig, &prim_cur_i, DB_POSITION);
    //printf("key: %s\n", (char*)dummy_key.data);
    //printf("pkey: %lu\n", (u_long)*(u_int32_t*)prim_key_i.data);
    //prim_cur_i->pget(prim_cur_i, &dummy_key, &prim_key_i, &dummy_data, DB_FIRST);

    //corrected = 0;
    //while(DB_NOTFOUND != 
    //  prim_cur_i->pget(prim_cur_i, &dummy_key, &prim_key_i, &dummy_data, DB_NEXT_NODUP)){
        do{
            records = 0;
            orig->dup(orig, &prim_cur_i, DB_POSITION);
            prim_cur_i->pget(prim_cur_i, &dummy_key, &prim_key_i, &dummy_data, DB_CURRENT);
            delta = 0;
            do{
                if(!(++records % 100)) printf("%lu records of block processed...\n", (u_long)records);
                prim_cur_i->dup(prim_cur_i, &prim_cur_j, DB_POSITION);
                while(DB_NOTFOUND !=
                  prim_cur_j->pget(prim_cur_j, &dummy_key, &prim_key_j, &dummy_data, DB_NEXT_DUP)){
                    fetch_lik(lik, &cur_ij, cur_i, cur_j, &prim_key_i, &prim_key_j, &ij_key, &ij_data);
                    if((mode==HI) ? *(double*)ij_data.data < PR_T : *(double*)ij_data.data > PR_T){
                        cur_ij->close(cur_ij);
                        continue;
                    }
                    
                    prim_cur_j->dup(prim_cur_j, &prim_cur_k, DB_POSITION);
                    while(DB_NOTFOUND !=
                      prim_cur_k->pget(prim_cur_k, &dummy_key, &prim_key_k, &dummy_data, DB_NEXT_DUP)){
                        fetch_lik(lik, &cur_ik, cur_i, cur_j, &prim_key_i, &prim_key_k, &ik_key, &ik_data);
                        //printf("ij_var: %g\n", ij);
                        fetch_lik(lik, &cur_jk, cur_i, cur_j, &prim_key_j, &prim_key_k, &jk_key, &jk_data);
                        //printf("ij: %g, ik: %g, jk: %g\n", *(double*)ij_data.data, *(double*)ik_data.data, *(double*)jk_data.data);

                        if(*(double*)ij_data.data + *(double*)jk_data.data - *(double*)ik_data.data - 1 > 0 ||
                           *(double*)jk_data.data + *(double*)ik_data.data - *(double*)ij_data.data - 1 > 0 ||
                           *(double*)ik_data.data + *(double*)ij_data.data - *(double*)jk_data.data - 1 > 0){
                            if(!(*(double*)ij_data.data >= 0 && *(double*)ik_data.data >= 0 && *(double*)jk_data.data >= 0)){
                                printf("ij: %g, ik: %g, jk: %g\n", *(double*)ij_data.data, *(double*)ik_data.data, *(double*)jk_data.data);
                            }
                            assert(*(double*)ij_data.data >= 0 && *(double*)ik_data.data >= 0 && *(double*)jk_data.data >= 0);
                            //if(!(triangles % 1000))
                            if(*(double*)ij_data.data > 1.00001){
                                printf("%s\n", (char*)ij_key.data);
                                printf("%u\n", ij_key.size);
                                printf("%g\n", *(double*)ij_data.data);
                            }

                            
                            ++corrected;

                            if((*(double*)ij_data.data) < (MIN(*(double*)jk_data.data, *(double*)ik_data.data))){
                             //   printf("ij smallest!\n");
                                small = &ij_data;
                            }
                            else if((*(double*)jk_data.data) < (MIN(*(double*)ij_data.data, *(double*)ik_data.data))){
                              //  printf("jk smallest!\n");
                                small = &jk_data;
                            }
                            else{
                                small = &ik_data;
                               // printf("ik smallest!\n");
                            }
                            big1 = (&jk_data==small) ? &ij_data : &jk_data;
                            big2 = (&ik_data==small) ? ((&jk_data==big1) ? &ij_data : &jk_data) : &ik_data;
                            assert(big1 != big2 && small != big1 && small != big2);
                            assert(*(double*)big1->data >= *(double*)small->data && *(double*)big2->data >= *(double*)small->data);
                            //printf("big1: %g\nbig2: %g\nsmall: %g\n", *(double*)big1->data, *(double*)big2->data, *(double*)small->data);
                                
                            big1_hat = ((1+w)*(*(double*)big1->data) - *(double*)big2->data + *(double*)small->data + 1)/(2+w);
                            big2_hat = ((1+w)*(*(double*)big2->data) - *(double*)big1->data + *(double*)small->data + 1)/(2+w);
                            small_hat = (w*(*(double*)big1->data) + w*(*(double*)big2->data) + 2*(*(double*)small->data) - w)/(2+w);
                            delta += fabs(big1_hat - *(double*)big1->data) + fabs(big2_hat-*(double*)big2->data) + fabs(small_hat-*(double*)small->data);
                            //printf("delta: %g\n", delta);
                            *(double*)big1->data = big1_hat;
                            *(double*)big2->data = big2_hat;
                            *(double*)small->data = small_hat;
                            //printf("ij: %g\njk: %g\nik: %g\n", *(double*)ij_data.data, *(double*)jk_data.data, *(double*)ik_data.data);
                            assert(*(double*)ij_data.data >= 0 && *(double*)ik_data.data >= 0 && *(double*)jk_data.data >= 0);
                            
                            lik->put(lik, NULL, &ij_key, &ij_data, 0);
                            lik->put(lik, NULL, &jk_key, &jk_data, 0);
                            lik->put(lik, NULL, &ik_key, &ik_data, 0);
                        }
                        cur_jk->close(cur_jk);
                        cur_ik->close(cur_ik);
                        ++triangles;
                    }
                    cur_ij->close(cur_ij);
                    prim_cur_k->close(prim_cur_k);
                }
                prim_cur_j->close(prim_cur_j);
            } while(DB_NOTFOUND !=
                prim_cur_i->pget(prim_cur_i, &dummy_key, &prim_key_i, &dummy_data, DB_NEXT_DUP));
            prim_cur_i->close(prim_cur_i);
            //printf("delta: %g\n", delta);
            ++i;
        } while(delta > records * 0.1);
        //if(!(++blocks % 1000)){
        //    printf("\t%lu blocks processed...\n", (u_long)blocks);
        //    printf("\t%lu triangles checked...\n", (u_long)triangles);
        //    printf("\t%lu triangles corrected so far...\n", (u_long)corrected);
        //}
    //}

    //orig->close(orig);
    lik_cur->close(lik_cur);
    //prim_cur_k->close(prim_cur_k);
    //prim_cur_j->close(prim_cur_j);
    //prim_cur_i->close(prim_cur_i);
    cur_i->close(cur_i);
    cur_j->close(cur_j);
    //block_db->close(block_db, 0);
    //prim_db->close(prim_db, 0);

    //printf("%lu triangles corrected...\n", (u_long)corrected);
    //lik->sync(lik, 0);
    return(0);
}

int fetch_lik(DB* lik, DBC** join_cur, DBC* first, DBC* second, DBT* k1, DBT* k2, DBT* res_k, DBT* res_d){
    DBT data;
    //DBT_CLEAR(key);
    DBT_CLEAR(data);
    DBC* carray[3] = {first, second, NULL};
    first->get(first, k1, &data, DB_SET);
    second->get(second, k2, &data, DB_SET);
    lik->join(lik, carray, join_cur, 0);
    if(DB_NOTFOUND == (*join_cur)->get(*join_cur, res_k, res_d, 0))
    {
        (*join_cur)->close(*join_cur);
        first -> get(first, k2, &data, DB_SET);
        second -> get(second, k1, &data, DB_SET);
        lik->join(lik, carray, join_cur, 0);
        if(DB_NOTFOUND == (*join_cur)->get(*join_cur, res_k, res_d, 0))
            return 1;
    }
    assert(*(double*)res_d->data <= 1);
    //memcpy(res_k,&key,sizeof(key));
    //memcpy(res_d,&data,sizeof(data));
    if(!strncmp("77641-207430", (char*)res_k->data, 12)){
        printf("found it!\n");
        printf("res_key: %s\n", (char*)res_k->data);
        printf("res_data: %g\n", *(double*)res_d->data);
    }
    return(0);
}


int clump(DBC* orig, DB* ldb, DB* first, DB* second, DB* match, DB* prim){
    int i, write_cycle, changed, m=1, ret=0, no_matches;
    double val;
    int(*key_func)(DB*, const DBT*, const DBT*, DBT*);
    DBC* prim_cur_i, prim_cur_j, *first_cur, *second_cur, *match_cur;
    DBC* fs[2];
    //DBC* carray[3];
    DBT match_key;
    DBT ldb_key, ldb_dat;
    DBT dummy_dat;
    DBT key_i, pkey_i, data_i;
    DBT key_j, pkey_j, data_j;
    db_recno_t m_count;
    void* old;

    char invnum_buf[16];
    char *tagp;

    DBT_CLEAR(key_i);
    DBT_CLEAR(pkey_i);
    DBT_CLEAR(data_i);

    DBT_CLEAR(key_j);
    DBT_CLEAR(pkey_j);
    DBT_CLEAR(data_j);

    DBT_CLEAR(match_key);
    DBT_CLEAR(dummy_dat);

    DBT_CLEAR(ldb_key);
    DBT_CLEAR(ldb_dat);

    match_key.data = &m;
    match_key.size = sizeof(int);

    ret = first->cursor(first, NULL, &first_cur, 0);
    ret = second->cursor(second, NULL, &second_cur, 0);
    if(ret)
        printf("Cursor creation problem! %d\n", ret);
    fs[0] = first_cur;
    fs[1] = second_cur;
   /* 
    match->cursor(match, NULL, &match_cur, 0);
    no_matches = match_cur->get(match_cur, &match_key, &dummy_dat, DB_SET);
    printf("likelihood!: %g\n", *(double*)dummy_dat.data);
    match_cur->count(match_cur, &m_count, 0);
    printf("matches: %u\n", (size_t)m_count);
    */
    
    //return(0);

    changed=1;
    while(changed){
        //Repeat until none of the tags change.
        //printf("again!\n");
        changed=0;
        orig->dup(orig, &prim_cur_i, DB_POSITION);
        prim_cur_i->pget(prim_cur_i, &key_i, &pkey_i, &data_i, DB_CURRENT); //primary get.
        do {
            //Check for a tag
            tagp = has_tag((DbRecord*)data_i.data);
            if(tagp==NULL){
                apply_tag((DbRecord*)data_i.data, NULL);
                tagp = has_tag((DbRecord*)data_i.data);
                if(tagp == NULL){
                    printf("SERIOUS PROBLEM in tag application. Aborting.\n ");
                    exit(1);
                }
                //prim->put(prim, NULL, &pkey_i, &data_i, 0);
            }
            //memcpy(invnum_buf, tagp, 16);
            //printf("invnum_buf: %s\n", invnum_buf);
            //key_i.data = invnum_buf;
            //key_i.size = strlen(invnum_buf);
            for(write_cycle=0; write_cycle<2; ++write_cycle){
                //In the first pass, find the minimum tag that this record is associated with
                //In the second pass, write that tag to all records.
                if(write_cycle)
                    prim->put(prim, NULL, &pkey_i, &data_i, 0);

                for(i=0; i<2; ++i){
                //For each pass here, look for the record being the first in the comparison
                //then the second in the comparison
                    if(DB_NOTFOUND == (ret = fs[i]->pget(fs[i], &pkey_i, &key_i, &dummy_dat, DB_SET))){
                        //printf("join failed!\n");
                        continue;
                    }

                    do{
                        //printf("Keys: %s, ", (char*)key_i.data);
                        //printf("Sim: %f, ", *(double*)dummy_dat.data);
                        if(*(double*)dummy_dat.data < PR_T){
                        //    printf("\n");
                            continue;
                        }
                        key_func = i ? first_index : second_index;
                        key_func(first /*dummy*/, &key_i, &dummy_dat /*dummy*/, &pkey_j);
                        old = pkey_j.data;
                        //pkey_j.flags = DB_DBT_USERMEM;
                        //printf("ldb_key: %s\n", (char*)key_i.data);
                        //printf("pkey_j: %lu\n", *(u_long*)pkey_j.data);
                        prim->get(prim, NULL, &pkey_j, &data_j, 0);

                        if(!write_cycle){
                            if(tagcmp((DbRecord*)data_i.data, (DbRecord*)data_j.data) > 0){
                                apply_tag((DbRecord*)data_i.data,
                                    has_tag((DbRecord*)data_j.data));
                                //printf("\tNew Min: %s\n", has_tag((DbRecord*)data_i.data));
                                changed=1;
                            }
                            free(old);
                            continue;
                        }

                        //printf("Old Invnum_N: %s, ", ((DbRecord*)data_j.data)->Invnum_N);
                        if(tagcmp((DbRecord*)data_i.data, (DbRecord*)data_j.data)!=0){
                            apply_tag(((DbRecord*)data_j.data), has_tag((DbRecord*)data_i.data));
                            prim->put(prim, NULL, &pkey_j, &data_j, 0);
                            prim->get(prim, NULL, &pkey_j, &data_j, 0);
                            changed=1;
                        }
                        //printf("New Invnum_N: %s\n", ((DbRecord*)data_j.data)->Invnum_N);
                        //free(pkey_j.data);
                        free(old);
                    } while(DB_NOTFOUND != fs[i]->pget(fs[i], &pkey_i, &key_i, &dummy_dat, DB_NEXT_DUP));
                }//First, second idx
            }//Write cycle
        } while(DB_NOTFOUND !=
          prim_cur_i->pget(prim_cur_i, &key_i, &pkey_i, &data_i, DB_NEXT_DUP));
    }//changed
    first_cur->close(first_cur);
    second_cur->close(second_cur);
    return(0);
}

int analyze(DB* stat_db, DBT* block_dbt, DB* ldb, DB* primary){
	DBT lkey, lval;
	DBT first_pkey, second_pkey;
	DBT rec1, rec2;
    DBT sd_dbt, sk_dbt;
	DBC* lcur, *pcur1, *pcur2;
    stat_data sd;
    void *old1, *old2;
	double match_n, nonmatch_n;
	u_int32_t match_d, nonmatch_d, all;
	
	match_n = nonmatch_n = match_d = nonmatch_d = 0;
	
	DBT_CLEAR(lkey);
	DBT_CLEAR(lval);
	DBT_CLEAR(first_pkey);
	DBT_CLEAR(second_pkey);
    DBT_CLEAR(rec1);
    DBT_CLEAR(rec2);
    DBT_CLEAR(sd_dbt);
    DBT_CLEAR(sk_dbt);
	
	ldb->cursor(ldb, NULL, &lcur, 0);
    primary->cursor(primary, NULL, &pcur1, 0);
    primary->cursor(primary, NULL, &pcur2, 0);
	
	while(DB_NOTFOUND != lcur->get(lcur, &lkey, &lval, DB_NEXT)){
		first_index(NULL, &lkey, &lval, &first_pkey);
		second_index(NULL, &lkey, &lval, &second_pkey);

        old1 = first_pkey.data;
        old2 = second_pkey.data;

		pcur1->get(pcur1, &first_pkey, &rec1, DB_SET);
		pcur2->get(pcur2, &second_pkey, &rec2, DB_SET);

        if(tagcmp((DbRecord*)rec1.data, (DbRecord*)rec2.data)==0){
			match_n += *(double*)lval.data;
			++match_d;
		}
		else {
			nonmatch_n += *(double*)lval.data;
			++nonmatch_d;
		}

        free(old1);
        free(old2);
	}

    lcur->close(lcur);
    pcur1->close(pcur1);
    pcur2->close(pcur2);

    //open likelihood database
    sd.density = (match_n+nonmatch_n)/(match_d+nonmatch_d);
    sd.precision = match_n/match_d;
    sd.recall = nonmatch_n/nonmatch_d;
    sd.match_n = match_n;
    sd.nonmatch_n = nonmatch_n;
    sd.match_d = match_d;
    sd.nonmatch_d = nonmatch_d;
    sd.Pr_M = PR_M;
    sd.Pr_T = PR_T;

    sd_dbt.data = &sd;
    sd_dbt.size = sizeof(stat_data);

    stat_db->put(stat_db, NULL, block_dbt, &sd_dbt, 0);

    //create block tag, PR_M, PR_T stuct
    //create stat struct
    //write key-data pair to bdb
    //later on, index the database by block, Pr_M, and Pr_T
	
	printf("density: %g, precision: %g of %lu, recall: %g of %lu\n", (match_n+nonmatch_n)/(match_d+nonmatch_d), match_n/match_d, match_d, 1-nonmatch_n/nonmatch_d, nonmatch_d);
	return(0);
}
	

int first_index(DB* sec, const DBT* key, const DBT* data, DBT* result){
    u_int32_t *prim_key = (u_int32_t*) malloc(sizeof(u_int32_t));
    char *delim="-";
    char *old, *key_copy = (char*)malloc(key->size);
    old = key_copy;
    char *target;
    memcpy(key_copy, key->data, key->size);

    target=strsep(&key_copy, delim);

    *prim_key = (u_int32_t)strtoul(target,NULL,10);
    //new_copy = malloc(strlen(target)+1);
    //memcpy(new_copy, target, strlen(target)+1);
    free(old);
    
    result->data = prim_key;//new_copy; 
    result->size = sizeof(u_int32_t);//strlen(new_copy)+1;
    result->flags = DB_DBT_APPMALLOC;

    return(0);
}

int second_index(DB* sec, const DBT* key, const DBT* data, DBT* result){
    u_int32_t *prim_key = (u_int32_t*) malloc(sizeof(u_int32_t));
    char *delim="-";
    char *old, *key_copy = (char*)malloc(key->size+1);
    old = key_copy;
    memcpy(key_copy, key->data, key->size);
    key_copy[key->size]='\0';

    strsep(&key_copy, delim);
    //if(!(++records%1000000)) printf("\t%lu records processed...\n", (u_long)records);
   
    //printf("key_copy: %s\n", key_copy);
    *prim_key = (u_int32_t)strtoul(key_copy,NULL,10); 
    //printf("strtoul: %lu\n", *(u_long*)prim_key);
    //new_copy = malloc(strlen(key_copy)+1);
    //memcpy(new_copy, key_copy, strlen(key_copy)+1);
    free(old);
    
    result->data = prim_key;//new_copy;
    result->size = sizeof(u_int32_t);//strlen(new_copy)+1;
    result->flags = DB_DBT_APPMALLOC;

    return(0);
}

int match_index(DB* sec, const DBT* key, const DBT* data, DBT* result){
    int *match = (int*)malloc(sizeof(int));

    *match = ((*(double*)(data->data)) > PR_T);

    result->data = match;
    result->size = sizeof(int);
    result->flags = DB_DBT_APPMALLOC;

    return(0);
}
/*
int write_csv(DB* matches){
    FILE* csv;
    int setting = 1;
    char* delim="-";
    char *key_buf, *key_old;
    DB *primary;
    DBC *match_cur, *p_cur1, *p_cur2;
    DBT key, pkey, data;
    DBT pdata1, pdata2;
    u_int32_t new_key1, new_key2;
    csv = fopen("matches.csv", "w");

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memset(&pkey, 0, sizeof(pkey));
    memset(&pdata1, 0, sizeof(pdata1));
    memset(&pdata2, 0, sizeof(pdata2));

    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, 0, 0, compare_ulong);

    matches->cursor(matches, NULL, &match_cur, 0);
    key.data = &setting;
    key.size = sizeof(int);

    primary->cursor(primary, NULL, &p_cur1, 0);
    primary->cursor(primary, NULL, &p_cur2, 0);

    if(DB_NOTFOUND == match_cur->pget(match_cur, &key, &pkey, &data, DB_SET))
        return(1);

    do{
        //printf("%s\n", (char*)pkey.data);
        key_buf = (char*)malloc(pkey.size+1);
        memcpy(key_buf, (char*)pkey.data, pkey.size);
        key_buf[pkey.size] = '\0';
       // printf("%d\n", pkey.size);
        key_old = strsep(&key_buf, "-");
        //printf("%s\n", key_old);
        //printf("%s\n", key_buf);
        new_key1 = strtoul(key_old, NULL, 10);
        new_key2 = strtoul(key_buf, NULL, 10);

        key.data = &new_key1;
        key.size = sizeof(u_int32_t);

        p_cur1->get(p_cur1, &key, &pdata1, DB_SET);

        key.data = &new_key2;
        key.size = sizeof(u_int32_t);

        p_cur2->get(p_cur2, &key, &pdata2, DB_SET);

        fprintf(csv, "%lu,%s,%lu,%s,%g\n", ((DbRecord*)pdata1.data)->InvSeq, ((DbRecord*)pdata1.data)->Patent,
                                          ((DbRecord*)pdata2.data)->InvSeq, ((DbRecord*)pdata2.data)->Patent,
                                          *(double*)data.data);
        free(key_old);
    }while(DB_NOTFOUND != match_cur->pget(match_cur, &key, &pkey, &data, DB_NEXT_DUP));

    p_cur1->close(p_cur1);
    p_cur2->close(p_cur2);
    primary->close(primary, 0);

    return(0);
}*/
