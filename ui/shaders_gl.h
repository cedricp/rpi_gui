#ifndef SHADERS_GL_H
#define SHADERS_GL_H

const char* vertex_shader_src =
	"attribute vec3		position;\n"
	"attribute vec2		st;\n"
	"uniform mat4		projection_matrix;\n"
	"uniform mat4		model_matrix;\n"
	"uniform vec4		input_color;\n"
	"varying vec4		out_fragment_color;\n"
	"varying vec2		out_st_coord_frag;\n"
	"void main(void) {\n"
	"   out_st_coord_frag = st;\n"
	"   out_fragment_color = input_color;"
	"   gl_Position = projection_matrix * model_matrix * vec4(position,1);\n"
	"}\n";

const char* vertex_color_shader_src =
	"attribute vec3		position;\n"
	"attribute vec2		st;\n"
	"attribute vec4		input_color;\n"
	"uniform mat4		projection_matrix;\n"
	"uniform mat4		model_matrix;\n"
	"varying vec4		out_fragment_color;\n"
	"varying vec2		out_st_coord_frag;\n"
	"void main(void) {\n"
	"   out_st_coord_frag = st;\n"
	"   out_fragment_color = input_color;"
	"   gl_Position = projection_matrix * model_matrix * vec4(position,1);\n"
	"}\n";

const char* vertex_shader_simple_src =
	"attribute vec3		position;\n"
	"uniform mat4		projection_matrix;\n"
	"uniform mat4		model_matrix;\n"
	"uniform vec4		input_color;\n"
	"varying vec4		out_fragment_color;\n"
	"void main(void) {\n"
	"   out_fragment_color = input_color;"
	"   gl_Position = projection_matrix * model_matrix * vec4(position,1);\n"
	"}\n";

const char * frag_shader_src =
	"precision mediump  float;\n"
	"uniform sampler2D	in_texture_sampler;\n"
	"varying vec2 		out_st_coord_frag;\n"
	"varying vec4 		out_fragment_color;\n"
	"void main()\n"
	"{\n"
	"    vec4 tex_color = texture2D(in_texture_sampler, out_st_coord_frag);\n"
	"    gl_FragColor = out_fragment_color * tex_color;\n"
	"}\n";

const char * frag_shader_font_src =
	"precision mediump  float;\n"
	"uniform sampler2D	in_texture_sampler;\n"
	"varying vec2 		out_st_coord_frag;\n"
	"varying vec4 		out_fragment_color;\n"
	"void main()\n"
	"{\n"
	"    float tex_alpha = texture2D(in_texture_sampler, out_st_coord_frag)[3];\n"
	"    gl_FragColor = vec4(out_fragment_color.xyz * tex_alpha, tex_alpha);\n"
	"}\n";

const char * frag_shader_solid_src =
	"precision mediump  float;\n"
	"varying vec4 		out_fragment_color;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = out_fragment_color;\n"
	"}\n";

#endif
