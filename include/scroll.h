#ifndef SCROLL_H
#define SCROLL_H

#include "widget.h"

class Scroll : public Widget
{
	Widget* m_scroll_widget;
	int m_oldx, m_oldy;
protected:
	virtual void draw();
public:
	Scroll(int x=0, int y=0, int width=0, int height=0, const char* name = "", Widget* parent = NULL);
	virtual bool accept_drag(int x, int y);
	virtual bool drag_event(int rel_x, int rel_y);
	virtual void widget_added_event(Widget* widget);

	void set_scroll_widget(Widget* scroll_widget);
	void reset();
};

#endif
