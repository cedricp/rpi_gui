#ifndef PAINTER_H
#define PAINTER_H

#include "color.h"
#include "bbox.h"
#include <matrix.h>
#include <string.h>
#include <string>

struct PImpl;
struct FontImpl;

struct Text_data{
	Text_data();
	~Text_data();
	std::string text;
	FontImpl* data;
	IBbox bbox;
};

enum TextureMode {
	TEXTURE_RGBA,
	TEXTURE_RGB,
	TEXTURE_ALPHA
};

enum TextureBorder {
	TEXBORDER_CLAMP,
	TEXBORDER_REPEAT,
	TEXBORDER_MIRROR_REPEAT
};

class Vec2f{
	float m_x, m_y;
public:
	Vec2f(float x, float y){
		m_x = x;
		m_y = y;
	}
	inline float x(){return m_x;}
	inline float y(){return m_y;}
};

class vertex_container{
	float* m_data;
	size_t m_size;
public:
	vertex_container(size_t size){
		m_data = new float[size];
		m_size = size;
	}
	~vertex_container(){
		delete[] m_data;
	}
	void resize(size_t size){
		float* temp = new float[size];
		size_t min_size = size < m_size ? size : m_size;
		memcpy((void*)temp, (void*)m_data, min_size * sizeof(float));
		delete[] m_data;
		m_data = temp;
		m_size = size;
	}
	inline float* data(){return m_data;}
	inline size_t size(){return m_size;}
};

class Painter
{
	PImpl *m_impl;
	void init_gles2();
public:
	Painter();
	~Painter();

	void viewport(int x,int y,int width, int height);
	void create_ortho_matrix(float left, float right, float bottom, float top,
			 	 	 	 	 float near_val, float far_val, Matrix outmatrix);
	void create_perspective_matrix(float aspect_ratio, float fovy, float znear, float zfar, Matrix outmatrix);
	void load_projection_matrix(Matrix matrix);
	void load_model_matrix(Matrix matrix);
	void scissor_begin(int x, int y, int width,	int height);
	void scissor_end();
	void load_identity(Matrix matrix);
	void clear_color_buffer(const FColor&c = FColor(0.0,0.0,0.0,1.0));
	void color(const FColor& c);
	void enable_alpha(bool on);

	int load_fonts(std::string font_filename, int font_size, int atlas_size = 1024);
	int font_by_name(std::string name);
	void remove_fonts(int idx);
	char* load_svg_image(std::string filename, int &w, int &h);

	bool locate_resource(std::string name, std::string &path);

	void draw_text(const Text_data& data);
	void draw_quad(int x, int y, int width, int height, bool fill, bool solid = false, float linewidth = 1.0);
	void draw_quad_gradient(int x, int y, int width, int height, FColor& color_top, FColor& color_bottom, int pattern_texture = -1, float pattern_size = 1);

	bool build_text(int font_id, std::string text, int start_x,int start_y, Text_data& data);

	void delete_texture(unsigned int idx);
	void texture_size(std::string name, int& w, int& h);
	unsigned int create_texture_svg(std::string filename);
	unsigned int create_texture_bmp(std::string filename);
	unsigned int create_texture(std::string name, const char* img, int w, int h, TextureMode mode = TEXTURE_RGBA, TextureBorder = TEXBORDER_REPEAT);
	bool use_texture(std::string name);
	void use_texture(unsigned int texid);
	void disable_texture();
	unsigned int default_font_idx();
	void use_default_gles_program();
	vertex_container* build_solid_rounded_rectangle( const FBbox &r, float cornerRadius, int numSegmentsPerCorner );
	void draw_solid_rounded_rectangle(vertex_container& vc);
	vertex_container* build_rounded_rectangle( const FBbox &r, float cornerRadius, int numSegmentsPerCorner );
	void draw_rounded_rectangle(vertex_container& vc, float width = 1.5);
};

#endif
