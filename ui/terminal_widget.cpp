#include "terminal_widget.h"
#include <SDL2/SDL.h>
extern "C"{
#include "hl_vt100.h"
#include "lw_terminal_parser.h"
#include "lw_terminal_vt100.h"
}
struct Terminal_widget::impl{
	SDL_Thread *thread;
	struct vt100_headless* vt100_headless;
	UIEvent* thread_event;
};

void disp(struct vt100_headless *vt100)
{
	Terminal_widget::impl *inst = (Terminal_widget::impl*)vt100->custom_data;
    Widget::push_event(inst->thread_event);
}

int
terminal_thread(void* data)
{
	Terminal_widget::impl *inst = (Terminal_widget::impl*)data;
	inst->vt100_headless->changed = disp;
	inst->vt100_headless->custom_data = inst;
	char* args[] = {"",NULL};
    vt100_headless_fork(inst->vt100_headless, "/bin/bash", args);
    vt100_headless_main_loop(inst->vt100_headless);
    return 1;
}

Terminal_widget::Terminal_widget(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_impl = new Terminal_widget::impl;
	m_impl->thread_event = create_event();
	m_impl->vt100_headless = new_vt100_headless();
	m_impl->thread = SDL_CreateThread(terminal_thread, "terminal_thread", (void*)m_impl);
	m_text = new Label(0,0, w(), h(), "", this);
	m_text->alignment(ALIGN_LEFT, ALIGN_TOP);
	m_text->label("Init...");
	std::string path;
	if(painter().locate_resource("fonts/white_rabbit.ttf", path))
		m_text->load_font(path, 12, 1024);
}

Terminal_widget::~Terminal_widget()
{
	vt100_headless_stop(m_impl->vt100_headless);
	int retval;
	SDL_WaitThread(m_impl->thread, &retval);
	delete_vt100_headless(m_impl->vt100_headless);
}

bool
Terminal_widget::custom_event(void* data)
{
	unsigned int y;
	const char **lines;

	lines = vt100_headless_getlines(m_impl->vt100_headless);

	std::string txt;
	for (y = 0; y < m_impl->vt100_headless->term->height; ++y)
	{
		std::string l = lines[y];
		txt += l.substr(0, 80);
		txt += "\n";
	}
	m_text->label(txt);
	return true;
}

bool
Terminal_widget::key_press_event(const char* key)
{
	char buf[32];buf[1] = 0;
	std::cout << key << std::endl;
	if (strcmp(key, "Return") == 0){
		write(m_impl->vt100_headless->master, "\n", 1);
	} else if(strcmp(key, "Backspace") == 0){
		write(m_impl->vt100_headless->master, "\b", 1);
	} else if(strcmp(key, "Space") == 0){
		write(m_impl->vt100_headless->master, " ", 1);
	} else if(strcmp(key, "Left") == 0){
		sprintf(buf, "\033[D");
		write(m_impl->vt100_headless->master, " ", 1);
	} else {
		buf[0] = key[0]+32;
		write(m_impl->vt100_headless->master, buf, 2);
	}
	return true;
}

void
Terminal_widget::draw()
{

}
