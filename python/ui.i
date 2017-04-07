%feature("autodoc","1") ;

%module (directors="1") ui

// macro to delegate the ownership of a class to C++
%define CHANGE_OWNERSHIP(name)
%pythonappend name##::##name %{
self.this.disown()
%}
%enddef

%include stl.i

%include "../include/color.h"
%template(FColor) Color<float>;

%include "../include/bbox.h"
%template(IBbox) Bbox<int>;
%template(FBbox) Bbox<float>;

%include "compositor.swig"
%include "widget.swig"
%include "button.swig"
%include "painter.swig"
%include "layout.swig"
%include "scroll.swig"
%include "slider.swig"
%include "tab_widget.swig"
%include "multi_panel.swig"
%include "label.swig"
%include "file_chooser.swig"
%include "list_widget.swig"

%pythoncode %{
def get_compositor():
	return Compositor.get_singleton()
%}