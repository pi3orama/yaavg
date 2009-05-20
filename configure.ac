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

##################
# Check features #
##################

checked_x11=no
CheckX11()
{
	
	if test "x$checked_x11" = "xyes" ; then
		return
	fi
	AC_PATH_X
	checked_x11=yes
}


# Check for SDL
checked_sdl=no
have_sdl=no
CheckSDL()
{
	if test "x$checked_sdl" = "xyes" ; then
		return
	fi


	AC_PATH_PROGS([SDL_CONFIG], [sdl-config], [none])
	if test "x$SDL_CONFIG" = "xnone"; then
		have_sdl=no
		SDL_CFLAGS=""
		SDL_LIBS=""
	else
		have_sdl=yes
		SDL_CFLAGS=`$SDL_CONFIG --cflags`
		SDL_LIBS=`$SDL_CONFIG --libs`
		AC_DEFINE(HAVE_SDL, 1, [Define to 1 to use SDL support utils])
	fi
	checked_sdl=yes
}

checked_opengl=no
have_opengl=no
CheckOpenGL()
{
	if test "x$checked_opengl" = "xyes" ; then
		return
	fi

	CheckX11
	# Check for OpenGL library
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
					   ]],, [AC_MSG_RESULT([*** OpenGL Header not found]
					   have_opengl=no
					   return
					   )])
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
						   glGetString(GL_VERSION);
						   return 0;
					   }
					   ]],[
						   have_opengl=yes
						   AC_MSG_RESULT([found OpenGL support])
						   ],
					   [
							 AC_MSG_RESULT([*** OpenGL Library not found!])
							 have_opengl=no
							 ])
	CFLAGS=$CFLAGS_save
	LDFLAGS=$LDFLAGS_save
	LIBS=$LIBS_save


	checked_opengl=yes
	if test "x$have_opengl" = "xno"; then
		return
	fi

	# Check for OpenGL linkage

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

}


# Checks for libpng

checked_libpng=no
libpng12_exists="no"
CheckLibPNG()
{
	if test "x$checked_libpng" = "xyes"; then
		return
	fi

	PKG_CHECK_MODULES([LIBPNG], [libpng12 >= 1.2.7],
					  [libpng12_exists="yes"], [libpng12_exists="no"])
	if test x$libpng12_exists = "xno" ; then
		AC_MSG_RESULT([*** libpng not found, some features disabled])
	fi
	checked_libpng=yes
}

checked_glx=no
have_glx=no
CheckGLX()
{
	if test "x$checked_glx" = "xyes"; then
		return
	fi

	CheckX11
	CheckOpenGL

	AC_ARG_WITH(glx-cflags, [  --with-glx-cflags=CFLAGS ],
				[GLX_CFLAGS_TEMP="$withval"],
				[GLX_CFLAGS_TEMP="-I$x_includes"])
	AC_ARG_WITH(glx-ldflags, [  --with-glx-ldflags=LDFLAGS ],
				[GLX_LDFLAGS_TEMP="$withval"],
				[GLX_LDFLAGS_TEMP="$x_libraries"])
	AC_ARG_WITH(glx-libs, [  --with-glx-libs=LIBS ],
				[GLX_LIBS_TEMP="$withval"],
				[GLX_LIBS_TEMP="-lX11"])

	AC_MSG_CHECKING(for OpenGL (GLX) support)
	CFLAGS_save=$CFLAGS
	LDFLAGS_save=$LDFLAGS
	LIBS_save=$LIBS

	CFLAGS="$CFLAGS $GLX_CFLAGS_TEMP $GL_CFLAGS"
	LDFLAGS="$LDFLAGS $GLX_LDFLAGS_TEMP $GL_LDFLAGS_TEMP"
	LIBS="$LIBS $GLX_LIBS_TEMP $GL_LIBS_TEMP"

	AC_COMPILE_IFELSE([[
					   #include <GL/gl.h>
					   #include <GL/glx.h>
					   #include <GL/glu.h>
					   int main()
					   {
						   glXChooseVisual(NULL, 0, NULL);
						   return 0;
					   }
					   ]],[
						   have_glx=yes
						   AC_MSG_RESULT($have_glx)
						   ],[
							  have_glx=no
							  AC_MSG_RESULT([   *** GLX compile error!])
							  ])
	CFLAGS=$CFLAGS_save
	LDFLAGS=$LDFLAGS_save
	LIBS=$LIBS_save

	checked_glx=yes

	GLX_CFLAGS=$GLX_CFLAGS_TEMP
	if test "x$GL_STATIC" = "xyes"; then
		GLX_LDFLAGS="$GLX_LDFLAGS_TEMP"
		GLX_LIBS="$GLX_LIBS_TEMP"
	else
		GLX_LDFLAGS=""
		GLX_LIBS=""
	fi
}



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



##################
# Check features #
##################

# screenshot check
SCREENSHOT="yes"
AC_ARG_ENABLE([screenshot], [AC_HELP_STRING(--disable-screenshot, [Forbid user to take screen shot. default: enable])],
	[
	 if test "x$enableval" = "xno" ; then
		 SCREENSHOT="no"
	fi], [])


if test "x$SCREENSHOT" = "xyes"; then
	CheckLibPNG
	if test "x$libpng12_exists" = "xno"; then
		AC_MSG_WARN([*** no libpng, disable screenshot])
		SCREENSHOT="no"
	fi
fi


# output enable_screenshot into config.h
if test "x$SCREENSHOT" = "xyes" ; then
	AC_DEFINE(ENABLE_SCREENSHOT, 1, [Define to 1 to make screenshot enable])
fi

#####################################
# video engine and driver selection #
#####################################

VIDEO_ENGINE="OpenGL"
AC_ARG_WITH(video-engine, [AC_HELP_STRING(--with-video-engine=engine,
						   [select video engine. canbe: OpenGL. default: OpenGL])],
						   [VIDEO_ENGINE="$withval"],
						   [VIDEO_ENGINE="OpenGL"])

VIDEO_DRIVER="SDL"

AC_ARG_WITH(video-driver, [AC_HELP_STRING(--with-video-driver,
						   [select video driver. canbe: SDL, GLX. default: SDL for opengl engine])],
						   [VIDEO_DRIVER="$withval"],
						   [])

if test "x$VIDEO_ENGINE" = "xOpenGL"; then
	CheckOpenGL
	if test "x$have_opengl" != "xyes"; then
		AC_MSG_ERROR(   *** Selected engine OpenGL doesn't exist)
	fi
	AC_DEFINE(OPENGL_ENGINE, 1, [Define to 1 to use OpenGL engine])
# driver can be SDL or GLX
	if test "x$VIDEO_DRIVER" = "xSDL"; then
		CheckSDL
		if test "x$have_sdl" != "xyes"; then
			AC_MSG_ERROR(   *** Selected driver SDL doesn't exist)
		fi
		AC_DEFINE(OPENGL_SDL_DRIVER, 1, [Define to 1 to use SDL support OpenGL])
	elif test "x$VIDEO_DRIVER" = "xGLX"; then
		CheckGLX
		if test "x$have_glx" != "xyes"; then
			AC_MSG_ERROR(   *** Selected driver GLX doesn't exist)
		fi
		AC_DEFINE(OPENGL_GLX_DRIVER, 1, [Define to 1 to use GLX support OpenGL])
	else
		AC_MSG_ERROR([   *** We don't support video driver $VIDEO_DRIVER])
	fi
else
	AC_MSG_ERROR([   *** We don't support video engine $VIDEO_ENGINE])
fi

AC_SUBST([GL_CFLAGS])
AC_SUBST([GL_LDFLAGS])
AC_SUBST([GL_LIBS])
AC_SUBST([SDL_CFLAGS])
AC_SUBST([SDL_LDFLAGS])
AC_SUBST([SDL_LIBS])
AC_SUBST([GLX_CFLAGS])
AC_SUBST([GLX_LDFLAGS])
AC_SUBST([GLX_LIBS])
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


if test "x$checked_sdl" = "xyes"; then
if test "x$have_sdl" = "xyes" ; then
echo "SDL.............................. found"
echo " CFLAGS.......................... $SDL_CFLAGS"
echo " LDFLAGS......................... $SDL_LDFLAGS"
echo " LIBS............................ $SDL_LIBS"
else
echo "SDL.............................. no"
fi
else
	echo "SDL.............................. no need"
fi

if test "x$checked_opengl" = "xyes"; then
if test "x$have_opengl" = "xyes" ; then
echo "OpenGL........................... found"
echo " CFLAGS.......................... $GL_CFLAGS"
if test "x$GL_STATIC" = "xyes"; then
echo " linkage......................... static"
echo " LDFLAGS......................... $GL_LDFLAGS"
echo " LIBS............................ $GL_LIBS"
else
echo " linkage......................... dynamic"
fi
else
echo "OpenGL........................... no"
fi
else
	echo "OpenGL........................... no need"
fi

if test "x$checked_glx" = "xyes"; then
if test "x$have_glx" = "xyes" ; then
echo "GLX.............................. found"
echo " CFLAGS.......................... $GLX_CFLAGS"
if test "x$GL_STATIC" = "xyes"; then
echo " linkage......................... static"
echo " LDFLAGS......................... $GLX_LDFLAGS"
echo " LIBS............................ $GLX_LIBS"
else
echo " linkage......................... dynamic"
fi
else
echo "GLX.............................. no"
fi
else
echo "GLX.............................. no need"
fi

if test "x$checked_libpng" = "xyes"; then
if test "x$libpng12_exists" = "xyes"; then
echo "libpng12......................... found"
echo " CFLAGS.......................... $LIBPNG_CFLAGS"
echo " LIBS............................ $LIBPNG_LIBS"
else
echo "libpng12......................... no"
fi
else
echo "libpng12......................... no need"
fi

echo
echo "Features"
echo "Screen shot support.............. $SCREENSHOT"

# vim:tabstop=4:shiftwidth=4

