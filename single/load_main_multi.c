/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005,2008 Oracle.  All rights reserved.
 *
 * $Id: load_main.c,v 1.13 2008/01/08 20:58:23 bostic Exp $
 */

#include "sqlite_db.h"
#include "sqlite_db_local.h"
#include "sqlite_db_extern.h"

static int usage(void);

/*
 * Globals
 */
DB_ENV	 *dbenv;			/* Database environment */
int	  verbose;			/* Program verbosity */
char	 *progname;			/* Program name */

int
main(int argc, char *argv[])
{
    DB* db;
    clock_t start, end;
	u_long version;
	int ch, ret, t_ret;
	char *home;
    char *sql_table;
    sqlite3 *sql_db;

	/* Initialize globals. */
	dbenv = NULL;
	if ((progname = strrchr(argv[0], '/')) == NULL)
		progname = argv[0];
	else
		++progname;
	verbose = 0;

	/* Initialize arguments. */
	home = NULL;
	version = 1;

	/* Process arguments. */
	while ((ch = getopt(argc, argv, "d:t:h:V:v")) != EOF)
		switch (ch) {
		case 'd':
            /*Open the SQLITE DB*/
			if (sqlite3_open(optarg, &sql_db)) {
				fprintf(stderr,
				    "%s: Can't open database. %s\n", optarg, db_strerror(errno));
				return (EXIT_FAILURE);
			}
			break;
        case 't':
            sql_table = optarg; 
            break;
		case 'h':
			home = optarg;
			break;
		case 'V':
			if (strtoul_err(optarg, &version))
				return (EXIT_FAILURE);
			break;
		case 'v':
			++verbose;
			break;
		case '?':
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	if (*argv != NULL)
		return (usage());

	/*
	 * The home directory may not exist -- try and create it.  We don't
	 * bother to distinguish between failure to create it and it already
	 * existing, as the database environment open will fail if we aren't
	 * successful.
	 */
	if (home == NULL)
		home = getenv("DB_HOME");
	if (home != NULL)
		(void)mkdir(home, S_IRWXU);

	/* Create or join the database environment. */
	if (sqlite_db_env_open(home) != 0)
		return (EXIT_FAILURE);
    if(ret = sqlite_db_primary_open(&db, "primary_multi", DB_BTREE, 32*1024, DB_CREATE, DB_DUPSORT, NULL))
        return(EXIT_FAILURE);

	/* Load records into the database. */
    printf("Timing started ...\n");
    start = clock();
	ret = input_load(db, sql_db, sql_table);
    sqlite3_close(sql_db);
    end = clock();
    printf("Timing stopped.\n");
    
    printf("CPU Time: %f\n", ((double) (end - start))/CLOCKS_PER_SEC);

	/* Close the database environment. */
    db->close(db, 0);
	if ((t_ret = sqlite_db_env_close()) != 0 && ret == 0)
		ret = t_ret;

	return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

/*
 * usage --
 *	Program usage message.
 */
static int
usage(void)
{
	(void)fprintf(stderr,
	    "usage: %s [-v] [-d sqlite_db-file] [-t table_name] [-h home]\n", progname);
	return (EXIT_FAILURE);
}
