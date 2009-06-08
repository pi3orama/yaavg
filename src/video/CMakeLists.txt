SET(SRC rlist.c rcommand.c 
        rlist.h rcommand.h
        video.h video_gl.c video.c 
		video_engine.h
        video_gl.h video_gl_sdl.c 
        video_gl_glx.c	
        video.h texture.h texture.c texture_gl.c
        video_screenshot.c
		utils_x11.c
		utils_x11.h
		x_common.h
		gl_funcs.h
		glx_funcs.h)

ADD_LIBRARY(video STATIC ${SRC})

IF(${VIDEO_ENGINE} STREQUAL "OpenGL")
	ADD_CUSTOM_COMMAND(OUTPUT gl_funcs.h
		COMMAND	python ./generate_gl_funcs.py ./gl_funcs_list > ${YAAVG_BINARY_DIR}/src/video/gl_funcs.h
		DEPENDS generate_gl_funcs.py gl_funcs_list
		WORKING_DIRECTORY	${YAAVG_SOURCE_DIR}/src/video
		VERBATIM)
	INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
	IF(STATIC_OPENGL)
		TARGET_LINK_LIBRARIES(video ${OPENGL_LIBRARIES})
	ENDIF(STATIC_OPENGL)
ELSE(${VIDEO_ENGINE} STREQUAL "OpenGL")
	ADD_CUSTOM_COMMAND(OUTPUT gl_funcs.h
		COMMAND touch ${YAAVG_BINARY_DIR}/src/video/gl_funcs.h
		WORKING_DIRECTORY   ${YAAVG_SOURCE_DIR}/src/video
		VERBATIM)
ENDIF(${VIDEO_ENGINE} STREQUAL "OpenGL")

IF(${VIDEO_DRIVER} STREQUAL "GLX")
	ADD_CUSTOM_COMMAND(OUTPUT glx_funcs.h
		COMMAND	python ./generate_gl_funcs.py ./glx_funcs_list glx > ${YAAVG_BINARY_DIR}/src/video/glx_funcs.h
		DEPENDS generate_gl_funcs.py glx_funcs_list
		WORKING_DIRECTORY	${YAAVG_SOURCE_DIR}/src/video
		VERBATIM)
	INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR} ${X11_xf86vmode_INCLUDE_PATH})
	LINK_DIRECTORIES("/usr/X11R7/lib")
	TARGET_LINK_LIBRARIES(video ${X11_LIBRARIES} ${X11_Xxf86misc_LIB} ${X11_Xxf86vm_LIB})
ELSE(${VIDEO_DRIVER} STREQUAL "GLX")
 	ADD_CUSTOM_COMMAND(OUTPUT glx_funcs.h
 		COMMAND touch ${YAAVG_BINARY_DIR}/src/video/glx_funcs.h
 		WORKING_DIRECTORY   ${YAAVG_SOURCE_DIR}/src/video
 		VERBATIM)
ENDIF(${VIDEO_DRIVER} STREQUAL "GLX")

IF (${video_setting} MATCHES "SDL")
	INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
	TARGET_LINK_LIBRARIES(video ${SDL_LIBRARY})
ENDIF(${video_setting} MATCHES "SDL")


IF (ENABLE_SCREENSHOT)
	INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
	ADD_DEFINITIONS(${PNG_DEFINITIONS})
	TARGET_LINK_LIBRARIES(video ${PNG_LIBRARIES})
ENDIF (ENABLE_SCREENSHOT)

TARGET_LINK_LIBRARIES(video common resource econfig)

OPTION(X11_DEBUG	"Enable X11 Synchronize")
# vim:tabstop=4:shiftwidth=4

