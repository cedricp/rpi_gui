#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

class Button : public Widget
{
	FColor m_color_push, m_color_hover, m_original_fg;
	unsigned int m_image_id;
	int m_imgw, m_imgh;
	std::string m_label;

protected:
	virtual void draw();
public:
	Button(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);

	virtual bool mouse_press_event(int button);
	virtual bool mouse_release_event(int button);
	virtual bool leave_event();
	virtual bool enter_event();
	//virtual bool accept_drag(){return true;}

	void set_image(std::string image);
};

#endif
