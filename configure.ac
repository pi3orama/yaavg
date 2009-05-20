#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(yaavg, 0.00, pi3orama@gmail.com)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/video/rcommand.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB


AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

CFLAGS="-std=gnu99 -Wall"

AC_ARG_ENABLE([link-rdynamic], [AC_HELP_STRING(--enable-link-rdynamic, [Use -rdynamic option for backtrace. default: no])],
	[
	 if test "x$enableval" = "xyes" ; then
		 LDFLAGS="$LDFLAGS -rdynamic"
	 fi],
	[])

CFLAGS_save="$CFLAGS"
CFLAGS="$CFLAGS -g -O2"
AC_ARG_ENABLE([debug], [AC_HELP_STRING(--enable-debug, [set CFLAGS to "-g -O0". default: no])],
	[
	 if test "x$enableval" = "xyes" ; then
		 CFLAGS="$CFLAGS_save -g -O0"
	 fi],
	[])


# Checks for libraries.


AC_CHECK_FUNCS(sigaction select nanosleep)
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

# Check for SDL
AC_PATH_PROGS([SDL_CONFIG], [sdl-config], [none])
if test "x$SDL_CONFIG" = "xnone"; then
	AC_MSG_ERROR([*** SDL not found!])
fi


SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_LIBS=`$SDL_CONFIG --libs`

# Check for OpenGL library
AC_PATH_X

AC_MSG_CHECKING(for GL_CFLAGS)
AC_ARG_WITH(gl-cflags, [  --with-gl-cflags=CFLAGS ],
		       [GL_CFLAGS_TEMP="$withval"],
		       [GL_CFLAGS_TEMP="-I$x_includes"])
CFLAGS_save=$CFLAGS
CFLAGS="$CFLAGS $GL_CFLAGS_TEMP"
AC_COMPILE_IFELSE([[
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
		   ]],, [AC_MSG_ERROR([*** OpenGL Header not found!])])
CFLAGS=$CFLAGS_save
AC_MSG_RESULT($GL_CFLAGS_TEMP)


AC_MSG_CHECKING(for GL_LDFLAGS)
AC_ARG_WITH(gl-ldflags, [  --with-gl-ldflags=LDFLAGS ],
		       [GL_LDFLAGS_TEMP="$withval"],
		       [GL_LDFLAGS_TEMP="$x_libraries"])
AC_MSG_RESULT($GL_LDFLAGS_TEMP)

AC_MSG_CHECKING(for GL_LIBS)
AC_ARG_WITH(gl-libs, [  --with-gl-libs=LIBS ],
		     [GL_LIBS_TEMP="$withval"],
		     [GL_LIBS_TEMP="-lGL -lGLU -lglut"])
AC_MSG_RESULT($GL_LIBS_TEMP)


CFLAGS_save=$CFLAGS
LDFLAGS_save=$LDFLAGS
LIBS_save=$LIBS

CFLAGS="$CFLAGS $GL_CFLAGS_TEMP"
LDFLAGS="$LDFLAGS $GL_LDFLAGS_TEMP"
LIBS="$LIBS $GL_LIBS_TEMP"
AC_COMPILE_IFELSE([[
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
int main()
{
	glEnd();
	return 0;
}
		   ]],, [AC_MSG_ERROR([*** OpenGL Library not found!])])
CFLAGS=$CFLAGS_save
LDFLAGS=$LDFLAGS_save
LIBS=$LIBS_save

# Checks for libpng

libpng12_exists="no"
PKG_CHECK_MODULES([LIBPNG], [libpng12 >= 1.2.7],
		  [libpng12_exists="yes"], [libpng12_exists="no"])
if test x$libpng12_exists = "xno" ; then
	AC_MSG_WARN([*** libpng not found, some features disabled])
fi

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
AC_CHECK_FUNCS([atexit gettimeofday localtime_r memset regcomp strdup strerror])


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

GL_STATIC="no"
GL_CFLAGS="$GL_CFLAGS_TEMP"
GL_LDFLAGS=""
GL_LIBS=""

AC_ARG_ENABLE([static-opengl], [AC_HELP_STRING(--enable-static-opengl,
	       [use ld link opengl symbol directly. default: no])],
	[
	 if test "x$enableval" = "xyes"; then
		 GL_STATIC="yes"
		 GL_CFLAGS="$GL_CFLAGS_TEMP -DSTATIC_OPENGL"
		 GL_LDFLAGS="$GL_LDFLAGS_TEMP"
		 GL_LIBS="$GL_LIBS_TEMP"
	 fi
	 ],
	[])

SCREENSHOT="yes"
AC_ARG_ENABLE([screenshot], [AC_HELP_STRING(--disable-screenshot, [Forbid user to take screen shot. default: enable])],
	[
	 if test "x$enableval" = "xno" ; then
		 SCREENSHOT="no"
	fi], [])

# Begin check features
# if no libpng, no screenshot
if test "x$SCREENSHOT" = "xyes" -a "x$libpng12_exists" = "xno"; then
	AC_MSG_WARN([*** no libpng, disable screenshot])
	SCREENSHOT="no"
fi

# output enable_screenshot into config.h
if test "x$SCREENSHOT" = "xyes" ; then
	AC_DEFINE(ENABLE_SCREENSHOT, 1, [Define to 1 to make screenshot enable])
fi



AC_SUBST([GL_CFLAGS])
AC_SUBST([GL_LIBS])
AC_SUBST([SDL_CFLAGS])
AC_SUBST([SDL_LIBS])
AC_SUBST([LIBPNG_CFLAGS])
AC_SUBST([LIBPNG_LIBS])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 src/common/Makefile
		 src/econfig/Makefile
		 src/event/Makefile
		 src/resource/Makefile
		 src/video/Makefile
		 src/spikes/Makefile])

AC_OUTPUT

# finally, report configuration
echo
echo "Environments"
echo "CFLAGS........................... $CFLAGS"
echo "LDFLAGS.......................... $LDFLAGS"
echo
echo "Packages"
echo "SDL.............................. $SDL_CFLAGS"
echo " LDFLAGS......................... $SDL_LDFLAGS"
echo " LIBS............................ $SDL_LIBS"
echo "OpenGL........................... $GL_CFLAGS"
if test "x$GL_STATIC" = "xyes"; then
echo "OpenGL Linkage................... static"
echo " LDFLAGS......................... $GL_LDFLAGS"
echo " LIBS............................ $GL_LIBS"
else
echo "OpenGL Linkage................... dynamic"
fi
if test "x$libpng12_exists" = "xno"; then
echo "libpng12......................... no"
else
echo "libpng12......................... found"
echo " CFLAGS.......................... $LIBPNG_CFLAGS"
echo " LIBS............................ $LIBPNG_LIBS"
fi
echo
echo "Features"
echo "Screen shot support.............. $SCREENSHOT"

