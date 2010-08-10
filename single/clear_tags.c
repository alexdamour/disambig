#include "comp_engine.h"

DB_ENV	 *dbenv;			/* Database environment */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */

int main(int argc, char *argv[])
{
    char ch;
    char *tagp;
    ulong num=0;
    DB *primary;
    DBC *cur;
    DBT key, data;
    u_int32_t start_rec=0;
    u_int32_t num_recs=0;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    while ((ch = getopt(argc, argv, "n:")) != EOF)
		switch (ch) {
		case 'n':
            start_rec = (u_int32_t)atol(optarg);
            break;
        }

    argc -= optind;
	argv += optind;

    if (*argv != NULL)
		return (EXIT_FAILURE);

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&primary, "primary", DB_BTREE, 32*1024, DB_CREATE, 0, compare_uint32);
    //primary->stat(primary, NULL, &stat, 0);
    //printf("numkeys in primary: %lu\n", (u_long)stat->bt_nkeys);
    //free(stat);
    //sqlite3_open("sqlite_dbs/invpat.sqlite3", &sql_db);

    primary->cursor(primary, NULL, &cur, 0);

    while(DB_NOTFOUND != cur->get(cur, &key, &data, DB_NEXT)){
        if(num_recs++ < start_rec){
            //printf("skipping!");
            continue;
        }
        if((tagp = has_tag((DbRecord*)data.data))!=NULL){
//            printf("found one!\n");
            memset(tagp, '\0', 16);
            primary->put(primary, NULL, &key, &data, 0);
        }/*
        if(!(num%100000)){
            printf("\t%lu\n", num);
            cur->dup(cur, &tmp_cur, DB_POSITION);
            tmp_cur->get(cur, &key, &data, DB_CURRENT);
            cur->close(cur);
            cur = tmp_cur;
        }
        ++num;*/
    }
    cur->close(cur);
    primary->close(primary, 0);
    return(0);
}
