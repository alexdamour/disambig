/*
 *  DO NOT EDIT: automatically built by sqlite_db_code.
 *
 * Initialized record structure.
 */

#include "sqlite_db.h"
#include "sqlite_db_local.h"

DbRecord DbRecord_base = {
	"",		/* Firstname */
	"",		/* Lastname */
	"",		/* Street */
	"",		/* City */
	"",		/* State */
	"",		/* Country */
	"",		/* Zipcode */
	0,		/* Lat */
	0,		/* Lon */
	"",		/* Nationality */
	0,		/* InvSeq */
	"",		/* Patent */
	"",		/* Kind */
	0,		/* Claims */
	0,		/* AppType */
	"",		/* AppNum */
	"",		/* AppYearStr */
	{0,0,0,1,0,0,0,0,0,0,"GMT"},		/* AppDateStr */
	"",		/* Law_id */
	"",		/* Assignee */
	0,		/* AsgNum */
	"",		/* Class */
	"",		/* ClassSub */
	"",		/* Invnum_N */
	"",		/* Coauths */
	0,		/* Num_Coauths */
};

DbField fieldlist[] = {
	{ "Firstname", SQLITE_DB_INDX_FIRSTNAME,
	    STRING, 32, 0, NULL, FIELD_OFFSET(Firstname)},
	{ "Lastname", SQLITE_DB_INDX_LASTNAME,
	    STRING, 64, 0, NULL, FIELD_OFFSET(Lastname)},
	{ "Street", SQLITE_DB_INDX_STREET,
	    STRING, 64, 0, NULL, FIELD_OFFSET(Street)},
	{ "City", SQLITE_DB_INDX_CITY,
	    STRING, 32, 0, NULL, FIELD_OFFSET(City)},
	{ "State", SQLITE_DB_INDX_STATE,
	    STRING, 3, 0, NULL, FIELD_OFFSET(State)},
	{ "Country", SQLITE_DB_INDX_COUNTRY,
	    STRING, 3, 0, NULL, FIELD_OFFSET(Country)},
	{ "Zipcode", SQLITE_DB_INDX_ZIPCODE,
	    STRING, 6, 0, NULL, FIELD_OFFSET(Zipcode)},
	{ "Lat", SQLITE_DB_INDX_LAT,
	    DOUBLE, sizeof(double), 0, NULL, FIELD_OFFSET(Lat)},
	{ "Lon", SQLITE_DB_INDX_LON,
	    DOUBLE, sizeof(double), 0, NULL, FIELD_OFFSET(Lon)},
	{ "Nationality", SQLITE_DB_INDX_NATIONALITY,
	    STRING, 3, 0, NULL, FIELD_OFFSET(Nationality)},
	{ "InvSeq", SQLITE_DB_INDX_INVSEQ,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, FIELD_OFFSET(InvSeq)},
	{ "Patent", SQLITE_DB_INDX_PATENT,
	    STRING, 9, 0, NULL, FIELD_OFFSET(Patent)},
	{ "Kind", SQLITE_DB_INDX_KIND,
	    STRING, 3, 0, NULL, FIELD_OFFSET(Kind)},
	{ "Claims", SQLITE_DB_INDX_CLAIMS,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, FIELD_OFFSET(Claims)},
	{ "AppType", SQLITE_DB_INDX_APPTYPE,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, FIELD_OFFSET(AppType)},
	{ "AppNum", SQLITE_DB_INDX_APPNUM,
	    STRING, 16, 0, NULL, FIELD_OFFSET(AppNum)},
	{ "AppYearStr", SQLITE_DB_INDX_APPYEARSTR,
	    STRING, 5, 0, NULL, FIELD_OFFSET(AppYearStr)},
	{ "AppDateStr", SQLITE_DB_INDX_APPDATESTR,
	    DATE, sizeof(struct tm), 0, NULL, FIELD_OFFSET(AppDateStr)},
	{ "Law_id", SQLITE_DB_INDX_LAW_ID,
	    STRING, 64, 0, NULL, FIELD_OFFSET(Law_id)},
	{ "Assignee", SQLITE_DB_INDX_ASSIGNEE,
	    STRING, 64, 0, NULL, FIELD_OFFSET(Assignee)},
	{ "AsgNum", SQLITE_DB_INDX_ASGNUM,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, FIELD_OFFSET(AsgNum)},
	{ "Class", SQLITE_DB_INDX_CLASS,
	    STRING, 32, 0, NULL, FIELD_OFFSET(Class)},
	{ "ClassSub", SQLITE_DB_INDX_CLASSSUB,
	    STRING, 64, 0, NULL, FIELD_OFFSET(ClassSub)},
	{ "Invnum_N", SQLITE_DB_INDX_INVNUM_N,
	    STRING, 16, 0, NULL, FIELD_OFFSET(Invnum_N)},
	{ "Coauths", SQLITE_DB_INDX_COAUTHS,
	    STRING, 192, 0, NULL, FIELD_OFFSET(Coauths)},
	{ "Num_Coauths", SQLITE_DB_INDX_NUM_COAUTHS,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, FIELD_OFFSET(Num_Coauths)},
	{NULL, 0, STRING, 0, 0, NULL, 0}
};

char* sql_query = "select * from invpat_sort;";
