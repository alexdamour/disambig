/*
 *  DO NOT EDIT: automatically built by sqlite_db_code.
 *
 * Initialized record structure.
 */

#include "sqlite_db.h"
#include "sqlite_db_local.h"

DbRecord DbRecord_base = {
{	"",},		/* Firstname */
{	"",},		/* Lastname */
{	"",},		/* Street */
{	"",},		/* City */
{	"",},		/* State */
{	"",},		/* Country */
{	"",},		/* Zipcode */
{	0,},		/* Lat */
{	0,},		/* Lon */
	0,		/* InvSeq */
	"",		/* Patent */
	"",		/* AppDateStr */
	0,		/* GYear */
{	"",},		/* Assignee */
{	0,},		/* AsgNum */
	"",		/* Class */
	"",		/* Coauths */
	0,		/* Cnt */
	"",		/* Invnum_N */
	"",		/* Invnum */
	"",		/* Block1 */
	"",		/* Block2 */
	"",		/* Block3 */
};

DbField fieldlist[] = {
	{ "Firstname", SQLITE_DB_INDX_FIRSTNAME,
	    STRING, 32, 0, NULL, 1, 1, FIELD_OFFSET(Firstname)},
	{ "Lastname", SQLITE_DB_INDX_LASTNAME,
	    STRING, 64, 0, NULL, 1, 1, FIELD_OFFSET(Lastname)},
	{ "Street", SQLITE_DB_INDX_STREET,
	    STRING, 64, 0, NULL, 1, 1, FIELD_OFFSET(Street)},
	{ "City", SQLITE_DB_INDX_CITY,
	    STRING, 32, 0, NULL, 1, 1, FIELD_OFFSET(City)},
	{ "State", SQLITE_DB_INDX_STATE,
	    STRING, 3, 0, NULL, 1, 1, FIELD_OFFSET(State)},
	{ "Country", SQLITE_DB_INDX_COUNTRY,
	    STRING, 3, 0, NULL, 1, 1, FIELD_OFFSET(Country)},
	{ "Zipcode", SQLITE_DB_INDX_ZIPCODE,
	    STRING, 6, 0, NULL, 1, 1, FIELD_OFFSET(Zipcode)},
	{ "Lat", SQLITE_DB_INDX_LAT,
	    DOUBLE, sizeof(double), 0, NULL, 1, 1, FIELD_OFFSET(Lat)},
	{ "Lon", SQLITE_DB_INDX_LON,
	    DOUBLE, sizeof(double), 0, NULL, 1, 1, FIELD_OFFSET(Lon)},
	{ "InvSeq", SQLITE_DB_INDX_INVSEQ,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, 0, 0, FIELD_OFFSET(InvSeq)},
	{ "Patent", SQLITE_DB_INDX_PATENT,
	    STRING, 10, 0, NULL, 0, 0, FIELD_OFFSET(Patent)},
	{ "AppDateStr", SQLITE_DB_INDX_APPDATESTR,
	    STRING, 8, 0, NULL, 0, 0, FIELD_OFFSET(AppDateStr)},
	{ "GYear", SQLITE_DB_INDX_GYEAR,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, 0, 0, FIELD_OFFSET(GYear)},
	{ "Assignee", SQLITE_DB_INDX_ASSIGNEE,
	    STRING, 64, 0, NULL, 1, 1, FIELD_OFFSET(Assignee)},
	{ "AsgNum", SQLITE_DB_INDX_ASGNUM,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, 1, 1, FIELD_OFFSET(AsgNum)},
	{ "Class", SQLITE_DB_INDX_CLASS,
	    STRING, 32, 0, NULL, 0, 0, FIELD_OFFSET(Class)},
	{ "Coauths", SQLITE_DB_INDX_COAUTHS,
	    STRING, 192, 0, NULL, 0, 0, FIELD_OFFSET(Coauths)},
	{ "Cnt", SQLITE_DB_INDX_CNT,
	    UNSIGNED_LONG, sizeof(u_int32_t), 0, NULL, 0, 0, FIELD_OFFSET(Cnt)},
	{ "Invnum_N", SQLITE_DB_INDX_INVNUM_N,
	    STRING, 16, 0, NULL, 0, 0, FIELD_OFFSET(Invnum_N)},
	{ "Invnum", SQLITE_DB_INDX_INVNUM,
	    STRING, 16, 0, NULL, 0, 0, FIELD_OFFSET(Invnum)},
	{ "Block1", SQLITE_DB_INDX_BLOCK1,
	    STRING, 16, 0, NULL, 0, 0, FIELD_OFFSET(Block1)},
	{ "Block2", SQLITE_DB_INDX_BLOCK2,
	    STRING, 3, 0, NULL, 0, 0, FIELD_OFFSET(Block2)},
	{ "Block3", SQLITE_DB_INDX_BLOCK3,
	    STRING, 3, 0, NULL, 0, 0, FIELD_OFFSET(Block3)},
	{NULL, 0, STRING, 0, 0, NULL, 0, 0, 0}
};

char* sql_query = "select * from invpat_8_sort;";
