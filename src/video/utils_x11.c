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
#define BUFSIZ	(2048)

void
XPrintDefaultError(Display * dpy, XErrorEvent * event)
{

	char buffer[BUFSIZ];
	char mesg[BUFSIZ];
	char number[32];
	char *mtype = "XlibMessage";


	XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
	XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
	WARNING(VIDEO, "%s:  %s\n", mesg, buffer);

	XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", 
			mesg, BUFSIZ);
	WARNING(VIDEO, mesg, event->request_code);

	if (event->request_code < 128) {
		sprintf(number, "%d", event->request_code);
		XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
	} else {
		const char str[] = "Extension error\n";
		strncpy(buffer, str, sizeof(str));
	}

	WARNING_CONT(VIDEO, " (%s)\n", buffer);


	if (event->request_code >= 128) {
		XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
				mesg, BUFSIZ);
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
					mesg, BUFSIZ);
		else if (event->error_code == BadAtom)
			XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
					mesg, BUFSIZ);
		else
			XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
					mesg, BUFSIZ);
		WARNING(VIDEO, "  ");
		WARNING_CONT(VIDEO, mesg, event->resourceid);
		WARNING_CONT(VIDEO, "\n");
	}

	XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
			mesg, BUFSIZ);
	WARNING(VIDEO, "  ");
	WARNING_CONT(VIDEO, mesg, event->serial);
	WARNING_CONT(VIDEO, "\n");
	return;
}

// vim:tabstop=4:shiftwidth=4

#endif /* VIDEO_OPENGL_GLX_DRIVER */
