#include "navit.h"
#include "compositor.h"

#include <SDL2/SDL.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

static char * nicfifo = "/tmp/navit.command.fifo";
static char * niififo = "/tmp/navit.image.fifo";

struct impl{
	impl(){
		kill_thread = 0;
		data = NULL;
		data_size = 0;
		texid = (unsigned int)-1;
	    SDL_memset(&event, 0, sizeof(SDL_Event));
	    in_use = false;
	}
	SDL_Thread* sdl_thread;
	SDL_Event 	event;
	void		*data;
	size_t		data_size;
	bool		kill_thread;
	int w, h;
	unsigned int texid;
	bool in_use;
};

struct img_header{
	int magic;
    int w;
    int h;
    int size;
};

static bool
send_cmd(char* command)
{
	int command_fd = open(nicfifo, O_WRONLY | O_NONBLOCK);
	if (command_fd >= 0){
		write(command_fd, command, strlen(command));
		::close(command_fd);
		return true;
	}
	return false;
}

static int
read_image_thread(void* ptr)
{
	impl* inst = (impl*)ptr;
	int fd_image = -1;
	img_header h;

	while(!inst->kill_thread){
		if (fd_image < 0){
			usleep(5000);
			fd_image = open(niififo, O_RDONLY | O_NONBLOCK);
			continue;
		}

		int ret = read(fd_image, &h, sizeof(img_header));
		if (ret < 0 || errno == EPIPE){
			fd_image = -1;
			continue;
		}

		if (h.magic != 0xDEADBEEF){
			std::cout << "bad header..." << std::endl;
			continue;
		}

		while(inst->in_use){
		}

		if (inst->data_size != h.size){
			if (inst->data)
				free(inst->data);
			inst->data = malloc(h.size);
		}

		inst->data_size = h.size;
		inst->w = h.w;
		inst->h = h.h;

		ret = read(fd_image, inst->data, h.size);
		if (ret < 0){
			fd_image = -1;
			continue;
		}

		if (ret > 0)
			SDL_PushEvent(&inst->event);
		else
			usleep(5000);
	}

	close(fd_image);
	return 0;
}

Navit::Navit(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_impl = new impl;

	Uint32 evt = SDL_RegisterEvents(1);
	if (evt != ((Uint32)-1)) {
	    m_impl->event.type = evt;
	    m_impl->event.user.code = USER_EVENT;
	    m_impl->event.user.data1 = this;
	    m_impl->event.user.data2 = 0;
	}

	m_impl->sdl_thread = SDL_CreateThread(read_image_thread, "navit_image", (void*)m_impl);
	resize(x, y, width, height);
}

Navit::~Navit()
{
	int retval;
	m_impl->kill_thread = 1;
	SDL_WaitThread(m_impl->sdl_thread, &retval);
}

void
Navit::resize(int x, int y, int w, int h)
{
	char buffer[40];
	sprintf(buffer, "resize=%ix%i", w, h);
	Widget::resize(x, y, w, h);
	send_cmd(buffer);
}

bool
Navit::custom_event(void* data)
{
	dirty(true);
	return true;
}

bool
Navit::accept_drag(int x, int y)
{
	return false;
}

bool
Navit::drag_event(int rel_x, int rel_y)
{
	return false;
}

bool
Navit::mouse_release_event(int button)
{
	char buffer[40];
	int x, y;
	mouse_pos(x, y);
	sprintf(buffer, "release=%i-%i", x, y);
	send_cmd(buffer);
	return true;
}

bool
Navit::mouse_press_event(int button)
{
	char buffer[40];
	int x, y;
	mouse_pos(x, y);

	sprintf(buffer, "press=%i-%i", x, y);
	send_cmd(buffer);
	return true;
}

bool
Navit::mouse_motion_event(int x, int y)
{
	char buffer[40];
	sprintf(buffer, "move=%i-%i", x, y);
	send_cmd(buffer);
	return true;
}

bool
Navit::leave_event()
{
	return false;
}

void
Navit::draw(){
	m_impl->in_use = true;
	if (m_impl->texid != (unsigned int)-1)
		painter().delete_texture(m_impl->texid);
	m_impl->texid = painter().create_texture("navit_img", (char*)m_impl->data, m_impl->w, m_impl->h, TEXTURE_RGBA, TEXBORDER_CLAMP);
	painter().use_texture(m_impl->texid);
	painter().draw_quad(0,0, w(), h(), true);
	m_impl->in_use = false;
}

