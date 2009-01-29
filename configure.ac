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

AC_ARG_ENABLE([debug], [AC_HELP_STRING(--enable-debug, [set CFLAGS="-g -O0"])],
	[CFLAGS="$CFLAGS -g -O0"],
	[])

# Checks for libraries.

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
		       [GL_CFLAGS="$withval"],
		       [GL_CFLAGS="-I$x_includes"])
CFLAGS_save=$CFLAGS
CFLAGS="$CFLAGS $GL_CFLAGS"
AC_COMPILE_IFELSE([[
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
		   ]],, [AC_MSG_ERROR([*** OpenGL Header not found!])])
CFLAGS=$CFLAGS_save
AC_MSG_RESULT($GL_CFLAGS)


AC_MSG_CHECKING(for GL_LDFLAGS)
AC_ARG_WITH(gl-ldflags, [  --with-gl-ldflags=LDFLAGS ],
		       [GL_LDFLAGS="$withval"],
		       [GL_LDFLAGS="$x_libraries"])
AC_MSG_RESULT($GL_LDFLAGS)

AC_MSG_CHECKING(for GL_LIBS)
AC_ARG_WITH(gl-libs, [  --with-gl-libs=LIBS ],
		     [GL_LIBS="$withval"],
		     [GL_LIBS="-lGL -lGLU -lglut"])
AC_MSG_RESULT($GL_LIBS)


CFLAGS_save=$CFLAGS
LDFLAGS_save=$LDFLAGS
LIBS_save=$LIBS

CFLAGS="$CFLAGS $GL_CFLAGS"
LDFLAGS="$LDFLAGS $GL_LDFLAGS"
LIBS="$LIBS $GL_LIBS"
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

AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)
AC_SUBST([SDL_CFLAGS])
AC_SUBST([SDL_LIBS])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 src/common/Makefile
		 src/config/Makefile
		 src/video/Makefile])

AC_OUTPUT

