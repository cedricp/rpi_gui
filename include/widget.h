#ifndef WIDGET_H
#define WIDGET_H

#include "color.h"
#include "bbox.h"
#include "events.h"
#include "painter.h"
#include "keycode.h"
#include <vector>
#include <string>

struct WImpl;
class Widget;
struct UIEvent;

typedef void (WCallback )(Widget*, void*);
typedef WCallback* WCallback_p;

#define attachCallback( widget, method ) \
   widget->callback( reinterpret_cast< void(*)(Widget*,void*) >( method ), this )

class Widget{
    friend class Compositor;
    IBbox   	m_bbox;
    bool   		m_visibility, m_transparent, m_is_root, m_resize_children, m_modal;
    WCallback	*m_callback;
    void		*m_callbackdata;
    std::string m_name;
    bool   		m_dirty;
    WImpl*  	m_impl;
    Painter* 	m_painter;
    std::vector<float*> m_model_matrix_stack;
    void*		m_user_data;
    bool 		m_refresh_backbuffer;
    std::vector<int> m_timers;
    
    void add_child(Widget* w);
    void remove_child(Widget* w);
    void internal_draw(bool force = false);
    void get_all_children(std::vector<Widget*>&);
    bool internal_key_event(KB_Scancode scancode, bool press);
    bool internal_mouse_button_event(int x, int y, int button, Widget **w, bool press);
    bool internal_mouse_motion_event(int x, int y, Widget **w);
    bool internal_mouse_wheel_event(int x, int y, int we, Widget **w);
    Widget* child_widget_in(int x, int y);

protected:
    std::vector<Widget*> m_children_widgets;
    int  		m_fixed_width, m_fixed_height, m_horizontal_margin, m_vertical_margin;
    unsigned int m_pattern_texture;
    Widget 		*m_parent;
    FColor 		m_bgcolor;
    FColor 		m_fgcolor;
    FColor		m_bg_gradient_bottom, m_bg_gradient_top;
    int 		m_font_id;
    bool		m_bg_gradient_enabled, m_tiles_enabled;
    Matrix		m_projection_matrix, m_model_matrix;
    void		do_callback(Widget* w, void* arg);
    void		do_callback();
    static void default_callback(Widget*, void*);

    virtual void parent_resize_event(int width, int height);
    virtual bool enter_event();
    virtual bool leave_event();
    virtual bool key_press_event(KB_Scancode code);
    virtual bool key_release_event(KB_Scancode code);
    virtual bool mouse_press_event(int button);
    virtual bool mouse_release_event(int button);
    virtual bool mouse_wheel_event(int button);
    virtual bool mouse_motion_event(int x, int y);
    virtual bool drag_event(int rel_x, int rel_y);
    virtual bool timer_event(void* data);
    virtual bool custom_event(void* data);
    virtual bool accept_drag(int x, int y);
    virtual void widget_added_event(Widget* widget);
    virtual void init_viewport(int x, int y, int width, int height);
    virtual void fonts_changed(int font_id);

    virtual void draw();
    virtual void post_draw();

public:
    Widget(int x=0, int y=0, int width=0, int height=0, const char* name = "", Widget* parent = NULL);
    virtual ~Widget();

    void user_data(void* ud){m_user_data = ud;}
    void* user_data(){return m_user_data;}

    bool touchscreen_enabled();
    bool backbuffer_refresh(){return m_refresh_backbuffer;}
    void backbuffer_refresh(bool r){m_refresh_backbuffer = r;}

    Widget* parent(){
        return m_parent;
    }
    
    std::string name(){return m_name;}

    void fg_color(const FColor& color){
        m_fgcolor = color;
    }
    
    void bg_color(const FColor& color){
        m_bgcolor = color;
    }

    void gradient(const FColor&top, const FColor& bottom);

    void backgroung_gradient_enable(bool e){
    	m_bg_gradient_enabled = e;
    }

    void tiles_enabled(bool e){
    	m_tiles_enabled = e;
    }

    int w();
    int h();
    int x();
    int y();

    void x(int x);
    void y(int y);

    IBbox 	screen_bbox_internal();
    IBbox 	screen_bbox();
    IBbox 	relative_bbox();
    void	drawing_area(IBbox& area);
    void 	screen_to_widget_coordinates(int sx, int sy, int &wx, int &wy);
    int		add_timer(int ms);
    void	remove_timer(int timer_id);
    void	modal(int x, int y);

    virtual void resize(int x, int y, int w, int h);
    virtual void resize(int ww, int hh);

    void move(int x, int y);

    void push_model_matrix();
    void pop_model_matrix();
    void translate(float x, float y, float z = 0.);
    void rotate(float x, float y, float z, float angle);
    void scale(float x, float y, float z);
    void identity();
    
    void mouse_coordinates(int&x, int&y);

    void hide();
    void show();
    void close();
    void destroy_children();
    
    int  fixed_width(){return m_fixed_width;}
    int  fixed_height(){return m_fixed_height;}

    void  fixed_width(int w);
    void  fixed_height(int h);

    void horizontal_margin(int h){m_horizontal_margin = h;}
    void vertical_margin(int h){m_vertical_margin = h;}

    int horizontal_margin(){return m_horizontal_margin;}
    int vertical_margin(){return m_vertical_margin;}

    bool hidden();
    void parent(Widget* w);
    bool update(bool full_redraw = false, bool clear_dirty_flag = true);
    void damage(const IBbox& other);
    void transparent(bool on){m_transparent = on;}
    void set_background_tiles(std::string filename);

    UIEvent* create_event(void* data = NULL);
    static void push_event(UIEvent* evt);

    static Painter& painter();

    bool dirty() {
        return m_dirty;
    }
    
    void dirty(bool d){
        m_dirty = d;
    }
    
    void margin(int w, int h){
    	m_vertical_margin = h;
    	m_horizontal_margin = w;
    }

    void root(bool r){m_is_root = r;}
    bool root(){return m_is_root;}

    void use_fonts(std::string font_name);
    void use_fonts_id(int font_id);
    void use_default_fonts();
    int  load_fonts(std::string font_id, int size, int atlas_size = 1024);
    void set_top_widget(Widget* w);

    void callback(WCallback* cb);
    void callback(WCallback* cb, void* user_data);

    void resize_children(bool r){m_resize_children = r;}
};

#endif
