#include "compositor.h"
#include "widget.h"
#include "cursors/cross_cursor.h"

#include <iostream>
#include <algorithm>
#include <sys/shm.h>
#include <map>

#include <SDL2/SDL.h>
#ifdef USE_OPENGL
#include <GL/gl.h>
#else
#include <GLES2/gl2.h>
#endif

struct Impl {
	Impl(){
		window  = NULL;
		widget_under_mouse = NULL;
		glcontext = NULL;
	}

    SDL_Window 		*window;
    SDL_GLContext 	glcontext;
    Widget			*widget_under_mouse;
    std::map<int, SDL_Cursor*> cursors;
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

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const unsigned int  rmask = 0xff000000;
    const unsigned int  gmask = 0x00ff0000;
    const unsigned int  bmask = 0x0000ff00;
    const unsigned int  amask = 0x000000ff;
#else
    const unsigned int  rmask = 0x000000ff;
    const unsigned int  gmask = 0x0000ff00;
    const unsigned int  bmask = 0x00ff0000;
    const unsigned int  amask = 0xff000000;
#endif

Compositor::Compositor()
{
    m_impl = new Impl;
    m_curr_mousex = m_curr_mousey = m_drag_x = m_drag_y = 0;
    int sdl_status =  SDL_Init(SDL_INIT_VIDEO);
    if (sdl_status != 0){
    	fprintf(stderr, "\nUnable to initialize SDL: %i %s\n", sdl_status, SDL_GetError() );
        exit(1);
    }


#ifdef USE_OPENGL
	Uint32 flags = SDL_WINDOW_OPENGL;
#else
	Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //double buffering on obviously

	//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 5);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


    m_impl->window = SDL_CreateWindow(
        "GUI",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        800,                               // width, in pixels
        600,                               // height, in pixels
		flags
    );

    if (m_impl->window == NULL) {
        std::cerr << "Cannot create OpenGL(ES) window, aborting : " << SDL_GetError() << std::endl;
        finish();
        exit(-1);
    }

    m_impl->glcontext = SDL_GL_CreateContext(m_impl->window);
    if (m_impl->glcontext == NULL){
        finish();
        exit(-1);
    }

    SDL_GL_SetSwapInterval(1);

    m_focus_drag_widget = NULL;
    m_drag_started = false;
    g_painter = new Painter;
}

Compositor::~Compositor()
{
    finish();
    delete m_impl;
}

void
Compositor::create_cursors()
{
	SDL_Surface* cursor_cross_surface = SDL_CreateRGBSurfaceFrom(gimp_image_cross.pixel_data, gimp_image_cross.width,
										gimp_image_cross.height, gimp_image_cross.bytes_per_pixel * 8,
										gimp_image_cross.bytes_per_pixel * gimp_image_cross.width, rmask, gmask, bmask, amask);
	SDL_Cursor* cursor_cross = SDL_CreateColorCursor(cursor_cross_surface, 0, 0);

	m_impl->cursors[CURSOR_CROSS] = cursor_cross;
	m_impl->cursors[CURSOR_ARROW] = NULL;
}

void
Compositor::cursor(Compositor_cursors cursor){
	SDL_SetCursor(m_impl->cursors[cursor]);
}

void
Compositor::finish()
{
    if(m_impl->glcontext)
        SDL_GL_DeleteContext(m_impl->glcontext);  
    if(m_impl->window)
        SDL_DestroyWindow(m_impl->window);
    SDL_Quit();
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

    // A drag operation is in progress, let's manage it
    if (m_focus_drag_widget){
    	bool taken = false;
    	bool still_in_focus_widget = m_focus_drag_widget->screen_bbox().contains(x, y);

    	if (still_in_focus_widget && !m_drag_started){
    		taken = m_focus_drag_widget->internal_mouse_button_event(x, y, reassigned, &which, push);
    	}

    	if (reassigned==EVENT_MOUSE_BUTTON_LEFT && !push){
    		if (!still_in_focus_widget)
    			m_focus_drag_widget->leave_event();
    		else
    			m_focus_drag_widget->mouse_release_event(EVENT_MOUSE_BUTTON_LEFT);
    		m_focus_drag_widget = NULL;
    	}

    	return true;
    }

    // Normal operations, send events to widgets
    for (; it != m_widgets.rend(); ++it){
        if ((*it)->screen_bbox().contains(x, y)){
            bool taken = (*it)->internal_mouse_button_event(x, y, reassigned, &which, push);
            if (taken && which && push && reassigned == EVENT_MOUSE_BUTTON_LEFT){
            	m_drag_started = false;
            	// We want to pass drag event to widget or parent widget accepting it
            	Widget* w = which;
            	while(w){
            		IBbox scrb = w->screen_bbox();
            		int rel_x = x - scrb.xmin();
            		int rel_y = y - scrb.ymin();
            		if (w->accept_drag(rel_x, rel_y))
            			break;
            		w = w->parent();
            	}
            	m_focus_drag_widget = w;
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
	m_curr_mousex = x;
	m_curr_mousey = y;

    Widget *which;
    which = NULL;
    bool taken = false;

    if (m_focus_drag_widget){
    	m_drag_started = true;
    	return m_focus_drag_widget->drag_event(m_drag_x - x, m_drag_y - y);
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
    int w, h;
    SDL_GetWindowSize(m_impl->window, &w, &h);
    Widget* new_widget = new Widget(0, 0, w, h, "Window", NULL);
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

    while(!quit) {
    	bool full_update = false;
    	bool need_update = false;

        SDL_WaitEvent(&event);
        int windowID = SDL_GetWindowID(m_impl->window);

        do{
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
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
					quit = true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				need_update |= handle_mouse_button_event(event.button.button, false);
				break;
			case SDL_MOUSEBUTTONDOWN:
				need_update |= handle_mouse_button_event(event.button.button, true);
				break;
			case SDL_MOUSEWHEEL:
				need_update |= handle_mouse_wheel_event(event.wheel.y);
				break;
			case SDL_MOUSEMOTION:
				need_update |= handle_mouse_move_event(event.motion.x, event.motion.y);
				break;
			case SDL_WINDOWEVENT:
				if (event.window.windowID == windowID)  {
					switch(event.window.event) {
					case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_ENTER:
						full_update |= true;
					default:
						break;
					}
			   }
			}
        } while (SDL_PollEvent(&event) != 0); // Pump events before drawing

        if (need_update || full_update){
#ifdef USE_OPENGL
        	full_update = true;
#else
        	// Back buffer should be clean (modified SDL with GL_PRESERVE_BUFFER)
#endif

        	it = m_widgets.begin();
			for(;it != m_widgets.end();++it)
				(*it)->update(full_update);

			SDL_GL_SwapWindow(m_impl->window);
        }
    }

    for(it = m_widgets.begin(); it != m_widgets.end();++it)
    	(*it)->destroy_children();

    finish();
    return 0;
}

void
Compositor::mouse_position(int &x, int &y)
{
	x = m_curr_mousex;
	y = m_curr_mousey;
}
