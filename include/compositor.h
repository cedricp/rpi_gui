#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <vector>
#include "widget.h"

struct Impl;

enum custom_events{
	TIMER_EVENT,
	CALLBACK_EVENT,
	USER_EVENT
};

enum Compositor_cursors {
	CURSOR_ARROW,
	CURSOR_CROSS
};

struct SDL_KeyboardEvent;

class Compositor {
    std::vector<Widget*> m_widgets;
    Widget* m_focus_drag_widget;
    int		m_drag_x, m_drag_y;
    bool	m_drag_started;
    Impl 	*m_impl;
    int 	m_curr_mousex, m_curr_mousey;
    bool handle_mouse_button_event(int, bool);
    bool handle_key_event(SDL_KeyboardEvent* key_ev, bool push);
    bool handle_mouse_wheel_event(int);
    bool handle_mouse_move_event(int x, int y);
    void resize_widget_to_window(Widget*);
    void* create_shm(int key, size_t size);
    void* get_shm(int key, size_t size);
    void create_cursors();

public:
    Compositor();
    ~Compositor();
    
    Painter& painter();
    int		add_timer_event(int ms, void* data);
    void	send_callback_event(void *from, void* data);
    void	remove_timer(int timer_id);
    static 	Compositor* get_singleton();
    void 	remove_widget(Widget* w);
    void 	add_widget(Widget* w);
    bool 	widget_exists(Widget* w);
    void 	set_root_widget(Widget* w);
    void	set_top_widget(Widget* w);
    Widget *get_root_widget();
    Widget *get_top_widget();
    Widget *create_new_window();
    int    	screen_height();
    int    	screen_width();
    int 	run();
    void	mouse_position(int &x, int &y);
    void 	finish();
    void	cursor(Compositor_cursors c);
    void	advert_widget_deleted(Widget* w);
    void	set_widget_as_window(Widget* wid);
};

#define COMPOSITOR Compositor::get_singleton()

#endif
