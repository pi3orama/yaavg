SET(SRC debug.c 
	exception.c
	utils.c
	utils_sdl.c
	utils_common.c
	utils_png.h
	utils_png_rw.c
	utils_png_bitmap.c
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
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
ADD_DEFINITIONS(${PNG_DEFINITIONS})
TARGET_LINK_LIBRARIES(common resource ${PNG_LIBRARIES})
IF (HAVE_SDL)
	INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
	TARGET_LINK_LIBRARIES(common ${SDL_LIBRARY})
ENDIF(HAVE_SDL)
#TARGET_LINK_LIBRARIES(common resource})


SET(tests
	test_debug
	test_debug2
	test_exception
	test_exception2
	test_exception3
	test_geom)
build_tests("${tests}" "common")


SET(RGB_PNG_FILE "src/common/rgb.png")
SET(RGBA_PNG_FILE "src/common/rgba.png")
ADD_CUSTOM_TARGET(CopyPng ALL
	COMMAND  ${CMAKE_COMMAND} -E copy_if_different
	${YAAVG_SOURCE_DIR}/${RGB_PNG_FILE} ${YAAVG_BINARY_DIR}/${RGB_PNG_FILE} 
	COMMAND  ${CMAKE_COMMAND} -E copy_if_different
	${YAAVG_SOURCE_DIR}/${RGBA_PNG_FILE} ${YAAVG_BINARY_DIR}/${RGBA_PNG_FILE}
	)

# vim:tabstop=4:shiftwidth=4

