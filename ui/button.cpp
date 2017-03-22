#include "button.h"
#include "string_utils.h"

Button::Button(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_color_push = FColor(.5, .5, 1., 1);
	m_color_hover = FColor(.5, .5, 1., .9);
	m_original_fg = FColor(.5, .5, 1., .8);
	m_text_color = FColor(1.,1.,1.,1.);
	m_fgcolor = m_original_fg;
	m_image_id = -1;
	m_imgh = m_imgw = 0;
	m_rounded_rect_data = NULL;
	m_style = STYLE_ROUNDED_FILLED;
	m_toggled = false;
	m_icon_align = ICON_ALIGN_LEFT;

	label(name);
}

Button::~Button()
{
	if(m_image_id!=(unsigned int)-1)
		painter().delete_texture(m_image_id);
	if (m_rounded_rect_data)
		delete m_rounded_rect_data;
}

void
Button::icon_align(ICON_ALIGN a)
{
	m_icon_align = a;
}

void
Button::toggle(bool t)
{
	if (t){
		m_fgcolor = m_color_push;
	} else {
		m_fgcolor = m_original_fg;
	}
	m_toggled = t;
}

bool
Button::toggled()
{
	return m_toggled;
}

void
Button::image(std::string image_filename)
{
	m_image_id = painter().create_texture_svg(image_filename);
	if (m_image_id == -1){
		std::cerr << "Button::set_image : Image not loaded " <<  image_filename << std::endl;
		m_imgh = m_imgw = 0;
		return;
	}
	painter().texture_size(image_filename, m_imgw, m_imgh);
}

void
Button::draw(){
	IBbox area;
	drawing_area(area);
	int dah = area.height();
	int daw = area.width();

	int x, y;
	const int ww = daw - m_horizontal_margin * 2;
	const int hh = dah - m_vertical_margin * 2;
	if (m_icon_align == ICON_ALIGN_CENTER){
		x = (ww - m_imgw) / 2;
		y = (hh - m_imgh) / 2;
	} else if (m_icon_align == ICON_ALIGN_LEFT){
		x = (ww / 10);
		y = (hh - m_imgh) / 2;
	} else if (m_icon_align == ICON_ALIGN_RIGHT){
		x = (ww - w() / 10);
		y = (hh - m_imgh) / 2;
	}

	FBbox areaf(area.xmin(), area.xmax(), area.ymin(), area.ymax());
	painter().disable_texture();

	if (m_style == STYLE_ROUNDED_FILLED){
		if (!m_rounded_rect_data)
			m_rounded_rect_data = painter().build_solid_rounded_rectangle(areaf, 8, 10);
		painter().draw_solid_rounded_rectangle(*m_rounded_rect_data);
	} else if (m_style == STYLE_TAB){
		float hh = dah;
		float ww = daw;
		float vtx[] = { 0,hh, 0,0, ww - 8,0, ww,8, ww,hh };
		vertex_container vc(10);
		memcpy(vc.data(), vtx, 10*sizeof(float));
		painter().draw_solid_rounded_rectangle(vc);
	} else if (m_style == STYLE_ROUNDED) {
		if (!m_rounded_rect_data)
			m_rounded_rect_data = painter().build_rounded_rectangle(areaf, 8, 10);
		painter().draw_rounded_rectangle(*m_rounded_rect_data);
	}

	if(m_image_id >= 0){
		painter().use_texture(m_image_id);
		painter().enable_alpha(true);
		painter().draw_quad(x, y, m_imgw, m_imgh, true);
	}

	if (!m_text_data.text.empty()){
		IBbox& bound = m_text_data.bbox;
		int x = (w() - bound.width()) / 2;
		int y = (h() - bound.height()) / 2 + bound.height();
		push_model_matrix();
		translate(x, y);
		if (m_toggled || m_style != STYLE_TAB)
			painter().color(m_text_color);
		else
			painter().color(m_text_color.darken());
		painter().draw_text(m_text_data);
		pop_model_matrix();
	}
}

void
Button::text_color(const FColor& tc)
{
	m_text_color = tc;
}

void
Button::label(std::string label)
{
	painter().build_text(m_font_id, label.c_str(), 0, 0, m_text_data);
}

void
Button::style(BUTTON_STYLE style)
{
	m_style = style;
	if (m_rounded_rect_data)
		delete m_rounded_rect_data;
	m_rounded_rect_data = NULL;
	dirty(true);
}

bool
Button::mouse_press_event(int button){
	if (button != EVENT_MOUSE_BUTTON_LEFT)
		return false;
	m_fgcolor = m_color_push;
	dirty(true);
	return true;
}

bool
Button::mouse_release_event(int button){
	m_fgcolor = m_color_hover;
	do_callback();
	dirty(true);
	return true;
}

bool
Button::enter_event(){
	Widget::enter_event();
	if (m_toggled)
		return false;
	m_fgcolor = m_color_hover;
	dirty(true);
	return true;
}

bool
Button::leave_event(){
	if (m_toggled)
		return false;
	m_fgcolor = m_original_fg;
	dirty(true);
	return true;
}

int
Button::text_width()
{
	return m_text_data.bbox.width();
}

int
Button::text_height()
{
	return m_text_data.bbox.height();
}
