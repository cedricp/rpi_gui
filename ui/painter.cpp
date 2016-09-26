#include "painter.h"
#include "bbox.h"
#include "string_utils.h"
#include "shaders_gl.h"

#include "nanosvg.h"
#include "nanosvgrast.h"

#include "fontlib/texture-font.h"

#ifdef USE_OPENGL
#include <GL/gl.h>
#else
#include <GLES2/gl2.h>
#endif

#include <stdlib.h>
#include <map>
#include <vector>
#include <stdio.h>
#include <unistd.h>

struct Img_info{
	Img_info(){}
	Img_info(GLuint id, int w, int h){
		texid = id;
		width = w;
		height = h;
	}
	Img_info(const Img_info& info){
		texid = info.texid;
		width = info.width;
		height = info.height;
	}
	GLuint texid;
	int width, height;
};

struct Font_info{
	Font_info(){
		atlas = NULL;
		font  = NULL;
	}
	texture_atlas_t *atlas;
	texture_font_t *font;
	std::string    name;
	int size, atlas_size;
};

struct FontImpl{
	FontImpl(){
		text_vector = NULL;
	}
	~FontImpl(){
		if(text_vector)
			vector_delete(text_vector);
	}
	vector_t* text_vector;
	Font_info finfo;
};

#ifndef USE_OPENGL
struct gl_program
{
	GLint	matrix_projection, matrix_model;
	GLuint	program_handle;
	int 	vertex_handle, texture_handle, sampler_handle, color_handler;
};

#endif

struct PImpl{
	std::map< std::string, Img_info > textures;
	std::vector< Font_info > fonts;
	unsigned int default_font_idx;
#ifndef USE_OPENGL
	gl_program texture_program, font_program, solid_program;
#endif
};

Text_data::Text_data()
{
	data = new FontImpl;
}

Text_data::~Text_data(){
	if (data->text_vector)
		vector_delete(data->text_vector);
	delete data;
};


void generate_text(Text_data& td)
{
	size_t i;
	int start_x_mem = 0;
	int start_x = 0;
	int start_y = 0;
	empty_bbox(td.bbox);

	for( i=0; i< td.text.size(); ++i )
	{
		if (td.text[i] == '\n'){
			start_x = start_x_mem;
			start_y += td.data->finfo.size;
		}
		texture_glyph_t *glyph = texture_font_get_glyph( td.data->finfo.font, td.text[i] );
		if( glyph != NULL )
		{
			int kerning = 0;
			if( i > 0)
			{
				kerning = texture_glyph_get_kerning( glyph, td.text[i-1] );
			}
			start_x += kerning;
			int x0  = (int)( start_x + glyph->offset_x );
			int y0  = (int)( start_y  );
			int x1  = (int)( x0 + glyph->width );
			int y1  = (int)( y0 - glyph->height );
			float s0 = glyph->s0;
			float t0 = glyph->t1;
			float s1 = glyph->s1;
			float t1 = glyph->t0;

			td.bbox.extend(x0, y0);
			td.bbox.extend(x1, y1);

			// data is x,y,z,s,t
			GLfloat vertices[] = {
			  x0,y0,
			  s0,t0,
			  x0,y1,
			  s0,t1,
			  x1,y1,
			  s1,t1,
			  x0,y0,
			  s0,t0,
			  x1,y1,
			  s1,t1,
			  x1,y0,
			  s1,t0,
			};

			vector_push_back_data( td.data->text_vector, vertices, 6*4);

			start_x += glyph->advance_x;
		}
	}
}

Painter::Painter()
{
	m_impl = new PImpl;
#ifndef USE_OPENGL
	init_gles2();
#endif
	std::string font_file;
	std::string font_name = "custom.ttf";
	if( locate_resource(font_name, font_file) ){
		m_impl->default_font_idx = load_fonts(font_file, 14);
	} else {
		std::cerr << "Painter::Painter : cannot initialize default fonts " << font_name << std::endl;
	}
}

Painter::~Painter()
{
	delete m_impl;
}
#ifndef USE_OPENGL

static GLuint
load_shader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	// Create the shader object
	shader = glCreateShader(type);
	if(shader == 0)
		return 0;
	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);

	// Compile the shader
	glCompileShader(shader);
	// Check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if(!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if(infoLen > 1)
		{
			char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

void
Painter::init_gles2()
{
	GLuint vertex_shader 		 = load_shader(GL_VERTEX_SHADER, vertex_shader_src);
	GLuint vertex_shader_simple	 = load_shader(GL_VERTEX_SHADER, vertex_shader_src);
	GLuint fragment_shader 		 = load_shader(GL_FRAGMENT_SHADER, frag_shader_src);
	GLuint fragment_shader_font  = load_shader(GL_FRAGMENT_SHADER, frag_shader_font_src);
	GLuint fragment_shader_solid = load_shader(GL_FRAGMENT_SHADER, frag_shader_solid_src);

	m_impl->texture_program.program_handle 	= glCreateProgram();
	m_impl->font_program.program_handle 	= glCreateProgram();
	m_impl->solid_program.program_handle 	= glCreateProgram();

	if(m_impl->texture_program.program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'texture'" << std::endl;
		return;
	}

	if(m_impl->font_program.program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'font'" << std::endl;
		return;
	}

	if(m_impl->solid_program.program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'solid'" << std::endl;
		return;
	}

	// Normal texture shader
	glAttachShader(m_impl->texture_program.program_handle, vertex_shader);
	glAttachShader(m_impl->texture_program.program_handle, fragment_shader);

	glLinkProgram(m_impl->texture_program.program_handle);

	m_impl->texture_program.matrix_projection 	= glGetUniformLocation(m_impl->texture_program.program_handle, "mvp");
	m_impl->texture_program.matrix_model 		= glGetUniformLocation(m_impl->texture_program.program_handle, "xform");
	m_impl->texture_program.vertex_handle  		= glGetAttribLocation (m_impl->texture_program.program_handle, "position" );
	m_impl->texture_program.texture_handle 		= glGetAttribLocation (m_impl->texture_program.program_handle, "st" );
	m_impl->texture_program.sampler_handle  	= glGetUniformLocation(m_impl->texture_program.program_handle, "texture_uniform" );
	m_impl->texture_program.color_handler		= glGetUniformLocation(m_impl->texture_program.program_handle, "ucolor" );


	// Font (alpha) texture shader
	glAttachShader(m_impl->font_program.program_handle, vertex_shader);
	glAttachShader(m_impl->font_program.program_handle, fragment_shader_font);

	glLinkProgram(m_impl->font_program.program_handle);

	m_impl->font_program.matrix_projection 	= glGetUniformLocation(m_impl->font_program.program_handle, "mvp");
	m_impl->font_program.matrix_model 		= glGetUniformLocation(m_impl->font_program.program_handle, "xform");
	m_impl->font_program.vertex_handle  	= glGetAttribLocation (m_impl->font_program.program_handle, "position" );
	m_impl->font_program.texture_handle 	= glGetAttribLocation (m_impl->font_program.program_handle, "st" );
	m_impl->font_program.sampler_handle	 	= glGetUniformLocation(m_impl->font_program.program_handle, "texture_uniform" );
	m_impl->font_program.color_handler		= glGetUniformLocation(m_impl->font_program.program_handle, "ucolor" );

	// Solid texture shader
	glAttachShader(m_impl->solid_program.program_handle, vertex_shader_simple);
	glAttachShader(m_impl->solid_program.program_handle, fragment_shader_solid);

	glLinkProgram(m_impl->solid_program.program_handle);

	m_impl->solid_program.matrix_projection = glGetUniformLocation(m_impl->solid_program.program_handle, "mvp");
	m_impl->solid_program.matrix_model 		= glGetUniformLocation(m_impl->solid_program.program_handle, "xform");
	m_impl->solid_program.vertex_handle  	= glGetAttribLocation (m_impl->solid_program.program_handle, "position" );
	m_impl->solid_program.color_handler		= glGetUniformLocation(m_impl->solid_program.program_handle,  "ucolor" );
}
#endif

void
Painter::viewport(int x,int y,int width, int height)
{
	glViewport( x, y, width, height );
}

void
Painter::use_default_gles_program()
{
#ifndef USE_OPENGL
	glUseProgram(m_impl->texture_program.program_handle);
#endif
}

void Painter::create_perspective_matrix(float aspect_ratio, float fovy_degrees, float znear, float zfar, Matrix outmatrix)
{
	float temp, temp2, temp3, temp4;

	float top 		= znear * tanf(fovy_degrees * M_PI / 360.0);
	float bottom 	= -top;
	float left 		= bottom * aspect_ratio;
	float right 	= top * aspect_ratio;

	temp 	= 2.0 * znear;
	temp2 	= right - left;
	temp3 	= top - bottom;
	temp4 	= zfar - znear;

	outmatrix[0] = temp / temp2;
	outmatrix[1] = 0.0;
	outmatrix[2] = 0.0;
	outmatrix[3] = 0.0;
	outmatrix[4] = 0.0;
	outmatrix[5] = temp / temp3;
	outmatrix[6] = 0.0;
	outmatrix[7] = 0.0;
	outmatrix[8] = (right + left) / temp2;
	outmatrix[9] = (top + bottom) / temp3;
	outmatrix[10] = (-zfar - znear) / temp4;
	outmatrix[11] = -1.0;
	outmatrix[12] = 0.0;
	outmatrix[13] = 0.0;
	outmatrix[14] = (-temp * zfar) / temp4;
	outmatrix[15] = 0.0;
}

void
Painter::create_ortho_matrix(float left, float right,
							 float bottom, float top,
							 float near, float far, Matrix outmatrix )
{
    GLfloat r_l = right - left;
    GLfloat t_b = top - bottom;
    GLfloat f_n = far - near;
    GLfloat tx = - (right + left) / (right - left);
    GLfloat ty = - (top + bottom) / (top - bottom);
    GLfloat tz = - (far + near) / (far - near);

    outmatrix[0] = 2.0f / r_l;
    outmatrix[4] = 0.0f;
    outmatrix[8] = 0.0f;
    outmatrix[12] = tx;

    outmatrix[1] = 0.0f;
    outmatrix[5] = 2.0f / t_b;
    outmatrix[9] = 0.0f;
    outmatrix[13] = ty;

    outmatrix[2] = 0.0f;
    outmatrix[6] = 0.0f;
    outmatrix[10] = -2.0f / f_n;
    outmatrix[14] = tz;

    outmatrix[3] = 0.0f;
    outmatrix[7] = 0.0f;
    outmatrix[11] = 0.0f;
    outmatrix[15] = 1.0f;
}

void
Painter::load_projection_matrix(Matrix matrix)
{
#ifdef USE_OPENGL
	glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrix);
#else
    glUniformMatrix4fv(m_impl->texture_program.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
    glUniformMatrix4fv(m_impl->font_program.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
    glUniformMatrix4fv(m_impl->solid_program.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
#endif
}

void
Painter::load_model_matrix(Matrix matrix)
{
#ifdef USE_OPENGL
	glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(matrix);
#else
    glUniformMatrix4fv(m_impl->texture_program.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
    glUniformMatrix4fv(m_impl->font_program.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
    glUniformMatrix4fv(m_impl->solid_program.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
#endif
}

void
Painter::load_identity(Matrix matrix)
{
	matrix4_identity(matrix);
}

void
Painter::scissor_begin(int x, int y, int width,	int height)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
}

void
Painter::scissor_end()
{
	glDisable(GL_SCISSOR_TEST);
}

void
Painter::clear_color_buffer(const FColor&c)
{
	glClearColor(c.red(), c.green(), c.blue(), c.alpha());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
Painter::color(const FColor& c)
{
#ifdef USE_OPENGL
	glColor4f(c.red(), c.green(), c.blue(), c.alpha());
#else
	glUniform4f(m_impl->texture_program.color_handler, c.red(), c.green(), c.blue(), c.alpha());
	glUniform4f(m_impl->font_program.color_handler, c.red(), c.green(), c.blue(), c.alpha());
	glUniform4f(m_impl->solid_program.color_handler, c.red(), c.green(), c.blue(), c.alpha());
#endif
}

char*
Painter::load_svg_image(std::string filename, int &w, int &h)
{
	std::string fullpath;
	if (!locate_resource(filename, fullpath)){
		fullpath = filename;
	}

	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;

	image = nsvgParseFromFile(fullpath.c_str(), "px", 96.0f);
	if ( image == NULL )
		return NULL;

	w = image->width;
	h = image->height;

	rast = nsvgCreateRasterizer();
	if ( rast == NULL )
		goto error;

	img = (unsigned char*)malloc(w*h*4);

	if ( img == NULL)
		goto error;

	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);

error:
	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	return (char*)img;
}

unsigned int
Painter::create_texture_svg(std::string name, std::string filename)
{
	if (m_impl->textures.find(name) != m_impl->textures.end()){
		return m_impl->textures[name].texid;
	}

	int w, h;
	char* img;
	img = load_svg_image(filename, w, h);
	if(!img){
		std::cerr << "Painter::create_texture_svg : cannot load image file " << filename << std::endl;
		return 0;
	}
	int texid = create_texture(name, img, w, h);

	free(img);
	return texid;
}

unsigned int
Painter::create_texture(std::string name, const char* img, int w, int h, TextureMode mode)
{
	if (m_impl->textures.find(name) != m_impl->textures.end()){
		return m_impl->textures[name].texid;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (mode == TEXTURE_RGBA)
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	else if (mode == TEXTURE_ALPHA)
		glTexImage2D(GL_TEXTURE_2D, 0,GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, img);

	m_impl->textures[name] = Img_info(textureID, w, h);
	return textureID;
}

void
Painter::delete_texture(unsigned int idx){
	std::map<std::string, Img_info>::iterator it;

	for (it = m_impl->textures.begin(); it != m_impl->textures.end(); ++it){
		if (it->second.texid == idx)
			break;
	}
	if (it == m_impl->textures.end()){
		std::cerr << "Painter::delete_texture : cannot delete texture #" << idx << std::endl;
		return;
	}

	glDeleteTextures( 1, &idx );
	m_impl->textures.erase(it);
}

void
Painter::delete_texture(std::string name){
	std::map<std::string, Img_info>::iterator it;
	it = m_impl->textures.find(name);

	if (it == m_impl->textures.end()){
		std::cerr << "Painter::delete_texture : cannot delete texture " << name << std::endl;
		return;
	}

	glDeleteTextures(1, &it->second.texid);
	m_impl->textures.erase(it);
}

void
Painter::use_texture(unsigned int texid)
{
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, texid);

	glEnable(GL_TEXTURE_2D);
#ifndef USE_OPENGL
	glUniform1i ( m_impl->texture_program.sampler_handle, 0);
	glUniform1i ( m_impl->font_program.sampler_handle, 0);
#endif
}

void
Painter::enable_alpha(bool on)
{
	if (on){
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

void
Painter::disable_texture()
{
	glDisable(GL_TEXTURE_2D);
}

bool
Painter::use_texture(std::string name)
{
	if (m_impl->textures.find(name) == m_impl->textures.end()){
		return false;
	}
	use_texture(m_impl->textures[name].texid);
	return true;
}

void
Painter::texture_size(std::string name, int& w, int& h)
{
	if (m_impl->textures.find(name) == m_impl->textures.end()){
		std::cerr << "Painter::use_texture : Texture " << name << " does not exist " << m_impl->textures.size() << std::endl;
		return;
	}
	w = m_impl->textures[name].width;
	h = m_impl->textures[name].height;
}


void
Painter::draw_quad(int x, int y, int width, int height, bool fill)
{
#ifdef USE_OPENGL
	height += y;
	width  += x;
	if (!fill){
		glBegin(GL_LINE_LOOP);
		glVertex2f(x, y);
		glVertex2f(x , height);
		glVertex2f(width, height);
		glVertex2f(width, y);
		glVertex2f(x, y);
		glEnd();
	} else {
		glPolygonMode(GL_FRONT, GL_FILL);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(0, 1);
		glVertex2f(x , height);
		glTexCoord2f(1, 1);
		glVertex2f(width, height);
		glTexCoord2f(1, 0);
		glVertex2f(width, y);
		glEnd();
	}
#else
	float xx = x;
	float yy = y;
	float ww = width;
	float hh = height;
	glDisable(GL_CULL_FACE);

	glUseProgram(m_impl->solid_program.program_handle);
	if (!fill){
		GLfloat gl_data[] = {  xx, yy, 0., 0.,
			               	   xx, hh, 0., 1.,
							   ww, hh, 1., 1.,
							   ww, yy, 1., 0.};

		glVertexAttribPointer ( m_impl->solid_program.vertex_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLfloat*)gl_data );
		glEnableVertexAttribArray ( m_impl->solid_program.vertex_handle );

		glUniform1i ( m_impl->solid_program.sampler_handle, 0);

		glDrawArrays ( GL_LINE_LOOP, 0, 4 );
	} else {
		GLfloat gl_data[] = {  xx, yy, 0., 0.,
							   xx, hh, 0., 1.,
							   ww, yy, 1., 0.,
							   xx, hh, 0., 1.,
							   ww, hh, 1., 1.,
							   ww, yy, 1., 0.};

		glVertexAttribPointer ( m_impl->texture_program.vertex_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLfloat*)gl_data );
		glEnableVertexAttribArray ( m_impl->texture_program.vertex_handle );
		glVertexAttribPointer ( m_impl->texture_program.texture_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), ((GLfloat*)gl_data) +2 );
		glEnableVertexAttribArray ( m_impl->texture_program.texture_handle );

		glUniform1i ( m_impl->texture_program.sampler_handle, 0);

		glDrawArrays ( GL_TRIANGLES, 0, 6 );
	}
#endif
}

int
Painter::load_fonts(std::string font_filename, int font_size, int atlas_size)
{
	Font_info finfo;
	std::string font_name = string_basename(font_filename);

	for(int i = 0; i < m_impl->fonts.size(); ++i){
		if (m_impl->fonts[i].name == font_filename)
			return i;
	}

	finfo.atlas = texture_atlas_new( atlas_size, atlas_size, 1 );
	if (finfo.atlas == NULL){
		std::cerr << "Painter::load_fonts : cannot create atlas for fonts " << font_name << std::endl;
		return -1;
	}

	finfo.font = texture_font_new( finfo.atlas, font_filename.c_str(), font_size );
	if (finfo.font == NULL){
		std::cerr << "Painter::load_fonts : cannot create fonts " << font_name << std::endl;
		return -1;
	}

	texture_font_load_glyphs( finfo.font, L" !\"#$%&'()*+,-./0123456789:;<=>?"
		    							  L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
										  L"`abcdefghijklmnopqrstuvwxyz{|}~");

	finfo.size 			= font_size;
	finfo.atlas_size 	= atlas_size;
	finfo.name 			= font_name;

	m_impl->fonts.push_back(finfo);

	return m_impl->fonts.size() - 1;
}

int
Painter::font_by_name(std::string name)
{
	for(int i = 0; i < m_impl->fonts.size(); ++i){
		if (m_impl->fonts[i].name == name)
			return i;
	}
	return -1;
}

void
Painter::remove_fonts(int idx)

{
	if (idx >= m_impl->fonts.size()){
		std::cerr << "Painter::remove_fonts : font index error : " << idx << std::endl;
		return;
	}

	texture_atlas_delete(m_impl->fonts[idx].atlas);
	texture_font_delete(m_impl->fonts[idx].font);

	m_impl->fonts.erase(m_impl->fonts.begin() + idx);
}

void
Painter::build_text(int font_id, std::string text, int start_x,int start_y, Text_data& data)
{
	data.data->finfo = m_impl->fonts[font_id];
	data.text = text;
	if (data.data->text_vector)
		vector_delete(data.data->text_vector);
	data.data->text_vector = vector_new(sizeof(GLfloat));
	generate_text(data);
}

void
Painter::draw_text(const Text_data& data)
{
	if (data.data->text_vector == NULL)
		return;

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, data.data->finfo.atlas->id);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	vector_t *vVector = data.data->text_vector;


#ifdef USE_OPENGL
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < vVector->size / 4; ++i){
		float vx = *((float*)vVector->items + (i * 4));
		float vy = *((float*)vVector->items + (i * 4) + 1);
		float u = *((float*)vVector->items + (i * 4) + 2);
		float v = *((float*)vVector->items + (i * 4) + 3);
		glTexCoord2f(u,v);
		glVertex2f(vx, vy);
	}
	glEnd();
#else
	glUseProgram(m_impl->font_program.program_handle);
	glDisable(GL_CULL_FACE);
	glVertexAttribPointer ( m_impl->font_program.vertex_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), vVector->items );
	glEnableVertexAttribArray ( m_impl->font_program.vertex_handle );
	glVertexAttribPointer ( m_impl->font_program.texture_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLfloat*)vVector->items+2 );
	glEnableVertexAttribArray ( m_impl->font_program.texture_handle );

	glUniform1i ( m_impl->font_program.sampler_handle, 0);

	glDrawArrays ( GL_TRIANGLES, 0, vVector->size/4 );
#endif
}

bool
Painter::locate_resource(std::string name, std::string &path)
{
	const char* resource_env = getenv("GUI_HOME");
	if (!resource_env){
		std::cerr << "Painter::locate_resource : no GUI_HOME variable found, please set it" << std::endl;
		return false;
	}

	std::string resource_path = std::string(resource_env) + "/resources/" + name;

	if( access( resource_path.c_str(), F_OK ) == 0 ) {
		path = resource_path;
		return true;
	}
	return false;
}

unsigned int
Painter::default_font_idx()
{
	return m_impl->default_font_idx;
}
