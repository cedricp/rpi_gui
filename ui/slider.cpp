#include "slider.h"

Slider::Slider(int x, int y, int width, int height, const char* name, Widget* parent, Slider_type type) : Widget( x, y, width, height, name, parent)
{
	m_value = 1;
	m_value_max = 1000;
	m_value_min = 0;
	m_hold_value = 0;
	m_type = type;

	m_cursor.xmin(0);
	m_cursor.xmax(width);
	m_cursor.ymin(0);
	m_cursor.ymax(height);
	compute_cursor();
	callback(default_callback);
}

void
Slider::draw()
{
	compute_cursor();

	painter().use_texture(false);
	painter().draw_quad(relative_bbox().xmin()+1, relative_bbox().ymin(), relative_bbox().width()-1, relative_bbox().height()-1, false);
	painter().draw_quad(m_cursor.xmin(), m_cursor.ymin(), m_cursor.width(), m_cursor.height(), true);
}

void
Slider::compute_cursor(){
	float range = m_value_max - m_value_min;
	float value_from_zero = m_value - m_value_min;
	if (m_type == SLIDER_TYPE_HORIZONTAL){
		float slider_size = (float)w() / range;
		slider_size = slider_size > 5 ? slider_size : 5;

		float pos = value_from_zero * ((float)w() - slider_size) / range;

		m_cursor.move_to(pos, 0);
		m_cursor.width(slider_size);
		m_cursor.height(h());
	} else {
		float slider_size = (float)h() / range;
		slider_size = slider_size > 5 ? slider_size : 5;

		float pos = value_from_zero * ((float)h() - slider_size) / range;

		m_cursor.move_to(0, pos);
		m_cursor.height(slider_size);
		m_cursor.width(w());
	}
}

bool
Slider::accept_drag(int x, int y)
{
	if (m_cursor.contains(x, y)){
		m_hold_value = m_value;
		return true;
	}
	return false;
}

bool
Slider::drag_event(int rel_x, int rel_y)
{
	if (m_type == SLIDER_TYPE_HORIZONTAL){
		float scaled = ((float)rel_x / (float)w()) *  float(m_value_max - m_value_min);
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
	do_callback();
	return true;
}
