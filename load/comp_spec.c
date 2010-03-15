/*
 *  DO NOT EDIT: automatically built by sp_code.
 * Initialized arrays for loading custom comparison
 * and extractor functions.
 */

#include "comp_spec.h"

/* Comparison function array. */
int (*comp_funcs[])(const void*, const void*, size_t) = {
	jwcmp,
	midnamecmp,
	distcmp,
	disttypecmp,
	asgcmp,
	jwcmp,
	classcmp,
	coauthcmp,
};

/* Extractor index array. */
int extract_idxs[] = {
	SQLITE_DB_INDX_FIRSTNAME, 
	SQLITE_DB_INDX_FIRSTNAME, 
	LATLON, 
	SQLITE_DB_INDX_COUNTRY, 
	ASG_FIELDS, 
	SQLITE_DB_INDX_LAW_ID, 
	SQLITE_DB_INDX_CLASS, 
	SQLITE_DB_INDX_COAUTHS, 
};

/* Simprof offset array. */
size_t sp_offsets[] = {
	SP_OFFSET(fname), 
	SP_OFFSET(midname), 
	SP_OFFSET(dist), 
	SP_OFFSET(dt), 
	SP_OFFSET(asg), 
	SP_OFFSET(firm), 
	SP_OFFSET(cl), 
	SP_OFFSET(coauths), 
};

