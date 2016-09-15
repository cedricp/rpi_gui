#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

enum Layout_style{
	LAYOUT_HORIZONTAL,
	LAYOUT_VERTICAL
};

class Layout : public Widget
{
	Layout_style m_style;
	std::vector<Widget*> m_layout_widget;
public:
	Layout(int x, int y, int w, int h, const char* name = "", Widget* parent = NULL);
	~Layout();

	void add_widget(Widget*);
	void set_style(Layout_style style);
	void compute_layout();
};

#endif
