#ifndef PAINTER_H
#define PAINTER_H

#include "color.h"
#include "bbox.h"
#include <string>

struct PImpl;

class Painter
{
	PImpl *m_impl;
public:
	Painter();
	~Painter();

	void viewport(int x,int y,int width, int height);
	void create_ortho_matrix(float left, float right, float bottom, float top,
			 	 	 	 	 float near_val, float far_val);
	void scissor_begin(int x, int y, int width,	int height);
	void scissor_end();
	void load_identity();
	void clear_color_buffer(const FColor&c = FColor(0.0,0.0,0.0,1.0));
	void color(const FColor& c);
	void enable_alpha(bool on);

	int load_fonts(std::string font_filename, int font_size, int atlas_size = 1024);
	int font_by_name(std::string name);
	void remove_fonts(int idx);
	char* load_svg_image(std::string filename, int &w, int &h);

	bool locate_resource(std::string name, std::string &path);

	void draw_text(int font_id, std::string text, int start_x, int start_y);
	void draw_quad(int x, int y, int width, int height, bool fill);

	IBbox bound_text(int font_id, std::string text);

	void delete_texture(unsigned int idx);
	void delete_texture(std::string name);
	void texture_size(std::string name, int& w, int& h);
	unsigned int create_texture_svg(std::string name,std::string filename);
	void use_texture(std::string name);
	void use_texture(unsigned int texid);
	void disable_texture();
	unsigned int default_font_idx();
};

#endif
