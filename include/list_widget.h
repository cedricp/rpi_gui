#ifndef LIST_WIDGET_H
#define LIST_WIDGET_H

#include "widget.h"

class Layout;
class Scroll;
class Label;

class List_widget : public Widget
{
	std::vector<std::string> m_list;
	Layout* m_list_layout;
	Scroll* m_scroll_view;

	void click_callback(Label* l);
	static void static_click_callback(Widget* w, void* data){
		((List_widget*)data)->click_callback((Label*)w);
	}
protected:
	void draw();
	void resize(int x, int y, int w, int h);
public:
	List_widget(int x=0, int y=0, int width=0, int height=0, const char* name = "", Widget* parent = NULL);
	void init();
	int add_element(std::string);
};

#endif
