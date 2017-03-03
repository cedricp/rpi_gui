#ifndef FILECHOOSER_H
#define FILECHOOSER_H

#include "widget.h"

class Layout;
class Scroll;
class Label;

class File_chooser : public Widget
{
	std::string m_path, m_file;
	Layout* m_files_layout, *m_main_layout, *m_header_layout;
	Scroll* m_scroll_view;
	void dir_callback(Label* l);
	void file_callback(Label* l);
	void dir_up_callback(Label* l);
	static void static_dir_callback(Widget* w, void* data){
		((File_chooser*)data)->dir_callback((Label*)w);
	}
	static void static_file_callback(Widget* w, void* data){
		((File_chooser*)data)->file_callback((Label*)w);
	}
	static void static_dir_up_callback(Widget* w, void* data){
		((File_chooser*)data)->dir_up_callback((Label*)w);
	}
protected:
	void draw();
	void resize(int x, int y, int w, int h);
public:
	File_chooser(int x, int y, int width, int height, const char* name = "", Widget* parent = NULL);
	void set_path(std::string path);
	std::string get_current_path(){return m_path;}
	std::string get_current_file(){return m_file;}
};

#endif
