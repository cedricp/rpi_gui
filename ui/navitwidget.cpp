/***
 *     ____    ____   _____ ____   ____     ___  ____   ____   __ __       ____  __ __  ____
 *    |    \  /    | / ___/|    \ |    \   /  _]|    \ |    \ |  |  |     /    ||  |  ||    |
 *    |  D  )|  o  |(   \_ |  o  )|  o  ) /  [_ |  D  )|  D  )|  |  |    |   __||  |  | |  |
 *    |    / |     | \__  ||   _/ |     ||    _]|    / |    / |  ~  |    |  |  ||  |  | |  |
 *    |    \ |  _  | /  \ ||  |   |  O  ||   [_ |    \ |    \ |___, |    |  |_ ||  :  | |  |
 *    |  .  \|  |  | \    ||  |   |     ||     ||  .  \|  .  \|     |    |     ||     | |  |
 *    |__|\_||__|__|  \___||__|   |_____||_____||__|\_||__|\_||____/     |___,_| \__,_||____|
 *
 * (C) 2017 Cedric PAILLE (cedricpaille(at)gmail.com)
 */

#include "compositor.h"

#include <SDL2/SDL.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <navitwidget.h>
#include <signal.h>

static char * nicfifo = "/tmp/navit.command.fifo";
static char * niififo = "/tmp/navit.image.fifo";

struct impl{
	impl(Widget* wg){
		kill_thread = 0;
		data = NULL;
		data_size = 0;
		texid = (unsigned int)-1;
	    SDL_memset(&event, 0, sizeof(SDL_Event));
	    in_use = false;
	    reset_image_thread = false;
	    size_match = false;
	    h = w = 0;
	    sdl_thread = NULL;
		Uint32 evt = SDL_RegisterEvents(1);
		if (evt != ((Uint32)-1)) {
		    event.type = evt;
		    event.user.code = USER_EVENT;
		    event.user.data1 = wg;
		    event.user.data2 = 0;
		}
	}
	SDL_Thread* sdl_thread;
	SDL_Event 	event;
	void		*data;
	size_t		data_size;
	bool		kill_thread;
	int w, h;
	unsigned int texid;
	bool in_use;
	bool size_match;
	bool reset_image_thread;
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
	std::string cmd = command;
	cmd += "\n";
	int command_fd = open(nicfifo, O_WRONLY | O_NONBLOCK);
	if (command_fd >= 0){
		int ret = write(command_fd, cmd.c_str(), cmd.size());
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
	struct stat st;

	while(!inst->kill_thread){
		if (fd_image < 0){
			usleep(500);
			fd_image = open(niififo, O_RDONLY | O_NONBLOCK);
		}

		int ret = read(fd_image, &h, sizeof(img_header));
		if (ret < 0 || errno == EPIPE){
			fd_image = -1;
			continue;
		}

		if (ret == 0){
			goto no_data;
		}

		if (h.magic != 0xDEADBEEF){
			std::cout << "bad header..." << std::endl;
			char dummy;
			int cnt = 1;
			// Flush remaining data
			while(cnt > 0){
				cnt = read(fd_image, &dummy, 1);
				if (cnt == -1)
					break;
			}
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

		if (ret > 0 && inst->event.type != (Uint32)-1)
			SDL_PushEvent(&inst->event);

no_data:
		usleep(500);
		close(fd_image);
		fd_image = -1;
	}

	close(fd_image);
	return 0;
}

Navit_widget::Navit_widget(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_impl = new impl(this);

	m_impl->sdl_thread = SDL_CreateThread(read_image_thread, "navit_image", (void*)m_impl);
	resize(x, y, width, height);
}

Navit_widget::~Navit_widget()
{
	int retval;
	m_impl->kill_thread = 1;
	SDL_WaitThread(m_impl->sdl_thread, &retval);
}

void
Navit_widget::resize(int x, int y, int w, int h)
{
	char buffer[40];
	sprintf(buffer, "resize=%ix%i", w, h);
	Widget::resize(x, y, w, h);
	send_cmd(buffer);
}

void
Navit_widget::zoom(bool in)
{
	char buffer[40];
	if (in)
		sprintf(buffer, "zoomin");
	else
		sprintf(buffer, "zoomout");

	send_cmd(buffer);
}


bool
Navit_widget::custom_event(void* data)
{
	m_impl->in_use = true;
	if (m_impl->w != w() || m_impl->h != h()){
		resize(x(), y(), w(), h());
		m_impl->in_use = false;
		return true;
	}
	m_impl->in_use = false;
	dirty(true);
	return true;
}

bool
Navit_widget::mouse_release_event(int button)
{
	char buffer[40];
	int x, y;
	mouse_coordinates(x, y);
	sprintf(buffer, "release=%i-%i", x, y);
	send_cmd(buffer);
	return true;
}

bool
Navit_widget::mouse_press_event(int button)
{
	char buffer[40];
	int x, y;
	mouse_coordinates(x, y);

	sprintf(buffer, "press=%i-%i", x, y);
	send_cmd(buffer);
	return true;
}

bool
Navit_widget::mouse_motion_event(int x, int y)
{
	char buffer[40];
	sprintf(buffer, "move=%i-%i", x, y);
	send_cmd(buffer);
	return true;
}

bool
Navit_widget::leave_event()
{
	return false;
}

void
Navit_widget::draw(){
	if (m_impl->texid != (unsigned int)-1)
		painter().delete_texture(m_impl->texid);

	m_impl->texid = painter().create_texture("navit_img", (char*)m_impl->data, m_impl->w, m_impl->h, TEXTURE_RGBA, TEXBORDER_CLAMP);
	painter().use_texture(m_impl->texid);
	painter().draw_quad(0,0, w(), h(), true);
	m_impl->in_use = false;
}

