/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005,2008 Oracle.  All rights reserved.
 *
 * $Id: csv.h,v 1.20 2008/01/08 20:58:23 bostic Exp $
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include <time.h>

#ifdef _WIN32
#define	WIN32_LEAN_AND_MEAN	1

#include <direct.h>
#include <db.h>

extern int getopt(int, char * const *, const char *);
extern char *optarg;
extern int optind;
#else
#define	HAVE_WILDCARD_SUPPORT	1

#include <regex.h>
#include <unistd.h>
#endif

#include "db.h"

/*
 * MAP_VERSION
 * This code has hooks for versioning, but does not directly support it.
 * See the README file for details.
 */
#define	MAP_VERSION	1

/*
 * OFFSET_LEN
 *	The length of any item can be calculated from the two offset fields.
 * OFFSET_OOB
 *	An offset that's illegal, used to detect unavailable fields.
 */
#define	OFFSET_LEN(offset, indx)					\
	(((offset)[(indx) + 1] - (offset)[(indx)]))

#define	OFFSET_OOB	0

/*
 * Field comparison operators.
 */
typedef enum { EQ=1, NEQ, GT, GTEQ, LT, LTEQ, WC, NWC } OPERATOR;

/*
 * Supported data types.
 */
typedef enum { NOTSET=1, DOUBLE, STRING, UNSIGNED_LONG, DATE } datatype;

/*
 * C structure that describes the csv fields.
 */
typedef struct {
	char	 *name;				/* Field name */
	u_int32_t fieldno;			/* Field index */
	datatype  type;				/* Data type */
    size_t size;                /* Field size in bytes */

	int	  indx;				/* Indexed */
	DB	 *secondary;			/* Secondary index handle */

    int array;
    size_t array_len;

#define	FIELD_OFFSET(field)	((size_t)(&(((DbRecord *)0)->field)))
	size_t	  offset;			/* DbRecord field offset */
} DbField;

#define MAX(X,Y) ((X > Y) ? X : Y)
#define MIN(X,Y) ((X < Y) ? X : Y)
#define DBT_CLEAR(X) memset(&X, 0, sizeof(X))

#define PRIMARY_PAGE_KB 64 

/*
 * Globals
 */
extern DbField	  fieldlist[];			/* Field list */
extern DB_ENV	 *dbenv;			/* Database environment */
extern char	 *progname;			/* Program name */
extern int	  verbose;			/* Program verbosity */
#ifdef _WIN32
#undef strcasecmp
#define	strcasecmp _stricmp
#undef strncasecmp
#define	strncasecmp _strnicmp
#define	mkdir(d, perm) _mkdir(d)
#endif
