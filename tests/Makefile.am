if WITH_UNITTESTS

AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif

check_PROGRAMS = pire_test

pire_test_SOURCES = \
	stub/cppunit.cpp \
	stub/cppunit.h \
	common.h \
	pire_ut.cpp

if ENABLE_EXTRA
pire_test_SOURCES += \
	capture_ut.cpp \
	count_ut.cpp \
	glyph_ut.cpp
endif

nodist_pire_test_SOURCES = inline_ut_2.cpp

EXTRA_DIST = inline_ut.cpp

pire_test_LDADD = ../pire/libpire.la
pire_test_CXXFLAGS = -I$(top_srcdir)/pire $(CPPUNIT_CFLAGS) $(AM_CXXFLAGS)
pire_test_LDFLAGS  = $(CPPUNIT_LIBS)

TESTS = pire_test

CLEANFILES = inline_ut_2.cpp

inline_ut_2.cpp: inline_ut.cpp
	../pire/pire_inline -o $@ $<

endif

