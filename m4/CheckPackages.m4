AC_DEFUN([DEFINE_CHECHERS], [
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
])

