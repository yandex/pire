
AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif
if ENABLE_EXTRA
AM_CXXFLAGS += -DBENCH_EXTRA_ENABLED
endif

noinst_PROGRAMS = bench
dist_noinst_SCRIPTS = run-bench
dist_noinst_DATA = test_file

bench_SOURCES  = bench.cpp ../common/filemap.h
bench_LDADD    = ../../pire/libpire.la
bench_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS)
