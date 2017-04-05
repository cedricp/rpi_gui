#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

enum halign {
	ALIGN_CENTERH = 1,
	ALIGN_LEFT,
	ALIGN_RIGHT
} ;

enum valign {
	ALIGN_CENTERV = 1,
	ALIGN_TOP,
	ALIGN_BOTTOM
} ;

class Label : public Widget
{
	halign m_halign;
	valign m_valign;
	Text_data m_label_info;
protected:
	virtual void draw();
	bool mouse_release_event(int button){
		do_callback();
		return true;
	}
public:
	~Label();
	Label(int x=0, int y=0, int width=0, int height=0, const char* name = "", Widget* parent = NULL);
	void alignment(halign h, valign v);
	void label(std::string);
};

#endif
