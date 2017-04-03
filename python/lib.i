%feature("autodoc","1") ;

%module (directors="1") ui

// macro to delegate the ownership of a class to C++
%define CHANGE_OWNERSHIP(name)
%pythonappend name##::##name %{
if len(args) == 6:          
    # retain reference to label
    self.my_label = args[5]

self.this.disown()
%}
%enddef

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

%pythoncode %{
def get_compositor():
	return Compositor.get_singleton()
%}