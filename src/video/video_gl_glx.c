/* 
 * video_gl_glx.c
 * by WN @ May. 20, 2009
 */
#include <common/defs.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <video/video_engine.h>
#include <video/video_gl.h>

#ifdef VIDEO_OPENGL_GLX_DRIVER
#include <video/utils_x11.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xext.h>

#include <errno.h>
#include <string.h>

#ifndef STATIC_OPENGL
# include <dlfcn.h>
#endif

#ifdef GLOPS
# undef GLOPS
#endif
#define GLOPS	(&(_glx_ctx.funcs))
#include <video/glx_funcs.h>

/* Some x proc use error handler send error, we can't
 * find whether there's an error or not by their return
 * values. When their failed, default behavior is call
 * exit(). However, we want to free some resource.  */
static bool_t x_failed = FALSE;

#define XCheckError()	do {			\
	if (x_failed)	{					\
		FATAL(GLX, "X11 error\n");	\
		THROW(EXCEPTION_FATAL, "X11 error");\
	}									\
} while(0)

static struct glx_context {
	struct gl_context base;
	void * dlhandle;
	const char * extensions;
	int bpp;
	int samples;
	Window root_win;
	Display * display;
	XVisualInfo * visinfo;
	Colormap colormap;
	/* SDL use 3 windows. I don't know why,
	 * just copy its code */
	Window wm_win;
	Window fs_win;
	Window main_win;
	GLXContext glx_context;
	int screen;
	struct glx_funcs funcs;
} _glx_ctx;

static struct glx_context * glx_ctx = NULL;

static void
glx_cleanup(struct cleanup * str);
static struct cleanup glx_cleanup_str = {
	.function	= glx_cleanup,
	.list		= {NULL, NULL},
};


static int (*old_x_err_handler)(Display *, XErrorEvent *)  = NULL;
static int (*old_x_ext_err_handler)(Display *, char *, char *) = NULL;
static int (*old_x_io_err_handler)(Display *)  = NULL;

static void
glx_cleanup(struct cleanup * str)
{
	int err;
	assert(str == &glx_cleanup_str);
	remove_cleanup(str);

	TRACE(GLX, "GLX cleanup\n");

	/* reset error handler */
	if (old_x_err_handler) {
		XSetErrorHandler(old_x_err_handler);
		old_x_err_handler = NULL;
	}

	if (old_x_ext_err_handler) {
		XSetExtensionErrorHandler(old_x_ext_err_handler);
		old_x_ext_err_handler = NULL;
	}

	if (old_x_io_err_handler) {
		XSetIOErrorHandler(old_x_io_err_handler);
		old_x_io_err_handler = NULL;
	}



	if (_glx_ctx.visinfo) {
		TRACE(GLX, "XFree visinfo\n");
		XFree(_glx_ctx.visinfo);
		_glx_ctx.visinfo = NULL;
	}

	if (_glx_ctx.display) {
		Display * d = _glx_ctx.display;

		if (_glx_ctx.glx_context) {
			GLXContext ctx = _glx_ctx.glx_context;
			TRACE(GLX, "GLX destroy context %p\n", ctx);
			glXMakeCurrent(d, None, NULL);
			glXDestroyContext(d, ctx);
			_glx_ctx.glx_context = NULL;
			XCheckError();
		}

		Window main_win = _glx_ctx.main_win;
		Window fs_win = _glx_ctx.fs_win;
		Window wm_win = _glx_ctx.wm_win;
		if (main_win) {
			TRACE(GLX, "Destory main_win 0x%x\n", main_win);
			XDestroyWindow(d, main_win);
			_glx_ctx.main_win = 0;
			XCheckError();
		}

		if (fs_win) {
			TRACE(GLX, "Destory fs_win 0x%x\n", fs_win);
			XDestroyWindow(d, fs_win);
			_glx_ctx.fs_win = 0;
			XCheckError();
		}

		if (wm_win) {
			TRACE(GLX, "Destory fs_win 0x%x\n", wm_win);
			XDestroyWindow(d, wm_win);
			_glx_ctx.wm_win = 0;
			XCheckError();
		}


		if (_glx_ctx.colormap != 0) {
			TRACE(GLX, "Free colormap %d\n", _glx_ctx.colormap);
			XFreeColormap(d, _glx_ctx.colormap);
			_glx_ctx.colormap = 0;
			XCheckError();
		}
		TRACE(GLX, "Close display %p\n", _glx_ctx.display);
		XCloseDisplay(_glx_ctx.display);
		_glx_ctx.display = NULL;
	}

	if (_glx_ctx.dlhandle) {
		TRACE(GLX, "free library\n");
		err = dlclose(_glx_ctx.dlhandle);
		if (err)
			WARNING(GLX, "Close dl library error\n");
		_glx_ctx.dlhandle = NULL;
	}

}


static void
load_gl_library(void)
{
#ifndef STATIC_OPENGL

	TRACE(GLX, "Begin to load glx library\n");

	/* Check conf */
	const char * library_name = NULL;
	library_name = conf_get_string("video.opengl.gllibrary", NULL);
	if (library_name == NULL)
		library_name = "libGL.so";

	TRACE(GLX, "libGL soname: %s\n", library_name);

	/* dlopen */
	void * handle;
	handle = dlopen(library_name, (RTLD_NOW|RTLD_GLOBAL));
	if (handle == NULL) {
		FATAL(GLX, "Load OpenGL library %s error: %s\n",
				library_name, strerror(errno));
		THROW(EXCEPTION_FATAL, "Load OpenGL library failed\n");
	}

	_glx_ctx.dlhandle = handle;

	/* init funcs */
#define INIT_GLX_FUNC_LIST
#include <video/glx_funcs.h>
#undef INIT_GLX_FUNC_LIST
	struct glfunc_init_item * item = &gl_func_init_list[0];
	while (item->name != NULL) {
		TRACE(GLX, "init GLX func %s\n", item->name);
		*(item->func) = (void*)dlsym(handle, item->name);
		if (*item->func == NULL)
			WARNING(GLX, "glx function %s not found\n", item->name);
		item ++;
	}

	/* check the existence of some important functions */

	if ( (glXChooseVisual == NULL) || 
			(glXCreateContext == NULL) ||
			(glXDestroyContext == NULL) ||
			(glXMakeCurrent == NULL) ||
			(glXSwapBuffers == NULL) ||
			(glXGetConfig == NULL) ||
			(glXQueryExtensionsString == NULL)) {
		FATAL(GLX, "glx library doesn't have some important functnins, "
				"check for your configuration\n");
		THROW(EXCEPTION_FATAL, "GLX load library error");
	}
#endif

}

static int
x_err_handler(Display * d, XErrorEvent * e)
{
	char m[1024];
	XGetErrorText(d, e->error_code, m, sizeof(m));
	WARNING(GLX, "X11 error: %s\n", m);
	XPrintDefaultError(d, e);

	x_failed = TRUE;
	/* return value is useless */
	return 0;
}

static int
x_ext_err_handler(Display * d, char * x, char * r)
{
	WARNING(GLX, "Xext error (may be harmless):\n"
			"\tExtension \"%s\" %s on display \"%s\"\n",
			x, r, XDisplayString(d));
	if (strcmp(r, "missing") == 0)
		return 0;
	else if (old_x_ext_err_handler)
		return old_x_ext_err_handler(d, x, r);
	return -1;
}

static int
x_io_err_handler(Display * d)
{
	/* FIXME */
	/* How to deal with IO error? */
	FATAL(GLX, "X11 IO error\n");
	
	/* set our display to NULL */
	if (_glx_ctx.display) {
		_glx_ctx.display = NULL;
	}

	if (old_x_io_err_handler)
		return old_x_io_err_handler(d);
	return 0;
}

static void
fill_visual_attrs(int * attrs)
{
	int i = 0;
	int bpp;

	attrs[i++] = GLX_RGBA;
#define setkv(k, v)	do {\
	attrs[i++] = k;			\
	attrs[i++] = v;			\
} while(0)

#define setk(k)	do {\
	attrs[i++] = k;			\
} while(0)

	bpp = conf_get_integer("video.opengl.bpp", 16);
	if (bpp >= 32) {
		setkv(GLX_RED_SIZE, 8);
		setkv(GLX_GREEN_SIZE, 8);
		setkv(GLX_BLUE_SIZE, 8);
		setkv(GLX_ALPHA_SIZE, 8);
		setkv(GLX_DEPTH_SIZE, 24);
	} else {
		setkv(GLX_RED_SIZE, 5);
		setkv(GLX_GREEN_SIZE, 5);
		setkv(GLX_BLUE_SIZE, 5);
		/* FIXME Why no alpha_size? */
		setkv(GLX_DEPTH_SIZE, 16);
	}
	_glx_ctx.bpp = bpp;

	setk(GLX_DOUBLEBUFFER);

	/* multisample */
	int samples = conf_get_integer("video.opengl.multisample", 0);
	if (samples != 0) {
		if (!match_word("GLX_ARB_multisample", _glx_ctx.extensions)) {
			samples = 0;
		} else {
			setkv(GLX_SAMPLE_BUFFERS_ARB, 1);
			setkv(GLX_SAMPLES_ARB, samples);
		}
	}
	_glx_ctx.samples = samples;


	if (match_word("GLX_EXT_visual_rating", _glx_ctx.extensions)) {
		setkv(GLX_VISUAL_CAVEAT_EXT, GLX_NONE_EXT);
	}

	setk(None);

#undef setkv
#undef setk
	return;
}

static inline void
check_destroy_window(Display * d, Window * win)
{
	if (*win) {
		WARNING(GLX, "Window 0x%x has not been destroied\n",
				*win);
		XDestroyWindow(d, *win);
		XCheckError();
		*win = 0;
	}
}

static void
create_aux_windows(void)
{
	Display * d = _glx_ctx.display;
	assert(d != NULL);
	int s = _glx_ctx.screen;
	Colormap cm = _glx_ctx.colormap;
	Window fs_win, wm_win;
	XVisualInfo * visinfo = _glx_ctx.visinfo;

	check_destroy_window(d, &_glx_ctx.fs_win);
	check_destroy_window(d, &_glx_ctx.wm_win);

	bool_t def_vis = (visinfo->visual ==
			DefaultVisual(d, s));

	XSetWindowAttributes xattr;
	xattr.override_redirect = True;
	xattr.background_pixel = def_vis ? BlackPixel(d, s) : 0;
	xattr.border_pixel = 0;
	xattr.colormap = cm;

	fs_win = XCreateWindow(d, _glx_ctx.root_win,
			0, 0, 32, 32, 0,
			visinfo->depth, InputOutput, visinfo->visual,
			CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap,
			&xattr);
	XCheckError();
	TRACE(GLX, "Create fs_win = 0x%x\n", fs_win);
	XSelectInput(d, fs_win, StructureNotifyMask);
	_glx_ctx.fs_win = fs_win;

	/* Copy from SDL code */
	{
		/* Tell KDE to keep the fullscreen window on top */
		XEvent ev;
		long mask;

		memset(&ev, 0, sizeof(ev));
		ev.xclient.type = ClientMessage;
		ev.xclient.window = _glx_ctx.root_win;
		ev.xclient.message_type = XInternAtom(d,
				"KWM_KEEP_ON_TOP", False);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = fs_win;
		ev.xclient.data.l[1] = CurrentTime;
		mask = SubstructureRedirectMask;
		XSendEvent(d, _glx_ctx.root_win, False, mask, &ev);
	}
	
	wm_win = XCreateWindow(d, _glx_ctx.root_win,
			0, 0, 32, 32, 0,
			visinfo->depth, InputOutput, visinfo->visual,
			CWBackPixel | CWBorderPixel | CWColormap,
			&xattr);
	XCheckError();
	TRACE(GLX, "Create wm_win = 0x%x\n", wm_win);
	_glx_ctx.wm_win = wm_win;

	/* Set WMHints */
	XWMHints * hints = NULL;
	hints = XAllocWMHints();
	assert(hints != NULL);
	hints->input = True;
	hints->flags = InputHint;
	XSetWMHints(d, wm_win, hints);
	XFree(hints);
	XCheckError();

    XSelectInput(d, wm_win,
			FocusChangeMask |
			KeyPressMask |
			KeyReleaseMask |
			PropertyChangeMask |
			StructureNotifyMask |
			KeymapStateMask);
	XCheckError();

	/* Set class hint */
	XClassHint * class_hint = NULL;
	class_hint = XAllocClassHint();
	assert(class_hint);
	class_hint->res_name = "YAAVG_APP";
	class_hint->res_class = "YAAVG_APP";
	XSetClassHint(d, wm_win, class_hint);
	XFree(class_hint);
	XCheckError();
	return;
}

static void
set_size_hints(int w, int h, bool_t full_screen, bool_t resizable)
{
	Display * d = _glx_ctx.display;
	assert(d != NULL);

	Window wm_win = _glx_ctx.wm_win;

	XSizeHints *hints;
	hints = XAllocSizeHints();
	assert(hints != NULL);

	if (resizable) {
		hints->min_width = 32;
		hints->min_height = 32;
		hints->max_height = 4096;
		hints->max_width = 4096;
	} else {
		hints->min_width = hints->max_width = w;
		hints->min_height = hints->max_height = h;
	}

	hints->flags = PMaxSize | PMinSize;
	if (full_screen) {
		hints->x = 0;
		hints->y = 0;
		hints->flags |= USPosition;
	}
	XSetWMNormalHints(d, wm_win, hints);
	XFree(hints);
	XCheckError();

	/* then, the window manager */
	/* Copy code from SDL */
	bool_t set;
	Atom WM_HINTS;

	/* We haven't modified the window manager hints yet */
	set = FALSE;

	/* First try to unset MWM hints */
	WM_HINTS = XInternAtom(d, "_MOTIF_WM_HINTS", True);
	if (WM_HINTS != None) {
		XDeleteProperty(d, wm_win, WM_HINTS);
		set = TRUE;
	}
	/* Now try to unset KWM hints */
	WM_HINTS = XInternAtom(d, "KWM_WIN_DECORATION", True);
	if (WM_HINTS != None) {
		XDeleteProperty(d, wm_win, WM_HINTS);
		set = TRUE;
	}
	/* Now try to unset GNOME hints */
	WM_HINTS = XInternAtom(d, "_WIN_HINTS", True);
	if (WM_HINTS != None) {
		XDeleteProperty(d, wm_win, WM_HINTS);
		set = TRUE;
	}
	/* Finally unset the transient hints if necessary */
	if (!set) {
		/* NOTE: Does this work? */
		XSetTransientForHint(d, wm_win, None);
	}

	/* resize window */
	XResizeWindow(d, wm_win, w, h);
}

static void
glx_create_context(void)
{
	Display * d = _glx_ctx.display;
	Window w = _glx_ctx.main_win;
	XVisualInfo * visinfo = _glx_ctx.visinfo;
	GLXContext context = NULL;
	bool_t err;

	assert(d != NULL);
	assert(visinfo != NULL);
	assert(w != 0);

	TRACE(GLX, "Begin to create glx context\n");
	XSync(d, False);
	context = glXCreateContext(d, visinfo,
			NULL,	/* Shared list */
			True	/* Direct render */
			);

	if (context == NULL) {
		ERROR(GLX, "glx create context failed, check your configuration\n");
		XCheckError();
	}

	TRACE(GLX, "GLX context created: %p\n", context);
	_glx_ctx.glx_context = context;

	err = glXMakeCurrent(d, w, context);
	XSync(d, False);
	XCheckError();
	if (!err) {
		ERROR(GLX, "bind context %p to window 0x%x failed, check your configuration\n",
				context, w);
		THROW(EXCEPTION_FATAL, "glx bind context failed");
	}

	/* set swap control */
	int swapcontrol = conf_get_integer("video.opengl.swapcontrol", 0);
	if (swapcontrol < 0)
		swapcontrol = 0;

	/* select func */
	if (swapcontrol > 0) {
		TRACE(GLX, "Set swapcontrol to %d\n", swapcontrol);
		if (glXSwapIntervalMESA) {
			TRACE(GLX, "\tuse glXSwapIntervalMESA\n");
			glXSwapIntervalMESA(swapcontrol);
		} else if (glXSwapIntervalSGI) {
			TRACE(GLX, "\tuse glXSwapIntervalSGI\n");
			glXSwapIntervalSGI(swapcontrol);
		} else {
			WARNING(GLX, "Ignore configuration setting of swapcontrol (%d)\n", swapcontrol);
		}
		XSync(d, False);
		XCheckError();
	}
}

static void
make_window(void)
{
	Display * d = _glx_ctx.display;
	/* SDL use 64 */
	int visual_attrs[64];
	XSetWindowAttributes win_attrs;
	Window root, win;
	int s = _glx_ctx.screen;

	assert(d != NULL);

	check_destroy_window(d, &_glx_ctx.main_win);

	root = RootWindow(d, s);
	_glx_ctx.root_win = root;

	/* Choose visual */
	fill_visual_attrs(visual_attrs);
	XVisualInfo * visinfo = glXChooseVisual(d, s, visual_attrs);
	if (visinfo == NULL) {
		FATAL(GLX, "GLX cannot choose visual, check your configuration\n");
		THROW(EXCEPTION_FATAL, "GLC cannot choose visual");
	}
	TRACE(GLX, "GLX choose visual: %p\n", visinfo);
	_glx_ctx.visinfo = visinfo;

	/* Create Window */
	/* create color map */
	Colormap cm =  XCreateColormap(d, root, visinfo->visual, AllocNone);
	XCheckError();
	_glx_ctx.colormap = cm;

	int black =
		(visinfo->visual == DefaultVisual(d, s)) ? BlackPixel(d, s) : 0;

	win_attrs.background_pixel = black;
	win_attrs.border_pixel = black;
	win_attrs.colormap = cm;
	int mask = CWBackPixel | CWBorderPixel | CWColormap;

	int w, h;
	bool_t full_screen, resizable;

	w = conf_get_integer("video.resolution.w", 640);
	h = conf_get_integer("video.resolution.h", 480);
	full_screen = conf_get_bool("video.fullscreen", FALSE);
	resizable = conf_get_bool("video.resizable", FALSE);

	TRACE(GLX, "ready to init window:\n");
	TRACE(GLX, "\t width = %d\n", w);
	TRACE(GLX, "\t height = %d\n", h);
	TRACE(GLX, "\t full_screen = %d\n", full_screen);
	TRACE(GLX, "\t resizable = %d\n", resizable);

	create_aux_windows();

	/* Copy code from SDL, don't know why */
	XSetWindowBackground(d, _glx_ctx.fs_win, 0);
	XClearWindow(d, _glx_ctx.fs_win);

	_glx_ctx.base.base.width = w;
	_glx_ctx.base.base.height = h;
	_glx_ctx.base.base.full_screen = full_screen;

	set_size_hints(w, h, full_screen, resizable);

	if (full_screen) {
		FATAL(GLX, "Create full screen window is not supported now\n");
		THROW(EXCEPTION_FATAL, "doesn't support full screen");
	}

	win = XCreateWindow(d, _glx_ctx.wm_win,
			0, 0, w, h, 0, visinfo->depth,
			InputOutput, visinfo->visual,
			mask, &win_attrs);
	XCheckError();
	TRACE(GLX, "Main window 0x%x created\n", win);
	_glx_ctx.main_win = win;


	XSelectInput(d, win,
			( EnterWindowMask | LeaveWindowMask
			  | ButtonPressMask | ButtonReleaseMask
			  | PointerMotionMask | ExposureMask ));

	glx_create_context();

	/* Backing store settings */
	/* Cache the window in the server, when possible */
	{
		Screen *xscreen;
		XSetWindowAttributes a;

		xscreen = ScreenOfDisplay(d, _glx_ctx.screen);
		a.backing_store = DoesBackingStore(xscreen);
		if ( a.backing_store != NotUseful ) {
			TRACE(GLX, "make window backing store\n");
			XChangeWindowAttributes(d, win, CWBackingStore, &a);
		}
	}
}

static void
map_window(void)
{
	Display * d = _glx_ctx.display;
	Window main_win = _glx_ctx.main_win;
	Window wm_win = _glx_ctx.wm_win;
	assert(d != NULL);
	assert(main_win != 0);
	assert(wm_win != 0);

	XMapWindow(d, main_win);
	XCheckError();
	XMapWindow(d, wm_win);
	XCheckError();

	XWaitMapped(d, wm_win);
	XCheckError();
	TRACE(GLX, "wm window mapped\n");
}

struct gl_context *
gl_init(void)
{
	if (glx_ctx != NULL) {
		WARNING(GLX, "multi init OpenGL\n");
		return &glx_ctx->base;
	}

	memset(&_glx_ctx, 0, sizeof(_glx_ctx));

	make_cleanup(&glx_cleanup_str);

	/* first: load libGL library */
	load_gl_library();

	/* make X connection */
	TRACE(GLX, "OpenDisplay\n");
	Display * display = NULL;
	display = XOpenDisplay(NULL);

	if (display == NULL) {
		FATAL(GLX, "GLX open display failed\n");
		THROW(EXCEPTION_FATAL, "GLX open display failed\n");
	}

	TRACE(GLX, "X11 display: %p\n", display);
	_glx_ctx.display = display;

	int screen;
	screen = DefaultScreen(display);
	_glx_ctx.screen = screen;

#ifdef X11_DEBUG
	XSynchronize(display, True);
#endif

	/* set error handler */
	old_x_err_handler = XSetErrorHandler(x_err_handler);
	old_x_io_err_handler = XSetIOErrorHandler(x_io_err_handler);
	old_x_ext_err_handler = XSetExtensionErrorHandler(x_ext_err_handler);

	/* extensions */
	const char * extensions = glXQueryExtensionsString(
			display, screen);
	TRACE(GLX, "GLX extensions string: %s\n", extensions);
	_glx_ctx.extensions = extensions;
	/* Open Window */
	make_window();

	map_window();

	glx_ctx = &_glx_ctx;
	

//	return NULL;
	return &glx_ctx->base;
}

void
gl_close(void)
{
	glx_cleanup(&glx_cleanup_str);
}
































void
gl_reinit(void)
{
	THROW(EXCEPTION_FATAL, "glx reinit not implentmented");
}

void *
gl_get_proc_address(const char * name)
{
	if (glx_ctx == NULL) {
		ERROR(GLX, "Try to get proc before glx inited\n");
		THROW(EXCEPTION_FATAL, "Impossiable bug");
	}
	return (dlsym)(glx_ctx->dlhandle, name);
}

void
video_swap_buffers(void)
{
	if ((glx_ctx == NULL) ||
			(_glx_ctx.display == NULL) ||
			(_glx_ctx.main_win == 0))
	{
		FATAL(GLX, "Call swap buffers before init glx");
		THROW(EXCEPTION_FATAL, "internal error");
	}
	glXSwapBuffers(glx_ctx->display, glx_ctx->main_win);
}

void
video_set_caption(const char * caption)
{
//	THROW(EXCEPTION_FATAL, "glx set caption not implented\n");
	return;
}

void
video_set_icon(const icon_t icon)
{
	THROW(EXCEPTION_FATAL, "glx set icom not implented\n");
	return;
}

#endif	/* VIDEO_OPENGL_GLX_DRIVER */

// vim:tabstop=4:shiftwidth=4

