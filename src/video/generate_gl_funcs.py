#!/bin/env python
import sys, os

gl_prefix="_"

class func:
	name = ""
	def __init__(self, name):
		self.name = name;

	def output_prototype(self):
		print '\ttypeof(%s) * %s%s;' % (self.name, gl_prefix, self.name)
	
	def output_macro_static(self):
		pass
	
	def output_macro_dynamic(self):
		print '# define %s (GLOPS->%s%s)' % (self.name, gl_prefix, self.name)


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


# At first, check and load 'gl_funcs_list'

filename = sys.argv[1]

if not os.path.isfile(filename):
	print "gl_funcs_list not found"
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
print "#ifndef __GL_GLFUNCS_H"
print "#define __GL_GLFUNCS_H"
print "#include <GL/gl.h>"
print "#include <GL/glext.h>"
print ""
print "struct GLFuncs {"
for o in objects:
	o.output_prototype()
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

