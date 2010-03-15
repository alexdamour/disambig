#include "comp_spec.h"

extern int compare_records(DBT *, DBT *, simprof *);

//From comp_spec.c
int num_comps;
int (*comp_funcs[])(const void*, const void*, size_t);
int extract_idxs[];
size_t sp_offsets[];

char* has_tag(DbRecord*);
int apply_tag(DbRecord*, char*);

int stop_comp(DbRecord*, DbRecord*);

int triplet_correct(DBC*, DB*, DB*, DB*);
int clump(DBC*, DB*, DB*, DB*, DB*, DB*);
int fetch_lik(DB*, DBC**, DBC*, DBC*, DBT*, DBT*, DBT*, DBT*);
int first_index(DB*, const DBT*, const DBT*, DBT*);
int second_index(DB*, const DBT*, const DBT*, DBT*);
int match_index(DB*, const DBT*, const DBT*, DBT*);
int simprof_dump(simprof*);
