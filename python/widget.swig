%feature("docstring") ::Widget
"""
Widget class
""" ;

%feature("director") ::Widget;


CHANGE_OWNERSHIP(Widget)

%{
#include <../include/widget.h>
%}

%{ 
  #include "callback.h"

  static void PythonCallBack(Widget *widget, void *clientdata)
    {
      PyObject *func, *arglist;
      PyObject *result;
      PyObject *obj = 0;
      callbackstruct* cb = (callbackstruct*)clientdata;

      // This is the function .... 
      func = cb->func;

      if (cb->widget != 0) {
        // the parent widget
        obj = (PyObject *)( ((callbackstruct *)clientdata)->widget);
      }
      else if (cb->type != 0) {
        // this is the type of widget
        swig_type_info *descr = (swig_type_info *)cb->type;
        if (descr != 0) {
          obj = SWIG_NewPointerObj(widget, (swig_type_info *)descr, 0);
        }
      }
      if (obj == 0) {
        // generic fallback
        obj = SWIG_NewPointerObj(widget, SWIGTYPE_p_Widget, 0);
      }

      if (((callbackstruct *)clientdata)->data)
	{
	  arglist = Py_BuildValue("(OO)", obj, (PyObject *)(((callbackstruct *)clientdata)->data) ); 
	}
      else
	{
	  arglist = Py_BuildValue("(O)", obj ); 
	}

      result =  PyEval_CallObject(func, arglist);
   
      Py_XDECREF(result);
      if (PyErr_Occurred())
	{
	  PyErr_Print();
	}
   
      return /*void*/;
    }
  %}
  
%include "../include/widget.h"

%ignore Widget::mouse_pos(int&x, int&y);

%extend Widget {
	PyObject*
	mouse_pos()
	{
		int x, y;
		self->mouse_pos(x,y);
		PyObject* ret = PyTuple_New(2);
		PyObject* pyx = PyInt_FromLong(x);
		PyObject* pyy = PyInt_FromLong(y);
		PyTuple_SetItem(ret, 0, pyx);
		PyTuple_SetItem(ret, 1, pyy);
		return ret;
	}

  	void
    callback(PyObject *PyFunc, PyObject *PyWidget, PyObject *PyData = 0)
    {
      callbackstruct *cb = (callbackstruct*)self->user_data();

      if (cb) {
		cb->func = PyFunc;
		cb->widget = PyWidget;
		if (PyData) {
		  cb->data = PyData;
		}
			cb->widget = PyWidget;
	    }
      else
		cb = new callbackstruct( PyFunc, PyData, PyWidget );

      // Add a reference to new callback
      Py_INCREF(PyFunc);			
      Py_XINCREF(PyData);
      Py_XINCREF(PyWidget);

      self->callback(PythonCallBack, (void *)cb);
    }
 
   	void
    user_data(PyObject *PyData)
    {
      // Add a reference to new callback
      Py_XINCREF(PyData);
	
      callbackstruct *cb = (callbackstruct*)self->user_data();
      if (cb == NULL) {
		cb = new callbackstruct(0, PyData, (PyObject*)0);
		self->user_data((void *)cb);
      }
      else {
		cb->data = PyData;
      }
    }

  	PyObject* user_data() {
    PyObject *obj = 0;
    callbackstruct *cb = (callbackstruct*)self->user_data();
    if (cb) {
      if (cb->data) {
		obj = (PyObject*)cb->data;
      }
    }

    Py_XINCREF(obj);
    return obj;
  }
}


%typemap(in) PyObject *PyFunc {
  if (!PyCallable_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    return NULL;
  }
  $1 = $input;
}