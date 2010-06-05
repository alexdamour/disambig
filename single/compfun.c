#include "comp_spec.h"
#include "strcmp95.h"

/*
 * Globals
 */

#define DEG2RAD 5729.58

DbField fieldlist[];
int *jw_opts[] = {0,0};


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

int jwcmp(const void* str1, const void* str2, size_t size){
    //res = (s1 == NULL || s2 == NULL || strlen(s1)*strlen(s2)==0) + 2*(myres >= 0.75) + (myres >= 0.85) + (myres >= 0.95) + (myres >= 0.9999);
    //printf("res: %g\n", myres);
    //return(res);

    char *delim=" ";

    //char *num_delim="~";
    char *name1, *name2, *n1, *n2, *n1_last, *n2_last;
    int len1, len2, tok_len1, tok_len2, num_tok1=0, num_tok2=0;
    int missing=0, same_len=0;
    double score=0, tok_score, myres;

    if(str1 == NULL || str2 == NULL || strlen((char*)str1)*strlen((char*)str2) == 0)
        //missing!
        return 1;

    name1 = (char*)malloc(size);
    name2 = (char*)malloc(size);
    //if(strlen((char*)str1) <= strlen((char*)str2)){
        memcpy(name1, (char*)str1, size);
        memcpy(name2, (char*)str2, size);
        /*
    }
    else{
        memcpy(name1, (char*)str2, size);
        memcpy(name2, (char*)str1, size);
    }
    */
    //printf("\targs: %s, %s\n", class1, class2);
    
    len1 = strlen(name1);
    len2 = strlen(name2);

    n1 = n1_last = name1;

    while((n1_last=strsep(&n1, delim)) != NULL){
        tok_len1 = strlen(n1_last);
        tok_score=0;
        num_tok2=0;
        memcpy(name2, (char*)str2, size);
        n2 = n2_last = name2;
        while((n2_last=strsep(&n2, delim)) != NULL){
//            printf("\t\t%s, %s\n", n1_last, n2_last);
            tok_len2 = strlen(n2_last);
            tok_score = (MIN(tok_len1, tok_len2) <= 1) ? 0 : MAX(tok_score, strcmp95(n1_last, n2_last, MAX(tok_len1, tok_len2), jw_opts));
            num_tok2 += (tok_len2 > 1);
            //printf("\t\ttok_score: %g\n", tok_score);
        }
        score += tok_score > 0.95;
        num_tok1 += (tok_len1 > 1);
    }
//    printf("score: %g, num_tok1: %d, num_tok2: %d\n", score, num_tok1, num_tok2);

    myres = (MIN(num_tok1, num_tok2) == 0) ? 0 : score/MIN(num_tok1,num_tok2);
    same_len = (num_tok1==num_tok2);
//    printf("myres: %g\n", myres);

    //printf("nc1: %d\n", nc1);
    //printf("nc2: %d\n", nc2);

    free(name1);
    free(name2);

    return(2*(myres >= 0.33) + (myres >= 0.66) + (myres > 0.99) + (myres > 0.99 && MIN(num_tok1, num_tok2) >= 2) + (myres > 0.99 && same_len));

}

int midnamecmp(const void *str1, const void *str2, size_t size){
    int res, matches = 0, missing=0;
    double raw = 0;
    char* delim=" ";
    char *name1 = (char*)malloc(size);
    char *name2 = (char*)malloc(size);
    char *n1, *n2, *n1_last, *n2_last;
    int num_names_1=0, num_names_2=0;
    memcpy(name1, (char*)str1, size);
    memcpy(name2, (char*)str2, size);

    //printf("%s, %s\n", name1, name2);

    n1 = n1_last = name1;
    //Pull off first initial because we don't care anymore
    n1_last = strsep(&n1, delim);

    while((n1_last=strsep(&n1, delim)) != NULL){
        num_names_2=0;
        memcpy(name2, (char*)str2, size);
        n2 = n2_last = name2;
        n2_last = strsep(&n2, delim);
        while((n2_last=strsep(&n2, delim)) != NULL){
            matches += (n1_last[0] == n2_last[0]);
            ++num_names_2;
        }
        ++num_names_1;
    }

    free(name1);
    free(name2);

    //printf("num_names_1: %d\n", num_names_1);
    //printf("num_names_2: %d\n", num_names_2);

    missing = (MIN(num_names_1, num_names_2)==0);
    raw = missing ? 0 : (double)matches/(double)(MIN(num_names_1, num_names_2));
    //printf("midname_raw: %g\n", raw);

    return(missing + 2*(raw > 0.33) + (raw > 0.67) + (raw > 0.99));
}

int directcmp(const void* str1, const void* str2, size_t size){
//    printf("DCOMP:\n");
    int res;
    char *s1 = (char*)str1, *s2 = (char*)str2;
//    printf("\targs: %s, %s\n", s1, s2);
    res = strncmp(s1, s2, size);
//    printf("\traw: %d\n", res);
    return((strlen(s1)*strlen(s2)==0 || (res==0)) + (res==0));
}

int distcmp(const void* latlon1, const void* latlon2, size_t size){
//    printf("DISTCOMP:\n");
    double dist;
    latlon *ll1 = (latlon*)latlon1, *ll2 = (latlon*)latlon2;
    int missing = ((abs(ll1->lat)<0.0001 && abs(ll1->lon)< 0.0001) || (abs(ll2->lat)<0.0001 && abs(ll2->lon)<0.0001));
    size = size;

    dist = missing ? 101 : (3963.0 * acos(sin(ll1->lat/DEG2RAD) * sin(ll2->lat/DEG2RAD) + cos(ll1->lat/DEG2RAD) * cos(ll2->lat/DEG2RAD) *  cos(ll2->lon/DEG2RAD -ll1->lon/DEG2RAD)));
    //if(dist > 1){
        //printf("\targs: %f, %f ; %f, %f\n", ll1->lat, ll1->lon, ll2->lat, ll2->lon);
        //printf("\traw: %f\n", dist);
    //}

    return(missing + 2*(dist < 100) + (dist < 75) + (dist < 50) + (dist < 10) + (dist < 1));
}

int disttypecmp(const void* dt1, const void* dt2, size_t size){
    int res;
    res = ((!strncmp((char*)dt1, "US", 2)) && (!strncmp((char*)dt2, "US", 2))) ? HAVE_STREET : NO_STREET ;
    //printf("has street: %d\n", res);
    return(res);
}

int asgcmp(const void* asg1, const void* asg2, size_t size){
    if(((asg_struct*)asg1)->asgnum != 0 || ((asg_struct*)asg2)->asgnum != 0)
        if(((asg_struct*)asg1)->asgnum == 0 || ((asg_struct*)asg2)->asgnum == 0 ||
            (((asg_struct*)asg1)->asgnum) != (((asg_struct*)asg2)->asgnum)){
    //        printf("%lu:%s :: %lu:%s\n", (ulong)((asg_struct*)asg1)->asgnum, ((asg_struct*)asg1)->asgname,
    //                                     (ulong)((asg_struct*)asg2)->asgnum, ((asg_struct*)asg2)->asgname);
            return JWSUB33;
        }
        else
            return JW100MULTFULL;
    else
        return(jwcmp(((asg_struct*)asg1)->asgname, ((asg_struct*)asg2)->asgname, fieldlist[SQLITE_DB_INDX_ASSIGNEE-1].size));
}
    

int classcmp(const void* cstr1, const void* cstr2, size_t size){
    //printf("CLASSCOMP:\n");
    char *delim="/";
    char *num_delim="~";
    char *class1, *class2, *c1, *c2, *c1_last, *c2_last;
    class1 = (char*)malloc(size);
    class2 = (char*)malloc(size);
    memcpy(class1, (char*)cstr1, size);
    memcpy(class2, (char*)cstr2, size);
    //printf("\targs: %s, %s\n", class1, class2);
    int nc1=-1, nc2=-1, matches=0;
    int missing=0;
    double raw;

    c1 = c1_last = class1;

    while((c1_last=strsep(&c1, delim)) != NULL){
        c1_last = strsep(&c1_last, num_delim);
        nc2=-1;
        memcpy(class2, (char*)cstr2, size);
        c2 = c2_last = class2;
        while((c2_last=strsep(&c2, delim)) != NULL){
            c2_last = strsep(&c2_last, num_delim);
            //printf("\t\t%s, %s\n", c1_last, c2_last);
            matches += (strcmp(c1_last, c2_last)==0);
            ++nc2;
        }
        ++nc1;
    }

    //printf("nc1: %d\n", nc1);
    //printf("nc2: %d\n", nc2);

    free(class1);
    free(class2);
    //printf("\tmatches: %d", matches);

    missing = (MIN(nc1,nc2)==0);
    raw = missing ? 0 : (double)matches/(double)(MIN(nc1, nc2));
    //printf("\traw: %f\n", raw);
    return(missing + 2*(raw > 0.25) + (raw > 0.5) + (raw > 0.75));
}

int coauthcmp(const void* cstr1, const void* cstr2, size_t size){
    char *delim="/";
    char *num_delim="~";
    char *coauths1, *coauths2, *c1, *c2, *c1_last, *c2_last;
    coauths1 = (char*)malloc(size);
    coauths2 = (char*)malloc(size);
    memcpy(coauths1, (char*)cstr1, size);
    memcpy(coauths2, (char*)cstr2, size);
    int matches=0;

    c1 = c1_last = coauths1;
    c2 = c2_last = coauths2;
    //printf("%s\n%s\n", c1, c2);

    while((c1_last=strsep(&c1, delim)) != NULL){
        c1_last = strsep(&c1_last, num_delim);
        memcpy(coauths2, (char*)cstr2, size);
        c2 = c2_last = coauths2;
        while((c2_last=strsep(&c2, delim)) != NULL){
            c2_last = strsep(&c2_last, num_delim);
            //printf("%s, %s\n", c1_last, c2_last);
            matches += (strcmp(c1_last, c2_last)==0);
            if(c2 != NULL)
                *(c2-1) = delim[0];
        }
    }

//    if(matches > 10)
//        printf("%d matches:\n\ts1: %s\n\ts2: %s\n", matches, (char*)cstr1, (char*)cstr2);

    free(coauths1);
    free(coauths2);

    return(MIN(matches, 10));
}
