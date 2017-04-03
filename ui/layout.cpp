#include "layout.h"

Layout::Layout(int x, int y, int w, int h, const char* name, Widget* parent, Layout_style style) : Widget(x, y, w, h, name, parent)
{
	m_style = style;
	m_autoresize = false;
}

Layout::~Layout()
{

}

void
Layout::style(Layout_style style)
{
	m_style = style;
	compute_layout();
}

void
Layout::parent_resize_event(const IBbox& bbox)
{
	if (m_autoresize){
		resize(bbox.xmin(), bbox.ymin(), bbox.width(), bbox.height());
		compute_layout();
	}
}

void
Layout::compute_layout()
{
	if (m_children_widgets.empty())
		return;

	IBbox draw_area;
	drawing_area(draw_area);
	int daw = draw_area.width();
	int dah = draw_area.height();
	int minx = draw_area.xmin();
	int miny = draw_area.ymin();

	if (m_style == LAYOUT_VERTICAL){
		int total_fixed_h = 0;
		int num_children = m_children_widgets.size();
		std::vector<Widget*>::iterator it = m_children_widgets.begin();
		for (; it != m_children_widgets.end(); ++it){
			if ((*it)->fixed_height() > 0){
				total_fixed_h += (*it)->fixed_height();
				num_children--;
			}
		}
		if (!num_children)
			// Avoid nasty division by zero
			num_children++;

		int child_size_y = (dah - total_fixed_h) / num_children;
		int child_size_x = daw;
		int inc = draw_area.ymin();
		for (it = m_children_widgets.begin(); it != m_children_widgets.end(); ++it){
			if ((*it)->fixed_height() < 0){
				(*it)->resize(minx, inc, child_size_x, child_size_y);
				inc += child_size_y;
			} else {
				(*it)->resize(minx, inc, child_size_x, (*it)->fixed_height());
				inc += (*it)->fixed_height();
			}
		}
	} else {
		int total_fixed_w = 0;
		int num_children = m_children_widgets.size();

		std::vector<Widget*>::iterator it = m_children_widgets.begin();
		for (; it != m_children_widgets.end(); ++it){
			if ((*it)->fixed_width() > 0){
				total_fixed_w += (*it)->fixed_width();
				num_children--;
			}
		}

		if (!num_children)
			num_children++;

		int child_size_y = dah;
		int child_size_x = (daw - total_fixed_w) / num_children;
		int inc = draw_area.xmin();
		for (it = m_children_widgets.begin(); it != m_children_widgets.end(); ++it){
			if ((*it)->fixed_width() < 0){
				(*it)->resize(inc, miny, child_size_x, child_size_y);
				inc += child_size_x;
			} else {
				(*it)->resize(inc, miny, (*it)->fixed_width(), child_size_y);
				inc += (*it)->fixed_width();
			}
		}
	}
}

void
Layout::widget_added_event(Widget* widget)
{
	compute_layout();
}
