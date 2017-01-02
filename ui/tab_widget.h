#ifndef TAB_WIDGET_H
#define TAB_WIDGET_H

#include "widget.h"
#include "button.h"
#include "scroll.h"
#include "layout.h"
#include "button.h"

class Tab_container : public Widget
{
	std::vector<Button*> m_buttons;
public:
	Tab_container(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL) : Widget(x, y, width, height, name, parent){

	}

	Button* add_button(std::string name){
		Button* button = new Button(0, 0, 0, 0, name.c_str(), this);
		button->bg_color(FColor(0, 0., .2, 1));
		m_buttons.push_back(button);
		recompute_width();

		return button;
	}

	void recompute_width(){
		int width = 0;
		for (int i = 0; i < m_buttons.size(); ++i){
			int text_width = m_buttons[i]->text_width() + 10;
			m_buttons[i]->resize(width, 5,  text_width, h() - 5);
			width += text_width;
		}
		resize(width, h());
	}
	
};

class Tab_widget : public Widget
{
	Tab_container* m_tabs;
	Widget* m_visible_widget;
	std::vector<Widget*> m_widgets;
protected:
	void draw();
public:
	Tab_widget(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
	void add_tab(Widget* w);
	void show_tab(int tabnum);
	void button_callback(int butnum);

	static void static_button_callback(Widget* w, void* data){
		((Tab_widget*)data)->button_callback(((Button*)w)->id());
	}
};

#endif
