/* 
 * engine.c
 * by WN @ Feb 03, 2009
 */

#include <common/debug.h>
#include <econfig/econfig.h>
#include <video/engine.h>

#include <video/engine.h>
#include <video/engine_driver.h>

/* 
 * engine.c implentment the interfaces defined in engine.h.
 *
 * If the only operations is Init, OpenWindow, Close..., then there's no
 * need for a specific 'engine.c', just implentment them in engine_driver.c
 * is OK. However, the render list operations and plugin operations are
 * important and shared by all driver. Therefore we use a separate engine.c.
 */


