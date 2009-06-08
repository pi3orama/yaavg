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

/* event needs some infos */
#include <video/x_common.h>

#include <video/utils_x11.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xext.h>
#include <X11/Xatom.h>
#ifdef HAVE_XF86VMODE
# include <X11/extensions/xf86vmode.h>
#endif

#ifdef HAVE_XRANDR
# include <X11/extensions/Xrandr.h>
#endif

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
bool_t x_failed = FALSE;

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

static struct cleanup * restore_fullscreen_cleanup = NULL;

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
	if (sigpipe_arised) {
		_glx_ctx.display = NULL;
		_glx_ctx.glx_context = NULL;
		_glx_ctx.main_win = 0;
		_glx_ctx.wm_win = 0;
		_glx_ctx.fs_win = 0;
		_glx_ctx.colormap = 0;
	}

	if (restore_fullscreen_cleanup != NULL) {
		TRACE(GLX, "restore from fullscreen\n");
		CLEANUP(restore_fullscreen_cleanup);
		restore_fullscreen_cleanup = NULL;
	}

	if (_glx_ctx.display) {
		Display * d = _glx_ctx.display;

		if (_glx_ctx.glx_context) {
			GLXContext ctx = _glx_ctx.glx_context;
			TRACE(GLX, "GLX destroy context %p\n", ctx);
			glXMakeCurrent(d, None, NULL);
			glXDestroyContext(d, ctx);
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
		THROW(EXCEPTION_FATAL, "Load OpenGL library failed");
	}

	_glx_ctx.dlhandle = handle;

	/* init funcs */
#define INIT_GLX_FUNC_LIST
#include <video/glx_funcs.h>
#undef INIT_GLX_FUNC_LIST
	struct glfunc_init_item * item = &gl_func_init_list[0];
	while (item->name != NULL) {
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

	sigpipe_arised = TRUE;
	fatal_cleanup();
	return 0;
}

static void
fill_visual_attrs(int * attrs)
{
	int i = 0;
	int bpp;

#define setkv(k, v)	do {\
	attrs[i++] = k;			\
	attrs[i++] = v;			\
} while(0)

#define setk(k)	do {\
	attrs[i++] = k;			\
} while(0)

	setk(GLX_RGBA);

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
		THROW(EXCEPTION_FATAL, "GLX cannot choose visual");
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
map_window(void);

struct _xvid_state {
	XF86VidModeModeLine	saved_modeline;
	int dotclock;
	int view_x;
	int view_y;
	bool_t need_restore;
};

static struct _xvid_state xvid_saved_state = {
	.need_restore = FALSE,
};


static void
map_fullscreen(Display * d, Window main_win, Window wm_win, Window fs_win,
		int x, int y, int real_x, int real_y)
{
	/* Map Windows */
	XMoveResizeWindow(d, fs_win, 0, 0, real_x, real_y);
	XReparentWindow(d, main_win, fs_win,
			(real_x - x) / 2,
			(real_y - y) / 2);

	XMoveCursorTo(d, _glx_ctx.root_win, real_x / 2,
			real_y / 2);

	/* Install colormap */
	if (_glx_ctx.colormap != 0)
		XInstallColormap(d, _glx_ctx.colormap);

	XMapWindow(d, main_win);
	XCheckError();
	/* We must map wm_win to receive key input */
	XMapWindow(d, wm_win);
	XCheckError();
	XMapRaised(d, fs_win);
	XCheckError();

	XWaitMapped(d, fs_win);

	XGrabPointer(d, main_win, True, 0,
			GrabModeAsync, GrabModeAsync,
			main_win, None, CurrentTime);
	XGrabKeyboard(d, wm_win, False,
			GrabModeAsync, GrabModeAsync, CurrentTime);
	XSync(d, False);
	XCheckError();
}
static void
xvid_cleanup(struct cleanup * _str)
{
#ifdef HAVE_XF86VMODE
	Display * d = _glx_ctx.display;
	int s = _glx_ctx.screen;

	remove_cleanup(_str);

	if (!xvid_saved_state.need_restore)
		return;
	if (d == NULL) {
		WARNING(GLX, "X11 connection has been destroied, reconnect.\n");
		d = XOpenDisplay(NULL);
		if (d == NULL) {
			FATAL(GLX, "reconnect failed, don't restore video mode\n");
			return;
		}
		s = DefaultScreen(d);
	}

	TRACE(GLX, "unlock mode switch mode\n");
	XF86VidModeLockModeSwitch(d, s, False);

	/* restore mode */
	XF86VidModeModeInfo mode;
	mode.dotclock = xvid_saved_state.dotclock;
	memcpy(&(mode.hdisplay), &(xvid_saved_state.saved_modeline),
			sizeof(XF86VidModeModeLine));
	TRACE(GLX, "restore from VID fullscreen\n");
	TRACE(GLX, "restore old mode\n");
	XF86VidModeSwitchToMode(d, s, &mode);

	TRACE(GLX, "restore viewport\n");
	int vx = xvid_saved_state.view_x;
	int vy = xvid_saved_state.view_y;
	XF86VidModeSetViewPort(d, s, vx, vy);

	if (_glx_ctx.display == NULL)
		XCloseDisplay(d);
#endif
}

static bool_t
trival_fullscreen(Display * d, Window main_win, Window wm_win, Window fs_win);

static bool_t
xvid_fullscreen(Display * d, Window main_win, Window wm_win, Window fs_win)
{
#ifdef HAVE_XF86VMODE
	/* some static values, used in cleanup */
	static struct cleanup fs_cleanup = {
		.function	= xvid_cleanup,
	};

	TRACE(GLX, "Try xvid fullscreen\n");

	Bool res;
	int event_base, error_base, major_ver, minor_ver;
	res = XF86VidModeQueryExtension(d, &event_base, &error_base);
	if (!res) {
		ERROR(GLX, "server doesn't support XF86VidMode\n");
		return FALSE;
	}
	TRACE(GLX, "XF86VidModeQueryExtension: event_base=%d, error_base=%d\n",
			event_base, error_base);

	res = XF86VidModeQueryVersion(d, &major_ver, &minor_ver);
	if (!res) {
		ERROR(GLX, "server doesn't support XF86VidMode\n");
		return FALSE;
	}
	TRACE(GLX, "XF86VidModeQueryVersion: major_ver=%d, minor_ver=%d\n",
			major_ver, minor_ver);

	int s = _glx_ctx.screen;
	int dotclock;
	XF86VidModeModeLine saved_modeline;

	/* Save mode */
	res = XF86VidModeGetModeLine(d, s, &dotclock, &saved_modeline);
	if (!res) {
		ERROR(GLX, "XF86 cannot get modeline\n");
		return FALSE;
	}

	int saved_x, saved_y;
	res = XF86VidModeGetViewPort(d, s, &saved_x, &saved_y);
	if (!res) {
		ERROR(GLX, "XF86 cannot get viewport\n");
		return FALSE;
	}

	VERBOSE(GLX, "Saved modeline:\n");
#define PELEMENT(s) VERBOSE(GLX, "\t"#s"=%d\n", saved_modeline.s)
	PELEMENT(hdisplay);
	PELEMENT(hsyncstart);
	PELEMENT(hsyncend);
	PELEMENT(htotal);
	PELEMENT(hskew);
	PELEMENT(vdisplay);
	PELEMENT(vsyncstart);
	PELEMENT(vsyncend);
	PELEMENT(vtotal);
	PELEMENT(flags);
#undef PELEMENT
	VERBOSE(GLX, "Saved viewport: %d x %d\n", saved_x, saved_y);

	/* set cleanup */
	xvid_saved_state.saved_modeline = saved_modeline;
	xvid_saved_state.dotclock = dotclock;
	xvid_saved_state.view_x = saved_x;
	xvid_saved_state.view_y = saved_y;

	/* find best mode */
	XF86VidModeModeInfo ** modes = NULL;
	int nmodes;
	res = XF86VidModeGetAllModeLines(d, s, &nmodes, &modes);
	if (!res) {
		ERROR(GLX, "XF86VidModeGetAllModeLines failed\n");
		return FALSE;
	}

	TRACE(GLX, "Find best mode in %d modes\n", nmodes);
	int best = -1;
	int w = _glx_ctx.base.base.width;
	int h = _glx_ctx.base.base.height;
	for (int i = 0; i < nmodes; i++) {
		if ((modes[i]->hdisplay == w) && (modes[i]->vdisplay == h)) {
			best = i;
			break;
		}

		int mw = modes[i]->hdisplay;
		int mh = modes[i]->vdisplay;

		if ((mw >= w) && (mh >= h)) {
			if (best < 0) {
				best = i;
			} else {
				if ((mw <= modes[best]->hdisplay) &&
						(mh <= modes[best]->vdisplay)) {
					if (mw != modes[best]->hdisplay)
						best = i;
					if (mh != modes[best]->vdisplay)
						best = i;
				}
			}
		}
	}

	if (best < 0) {
		ERROR(GLX, "cannot find suitable mode using XF86VidMode\n");
		XFree(modes);
		return FALSE;
	}

	VERBOSE(GLX, "best mode: mode %d:\n");
#define PELEMENT(s) VERBOSE(GLX, "\t"#s"=%d\n", modes[best]->s)
	PELEMENT(hdisplay);
	PELEMENT(hsyncstart);
	PELEMENT(hsyncend);
	PELEMENT(htotal);
	PELEMENT(hskew);
	PELEMENT(vdisplay);
	PELEMENT(vsyncstart);
	PELEMENT(vsyncend);
	PELEMENT(vtotal);
	PELEMENT(flags);
#undef PELEMENT

	if ((modes[best]->hdisplay == saved_modeline.hdisplay) &&
		(modes[best]->vdisplay == saved_modeline.vdisplay)) {
		/* We are already in an OK mode */
		XFree(modes);
		goto map_fs_window;
	}

	/* switch to best mode */

	/* Mouse position is very important */
	/* ungrab mouse pointer so we can move mouse around */
	XUngrabPointer(d, CurrentTime);
	XMoveCursorTo(d, _glx_ctx.root_win, 0, 0);

	res = XF86VidModeSwitchToMode(d, s, modes[best]);
	if (!res) {
		ERROR(GLX, "XF86VidModeSwitchToMode to mode %d failed\n", best);
		XFree(modes);
		return FALSE;
	}

	XSync(d, False);

	/* setup cleanup */
	xvid_saved_state.need_restore = TRUE;
	make_cleanup(&fs_cleanup);
	restore_fullscreen_cleanup = &fs_cleanup;

	TRACE(GLX, "XF86VidMode switch mode OK\n");
	XFree(modes);

	/* Get real resolusion */
	XF86VidModeModeLine real_mode;
	int unused;


	XMoveCursorTo(d, _glx_ctx.root_win, 0, 0);
	res = XF86VidModeGetModeLine(d, s, &unused, &real_mode);
	if (!res) {
		ERROR(GLX, "XF86VidMode: cannot get real mode after mode switch\n");
		CLEANUP(&fs_cleanup);
		return FALSE;
	}

	TRACE(GLX, "real mode:\n");
#define PELEMENT(s) VERBOSE(GLX, "\t"#s"=%d\n", real_mode.s)
	PELEMENT(hdisplay);
	PELEMENT(hsyncstart);
	PELEMENT(hsyncend);
	PELEMENT(htotal);
	PELEMENT(hskew);
	PELEMENT(vdisplay);
	PELEMENT(vsyncstart);
	PELEMENT(vsyncend);
	PELEMENT(vtotal);
	PELEMENT(flags);
#undef PELEMENT

map_fs_window:
	map_fullscreen(d, main_win, wm_win, fs_win,
			w, h, real_mode.hdisplay, real_mode.vdisplay);
	return TRUE;
#else
	return FALSE;
#endif
}


struct _xrandr_state {
	SizeID ori_size;
	Rotation ori_rotation;
	bool_t need_restore;
};

static struct _xrandr_state xrandr_saved_state = {
	.need_restore = FALSE,
};

static void
xrandr_cleanup(struct cleanup * _str)
{
	remove_cleanup(_str);
	if (!xrandr_saved_state.need_restore)
		return;

	TRACE(GLX, "xrandr cleanup\n");

	Display * d = _glx_ctx.display;

	if (d == NULL) {
		WARNING(GLX, "X11 connection has been destroied, reconnect.\n");
		d = XOpenDisplay(NULL);
		if (d == NULL) {
			FATAL(GLX, "reconnect failed, don't restore video mode\n");
			return;
		}
	}

	/* first, get sc */
	XRRScreenConfiguration * sc;
	Window root_win = _glx_ctx.root_win;
	sc = XRRGetScreenInfo (d, root_win);

	/* then, reset size */
	XRRSetScreenConfig(d, sc, root_win,
			xrandr_saved_state.ori_size,
			xrandr_saved_state.ori_rotation,
			CurrentTime);
	restore_fullscreen_cleanup = NULL;
	if (_glx_ctx.display == NULL)
			XCloseDisplay(d);
	return;
}

static bool_t
xrandr_fullscreen(Display * d, Window main_win, Window wm_win, Window fs_win)
{
#if HAVE_XRANDR
	TRACE(GLX, "Try xrandr fullscreen\n");
	/* Check for xrandr support */
	Bool res;
	bool_t retval = FALSE;

	int major_ver, minor_ver;
	res = XRRQueryVersion(d, &major_ver, &minor_ver);
	if (!res) {
		ERROR(GLX, "server doesn't support Xrandr\n");
		return FALSE;
	}

	TRACE(GLX, "Server reports RandR version %d.%d\n", major_ver,
			minor_ver);

	/* save current state */
	XRRScreenConfiguration * sc;
	Window root_win = _glx_ctx.root_win;
	sc = XRRGetScreenInfo (d, root_win);
	if (sc == NULL) {
		ERROR(GLX, "xrandr check screen info failed\n");
		return FALSE;
	}

	SizeID ori_size;
	Rotation ori_rotation;

	ori_size = XRRConfigCurrentConfiguration(sc, &ori_rotation);

	TRACE(GLX, "XRandr current setting:\n");
	TRACE(GLX, "\tsize: %d\n", ori_size);
	TRACE(GLX, "\trotation: %d\n", ori_rotation);

	XRRScreenSize *sizes;
	int nsizes;
	sizes = XRRConfigSizes(sc, &nsizes);
	if (sizes == NULL) {
		ERROR(GLX, "XRRConfSizes failed\n");
		goto free_sc;
	}


	int desired_w, desired_h;
	desired_w = _glx_ctx.base.base.width;
	desired_h = _glx_ctx.base.base.height;

	/* iterate over sizes, search the best size */
	/* copy from xrandr's code */
	int best_size = -1;
	int best_w, best_h;
	TRACE(GLX, " SZ:    Pixels          Physical\n");
	for (int i = 0; i < nsizes; i++) {
		int w, h;
		w = sizes[i].width;
		h = sizes[i].height;
		TRACE(GLX, "%c%-2d %5d x %-5d  (%4dmm x%4dmm )\n",
				i == ori_size ? '*' : ' ',
				i, w, h,
				sizes[i].mwidth, sizes[i].mheight);

		if ((w >= desired_w) && (h >= desired_h)) {
			if (best_size < 0) {
				best_size = i;
			} else {
				if ((w <= sizes[best_size].width) &&
						(h <= sizes[best_size].height)) {
					if (w != sizes[best_size].width)
						best_size = i;
					if (h != sizes[best_size].height)
						best_size = i;
				}
			}
		}
	}

	if (best_size == -1) {
		ERROR(GLX, "Cannot find best size with Xrandar\n");
		goto free_sc;
	}
	best_w = sizes[best_size].width;
	best_h = sizes[best_size].height;
	TRACE(GLX, "best size is %d x %d (number %d)\n",
			best_w, best_h, best_size);

	/* Check whether we really need switch mode */
	if (best_size == ori_size) {
		TRACE(GLX, "best size is current size, needn't switch mode\n");
		retval = TRUE;
		goto free_sc;
	}



	/* begin mode switch */
	xrandr_saved_state.ori_size = ori_size;
	xrandr_saved_state.ori_rotation = ori_rotation;
	xrandr_saved_state.need_restore = TRUE;
	static struct cleanup fs_cleanup = {
		.function = xrandr_cleanup,
	};
	make_cleanup(&fs_cleanup);
	restore_fullscreen_cleanup = &fs_cleanup;

	Status res_stat;
	res_stat = XRRSetScreenConfig(d, sc, root_win,
			best_size, ori_rotation, CurrentTime);
	XSync(d, True);
	XCheckError();
	if (res_stat != 0) {
		ERROR(GLX, "reset size with xrandr failed\n");
		goto free_sc;
	}
	retval = TRUE;

free_sc:
	XRRFreeScreenConfigInfo(sc);
	if (!retval)
		return FALSE;

	/* Map Windows */
	map_fullscreen(d, main_win, wm_win, fs_win,
			desired_w, desired_h, best_w, best_h);
	return TRUE;
#else
	return FALSE;
#endif
}


static bool_t
trival_fullscreen(Display * d, Window main_win, Window wm_win, Window fs_win)
{
	TRACE(GLX, "Try trival fullscreen\n");
	/* FIXME */
	/* Copy from gtk's code, it may be only used in gnome based WM? */
	/* I've tested a lot of method, this is the only one works prefect.
	 * Other method, such as reparent to fswin, and/or set override_redirect, 
	 * sometimes work, but occasionally (often) meet such problem:
	 *
	 * 1. freeze, only mouse move. from gdb attached debug, I believe there's
	 *    an race condition between printf (onto my gnome-terminal) and GrabInput
	 * 2. cannot receive input. only mouse motion and button can be intercepted,
	 *    kbd inputs go to gnome-terminal or my IM (fcitx)
	 * */
	Atom atom[2];
	atom[0] = XInternAtom(d, "_NET_WM_STATE", True);
	atom[1] = XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", True);
	XChangeProperty(d, wm_win, atom[0],
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *)(&atom[1]), 1);

	/* detect the W and H of the screen */
	int screen_w = DisplayWidth(d, _glx_ctx.screen);
	int screen_h = DisplayHeight(d, _glx_ctx.screen);
	int w = _glx_ctx.base.base.width;
	int h = _glx_ctx.base.base.height;

	w = w > screen_w ? screen_w : w;
	h = h > screen_w ? screen_w : h;

	XMoveWindow(d, main_win,
			(screen_w - w) / 2,
			(screen_h - h) / 2);
	XCheckError();
	XSync(d, False);
	map_window();

	/* don't set restore_fullscreen_cleanup to NULL.
	 * other fullscreen utils may call me */
	return TRUE;
}

static bool_t
enter_fullscreen(void)
{
	Display * d = _glx_ctx.display;
	Window main_win = _glx_ctx.main_win;
	Window wm_win = _glx_ctx.wm_win;
	Window fs_win = _glx_ctx.fs_win;
	bool_t succ;

	int fsengine = conf_get_integer("video.gl.glx.fullscreen.engine", 0);
	switch (fsengine) {
		case 0:
		case 1:
			succ = xrandr_fullscreen(d, main_win, wm_win, fs_win);
			if (succ)
				return TRUE;
		case 2:
			succ = xvid_fullscreen(d, main_win, wm_win, fs_win);
			if (succ)
				return TRUE;
		default:
			return trival_fullscreen(d, main_win, wm_win, fs_win);
	}

	return FALSE;
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

	bool_t confine = conf_get_bool("video.opengl.glx.confinemouse", FALSE);
	if (confine) {
		TRACE(GLX, "confine the mouse to main window\n");
		int err;
		err = XGrabPointer(d, main_win, True, 0,
				GrabModeAsync, GrabModeAsync, main_win,
				None, CurrentTime);
		if (err != GrabSuccess)
			WARNING(GLX, "Grab pointer failed\n");
		XRaiseWindow(d, wm_win);
		XCheckError();
	}

	bool_t grabkbd = conf_get_bool("video.opengl.glx.grabkbd", FALSE);
	if (grabkbd) {
		int err;
		WARNING(GLX, "Trying to grab keyborad, all WM hotkeys are disabled, including M-F4\n");
		err = XGrabKeyboard(d, wm_win, True,
				GrabModeAsync, GrabModeAsync, CurrentTime);
		if (err != GrabSuccess)
			WARNING(GLX, "Grab kbd failed\n");
		XCheckError();
	}
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

	intercept_signal(SIGPIPE);

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

	int succ = TRUE;
	/* if enter_fullscreen failed, it should restore everything */
	if (_glx_ctx.base.base.full_screen)
		succ = enter_fullscreen();

	if (!succ) {
		_glx_ctx.base.base.full_screen = FALSE;
		WARNING(GLX, "enter fullscreen failed, try window mode\n");
	}

	if (!_glx_ctx.base.base.full_screen)
		map_window();

	glx_ctx = &_glx_ctx;
	

#if 0
	/* code copy from 
	 * http://encelo.netsons.org/blog/2009/01/16/habemus-opengl-30/
	 * but doesn't work
	 * */
	/* Create OpenGL 3.0 context */
	bool_t gl3ctx = conf_get_bool("video.opengl.gl3context", FALSE);
	/* I use while only because I want to use break */
	do {
		if (!gl3ctx)
			break;

		TRACE(GLX, "Try to create a gl3 context\n");
		if (glXCreateContextAttribsARB == NULL) {
			WARNING(GLX, "no glXCreateContextAttribsARB\n");
			break;
		}
#ifndef GLX_CONTEXT_MAJOR_VERSION_ARB
# define GLX_CONTEXT_MAJOR_VERSION_ARB (0x2091)
#endif

#ifndef GLX_CONTEXT_MINOR_VERSION_ARB
# define GLX_CONTEXT_MINOR_VERSION_ARB (0x2092)
#endif
		WARNING(GLX, "Create OpenGL 3 context doesn't support now\n");

Display *dpy;
GLXDrawable draw, read;
GLXContext ctx, ctx3;
GLXFBConfig *cfg;
int nelements;
int attribs[]= {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    0
};

ctx = glXGetCurrentContext();
dpy = glXGetCurrentDisplay();
draw = glXGetCurrentDrawable();
read = glXGetCurrentReadDrawable();
cfg = glXGetFBConfigs(dpy, 0, &nelements);
ctx3 = glXCreateContextAttribsARB(dpy, *cfg, 0, 1, attribs);
glXMakeContextCurrent(dpy, draw, read, ctx3);
glXDestroyContext(dpy, ctx);
XSync(dpy, False);
XCheckError();

	} while(0);
#endif
	return &glx_ctx->base;
}

void
gl_close(void)
{
	glx_cleanup(&glx_cleanup_str);
}

Display *
x_get_display(void)
{
	assert(glx_ctx);
	return _glx_ctx.display;
}

Window
x_get_main_window(void)
{
	assert(glx_ctx);
	return _glx_ctx.main_win;
}

int
x_get_screen(void)
{
	assert(glx_ctx);
	return _glx_ctx.screen;
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

