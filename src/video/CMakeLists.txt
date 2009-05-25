SET(SRC rlist.c rcommand.c 
        rlist.h rcommand.h
        video.h video_gl.c video.c 
        video_gl.h video_gl_sdl.c 
        video_gl_glx.c	
        video.h texture.h texture.c texture_gl.c
        video_screenshot.c
		gl_funcs.h)

ADD_CUSTOM_COMMAND(OUTPUT gl_funcs.h
	COMMAND	python ./generate_gl_funcs.py ./gl_funcs_list > ${YAAVG_BINARY_DIR}/src/video/gl_funcs.h
	DEPENDS generate_gl_funcs.py gl_funcs_list
	WORKING_DIRECTORY	${YAAVG_SOURCE_DIR}/src/video
	VERBATIM)

ADD_LIBRARY(video STATIC ${SRC})

IF(${VIDEO_ENGINE} STREQUAL "OpenGL")
	INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
	IF(STATIC_OPENGL)
		TARGET_LINK_LIBRARIES(video ${OPENGL_LIBRARIES})
	ENDIF(STATIC_OPENGL)
ENDIF(${VIDEO_ENGINE} STREQUAL "OpenGL")

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


# vim:tabstop=4:shiftwidth=4

