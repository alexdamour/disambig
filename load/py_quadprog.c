#include <Python.h>
#include "train.h"

/*
 * Globals
 */

DB_ENV *dbenv;
int verbose;
char *progname;

//From comp_spec.h
size_t sp_offsets[];
int num_comps;

static PyObject *raw_count_table(PyObject *, PyObject *);
static PyObject *write_ratio_db(PyObject *, PyObject *);

static PyMethodDef DisambigDBMethods[] = {
    {"rawCountTable", raw_count_table, METH_VARARGS, "Retrieve Berkeley DB as a dictionary."},
    {"writeRatioDB", write_ratio_db, METH_VARARGS, "Write ratio dictionary to Berkeley DB."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdisambig_db(void){
    (void) Py_InitModule("disambig_db", DisambigDBMethods);
}

static PyObject *
raw_count_table(PyObject *self, PyObject *args){
    DB *tset_db, *sdb;
    DBC *cursor;
    DBT key, pkey, data;
    char *tset_name, *sdb_name;
    int func_num;
    int key_elts, i, numkeys;
    u_int32_t count;
    //DB_BTREE_STAT *stat;
    
    PyObject *key_tuple, *count_dict;
    
    if(!PyArg_ParseTuple(args, "ssi", &tset_name, &sdb_name, &func_num))
        return NULL;

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&tset_db, tset_name, DB_BTREE, 4*1024, 0, 0, NULL); 
    if (0 != sqlite_db_secondary_open(tset_db, &sdb, sdb_name, 4*1024, DB_DUPSORT, idx_funcs[func_num], NULL)) printf("PROBLEM!\n");
/*
    sdb->stat(sdb, NULL, &stat, 0);
    printf("numkeys: %lu\n", (u_long)(stat->bt_nkeys));
    free(stat);
*/
    sdb->cursor(sdb, NULL, &cursor, 0);

    count_dict = PyDict_New();

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    memset(&pkey, 0, sizeof(pkey));
    
    numkeys = 0;
    while(DB_NOTFOUND != cursor->pget(cursor, &key, &pkey, &data, DB_NEXT_NODUP)){
        ++numkeys;
        count = 0;
        key_elts = key.size/sizeof(u_int32_t);
        key_tuple = PyTuple_New((Py_ssize_t)key_elts); 
        for(i=0; i<key_elts; i++){
            PyTuple_SetItem(key_tuple, (Py_ssize_t)i, Py_BuildValue("i", ((u_int32_t*)key.data)[i]));
        }
        do{
            count += *(u_int32_t*)data.data;
        }while(DB_NOTFOUND != cursor->pget(cursor, &key, &pkey, &data, DB_NEXT_DUP));
        PyDict_SetItem(count_dict, key_tuple, Py_BuildValue("k", count));
    }

    cursor->close(cursor);
    sdb->close(sdb, 0);
    tset_db->close(tset_db, 0);
    sqlite_db_env_close();

    return(count_dict);
}

static PyObject *
write_ratio_db(PyObject *self, PyObject *args){
    PyObject *ratio_dict;
    PyObject *dict_key, *dict_value;
    DBT key, data;
    DB *rdb;
    double ratio;
    simprof sp;
    Py_ssize_t ppos;
    int i;
    DB_BTREE_STAT *stat;

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    if(!PyArg_ParseTuple(args, "O", &ratio_dict))
        return NULL;
    Py_INCREF(ratio_dict);

    printf("dict_check: %d\n", PyDict_Check(ratio_dict));
    printf("dict_size: %d\n", (int)PyDict_Size(ratio_dict));

    sqlite_db_env_open(NULL);
    sqlite_db_primary_open(&rdb, "ratios", DB_BTREE, 4*1024, DB_CREATE, 0, NULL);
    printf("opened db!\n");

    ppos = 0;
    while(PyDict_Next(ratio_dict, &ppos, &dict_key, &dict_value)){
        for(i=0; i < NUM_COMPS; ++i){
           *(int*)(((char*)&sp)+sp_offsets[i]) = (int)PyInt_AsLong(PyTuple_GetItem(dict_key, (Py_ssize_t)i));
        }
        ratio = PyFloat_AsDouble(dict_value);

        key.data = &sp;
        key.size = sizeof(sp);
        data.data = &ratio;
        data.size = sizeof(double);

        rdb->put(rdb, NULL, &key, &data, 0);
    }

    rdb->stat(rdb, NULL, &stat, 0);
    //printf("numkeys: %lu\n", (u_long)stat->bt_nkeys);
    free(stat);

    rdb->close(rdb, 0);
    sqlite_db_env_close();

    Py_DECREF(ratio_dict);
    Py_INCREF(Py_None);
    return Py_None;
}
