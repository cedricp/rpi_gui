#ifndef MULTI_PANEL_H_
#define MULTI_PANEL_H_

#include "widget.h"
#include "button.h"
#include "layout.h"

enum Panel_layout{
	PANEL_LAYOUT_HORIZONTAL,
	PANEL_LAYOUT_VERTICAL
};

class Multi_panel : public Widget
{
public:
	Multi_panel(int x = 0, int y = 0, int width = 0, int height = 0, const char* name = "", Widget* parent = NULL);
	~Multi_panel();
	Button* add_tab(Widget* w);
	void show_tab(int tabnum);
	void layout_style(Panel_layout l);
private:
	Widget* m_visible_widget;
	std::vector<Widget*> m_widgets;
	std::vector<Button*> m_buttons;
	Layout* m_buttons_layout;
	Panel_layout m_layout_style;
	void button_callback(int butnum);
	void compute_layout();
	static void static_button_callback(Widget* w, void* data){
		((Multi_panel*)data)->button_callback(((Button*)w)->id());
	}
protected:
	//void draw();
	virtual void resize(int x, int y, int w, int h);
};

#endif
