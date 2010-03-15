/*
 *  DO NOT EDIT: automatically built by sp_code.
 *
 * Record structure.
 */

#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"
#define SP_OFFSET(field) ((size_t)(&(((simprof*)0)->field)))

/* Begin custom header from sp.desc file. */
#include <math.h>
#include "strcmp95.h"

#define LATLON SQLITE_DB_NUMFIELDS
#define ASG_FIELDS  SQLITE_DB_NUMFIELDS+1

typedef struct {
    double lat;
    double lon;
} latlon;

typedef struct {
    char* asgname;
    u_int32_t asgnum;
} asg_struct;





/* End custom header from sp.desc file. */

/* Define result space levels. */
typedef enum {JWSUB75,JWMISSING,JW75,JW85,JW95,JW100} jwres;
typedef enum {DIST100PLUS,DISTMISSING,DIST100,DIST75,DIST50,DIST10,DIST0} distres;
typedef enum {NO_STREET,HAVE_STREET} disttype;
typedef enum {CLASS0,CLASSMISS,CLASS25,CLASS50,CLASS75PLUS} classres;
typedef enum {C0,C1,C2,C3,C4,C5,C6,C7,C8,C9,C10} coauthres;
typedef enum {M0,MMISSING,M33,M67,M100} midnameres;

/* Define the simprof data structure.*/
typedef struct __simprof {
	jwres fname;
	midnameres midname;
	distres dist;
	disttype dt;
	jwres asg;
	jwres firm;
	classres cl;
	coauthres coauths;
} simprof;

/* Comparison specific globals. */
int (*comp_funcs[])(const void*, const void*, size_t);
int extract_idxs[];
size_t sp_offsets[];

/* Custom function prototypes. */
/* Extractor function. */
int extract(DbRecord*, const int, void**, size_t*);
/* Comparison functions. */
int jwcmp(const void*, const void*, size_t);
int midnamecmp(const void*, const void*, size_t);
int distcmp(const void*, const void*, size_t);
int disttypecmp(const void*, const void*, size_t);
int asgcmp(const void*, const void*, size_t);
int jwcmp(const void*, const void*, size_t);
int classcmp(const void*, const void*, size_t);
int coauthcmp(const void*, const void*, size_t);

#define NUM_COMPS	8
