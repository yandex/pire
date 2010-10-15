
AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif

noinst_PROGRAMS = bench

bench_SOURCES  = bench.cpp
bench_LDADD    = ../../pire/libpire.la
bench_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS)
