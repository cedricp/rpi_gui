#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

class Button : public Widget
{
public:
	enum BUTTON_STYLE{
		STYLE_ROUNDED_FILLED,
		STYLE_ROUNDED,
		STYLE_TAB,
		STYLE_CARBON
	};
	enum ICON_ALIGN{
		ICON_ALIGN_LEFT,
		ICON_ALIGN_CENTER,
		ICON_ALIGN_RIGHT
	};
private:
	FColor m_color_push, m_color_hover, m_original_fg, m_text_color;
	unsigned int m_image_id;
	int m_imgw, m_imgh;
	Text_data m_text_data;
	int m_id;
	BUTTON_STYLE m_style;
	bool m_toggled;
	ICON_ALIGN m_icon_align;
	vertex_container* m_rounded_rect_data;
protected:
	virtual void draw();
public:
	Button(int x=0, int y=0, int width=0, int height=0, const char* name = "", Widget* parent = NULL);
	~Button();

	virtual bool mouse_press_event(int button);
	virtual bool mouse_release_event(int button);
	virtual bool leave_event();
	virtual bool enter_event();

	void image(std::string image);
	void label(std::string label);
	void style(BUTTON_STYLE style);

	int text_width();
	int text_height();

	void text_color(const FColor& tc);
	void toggle(bool);
	bool toggled();

	void icon_align(ICON_ALIGN a);

	void id(int id){
		m_id = id;
	}
	int id(){
		return m_id;
	}
};

#endif
