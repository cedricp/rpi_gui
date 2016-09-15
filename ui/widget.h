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

typedef void (WCallback )(Widget*, void*);
typedef WCallback* WCallback_p;

#define attachCallback( widget, method ) \
   widget->callback( reinterpret_cast< void(*)(Widget*,void*) >( method ), this )

class Widget{
    friend class Compositor;
    IBbox   	m_bbox;
    bool   		m_visibility, m_transparent;
    WCallback	*m_callback;
    void		*m_callbackdata;
    std::vector<Widget*> m_children_widgets;
    std::string m_name;
    bool   m_dirty;
    WImpl*  m_impl;
    Painter* m_painter;
    
    void add_child(Widget* w);
    void remove_child(Widget* w);
    void internal_draw(bool force = false);
    void get_all_children(std::vector<Widget*>&);
    bool internal_mouse_button_event(int x, int y, int button, Widget **w, bool press);
    bool internal_mouse_motion_event(int x, int y, Widget **w);
    bool internal_mouse_wheel_event(int x, int y, int we, Widget **w);
    Widget* child_widget_in(int x, int y);
    static void default_callback(Widget*, void*);
protected:

    Widget 		*m_parent;
    FColor 		m_bgcolor;
    FColor 		m_fgcolor;
    int 		m_font_id;
    void		do_callback(Widget* w, void* arg);
    void		do_callback();
public:
    Widget(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
    virtual ~Widget();

    Widget* parent(){
        return m_parent;
    }
    
    std::string name(){return m_name;}

    void set_fg_color(const FColor& color){
        m_fgcolor = color;
    }
    
    void set_bg_color(const FColor& color){
        m_bgcolor = color;
    }

    int w();
    int h();
    int x();
    int y();

    IBbox 	screen_bbox();
    IBbox 	relative_bbox();
    void 	screen_to_widget_coordinates(int sx, int sy, int &wx, int &wy);
    
    virtual void draw();
    virtual void resize(int x, int y, int w, int h);
    virtual void resize(int w, int h);
    
    void hide();
    void show();
    void close();
    void destroy_children();
    
    bool hidden(){return m_visibility;}
    void parent(Widget* w);
    void update(bool full_redraw = false);
    void damage(const IBbox& other);
    void transparent(bool on){m_transparent = on;}

    static Painter& painter();

    bool dirty() {
        return m_dirty;
    }
    
    void dirty(bool d){
        m_dirty = d;
    }
    
    void use_font(std::string font_name);
    void use_default_font();
    void load_font(std::string font_id, int size, int atlas_size = 1024);

    void		 callback(WCallback* cb);
    void		 callback(WCallback* cb, void* user_data);

    virtual void parent_resize_event(const IBbox& bbox);
    virtual bool enter_event();
    virtual bool leave_event();
    virtual bool mouse_press_event(int button);
    virtual bool mouse_release_event(int button);
    virtual bool mouse_wheel_event(int button);
    virtual bool mouse_motion_event(int x, int y);
    virtual bool drag_event(int rel_x, int rel_y);
    virtual void timer_event(void* data);

};

#endif
