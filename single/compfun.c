#include "comp_spec.h"
#include "strcmp95.h"

/*
 * Globals
 */

#define DEG2RAD 5729.58

DbField fieldlist[];
int *jw_opts[] = {0,0};

/* Managed in db_env.c */
DB* asg_freq_hash;

int nospacecmp(const char* str1, const char* str2){
    char *c1, *c2;
    for(c1 = str1, c2=str2; (*c1 != '\0') && (*c2 != '\0'); c1++, c2++){
        if(*c1 == ' ') c1++;
        if(*c2 == ' ') c2++;
        if(*c1!=*c2){
            return -1 + (*c1 > *c2)*2;
        }
    }
    return 0 + *c1!='\0' - *c2!='\0';
}

int jwcmp(const void* str1, const void* str2, size_t size){
    char *delim=" ";

    char *name1, *name2, *n1, *n2, *n1_last, *n2_last;
    int len1, len2, tok_len1, tok_len2, num_tok1=0, num_tok2=0;
    int same_len=0;
    double score=0, tok_score, myres;

    if(str1 == NULL || str2 == NULL || strlen((char*)str1)*strlen((char*)str2) == 0)
        //missing!
        return 1;
/*
    if(strcmp((char*)str1, "BARCOHEN")==0 || strcmp((char*)str2, "BARCOHEN")==0)
        printf("nospacecmp(%s, %s): %d\n", (char*)str1, (char*)str2, nospacecmp((char*)str1, (char*)str2));
*/
    if(strcmp((char*)str1, (char*)str2) && 0==nospacecmp((char*)str1, (char*)str2)){
        //printf("Nospace match: %s, %s\n", (char*)str1, (char*)str2);
        return JW100;
    }

    name1 = (char*)malloc(size);
    name2 = (char*)malloc(size);
    memcpy(name1, (char*)str1, size);
    memcpy(name2, (char*)str2, size);

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
            // printf("\t\t%s, %s\n", n1_last, n2_last);
            tok_len2 = strlen(n2_last);
            tok_score = MAX(tok_score, ((MIN(tok_len1, tok_len2) <= 1) ? 0 : strcmp95(n1_last, n2_last, MAX(tok_len1, tok_len2)+1, jw_opts)));
            num_tok2 += (tok_len2 > 1);
            
            //printf("\t\ttok_score: %g\n", tok_score);
        }
        score += (tok_score > 0.95);
        num_tok1 += (tok_len1 > 1);
    }
    /*
        printf("%s, %s\n", name1, name2);
        printf("score: %g, num_tok1: %d, num_tok2: %d\n", score, num_tok1, num_tok2);
        */

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

/*
int directcmp(const void* str1, const void* str2, size_t size){
//    printf("DCOMP:\n");
    int res;
    char *s1 = (char*)str1, *s2 = (char*)str2;
//    printf("\targs: %s, %s\n", s1, s2);
    res = strncmp(s1, s2, size);
//    printf("\traw: %d\n", res);
    return((strlen(s1)*strlen(s2)==0 || (res==0)) + (res==0));
}
*/

/*                  DISTANCE COMPARISON                 */
/* We have more detailed information about addresses inside */
/* the United States, so we take this into account by roughly */
/* drawing a circumscribing rectangle around the contiguous 48. */
/* Super low-tech GIS.*/
int distcmp(const void* latlon1, const void* latlon2, size_t size){
//    printf("DISTCOMP:\n");
    /* Extreme points of contiguous 48 */
    double northernmost=4938;
    double southernmost=2454;
    double easternmost=-6695;
    double westernmost=-12451;

    double dist;
    int streetmatch;
    
    latlon *ll1 = (latlon*)latlon1, *ll2 = (latlon*)latlon2;
    int missing = ((abs(ll1->lat)<0.0001 && abs(ll1->lon)< 0.0001) || (abs(ll2->lat)<0.0001 && abs(ll2->lon)<0.0001));
    int in_us = ll1->lat < northernmost && ll1->lat > southernmost &&
                ll1->lon < easternmost && ll1->lon > westernmost &&
                ll2->lat < northernmost && ll2->lat > southernmost &&
                ll2->lon < easternmost && ll1->lon > westernmost;
    size = size;

    dist = 3963.0 * acos(sin(ll1->lat/DEG2RAD) * sin(ll2->lat/DEG2RAD) + cos(ll1->lat/DEG2RAD) * cos(ll2->lat/DEG2RAD) *  cos(ll2->lon/DEG2RAD -ll1->lon/DEG2RAD));
    //if(dist > 1){
        //printf("\targs: %f, %f ; %f, %f\n", ll1->lat, ll1->lon, ll2->lat, ll2->lon);
        //printf("\traw: %f\n", dist);
    //}
    streetmatch = (((latlon*)latlon1)->street!=NULL) &&
                  (((latlon*)latlon2)->street!=NULL) &&
                  (((latlon*)latlon1)->street[0] != '\0')&&
                  (((latlon*)latlon2)->street[0] != '\0')&&
                  (strcmp(((latlon*)latlon1)->street, ((latlon*)latlon2)->street)==0);

    return missing +
           in_us ?
               2*(dist < 100) + (dist < 75) + (dist < 50) + 2*(dist < 10) +
                   ((dist < 1) &&  streetmatch): 
               2*(dist < 100) + (dist < 75) + (dist < 50) + (dist < 10);
}
/*
int disttypecmp(const void* dt1, const void* dt2, size_t size){
    int res;
    res = ((!strncmp((char*)dt1, "US", 2)) && (!strncmp((char*)dt2, "US", 2))) ? HAVE_STREET : NO_STREET ;
    //printf("has street: %d\n", res);
    return(res);
}
*/
int asgcmp(const void* asg1, const void* asg2, size_t size){
    DBT key, data;
    int score;
    if(((asg_struct*)asg1)->asgnum != 0 && ((asg_struct*)asg2)->asgnum != 0 &&
            ((asg_struct*)asg1)->asgnum == ((asg_struct*)asg2)->asgnum){
        DBT_CLEAR(key);
        DBT_CLEAR(data);
        key.data = &(((asg_struct*)asg1)->asgnum);
        key.size = sizeof(((asg_struct*)asg1)->asgnum);
        if(DB_NOTFOUND!=asg_freq_hash->get(asg_freq_hash, NULL, &key, &data, 0)){
            /*
            printf("%lu:%s :: %lu:%s\n", (ulong)((asg_struct*)asg1)->asgnum, ((asg_struct*)asg1)->asgname,
                                     (ulong)((asg_struct*)asg2)->asgnum, ((asg_struct*)asg2)->asgname);
            printf("size: %lu\n", (ulong)*(u_int32_t*)data.data);
            */
            return ASGNUM + (*(u_int32_t*)data.data < 1000) + (*(u_int32_t*)data.data < 100);
        }
        return ASGNUM;
    }
    else{
        score = jwcmp(((asg_struct*)asg1)->asgname,
                      ((asg_struct*)asg2)->asgname,
                      fieldlist[SQLITE_DB_INDX_ASSIGNEE-1].size);
        /*if(score==JW100MULTFULL)
            return ASGNUMSMALL;
        else*/
            return score;
    }
}
    

int classcmp(const void* cstr1, const void* cstr2, size_t size){
    //printf("CLASSCOMP:\n");
    char *delim="/";
    char *num_delim="~";
    char *class1, *class2, *c1, *c2, *c1_last, *c2_last, *debug1, *debug2;
    char* numstr1, *numstr2;
    int num1, num2;
    class1 = (char*)malloc(size);
    class2 = (char*)malloc(size);
    //debug1 = (char*)malloc(size);
    //debug2 = (char*)malloc(size);
    memcpy(class1, (char*)cstr1, size);
    memcpy(class2, (char*)cstr2, size);
    //memcpy(debug1, (char*)cstr1, size);
    //memcpy(debug2, (char*)cstr2, size);
   // printf("\targs: %s, %s\n", class1, class2);
    int nc1=0, nc2=0, matches=0;
    int missing=0;
    double raw;

    c1 = c1_last = class1;

    while((c1_last=strsep(&c1, delim)) != NULL){
        if(*c1_last=='\0')
            break;
        numstr1 = c1_last;
        c1_last = strsep(&numstr1, num_delim);
        nc2=0;
        memcpy(class2, (char*)cstr2, size);
        c2 = c2_last = class2;
        while((c2_last=strsep(&c2, delim)) != NULL){
            if(*c2_last=='\0')
                break;
            numstr2 = c2_last;
            c2_last = strsep(&numstr2, num_delim);
            //printf("\t\t%s, %s\n", c1_last, c2_last);
            if(strcmp(c1_last, c2_last)==0){
                matches += (numstr1==NULL || numstr2==NULL) ? 1 : MIN(atoi(numstr1), atoi(numstr2));
            }
            //matches += (strcmp(c1_last, c2_last)==0);
            ++nc2;
        }
        ++nc1;
    }
    free(class1);
    free(class2);
    //printf("\tmatches: %d", matches);

    missing = (MIN(nc1,nc2)==0);
    return MIN(matches,10);
    //raw = missing ? 0 : (double)matches/(double)(MIN(nc1, nc2));
    /*if(!missing){
        printf("\targs: %s, %s\n", debug1, debug2);
        printf("\traw: %f, missing: %d, score: %d\n", raw, missing, missing + 2*(raw > 0.25) + (raw > 0.5) + (raw > 0.75));
    }
    free(debug1);
    free(debug2);
   */ 
    //return(missing + 2*(raw > 0.25) + (raw > 0.5) + (raw > 0.75));
}

int coauthcmp(const void* cstr1, const void* cstr2, size_t size){
    char *delim="/";
    char *num_delim="~";
    char *coauths1, *coauths2, *c1, *c2, *c1_last, *c2_last;

    if(strncmp((char*)cstr1, "\0", 1)==0 || strncmp((char*)cstr2, "\0", 1)==0)
        return 0;

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
    //        printf("%s, %s\n", c1_last, c2_last);
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
