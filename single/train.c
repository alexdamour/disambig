/*
 *  DO NOT EDIT: automatically built by sp_code.
 *
 * Specifications for indices of training counts.
 */

#include "train.h"

char* matchset_names[] = {
	"tset01_F",
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
	"fname",
	"loc",
	"other",
	"coauths",
};

int(*idx_funcs[])(DB*, const DBT*, const DBT*, DBT*) = {
	fname_idx_callback,
	loc_idx_callback,
	other_idx_callback,
	coauths_idx_callback,
};

int fname_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){
	sec = sec;
	data = data;
	int* fields = malloc(sizeof(int)*2);
	fields[0] = ((simprof*)key->data)->fname;
	fields[1] = ((simprof*)key->data)->midname;
	result->data = fields;
	result->size = sizeof(int)*2;
	result->flags = result->flags | DB_DBT_APPMALLOC;
	return(0);
}

int loc_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){
	sec = sec;
	data = data;
	int* fields = malloc(sizeof(int)*2);
	fields[0] = ((simprof*)key->data)->dist;
	fields[1] = ((simprof*)key->data)->dt;
	result->data = fields;
	result->size = sizeof(int)*2;
	result->flags = result->flags | DB_DBT_APPMALLOC;
	return(0);
}

int other_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){
	sec = sec;
	data = data;
	int* fields = malloc(sizeof(int)*3);
	fields[0] = ((simprof*)key->data)->asg;
	fields[1] = ((simprof*)key->data)->firm;
	fields[2] = ((simprof*)key->data)->cl;
	result->data = fields;
	result->size = sizeof(int)*3;
	result->flags = result->flags | DB_DBT_APPMALLOC;
	return(0);
}

int coauths_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){
	sec = sec;
	data = data;
	int* fields = malloc(sizeof(int)*1);
	fields[0] = ((simprof*)key->data)->coauths;
	result->data = fields;
	result->size = sizeof(int)*1;
	result->flags = result->flags | DB_DBT_APPMALLOC;
	return(0);
}

