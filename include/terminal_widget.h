#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include "widget.h"
#include "label.h"

class Terminal_widget : public Widget
{
	Label* m_text;
protected:
	virtual void draw();
	virtual bool custom_event(void* data);
	virtual bool key_press_event(const char* key);
public:
	struct impl;
	impl *m_impl;
	~Terminal_widget();
	Terminal_widget(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
};

#endif
