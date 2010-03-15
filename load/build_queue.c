#include "build_queue.h"

int
list_blocks(DB *sdb){
    int ret, queue_lim = 100;
    db_recno_t  dup_count;
    DB *block_queue;
    DBC *cursor;
    DBT key, data;
    db_recno_t block_count = 0;
    char *filename = "big_blocks.txt";
    size_t numblocks = 0;
    FILE *fp;

    if((ret = sqlite_db_primary_open(&block_queue, "block_queue_2", DB_QUEUE, 4*1024, DB_CREATE, 0, NULL))){
        printf("Open Failed!");
        abort();
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    sdb->cursor(sdb, NULL, &cursor, 0);
    printf("Starting...\n");
    while(DB_NOTFOUND != cursor->get(cursor, &key, &data, DB_NEXT_NODUP)){
        cursor->count(cursor, &dup_count, 0);
        if((int)dup_count >= 80 and  (int)dup_count <= 110){
            DBT_CLEAR(data);
            ++block_count;
            if(!(++numblocks%10000)) printf("\t%lu blocks imported...\n", (ulong)numblocks);
            block_queue->put(block_queue, NULL, &data, &key, DB_APPEND);   
            //cursor->count(cursor, &dup_count, 0);
            //if((int)dup_count > 500)
            //    fprintf(fp, "%s, %u\n", (char*)key.data, (size_t)dup_count);
            if(queue_lim > 0)
                if(--queue_lim <= 0) break;
        }
    }
    printf("Num blocks: %lu\n", (ulong)block_count);
    cursor->close(cursor);
    block_queue->close(block_queue, 0);
    return 0;
}

int
read_blocks(FILE *fp){
    int ret;
    char *p, *t, save_ch, buf[512];
    DBT key, data;
    DB *block_queue;
    db_recno_t numrecs = 0;
    DBT_CLEAR(key);
    DBT_CLEAR(data);

    if((ret = sqlite_db_primary_open(&block_queue, "block_queue_file", DB_QUEUE, 4*1024, DB_CREATE, 0, NULL))){
        printf("Open Failed!");
        abort();
    }

    while(fgets(buf, sizeof(buf), fp) != NULL) {
        if((p=strchr(buf, '\n')) == NULL) {
            fprintf(stderr, "Input line too long\n");
            return(1);
        }
        p = buf;

        for (t=p; *t != '\n' || *t != ','; ++t)
            ;
        save_ch = *t;
        *t = '\0';

        ++numrecs;

        key.data = &numrecs;
        key.size = sizeof(numrecs);

        data.data = p;
        data.size = strlen(p)+1;

        block_queue->put(block_queue, NULL, &data, &key, DB_APPEND);
    }
    printf("Num blocks: %lu\n", (ulong)numrecs);
    return(0);
}
