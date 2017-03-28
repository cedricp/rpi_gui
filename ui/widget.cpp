#include "widget.h"
#include "compositor.h"

#include <algorithm>
#include <iostream>
#include <string.h>

#include <SDL2/SDL.h>

struct WImpl{
	int		timer_id;
};

struct UIEvent
{
	SDL_Event 	event;
};

Widget::Widget(int x, int y, int width, int height, const char* name, Widget* parent)
{
    m_impl      = new WImpl;
	m_parent 	= parent;
    m_bgcolor 	= FColor(0., 0., 0., 1.);
    m_fgcolor 	= FColor(1., 1., 1., 1.);
    m_bg_gradient_enabled = false;
    m_tiles_enabled		  = false;
    m_is_root	= false;
    
    m_bg_gradient_top = FColor(0.2f,0.2f,.2f, 1.f);
    m_bg_gradient_bottom = FColor(.5f,.5f,.5f, 1.0f);

    if ((width == 0 || height == 0) && parent){
    	m_bbox = IBbox(x, x + parent->w(), y, y +  parent->h());
    } else {
    	m_bbox = IBbox(x, x + width, y, y + height);
    }

    m_name      	= name;
    m_dirty     	= true;
    m_visibility	= true;
    m_callback  	= default_callback;
    m_callbackdata 	= NULL;
    m_transparent	= false;
    m_fixed_width 	= m_fixed_height = -1;
    m_horizontal_margin = m_vertical_margin = 0;
    use_default_font();
    m_pattern_texture = painter().create_texture_bmp("tiles/carbon.bmp");

    matrix4_identity(m_model_matrix);

    if (parent == NULL){
        COMPOSITOR->add_widget(this);
    } else {
        parent->add_child(this);
    }
}

Widget::~Widget()
{
	painter().delete_texture(m_pattern_texture);
    delete m_impl;
    std::vector<float*>::iterator it = m_model_matrix_stack.begin();
    for (; it != m_model_matrix_stack.end(); ++it)
    	delete(*it);
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
Widget::set_background_tiles(std::string filename)
{
	if (m_pattern_texture != (unsigned int)-1)
		painter().delete_texture(m_pattern_texture);

	m_pattern_texture = painter().create_texture_bmp(filename);
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

void
Widget::post_draw()
{

}

void Widget::init_viewport(int x, int y, int width, int height)
{
    painter().viewport( x, y, width, height );
    painter().create_ortho_matrix(0, width, height, 0, -1.0, 1.0, m_projection_matrix);
    painter().load_projection_matrix(m_projection_matrix);
    painter().load_model_matrix(m_model_matrix);
}

void Widget::internal_draw(bool force)
{
	if (!force){
		if (!m_dirty)
			return;
	}

	if (m_visibility == false)
		return;

    int screen_h 	= COMPOSITOR->screen_height();
    IBbox bscr 		= screen_bbox_corrected();

    // Clamp drawing to parent size
    Widget* parent = m_parent;
    while (parent){
    	bscr.crop(parent->screen_bbox_corrected());
    	parent = parent->m_parent;
    }

    // Widget is outside parent bounds
    if (bscr.width() == 0 || bscr.height() == 0){
    	return;
    }

    // Start drawing stuffs
    init_viewport(screen_bbox_corrected().xmin(), screen_bbox_corrected().ymin(), screen_bbox_corrected().width(), screen_bbox_corrected().height());
    painter().scissor_begin( bscr.xmin(), bscr.ymin(), bscr.width(), bscr.height() );
    if (!m_transparent)
    	painter().clear_color_buffer(m_bgcolor);

    if (m_bg_gradient_enabled || m_tiles_enabled)
    	painter().draw_quad_gradient(0, 0, w(), h(), m_bg_gradient_enabled ? m_bg_gradient_top : m_bgcolor,
    								 m_bg_gradient_enabled ? m_bg_gradient_bottom : m_bgcolor,
    								 m_tiles_enabled ? m_pattern_texture : -1, 18);

    painter().color(m_fgcolor);
    // Call widget custom draw method
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
    m_dirty = true;
}

void
Widget::show()
{
    m_visibility = true;
    m_dirty = true;
}

void
Widget::destroy_children()
{
	std::vector<Widget*>::iterator it = m_children_widgets.begin();
    for (; it != m_children_widgets.end(); ++it){
        (*it)->destroy_children();
        COMPOSITOR->advert_widget_deleted(*it);
        delete(*it);
    }
    m_children_widgets.clear();
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
         w->add_child(this);
    } else {
        if (comp->widget_exists(this)){
            comp->remove_widget(this);
        }
        w->add_child(this);
    }
    m_parent = w;
}

bool
Widget::accept_drag(int x, int y)
{
	return false;
}

void
Widget::add_child(Widget* w)
{
    if (std::find(m_children_widgets.begin(), m_children_widgets.end(), w) == m_children_widgets.end()){
        m_children_widgets.push_back(w);
    } else {
        std::cerr << "Warning : trying to re-add child widget" << std::endl;
    }
    widget_added_event(w);
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
	if (m_visibility == false)
		return;

    std::vector<Widget*>::iterator it = m_children_widgets.begin();
    for (; it < m_children_widgets.end(); ++it){
    	if ((*it)->m_dirty && (*it)->m_transparent){
    		full_redraw = true;
    		break;
    	}
    }

    if (m_dirty)
    	full_redraw = true;

	if (m_dirty || full_redraw)
    	internal_draw(full_redraw);

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
	if (m_parent && m_parent->root()){
		m_parent->set_top_widget(this);
		dirty(true);
		return true;
	} else if (m_parent){
		return m_parent->enter_event();
	}
	return false;
}

void
Widget::set_top_widget(Widget* w)
{
	std::vector<Widget*>::iterator it = std::find(m_children_widgets.begin(), m_children_widgets.end(), w);
	if (it == m_children_widgets.end())
		return;
	m_children_widgets.erase(it);
	m_children_widgets.push_back(w);
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

bool
Widget::custom_event(void* data)
{
	return false;
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

void
Widget::drawing_area(IBbox &area)
{
	area.xmin(m_horizontal_margin);
	area.ymin(m_vertical_margin);
	area.xmax(m_bbox.width() - m_horizontal_margin*2);
	area.ymax(m_bbox.height() - m_vertical_margin*2);
}

IBbox
Widget::screen_bbox_corrected()
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

    int new_y = COMPOSITOR->screen_height() - y - height;
    return IBbox(x, x + width, new_y, new_y + height);
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
Widget::internal_mouse_motion_event(int xx, int yy, Widget **w){
	if (hidden()) return false;
	std::vector<Widget*>::reverse_iterator it = m_children_widgets.rbegin();
	for (; it < m_children_widgets.rend(); ++it){
		bool infocus = (*it)->screen_bbox().contains(xx, yy);
		if (infocus){
			 bool ev = (*it)->internal_mouse_motion_event(xx, yy, w);
			 if (ev)
				 break;
		}
	}

	int xp, yp;
	mouse_pos(xp, yp);
	mouse_motion_event(xp, yp);

	if(*w == NULL){
		*w = this;
		return true;
	}

	return false;
}

void
Widget::mouse_pos(int& x, int& y)
{
	COMPOSITOR->mouse_position(x, y);
	IBbox b = screen_bbox();
	x -= b.xmin();
	y -= b.ymin();
}

bool
Widget::internal_key_event(const char* code, bool press)
{
	bool taken = false;
	if (press)
		taken = key_press_event(code);
	else
		taken = key_release_event(code);

	if (!taken && parent()){
		taken = parent()->internal_key_event(code, press);
	}
	return taken;
}

bool
Widget::key_press_event(const char* code)
{
	return false;
}

bool
Widget::key_release_event(const char* code)
{
	return false;
}

bool
Widget::internal_mouse_button_event(int x, int y, int button, Widget **w, bool press)
{
	if (hidden()) return false;

	std::vector<Widget*>::reverse_iterator it = m_children_widgets.rbegin();
	for (; it < m_children_widgets.rend(); ++it){
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
	if (hidden()) return false;
	std::vector<Widget*>::reverse_iterator it = m_children_widgets.rbegin();
	for (; it < m_children_widgets.rend(); ++it){
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
Widget::x(int x)
{
	int w_hold= w();
	m_bbox.xmin(x);
	m_bbox.xmax(x + w_hold);
}

void
Widget::y(int y)
{
	int h_hold = h();
	m_bbox.ymin(y);
	m_bbox.ymax(y + h_hold);
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

void
Widget::widget_added_event(Widget* widget)
{

}

void
Widget::push_model_matrix()
{
	float* matrix = new float[16];
	memcpy(matrix, m_model_matrix, 16*sizeof(float));
	m_model_matrix_stack.push_back(matrix);
}

void
Widget::pop_model_matrix()
{
	if (m_model_matrix_stack.empty())
		return;
	float* matrix = m_model_matrix_stack.back();
	memcpy(m_model_matrix, matrix, 16*sizeof(float));
	delete[] matrix;
	m_model_matrix_stack.pop_back();
	painter().load_model_matrix(m_model_matrix);
}

void
Widget::translate(float x, float y, float z)
{
	Matrix pos_matrix;
	matrix4_translate(pos_matrix, x, y);
	matrix4_mult(m_model_matrix, pos_matrix, m_model_matrix);
	painter().load_model_matrix(m_model_matrix);
}

void
Widget::rotate(float x, float y, float z, float angle)
{
	Matrix pos_matrix;
	matrix4_rotate(pos_matrix, x, y, z, angle * M_PI / 180.);
	matrix4_mult(m_model_matrix, pos_matrix, m_model_matrix);
	painter().load_model_matrix(m_model_matrix);
}

UIEvent*
Widget::create_event(void* data){
	UIEvent* event = new UIEvent;
	SDL_memset(&event->event, 0, sizeof(SDL_Event));
	Uint32 evt = SDL_RegisterEvents(1);
	if (evt == (Uint32)-1)
		return NULL;
	if (evt != ((Uint32)-1)) {
	    event->event.type = evt;
	    event->event.user.code = USER_EVENT;
	    event->event.user.data1 = this;
	    event->event.user.data2 = data;
	}
	return event;
}

void
Widget::push_event(UIEvent* evt){
	if (evt)
		SDL_PushEvent(&evt->event);
}
