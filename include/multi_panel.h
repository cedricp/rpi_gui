#ifndef MULTI_PANEL_H_
#define MULTI_PANEL_H_

#include "widget.h"
#include "button.h"
#include "layout.h"

class Multi_panel : public Widget
{
public:
	Multi_panel(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
	~Multi_panel();
	Button* add_tab(Widget* w);
	void show_tab(int tabnum);
private:
	Widget* m_visible_widget;
	std::vector<Widget*> m_widgets;
	std::vector<Button*> m_buttons;
	Layout* m_buttons_layout;
	void button_callback(int butnum);
	static void static_button_callback(Widget* w, void* data){
		((Multi_panel*)data)->button_callback(((Button*)w)->id());
	}
protected:
	//void draw();
};

#endif
