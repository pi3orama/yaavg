/* 
 * gl3_fix.h
 * by WN @ Jun 09, 2009
 *
 * gl3 fixup
 */

#ifndef GL3_FIX_H
#define GL3_FIX_H


#ifndef GLX_ARB_create_context
#define GLX_ARB_create_context 1

extern GLXContext glXCreateContextAttribsARB (Display *, GLXFBConfig, GLXContext, Bool, const int *);
typedef GLXContext ( * PFNGLXCREATECONTEXTATTRIBSARBPROC) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);

#endif /* GLX_ARB_create_context */

#endif

