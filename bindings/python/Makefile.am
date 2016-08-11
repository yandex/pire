SUBDIRS = . tests

RENDER_PY = $(srcdir)/render.py

MAKO_SOURCES = $(srcdir)/pire.pyx.mako $(srcdir)/impl.pxd.mako
GENERATED_CYTHON_SOURCES = $(srcdir)/pire.pyx $(srcdir)/impl.pxd
CYTHON_SOURCES = $(GENERATED_CYTHON_SOURCES) $(srcdir)/stub.pxd
CXX_SOURCES = $(srcdir)/pire_wrap.cpp

MAINTAINERCLEANFILES = $(CXX_SOURCES) pire_wrap.h $(GENERATED_CYTHON_SOURCES)

pkgpython_PYTHON =
pkgpyexec_LTLIBRARIES =

if ENABLE_PYTHON

pkgpyexec_LTLIBRARIES += pire.la
pire_la_SOURCES = $(CXX_SOURCES) $(CYTHON_SOURCES) $(MAKO_SOURCES) $(RENDER_PY)
pire_la_CPPFLAGS = $(PYTHON_CPPFLAGS) -I$(top_srcdir) $(AM_CXXFLAGS)
pire_la_LDFLAGS = -module
pire_la_LIBADD = ../../pire/libpire.la

endif

%.pyx: %.pyx.mako $(RENDER_PY)
	$(PYTHON) $(RENDER_PY) -i $< -o $@
%.pxd: %.pxd.mako $(RENDER_PY)
	$(PYTHON) $(RENDER_PY) -i $< -o $@

CYTHONFLAGS=

$(srcdir)/pire_wrap.cpp: $(CYTHON_SOURCES)
	$(CYTHON) -2 --cplus $(CYTHONFLAGS) -I$(top_srcdir) -o $@ $(srcdir)/pire.pyx
