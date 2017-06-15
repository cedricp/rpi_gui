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
	m_text_fonts = -1;
	m_is_text_init = false;
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

	m_scroll_view->backgroung_gradient_enable(true);
	m_scroll_view->tiles_enabled(true);
	m_main_layout->transparent(true);
	m_files_layout->transparent(true);

	up->callback(static_dir_up_callback, this);

	set_path(m_path);
}

void
File_chooser::set_text_font(std::string fontname, int fontsize, int atlassize)
{
	m_text_fonts = painter().load_fonts(fontname, fontsize, atlassize);
	m_is_text_init = false;
}

void
File_chooser::dir_up_callback(Label* l)
{
	std::string new_path = m_path + "/..";
	set_path(new_path);
}

void
File_chooser::dir_callback(Label* l)
{
	std::string new_path = m_path + "/";
	new_path += l->name();
	set_path(new_path);
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
	if (path == m_path)
		return;
	m_path = path;
	m_is_text_init = false;
	dirty(true);
}

void
File_chooser::init_text()
{
	std::vector<std::string> dirs, files;
	get_content(m_path, dirs, files);

	m_files_layout->destroy_children();

	int h = 0;
	IBbox lay_area;
	m_main_layout->drawing_area(lay_area);
	int text_max = lay_area.width();

	for (int i = 0; i < dirs.size(); ++i){
		Label* label_dir = new Label(0, 0, 0, 0, dirs[i].c_str(), m_files_layout);
		if (m_text_fonts >= 0){
			label_dir->label(dirs[i].c_str());
			label_dir->use_fonts_id(m_text_fonts);
		}
		int text_height = label_dir->text_height() + 6;
		label_dir->alignment(ALIGN_LEFT, ALIGN_CENTERV);
		label_dir->fixed_height(text_height);
		label_dir->fg_color(FColor(0.2, 0, 1, 1));
		attachCallback(label_dir, static_dir_callback);
		label_dir->transparent(true);
		if (label_dir->text_width() > text_max)
			text_max = label_dir->text_width();
		h += text_height;
	}
	for (int i = 0; i < files.size(); ++i){
		Label* label_file = new Label(0, 0, 0, 0, files[i].c_str(), m_files_layout);
		if (m_text_fonts >= 0){
			label_file->label( files[i].c_str());
			label_file->use_fonts_id(m_text_fonts);
		}
		int text_height = label_file->text_height() + 6;
		label_file->alignment(ALIGN_LEFT, ALIGN_CENTERV);
		label_file->fixed_height(text_height);
		attachCallback(label_file, static_file_callback);
		label_file->transparent(true);
		if (label_file->text_width() > text_max)
			text_max = label_file->text_width();
		h += text_height;
	}

	((Widget*)m_files_layout)->resize(text_max, h);

	m_files_layout->compute_layout();
	// Reset scrollview
	m_scroll_view->reset();
	// We must redraw the view...

	m_is_text_init = true;
	dirty(true);
}

void
File_chooser::resize(int x, int y, int ww, int hh)
{
	Widget::resize(x,y,ww,hh);
	IBbox lay_area;
	dirty(true);
}

void
File_chooser::draw()
{
	if(!m_is_text_init)
		init_text();
	painter().disable_texture();
	painter().enable_alpha(false);
	painter().color(FColor(.1,.1,.3,1.));
	painter().draw_quad(0,0, w(), h(), false, false, 2.5);
}
