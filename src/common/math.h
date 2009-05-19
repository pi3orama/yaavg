/* 
 * math.h
 * by WN @ May. 19, 2009
 *
 * Use this std filename to indicate that it is just a
 * wrapper for std math.h. Use this wrapper because there's
 * namespace pullution: struct exception
 */

#ifndef __cplusplus
# define exception math_exception
#endif
#include <math.h>
#ifndef __cplusplus
# undef exception
#endif


