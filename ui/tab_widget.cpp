#include "tab_widget.h"

Tab_widget::Tab_widget(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name , parent)
{
	m_tabs = new Tab_container(0, 0, width, 40, "internal_tab_scroll", this);
	m_visible_widget = NULL;
	vertical_margin(4);
	horizontal_margin(4);
}

void
Tab_widget::add_tab(Widget* wid)
{
	if (wid->parent() != this)
		wid->parent(this);

	wid->resize(0, 41, wid->w(), wid->h() -41);
	Button* button = m_tabs->add_button(wid->name());
	m_widgets.push_back(wid);
	button->callback(static_button_callback, (void*)this);
	button->id(m_widgets.size()-1);

	// First widget
	if (m_widgets.size() == 1){
		wid->show();
		m_visible_widget = wid;
		dirty(true);
	} else {
		wid->hide();
	}
}

void
Tab_widget::show_tab(int tabnum){
	if (tabnum < m_widgets.size()){

		if(m_visible_widget){
			m_visible_widget->hide();
		}
		m_visible_widget = m_widgets[tabnum];
		m_visible_widget->show();
		dirty(true);
	}
}

void
Tab_widget::draw()
{
	painter().disable_texture();
	painter().enable_alpha(false);
	painter().draw_quad(0,0, w(), h(), false, 2.);
}

void
Tab_widget::button_callback(int butnum)
{
	show_tab(butnum);
}
