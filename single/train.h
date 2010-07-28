/*
 *  DO NOT EDIT: automatically built by sp_code.
 *
 * Specifications for indices of training counts.
 */

#include "comp_engine.h"

extern int name_idx_callback(DB*, const DBT*, const DBT*, DBT*);
extern int other_idx_callback(DB*, const DBT*, const DBT*, DBT*);

#define NUM_IDXS 2
extern char* idx_names[];

extern int(*idx_funcs[])(DB*, const DBT*, const DBT*, DBT*);

extern char* matchset_names[];
extern char* nonmatchset_names[];