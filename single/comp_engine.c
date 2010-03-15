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
    int i, freeme;
    size_t sz;

    for(i=0; i < NUM_COMPS; ++i){
        cfunc = comp_funcs[i];
        freeme = extract((DbRecord*)rec1->data, extract_idxs[i], &arg1, &sz);
        freeme = extract((DbRecord*)rec2->data, extract_idxs[i], &arg2, &sz);
        *(int*)((char*)sp+sp_offsets[i]) = cfunc(arg1, arg2, sz);
//        printf("\tfinal_res: %d\n", *(int*)((char*)sp+sp_offsets[i]));
        if(freeme){
            free(arg1); 
            free(arg2);
        }
    }
    //simprof_dump(sp);
    
    return(0);
}

int simprof_dump(simprof* sp){
    int i;
    printf("(");
    for(i=0; i < NUM_COMPS; ++i)
        printf("%d,", *(int*)((char*)sp+sp_offsets[i]));
    printf(")\n");
    return 0;
}
