#include "file_chooser.h"
#include "label.h"
#include "layout.h"
#include "scroll.h"
#include "compositor.h"
#include "button.h"
#include <dirent.h>
#include <stdio.h>
#include <algorithm>

void
get_content(std::string path, std::vector<std::string>& dirs, std::vector<std::string>& files)
{
	DIR * d = opendir(path.c_str());
	if(d==NULL)
		return;
	struct dirent * dir;
	while ((dir = readdir(d)) != NULL)
	{
		if(dir-> d_type != DT_DIR)
			files.push_back(dir->d_name);
		else if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 )
			dirs.push_back(dir->d_name);
	}

	std::sort(dirs.begin(), dirs.end());
	std::sort(files.begin(), files.end());

	closedir(d);
}

File_chooser::File_chooser(int x, int y, int width, int height, const char* name, Widget* parent) : Widget(x, y, width, height, name, parent)
{
	m_path = "/";

	m_main_layout = new Layout(0, 0, 0, 0, "", this, LAYOUT_VERTICAL);

	m_main_layout->vertical_margin(2);
	m_main_layout->horizontal_margin(2);

	m_header_layout = new Layout(0, 0, 0, 0, "",  m_main_layout, LAYOUT_HORIZONTAL);
	Button* up = new Button(0, 0, 0, 0, "UP", m_header_layout);
	Button* close = new Button(0, 0, 0, 0, "CLOSE", m_header_layout);

	m_scroll_view = new Scroll(0, 0, 0, 0, "", m_main_layout);
	m_files_layout = new Layout(0, 0, 0, 0, "", m_scroll_view, LAYOUT_VERTICAL);

	m_header_layout->fixed_height(20);

	m_main_layout->compute_layout();
	m_header_layout->compute_layout();

	backgroung_gradient_enable(true);
	tiles_enabled(true);
	m_main_layout->transparent(true);
	m_scroll_view->transparent(true);
	m_files_layout->transparent(true);

	up->callback(static_dir_up_callback, this);

	set_path(m_path);
}

void
File_chooser::dir_up_callback(Label* l)
{
	m_path += "/..";
	set_path(m_path);
}

void
File_chooser::dir_callback(Label* l)
{
	m_path += "/";
	m_path += l->name();
	set_path(m_path);
}

void
File_chooser::file_callback(Label* l)
{
	char resolved_path[PATH_MAX];
	std::string path = m_path + "/";
	path += l->name();
	realpath(path.c_str(), resolved_path);
	m_file = resolved_path;
	do_callback();
}

void
File_chooser::set_path(std::string path)
{
	m_path = path;
	char resolved_path[PATH_MAX];
	realpath(m_path.c_str(), resolved_path);
	m_path = resolved_path;
	std::vector<std::string> dirs, files;
	get_content(path, dirs, files);

	m_files_layout->destroy_children();

	int h = 20 * (dirs.size() + files.size());
	IBbox lay_area;
	m_main_layout->drawing_area(lay_area);
	m_files_layout->resize(lay_area.width(), h);

	for (int i = 0; i < dirs.size(); ++i){
		Label* label_dir = new Label(0, 0, m_files_layout->w(), 20, dirs[i].c_str(), m_files_layout);
		label_dir->alignment(ALIGN_LEFT, ALIGN_CENTERV);
		label_dir->fixed_height(20);
		label_dir->fg_color(FColor(0.2, 0, 1, 1));
		attachCallback(label_dir, static_dir_callback);
		label_dir->transparent(true);
	}
	for (int i = 0; i < files.size(); ++i){
		Label* label_file = new Label(0, 0, m_files_layout->w(), 20, files[i].c_str(), m_files_layout);
		label_file->alignment(ALIGN_LEFT, ALIGN_CENTERV);
		label_file->fixed_height(20);
		attachCallback(label_file, static_file_callback);
		label_file->transparent(true);
	}

	m_files_layout->compute_layout();
	// Reset scrollview
	m_scroll_view->reset();
	// We must redraw the view...
	dirty(true);
}

void
File_chooser::resize(int x, int y, int w, int h)
{
	Widget::resize(x,y,w,h);
	m_main_layout->resize(w,h);
	m_header_layout->resize(w,h);
	set_path(m_path);
	m_main_layout->compute_layout();
	m_header_layout->compute_layout();
	dirty(true);
}

void
File_chooser::draw()
{
	painter().disable_texture();
	painter().enable_alpha(false);
	painter().color(FColor(.1,.1,.3,1.));
	painter().draw_quad(0,0, w(), h(), false, false, 2.5);
}
