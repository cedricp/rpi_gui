#include "label.h"

Label::Label(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_halign = ALIGN_CENTERH;
	m_valign = ALIGN_CENTERV;
	m_label = name;
}

void
Label::alignment(halign h, valign v)
{
	m_halign = h;
	m_valign = v;
}

void
Label::label(std::string l)
{
	m_label = l;
}

void
Label::draw()
{
	if (m_label.empty())
		return;

	IBbox bound(painter().bound_text(m_font_id, m_label.c_str()));
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
		yt = (h() - bound.height()) / 2 + bound.height() / 2;
		break;
	case ALIGN_TOP:
		yt = bound.height();
		break;
	case ALIGN_BOTTOM:
		yt = h();
		break;
	}


	painter().draw_text(m_font_id, m_label.c_str(), xt, yt);
}
