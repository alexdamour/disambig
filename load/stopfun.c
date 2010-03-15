#include "comp_engine.h"

int stop_comp(DbRecord* dbi, DbRecord* dbj){ 
    return( strcmp(dbi->Patent, dbj->Patent)==0 );
}
