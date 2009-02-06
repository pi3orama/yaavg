#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(yaavg, 0.00, pi3orama@gmail.com)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/video/rcommand.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

CFLAGS="-std=gnu99"
AC_ARG_ENABLE([link-rdynamic], [AC_HELP_STRING(--enable-link-rdynamic, [Use -rdynamic option for backtrace])],
	[LDFLAGS="$LDFLAGS -rdynamic"],
	[])

AC_ARG_ENABLE([debug], [AC_HELP_STRING(--enable-debug, [set CFLAGS to "-g -O0"])],
	[CFLAGS="$CFLAGS -g -O0"],
	[CFLAGS="$CFLAGS -g -O2"])


# Checks for libraries.

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


# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CHECK_HEADER([execinfo.h],[
    AC_CHECK_LIB(c, backtrace, [
        AC_DEFINE(HAVE_BACKTRACE, 1, [Has backtrace support])
        AC_DEFINE(HAVE_EXECINFO_H, 1, [Have execinfo.h])
    ])]
)

AC_ARG_ENABLE([static-opengl], [AC_HELP_STRING(--enable-static-opengl, [use ld link opengl symbol directly])],
	[
	 GL_CFLAGS="$GL_CFLAGS_TEMP -DSTATIC_OPENGL"
	 GL_LDFLAGS="$GL_LDFLAGS_TEMP"
	 GL_LIBS="$GL_LIBS_TEMP"
	 ],
	[
	 GL_CFLAGS="$GL_CFLAGS_TEMP"
	 GL_LDFLAGS=""
	 GL_LIBS=""
	 ])

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)
AC_SUBST([SDL_CFLAGS])
AC_SUBST([SDL_LIBS])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 src/common/Makefile
		 src/econfig/Makefile
		 src/event/Makefile
		 src/video/Makefile])

AC_OUTPUT

