%define pkgname libpire

Name: libpire
Version: 0.0.4
Release: my1

Summary: Perl Incompatible Regular Expressions library
License: LGPLv3+
Group: System/Libraries
Url: http://wiki.yandex-team.ru/DmitrijjProkopcev/pire
Packager: Dmitry Prokoptsev <dprokoptsev@yandex-team.ru>

Source: pire-0.0.4.tar.gz

Provides: libpire = %version-%release

BuildPreReq: rpm-build >= 4.0.0
BuildRequires: gcc-c++ >= 3.0.0, libstdc++-devel

%description
An extremely fast (but with limited functionality) regular expressions
implementation library, aimed at quickly matching input text against
a huge amount of regular expression patterns in a nearly-realtime tasks.

%package devel
Summary: Development environment for libpire
Group: Development/C++
Provides: %name-devel = %version-%release
Requires: %name = %version-%release

%description devel
This package contains the headers of %pkgname, which are needed when
developing or compiling application which use %pkgname.

%package devel-static
Summary: Static version of %pkgname
Group: Development/C++
Provides: %name-devel-static = %version-%release
Requires: %name-devel = %version-%release

%description devel-static
This package contains static libraries for building statically linked
programs which use %pkgname.

%prep
%setup -q -n pire-%version

%build
%configure
%make_build

%install
%makeinstall

%files
%_libdir/libpire.so*

%files devel
%_includedir/pire/
%_bindir/*

%files devel-static
%_libdir/libpire.a

%changelog
* Wed Nov 17 2010 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru> 0.0.4-my1
- Implemented SSE2-bases shortcutting, resulting
  in dramatic performance boost on simple regexps;
- Fixed a few build issues;
- Made Pire work on ia64 and armel.

* Thu Nov 11 2010 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru> 0.0.3-my1
- Lots of performance optimizations.

* Sat Jul 10 2010 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru> 0.0.2-my1
- Slashes no longer require to be escaped.
- Implemented Fsm::Reverse().
- Scan() now searches longest acceptable prefix.
- Optimized compiling of a union of a huge amount of relatively small patterns.

* Fri Jul 02 2010 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru> 0.0.1-my1
- Initial revision

