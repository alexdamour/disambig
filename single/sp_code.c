#include "sqlite_db.h"

typedef enum _sp_field_num {SP_TYPE, SP_NAME, SP_FUNC, SP_EXTRACT_IDX, SP_GROUP} sp_field_num;
typedef enum _res_field_num {RES_TYPE, RES_LEVELS} res_field_num;
typedef enum _train_field_num {TR_GROUP, TR_MATCH_SETS, TR_NONMATCH_SETS} train_field_num;

typedef struct {
    char *type;
    char *levels;
} RESFIELD;

typedef struct {
    char *type;
    char *name;
    char *func;
    char *extract_idx;
    char *group;
} SPFIELD;

typedef struct {
    char *group;
    char *match_sets;
    char *nonmatch_sets;
} TRAINFIELD;

typedef struct{
    char *name;
} TSETFIELD;

typedef struct{
    char *name;
} XSETFIELD;

/*
 * Globals
 */
char    *progname;
FILE    *cfp; /* C source file being written. */
FILE    *hfp; /* C header file being written. */
FILE    *trcfp;/* Training C source file being written. */
FILE    *trhfp;/* Training C header file being written. */
char    *cfile="comp_spec.c"; /* C source filename with default. */
char    *hfile="comp_spec.h"; /* C header filename with default. */
char    *tr_hfile="train.h";
char    *tr_cfile="train.c";
int verbose;

int linenum = 0;

u_int   sp_field_cnt; /* Number of fields on final simprof. */
SPFIELD *spfields;   /* Holder for field properties. */
u_int sp_field_alloc; /* Number of fields actually allocated. */

u_int res_field_cnt;
RESFIELD *resfields;
u_int res_field_alloc;

u_int train_field_cnt;
TRAINFIELD *trainfields;
u_int train_field_alloc;

u_int tset_field_cnt;
TSETFIELD *tsetfields;
u_int tset_field_alloc;

u_int xset_field_cnt;
XSETFIELD *xsetfields;
u_int xset_field_alloc;

int parse_simprof(char*);
int parse_res_space(char*);
int parse_training(char*);
int parse_matchsets(char*);
int parse_nonmatchsets(char*);

int code_source(void);
int code_header_start(void);
int code_header(void);
int tr_code_header(void);
int tr_code_source(void);
int sp_desc_dump(void);
int sp_desc_load(void);
int usage(void);
void free_subfields(SPFIELD *);

int
main(int argc, char *argv[])
{
	int ch;

	/* Initialize globals. */
	if ((progname = strrchr(argv[0], '/')) == NULL)
		progname = argv[0];
	else
		++progname;

	/* Initialize arguments. */
    verbose = 0;

    if (argc < 2)
        return(usage());

	/* Process arguments. */
	while ((ch = getopt(argc, argv, "c:f:h:v")) != EOF){
		switch (ch) {
		case 'c':
			cfile = optarg;
			break;
		case 'f':
			if (freopen(optarg, "r", stdin) == NULL) {
				fprintf(stderr,
				    "%s: %s\n", optarg, strerror(errno));
				return (EXIT_FAILURE);
			}
			break;
		case 'h':
			hfile = optarg;
			break;
        case 'v':
			++verbose;
			break;
		case '?':
		default:
			return (usage());
        }
    }

	argc -= optind;
	argv += optind;

	if (*argv != NULL)
		return (usage());

    /* Open header file. Some lines will be copied directly to header
       file while loading the description file.*/
    if ((hfp = fopen(hfile, "w")) == NULL) {
		fprintf(stderr,
		    "%s: %s: %s\n", progname, hfile, strerror(errno));
		return (EXIT_FAILURE);
	}

    if (code_header_start())
        return(EXIT_FAILURE);

	/* Load records from the input file. */
	if (sp_desc_load())
		return (EXIT_FAILURE);

	/* Dump records for debugging. */
	if (verbose && sp_desc_dump())
		return (EXIT_FAILURE);

	/* Open source file. */
	if ((cfp = fopen(cfile, "w")) == NULL) {
		fprintf(stderr,
		    "%s: %s: %s\n", progname, cfile, strerror(errno));
		return (EXIT_FAILURE);
	}
		/* Build the source and header files. */
	if (code_header())
		return (EXIT_FAILURE);
	if (code_source())
		return (EXIT_FAILURE);

    fclose(hfp);
    fclose(cfp);

    if((trhfp = fopen(tr_hfile, "w")) == NULL){
        fprintf(stderr,
		    "%s: %s: %s\n", progname, tr_hfile, strerror(errno));
		return (EXIT_FAILURE);
    }

    if((trcfp = fopen(tr_cfile, "w")) == NULL){
        fprintf(stderr,
		    "%s: %s: %s\n", progname, tr_cfile, strerror(errno));
		return (EXIT_FAILURE);
	}

    if (tr_code_header())
        return(EXIT_FAILURE);
    if (tr_code_source())
        return(EXIT_FAILURE);

    fclose(trhfp);
    fclose(trcfp);
    
    //for(f = fields; field_cnt > 0; --field_cnt, ++f)
    //    free_subfields(f);

    //free(sp_fields);

	return (EXIT_SUCCESS);
}

int
sp_desc_load()
{
	int in_field,in_res_space,in_simprof,in_training,in_tset,in_xset;
	char *p, buf[512];

    linenum = 0;
    in_field = in_res_space = in_simprof = in_training = in_tset = in_xset = 0;
	sp_field_alloc =  res_field_alloc = train_field_alloc = 0;
    sp_field_cnt = res_field_cnt = train_field_cnt = xset_field_cnt = tset_field_cnt = 0;

	while (fgets(buf, sizeof(buf), stdin) != NULL) {
        ++linenum;
        //printf("%s", buf);
		if ((p = strchr(buf, '\n')) == NULL) {
			fprintf(stderr, "%s: input line too long\n", progname);
			return (1);
		}
		*p = '\0';

		/* Skip leading whitespace. */
		for (p = buf; isspace(*p); ++p)
			;

		/* Skip empty lines and lines beginning with '%'. */
		if ((in_field && *p == '\0') || *p == '%')
			continue;
        
        /*
           Skip lines beginning with '%', unless they are #define or #include.
           Other compiler directives should probably be added here.
        if (*p == '#'){
            if (strncasecmp(p, "#define", sizeof("#define") - 1) ||
                strncasecmp(p, "#include", sizeof("#include") - 1))
                continue;
        }
        */

		/* Check parser state. */
		if (!in_field) {
            if (strncasecmp(
                p, "res_space{", sizeof("res_space{") - 1) == 0){
                in_res_space = 1;
            }
			else if (strncasecmp(
			    p, "simprof{", sizeof("simprof{") - 1) == 0) {
				in_simprof = 1;
			}
            else if (strncasecmp(
                p, "training{", sizeof("training{") - 1) == 0) {
                in_training = 1;
            }
            else if (strncasecmp(
                p, "match_sets{", sizeof("match_sets{") - 1) == 0) {
                in_tset = 1;
            }
            else if (strncasecmp(
                p, "nonmatch_sets{", sizeof("nonmatch_sets{") - 1) == 0) {
                in_xset = 1;
            }

            if ((in_field = in_res_space + in_simprof + in_training + in_tset + in_xset))
                continue;
            else {
                fprintf(hfp, "%s\n", buf);
            }
		}

		/*
		 * On block close, continue. 
		 */
		else { // if (in_field)
            if(*p == '}'){
                in_res_space = in_simprof = in_training = in_tset = in_xset = in_field = 0;
                continue;
            }

            /* Allocate a new field structure as necessary. */
            if(in_simprof)
                parse_simprof(p);
            else if(in_res_space)
                parse_res_space(p);
            else if(in_training)
                parse_training(p);
            else if(in_tset)
                parse_matchsets(p);
            else if(in_xset)
                parse_nonmatchsets(p);
        }
	}
	if (ferror(stdin)) {
		fprintf(stderr, "%s: stdin: %s\n", progname, strerror(errno));
		return (1);
	}

	return (0);
}

int
parse_simprof(char *p){
    sp_field_num cur_field;
    char *t, save_ch;
    if (sp_field_cnt*sizeof(SPFIELD) == sp_field_alloc &&
        (spfields = realloc(spfields, sp_field_alloc += sizeof(SPFIELD)*10)) == NULL) {
        fprintf(stderr, "%s: %s\n", progname, strerror(errno));
        return (1);
    }

    for (cur_field = 0; cur_field <= SP_GROUP; ++cur_field) {
        /* Skip to the next field, if any. */
        for (; *p != '\0' && isspace(*p); ++p)
            ;

        if (*p == '\0' || *p == '%'){
            fprintf(stderr, "Simprof parse error: not enough fields, line %d. Expected %d, found only %d\n",
                    linenum, SP_EXTRACT_IDX+1, cur_field);
            return(EXIT_FAILURE);
        }

        /* Find the end of the field. */
        for (t = p; *t != '\0' && !isspace(*t) && *t != '('; ++t)
            ;
        save_ch = *t;
        *t = '\0';
        switch(cur_field){
            case SP_TYPE:
                spfields[sp_field_cnt].type = strdup(p);
                break;
            case SP_NAME:
                spfields[sp_field_cnt].name = strdup(p);
                break;
            case SP_FUNC:
                spfields[sp_field_cnt].func = strdup(p);
                break;
            case SP_EXTRACT_IDX:
                spfields[sp_field_cnt].extract_idx = strdup(p);
                break;
            case SP_GROUP:
                spfields[sp_field_cnt].group = strdup(p);
                break;
            default:
                /*NOTRUN*/
                fprintf(stderr, "SP: Switch weirdness when saving field properties.\n");
        }
        *t = save_ch;
        p = t;
    }
    ++sp_field_cnt;
    return 0;
}

int
parse_res_space(char *p){
    res_field_num cur_field;
    char *t, save_ch;
    if (res_field_cnt*sizeof(RESFIELD) == res_field_alloc &&
        (resfields = realloc(resfields, res_field_alloc += sizeof(RESFIELD)*10)) == NULL) {
        fprintf(stderr, "%s: %s\n", progname, strerror(errno));
        return (1);
    }

    for (cur_field = 0; cur_field <= RES_LEVELS; ++cur_field) {
        /* Skip to the next field, if any. */
        for (; *p != '\0' && isspace(*p); ++p)
            ;

        if (*p == '\0' || *p == '%'){
            fprintf(stderr, "Res_space parse error: not enough fields, line %d. Expected %d, found only %d\n",
                    linenum, RES_LEVELS+1, cur_field);
            return(EXIT_FAILURE);
        }

        /* Find the end of the field. */
        for (t = p; *t != '\0' && !isspace(*t) && *t != '('; ++t)
            ;
        save_ch = *t;
        *t = '\0';
        switch(cur_field){
            case RES_TYPE:
                resfields[res_field_cnt].type = strdup(p);
                break;
            case RES_LEVELS:
                resfields[res_field_cnt].levels = strdup(p);
                break;
            default:
                /*NOTRUN*/
                fprintf(stderr, "RES: Switch weirdness when saving field properties.\n");
        }
        *t = save_ch;
        p = t;
    }
    ++res_field_cnt;
    return 0;
}

int
parse_matchsets(char *p){
    u_int cur_field;
    char *t, save_ch;
    if (tset_field_cnt*sizeof(TSETFIELD) == tset_field_alloc &&
        (tsetfields = realloc(tsetfields, tset_field_alloc += sizeof(TSETFIELD)*10)) == NULL) {
        fprintf(stderr, "%s: %s\n", progname, strerror(errno));
        return (1);
    }

    for (cur_field = 0; cur_field <= 0; ++cur_field) {
        /* Skip to the next field, if any. */
        for (; *p != '\0' && isspace(*p); ++p)
            ;

        if (*p == '\0' || *p == '%'){
            fprintf(stderr, "Tset parse error: not enough fields, line %d. Expected %d, found only %d\n",
                    linenum, 0+1, cur_field);
            return(EXIT_FAILURE);
        }

        /* Find the end of the field. */
        for (t = p; *t != '\0' && !isspace(*t) && *t != '('; ++t)
            ;
        save_ch = *t;
        *t = '\0';
        switch(cur_field){
            case 0:
                tsetfields[tset_field_cnt].name = strdup(p);
                break;
            default:
                /*NOTRUN*/
                fprintf(stderr, "TSET: Switch weirdness when saving field properties.\n");
        }
        *t = save_ch;
        p = t;
    }
    ++tset_field_cnt;
    return 0;
}

int
parse_nonmatchsets(char *p){
    u_int cur_field;
    char *t, save_ch;
    if (xset_field_cnt*sizeof(XSETFIELD) == xset_field_alloc &&
        (xsetfields = realloc(xsetfields, xset_field_alloc += sizeof(XSETFIELD)*10)) == NULL) {
        fprintf(stderr, "%s: %s\n", progname, strerror(errno));
        return (1);
    }

    for (cur_field = 0; cur_field <= 0; ++cur_field) {
        /* Skip to the next field, if any. */
        for (; *p != '\0' && isspace(*p); ++p)
            ;

        if (*p == '\0' || *p == '%'){
            fprintf(stderr, "Tset parse error: not enough fields, line %d. Expected %d, found only %d\n",
                    linenum, 0+1, cur_field);
            return(EXIT_FAILURE);
        }

        /* Find the end of the field. */
        for (t = p; *t != '\0' && !isspace(*t) && *t != '('; ++t)
            ;
        save_ch = *t;
        *t = '\0';
        switch(cur_field){
            case 0:
                xsetfields[xset_field_cnt].name = strdup(p);
                break;
            default:
                /*NOTRUN*/
                fprintf(stderr, "XSET: Switch weirdness when saving field properties.\n");
        }
        *t = save_ch;
        p = t;
    }
    ++xset_field_cnt;
    return 0;
}

int
parse_training(char *p){
    res_field_num cur_field;
    char *t, save_ch;
    if (train_field_cnt*sizeof(TRAINFIELD) == train_field_alloc &&
        (trainfields = realloc(trainfields, train_field_alloc += sizeof(RESFIELD)*10)) == NULL) {
        fprintf(stderr, "%s: %s\n", progname, strerror(errno));
        return (1);
    }

    for (cur_field = 0; cur_field <= TR_NONMATCH_SETS; ++cur_field) {
        /* Skip to the next field, if any. */
        for (; *p != '\0' && isspace(*p); ++p)
            ;

        if (*p == '\0' || *p == '%'){
            fprintf(stderr, "Training parse error: not enough fields, line %d. Expected %d, found only %d\n",
                    linenum, TR_NONMATCH_SETS+1, cur_field);
            return(EXIT_FAILURE);
        }

        /* Find the end of the field. */
        for (t = p; *t != '\0' && !isspace(*t) && *t != '('; ++t)
            ;
        save_ch = *t;
        *t = '\0';
        switch(cur_field){
            case TR_GROUP:
                trainfields[train_field_cnt].group = strdup(p);
                break;
            case TR_MATCH_SETS:
                trainfields[train_field_cnt].match_sets = strdup(p);
                break;
            case TR_NONMATCH_SETS:
                trainfields[train_field_cnt].nonmatch_sets = strdup(p);
                break;
            default:
                /*NOTRUN*/
                fprintf(stderr, "TRAIN: Switch weirdness when saving field properties.\n");
        }
        *t = save_ch;
        p = t;
    }
    ++train_field_cnt;
    return 0;
}

int
tr_code_header(){
    TRAINFIELD *trf;
    u_int i;
    
    fprintf(trhfp, "/*\n");
	fprintf(trhfp, " *  DO NOT EDIT: automatically built by %s.\n", progname);
    fprintf(trhfp, " *\n");
    fprintf(trhfp, " * Specifications for indices of training counts.\n");
    fprintf(trhfp, " */\n\n");

    //fprintf(trhfp, "#include \"sqlite_db.h\"\n");
    //fprintf(trhfp, "#include \"sqlite_db_local.h\"\n");
    //fprintf(trhfp, "#include \"sqlite_db_extern.h\"\n");
    fprintf(trhfp, "#include \"comp_engine.h\"\n\n");

    for(i=0, trf=trainfields; i < train_field_cnt; ++i, ++trf)
        fprintf(trhfp, "extern int %s_idx_callback(DB*, const DBT*, const DBT*, DBT*);\n", trf->group);
    fprintf(trhfp, "\n");

    fprintf(trhfp, "#define NUM_IDXS %d\n", train_field_cnt);
    fprintf(trhfp, "extern char* idx_names[];\n\n");
    fprintf(trhfp, "extern int(*idx_funcs[])(DB*, const DBT*, const DBT*, DBT*);\n\n");

    fprintf(trhfp, "extern char* matchset_names[];\n");
    fprintf(trhfp, "extern char* nonmatchset_names[];");

        return 0;
}

int
tr_code_source(){
    SPFIELD *spf;
    TRAINFIELD *trf;
    TSETFIELD *tsf;
    XSETFIELD *xsf;
    u_int i,j,k,num_fields;
    
    fprintf(trcfp, "/*\n");
	fprintf(trcfp, " *  DO NOT EDIT: automatically built by %s.\n", progname);
    fprintf(trcfp, " *\n");
    fprintf(trcfp, " * Specifications for indices of training counts.\n");
    fprintf(trcfp, " */\n\n");

    fprintf(trcfp, "#include \"train.h\"\n\n");

    fprintf(trcfp, "char* matchset_names[] = {\n");
    for(i=0, tsf=tsetfields; i < tset_field_cnt; ++i, ++tsf)
        fprintf(trcfp,"\t\"%s\",\n", tsf->name);
    fprintf(trcfp, "\tNULL,\n};\n\n");

    fprintf(trcfp, "char *nonmatchset_names[] = {\n");
    for(i=0, xsf=xsetfields; i < xset_field_cnt; ++i, ++xsf)
        fprintf(trcfp, "\t\"%s\",\n", xsf->name);
    fprintf(trcfp, "\tNULL,\n};\n\n");

    fprintf(trcfp, "char *idx_names[] = {\n");
    for(i=0, trf=trainfields; i < train_field_cnt; ++i, ++trf)
        fprintf(trcfp, "\t\"%s\",\n", trf->group);
    fprintf(trcfp, "};\n\n");

    fprintf(trcfp, "int(*idx_funcs[])(DB*, const DBT*, const DBT*, DBT*) = {\n");
    for(i=0, trf=trainfields; i < train_field_cnt; ++i, ++trf){
        fprintf(trcfp, "\t%s_idx_callback,\n", trf->group);
    }
    fprintf(trcfp, "};\n\n");

    for(i=0, trf=trainfields; i < train_field_cnt; ++i, ++trf){
        fprintf(trcfp, "int %s_idx_callback(DB* sec, const DBT* key, const DBT* data, DBT* result){\n", trf->group);
        fprintf(trcfp, "\tsec = sec;\n");
        fprintf(trcfp, "\tdata = data;\n");
        num_fields = 0;

        for(j=0, spf=spfields; j < sp_field_cnt; ++j, ++spf){
            printf("%s, %s\n", spf->group, trf->group);
            if(strcmp(spf->group, trf->group)==0)
                ++num_fields;
        }
        fprintf(trcfp, "\tint* fields = malloc(sizeof(int)*%u);\n", num_fields);

        k = 0;
        for(j=0, spf=spfields; j < sp_field_cnt; ++j, ++spf)
            if(strcmp(spf->group, trf->group)==0)
                fprintf(trcfp, "\tfields[%u] = ((simprof*)key->data)->%s;\n", k++, spf->name);

        fprintf(trcfp, "\tresult->data = fields;\n");
        fprintf(trcfp, "\tresult->size = sizeof(int)*%u;\n", num_fields);
        fprintf(trcfp, "\tresult->flags = result->flags | DB_DBT_APPMALLOC;\n");
        fprintf(trcfp, "\treturn(0);\n");
    
        fprintf(trcfp, "}\n");
        fprintf(trcfp, "\n");
    }

    return 0;
}

int
usage(){
   fprintf(stderr,
        "usage: %s [-v] [-c source-file] [-f input] [-h header-file]\n",
        progname);
    exit(1); 
}

int
code_header_start(){
    fprintf(hfp, "/*\n");
	fprintf(hfp, " *  DO NOT EDIT: automatically built by %s.\n", progname);
    fprintf(hfp, " *\n");
    fprintf(hfp, " * Record structure.\n");
    fprintf(hfp, " */\n");
    fprintf(hfp, "\n");

    fprintf(hfp, "#include \"sqlite_db.h\"\n");
    fprintf(hfp, "#include \"sqlite_db_local.h\"\n");
    fprintf(hfp, "#include \"sqlite_db_extern.h\"\n");

    fprintf(hfp, "#define SP_OFFSET(field) ((size_t)(&(((simprof*)0)->field)))\n\n");

    fprintf(hfp, "/* Begin custom header from sp.desc file. */\n");
    return 0;
}

int
code_header(){
    RESFIELD *rf;
    SPFIELD *spf;
    u_int i;

    fprintf(hfp, "/* End custom header from sp.desc file. */\n\n");

    fprintf(hfp, "/* Define result space levels. */\n");
    for(rf = resfields, i=0; i < res_field_cnt; ++i, ++rf)
        fprintf(hfp, "typedef enum {%s} %s;\n", rf->levels, rf->type);
    fprintf(hfp, "\n");

    fprintf(hfp, "/* Define the simprof data structure.*/\n");
	fprintf(hfp, "typedef struct __simprof {\n");
    for(spf = spfields, i=0; i < sp_field_cnt; ++i, ++spf)
        fprintf(hfp, "\t%s %s;\n", spf->type, spf->name);
    fprintf(hfp, "} simprof;\n");
    fprintf(hfp, "\n");

    fprintf(hfp, "/* Comparison specific globals. */\n");
    fprintf(hfp, "extern int (*comp_funcs[])(const void*, const void*, size_t);\n");
    fprintf(hfp, "extern int extract_idxs[];\n");
    fprintf(hfp, "extern size_t sp_offsets[];\n");
    fprintf(hfp, "\n");

    fprintf(hfp, "/* Custom function prototypes. */\n");
    fprintf(hfp, "/* Comparion environment functions. */\n");
    fprintf(hfp, "int comp_env_init(void);");
    fprintf(hfp, "int comp_env_clean(void);");
    fprintf(hfp, "/* Extractor function. */\n");
    fprintf(hfp, "int extract(DbRecord*, const int, void**, size_t*, int*, size_t*, size_t*);\n");
    fprintf(hfp, "/* Comparison functions. */\n");
    for(spf = spfields, i=0; i < sp_field_cnt; ++i, ++spf)
        fprintf(hfp, "int %s(const void*, const void*, size_t);\n", spf->func);
    fprintf(hfp, "\n");

    fprintf(hfp, "#define NUM_COMPS\t%d\n", sp_field_cnt);

    return 0;
}


int
code_source() {
    SPFIELD *f;
	u_int i;

	fprintf(cfp, "/*\n");
	fprintf(cfp,
	   " *  DO NOT EDIT: automatically built by %s.\n", progname);
    fprintf(cfp, " * Initialized arrays for loading custom comparison\n");
    fprintf(cfp, " * and extractor functions.\n");
	fprintf(cfp, " */\n\n");

    fprintf(cfp, "#include \"%s\"\n", hfile);
    fprintf(cfp, "\n");

    fprintf(cfp, "/* Comparison function array. */\n");
    fprintf(cfp, "int (*comp_funcs[])(const void*, const void*, size_t) = {\n");
    for(f = spfields, i=0; i < sp_field_cnt; ++i, ++f)
        fprintf(cfp, "\t%s,\n", f->func);
    fprintf(cfp, "};\n\n");

    fprintf(cfp, "/* Extractor index array. */\n");
    fprintf(cfp, "int extract_idxs[] = {\n");
    for(f = spfields, i=0; i < sp_field_cnt; ++i, ++f)
        fprintf(cfp, "\t%s, \n", f->extract_idx);
    fprintf(cfp, "};\n\n");

    fprintf(cfp, "/* Simprof offset array. */\n");
    fprintf(cfp, "size_t sp_offsets[] = {\n");
    for(f = spfields, i=0; i < sp_field_cnt; ++i, ++f)
        fprintf(cfp, "\tSP_OFFSET(%s), \n", f->name);
    fprintf(cfp, "};\n\n");

    return 0;
}

/*
 * For debugging, print out all of the elements of the field.
 */
int
sp_desc_dump()
{
    SPFIELD *f;
    u_int i;
    
    for (f = spfields, i=0; i < sp_field_cnt; ++i, ++f){
        printf("field %s{\n", f->name);
        printf("\ttype: %s\n", f->type);
        printf("\tfunc: %s\n", f->func);
        printf("\textract_idx: %s\n", f->extract_idx);
        printf("}\n");
    }
    return 0;
}
/*
 * Free memory allocated by strdup.
 */
void
free_subfields(SPFIELD *s){
    free(s->type);
    free(s->name);
    free(s->func);
    free(s->extract_idx);
}
