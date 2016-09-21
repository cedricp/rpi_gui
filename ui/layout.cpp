#include "layout.h"

Layout::Layout(int x, int y, int w, int h, const char* name, Widget* parent, Layout_style style) : Widget(x, y, w, h, name, parent)
{
	m_style = style;
}

Layout::~Layout()
{

}

void
Layout::set_style(Layout_style style)
{
	m_style = style;
	compute_layout();
}

void
Layout::compute_layout()
{
	if (m_children_widgets.empty())
		return;

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
			num_children++;
		// Avoid nasty division by zero
		int child_size_y = (h() - total_fixed_h) / num_children;
		int child_size_x = w();
		int inc = 0;
		for (it = m_children_widgets.begin(); it != m_children_widgets.end(); ++it){
			if ((*it)->fixed_height() < 0){
				(*it)->resize(0, inc, child_size_x, child_size_y);
				inc += child_size_y;
			} else {
				(*it)->resize(0, inc, child_size_x, (*it)->fixed_height());
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

		int child_size_y = h();
		int child_size_x = (w() - total_fixed_w) / num_children;
		int inc = 0;
		for (it = m_children_widgets.begin(); it != m_children_widgets.end(); ++it){
			if ((*it)->fixed_width() < 0){
				(*it)->resize(inc, 0, child_size_x, child_size_y);
				inc += child_size_x;
			} else {
				(*it)->resize(inc, 0, (*it)->fixed_width(), child_size_y);
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
