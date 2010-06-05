/*
 *  DO NOT EDIT: automatically built by sp_code.
 *
 * Specifications for indices of training counts.
 */

#include "comp_engine.h"

extern int name_idx_callback(DB*, const DBT*, const DBT*, DBT*);
extern int loc_idx_callback(DB*, const DBT*, const DBT*, DBT*);
extern int other_idx_callback(DB*, const DBT*, const DBT*, DBT*);
extern int coauths_idx_callback(DB*, const DBT*, const DBT*, DBT*);

#define NUM_IDXS 4
char* idx_names[];

int(*idx_funcs[])(DB*, const DBT*, const DBT*, DBT*);

char* matchset_names[];char* nonmatchset_names[];