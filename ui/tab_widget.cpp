/***
 *     ____    ____   _____ ____   ____     ___  ____   ____   __ __       ____  __ __  ____
 *    |    \  /    | / ___/|    \ |    \   /  _]|    \ |    \ |  |  |     /    ||  |  ||    |
 *    |  D  )|  o  |(   \_ |  o  )|  o  ) /  [_ |  D  )|  D  )|  |  |    |   __||  |  | |  |
 *    |    / |     | \__  ||   _/ |     ||    _]|    / |    / |  ~  |    |  |  ||  |  | |  |
 *    |    \ |  _  | /  \ ||  |   |  O  ||   [_ |    \ |    \ |___, |    |  |_ ||  :  | |  |
 *    |  .  \|  |  | \    ||  |   |     ||     ||  .  \|  .  \|     |    |     ||     | |  |
 *    |__|\_||__|__|  \___||__|   |_____||_____||__|\_||__|\_||____/     |___,_| \__,_||____|
 *
 * (C) 2017 Cedric PAILLE (cedricpaille(at)gmail.com)
 */

#include "tab_widget.h"

Tab_widget::Tab_widget(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name , parent)
{
	m_tabs = new Tab_container(1, 1, width, 25, "internal_tab_scroll", this);
	m_visible_widget = NULL;
	vertical_margin(4);
	horizontal_margin(4);
}

void
Tab_widget::add_tab(Widget* wid)
{
	if (wid->parent() != this)
		wid->parent(this);

	wid->resize(2, 27, w() - 4, h() - 29);
	Button* button = m_tabs->add_button(wid->name());
	button->style(Button::STYLE_TAB);
	m_widgets.push_back(wid);
	attachCallback(button, static_button_callback);
	button->id(m_widgets.size()-1);

	// First widget
	if (m_widgets.size() == 1){
		wid->show();
		button->toggle(true);
		m_visible_widget = wid;
		dirty(true);
	} else {
		wid->hide();
	}
}

void
Tab_widget::show_tab(int tabnum){
	if (tabnum < m_widgets.size()){
		m_tabs->activate(tabnum);
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
	//painter().disable_texture();
	//painter().enable_alpha(false);
	//painter().draw_quad(0,0, w(), h(), false, false, 2.5);
}

void
Tab_widget::button_callback(int butnum)
{
	show_tab(butnum);
}
