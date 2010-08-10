# $Id: Makefile,v 1.14 2006/10/27 00:56:44 bostic Exp $

# Berkeley DB installation.
DB_INCLUDE=/usr/local/bdb/include
DB_LIBS=/usr/local/bdb/lib
LIBS=	-L$(DB_LIBS) -ldb -lpthread -lsqlite3 -lm

INC=	-I. -I/usr/include -I$(DB_INCLUDE)
CFLAGS=	$(INC) -g -W -Wall -Wpointer-arith -Wmissing-prototypes -ffloat-store -fPIC

PROGS=	sqlite_db_code sqlite_db_load comp_engine train clear
SRCS=	sqlite_db_code.c sqlite_db_local.c db.c util.c \
	load.c load_main.c \
    blockfun.c \
    indexfun.c \
    sp_code.c \
    extractfun.c \
    compfun.c comp_spec.c comp_env.c comp_engine.c comp_engine_main.c\
    strcmp95.c \
    train_main.c train.c\
    likelihood.c\
    asg_freq_hash_load.c\
    dump_primary.c\
    clear_tags.c\
    #DbRecord.c 

all: sqlite_db_load comp_engine train dump clear python #sqlite_db_load_multi #multi_test
 #likelihood
 #db_block csv_query

sqlite_db_code: sqlite_db_code.o
	$(CC) -o $@ $? $(LIBS)

LOAD_OBJS=sqlite_db_local.o blockfun.o indexfun.o db.o load.o load_main.o util.o
#DbRecord.o
sqlite_db_load: $(LOAD_OBJS)
	$(CC) -o $@ $(LOAD_OBJS) $(LIBS)

SHRINK_OBJS=sqlite_db_local.o db.o util.o blockfun.o shrink.o
shrink: $(SHRINK_OBJS)
	$(CC) -o $@ $(SHRINK_OBJS) $(LIBS)

QUEUE_OBJS=sqlite_db_local.o blockfun.o indexfun.o db.o load.o util.o build_queue.o build_queue_main.o
build_queue: $(QUEUE_OBJS)
	$(CC) -o $@ $(QUEUE_OBJS) $(LIBS)

LOAD_OBJS_MULTI=sqlite_db_local.o blockfun.o indexfun.o db.o load_multi.o load_main_multi.o util.o
sqlite_db_load_multi: $(LOAD_OBJS_MULTI)
	$(CC) -o $@ $(LOAD_OBJS_MULTI) $(LIBS)

SP_OBJS=sqlite_db_local.o util.o blockfun.o indexfun.o db.o load.o
sp_code: sp_code.o
	$(CC) -o $@ $? $(LIBS)

COMP_OBJS=sqlite_db_local.o compfun.o extractfun.o comp_spec.o comp_env.o comp_engine.o comp_engine_main.o likelihood.o blockfun.o db.o util.o strcmp95.o tagfun.o stopfun.o
comp_engine: $(COMP_OBJS) sqlite_db_load 
	$(CC) -o $@ $(COMP_OBJS) $(LIBS)

TRAIN_OBJS=sqlite_db_local.o db.o util.o train.o train_main.o compfun.o comp_env.o extractfun.o comp_engine.o comp_spec.o strcmp95.o indexfun.o
train: $(TRAIN_OBJS)
	$(CC) -o $@ $(TRAIN_OBJS) $(LIBS)

DUMP_OBJS=sqlite_db_local.o db.o util.o dump_primary.o blockfun.o tagfun.o indexfun.o
dump: $(DUMP_OBJS)
	$(CC) -o $@ $(DUMP_OBJS) $(LIBS)

ASG_OBJS =sqlite_db_local.o db.o util.o asg_freq_hash_load.o
asg: $(ASG_OBJS)
	$(CC) -o $@ $(ASG_OBJS) $(LIBS)

CLEAR_OBJS=sqlite_db_local.o db.o util.o clear_tags.o tagfun.o
clear: $(CLEAR_OBJS)
	$(CC) -o $@ $(CLEAR_OBJS) $(LIBS)

COPY_STAT_OBJS=sqlite_db_local.o db.o util.o tagfun.o copy_stat_db.o
copy_stat: $(COPY_STAT_OBJS)
	$(CC) -o $@ $(COPY_STAT_OBJS) $(LIBS)

clean distclean realclean:
	rm -rf $(PROGS) TESTDIR eBay tags *.o *.core sqlite_db_local.[ch] build #primary block_idx idx simprof tset0* xset0*

primaryclean:
	rm -rf primary block_idx idx

compclean:
	rm -rf first_idx second_idx simprof lik_db __*

tsetclean:
	rm tset* xset*

ratioclean:
	rm ratios

asgclean:
	rm asg_freq_hash

backup:
	cp -r *.c *.h *.py build Makefile ../multi 

python: py_quadprog.c train.h train.c train.o extractfun.o
	python setup.py build
	sudo python setup.py install 

tags:
	rm -f tags
	ctags $(SRCS) code.c

sqlite_db_local.o load.o load_main.o train.o: sqlite_db_local.h sqlite_db.h
#DbRecord.o 

db.o: blockfun.o indexfun.o

train.o: comp_engine.o comp_spec.o db.o sqlite_db.h

query_main.o util.o blockfun.o create_secondary.o: sqlite_db_local.h sqlite_db.h

comp_engine_main.o comp_engine.o comp_spec.o: sqlite_db_local.h sqlite_db.h strcmp95.h comp_spec.h likelihood.o tagfun.o stopfun.o comp_spec.c comp_spec.h

likelihood.o: sqlite_db_local.h sqlite_db.h strcmp95.h comp_spec.h tagfun.o stopfun.o comp_spec.c comp_spec.h

sqlite_db_local.c sqlite_db_local.h: sqlite_db_code data.desc
	./sqlite_db_code -v -c sqlite_db_local.c -h sqlite_db_local.h -f data.desc

comp_spec.c comp_spec.h compfun.c train.h: sp_code sp.desc
	./sp_code -v -f sp.desc

lint_code:
	flexelint +fll \
	    "-e801" \
	    "-e818" \
	    "-esym(534,fprintf)" \
	    "-esym(534,memcpy)" \
	    "-esym(534,memmove)" \
	    "-esym(534,memset)" \
	    "-esym(534,printf)" \
	    "-wlib(1)" \
	    -i$(DB_INCLUDE) "-i/usr/include" \
	    code.c
lint_load:
	flexelint +fll \
	    "-e801" \
	    "-e818" \
	    "-esym(534,fprintf)" \
	    "-esym(534,memcpy)" \
	    "-esym(534,memmove)" \
	    "-esym(534,memset)" \
	    "-esym(534,printf)" \
	    "-wlib(1)" \
	    -i$(DB_INCLUDE)  "-i/usr/include" \
	    DbRecord.c csv_local.c db.c load.c load_main.c util.c
lint_query:
	flexelint +fll \
	    "-e801" \
	    "-e818" \
	    "-esym(534,fprintf)" \
	    "-esym(534,memcpy)" \
	    "-esym(534,memmove)" \
	    "-esym(534,memset)" \
	    "-esym(534,printf)" \
	    "-wlib(1)" \
	    -i$(DB_INCLUDE)  "-i/usr/include" \
	    DbRecord.c csv_local.c db.c query.c query_main.c util.c
