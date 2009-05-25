SET(SRC debug.c 
	exception.c
	utils.c
	utils_sdl.c
	utils_common.c
	utils_png.c
	utils_linux.c
	mm.c
	utils.h
	debug.h
	exception.h
	defs.h
	mm.h
	math.h
	list.h)

ADD_LIBRARY(common STATIC ${SRC})

SET(tests test_debug test_debug2 test_exception test_exception2 test_geom)
build_tests("${tests}" "common")

ADD_CUSTOM_COMMAND(
	OUTPUT rgb.png
	COMMAND touch rgb.png)

ADD_DEPENDENCIES(common rgb)

#
#ADD_CUSTOM_COMMAND(OUTPUT rgb.png rgba.png
#	COMMAND ln -s ${YAAVG_SOURCE_DIR}/src/common/rgb.png .
#	COMMAND ln -s ${YAAVG_SOURCE_DIR}/src/common/rgba.png .
#	)

# vim:tabstop=4:shiftwidth=4

