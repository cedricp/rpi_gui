#include "widget.h"
#include "compositor.h"

#include <algorithm>
#include <iostream>

struct WImpl{
	int		timer_id;
};

Widget::Widget(int x, int y, int width, int height, const char* name, Widget* parent)
{
    m_impl      = new WImpl;
	m_parent 	= parent;
    m_bgcolor 	= FColor(0., 0., 0., 1.);
    m_fgcolor 	= FColor(1., 1., 1., 1.);
    
    m_bbox = IBbox(x, x + width, y, y + height);
    
    if (parent == NULL){
        COMPOSITOR->add_widget(this);
    } else {
        parent->add_child(this);
    }
    
    m_name      	= name;
    m_dirty     	= true;
    m_visibility	= true;
    m_callback  	= default_callback;
    m_callbackdata 	= NULL;
    m_transparent	= false;
    use_default_font();
}

Widget::~Widget()
{
    delete m_impl;
}

Painter&
Widget::painter()
{
	return COMPOSITOR->painter();
}

void
Widget::default_callback(Widget* widget, void* data)
{
	std::cout << "Default callback method" << std::endl;
}

void
Widget::do_callback(Widget* w, void* arg)
{
	m_callback(w, arg);
}

void
Widget::do_callback()
{
	m_callback(this, m_callbackdata);
}

void
Widget::callback(WCallback* cb)
{
	m_callback = cb;
	m_callbackdata = NULL;
}

void
Widget::callback(WCallback* cb, void* user_data)
{
	m_callback = cb;
	m_callbackdata = user_data;
}

void Widget::draw()
{

#ifdef USE_OPENGL

#else

#endif
}

void Widget::internal_draw(bool force)
{
	if (!force){
		if (!m_visibility || !m_dirty)
			return;
	}

    int screen_h = COMPOSITOR->screen_height();
    IBbox bscr = screen_bbox();

    // Clamp drawing to parent size
    if (m_parent){
    	IBbox bpar = m_parent->screen_bbox();
    	if (bscr.xmax() > bpar.xmax()){
    		bscr.xmax(bpar.xmax());
    	}
    	if (bscr.ymax() > bpar.ymax()){
    		bscr.ymax(bpar.ymax());
    	}
    }

    painter().load_identity();
    painter().viewport( bscr.xmin(), screen_h - bscr.ymin() - bscr.height(), bscr.width(), bscr.height() );
    painter().create_ortho_matrix(0, bscr.width(), m_bbox.height(), 0, -1.0, 1.0);
    painter().scissor_begin( bscr.xmin(), screen_h - bscr.ymin() - bscr.height(), bscr.width(), bscr.height() );
    if (!m_transparent)
    	painter().clear_color_buffer(m_bgcolor);
    painter().color(m_fgcolor);
    draw();
    painter().scissor_end();

    m_dirty = false;
}

void
Widget::resize(int x, int y, int w, int h)
{
    m_bbox = IBbox(x, x + w, y, y + h);
    std::vector<Widget*>::iterator it = m_children_widgets.begin();
    for (; it < m_children_widgets.end(); ++it){
    	(*it)->parent_resize_event(m_bbox);
    }
}

void
Widget::resize(int w, int h)
{
    resize(m_bbox.xmin(), m_bbox.ymin(), w, h); 
}

void
Widget::parent_resize_event(const IBbox& bbox)
{

}

void
Widget::hide()
{
    m_visibility = false;
}

void
Widget::show()
{
    m_visibility = true;
}

void
Widget::destroy_children()
{
	std::vector<Widget*>::iterator it = m_children_widgets.begin();
    for (; it != m_children_widgets.end(); ++it){
        (*it)->destroy_children();
    }
}

void
Widget::close()
{
    destroy_children();

    if (m_parent != NULL)
        m_parent->remove_child(this);

    if (COMPOSITOR->widget_exists(this)){
    	COMPOSITOR->remove_widget(this);
    }
    
    delete this;
}

void
Widget::parent(Widget* w)
{
    Compositor * comp = Compositor::get_singleton();

    if (m_parent != NULL){
         m_parent->remove_child(this);
    } else {
        if (comp->widget_exists(this)){
            comp->remove_widget(this);
        }
        comp->add_widget(this);
    }

    m_parent->add_child(w);
}

void
Widget::add_child(Widget* w)
{
    if (std::find(m_children_widgets.begin(), m_children_widgets.end(), w) == m_children_widgets.end()){
        m_children_widgets.push_back(w);
    } else {
        std::cerr << "Warning : trying to re-add child widget" << std::endl;
    }
}

void Widget::remove_child(Widget* w)
{
    std::vector<Widget*>::iterator it = std::find(m_children_widgets.begin(), m_children_widgets.end(), w);
    if (it != m_children_widgets.end()){
        m_children_widgets.erase(it);
    } else {
        std::cerr << "Warning : trying to remove non-child widget" << std::endl;
    }
}

void 
Widget::update(bool full_redraw)
{
    std::vector<Widget*>::iterator it = m_children_widgets.begin();
    for (; it < m_children_widgets.end(); ++it){
    	if ((*it)->m_dirty && (*it)->m_transparent){
    		full_redraw = true;
    		break;
    	}
    }

	if (m_dirty || full_redraw)
    	internal_draw(full_redraw);

    if (m_dirty)
    	full_redraw = true;

    it = m_children_widgets.begin();
    for (; it < m_children_widgets.end(); ++it){
    	(*it)->update(full_redraw);
    }
}

// If a widget is overlapping this one, tag it as dirty to force redraw
void
Widget::damage(const IBbox& other)
{
    if (!m_bbox.intersects(other))
        return;
        
    m_dirty = true;
    std::vector<Widget*>::iterator it = m_children_widgets.begin();
    for (; it < m_children_widgets.end(); ++it){
        (*it)->damage(other);
    }
}

bool
Widget::enter_event()
{
	return false;
}

bool
Widget::leave_event()
{
	return false;
}

bool
Widget::mouse_press_event(int button)
{
	return true;
}

void
Widget::timer_event(void* data)
{
}

bool Widget::mouse_release_event(int button)
{
    return false;
}


bool Widget::mouse_wheel_event(int wheel_ev)
{
	return false;
}

bool
Widget::mouse_motion_event(int x, int y)
{
	return false;
}

bool
Widget::drag_event(int rel_x, int rel_y)
{
	return false;
}

IBbox
Widget::relative_bbox()
{
    return m_bbox;
}

IBbox
Widget::screen_bbox()
{
    int width  = m_bbox.width();
    int height = m_bbox.height();
    int x = m_bbox.xmin();
    int y = m_bbox.ymin();
    
    Widget* parent = m_parent;
    while(parent){
        x += parent->m_bbox.xmin();
        y += parent->m_bbox.ymin();
        parent = parent->m_parent;
    }
    return IBbox(x, x + width, y, y + height);
}

void
Widget::screen_to_widget_coordinates(int sx, int sy, int &wx, int &wy)
{
    int x = 0;
    int y = 0;

    Widget* parent = m_parent;
    while(parent){
        x += parent->m_bbox.xmin();
        y += parent->m_bbox.ymin();
        parent = parent->m_parent;
    }

    wx = sx - x;
    wy = sy - y;
}

Widget*
Widget::child_widget_in(int x, int y){
    Widget* ret = NULL;

    for (int i = 0; i < m_children_widgets.size(); ++i){
        ret = m_children_widgets[i]->child_widget_in(x, y);
        if (ret) break;
    }
      
    if (ret == NULL && screen_bbox().contains(x,y))
        return this;
        
    return NULL;
}

void
Widget::get_all_children(std::vector<Widget*>& list)
{
    list.insert(list.end(), m_children_widgets.begin(), m_children_widgets.end());
    for (int i = 0; i < m_children_widgets.size(); ++i)
        get_all_children(list);
}

bool
Widget::internal_mouse_motion_event(int x, int y, Widget **w){
	bool event_taken = false;

	std::vector<Widget*>::iterator it = m_children_widgets.begin();
	for (; it < m_children_widgets.end(); ++it){
		bool infocus = (*it)->screen_bbox().contains(x, y);
		if (infocus){
			 bool ev = (*it)->internal_mouse_motion_event(x, y, w);
			 if (ev)
				 break;
		}
	}

	bool state;
	int wx, wy;
	screen_to_widget_coordinates(x, y, wx, wy);

	if(*w == NULL){
		*w = this;
		return true;
	}

	return false;
}

bool
Widget::internal_mouse_button_event(int x, int y, int button, Widget **w, bool press)
{
	bool event_taken = false;

	std::vector<Widget*>::iterator it = m_children_widgets.begin();
	for (; it < m_children_widgets.end(); ++it){
		bool infocus = (*it)->screen_bbox().contains(x, y);
		if (infocus){
			 bool ev = (*it)->internal_mouse_button_event(x, y, button, w, press);
			 if (ev)
				 return true;
		}
	}

	bool state;
	if (press){
		state = mouse_press_event(button);
	} else {
		state = mouse_release_event(button);
	}

	if(!*w && state){
		*w = this;
		return true;
	}

	return false;
}

bool
Widget::internal_mouse_wheel_event(int x, int y, int we, Widget **w)
{
	bool event_taken = false;

	std::vector<Widget*>::iterator it = m_children_widgets.begin();
	for (; it < m_children_widgets.end(); ++it){
		bool infocus = (*it)->screen_bbox().contains(x, y);
		if (infocus){
			 bool ev = (*it)->internal_mouse_wheel_event(x, y, we, w);
			 if (ev)
				 return true;
		}
	}

	bool state = mouse_wheel_event(we);

	if(!*w && state){
		*w = this;
		return true;
	}

	return false;
}

int
Widget::w()
{
	return m_bbox.width();
}

int
Widget::h()
{
	return m_bbox.height();
}

int
Widget::x()
{
	return m_bbox.xmin();
}

int
Widget::y()
{
	return m_bbox.ymin();
}

void
Widget::use_font(std::string font_name)
{
	m_font_id = painter().font_by_name(font_name);
}

void
Widget::use_default_font()
{
	m_font_id = painter().default_font_idx();
}

void
Widget::load_font(std::string font_filename, int size, int atlas_size)
{
	m_font_id  = painter().load_fonts(font_filename, size, atlas_size);
}
