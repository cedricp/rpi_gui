#include "compositor.h"
#include "widget.h"

#include <iostream>
#include <algorithm>
#include <sys/shm.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>

struct Impl {
	Impl(){
		window  = NULL;
		widget_under_mouse = NULL;
		glcontext = NULL;
	}

    SDL_Window 		*window;
    SDL_GLContext 	glcontext;
    Widget			*widget_under_mouse;
};

static Compositor *GLOBAL_COMPOSITOR = NULL;
static Painter* g_painter;

Uint32 timerCallback(Uint32 interval, void *param)
{
  SDL_Event event;

  event.type = SDL_USEREVENT;
  event.user.code = TIMER_EVENT;
  event.user.data1 = (void *)param;
  event.user.data2 = (void *)0;

  SDL_PushEvent(&event);

  return interval;
}

Compositor::Compositor()
{
    m_impl = new Impl;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //double buffering on obviously
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_impl->window = SDL_CreateWindow(
        "GUI",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL
    ); // SDL_WINDOW_FULLSCREEN

    if (m_impl->window == NULL) {
        std::cerr << "Cannot create OpenGL(ES) window, aborting : " << SDL_GetError() << std::endl;
        exit(1);
    }

    m_impl->glcontext = SDL_GL_CreateContext(m_impl->window);
    m_focus_widget = NULL;

    g_painter = new Painter;
}

Compositor::~Compositor()
{
    delete m_impl;
}

Painter&
Compositor::painter()
{
	return *g_painter;
}

void*
Compositor::create_shm(int key, size_t size)
{
	int shmid;
	void* shm = NULL;
	if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
		std::cerr << "Compositor : create_shm : Error, cannot create shared memory" << std::endl;
		return NULL;
	}

	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		std::cerr << "Compositor : create_shm : Error, cannot attach shared memory" << std::endl;
		return NULL;
	}

	return shm;
}

void*
Compositor::get_shm(int key, size_t size)
{
	int shmid;
	void* shm = NULL;
	if ((shmid = shmget(key, size, 0666)) < 0) {
		std::cerr << "Compositor : get_shm : Error, cannot create shared memory" << std::endl;
		return NULL;
	}

	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		std::cerr << "Compositor : get_shm : Error, cannot attach shared memory" << std::endl;
		return NULL;
	}

	return shm;
}

Compositor* Compositor::get_singleton()
{
    if (GLOBAL_COMPOSITOR == NULL){
        GLOBAL_COMPOSITOR = new Compositor;
    }
    return GLOBAL_COMPOSITOR;
}

void 
Compositor::add_widget(Widget* w)
{
    m_widgets.push_back(w);
}

Widget*
Compositor::get_root_widget()
{
    if (m_widgets.size() > 0){
        return m_widgets[0];
    }
    return NULL;
}

Widget*
Compositor::get_top_widget()
{
    if (m_widgets.size() > 0){
        return m_widgets.back();
    }
    return NULL;
}

bool
Compositor::widget_exists(Widget* w)
{
    std::vector<Widget*>::iterator it = std::find(m_widgets.begin(), m_widgets.end(), w);
    if (it == m_widgets.end())
        return false;
        
    return true;
}

void 
Compositor::remove_widget(Widget* w)
{
    std::vector<Widget*>::iterator it = std::find(m_widgets.begin(), m_widgets.end(), w);
    if ( it != m_widgets.end() ){
        m_widgets.erase(it);
    } else {
        std::cerr << "Warning, trying to remove an unregistered widget" << std::endl;
    }
}

bool
Compositor::handle_mouse_button_event(int button, bool push)
{
    int reassigned = 0;
    
    switch (button){
    case SDL_BUTTON_LEFT:
        reassigned = EVENT_MOUSE_BUTTON_LEFT;
        break;
    case SDL_BUTTON_MIDDLE:
        reassigned = EVENT_MOUSE_BUTTON_MIDDLE;
        break;
    case SDL_BUTTON_RIGHT:
        reassigned = EVENT_MOUSE_BUTTON_RIGHT;
        break;
    default:
        break;
    }

    std::vector<Widget*>::reverse_iterator it = m_widgets.rbegin();

    int x, y;
    SDL_GetMouseState(&x, &y);

    Widget *which;
    which = NULL;

    if (m_focus_widget){
    	bool taken = false;
    	if (m_focus_widget->screen_bbox().contains(x, y)){
    		taken = m_focus_widget->internal_mouse_button_event(x, y, reassigned, &which, push);
    	}
    	if (reassigned==EVENT_MOUSE_BUTTON_LEFT && !push){
    		if (!m_focus_widget->screen_bbox().contains(x, y))
    			m_focus_widget->leave_event();
    		m_focus_widget = NULL;
    	}
    	return true;
    }

    for (; it != m_widgets.rend(); ++it){
        if ((*it)->screen_bbox().contains(x, y)){
            bool taken = (*it)->internal_mouse_button_event(x, y, reassigned, &which, push);
            if (taken && which && push && reassigned == EVENT_MOUSE_BUTTON_LEFT){
            	m_focus_widget = which;
            	m_drag_x = x;
            	m_drag_y = y;
            }
            if (taken){
                return true;
            }
        }
    }
    return false;
}

int
Compositor::add_timer_event(int ms, void* data)
{
	SDL_TimerID timer = 0;
	return SDL_AddTimer(ms, timerCallback, data);
}

void
Compositor::remove_timer(int timer_id)
{
	SDL_RemoveTimer(timer_id);
}

bool
Compositor::handle_mouse_wheel_event(int wheel_ev)
{
    std::vector<Widget*>::reverse_iterator it = m_widgets.rbegin();

    int x, y;
    SDL_GetMouseState(&x, &y);

    Widget *which;
    which = NULL;

    for (; it != m_widgets.rend(); ++it){
        if ((*it)->screen_bbox().contains(x, y)){
            bool taken = (*it)->internal_mouse_wheel_event(x, y, wheel_ev, &which);
            if (taken)
                return true;
        }
    }

    return false;
}

bool
Compositor::handle_mouse_move_event(int x, int y)
{
    Widget *which;
    which = NULL;
    bool taken = false;

    if (m_focus_widget){
    	return m_focus_widget->drag_event(m_drag_x - x, m_drag_y - y);
    }

    std::vector<Widget*>::reverse_iterator it = m_widgets.rbegin();
    for (; it != m_widgets.rend(); ++it){
    	if ((*it)->screen_bbox().contains(x, y)){
            taken = (*it)->internal_mouse_motion_event(x, y, &which);
            if (taken)
                break;
        }
    }

    if (m_impl->widget_under_mouse){
    	if (m_impl->widget_under_mouse != which){
    		taken |= m_impl->widget_under_mouse->leave_event();
    		if (which)
    			taken |= which->enter_event();
    		m_impl->widget_under_mouse = which;
    	}
    } else {
    	m_impl->widget_under_mouse = which;
    }

    return taken;
}

Widget*
Compositor::create_new_window()
{
    Widget* new_widget = new Widget(0, 0, 100, 100, "Window", NULL);
    return new_widget;
}

void
Compositor::resize_widget_to_window(Widget* w)
{
    int x, y;
    SDL_GetWindowSize(m_impl->window, &x, &y);
    w->resize(x, y);
}

int
Compositor::screen_height()
{
    int x, y;
    SDL_GetWindowSize(m_impl->window, &x, &y);
    return y;
}

int
Compositor::screen_width()
{
    int x, y;
    SDL_GetWindowSize(m_impl->window, &x, &y);
    return x;
}

void
Compositor::send_callback_event(void *from, void* data)
{
  SDL_Event event;

  event.type = SDL_USEREVENT;
  event.user.code = CALLBACK_EVENT;
  event.user.data1 = (void *)from;
  event.user.data2 = (void *)data;

  SDL_PushEvent(&event);
}

int
Compositor::run()
{
    SDL_Event event;

    std::vector<Widget*>::iterator it = m_widgets.begin();
    for(;it != m_widgets.end();++it)
        resize_widget_to_window(*it);

    it = m_widgets.begin();
    for(;it != m_widgets.end();++it)
        (*it)->update(true);

	SDL_GL_SwapWindow(m_impl->window);
    bool quit = false;
    bool dragging[4] = {0, 0, 0, 0};
    
    while(!quit) {
    	bool full_update = false;
    	bool need_update = false;
        SDL_WaitEvent(&event);
        int windowID = SDL_GetWindowID(m_impl->window);
        
        switch(event.type){
        case SDL_USEREVENT:
        	switch (event.user.code){
        	case TIMER_EVENT:
        		((Widget*)event.user.data1)->timer_event(event.user.data2);
        		break;
        	default:
        		break;
        	}
        	break;
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_MOUSEBUTTONUP:
        	need_update = handle_mouse_button_event(event.button.button, false);
            break;
        case SDL_MOUSEBUTTONDOWN:
        	need_update = handle_mouse_button_event(event.button.button, true);
            break;
        case SDL_MOUSEWHEEL:
        	need_update = handle_mouse_wheel_event(event.wheel.y);
            break;
        case SDL_MOUSEMOTION:
        	need_update = handle_mouse_move_event(event.motion.x, event.motion.y);
            break;
        case SDL_WINDOWEVENT:
            if (event.window.windowID == windowID)  {
                switch(event.window.event) {
                case SDL_WINDOWEVENT_EXPOSED:
                case SDL_WINDOWEVENT_RESTORED:
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_ENTER:
                	full_update = true;
                default:
                    break;
                }   
           }     
        }

        if (need_update || full_update){
            if (full_update)
            	glDrawBuffer(GL_BACK);
            else
            	glDrawBuffer(GL_FRONT);

        	it = m_widgets.begin();
			for(;it != m_widgets.end();++it)
				(*it)->update(full_update);

			if (full_update)
				SDL_GL_SwapWindow(m_impl->window);
			else
				glFlush();
        }
    }

    for(it = m_widgets.begin(); it != m_widgets.end();++it)
    	(*it)->destroy_children();

    SDL_GL_DeleteContext(m_impl->glcontext);  
    SDL_DestroyWindow(m_impl->window);
    SDL_Quit();
    return 0;
}
