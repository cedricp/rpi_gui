#include "painter.h"
#include "bbox.h"
#include "compositor.h"
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
		ref_count = 0;
	}
	Img_info(const Img_info& info){
		texid = info.texid;
		width = info.width;
		height = info.height;
		ref_count = info.ref_count;
	}
	GLuint texid;
	size_t ref_count;
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
enum gl_program_type{
	GL_PROGRAM_TEXTURE,
	GL_PROGRAM_FONT,
	GL_PROGRAM_SOLID,
	GL_PROGRAM_VERTEX_COLOR,
	GL_PROGRAM_VERTEX_COLOR_TEX,
	GL_PROGRAM_SIZE
};

struct gl_program
{
	GLint	matrix_projection, matrix_model;
	GLuint	program_handle;
	GLint 	vertex_handle, texture_handle, sampler_handle, color_handle;
};

#endif

struct PImpl{
	std::map< std::string, Img_info > textures;
	std::vector< Font_info > fonts;
	unsigned int default_font_idx;
#ifndef USE_OPENGL
	gl_program gl_pgm[GL_PROGRAM_SIZE];
#endif
};

struct point
{
	float x, y;
};

struct contour
{
   struct point *p;
   unsigned int count;
};


/*
* C conversion of Efficient Polygon Triangulation
* Found at http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
* Adapted and debugged for this use
*/
float area(const struct contour* contour)
{
   int p, q;
   int n = contour->count - 1;
   float A = 0.f;

   for (p=n-1, q=0; q < n; p=q++)
   {
       A += contour->p[p].x * contour->p[q].y - contour->p[q].x * contour->p[p].y;
   }
   return A * .5f;
}

int
inside_triangle(float Ax, float Ay,
               float Bx, float By,
               float Cx, float Cy,
               float Px, float Py)
{
   float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
   float cCROSSap, bCROSScp, aCROSSbp;

   ax = Cx - Bx;  ay = Cy - By;
   bx = Ax - Cx;  by = Ay - Cy;
   cx = Bx - Ax;  cy = By - Ay;
   apx= Px - Ax;  apy= Py - Ay;
   bpx= Px - Bx;  bpy= Py - By;
   cpx= Px - Cx;  cpy= Py - Cy;

   aCROSSbp = ax*bpy - ay*bpx;
   cCROSSap = cx*apy - cy*apx;
   bCROSScp = bx*cpy - by*cpx;

   return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}

int
snip(const struct point* pnt,int u,int v,int w,int n,int *V)
{
   int p;
   float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

   Ax = pnt[V[u]].x;
   Ay = pnt[V[u]].y;

   Bx = pnt[V[v]].x;
   By = pnt[V[v]].y;

   Cx = pnt[V[w]].x;
   Cy = pnt[V[w]].y;

   if ( (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) < 0.f )
       return 0;

   for (p=0;p<n;p++)
   {
       if( (p == u) || (p == v) || (p == w) )
           continue;
       Px = pnt[V[p]].x;
       Py = pnt[V[p]].y;
       if (inside_triangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py))
           return 0;
   }

   return 1;
}

int
process_triangles_2d(const struct contour* contour, struct contour* result)
{
   int v;
   int contour_size = contour->count;
   int polygon_temp_size = contour_size * 80;
   int final_count = 0;
   result->p = (point*)malloc(sizeof(struct point) * polygon_temp_size);

   int n = contour_size;
   if ( n < 3 ) return 0;

   int *V = (int*)alloca(sizeof(int)*n);

   if ( 0.0f < area(contour) )
       for (v=0; v<n; v++) V[v] = v;
   else
       for(v=0; v<n; v++) V[v] = (n-1)-v;

   int nv = n;

   int count = 2*nv;

   for(v=nv-1; nv>2; )
   {
       /* if we loop, it is probably a non-simple polygon */
       if (0 >= (count--))
       {
           //** Triangulate: ERROR - probable bad polygon!
           break;
       }

       /* three consecutive vertices in current polygon, <u,v,w> */
       int u = v  ; if (nv <= u) u = 0;     /* previous */
       v = u+1; if (nv <= v) v = 0;         /* new v    */
       int w = v+1; if (nv <= w) w = 0;     /* next     */

       if ( snip(contour->p,u,v,w,nv,V) )
       {
           int a,b,c,s,t;

           /* true names of the vertices */
           a = V[u]; b = V[v]; c = V[w];

           /* output Triangle */
           result->p[final_count++] = contour->p[a];
           result->p[final_count++] = contour->p[b];
           result->p[final_count++] = contour->p[c];

           if (final_count >= polygon_temp_size){
               free(result->p);
               return 0;
           }

           /* remove v from remaining polygon */
           for(s=v,t=v+1;t<nv;s++,t++)
               V[s] = V[t];
           nv--;

           /* reset error detection counter */
           count = 2*nv;
       }
   }

   result->count = final_count;

   return 1;
}

Text_data::Text_data()
{
	data = new FontImpl;
}

Text_data::~Text_data(){
	delete data;
};

#pragma pack(push, 1)
// Stupid M$ window$ definitions
typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;


struct BITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;

  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
};
#pragma pack(pop)
// Very naive BMP loader implementation
char*
load_bmp_texture(const char * pic, int& width, int &height, int& num_channel)
{
	BITMAPFILEHEADER bmpHeaderFile;
	char* data;
	FILE * picfile;

	picfile = fopen(pic, "rb");
	if (picfile == NULL)
	return NULL;

	fread((void*)&bmpHeaderFile, sizeof(BITMAPFILEHEADER), 1, picfile);
	if (bmpHeaderFile.bfType != 0x4D42)
		return NULL;

	width = bmpHeaderFile.biWidth;
	height = bmpHeaderFile.biHeight;

	num_channel = bmpHeaderFile.biBitCount / 8;

	data = (char *)malloc(bmpHeaderFile.biSizeImage);

	fseek(picfile, bmpHeaderFile.bfOffBits, SEEK_SET);
	fread((void*)data, bmpHeaderFile.biSizeImage, 1, picfile);

	fclose(picfile);

	return data;
}

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
			int y0  = (int)( start_y - (glyph->offset_y-glyph->height) );
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
	std::string font_name = "fonts/Roboto-Regular.ttf";
	if( locate_resource(font_name, font_file) ){
		m_impl->default_font_idx = load_fonts(font_file, 16);
	} else {
		std::cerr << "Cannot load default fonts, aborting" << std::endl;
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
	GLint linkSuccessful;

	GLuint vertex_shader 		 = load_shader(GL_VERTEX_SHADER, vertex_shader_src);
	GLuint vertex_shader_simple	 = load_shader(GL_VERTEX_SHADER, vertex_shader_simple_src);
	GLuint vertex_shader_color	 = load_shader(GL_VERTEX_SHADER, vertex_color_shader_src);
	GLuint fragment_shader 		 = load_shader(GL_FRAGMENT_SHADER, frag_shader_src);
	GLuint fragment_shader_font  = load_shader(GL_FRAGMENT_SHADER, frag_shader_font_src);
	GLuint fragment_shader_solid = load_shader(GL_FRAGMENT_SHADER, frag_shader_solid_src);

	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle 		= glCreateProgram();
	m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle 			= glCreateProgram();
	m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle 		= glCreateProgram();
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle 	= glCreateProgram();
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle 	= glCreateProgram();

	if(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'texture'" << std::endl;
		return;
	}

	if(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'font'" << std::endl;
		return;
	}

	if(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'solid'" << std::endl;
		return;
	}

	if(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'vertex_color'" << std::endl;
		return;
	}

	if(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle== 0){
		std::cerr << "Painter::init_gles2() : Cannot create program 'vertex_color'" << std::endl;
		return;
	}

	// Normal texture shader
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, vertex_shader);
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, fragment_shader);

	glLinkProgram(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle);
	glGetProgramiv(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, GL_LINK_STATUS, &linkSuccessful);
	if (!linkSuccessful){
		std::cerr << "Failed to link texture shader" << std::endl;
	}

	// Font (alpha) texture shader
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, vertex_shader);
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, fragment_shader_font);

	glLinkProgram(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle);
	glGetProgramiv(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, GL_LINK_STATUS, &linkSuccessful);
	if (!linkSuccessful){
		std::cerr << "Failed to link font shader" << std::endl;
	}

	// Solid texture shader
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle, vertex_shader_simple);
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle, fragment_shader_solid);

	glLinkProgram(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle);
	glGetProgramiv(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle, GL_LINK_STATUS, &linkSuccessful);
	if (!linkSuccessful){
		std::cerr << "Failed to link solid shader" << std::endl;
	}

	// Vertex color shader tex
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, vertex_shader_color);
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, fragment_shader);

	glLinkProgram(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle);
	glGetProgramiv(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, GL_LINK_STATUS, &linkSuccessful);
	if (!linkSuccessful){
		std::cerr << "Failed to link vertex color shader" << std::endl;
	}

	// Vertex color shader
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, vertex_shader_color);
	glAttachShader(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, fragment_shader_solid);

	glLinkProgram(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle);
	glGetProgramiv(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, GL_LINK_STATUS, &linkSuccessful);
	if (!linkSuccessful){
		std::cerr << "Failed to link vertex color shader" << std::endl;
	}

	// Attribs locations
	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].matrix_projection= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, "projection_matrix");
	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].matrix_model 	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, "model_matrix");
	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].vertex_handle  	= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, "position" );
	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].texture_handle 	= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, "st" );
	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].sampler_handle  	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, "in_texture_sampler" );
	m_impl->gl_pgm[GL_PROGRAM_TEXTURE].color_handle		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle, "input_color" );

	m_impl->gl_pgm[GL_PROGRAM_FONT].matrix_projection 	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, "projection_matrix");
	m_impl->gl_pgm[GL_PROGRAM_FONT].matrix_model 		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, "model_matrix");
	m_impl->gl_pgm[GL_PROGRAM_FONT].vertex_handle  		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, "position" );
	m_impl->gl_pgm[GL_PROGRAM_FONT].texture_handle 		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, "st" );
	m_impl->gl_pgm[GL_PROGRAM_FONT].sampler_handle	 	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, "in_texture_sampler" );
	m_impl->gl_pgm[GL_PROGRAM_FONT].color_handle		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle, "input_color" );

	m_impl->gl_pgm[GL_PROGRAM_SOLID].matrix_projection 	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle, "projection_matrix");
	m_impl->gl_pgm[GL_PROGRAM_SOLID].matrix_model 		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle, "model_matrix");
	m_impl->gl_pgm[GL_PROGRAM_SOLID].vertex_handle  	= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle, "position" );
	m_impl->gl_pgm[GL_PROGRAM_SOLID].color_handle		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_SOLID].program_handle,  "input_color" );

	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].matrix_projection 	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, "projection_matrix");
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].matrix_model 		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, "model_matrix");
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].color_handle		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, "input_color" );
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].vertex_handle  		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, "position" );
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].texture_handle 		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, "st" );
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].sampler_handle  	= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle, "in_texture_sampler" );

	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].matrix_projection 	= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, "projection_matrix");
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].matrix_model 		= glGetUniformLocation(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, "model_matrix");
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].vertex_handle  		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, "position" );
	m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].color_handle		= glGetAttribLocation (m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR].program_handle, "input_color" );

	glUseProgram(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle);
	glUniform1i ( m_impl->gl_pgm[GL_PROGRAM_TEXTURE].sampler_handle, 0);
	glUseProgram(m_impl->gl_pgm[GL_PROGRAM_FONT].program_handle);
	glUniform1i ( m_impl->gl_pgm[GL_PROGRAM_FONT].sampler_handle, 0);
	glUseProgram(m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].program_handle);
	glUniform1i ( m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX].sampler_handle, 0);
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
	glUseProgram(m_impl->gl_pgm[GL_PROGRAM_TEXTURE].program_handle);
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
    gl_program& glprog1 = m_impl->gl_pgm[GL_PROGRAM_TEXTURE];
    gl_program& glprog2 = m_impl->gl_pgm[GL_PROGRAM_FONT];
    gl_program& glprog3 = m_impl->gl_pgm[GL_PROGRAM_SOLID];
    gl_program& glprog4 = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR];
    gl_program& glprog5 = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX];
    glUseProgram(glprog1.program_handle);
    glUniformMatrix4fv(glprog1.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog2.program_handle);
    glUniformMatrix4fv(glprog2.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog3.program_handle);
    glUniformMatrix4fv(glprog3.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog4.program_handle);
    glUniformMatrix4fv(glprog4.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog5.program_handle);
    glUniformMatrix4fv(glprog5.matrix_projection, 1, GL_FALSE, (GLfloat *)matrix);
#endif
}

void
Painter::load_model_matrix(Matrix matrix)
{
#ifdef USE_OPENGL
	glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(matrix);
#else
    gl_program& glprog1 = m_impl->gl_pgm[GL_PROGRAM_TEXTURE];
    gl_program& glprog2 = m_impl->gl_pgm[GL_PROGRAM_FONT];
    gl_program& glprog3 = m_impl->gl_pgm[GL_PROGRAM_SOLID];
    gl_program& glprog4 = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR];
    gl_program& glprog5 = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX];
    glUseProgram(glprog1.program_handle);
    glUniformMatrix4fv(glprog1.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog2.program_handle);
    glUniformMatrix4fv(glprog2.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog3.program_handle);
    glUniformMatrix4fv(glprog3.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog4.program_handle);
    glUniformMatrix4fv(glprog4.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
    glUseProgram(glprog5.program_handle);
    glUniformMatrix4fv(glprog5.matrix_model, 1, GL_FALSE, (GLfloat *)matrix);
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
    gl_program& glprog1 = m_impl->gl_pgm[GL_PROGRAM_TEXTURE];
    gl_program& glprog2 = m_impl->gl_pgm[GL_PROGRAM_FONT];
    gl_program& glprog3 = m_impl->gl_pgm[GL_PROGRAM_SOLID];
    gl_program& glprog4 = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR];
    gl_program& glprog5 = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX];
	glUseProgram(glprog1.program_handle);
	glUniform4f(glprog1.color_handle, c.red(), c.green(), c.blue(), c.alpha());
	glUseProgram(glprog2.program_handle);
	glUniform4f(glprog2.color_handle, c.red(), c.green(), c.blue(), c.alpha());
	glUseProgram(glprog3.program_handle);
	glUniform4f(glprog3.color_handle, c.red(), c.green(), c.blue(), c.alpha());
	glUseProgram(glprog4.program_handle);
	glUniform4f(glprog4.color_handle, c.red(), c.green(), c.blue(), c.alpha());
	glUseProgram(glprog5.program_handle);
	glUniform4f(glprog5.color_handle, c.red(), c.green(), c.blue(), c.alpha());
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
Painter::create_texture_svg(std::string filename)
{
	if (m_impl->textures.find(filename) != m_impl->textures.end()){
		m_impl->textures[filename].ref_count++;
		return m_impl->textures[filename].texid;
	}

	int w, h;
	char* img;
	img = load_svg_image(filename, w, h);
	if(!img){
		std::cerr << "Painter::create_texture_svg : cannot load image file " << filename << std::endl;
		return -1;
	}
	int texid = create_texture(filename, img, w, h);

	free(img);
	return texid;
}

unsigned int
Painter::create_texture_bmp(std::string filename)
{
	std::string fullpath;

	if (m_impl->textures.find(filename) != m_impl->textures.end()){
		m_impl->textures[filename].ref_count++;
		return m_impl->textures[filename].texid;
	}

	if (!locate_resource(filename, fullpath)){
		fullpath = filename;
	}

	int w, h, n;
	char* img;
	img = ::load_bmp_texture(fullpath.c_str(), w, h, n);
	if(!img){
		std::cerr << "Painter::create_texture_bmp : cannot load image file " << filename << std::endl;
		return 0;
	}
	int texid = create_texture(filename, img, w, h, TEXTURE_RGB);

	free(img);
	return texid;
}

unsigned int
Painter::create_texture(std::string name, const char* img, int w, int h, TextureMode mode, TextureBorder border)
{
	if (m_impl->textures.find(name) != m_impl->textures.end()){
		m_impl->textures[name].ref_count++;
		return m_impl->textures[name].texid;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
#ifdef USE_OPENGL
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	switch(border){
	case TEXBORDER_REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case TEXBORDER_CLAMP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case TEXBORDER_MIRROR_REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	}

	if (mode == TEXTURE_RGBA)
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	else if (mode == TEXTURE_RGB)
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	else if (mode == TEXTURE_ALPHA)
		glTexImage2D(GL_TEXTURE_2D, 0,GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, img);

	m_impl->textures[name] = Img_info(textureID, w, h);
	m_impl->textures[name].ref_count++;
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

	--it->second.ref_count;
	if (it->second.ref_count > 0)
		return;

	glDeleteTextures( 1, &idx );
	m_impl->textures.erase(it);
}

void
Painter::use_texture(unsigned int texid)
{
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, texid);

#ifdef USE_OPENGL
	glEnable(GL_TEXTURE_2D);
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
Painter::draw_quad_gradient(int x, int y, int width, int height, FColor& color_top, FColor& color_bottom, int pattern_texture, float pattern_size)
{
	if (pattern_texture >= 0){
		use_texture(pattern_texture);
	} else
		disable_texture();

	width += x;
	height += y;
#ifdef USE_OPENGL
	if (pattern_texture >= 0)
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glPolygonMode(GL_FRONT, GL_FILL);
	glBegin(GL_QUADS);
	glTexCoord2f(x/pattern_size, y);
	glColor4f(color_top.red(), color_top.green(), color_top.blue(), color_top.alpha());
	glVertex2f(x, y);
	glTexCoord2f(x/pattern_size, height/pattern_size);
	glColor4f(color_bottom.red(), color_bottom.green(), color_bottom.blue(), color_bottom.alpha());
	glVertex2f(x , height);
	glTexCoord2f(width/pattern_size, height/pattern_size);
	glColor4f(color_bottom.red(), color_bottom.green(), color_bottom.blue(), color_bottom.alpha());
	glVertex2f(width, height);
	glTexCoord2f(width/pattern_size, y/pattern_size);
	glColor4f(color_top.red(), color_top.green(), color_top.blue(), color_top.alpha());
	glVertex2f(width, y);
	glEnd();
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#else
	GLfloat xx = x;
	GLfloat yy = y;
	GLfloat ww = width;
	GLfloat hh = height;
	glDisable(GL_CULL_FACE);

	GLfloat color_data[] = {
			color_top.red(), color_top.green(), color_top.blue(), color_top.alpha(),
			color_bottom.red(), color_bottom.green(), color_bottom.blue(), color_bottom.alpha(),
			color_top.red(), color_top.green(), color_top.blue(), color_top.alpha(),
			color_bottom.red(), color_bottom.green(), color_bottom.blue(), color_bottom.alpha(),
	};

	GLfloat vtx_data[] = { xx, yy, xx, hh, ww, yy, ww, hh };

	if (pattern_texture >= 0){
		GLfloat st_data[] = {
				xx/pattern_size, yy,
				xx/pattern_size, height/pattern_size,
				width/pattern_size, yy/pattern_size,
				width/pattern_size, height/pattern_size
		};

		gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR_TEX];
		glUseProgram(glprog.program_handle);

		glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, vtx_data);
		glEnableVertexAttribArray ( glprog.vertex_handle );
		glVertexAttribPointer ( glprog.color_handle, 4, GL_FLOAT, GL_FALSE, 0, color_data);
		glEnableVertexAttribArray ( glprog.color_handle );
		glVertexAttribPointer ( glprog.texture_handle, 2, GL_FLOAT, GL_FALSE, 0, st_data);
		glEnableVertexAttribArray ( glprog.texture_handle );
		glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4);
	} else {
		gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_VERTEX_COLOR];
		glUseProgram(glprog.program_handle);
		glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, vtx_data);
		glEnableVertexAttribArray ( glprog.vertex_handle );
		glVertexAttribPointer ( glprog.color_handle, 4, GL_FLOAT, GL_FALSE, 0, color_data);
		glEnableVertexAttribArray ( glprog.color_handle );
		glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4);
	}
#endif
}

void
Painter::draw_quad(int x, int y, int width, int height, bool fill, bool solid, float linewidth)
{
#ifdef USE_OPENGL
	height += y;
	width  += x;
	if (!fill){
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glLineWidth(linewidth);
		glEnable(GL_LINE_SMOOTH);
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
	float ww = width + x;
	float hh = height + y;
	glDisable(GL_CULL_FACE);


	if (!fill){
		gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_SOLID];
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glLineWidth(linewidth);
       	glUseProgram(glprog.program_handle);
		GLfloat gl_data[] = {
							xx, yy,
							xx, hh,
							ww, hh,
							ww, yy};

		glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, (GLfloat*)gl_data );
		glEnableVertexAttribArray ( glprog.vertex_handle );

		glDrawArrays ( GL_LINE_LOOP, 0, 4 );
	} else {
		if(!solid){
			gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_TEXTURE];
        	glUseProgram(glprog.program_handle);
			GLfloat gl_data[] = {
						xx, yy, 0., 0.,
						xx, hh, 0., 1.,
						ww, yy, 1., 0.,
						xx, hh, 0., 1.,
						ww, hh, 1., 1.,
						ww, yy, 1., 0.};

			glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLfloat*)gl_data );
			glEnableVertexAttribArray ( glprog.vertex_handle );
			glVertexAttribPointer ( glprog.texture_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), ((GLfloat*)gl_data) +2 );
			glEnableVertexAttribArray ( glprog.texture_handle);

			glDrawArrays ( GL_TRIANGLES, 0, 6 );
		} else {
			gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_SOLID];
			glUseProgram(glprog.program_handle);
			GLfloat gl_data[] = { xx, yy, xx, hh, ww, yy, xx, hh, ww, hh, ww, yy };
			glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, gl_data);
			glEnableVertexAttribArray ( glprog.vertex_handle );
			glDrawArrays ( GL_TRIANGLES, 0, 6);
		}
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

bool
Painter::build_text(int font_id, std::string text, int start_x,int start_y, Text_data& data)
{
	if (font_id >= m_impl->fonts.size())
		return false;

	data.data->finfo = m_impl->fonts[font_id];
	data.text = text;
	if (data.data->text_vector){
		vector_delete(data.data->text_vector);
		data.data->text_vector = NULL;
	}
	if (text.empty())
		return true;

	data.data->text_vector = vector_new(sizeof(GLfloat));
	generate_text(data);
	return true;
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
	gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_FONT];
	glUseProgram(glprog.program_handle);
	glDisable(GL_CULL_FACE);
	glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), vVector->items );
	glEnableVertexAttribArray ( glprog.vertex_handle );
	glVertexAttribPointer ( glprog.texture_handle, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLfloat*)vVector->items+2 );
	glEnableVertexAttribArray ( glprog.texture_handle );

	glUniform1i ( glprog.sampler_handle, 0);

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

	if( ::access( resource_path.c_str(), F_OK ) == 0 ) {
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

vertex_container*
Painter::build_solid_rounded_rectangle( const FBbox &r, float cornerRadius, int numSegmentsPerCorner )
{
	// automatically determine the number of segments from the circumference
	if( numSegmentsPerCorner <= 0 ) {
		numSegmentsPerCorner = (int)floor( cornerRadius * M_PI * 2 / 4 );
	}
	if( numSegmentsPerCorner < 2 ) numSegmentsPerCorner = 2;

	float center_x, center_y;
	r.get_center(center_x, center_y);

	size_t num_coords = 4 + (numSegmentsPerCorner+1)*2*4;
	vertex_container* vc = new vertex_container(num_coords);
	float *verts = vc->data();
	verts[0] = center_x;
	verts[1] = center_y;
	size_t tri = 1;
	const float angleDelta = 1 / (float)numSegmentsPerCorner * M_PI / 2;
	const float cornerCenterVerts[8] = { r.xmax() - cornerRadius, r.ymax() - cornerRadius, r.xmin() + cornerRadius, r.ymax() - cornerRadius,
			r.xmin() + cornerRadius, r.ymin() + cornerRadius, r.xmax() - cornerRadius, r.ymin() + cornerRadius };
	for( size_t corner = 0; corner < 4; ++corner ) {
		float angle = corner * M_PI / 2.0f;
		Vec2f cornerCenter( cornerCenterVerts[corner*2], cornerCenterVerts[corner*2+1] );
		for( int s = 0; s <= numSegmentsPerCorner; s++ ) {
			Vec2f pt( cornerCenter.x() + cos( angle ) * cornerRadius, cornerCenter.y() + sin( angle ) * cornerRadius );
			verts[tri*2+0] = pt.x();
			verts[tri*2+1] = pt.y();
			++tri;
			angle += angleDelta;
		}
	}
	verts[tri*2] = r.xmax();
	verts[tri*2+1] = r.ymax() - cornerRadius;
	return vc;
}

void
Painter::draw_solid_tri_fans(vertex_container& vc)
{
	GLfloat *verts = vc.data();
#ifdef USE_OPENGL
	glPolygonMode(GL_FRONT, GL_FILL);
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < vc.size(); i+=2){
		glVertex2f(verts[i], verts[i+1]);
	}
	glEnd();
#else
	gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_SOLID];
	glUseProgram(glprog.program_handle);
	glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray ( glprog.vertex_handle );
	glDrawArrays ( GL_TRIANGLE_FAN, 0, vc.size() / 2);
#endif
}

void
Painter::draw_solid_polygon_2d(vertex_container& vc)
{
	GLfloat *verts = vc.data();
#ifdef USE_OPENGL
	glPolygonMode(GL_FRONT, GL_FILL);
	glBegin(GL_TRIANGLE_STRIP);
	for(int i = 0; i < vc.size(); i+=2){
		glVertex2f(verts[i], verts[i+1]);
	}
	glEnd();
#else
	gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_SOLID];
	glUseProgram(glprog.program_handle);
	glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray ( glprog.vertex_handle );
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, vc.size() / 2);
#endif
}

vertex_container*
Painter::build_rounded_rectangle( const FBbox &r, float cornerRadius, int numSegmentsPerCorner )
{
	if( numSegmentsPerCorner <= 0 ) {
		numSegmentsPerCorner = (int)floor( cornerRadius * M_PI * 2 / 4 );
	}
	if( numSegmentsPerCorner < 2 ) numSegmentsPerCorner = 2;

	size_t num_coords = (numSegmentsPerCorner+2)*2*4;
	vertex_container* vc = new vertex_container(num_coords);
	float *verts = vc->data();
	size_t lines = 0;
	const float angleDelta = 1 / (float)numSegmentsPerCorner * M_PI / 2;
	const float cornerCenterVerts[8] = { r.xmax() - cornerRadius, r.ymax() - cornerRadius, r.xmin() + cornerRadius, r.ymax() - cornerRadius,
			r.xmin() + cornerRadius, r.ymin() + cornerRadius, r.xmax() - cornerRadius, r.ymin() + cornerRadius };
	for( size_t corner = 0; corner < 4; ++corner ) {
		float angle = corner * M_PI / 2.0f;
		Vec2f cornerCenter( cornerCenterVerts[corner*2+0], cornerCenterVerts[corner*2+1] );
		for( int s = 0; s <= numSegmentsPerCorner; s++ ) {
			Vec2f pt( cornerCenter.x() + cos( angle ) * cornerRadius, cornerCenter.y() + sin( angle ) * cornerRadius );
			verts[lines*2+0] = pt.x();
			verts[lines*2+1] = pt.y();
			++lines;
			angle += angleDelta;
		}
	}
	vc->resize(lines*2);
	return vc;
}

void
Painter::draw_line_loop(vertex_container& vc, float width)
{
	GLfloat *verts = vc.data();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glLineWidth(width);
#ifdef USE_OPENGL
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < vc.size(); i+=2){
		glVertex2f(verts[i], verts[i+1]);
	}
	glEnd();
#else
	gl_program& glprog = m_impl->gl_pgm[GL_PROGRAM_SOLID];
	glUseProgram(glprog.program_handle);
	glVertexAttribPointer ( glprog.vertex_handle, 2, GL_FLOAT, GL_FALSE, 0, (GLfloat*)verts );
	glEnableVertexAttribArray ( glprog.vertex_handle );
	glDrawArrays ( GL_LINE_LOOP, 0, vc.size() / 2 );
#endif
}

vertex_container*
Painter::create_polygon_2d(vertex_container* vc)
{
	contour in, out;
	in.count = vc->size() / 2;
	in.p = new point[in.count];
	float *data = vc->data();

	for (int i = 0; i < in.count; ++i){
		in.p[i].x = data[i*2];
		in.p[i].y = data[i*2+1];
	}

	bool ok = process_triangles_2d(&in, &out);
	delete[] in.p;

	if(ok){
		vertex_container* newvc = new vertex_container(out.count*2);
		float* newvcdata = newvc->data();
		for (int i = 0; i < out.count; ++i){
			newvcdata[i*2] = out.p[i].x;
			newvcdata[i*2+1] = out.p[i].y;
		}
		free(out.p);
		return newvc;
	}
	return NULL;
}

void
Painter::copy_front_to_back(int x, int y, int width, int height)
{
	glReadBuffer( GL_FRONT );
	glDrawBuffer( GL_BACK );
	glCopyPixels( x, y, width, height, GL_COLOR );
#ifdef USE_OPENGL
	glFlush();
#endif
}
