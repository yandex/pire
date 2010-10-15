
AM_CXXFLAGS = -Wall
if ENABLE_DEBUG
AM_CXXFLAGS += -DPIRE_DEBUG
endif
if ENABLE_CHECKED
AM_CXXFLAGS += -DPIRE_CHECKED
endif

bin_PROGRAMS = pigrep

pigrep_SOURCES  = pigrep.cpp
pigrep_LDADD    = ../../pire/libpire.la
pigrep_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS)
