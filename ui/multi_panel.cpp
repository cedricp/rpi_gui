#include "multi_panel.h"

Multi_panel::Multi_panel(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name , parent)
{
	m_visible_widget = NULL;
	m_layout_style = PANEL_LAYOUT_HORIZONTAL;
	m_buttons_layout = new Layout(0,0, w() / 6, h(), "MainLayout", this, LAYOUT_HORIZONTAL);
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
	attachCallback(button, static_button_callback);

	// First widget
	if (m_widgets.size() == 1){
		wid->show();
		button->toggle(true);
		m_visible_widget = wid;
		dirty(true);
	} else {
		wid->hide();
	}

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
	if (m_layout_style == PANEL_LAYOUT_VERTICAL){
		int buttons_layout_size_x = m_buttons_layout->w() + 1;

		for (int i = 0; i < m_buttons.size(); ++i){
			m_buttons[i]->fixed_width(buttons_layout_size_x - 2);
		}

		for (int i = 0; i < m_widgets.size(); ++i){
			m_widgets[i]->resize(buttons_layout_size_x, 0, w() - buttons_layout_size_x, h());
		}
		m_buttons_layout->resize(0,0, w() / 6, h());
		m_buttons_layout->compute_layout();
	} else {
		int buttons_layout_size_y = m_buttons_layout->h() + 1;

		for (int i = 0; i < m_buttons.size(); ++i){
			m_buttons[i]->fixed_height(buttons_layout_size_y - 2);
		}

		for (int i = 0; i < m_widgets.size(); ++i){
			m_widgets[i]->resize(0, buttons_layout_size_y, w(), h() - buttons_layout_size_y);
		}
		m_buttons_layout->resize(0,0, w(), h() / 6);
		m_buttons_layout->compute_layout();
	}
}

void
Multi_panel::layout_style(Panel_layout l)
{
	m_layout_style = l;
	if (l == PANEL_LAYOUT_HORIZONTAL)
		m_buttons_layout->style(LAYOUT_HORIZONTAL);
	else
		m_buttons_layout->style(LAYOUT_VERTICAL);
}

void
Multi_panel::resize(int X, int Y, int W, int H)
{
	Widget::resize(X, Y, W, H);
	compute_layout();
	dirty(true);
}
