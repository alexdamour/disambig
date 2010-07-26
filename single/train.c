/*
 *  DO NOT EDIT: automatically built by sp_code.
 *
 * Specifications for indices of training counts.
 */

#include "train.h"

char* matchset_names[] = {
	"tset02_F",
	"tset05_F",
	NULL,
};

char *nonmatchset_names[] = {
	"xset01_F",
	"xset03_F",
	NULL,
};

char *idx_names[] = {
	"name",
	"other",
};

int(*idx_funcs[])(DB*, const DBT*, const DBT*, DBT*) = {
	name_idx_callback,
	other_idx_callback,
};

int name_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){
	sec = sec;
	data = data;
	int* fields = malloc(sizeof(int)*3);
	fields[0] = ((simprof*)key->data)->fname;
	fields[1] = ((simprof*)key->data)->midname;
	fields[2] = ((simprof*)key->data)->lname;
	result->data = fields;
	result->size = sizeof(int)*3;
	result->flags = result->flags | DB_DBT_APPMALLOC;
	return(0);
}

int other_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){
	sec = sec;
	data = data;
	int* fields = malloc(sizeof(int)*4);
	fields[0] = ((simprof*)key->data)->dist;
	fields[1] = ((simprof*)key->data)->asg;
	fields[2] = ((simprof*)key->data)->cl;
	fields[3] = ((simprof*)key->data)->coauths;
	result->data = fields;
	result->size = sizeof(int)*4;
	result->flags = result->flags | DB_DBT_APPMALLOC;
	return(0);
}

