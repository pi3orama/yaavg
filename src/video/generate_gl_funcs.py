#!/bin/env python
import sys, os

gl_prefix="_"
gl_or_glx="gl"

class func:
	name = ""
	def __init__(self, name):
		self.name = name;

	def output_prototype(self):
		print '\ttypeof(%s)\t * %s%s;' % (self.name, gl_prefix, self.name)
	
	def output_macro_static(self):
		pass
	
	def output_macro_dynamic(self):
		print '# define %s ((GLOPS)->%s%s)' % (self.name, gl_prefix, self.name)
	
	def output_init_list(self):
		print "\t{\"%s\", (void**)&((GLOPS)->%s%s)}," % (self.name, gl_prefix, self.name)


class comment:
	comm = ""
	def __init__(self, comm):
		self.comm = comm.lstrip('-');

	def output_prototype(self):
		print '\t/* %s */ ' % (self.comm)
	
	def output_macro_static(self):
		print '/* %s */ ' % (self.comm)

	def output_macro_dynamic(self):
		print '/* %s */ ' % (self.comm)
	def output_init_list(self):
		print '/* %s */ ' % (self.comm)


# At first, check and load 'gl_funcs_list'

filename = sys.argv[1]
if (len(sys.argv) >= 3):
	gl_or_glx = sys.argv[2]

if not os.path.isfile(filename):
	print "funcs list not found"
	sys.exit(1)

# Open and read that file
file = open(filename)

# read each line
objects = []
for line in file:
	line = line.replace("\n", "")
	if len(line) is 0:
		continue
	if line[0] is '#':
		continue
	if line[0] is '-':
		objects.append(comment(line))
		continue
	objects.append(func(line))

# print header
if gl_or_glx == "gl":
	print "#ifndef INIT_GL_FUNC_LIST"
	print "#ifndef __GL_GLFUNCS_H"
	print "#define __GL_GLFUNCS_H"
	print "#define GL_GLEXT_PROTOTYPES"
	print "#include <GL/gl.h>"
	print "#include <GL/glu.h>"
	print "#include <GL/glut.h>"
	print "#include <GL/glext.h>"
	print ""
	print "struct gl_funcs {"
elif gl_or_glx == "glx":
	print "#ifndef INIT_GLX_FUNC_LIST"
	print "#ifndef __GLX_GLFUNCS_H"
	print "#define __GLX_GLFUNCS_H"
	print "#define GLX_GLXEXT_PROTOTYPES"
	print "#include <GL/glx.h>"
	print "#include <GL/glxext.h>"
	print ""
	print "struct glx_funcs {"

print "#ifndef STATIC_OPENGL"
for o in objects:
	o.output_prototype()
print "#endif"
print "};"


print "#ifdef STATIC_OPENGL"
for o in objects:
	o.output_macro_static()
print "#else"
print "/* GLOPS is a macro defined in engine_gl.h */"

for o in objects:
	o.output_macro_dynamic()
print "#endif"

print "#endif"

print "#else"
print "struct glfunc_init_item {const char * name; void ** func;};"
print "struct glfunc_init_item gl_func_init_list[] = {"
for o in objects:
	o.output_init_list()
print "\t{NULL, NULL}"
print "};"
print "#endif"

