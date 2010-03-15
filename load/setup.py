from distutils.core import setup, Extension

DB_INCLUDE = '/usr/local/bdb/include'
DB_LIBS = '/usr/local/bdb/lib'

module1 = Extension('disambig_db',
                    sources = ['py_quadprog.c'],
                    extra_objects = ['db.o','util.o','sqlite_db_local.o',
                        'compfun.o','comp_spec.o','comp_engine.o','strcmp95.o','train.o'],
                    include_dirs = ['.',
                        DB_INCLUDE,
                        '/usr/include'],
                    library_dirs = [DB_LIBS],
                    libraries = ['db', 'pthread', 'sqlite3', 'm'])

setup (name = 'disambig_db',
        version = '0.1',
        description = 'Bridge functionality for disambiguation.',
        ext_modules = [module1])
