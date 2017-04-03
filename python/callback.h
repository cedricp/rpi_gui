// This struct manages callbacks into Python.  It is stored as the
// user data portion of a widget, and the PythonCallback function is
// set as the callback.  PythonCallback unmarshalls the pointer to
// the Python function and Python user data and calls back into
// Python.
// Borrowed from PyFLTK

#ifndef Callback_h
#define Callback_h

#include <Python.h>

class callbackstruct
{
public:
  PyObject *func;
  PyObject *data;
  PyObject *widget;
  void     *type;
  PyObject *link;
  callbackstruct( PyObject *theFunc, PyObject *theData, PyObject *theWidget, PyObject *theLink = 0):
    func(theFunc),
    data(theData),
    widget(theWidget)
  {}
  callbackstruct( PyObject *theFunc, PyObject *theData, void *theType):
    func(theFunc),
    data(theData),
    widget(0),
    type(theType)
  {}

};

#endif
