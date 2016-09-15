#include "layout.h"

Layout::Layout(int x, int y, int w, int h, const char* name, Widget* parent) : Widget(x, y, w, h, name, parent)
{
	m_style = LAYOUT_VERTICAL;
}

Layout::~Layout()
{

}

void
Layout::add_widget(Widget* w)
{
	m_layout_widget.push_back(w);
	compute_layout();
}

void
Layout::set_style(Layout_style style)
{
	m_style = style;
}

void
Layout::compute_layout()
{
	if (m_style == LAYOUT_VERTICAL){
		int num_children = m_layout_widget.size();
		int child_size_y = h() / num_children;
		int child_size_x = w();
		std::vector<Widget*>::iterator it = m_layout_widget.begin();
		int inc = 0;
		for (; it != m_layout_widget.end(); ++it){
			(*it)->resize(0, inc, child_size_x, child_size_y);
			inc += child_size_y;
		}
	} else {
		int num_children = m_layout_widget.size();
		int child_size_y = h();
		int child_size_x = w() / num_children;
		std::vector<Widget*>::iterator it = m_layout_widget.begin();
		int inc = 0;
		for (; it != m_layout_widget.end(); ++it){
			(*it)->resize(inc, 0, child_size_x, child_size_y);
			inc += child_size_x;
		}
	}
}
