#include "label.h"

Label::Label(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_halign = ALIGN_CENTERH;
	m_valign = ALIGN_CENTERV;
	label(name);
}

Label::~Label()
{

}

void
Label::alignment(halign h, valign v)
{
	m_halign = h;
	m_valign = v;
}

void
Label::fonts_changed(int font_id)
{
	text_build();
	dirty(true);
}

void
Label::label(std::string l)
{
	if (l == m_current_string)
		return;

	m_current_string = l;
	text_build();
	dirty(true);
}

void
Label::text_build()
{
	painter().build_text(m_font_id, m_current_string.c_str(), 0, 0, m_label_info);
}

int
Label::text_width()
{
	return m_label_info.bbox.width();
}

int
Label::text_height()
{
	return m_label_info.bbox.height();
}

void
Label::draw()
{
	IBbox bound(m_label_info.bbox);
	int xt;
	int yt;

	switch(m_halign){
	case ALIGN_CENTERH:
		xt = (w() - bound.width()) / 2;
		break;
	case ALIGN_LEFT:
		xt = 0;
		break;
	case ALIGN_RIGHT:
		xt = w() - bound.width();
		break;
	}

	switch(m_valign){
	case ALIGN_CENTERV:
		yt = (h() - bound.height()) / 2 + bound.height();
		break;
	case ALIGN_TOP:
		yt = h() - bound.height();
		break;
	case ALIGN_BOTTOM:
		yt = h();
		break;
	}

	push_model_matrix();
	translate(xt, yt);
	painter().draw_text(m_label_info);
	pop_model_matrix();
}
