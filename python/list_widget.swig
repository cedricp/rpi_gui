%feature("docstring") ::List_widget
"""
Button module
""" ;

%feature("director") ::List_widget;

%{
#include <../include/list_widget.h>
%}

CHANGE_OWNERSHIP(List_widget)

%include "../include/list_widget.h"
