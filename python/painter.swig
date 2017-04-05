%feature("docstring") ::Painter
"""
Painter module
""" ;

%{
#include <../include/painter.h>

bool get_matrix(PyObject* matob, Matrix m)
{
	Matrix mat;
	if (!PyList_CheckExact(matob)){
		std::cerr << "Painter.load_projection_matrix take list as argument" << std::endl;
		return false;
	}
	if (PyList_Size(matob) != 16){
		std::cerr << "Painter.load_projection_matrix take list with 16 elements" << std::endl;
		return false;
	}
	for (int i = 0; i < 16; ++i){
		PyObject* elem = PyList_GetItem(matob, i);
		if (!PyFloat_Check(elem)){
			std::cerr << "Painter.load_projection_matrix take list of float as argument" << std::endl;
			return false; 
		}
		m[i] = PyFloat_AsDouble(elem);
	}
	return true;
}

PyObject* create_matrix(Matrix m){
	PyObject* list = PyList_New(16);
	for (int i=0; i < 16; ++i)
		PyList_SetItem(list, i, PyFloat_FromDouble(m[i]));
	return list;
}

%}

%ignore Vec2f;
%ignore Painter::create_perspective_matrix(float aspect_ratio, float fovy, float znear, float zfar, Matrix outmatrix);
%ignore Painter::create_ortho_matrix(float left, float right, float bottom, float top, float near_val, float far_val, Matrix outmatrix);
%ignore Painter::load_identity(Matrix matrix);
%ignore Painter::load_projection_matrix(Matrix matrix);
%ignore Painter::load_model_matrix(Matrix matrix);
%ignore Painter::build_text(int font_id, std::string text, int start_x,int start_y, Text_data& data);
%ignore Painter::load_svg_image(std::string filename, int &w, int &h);
%ignore Painter::locate_resource(std::string name, std::string &path);
%ignore Painter::texture_size(std::string name, int& w, int& h);

%include typemaps.i

%include "../include/painter.h"

%extend Painter {
	PyObject* locate_resource(const char* filename){
		std::string path;
		self->locate_resource(filename, path);
		return PyString_FromString(path.c_str());
	}

	PyObject*
	load_svg_image(const char* filename){
		int x, y;
		self->load_svg_image(filename, x, y);
		PyObject* ret = PyTuple_New(2);
		PyObject* pyx = PyInt_FromLong(x);
		PyObject* pyy = PyInt_FromLong(y);
		PyTuple_SetItem(ret, 0, pyx);
		PyTuple_SetItem(ret, 1, pyy);
		return ret;
	}

	PyObject* 
	build_text(int font_id, const char* text, int start_x,int start_y){
		Text_data* tdata = new Text_data;
		self->build_text(font_id, std::string(text), start_x, start_y, *tdata);
		return SWIG_NewPointerObj((void*)tdata, SWIGTYPE_p_Text_data, 1);
	}
	
	PyObject*
	create_ortho_matrix(float left, float right, float bottom, float top, float near_val, float far_val){
		Matrix mat;
		self->create_ortho_matrix(left, right, bottom, top, near_val, far_val, mat);
		PyObject* list = create_matrix(mat);
		return list;
	}
	
	PyObject*
	create_perspective_matrix(float aspect_ratio, float fovy, float znear, float zfar){
		Matrix mat;
		self->create_perspective_matrix(aspect_ratio, fovy, znear, zfar, mat);
		PyObject* list = create_matrix(mat);
		return list;
	}
	
	void
	load_projection_matrix(PyObject* matob){
		Matrix mat;
		if (get_matrix(matob, mat))
			self->load_projection_matrix(mat);
	}
	
	void
	load_model_matrix(PyObject* matob){
		Matrix mat;
		if (get_matrix(matob, mat))
			self->load_model_matrix(mat);
	}
	
	PyObject*
	create_identity(){
		Matrix mat;
		self->load_identity(mat);
		PyObject* list = PyList_New(16);
		for (int i=0; i < 16; ++i)
			PyList_SetItem(list, i, PyFloat_FromDouble(mat[i]));
		return list;
	}
}