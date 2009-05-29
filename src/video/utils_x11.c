/* 
 * by WN @ May 30, 2009
 */

#include <config.h>
#ifdef VIDEO_OPENGL_GLX_DRIVER
#include <common/debug.h>
#include <X11/Xlib.h>
#include <video/utils_x11.h>

#include <stdio.h>
/* Copy from XLibInt.c of Xlib's code */
#define XBUFSIZ	(2048)

void
XPrintDefaultError(Display * dpy, XErrorEvent * event)
{

	char buffer[XBUFSIZ];
	char mesg[XBUFSIZ];
	char number[32];
	char *mtype = "XlibMessage";


	XGetErrorText(dpy, event->error_code, buffer, XBUFSIZ);
	XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, XBUFSIZ);
	WARNING(VIDEO, "%s:  %s\n", mesg, buffer);

	XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", 
			mesg, XBUFSIZ);
	WARNING(VIDEO, mesg, event->request_code);

	if (event->request_code < 128) {
		sprintf(number, "%d", event->request_code);
		XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, XBUFSIZ);
	} else {
		const char str[] = "Extension error\n";
		strncpy(buffer, str, sizeof(str));
	}

	WARNING_CONT(VIDEO, " (%s)\n", buffer);


	if (event->request_code >= 128) {
		XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
				mesg, XBUFSIZ);
		WARNING(VIDEO, "  ");
		WARNING_CONT(VIDEO, mesg, event->minor_code);
		WARNING_CONT(VIDEO, "\n");
	}

	if ((event->error_code == BadWindow) ||
			(event->error_code == BadPixmap) ||
			(event->error_code == BadCursor) ||
			(event->error_code == BadFont) ||
			(event->error_code == BadDrawable) ||
			(event->error_code == BadColor) ||
			(event->error_code == BadGC) ||
			(event->error_code == BadIDChoice) ||
			(event->error_code == BadValue) ||
			(event->error_code == BadAtom)) {
		if (event->error_code == BadValue)
			XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
					mesg, XBUFSIZ);
		else if (event->error_code == BadAtom)
			XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
					mesg, XBUFSIZ);
		else
			XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
					mesg, XBUFSIZ);
		WARNING(VIDEO, "  ");
		WARNING_CONT(VIDEO, mesg, event->resourceid);
		WARNING_CONT(VIDEO, "\n");
	}

	XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
			mesg, XBUFSIZ);
	WARNING(VIDEO, "  ");
	WARNING_CONT(VIDEO, mesg, event->serial);
	WARNING_CONT(VIDEO, "\n");
	return;
}

void
XWaitMapped(Display * dpy, Window win)
{
    XEvent event;
    do {
        XMaskEvent(dpy, StructureNotifyMask, &event);
    } while ( (event.type != MapNotify) || (event.xmap.event != win) );
}

void
XWaitUnmapped(Display * dpy, Window win)
{
    XEvent event;
    do {
        XMaskEvent(dpy, StructureNotifyMask, &event);
    } while ( (event.type != UnmapNotify) || (event.xmap.event != win) );
}


// vim:tabstop=4:shiftwidth=4

#endif /* VIDEO_OPENGL_GLX_DRIVER */
