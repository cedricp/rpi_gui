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
Label::label(std::string l)
{
	painter().build_text(m_font_id, l.c_str(), 0, 0, m_label_info);
	dirty(true);
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
		yt = bound.height();
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
