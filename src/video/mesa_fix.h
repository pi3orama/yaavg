/* 
 * mesa_fix.h
 * by WN @ May 31, 2009
 *
 * some mesa extensions, NV doesn't have them
 */

#ifndef MESA_FIX_H
#define MESA_FIX_H

/* copy from mesa's glx.h */

#ifndef GLX_MESA_swap_control
#define GLX_MESA_swap_control 1

extern int glXSwapIntervalMESA(unsigned int interval);
extern int glXGetSwapIntervalMESA(void);

typedef int (*PFNGLXSWAPINTERVALMESAPROC)(unsigned int interval);
typedef int (*PFNGLXGETSWAPINTERVALMESAPROC)(void);

#endif /* GLX_MESA_swap_control */

#endif

