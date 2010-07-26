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
	char		Firstname[1][32];

#define	SQLITE_DB_INDX_LASTNAME	2
	char		Lastname[1][64];

#define	SQLITE_DB_INDX_STREET	3
	char		Street[1][64];

#define	SQLITE_DB_INDX_CITY	4
	char		City[1][32];

#define	SQLITE_DB_INDX_STATE	5
	char		State[1][3];

#define	SQLITE_DB_INDX_COUNTRY	6
	char		Country[1][3];

#define	SQLITE_DB_INDX_ZIPCODE	7
	char		Zipcode[1][6];

#define	SQLITE_DB_INDX_LAT	8
	double		 Lat[1];

#define	SQLITE_DB_INDX_LON	9
	double		 Lon[1];

#define	SQLITE_DB_INDX_INVSEQ	10
	u_int32_t		 InvSeq;

#define	SQLITE_DB_INDX_PATENT	11
	char		Patent[10];

#define	SQLITE_DB_INDX_APPDATESTR	12
	char		AppDateStr[8];

#define	SQLITE_DB_INDX_GYEAR	13
	u_int32_t		 GYear;

#define	SQLITE_DB_INDX_ASSIGNEE	14
	char		Assignee[1][64];

#define	SQLITE_DB_INDX_ASGNUM	15
	u_int32_t		 AsgNum[1];

#define	SQLITE_DB_INDX_CLASS	16
	char		Class[32];

#define	SQLITE_DB_INDX_COAUTHS	17
	char		Coauths[192];

#define	SQLITE_DB_INDX_CNT	18
	u_int32_t		 Cnt;

#define	SQLITE_DB_INDX_INVNUM_N	19
	char		Invnum_N[16];

#define	SQLITE_DB_INDX_INVNUM	20
	char		Invnum[16];

#define	SQLITE_DB_INDX_BLOCK1	21
	char		Block1[16];

#define	SQLITE_DB_INDX_BLOCK2	22
	char		Block2[3];

#define	SQLITE_DB_INDX_BLOCK3	23
	char		Block3[3];
} DbRecord;
#define SQLITE_DB_NUMFIELDS	24