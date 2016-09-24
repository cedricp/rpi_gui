#ifndef SHADERS_GL_H
#define SHADERS_GL_H

const char* vertex_shader_src =
	"attribute vec3		position;\n"
	"attribute vec2		st;\n"
	"uniform mat4		mvp;\n"
	"uniform mat4		xform;\n"
	"uniform vec4		ucolor;\n"
	"varying vec4		fcolor;\n"
	"varying vec2		frag_uv;\n"
	"void main(void) {\n"
	"       frag_uv = st;\n"
	"	fcolor = ucolor;"
	"       gl_Position = mvp * xform * vec4(position,1);\n"
	"}\n";

const char * frag_shader_src =
	"precision mediump  float;\n"
	"uniform sampler2D	texture_uniform;\n"
	"varying vec2 		frag_uv;\n"
	"varying vec4 		fcolor;\n"
	"void main()\n"
	"{\n"
	"    vec4 tex_color = texture2D(texture_uniform, frag_uv);\n"
	"    gl_FragColor = fcolor * tex_color[3];\n"
	"}\n";

#endif
