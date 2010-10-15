
AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif

noinst_PROGRAMS = blacklist

blacklist_SOURCES  = blacklist.cpp
blacklist_LDADD    = ../../pire/libpire.la
blacklist_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS)
