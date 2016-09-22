#ifndef PAINTER_H
#define PAINTER_H

#include "color.h"
#include "bbox.h"
#include <matrix.h>
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
	TEXTURE_ALPHA
};

class Painter
{
	PImpl *m_impl;
public:
	Painter();
	~Painter();

	void init_gles2();

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
	void draw_quad(int x, int y, int width, int height, bool fill);

	void build_text(int font_id, std::string text, int start_x,int start_y, Text_data& data);

	void delete_texture(unsigned int idx);
	void delete_texture(std::string name);
	void texture_size(std::string name, int& w, int& h);
	unsigned int create_texture_svg(std::string name,std::string filename);
	unsigned int create_texture(std::string name, const char* img, int w, int h, TextureMode mode = TEXTURE_RGBA);
	bool use_texture(std::string name);
	void use_texture(unsigned int texid);
	void disable_texture();
	unsigned int default_font_idx();
	void use_default_gles_program();
};

#endif
