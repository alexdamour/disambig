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

#define ADAPTIVE_PRIOR 1
#define TRIPLET_ON 1
#define TRIPLET_ITERS 5
#define PR_M 0.33
#define PR_T 0.95
#define LIK_WT 4.0 

#define LATLON SQLITE_DB_NUMFIELDS
#define ASG_FIELDS  SQLITE_DB_NUMFIELDS+1

typedef struct {
    double lat;
    double lon;
    char* street;
} latlon;

typedef struct {
    char* asgname;
    u_int32_t asgnum;
} asg_struct;





/* End custom header from sp.desc file. */

/* Define result space levels. */
typedef enum {JWSUB33,JWMISSING,JW33,JW66,JW100,JW100MULT,JW100MULTFULL} jwres;
typedef enum {ASGSUB33,ASGMISSING,ASG33,ASG66,ASG100,ASG100MULT,ASGNUM,ASGNUMMED,ASGNUMSMALL} asgres;
typedef enum {DIST100PLUS,DISTMISSING,DIST100,DIST75,DIST50,DIST10NOST,DIST10ST,DIST0ST} distres;
typedef enum {CLASS0,CLASSMISS,CLASS25,CLASS50,CLASS75PLUS} classres;
typedef enum {C0,C1,C2,C3,C4,C5,C6,C7,C8,C9,C10} coauthres;
typedef enum {M0,MMISSING,M33,M67,M100} midnameres;

/* Define the simprof data structure.*/
typedef struct __simprof {
	jwres fname;
	midnameres midname;
	jwres lname;
	distres dist;
	asgres asg;
	coauthres cl;
	coauthres coauths;
} simprof;

/* Comparison specific globals. */
extern int (*comp_funcs[])(const void*, const void*, size_t);
extern int missing_res[];
extern int extract_idxs[];
extern size_t sp_offsets[];

/* Custom function prototypes. */
/* Comparion environment functions. */
int comp_env_init(void);int comp_env_clean(void);/* Extractor function. */
int extract(DbRecord*, const int, void**, size_t*, int*, size_t*, size_t*);
/* Comparison functions. */
int jwcmp(const void*, const void*, size_t);
int midnamecmp(const void*, const void*, size_t);
int jwcmp(const void*, const void*, size_t);
int distcmp(const void*, const void*, size_t);
int asgcmp(const void*, const void*, size_t);
int classcmp(const void*, const void*, size_t);
int coauthcmp(const void*, const void*, size_t);

#define NUM_COMPS	7
