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


CHECK_SYSTEM
DEFINE_CHECHERS

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
	else
		AC_DEFINE(ENABLE_SCREENSHOT, 1, [Define to 1 to make screenshot enable])
	fi
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
	AC_DEFINE(VIDEO_OPENGL_ENGINE, 1, [Define to 1 to use OpenGL engine])
# driver can be SDL or GLX
	if test "x$VIDEO_DRIVER" = "xSDL"; then
		CheckSDL
		if test "x$have_sdl" != "xyes"; then
			AC_MSG_ERROR(   *** Selected driver SDL doesn't exist)
		fi
		AC_DEFINE(VIDEO_OPENGL_SDL_DRIVER, 1, [Define to 1 to use SDL support OpenGL])
	elif test "x$VIDEO_DRIVER" = "xGLX"; then
		CheckGLX
		if test "x$have_glx" != "xyes"; then
			AC_MSG_ERROR(   *** Selected driver GLX doesn't exist)
		fi
		AC_DEFINE(VIDEO_OPENGL_GLX_DRIVER, 1, [Define to 1 to use GLX support OpenGL])
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

