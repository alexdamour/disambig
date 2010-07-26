#include "comp_spec.h"

DB* asg_freq_hash;

int comp_env_init(){
    sqlite_db_primary_open(&asg_freq_hash, "asg_freq_hash", DB_BTREE, 8*1024, DB_CREATE, 0, compare_uint32);
}

int comp_env_clean(){
    asg_freq_hash->close(asg_freq_hash, 0);
}
