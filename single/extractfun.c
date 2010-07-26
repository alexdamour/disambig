#include "comp_spec.h"

DbField fieldlist[];

int extract(DbRecord *rp, const int func_idx, void **result, size_t* size,int* is_array, size_t *array_len_real, size_t *array_len_max){
    int extract_malloc=0, i;
    if(func_idx < SQLITE_DB_NUMFIELDS){ 
        *is_array = fieldlist[func_idx-1].array;
        *array_len_max = *is_array ? fieldlist[func_idx-1].array_len : 0;
        *result = (char*)rp + fieldlist[func_idx-1].offset;
        *size = fieldlist[func_idx-1].size;

        for(i=0; i< (int)*array_len_max; ++i){
            if(*((char*)rp+fieldlist[func_idx-1].offset+(*size)*i) ==
               *((char*)(&DbRecord_base)+fieldlist[func_idx-1].offset+(*size)*i))
                break;
        }
        *array_len_real = (size_t)i;
        //if(func_idx==SQLITE_DB_INDX_FIRSTNAME)
        //    printf("array_len: %d\n",(int)*array_len_real);
    }
    else if(func_idx == (int)LATLON){
        extract_malloc=1;
        *is_array = 1;
        *array_len_max = fieldlist[SQLITE_DB_INDX_LAT].array_len;
        *result = malloc(sizeof(latlon)*(*array_len_max));
        for(i=0; i < (int)*array_len_max; ++i){
            (((latlon*)*(result))+i)->lat = rp->Lat[i];
            (((latlon*)*(result))+i)->lon = rp->Lon[i];
            (((latlon*)*(result))+i)->street = rp->Street[i];
            if(rp->Lat[i]==DbRecord_base.Lat[i])
                break;
        }
        *size = sizeof(latlon);
        *array_len_real = (size_t)i;
    }
    else if(func_idx == (int)ASG_FIELDS){
        extract_malloc=1;
        *is_array = 1;
        *array_len_max = fieldlist[SQLITE_DB_INDX_ASSIGNEE].array_len;
        *result = malloc(sizeof(asg_struct)*(*array_len_max));
        for(i=0; i<(int)*array_len_max; ++i){
            (((asg_struct*)*(result))+i)->asgname = rp->Assignee[i];
            (((asg_struct*)*(result))+i)->asgnum = rp->AsgNum[i];
            if(0==strcmp(rp->Assignee[i],DbRecord_base.Assignee[i]) && 
                rp->AsgNum[i]==DbRecord_base.AsgNum[i])
                break;
        }
        *size = sizeof(asg_struct);
        *array_len_real = (size_t)i;
    }
    return(extract_malloc);
}
