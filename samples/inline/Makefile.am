
AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif

noinst_PROGRAMS = validator

nodist_validator_SOURCES = validator_2.cpp
validator_LDADD    = ../../pire/libpire.la
validator_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS)

EXTRA_DIST = validator.cpp
CLEANFILES = validator_2.cpp

validator_2.cpp: $(srcdir)/validator.cpp
	../../pire/pire_inline -o validator_2.cpp $(srcdir)/validator.cpp
