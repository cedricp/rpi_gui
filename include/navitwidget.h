#ifndef NAVIT_H
#define NAVIT_H

#include "widget.h"

struct impl;

class Navit_widget : public Widget
{
	impl* m_impl;
protected:
	virtual void draw();
	virtual void resize(int x, int y, int w, int h);
public:
	Navit_widget(int x=0, int y=0, int width=0, int height=0, const char* name = "", Widget* parent = NULL);
	~Navit_widget();

	virtual bool mouse_release_event(int button);
	virtual bool mouse_press_event(int button);
	virtual bool mouse_motion_event(int x, int y);
	virtual bool leave_event();
	virtual bool custom_event(void* data);
};

#endif
