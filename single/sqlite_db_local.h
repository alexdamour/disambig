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
	char		Firstname[3][32];

#define	SQLITE_DB_INDX_LASTNAME	2
	char		Lastname[1][64];

#define	SQLITE_DB_INDX_STREET	3
	char		Street[10][64];

#define	SQLITE_DB_INDX_CITY	4
	char		City[10][32];

#define	SQLITE_DB_INDX_STATE	5
	char		State[10][3];

#define	SQLITE_DB_INDX_COUNTRY	6
	char		Country[6][10];

#define	SQLITE_DB_INDX_ZIPCODE	7
	char		Zipcode[10][10];

#define	SQLITE_DB_INDX_LAT	8
	double		 Lat[10];

#define	SQLITE_DB_INDX_LON	9
	double		 Lon[10];

#define	SQLITE_DB_INDX_INVSEQ	10
	u_int32_t		 InvSeq;

#define	SQLITE_DB_INDX_PATENT	11
	char		Patent[10];

#define	SQLITE_DB_INDX_APPDATESTR	12
	char		AppDateStr[64];

#define	SQLITE_DB_INDX_GYEAR	13
	u_int32_t		 GYear;

#define	SQLITE_DB_INDX_ASSIGNEE	14
	char		Assignee[10][64];

#define	SQLITE_DB_INDX_ASGNUM	15
	u_int32_t		 AsgNum[10];

#define	SQLITE_DB_INDX_CLASS	16
	char		Class[64];

#define	SQLITE_DB_INDX_COAUTHS	17
	char		Coauths[256];

#define	SQLITE_DB_INDX_CNT	18
	u_int32_t		 cnt;

#define	SQLITE_DB_INDX_INVNUM_N	19
	char		Invnum_N[16];

#define	SQLITE_DB_INDX_INVNUM	20
	char		Invnum[16];

#define	SQLITE_DB_INDX_BLOCK1	21
	char		Block1[6];

#define	SQLITE_DB_INDX_BLOCK2	22
	char		Block2[9];

#define	SQLITE_DB_INDX_BLOCK3	23
	char		Block3[2];
} DbRecord;
#define SQLITE_DB_NUMFIELDS	24