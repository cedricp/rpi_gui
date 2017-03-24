#ifndef WIDGET_H
#define WIDGET_H

#include "color.h"
#include "bbox.h"
#include "events.h"
#include "painter.h"
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
    bool   		m_visibility, m_transparent, m_is_root;
    WCallback	*m_callback;
    void		*m_callbackdata;
    std::string m_name;
    bool   		m_dirty;
    WImpl*  	m_impl;
    Painter* 	m_painter;
    std::vector<float*> m_model_matrix_stack;
    
    void add_child(Widget* w);
    void remove_child(Widget* w);
    void internal_draw(bool force = false);
    void get_all_children(std::vector<Widget*>&);
    bool internal_key_event(const char* code, bool press);
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
    bool		m_bg_gradient_enabled;
    Matrix		m_projection_matrix, m_model_matrix;
    void		do_callback(Widget* w, void* arg);
    void		do_callback();
    static void default_callback(Widget*, void*);

    virtual void parent_resize_event(const IBbox& bbox);
    virtual bool enter_event();
    virtual bool leave_event();
    virtual bool key_press_event(const char* code);
    virtual bool key_release_event(const char* code);
    virtual bool mouse_press_event(int button);
    virtual bool mouse_release_event(int button);
    virtual bool mouse_wheel_event(int button);
    virtual bool mouse_motion_event(int x, int y);
    virtual bool drag_event(int rel_x, int rel_y);
    virtual void timer_event(void* data);
    virtual bool custom_event(void* data);
    virtual bool accept_drag(int x, int y);
    virtual void widget_added_event(Widget* widget);
    virtual void init_viewport(int x, int y, int width, int height);

    virtual void draw();
    virtual void post_draw();
public:
    Widget(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
    virtual ~Widget();

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

    void backgroung_gradient_enable(bool e){
    	m_bg_gradient_enabled = e;
    }

    int w();
    int h();
    int x();
    int y();

    void x(int x);
    void y(int y);

    IBbox 	screen_bbox();
    IBbox 	screen_bbox_corrected();
    IBbox 	relative_bbox();
    void	drawing_area(IBbox& area);
    void 	screen_to_widget_coordinates(int sx, int sy, int &wx, int &wy);

    virtual void resize(int x, int y, int w, int h);
    virtual void resize(int w, int h);

    void push_model_matrix();
    void pop_model_matrix();
    void translate(float x, float y, float z = 0.);
    void rotate(float x, float y, float z, float angle);
    
    void mouse_pos(int&x, int&y);

    void hide();
    void show();
    void close();
    void destroy_children();
    
    int  fixed_width(){return m_fixed_width;}
    int  fixed_height(){return m_fixed_height;}

    void  fixed_width(int w){m_fixed_width = w;}
    void  fixed_height(int h){m_fixed_height = h;}

    void horizontal_margin(int h){m_horizontal_margin = h;}
    void vertical_margin(int h){m_vertical_margin = h;}

    int horizontal_margin(){return m_horizontal_margin;}
    int vertical_margin(){return m_vertical_margin;}

    bool hidden(){return !m_visibility;}
    void parent(Widget* w);
    void update(bool full_redraw = false);
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

    void use_font(std::string font_name);
    void use_default_font();
    void load_font(std::string font_id, int size, int atlas_size = 1024);
    void set_top_widget(Widget* w);

    void		 callback(WCallback* cb);
    void		 callback(WCallback* cb, void* user_data);

};

#endif
