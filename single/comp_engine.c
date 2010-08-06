/* Open primary and secondary DBs, create two cursors on the secondary DB.
 * Multi-fetch a block.
 * Iterate within each block to perform each comparison. These iterations will
 * later be turned into kernels.
 * Write the results to a profile array.
 * Output the profiles to a new DB whose key is a pair-ID string.
 */

#include "comp_engine.h"

int compare_records(DBT *rec1, DBT *rec2, simprof *sp){
    void *arg1, *arg2;
    int (*cfunc)(const void *, const void*, size_t);
    int i, freeme, is_array;
    size_t sz, array_len_1, array_len_2, array_len_max;

    //DbRecord_dump((DbRecord*)rec1->data);
    //DbRecord_dump((DbRecord*)rec2->data);

    for(i=0; i < NUM_COMPS; ++i){
        cfunc = comp_funcs[i];
        freeme = extract((DbRecord*)rec1->data, extract_idxs[i], &arg1, &sz, &is_array, &array_len_1, &array_len_max);
        freeme = extract((DbRecord*)rec2->data, extract_idxs[i], &arg2, &sz, &is_array, &array_len_2, &array_len_max);
        if(is_array)
            *(int*)((char*)sp+sp_offsets[i]) = listcmp(arg1, arg2, sz, missing_res[i],
                                                cfunc, array_len_1, array_len_2, array_len_max);
        else
            *(int*)((char*)sp+sp_offsets[i]) = cfunc(arg1, arg2, sz);
//        printf("\tfinal_res: %d\n", *(int*)((char*)sp+sp_offsets[i]));
        /*
        if(sp->fname < 4){
            DbRecord_dump((DbRecord*)rec1->data);
            DbRecord_dump((DbRecord*)rec2->data);
            simprof_dump(sp);
        }
        */
        if(freeme){
            free(arg1); 
            free(arg2);
        }
    }
//    simprof_dump(sp);
    
    return(0);
}

int listcmp(const void* arg1, const void* arg2, size_t size, int missing_val,
        int (*cfunc)(const void *, const void*, size_t), size_t array_len_1, size_t array_len_2, size_t array_len){
    int res=0, tmp;
    int modified = 0;
    int i,j;

    //printf("array_len_1: %d, array_len_2: %d\n", array_len_1, array_len_2);
    for(i=0; i<MAX((int)array_len_1,1); ++i){
        for(j=0; j<MAX((int)array_len_2,1); ++j){
            tmp = cfunc((void*)((char*)arg1+size*i), (void*)((char*)arg2+size*j), size);
            if(res==missing_val)
                res = tmp;
            else if(!modified || tmp!=missing_val)
                res = MAX(res, tmp);

            if(!modified)
                modified = 1;
        }
    }

    return res;
}

int simprof_dump(simprof* sp){
    int i;
    printf("(");
    for(i=0; i < NUM_COMPS; ++i)
        printf("%d,", *(int*)((char*)sp+sp_offsets[i]));
    printf(")\n");
    return 0;
}
