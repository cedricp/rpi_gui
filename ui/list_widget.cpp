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

#include "list_widget.h"
#include "label.h"
#include "layout.h"
#include "scroll.h"
#include "compositor.h"
#include "button.h"
#include <dirent.h>
#include <stdio.h>
#include <algorithm>


List_widget::List_widget(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_scroll_view = new Scroll(0, 0, 0, 0, "ScrollView", this);
	m_list_layout = new Layout(0, 0, 0, 0, "ListLayout", m_scroll_view, LAYOUT_VERTICAL);


	backgroung_gradient_enable(true);
	tiles_enabled(true);
	m_scroll_view->transparent(true);
	m_list_layout->transparent(true);
}

void
List_widget::init()
{
	m_list_layout->destroy_children();

	int h = 0;
	IBbox this_area;
	m_scroll_view->drawing_area(this_area);

	int max_width = this_area.width();

	for (int i = 0; i < m_list.size(); ++i){
		Label* entry_label = new Label(0, 0, 0, 0, m_list[i].c_str(), m_list_layout);
		int text_height = entry_label->text_height() + 6;
		entry_label->alignment(ALIGN_LEFT, ALIGN_CENTERV);
		entry_label->fixed_height(text_height);
		entry_label->fg_color(FColor(0.2, 0, 1, 1));
		attachCallback(entry_label, static_click_callback);
		entry_label->transparent(true);
		if (entry_label->text_width() > max_width)
			max_width = entry_label->text_width();
		h += text_height;
	}

	m_list_layout->resize(this_area.xmin(), this_area.ymin(), max_width, h);
	m_list_layout->compute_layout();

	// Reset scrollview
	m_scroll_view->reset();

	// We must redraw the view...
	dirty(true);
}

void
List_widget::resize(int xx, int yy, int ww, int hh)
{
	Widget::resize(xx,yy,ww,hh);
	m_scroll_view->resize(0, 0, ww, hh);
}

void
List_widget::click_callback(Label* l)
{
	std::cout << l->name() << std::endl;
}

int
List_widget::add_element(std::string elem)
{
	m_list.push_back(elem);
	init();
	return m_list.size() - 1;
}

void
List_widget::draw()
{
	painter().disable_texture();
	painter().enable_alpha(false);
	painter().color(FColor(.1,.1,.3,1.));
	painter().draw_quad(0,0, w(), h(), false, false, 2.5);
}
