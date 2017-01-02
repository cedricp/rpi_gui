#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

class Button : public Widget
{
	FColor m_color_push, m_color_hover, m_original_fg, m_text_color;
	unsigned int m_image_id;
	int m_imgw, m_imgh;
	Text_data m_text_data;
	int m_id;
	vertex_container* m_rounded_rect_data;
protected:
	virtual void draw();
public:
	Button(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
	~Button();

	virtual bool mouse_press_event(int button);
	virtual bool mouse_release_event(int button);
	virtual bool leave_event();
	virtual bool enter_event();

	void set_image(std::string image);
	void set_label(std::string label);

	int text_width();
	int text_height();

	void text_color(const FColor& tc);

	void id(int id){
		m_id = id;
	}
	int id(){
		return m_id;
	}
};

#endif
