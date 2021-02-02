# contrib/pg_tsp_chinese/Makefile

MODULE_big = pg_tsp_chinese
OBJS = pg_tsp_chinese.o $(WIN32RES)

EXTENSION = pg_tsp_chinese
DATA = pg_tsp_chinese.sql
PGFILEDESC = "pg_tsp_chinese - a parser for full-text search of Chinese."

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_tsp_chinese
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
