#include "painter.h"
#include "bbox.h"
#include "string_utils.h"

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
	texture_atlas_t *atlas;
	texture_font_t *font;
	std::string    name;
	int size, atlas_size;
};

struct PImpl{
	GLfloat current_matrix[16];
	std::map< std::string, Img_info > textures;
	std::vector< Font_info > fonts;
	unsigned int default_font_idx;
#ifndef USE_OPENGL
	GLint	mat_loc;
	GLuint	program;
	GLuint	vertex_shader, fragment_shader;
#endif
};

#ifndef USE_OPENGL
const unsigned int ID_VERTS=0;
const unsigned int ID_COLOURS=1;
const unsigned int UI_TEXCOORDS=2;
#endif

const char* vertex_shader_src = "\
#version 130\
in vec3 position;\
in vec3 colour;\
uniform mat4 matrix;\
smooth out vec4 vColor;\
\
void main(void)\
{\
\
	gl_Position = vec4(position, 1.0f)*projection;\
	vColor = vec4(colour, 1.0); \
}\
";

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]
static void matmul4( GLfloat *product, const GLfloat *a, const GLfloat *b )
{
   GLint i;
   for (i = 0; i < 4; i++) {
      const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
}

void generate_text(const Font_info& finfo, const std::string& text, int start_x, int start_y, vector_t* vVector)
{
	size_t i;
	int start_x_mem = start_x;
	for( i=0; i< text.size(); ++i )
	{
		if (text[i] == '\n'){
			start_x = start_x_mem;
			start_y += finfo.size;
		}
		texture_glyph_t *glyph = texture_font_get_glyph( finfo.font, text[i] );
		if( glyph != NULL )
		{
			int kerning = 0;
			if( i > 0)
			{
				kerning = texture_glyph_get_kerning( glyph, text[i-1] );
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

			vector_push_back_data( vVector, vertices, 6*4);

			start_x += glyph->advance_x;
		}
	}
}

Painter::Painter()
{
	m_impl = new PImpl;
	load_identity();
	std::string font_file;
	if( locate_resource("custom.ttf", font_file) ){
		m_impl->default_font_idx = load_fonts(font_file, 14);
	} else {
		std::cerr << "Painter::Painter : cannot initialize default fonts" << std::endl;
	}
}

Painter::~Painter()
{
	delete m_impl;
}
#ifndef USE_OPENGL

GLuint load_shader(const char *shaderSrc, GLenum type)
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
	ch02.fm Page 21 Thursday, June 19, 2008 3:21 PM
	22 Chapter 2: Hello Triangle: An OpenGL ES 2.0 Example
	if(!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if(infoLen > 1)
		{
			char* infoLog = malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			esLogMessage("Error compiling shader:\n%s\n", infoLog);
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
	GLuint vertex_shader = load_shader(GL_FRAGMENT_SHADER, vertex_shader_src);
	m_impl->program = glCreateProgram();

	if(m_impl->program == 0)
		return;

	glAttachShader(m_impl->program, vertex_shader);

	glUseProgram(m_impl->program);
	glBindAttribLocation(m_impl->program, VERTS, "position");
	glBindAttribLocation(m_impl->program, TEXCOORDS, "vtexCoord");
	glLinkProgram(m_impl->program);

	m_impl->matloc = glGetUniformLocation(m_impl->program, "projection");
}
#endif

void
Painter::viewport(int x,int y,int width, int height)
{
	glViewport( x, y, width, height );
}

void
Painter::create_ortho_matrix(float left, float right,
							 float bottom, float top,
							 float near, float far )
{
	GLfloat matrix[16];
    GLfloat r_l = right - left;
    GLfloat t_b = top - bottom;
    GLfloat f_n = far - near;
    GLfloat tx = - (right + left) / (right - left);
    GLfloat ty = - (top + bottom) / (top - bottom);
    GLfloat tz = - (far + near) / (far - near);

    matrix[0] = 2.0f / r_l;
    matrix[4] = 0.0f;
    matrix[8] = 0.0f;
    matrix[12] = tx;

    matrix[1] = 0.0f;
    matrix[5] = 2.0f / t_b;
    matrix[9] = 0.0f;
    matrix[13] = ty;

    matrix[2] = 0.0f;
    matrix[6] = 0.0f;
    matrix[10] = -2.0f / f_n;
    matrix[14] = tz;

    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[15] = 1.0f;

    // glMultMatrix equivalent
    matmul4(m_impl->current_matrix, m_impl->current_matrix, matrix );

#ifdef USE_OPENGL
    glLoadMatrixf(m_impl->current_matrix);
#else
    glUniformMatrix4fv(m_impl->mat_loc, 1, GL_FALSE, &m_impl->current_matrix);
#endif
}

void
Painter::load_identity()
{
    m_impl->current_matrix[0] = 1.0f;
    m_impl->current_matrix[4] = 0.0f;
    m_impl->current_matrix[8] = 0.0f;
    m_impl->current_matrix[12] =0.0f;

    m_impl->current_matrix[1] = 0.0f;
    m_impl->current_matrix[5] = 1.0f;
    m_impl->current_matrix[9] = 0.0f;
    m_impl->current_matrix[13] = 0.0f;

    m_impl->current_matrix[2] = 0.0f;
    m_impl->current_matrix[6] = 0.0f;
    m_impl->current_matrix[10] = 1.0f;
    m_impl->current_matrix[14] = 0.0f;

    m_impl->current_matrix[3] = 0.0f;
    m_impl->current_matrix[7] = 0.0f;
    m_impl->current_matrix[11] = 0.0f;
    m_impl->current_matrix[15] = 1.0f;
#ifdef USE_OPENGL
	glLoadMatrixf(m_impl->current_matrix);
#else
	glUniformMatrix4fv(m_impl->mat_loc, 1, GL_FALSE, &m_impl->current_matrix);
#endif
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
	glClear(GL_COLOR_BUFFER_BIT);
}

void
Painter::color(const FColor& c)
{
	glColor4f(c.red(), c.green(), c.blue(), c.alpha());
}

char*
Painter::load_svg_image(std::string filename, int &w, int &h)
{
	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;

	image = nsvgParseFromFile(filename.c_str(), "px", 96.0f);
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
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);


	m_impl->textures[name] = Img_info(textureID, w, h);
	free(img);
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
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texid);
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

void
Painter::use_texture(std::string name)
{
	if (m_impl->textures.find(name) == m_impl->textures.end()){
		std::cerr << "Painter::use_texture : Texture " << name << " does not exist " << m_impl->textures.size() << std::endl;
		return;
	}
	use_texture(m_impl->textures[name].texid);
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
		glBegin(GL_LINE_STRIP);
		glVertex2f(x, y);
		glVertex2f(x , height);
		glVertex2f(width, height);
		glVertex2f(width, y);
		glVertex2f(x, y);
		glEnd();
	} else {
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
Painter::draw_text(int font_id, std::string text, int start_x, int start_y)
{
	vector_t * vVector = vector_new(sizeof(GLfloat));

	Font_info finfo = m_impl->fonts[font_id];
	generate_text(finfo, text, start_x, start_y, vVector);

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_impl->fonts[font_id].atlas->id );
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

#endif

	vector_delete(vVector);
}

IBbox
Painter::bound_text(int font_id, std::string text)
{
	vector_t * vVector = vector_new(sizeof(GLfloat));
	Font_info finfo = m_impl->fonts[font_id];
	IBbox box;

	generate_text(finfo, text, 0, 0, vVector);

	for (int i = 0; i < vVector->size / 4; ++i){
		float vx = *((float*)vVector->items + (i * 4));
		float vy = *((float*)vVector->items + (i * 4) + 1);
		box.extend(int(vx), int(vy));
	}

	vector_delete(vVector);
	return box;
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
	if( access( resource_path.c_str(), F_OK ) != -1 ) {
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
