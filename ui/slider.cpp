#include "slider.h"

Slider::Slider(int x, int y, int width, int height, const char* name, Widget* parent, Slider_orientation type) : Widget( x, y, width, height, name, parent)
{
	m_value = 6;
	m_value_max = 30;
	m_value_min = 0;
	m_hold_value = 0;
	m_type = type;

	m_cursor.xmin(0);
	m_cursor.xmax(width);
	m_cursor.ymin(0);
	m_cursor.ymax(height);
	compute_cursor();
	callback(default_callback);
	m_bgcolor = FColor(.3,.3,.3,1);
}

void
Slider::draw()
{
	IBbox draw_area;
	compute_cursor();

	painter().use_texture(false);
	drawing_area(draw_area);
	painter().use_texture(true);
	painter().draw_quad_gradient(draw_area.xmin(), draw_area.ymin(), draw_area.width(), draw_area.height(), m_bgcolor, m_bgcolor, m_pattern_texture, 18);
	painter().use_texture(false);
	painter().color(m_fgcolor);
	painter().draw_quad(draw_area.xmin(), draw_area.ymin(), draw_area.width(), draw_area.height(), false, true, 2.);
	painter().draw_quad(m_cursor.xmin(), m_cursor.ymin(), m_cursor.width(), m_cursor.height(), true, true);
}

void
Slider::compute_cursor(){
	IBbox draw_area;
	drawing_area(draw_area);

	float range = m_value_max - m_value_min;
	float value_from_zero = m_value - m_value_min;
	if (m_type == SLIDER_TYPE_HORIZONTAL){
		float slider_size = (float)draw_area.width() / range;
		slider_size = slider_size > 40 ? slider_size : 40;

		float pos = value_from_zero * ((float)draw_area.width() - slider_size) / range;

		m_cursor.move_to(pos, draw_area.ymin() + 3);
		m_cursor.width(slider_size);
		m_cursor.height(draw_area.height() - 5);
	} else {
		float slider_size = (float)draw_area.height() / range;
		slider_size = slider_size > 40 ? slider_size : 40;

		float pos = value_from_zero * ((float)draw_area.height() - slider_size) / range;

		m_cursor.move_to(draw_area.xmin() + 3, pos);
		m_cursor.height(slider_size);
		m_cursor.width(draw_area.width() - 5);
	}
}

bool
Slider::accept_drag(int x, int y)
{
	if (m_cursor.contains(x, y)){
		m_hold_value = (float)x / (float)w() * (m_value_max - m_value_min) + m_value_min + 1;
		return true;
	}
	return false;
}

void
Slider::set_range(int min, int max)
{
	m_value_min = min;
	m_value_max = max;
	m_value = min;
	compute_cursor();
	dirty(true);
}

bool
Slider::drag_event(int rel_x, int rel_y)
{
	IBbox draw_area;
	drawing_area(draw_area);
	if (m_type == SLIDER_TYPE_HORIZONTAL){
		float scaled = (float)rel_x / (float)w() * (m_value_max - m_value_min);
		m_value = m_hold_value - scaled;
		m_value = m_value < m_value_min ? m_value_min : m_value;
		m_value = m_value > m_value_max ? m_value_max : m_value;
	} else {
		float scaled = ((float)rel_y / (float)h()) *  float(m_value_max - m_value_min);
		m_value = m_hold_value - scaled;
		m_value = m_value < m_value_min ? m_value_min : m_value;
		m_value = m_value > m_value_max ? m_value_max : m_value;
	}

	dirty(true);
	return true;
}

bool
Slider::mouse_release_event(int button)
{
	do_callback();
	return true;
}

bool
Slider::leave_event()
{
	return true;
}
