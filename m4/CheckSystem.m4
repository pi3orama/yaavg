AC_DEFUN([CHECK_SYSTEM],[






AC_FUNC_SELECT_ARGTYPES

dnl Check if we have malloc_stats. see debug.c/h
AC_CHECK_FUNC([malloc_stats], [have_malloc_stats=yes])
if test "x${have_malloc_stats}" = "xyes"; then
   AC_DEFINE(HAVE_MALLOC_STATS, 1, [define if found malloc_stats])
fi
AC_SUBST(HAVE_MALLOC_STATS)

dnl Check if we have mallinfo. see debug.c/h
AC_CHECK_FUNC([mallinfo], [have_mallinfo=yes])
if test "x${have_mallinfo}" = "xyes"; then
   AC_DEFINE(HAVE_MALLINFO, 1, [define if found mallinfo])
fi
AC_SUBST(HAVE_MALLINFO)

have_clock_gettime=no
AC_CHECK_LIB(rt, clock_gettime, have_clock_gettime=yes)
if test "x$have_clock_gettime" = "xyes"; then
	AC_DEFINE(HAVE_CLOCK_GETTIME, [], [librt have clock_gettime()])
fi

AC_FUNC_ALLOCA


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([malloc.h memory.h stdint.h stdlib.h string.h unistd.h \
		  setjmp.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT64_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit gettimeofday localtime_r memset regcomp strdup strerror sigaction select nanosleep])


# Check if sigsetjmp is available.  Using AC_CHECK_FUNCS won't do
# since sigsetjmp might only be defined as a macro.
AC_CACHE_CHECK([for sigsetjmp], yaavg_cv_func_sigsetjmp,
[AC_TRY_COMPILE([
#include <setjmp.h>
], [sigjmp_buf env; while (! sigsetjmp (env, 1)) siglongjmp (env, 1);],
yaavg_cv_func_sigsetjmp=yes, yaavg_cv_func_sigsetjmp=no)])
if test $yaavg_cv_func_sigsetjmp = yes; then
  AC_DEFINE(HAVE_SIGSETJMP, 1, [Define if sigsetjmp is available. ])
fi

AC_CHECK_HEADER([execinfo.h],[
    AC_CHECK_LIB(c, backtrace, [
        AC_DEFINE(HAVE_BACKTRACE, 1, [Has backtrace support])
        AC_DEFINE(HAVE_EXECINFO_H, 1, [Have execinfo.h])
    ])]
)

])
