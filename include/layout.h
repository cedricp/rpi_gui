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
	bool 		 m_autoresize;
protected:
	virtual void widget_added_event(Widget* widget);
public:
	Layout(int x=0, int y=0, int w=0, int h=0, const char* name = "", Widget* parent = NULL, Layout_style style = LAYOUT_VERTICAL);
	~Layout();

	void style(Layout_style style);
	void compute_layout();
	void parent_resize_event(const IBbox& bbox);
	void autoresize(bool a){m_autoresize = a;}
	bool autoresize(){return m_autoresize;}
};

#endif
