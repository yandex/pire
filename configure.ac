AC_PREREQ([2.60])
AC_INIT([pire], [0.0.5], [dprokoptsev@yandex-team.ru])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([pire/classes.cpp])
AC_CONFIG_HEADERS([config.h.in])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG_CPLUSPLUS

# Require neccessary binaries to build ourselves
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_LIBTOOL

# Work around autoconf's insanity of defining YACC to nonexistent 'yacc'
# if no yacc-like generator was found
if ! test -r "$srcdir"/pire/re_parser.cpp && test x"$YACC" = "xyacc"; then
	AC_CHECK_PROG([YACC_EXISTS], [yacc], [yes], [no])
    if test x"$YACC_EXISTS" != xyes; then
		AC_MSG_ERROR([[bison/byacc/yacc not found (and no prebuilt parser used)]])
	fi
fi

# The same thing for lex
if ! test -r "$srcdir"/pire/inline.cpp && test x"$LEX" = "x:"; then
	AC_MSG_ERROR([[flex/lex not found (and no prebuilt pire-inline lexer used)]])
fi

# Just for conscience' sake
AC_CHECK_HEADERS([stdlib.h stdint.h string.h sys/time.h])
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strchr])

AC_C_BIGENDIAN

# Utility check routine combining AC_TRY_COMPILE, AC_CACHE_CHECK and AC_DEFINE.
AC_DEFUN([AX_DEFINE_IF_COMPILES], [
	pire_saved_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -Wall -Wextra -Werror"
	AC_CACHE_CHECK([[whether $2]], [pire_cv_$1], AC_TRY_COMPILE([], [$3], [pire_cv_$1=yes], [pire_cv_$1=no]))
	CXXFLAGS="$pire_saved_CXXFLAGS"
	if test x[$]pire_cv_$1 = xyes; then
		AC_DEFINE([$1], 1, [Define to 1 if $2])
	fi
])


# Alignment check
AC_CHECK_ALIGNOF(size_t)
AC_CHECK_ALIGNOF(uint64_t)

AX_DEFINE_IF_COMPILES([HAVE_ALIGNAS], [C++11 alignas is supported], [[
	struct X { void* p; };
	alignas(X) static const char x[] = "foo";
	return x[0] == x[1];
]])

AX_DEFINE_IF_COMPILES([HAVE_ATTR_ALIGNED], [__attribute__((aligned)) is supported], [[
	static const char x[] __attribute__((aligned(sizeof(void*)))) = "foo";
	return x[0] == x[1];
]])

# Scoped init
AX_DEFINE_IF_COMPILES([HAVE_LAMBDAS], [C++11 lambdas are supported], [[
	return ([](int x) -> int { return x - 1; })(1)
]])

AX_DEFINE_IF_COMPILES([HAVE_SCOPED_EXPR], [gcc-specific scoped expressions are supported], [[
	return ({ int a = 1; int b = 1; a - b; });
]])


# Optional features
AC_ARG_ENABLE([extra], AS_HELP_STRING([--enable-extra], [Add extra functionality (capturing scanner, etc...)]))
AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Make Pire dump all constructed FSMs to std::clog (useless unless debugging Pire)]))
AC_ARG_ENABLE([checked], AS_HELP_STRING([--enable-checked], [Enable asserts]))
AM_CONDITIONAL([ENABLE_EXTRA], [test x"$enable_extra" = xyes])
AM_CONDITIONAL([ENABLE_DEBUG], [test x"$enable_debug" = xyes])
AM_CONDITIONAL([ENABLE_CHECKED], [test x"$enable_checked" = xyes])

AC_ARG_ENABLE([valgrind_safe], AS_HELP_STRING([--enable-valgrind-safe], [Make Pire fetch data in a way which does not upset Valgrind]))
if test x"$enable_valgrind_safe" = xyes; then
	AC_DEFINE(ENABLE_VALGRIND_SAFE, 1, [Define to 1 if valgrind-compatible memory fetch is needed])
fi

AC_CACHE_CHECK([[for valgrind]], [pire_cv_have_valgrind], AC_CHECK_PROG([pire_cv_have_valgrind], [valgrind], [yes], [no]))
AM_CONDITIONAL([HAVE_VALGRIND], [test x"$pire_cv_have_valgrind" = xyes])

AC_CONFIG_FILES([
	Makefile
	pire/Makefile
	tests/Makefile
	pkg/Makefile
	tools/Makefile
	tools/bench/Makefile
	samples/Makefile
	samples/inline/Makefile
	samples/blacklist/Makefile
	samples/pigrep/Makefile
])
AC_OUTPUT_COMMANDS([sed 's/#define \([A-Z]\)/#define PIRE_\1/' < config.h.in > pire/config.h])
AC_OUTPUT
