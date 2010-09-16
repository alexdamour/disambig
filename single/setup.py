from distutils.core import setup, Extension

DB_INCLUDE = '/usr/local/bdb/include'
DB_LIBS = '/usr/local/bdb/lib'

module1 = Extension('disambig_db',
                    sources = ['py_quadprog.c'],
                    extra_objects=['sqlite_db_local.o','db.o','util.o','train.o','train_main.o','compfun.o','comp_env.o','extractfun.o','comp_engine.o','comp_spec.o','strcmp95.o','indexfun.o'],

                    include_dirs = ['.',
                        DB_INCLUDE,
                        '/usr/include'],
                    library_dirs = [DB_LIBS],
                    libraries = ['db', 'pthread', 'sqlite3', 'm'])

setup (name = 'disambig_db',
        version = '0.1',
        description = 'Bridge functionality for disambiguation.',
        ext_modules = [module1])
