#include "comp_spec.h"
#include "strcmp95.h"

/*
 * Globals
 */

DbField fieldlist[];
int *jw_opts[] = {0,0};

int extract(DbRecord *rp, const int func_idx, void **result, size_t* size){
    int extract_malloc=0;
    if(func_idx < SQLITE_DB_NUMFIELDS){ 
        *result = (char*)rp + fieldlist[func_idx-1].offset;
        *size = fieldlist[func_idx-1].size;
    }
    else if(func_idx == (int)LATLON){
        extract_malloc=1;
        *result = malloc(sizeof(latlon));
        ((latlon*)*result)->lat = rp->Lat;
        ((latlon*)*result)->lon = rp->Lon;
        *size = sizeof(latlon);
    }
    else if(func_idx == (int)ASG_FIELDS){
        extract_malloc=1;
        *result = malloc(sizeof(asg_struct));
        ((asg_struct*)*result)->asgname = rp->Assignee;
        ((asg_struct*)*result)->asgnum = rp->AsgNum;
        *size = sizeof(asg_struct);
    }
    return(extract_malloc);
}

int jwcmp(const void* str1, const void* str2, size_t size){
//    printf("JWCOMP:\n");
    int res;
    double myres = 0;
    char *s1 = (char*)str1, *s2 = (char*)str2;
    myres = (s1 == NULL || s2 == NULL) ? 0 : strcmp95(s1, s2, MAX(strlen(s1)+1, strlen(s2)+1), jw_opts);

    res = (s1 == NULL || s2 == NULL || strlen(s1)*strlen(s2)==0) + 2*(myres >= 0.75) + (myres >= 0.85) + (myres >= 0.95) + (myres >= 0.9999);
    return(res);
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

    n1 = n1_last = name1;
    //Pull off first initial because we don't care anymore
    n1_last = strsep(&n1, delim);

    while((n1_last=strsep(&n1, delim)) != NULL){
        num_names_2=0;
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


    missing = (MIN(num_names_1, num_names_2)==0);
    raw = missing ? 0 : (double)matches/(double)(MIN(num_names_1, num_names_2));

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
    int missing = ((ll1->lat==0 && ll1->lon==0) || (ll2->lat==0 && ll2->lon==0));
    size = size;

//    printf("\targs: %f, %f ; %f, %f\n", ll1->lat, ll1->lon, ll2->lat, ll2->lon);
    dist = missing ? 101 : (3963.0 * acos(sin(ll1->lat/57.2958) * sin(ll2->lat/57.2958) + cos(ll1->lat/57.2958) * cos(ll2->lat/57.2958) *  cos(ll2->lon/57.2958 -ll1->lon/57.2958)));
//    printf("\traw: %f\n", dist);

    return(missing + 2*(dist < 100) + (dist < 75) + (dist < 50) + (dist < 10) + (dist < 1));
}

int disttypecmp(const void* dt1, const void* dt2, size_t size){
    return( ((!strncmp((char*)dt1, "US", 2)) && (!strncmp((char*)dt2, "US", 2))) ? HAVE_STREET : NO_STREET );
}

int asgcmp(const void* asg1, const void* asg2, size_t size){
    if(((asg_struct*)asg1)->asgnum != 0 || ((asg_struct*)asg2)->asgnum != 0)
        if(((asg_struct*)asg1)->asgnum == 0 || ((asg_struct*)asg2)->asgnum == 0 ||
            (((asg_struct*)asg1)->asgnum) != (((asg_struct*)asg2)->asgnum)){
            //printf("%lu:%s :: %lu:%s\n", (ulong)((asg_struct*)asg1)->asgnum, ((asg_struct*)asg1)->asgname,
            //                             (ulong)((asg_struct*)asg2)->asgnum, ((asg_struct*)asg2)->asgname);
            return JWSUB75;
        }
        else
            return JW100;
    else
        return(jwcmp(((asg_struct*)asg1)->asgname, ((asg_struct*)asg2)->asgname, fieldlist[SQLITE_DB_INDX_ASSIGNEE-1].size));
}
    

int classcmp(const void* cstr1, const void* cstr2, size_t size){
    //printf("CLASSCOMP:\n");
    char *delim="/";
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
        nc2=-1;
        c2 = c2_last = class2;
        while((c2_last=strsep(&c2, delim)) != NULL){
        //    printf("\t\t%s, %s\n", c1_last, c2_last);
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
    char *coauths1, *coauths2, *c1, *c2, *c1_last, *c2_last;
    coauths1 = (char*)malloc(size);
    coauths2 = (char*)malloc(size);
    memcpy(coauths1, (char*)cstr1, size);
    memcpy(coauths2, (char*)cstr2, size);
    int matches=0;

    c1 = c1_last = coauths1;
    c2 = c2_last = coauths2;

    while((c1_last=strsep(&c1, delim)) != NULL){
        c2 = c2_last = coauths2;
        while((c2_last=strsep(&c2, delim)) != NULL){
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
