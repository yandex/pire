AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif

lib_LTLIBRARIES = libpire_unit.la
libpire_unit_la_SOURCES = \
	stub/cppunit.cpp \
	stub/cppunit.h
libpire_unit_la_CXXFLAGS = -I$(top_srcdir)/pire

check_PROGRAMS = pire_test

pire_test_SOURCES = \
	common.h \
	pire_ut.cpp \
	easy_ut.cpp

if ENABLE_EXTRA
pire_test_SOURCES += \
	capture_ut.cpp \
	count_ut.cpp \
	glyph_ut.cpp
endif

nodist_pire_test_SOURCES = inline_ut_2.cpp

EXTRA_DIST = inline_ut.cpp pire_test_valgrind.sh

pire_test_LDADD = ../pire/libpire.la libpire_unit.la
pire_test_CXXFLAGS = -I$(top_srcdir)/pire $(AM_CXXFLAGS)

TESTS = pire_test

check_PROGRAMS += pire_test_valgrind
pire_test_valgrind_SOURCES = valgrind_ut.cpp
pire_test_valgrind_LDADD = ../pire/libpire.la libpire_unit.la
pire_test_valgrind_CXXFLAGS = -I$(top_srcdir)/pire $(AM_CXXFLAGS)
TESTS += pire_test_valgrind

if HAVE_VALGRIND
TESTS += pire_test_valgrind.sh
endif

CLEANFILES = inline_ut_2.cpp

inline_ut_2.cpp: $(srcdir)/inline_ut.cpp
	../pire/pire_inline -o inline_ut_2.cpp $(srcdir)/inline_ut.cpp
