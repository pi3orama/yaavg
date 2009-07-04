/* 
 * test_video.c
 * by WN @ Mar. 15, 2009
 */

#include <stdio.h>
#include <common/math.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <video/video.h>
#include <video/video_gl.h>
#include <video/texture_gl.h>
#include <event/event.h>

#include <math/matrix.h>

static const char * vertex_shader_bak[] = {
	"#version 130\n",
	"uniform mat4 matProj;\n",
	"in vec4 iPosition;\n",
	"out vec4 voColor;\n",
	"void main() {\n",
	"    voColor = iColor;\n",
	"    gl_Position = matProj * iPosition;",
	"}\n",
};

static const char * vertex_shader[] = {
	"#version 130\n",
	"in vec4 iPosition;\n",
	"uniform mat4 iModelView;\n"
	"out vec4 voColor;\n",
	"void main() {\n",
	"    gl_Position = iModelView * iPosition;\n",
	"    gl_PointSize = 256;\n",
	"}\n",
};

static const char * fragment_shader[] = {
	"#version 130\n",
	"uniform sampler2D inTex;\n",
	"out vec4 fragColor;\n",
	"void main() {\n",
//	"    fragColor.r = 1;\n"
//	"    fragColor.g = gl_PointCoord.t;\n"
    "    fragColor = texture(inTex, gl_PointCoord);\n"
//  "    fragColor = texture(inTex, vec2(0.5, 0.3));\n"
//	"    fragColor.g = voColor.g;\n"
	"}\n",
};



struct rcmd_draw {
	struct render_command base;
	tick_t total_time;
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint buffer;

	mat4x4 modelview;
	GLuint idx_mv;
	GLuint idx_tex;
	GLuint idx_position;
	GLuint idx_texcoord;

	struct texture_gl * tex;
	struct texture_gl * tex2;
};

static GLuint
compile_shader(GLenum type, GLsizei count, const char ** lines)
{
	GLuint shader = glCreateShader(type);
	assert(shader != 0);
	glShaderSource(shader, count, lines, NULL);
	/* compile and check */
	GLint s;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &s);
	if (s == GL_FALSE) {
		GLint len;
		char * log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, NULL, log);
		THROW(EXCEPTION_FATAL, "compile error: %s", log);
		free(log);
	}
	return shader;
}

static int
init_program(GLuint vert, GLuint frag)
{
	GLuint prog;
	prog = glCreateProgram();
	if (vert != 0)
		glAttachShader(prog, vert);
	if (frag != 0)
		glAttachShader(prog, frag);

	/* must bind draw buffer before the program linkage, according to GL3 spec
	 * section 3.9.2 - Shader outputs:
	 *
	 * The binding of a user-defined varying out variable to a fragment color
	 * number can be specified explicitly. The command
	 * 
	 * void BindFragDataLocation( uint program, uint colorNumber, const char
	 * *name );
	 * 
	 * specifies that the varying out variable name in program should be bound
	 * to fragment color colorNumber when the program is next linked.
	 */
	glBindFragDataLocation(prog, 0, "fragColor");

	GLint s;
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &s);
	if (s == GL_FALSE) {
		GLint len;
		char * log;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		log = (char*) malloc(len);
		glGetProgramInfoLog(prog, len, NULL, log);
		THROW(EXCEPTION_FATAL, "link log: %s\n", log);
		free(log);
	}
	return prog;
}


static int
draw_init(struct render_command * __rcmd)
{
	struct rcmd_draw * rcmd =
		container_of(__rcmd, struct rcmd_draw, base);
	FORCE(SYSTEM, "Draw init\n");

	rcmd->vertex_shader = compile_shader(GL_VERTEX_SHADER,
			sizeof(vertex_shader) / sizeof(char*), vertex_shader);
	rcmd->fragment_shader = compile_shader(GL_FRAGMENT_SHADER,
			sizeof(fragment_shader) / sizeof(char*), fragment_shader);
	rcmd->program = init_program(
			rcmd->vertex_shader,
			rcmd->fragment_shader);

	GL_POP_ERROR();
	rcmd->idx_mv = glGetUniformLocation(rcmd->program, "iModelView");
	GL_POP_ERROR();
	rcmd->idx_tex = glGetUniformLocation(rcmd->program, "inTex");
	GL_POP_ERROR();
	rcmd->idx_position = glGetAttribLocation(rcmd->program, "iPosition");
	GL_POP_ERROR();
//	rcmd->idx_color = glGetAttribLocation(rcmd->program, "iColor");
//	GL_POP_ERROR();
//	glVertexAttrib3f(rcmd->idx_color, 1.0f, 0.0f, 0.1f);
//	GL_POP_ERROR();

	load_identity(&rcmd->modelview);


	glGenBuffers(1, &rcmd->buffer);
	glBindBuffer(GL_ARRAY_BUFFER, rcmd->buffer);

	const GLfloat varray[] = {
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 1.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 1.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 1.0,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(varray), varray, GL_STATIC_DRAW);

	GL_POP_ERROR();

	res_id_t texres;
	char * fn = "common/rgb.png";
	texres = (uint64_t)(uint32_t)fn;
	struct rectangle rect = {
		0, 0, 4096, 4096
	};

	rcmd->tex = texgl_create(texres, rect, NULL, NULL);
	GL_POP_ERROR();

	return 0;
}

static int draw_remove(struct render_command * __rcmd,
			rcmd_remove_reason_t reason, int flags)
{
	struct rcmd_draw * rcmd =
		container_of(__rcmd, struct rcmd_draw, base);
	FORCE(SYSTEM, "Draw remove\n");
	glDeleteProgram(rcmd->program);
	glDeleteShader(rcmd->vertex_shader);
	glDeleteShader(rcmd->fragment_shader);
	glDeleteBuffers(1, &rcmd->buffer);

	if (rcmd->tex)
		TEXGL_RELEASE(rcmd->tex);

	return 0;
}

static int
draw_render(struct render_command * __rcmd,
		dtick_t delta_ticks)
{
	struct rcmd_draw * rcmd =
		container_of(__rcmd, struct rcmd_draw, base);

	static int n = 0;
	if (n == 0) {
		_matrix_scale(&rcmd->modelview, 0.3f, 0.3f, 0.3f);
	}
	n++;
	//	_matrix_translate(&rcmd->modelview, .005, 0.005, .0);
	_matrix_rotate(&rcmd->modelview, 0.1f * delta_ticks, 1.0f, 1.0f, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, rcmd->buffer);

	glUseProgram(rcmd->program);
	glVertexAttribPointer(rcmd->idx_position, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(rcmd->idx_position);



	glUniformMatrix4fv(rcmd->idx_mv, 1, GL_FALSE, rcmd->modelview.f);

	int elements[] = {
		0, 1,
		0, 2,
		1, 3,
		2, 3,

		0, 4,
		1, 5,
		2, 6,
		3, 7,

		4, 5,
		4, 6,
		5, 7,
		6, 7
	};

	glActiveTexture(GL_TEXTURE0);
	/* Enable targets of all dimensionalities,
	 * including TEXTURE_2D, have been deprecated */
	//	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rcmd->tex->hwtexs[0]);

	glUniform1i(rcmd->idx_tex, 0);
	GL_POP_ERROR();

	//	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, elements);
	//	glDrawArrays(GL_TRIANGLES, 0, 3);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	//	glPointSize(32);
	GL_POP_ERROR();


	/* STRANGE:
	 * according to gl3 spec E.1: 
	 *
	 * Non-sprite points - Enable/Disable targets POINT_SMOOTH
	 * and POINT_SPRITE, and all associated state. Point rasterization is always
	 * performed as though POINT SPRITE were enabled.
	 *
	 * however, in NV's implentmetion, POINT_SPRITE is disabled
	 * by default, while glEnable target GL_POINT_SPRITE is disabled too.
	 * These make there's no way to enable POINT SPRITE in NV's
	 * forward compatible context.
	 */
	glEnable(GL_POINT_SPRITE);
	
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,
			GL_LOWER_LEFT);

	glDrawElements(GL_POINTS, 2, GL_UNSIGNED_INT, elements);
	GL_POP_ERROR();


#if 0
	glUseProgram(0);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord2f(1.0, 1.0);
	glVertex2d(0.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2d(-0.4, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex2d(-0.4, -0.4);
	glTexCoord2f(1.0, 0.0);
	glVertex2d(0.0, -0.4);
	glEnd();
#endif



	return RENDER_OK;
}

static struct rcmd_operations draw_ops = {
	.init	= draw_init,
	.render = draw_render,
	.remove = draw_remove,
};



/* ****************************************************************************** */



struct rcmd_clear {
	struct render_command base;
};

static int
clear_render(struct render_command * __rcmd,
		dtick_t delta_ticks)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return RENDER_OK;
}

static struct rcmd_operations clear_ops = {
	.render = clear_render,
};


/* ****************************************************************************** */


static struct time_controller {
	tick_t realtime, oldrealtime;
	dtick_t deltatime;
	int mspf_fallback;
	int mspf;
} time_controller;

static void
frame(struct video_context * video_ctx, dtick_t deltatime,
		int event)
{
	volatile struct exception exp;
entry:
	TRY_CATCH(exp, MASK_SUBSYS_ALL) {
		video_render(deltatime);
		video_swap_buffers();
	}
	CATCH(exp)
	{
		case EXCEPTION_NO_ERROR:
			break;
		case EXCEPTION_SUBSYS_RERUN:
			WARNING(VIDEO, "video frame rerender: %s\n", exp.message);
			goto entry;
			break;
		case EXCEPTION_SUBSYS_SKIPFRAME:
			WARNING(VIDEO, "video frame skipped: %s\n", exp.message);
			break;
		case EXCEPTION_SUBSYS_REINIT:
			WARNING(VIDEO, "video reinit: %s\n", exp.message);
			video_reinit();
			THROW(EXCEPTION_SYS_SKIPFRAME, "video reinit, skip this frame");
			break;
		default:
			INTERNAL_ERROR(SYSTEM, "!@#$%^&\n");
			break;
	}
}

static void
render(struct video_context * video_ctx)
{

	/* start render */
	int event = event_poll();
	int frames = 0;

	tick_t realtime, oldrealtime;
	dtick_t deltatime;
	tick_t start_time;


	start_time = realtime = get_ticks();

	while((event != 1)) {
		oldrealtime = realtime;
		realtime = get_ticks();
		deltatime = realtime - oldrealtime;
		
		if (frames >= 300) {
			VERBOSE(VIDEO, "300 frames in %d ticks\n", realtime - start_time);
			VERBOSE(VIDEO, "fps=%f\n", 300.0f / ((realtime - start_time) / 1000.0f));
			frames = 0;
			start_time = realtime;
		}

		int render_time;

		if (deltatime > time_controller.mspf_fallback)
			deltatime = time_controller.mspf_fallback;
		if (deltatime < 0) {
			WARNING(VIDEO, "Time stepped backwards, from %u to %u\n",
					oldrealtime, realtime);
			deltatime = 0;
		}

		time_controller.realtime = realtime;
		time_controller.oldrealtime = oldrealtime;
		time_controller.deltatime = deltatime;

		struct exception exp;

entry:
		TRY_CATCH(exp, MASK_SYS_ALL) {
			if (event == 2) {
				/* XXX econfig is not finished yet! if econfig.c
				 * doesn't contain such entry, the set and get both do
				 * nothing! XXX */

				if (conf_get_bool("video.fullscreen", TRUE)) {
					conf_set_integer("video.resolution.w", 800);
					conf_set_integer("video.resolution.h", 600);
					conf_set_bool("video.fullscreen", FALSE);
				}
				else {
					conf_set_integer("video.resolution.w", 1280);
					conf_set_integer("video.resolution.h", 800);
					conf_set_bool("video.fullscreen", TRUE);
				}
				conf_set_string("video.opengl.gllibrary", NULL);
				THROW(EXCEPTION_SYS_REINIT, "normal reinit");
			}
			if (event == 3)
				video_screen_shot();

			if (event == 4) {
				THROW(EXCEPTION_SYS_RERUN, "normal rerun");
			}

			if (event == 5) {
				THROW(EXCEPTION_SYS_SKIPFRAME, "normal skip");
			}

			frame(video_ctx, deltatime, event);
			frames ++;
		}
		CATCH(exp) {
			case EXCEPTION_NO_ERROR:
				break;
			case EXCEPTION_SYS_RERUN:
				WARNING(SYSTEM, "rerun this frame: %s\n", exp.message);
				event = 0;
				goto entry;
				break;
			case EXCEPTION_SYS_SKIPFRAME:
				WARNING(SYSTEM, "skip this frame: %s\n", exp.message);
				delay(time_controller.mspf * 10);
				event = 0;
				break;
			case EXCEPTION_SYS_REINIT:
				VERBOSE(SYSTEM, "System reinit: %s\n", exp.message);
				video_reinit();
				break;
			default:
				print_exception(FATAL, SYSTEM, exp);
				INTERNAL_ERROR(SYSTEM, "@!#!@$%\n");
				break;
		}

		if (time_controller.mspf > 0) {
			render_time = get_ticks() - realtime;
			if (render_time < time_controller.mspf)
				delay(time_controller.mspf - render_time);
		}

		/* ???? */
		event = event_poll();
	}


}

int main(int argc, char * argv[])
{
//	DEBUG_INIT("/tmp/debug");
	DEBUG_INIT(NULL);
	VERBOSE(SYSTEM, "Start!!!\n");

	math_init();

	struct video_context * video_ctx = NULL;

	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		conf_init(argc, argv);
		video_ctx = video_init();
		event_init();

		int mspf = conf_get_integer("video.mspf", 50);	/* fps=20 */
		int mspf_fallback = conf_get_integer("video.mspf.fallback", 100);

		if (mspf > mspf_fallback)
			mspf = mspf_fallback;

		VERBOSE(VIDEO, "Desired fps is %f\n", 1000.0 / mspf);
		VERBOSE(VIDEO, "Fps fallback is %f\n", 1000.0 / mspf_fallback);

		time_controller.mspf = mspf;
		time_controller.mspf_fallback = mspf_fallback;



		/* create textures */


		/* link commands */
		struct rcmd_draw draw_cmd;
		rcmd_init(&(draw_cmd.base), "Draw", FALSE, video_ctx, &draw_ops);
		struct rcmd_clear clear_cmd;
		rcmd_init(&(clear_cmd.base), "Clear", FALSE, video_ctx, &clear_ops);
		video_insert_command(&(draw_cmd.base), AFTER, NULL);
		video_insert_command(&(clear_cmd.base), AFTER, NULL);
		rcmd_set_active(&(draw_cmd.base));
		rcmd_set_active(&(clear_cmd.base));

		render(video_ctx);
	}
	CATCH(exp) {
		case (EXCEPTION_NO_ERROR):
			VERBOSE(SYSTEM, "No error!\n");
			break;
		case (EXCEPTION_USER_QUIT):
			VERBOSE(SYSTEM, "User quit: %s\n", exp.message);
			break;
		default:
			print_exception(ERROR, SYSTEM, exp);
	}

	do_cleanup();
	gc_cleanup();

	show_mem_info();

	return 0;
}
// vim:tabstop=4:shiftwidth=4

