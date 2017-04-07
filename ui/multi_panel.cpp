#include "multi_panel.h"

Multi_panel::Multi_panel(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name , parent)
{
	m_visible_widget = NULL;
	m_layout_style = PANEL_LAYOUT_HORIZONTAL;
	m_buttons_layout = new Layout(0, 0, 0, 0, "MainLayout", this, LAYOUT_HORIZONTAL);
	resize_children(false);
}

Multi_panel::~Multi_panel()
{

}

Button*
Multi_panel::add_tab(Widget* wid)
{
	if (wid->parent() != this)
		wid->parent(this);

	m_widgets.push_back(wid);

	Button* button = new Button(0, 0, 0, 0, wid->name().c_str(), m_buttons_layout);
	button->style(Button::STYLE_CARBON);
	m_buttons.push_back(button);

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

	attachCallback(button, static_button_callback);

	compute_layout();

	return button;
}

void
Multi_panel::show_tab(int tabnum)
{
	if (tabnum < m_widgets.size()){
		for (int i = 0; i <	m_buttons.size(); ++i){
			m_buttons[i]->toggle(false);
		}
		if(m_visible_widget){
			m_visible_widget->hide();
		}
		m_visible_widget = m_widgets[tabnum];
		m_visible_widget->show();
		m_buttons[tabnum]->toggle(true);
		m_visible_widget->dirty(true);
	}
}

void
Multi_panel::button_callback(int butnum)
{
	show_tab(butnum);
}

void
Multi_panel::compute_layout()
{
	int ratio = 6;

	if (m_layout_style == PANEL_LAYOUT_VERTICAL){
		int buttons_layout_size_x = w() / ratio + 1;

		for (int i = 0; i < m_buttons.size(); ++i){
			m_buttons[i]->fixed_width(buttons_layout_size_x - 2);
			m_buttons[i]->fixed_height(-1);
		}
		m_buttons_layout->resize(0,0, w() / ratio, h());
		for (int i = 0; i < m_widgets.size(); ++i){
			m_widgets[i]->resize(buttons_layout_size_x, 0, w() - buttons_layout_size_x, h());
		}
	} else {
		int buttons_layout_size_y = h() / ratio + 1;

		for (int i = 0; i < m_buttons.size(); ++i){
			m_buttons[i]->fixed_height(buttons_layout_size_y - 2);
			m_buttons[i]->fixed_width(-1);
		}
		m_buttons_layout->resize(0,0, w(), h() / ratio);
		for (int i = 0; i < m_widgets.size(); ++i){
			m_widgets[i]->resize(0, buttons_layout_size_y, w(), h() - buttons_layout_size_y);
		}
	}

	dirty(true);
}

void
Multi_panel::layout_style(Panel_layout l)
{
	m_layout_style = l;
	if (l == PANEL_LAYOUT_HORIZONTAL)
		m_buttons_layout->style(LAYOUT_HORIZONTAL);
	else
		m_buttons_layout->style(LAYOUT_VERTICAL);
	compute_layout();
}

void
Multi_panel::resize(int x, int y, int ww, int hh)
{
	Widget::resize(x, y, ww, hh);
	compute_layout();
}
