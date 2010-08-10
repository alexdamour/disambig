#include "comp_spec.h"

#define HI 0
#define LO 1

typedef struct _stat_data {
    double density;
    double precision;
    double recall;
    double match_n;
    double nonmatch_n;
    int match_d;
    int nonmatch_d;
    double Pr_M;
    double Pr_T;
} stat_data;

extern int compare_records(DBT *, DBT *, simprof *);
int listcmp(const void *, const void *, size_t, int, int (*)(const void *, const void*, size_t), size_t, size_t, size_t);

//From comp_spec.c
int num_comps;
int (*comp_funcs[])(const void*, const void*, size_t);
int extract_idxs[];
size_t sp_offsets[];

char* has_tag(DbRecord*);
int apply_tag(DbRecord*, char*);
int tagcmp(DbRecord*, DbRecord*);

int stop_comp(DbRecord*, DbRecord*);

int triplet_correct(DBC*, DB*, DB*, DB*, int);
int clump(DBC*, DB*, DB*, DB*, DB*, DB*);
int analyze(DB*, DBT*, db_recno_t, DB*, DB*);
int fetch_lik(DB*, DBC**, DBC*, DBC*, DBT*, DBT*, DBT*, DBT*);
int first_index(DB*, const DBT*, const DBT*, DBT*);
int second_index(DB*, const DBT*, const DBT*, DBT*);
int match_index(DB*, const DBT*, const DBT*, DBT*);
int simprof_dump(simprof*);
