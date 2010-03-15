#include "comp_engine.h"

DB_ENV	 *dbenv;			/* Database environment */
DB	 *db;				/* Primary database */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */
double Pr_M = 1./100;

int main(int argc, char** argv){
    int ret;

    DB  *db, *block_db;
    DBT mult_data;
    DBT data_i, key_i, pkey_i;
    DBT data_j, key_j, pkey_j;
    DBC *better_work;

    void *multp_i, *multp_j;
    
    DBT_CLEAR(mult_data);

    DBT_CLEAR(data_i);
    DBT_CLEAR(key_i);
    DBT_CLEAR(pkey_i);

    DBT_CLEAR(data_j);
    DBT_CLEAR(key_j);
    DBT_CLEAR(pkey_j);

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&db, "primary", DB_BTREE, 32*1024, 0, 0, compare_uint32);
    sqlite_db_secondary_open(db, &block_db, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);
    block_db->cursor(block_db, NULL, &better_work, 0);


    key_i.data = "L.FLEMING";
    key_i.size = sizeof("L.FLEMING");
    better_work->get(better_work, &key_i, &pkey_i, &mult_data, DB_MULTIPLE | DB_SET);

    DB_MULTIPLE_INIT(multp_i, &mult_data);
    DB_MULTIPLE_INIT(multp_j, &mult_data);

    while(multp_i != NULL){
        DB_MULTIPLE_NEXT(multp_i, &mult_data, (data_i.data), (data_i.size));
        printf("%s %lu, ", ((DbRecord*)data_i.data)->Patent, (u_long)((DbRecord*)data_i.data)->InvSeq);
        multp_j = multp_i;
        while(multp_j != NULL){
            DB_MULTIPLE_NEXT(multp_j, &mult_data, (data_j.data), (data_j.size));
            printf("%s %li\n", ((DbRecord*)data_j.data)->Patent, (u_long)((DbRecord*)data_j.data)->InvSeq);
        }
    }

    block_db->close(block_db,0); 
    db->close(db,0);
    dbenv->close(dbenv,0);
    return(ret);
}
