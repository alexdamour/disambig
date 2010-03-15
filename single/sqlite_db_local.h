/*
 *  DO NOT EDIT: automatically built by sqlite_db_code.
 *
 * Record structure.
 */
typedef struct __DbRecord {
	/*
	 * Indexed fields
	 */
#define	SQLITE_DB_INDX_FIRSTNAME	1
	char		Firstname[32];

#define	SQLITE_DB_INDX_LASTNAME	2
	char		Lastname[64];

#define	SQLITE_DB_INDX_STREET	3
	char		Street[64];

#define	SQLITE_DB_INDX_CITY	4
	char		City[32];

#define	SQLITE_DB_INDX_STATE	5
	char		State[3];

#define	SQLITE_DB_INDX_COUNTRY	6
	char		Country[3];

#define	SQLITE_DB_INDX_ZIPCODE	7
	char		Zipcode[6];

#define	SQLITE_DB_INDX_LAT	8
	double		 Lat;

#define	SQLITE_DB_INDX_LON	9
	double		 Lon;

#define	SQLITE_DB_INDX_NATIONALITY	10
	char		Nationality[3];

#define	SQLITE_DB_INDX_INVSEQ	11
	u_int32_t		 InvSeq;

#define	SQLITE_DB_INDX_PATENT	12
	char		Patent[9];

#define	SQLITE_DB_INDX_KIND	13
	char		Kind[3];

#define	SQLITE_DB_INDX_CLAIMS	14
	u_int32_t		 Claims;

#define	SQLITE_DB_INDX_APPTYPE	15
	u_int32_t		 AppType;

#define	SQLITE_DB_INDX_APPNUM	16
	char		AppNum[16];

#define	SQLITE_DB_INDX_APPYEARSTR	17
	char		AppYearStr[5];

#define	SQLITE_DB_INDX_APPDATESTR	18
	struct tm		 AppDateStr;

#define	SQLITE_DB_INDX_LAW_ID	19
	char		Law_id[64];

#define	SQLITE_DB_INDX_ASSIGNEE	20
	char		Assignee[64];

#define	SQLITE_DB_INDX_ASGNUM	21
	u_int32_t		 AsgNum;

#define	SQLITE_DB_INDX_CLASS	22
	char		Class[32];

#define	SQLITE_DB_INDX_CLASSSUB	23
	char		ClassSub[64];

#define	SQLITE_DB_INDX_INVNUM_N	24
	char		Invnum_N[16];

#define	SQLITE_DB_INDX_COAUTHS	25
	char		Coauths[192];

#define	SQLITE_DB_INDX_NUM_COAUTHS	26
	u_int32_t		 Num_Coauths;
} DbRecord;
#define SQLITE_DB_NUMFIELDS	27