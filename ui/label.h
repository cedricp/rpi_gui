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
	std::string m_label;
	halign m_halign;
	valign m_valign;
protected:
	virtual void draw();
public:
	Label(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
	void alignment(halign h, valign v);
	void label(std::string);
};

#endif
