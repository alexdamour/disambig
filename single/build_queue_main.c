#include "build_queue.h"

DB_ENV *dbenv;
char   *progname;

int
main(int argc, char** argv){
    char ch;
    DB* prim, *block;
    progname = argv[0];
    int fromfile = 0;
    FILE *fp;

    while ((ch = getopt(argc, argv, "f:")) != EOF)
		switch (ch) {
		case 'f':
            fromfile = 1;
            fp = fopen(optarg, "r");
			break;
        }

    sqlite_db_env_open(NULL);

    if(fromfile)
        read_blocks(fp);

    else{
        sqlite_db_primary_open(&prim, "primary", DB_BTREE, 32*1024, DB_CREATE, 0, compare_uint32);
        sqlite_db_secondary_open(prim, &block, "block_idx", 8*1024, DB_DUPSORT, blocking_callback, compare_uint32);

        list_blocks(block);

        block->close(block,0);
        prim->close(prim,0);
    }

    dbenv->close(dbenv,0);
}
